/***
 *  $Id$
 **
 *  File: context.hpp
 *  Created: Feb 14, 2019
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2019 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef CONTEXT_HPP
#define CONTEXT_HPP

// Class: Context
// Specification of the Context interface.
// Context provides a minimal interface that programmers can use
// to interact with the runtime system.
class Context {
public:
    // Type: task_type
    // Alias to the user provided type representing a task.
    using task_type = Task;

    // Function: iteration
    //
    // Returns:
    //   the iteration (i.e., superstep) the runtime is currently in.
    int iteration() const;

    // Function: push
    // Add a task to the runtime system. The task will be processed,
    // i.e., its function <Task::process()> will be called, in the following superstep.
    //
    // Parameters:
    // t - Object of *TaskType* model representing a task to be added to the execution.
    //     The task will processed in the next superstep.
    void push(const task_type& t);

}; // class Context

#endif // CONTEXT_HPP
