//#---------------------------------------------------------------------------
//# SDPol.h: Polarimetric processing
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------
#ifndef SDPOL_H
#define SDPOL_H

//# Includes
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <measures/Measures/Stokes.h>
#include <tables/Tables/BaseMappedArrayEngine.h>


namespace asap {

class SDStokesEngine : public casa::BaseMappedArrayEngine<casa::Float, casa::Float>
{
  //# Make members of parent class known.
public:
  using casa::BaseMappedArrayEngine<casa::Float,casa::Float>::virtualName;
protected:
  using casa::BaseMappedArrayEngine<casa::Float,casa::Float>::storedName;
  using casa::BaseMappedArrayEngine<casa::Float,casa::Float>::table;
  using casa::BaseMappedArrayEngine<casa::Float,casa::Float>::roColumn;
  using casa::BaseMappedArrayEngine<casa::Float,casa::Float>::rwColumn;

public:
    // Construct engine.  The sourveColumnName holds the XX,YY,R(XY),I(XY)
    // correlations
    SDStokesEngine (const casa::String& virtualColumnName,
                    const casa::String& sourceColumnName);

    // Construct from a record specification as created by getmanagerSpec().
    SDStokesEngine (const casa::Record& spec);

    // Destructor is mandatory.
    ~SDStokesEngine();

    // Return the type name of the engine (i.e. its class name).
    virtual casa::String dataManagerType() const;

    // Get the name given to the engine (is the source column name).
    virtual casa::String dataManagerName() const;
  
    // casa::Record a casa::Record containing data manager specifications.
    virtual casa::Record dataManagerSpec() const;

    // Return the name of the class.
    // This includes the names of the template arguments.
    static casa::String className();

   // The engine can access column cells.
    virtual casa::Bool canAccessArrayColumnCells (casa::Bool& reask) const;

    // Register the class name and the static makeObject "constructor".
    // This will make the engine known to the table system.
    // The automatically invoked registration function in DataManReg.cc
    // contains SDStokesEngine
    // Any other instantiation of this class must be registered "manually"
    // (or added to DataManReg.cc).
    static void registerClass();

    // Non writable
//    virtual casa::Bool isWritable () const {return casa::False;}

private:
    // Copy constructor is only used by clone().
    // (so it is made private).
    SDStokesEngine (const SDStokesEngine&);

    // Assignment is not needed and therefore forbidden
    // (so it is made private and not implemented).
    SDStokesEngine& operator=(const SDStokesEngine&);

    // Clone the engine object.
    DataManager* clone() const;

    // Initialize the object for a new table.
    // It defines the keywords containing the engine parameters.
    void create (casa::uInt initialNrrow);

    // Preparing consists of setting the writable switch and
    // adding the initial number of rows in case of create.
    // Furthermore it reads the keywords containing the engine parameters.
    void prepare();

    // Get an array in the given row.
    void getArray (casa::uInt rownr, casa::Array<casa::Float>& array);

    // Exception
    void putArray (casa::uInt rownr, const casa::Array<casa::Float>& array);

    // Compute Stokes parameters
    void computeOnGet (casa::Array<casa::Float>& array,
    		     const casa::Array<casa::Float>& target);

    // Get shape
    virtual casa::IPosition shape (casa::uInt rownr);

    // Convert input to output (virtual) shape
    casa::IPosition findOutputShape (const casa::IPosition& inputShape) const;


public:
    //*display 4
    // Define the "constructor" to construct this engine when a
    // table is read back.
    // This "constructor" has to be registered by the user of the engine.
    // If the engine is commonly used, its registration can be added
    // to the registerAllCtor function in DataManReg.cc. 
    // That function gets automatically invoked by the table system.
    static DataManager* makeObject (const casa::String& dataManagerType,
				    const casa::Record& spec);
};



class SDPolUtil
{
 public:
// Convert Q and U to linearly polarized intensity
  static casa::Array<casa::Float> polarizedIntensity (const casa::Array<casa::Float>& Q,
                                                      const casa::Array<casa::Float>& U);
// Convert Q and U to linearly polarized position angle (degrees)
  static casa::Array<casa::Float> positionAngle (const casa::Array<casa::Float>& Q,
                                                 const casa::Array<casa::Float>& U);

 // Rotate phase of Complex correlation R+iI by phase (degrees)
  static void rotatePhase (casa::Array<casa::Float>& R,
                           casa::Array<casa::Float>& I,
                           casa::Float phase);

