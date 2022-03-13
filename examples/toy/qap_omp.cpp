#include "scool/simple_executor.hpp"
#include "scool/omp_executor.hpp"

#include "qap_common.hpp"
#include "qap_state.hpp"
#include "qap_task.hpp"
#include "scool/partitioner.hpp"

int main(int argc, char* argv[]) {

    read_qaplib_instance(argv[1], qap_task::n_, qap_task::F_, qap_task::D_);

    std::vector<int> res (qap_task::n_);

    std::iota(std::begin(res), std::end(res), 0);

    qap_task t(std::begin(res), std::end(res));
    qap_state st(qap_task::compute_cost(t.p_), t.p_);

    scool::omp_executor<qap_task, qap_state> exec;
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

    auto t1 = std::chrono::system_clock::now();

    exec.log().info() << "final result:" << std::endl;
    exec.state().print(exec.log().info());

    // std::chrono::duration<double> T = t1-t0;
    // exec.log().info() << "time to solution: " << T.count() << "s" << std::endl;



    return 0;
} // main