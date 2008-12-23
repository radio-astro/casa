//# nPBWProjectFT.h: Definition for nPBWProjectFT
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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

#ifndef SYNTHESIS_PBMOSAICFT_H
#define SYNTHESIS_PBMOSAICFT_H

// #include <synthesis/MeasurementComponents/FTMachine.h>
// #include <casa/Arrays/Matrix.h>
// #include <scimath/Mathematics/FFTServer.h>
// #include <msvis/MSVis/VisBuffer.h>
// #include <images/Images/ImageInterface.h>
// #include <images/Images/ImageInterface.h>
// #include <casa/Containers/Block.h>
// #include <casa/Arrays/Array.h>
// #include <casa/Arrays/Vector.h>
// #include <casa/Arrays/Matrix.h>
// #include <scimath/Mathematics/ConvolveGridder.h>
#include <synthesis/MeasurementComponents/nPBWProjectFT.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class EPJones;
  class PBMosaicFT : public nPBWProjectFT {
  public:
    
    // Constructor: cachesize is the size of the cache in words
    // (e.g. a few million is a good number), tilesize is the
    // size of the tile used in gridding (cannot be less than
    // 12, 16 works in most cases). 
    // <group>
    PBMosaicFT(MeasurementSet& ms, 
	       Int nFacets, Long cachesize, String& cfCacheDirName,
	       Bool applyPointingOffset=True,
	       Bool doPBCorr=True,
	       Int tilesize=16, 
	       Float paSteps=5.0, Float pbLimit=5e-2,
	       Bool usezero=False);
    // </group>
    
    // Construct from a Record containing the PBMosaicFT state
    PBMosaicFT(const RecordInterface& stateRec);
    
    // Copy constructor
    //    PBMosaicFT(const PBMosaicFT &other);
    
    // Assignment operator
    PBMosaicFT &operator=(const PBMosaicFT &othher);
    
    ~PBMosaicFT() {};

    void makeAveragePB0(const VisBuffer& vb, 
 		       const ImageInterface<Complex>& image,
 		       Int& polInUse,
 		       TempImage<Float>& avgPB);
    Int findPointingOffsets(const VisBuffer& vb, 
			    Array<Float> &l_off,
			    Array<Float> &m_off,
			    Bool Evaluate);
    void normalizeAvgPB();
    void normalizePB(ImageInterface<Float> &pb,const Float& nSamp=1);
    virtual ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);
    inline virtual Float pbFunc(Float& a) {return (a);};
    void setObservatoryLocation(const MPosition& mLocation) {mLocation_p=mLocation;};
    virtual void finalizeToSky();
    virtual void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
				 const VisBuffer& vb);

  private:
    Complex nApertures;
    Vector<Int> fieldIds_p;
    Bool avgPBReady;
    TempImage<Complex> cavgPB;
    Float pbNorm;
    virtual void runFortranGet(Matrix<Double>& uvw,Vector<Double>& dphase,
		       Cube<Complex>& visdata,
		       IPosition& s,
		       //Cube<Complex>& gradVisAzData,
		       //Cube<Complex>& gradVisElData,
		       //IPosition& gradS,
		       Int& Conj,
		       Cube<Int>& flags,Vector<Int>& rowFlags,
		       Int& rownr,Vector<Double>& actualOffset,
		       Array<Complex>* dataPtr,
		       Int& aNx, Int& aNy, Int& npol, Int& nchan,
		       VisBuffer& vb,Int& Nant_p, Int& scanNo,
		       Double& sigma,
		       Array<Float>& raoffsets,
		       Array<Float>& decoffsets,
		       Double area,
		       Int& doGrad,Int paIndex);
    virtual void runFortranPut(Matrix<Double>& uvw,Vector<Double>& dphase,
		       const Complex& visdata_p,
		       IPosition& s,
		       //Cube<Complex>& gradVisAzData,
		       //Cube<Complex>& gradVisElData,
		       //IPosition& gradS,
		       Int& Conj,
		       Cube<Int>& flags,Vector<Int>& rowFlags,
		       const Matrix<Float>& weight,
		       Int& rownr,Vector<Double>& actualOffset,
		       Array<Complex>& dataPtr,
		       Int& aNx, Int& aNy, Int& npol, Int& nchan,
		       const VisBuffer& vb,Int& Nant_p, Int& scanNo,
		       Double& sigma,
		       Array<Float>& raoffsets,
		       Array<Float>& decoffsets,
		       Matrix<Double>& sumWeight,
		       Double& area,
		       Int& doGrad,
		       Int& doPSF,Int paIndex);
    virtual void runFortranGetGrad(Matrix<Double>& uvw,Vector<Double>& dphase,
			 Cube<Complex>& visdata,
			 IPosition& s,
			 Cube<Complex>& gradVisAzData,
			 Cube<Complex>& gradVisElData,
			 //			 IPosition& gradS,
			 Int& Conj,
			 Cube<Int>& flags,Vector<Int>& rowFlags,
			 Int& rownr,Vector<Double>& actualOffset,
			 Array<Complex>* dataPtr,
			 Int& aNx, Int& aNy, Int& npol, Int& nchan,
			 VisBuffer& vb,Int& Nant_p, Int& scanNo,
			 Double& sigma,
			 Array<Float>& l_off,
			 Array<Float>& m_off,
			 Double area,
			 Int& doGrad,
			 Int paIndex);

  };

} //# NAMESPACE CASA - END

#endif
