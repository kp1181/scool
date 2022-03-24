/***
 *  $Id$
 **
 *  File: bnsl_task.hpp
 *  Created: Jan 13, 2022
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2022 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef BNSL_TASK_HPP
#define BNSL_TASK_HPP

#include <iostream>
#include <vector>

#include "scool/jaz/hash.hpp"

#include "MPSList.hpp"


template <int N = 2> struct bnsl_task {
    static const int PATH_SIZE = set_max_item<N>() + 1;

    using set_type = uint_type<N>;

    set_type id = set_empty<set_type>();
    double score = 0.0;
    uint8_t path[PATH_SIZE];


    template <typename ContextType, typename StateType>
    void process(ContextType& ctx, StateType& st) const {
        int l = ctx.iteration();
        int sz = set_size(id);

        // last task holds the solution
        if (sz == n) {
            if (score < st.score) {
                st.tid = id;
                st.score = score;
                st.path.resize(n);
                std::copy(path, path + n, std::begin(st.path));
            }
        }

        // if task is not ready for processing, we simply defer
        if (l < sz) {
            ctx.push(*this);
            return;
        }

        bnsl_task t;

        // consider all possible extensions of the current node
        for (int xi = 0; xi < n; ++xi) {
            if (in_set(id, xi)) continue;

            t.id = set_add(id, xi);
            t.score = score + mps_list.find(xi, id).s;

            std::copy(path, path + l, t.path);
            t.path[l] = xi;

            // apply OPE
            ope(l + 1, t);

            ctx.push(t);
        } // for xi

    } // process

    void merge(const bnsl_task& t) {
        if (t.score < score) {
            score = t.score;
            std::copy(t.path, t.path + PATH_SIZE, path);
        }
    } // merge


    void ope(int l, bnsl_task& node) const {
        bool extend = true;

        for (int i = 0; ((i < n) && extend); ++i) {
            extend = false;

            for (int xi = 0; xi < n; ++xi) {
                if (!in_set(node.id, xi) && is_superset(node.id, opt_pa[xi].first)) {
                    extend = true;
                    node.id = set_add(node.id, xi);
                    node.score += opt_pa[xi].second;
                    node.path[l] = xi;
                    l++;
                }
            } // for xi

        } // for i

    } // ope


    inline static int n;
    inline static MPSList<N> mps_list;
    inline static std::vector<std::pair<uint_type<N>, double>> opt_pa;

}; // struct bnsl_task


template <int N>
inline bool operator==(const bnsl_task<N>& t1, const bnsl_task<N>& t2) {
    return (t1.id == t2.id);
} // operator==

template <int N>
inline std::ostream& operator<<(std::ostream& os, const bnsl_task<N>& t) {
    os.write(reinterpret_cast<const char*>(&t.id), sizeof(t.id));
    os.write(reinterpret_cast<const char*>(&t.score), sizeof(t.score));
    os.write(reinterpret_cast<const char*>(&t.path), sizeof(t.path));
    return os;
} // operator<<

template <int N>
inline std::istream& operator>>(std::istream& is, bnsl_task<N>& t) {
    is.read(reinterpret_cast<char*>(&t.id), sizeof(t.id));
    is.read(reinterpret_cast<char*>(&t.score), sizeof(t.score));
    is.read(reinterpret_cast<char*>(&t.path), sizeof(t.path));
    return is;
} // operator>>


namespace std {

  template <int N> struct hash<bnsl_task<N>> {

      std::size_t operator()(const bnsl_task<N>& t) const noexcept {
          return h(t.id);
      } // operator()

      uint_hash h;
  }; // struct hash

} // namespace std


template <int N> struct bnsl_hyper_partitioner {
    explicit bnsl_hyper_partitioner(int b = 1) : b_(1) { }

    unsigned long long operator()(const bnsl_task<N>& t) const {
        auto val = shift_right(t.id, b_);
        uint_hash h;
        return h(val);
    } // operator()

    int b_ = 1;

}; // struct bnsl_hyper_partitioner


#endif // BNSL_TASK_HPP
