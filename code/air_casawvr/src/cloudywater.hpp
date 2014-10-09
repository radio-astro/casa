/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version June 2009
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file cloudywater.hpp
    
   Declaration of ICloudyWater, a model of a water vapour layer plus
   cloud continuum
   
*/
#ifndef _LIBAIR_CLOUDYWATER_HPP__
#define _LIBAIR_CLOUDYWATER_HPP__

#include "singlelayerwater.hpp"
#include "slice.hpp"
#include "columns.hpp"
#include "rtranfer.hpp"

namespace LibAIR2 {

  /** \brief Model with single layer water and background continuum
      
      It is assumed the continuum opacity increases as square of
      frequency 
   */
  class ICloudyWater:
    public ISingleLayerWater
  {

  public:

    // -------------------Public data----------------------------------
    
    /// The opacity of continuum at 183 GHz
    double tau183;
    
    /// The temperature of the continuum emmitting medium
    double CT;

    // ---------- Construction / Destruction --------------------------
    ICloudyWater(const std::vector<double> &fgrid,
		 LibAIR2::WaterData::Lines l=LibAIR2::WaterData::L183,
		 PartitionTreatment t=PartTable,
		 Continuum c=AirCont,
		 double PDrop=0);		     
    
    // ----------------- Public interface ------------------------------
    

    //------------------ Inherited from ISingleLayerWater --------------

    /**
       \note In the present design this function is NOT virtual
     */
    const std::vector<double> &   TbGrid(void);


  private:
    
    Slice contslice;
    SliceResult sr;
    EmpContColumn col;

    /** Transfer model parameters stored in public fields to the
	subcomponents of this class.
     */
    void updatePars(void);

  };


}

#endif
