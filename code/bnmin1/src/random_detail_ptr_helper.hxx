/* 
   This file is part of BNMin1 and is licensed under GNU General
   Public License version 2

 * based on boost random/detail/ptr_helper.hpp header file
 * Copyright Jens Maurer 2000-2001
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)

 */

#ifndef _BNMIN1_RANDOM_DETAIL_PTR_HELPER_HXX__
#define _BNMIN1_RANDOM_DETAIL_PTR_HELPER_HXX__

#include <boost/config.hpp>


namespace bnmin1boost {
namespace random {
namespace detail {

// type_traits could help here, but I don't want to depend on type_traits.
template<class T>
struct ptr_helper
{
  typedef T value_type;
  typedef T& reference_type;
  typedef const T& rvalue_type;
  static reference_type ref(T& r) { return r; }
  static const T& ref(const T& r) { return r; }
};

#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
template<class T>
struct ptr_helper<T&>
{
  typedef T value_type;
  typedef T& reference_type;
  typedef T& rvalue_type;
  static reference_type ref(T& r) { return r; }
  static const T& ref(const T& r) { return r; }
};

template<class T>
struct ptr_helper<T*>
{
  typedef T value_type;
  typedef T& reference_type;
  typedef T* rvalue_type;
  static reference_type ref(T * p) { return *p; }
  static const T& ref(const T * p) { return *p; }
};
#endif

} // namespace detail
} // namespace random
} 

//
// BOOST_RANDOM_PTR_HELPER_SPEC --
//
//  Helper macro for broken compilers defines specializations of
//  ptr_helper.
//
#ifdef BNMIN1_NO_TEMPLATE_PARTIAL_SPECIALIZATION
# define BNMIN1_RANDOM_PTR_HELPER_SPEC(T)                \
namespace bnmin1boost { namespace random { namespace detail { \
template<>                                              \
struct ptr_helper<T&>                                   \
{                                                       \
  typedef T value_type;                                 \
  typedef T& reference_type;                            \
  typedef T& rvalue_type;                               \
  static reference_type ref(T& r) { return r; }         \
  static const T& ref(const T& r) { return r; }         \
};                                                      \
                                                        \
template<>                                              \
struct ptr_helper<T*>                                   \
{                                                       \
  typedef T value_type;                                 \
  typedef T& reference_type;                            \
  typedef T* rvalue_type;                               \
  static reference_type ref(T * p) { return *p; }       \
  static const T& ref(const T * p) { return *p; }       \
};                                                      \
}}}
#else
# define BNMIN1_RANDOM_PTR_HELPER_SPEC(T)
#endif 

#endif 
