/***
 *  $Id$
 **
 *  File: tsp_state.hpp
 *  Created: Sep 23, 2021
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *          Zainul Abideen Sayed <zsayed@buffalo.edu>
 *  Copyright (c) 2021 SCoRe Group
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef TSP_STATE_HPP
#define TSP_STATE_HPP

#include <istream>
#include <limits>
#include <ostream>
#include <vector>


struct tsp_state {
    tsp_state() = default;

    void identity() { }

    void operator+=(const tsp_state& st) {
        if (st.best_cost < best_cost) {
            best_cost = st.best_cost;
            best_solution = st.best_solution;
        }
    } // operator+=

    bool operator==(const tsp_state& st) const { return (st.best_cost == best_cost); }

    void print(std::ostream& os) const {
        os << best_cost;
        for (auto x : best_solution) os << " " << x;
        os << std::endl;
    } // print


    float best_cost = std::numeric_limits<float>::max();
    std::vector<int> best_solution;

}; // tsp_state

inline std::ostream& operator<<(std::ostream& os, const tsp_state& st) {
    int n = st.best_solution.size();
    os.write(reinterpret_cast<const char*>(&st.best_cost), sizeof(st.best_cost));
    os.write(reinterpret_cast<const char*>(&n), sizeof(n));
    os.write(reinterpret_cast<const char*>(st.best_solution.data()), n * sizeof(float));
    return os;
} // operator<<

inline std::istream& operator>>(std::istream& is, tsp_state& st) {
    int n = 0;
    is.read(reinterpret_cast<char*>(&st.best_cost), sizeof(st.best_cost));
    is.read(reinterpret_cast<char*>(&n), sizeof(n));
    st.best_solution.resize(n);
    is.read(reinterpret_cast<char*>(st.best_solution.data()), n * sizeof(float));
    return is;
} // operator>>

#endif // TSP_STATE_HPP
