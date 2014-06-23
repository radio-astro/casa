//# CFTerms.h: Definition for CFTerms
//# Copyright (C) 2007
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_CFTERMS_H
#define SYNTHESIS_CFTERMS_H


#include <casa/Arrays/Vector.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Containers/Block.h>

namespace casa{
  // <summary>  
  //  The base class to represent the Aperture-Term of the Measurement Equation. 
  // </summary>
  
  // <use visibility=export>
  // <prerequisite>
  // </prerequisite>
  // <etymology>
  //   A-Term to account for the effects of the antenna primary beam(s).
  // </etymology>
  //
  // <synopsis> 
  // 
  //</synopsis>
  class CFTerms
  {
  public:
    enum OpCodes {NOOP=0,NORMAL};
    CFTerms ():opCode_p(NORMAL) {};
    virtual ~CFTerms () {};

    virtual void setOpCode(OpCodes code) {opCode_p = code;}
    virtual Bool isNoOp() {return opCode_p==NOOP;};

    virtual String name() = 0;

    virtual void applySky(ImageInterface<Float>& outputImages,
			  const VisBuffer& vb, 
			  const Bool doSquint=True,
			  const Int& cfKey=0,
			  const Double freqVal=-1) = 0;
    virtual void applySky(ImageInterface<Complex>& outputImages,
			  const VisBuffer& vb, 
			  const Bool doSquint=True,
			  const Int& cfKey=0,
			  const Double freqVal=-1) = 0;
    //
    // Not sure if the following method is requried.  Leaving it in
    // the code for now with an implementation that does nothing.
    //
    // virtual void applySky(Matrix<Complex>& screen, const Int wPixel, 
    // 			  const Vector<Double>& sampling,
    // 			  const Int wConvSize, const Double wScale,
    // 			  const Int inner) 
    // {(void)screen; (void)wPixel; (void)sampling; (void)wConvSize; (void)wScale; (void)inner;};

    //
    // Returns a vector of integers that map each row in the given
    // VisBuffer to an index that is used to pick the appropriate
    // convolution function plane.  It also returns the number of
    // unique baselines in the nUnique parameter (unique baselines are
    // defined as the number of baselines each requiring a unique
    // convolution function).
    //
    // This is required for Heterogeneous antenna arrays (like ALMA)
    // and for all arrays where not all antenna aperture illuminations
    // can be treated as identical.
    //
    virtual Vector<Int> vbRow2CFKeyMap(const VisBuffer& vb, Int& nUnique) = 0;
    virtual Int makePBPolnCoords(const VisBuffer& vb,
				 const Int& convSize,
				 const Int& convSampling,
				 const CoordinateSystem& skyCoord,
				 const Int& skyNx, const Int& skyNy,
				 CoordinateSystem& feedCoord) = 0;

    virtual Int getConvSize() = 0;
    virtual Int getOversampling() = 0;
    virtual Float getConvWeightSizeFactor() = 0;
    virtual Float getSupportThreshold() = 0;
    
    virtual void normalizeImage(Lattice<Complex>& skyImage,
				const Matrix<Float>& weights) = 0;

    virtual int getVisParams(const VisBuffer& vb, const CoordinateSystem& skyCoord=CoordinateSystem()) = 0;
    //
    // The mapping from VisBuffer polarizations map to the Image plane
    // polarization.  The latter is determined by the user input,
    // which is passed to the FTMachine in Imager.cc
    //
    // The map is available in the FTMachine which uses this method to
    // set the map for the CFTerms object.
    //
    virtual void setPolMap(const Vector<Int>& polMap) {polMap_p_base.resize(0);polMap_p_base=polMap;}
    virtual void getPolMap(Vector<Int>& polMap) {polMap.resize(0); polMap = polMap_p_base;};



  protected:
    LogIO& logIO() {return logIO_p;}
    LogIO logIO_p;
    Vector<Int> polMap_p_base;
    OpCodes opCode_p;
  };

};

#endif
