/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file layers.hpp
*/

#ifndef _LIBAIR_LAYER_HPP__
#define _LIBAIR_LAYER_HPP__

#include <deque>
#include <boost/shared_ptr.hpp>

namespace LibAIR2 {

  class Slice;
  class SliceResult;
  class Column;

  /**
     A layer is a part of the atmosphere with a single description but
     not necessarily completely uniform properties. In particular,
     layers vary due to hydrostatic effects, so they are split in
     otherwise identical slices with varying pressures.

   */
  class Layer
  {

  public:

    typedef boost::shared_ptr<Slice> pSlice_t;
    typedef std::deque<pSlice_t> sliceL_t;

  protected:
    /**
       The list containing slices belonging to this layer.

       First element is the element furthest from the observer.
    */
    sliceL_t sliceL;

  public:

    // ---------- Construction / Destruction --------------

  protected:

    /// Protected to prevent creation of objects of this class.
    Layer(void);

  public:

    // ---------- Construction / Destruction --------------

    virtual ~Layer(void);

    // ---------- Public interface ------------------------
    
    /**
       Add a slice to this layer. 
     */
    void addSliceFar(pSlice_t s);

    /** 
	Add a column to this layer.
     */
    virtual void AddColumn (const Column & c);


    // ---------- Accessor elements  ---------------------


    /// Iterator to the layer furthest from observr
    sliceL_t::const_iterator 
    getFar(void) const ;
    
    /// Iterator at end of layer containers
    sliceL_t::const_iterator 
    getEnd() const;
    
    const Slice & getNear(void) const;
    

  };


  /**\brief A layer that has constant temperature through its thickness

  */
  class IsoTLayer:
    public Layer
  {

  public:

    /// Units of mbar
    static const double slice_dP;

    // ---------- Construction / Destruction --------------

    /**
       \param T Temperature of the layer (assume all at same
       temperature).
    */
    IsoTLayer(double T, double PBase, double PDrop);

    // ---------- Public interface ------------------------

    void setT(double T);
    
    void setP(double P);
    

  };


}

#endif
