/***
 *  $Id$
 **
 *  File: MPSList.hpp
 *  Created: May 24, 2017
 *
 *  Author: Subhadeep Karan <skaran@buffalo.edu>
 *  Copyright (c) 2017 SCoRe Group http://www.score-group.org/
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 */

#ifndef MPSLIST_HPP
#define MPSLIST_HPP

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "bit_util.hpp"
#include "limits.hpp"


template <int N>
class MPSList {
public:
    using set_type = uint_type<N>;

    struct MPSNode {
        double s;
        set_type pa;
    }; // struct MPSNode


    int n() const { return n_; }

    int size() const {
        int S = 0;
        for (auto& x : mps_list_) S += x.size();
        return S;
    } // size

    void init(int n) { n_ = n; mps_list_.resize(n_, std::vector<MPSNode>{}); }

    const MPSList::MPSNode& optimal(int xi) const { return mps_list_[xi].back();  }

    int max_pa_size(int xi) const {
        int pa_s = 0;
        for (auto& x : mps_list_[xi]) pa_s = std::max(pa_s, set_size(x.pa));
        return pa_s;
    } // max_pa_size

    const MPSNode& find(int xi, set_type u)  const {
        int idx = mps_list_[xi].size() - 1;
        for (; idx > 0 && !is_superset(u, mps_list_[xi][idx].pa); --idx);
        if (idx >= 0) return mps_list_[xi][idx];
        return null_mps_;
    } // find

    set_type find_all_pa(int xi, set_type u) const {
        set_type all_pa = set_empty<set_type>();
        int idx = mps_list_[xi].size() - 1;
        for (; idx > 0; --idx) {
            for (const int xj : as_vector(mps_list_[xi][idx].pa)) { all_pa = set_add(all_pa, xj); }
            if (is_superset(u, mps_list_[xi][idx].pa)) { break; }
        }
        return all_pa;
    } // find_all_pa

    const MPSNode& find_sample(int xi, set_type u, const double rand_val) const {
        std::vector<int> vec_idx;

        for (int i = mps_list_[xi].size() - 1; i >= 0; --i) {
            if (!is_superset(u, mps_list_[xi][i].pa)) { continue; }
            vec_idx.push_back(i);
        }

        return mps_list_[xi][static_cast<int>(rand_val * vec_idx.size())];
    } // find_sample

    bool insert(int xi, set_type u, double s) {
        auto it = std::upper_bound(std::begin(mps_list_[xi]), std::end(mps_list_[xi]), MPSNode{s, set_type{0}}, [](const MPSNode& x, const MPSNode& y) { return x.s > y.s; });
        mps_list_[xi].insert(it, MPSNode{s, u});
        return true;
    } // insert

    bool insert(int xi, set_type u, double s, int idx) {
        while (idx > 0 && mps_list_[xi][idx].s < s) --idx;
        mps_list_[xi].insert(std::begin(mps_list_[xi]) + idx, MPSNode{s, u});
        return true;
    } // insert

    void erase(int xi, set_type u) {
        auto it = std::begin(mps_list_[xi]);
        auto it_end = std::end(mps_list_[xi]);
        for (; it != it_end && it->pa != u; ++it);
        if (it != it_end) mps_list_[xi].erase(it);
    } // erase

    // std::vector<set_type> adjacency() const {
    //     std::vector<set_type> adj(n_, set_empty<set_type>());
    //     for (int xi = 0; xi < n_; ++xi) {
    //         for (auto& nd : mps_list_[xi]) {
    //             for (auto& xj : as_vector(nd.pa)) adj[xj] = set_add(adj[xj], xi);
    //         }
    //     }
    //     return adj;
    // } // adjacency

    // std::vector<set_type> all_better_pa(const int xi, const set_type& pa) const {
    //     std::vector<set_type> better_pa;

    //     int idx = mps_list_[xi].size() - 1;
    //     for (; idx > 0; --idx) {
    //         better_pa.push_back(mps_list_[xi][idx].pa);
    //         if (is_superset(pa, mps_list_[xi][idx].pa)) { break; }
    //     }

    //     return better_pa;
    // } // all_better_pa

    void verify_rebuild() {
        for (int xi = 0; xi < n_; ++xi) {
            std::set<int> idx;

            int mps_size = mps_list_[xi].size();

            for (int i = mps_size - 1; i > 0; --i) {
                auto node1 = mps_list_[xi][i];

                for (int j = mps_size - 1; j >= 0; --j) {
                    if (i == j) { continue; }
                    auto node2 = mps_list_[xi][j];
                    if (is_superset(node2.pa, node1.pa) && (node2.s >= node1.s)) { idx.insert(j); }
                }
            } // for i

            if (idx.empty()) { continue; }

            // we remove from the end so that erase is not undefined
            for (auto it = std::rbegin(idx); it != std::rend(idx); ++it) {
                mps_list_[xi].erase(mps_list_[xi].begin() + (*it));
            }
        } // for xi
    } // verify_rebuild

