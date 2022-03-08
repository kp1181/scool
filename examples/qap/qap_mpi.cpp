/***
 *  $Id$
 **
 *  File: qap_seq.cpp
 *  Created: Feb 21, 2019
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2019 Jaroslaw Zola
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

#include "qap_common.hpp"
#include "qap_state.hpp"
#include "qap_task.hpp"


void qap_search(MPI_Comm Comm) {
    int rank;
    MPI_Comm_rank(Comm, &rank);
    std::vector<int> res (qap_task::n_);

    std::iota(std::begin(res), std::end(res), 0);

    qap_task t(std::begin(res), std::end(res));
    qap_state st(qap_task::compute_cost(t.p_), t.p_);

    scool::mpi_executor<qap_task, qap_state, qap_partitioner, false> exec(Comm);

    //exec.log() = std::move(mpix::Logger(rank, "mpi"));
    exec.log().level(mpix::Logger::DEBUG);

    exec.init(t, st);
    auto t0 = std::chrono::steady_clock::now();

    long long int total_task = 0;

    do {
        auto start = std::chrono::steady_clock::now();
        total_task = exec.step();
        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> t = end - start;

        exec.state().print(exec.log().info());
        exec.log().info() << "time between step: " << t.count() << "s" << std::endl;
    } while (total_task > 0);

    auto t1 = std::chrono::steady_clock::now();

    exec.log().info() << "final result:" << std::endl;
    exec.state().print(exec.log().info());

    std::chrono::duration<double> T = t1 - t0;
    exec.log().info() << "time to solution: " << T.count() << "s" << std::endl;
} // qap_search


int main(int argc, char* argv[]) {
    int tlevel, size, rank;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &tlevel);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (tlevel != MPI_THREAD_MULTIPLE) {
        if (rank == 0) std::cout << "error: insufficient threading support in MPI" << std::endl;
        return MPI_Finalize();
    }

    if (argc != 2) {
        if (rank == 0) std::cout << "usage: qap_mpi qaplib_instance" << std::endl;
        return MPI_Finalize();
    }

    if (read_qaplib_instance(argv[1], qap_task::n_, qap_task::F_, qap_task::D_)) {
        qap_search(MPI_COMM_WORLD);
    } else {
        if (rank == 0) std::cout << "error: could not read instance" << std::endl;
    }

    return MPI_Finalize();
} // main
