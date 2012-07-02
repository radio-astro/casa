//# MosaicSkyEquation.cc: Implementation of Multi Field Sky Equation classes
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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


#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MeasConvert.h>

#include <synthesis/MeasurementEquations/MosaicSkyEquation.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/SubImage.h>
#include <synthesis/MeasurementComponents/SkyJones.h>
#include <synthesis/MeasurementComponents/FTMachine.h>
#include <components/ComponentModels/Flux.h>
#include <synthesis/MeasurementComponents/ComponentFTMachine.h>
#include <synthesis/MeasurementComponents/SkyModel.h>
#include <msvis/MSVis/VisSet.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisBufferUtil.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>
#include <lattices/Lattices/Lattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <lattices/Lattices/LatticeIterator.h>


#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/iostream.h>

#include <casa/System/ProgressMeter.h>
#include <casa/OS/Timer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// ***************************************************************************
// ********************  Start of public member functions ********************
// ***************************************************************************


void MosaicSkyEquation::init()
{
   nXFR_p = 0;
   nWarnings =0;
   maxNumXFR_p=0;
   fSubIm_p=0;
   cSubIm_p=0;
};

//----------------------------------------------------------------------
MosaicSkyEquation::MosaicSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft,
			 FTMachine& ift, const Float padding)
  :  SkyEquation(sm, vs, ft, ift), padding_p(padding)
{
  init();
};

//----------------------------------------------------------------------
MosaicSkyEquation::MosaicSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, 
			     const Float padding)
  :  SkyEquation(sm, vs, ft), padding_p(padding)
{
  init();
};

//----------------------------------------------------------------------
MosaicSkyEquation::MosaicSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft,
			     FTMachine& ift, ComponentFTMachine& cft, 
			     const Float padding)
  : SkyEquation(sm, vs, ft, ift, cft), padding_p(padding)
{
  init();
};

//----------------------------------------------------------------------
MosaicSkyEquation::MosaicSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft,
			 ComponentFTMachine& cft, const Float padding)
  : SkyEquation(sm, vs, ft, cft), padding_p(padding)
{
  init();
};

//----------------------------------------------------------------------
MosaicSkyEquation::~MosaicSkyEquation() {
  for (Int nmod=0; nmod<sm_->numberOfModels(); nmod++) {
    for (Int nXFR=0; nXFR<maxNumXFR_p; nXFR++) {
      if(imgreg_p[nmod*maxNumXFR_p+nXFR])
        delete imgreg_p[nmod*maxNumXFR_p+nXFR];
    }
  }
  if (fSubIm_p) {
    delete fSubIm_p;
    fSubIm_p = 0;
  }
  if (cSubIm_p) {
    delete cSubIm_p;
    cSubIm_p = 0;
  }
}

//---------------------------------------------------------------------- 
MosaicSkyEquation& MosaicSkyEquation::operator=(const MosaicSkyEquation& other)
{
  if(this!=&other) {
    SkyEquation::operator=(other);
  };
  return *this;
};

//----------------------------------------------------------------------
MosaicSkyEquation::MosaicSkyEquation(const MosaicSkyEquation& other)
{
  SkyEquation::operator=(other);
  padding_p = other.padding_p;
}

// this version makes the UNWEIGHTED one
// Just use weight of 1.0 for each pointing

