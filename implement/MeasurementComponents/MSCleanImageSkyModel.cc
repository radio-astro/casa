//# MSCleanImageSkyModel.cc: Implementation of MSCleanImageSkyModel class
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$


#include <casa/Arrays/ArrayMath.h>
#include <synthesis/MeasurementComponents/MSCleanImageSkyModel.h>
#include <casa/OS/File.h>
#include <lattices/Lattices/LatticeCleaner.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <synthesis/MeasurementEquations/LatticeModel.h>
#include <synthesis/MeasurementEquations/LatConvEquation.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <lattices/Lattices/TempLattice.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>


namespace casa { //# NAMESPACE CASA - BEGIN

MSCleanImageSkyModel::MSCleanImageSkyModel(const Int nscales, const Float& smallScaleBias)
: method_p(NSCALES), nscales_p(nscales), smallScaleBias_p(smallScaleBias), progress_p(0)
{
  modified_p=True;
  donePSF_p=False;

};

MSCleanImageSkyModel::MSCleanImageSkyModel(const Vector<Float>& userScaleSizes, const Float& smallScaleBias)
: method_p(USERVECTOR), userScaleSizes_p(userScaleSizes), smallScaleBias_p(smallScaleBias), progress_p(0)
{
  modified_p=True;
  donePSF_p=False;

};

MSCleanImageSkyModel::~MSCleanImageSkyModel()
{
  if (progress_p) {
    delete progress_p;
  }
};

// Clean solver
Bool MSCleanImageSkyModel::solve(SkyEquation& se) {


  LogIO os(LogOrigin("MSCleanImageSkyModel","solve"));
  
  if(numberOfModels()>1) {
    os << "Cannot process more than one field" << LogIO::EXCEPTION;
  }
  
  if (displayProgress_p) {
    progress_p = new LatticeCleanProgress( pgplotter_p );
  }

  // Make the residual image
  if(modified_p)
    makeNewtonRaphsonStep(se);
  
  //Make the PSF
  if(!donePSF_p)
    makeApproxPSFs(se);

  if(numberIterations() <1){
    return True;
  }
  
  
  if(!isSolveable(0)) {
    os << "Model 1 is not solveable!" << LogIO::EXCEPTION;
  }
  
  Vector<Float> scaleSizes(nscales_p);  
  if (method_p == USERVECTOR) {
    if (userScaleSizes_p.nelements() <= 0) {
      os << LogIO::SEVERE 
	 << "Need at least one scale for method uservector"
	 << LogIO::POST;
    }
    os << "Creating scales from uservector method: " << LogIO::POST;
    for(uInt scale=0; scale < userScaleSizes_p.nelements(); scale++) {
      os << "scale " << scale+1 << " = " << userScaleSizes_p(scale)
	 << " pixels" << LogIO::POST;
    }
  } else {
    if (nscales_p <= 0) nscales_p = 1;
    scaleSizes.resize(nscales_p);  
    os << "Creating " << nscales_p << 
      " scales from powerlaw nscales method" << LogIO::POST;
    scaleSizes(0) = 0.0;
    os << "scale 1 = 0.0 pixels " << LogIO::POST;
    Float scaleInc = 2.0;
    for (Int scale=1; scale<nscales_p;scale++) {
      scaleSizes(scale) =
	scaleInc * pow(10.0, (Float(scale)-2.0)/2.0);
      os << "scale " << scale+1 << " = " << scaleSizes(scale)
	 << " pixels" << LogIO::POST;
    }  
  }

  Int npol=image(0).shape()(2);
  Int nchan=image(0).shape()(3);
  
  AlwaysAssert((npol==1)||(npol==2)||(npol==3)||(npol==4), AipsError);  
  IPosition blcDirty(image(0).shape().nelements(), 0);
  IPosition trcDirty(image(0).shape()-1);

  if(hasMask(0)) {
    Int masknpol=mask(0).shape()(2);
    if(masknpol>1) {
      if(masknpol!=npol) {
	os << "Mask has more than one polarization but not the same as the image" << LogIO::EXCEPTION;
      }
      else {
	os << "Mask is a cube in polarization - will use appropriate plane for each polarization" << LogIO::POST;
      }
    }
    Int masknchan=mask(0).shape()(3);
    if(masknchan>1) {
      if(masknchan!=nchan) {
	os << "Mask has more than one channel but not the same as the image" << LogIO::EXCEPTION;
      }
      else {
	os << "Mask is a spectral cube - will use appropriate plane for each channel" << LogIO::POST;
      }
    }
  }
  
  Int converged=0;
  // Loop over all channels and polarizations
  for (Int chan=0; chan<nchan; chan++) {

    LCBox psfbox(blcDirty, trcDirty,PSF(0).shape());
    SubLattice<Float>  psf_sl (PSF(0), psfbox, False);
    Float psfmax;
    {
      LatticeExprNode node = max(psf_sl);
      psfmax = node.getFloat();
    }
    if(nchan>1) {
     os<<"Processing channel "<<chan+1<<" of "<<nchan<<LogIO::POST;
    }
    if(psfmax==0.0) {
      os << "No data for this channel: skipping" << LogIO::POST;
    } else {
      blcDirty(3) = chan; trcDirty(3) = chan;
      blcDirty(2) = 0; trcDirty(2) = 0;
      LCBox firstPolPlane(blcDirty, trcDirty, image(0).shape());
      SubLattice<Float> subPsf(PSF(0), firstPolPlane);
      
      for (Int pol=0; pol<npol; pol++) {
	blcDirty(2) = pol; trcDirty(2) = pol;
	if(npol>1) {
	  os<<"Processing polarization "<<pol+1<<" of "<<npol<<LogIO::POST;
	}

	LCBox onePlane(blcDirty, trcDirty, image(0).shape());
	SubLattice<Float> subDirty(residual(0), onePlane, True);
	LatticeCleaner<Float> cleaner(subPsf, subDirty);

	Bool doClean=True;
	if(hasMask(0)) {
	  IPosition blcMask(mask(0).shape().nelements(), 0);
	  IPosition trcMask(mask(0).shape()-1);

	  Int masknpol=mask(0).shape()(2);
	  Int masknchan=mask(0).shape()(3);
  
	  if(masknpol==npol) {
	    blcMask(2)=pol;
	    trcMask(2)=pol;
	  }
	  else {
	    blcMask(2)=0;
	    trcMask(2)=0;
	  }
	  if(masknchan==nchan) {
	    blcMask(3)=chan;
	    trcMask(3)=chan;
	  }
	  else {
	    blcMask(3)=0;
	    trcMask(3)=0;
	  }
	  LCBox maskPlane(blcMask, trcMask, mask(0).shape());
	  SubLattice<Float> subMask( mask(0), maskPlane, False); 
	  // Check for empty mask
	  LatticeExprNode sumMask = sum(subMask);
	  if(sumMask.getFloat()==0.0) {
	    os << LogIO::WARN << "Mask is specified but empty - no cleaning performed" << LogIO::POST;
	    doClean=False;
	  }
	  else {
	    cleaner.setMask( subMask );
	  }
	}

	

	if(doClean) {
	  SubLattice<Float> subImage(image(0), onePlane, True);
	  
	  if (method_p == USERVECTOR) {
	    cleaner.setscales(userScaleSizes_p);   
	  } else {
	    cleaner.setscales(scaleSizes);   
	  }
          cleaner.setSmallScaleBias(smallScaleBias_p);
	  cleaner.setcontrol(CleanEnums::MULTISCALE, numberIterations(), gain(), 
			     Quantity(threshold(), "Jy"), True);
	  
	  converged=cleaner.clean(subImage, progress_p);
	
	  // calculate residuals 
	  
	  LatticeModel lm(subImage);
	  LatConvEquation eqn(subPsf, subDirty);
	  TempLattice<Float> restl( subDirty.shape() );
	  eqn.residual(restl, lm);
	  subDirty.copyData(restl);
	}
      }
    }
  }
  modified_p=True;

  return(converged);
};

} //# NAMESPACE CASA - END

