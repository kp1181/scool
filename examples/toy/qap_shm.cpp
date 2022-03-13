/***
 *  $Id$
 **
 *  File: qap_shm.cpp
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

#include <omp_executor.hpp>
#include <simple_executor.hpp>

#include "qap_common.hpp"
#include "qap_state.hpp"
#include "qap_task.hpp"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "usage: qap_seq qaplib_instance" << std::endl;
        return 0;
    }

    auto t0 = std::chrono::steady_clock::now();

    if (read_qaplib_instance(argv[1], qap_task::n_, qap_task::F_, qap_task::D_)) {
        std::vector<int> res(qap_task::n_);
        std::iota(std::begin(res), std::end(res), 0);

        qap_task t(std::begin(res), std::end(res));
        qap_state st(qap_task::compute_cost(t.p_), t.p_);

        scool::omp_executor<qap_task, qap_state, scool::simple_partitioner<qap_task>, true> exec;
        exec.log().level(Logger::DEBUG);

        exec.init(t, st);

        t0 = std::chrono::steady_clock::now();

        while (exec.step() > 0) { }

        auto t1 = std::chrono::steady_clock::now();

        exec.log().info() << "final result:" << std::endl;
        exec.state().print(exec.log().info());

        std::chrono::duration<double> T = t1 - t0;
        exec.log().info() << "time to solution: " << T.count() << "s" << std::endl;
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
