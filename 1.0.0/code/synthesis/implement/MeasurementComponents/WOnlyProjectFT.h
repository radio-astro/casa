//# WOnlyProjectFT.h: Definition for WOnlyProjectFT
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

#ifndef SYNTHESIS_WONLYPROJECTFT_H
#define SYNTHESIS_WONLYPROJECTFT_H

#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/VLAIlluminationConvFunc.h>
//#include <synthesis/TransformMachines/ConvolutionFunction.h>
#include <synthesis/MeasurementComponents/EVLAConvFunc.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/TransformMachines/VPSkyJones.h>
#include <synthesis/TransformMachines/FTMachine.h>
//#include <synthesis/TransformMachines/CFCache.h>
#include <synthesis/TransformMachines/Utils.h>

#include <scimath/Mathematics/FFTServer.h>
#include <synthesis/MSVis/VisBuffer.h>

#include <casa/Containers/Block.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>

#include <scimath/Mathematics/ConvolveGridder.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/Measure.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <images/Images/ImageInterface.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  // <summary>  An FTMachine for Gridded Fourier transforms including effects of primary beam and pointing offsets and the w-term</summary>
  
  // <use visibility=export>
  
  // <reviewed reviewer="" date="" tests="" demos="">
  
  // <prerequisite>
  //   <li> <linkto class=FTMachine>FTMachine</linkto> module
  //   <li> <linkto class=SkyEquation>SkyEquation</linkto> module
  //   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
  //   <li> <linto class=EPJones>EPJones</linkto> module
  // </prerequisite>
  //
  // <etymology> 
  // FTMachine is a Machine for Fourier Transforms. Like
  // WProjectFT, WOnlyProjectFT does Grid-based Fourier transforms but
  // also includes the effects of primary beam and antenna pointing
  // offsets. 
  // </etymology>
  //
  // <synopsis> 
  //
  // The <linkto class=SkyEquation>SkyEquation</linkto> needs to be
  // able to perform Fourier transforms on visibility
  // data. WOnlyProjectFT allows efficient handling of direction
  // dependent effects due to the primary beam and antenna pointing
  // offsets using a <linkto class=VisBuffer>VisBuffer</linkto> which
  // encapsulates a chunk of visibility (typically all baselines for
  // one time) together with all the information needed for processing
  // (e.g. UVW coordinates).
  //
  // Using this FTMachine, errors due antenna pointing offsets can be
  // corrected during deconvolution.  One form of antenna pointing
  // error which is known a-priori is the VLA polarization squint
  // (about 6% of the Primary beam width at any frequency).  For
  // Stokes imaging, using this FTMachine, the VLA polarization squint
  // and beam polarization can also be corrected.  Also since the
  // effects of antenna pointing errors is strongest in the range of
  // 1-2GHz band (where the sky is not quite empty while the beams are
  // not too large either), this FTMachine can also be setup to
  // correct for the w-term.
  //
  // Switches are provided in the get() method to compute the
  // derivatives with respect to the parameters of the primary beam
  // (only pointing offsets for now).  This is used in the pointing
  // offset solver.
  //
  // See the documentation of other FTMachines for details about the
  // design of the FTMachines in general.
  //
  // </synopsis> 
  //
  // <example>
  // See the example for <linkto class=SkyModel>SkyModel</linkto>.
  // </example>
  //
  // <motivation>
  //
  // Encapsulate the correction of direction dependent effects via
  // visibility plane convolutions with a potentially different
  // convolution function for each baseline.
  //
  // </motivation>
  //
  // <todo asof="2005/07/21">
  //
  // <ul> Include the antenna based complex gain term as well since
  // that can interfere with the effects of pointing offsets.  
  //
  // <ul> Factor out the actual convolution functions as a separate
  // class making FTMachines for various direction dependent effects
  // generic.
  //
  // </todo>
  
  //  class EPJones;
  class SolvableVisJones;
  class WOnlyProjectFT : public FTMachine {
  public:
    
    // Constructor: cachesize is the size of the cache in words
    // (e.g. a few million is a good number), tilesize is the
    // size of the tile used in gridding (cannot be less than
    // 12, 16 works in most cases). 
    // <group>
    WOnlyProjectFT(Int nFacets, Long cachesize, 
		   CountedPtr<CFCache>& cfcache,
		   CountedPtr<ConvolutionFunction>& cf,
		   CountedPtr<VisibilityResampler>& reSampler,
		   Int tilesize=16, 
		   Float pbLimit=5e-2,
		   Bool usezero=False);
    // </group>
    
    // Construct from a Record containing the WOnlyProjectFT state
    WOnlyProjectFT(const RecordInterface& stateRec);
    
    // Copy constructor
    WOnlyProjectFT(const WOnlyProjectFT &other);
    
    // Assignment operator
    WOnlyProjectFT &operator=(const WOnlyProjectFT &other);
    
    ~WOnlyProjectFT();
    
    virtual void initializeToVis(ImageInterface<Complex>& image,
			 const VisBuffer& vb);
    // This version returns the gridded vis...should be used in conjunction 
    // with the version of 'get' that needs the gridded visdata 
    virtual void initializeToVis(ImageInterface<Complex>& image,
			 const VisBuffer& vb, Array<Complex>& griddedVis,
			 Vector<Double>& uvscale);
    
    // Finalize transform to Visibility plane: flushes the image
    // cache and shows statistics if it is being used.
    virtual void finalizeToVis();
    
    // Initialize transform to Sky plane: initializes the image
    virtual void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
			 const VisBuffer& vb);
    
    // Finalize transform to Sky plane: flushes the image
    // cache and shows statistics if it is being used. DOES NOT
    // DO THE FINAL TRANSFORM!
    virtual void finalizeToSky();
    
    virtual void initVisBuffer(VisBuffer& vb, Type whichVBColumn);
    void initVisBuffer(VisBuffer& vb, Type whichVBColumn, Int row);

    // Get actual coherence from grid by degridding
    void get(VisBuffer& vb, Int row=-1);
    
    // Get the coherence from grid return it in the degrid 
    // is used especially when scratch columns are not 
    // present in ms.
    void get(VisBuffer& vb, Cube<Complex>& degrid, 
	     Array<Complex>& griddedVis, Vector<Double>& scale, 
	     Int row=-1);
    
    
    
    // Put coherence to grid by gridding.
    void put(const VisBuffer&, TempImage<Complex>&, Vector<Double>&, int,
	     UVWMachine*, Bool) 
    {
      //    throw(AipsError("WOnlyProjectFT::put is not implemented"));
    }
    void put(const VisBuffer& vb, Int row=-1, Bool dopsf=False,
	     FTMachine::Type type=FTMachine::OBSERVED);
    
    // Make the entire image
    void makeImage(FTMachine::Type type,
		   VisSet& vs,
		   ImageInterface<Complex>& image,
		   Matrix<Float>& weight);
    
    // Get the final image: do the Fourier transform and
    // grid-correct, then optionally normalize by the summed weights
    virtual ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);

    // Save and restore the WOnlyProjectFT to and from a record
    Bool toRecord(RecordInterface& outRec,  Bool withImage=False);
    Bool fromRecord(const RecordInterface& inRec);
    
    // Can this FTMachine be represented by Fourier convolutions?
    Bool isFourier() {return True;}
    
    //
    // Make a sensitivity image (sensitivityImage), given the gridded
    // weights (wtImage).  These are related to each other by a
    // Fourier transform and normalization by the sum-of-weights
    // (sumWt) and normalization by the product of the 2D FFT size
    // along each axis.  If doFFTNorm=False, normalization by the FFT
    // size is not done.  If sumWt is not provided, normalization by
    // the sum of weights is also not done.
    //
    virtual void makeSensitivityImage(Lattice<Complex>& wtImage,
				      ImageInterface<Float>& sensitivityImage,
				      const Matrix<Float>& sumWt=Matrix<Float>(),
				      const Bool& doFFTNorm=True) {};
    virtual void makeSensitivityImage(const VisBuffer& vb, 
				      const ImageInterface<Complex>& imageTemplate,
				      ImageInterface<Float>& sensitivityImage);
    //
    // Given the sky image (Fourier transform of the visibilities),
    // sum of weights and the sensitivity image, this method replaces
    // the skyImage with the normalized image of the sky.
    //
    virtual void normalizeImage(Lattice<Complex>& skyImage,
				const Matrix<Double>& sumOfWts,
				Lattice<Float>& sensitivityImage,
				Bool fftNorm=True);
    virtual void normalizeImage(Lattice<Complex>& skyImage,
				const Matrix<Double>& sumOfWts,
				Lattice<Float>& sensitivityImage,
				Lattice<Complex>& sensitivitySqImage,
				Bool fftNorm=True);
    
    virtual ImageInterface<Float>& getSensitivityImage() {return *avgPB_p;}
    virtual Matrix<Double>& getSumOfWeights() {return sumWeight;};

    Vector<Int>& getPolMap() {return polMap;};
    virtual String name(){ return "WOnlyProjectFT";};
    virtual Bool verifyShapes(IPosition shape0, IPosition shape1);
    virtual void setMiscInfo(const Int qualifier){(void)qualifier;};

  protected:
    
    Int nint(Double val) {return Int(floor(val+0.5));};
    // Locate convolution functions on the disk
    //    Int locateConvFunction(const Int Nw, const Float pa);
    //    void cacheConvFunction(Int which, Array<Complex>& cf, CoordinateSystem& coord);
    // Find the convolution function
    void findConvFunction(const ImageInterface<Complex>& image,
			  const VisBuffer& vb);
    
    // Get the appropriate data pointer
    Array<Complex>* getDataPointer(const IPosition&, Bool);
    
    void ok();
    
    void init();
    //    virtual void initPolInfo(const VisBuffer& vb);
    // Is this record on Grid? check both ends. This assumes that the
    // ends bracket the middle
    Bool recordOnGrid(const VisBuffer& vb, Int rownr) const;
    
    // Padding in FFT
    Float padding_p;
    
    Int nWPlanes_p;
    // Image cache
    LatticeCache<Complex> * imageCache;
    
    // Sizes
    Long cachesize;
    Int tilesize;
    
    // Gridder
    ConvolveGridder<Double, Complex>* gridder;
    
    // Is this tiled?
    Bool isTiled;
    
    // Array lattice
    CountedPtr<Lattice<Complex> > arrayLattice;
    
    // Lattice. For non-tiled gridding, this will point to arrayLattice,
    //  whereas for tiled gridding, this points to the image
    CountedPtr<Lattice<Complex> > lattice;
    
    Float maxAbsData;
    
    // Useful IPositions
    IPosition centerLoc, offsetLoc;
    
    // Image Scaling and offset
    Vector<Double> uvScale, uvOffset;
    
    // Array for non-tiled gridding
    Array<Complex> griddedData;
    
    //    DirectionCoordinate directionCoord;
    MDirection::Convert* pointingToImage;
    
    // Grid/degrid zero spacing points?
    Bool usezero_p;
    
    //    CountedPtr<ConvolutionFunction> telescopeConvFunc_p;
    //    CFStore cfs_p, cfwts_p;
    Array<Complex> convFunc_p, convWeights_p;
    CountedPtr<VisibilityResampler> reSampler_p;
    //
    // Vector to hold the support size info. for the convolution
    // functions pointed to by the elements of convFunctions_p.  The
    // co-ordinates of this array are (W-term, Poln, PA).
    //
    Int convSize, convSampling, wConvSize, lastIndex_p;
    
    //
    // No. of vis. polarization planes used in making the user defined
    // Stokes images
    //
    Int maxConvSupport;

    Int Nant_p;
    Bool makingPSF;
    
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
  void runFortranGetGrad(Matrix<Double>& uvw,Vector<Double>& dphase,
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
    // void makeAntiAliasingOp(Vector<Complex>& val, const Int len, const Double HPBW);
    // void makeAntiAliasingCorrection(Vector<Complex>& correction, 
    // 				    const Vector<Complex>& op, 
    // 				    const Int nx);
    // void applyAntiAliasingOp(ImageInterface<Complex>& cf, 
    // 			     Vector<IPosition>& offset,
    // 			     Double HPBW,
    // 			     Int op=0, 
    // 			     Bool Square=False);
    // void applyAntiAliasingOp(ImageInterface<Float>& cf, 
    // 			     Vector<IPosition>& offset,
    // 			     Double HPBW,
    // 			     Int op=0, 
    // 			     Bool Square=False);
    // void correctAntiAliasing(Lattice<Complex>& cf);

#endif
