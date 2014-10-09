/**
   \file tbutils.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version May 2008
   Maintained by ESO since 2013.

*/

#include "tbutils.hpp"
#include "basicphys.hpp"

namespace LibAIR2 {
  
  double PlanckToRJ(double f,
		    double TPlanck)
  {
    const double I = BPlanck(f, TPlanck);
    const double TRJ=  I / std::pow(f,2)  * CSquared_On_TwoK;
    return TRJ;
  }

}

