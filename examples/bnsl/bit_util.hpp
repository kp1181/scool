/***
 *  $Id$
 **
 *  File: bit_util.hpp
 *  Created: Nov 11, 2015
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2015-2022 SCoRe Group http://www.score-group.org/
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 */

#ifndef BIT_UTIL_HPP
#define BIT_UTIL_HPP

#include <cstdint>
#include <cstring>
#include <ostream>
#include <vector>


#ifndef BIT_UTIL_64BIT

using bit_util_base_type = uint32_t;
constexpr int bit_util_type_shift = 5;

#define BIT_UTIL_CLZ __builtin_clz
#define BIT_UTIL_POPCNT __builtin_popcount
#define BIT_UTIL_FFS __builtin_ffs

#else

using bit_util_base_type = uint64_t;
constexpr int bit_util_type_shift = 6;

#define BIT_UTIL_CLZ __builtin_clzll
#define BIT_UTIL_POPCNT __builtin_popcountll
#define BIT_UTIL_FFS __builtin_ffsll

#endif // BIT_UTIL_64BIT


template <int N> constexpr int set_max_word() { return N * (1 << bit_util_type_shift); }


template <int N> struct uint_type {
    // b[0] represents elements 0..(8 * sizeof(bit_util_base_type) - 1)
    bit_util_base_type b[N];
}; // struct uint_type

inline bool operator==(uint_type<1> lhs, uint_type<1> rhs) { return lhs.b[0] == rhs.b[0]; }


template <int N> inline bool operator==(const uint_type<N>& lhs, const uint_type<N>& rhs) {
    for (int i = 0; i < N; ++i) {
        if (lhs.b[i] != rhs.b[i]) return false;
    }
    return true;
} // operator==

template <int N> inline bool operator!=(const uint_type<N>& lhs, const uint_type<N>& rhs) {
    return !(lhs == rhs);
} // operator!=

inline uint_type<1> operator~(uint_type<1> x) {
    uint_type<1> res = x;
    res.b[0] = ~res.b[0];
    return res;
}

template<int N> inline uint_type<N> operator~(const uint_type<N>& x) {
    uint_type<N> res;
    for (int i = 0; i < N; ++i) res.b[i] = ~x.b[i];
    return res;
} // operator~

template <int N> inline uint_type<N> operator^(const uint_type<N>& lhs, const uint_type<N>& rhs) {
    uint_type<N> res;
    for (int i = 0; i < N; ++i) res.b[i] = lhs.b[i] ^ rhs.b[i];
    return res;
} // operator&

template <int N> inline uint_type<N> operator&(const uint_type<N>& lhs, const uint_type<N>& rhs) {
    uint_type<N> res;
    for (int i = 0; i < N; ++i) res.b[i] = lhs.b[i] & rhs.b[i];
    return res;
} // operator&

template <int N> inline uint_type<N> operator|(const uint_type<N>& lhs, const uint_type<N>& rhs) {
    uint_type<N> res;
    for (int i = 0; i < N; ++i) res.b[i] = lhs.b[i] | rhs.b[i];
    return res;
} // operator|

template <int N> inline uint_type<N> shift_right(const uint_type<N>& x, int shft) {
    const int osz = 8 * sizeof(bit_util_base_type);
    auto res = x;
    res.b[0] = (res.b[0] >> shft);
    for (int i = 1; i < N; ++i) {
        res.b[i - 1] |= (res.b[i] << (osz - shft));
        res.b[i] = (res.b[i] >> shft);
    }
    return res;
} // shift_right



struct uint_hash {

    // Based on 64-bit FNV
    template <int N> uint64_t operator()(const uint_type<N>& x) const {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&x.b);
        const int n = N * sizeof(bit_util_base_type);

        const uint64_t prime = 1099511628211;
        uint64_t hash = 0xcbf29ce484222325;

        for (int i = 0; i < n; ++i) {
            hash *= prime;
            hash ^= p[i];
        }

        return hash;
    } // operator

}; // struct uint_hash

struct tbb_uint_hash {

    // Based on 64-bit FNV
    template <int N> uint64_t hash(const uint_type<N>& x) const {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&x.b);
        const int n = N * sizeof(bit_util_base_type);

        const uint64_t prime = 1099511628211;
        uint64_t hash = 0xcbf29ce484222325;

        for (int i = 0; i < n; ++i) {
            hash *= prime;
            hash ^= p[i];
        }

        return hash;
    } // hash

    template <int N> bool equal(const uint_type<N>& x, const uint_type<N>& y) const { return (x == y); }

}; // struct tbb_uint_hash


// copy nb bits starting at position f (unsafe: do not copy more than bit_util_base_type can hold)
template <int N> inline bit_util_base_type copy_bits_out(const uint_type<N>& S, int f, int nb) {
    const int osz = 8 * sizeof(bit_util_base_type);

    int b = (f >> bit_util_type_shift);
    f = f - (b << bit_util_type_shift);

    bit_util_base_type out = S.b[b] >> f;
    if (f + nb > osz) out |= (S.b[b + 1] << (osz - f));

    return out & ((1 << nb) - 1);
} // copy_bits_out


template <int N> inline int msb(const uint_type<N>& x) {
    const int osz = 8 * sizeof(bit_util_base_type);
    for (int i = N - 1; i >= 0; --i) {
        if (x.b[i] != 0) return (osz * i + osz - 1 - BIT_UTIL_CLZ(x.b[i]));
    }
    return -1;
} // msb


template <typename set_type> constexpr int set_max_size() { return 8 * sizeof(set_type); }

