//# cppconfig.h: setup preprocessor macro and import standard classes
//# Copyright (C) 2014
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#pragma once

// include configuration includes:
//         libc++ detected:     _LIBCPP_VERSION
//         libstdc++ detected:  __GLIBCXX__
#if defined(__clang__)
#   if __has_include(<__config>) // defines _LIBCPP_VERSION
#       include <__config>
#       define HAVE_CPP11
#   elif __has_include(<bits/c++config.h>) // defines __GLIBCXX__
#       include <bits/c++config.h>
#   else
#       include <ios>
#   endif
#elif defined(__GNUC__) // gcc does not have __has_include
#   include <ios> // ios should include the c++config.h which defines __GLIBCXX__
// Test for GCC >= 4.7
#   if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6))
#       define HAVE_CPP11
#   endif
#endif

// include fundamental header files
#if defined(_LIBCPP_VERSION)
#define HAVE_LIBCPP
#include <memory>
#include <complex>
#include <utility>
#include <functional>
extern int including_tuple_begin;
#include <tuple>
extern int including_tuple_end;
#else
#if defined(__GLIBCXX__)
#define HAVE_STDCPP
#include <tr1/memory>
#include <tr1/tuple>
#include <tr1/functional>
namespace std {
  template<class T> class complex;
}
namespace std {
	template <class T1, class T2> class pair;
}
#endif
#endif

// set up fundamental types
namespace casa {
#if defined(_LIBCPP_VERSION)
#define HAVE_LIBCPP
using std::auto_ptr;
using std::shared_ptr;
using std::dynamic_pointer_cast;
using std::const_pointer_cast;
using std::function;
#else
#if defined(__GLIBCXX__)
#define HAVE_STDCPP
#if defined(HAVE_CPP11)
using std::auto_ptr;
using std::shared_ptr;
using std::dynamic_pointer_cast;
using std::const_pointer_cast;
using std::function;
#else
using std::auto_ptr;
using std::tr1::shared_ptr;
using std::tr1::dynamic_pointer_cast;
using std::tr1::const_pointer_cast;
using std::tr1::function;
#endif
#endif
#endif
}
