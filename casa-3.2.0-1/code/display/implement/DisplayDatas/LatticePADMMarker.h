//# LatticePADMMarker.h: drawing axis bound lattice elements as Marker fields
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
//# $Id$

#ifndef TRIALDISPLAY_LATTICEPADMMARKER_H
#define TRIALDISPLAY_LATTICEPADMMARKER_H

//# aips includes:
#include <casa/aips.h>

//# display library includes:
#include <display/DisplayDatas/LatticePADM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forwards:
class IPosition;
class WorldCanvas;
template <class T> class LatticePADisplayData;
template <class T> class Vector;
template <class T> class Matrix;

// <summary>
// Class to draw a single vector map of a slice from an AIPS++ Lattice.
// </summary>
//
// <use visibility=local>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
// <li> <linkto class="LatticeAsMarker">LatticeAsMarker</linkto>
// </prerequisite>
//
// <etymology>
// The purpose of this class is to draw "vector" maps of data that
// are "lattice"-based.  The "PADM" refers to PrincipalAxesDisplayMethod,
// meaning that one or more instances of this class are used to actually
// draw slices of a Lattice along its main axes.
// </etymology>
//
// <synopsis>
// This is a helper class for the LatticeAsMarker class.  One or more
// instances of this class are created by a single LatticeAsMarker
// object, each being responsible for drawing a different slice of the 
// data.
// </synopsis>
//
// <example>
// This class should only be used by the LatticeAsMarker class to
// setup a number of views of an AIPS++ Image or Array.  As such, this
// example simply outlines how this class is used by LatticeAsMarker,
// in, for example, a support function for a constructor:
// <srcblock>
// void LatticeAsMarker::setupElements(IPosition fixedPos) {
//   if (nPixelAxes > 2) {
//     nImages = dataLattice()->shape()(zAxisNum);
//     DDelement.resize(nImages);
//     for (uInt index = 0; index < nImages; index++) {
//       fixedPos(zAxisNum) = index;
//       DDelement[index] = (LatticePADisplayMethod<Complex> *)new 
//       LatticePADMVector(dataLattice(), xAxisNum, yAxisNum,
//                            zAxisNum, fixedPos, this);
//     }
//   } else {
//     nImages = 1;
//     DDelement.resize(nImages);
//     DDelement[0] = (LatticePADisplayMethod<Complex> *)new
//       LatticePADMVector(dataLattice(), xAxisNum, yAxisNum, this);
//   }
// }
// </srcblock>
// </example>
//
// <motivation>
// Displaying 2-dimensional slices of a lattice-based data volume is
// a standard display requirement for astronomical data visualization
// and presentation.
// </motivation>
//
// <thrown>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

template <class T> class LatticePADMMarker : public LatticePADisplayMethod<T> {

 public:
  
  // Constructors: >2d and 2d.  xAxis and yAxis specify which axis in 
  // the Lattice (0-based) should be mapped to X and Y on the display
  // device: ie. 2-d slices of the data to be displayed have these as
  // axes.  mAxis specifies the "movie" axis, which is the axis along
  // which different slices are taken.  fixedPos is an IPosition
  // having the same length as the number of dimensions in the array, 
  // and indicates the fixed axis values for axes in the data that are
  // not specified as xAxis or yAxis: indeed, <src>fixedPos(mAxis)</src>
  // indicates which pixel value along the movie axis that this
  // particular object looks after.
  // <group>
  LatticePADMMarker(const uInt xAxis,
                    const uInt yAxis, const uInt mAxis,
                    const IPosition fixedPos,
                    LatticePADisplayData<T>* arDat);
  LatticePADMMarker(const uInt xAxis,
                    const uInt yAxis, LatticePADisplayData<T>* arDat);
  // </group>

  // Destructor
  virtual ~LatticePADMMarker();

  // Actually draw on the display device.  The WorldCanvasHolder will
  // tell the LatticeAsMarker that it should now draw, which will in
  // turn determine which of its one or more LatticePADMVector objects 
  // should draw by matching the movie value on the WorldCanvas.  The 
  // vector is drawn in the world coordinate range blc to trc.
  virtual uInt dataDrawSelf(WorldCanvas *wCanvas,
			    const Vector<Double> &blc,
			    const Vector<Double> &trc,
			    const IPosition &start,
			    const IPosition &shape,
			    const IPosition &stride,
			    const Bool usePixelEdges = False);

 private:
//
  Matrix<Float> getAmplitude(const Matrix<T>& data) const;

  //# Make parent members known.
protected:
  using LatticePADisplayMethod<T>::parentDisplayData;
};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/DisplayDatas/LatticePADMMarker.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
