//# ATerm.h: Definition for ATerm
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

#ifndef SYNTHESIS_ATERM_H
#define SYNTHESIS_ATERM_H


#include <casa/Arrays/Vector.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <msvis/MSVis/VisBuffer.h>


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
  class ATerm
  {
  public:
    ATerm () {};
    virtual ~ATerm () {};

    virtual void applySky(Matrix<Complex>& screen, const Int wPixel, 
			  const Vector<Double>& sampling,
			  const Int wConvSize, const Double wScale,
			  const Int inner) 
    {(void)screen; (void)wPixel; (void)sampling; (void)wConvSize; (void)wScale; (void)inner;};

    virtual void applySky(ImageInterface<Float>& twoDPB, 
			  const VisBuffer& vb, 
			  const Bool doSquint=True)=0;
    virtual void applySky(ImageInterface<Complex>& twoDPB, 
			  const VisBuffer& vb, 
			  const Bool doSquint=True)=0;
    
    virtual Int makePBPolnCoords(const VisBuffer&vb,
				 const Vector<Int>& polMap,
				 const Int& convSize,
				 const Int& convSampling,
				 const CoordinateSystem& skyCoord,
				 const Int& skyNx, const Int& skyNy,
				 CoordinateSystem& feedCoord,
				 Vector<Int>& cfStokes) = 0;

    virtual void setPolMap(const Vector<Int>& polMap) = 0;
    virtual void setFeedStokes(const Vector<Int>& feedStokes) = 0;
    virtual void setParams(const Vector<Int>& polMap, const Vector<Int>& feedStokes)
    {setPolMap(polMap); setFeedStokes(feedStokes);};

    virtual Int getConvSize() = 0;
    virtual Float getConvWeightSizeFactor() = 0;
    virtual Int getOversampling() = 0;
    virtual Float getSupportThreshold() = 0;

    virtual void getPolMap(Vector<Int>& polMap) = 0;
    virtual void getFeedStokes(Vector<Int>& feedStokes) = 0;
    virtual void getParams(Vector<Int>& polMap, Vector<Int>& feedStokes)
    {getPolMap(polMap); getFeedStokes(feedStokes);};

    virtual int getVisParams(const VisBuffer& vb) = 0;
  protected:
    LogIO& logIO() {return logIO_p;}
    LogIO logIO_p;
  };

};

#endif
