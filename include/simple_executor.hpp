/***
 *  $Id$
 **
 *  File: simple_executor.hpp
 *  Created: Jan 31, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef SIMPLE_EXECUTOR_HPP
#define SIMPLE_EXECUTOR_HPP

#include "impl.hpp"
#include "partitioner.hpp"

#include "jaz/logger.hpp"


using jaz::Logger;


namespace scool {

  template <typename ExecutorType, bool Unique>
  class simple_context {
  public:
      using task_type = typename ExecutorType::task_type;

      explicit simple_context(ExecutorType& exec) : exec_(exec) { }

      int iteration() const { return exec_.iteration(); }

      // take a task and add it to the execution environment
      void push(const task_type& t) { impl::add_to<Unique>(exec_.next_, t); }

  private:
      simple_context(const simple_context&) = delete;
      void operator=(const simple_context&) = delete;

      ExecutorType& exec_;

  }; // class simple_context


  // Class: simple_executor
  // Simple sequential executor. This is efficient but very basic model
  // of the <Executor> concept.
  //
  // Parameters:
  // Unique - if *true*, the search space is assumed to be a tree (i.e., tasks are unique), otherwise it is a graph.
  template <typename TaskType, typename StateType, typename Partitioner = simple_partitioner<TaskType>, bool Unique = true>
  class simple_executor {
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


      // Function: simple_executor
      simple_executor() : ctx_(*this) { }

      // Function: log
      jaz::Logger& log() { return log_; }

      // Function: init
      template <typename Iter>
      void init(Iter first, Iter last, const state_type& st,
                const partitioner& pt = partitioner()) {
          // add task to current layer
          for (; first != last; ++first) impl::add_to<Unique>(curr_, *first);
          st_ = st;
      } // init

      // Function: init
      void init(const task_type& t, const state_type& st,
                const partitioner& pt = partitioner()) {
          std::vector<task_type> v{t};
          init(std::begin(v), std::end(v), st);
      } // init

      // Function: iteration
      int iteration() const { return iter_; }

      // Function: state
      const state_type& state() { return st_; }

      // Function: step
      long long int step() {
          log_.info("SimpleExecutor") << "processing " << curr_.size() << " tasks, superstep " << iter_ << "..." << std::endl;

          m_process_current__();
          st_.identity();

          // exchange the queue and clear for next superstep
          std::swap(curr_, next_);
          next_.clear();

          iter_++;

          return curr_.size();
      } // step


  private:
      using task_storage_type = typename std::conditional_t<Unique, std::vector<task_type>, phmap::node_hash_set<task_type>>;

      void m_process_current__() {
          for (auto it = std::begin(curr_), end = std::end(curr_); it != end; ++it) {
              it->process(ctx_, st_);
          }
      } // m_process_current__

      friend simple_context<simple_executor, Unique>;
      simple_context<simple_executor, Unique> ctx_;

      state_type st_;
      int iter_ = 0;

      task_storage_type curr_;
      task_storage_type next_;

      jaz::Logger log_;

  }; // class simple_executor

}; // namespace scool

#endif // SIMPLE_EXECUTOR_HPP
