/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file rtranfer.hpp

   Radiative transfer. Some code was previously in slice.hpp/slice.cpp
*/
#ifndef __LIBAIR_RTRANSFER_HPP__
#define __LIBAIR_RTRANSFER_HPP__

#include <vector>

namespace LibAIR2 {

  // Forward declarations
  class Slice;
  class Layer;
  class SliceResult;

  /**
     A container to store the results of radiative transfer
     calculation.
   */
  class RTResult {

  protected:
    /// Irradiance of this component
    std::vector<double> I;

  private:
    /// Brighness temperature of this component
    std::vector<double> Tb;

    /// Our copy of the frequency grid
    std::vector<double>  _f;

  public:
    
    /// Frequency grid
    const std::vector<double> &f;
    
    // ---------- Construction / Destruction --------------    

    /**
       \param f The grid of frequencies at which result will be
       computed.
     */
    RTResult(const std::vector<double> &fp);
    
    virtual ~RTResult() {};


    // ---------- Public interface ------------------------

    /**
       \brief Compute the brightness temperature of this component of
       the atmosphere.

       Uses the Raleigh-Jeans approximation.
     */
    const std::vector<double> & UpdateTb(void);

    /** \brief Update the irradiance of this slice
     */
    virtual void UpdateI(const SliceResult & bckg) = 0;


    const std::vector<double>  & getI(void)
    {
      return I;
    }
  };

  /**
     The result of the computation for one slice
   */
  class SliceResult:
    public RTResult
  {
    /// Transmissivity of this slice
    std::vector<double> tx;

  public:

    /// Const reference to slice that this result is for
    const Slice & slice;

    // ---------- Construction / Destruction --------------
    SliceResult( const Slice & p_slice,
		 const std::vector<double> & f ) ;

    // ---------- Public interface ------------------------

    virtual void UpdateI(const SliceResult & bckg);
    /**
       Update irradiance assuming zero-temperature background. Only
       makes sense really for opaque slices.
     */
    void UpdateI(void);

  };

  /**
     Compute radiative transfer for a layer
  */
  class LayerResult:
    public RTResult
  {

  public:    
    
    const Layer & layer;

    // ---------- Construction / Destruction --------------
    LayerResult(const Layer & p_layer,
		const std::vector<double> & f );
    
    // ---------- Public interface ------------------------    

    void UpdateI(const SliceResult & bckg);

  };
  
  

}

#endif
