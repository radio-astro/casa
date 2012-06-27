//# ScrollingRasterDM.cc: Base class for scrolling DisplayData objects
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2004
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
//

#include <display/DisplayDatas/ScrollingRasterDM.h>
#include <display/DisplayDatas/ScrollingRasterDD.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <casa/Arrays/MatrixMath.h>
#include <lattices/Lattices/MaskedLattice.h>

#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#define CDEBUG

ScrollingRasterDM::ScrollingRasterDM(uInt xAxis, uInt yAxis, Int mAxis, 
				      IPosition fixedPos, 
				      PrincipalAxesDD *padd):
  PrincipalAxesDM (xAxis, yAxis, mAxis, (PrincipalAxesDD *)padd){
#ifdef CDEBUG  
  cerr << "ScrollingRasterDM::ScrollingRasterDM(xAxis=" << xAxis
       << ",yAxis=" << yAxis << ",mAxis=" << mAxis << ",bAxis=" << bAxis
       << ",fixedPos=" << fixedPos << ",*padd)" << endl;
#endif
  setup(fixedPos);
 }

void ScrollingRasterDM::setup(IPosition ip){
#ifdef CDEBUG
  cerr << "ScrollingRasterDM::setup(ip=" << ip << ") called." << endl;
#endif
  PrincipalAxesDM::setup(ip);
}

IPosition ScrollingRasterDM::dataShape(){
  return (static_cast<ScrollingRasterDD *>(parentDisplayData()))->dataShape();
}

uInt ScrollingRasterDM::dataDrawSelf(WorldCanvas *wCanvas,
  	const Vector<Double> &blc,
	const Vector<Double> &trc,
	const IPosition &start,
	const IPosition &sliceShape,
	const IPosition &stride,
	const Bool usePixelEdges){
#ifdef CDEBUG
  cerr << " ScrollingRasterDM::dataDrawSelf: start = " << start
       << ", shape = " << sliceShape << endl;
#endif
  uInt drawListNumber = wCanvas->newList();
  ScrollingRasterDD *lar = (static_cast<ScrollingRasterDD *>(parentDisplayData()));
  
  // set min and max datavalues to help out scalehandler on WC.
  Attribute dmin("dataMin", Double(lar->getDataMin()));
  Attribute dmax("dataMax", Double(lar->getDataMax()));
  
  wCanvas->setAttribute(dmin);
  wCanvas->setAttribute(dmax);


  try {
    wCanvas->setResampleHandler(lar->resampleHandler());
    //wCanvas->setDataScaleHandler(lar->itsPowerScaleHandler);
    Matrix<Float> datMatrix;
    Matrix<Bool> maskMatrix;
    dataGetSlice(datMatrix, maskMatrix, start, sliceShape, stride);
    Bool useMask = (maskMatrix.nelements() == datMatrix.nelements());
    switch (wCanvas->pixelCanvas()->pcctbl()->colorModel()) {
    case Display::Index:
      {
        //lar->itsOptionsColorMode = "colormap"; // explicit
	if(1) { //if (lar->itsOptionsColorMode == "colormap") {
	  if (useMask) {
	    wCanvas->drawImage(blc, trc, datMatrix, maskMatrix, usePixelEdges);
	    //cerr << "ScrollingRasterDM -> with useMask:" << endl;
	  } else {
	    //cerr << "ScrollingRasterDM -> without useMask:K" << endl;
	    wCanvas->drawImage(blc, trc, datMatrix, usePixelEdges);
	  }
	} else {
	  LogIO os;
	  os << LogIO::WARN << LogOrigin("ScrollingRasterDM", 
					 "dataDrawSelf", WHERE) 
	     << "Invalid colormap mode for an Indexed PixelCanvas" 
	     << LogIO::POST;
	}
      }
      break;

/*
    case Display::RGB:
#ifdef CDEBUG
      cerr << "colorModel = RGB" << endl;
#endif
      lar->itsOptionsColorMode = "red";
      if (useMask) {
	//cerr << "mask available, but being ignored in "
	//    << "ScrollingRasterDM::dataDrawSelf" << endl;
      }
      if (lar->itsOptionsColorMode == "red") {
	wCanvas->drawImage(blc, trc, datMatrix, Display::Red, usePixelEdges);
      } else if (lar->itsOptionsColorMode == "green") {
	wCanvas->drawImage(blc, trc, datMatrix, Display::Green, usePixelEdges);
      } else if (lar->itsOptionsColorMode == "blue") {
	wCanvas->drawImage(blc, trc, datMatrix, Display::Blue, usePixelEdges);
      } else {
	LogIO os;
	os << LogIO::WARN << LogOrigin("ScrollingRasterDM", 
				       "dataDrawSelf", WHERE) 
	   << "Invalid colormap mode for an RGB PixelCanvas" 
	   << LogIO::POST;
      }
      break;
    case Display::HSV:
#ifdef CDEBUG
      cerr << "colorModel = HSV" << endl;
#endif
      lar->itsOptionsColorMode = "value";
      if (useMask) {
	// cerr << "mask available, but being ignored in "
	 //    << "ScrollingRasterDM::dataDrawSelf" << endl;
      }
      if (lar->itsOptionsColorMode == "hue") {
	wCanvas->drawImage(blc, trc, datMatrix, Display::Hue, usePixelEdges);
      } else if (lar->itsOptionsColorMode == "saturation") {
	wCanvas->drawImage(blc, trc, datMatrix, Display::Saturation,
			   usePixelEdges);
      } else if (lar->itsOptionsColorMode == "value") {
	wCanvas->drawImage(blc, trc, datMatrix, Display::Value,
			   usePixelEdges);
      } else {
	  LogIO os;
	  os << LogIO::WARN << LogOrigin("ScrollingRasterDM", 
					 "dataDrawSelf", WHERE) 
	     << "Invalid colormap mode for an HSV PixelCanvas" 
	     << LogIO::POST;
      }
      break;
 */
    default:
      throw(AipsError("Unknown PixelCanvas ColorModel in "
		      "ScrollingRasterDM::dataDrawSelf"));
      break;
    }
//    wCanvas->setDataScaleHandler(0);
//    wCanvas->setResampleHandler(0);
  } catch (const AipsError &x) {
    wCanvas->endList();
    if (wCanvas->validList(drawListNumber)) {
      wCanvas->deleteList(drawListNumber);
    }
    throw(AipsError(x));
  }
  
  wCanvas->endList();
  
  return drawListNumber;
	
}

