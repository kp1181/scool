/***
 *  $Id$
 **
 *  File: omp_executor.hpp
 *  Created: Mar 31, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020-2021 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef OMP_EXECUTOR_HPP
#define OMP_EXECUTOR_HPP

#include <numeric>
#include <omp.h>

#include "impl.hpp"
#include "omp_impl.hpp"
#include "partitioner.hpp"

#include "jaz/logger.hpp"
#include "omp_process_table.hpp"
#include "unistd.h"


using jaz::Logger;


namespace scool {

  template <typename ExecutorType, bool Unique>
  class omp_context {
  public:
      using task_type = typename ExecutorType::task_type;

      explicit omp_context(ExecutorType& exec) : exec_(exec) { }

      int iteration() const { return exec_.iteration(); }

      // this will be always called from parallel region
      void push(const task_type& t) {
          int tid = omp_get_thread_num();
          if constexpr (!Unique) {
              exec_.next_.insert(t);
           }
           else{
              impl::add_to<Unique>(exec_.next_[tid], t);
           }
          
      } // push

  private:
      omp_context(const omp_context&) = delete;
      void operator=(const omp_context&) = delete;

      ExecutorType& exec_;

  }; // class omp_context


  // Class: omp_executor_base__
  // Base class to derive <omp_executor>.
  // Do not use directly!
  template <typename TaskType, typename StateType, typename Partitioner = simple_partitioner<TaskType>>
  class omp_executor_base__ {
  public:
      using task_type = TaskType;
      using state_type = StateType;
      using partitioner = Partitioner;


      omp_executor_base__() = default;


      // Function: log
      jaz::Logger& log() { return log_; }

      // Function: iteration
      int iteration() const { return iter_; }

      // Function: state
      const state_type& state() { return gst_; }


  protected:
      template <bool Unique, typename Iter, typename Store>
      void m_init__(Iter first, Iter last, Store& store, const state_type& st) {
          this->ntasks_ = 0;

          for (; first != last; ++first, ++this->ntasks_) {
              impl::add_to<Unique>(store[0], *first);
          }

          this->gst_ = st;
      } // m_init__

      template <typename Iter, typename Context>
      void m_process_group__(Iter first, Iter last, Context& ctx) {
          int tid = omp_get_thread_num();
          for (; first != last; ++first) first->process(ctx, sts_[tid]);
      } // m_process_group__

      void m_reduce_state__() {
          // here we go with the global state
          //log().debug(NAME_) << "reducing to global state..." << std::endl;

          for (auto& st : sts_) gst_ += st;
          gst_.identity();
          for (auto& st : sts_) st = gst_;
      } // m_reduce_state__


      const std::string NAME_ = "OMPExecutor";
      jaz::Logger log_;

      std::vector<state_type> sts_;
      state_type gst_;

      long long int ntasks_ = 0;
      int iter_ = 0;

  private:
      omp_executor_base__(const omp_executor_base__&) = delete;
      void operator=(const omp_executor_base__&) = delete;

  }; // class omp_executor_base__


  // General omp_executor (default choice)

  // Class: omp_executor
  // Parallel <Executor> model built on top of OpenMP.
  // The execution of superstep is handled by multiple threads managed via OMP.
  // The properties of the executor, e.g., the number of threads, can be customized
  // using standard OMP environment variables.  In the current implementation,
  // partitioner is not being used. *This implementation is incomplete!*
  //
  // Parameters:
  // Unique - if *true*, the search space is assumed to be a tree (i.e., tasks are unique), otherwise it is a graph.
  template <typename TaskType, typename StateType, typename Partitioner = simple_partitioner<TaskType>, bool Unique = false>
  class omp_executor : public omp_executor_base__<TaskType, StateType, Partitioner> {
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


      // Function: omp_executor
      omp_executor() : ctx_(*this) {
          std::cout << "In first one" <<std::endl;
          #pragma omp parallel
          #pragma omp single
          {
              p = omp_get_num_threads();

              this->sts_.resize(p);

              B_ = 99991;
              curr_.init(B_, p);
              next_.init(B_, p);

              this->log().info(this->NAME_) << "ready with " << p << " threads" << std::endl;
          }
      } // omp_executor


      // TODO - Know the usage
      // Function: init
      template <typename Iter>
      void init(Iter first, Iter last, const state_type& st,
                const partitioner& pt = partitioner()) {
          this->template m_init__<Unique>(first, last, next_, st);
      } // init

      // Function: init
      void init(const task_type& t, const state_type& st,
                const partitioner& pt = partitioner()) {
            next_.insert(t);
            this->ntasks_ = 1;
            this->gst_ = st;
      } // init


      // Function: step
      long long int step() {
          this->log().info(this->NAME_) << "processing " << this->ntasks_
                                        << " tasks, superstep " << this->iter_
                                        << "..." << std::endl;

          std::swap(curr_, next_);

          //next_.soft_clear();
          next_.lazy_clear();
          this->iter_++;
          m_process__();

          this->m_reduce_state__();

          next_.reconcile();

          this->ntasks_ = 0;
          this->ntasks_ += next_.master_view_size();
          
          return this->ntasks_;
      } // step


  private:
      using local_storage_type = omp_process_table<task_type, std::hash<task_type>, std::allocator>;

      void m_process__() {
            int p = curr_.num_views();
            state_type* sts = this->sts_.data();


            #pragma omp parallel default(none) shared( curr_, ctx_, sts, next_, std::cout) firstprivate(p)
            #pragma omp single nowait
            {
                
                for(auto t = curr_.begin(); t!=curr_.end();++t){
                    {
                        #pragma omp task
                        {
                            int tid = omp_get_thread_num();
                            //std::cout << "tid " << tid << " / "<< omp_get_num_threads() << std::endl;
                            t->process(ctx_, sts[tid]);   
                        }
  
                    }
                }
            }
          
      } // m_process__

      friend omp_context<omp_executor, Unique>;
      omp_context<omp_executor, Unique> ctx_;

      int B_= 0;
      int p = 0;

      local_storage_type curr_;
      local_storage_type next_;

  }; // class omp_executor


  // Specialized omp_executor for cases where tasks are guaranteed to be unique

  // Class: omp_executor
  // This is template specialization for when the search space is assumed to be a tree,
  // (i.e., tasks are guaranteed to be unique). This implementation is fully functional.
  template <typename TaskType, typename StateType, typename Partitioner>
  class omp_executor<TaskType, StateType, Partitioner, true>
      : public omp_executor_base__<TaskType, StateType, Partitioner> {
  public:
      using task_type = TaskType;
      using state_type = StateType;
      using partitioner = Partitioner;


      omp_executor() : ctx_(*this) {
          #pragma omp parallel
          #pragma omp single
          {
              int p = omp_get_num_threads();

              this->sts_.resize(p);

              curr_.resize(p);
              next_.resize(p);

              this->log().info(this->NAME_) << "ready with " << p << " threads" << std::endl;
          }
      } // omp_executor


      template <typename Iter>
      void init(Iter first, Iter last, const state_type& st,
                const partitioner& pt = partitioner()) {
          this->template m_init__<true>(first, last, next_, st);
      } // init

      void init(const task_type& t, const state_type& st,
                const partitioner& pt = partitioner()) {
          std::vector<task_type> v{t};
          this->template m_init__<true>(std::begin(v), std::end(v), next_, st);
      } // init


      long long int step() {
          this->log().info(this->NAME_) << "processing " << this->ntasks_
                                        << " tasks, superstep " << this->iter_
                                        << "..." << std::endl;
          std::swap(curr_, next_);

          this->iter_++;

          m_process__();
          this->m_reduce_state__();

          this->ntasks_ = 0;
          for (auto& ts : next_) this->ntasks_ += ts.size();

          return this->ntasks_;
      } // step


  private:
      using task_storage_type = std::vector<task_type>;
      using local_storage_type = std::vector<task_storage_type>;

      void m_process__() {
          int p = curr_.size();
          state_type* sts = this->sts_.data();

          #pragma omp parallel default(none) shared(p, curr_, ctx_, sts)
          {
              #pragma omp single nowait
              {
                  for (int i = 0; i < p; ++i) {
                      int end = curr_[i].size();
                      auto* curr = curr_[i].data();

                      #pragma omp taskloop firstprivate(end) untied
                      for (int j = 0; j < end; ++j) {
                          int tid = omp_get_thread_num();
                          curr[j].process(ctx_, sts[tid]);
                      }
                  }

                  for (int i = 0; i < p; ++i) curr_[i].clear();
              }
          }
      } // m_process__

      friend omp_context<omp_executor, true>;
      omp_context<omp_executor, true> ctx_;

      local_storage_type curr_;
      local_storage_type next_;

  }; // class omp_executor

} // namespace scool

#endif // OMP_EXECUTOR_HPP
