//# PrincipalAxesDM.h: Base class for drawing axis-bound datasets
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
//# $Id$

#ifndef TRIALDISPLAY_PRINCIPALAXESDM_H
#define TRIALDISPLAY_PRINCIPALAXESDM_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayDatas/DisplayMethod.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvasHolder;
class IPosition;
class WorldCanvas;
template <class T> class Vector;

// <summary>
// Interface for DisplayMethods which have data arranged in "axes."
// </summary>
//
// <synopsis>
// This class adds to the interface defined by DisplayMethod to 
// provide further infrastructure relevant to data which is 
// arranged by axis (eg. lattice or column-based data).
// </synopsis>

class PrincipalAxesDM : public DisplayMethod {

 public:

  // User constructor.
  PrincipalAxesDM(uInt xAxis, uInt yAxis, uInt mAxis, 
		  PrincipalAxesDD *padd);
  
  // Destructor.
  virtual ~PrincipalAxesDM();

  // Draw on the provided WorldCanvasHolder.  This method provides
  // generic preparation that is common to all objects which are
  // being sliced along principal axes.  It calls the pure virtual
  // functions (below) which must be defined in fully typed derived
  // classes.
  virtual void draw(Display::RefreshReason reason,
		    WorldCanvasHolder &wcHolder);

  // clear drawlist state.
  virtual void cleanup();

 protected:

  // This method does setup stuff that is common to all elements
  // of an axis-bound display data element.
  virtual void setup(IPosition fixedPos);

  virtual void setup2d();

  // This method should be defined in derived classes to simply
  // return the shape of the data object, eg. Array.shape() or
  // Image.shape(), etc.
  virtual IPosition dataShape() = 0;

  // This method should be defined in derived classes to actually
  // draw the data contained in datMatrix, however it likes, starting
  // at the point blc, on *wCanvas.  It *must* return a uInt which 
  // indicates the drawListNumber it allocated for this drawing.
  // If <src>usePixelEdges</src> is True, then the given blc and
  // trc correspond to the world blc and trc of the first and last 
  // pixels in the given data, otherwise they correspond to the world
  // centres of the blc and trc pixels.
  virtual uInt dataDrawSelf(WorldCanvas *wCanvas,
			    const Vector<Double> &blc,
			    const Vector<Double> &trc,
			    const IPosition &start,
			    const IPosition &sliceShape,
			    const IPosition &stride,
			    const Bool usePixelEdges = False) = 0;
  
  // Called by draw(): an optimization for ColormapChange in 24bit mode.
  // Redraws the last image using only mapToColor on the WorldCanvas,
  // if possible.  If it returns True, the new method
  // WC::redrawIndexedImage() was used successfully (otherwise, draw()
  // continues in the normal way).  Override to enable, if necessary
  // (see LatticePADMRaster for an example).
  virtual Bool dataRedrawSelf(WorldCanvas*, Display::RefreshReason) {
    return False;
  };
  
  // Some data members which all display elements along principal
  // axes will play around with:
  IPosition start;
  IPosition sliceShape;
  IPosition stride;

  // Is a transpose necessary?
  virtual Bool needToTranspose() {
    return sliceShape(itsYAxisNum) >1 &&
	  (sliceShape(itsXAxisNum)==1 || itsXAxisNum>itsYAxisNum); }
	// The logic behind this cryptic code (see LatticePADM::dataGetSlice):
	// If a either a 1xN or Nx1 slice (including 1x1) is requested,
	// LatticePADM's latt.getSlice() Array will be 1-dimensional, which
	// the Matrix = Array operator will turn into an Nx1 matrix.
	// If, on the other hand, there is no degeneracy in the desired
	// slice Matrix, it is returned in lattice (not X,Y) order.  (dk)

  // (Required) default constructor.
  PrincipalAxesDM();

  // (Required) copy constructor.
  PrincipalAxesDM(const PrincipalAxesDM &other);

  // (Required) copy assignment.
  void operator=(const PrincipalAxesDM &other);
  
 private:

  // Axis numbers for internal book-keeping.
  uInt itsXAxisNum, itsYAxisNum, itsZAxisNum;

  // Drawlist state.  Moved here, where it's used, from DisplayMethod,
  // and made private.  11/03  dk.  The Caching side uses different
  // state (and purgeCache(), rather than cleanup()).

  Bool               notUsed;
  WorldCanvasHolder *holder;
  AttributeBuffer    drawState;
  uInt               drawListNumber;

};


} //# NAMESPACE CASA - END

#endif
