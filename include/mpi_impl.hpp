/***
 *  $Id$
 **
 *  File: mpi_impl.hpp
 *  Created: Jun 24, 2020
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2020-2021 SCoRe Group
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 *
 *  This file is part of SCoOL.
 */

#ifndef MPI_IMPL_HPP
#define MPI_IMPL_HPP

#include <istream>
#include <ostream>
#include <type_traits>

#include <mpi.h>

#include "impl.hpp"
#include "utility.hpp"


namespace scool {

  namespace mpi_impl {

    template <typename Iter>
    void serialize(Iter first, Iter last, std::vector<char>& buf) {
        scool::util::fast_vector_obuf<char> ob(buf);
        std::ostream os(&ob);

        if (first == last) {
            buf.clear();
            return;
        }

        for (; first != last; ++first) os << *first;
    } // serialize


    template <typename T, bool Unique, typename Container>
    void deserialize_and_add(std::vector<char>& data, Container& S) {
        if (data.empty()) return;

        scool::util::fast_vector_ibuf<char> ib(data);
        std::istream is(&ib);

        while (!is.eof()) {
            T t;
            is >> t;
            impl::add_to<Unique>(S, t);
        }
    } // deserialize_and_add


    template <typename T>
    void serialize_and_send(const T& t, int rank, int Tag, MPI_Comm Comm) {
        std::vector<char> data;

        scool::util::fast_vector_obuf<char> ob(data);
        std::ostream os(&ob);
        os << t;

        int buf = data.size();

        MPI_Send(&buf, 1, MPI_INT, rank, Tag, Comm);
        if (buf > 0) MPI_Send(data.data(), buf, MPI_CHAR, rank, Tag, Comm);
    } // serialize_and_send

    template <typename Iter>
    void serialize_and_send(Iter first, Iter last, int rank, int Tag, MPI_Comm Comm) {
        std::vector<char> data;
        serialize(first, last, data);

        int buf = data.size();

        MPI_Send(&buf, 1, MPI_INT, rank, Tag, Comm);
        if (buf > 0) MPI_Send(data.data(), buf, MPI_CHAR, rank, Tag, Comm);
    } // serialize_and_send


    template <typename T>
    bool receive_and_deserialize(T& t, int rank, int Tag, MPI_Comm Comm) {
        MPI_Status stat;

        int buf = 0;
        MPI_Recv(&buf, 1, MPI_INT, rank, Tag, Comm, &stat);

        if (buf == 0) return false;

        std::vector<char> data;
        data.resize(buf);

        MPI_Recv(data.data(), buf, MPI_CHAR, rank, Tag, Comm, &stat);

        scool::util::fast_vector_ibuf<char> ib(data);
        std::istream is(&ib);

        is >> t;

        return true;
    } // receive_and_deserialize

    template <typename T, typename Output>
    bool receive_and_deserialize(Output out, int rank, int Tag, MPI_Comm Comm) {
        MPI_Status stat;

        int buf = 0;
        MPI_Recv(&buf, 1, MPI_INT, rank, Tag, Comm, &stat);

        if (buf == 0) return false;

        std::vector<char> data;
        data.resize(buf);

        MPI_Recv(data.data(), buf, MPI_CHAR, rank, Tag, Comm, &stat);

        scool::util::fast_vector_ibuf<char> ib(data);
        std::istream is(&ib);

        while (!is.eof()) {
            T t;
            is >> t;
            if (!is) break;
            *(out++) = std::move(t);
        }

        return true;
    } // receive_and_deserialize

    template <typename T> void reduce(T& t, MPI_Comm Comm) {
        const int RDC_TAG = 1101;

        int size, rank;

        MPI_Comm_size(Comm, &size);
        MPI_Comm_rank(Comm, &rank);

        if (size < 2) return;

        int d = 0;
        T lt = t;

        do {
            d = (size + 1) >> 1;

            if (rank + d < size) {
                receive_and_deserialize(lt, rank + d, RDC_TAG, Comm);
                t += lt;
            } else {
                if ((rank < size) && (rank - d >= 0)) {
                    serialize_and_send(t, rank - d, RDC_TAG, Comm);
                }
            }

            lt = t;
            size = d;
        } while (d > 1);

    } // reduce

    template <typename T> void broadcast(T& t, MPI_Comm Comm) {
        int size, rank;

        MPI_Comm_size(Comm, &size);
        MPI_Comm_rank(Comm, &rank);

        if (size < 2) return;

        std::vector<char> data;
        int buf = 0;

        if (rank == 0) {
            scool::util::fast_vector_obuf<char> ob(data);
            std::ostream os(&ob);
            os << t;

            buf = data.size();

            MPI_Bcast(&buf, 1, MPI_INT, 0, Comm);
            MPI_Bcast(data.data(), buf, MPI_CHAR, 0, Comm);
        } else {
            MPI_Bcast(&buf, 1, MPI_INT, 0, Comm);

            data.resize(buf);

            MPI_Bcast(data.data(), buf, MPI_CHAR, 0, Comm);

            scool::util::fast_vector_ibuf<char> ib(data);
            std::istream is(&ib);
            is >> t;
        }
    } // broadcast

  } // namespace mpi_impl

} // namespace scool

#endif // MPI_IMPL_HPP
