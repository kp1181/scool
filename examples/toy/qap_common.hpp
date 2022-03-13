/***
 *  $Id$
 **
 *  File: qap_common.hpp
 *  Created: Mar 01, 2019
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2019 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef QAP_COMMON_HPP
#define QAP_COMMON_HPP

#include <fstream>
#include <string>
#include <vector>


// read in QAP instance as defined in QAPLIB: http://anjos.mgi.polymtl.ca/qaplib/inst.html
// in literature A is often denoted as F, and B as D
// we have min(p) \sum_i \sum_j a_ij b_{p(i)p(j)} where p is permutation
bool read_qaplib_instance(const std::string& name, int& n, std::vector<int>& A, std::vector<int>& B) {
    std::ifstream f(name);
    if (!f) return false;

    f >> n;
    if (n < 2) return false;

    A.resize(n * n);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) f >> A[i * n + j];
    }

    B.resize(n * n);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) f >> B[i * n + j];
    }

    return true;
} // read_qaplib_instance

// write solution in QAPLIB format
void print_qalib_solution(std::ostream& os, int n, double score, const std::vector<int>& p) {
    os << n << " " << score << std::endl;
    for (auto x : p) os << (x + 1) << " ";
    os << std::endl;
} // print_qalib_solution

#endif // QAP_COMMON_HPP
