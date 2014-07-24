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
#define DELTAPA 1.0
#define MAGICPAVALUE -999.0


#include <synthesis/TransformMachines/AWProjectFT.h>

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
		  CountedPtr<VisibilityResamplerBase>& visResampler,
		  Bool applyPointingOffset=True,
		  Bool doPBCorr=True,
		  Int tilesize=16, 
		  Float paSteps=5.0, 
		  Float pbLimit=5e-4,
		  Bool usezero=False,
		  Bool conjBeams_p=True,
		  Bool doublePrecGrid=False);
    // </group>
    
    // Construct from a Record containing the AWProjectWBFT state
    AWProjectWBFT(const RecordInterface& stateRec);
    
    // Copy constructor
    AWProjectWBFT(const AWProjectWBFT &other):AWProjectFT() {operator=(other);};

    // Assignment operator
    AWProjectWBFT &operator=(const AWProjectWBFT &other);
    
    ~AWProjectWBFT() {};

    virtual String name() const {return "AWProjectWBFT";};


    Int findPointingOffsets(const VisBuffer& vb, Array<Float> &l_off, Array<Float> &m_off,
			    Bool Evaluate);
    void normalizeAvgPB();
    void normalizeAvgPB(ImageInterface<Complex>& /*inImage*/, ImageInterface<Float>& /*outImage*/) 
    {throw(AipsError("AWPWBFT::normalizeAvgPB(Complex,Float)"));}

    //
    // This method is called from AWProjectFT to compute the
    // sensitivity image by accumulating in the image domain
    // (i.e. directly accumulate the Primay Beam functions).  This is
    // called from findConvFunction() so that sensitivity pattern is
    // also pre-computed along with the convolution functions.  This
    // in-turn calls the ATerm::makeAverageResponse().
    //
    // For AWProjectWBFT class of FTMachines, this just issues a log
    // message indicating that this is only setting up things for
    // accumulation of weight images in the first gridding cycle.  The
    // actual sensitivity patterns are computed by overloaded function
    // below.
    //
    virtual void makeSensitivityImage(const VisBuffer& vb, 
				      const ImageInterface<Complex>& imageTemplate,
				      ImageInterface<Float>& sensitivityImage);
    //
    // In AWProjectWBFT and its derivatives, sensitivity image is
    // computed by accumulating weight functions (images) during the
    // first gridding cycle.  AWProjectFT::makeSensitivityImage() is
    // overloaded in AWProjectWBFT and only issues a log message.
    //
    // The following method is used to Fourier transform normalize the
    // accumulated weight images.  doFFTNorm when True, the FFT
    // normalization (by pixel volume) is also done.
    //
    virtual void makeSensitivityImage(Lattice<Complex>& wtImage,
				      ImageInterface<Float>& sensitivityImage,
				      const Matrix<Float>& sumWt=Matrix<Float>(),
				      const Bool& doFFTNorm=True);
    virtual void makeSensitivitySqImage(Lattice<Complex>& wtImage,
					ImageInterface<Complex>& sensitivitySqImage,
					const Matrix<Float>& sumWt=Matrix<Float>(),
					const Bool& doFFTNorm=True);

    //
    // Method used to make normalized image from gridded visibilites.
    // This calls makeSensitivityImage() to make the sensitivity image
    // and AWProjectFT::getImage() to make the image from gridded
    // visibilites.  AWProjectFT::getImage() internally calls
    // normalizeImage() which uses the sensitivty image computed by
    // makeSensitivtyImage().
    //
    virtual ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);
    //
    // Method used to convert the pixel value of the PB image, passed
    // as pbPixValue, to a value used for PB-normalization.
    // Typically, this will depend on the units of the "PB image"
    // constructed by the makeSensitivtyImage() methods. pbLimit is
    // the fractional pb-gain below which imaging is not required
    // (this value is typically the user-defined parameter in the
    // private member variable pbLimit_p).
    //
    inline virtual Float pbFunc(const Float& /*pbPixValue*/, const Float& /*pbLimit*/) 
    {return  1.0;};
    //   {Float tt=(pbPixValue);return  (abs(tt) >= pbLimit)?tt:1.0;};
 

   //    {Float tt=sqrt(pbPixValue);return  (abs(tt) >= pbLimit)?tt:1.0;};

    virtual void finalizeToSky();
    virtual void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
				 const VisBuffer& vb);
    void setObservatoryLocation(const MPosition& mLocation) {mLocation_p=mLocation;};

    virtual Bool verifyShapes(IPosition shape0, IPosition shape1)
    {(void)shape0; (void)shape1;return False;};

    //
    // Returns True if accumulation during gridding to compute the
    // average PB must be done.
    //
    virtual Bool computeAvgPB(const Double& /*actualPA*/, const Double& /*lastPAUsedForWtImg*/) 
    {return (avgPBReady_p==False);};

    // virtual void setMiscInfo(const Int qualifier)
    // {sensitivityPatternQualifier_p=qualifier;}
    //    {qualifier_p = qualifier;taylorQualifier_p = "_MFS_"+String::toString(qualifier_p)+"_";};

    //  virtual void ComputeResiduals(VisBuffer&vb, Bool useCorrected) {};
  protected:
    virtual void ftWeightImage(Lattice<Complex>& wtImage, 
			       const Matrix<Float>& sumWt,
			       const Bool& doFFTNorm);

    virtual void resampleDataToGrid(Array<Complex>& griddedData,VBStore& vbs, 
				    const VisBuffer& vb, Bool& dopsf);
    virtual void resampleDataToGrid(Array<DComplex>& griddedData,VBStore& vbs, 
				    const VisBuffer& vb, Bool& dopsf);
    //    virtual void resampleGridToData(VBStore& vbs, const VisBuffer& vb);
    void resampleCFToGrid(Array<Complex>& wtsGrid, 
			  VBStore& vbs, const VisBuffer& vb);

    Bool avgPBReady_p,resetPBs_p, wtImageFTDone_p;

  private:
    String tt_pp;
    Vector<Int> fieldIds_p;
    TempImage<Complex> griddedWeights, griddedConjWeights;
    CFStore rotatedCFWts_p;
    Float pbNorm;
    CountedPtr<VisibilityResamplerBase> visResamplerWt_p;
    // //
    // // These ugly methods (ugly due to their flirtation with FORTRAN) should go!
    // //
    // virtual void runFortranGet(Matrix<Double>& uvw,Vector<Double>& dphase,
    // 			       Cube<Complex>& visdata,
    // 			       IPosition& s,
    // 			       Int& Conj,
    // 			       Cube<Int>& flags,Vector<Int>& rowFlags,
    // 			       Int& rownr,Vector<Double>& actualOffset,
    // 			       Array<Complex>* dataPtr,
    // 			       Int& aNx, Int& aNy, Int& npol, Int& nchan,
    // 			       VisBuffer& vb,Int& Nant_p, Int& scanNo,
    // 			       Double& sigma,
    // 			       Array<Float>& raoffsets,
    // 			       Array<Float>& decoffsets,
    // 			       Double area,
    // 			       Int& doGrad,Int paIndex);
    // virtual void runFortranPut(Matrix<Double>& uvw,Vector<Double>& dphase,
    // 			       const Complex& visdata_p,
    // 			       IPosition& s,
    // 			       Int& Conj,
    // 			       Cube<Int>& flags,Vector<Int>& rowFlags,
    // 			       const Matrix<Float>& weight,
    // 			       Int& rownr,Vector<Double>& actualOffset,
    // 			       Array<Complex>& dataPtr,
    // 			       Int& aNx, Int& aNy, Int& npol, Int& nchan,
    // 			       const VisBuffer& vb,Int& Nant_p, Int& scanNo,
    // 			       Double& sigma,
    // 			       Array<Float>& raoffsets,
    // 			       Array<Float>& decoffsets,
    // 			       Matrix<Double>& sumWeight,
    // 			       Double& area,
    // 			       Int& doGrad,
    // 			       Int& doPSF,Int paIndex);
    // virtual void runFortranGetGrad(Matrix<Double>& uvw,Vector<Double>& dphase,
    // 				   Cube<Complex>& visdata,
    // 				   IPosition& s,
    // 				   Cube<Complex>& gradVisAzData,
    // 				   Cube<Complex>& gradVisElData,
    // 				   Int& Conj,
    // 				   Cube<Int>& flags,Vector<Int>& rowFlags,
    // 				   Int& rownr,Vector<Double>& actualOffset,
    // 				   Array<Complex>* dataPtr,
    // 				   Int& aNx, Int& aNy, Int& npol, Int& nchan,
    // 				   VisBuffer& vb,Int& Nant_p, Int& scanNo,
    // 				   Double& sigma,
    // 				   Array<Float>& l_off,
    // 				   Array<Float>& m_off,
    // 				   Double area,
    // 				   Int& doGrad,
    // 				   Int paIndex);
    Bool oneTimeMessage_p;
  };
} //# NAMESPACE CASA - END

#endif