//----------------------------------------------------------------------
void MosaicSkyEquation::makeApproxPSF(Int model, ImageInterface<Float>& PSF) {

  AlwaysAssert(ok(),AipsError);
  LogIO os(LogOrigin("MosaicSkyEquation", "makeApproxPSF"));

  // In this version, we calculate the PSF explicitly so
  // we need to avoid using the shortcut
  Bool doPSF=False;
  sumwt = 0.0;
  chisq = 0.0;

  {
    VisIter& vi(vs_->iter());
    
    // Loop over all visibilities and pixels
    VisBuffer vb(vi);
    
    // Initialize put (i.e. transform to Sky) for this model
    vi.origin();

    IPosition start(4, sm_->image(model).shape()(0)/2,
		    sm_->image(model).shape()(1)/2, 0, 0);
    IPosition shape(4, 1, 1, 1, sm_->image(model).shape()(3));
    Array<Float> line(shape);
    sm_->image(model).set(0.0);
    line=1.0;
    sm_->image(model).putSlice(line, start);
    initializeGet(vb, -1, model, False);
    // Loop over all visibilities
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {
	vb.setModelVisCube(Complex(0.0,0.0));
	vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	get(vb, model, False);
	vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
      }
    }
    finalizeGet();
  }

  // Initialize the gradients
  sm_->initializeGradients();

  ROVisIter& vi(vs_->iter());

  // Reset the various SkyJones
  resetSkyJones();
  
  // Loop over all visibilities and pixels
  VisBuffer vb(vi);
  
  if(sm_->isSolveable(model)) {
    
    // Initialize 
    vi.origin();
    
    // Change the model polarization frame
    if(vb.polFrame()==MSIter::Linear) {
      StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					SkyModel::LINEAR);
    }
    else {
      StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					SkyModel::CIRCULAR);
    }
    
    vi.originChunks();
    vi.origin();
    initializePutPSF(vb, model);
    Int cohDone=0;
    
    ostringstream modelName;modelName<<"Model "<<model+1
				  <<" : making PSF";
    ProgressMeter pm(0.0, Double(vs_->numberCoh()),
		     modelName, "", "", "", True);
    // Loop over the visibilities, putting VisBuffers
    
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {
	
	putPSF(vb, model, doPSF);
	cohDone+=vb.nRow();
	pm.update(Double(cohDone));
      }
    }
    // Do the transform, apply the SkyJones transformation
    // and sum the statistics for this model
    finalizePutPSF(vb, model);

    // Finish off any calculations needed internal to SkyModel
    sm_->finalizeGradients();

    LatticeExprNode LEN = max( sm_->gS(model)  );
    Float psfMax = LEN.getFloat();

    if (psfMax > 0.0) {
      PSF.copyData( (LatticeExpr<Float>) ( sm_->gS(model) / psfMax) );
    } else {
      os << LogIO::SEVERE << "Peak of PSF <= 0.0" <<  LogIO::POST;
    }
  }
}


void MosaicSkyEquation::initializePutPSF(const VisBuffer& vb, Int model) {
  AlwaysAssert(ok(),AipsError);

  sm_->weight(model) = 1.0;
  ift_->initializeToSky(sm_->cImage(model),  sm_->weight(model), vb);
  assertSkyJones(vb, -1);
  vb_p.assign(vb, False);
  vb_p.updateCoordInfo();

}

void MosaicSkyEquation::putPSF(const VisBuffer & vb, Int model, Bool dopsf) {

  AlwaysAssert(ok(),AipsError);

  Bool IFTChanged=changedIFTMachine(vb);

  // Check to see if the SkyJones are constant
  // The SkyJones are not constant. We need to find out if there are
  // any changes in this buffer
  Bool internalChanges=False;  // Does this VB change inside itself?
  Bool firstOneChanges=False;  // Has this VB changed from the previous one?
  changedSkyJonesLogic(vb, firstOneChanges, internalChanges);
  
  Int nRow=vb.nRow();
  if(internalChanges) {
    // Yes there are changes: go row by row. 
    for (Int row=0; row<nRow; row++) {
      if(IFTChanged||changedSkyJones(vb,row)) {
	// Need to apply the SkyJones from the previous row
	// and finish off before starting with this row
	finalizePutPSF(vb_p, model);
	initializePutPSF(vb, model);
      }
      ift_->put(vb, row, dopsf);
    }
  }
  else {
    if (IFTChanged||firstOneChanges) {
      finalizePutPSF(vb_p, model);
      initializePutPSF(vb, model);
    }
    ift_->put(vb, -1, dopsf);
  }
}


