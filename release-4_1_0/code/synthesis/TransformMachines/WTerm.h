//# WTerm.h: Definition for WTerm
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

#ifndef SYNTHESIS_WTERM_H
#define SYNTHESIS_WTERM_H


#include <casa/Arrays/Vector.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <synthesis/TransformMachines/CFTerms.h>


namespace casa{
  // <summary>  A class to apply the W-Term to the given image </summary>
  
  // <use visibility=export>
  // <prerequisite>
  // </prerequisite>
  // <etymology>
  // W-Term to account for non co-planar baselines
  // </etymology>
  //
  // <synopsis> 
  // 
  //</synopsis>
  class WTerm: public CFTerms
  {
  public:
    WTerm () : CFTerms() {};
    ~WTerm () {};

    virtual void applySky(Matrix<Complex>& screen, 
			  const Int wPixel, 
			  const Vector<Double>& sampling,
			  const Double wScale,
			  const Int inner);
    virtual void applySky(Matrix<Complex>& screen, 
			  const Vector<Double>& sampling,
			  const Double wValue,
			  const Int inner);
    int getVisParams(const VisBuffer& vb,const CoordinateSystem& skyCoord=CoordinateSystem()) 
    {(void)vb;(void)skyCoord;return 0;};
    void setPolMap(const Vector<Int>& polMap) {(void)polMap;};
    virtual Float getSupportThreshold() {return 1e-3;};

    // WTerm normalizes the image be unity
    virtual void normalizeImage(Lattice<Complex>& skyImage,
				const Matrix<Float>& weights) 
    {(void)skyImage;(void)weights;};
    virtual String name() {return String("W Term");};
    //
    // The following functions are not required for W-Term but need to
    // be implemented here since they are pure virtuals in CFTerms
    // base class.
    void applySky(ImageInterface<Float>& outputImages,
		  const VisBuffer& vb, 
		  const Bool doSquint=True,
		  const Int& cfKey=0,
		  const Double /*freqVal*/=-1) 
    {(void)outputImages;(void)vb;(void)doSquint;(void)cfKey;}
    void applySky(ImageInterface<Complex>& outputImages,
		  const VisBuffer& vb, 
		  const Bool doSquint=True,
		  const Int& cfKey=0,
		  const Double /*freqVal*/=-1) 
    {(void)outputImages;(void)vb;(void)doSquint;(void)cfKey;};
    Vector<Int> vbRow2CFKeyMap(const VisBuffer& vb, Int& nUnique) 
    {(void)vb;(void)nUnique;return Vector<Int>();};
    Int makePBPolnCoords(const VisBuffer& vb,
			 const Int& convSize,
			 const Int& convSampling,
			 const CoordinateSystem& skyCoord,
			 const Int& skyNx, const Int& skyNy,
			 CoordinateSystem& feedCoord) 
    {
      (void)vb;(void)convSize;(void)convSampling;(void)skyCoord;(void)skyNx;(void)skyNy;(void)feedCoord;
      return 0;
    };

    Int getConvSize() {return 0;};
    Int getOversampling() {return 1;};
    Float getConvWeightSizeFactor() {return 1.0;};
  };

};

#endif
