/***
 *  $Id$
 **
 *  File: impl.hpp
 *  Created: Feb 20, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *          Zainul Abideen Sayed <zsayed@buffalo.edu>
 *  Copyright (c) 2020-2021 SCoRe Group
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef IMPL_HPP
#define IMPL_HPP

#include <bit>
#include <cstdint>
#include <deque>
#include <unordered_set>
#include <vector>

#include "parallel_hashmap/phmap.h"


namespace scool {

  namespace impl {

    inline uint32_t map_to(uint32_t key, uint32_t range) {
        // Lemire's fastrange mapper
        return (static_cast<uint64_t>(key) * static_cast<uint64_t>(range)) >> 32;
        // simple mod mapper
        //return key % range;
    } // map_to

    template <bool Unique, typename T, typename Alloc>
    inline void add_to(std::vector<T, Alloc>& S, const T& t) {
        if constexpr (Unique) S.push_back(t);
        else {
            // check if task is already in the queue
            // if yes merge the task
            // else this is a new/unique task push it to the queue
            auto it = std::find(std::begin(S), std::end(S), t);
            if (it != std::end(S)) it->merge(t);
            else S.push_back(t);
        }
    } // add_to

    template <bool Unique, typename T, typename Alloc>
    inline void add_to(std::deque<T, Alloc>& S, const T& t) {
        if constexpr (Unique) S.push_back(t);
        else {
            auto it = std::find(std::begin(S), std::end(S), t);
            if (it != std::end(S)) it->merge(t);
            else S.push_back(t);
        }
    } // add_to

    template <typename Container, typename T>
    inline void update_table__(Container& S, const T& t) {
        // if key is already present the insertion will fail
        auto [it, res] = S.insert(t);

        // extracting the existing key
        // merge it with current key
        // insert back the key
        if (res == false) {
            auto x = S.extract(it);
            x.value().merge(t);
            S.insert(std::move(x));
        }
    } // update_table__

    template <bool Unique, typename T>
    inline void add_to(std::unordered_set<T>& S, const T& t) {
        if constexpr (Unique) S.insert(t);
        else update_hash_table__(S, t);
    } // add_to

    template <bool Unique, typename T>
    inline void add_to(phmap::flat_hash_set<T>& S, const T& t) {
        if constexpr (Unique) S.insert(t);
        else update_table__(S, t);
    } // add_to

    template <bool Unique, typename T>
    inline void add_to(phmap::node_hash_set<T>& S, const T& t) {
        if constexpr (Unique) S.insert(t);
        else update_table__(S, t);
    } // add_to


  class bitmap {
  public:
      using storage_type = unsigned char;

      explicit bitmap(int n = 0) { if (n != 0) resize(n); }


      // standard sequence interface
      auto size() const { return size_; }

      void resize(int n) {
          size_ = n;
          int sz = (size_ + CAPACITY - 1) / CAPACITY;
          data_.resize(sz, 0);
      } // resize

      bool operator[](int n) const {
          int w = n / CAPACITY;
          int b = n - (CAPACITY * w);
          return data_[w] & (1 << b);
      } // operator[]


      // non-standard interface
      void set(int n, bool val = true) {
          int w = n / CAPACITY;
          int b = n - (CAPACITY * w);
          data_[w] = val ? (data_[w] | (1 << b)) : (data_[w] & ~(1 << b));
      } // set

      void reset() { for (auto& x : data_) x = 0; }

      const storage_type* storage() const { return data_.data(); }

      auto storage_size() const { return data_.size(); }


      void OR(const storage_type* d) {
          int sz = data_.size();
          for (int i = 0; i < sz; ++i) data_[i] |= d[i];
      } // OR

      void AND(const storage_type* d) {
          int sz = data_.size();
          for (int i = 0; i < sz; ++i) data_[i] &= d[i];
      } // AND

      // returns population count of 1 bits
      int count() {
        int c = 0;
        int sz = data_.size();
        for (int i = 0; i < sz; ++i) c += std::popcount(data_[i]);
        return c;
      } // count

  private:
      const int CAPACITY = 8 * sizeof(storage_type);

      int size_;
      std::vector<storage_type> data_;

  }; // class bitmap

  } // namespace impl

} // namespace scool

#endif // IMPL_HPP
