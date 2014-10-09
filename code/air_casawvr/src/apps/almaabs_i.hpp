/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2009
   Maintained by ESO since 2013.

   \file almaabs_i.hpp
   
*/
#ifndef _LIBAIR_ALMAABS_I_HPP__
#define _LIBAIR_ALMAABS_I_HPP__

#include <vector>
#include <list>

#include <boost/scoped_ptr.hpp>

#include "almawvr/nestedsampler.hxx"
#include "almawvr/priors.hxx"

#include "almaabs.hpp"
#include "../model_iface.hpp"
#include "../dipmodel_iface.hpp"
#include "../measure_iface.hpp"

#include "almaresults.hpp"


namespace LibAIR2 {

  /// Structures to represent likelihood of a measurement for an
  /// absolute retrieval from ALMA data
  struct iALMAAbsRetLL
  {
    /// Assume thermal noise 
    static const double thermNoise;

    /// Model of the atmosphere before taking into account elevation
    /// of observation
    CouplingModel *cm;

    /// Model of the atmosphere after taking into account elevation
    PPDipModel m;

    /// Representation of the measured values and errors
    AbsNormMeasure *ll;

    iALMAAbsRetLL(const std::vector<double> &TObs,
		  double el,
		  const ALMAWVRCharacter &WVRChar);

  };

  class iALMAAbsRet
  {

  public:

    iALMAAbsRetLL ls;

    /// The posterior 
    std::list<Minim::WPPoint> post;

    /// Representation of the likelihood and priors 
    Minim::IndependentFlatPriors pll;

    /// Evidence value
    double evidence;

    /// The nested sampler
    boost::scoped_ptr<Minim::NestedS> ns;
    

    /// Number of points in the live set
    static const size_t n_ss;

    iALMAAbsRet(const std::vector<double> &TObs,
		double el,
		const ALMAWVRCharacter &WVRChar);

    void sample(void);

    // -------------- Retrieval of results ------------------
    
    /** Get the important model parameters and estimated errors
     */
    void  g_Pars(ALMAResBase &r);

    /** Get the inferred phase-correction coefficients and estimated
	errors
    */
    void  g_Coeffs(ALMAResBase &r);
    
  };


}



#endif
