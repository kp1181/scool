/***
 *  $Id$
 **
 *  File: hungarian.hpp
 *  Created: Jan 27, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020 Jaroslaw Zola
 *  Distributed under the MIT License.
 */

#ifndef HUNGARIAN_HPP
#define HUNGARIAN_HPP

#include <cstdlib>
#include <cstring>
#include <vector>

#include "hungarian.h"


inline int** array_to_matrix(const int* D, int rows, int cols) {
  int i, j;
  int** res;

  res = static_cast<int**>(std::calloc(rows, sizeof(int*)));

  for(i = 0; i < rows; ++i) {
      res[i] = static_cast<int*>(std::calloc(cols, sizeof(int)));
      std::memcpy(res[i], D + (i * cols), cols * sizeof(int));
  }

  return res;
} // array_to_matrix

inline void release_matrix(int** M, int rows) {
    for (int i = 0; i < rows; ++i) std::free(M[i]);
    std::free(M);
} // release_matrix

inline int linear_assignment_cost(const std::vector<int>& D, int n) {
    int** M = array_to_matrix(D.data(), n, n);

    hungarian_problem_t pm;

    hungarian_init(&pm, M, n, n, HUNGARIAN_MODE_MINIMIZE_COST);
    hungarian_solve(&pm);

    int res = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) res += (M[i][j] * pm.assignment[i][j]);
    }

    hungarian_free(&pm);
    release_matrix(M, n);

    return res;
} // linear_assignment_cost

inline std::vector<int> linear_assignment(const std::vector<int>& D, int n) {
    int** M = array_to_matrix(D.data(), n, n);

    hungarian_problem_t pm;

    hungarian_init(&pm, M, n, n, HUNGARIAN_MODE_MINIMIZE_COST);
    hungarian_solve(&pm);

    std::vector<int> p(n);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (pm.assignment[i][j] == 1) {
                p[i] = j;
                break;
            }
        } // for j
    } // for i

    hungarian_free(&pm);
    release_matrix(M, n);

    return p;
} // linear_assignment

#endif // HUNGARIAN_HPP
