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
//#include <synthesis/MeasurementComponents/ATerm.h>
#include <synthesis/TransformMachines/AzElAperture.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
//
//---------------------------------------------------------------------
//
namespace casa { //# NAMESPACE CASA - BEGIN
  template<class T> class ImageInterface;
  template<class T> class Matrix;
  class VisBuffer;
  //  class EVLAAperture : public ATerm
  class EVLAAperture : public AzElAperture
  {
  public:
    //    EVLAAperture(): ATerm(), polMap_p(), feedStokes_p() {};
    EVLAAperture(): AzElAperture(), polMap_p(), feedStokes_p() {};
    ~EVLAAperture() {};
    EVLAAperture& operator=(const EVLAAperture& other);
    //
    // Overload these functions.  They are pure virtual in the base class (ATerm).
    //
    virtual String name() {return String("EVLA Aperture");};

    virtual void makeFullJones(ImageInterface<Complex>& pbImage,
			       const VisBuffer& vb,
			       Bool doSquint, Int& bandID, Double freqVal);

    virtual void applySky(ImageInterface<Float>& outputImages,
			  const VisBuffer& vb, 
			  const Bool doSquint=True,
			  const Int& cfKey=0,
			  const Int& muellerTerm=0,
			  const Double freqVal=-1.0);
    virtual void applySky(ImageInterface<Complex>& outputImages,
			  const VisBuffer& vb, 
			  const Bool doSquint=True,
			  const Int& cfKey=0,
			  const Int& muellerTerm=0,
			  const Double freqVal=-1.0);
    virtual void applySky(ImageInterface<Complex>& outImages,
			  const Double& pa,
			  const Bool doSquint,
			  const Int& cfKey,
			  const Int& muellerTerm,
			  const Double freqVal=-1.0);

    void cacheVBInfo(const String& telescopeName, const Float& diameter);
    void cacheVBInfo(const VisBuffer& vb);
    Int getBandID(const Double& freq, const String& telescopeName);

    virtual Vector<Int> vbRow2CFKeyMap(const VisBuffer& vb, Int& nUnique)
    {Vector<Int> tmp; tmp.resize(vb.nRow()); tmp=0; nUnique=1; return tmp;}

    virtual void getPolMap(Vector<Int>& polMap) {polMap.resize(0);polMap=polMap_p;};

    // For this class, these will be served from the base classs (ATerm.h)
    // virtual Int getConvSize() {return CONVSIZE;};
    // virtual Int getOversampling() {return OVERSAMPLING;}
    // virtual Float getConvWeightSizeFactor() {return CONVWTSIZEFACTOR;};
    // virtual Float getSupportThreshold() {return THRESHOLD;};

  protected:
    int getVisParams(const VisBuffer& vb,const CoordinateSystem& skyCoord=CoordinateSystem());
    Bool findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R);
    Int getVLABandID(Double& freq,String&telescopeName, const CoordinateSystem& skyCoord=CoordinateSystem());
    Int makePBPolnCoords(const VisBuffer&vb,
			 const Int& convSize,
			 const Int& convSampling,
			 const CoordinateSystem& skyCoord,
			 const Int& skyNx, const Int& skyNy,
			 CoordinateSystem& feedCoord);

  private:
    Vector<Int> polMap_p;
    Vector<Int> feedStokes_p;
  };
};
#endif
