/***
 *  $Id$
 **
 *  File: limits.hpp
 *  Created: Jun 01, 2018
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2018 SCoRe Group http://www.score-group.org/
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 */


#ifndef LIMITS_HPP
#define LIMITS_HPP

#include <limits>

constexpr double SABNA_DBL_INFTY = std::numeric_limits<double>::has_infinity ?
    std::numeric_limits<double>::infinity() : std::numeric_limits<double>::max();

#endif // LIMITS_HPP