template <typename set_type> inline set_type set_empty() {
    set_type S;
    std::memset(&S, 0, sizeof(set_type));
    return S;
} // set_empty

template <typename set_type> inline set_type set_full(int n) {
    set_type S = set_empty<set_type>();
    int b = (n >> bit_util_type_shift);
    S.b[b] = (static_cast<bit_util_base_type>(1) << (n - (b << bit_util_type_shift))) - 1;
    std::memset(&S, 255, b * sizeof(bit_util_base_type));
    return S;
} // set_full


inline uint64_t set_add(uint64_t S, int x) { return S | (static_cast<uint64_t>(1) << x); }

inline uint_type<1> set_add(uint_type<1> S, int x) {
    S.b[0] = S.b[0] | (static_cast<bit_util_base_type>(1) << x);
    return S;
} // set_add

template <int N> inline uint_type<N> set_add(uint_type<N> S, int x) {
    int b = (x >> bit_util_type_shift);
    S.b[b] = S.b[b] | (static_cast<bit_util_base_type>(1) << (x - (b << bit_util_type_shift)));
    return S;
} // set_add


inline uint64_t set_remove(uint64_t S, int x) { return S & ~(static_cast<uint64_t>(1) << x); }

inline uint_type<1> set_remove(uint_type<1> S, int x) {
    S.b[0] = S.b[0] & ~(static_cast<bit_util_base_type>(1) << x);
    return S;
} // set_remove

template <int N> inline uint_type<N> set_remove(uint_type<N> S, int x) {
    int b = (x >> bit_util_type_shift);
    S.b[b] = S.b[b] & ~(static_cast<bit_util_base_type>(1) << (x - (b << bit_util_type_shift)));
    return S;
} // set_remove


inline uint64_t set_diff(uint64_t S, uint64_t U) { return (S & ~U); };

inline uint_type<1> set_diff(uint_type<1> S, uint_type<1> U) {
    S.b[0] = S.b[0] & ~U.b[0];
    return S;
} // set_diff

template <int N> inline uint_type<N> set_diff(const uint_type<N>& S, const uint_type<N>& U) {
    uint_type<N> res;
    for (int i = 0; i < N; ++i) res.b[i] = S.b[i] & ~U.b[i];
    return res;
} // set_diff


inline int set_size(uint64_t S) { return __builtin_popcountll(S); }

inline int set_size(uint_type<1> S) { return BIT_UTIL_POPCNT(S.b[0]); }

template <int N> inline int set_size(const uint_type<N>& S) {
    int w = 0;
    for (int i = 0; i < N; ++i) w += BIT_UTIL_POPCNT(S.b[i]);
    return w;
} // set_size


inline bool in_set(uint64_t S, int x) { return S & (static_cast<uint64_t>(1) << x); }

inline bool in_set(uint_type<1> S, int x) {
    return S.b[0] & (static_cast<bit_util_base_type>(1) << x);
} // in_set

template <int N> inline bool in_set(const uint_type<N>& S, int x) {
    int b = (x >> bit_util_type_shift);
    return S.b[b] & (static_cast<bit_util_base_type>(1) << (x - (b << bit_util_type_shift)));
} // in_set



inline bool is_emptyset(uint64_t S) { return (S == 0); }

template <int N> inline bool is_emptyset(const uint_type<N>& S) {
    bool empty = true;
    for (int i = 0; i < N; ++i) empty = empty && (S.b[i] == 0);
    return empty;
} // is_emptyset


// test if S is a superset of U
inline bool is_superset(uint64_t S, uint64_t U) { return ((S & U) == U); }

template <int N> inline bool is_superset(const uint_type<N>& S, const uint_type<N>& U) {
    bool super = true;
    for (int i = 0; i < N; ++i) super = super && ((S.b[i] & U.b[i]) == U.b[i]);
    return super;
} // is_superset


template <typename set_type, typename Iter> inline set_type as_set(Iter first, Iter last) {
    set_type S = set_empty<set_type>();
    for (; first != last; ++first) S = set_add(S, *first);
    return S;
} // as_set

template <typename Sequence, typename set_type>
inline set_type as_set(const Sequence& S) { return as_set(std::begin(S), std::end(S)); }

template <typename set_type>
inline set_type as_set(std::initializer_list<int> S) { return as_set<set_type>(std::begin(S), std::end(S)); }


template <typename set_type> inline std::vector<int> as_vector(const set_type& S) {
    int s = set_max_size<set_type>();
    std::vector<int> v;
    v.reserve(s);
    for (int i = 0; i < s; ++i) if (in_set(S, i)) v.push_back(i);
    return v;
} // as_vector

template <typename set_type> inline void as_vector(const set_type& S, std::vector<int>& v) {
    int s = set_max_size<set_type>();
    v.clear();
    for (int i = 0; i < s; ++i) if (in_set(S, i)) v.push_back(i);
} // as_vector


template <int N> inline bool lexicographical_less(const uint_type<N>& lhs, const uint_type<N>& rhs) {
    if (lhs == rhs) return false;

    for (int i = 0; i < N; ++i) {
        bit_util_base_type res = lhs.b[i] ^ rhs.b[i];
        if (res != 0) {
            int pos = BIT_UTIL_FFS(res) - 1;
            return in_set(lhs.b[i], pos);
        }
    }

    return false;
} // lexicographical_less


template <int N> inline std::ostream& operator<<(std::ostream& os, const uint_type<N>& S) {
    int s = set_max_size<uint_type<N>>();
    for (int i = 0; i < s; ++i) os << (in_set(S, i) ? 1 : 0);
    return os;
} // operator<<

#endif // BIT_UTIL_HPP
