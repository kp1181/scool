/***
 *  $Id$
 **
 *  File: task.hpp
 *  Created: Feb 13, 2019
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2019 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef TASK_HPP
#define TASK_HPP

// Class: Task
// This is a code template demonstrating requirements (i.e., interface specification)
// that a type must satisfy to model the *TaskType* concept.
class Task {
public:
    // Constructor: Task
    // Any *TaskType* model must provide default constructor.
    // While other constructors may be provided for the end-user's convenience,
    // they are ignored by the runtime system. In the essence, *TaskType* must be:
    // <DefaultConstructible: https://en.cppreference.com/w/cpp/named_req/DefaultConstructible>,
    // <CopyConstructible: https://en.cppreference.com/w/cpp/named_req/CopyConstructible>,
    // <CopyAssignable: https://en.cppreference.com/w/cpp/named_req/CopyAssignable>.
    Task();

    // Function: process
    // Implements the task processing routine. This function is called
    // implicitly by the runtime system. When executing, a task may create
    // new tasks, which can be pushed to the runtime by calling <Context::push()>
    // method from *ctx*. A task can also read/write the global state *st*.
    //
    // Parameters:
    // ctx - Object of *ContextType* model representing the current runtime.
    // st  - Object of *StateType* model maintaining a local view of the global state.
    template <typename ContextType, typename StateType>
    void process(ContextType& ctx, StateType& st);

    // Function: merge
    // Implements the task merging routine. In some cases, the same task may be generated
    // via two different execution paths, especially when running in parallel. In such
    // cases, two equal tasks would exist unnecessarily increasing computational work.
    // When such situation is detected, the runtime merges tasks x and y
    // by calling x.merge(y) and discarding task y. The order in which merging is executed
    // is unspecified, hence x.merge(y) and y.merge(x) must be semantically equivalent.
    //
    // Parameters:
    // t - Object of *TaskType* model that should be merged with the calling task.
    void merge(const Task& t);

}; // class Task

// Function: operator==
// Implements equality comparison of tasks. The function should return
// true if and only if two tasks are semantically identical. It is recommended
// that the routine is lightweight.
bool operator==(const Task& t1, const Task& t2);

// Function: operator<<
// Implements the task serialization routine. Tasks might be serialized
// by the runtime at different points of execution, e.g., to be staged in
// a persistent storage or to be sent over a network. It is recommended
// that the routine is lightweight, and generates compact representation
// of the object (e.g., without unnecessary decorations).
//
// Parameters:
// os - Output stream to store serialized object.
// t  - Object to serialize.
std::ostream& operator<<(std::ostream& os, const Task& t);

// Function: operator>>
// Implements the task deserialization routine. The routine must
// complement serialization routine as implemented in <operator<<()>.
//
// Parameters:
// is - Input stream to deserialize from.
// t  - Object to deserialize into.
std::istream& operator>>(std::istream& is, Task& t);


// Namespace: C++ Standard Namespace
namespace std {
  // Class: hash
  // Any *TaskType* model must provide a custom specialization
  // of the standard C++ <hash: https://en.cppreference.com/w/cpp/utility/hash>.
  // The hashing function is necessary to enable efficient task storage management.
  template <> struct hash<Task> {
      // Function: operator()
      // Implements hashing routine according to the standard hash
      // <requirements: https://en.cppreference.com/w/cpp/utility/hash>.
      // It is recommended that the routine is lightweight.
      std::size_t operator()(const Task& t) const noexcept;
  }; // struct hash
} // namespace std

#endif // TASK_HPP
