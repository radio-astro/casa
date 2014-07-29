//# FTMachine.h: Definition for FTMachine
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#ifndef SYNTHESIS_TRANSFORM2_FTMACHINE_H
#define SYNTHESIS_TRANSFORM2_FTMACHINE_H

#include <measures/Measures/Measure.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Containers/Block.h>
#include <images/Images/TempImage.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <synthesis/TransformMachines/CFCache.h>
#include <synthesis/TransformMachines/CFStore2.h>

#include <synthesis/TransformMachines/ConvolutionFunction.h>
#include <synthesis/TransformMachines/PolOuterProduct.h>

#include <images/Images/ImageInterface.h>
#include <images/Images/SubImage.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>

#include <synthesis/ImagerObjects/SIImageStore.h>
#include <synthesis/ImagerObjects/SIImageStoreMultiTerm.h>

namespace casa{ //# namespace casa

  class UVWMachine;
  class VisModelData;
  namespace vi{ class VisBuffer2;
                  class VisibilityIterator2;
  }
 namespace refim{ //#	 namespace for refactored imaging code with vi2/vb2

  class SkyJones;
// <summary> defines interface for the Fourier Transform Machine </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=SkyModel>SkyModel</linkto> module
//   <li> <linkto class=SkyEquation>SkyEquation</linkto> module
//   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
// </prerequisite>
//
// <etymology>
// FTMachine is a Machine for Fourier Transforms
// </etymology>
//
// <synopsis> 
// The <linkto class=SkyEquation>SkyEquation</linkto> needs to be able
// to perform Fourier transforms on visibility data. FTMachine
// allows efficient Fourier Transform processing using a 
// <linkto class=VisBuffer>VisBuffer</linkto> which encapsulates
// a chunk of visibility (typically all baselines for one time)
// together with all the information needed for processing
// (e.g. UVW coordinates).
// </synopsis> 
//
// <example>
// A simple example of a FTMachine is found in 
// <linkto class=GridFT>GridFT</linkto>.
// See the example for <linkto class=SkyModel>SkyModel</linkto>.
// </example>
//
// <motivation>
// Define an interface to allow efficient processing of chunks of 
// visibility data
//
// Note that the image must be Complex. It must contain the
// Complex Stokes values (e.g. RR,RL,LR,LL). FTMachine
// uses the image coordinate system to determine mappings
// between the polarization and frequency values in the
// PagedImage and in the VisBuffer.
//
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class FTMachine {
public:

  //# Enumerations
  // Types of known Images that may be made using the makeImage method 
  enum Type {
    OBSERVED=0,		// From OBSERVED visibility data (default)
    MODEL,		// From MODEL visibility data
    CORRECTED,		// From CORRECTED visibility data
    RESIDUAL,		// From RESIDUAL (OBSERVED-MODEL) visibility data
    PSF,		// POINT SPREAD FUNCTION
    COVERAGE,		// COVERAGE (SD only)
    N_types,		// Number of types
    DEFAULT=OBSERVED
  };

  FTMachine();


  FTMachine(CountedPtr<CFCache>& cfcache,CountedPtr<ConvolutionFunction>& cfctor);

  FTMachine(const FTMachine& other);

  FTMachine& operator=(const FTMachine& other);

  void setBasePrivates(const FTMachine& other){FTMachine::operator=(other);}

  virtual ~FTMachine();
  

  //clone copy
  //should make it pure virtual forcing every ftm to have a cloner
  virtual FTMachine* cloneFTM(){return NULL;};
  // Initialize transform to Visibility plane
  virtual void initializeToVis(ImageInterface<Complex>& image, const vi::VisBuffer2& vb) = 0;

  virtual void initializeToVisNew(const vi::VisBuffer2& vb,
					     CountedPtr<SIImageStore> imstore);

  //-------------------------------------------------------------------------------------
  // Finalize transform to Visibility plane
  // This is mostly a no-op, and is not-even called from CubeSkyEquation.
  virtual void finalizeToVis() = 0;

  // Note : No vectorized form of finalizeToVis yet.....

  //-------------------------------------------------------------------------------------
  // Initialize transform to Sky plane
  
  virtual void initializeToSky(ImageInterface<Complex>& image,
  			       Matrix<Float>& weight, const vi::VisBuffer2& vb) = 0;
  

  virtual void initializeToSkyNew(const Bool dopsf, 
				  const vi::VisBuffer2& vb, 
				  CountedPtr<SIImageStore> imstore);

  //-------------------------------------------------------------------------------------
  // Finalize transform to Sky plane
  virtual void finalizeToSky() = 0;

  virtual void finalizeToSky(ImageInterface<Complex>& iimage){(void)iimage;};

 
  virtual void finalizeToSkyNew(Bool dopsf, 
				const vi::VisBuffer2& vb,
					   CountedPtr<SIImageStore> imstore  );

  //-------------------------------------------------------------------------------------

  // Get actual coherence from grid
  virtual void get(vi::VisBuffer2& vb, Int row=-1) = 0;

  // Put coherence to grid
  virtual void put(const vi::VisBuffer2& vb, Int row=-1, Bool dopsf=False,
  		   FTMachine::Type type= FTMachine::OBSERVED)=0;
  

  //-------------------------------------------------------------------------------------
  virtual void correlationToStokes(ImageInterface<Complex>& compImage, 
				   ImageInterface<Float>& resImage, 
				   const Bool dopsf);
 
  virtual void stokesToCorrelation(ImageInterface<Float>& modelImage,
				   ImageInterface<Complex>& compImage);

  /*
  virtual void normalizeSumWeight(ImageInterface<Float>& inOutImage, 
			       ImageInterface<Float>& weightImage, 
			       const Bool dopsf);
  */

  virtual void normalizeImage(Lattice<Complex>&,//skyImage,
			      const Matrix<Double>&,// sumOfWts,
			      Lattice<Float>&,// sensitivityImage,
			      Bool /*fftNorm*/){return;};

  virtual void normalizeImage(ImageInterface<Float>& skyImage,
			      Matrix<Float>& sumOfWts,
			      ImageInterface<Float>& sensitivityImage,
			      Bool dopsf, Float pblimit, Int normtype);


  // All FTMachines that fill weightimage, need to set this.
  // TODO : Make this pure virtual.
  virtual Bool useWeightImage(){return False;}; 
  //virtual Bool isSkyJonesSet(){return (sj_p.nelements()>0) && !(sj_p[0]).null()  ;}
  //virtual Bool isSkyJonesChanged(vi::VisBuffer2& vb, Int row){if(sj_p.nelements()>0){return sj_p[0]->changed(vb,row);} else {return False;} };

  // Set SkyJones if image domain corrections /applycation are needed
  // To reset the the FTMachine for stopping image based correction/applycation
  // set in a Vector of size 0.
  // The pointers have to be handled by the caller ..no delete happening here
  virtual void setSkyJones(Vector<CountedPtr<SkyJones> >& sj);
  
  Bool changedSkyJonesLogic(const vi::VisBuffer2& vb, Bool& firstRow, Bool& internalRow);


  //-------------------------------------------------------------------------------------

  // Get the final image
  virtual ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True) = 0;

