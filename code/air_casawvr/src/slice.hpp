/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file slice.hpp

*/

#ifndef _LIBAIR_SLICE_HPP__
#define _LIBAIR_SLICE_HPP__

#include <vector>

namespace LibAIR2 {

  // Forward declarations
  class Column;

  /**
     \brief A slice is an infinitesimally thin layer of the atmosphere
   described by single temperature and pressure.

  */
  class Slice {

    /*
      Not exposing these as const public members since one could
      envisage performance saving by updating existing slices.
     */

    /// Slice temperature
    double T;
    /// Slice pressure
    double P;

    std::vector<const Column *> cols;

  public:
    
    /// Scaling for all of the columns
    const double scale;

    // ---------- Construction / Destruction --------------

    Slice( double T , double P,
	   double scale=1.0);

    virtual ~Slice() {};

    // ---------- Public interface ------------------------

    /**
       Add a column to this slice. The list of columns is used to
       compute the transmission of this slice.
     */
    void AddColumn (const Column & c);
    
    /// Returns the slice temperature
    double getT(void) const
    { return T ; } ;

    /// Set the temperature
    void setT(double Tnew)
    {
      T=Tnew;
    }
    

    /// Returns the slice pressure
    double getP(void) const
    { return P ; } ;

    /// Set the pressure of the slice
    void setP(double Pnew) 
    {
      P=Pnew;
    }

    /**
       \brief Compute the transmisison of this slice

       \param f The frequency grid  
       \param res The transmission coefficients are stored here

     */
    virtual void ComputeTx (const std::vector<double> & f,
			    std::vector<double> & res) const ;

  };

  /** \brief A slice whose transmission is always zero
   */
  class OpaqueSlice :
    public Slice
  {

  public:

    // ---------- Construction / Destruction --------------

    OpaqueSlice( double T , double P);

    // ---------- Inherited from Slice
    virtual void ComputeTx (const std::vector<double> & f,
			    std::vector<double> & res) const ;

  };



}

#endif

