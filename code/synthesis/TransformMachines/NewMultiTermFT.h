//# NewMultiTermFT.h: Definition for NewMultiTermFT
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

#ifndef SYNTHESIS_NEWMULTITERMFT_H
#define SYNTHESIS_NEWMULTITERMFT_H

#include <synthesis/TransformMachines/FTMachine.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/FFTServer.h>
#include <msvis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageInterface.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
//#include <synthesis/MeasurementComponents/SynthesisPeek.h>
#include <casa/OS/Timer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class UVWMachine;

class NewMultiTermFT : public FTMachine {
public:

  // Construct using an existing FT-Machine 
  NewMultiTermFT(FTMachine *subftm, Int nterms=1, Double reffreq=0.0);

  // Construct from a Record containing the NewMultiTermFT state
  NewMultiTermFT(const RecordInterface& stateRec);

  // Copy constructor. 
  // This first calls the default "=" operator, and then instantiates objects for member pointers.
  NewMultiTermFT(const NewMultiTermFT &other);

  // Assignment operator --- leave it as the default
  NewMultiTermFT &operator=(const NewMultiTermFT &other);

  // Destructor
  ~NewMultiTermFT();

  // Called at the start of de-gridding : subftm->initializeToVis()
  // Note : Pre-de-gridding model-image divisions by PBs will go here.
  void initializeToVis(ImageInterface<Complex>& /*image*/, 
                       const VisBuffer& /*vb*/)
  {throw(AipsError("NewMultiTermFT::initializeToVis called without vectors !"));};

   // Vectorized InitializeToVis
  void initializeToVis(Block<CountedPtr<ImageInterface<Complex> > > & compImageVec,PtrBlock<SubImage<Float> *> & modelImageVec, PtrBlock<SubImage<Float> *>& weightImageVec, PtrBlock<SubImage<Float> *>& fluxScaleVec, Block<Matrix<Float> >& weightsVec, const VisBuffer& vb);

  // Called at the end of de-gridding : subftm->finalizeToVis()
  void finalizeToVis();

  // Called at the start of gridding : subftm->initializeToSky()
  void initializeToSky(ImageInterface<Complex>& /*image*/,  
                       Matrix<Float>& /*weight*/, const VisBuffer& /*vb*/) 
   {throw(AipsError("NewMultiTermFT::initializeToSky() called without vectors!"));};

  void initializeToSky(Block<CountedPtr<ImageInterface<Complex> > > & compImageVec, Block<Matrix<Float> >& weightsVec, const VisBuffer& vb, const Bool dopsf);


  // Called at the end of gridding : subftm->finalizeToSky()
  void finalizeToSky(){throw(AipsError("NewMultiTermFT::finalizeToSky() called without arguments!"));};

  void finalizeToSky(Block<CountedPtr<ImageInterface<Complex> > > & compImageVec, PtrBlock<SubImage<Float> *> & resImageVec, PtrBlock<SubImage<Float> *>& weightImageVec, PtrBlock<SubImage<Float> *>& fluxScaleVec, Bool dopsf, Block<Matrix<Float> >& weightsVec, const VisBuffer& vb);

  //  void normalizeToSky(ImageInterface<Complex>& compImage, ImageInterface<Float>& resImage, ImageInterface<Float>& weightImage, Bool dopsf, Matrix<Float>& weights)
  // {throw(AipsError("NewMultiTermFT::normalizeToSky should not get called !"));};


  // Do the degridding via subftm->get() and modify model-visibilities by Taylor-weights
  void get(VisBuffer& vb, Int row=-1);

  // Modify imaging weights with Taylor-weights and do gridding via subftm->put()
  void put(VisBuffer& vb, Int row=-1, Bool dopsf=False,
	   FTMachine::Type type=FTMachine::OBSERVED);

  // Have a const version for compatibility with other FTMs.. Throw an exception if called.
  void put(const VisBuffer& /*vb*/, Int /*row*/=-1, Bool /*dopsf*/=False,
	   FTMachine::Type /*type*/=FTMachine::OBSERVED)
  {throw(AipsError("NewMultiTermFT::put called with a const vb. This FTM needs to modify the vb."));};

  // Calculate residual visibilities if possible.
  // The purpose is to allow rGridFT to make this multi-threaded
  virtual void ComputeResiduals(VisBuffer&vb, Bool useCorrected); 

  // Make an image : subftm->makeImage()
  void makeImage(FTMachine::Type type,
		 VisSet& vs,
		 ImageInterface<Complex>& image,
		 Matrix<Float>& weight);
  