  virtual void findConvFunction(const ImageInterface<Complex>&,// image,
				const vi::VisBuffer2& /*vb*/) {};
  // Get the final weights image
  virtual void getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights) = 0;

  // Get a flux (divide by this to get a flux density correct image) 
  // image if there is one
  virtual void getFluxImage(ImageInterface<Float>& image){(void)image;};

  // Make the entire image
  // Make the entire image using a ROVisIter
  virtual void makeImage(FTMachine::Type type,
			 vi::VisibilityIterator2& vi,
			 ImageInterface<Complex>& image,
			 Matrix<Float>& weight);

  //-------------------------------------------------------------------------------------

  // Rotate the uvw from the observed phase center to the
  // desired phase center.
  void rotateUVW(Matrix<Double>& uvw, Vector<Double>& dphase,
  		 const vi::VisBuffer2& vb);
  // Refocus on a finite distance
  void refocus(Matrix<Double>& uvw, const Vector<Int>& ant1,
  	       const Vector<Int>& ant2,
  	       Vector<Double>& dphase, const vi::VisBuffer2& vb);
  //helper function for openmp to call ...no private dependency
  static void locateuvw(const Double*& uvw, const Double*&dphase, const Double*& freq, const Int& nchan, const Double*& scale, const Double*& offset,  const Int& sampling, Int*& loc,Int*& off, Complex*& phasor, const Int& row, const Bool& doW=False); 
		 

