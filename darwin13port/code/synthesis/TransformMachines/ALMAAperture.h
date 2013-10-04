//# ALMAAperture.h: Definition of the ALMAAperture class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//# Copyright (C) 2011 by ESO (in the framework of the ALMA collaboration)
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
#ifndef SYNTHESIS_ALMAAPERTURE_H
#define SYNTHESIS_ALMAAPERTURE_H

#include <images/Images/PagedImage.h>
#include <synthesis/TransformMachines/AzElAperture.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <imageanalysis/ImageAnalysis/AntennaResponses.h>
#include <imageanalysis/ImageAnalysis/ImageConvolver.h>
#include <imageanalysis/ImageAnalysis/ImageFFT.h>
#include <images/Images/ImageRegrid.h>
#include <images/Images/ImageRegrid.h>

//
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// TEMPS The following #defines should REALLY GO!
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//
#define CONVSIZE (1024*2)
#define CONVWTSIZEFACTOR sqrt(2.0)
#define OVERSAMPLING 10
#define THRESHOLD 1E-3

namespace casa { //# NAMESPACE CASA - BEGIN
  template<class T> class ImageInterface;
  template<class T> class Matrix;
  class VisBuffer;

  enum ALMAAntennaType {
    ALMA_INVALID = 0,
    ALMA_DA = 1,
    ALMA_DV,
    ALMA_CM,
    ALMA_PM,
    ALMA_numAntTypes
  };

  //  class ALMAAperture : public ATerm
  class ALMAAperture : public AzElAperture
  {
  public:
    ALMAAperture();

    ~ALMAAperture();

    ALMAAperture& operator=(const ALMAAperture& other);
    //
    // Overload these functions.  They are pure virtual in the base class (ATerm).
    //
    virtual String name() {return String("ALMA Aperture");};

    virtual void applySky(ImageInterface<Float>& outputImage,
			  const VisBuffer& vb, 
			  const Bool doSquint=True,
			  const Int& cfKey=0){
      applySky(outputImage, vb, doSquint, cfKey, False);};
    virtual void applySky(ImageInterface<Complex>& outputImage,
			  const VisBuffer& vb, 
			  const Bool doSquint=True,
			  const Int& cfKey=0){
      applySky(outputImage, vb, doSquint, cfKey, False);};

    void applySky(ImageInterface<Float>& outputImage,
		  const VisBuffer& vb, 
		  const Bool doSquint,
		  const Int& cfKey,
		  const Bool raytrace); // if True, use raytracing even if canned responses exist
    void applySky(ImageInterface<Complex>& outputImage,
		  const VisBuffer& vb, 
		  const Bool doSquint,
		  const Int& cfKey,
		  const Bool raytrace); // if True, use raytracing even if canned responses exist

    virtual Vector<Int> vbRow2CFKeyMap(const VisBuffer& vb, Int& nUnique);

    virtual void setPolMap(const Vector<Int>& polMap) {polMap_p.resize(0);polMap_p=polMap;};
    virtual void getPolMap(Vector<Int>& polMap) {polMap.resize(0);polMap=polMap_p;};
    virtual Int getConvSize() {return CONVSIZE;};
    virtual Int getOversampling() {return OVERSAMPLING;}
    virtual Float getConvWeightSizeFactor() {return CONVWTSIZEFACTOR;};
    virtual Float getSupportThreshold() {return THRESHOLD;};

    // tell the antenna type number for each antenna in the antenna table
    Vector<ALMAAntennaType> antTypeMap(const VisBuffer& vb);

    // call this before reusing the same ALMAAperture object on a different MS
    void resetAntTypeMap(){antTypeMap_p.resize(0);};

    // derive type number from first two characters in antenna name, 
    // return -1 if not recognised 
    static ALMAAntennaType antTypeFromName(const String& name);
    static String antTypeStrFromType(const ALMAAntennaType& aType);

    static Int cFKeyFromAntennaTypes(const ALMAAntennaType aT1, const ALMAAntennaType aT2);
    static Vector<ALMAAntennaType> antennaTypesFromCFKey(const Int& cFKey);

    // generate the lists of antenna types for the whole vb
    Vector<ALMAAntennaType> antTypeList(const VisBuffer& vb);

    Int getVisParams(const VisBuffer& vb, const CoordinateSystem& skyCoord=CoordinateSystem());
    Int makePBPolnCoords(const VisBuffer&vb,
			 const Int& convSize,
			 const Int& convSampling,
			 const CoordinateSystem& skyCoord,
			 const Int& skyNx, const Int& skyNy,
			 CoordinateSystem& feedCoord);

    void destroyAntResp(){ delete aR_p; aR_p=0;};


  private:
    static AntennaResponses* aR_p; // shared between all instances of this class
    static Bool orderMattersInCFKey;
    Vector<Int> polMap_p;
    Bool haveCannedResponses_p; // true if there are precalculated response images available
    Vector<ALMAAntennaType> antTypeMap_p; // maps antenna id to antenna type
    Vector<PagedImage<Complex>* > respImage_p;
  };
};
#endif
