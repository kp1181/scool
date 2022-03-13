#include "scool/simple_executor.hpp"
#include "scool/omp_executor.hpp"

#include "bnsl_task.hpp"
#include "bnsl_state.hpp"
#include "scool/partitioner.hpp"

const int N = 2;
using task_type = bnsl_task<N>;
int main(int argc, char* argv[]) {
    
    int n = std::atoi(argv[1]);

    task_type::n = n;
    auto res = task_type::mps_list.read(n, argv[2]);

    //do some stuff

    // initialize remaining part of task_type
    task_type::opt_pa.resize(n);

    for (int xi = 0; xi < n; ++xi) {
        auto opt = task_type::mps_list.optimal(xi);
        task_type::opt_pa[xi] = {opt.pa, opt.s};
    }

    task_type t;
    bnsl_state<task_type::set_type> st;

    // bnsl tasks are never unique, they form poset lattice
    scool::omp_executor<task_type, bnsl_state<task_type::set_type>> exec;
    exec.init(t, st);

    auto t0 = std::chrono::system_clock::now();
    while (exec.step() > 0) { }

    auto t1 = std::chrono::system_clock::now();
    auto elapsed_par = std::chrono::duration<double>(t1 - t0);
    std::cout << "Time taken : " <<  elapsed_par.count() << std::endl;

    exec.log().info() << "final result:" << std::endl;
    exec.state().print(exec.log().info());

    return 0;
} // main