 // Rotate phase of P=(Q+iU) by phase (degrees), but do this by applying 
 // the corrections to the raw linear polarizations
  static void rotateLinPolPhase (casa::Array<casa::Float>& C1,
                                 casa::Array<casa::Float>& C2,
                                 casa::Array<casa::Float>& C3,
                                 casa::Array<casa::Float>& I,
                                 casa::Array<casa::Float>& Q,
                                 casa::Array<casa::Float>& U,
                                 casa::Float phase);

// Get Stokes slices from the Array.  Start and End should
// already be setup to access the Array at the current cursor location
// (beam, IF, Pol; see SDMemTable).   The desired Stokes
// is specified in the string from "I", "Q", "U", "V"
  static casa::Array<casa::Float> getStokesSlice (casa::Array<casa::Float>& input, const casa::IPosition& start,
                                                  const casa::IPosition& end, const casa::String& stokes);

// Compute Circular polarization RR or LL from I and V
  static casa::Array<casa::Float> circularPolarizationFromStokes (casa::Array<casa::Float>& I, 
                                                                  casa::Array<casa::Float>& V,  
                                                                  casa::Bool doRR);


// Compute value for auxilliary spectra when converting to Stokes parameters.
// This combines the raw correlation values, according to what was combined
// to actually convert to Stokes parameters.   The output shape may change from 
// the input shape according to
// XX or YY          -> I             (1 -> 1)
// XX,YY             -> I             (2 -> 1)
// XX,YY,R(XY),I(XY) -> I,Q,U,V       (4 -> 4)
// 
// It is meant for tSys (FLoat) and masks (Bool or uChar)
// The input array must be of shape [nBeam,nIF,nPol,nChan]
  template <class T>
  static casa::Array<T> stokesData (casa::Array<T>& rawData, casa::Bool doLinear);

// Find the number of STokes parameters given the input number
// of raw polarizations
   static casa::uInt numberStokes (casa::uInt nPol) {casa::uInt nOut = nPol; if (nPol==2) nOut = 1; return nOut;};

// Find the Stokes type for the given polarization axis (0,1,2,3)
// You can ask for STokes or raw correlations (linear or circular)
   static casa::Stokes::StokesTypes convertStokes(casa::Int val, casa::Bool toStokes, 
                                                  casa::Bool linear);

// Return a label for the given polarization index (0-3).   
   static casa::String polarizationLabel (casa::uInt polIdx, casa::Bool linear, 
                                          casa::Bool stokes, casa::Bool linPol);


// These two functions are explicitly for the SDWriter
//
// Compute value for auxilliary spectra (Tsys and flags) when converting to Stokes parameters.
// This combines the raw correlation values, according to what was combined
// to actually convert to Stokes parameters.   The output shape may change from 
// the input shape according to
// XX or YY          -> I             (1 -> 1)
// XX,YY             -> I             (2 -> 1)
// XX,YY,R(XY),I(XY) -> I,Q,U,V       (4 -> 4)
// 
// The input array must be of shape [nChan,nPol]  (flags)
// The input array must be of shape [nPol]        (tSys)
//
  static casa::Array<casa::uChar> computeStokesFlagsForWriter (casa::Array<casa::uChar>& rawData, casa::Bool doLinear)
                                                         {return computeStokesDataForWriter(rawData,doLinear);};
  static casa::Array<casa::Float> computeStokesTSysForWriter (casa::Array<casa::Float>& rawData, casa::Bool doLinear)
                                                         {return computeStokesDataForWriter(rawData,doLinear);};

// This function is explicitly for SDWriter. It extracts the Beam and IF slice (specified in start
// and end - must be length 1 in beam and IF) and flips the pol and channel axes.
  static casa::Array<casa::Float> extractStokesForWriter (casa::Array<casa::Float>& in, const casa::IPosition& start, 
                                                          const casa::IPosition& end);

private:
// Return a label for the given StokesType
   static casa::String stokesString (casa::Stokes::StokesTypes stokes);

// specializations
  static casa::Array<casa::Float> andArrays (const casa::Array<casa::Float>& in1,
                                             const casa::Array<casa::Float>& in2) 
                                             {return (in1+in2)/casa::Float(2.0);}
  static casa::Array<casa::Bool> andArrays (const casa::Array<casa::Bool>& in1,
                                            const casa::Array<casa::Bool>& in2) 
                                            {return in1&&in2;}
  static casa::Array<casa::uChar> andArrays (const casa::Array<casa::uChar>& in1,
                                            const casa::Array<casa::uChar>& in2);
  
  template <class T>
  static casa::Array<T> computeStokesDataForWriter (casa::Array<T>& rawData, casa::Bool doLinear);
};





} // namespace

#endif
