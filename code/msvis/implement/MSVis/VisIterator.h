//# VisIterator.h: Step through the MeasurementEquation by visibility
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
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
//# $Id: VisIterator.h,v 19.14 2006/02/28 04:48:58 mvoronko Exp $

#ifndef MSVIS_VISITERATOR_H
#define MSVIS_VISITERATOR_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Containers/Stack.h>
#include <casa/Containers/OrderedMap.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVDoppler.h>
#include <measures/Measures/MCDoppler.h>
#include <measures/Measures/MDoppler.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/String.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <scimath/Mathematics/RigidVector.h>

#include <ms/MeasurementSets/MSDerivedValues.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisImagingWeight.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <ms/MeasurementSets/MSIter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forward decl
class VisBuffer;

// <summary>
// ROVisIterator iterates through one or more readonly MeasurementSets
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MSIter">MSIter</linkto>
//   <li> <linkto class="MeasurementSet">MeasurementSet</linkto>
//   <li> <linkto class="VisSet">VisSet</linkto>
// </prerequisite>
//
// <etymology>
// The ROVisIterator is a readonly iterator returning visibilities
// </etymology>
//
// <synopsis>
// ROVisIterator provides iteration with various sort orders
// for one or more MSs. It has member functions to retrieve the fields
// commonly needed in synthesis calibration and imaging.
//
// One should use <linkto class="VisBuffer">VisBuffer</linkto>
// to access chunks of data.
// </synopsis>
//
// <example>
// <code>
// //
// </code>
// </example>
//
// <motivation>
// For imaging and calibration you need to access an MS in some consistent
// order (by field, spectralwindow, time interval etc.). This class provides
// that access.
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

class ROVisIterator : public ROVisibilityIterator
{
public:

  // Default constructor - useful only to assign another iterator later
  ROVisIterator();

  // Construct from MS and a Block of MS column enums specifying the 
  // iteration order, if none are specified, time iteration is implicit.
  // An optional timeInterval can be given to iterate through chunks of time.
  // The default interval of 0 groups all times together.
  // Every 'chunk' of data contains all data within a certain time interval
  // and with identical values of the other iteration columns (e.g.
  // SPECTRAL_WINDOW_ID and FIELD_ID).
  // Using selectChannel(), a number of groups of channels can be requested.
  // At present the channel group iteration will always occur before the 
  // interval iteration.
  ROVisIterator(const MeasurementSet& ms, 
		const Block<Int>& sortColumns, 
		Double timeInterval=0);

  // Copy construct. This calls the assigment operator.
  ROVisIterator(const ROVisIterator & other);

  // Destructor
  virtual ~ROVisIterator();

  // Assigment. Any attached VisBuffers are lost in the assign.
  ROVisIterator & operator=(const ROVisIterator &other);

  // Members
  
  // Advance iterator through data
  ROVisIterator & operator++(int);
  ROVisIterator & operator++();

  // Return channel numbers in selected VisSet spectrum
  // (i.e. disregarding possible selection on the iterator, but
  //  including the selection set when creating the VisSet)
  Vector<Int>& channel(Vector<Int>& chan) const;
  Vector<Int>& chanIds(Vector<Int>& chanids) const;
  Vector<Int>& chanIds(Vector<Int>& chanids,Int spw) const;

  // Return selected correlation indices
  Vector<Int>& corrIds(Vector<Int>& corrids) const;

  // Return flag for each polarization, channel and row
  Cube<Bool>& flag(Cube<Bool>& flags) const;

  // Return current frequencies
  Vector<Double>& frequency(Vector<Double>& freq) const;

  // Return the correlation type (returns Stokes enums)
  Vector<Int>& corrType(Vector<Int>& corrTypes) const;

  // Return sigma matrix (pol-dep)
  Matrix<Float>& sigmaMat(Matrix<Float>& sigmat) const;

  // Return the visibilities as found in the MS, Cube(npol,nchan,nrow).
  Cube<Complex>& visibility(Cube<Complex>& vis,
			    DataColumn whichOne) const;
  // Return weight matrix
  Matrix<Float>& weightMat(Matrix<Float>& wtmat) const;

  // Return weightspectrum (a weight for each corr & channel)
  Cube<Float>& weightSpectrum(Cube<Float>& wtsp) const;

  // Set up new chan/corr selection via Vector<Slice>
  void selectChannel(const Vector<Vector<Slice> >& chansel);
  void selectCorrelation(const Vector<Vector<Slice> >& corrsel);

  // Set up/return channel averaging bounds 
  Vector<Matrix<Int> >& setChanAveBounds(Float factor, Vector<Matrix<Int> >& bounds);

  // Return number of chans/corrs per spw/pol
  Int numberChan(Int spw) const;
  Int numberCorr(Int pol) const;

  // Return the row ids as from the original root table. This is useful 
  // to find correspondance between a given row in this iteration to the 
  // original ms row
  virtual Vector<uInt>& rowIds(Vector<uInt>& rowids) const; 

protected:
  virtual void setSelTable();

  virtual const Table attachTable() const;

  // update the DATA slicer
  virtual void updateSlicer();
  // attach the column objects to the currently selected table

  using ROVisibilityIterator::getDataColumn;
  virtual void getDataColumn(DataColumn whichOne, const Vector<Vector<Slice> >& slices, 
			     Cube<Complex>& data) const;

  Table selTable_p;