  // Get the final image: do the Fourier transform grid-correct, then 
  // optionally normalize by the summed weights
  // Note : Post-gridding residual-image divisions by PBs will go here.
  //           For now, it just calls subftm->getImage()
  //  ImageInterface<Complex>& getImage(Matrix<Float>& weights, Bool normalize=True)
  //{return getImage(weights,normalize,0);};
  //ImageInterface<Complex>& getImage(Matrix<Float>& weights, Bool normalize=True, 
  //                                                         const Int taylorindex=0);
  ImageInterface<Complex>& getImage(Matrix<Float>& /*weights*/, Bool /*normalize*/=True)
  {throw(AipsError("NewMultiTermFT::getImage() should not be called"));}
 

  // Place-holder for possible use with AWProject and AWProjectWB FTMs
  /*
  virtual void normalizeImage(Lattice<Complex>& skyImage,
			      const Matrix<Double>& sumOfWts,
			      Lattice<Float>& sensitivityImage,
			      Bool fftNorm)
    {throw(AipsError("NewMultiTermFT::normalizeImage() is not implemented"));}
  */
  // Get the final weights image - this will hold PB2
  //void getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights)
  //{getWeightImage(weightImage, weights, 0);};
  //void getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights, 
  //                                const Int taylorindex);
  void getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights)
  {subftms_p[0]->getWeightImage(weightImage, weights);}
  //  {throw(AipsError("NewMultiTermFT::getWeightImage() should not be called"));}

  // Save and restore the NewMultiTermFT to and from a record
  virtual Bool toRecord(String& error, RecordInterface& outRec, Bool withImage=False,
			const String diskimage="");
  virtual Bool fromRecord(String& error, const RecordInterface& inRec);

  // Various small inline functions
  virtual Bool isFourier() {return True;}
  virtual void setNoPadding(Bool nopad){subftms_p[0]->setNoPadding(nopad);};
  virtual String name()const {return machineName_p;};
  virtual void setMiscInfo(const Int qualifier){(void)qualifier;};

  void printFTTypes()
  {
    cout << "** Number of FTs : " << subftms_p.nelements() << " -- " ;
    for(uInt tix=0; tix<(subftms_p).nelements(); tix++)
      cout << tix << " : " << (subftms_p[tix])->name() << "   " ;
    cout << endl;
  };

  virtual void setDOPBCorrection(Bool doit=True) {doWideBandPBCorrection_p=doit;};
  virtual Bool getDOPBCorrection() {return doWideBandPBCorrection_p;};
  virtual void setConjBeams(Bool useit=True) {useConjBeams_p=useit;};
  virtual Bool getConjBeams() {return useConjBeams_p;};


protected:

  // Instantiate a new sub FTM
  FTMachine* getNewFTM(const FTMachine *ftm);

  // Multiply Imaging weights by Taylor-function weights - during "put"
  Bool modifyVisWeights(VisBuffer& vb, uInt thisterm);
  // Multiply model visibilities by Taylor-function weights - during "get"
  Bool modifyModelVis(VisBuffer &vb, uInt thisterm);
  // Restore vb.imagingweights to the original
  void restoreImagingWeights(VisBuffer &vb);

  // Use sumwts to make a Hessian, invert it, apply to weight images, fill in pbcoeffs_p
  void normAvgPBs(PtrBlock<SubImage<Float> *> & weightImageVec);
  void calculateTaylorPBs(PtrBlock<SubImage<Float> *> & weightImageVec);
  // Make pixel-by-pixel matrices from pbcoeffs, invert, apply to residuals
  //  void normalizeWideBandPB2(PtrBlock<SubImage<Float> *> & resImageVec);
  //  void normalizeWideBandPB(PtrBlock<SubImage<Float> *> & resImageVec, PtrBlock<SubImage<Float> *>& scratchImageVec);
  void applyWideBandPB(String action, PtrBlock<SubImage<Float> *> & imageVec);

  void multiplyHMatrix( Matrix<Double> &hmat, PtrBlock<SubImage<Float>* > &invec,
			PtrBlock<SubImage<Float>* > &outvec, String saveImagePrefix );

  // Helper function to write ImageInterfaces to disk
  Bool storeAsImg(String fileName, ImageInterface<Float> & theImg);


  Cube<Complex> modviscube_p;

  //// New MTFT specific internal parameters and functions
  uInt nterms_p;
  Bool donePSF_p, doingPSF_p;
  Double reffreq_p;
  Matrix<Float> imweights_p;
  String machineName_p;
  Float pblimit_p;
  Bool doWideBandPBCorrection_p;
  String cacheDir_p;
  Bool donePBTaylor_p;
  Bool useConjBeams_p;

  Block< CountedPtr<FTMachine> > subftms_p;
  Block<Matrix<Float> > sumweights_p;

  Double sumwt_p;
  Matrix<Double> hess_p, invhess_p; // Block is for the pol axis

  Block<CountedPtr<ImageInterface<Float> > > sensitivitymaps_p;
  PtrBlock<SubImage<Float>* > pbcoeffs_p;

  Bool dbg_p,dotime_p;
  Timer tmr_p;
  Double time_get, time_put, time_res;
};

} //# NAMESPACE CASA - END

#endif
