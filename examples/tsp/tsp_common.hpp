/***
 *  $Id$
 **
 *  File: tsp_common.hpp
 *  Created: Mar 27, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef TSP_COMMON_HPP
#define TSP_COMMON_HPP

#include <cmath>
#include <fstream>
#include <string>
#include <vector>


// read in TSP instance as defined in TSPLIB: http://elib.zib.de/pub/mp-testdata/tsp/tsplib/tsplib.html
bool read_tsp_instance(const std::string& name, int& n, std::vector<float>& D, std::vector<float>& b) {
    std::ifstream f(name);
    if (!f) return false;

    std::string buf;

    // process header
    do {
        f >> buf;

        if (buf == "DIMENSION:") {
            f >> n;
            continue;
        }

        if (buf == "DIMENSION") {
            f >> buf;
            f >> n;
            continue;
        }

        if (buf == "EDGE_WEIGHT_TYPE:") {
            f >> buf;
            if (buf != "EUC_2D") return false;
            continue;
        }

        if (buf == "EDGE_WEIGHT_TYPE") {
            f >> buf;
            f >> buf;
            if (buf != "EUC_2D") return false;
            continue;
        }
    } while (!f.eof() && buf != "NODE_COORD_SECTION");

    std::vector<std::pair<float, float>> C(n);

    for (int i = 0; i < n; ++i) {
        int idx;
        f >> idx;
        f >> C[i].first;
        f >> C[i].second;
    }

    if (!f) return false;

    D.clear();
    D.resize(n * n);

    b.clear();
    b.resize(n);

    for (int i = 0; i < n; ++i) {
        float mm[2] = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };

        for (int j = 0; j < n; ++j) {
            float dx = C[i].first - C[j].first;
            float dy = C[i].second - C[j].second;
            float d = std::sqrt((dx * dx) + (dy * dy));
            D[i * n + j] = d;
            if (d < mm[0]) {
                mm[1] = mm[0];
                mm[0] = d;
            } else if (d < mm[1]) mm[1] = d;
        }

        b[i] = (mm[0] + mm[1]) / 2;
    } // for i

    return true;
} // read_tsp_instance

#endif // TSP_COMMON_HPP