Bool ScrollingRasterDM::dataGetSlice(Matrix<Float>& data,
		     Matrix<Bool> &mask,
		     const IPosition& start,
	     	     const IPosition& sliceShape,
		     const IPosition& stride)  

//
// It is assumed that sliceShape has already been trimmed so
// that it doesn't dangle over the edge of the lattice
//
{
#ifdef CDEBUG
  cerr << "ScRDM::dataGetSlice: sliceShape = " << sliceShape << endl;
  cerr << "   data : " << data.shape() << endl;
  cerr << "   mask : " << mask.shape() << endl;
  cerr << "   start = " << start << endl;
  cerr << "   stride = " << stride << endl;
#endif  
  MaskedLattice<Float> *latt = 
    (static_cast<ScrollingRasterDD *>(parentDisplayData()))->maskedLattice();

  static Matrix<Float> tmpData;
  static Matrix<Bool> tmpMask;
  IPosition s;
  if (sliceShape.nelements() > 2) {
    s = sliceShape.nonDegenerate();
    //s = sliceShape.nonDegenerate(2);
#ifdef CDEBUG
    cerr << "sliceShape truncated to s = " << s << endl;
#endif
  } else {
    s = sliceShape;
  }
#ifdef CDEBUG
  cerr << "s.nelements = " << s.nelements() << endl;
#endif
  if (s.nelements()==2) {
    // I think this should always be true, but in case its not, other
    // safe code is offered
     if (!tmpData.shape().isEqual(s)) tmpData.resize(s(0),s(1));
     if (!tmpMask.shape().isEqual(s)) tmpMask.resize(s(0),s(1));
  } else {
  
     cerr << "  not 2D ?!" << endl;
     tmpData.resize(0,0);
     tmpMask.resize(0,0);
  }

  Bool allGood = True;
  
  tmpData = latt->getSlice(start, sliceShape, stride, True);
  tmpMask = latt->getMaskSlice(start, sliceShape, stride, True);

  if (anyEQ(tmpMask, False)) allGood = False;

  // reset Matrices, so they can resize automatically
  data.resize(0,0);
  mask.resize(0,0);
  if (needToTranspose()) {
    data = transpose(tmpData);
    if (!allGood) {
       mask = transpose(tmpMask);
    } else {
       mask.resize(0,0);
    }
  } else {
    data = tmpData;
    if (!allGood) {
       mask = tmpMask;
    } else {      
       mask.resize(0,0);
    }
  }
  return True;
}


} //# NAMESPACE CASA - END

