/***
 *  $Id$
 **
 *  File: qap_state.hpp
 *  Created: Jan 30, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *          Zainul Abideen Sayed <zsayed@buffalo.edu>
 *  Copyright (c) 2020-2021 SCoRe Group
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef QAP_STATE_HPP
#define QAP_STATE_HPP

#include <istream>
#include <limits>
#include <ostream>
#include <vector>


struct qap_state {
    qap_state() = default;
    qap_state(int bc, const std::vector<int>& bs) : best_cost(bc), best_solution(bs) { }

    void identity() { }

    void operator+=(const qap_state& st) {
        if (st.best_cost < best_cost) {
            best_cost = st.best_cost;
            best_solution = st.best_solution;
        }
    } // operator+=

    bool operator==(const qap_state& st) const { return (st.best_cost == best_cost); }

    void print(std::ostream& os) const {
        os << best_cost;
        for (auto x : best_solution) os << " " << x;
        os << std::endl;
    } // print

    int best_cost = std::numeric_limits<int>::max();
    std::vector<int> best_solution;
}; // qap_state

inline std::ostream& operator<<(std::ostream& os, const qap_state& st) {
    int n = st.best_solution.size();
    os.write(reinterpret_cast<const char*>(&st.best_cost), sizeof(st.best_cost));
    os.write(reinterpret_cast<const char*>(&n), sizeof(n));
    os.write(reinterpret_cast<const char*>(st.best_solution.data()), n * sizeof(int));
    return os;
} // operator<<

inline std::istream& operator>>(std::istream& is, qap_state& st) {
    int n = 0;
    is.read(reinterpret_cast<char*>(&st.best_cost), sizeof(st.best_cost));
    is.read(reinterpret_cast<char*>(&n), sizeof(n));
    st.best_solution.resize(n);
    is.read(reinterpret_cast<char*>(st.best_solution.data()), n * sizeof(int));
    return is;
} // operator>>

#endif // QAP_STATE_HPP
