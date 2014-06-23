//# SDAlgorithmClarkClean.cc: Implementation of SDAlgorithmClarkClean classes
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

#include<synthesis/ImagerObjects/SIMinorCycleController.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>

#include <casa/System/Choice.h>
#include <synthesis/MSVis/StokesVector.h>

#include <synthesis/ImagerObjects/SDAlgorithmClarkClean.h>
#include <synthesis/MeasurementEquations/ClarkCleanLatModel.h>
#include <synthesis/MeasurementEquations/LatConvEquation.h>


namespace casa { //# NAMESPACE CASA - BEGIN

  SDAlgorithmClarkClean::SDAlgorithmClarkClean():
    SDAlgorithmBase()
 {
   itsAlgorithmName=String("Clark");
   itsMatDeltaModel.resize();
 }

  SDAlgorithmClarkClean::~SDAlgorithmClarkClean()
 {
   
 }

  void SDAlgorithmClarkClean::initializeDeconvolver( Float &peakresidual, Float &modelflux )
  {
    LogIO os( LogOrigin("SDAlgorithmClarkClean","initializeDeconvolver",WHERE) );

    itsImages->residual()->get( itsMatResidual, True );
    itsImages->model()->get( itsMatModel, True );
    itsImages->psf()->get( itsMatPsf, True );

    //    cout << "initDecon : " << itsImages->residual()->shape() << " : " << itsMatResidual.shape() 
    //	 << itsImages->model()->shape() << " : " << itsMatModel.shape() 
    //	 << itsImages->psf()->shape() << " : " << itsMatPsf.shape() 
    //	 << endl;

    findMaxAbs( itsMatResidual, itsPeakResidual, itsMaxPos );
    itsModelFlux = sum( itsMatModel );

    peakresidual = itsPeakResidual;
    modelflux = itsModelFlux;

    itsImages->mask()->get( itsMatMask, True );
    //    cout << "Mask in SDAlHog : " << sum( itsMatMask ) << " pixels " << endl;

    if( sum( itsMatMask )==0 ) 
      {
	os << LogIO::WARN << "ZERO MASK. Forcing all pixels to 1.0" << LogIO::POST; 
	itsMatMask = 1.0; 
      }

    // Initialize the Delta Image model. Resize if needed.
    if ( itsMatDeltaModel.shape().nelements() != itsMatModel.shape().nelements() )
      { itsMatDeltaModel.resize ( itsMatModel.shape() ); }


    //cout << "Image Shapes : " << itsMatResidual.shape() << endl;

    ////////////////////////////////////////////////////////////////////////////////////
    // Get psf patch size
    // ...........fill this in....... after fitted beams are precomputed earlier..... and stored in ImageStore.

      // 4 pixels:  pretty arbitrary, but only look for sidelobes
      // outside the inner (2n+1) * (2n+1) square
      Int ncent=4;
      /*
      {//locate peak size
	CoordinateSystem cs= itsImages->psf()->coordinates();
	Vector<String> unitas=cs.worldAxisUnits();
	unitas(0)="arcsec";
	unitas(1)="arcsec";
	cs.setWorldAxisUnits(unitas);
	//Get the minimum increment in arcsec
	Double incr=abs(min(cs.increment()(0), cs.increment()(1)));
	if(incr > 0.0){
	  GaussianBeam beamModel=beam(model)(0,0);
	  ncent=max(ncent, Int(ceil(beamModel.getMajor("arcsec")/incr)));
	  ncent=max(ncent, Int(ceil(beamModel.getMinor("arcsec")/incr)));
	}
      }
      */
      psfpatch_p=3*ncent+1;



  }


  void SDAlgorithmClarkClean::takeOneStep( Float loopgain, 
					    Int cycleNiter, 
					    Float cycleThreshold, 
					    Float &peakresidual, 
					    Float &modelflux, 
					    Int &iterdone)
  {


    //---------------------------------
    /*
    ConvolutionEquation eqn(itsMatPsf,  itsMatResidual);
    //		  deltaimageli.rwCursor().set(Float(0.0));
    itsMatDeltaModel = 0.0;
    ClarkCleanModel cleaner(itsMatDeltaModel);
    cleaner.setMask(itsMatMask);
    cleaner.setGain(loopgain);
    cleaner.setNumberIterations(cycleNiter);
    cleaner.setInitialNumberIterations(0);
    cleaner.setThreshold(cycleThreshold);
    cleaner.setPsfPatchSize(IPosition(2,psfpatch_p)); 
    cleaner.setMaxNumberMajorCycles(10);
    cleaner.setHistLength(1024);
    cleaner.setMaxNumPix(32*1024);
    cleaner.setChoose(False);
    cleaner.setCycleSpeedup(-1.0);   // Can make this an input parameter too
    cleaner.setSpeedup(0.0);
    //		  if ( displayProgress_p ) {
    //		    cleaner.setProgress( *progress_p );    // A PGPlotter dependency...
    //		  }
    cleaner.singleSolve(eqn, itsMatResidual);
    
    iterdone=cleaner.numberIterations();
    
    cleaner.getModel(itsMatDeltaModel);
    */
    //---------------------------------

    // Store current model in this matrix.
    Bool ret = itsImages->model()->get( itsMatDeltaModel, True );
    itsMatModel.assign( itsMatDeltaModel ); // This should make an explicit copy

    // Set model to zero
    itsImages->model()->set( 0.0 );

    ClarkCleanLatModel cleaner(*(itsImages->model()));
    cleaner.setMask(*(itsImages->mask()));
    cleaner.setNumberIterations(cycleNiter);
    cleaner.setMaxNumberMajorCycles(10);
    cleaner.setMaxNumberMinorIterations(cycleNiter);
    cleaner.setGain(loopgain);
    cleaner.setThreshold(cycleThreshold);
    cleaner.setPsfPatchSize(IPosition(2,psfpatch_p));
    cleaner.setHistLength(1024);
    //    cleaner.setMaxExtPsf(..)
    cleaner.setSpeedup(-1.0);
    cleaner.setMaxNumPix(32*1024);

    LatConvEquation eqn(*(itsImages->psf()), *(itsImages->residual()) );
   
    //Bool result = 
    cleaner.solve( eqn );
    //    cout << result << endl;

    iterdone = cleaner.getNumberIterations();

    // Retrieve residual before major cycle
    itsImages->residual()->copyData( cleaner.getResidual() );

    // Add delta model to old model
    Bool ret2 = itsImages->model()->get( itsMatDeltaModel, True );
    itsMatModel += itsMatDeltaModel;

    //---------------------------------

    //  Find Peak Residual
    itsImages->residual()->get( itsMatResidual, True );
    itsImages->mask()->get( itsMatMask, True );
    findMaxAbsMask( itsMatResidual, itsMatMask, itsPeakResidual, itsMaxPos );
    peakresidual = itsPeakResidual;

    // Find Total Model flux
    modelflux = sum( itsMatModel ); // Performance hog ?
    (itsImages->model())->put( itsMatModel );
  }	    

  void SDAlgorithmClarkClean::finalizeDeconvolver()
  {
    (itsImages->residual())->put( itsMatResidual );
    (itsImages->model())->put( itsMatModel );
  }


} //# NAMESPACE CASA - END

