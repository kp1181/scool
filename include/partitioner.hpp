/***
 *  $Id$
 **
 *  File: partitioner.hpp
 *  Created: Feb 17, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef PARTITIONER_HPP
#define PARTITIONER_HPP

namespace scool {

  // Class: simple_partitioner
  // Naive implementation of the <Partitioner> concept that performs no partitioning.
  template <typename TaskType>
  struct simple_partitioner {
      // Function: operator()
      //
      // Returns:
      //   always 0.
      int operator()(const TaskType& t) const { return 0; }
  }; // struct simple_partitioner

} // namespace scool

#endif // PARTITIONER_HPP