  // New slicer supports multiple Slices in channel and correlation
  Vector<Vector<Slice> > chanSlices_p, corrSlices_p, newSlicer_p, newWtSlicer_p;
  Bool useNewSlicer_p;
  Vector<Matrix<Int> > chanAveBounds_p;

  // Column access functions
  virtual void getCol(const ROScalarColumn<Bool> &column, Vector<Bool> &array, Bool resize = False) const;
  virtual void getCol(const ROScalarColumn<Int> &column, Vector<Int> &array, Bool resize = False) const;
  virtual void getCol(const ROScalarColumn<Double> &column, Vector<Double> &array, Bool resize = False) const;

  virtual void getCol(const ROArrayColumn<Bool> &column, Array<Bool> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Float> &column, Array<Float> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Double> &column, Array<Double> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Complex> &column, Array<Complex> &array, Bool resize = False) const;

  virtual void getCol(const ROArrayColumn<Bool> &column, const Slicer &slicer, Array<Bool> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Float> &column, const Slicer &slicer, Array<Float> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Complex> &column, const Slicer &slicer, Array<Complex> &array, Bool resize = False) const;
};

// <summary>
// VisIterator iterates through one or more writable MeasurementSets
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="ROVisIterator">ROVisIterator</linkto>
// </prerequisite>
//
// <etymology>
// The VisIterator is a read/write iterator returning visibilities
// </etymology>
//
// <synopsis>
// VisIterator provides iteration with various sort orders
// for one or more MSs. It has member functions to retrieve the fields
// commonly needed in synthesis calibration and imaging. It is 
// derived from the read-only iterator
// <linkto class="ROVisIterator">ROVisIterator</linkto>.
//
// One should use <linkto class="VisBuffer">VisBuffer</linkto>
// to access chunks of data.
// </synopsis>
//
// <example>
// <code>
// //
// </code>
// </example>
//
// <motivation>
// For imaging and calibration you need to access an MS in some consistent
// order (by field, spectralwindow, time interval etc.). This class provides
// that access.
// </motivation>
//
// #<thrown>
//
// #</thrown>
//
// <todo asof="1997/05/30">
//   <li> cleanup the currently dual interface for visibilities and flags
//   <li> sort out what to do with weights when interpolating
// </todo>

class VisIterator : public ROVisIterator
{
public:

  // Constructors.
  // Note: The VisIterator is not initialized correctly by default, you
  // need to call origin() before using it to iterate.
  VisIterator();
  VisIterator(MeasurementSet & ms, const Block<Int>& sortColumns, 
	      Double timeInterval=0);

  VisIterator(const VisIterator & MSI);

  // Destructor
  virtual ~VisIterator();

  VisIterator & operator=(const VisIterator &MSI);

  // Members
  
  // Advance iterator through data
  VisIterator & operator++(int);
  VisIterator & operator++();

  // Set/modify the flag row column; dimension Vector(nrow)
  void setFlagRow(const Vector<Bool>& rowflags);

  // Set/modify the flags in the data.
  // This sets the flags as found in the MS, Cube(npol,nchan,nrow),
  // where nrow is the number of rows in the current iteration (given by
  // nRow()).
  void setFlag(const Cube<Bool>& flag);

  // Set/modify the visibilities
  // This sets the data as found in the MS, Cube(npol,nchan,nrow).
  void setVis(const Cube<Complex>& vis, DataColumn whichOne);

  // Set the visibility and flags, and interpolate from velocities if needed
  void setVisAndFlag(const Cube<Complex>& vis, const Cube<Bool>& flag,
		     DataColumn whichOne);

  // Set/modify the weightMat
  void setWeightMat(const Matrix<Float>& wtmat);

  // Set/modify the weightSpectrum
  void setWeightSpectrum(const Cube<Float>& wtsp);

protected:
  virtual void attachColumns(const Table &t);

  // deals with Float or Complex observed data (DATA and FLOAT_DATA).
  void putDataColumn(DataColumn whichOne, const Vector<Vector<Slice> >& slices,
		     const Cube<Complex>& data);
  void putDataColumn(DataColumn whichOne, const Cube<Complex>& data);

  // column access functions
  virtual void putCol(ScalarColumn<Bool> &column, const Vector<Bool> &array);

  virtual void putCol(ArrayColumn<Bool> &column, const Array<Bool> &array);
  virtual void putCol(ArrayColumn<Float> &column, const Array<Float> &array);
  virtual void putCol(ArrayColumn<Complex> &column, const Array<Complex> &array);

  virtual void putCol(ArrayColumn<Bool> &column, const Slicer &slicer, const Array<Bool> &array);
  virtual void putCol(ArrayColumn<Float> &column, const Slicer &slicer, const Array<Float> &array);
  virtual void putCol(ArrayColumn<Complex> &column, const Slicer &slicer, const Array<Complex> &array);

  ArrayColumn<Complex> RWcolVis;
  ArrayColumn<Float> RWcolFloatVis;
  ArrayColumn<Complex> RWcolModelVis;
  ArrayColumn<Complex> RWcolCorrVis;
  ArrayColumn<Float> RWcolWeight;
  ArrayColumn<Float> RWcolWeightSpectrum;
  ArrayColumn<Float> RWcolSigma;
  ArrayColumn<Float> RWcolImagingWeight;
  ArrayColumn<Bool> RWcolFlag;
  ScalarColumn<Bool> RWcolFlagRow;


};


} //# NAMESPACE CASA - END

#endif
