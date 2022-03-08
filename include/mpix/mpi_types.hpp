/***
 *  $Id$
 **
 *  File: mpi_types.hpp
 *  Created: Sep 17, 2012
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2012-2014 Jaroslaw Zola
 *  Distributed under the Boost Software License, Version 1.0.
 *  See accompanying file LICENSE_BOOST.txt.
 *
 *  This file is part of mpix2.
 */

#ifndef MPIX2_MPI_TYPES_HPP
#define MPIX2_MPI_TYPES_HPP

#include <complex>
#include <mpi.h>


namespace mpix {

  template <typename T> inline MPI_Datatype MPI_Type();


  // Based on Table 3.2 MPI 3.1 Report
  template <> inline MPI_Datatype MPI_Type<char>() { return MPI_CHAR; }

  template <> inline MPI_Datatype MPI_Type<short int>() { return MPI_SHORT; }

  template <> inline MPI_Datatype MPI_Type<int>() { return MPI_INT; }

  template <> inline MPI_Datatype MPI_Type<long int>() { return MPI_LONG; }

  template <> inline MPI_Datatype MPI_Type<long long int>() { return MPI_LONG_LONG_INT; }

  template <> inline MPI_Datatype MPI_Type<signed char>() { return MPI_SIGNED_CHAR; }


  template <> inline MPI_Datatype MPI_Type<unsigned char>() { return MPI_UNSIGNED_CHAR; }

  template <> inline MPI_Datatype MPI_Type<unsigned short int>() { return MPI_UNSIGNED_SHORT; }

  template <> inline MPI_Datatype MPI_Type<unsigned int>() { return MPI_UNSIGNED; }

  template <> inline MPI_Datatype MPI_Type<unsigned long int>() { return MPI_UNSIGNED_LONG; }

  template <> inline MPI_Datatype MPI_Type<unsigned long long int>() { return MPI_UNSIGNED_LONG_LONG; }


  template <> inline MPI_Datatype MPI_Type<float>() { return MPI_FLOAT; }

  template <> inline MPI_Datatype MPI_Type<double>() { return MPI_DOUBLE; }

  template <> inline MPI_Datatype MPI_Type<long double>() { return MPI_LONG_DOUBLE; }

  // we ignore fixed size types as they are aliased to basic types
  // we ignore C_TYPES

  // here go C++ types
  template <> inline MPI_Datatype MPI_Type<bool>() { return MPI_CXX_BOOL; }

  template <> inline MPI_Datatype MPI_Type<std::complex<float>>() { return MPI_CXX_FLOAT_COMPLEX; }

  template <> inline MPI_Datatype MPI_Type<std::complex<double>>() { return MPI_CXX_DOUBLE_COMPLEX; }

  template <> inline MPI_Datatype MPI_Type<std::complex<long double>>() { return MPI_CXX_LONG_DOUBLE_COMPLEX; }



  template <typename T> inline MPI_Datatype MPI_Type(T t) { return MPI_Type<T>(); }

} // namespace mpix

#endif // MPIX2_MPI_TYPES_HPP
