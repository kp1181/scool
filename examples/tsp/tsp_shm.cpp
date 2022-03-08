/***
 *  $Id$
 **
 *  File: tsp_shm.cpp
 *  Created: Mar 30, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

#include <simple_executor.hpp>
#include <omp_executor.hpp>

#include "tsp_common.hpp"
#include "tsp_state.hpp"
#include "tsp_task.hpp"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "usage: tsp_shm bf tsplib_instance" << std::endl;
        return 0;
    }

    auto t0 = std::chrono::steady_clock::now();

    tsp_task::bf_ = std::atoi(argv[1]);

    if (tsp_task::bf_ < 2) {
        std::cout << "error: too small branching factor" << std::endl;
        return -1;
    }

    if (read_tsp_instance(argv[2], tsp_task::n_, tsp_task::D_, tsp_task::b_)) {
        std::vector<int> res(tsp_task::n_);
        std::iota(std::begin(res), std::end(res), 0);

        tsp_task t(std::begin(res), std::end(res));
        tsp_state st;

        scool::omp_executor<tsp_task, tsp_state, scool::simple_partitioner<tsp_task>, true> exec;
        exec.log().level(Logger::DEBUG);

        exec.init(t, st);

        t0 = std::chrono::steady_clock::now();

        while (exec.step() > 0) { }

        auto t1 = std::chrono::steady_clock::now();

        exec.log().info() << "result: " << exec.state() << std::endl;

        std::chrono::duration<double> T = t1 - t0;
        std::cout << "time to solution: " << T.count() << "s" << std::endl;
    } else {
        std::cout << "error: could not read instance" << std::endl;
        return -1;
    }

    auto t2 = std::chrono::steady_clock::now();

    std::chrono::duration<double> T = t2 - t0;
    std::cout << "time: " << T.count() << "s" << std::endl;

    std::_Exit(0);
    return 0;
} // main
