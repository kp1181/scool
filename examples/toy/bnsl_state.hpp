/***
 *  $Id$
 **
 *  File: bnsl_state.hpp
 *  Created: Jan 13, 2022
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef BNSL_STATE_HPP
#define BNSL_STATE_HPP

#include <istream>
#include <ostream>
#include <vector>

#include "bit_util.hpp"
#include "limits.hpp"


template <typename set_type> struct bnsl_state {
    bnsl_state() = default;

    void identity() { }

    void operator+=(const bnsl_state& st) {
        if (st.score < score) {
            score = st.score;
            path = st.path;
        }
    } // operator+=

    bool operator==(const bnsl_state& st) const { return (tid == st.tid); }

    void print(std::ostream& os) const {
        os << "score: " << score << ", order:";
        for (int x : path) os << " " << x;
        os << std::endl;
    } // print

    set_type tid = set_empty<set_type>();
    double score = SABNA_DBL_INFTY;
    std::vector<uint8_t> path;

}; // struct bnsl_state

template <typename set_type>
inline std::ostream& operator<<(std::ostream& os, const bnsl_state<set_type>& st) {
    int n = st.path.size();
    os.write(reinterpret_cast<const char*>(&st.tid), sizeof(st.tid));
    os.write(reinterpret_cast<const char*>(&st.score), sizeof(st.score));
    os.write(reinterpret_cast<const char*>(&n), sizeof(n));
    os.write(reinterpret_cast<const char*>(st.path.data()), n * sizeof(uint8_t));
    return os;
} // operator<<

template <typename set_type>
inline std::istream& operator>>(std::istream& is, bnsl_state<set_type>& st) {
    int n = 0;
    is.read(reinterpret_cast<char*>(&st.tid), sizeof(st.tid));
    is.read(reinterpret_cast<char*>(&st.score), sizeof(st.score));
    is.read(reinterpret_cast<char*>(&n), sizeof(n));
    st.path.resize(n);
    is.read(reinterpret_cast<char*>(st.path.data()), n * sizeof(uint8_t));
    return is;
} // operator>>

#endif // BNSL_STATE_HPP
