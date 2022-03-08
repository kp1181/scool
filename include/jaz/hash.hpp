/***
 *  $Id$
 **
 *  File: hash.hpp
 *  Created: Apr 02, 2011
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2011-2012 Jaroslaw Zola
 *  Distributed under the Boost Software License, Version 1.0.
 *  See accompanying file LICENSE_BOOST.txt.
 *
 *  This file is part of jaz.
 */

#ifndef HASH_HPP
#define HASH_HPP

#include <inttypes.h>
#include <string.h>
#include <random>


/** File: hash.hpp
 */
namespace jaz {

  /** Class: djb32
   *  Functor implementing 32bit hash DJB2 with XOR. It is hard to say if this is
   *  a good or a bad function! Some claim it is good, but some tests show
   *  it is not: http://www.team5150.com/~andrew/noncryptohashzoo/DJB.html.
   */
  class djb32 {
  public:
      /** Constructor: djb32
       *
       *  Parameter:
       *  seed - Default seed from http://en.wikipedia.org/wiki/2147483647.
       */
      explicit djb32(uint32_t seed = 2147483647) : seed_(seed) { }

      /** Function: operator()
       *  Performs hashing.
       */
      uint32_t operator()(const char* s, unsigned int l) const {
          const unsigned char* S = (const unsigned char*)s;
          uint32_t len = l;
          uint32_t hash = 5381 + seed_ + len;

          for (; len & ~1; len -= 2, S += 2) {
              hash = ((((hash << 5) + hash) ^ S[0]) * 33) ^ S[1];
          }

          if (len & 1) hash = ((hash << 5) + hash) ^ S[0];

          return hash ^ (hash >> 16);
      } // operator()

      /** Function: operator()
       *  Performs hashing.
       */
      uint32_t operator()(const std::string& s) const {
          return this->operator()(s.c_str(), s.size());
      } // operator()

  private:
      uint32_t seed_;

  }; // class djb32


  /** Class: murmur64A
   *  Functor implementing 64bit hash MurmurHash64A.
   */
  class murmur64A {
  public:
      /** Constructor: murmur264
       */
      explicit murmur64A(uint64_t seed = 2147483647) : seed_(seed) { }

      /** Function: operator()
       *  Performs hashing.
       */
      uint64_t operator()(const char* s, unsigned int l) const {
          const uint64_t m = 0xc6a4a7935bd1e995ULL;
          const int r = 47;

          uint64_t len = l;
          uint64_t h = seed_ ^ (len * m);

          const uint64_t* data = reinterpret_cast<const uint64_t*>(s);
          const uint64_t* end = data + (len >> 3);

          while (data != end) {
              uint64_t k = *data++;

              k *= m;
              k ^= k >> r;
              k *= m;

              h ^= k;
              h *= m;
          } // while

          const unsigned char* data2 = reinterpret_cast<const unsigned char*>(data);

          switch (len & 7) {
            case 7: h ^= uint64_t(data2[6]) << 48;
            case 6: h ^= uint64_t(data2[5]) << 40;
            case 5: h ^= uint64_t(data2[4]) << 32;
            case 4: h ^= uint64_t(data2[3]) << 24;
            case 3: h ^= uint64_t(data2[2]) << 16;
            case 2: h ^= uint64_t(data2[1]) << 8;
            case 1: h ^= uint64_t(data2[0]);
                h *= m;
          }

          h ^= h >> r;
          h *= m;
          h ^= h >> r;

          return h;
      } // operator()

      /** Function: operator()
       *  Performs hashing.
       */
      uint64_t operator()(const std::string& s) const {
          return this->operator()(s.c_str(), s.size());
      } // operator()

  private:
      uint64_t seed_;

  }; // class murmur64A


  /** Class: tabulation8
   *  Functor implementing tabulation hashing based on 8bit blocks.
   *  Assumes that std::random_device works and unsigned int is 32bit.
   */
  class tabulation8 {
  public:
      explicit tabulation8(int L) : T_(L * 256), l_(L) {
          unsigned int* t = reinterpret_cast<unsigned int*>(T_.data());
          std::random_device rd;
          for (int i = 0; i < 2 * L * 256; ++i) t[i] = rd();
      } // tabulation8

      // provided if deterministic initialization required
      template <typename Random> void init(Random&& rng) {
          auto* t = reinterpret_cast<typename Random::result_type*>(T_.data());
          int l = 256 * l_ * (8 / sizeof(typename Random::result_type));
          for (int i = 0; i < l; ++i) t[i] = rng();
      } // init

     /** Function: operator()
      *  Performs hashing of first L characters (hence l must be equal to L).
      */
      uint64_t operator()(const char* s, unsigned int l) const {
          uint64_t H = T_[s[0]];
          for (int i = 1;  i < l_; ++i) H ^= T_[256 * i + s[i]];
          return H;
      } // operator()

