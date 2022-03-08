/***
 *  $Id$
 **
 *  File: tsp_task.hpp
 *  Created: Mar 27, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020-2021 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef TSP_TASK_HPP
#define TSP_TASK_HPP

#include <istream>
#include <ostream>
#include <vector>


class tsp_task {
public:
    using solution_type = std::vector<int>;

    solution_type p_;
    int i_range_[2];


    tsp_task() = default;

    template <typename Iter>
    tsp_task(Iter first, Iter last, int i_min = 0, int i_max = n_ - 2)
        : p_(first, last) {
        i_range_[0] = i_min;
        i_range_[1] = i_max;
    } // tsp_task


    template <typename ContextType, typename StateType>
    void process(ContextType& ctx, StateType& st) const {
        tsp_task t(std::begin(p_), std::end(p_));

        int d = (n_ / bf_) + 2;
        int nsucc = 0;

        for (auto i = i_range_[0]; i < i_range_[1]; ++i) {
            for (auto j = i + 2; j < n_; ++j) {
                opt2_swap(p_, i, j, t.p_);

                auto cost = compute_cost(t.p_);

                if (cost < st.best_cost) {
                    st.best_cost = cost;
                    st.best_solution = t.p_;

                    for (int k = 0; k < n_; k += d) {
                        t.i_range_[0] = k;
                        t.i_range_[1] = std::min(k + d, n_ - 2);
                        ctx.push(t);
                    }

                    nsucc++;
                }

                if (nsucc == bf_) break;
            } // for j
        } // for i
    } // process

    void merge(const tsp_task& t) { }


    static float compute_cost(const solution_type& p) {
        auto n = p.size();
        float S = D_[p[n - 1] * n_ + p[0]];
        for (auto i = 0; i < n - 1; ++i) S += D_[p[i] * n_ + p[i + 1]];
        return S;
    } // compute_cost


    // 2-OPT solver
    static void opt2_swap(const solution_type& p, int j, int k, solution_type& buf) {
        buf.resize(p.size());
        auto n = buf.size();
        for (auto i = 0; i < j; ++i) buf[i] = p[i];
        for (auto i = j; i < k; ++i) buf[i] = p[j + k - i - 1];
        for (auto i = k; i < n; ++i) buf[i] = p[i];
    } // opt2_swap


    inline static int n_;
    inline static std::vector<float> D_;

    inline static std::vector<float> b_;
    inline static int bf_;

}; // class tsp_task

inline bool operator==(const tsp_task& t1, const tsp_task& t2) {
    if ((t1.i_range_[0] != t2.i_range_[0]) || (t1.i_range_[1] != t2.i_range_[1])) return false;
    for (auto i = 0; i < tsp_task::n_; ++i) if (t1.p_[i] != t2.p_[i]) return false;
    return true;
} // operator==

inline std::ostream& operator<<(std::ostream& os, const tsp_task& t) {
    os.write(reinterpret_cast<const char*>(t.i_range_), 2 * sizeof(int));
    os.write(reinterpret_cast<const char*>(t.p_.data()), tsp_task::n_ * sizeof(int));
    return os;
} // operator<<

inline std::istream& operator>>(std::istream& is, tsp_task& t) {
    is.read(reinterpret_cast<char*>(t.i_range_), 2 * sizeof(int));
    t.p_.resize(tsp_task::n_);
    is.read(reinterpret_cast<char*>(t.p_.data()), tsp_task::n_ * sizeof(int));
    return is;
} // operator>>


namespace std {
  template <> struct hash<tsp_task> {
      std::size_t operator()(const tsp_task& t) const noexcept {
          return t.p_[t.i_range_[0]];
      } // operator()
  }; // struct hash
} // namespace std


struct tsp_partitioner {
    int operator()(const tsp_task& t) const {
        return t.p_[t.i_range_[0]];
    } // operator()
}; // struct tsp_partitioner

#endif // TSP_TASK_HPP
