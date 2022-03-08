/***
 *  $Id$
 **
 *  File: tsp_mpi.cpp
 *  Created: Sep 24, 2021
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2021 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

#include <mpi.h>

#include <mpi_executor.hpp>
#include <partitioner.hpp>

#include "tsp_common.hpp"
#include "tsp_state.hpp"
#include "tsp_task.hpp"


void tsp_search(MPI_Comm Comm) {
    int size, rank;

    MPI_Comm_size(Comm, &size);
    MPI_Comm_rank(Comm, &rank);

    std::vector<int> res(tsp_task::n_);
    std::iota(std::begin(res), std::end(res), 0);
    tsp_task t(std::begin(res), std::end(res));

    // we create a list of tasks to start with
    std::vector<tsp_task> tv;

    int d = (tsp_task::n_ / (2 * size)) + 1;

    for (int i = 0; i < tsp_task::n_; i += d) {
        t.i_range_[0] = i;
        t.i_range_[1] = std::min(i + d, tsp_task::n_ - 2);
        tv.push_back(t);
    }

    tsp_state st;

    scool::mpi_executor<tsp_task, tsp_state, tsp_partitioner, true> exec(Comm);

    //exec.log() = std::move(mpix::Logger(rank, "mpi"));
    exec.log().level(mpix::Logger::DEBUG);

    exec.init(std::begin(tv), std::end(tv), st);

    auto t0 = std::chrono::steady_clock::now();
    while (exec.step() > 0) { }
    auto t1 = std::chrono::steady_clock::now();

    exec.log().info() << "final result:" << std::endl;
    exec.state().print(exec.log().info());

    std::chrono::duration<double> T = t1 - t0;
    exec.log().info() << "time to solution: " << T.count() << "s" << std::endl;
} // tsp_search


int main(int argc, char* argv[]) {
    int tlevel, size, rank;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &tlevel);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (tlevel != MPI_THREAD_MULTIPLE) {
        if (rank == 0) std::cout << "error: insufficient threading support in MPI" << std::endl;
        return MPI_Finalize();
    }

    if (argc != 3) {
        if (rank == 0) std::cout << "usage: tsp_mpi bf tsplib_instance" << std::endl;
        return MPI_Finalize();
    }

    tsp_task::bf_ = std::atoi(argv[1]);

    if (tsp_task::bf_ < 2) {
        if (rank == 0) std::cout << "error: too small branching factor" << std::endl;
        return MPI_Finalize();
    }

    if (read_tsp_instance(argv[2], tsp_task::n_, tsp_task::D_, tsp_task::b_)) {
        tsp_search(MPI_COMM_WORLD);
    } else {
        if (rank == 0) std::cout << "error: could not read instance" << std::endl;
    }

    return MPI_Finalize();
} // main
