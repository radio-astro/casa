/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
   Initial version February 2009
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file singlelayerwater.hpp
*/
#ifndef _LIBAIR_SINGLELAYERWATER_HPP__
#define _LIBAIR_SINGLELAYERWATER_HPP__

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp> 

#include "model_enums.hpp"
#include "columns_data.hpp"

namespace LibAIR2 {

  // Forward decleration
  class Slice;
  class SliceResult;
  class RTResult;
  class IsoTLayer;

  /** \brief Sky brightness at fixed frequencies for single layer of
      water

      Model the sky brightness at a fixed set of frequency points for
      a single layer water model
   */
  class ISingleLayerWater:
    private WaterData,
    boost::noncopyable
  {

  public:
    
    // ---- Public data  --------------

    /// Water vapour column (mm pwv)
    double n;

    /// Water vapour temperature (K)
    double T;

    /// Water vapour pressure (mBar)
    double P;
    
    // ---------- Construction / Destruction --------------    
    
    /**
       \param fgrid The frequency grid on which to compute the sky
       brightness

       \param l Lines to include in the opacity calculation
       
       \param t Treatement of the partition function calculation
       
       \param c Continuum calculation contribution to use

       \param PDrop Pressure drop across the layer
       
     */
    ISingleLayerWater(const std::vector<double> &fgrid,
		      WaterData::Lines l=WaterData::L183,
		      PartitionTreatment t=PartTable,
		      Continuum c=AirCont,
		      double PDrop=0);		     


    // ----------------- Public interface ------------------------------
    
    /** \brief Calculate the brightness temperature at each point in
       the frequency grid.
       
       \note The reference returned is only valid for the lifetime of
       this object. 
       
       \note The temperatures are R-J temperatures in units of K
     */
    const std::vector<double> &   TbGrid(void);

    const std::vector<double> &   TbGrid(const SliceResult &background);

    /** \brief Set the temperature of the background 

	Normally the background will be the CMB @ 2.7K
     */
    void setBckgT(double Tbckg);    

    /** Return the frequncy grid on which we are computing
     */
    const std::vector<double> &getFGrid(void);

    /** \brief Return the background 
     */
    const SliceResult & getBckg(void) const;

  private:

    // -------------- Private data -------------------------------------
    
    boost::scoped_ptr<Slice>       s;
    boost::scoped_ptr<IsoTLayer>   layer;
    boost::scoped_ptr<RTResult> sr;
    boost::scoped_ptr<Slice>       cmbslice;
    // Contains the background, i.e., 2.7K
    boost::scoped_ptr<SliceResult> bckg;
    std::vector<double> scratch;


    
    // ------------------ Private interface ----------------------------

    
    /** Transfer model parameters stored in public fields to the
	subcomponents of this class.
     */
    void updatePars(void);


  };

}

#endif
