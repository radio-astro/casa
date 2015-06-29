/*
   This file is part of BNMin1 and is licensed under GNU General
   Public License version 2
 
 * based on boost random/variate_generator.hpp header file
 * Copyright Jens Maurer 2000-2001
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)

 */

#ifndef _BNMIN1_RANDOM_RANDOM_GENERATOR_HXX__
#define _BNMIN1_RANDOM_RANDOM_GENERATOR_HXX__

#include <boost/config.hpp>

// implementation details
#include "random_uniform_01.hxx"
#include "random_detail_pass_through_engine.hxx"
#include "random_detail_uniform_int_float.hxx"
#include "random_detail_ptr_helper.hxx"


namespace bnmin1boost {

namespace random {
namespace detail {

template<bool have_int, bool want_int>
struct engine_helper;

// for consistency, always have two levels of decorations
template<>
struct engine_helper<true, true>
{
  template<class Engine, class DistInputType>
  struct impl
  {
    typedef pass_through_engine<Engine> type;
  };
};

template<>
struct engine_helper<false, false>
{
  template<class Engine, class DistInputType>
  struct impl
  {
    typedef uniform_01<Engine, DistInputType> type;
  };
};

template<>
struct engine_helper<true, false>
{
  template<class Engine, class DistInputType>
  struct impl
  {
    typedef uniform_01<Engine, DistInputType> type;
  };
};

template<>
struct engine_helper<false, true>
{
  template<class Engine, class DistInputType>
  struct impl
  {
    typedef uniform_int_float<Engine, unsigned long> type;
  };
};

} // namespace detail
} // namespace random

template<class Engine, class Distribution>
class variate_generator
{
private:
  typedef random::detail::pass_through_engine<Engine> decorated_engine;

public:
  typedef typename decorated_engine::base_type engine_value_type;
  typedef Engine engine_type;
  typedef Distribution distribution_type;
  typedef typename Distribution::result_type result_type;

  variate_generator(Engine e, Distribution d)
    : _eng(decorated_engine(e)), _dist(d) { }

  result_type operator()() { return _dist(_eng); }
  template<class T>
  result_type operator()(T value) { return _dist(_eng, value); }

  engine_value_type& engine() { return _eng.base().base(); }
  const engine_value_type& engine() const { return _eng.base().base(); }

  distribution_type& distribution() { return _dist; }
  const distribution_type& distribution() const { return _dist; }

  result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () const { return (distribution().min)(); }
  result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const { return (distribution().max)(); }

private:
  enum {
    have_int = std::numeric_limits<typename decorated_engine::result_type>::is_integer,
    want_int = std::numeric_limits<typename Distribution::input_type>::is_integer
  };
  typedef typename random::detail::engine_helper<have_int, want_int>::BOOST_NESTED_TEMPLATE impl<decorated_engine, typename Distribution::input_type>::type internal_engine_type;


  internal_engine_type _eng;
  distribution_type _dist;
};

} // namespace bnmin1boost

#endif 