void MosaicSkyEquation::finalizePutPSF(const VisBuffer& vb, Int model) {

  // Actually do the transform. Update weights as we do so.
  ift_->finalizeToSky();

  // 1. Now get the (unnormalized) image and add the 
  // weight to the summed weight
  Matrix<Float> delta;
  sm_->cImage(model).copyData(ift_->getImage(delta, False));
  sm_->weight(model) +=delta;
  // 2. Apply the SkyJones and add to grad chisquared
  applySkyJonesInv(vb, -1, sm_->cImage(model), sm_->work(model),
                   sm_->gS(model));
  // 3. If we need it, apply the square of the SkyJones and add
  // this to gradgrad chisquared
  Matrix<Float> unit_weight;
  unit_weight.resize(sm_->weight(model).shape());
  unit_weight = 1.0;
  
  applySkyJonesSquare(vb, -1, unit_weight, sm_->work(model),
		      sm_->ggS(model));

  // 4. Finally, we add the statistics
  sm_->addStatistics(sumwt, chisq);
}


// This one is very similar to SkyEquation::incrementGradientsChiSquared(),
// but we look at progress making the separate XFR's
//----------------------------------------------------------------------
void MosaicSkyEquation::incrementGradientsChiSquared() {

  AlwaysAssert(ok(),AipsError);

  ROVisIter& vi(vs_->iter());

  // Reset the various SkyJones
  resetSkyJones();

  // Loop over all models in SkyModel
  for (Int model=0;model<sm_->numberOfModels();model++) {

    if(sm_->isSolveable(model)) {

      iDebug_p++;

      ostringstream modelName;modelName<<"Model "<<model+1
				    <<" : convolution for each pointing";
      ProgressMeter *pmp=0;      
      if (nXFR_p > 1) {
	pmp = new ProgressMeter(1.0, Double(nXFR_p),
		       modelName, "", "", "", True);
      }
      scaleDeltaImage(model);
      VisBuffer vb(vi);
      vi.origin();

      // Change the model polarization frame
      if(vb.polFrame()==MSIter::Linear) {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					  SkyModel::LINEAR);
      }
      else {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					  SkyModel::CIRCULAR);
      }

      Int numXFR=0;
      vi.originChunks();
      vi.origin();
      initializePutConvolve(vb, model, numXFR);
      // Iterate
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	for (vi.origin(); vi.more(); vi++) {
	  putConvolve(vb, model, numXFR);
	  if (pmp) pmp->update(Double(numXFR+1));
	}
      }
      // Finish off
      finalizePutConvolve(vb_p, model, numXFR);
      unScaleDeltaImage(model);    
      if (pmp) delete pmp;
    }
  }
  // Finish off any calculations needed internal to SkyModel
  sm_->finalizeGradients();

};



void MosaicSkyEquation::initializePutXFR(const VisBuffer& vb, Int model,
                                     Int numXFR) {
  LogIO os(LogOrigin("MosaicSkyEquation", "initializePutXFR"));
  AlwaysAssert(ok(),AipsError);
  Matrix<Float> weight;

  if (ej_ ){
    setImageRegion( ej_->extent( sm_->image(model), vb, -1, padding_p), model, numXFR);
    sm_->setImageRegion( imageRegion(model, numXFR) );
  } else {
    if (nWarnings == 0) {
      os << "MF algorithm seeks to do PB-sized FFTs but no VPSkyJones has been set" 
         << LogIO::POST;
      nWarnings++;
    }
  }  
  ift_->initializeToSky(sm_->XFR(model, numXFR), weight, vb);
  sm_->unsetImageRegion();
  assertSkyJones(vb, -1);
  vb_p.assign(vb, False);
  vb_p.updateCoordInfo();
};

void MosaicSkyEquation::finalizePutXFR(const VisBuffer& vb, Int model, Int numXFR) 
{
  nXFR_p++;
  SkyEquation::finalizePutXFR(vb, model, numXFR);
};



