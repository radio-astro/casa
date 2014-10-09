/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file numalgo.hpp
   
   Numerical algorithms used in LibAIR
*/
#ifndef _LIBAIR_NUMALGO_HPP__
#define _LIBAIR_NUMALGO_HPP__

#include <cmath>
#include <vector>

#include <boost/array.hpp>
#include <boost/function.hpp>
#include <boost/math/special_functions/binomial.hpp>

namespace LibAIR2 {

  inline double 
  GaussError( std::vector<double>::const_iterator obs,
	      const std::vector<double> & mod,
	      const std::vector<double> & noise)
  {
    double res= 0 ;
    double pref=0;
    for (size_t i =0 ; i < noise.size() ; ++i)
    {
      res +=   std::pow(   ( *(obs+i) - mod[i])/noise[i], 2)*0.5;
      pref +=  0.5*std::log(2*M_PI*pow(noise[i],2));
    }

    return pref+res;

  }

  /** Compute finite differences up to (N-1)-th order of function f

      This should be accellerated by storing the previous evaluations
      
      \param c The central point around which to compute the difference
   */
  template<size_t N>
  boost::array<double, N>
  CenFD(const boost::function<double (double)> &f,
	double c,
	double delt)
  {
    boost::array<double, N> res;
    for (size_t n=0; n<N; ++n)
    {
      res[n]=0;
      if (n==0)
      {
	res[n]=f(c);
      }
      else
      {
	for (size_t i=0; i<=n; ++i)
	  res[n]+=std::pow(-1.0,(int)i)*boost::math::binomial_coefficient<double>(n,i)*f(c+(0.5*n-i)*delt);
	res[n]/=std::pow(delt, (int)n);
      }
    }
    return res;
  }

  /** Compute the central difference derivative approximation of an
      array-valued function
      
      \param N Derivative order up to N-1 is calculated

      \param K Size of the array produced by the function

      \param f Function to differentiate

      \param c Central point around which to calculate the derivative
      
      \param delt Displacement to use for the differnetiation
   */
  template<size_t N, size_t K>
  boost::array< boost::array<double, K>, N>
  CenFDV(const boost::function< boost::array<double, K> (double)> &f,
	 double c,
	 double delt)
  {
    boost::array< boost::array<double, K>, N> res;
    for (size_t n=0; n<N; ++n)
    {
      for (size_t k=0; k<K; ++k)
      {
	res[n][k]=0;
      }

      if (n==0)
      {
	res[n]=f(c);
      }
      else
      {
	for (size_t i=0; i<=n; ++i)
	{
	  double coeff=std::pow(-1.0,(int)i)*boost::math::binomial_coefficient<double>(n,i);
	  boost::array<double, K> ir =f(c+(0.5*n-i)*delt);
	  for(size_t k=0; k<K; ++k)
	  {
	    res[n][k]+=coeff*ir[k];
	  }
	}	  
	for(size_t k=0; k<K; ++k)
	{
	  res[n][k]/=std::pow(delt, (int)n);
	}
      }
    }
    return res;
  }




}
#endif

