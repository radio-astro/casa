//# SDAlgorithmBase.cc: Implementation of SDAlgorithmBase classes
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
#include <casa/OS/HostInfo.h>
#include <synthesis/MeasurementEquations/SDAlgorithmBase.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentList.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <casa/OS/File.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Directory.h>
#include <tables/Tables/TableLock.h>

#include<synthesis/MeasurementEquations/SIMinorCycleController.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


  SDAlgorithmBase::SDAlgorithmBase()
 {

   // TESTING place-holder for the position of the clean component.
   tmpPos_p = IPosition(4,0,0,0,0);

 }

  SDAlgorithmBase::~SDAlgorithmBase()
 {
   
 }

  // TODO :  Overloadable function to be called from the Mapper. 
  // Use this to tell the Mapper how to partition
  // the image for separate calls to 'deconvolve'.
  // Give codes to signal one or more of the following.
    ///    - channel planes separate
    ///    - stokes planes separate
    ///    - partitioned-image clean (facets ?)

  // Later, can add more complex partitioning schemes.... 
  // but there will be one place to do it, per deconvolver.

  void SDAlgorithmBase::queryDesiredShape(Bool &onechan, Bool &onepol) // , nImageFacets.
  {  
    onechan = True;
    onepol = True;
  }

  Bool SDAlgorithmBase::deconvolve( SIMinorCycleController &loopcontrols, 
				    ImageInterface<Float>  &residual, 
				    ImageInterface<Float>  &psf, 
				    ImageInterface<Float>  &model, 
				    CountedPtr<SDMaskHandler> /*maskhandler*/, 
				    uInt subimageid,
				    Int deconvolverid)
  {

    LogIO os( LogOrigin("SDAlgorithmBase","deconvolve",WHERE) );

    Int startiteration = loopcontrols.getIterDone();
    Float peakresidual=residual.getAt(tmpPos_p);
    Float modelflux=model.getAt(tmpPos_p);

    Float startpeakresidual = peakresidual;
    Float startmodelflux = modelflux;

    while ( ! checkStop( loopcontrols,  peakresidual ) )
      {
	Float comp=0.0;

	// Optionally, fiddle with maskhandler for autoboxing.... 
	// mask = maskhandler->makeAutoBox();

	findNextComponent( residual, psf, loopcontrols.getLoopGain(), comp );
	  
	updateResidual( residual, comp );
	updateModel ( model, comp );

	peakresidual = residual.getAt(tmpPos_p);
	modelflux = model.getAt(tmpPos_p);

	loopcontrols.incrementMinorCycleCount( );
	loopcontrols.setPeakResidual( peakresidual );
	loopcontrols.addSummaryMinor( deconvolverid, subimageid, modelflux, peakresidual );
      }

    // same as checking on itscycleniter.....
    loopcontrols.setUpdatedModelFlag( loopcontrols.getIterDone()-startiteration );

    os << "[D" << deconvolverid << ":S" << subimageid << "]"
       <<" iters=" << startiteration << "-" << loopcontrols.getIterDone()
       << ", peakres=" << startpeakresidual << "-" << peakresidual 
       << ", model=" << startmodelflux << "-" << modelflux
       << LogIO::POST;

}

  Bool SDAlgorithmBase::checkStop( SIMinorCycleController &loopcontrols, 
				   Float currentresidual )
  {
    return loopcontrols.majorCycleRequired(currentresidual);
  }
  

  void SDAlgorithmBase::findNextComponent( ImageInterface<Float>  &residual, 
					 ImageInterface<Float>  &/*psf*/, Float loopgain, 
					 Float  &comp )
  {
    //    comp = loopgain * residual;

    comp =  loopgain * residual.getAt(tmpPos_p);

  }

  void SDAlgorithmBase::updateModel( ImageInterface<Float>  &model, Float  &comp )
  {
    //    model = model + comp;

    model.putAt( model.getAt(tmpPos_p) + comp  , tmpPos_p );

  }

  void SDAlgorithmBase::updateResidual( ImageInterface<Float>  &residual, Float  &comp )
  {
    //    residual = residual - comp;
    residual.putAt( residual.getAt(tmpPos_p) - comp  , tmpPos_p );
  }

  void SDAlgorithmBase::restore(CountedPtr<SIImageStore> imagestore )
  {

    LogIO os( LogOrigin("SDAlgorithmBase","restore",WHERE) );
    
    os << "Smooth model and add residuals for " << imagestore->getName() 
       << ". Optionally, PB-correct too." << LogIO::POST;

  }

} //# NAMESPACE CASA - END

