//# PolAverageTVI.h: Transforming VI for polarization averaging
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the Implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $

#ifndef _MSVIS_POLAVERAGINGTVI_H_
#define _MSVIS_POLAVERAGINGTVI_H_

#include <casacore/casa/aips.h>
#include <msvis/MSVis/ViImplementation2.h>
#include <msvis/MSVis/TransformingVi2.h>
#include <msvis/MSVis/VisibilityIterator2.h>

#include <map>
#include <vector>

#include <casacore/measures/Measures/Stokes.h>

namespace casacore {

template<typename T> class Vector;
}

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { // # NAMESPACE VI - BEGIN

//# forward decl

class VisBuffer2;

class ChannelSelector;
class ChannelSelectorCache;
typedef casacore::Vector<casacore::Vector<casacore::Slice> > ChannelSlicer;
class SpectralWindowChannelsCache;
class SpectralWindowChannels;
class SubtableColumns;

class PolAverageVi2Factory;

// <summary>
// PolAverageTVI
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MSIter">MSIter</linkto>
//   <li> <linkto class="casacore::MeasurementSet">casacore::MeasurementSet</linkto>
//   <li> <linkto class="VisSet">VisSet</linkto>
// </prerequisite>
//
// <etymology>
// PolAverageTVI
// </etymology>
//
// <synopsis>
//
// </synopsis>
//
// <example>
// <code>
// //
// </code>
// </example>
//
// <motivation>
//
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="1997/05/30">
//   <li> cleanup the currently dual interface for visibilities and flags
//   <li> sort out what to do with weights when interpolating
// </todo>

class PolAverageTVI: public TransformingVi2 {

public:

  // Destructor

  virtual ~PolAverageTVI();

  // Report the the ViImplementation type
  //  (should be specialized in child classes)
  virtual casacore::String ViiType() const {
    return casacore::String("PolAverage( ") + getVii()->ViiType() + " )";
  }

  // Return the correlation type (returns casacore::Stokes enums)

//  virtual void corrType(casacore::Vector<casacore::Int> & corrTypes) const;

  // Return flag for each polarization, channel and row

  virtual void flag(casacore::Cube<casacore::Bool> & flags) const;

  // Return flag for each channel & row

  virtual void flag(casacore::Matrix<casacore::Bool> & flags) const;

  // Return feed configuration matrix for specified antenna
  // Must fail
  virtual void jonesC(
      casacore::Vector<casacore::SquareMatrix<casacore::Complex, 2> > & cjones) const;

  // Return sigma

  virtual void sigma(casacore::Matrix<casacore::Float> & sigmat) const;

  // Return the visibilities as found in the casacore::MS, casacore::Cube (npol,nchan,nrow).

  virtual void visibilityCorrected(
      casacore::Cube<casacore::Complex> & vis) const;
  virtual void visibilityModel(casacore::Cube<casacore::Complex> & vis) const;
  virtual void visibilityObserved(
      casacore::Cube<casacore::Complex> & vis) const;

  // Return FLOAT_DATA as a casacore::Cube (npol, nchan, nrow) if found in the MS.

  virtual void floatData(casacore::Cube<casacore::Float> & fcube) const;

  // Return the shape of the visibility Cube

  virtual casacore::IPosition visibilityShape() const;

  // Return weight
  // TODO

  virtual void weight(casacore::Matrix<casacore::Float> & wtmat) const;

  // Return weightspectrum (a weight for each channel)
  // TODO

  virtual void weightSpectrum(casacore::Cube<casacore::Float> & wtsp) const;
  virtual void sigmaSpectrum(casacore::Cube<casacore::Float> & wtsp) const;

  // Return imaging weight (a weight for each channel)
  // virtual casacore::Matrix<casacore::Float> & imagingWeight (casacore::Matrix<casacore::Float> & wt) const;

  virtual const VisImagingWeight & getImagingWeightGenerator() const;

protected:

  // Constructor

  PolAverageTVI(ViImplementation2 * inputVi);

  // transform data (DATA, CORRECTED_DATA, MODEL_DATA, FLOAT_DATA)
  virtual void transformComplexData(
      casacore::Cube<casacore::Complex> const &dataIn,
      casacore::Cube<casacore::Bool> const &flagIn,
      casacore::Cube<casacore::Complex> &dataOut) const = 0;
  virtual void transformFloatData(casacore::Cube<casacore::Float> const &dataIn,
      casacore::Cube<casacore::Bool> const &flagIn,
      casacore::Cube<casacore::Float> &dataOut) const = 0;

  // transform weight (WEIGHT, WEIGHT_SPECTRUM)
  // TODO
  virtual void transformWeight(casacore::Cube<casacore::Float> const &weightIn,
      casacore::Cube<casacore::Float> &weightOut) const = 0;

private:

  friend PolAverageVi2Factory;
};

class GeometricPolAverageTVI: public PolAverageTVI {
public:
  GeometricPolAverageTVI(ViImplementation2 * inputVi);
  ~GeometricPolAverageTVI();

  // Report the the ViImplementation type
  //  (should be specialized in child classes)
  virtual casacore::String ViiType() const {
    return casacore::String("GeometricPolAverage( ") + getVii()->ViiType()
        + " )";
  }

  // transform data (DATA, CORRECTED_DATA, MODEL_DATA, FLOAT_DATA)
  virtual void transformComplexData(
      casacore::Cube<casacore::Complex> const &dataIn,
      casacore::Cube<casacore::Bool> const &flagIn,
      casacore::Cube<casacore::Complex> &dataOut) const;
  virtual void transformFloatData(casacore::Cube<casacore::Float> const &dataIn,
      casacore::Cube<casacore::Bool> const &flagIn,
      casacore::Cube<casacore::Float> &dataOut) const;

  // transform weight (WEIGHT, WEIGHT_SPECTRUM)
  // TODO
  virtual void transformWeight(casacore::Cube<casacore::Float> const &weightIn,
      casacore::Cube<casacore::Float> &weightOut) const;

protected:

  template<class T>
  void transformData(casacore::Cube<T> const &dataIn,
      casacore::Cube<casacore::Bool> const &flagIn,
      casacore::Cube<T> &dataOut) const;
};

class StokesPolAverageTVI: public PolAverageTVI {
public:
  StokesPolAverageTVI(ViImplementation2 * inputVi);
  ~StokesPolAverageTVI();

  // Report the the ViImplementation type
  //  (should be specialized in child classes)
  virtual casacore::String ViiType() const {
    return casacore::String("StokesPolAverage( ") + getVii()->ViiType() + " )";
  }

  // transform data (DATA, CORRECTED_DATA, MODEL_DATA, FLOAT_DATA)
  virtual void transformComplexData(
      casacore::Cube<casacore::Complex> const &dataIn,
      casacore::Cube<casacore::Bool> const &flagIn,
      casacore::Cube<casacore::Complex> &dataOut) const;
  virtual void transformFloatData(casacore::Cube<casacore::Float> const &dataIn,
      casacore::Cube<casacore::Bool> const &flagIn,
      casacore::Cube<casacore::Float> &dataOut) const;

  // transform weight (WEIGHT, WEIGHT_SPECTRUM)
  // TODO
  virtual void transformWeight(casacore::Cube<casacore::Float> const &weightIn,
      casacore::Cube<casacore::Float> &weightOut) const;

protected:

  template<class T>
  void transformData(casacore::Cube<T> const &dataIn,
      casacore::Cube<casacore::Bool> const &flagIn,
      casacore::Cube<T> &dataOut) const;
};

// <summary>
// A factory for generating ViImplementation2 for polarization averaging.
// </summary>
//
// <use visibility=export>
//
// <prerequisite>
//   <li> <linkto class="VisibilityIterator2:description">VisibilityIterator2</linkto>
// </prerequisite>
//
// <etymology>
// Factory for layered ViImplementation2 construction
// </etymology>
//
// <synopsis>
// PolAverageVi2Factory
// </synopsis>
//
// <motivation>
//
// </motivation>
//
// <example>
//
// </example>

class PolAverageVi2Factory: public ViFactory {

public:

  // Constructor
  PolAverageVi2Factory(casacore::Record const &configuration,
      ViImplementation2 *inputVII);

  // Destructor
  ~PolAverageVi2Factory();

  ViImplementation2 * createVi() const;

private:

  enum AveragingMode {
    GEOMETRIC, STOKES, NUM_MODES, DEFAULT = GEOMETRIC
  };

  ViImplementation2 *inputVII_p;

  AveragingMode mode_;
};

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif // _MSVIS_POLAVERAGINGTVI_H_