  // Save and restore the FTMachine to and from a record
  virtual Bool toRecord(String& error, RecordInterface& outRecord, 
			Bool withImage=False, const String diskimagename="");
  virtual Bool fromRecord(String& error, const RecordInterface& inRecord);

  // Has this operator changed since the last application?
  virtual Bool changed(const vi::VisBuffer2& vb);
  // Can this FTMachine be represented by Fourier convolutions?
  virtual Bool isFourier() {return False;}

  //set  spw for cube that will be used;
  Bool setSpw(Vector<Int>& spw, Bool validFrame);

  //return whether the ftmachine is using a double precision grid
  virtual Bool doublePrecGrid();

  // To make sure no padding is used in certain gridders
  virtual void setNoPadding(Bool nopad){(void)nopad;};
  
  // Return the name of the machine

  virtual String name() const =0;// { return "None";};
 
  // set and get the location used for frame 
  void setLocation(const MPosition& loc);
  MPosition& getLocation();

  // set a moving source aka planets or comets =>  adjust phase center
  // on the fly for gridding 
  virtual void setMovingSource(const String& sourcename);
  virtual void setMovingSource(const MDirection& mdir);

  //reset stuff in an FTMachine
  virtual void reset(){};

  //set frequency interpolation type
  virtual void setFreqInterpolation(const String& method);

  //tell ftmachine which Pointing table column to use for Direction
  //Mosaic or Single dish ft use this for example
  virtual void setPointingDirColumn(const String& column="DIRECTION");

  virtual String getPointingDirColumnInUse();

  virtual void setSpwChanSelection(const Cube<Int>& spwchansels);
  virtual void setSpwFreqSelection(const Matrix<Double>& spwfreqs);

  // set the order of the Taylor term for MFS this is to tell
  // A-Projection to qualify the accumulated avgPB for each Taylor
  // term in the CFCache.
  virtual void setMiscInfo(const Int qualifier)=0;

  virtual void setCanComputeResiduals(Bool& b) {canComputeResiduals_p=b;};
  virtual Bool canComputeResiduals() {return canComputeResiduals_p;};
  //
  // Make the VB and VBStore interefaces for the interim re-factoring
  // work.  Finally removed the VB interface.
  virtual void ComputeResiduals(vi::VisBuffer2&vb, Bool useCorrected) = 0;
  virtual Float getPBLimit() {return pbLimit_p;};
  //virtual void ComputeResiduals(VBStore& vb)=0;
  //get and set numthreads
  void setnumthreads(Int n);
  Int getnumthreads();

  String getCacheDir() { return cfCache_p->getCacheDir(); };

protected:

  friend class VisModelData;
  friend class MultiTermFT;
  friend class MultiTermFTNew;
  LogIO logIO_p;

  LogIO& logIO();

  ImageInterface<Complex>* image;

  casa::UVWMachine* uvwMachine_p;

  MeasFrame mFrame_p;

  // Direction of desired tangent plane
  Bool tangentSpecified_p;
  MDirection mTangent_p;

  MDirection mImage_p;

  // moving source stuff
  MDirection movingDir_p;
  Bool fixMovingSource_p;
  MDirection firstMovingDir_p;
    

  Double distance_p;

  uInt nAntenna_p;

  Int lastFieldId_p;
  Int lastMSId_p;
  //Use douple precision grid in gridding process
  Bool useDoubleGrid_p;

  virtual void initMaps(const vi::VisBuffer2& vb);
  virtual void initPolInfo(const vi::VisBuffer2& vb);

  // Sum of weights per polarization and per chan
  Matrix<Double> sumWeight, sumCFWeight;

  // Sizes
  Int nx, ny, npol, nchan, nvischan, nvispol;

  // Maps of channels and polarization
  Vector<Int> chanMap, polMap;

