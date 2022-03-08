/***
 *  $Id$
 **
 *  File: executor.hpp
 *  Created: Jan 31, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

// Class: Executor
// Specification of the Executor interface.
// Executor abstracts SCoOL's computational model.
template <typename TaskType, typename StateType, typename Partitioner>
class Executor {
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

    // Function: log
    // Exposes logger that a user can utilize for reporting to a console.
    Logger& log();

    // Function: init
    // Initializes the executor with a start task and the initial global state.
    // This is the default way of providing starting point for search space exploration.
    // Task passed to this function is processed in the first superstep.
    //
    // Parameters:
    // t - Object of *TaskType* model representing a task from which the execution should begin.
    // st - Object of *StateType* model that represents the initial global state.
    void init(const task_type& t, const state_type& st);

    // Function: init
    // Initializes the executor with a start task, the initial global state and specific partitioner.
    // Task passed to this function is processed in the first superstep.
    //
    // Parameters:
    // t - Object of *TaskType* model representing a task from which the execution should begin.
    // st - Object of *StateType* model that represents the initial global state.
    // pt - Object of *PartitionerType* model that should be used to partition tasks.
    void init(const task_type& t, const state_type& st, const partitioner& pt);

    // Function: init
    // Initializes the executor with a list of start tasks and the initial global state.
    // Tasks passed to this function are processed in the first superstep.
    //
    // Parameters:
    // [first, last) - Range containing input tasks.
    //                 The iterator type *InputIterator* must be at least
    //                 <InputIterator: https://en.cppreference.com/w/cpp/named_req/InputIterator>
    //                 and its underlying value type must be a <task_type>.
    // st - Object of *StateType* model that represents the initial global state.
    template <typename InputIterator>
    void init(InputIterator first, InputIterator last, const state_type& st);

    // Function: init
    // Initializes the executor with a list of start tasks, the initial global state and specific partitioner.
    // Tasks passed to this function are processed in the first superstep.
    //
    // Parameters:
    // [first, last) - Range containing input tasks.
    //                 The iterator type *InputIterator* must be at least
    //                 <InputIterator: https://en.cppreference.com/w/cpp/named_req/InputIterator>
    //                 and its underlying value type must be a <task_type>.
    // st - Object of *StateType* model that represents the initial global state.
    // pt - Object of *PartitionerType* model that should be used to partition tasks.
    template <typename InputIterator>
    void init(InputIterator first, InputIterator last, const state_type& st, const partitioner& pt);

    // Function: state
    //
    // Returns:
    //   the current global view of the shared state.
    const state_type& state();

    // Function: iteration
    //
    // Returns:
    //   the current iteration (i.e., superstep).
    //   The iteration counter starts at 0,
    //   and it is increased by one only at the end execution of <step>.
    int iteration() const;

    // Function: step
    // This is the main routine triggering superstep execution.
    // The routine is blocking, and finishes only if the subsequent
    // superstep may start executing.
    //
    // Returns:
    //   the number of tasks to process in the next superstep.
    long long int step();

}; // class Executor

#endif // EXECUTOR_HPP
