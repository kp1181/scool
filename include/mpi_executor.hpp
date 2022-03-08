/***
 *  $Id$
 **
 *  File: mpi_executor.hpp
 *  Created: Feb 04, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *          Zainul Abideen Sayed <zsayed@buffalo.edu>
 *  Copyright (c) 2020-2021 SCoRe Group
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef MPI_EXECUTOR_HPP
#define MPI_EXECUTOR_HPP

#include <atomic>
#include <cmath>
#include <future>
#include <mutex>
#include <random>
#include <thread>

#include <mpi.h>

#include "impl.hpp"
#include "mpi_impl.hpp"
#include "partitioner.hpp"
#include "utility.hpp"

#include "mpix/logger.hpp"
#include "mpix/mpi_types.hpp"


using mpix::Logger;


namespace scool {

  template <typename ExecutorType, bool Unique = true>
  class mpi_context {
  public:
      using task_type = typename ExecutorType::task_type;

      explicit mpi_context(ExecutorType& exec) : exec_(exec) { }

      int iteration() const { return exec_.iteration(); }

      void push(const task_type& t) {
          if constexpr (Unique) impl::add_to<Unique>(exec_.next_, t);
          else {
              // group by partitioner
              // TODO: use a efficient mod mapper
              auto rank = exec_.pt_(t) % static_cast<std::size_t>(exec_.size_);
              impl::add_to<Unique>(exec_.next_[rank], t);
          }
      } // push

  private:
      mpi_context(const mpi_context&) = delete;
      void operator=(const mpi_context&) = delete;

      ExecutorType& exec_;

  }; // mpi_context


  // Class: mpi_executor_base__
  // Base class to derive <mpi_executor>.
  // Do not use directly!
  template <typename TaskType, typename StateType, typename Partitioner>
  class mpi_executor_base__ {
  protected:
      using req_data_type = scool::impl::bitmap::storage_type;

  public:
      using task_type = TaskType;
      using state_type = StateType;
      using partitioner = Partitioner;

      // ids encoding requests used by mpi_executor
      // REQ_NONE - empty answer
      // REQ_ASK  - request to steal work
      // REQ_ANS  - answer to steal request
      // REQ_FIN  - notification to finalize execution
      // REQ_RDC  - request to participate in reduction
      enum request_type : req_data_type { REQ_NONE = 0, REQ_FIN = 1, REQ_ASK = 2, REQ_ANS = 3, REQ_RDC = 4 };

      // tags used for identifying background communication
      enum REQUEST_TAGS : int { REQ_TAG = 101, ANS_TAG = 102, RDC_TAG = 103 };


      explicit mpi_executor_base__(MPI_Comm Comm = MPI_COMM_WORLD, int seed = -1)
          : Comm_(Comm) {
          MPI_Comm_size(Comm_, &size_);
          MPI_Comm_rank(Comm_, &rank_);

          tokens_.resize(size_);

          if (seed == -1) {
              std::random_device rd;
              rng0_.seed(rd());
              rng1_.seed(rd());
          } else {
              rng0_.seed(seed);
              rng1_.seed(seed);
          }

          log_.rank(rank_);
      } // mpi_executor_base__

      virtual ~mpi_executor_base__() {
          // block and wait for everybody else to finish
          MPI_Barrier(Comm_);

          // initiate terminating the helper thread
          m_send_message_head__(REQ_FIN, rank_, Comm_hlp_);

          // now we can finish
          hlp_th_.join();

          MPI_Comm_free(&this->Comm_hlp_);
      } // ~mpi_executor_base__


      // Function: log
      mpix::Logger& log() { return log_; }

      // Function: iteration
      int iteration() const { return giter_; }

      // Function: state
      const state_type& state() { return gst_; }


  protected:
      // logger
      const std::string NAME_{"MPIExecutor"};
      mpix::Logger log_{-1};

      // global attributes
      // total tasks, local tasks, remote tasks, variance
      long long int gcount_[4] = { 0, 0, 0, 0 };

      int giter_ = 0;
      state_type gst_;
      state_type lst_; // local state
      state_type rst_; // received state

      // tokens to piggyback current stealing state
      scool::impl::bitmap tokens_;
      std::mutex tokens_mtx_;

      std::atomic_flag passive_;
      std::mutex rdc_mtx_;

      // MPI env
      MPI_Comm Comm_;

      int size_;
      int rank_;

      MPI_Comm Comm_hlp_;
      std::thread hlp_th_;

      // list of ranks to steal from
      // also keeps track of empty victims
      std::vector<int> vranks_;

      std::mt19937 rng0_;
      std::mt19937 rng1_;

      // this method reduces gst_ over the b-tree
      void m_reduce_and_forward__(MPI_Comm Comm) {
          rdc_mtx_.lock();
          gst_ += rst_;
          rst_.identity();
          lst_.identity();

          if ((rank_ > 0) && !(gst_ == lst_)) {
              int target =  (rank_ - 1) >> 1;

              m_send_message_head__(REQ_RDC, target, Comm);
              mpi_impl::serialize_and_send(gst_, target, RDC_TAG, Comm);
              lst_ = gst_;
          } // if rank

          rdc_mtx_.unlock();
      } // m_reduce_and_forward__


      // these methods implement basic protocol for task stealing
      // every message contains request id and current status of tokens
      std::pair<req_data_type, int> m_receive_message_head__(int Tag, MPI_Comm Comm) {
          std::vector<req_data_type> msg(1 + tokens_.storage_size());

          // receiving a request
          MPI_Status stat;
          MPI_Recv(msg.data(), msg.size(), mpix::MPI_Type<req_data_type>(), MPI_ANY_SOURCE, Tag, Comm, &stat);
          int target = stat.MPI_SOURCE;

          if (msg[0] == REQ_FIN) return { REQ_FIN, target };

          if (msg[0] == REQ_RDC) return { REQ_RDC, target };

          if (tokens_mtx_.try_lock()) {
              tokens_.OR(msg.data() + 1);
              if (msg[0] == REQ_NONE) tokens_.set(target);
              tokens_mtx_.unlock();
          }

          return { msg[0], target };
      } // m_receive_message_head__

      std::pair<req_data_type, int> m_receive_message_head__(MPI_Comm Comm) {
          return m_receive_message_head__(REQ_TAG, Comm);
      } // m_receive_message_head__

      void m_send_message_head__(request_type req, int target, int Tag, MPI_Comm Comm) {
          std::vector<req_data_type> msg(1 + tokens_.storage_size());

          // request id
          msg[0] = req;

          if (req == REQ_ASK) {
              // tokens
              auto tp = tokens_.storage();

              tokens_mtx_.lock();
              std::copy(tp, tp + tokens_.storage_size(), std::begin(msg) + 1);
              tokens_mtx_.unlock();
          }

          // here we go
          MPI_Send(msg.data(), msg.size(), mpix::MPI_Type<req_data_type>(), target, Tag, Comm);
      } // m_send_message_head__

      void m_send_message_head__(request_type req, int target, MPI_Comm Comm) {
          m_send_message_head__(req, target, REQ_TAG, Comm);
      } // m_send_message_head__


      void m_set_vranks__() {
          vranks_.resize(size_ - 1);
          int pos = 0;
          for (int i = 0; i < size_; ++i) if (i != rank_) vranks_[pos++] = i;
      } // m_set_vranks__


      template <typename Context>
      int m_steal_tasks__(MPI_Comm Comm, Context& ctx) {
          m_set_vranks__();

          int count = 0;

          std::vector<task_type> T;
          T.reserve(1024);

          auto out = std::back_inserter(T);
          int end = size_ - 1;

          // it seems we need this to avoid bias in load
          std::uniform_int_distribution<int> udist(0, end - 1);

          while (end > 0) {
              // randomly selects a victim
              int pos = udist(rng0_);
              int target = vranks_[pos];

              // abort steal if victim is passive
              tokens_mtx_.lock();
              auto vt = tokens_[target];
              tokens_mtx_.unlock();

              if (vt == true) {
                  end = end - 1;
                  udist = std::uniform_int_distribution<int>(0, end - 1);
                  std::swap(vranks_[pos], vranks_[end]);
                  continue;
              }

              // send request
              m_send_message_head__(REQ_ASK, target, Comm);
              auto [req, _] = m_receive_message_head__(ANS_TAG, Comm);

              if (req == REQ_ANS) {
                  // receive and deserialize the tasks
                  mpi_impl::receive_and_deserialize<task_type>(out, target, ANS_TAG, Comm);

                  for (auto& x : T) {
                      x.process(ctx, gst_);
                      count++;
                  }

                  T.clear();
              } else {
                  // remove target from consideration
                  end = end - 1;
                  udist = std::uniform_int_distribution<int>(0, end - 1);
                  std::swap(vranks_[pos], vranks_[end]);
              }
          } // while end

          passive_.test_and_set();
          m_reduce_and_forward__(Comm);

          return count;
      } // m_steal_tasks__


  private:
      mpi_executor_base__(const mpi_executor_base__&) = delete;
      void operator=(const mpi_executor_base__&) = delete;

  }; // class mpi_executor_base__


  // General mpi_executor (default choice)

  // Class: mpi_executor
  // Parallel <Executor> model built on top of Message Passing Interface.
  // The execution of supersteps is handled by MPI ranks within specified MPI communicator.
  // The executor uses variety of optimizations, e.g., work stealing, message piggybacking, etc.
  // to efficiently execute superstep and maintain shared state.
  //
  // Parameters:
  // Unique - if *true*, the search space is assumed to be a tree (i.e., tasks are unique), otherwise it is a graph.
  template <typename TaskType, typename StateType, typename Partitioner = simple_partitioner<TaskType>, bool Unique = false>
  class mpi_executor : public mpi_executor_base__<TaskType, StateType, Partitioner> {
  public:
      // Type: task_type
      // Alias to the user provided *TaskType* type representing a task.
      using task_type = TaskType;

      // Type: state_type
      // Alias to the user provided *StateType* type representing a global shared state.
      using state_type = StateType;

      // Type: partitioner
      // Alias to user provided *Partitioner* type representing a task partitioner.
      using partitioner = Partitioner;


      // Function: mpi_executor
      // Creates and launches mpi_executor runtime.
      //
      // Parameters:
      //   Comm - MPI communicator to that the executor should use for communication.
      //   seed - random number generator seed. The executor uses random numbers in
      //          several different places. The seed should be configured to value
      //          different than -1 only in cases where deterministic behavior
      //          between different executions is desired.
      explicit mpi_executor(MPI_Comm Comm = MPI_COMM_WORLD, int seed = -1)
          : mpi_executor_base__<TaskType, StateType, Partitioner>(Comm, seed), ctx_(*this),
            curr_(this->size_), next_(this->size_), curr_mtx_(this->size_), porder_(this->size_) {
          // we have separate communicator for requests processing
          MPI_Comm_dup(this->Comm_, &this->Comm_hlp_);
          this->hlp_th_ = std::thread(&mpi_executor::m_request_listener__, this, this->Comm_hlp_);

          MPI_Barrier(this->Comm_);

          this->log().info(this->NAME_) << "ready with " << this->size_ << " ranks" << std::endl;
      } // mpi_executor


      void init(const task_type& t, const state_type& st, const partitioner& pt = partitioner()) {
          pt_ = pt;
          std::vector<task_type> v{t};
          init(std::begin(v), std::end(v), st);
      } // init

      // Function: init
      template <typename Iter>
      void init(Iter first, Iter last, const state_type& st, const partitioner& pt = partitioner()) {
          pt_ = pt;

          for (; first != last; ++first) {
              auto rank = pt_(*first) % static_cast<std::size_t>(this->size_);
              if (rank == this->rank_) impl::add_to<Unique>(curr_[rank], *first);
          }

          curr_size_ = curr_[this->rank_].size();
          this->gst_ = st;

          long long int count = curr_[this->rank_].size();
          MPI_Allreduce(&count, &this->gcount_[0], 1, MPI_LONG_LONG_INT, MPI_SUM, this->Comm_);

          MPI_Barrier(this->Comm_);
      } // init


      // Function: step
      long long int step() {
          this->log().info(this->NAME_) << "processing " << this->gcount_[0]
                                        << " tasks, superstep " << this->giter_
                                        << "..." << std::endl;

          long long int global_tasks  = this->gcount_[0];
          long long int count[4] = {0, 0, 0, 0};
          long long int local_task;

          // process local queue
          count[1] = m_process_local_queue__();

          // go into stealing mode
          count[2] = this->m_steal_tasks__(this->Comm_hlp_, ctx_);

          // update size
          count[0] = 0;
          for (auto& x : next_) count[0] += x.size();

          // calculate standard deviation
          local_task = count[1] + count[2];

          double mean = (double) global_tasks / this->size_;
          double local_sq_diff = (local_task - mean) * (local_task - mean);

          count[3] = std::llround(local_sq_diff);

          // take care of global state
          MPI_Allreduce(count, this->gcount_, 4, MPI_LONG_LONG_INT, MPI_SUM, this->Comm_);

          float sd = std::sqrt(this->gcount_[3] / this->size_);
          float p_sd = (sd / mean) * 100;

          this->log().debug(this->NAME_) << "local tasks: " << this->gcount_[1]
                                         << ", remote tasks: " << this->gcount_[2]
                                         << ", standard deviation: "<< p_sd <<"%"
                                         << std::endl;

          long long int processed_task =  this->gcount_[1] + this->gcount_[2];
          if (processed_task != global_tasks) {
              this->log().error() << "something went very wrong, task numbers mismatch!" << std::endl;
          }

          // get local queues in proper shape
          // at this stage all ranks are in sync
          this->tokens_.reset();
          curr_.swap(next_);
          curr_size_ = count[0];

          mpi_impl::reduce(this->gst_, this->Comm_);
          this->gst_.identity();
          mpi_impl::broadcast(this->gst_, this->Comm_);

          this->giter_++;

          return this->gcount_[0];
      } // step


  private:
      using local_storage_type = std::vector<phmap::node_hash_set<task_type>>;

      friend mpi_context<mpi_executor, Unique>;
      mpi_context<mpi_executor, Unique> ctx_;

      // this runs in listener thread
      void m_request_listener__(MPI_Comm Comm) {
          const int NUM_TRY = 3;

          do {
              auto [req, target] = this->m_receive_message_head__(Comm);

              if (req == this->REQ_FIN) break;

              if (req == this->REQ_ASK) {
                  bool ans = false;

                  if (curr_size_.load() > 0) {
                      // perhaps we have something that can be stolen
                      auto step = this->rng1_() % (this->size_ - 1);

                      // let's make several attempts
                      // first is always work local to target (thief)
                      for (int i = 0; i < NUM_TRY; ++i) {
                          int pos = (target + i * step) % (this->size_);

                          if (curr_mtx_[pos].try_lock()) {
                              // by the time we got lock, there may be no more work to do
                              if (curr_[pos].empty()) {
                                  curr_mtx_[pos].unlock();
                                  break;
                              }

                              // serializing a batch
                              this->m_send_message_head__(this->REQ_ANS, target, this->ANS_TAG, Comm);
                              mpi_impl::serialize_and_send(curr_[pos].begin(), curr_[pos].end(), target, this->ANS_TAG, Comm);

                              int sz = curr_[pos].size();
                              curr_[pos].clear();

                              curr_size_.fetch_sub(sz);
                              curr_mtx_[pos].unlock();

                              ans = true;
                              break;
                          }
                      } // for i
                  } // if curr_size_

                  if (ans == false) this->m_send_message_head__(this->REQ_NONE, target, this->ANS_TAG, Comm);
              } // if req = REQ_ASK

          } while (true);
      } // m_request_listener__

      int m_process_local_queue__() {
          int count = 0;

          // set the order of local processing
          // local work is always first
          std::iota(std::begin(porder_), std::end(porder_), 0);
          std::shuffle(std::begin(porder_), std::end(porder_), this->rng0_);
          std::swap(porder_[0], porder_[this->rank_]);

          // this loop could be optimized to terminate earlier
          // and with fewer curr_size_ loads
          do {
              for (int rank = 0; rank < this->size_; ++rank) {
                  auto pos = porder_[rank];

                  if (curr_mtx_[pos].try_lock()) {
                      if (!curr_[pos].empty()) {
                          auto iter = curr_[pos].begin();
                          auto end = curr_[pos].end();

                          for (; iter != end; ++iter, ++count) {
                              iter->process(ctx_, this->gst_);
                          }

                          int sz = curr_[pos].size();
                          curr_[pos].clear();

                          curr_size_.fetch_sub(sz);
                      } // if curr_[pos]

                      curr_mtx_[pos].unlock();
                      if (curr_size_.load() <= 0) break;
                  } // if try_lock
              } // for rank
          } while (curr_size_.load() > 0);

          return count;
      } // m_process_local_queue__

      /*
        void m_deserialize_to_queue__(std::vector<char>&& data) {
        scool::mpi_impl::deserialize_and_add<task_type, Unique>(data, next_[this->rank_]);
        } // m_deserialize_to_queue__

        void m_exchange_queues__() {
        this->log().debug(this->NAME_) << "exchanging queues..." << std::endl;

        std::vector<char> data;
        std::vector<char> rdata;

        std::vector<int> scount(this->size_, 0);
        std::vector<int> sdispl(this->size_, 0);

        std::future<void> ft;

        for (int i = 0; i < this->size_; ++i) {
        data.clear();

        if (i != this->rank_) {
        scool::mpi_impl::serialize(next_[i].begin(), next_[i].end(), data);
        next_[i].clear();
        }

        int buf = data.size();
        MPI_Gather(&buf, 1, MPI_INT, scount.data(), 1, MPI_INT, i, this->Comm_);

        if (i == this->rank_) {
        std::partial_sum(std::begin(scount), std::end(scount) - 1, std::begin(sdispl) + 1);
        rdata.resize(sdispl.back() + scount.back());
        }

        MPI_Gatherv(data.data(), data.size(), MPI_CHAR, rdata.data(), scount.data(), sdispl.data(), MPI_CHAR, i, this->Comm_);

        if (i == this->rank_) {
        ft = std::async(std::launch::async, &mpi_executor::m_deserialize_to_queue__, this, rdata);
        }
        } // for i

          // we can swap now
          ft.wait();

          curr_.swap(next_);
          curr_size_.store(curr_[this->rank_].size());
          } // m_exchange_queues__
      */

      std::atomic_int_fast32_t curr_size_;
      std::vector<std::mutex> curr_mtx_;

      local_storage_type curr_;
      local_storage_type next_;

      std::vector<int> porder_;
      Partitioner pt_;

  }; // class mpi_executor


  // specialized mpi_executor for cases where tasks are guaranteed to be unique

  // Class: mpi_executor
  // This is template specialization for when the search space is assumed to be a tree,
  // (i.e., tasks are guaranteed to be unique).
  template <typename TaskType, typename StateType, typename Partitioner>
  class mpi_executor<TaskType, StateType, Partitioner, true>
      : public mpi_executor_base__<TaskType, StateType, Partitioner> {
  public:
      using task_type = TaskType;
      using state_type = StateType;
      using partitioner = Partitioner;


      explicit mpi_executor(MPI_Comm Comm = MPI_COMM_WORLD, int seed = -1)
          : mpi_executor_base__<TaskType, StateType, Partitioner>(Comm, seed), ctx_(*this) {

          // we have separate communicator for requests processing
          MPI_Comm_dup(this->Comm_, &this->Comm_hlp_);

          // helper thread that acts as a request listener
          this->hlp_th_ = std::thread(&mpi_executor::m_request_listener__, this, this->Comm_hlp_);

          // we have to synchronize, wait untill everybody is ready
          MPI_Barrier(this->Comm_);

          this->log().info(this->NAME_) << "ready with " << this->size_ << " ranks" << std::endl;
      } // mpi_executor


      void init(const task_type& t, const state_type& st,
                const partitioner& pt = partitioner()) {
          std::vector<task_type> v{t};
          init(std::begin(v), std::end(v), st);
      } // init

      template <typename Iter>
      void init(Iter first, Iter last, const state_type& st,
                const partitioner& pt = partitioner()) {
          if (this->rank_ == 0) {
              this->gcount_[0] = 0;

              for (; first != last; ++first, ++this->gcount_[0]) {
                  impl::add_to<true>(curr_, *first);
              }
          }

          this->gst_ = st;
          this->lst_ = st;
          this->rst_ = st;

          hlp_pos_ = curr_.size();
          curr_pos_ = 0;

          goal_post_ = std::ceil(LOCAL_QUEUE_SIZE * curr_.size());

          // to avoid data race between early stealing threads
          MPI_Barrier(this->Comm_);

          MPI_Bcast(&this->gcount_, 4, MPI_LONG_LONG_INT, 0, this->Comm_);
      } // init


      long long int step() {
          this->log().info(this->NAME_) << "processing " << this->gcount_[0]
                                        << " tasks, superstep " << this->giter_
                                        << "..." << std::endl;

          long long int global_tasks  = this->gcount_[0];
          long long int count[4] = {0, 0, 0, 0};
          long long int local_task;

          MPI_Barrier(this->Comm_);

          this->passive_.clear();
          this->lst_ = this->gst_;
          this->rst_ = this->gst_;

          // process local queue
          count[1] = m_process_local_queue__();

          // go into stealing mode
          count[2] = this->m_steal_tasks__(this->Comm_hlp_, ctx_);

          // take care of global state
          count[0] = next_.size();

          // calculate standard deviation
          local_task = count[1] + count[2];

          double mean = (double) global_tasks / this->size_;
          double local_sq_diff = (local_task - mean) * (local_task - mean);

          count[3] = std::llround(local_sq_diff);

          MPI_Barrier(this->Comm_);
          MPI_Allreduce(count, this->gcount_, 4, MPI_LONG_LONG_INT, MPI_SUM, this->Comm_);

          // get local queues in proper shape
          this->tokens_.reset();

          curr_.swap(next_);
          next_.clear();

          hlp_pos_ = curr_.size();
          curr_pos_ = 0;
          goal_post_ = std::ceil(LOCAL_QUEUE_SIZE * curr_.size());

          this->gst_.identity();

          float sd = std::sqrt(this->gcount_[3] / this->size_);
          float p_sd = (sd / mean) * 100;

          long long int processed_task =  this->gcount_[1] + this->gcount_[2];
          if (processed_task != global_tasks) {
              this->log().error() << "something went very wrong, task numbers mismatch!" << std::endl;
          }

          this->log().debug(this->NAME_) << "local tasks: " << this->gcount_[1]
                                         << ", remote tasks: " << this->gcount_[2]
                                         << ", standard deviation: "<< std::setprecision(3) << p_sd << "%"
                                         << std::endl;

          MPI_Barrier(this->Comm_);
          mpi_impl::broadcast(this->gst_, this->Comm_);

          this->giter_++;

          return this->gcount_[0];
      } // step


  private:
      using local_storage_type = std::vector<task_type>;

      friend mpi_context<mpi_executor, true>;
      mpi_context<mpi_executor, true> ctx_;


      // this runs in listener thread
      void m_request_listener__(MPI_Comm Comm) {
          do {
              auto [req, target] = this->m_receive_message_head__(Comm);

              if (req == this->REQ_FIN) break;

              if (req == this->REQ_RDC) {
                  state_type tmp_st = this->rst_;
                  mpi_impl::receive_and_deserialize(tmp_st, target, this->RDC_TAG, Comm);
                  this->rdc_mtx_.lock();
                  this->rst_ += tmp_st;
                  this->rdc_mtx_.unlock();

                  if (this->passive_.test()) {
                      if (this->rank_ != 0) this->gst_.identity();
                      this->m_reduce_and_forward__(Comm);
                  }
                  continue;
              }

              if (req == this->REQ_ASK) {
                  int start, end, batch, count;

                  // calculating the batch size based on active ranks
                  // more active ranks higher batch size (10.0% -> 1.0%)
                  count = this->size_ - this->tokens_.count();
                  task_batch_size = std::max((count / static_cast<float>(this->size_)) * 0.1, 0.01);

                  mtx_.lock();
                  batch = std::ceil((hlp_pos_ - goal_post_) * task_batch_size);
                  start = hlp_pos_ - batch;

                  if ((start <= goal_post_) || ((start - curr_pos_) < MIN_TASK_BATCH)) {
                      mtx_.unlock();
                      this->m_send_message_head__(this->REQ_NONE, target, this->ANS_TAG, Comm);
                      continue;
                  }

                  end = hlp_pos_;
                  hlp_pos_ = start;

                  mtx_.unlock();

                  this->m_send_message_head__(this->REQ_ANS, target, this->ANS_TAG, Comm);

                  auto first = std::next(curr_.begin(), hlp_pos_);
                  auto last = std::next(curr_.begin(), end);

                  mpi_impl::serialize_and_send(first, last, target, this->ANS_TAG, Comm);
              } // if req == REQ_ASK

          } while (true);
      } // m_request_listener__


      // this runs in main thread
      int m_process_local_queue__() {
          int S = 0;

          // 1. process local portion of the queue
          while (curr_pos_ < goal_post_) {
              curr_[curr_pos_].process(ctx_, this->gst_);
              curr_pos_++;
              S++;
          }

          // 2. process shared portion of the queue
          int pos = 0;

          while (true) {
              mtx_.lock();
              if (curr_pos_ == hlp_pos_) {
                  mtx_.unlock();
                  break;
              }

              pos = curr_pos_;
              curr_pos_ = std::min(curr_pos_ + MIN_TASK_BATCH, hlp_pos_);

              mtx_.unlock();

              for (; pos < curr_pos_; ++pos, ++S) curr_[pos].process(ctx_, this->gst_);
          }

          return S;
      } // m_process_local_queue__


      // queue indexes
      int goal_post_ = 0;
      int curr_pos_ = 0;
      int hlp_pos_ = 0;
      // stealing chunk size
      float task_batch_size = 0.10;

      std::mutex mtx_;

      // work queues
      local_storage_type curr_;
      local_storage_type next_;

      // local queue size
      const float LOCAL_QUEUE_SIZE = 0.20;

      // local thread - locks 10 tasks at a time
      // helper thread - allow stealing if tasks > MIN_TASK_BATCH
      const int MIN_TASK_BATCH = 10;

  }; // class mpi_executor

} // namespace scool

#endif // MPI_EXECUTOR_HPP
