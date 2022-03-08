/***
 *  $Id$
 **
 *  File: utility.hpp
 *  Created: Feb 18, 2019
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2019-2021 SCoRe Group
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <atomic>
#include <exception>
#include <streambuf>
#include <type_traits>
#include <vector>


namespace scool {

  namespace util {

    // buf cannot be changed in the middle of stream invocation!
    template <typename charT, typename traits = std::char_traits<charT>>
    class fast_vector_ibuf : public std::basic_streambuf<charT, traits> {
    public:
        fast_vector_ibuf(std::vector<charT>& buf) {
            this->setg(buf.data(), buf.data(), buf.data() + buf.size());
        } // fast_vector_ibuf
    }; // class fast_vector_ibuf

    // at least two characters must be inserted into the buffer
    template <typename charT, typename traits = std::char_traits<charT>>
    class fast_vector_obuf : public std::basic_streambuf<charT, traits> {
    public:
        using traits_type = traits;
        using int_type = typename traits_type::int_type;

        fast_vector_obuf(std::vector<charT>& buf) : buf_(buf) {
            buf_.reserve(1024);
            buf_.resize(2, 0);
            this->setp(buf_.data(), buf_.data() + buf_.size());
        } // fast_vector_obuf

        int_type overflow(int_type c = traits_type::eof()) {
            if (c != traits_type::eof()) buf_.push_back(static_cast<charT>(c));
            return c;
        } // overflow

    private:
        std::vector<charT>& buf_;

    }; // class fast_vector_obuf

    // at least two characters must be inserted into the buffer
    template <typename charT, typename traits = std::char_traits<charT>>
    class fast_raw_obuf : public std::basic_streambuf<charT, traits> {
    public:
        using traits_type = traits;
        using int_type = typename traits_type::int_type;

        fast_raw_obuf(char* buf, int sz) : buf_(buf) {
            this->setp(buf_, buf_ + sz);
        } // fast_vector_obuf

        int_type overflow(int_type c = traits_type::eof()) {
            if (c != traits_type::eof()) throw std::runtime_error("serialization buffer overflow");
            return c;
        } // overflow

        int_type size() const {
            return this->pptr() - buf_;
        } // size

    private:
        char* buf_;

    }; // class fast_raw_obuf

  } // namespace util

} // namespace scool

#endif // UTILITY_HPP
