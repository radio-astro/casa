// -*- C++ -*-
//# EVLAAperture.h: Definition of the EVLAAperture class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
#ifndef SYNTHESIS_EVLAAPERTURE_H
#define SYNTHESIS_EVLAAPERTURE_H

#include <images/Images/ImageInterface.h>
#include <synthesis/MeasurementComponents/ATerm.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
//
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// TEMPS The following #defines should REALLLLAY GO!
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//
#define CONVSIZE (1024*2)
#define CONVWTSIZEFACTOR 1.0
#define OVERSAMPLING 20
#define THRESHOLD 1E-4

namespace casa { //# NAMESPACE CASA - BEGIN
  template<class T> class ImageInterface;
  template<class T> class Matrix;
  class VisBuffer;
  class EVLAAperture : public ATerm
  {
  public:
    EVLAAperture():     
      ATerm(), polMap_p(), feedStokes_p()
    {};
    ~EVLAAperture() {};
    EVLAAperture& operator=(const EVLAAperture& other);
    Int getVLABandID(Double& freq,String&telescopeName);
    //
    // Overload these functions.  They are pure virtual in the base class (ATerm).
    //
    Bool findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R);

    void applySky(ImageInterface<Float>& twoDPB, 
		  const VisBuffer& vb, const Bool doSquint=True);
    void applySky(ImageInterface<Complex>& twoDPB, 
		  const VisBuffer& vb, const Bool doSquint=True);

    int getVisParams(const VisBuffer& vb);

    Int makePBPolnCoords(const VisBuffer&vb,
			 const Vector<Int>& polMap,
			 const Int& convSize,
			 const Int& convSampling,
			 const CoordinateSystem& skyCoord,
			 const Int& skyNx, const Int& skyNy,
			 CoordinateSystem& feedCoord,
			 Vector<Int>& cfStokes);
    virtual void setPolMap(const Vector<Int>& polMap);
    virtual void setFeedStokes(const Vector<Int>& feedStokes);
    virtual void getPolMap(Vector<Int>& polMap) {polMap.resize(0);polMap=polMap_p;};
    virtual void getFeedStokes(Vector<Int>& feedStokes) 
    {feedStokes.resize(0);feedStokes = feedStokes_p;};
    virtual Int getConvSize() {return CONVSIZE;};
    virtual Float getConvWeightSizeFactor() {return CONVWTSIZEFACTOR;};
    virtual Int getOversampling() {return OVERSAMPLING;}
    virtual Float getSupportThreshold() {return THRESHOLD;};
  private:
    Float Diameter_p, Nant_p, HPBW, sigma;
    Vector<Int> polMap_p;
    Vector<Int> feedStokes_p;
  };
};
#endif
