/***
 *  $Id$
 **
 *  File: bnsl_mpi.cpp
 *  Created: Jan 13, 2022
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
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

#include "bnsl_task.hpp"
#include "bnsl_state.hpp"


const int N = 2;
using task_type = bnsl_task<N>;
using partitioner_type = bnsl_hyper_partitioner<N>;

void bnsl_search(MPI_Comm Comm) {
    int rank, size;

    MPI_Comm_rank(Comm, &rank);
    MPI_Comm_size(Comm, &size);

    task_type t;
    bnsl_state<task_type::set_type> st;

    // bnsl tasks are never unique, they form poset lattice
    scool::mpi_executor<task_type, bnsl_state<task_type::set_type>, partitioner_type, false> exec(Comm);

    //exec.log() = std::move(mpix::Logger(rank, "mpi"));
    exec.log().level(mpix::Logger::DEBUG);

    exec.init(t, st, partitioner_type(3));
    auto t0 = std::chrono::steady_clock::now();

    for (int i = 0; i <= t.n; ++i) exec.step();

    auto t1 = std::chrono::steady_clock::now();

    exec.log().info() << "final result:" << std::endl;
    exec.state().print(exec.log().info());

    std::chrono::duration<double> T = t1 - t0;
    exec.log().info() << "time to solution: " << T.count() << "s" << std::endl;
} // bnsl_search


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
        if (rank == 0) std::cout << "usage: bnsl_mpi n mpsfile" << std::endl;
        return MPI_Finalize();
    }

    int n = std::atoi(argv[1]);

    task_type::n = n;
    auto res = task_type::mps_list.read(n, argv[2]);

    if (res.first) {
        // initialize remaining part of task_type
        task_type::opt_pa.resize(n);

        for (int xi = 0; xi < n; ++xi) {
            auto opt = task_type::mps_list.optimal(xi);
            task_type::opt_pa[xi] = {opt.pa, opt.s};
        }

        // let's go searching
        bnsl_search(MPI_COMM_WORLD);
    } else {
        if (rank == 0) std::cout << "error: " << res.second << std::endl;
    }

    return MPI_Finalize();
} // main