// Here we do the convolution and transform back
void MosaicSkyEquation::finalizePutConvolve(const VisBuffer& vb, Int model,
                                      Int numXFR) 
{
  LogIO os(LogOrigin("MosaicSkyEquation", "finalizePutConvolve"));

  if (ej_ ){
    // Minimum-sized FFT stuff
    // Our strategy: don't allocate any new images, just window in on
    // the SkyModel's existing images as SubImages.
    // SO, as cImage is writable and will contain the result,
    // we need to zero it out in advance of working on it
    
    SubImage<Float>  subDeltaImage( sm_->deltaImage(model), imageRegion(model, numXFR) );
    sm_->cImage(model).set(0.0);
    SubImage<Complex> subcImage( sm_->cImage(model), imageRegion(model, numXFR), True);

    applySkyJones(vb, -1, subDeltaImage, subcImage);
    LatticeFFT::cfft2d( subcImage );
    LatticeExpr<Complex> latticeExpr(conj(sm_->XFR(model, numXFR)) * subcImage );
    subcImage.copyData(latticeExpr);
    LatticeFFT::cfft2d(subcImage, False);
  } else {
    
    applySkyJones(vb, -1, sm_->deltaImage(model), sm_->cImage(model));
    LatticeFFT::cfft2d(sm_->cImage(model));
    LatticeExpr<Complex> latticeExpr(conj(sm_->XFR(model, numXFR))*sm_->cImage(model));
    sm_->cImage(model).copyData(latticeExpr);
    LatticeFFT::cfft2d(sm_->cImage(model), False);
  }
  applySkyJonesInv(vb, -1, sm_->cImage(model), sm_->work(model),
                   sm_->gS(model));
};



ImageRegion& MosaicSkyEquation::imageRegion(const Int model, const Int numXFR) {
  AlwaysAssert( imgreg_p[model * maxNumXFR_p + numXFR],AipsError);
  return *imgreg_p[model * maxNumXFR_p + numXFR];
}


void MosaicSkyEquation::setImageRegion(ImageRegion *imgreg, const Int model, const Int numXFR) 
{
  if (maxNumXFR_p<=numXFR) {
    // set up the PtrBlock    
    imgreg_p.resize(sm_->numberOfModels() * (numXFR+1));
    for (Int nmod=0; nmod<sm_->numberOfModels(); nmod++) {
      for (Int nXFR=maxNumXFR_p; nXFR<(numXFR+1); nXFR++) {
        imgreg_p[nmod*(numXFR+1)+nXFR]=0;
      }
    }
    maxNumXFR_p=numXFR+1;
  }

  imgreg_p[model * maxNumXFR_p + numXFR] = imgreg;

};





// Initialize
void MosaicSkyEquation::initializeGet(const VisBuffer& vb, Int row, Int model,
                                Bool incremental) {
  AlwaysAssert(ok(),AipsError);
  LogIO os(LogOrigin("MosaicSkyEquation", "initializeGet"));

  if (ej_ ){
    // Do the SubImage dance
    ImageRegion *myImageRegion =  ej_->extent(sm_->image(model), 
					      vb, -1, padding_p); 					     
    if(incremental) {
      fSubIm_p = new SubImage<Float>( sm_->deltaImage(model), *myImageRegion);
    } else {
      fSubIm_p = new SubImage<Float>( sm_->image(model), *myImageRegion);
    }
    cSubIm_p = new TempImage<Complex> ( fSubIm_p->shape(), fSubIm_p->coordinates() );
    //    os << "DEBUG: MosaicSkyEquation::initializeGet going from " << sm_->image(model).shape() <<
    //  " to " << cSubIm_p->shape() << LogIO::POST;

    applySkyJones(vb, row, *fSubIm_p, *cSubIm_p);


    // Note: can delete fSubIm_p, but we need to keep cSubIm_p
    // around to degrid from; finalizeGet() will take care of it
    delete fSubIm_p;  fSubIm_p = 0;
    delete myImageRegion;
    ft_->initializeToVis(*cSubIm_p, vb);
  }
  else {
    if (nWarnings == 0) {
      os << "MF algorithm seeks to do PB-sized FFTs but no VPSkyJones has been set" 
         << LogIO::POST;
    }
    if(incremental) {
      applySkyJones(vb, row, sm_->image(model), sm_->cImage(model));
    } else {
      applySkyJones(vb, row, sm_->image(model), sm_->cImage(model));
    }
  }
};