  // Is Stokes I only? iso XX,XY,YX,YY or LL,LR,RL,RR.
  Bool isIOnly;

  // Default Position used for phase rotations
  MPosition mLocation_p;

  // Set if uvwrotation is necessary

  Bool doUVWRotation_p;
  virtual void ok();

  // check if image is big enough for gridding
  
  virtual void gridOk (Int gridsupport);

  
  // setup multiple spectral window for cubes
  Block <Vector <Int> > multiChanMap_p;
  Vector<Int> selectedSpw_p;
  Vector<Int> nVisChan_p;
  Bool matchChannel(const Int& spw, 
		    const VisBuffer& vb);
  Bool matchChannel(const vi::VisBuffer2& vb);
  //redo all spw chan match especially if ms has changed underneath 
  Bool matchAllSpwChans(const VisBuffer& vb);
  Bool matchAllSpwChans(const vi::VisBuffer2& vb);
  //interpolate visibility data of vb to grid frequency definition
  //flag will be set the one as described in interpolateArray1D
  //return False if no interpolation is done...for e.g for nearest case

  virtual Bool interpolateFrequencyTogrid(const vi::VisBuffer2& vb,
  					  const Matrix<Float>& wt,
  					  Cube<Complex>& data,
  					  Cube<Int>& flag,
  					  Matrix<Float>& weight,
  					  FTMachine::Type type=FTMachine::OBSERVED );
  //degridded data interpolated back onto visibilities

  virtual Bool interpolateFrequencyFromgrid(vi::VisBuffer2& vb,
  					    Cube<Complex>& data,
  					    FTMachine::Type type=FTMachine::MODEL );

  //Interpolate visibilities to be degridded upon

  virtual void getInterpolateArrays(const vi::VisBuffer2& vb,
  				    Cube<Complex>& data, Cube<Int>& flag);


  void setSpectralFlag(const vi::VisBuffer2& vb, Cube<Bool>& modflagcube);
  //helper to save Measures in a record
  Bool saveMeasure(RecordInterface& rec, const String& name, String& error, const Measure& ms);

  // Private variables needed for spectral frame conversion 
  SpectralCoordinate spectralCoord_p;
  Vector<Bool> doConversion_p;
  Bool freqFrameValid_p;
  Vector<Double> imageFreq_p;
  //Vector of float lsrfreq needed for regridding
  Vector<Double> lsrFreq_p;
  Vector<Double> interpVisFreq_p;
  InterpolateArray1D<Double,Complex>::InterpolationMethod freqInterpMethod_p;
  String pointingDirCol_p;
  Cube<Int> spwChanSelFlag_p;
  Matrix<Double> spwFreqSel_p, expandedSpwFreqSel_p,expandedSpwConjFreqSel_p;
  Vector<Int> cfStokes_p;
  Int polInUse_p;
  CountedPtr<CFCache> cfCache_p;
  CFStore cfs_p, cfwts_p;
  CFStore2 cfs2_p, cfwts2_p;

  CountedPtr<ConvolutionFunction> convFuncCtor_p;
  CountedPtr<PolOuterProduct> pop_p;

  Bool canComputeResiduals_p;
  Bool toVis_p;
  Int numthreads_p;
  
  // Array for non-tiled gridding
  // These are common to most FTmachines
  Array<Complex> griddedData;
  Array<DComplex> griddedData2;


  Float pbLimit_p;
  //  Vector<SkyJones *> sj_p;
  Vector<CountedPtr<refim::SkyJones> > sj_p;
  //A holder for the complex image if nobody else is keeping it
  CountedPtr<ImageInterface<Complex> > cmplxImage_p;

 private:
  //Some temporary wasteful function for swapping axes because we don't 
  //Interpolation along the second axis...will need to implement 
  //interpolation on y axis of a cube. 
  
  void swapyz(Cube<Complex>& out, const Cube<Complex>& in);
  void swapyz(Cube<Bool>& out, const Cube<Bool>& in);
  void convUVW(Double& dphase, Vector<Double>& thisrow);
  
};

  }//# end namespace refim
}//#end of namespace casa
#endif



