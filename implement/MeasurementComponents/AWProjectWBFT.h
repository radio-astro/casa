//# AWProjectWBFT.h: Definition for AWProjectWBFT
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

#ifndef SYNTHESIS_AWPROJECTWBFT_H
#define SYNTHESIS_AWPROJECTWBFT_H

#include <synthesis/MeasurementComponents/AWProjectFT.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class EPJones;

  class AWProjectWBFT : public AWProjectFT {

  public:
    //
    // Constructor: cachesize is the size of the cache in words
    // (e.g. a few million is a good number), tilesize is the size of
    // the tile used in gridding (cannot be less than 12, 16 works in
    // most cases).  
    // <group>
    //
    AWProjectWBFT(Int nFacets, Long cachesize,
		  CountedPtr<CFCache>& cfcache,
		  CountedPtr<ConvolutionFunction>& cf,
		  Bool applyPointingOffset=True,
		  Bool doPBCorr=True,
		  Int tilesize=16, 
		  Float paSteps=5.0, 
		  Float pbLimit=5e-2,
		  Bool usezero=False);
    // </group>
    
    // Construct from a Record containing the AWProjectWBFT state
    AWProjectWBFT(const RecordInterface& stateRec);
    
    // Assignment operator
    AWProjectWBFT &operator=(const AWProjectWBFT &othher);
    
    ~AWProjectWBFT() {};

    virtual String name() {return "AWProjectWBFT";};


    Int findPointingOffsets(const VisBuffer& vb, Array<Float> &l_off, Array<Float> &m_off,
			    Bool Evaluate);
    void normalizeAvgPB();
    void normalizeAvgPB(ImageInterface<Complex>& inImage, ImageInterface<Float>& outImage) 
    {throw(AipsError("AWPWBFT::normalizeAvgPB(Complex,Float)"));}

    virtual void makeSensitivityImage(const VisBuffer& vb, 
				      const ImageInterface<Complex>& imageTemplate,
				      ImageInterface<Float>& sensitivityImage);
    virtual void makeSensitivityImage(Lattice<Complex>& wtImage,
				      ImageInterface<Float>& sensitivityImage,
				      const Matrix<Float>& sumWt=Matrix<Float>(),
				      const Bool& doFFTNorm=True);
    virtual void makeSensitivitySqImage(Lattice<Complex>& wtImage,
					ImageInterface<Complex>& sensitivitySqImage,
					const Matrix<Float>& sumWt=Matrix<Float>(),
					const Bool& doFFTNorm=True);

    virtual ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);

    virtual void finalizeToSky();
    virtual void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
				 const VisBuffer& vb);

    void setObservatoryLocation(const MPosition& mLocation) {mLocation_p=mLocation;};

    inline virtual Float pbFunc(Float& a) {return (a);};

    virtual Bool verifyShapes(IPosition shape0, IPosition shape1)
    {(void)shape0; (void)shape1;return False;};

  protected:
    void ftWeightImage(Lattice<Complex>& wtImage, 
		       const Matrix<Float>& sumWt,
		       const Bool& doFFTNorm);

    Bool avgPBReady_p,resetPBs_p, wtImageFTDone;

  private:
    Vector<Int> fieldIds_p;
    TempImage<Complex> griddedWeights;
    Float pbNorm;
    //
    // These ugly methods (ugly due to their flirtation with FORTRAN) should go!
    //
    virtual void runFortranGet(Matrix<Double>& uvw,Vector<Double>& dphase,
			       Cube<Complex>& visdata,
			       IPosition& s,
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