void MosaicSkyEquation::finalizeGet(){
  LogIO os(LogOrigin("MosaicSkyEquation", "finalizeGet"));
  AlwaysAssert(ok(),AipsError);
  if (cSubIm_p) {
    delete cSubIm_p;
    cSubIm_p = 0;
  }
  if (fSubIm_p) {
    delete fSubIm_p;
    fSubIm_p = 0;
  }
  AlwaysAssert( (fSubIm_p==0),AipsError);
  AlwaysAssert( (cSubIm_p==0),AipsError);
};



void MosaicSkyEquation::initializePut(const VisBuffer& vb, Int model) {
  AlwaysAssert(ok(),AipsError);
  LogIO os(LogOrigin("MosaicSkyEquation", "initializePut"));

  if (!ej_ ){
    os << LogIO::SEVERE 
       << "MF algorithm seeks to do PB-sized FFTs but no VPSkyJones has been set" 
       << LogIO::POST;
    return;
  }
  ImageRegion *myImageRegion =  ej_->extent(sm_->image(model), 
					    vb, -1, padding_p);  
  cSubIm_p = new SubImage<Complex>(sm_->cImage(model), *myImageRegion, True);
  delete myImageRegion;

  ift_->initializeToSky(*cSubIm_p, sm_->weight(model),vb);
  assertSkyJones(vb, -1);
  vb_p.assign(vb, False);
  vb_p.updateCoordInfo();
}


void MosaicSkyEquation::finalizePut(const VisBuffer& vb, Int model) {
  // Actually do the transform. Update weights as we do so.
  ift_->finalizeToSky();

  if(ej_) {
    ImageRegion ir(*ej_->extent(sm_->work(model), vb, -1, padding_p));  
    SubImage<Float> sgS(sm_->gS(model), ir, True);
    SubImage<Float> sWork(sm_->work(model), ir, True);
    SubImage<Float> sggS(sm_->ggS(model), ir, True);

    // 1. Now get the (unnormalized) image and add the 
    // weight to the summed weight
    Matrix<Float> delta;
    cSubIm_p->copyData(ift_->getImage(delta, False));
    sm_->weight(model)+=delta;
    
    // 2. Apply the SkyJones and add to grad chisquared
    applySkyJonesInv(vb, -1, *cSubIm_p, sWork, sgS);
    delete cSubIm_p;
    cSubIm_p = 0;
    
    // 3. If we need it, apply the square of the SkyJones and add
    // this to gradgrad chisquared
    applySkyJonesSquare(vb, -1, sm_->weight(model), sWork, sggS);
  }
  else {
    // 1. Now get the (unnormalized) image and add the 
    // weight to the summed weight
    Matrix<Float> delta;
    sm_->cImage(model).copyData(ift_->getImage(delta, False));
    sm_->weight(model)+=delta;
    
    // 2. Apply the SkyJones and add to grad chisquared
    applySkyJonesInv(vb, -1, sm_->cImage(model), sm_->work(model),
		     sm_->gS(model));
    
    // 3. If we need it, apply the square of the SkyJones and add
    // this to gradgrad chisquared
    applySkyJonesSquare(vb, -1, sm_->weight(model), sm_->work(model),
			sm_->ggS(model));
  }

  // 4. Finally, we add the statistics
  sm_->addStatistics(sumwt, chisq);
}


} //# NAMESPACE CASA - END