    std::vector<set_type> adjacency_matrix() const {
        set_type empty_set = set_empty<set_type>();
        set_type full_set = set_full<set_type>(n_);
        return m_subset_adjacency_matrix__(empty_set, full_set);
    } //adjacency_matrix

    std::vector<set_type> subset_adjacency_matrix(const set_type& given, const set_type& target) const { return m_subset_adjacency_matrix__(given, target); }


    void map_variables(const std::vector<int>& order) {
        std::vector<std::vector<MPSNode>> temp(n_);

        for (int i = 0; i < n_; ++i) {
            for (auto& x : mps_list_[i]) {
                set_type pa = set_empty<set_type>();
                for (auto xi : as_vector(x.pa)) pa = set_add(pa, order[xi]);
                temp[order[i]].emplace_back(MPSNode{x.s, pa});
            }
        } // for i

        mps_list_ = std::move(temp);
    } // map_variables


    friend std::istream& operator>>(std::istream& in, MPSList<N>& obj) {
        std::string str = "";
        while (in && (in >> str)) if (str.find(obj.starts) != std::string::npos) break;

        set_type E = set_empty<set_type>();

        obj.mps_list_.clear();
        int xi = -1;
        double s = -1;
        int npa = -1;

        while (in) {
            int t = 0;
            in >> str;
            if (str.find(obj.ends) != std::string::npos) break;
            in >> t >> str >> npa;
            if (str == "inf") s = SABNA_DBL_INFTY;
            else s = std::stod(str);
            if (xi != t) obj.mps_list_.push_back(std::vector<MPSNode>());
            xi = t;
            set_type pa = E;
            int xj = -1;
            for (int i = 0; i < npa; ++i) { in >> xj; pa = set_add(pa, xj); }
            obj.mps_list_[xi].push_back(MPSNode{s, pa});
        } // while in

        obj.n_ = obj.mps_list_.size();

        return in;
    } // operator>>

    friend std::ostream& operator<<(std::ostream& out, const MPSList<N>& obj) {
        std::string str = obj.starts + "\n";

        for (int xi = 0; xi < obj.n_; ++xi) {
            for (auto& nd : obj.mps_list_[xi]) {
                str += "mps " + std::to_string(xi) + " " + std::to_string(nd.s) + " " + std::to_string(set_size(nd.pa)) + " ";
                for (auto& xj : as_vector(nd.pa)) str += std::to_string(xj) + " ";
                str += "\n";
            }
        }

        str += obj.ends + "\n";
        return out << str;
    } // operator<<

    std::pair<bool, std::string> read(int n, const std::string& in) {
        std::ifstream f(in);
        if (!f) return { false, "could not read mps file" };
        f >> *this;
        if (n_ != n) return { false, "incorrect number of variables in mps file" };
        return { true, "" };
    } // read

    std::pair<bool, std::string> write(const std::string& out) {
        std::ofstream f(out);
        if (!f) return { false, "could not write mps file" };
        f << *this;
        return { true, "" };
    } // write


private:
    std::vector<set_type> m_subset_adjacency_matrix__(const set_type& given, const set_type& target) const {
        std::vector<set_type> adj(n_, set_empty<set_type>());
        set_type given_complement = ~given;

        for (int xi = 0; xi < n_; ++xi) {
            if (!in_set(target, xi)) continue;

            set_type temp = set_empty<set_type>();
            int idx = mps_list_[xi].size() - 1;

            for (; idx > 0; --idx) {
                temp = temp | mps_list_[xi][idx].pa;
                if (is_superset(given, mps_list_[xi][idx].pa)) break;
            }

            temp = temp & given_complement & target;

            for (int xj : as_vector(temp)) { adj[xj] = set_add(adj[xj], xi); }
        } // for xi

        return adj;
    } // m_adjacency_matrix__


    const MPSNode null_mps_{ SABNA_DBL_INFTY, set_empty<set_type>() };

    std::vector<std::vector<MPSNode>> mps_list_;
    int n_ = -1;

    const std::string starts = "MPSList_Begins";
    const std::string ends = "MPSList_Ends";
}; // class MPSList

#endif // MPSLIST_HPP
