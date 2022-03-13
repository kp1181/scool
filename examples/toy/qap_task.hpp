/***
 *  $Id$
 **
 *  File: qap_task.hpp
 *  Created: Jan 25, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef QAP_TASK_HPP
#define QAP_TASK_HPP

#include <algorithm>
#include <istream>
#include <ostream>
#include <vector>

#include "libhungarian/hungarian.hpp"


// The lower bound is based on:
// P.M. Pardalos, J.V. Crouse: A parallel algorithm for the quadratic assignment problem
// https://doi.org/10.1145/76263.76302
class qap_task {
public:
    using solution_type = std::vector<int>;

    solution_type p_;
    int level_ = 0;

    qap_task() = default;

    template <typename Iter>
    qap_task(Iter first, Iter last, int l = 0) : p_(first, last), level_(l) { }


    template <typename ContextType, typename StateType>
    void process(ContextType& ctx, StateType& st) const {
        if (level_ == n_ - 1) {
            int cost = compute_cost(p_);

            if (cost <= st.best_cost) {
                st.best_cost = cost;
                st.best_solution = p_;
            }
        } else {
            int lb = compute_lower_bound(p_, level_);

            if (lb <= st.best_cost) {
                qap_task t;
                t.level_ = level_ + 1;
                t.p_ = p_;
                // now we generate tasks
                for (int i = level_; i < n_; ++i) {
                    std::swap(t.p_[level_], t.p_[i]);
                    ctx.push(t);
                    std::swap(t.p_[level_], t.p_[i]);
                }
            } // if lb
        } // else
    } // process

    void merge(const qap_task&) { }


    // helper functions
    static int compute_cost(const std::vector<int>& p, int k = n_) {
        int Z = 0;

        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < k; ++j) Z += F_[i * n_ + j] * D_[p[i] * n_ + p[j]];
        } // for i

        return Z;
    } // compute_cost

    static int compute_lower_bound(const std::vector<int>& p, int k) {
        auto alpha = p.data();     // k finished assignments
        auto beta = p.data() + k;  // m assignments to make

        int m = n_ - k;

        // construct linear assignment matrix (Eq. 4)
        std::vector<int> B(m * m, 0);

        // remaining assignments
        std::vector<int> Fp(m * m, 0);
        std::vector<int> Dp(m * m, 0); // Dp is transposed

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j)  Fp[i * m + j] = F_[(k + i) * n_ + k + j];
        }

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j)  Dp[i * m + j] = D_[beta[j] * n_ + beta[i]];
        }

        auto OP = ordered_product(Fp, Dp, m);

        // assignments made
        for (int i = 0; i < m; ++i) {
            for (int bi = 0; bi < m; ++bi) {
                int b = 0;
                for (int j = 0; j < k; ++j) {
                    b += (F_[(k + i) * n_ + j] * D_[beta[bi] * n_ + alpha[j]]);
                }
                B[i * m + bi] = 2 * b + OP[i * m + bi];
            } // for bi
        } // for i

        // final cost
        int lb = compute_cost(p, k) + linear_assignment_cost(B, m);

        return lb;
    } // compute_lower_bound

    static std::vector<int> ordered_product(std::vector<int>& F, std::vector<int>& D, int m) {
        std::vector<int> OP(m * m, 0);

        for (auto iter = std::begin(F), end = std::end(F); iter < end; iter += m) {
            std::sort(iter, iter + m);
        }

        // D must be transposed
        for (auto iter = std::begin(D), end = std::end(D); iter < end; iter += m) {
            std::sort(iter, iter + m, std::greater());
        }

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) {
                // we avoid diagonal elements
                for (int k = 1; k < m; ++k) OP[i * m + j] += (F[i * m + k] * D[j * m + k - 1]);
            }
        }

        return OP;
    } // ordered_product


    inline static int n_;                // number of facilities/locations
    inline static std::vector<int> F_;   // flow matrix
    inline static std::vector<int> D_;   // distance matrix

}; // class qap_task

inline bool operator==(const qap_task& t1, const qap_task& t2) {
    if (t1.level_ != t2.level_) return false;
    for (int i = 0; i < t1.level_; ++i) if (t1.p_[i] != t2.p_[i]) return false;
    return true;
} // operator==

inline std::ostream& operator<<(std::ostream& os, const qap_task& t) {
    int n = t.p_.size();
    os.write(reinterpret_cast<const char*>(&n), sizeof(n));
    os.write(reinterpret_cast<const char*>(&t.level_), sizeof(t.level_));
    os.write(reinterpret_cast<const char*>(t.p_.data()), n * sizeof(int));
    return os;
} // operator<<

inline std::istream& operator>>(std::istream& is, qap_task& t) {
    int n = 0;
    is.read(reinterpret_cast<char*>(&n), sizeof(n));
    is.read(reinterpret_cast<char*>(&t.level_), sizeof(t.level_));
    t.p_.resize(n);
    is.read(reinterpret_cast<char*>(t.p_.data()), n * sizeof(int));
    return is;
} // operator>>


namespace std {
  template <> struct hash<qap_task> {
      std::size_t operator()(const qap_task& t) const noexcept {
          std::size_t seed = t.level_;
          for (int i = 0; i < t.level_; ++i) {
              seed ^= t.p_[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
          }
          return seed;
      } // operator()
  }; // struct hash
} // namespace std


struct qap_partitioner {
    int operator()(const qap_task& t) const {
        const auto& p = t.p_;
        return 100 * p[0] + 10 * p[1] + p[2];
    } // operator()
}; // struct qap_partitioner

#endif // QAP_TASK_HPP
