#include "scool/simple_executor.hpp"
#include "scool/omp_executor.hpp"

#include "toy_task.hpp"
#include "toy_state.hpp"
#include "scool/partitioner.hpp"


int main(int argc, char* argv[]) {
    toy_task::n = 9;
    scool::omp_executor<toy_task, toy_state> exec;

    exec.init(toy_task(true), toy_state());

    auto t0 = std::chrono::system_clock::now();
    while (exec.step() > 0) { }

    auto t1 = std::chrono::system_clock::now();
    auto elapsed_par = std::chrono::duration<double>(t1 - t0);
    std::cout << "Time taken : " <<  elapsed_par.count() << std::endl;

    return 0;
} // main
