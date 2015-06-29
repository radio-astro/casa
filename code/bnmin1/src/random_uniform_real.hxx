/* 
   This file is part of BNMin1 and is licensed under GNU General
   Public License version 2

 * based on boost random/uniform_real.hpp header file
 * Copyright Jens Maurer 2000-2001
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)

 */

#ifndef _BNMIN1_RANDOM_UNIFORM_REAL_HXX_
#define _BNMIN1_RANDOM_UNIFORM_REAL_HXX_

#include <cassert>
#include <iostream>
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/random/detail/config.hpp>

namespace bnmin1boost {

// uniform distribution on a real range
template<class RealType = double>
class uniform_real
{
public:
  typedef RealType input_type;
  typedef RealType result_type;

  explicit uniform_real(RealType min_arg = RealType(0),
                        RealType max_arg = RealType(1))
    : _min(min_arg), _max(max_arg)
  {
#ifndef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
    BOOST_STATIC_ASSERT(!std::numeric_limits<RealType>::is_integer);
#endif
    assert(min_arg <= max_arg);
  }

  // compiler-generated copy ctor and assignment operator are fine

  result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const { return _min; }
  result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const { return _max; }
  void reset() { }

  template<class Engine>
  result_type operator()(Engine& eng) {
    result_type numerator = static_cast<result_type>(eng() - eng.min BOOST_PREVENT_MACRO_SUBSTITUTION());
    result_type divisor = static_cast<result_type>(eng.max BOOST_PREVENT_MACRO_SUBSTITUTION() - eng.min BOOST_PREVENT_MACRO_SUBSTITUTION());
    assert(divisor > 0);
    assert(numerator >= 0 && numerator <= divisor);
    return numerator / divisor * (_max - _min) + _min;
  }

#ifndef BOOST_RANDOM_NO_STREAM_OPERATORS
  template<class CharT, class Traits>
  friend std::basic_ostream<CharT,Traits>&
  operator<<(std::basic_ostream<CharT,Traits>& os, const uniform_real& ud)
  {
    os << ud._min << " " << ud._max;
    return os;
  }

  template<class CharT, class Traits>
  friend std::basic_istream<CharT,Traits>&
  operator>>(std::basic_istream<CharT,Traits>& is, uniform_real& ud)
  {
    is >> std::ws >> ud._min >> std::ws >> ud._max;
    return is;
  }
#endif

private:
  RealType _min, _max;
};

} 

#endif 
