/***
 *  $Id$
 **
 *  File: algorithm.hpp
 *  Created: Sep 09, 2011
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2004-2013 Jaroslaw Zola
 *  Distributed under the Boost Software License, Version 1.0.
 *  See accompanying file LICENSE_BOOST.txt.
 *
 *  This file is part of jaz.
 */

#ifndef JAZ_ALGORITHM_HPP
#define JAZ_ALGORITHM_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <numeric>
#include <vector>


namespace jaz {

  /** Function: copy_n
   *  This function is different from std::copy_n.
   *  It copies at most n elements!
   */
  template <typename InputIter, typename Size, typename OutputIter>
  OutputIter copy_n(InputIter first, InputIter last, Size n, OutputIter out) {
      if (n > 0) for (; (first != last) && (n > 0); ++first, --n) *out++ = *first;
      return out;
  } // copy_n


  /** Function: count_unique
   */
  template <typename Iter, typename Comp>
  std::size_t count_unique(Iter first, Iter last, Comp comp) {
      if (first == last) return 0;
      std::size_t S = 1;
      Iter prev = first++;
      for (; first != last; ++first, ++prev) if (comp(*prev, *first) == false) ++S;
      return S;
  } // count_unique

  /** Function: count_unique
   */
  template <typename Iter>
  inline std::size_t count_unique(Iter first, Iter last) {
      using value_type = typename std::iterator_traits<Iter>::value_type;
      return count_unique(first, last, std::equal_to<value_type>());
  } // count_unique


  /** Function: range
   */
  template <typename Iter, typename Comp>
  Iter range(Iter first, Iter last, Comp comp) {
      if (first == last) return last;
      Iter iter = first;
      for (; (iter != last) && comp(*first, *iter); ++iter);
      return iter;
  } // range

  template <typename Iter>
  inline Iter range(Iter first, Iter last) {
      using value_type = typename std::iterator_traits<Iter>::value_type;
      return range(first, last, std::equal_to<value_type>());
  } // range


  /** Function: compact
   */
  template <typename Iter, typename Oper, typename Comp>
  Iter compact(Iter first, Iter last, Oper op, Comp comp) {
      if (first == last) return last;

      Iter res = first;

      while (first != last) {
          Iter iter = range(first, last, comp);
          *res = *first;
          ++first;
          *res = std::accumulate(first, iter, *res, op);
          first = iter;
          ++res;
      }

      return res;
  } // compact

  template <typename Iter, typename Oper>
  inline Iter compact(Iter first, Iter last, Oper op) {
      using value_type = typename std::iterator_traits<Iter>::value_type;
      return compact(first, last, op, std::equal_to<value_type>());
  } // compact


  /** Function: mode
   *  Finds the first range containing mode.
   */
  template <typename Iter, typename Comp>
  std::pair<Iter, Iter> mode(Iter first, Iter last, Comp comp) {
      if (first == last) return std::make_pair(last, last);

      auto res = std::make_pair(last, last);
      std::size_t mcount = 1;

      Iter iter = first;

      while (first != last) {
          std::size_t count = 0;
          for (; (iter != last) && comp(*first, *iter); ++iter, ++count);
          if (mcount < count) {
              res.first = first;
              res.second = iter;
              mcount = count;
          }
          first = iter;
      }

      return res;
  } // mode

  template <typename Iter>
  inline std::pair<Iter, Iter> mode(Iter first, Iter last) {
      using value_type = typename std::iterator_traits<Iter>::value_type;
      return mode(first, last, std::equal_to<value_type>());
  } // mode


  /** Function: intersection_size
   */
  template <typename Iter1, typename Iter2, typename Pred>
  std::size_t intersection_size(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, Pred pred) {
      std::size_t S = 0;

      while ((first1 != last1) && (first2 != last2)) {
          if (pred(*first1, *first2)) ++first1;
          else if (pred(*first2, *first1)) ++first2;
          else {
              ++first1;
              ++first2;
              ++S;
          }
      } // while

      return S;
  } // intersection_size

  /** Function: intersection_size
   */
  template <typename Iter1, typename Iter2>
  inline std::size_t intersection_size(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2) {
      using value_type = typename std::iterator_traits<Iter1>::value_type;
      return intersection_size(first1, last1, first2, last2, std::less<value_type>());
  } // intersection_size


  /** Function: find_all
   */
  template <typename InputIter, typename OutputIter, typename Pred>
  OutputIter find_all(InputIter first, InputIter last, OutputIter out, Pred pred) {
      for (; first != last; ++first) {
          if (pred(*first) == true) {
              *out = first;
              ++out;
          }
      } // for
      return out;
  } // find_all


  /** Function: max_vectors
   *  Reorganizes range such that max elements are at the beginning.
   */
  template <typename Iter, typename Pred>
  Iter max_vectors(Iter first, Iter last, Pred pred) {
      if (first == last) return first;

      Iter beg = first;
      Iter end = last;

      do {
          Iter cur = beg;
          Iter i = beg;

          ++i;

          // find max element
          for (; i != end; ++i) {
              if (pred(*i, *beg) == true) {
                  --end; std::swap(*i, *end);
                  --i;
              } else if (pred(*beg, *i) == true) {
                  std::swap(*i, *beg);
                  cur = i;
                  ++cur;
              }
          } // for i

          i = beg;
          ++i;

          // clean
          for (; (i != cur) && (i != end); ++i) {
              if (pred(*i, *beg) == true) {
                  --end; std::swap(*i, *end);
                  --i;
              }
          } // for i

          ++beg;
      } while (beg != end);

      return end;
  } // max_vectors


  template <typename Vector>
  inline double levenshtein_distance(const Vector& v1, const Vector& v2) {
      int n = v1.size();
      int m = v2.size();

      n++;
      m++;

      std::vector<int> D(2 * m, 0);

      int* Dp = D.data();
      int* Dc = Dp + m;

      // first row
      for (int j = 0; j < m; ++j) Dp[j] = j;

      for (int i = 1; i < n; ++i) {
          Dc[0] = i;
          for (int j = 1; j < m; ++j) {
              int t = (v1[i - 1] != v2[j - 1]);
              int d = std::min({ Dc[j - 1] + 1,  Dp[j] + 1, Dp[j - 1] + t });
              Dc[j] = d;
          }
          std::swap(Dp, Dc);
      } // for i

      return static_cast<double>(Dp[m - 1]) / (std::max(n, m) - 1);
  } // levenshtein_distance

} // namespace jaz

#endif // JAZ_ALGORITHM_HPP
