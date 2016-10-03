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
#include <msvis/MSVis/VisBuffer.h>

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
  // casacore::Stokes imaging, using this FTMachine, the VLA polarization squint
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
    WOnlyProjectFT(casacore::Int nFacets, casacore::Long cachesize, 
		   casacore::CountedPtr<CFCache>& cfcache,
		   casacore::CountedPtr<ConvolutionFunction>& cf,
		   casacore::CountedPtr<VisibilityResampler>& reSampler,
		   casacore::Int tilesize=16, 
		   casacore::Float pbLimit=5e-2,
		   casacore::Bool usezero=false);
    // </group>
    
    // Construct from a casacore::Record containing the WOnlyProjectFT state
    WOnlyProjectFT(const casacore::RecordInterface& stateRec);
    
    // Copy constructor
    WOnlyProjectFT(const WOnlyProjectFT &other);
    
    // Assignment operator
    WOnlyProjectFT &operator=(const WOnlyProjectFT &other);
    
    ~WOnlyProjectFT();
    
    virtual void initializeToVis(casacore::ImageInterface<casacore::Complex>& image,
			 const VisBuffer& vb);
    // This version returns the gridded vis...should be used in conjunction 
    // with the version of 'get' that needs the gridded visdata 
    virtual void initializeToVis(casacore::ImageInterface<casacore::Complex>& image,
			 const VisBuffer& vb, casacore::Array<casacore::Complex>& griddedVis,
			 casacore::Vector<casacore::Double>& uvscale);
    
    // Finalize transform to Visibility plane: flushes the image
    // cache and shows statistics if it is being used.
    virtual void finalizeToVis();
    
    // Initialize transform to Sky plane: initializes the image
    virtual void initializeToSky(casacore::ImageInterface<casacore::Complex>& image,  casacore::Matrix<casacore::Float>& weight,
			 const VisBuffer& vb);
    
    // Finalize transform to Sky plane: flushes the image
    // cache and shows statistics if it is being used. DOES NOT
    // DO THE FINAL TRANSFORM!
    virtual void finalizeToSky();
    
    virtual void initVisBuffer(VisBuffer& vb, Type whichVBColumn);
    void initVisBuffer(VisBuffer& vb, Type whichVBColumn, casacore::Int row);

    // Get actual coherence from grid by degridding
    void get(VisBuffer& vb, casacore::Int row=-1);
    
    // Get the coherence from grid return it in the degrid 
    // is used especially when scratch columns are not 
    // present in ms.
    void get(VisBuffer& vb, casacore::Cube<casacore::Complex>& degrid, 
	     casacore::Array<casacore::Complex>& griddedVis, casacore::Vector<casacore::Double>& scale, 
	     casacore::Int row=-1);
    
    
    
    // Put coherence to grid by gridding.
    void put(const VisBuffer&, casacore::TempImage<casacore::Complex>&, casacore::Vector<casacore::Double>&, int,
	     casacore::UVWMachine*, casacore::Bool) 
    {
      //    throw(casacore::AipsError("WOnlyProjectFT::put is not implemented"));
    }
    void put(const VisBuffer& vb, casacore::Int row=-1, casacore::Bool dopsf=false,
	     FTMachine::Type type=FTMachine::OBSERVED);
    
    // Make the entire image
    void makeImage(FTMachine::Type type,
		   VisSet& vs,
		   casacore::ImageInterface<casacore::Complex>& image,
		   casacore::Matrix<casacore::Float>& weight);
    
    // Get the final image: do the Fourier transform and
    // grid-correct, then optionally normalize by the summed weights
    virtual casacore::ImageInterface<casacore::Complex>& getImage(casacore::Matrix<casacore::Float>&, casacore::Bool normalize=true);

    // Save and restore the WOnlyProjectFT to and from a record
    casacore::Bool toRecord(casacore::RecordInterface& outRec,  casacore::Bool withImage=false);
    casacore::Bool fromRecord(const casacore::RecordInterface& inRec);
    
    // Can this FTMachine be represented by Fourier convolutions?
    casacore::Bool isFourier() {return true;}
    
    //
    // Make a sensitivity image (sensitivityImage), given the gridded
    // weights (wtImage).  These are related to each other by a
    // Fourier transform and normalization by the sum-of-weights
    // (sumWt) and normalization by the product of the 2D FFT size
    // along each axis.  If doFFTNorm=false, normalization by the FFT
    // size is not done.  If sumWt is not provided, normalization by
    // the sum of weights is also not done.
    //
    virtual void makeSensitivityImage(casacore::Lattice<casacore::Complex>& wtImage,
				      casacore::ImageInterface<casacore::Float>& sensitivityImage,
				      const casacore::Matrix<casacore::Float>& sumWt=casacore::Matrix<casacore::Float>(),
				      const casacore::Bool& doFFTNorm=true) {};
    virtual void makeSensitivityImage(const VisBuffer& vb, 
				      const casacore::ImageInterface<casacore::Complex>& imageTemplate,
				      casacore::ImageInterface<casacore::Float>& sensitivityImage);
    //
    // Given the sky image (Fourier transform of the visibilities),
    // sum of weights and the sensitivity image, this method replaces
    // the skyImage with the normalized image of the sky.
    //
    virtual void normalizeImage(casacore::Lattice<casacore::Complex>& skyImage,
				const casacore::Matrix<casacore::Double>& sumOfWts,
				casacore::Lattice<casacore::Float>& sensitivityImage,
				casacore::Bool fftNorm=true);
    virtual void normalizeImage(casacore::Lattice<casacore::Complex>& skyImage,
				const casacore::Matrix<casacore::Double>& sumOfWts,
				casacore::Lattice<casacore::Float>& sensitivityImage,
				casacore::Lattice<casacore::Complex>& sensitivitySqImage,
				casacore::Bool fftNorm=true);
    
    virtual casacore::ImageInterface<casacore::Float>& getSensitivityImage() {return *avgPB_p;}
    virtual casacore::Matrix<casacore::Double>& getSumOfWeights() {return sumWeight;};

    casacore::Vector<casacore::Int>& getPolMap() {return polMap;};
    virtual casacore::String name(){ return "WOnlyProjectFT";};
    virtual casacore::Bool verifyShapes(casacore::IPosition shape0, casacore::IPosition shape1);
    virtual void setMiscInfo(const casacore::Int qualifier){(void)qualifier;};

  protected:
    
    casacore::Int nint(casacore::Double val) {return casacore::Int(floor(val+0.5));};
    // Locate convolution functions on the disk
    //    casacore::Int locateConvFunction(const casacore::Int Nw, const casacore::Float pa);
    //    void cacheConvFunction(casacore::Int which, casacore::Array<casacore::Complex>& cf, casacore::CoordinateSystem& coord);
    // Find the convolution function
    void findConvFunction(const casacore::ImageInterface<casacore::Complex>& image,
			  const VisBuffer& vb);
    
    // Get the appropriate data pointer
    casacore::Array<casacore::Complex>* getDataPointer(const casacore::IPosition&, casacore::Bool);
    
    void ok();
    
    void init();
    //    virtual void initPolInfo(const VisBuffer& vb);
    // Is this record on Grid? check both ends. This assumes that the
    // ends bracket the middle
    casacore::Bool recordOnGrid(const VisBuffer& vb, casacore::Int rownr) const;
    
    // Padding in FFT
    casacore::Float padding_p;
    
    casacore::Int nWPlanes_p;
    // Image cache
    casacore::LatticeCache<casacore::Complex> * imageCache;
    
    // Sizes
    casacore::Long cachesize;
    casacore::Int tilesize;
    
    // Gridder
    casacore::ConvolveGridder<casacore::Double, casacore::Complex>* gridder;
    
    // Is this tiled?
    casacore::Bool isTiled;
    
    // casacore::Array lattice
    casacore::CountedPtr<casacore::Lattice<casacore::Complex> > arrayLattice;
    
    // Lattice. For non-tiled gridding, this will point to arrayLattice,
    //  whereas for tiled gridding, this points to the image
    casacore::CountedPtr<casacore::Lattice<casacore::Complex> > lattice;
    
    casacore::Float maxAbsData;
    
    // Useful IPositions
    casacore::IPosition centerLoc, offsetLoc;
    
    // Image Scaling and offset
    casacore::Vector<casacore::Double> uvScale, uvOffset;
    
    // casacore::Array for non-tiled gridding
    casacore::Array<casacore::Complex> griddedData;
    
    //    casacore::DirectionCoordinate directionCoord;
    casacore::MDirection::Convert* pointingToImage;
    
    // Grid/degrid zero spacing points?
    casacore::Bool usezero_p;
    
    //    casacore::CountedPtr<ConvolutionFunction> telescopeConvFunc_p;
    //    CFStore cfs_p, cfwts_p;
    casacore::Array<casacore::Complex> convFunc_p, convWeights_p;
    casacore::CountedPtr<VisibilityResampler> reSampler_p;
    //
    // casacore::Vector to hold the support size info. for the convolution
    // functions pointed to by the elements of convFunctions_p.  The
    // co-ordinates of this array are (W-term, Poln, PA).
    //
    casacore::Int convSize, convSampling, wConvSize, lastIndex_p;
    
    //
    // No. of vis. polarization planes used in making the user defined
    // casacore::Stokes images
    //
    casacore::Int maxConvSupport;

    casacore::Int Nant_p;
    casacore::Bool makingPSF;
    
    virtual void runFortranGet(casacore::Matrix<casacore::Double>& uvw,casacore::Vector<casacore::Double>& dphase,
			       casacore::Cube<casacore::Complex>& visdata,
			       casacore::IPosition& s,
			       //casacore::Cube<casacore::Complex>& gradVisAzData,
			       //casacore::Cube<casacore::Complex>& gradVisElData,
			       //casacore::IPosition& gradS,
			       casacore::Int& Conj,
			       casacore::Cube<casacore::Int>& flags,casacore::Vector<casacore::Int>& rowFlags,
			       casacore::Int& rownr,casacore::Vector<casacore::Double>& actualOffset,
			       casacore::Array<casacore::Complex>* dataPtr,
			       casacore::Int& aNx, casacore::Int& aNy, casacore::Int& npol, casacore::Int& nchan,
			       VisBuffer& vb,casacore::Int& Nant_p, casacore::Int& scanNo,
			       casacore::Double& sigma,
			       casacore::Array<casacore::Float>& raoffsets,
			       casacore::Array<casacore::Float>& decoffsets,
			       casacore::Double area,
			       casacore::Int& doGrad,casacore::Int paIndex);
    virtual void runFortranPut(casacore::Matrix<casacore::Double>& uvw,casacore::Vector<casacore::Double>& dphase,
			       const casacore::Complex& visdata_p,
			       casacore::IPosition& s,
			       //casacore::Cube<casacore::Complex>& gradVisAzData,
			       //casacore::Cube<casacore::Complex>& gradVisElData,
			       //casacore::IPosition& gradS,
		       casacore::Int& Conj,
		       casacore::Cube<casacore::Int>& flags,casacore::Vector<casacore::Int>& rowFlags,
		       const casacore::Matrix<casacore::Float>& weight,
		       casacore::Int& rownr,casacore::Vector<casacore::Double>& actualOffset,
		       casacore::Array<casacore::Complex>& dataPtr,
		       casacore::Int& aNx, casacore::Int& aNy, casacore::Int& npol, casacore::Int& nchan,
		       const VisBuffer& vb,casacore::Int& Nant_p, casacore::Int& scanNo,
		       casacore::Double& sigma,
		       casacore::Array<casacore::Float>& raoffsets,
		       casacore::Array<casacore::Float>& decoffsets,
		       casacore::Matrix<casacore::Double>& sumWeight,
		       casacore::Double& area,
		       casacore::Int& doGrad,
		       casacore::Int& doPSF,casacore::Int paIndex);
  void runFortranGetGrad(casacore::Matrix<casacore::Double>& uvw,casacore::Vector<casacore::Double>& dphase,
			 casacore::Cube<casacore::Complex>& visdata,
			 casacore::IPosition& s,
			 casacore::Cube<casacore::Complex>& gradVisAzData,
			 casacore::Cube<casacore::Complex>& gradVisElData,
			 //			 casacore::IPosition& gradS,
			 casacore::Int& Conj,
			 casacore::Cube<casacore::Int>& flags,casacore::Vector<casacore::Int>& rowFlags,
			 casacore::Int& rownr,casacore::Vector<casacore::Double>& actualOffset,
			 casacore::Array<casacore::Complex>* dataPtr,
			 casacore::Int& aNx, casacore::Int& aNy, casacore::Int& npol, casacore::Int& nchan,
			 VisBuffer& vb,casacore::Int& Nant_p, casacore::Int& scanNo,
			 casacore::Double& sigma,
			 casacore::Array<casacore::Float>& l_off,
			 casacore::Array<casacore::Float>& m_off,
			 casacore::Double area,
			 casacore::Int& doGrad,
			 casacore::Int paIndex);
  };
} //# NAMESPACE CASA - END
    // void makeAntiAliasingOp(casacore::Vector<casacore::Complex>& val, const casacore::Int len, const casacore::Double HPBW);
    // void makeAntiAliasingCorrection(casacore::Vector<casacore::Complex>& correction, 
    // 				    const casacore::Vector<casacore::Complex>& op, 
    // 				    const casacore::Int nx);
    // void applyAntiAliasingOp(casacore::ImageInterface<casacore::Complex>& cf, 
    // 			     casacore::Vector<casacore::IPosition>& offset,
    // 			     casacore::Double HPBW,
    // 			     casacore::Int op=0, 
    // 			     casacore::Bool Square=false);
    // void applyAntiAliasingOp(casacore::ImageInterface<casacore::Float>& cf, 
    // 			     casacore::Vector<casacore::IPosition>& offset,
    // 			     casacore::Double HPBW,
    // 			     casacore::Int op=0, 
    // 			     casacore::Bool Square=false);
    // void correctAntiAliasing(casacore::Lattice<casacore::Complex>& cf);

#endif
