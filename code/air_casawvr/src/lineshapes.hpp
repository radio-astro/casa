/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file lineshapes.hpp

   Definition of line shapes that are useful for modelling atmospheric
   transparency.
*/
#ifndef __LIBAIR_LINESHAPES_HPP__
#define __LIBAIR_LINESHAPES_HPP__

#include <cmath>

namespace LibAIR2 {

  // Forward declarations
  struct CLineParams;

  /** \brief The Gross line shape 

      \f$ \frac{4 S \gamma}{\pi} \frac{ f^2 }{\left(f^2-f_0^{2}\right)^2 + 4 \gamma^2 f^2} \f$
     
     \param f  frequency to evaluate at
     \param f0 centre frequency of the line
     \param gamma line width
     \param S line strength
     
   */
  template<class T> T 
  GrossLine( T f,
	     const T f0, const T gamma, const T S)
  {
    const T f2  = std::pow(f, 2);
    const T f02 = std::pow(f0, 2);
    const T gamma2 = std::pow(gamma, 2);

    return M_1_PI * 4 * S * gamma * f2 / (   std::pow( f2-f02, 2) + 4 * gamma2 * f2) ;

  }

  /** \brief Evaluate Gross line shape using packaged line parameters.

   */
  double CGrossLine(double f,
		    const CLineParams & cp);
  

}

#endif
