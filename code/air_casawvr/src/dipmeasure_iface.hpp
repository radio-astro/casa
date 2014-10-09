/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>   
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file dipmeasure_iface.hpp
*/
#ifndef _LIBAIR_DIPMEASURE_IFACE_HPP__
#define _LIBAIR_DIPMEASURE_IFACE_HPP__

#include <boost/tuple/tuple.hpp>

#include "measure_iface.hpp"

namespace LibAIR2{

  class PPDipModel;

  /** \brief A sky dip measurement with normal noise

   */
  class DipNormMeasure :
    public ALMAMeasure,
    public NormalNoise
  {
    typedef std::vector<double> obs_t;
    typedef boost::tuple< double, 
			  obs_t> datum;
    std::vector<datum> obs;
    
    PPDipModel & _model;
    
  public:

    // ---------- Construction / Destruction --------------

    /** Initialise and set the model. This class will calculate
	likelihoods for the supplied model. The model must not be
	deleted until this class is deleted.
     */
    DipNormMeasure(PPDipModel & model);

    // ---------- Public interface   --------------    

    /** \brief Add a sky-dip datum. 
	
	\param za     zenith angle (rad)

	\param skyTb sky brightness temperatures at this zenith angle
    */
    void addObs(double za,
		const std::vector<double> & skyTb);
    
    // ----------------Inherited from Minim::MLikelihood -------------------
    double lLikely (void) const;            
    
    
  };
  

}
#endif