      uint64_t operator()(const std::string& s) const {
          return this->operator()(s.c_str(), s.size());
      } // operator()

  private:
      std::vector<uint64_t> T_;
      int l_;

  }; // class tabulation8


  /** Class: rabin64
   *  Functor implementing Rabin fingerprint. This code is based (to some extent)
   *  on Java implementations available in the Internet (with junk removed
   *  and several bug-fixes).
   */
  class rabin64 {
  public:
      // p_ represents: x^64 + x^4 + x^3 + x + 1 (which I hope is irreducible in Z2)

      /** Constructor: rabin64
       */
      rabin64() : p_(0x000000000000001BLL), p_deg_(64) { m_init__(); }

      /** Function: operator()
       *  Performs hashing.
       */
      uint64_t operator()(const char* s, unsigned int l) const {
          const unsigned char* S = (const unsigned char*)s;

          uint64_t h = 0;
          unsigned int pos = l % 8;

          unsigned int i = 0;
          if (pos != 0) for (; i < pos; ++i) h = (h << 8) ^ S[i];

          while (i < l) {
              h = tab32_[h & 0xFF] ^
                  tab40_[(h >> 8) & 0xFF] ^
                  tab48_[(h >> 16) & 0xFF] ^
                  tab56_[(h >> 24) & 0xFF] ^
                  tab64_[(h >> 32) & 0xFF] ^
                  tab72_[(h >> 40) & 0xFF] ^
                  tab80_[(h >> 48) & 0xFF] ^
                  tab88_[(h >> 56) & 0xFF] ^
                  ((uint64_t)S[i] << 56) ^
                  ((uint64_t)S[i + 1] << 48) ^
                  ((uint64_t)S[i + 2] << 40) ^
                  ((uint64_t)S[i + 3] << 32) ^
                  ((uint64_t)S[i + 4] << 24) ^
                  ((uint64_t)S[i + 5] << 16) ^
                  ((uint64_t)S[i + 6] << 8) ^
                  (uint64_t)S[i + 7];
              i += 8;
          } // while

          return h;
      } // operator

      /** Function: operator()
       *  Performs hashing.
       */
      uint64_t operator()(const std::string& s) const {
          return this->operator()(s.c_str(), s.size());
      } // operator()

  private:
      uint64_t p_;
      unsigned int p_deg_;

      void m_init__() {
          uint64_t xp = (uint64_t)1 << (p_deg_ - 1);
          uint64_t* mods = new uint64_t[p_deg_];

          mods[0] = p_;

          for (unsigned int i = 1; i < p_deg_; i++) {
              mods[i] = mods[i - 1] << 1;
              if (mods[i - 1] & xp) mods[i] ^= p_;
          }

          memset(tab32_, 0, 256);
          memset(tab40_, 0, 256);
          memset(tab48_, 0, 256);
          memset(tab56_, 0, 256);
          memset(tab64_, 0, 256);
          memset(tab72_, 0, 256);
          memset(tab80_, 0, 256);
          memset(tab88_, 0, 256);

          for (unsigned int i = 0; i < 256; ++i) {
              unsigned int c = i;
              for (unsigned int j = 0; (j < 8) && (c > 0); ++j) {
                  if (c & 1) {
                      tab32_[i] ^= mods[j];
                      tab40_[i] ^= mods[j + 8];
                      tab48_[i] ^= mods[j + 16];
                      tab56_[i] ^= mods[j + 24];
                      tab64_[i] ^= mods[j + 32];
                      tab72_[i] ^= mods[j + 40];
                      tab80_[i] ^= mods[j + 48];
                      tab88_[i] ^= mods[j + 56];
                  } // if
                  c >>= 1;
              } // for j
          } // for i

          delete[] mods;
      } // m_init__

      uint64_t tab32_[256];
      uint64_t tab40_[256];
      uint64_t tab48_[256];
      uint64_t tab56_[256];
      uint64_t tab64_[256];
      uint64_t tab72_[256];
      uint64_t tab80_[256];
      uint64_t tab88_[256];

  }; // class rabin64


  /** Function: xorshift64star
   *  Generates next pseudo-random number from x
   *  using XORShift* algorithm.
   */
  inline uint64_t xorshift64star(uint64_t x) {
      x ^= x >> 12; // a
      x ^= x << 25; // b
      x ^= x >> 27; // c
      return x * 2685821657736338717ULL;
  } // xorshift64star

  /** Function: murmurhash3_mixer
   *  Plain MurmurHash3 finalizer.
   */
  inline uint64_t murmurhash3_mixer(uint64_t x) {
      x ^= (x >> 33);
      x *= 0xFF51AFD7ED558CCD;
      x ^= (x >> 33);
      x *= 0xC4CEB9FE1A85EC53;
      x ^= (x >> 33);
      return x;
  } // murmurhash3_mixer

} // namespace jaz

#endif // HASH_HPP
