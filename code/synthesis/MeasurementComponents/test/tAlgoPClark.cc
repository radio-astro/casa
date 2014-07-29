//# tPClark.cc: tPClark test program for parallelzation
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2004
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

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <synthesis/MeasurementComponents/PClarkCleanImageSkyModel.h>
#include <casa/OS/File.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>

#include <casa/Logging/LogSink.h>
#include <casa/Inputs/Input.h>

#include <synthesis/MeasurementEquations/ConvolutionEquation.h>
#include <synthesis/MeasurementEquations/ClarkCleanModel.h>
#include <synthesis/Parallel/Applicator.h>
#include <synthesis/MeasurementComponents/ClarkCleanAlgorithm.h>

#ifdef HasMPI
#include <mpi.h>
#endif
#include <errno.h>

#include <casa/namespace.h>
extern casa::Applicator casa::applicator;

void master(Int argc, Char *argv[]);

   // Main routine call master to set the problem up and then the 
   // slaves to do the work.

int main(Int argc, Char *argv[]){

   LogIO os(LogOrigin("tPClark","master solve in parallel",WHERE));
   casa::applicator.init(argc, argv);
   if(casa::applicator.isController()){
      try {
         master(argc, argv);
       }
       catch(AipsError x){
         cerr << "AipsError thrown : " << x.getMesg() << endl;
       } 
   }
   return(1);
}

  // Master setups up the clean;
void master(Int argc, Char *argv[]) {
   // this corresponds to the solve function for the ClarkCleanImageSkyModel

   // Input stuff is only needed for rank 0 so do the MPI_Init stuff
   // then split it up into master slave.


   Input inputs(1);
   inputs.create("ImageTable", "image.tab", "Input image table name");
   inputs.create("PSFTable", "psf.tab", "Input psf table name");
   inputs.create("ModelTable", "cleaned.tab", "Output model table name");
   inputs.create("Iters", "100", "Number of iterations", "Int");
   inputs.create("Threshold", "0.2", "Clean Threshold", "Float");
   inputs.create("Gain", "0.2", "Clean Gain", "Float");
   inputs.readArguments(argc, argv);

   String ImageTable = inputs.getString("ImageTable");
   String PSFTable = inputs.getString("PSFTable");
   String ModelTable = inputs.getString("ModelTable");
//
   Int numberIterations  = inputs.getInt("Iters");
   Float threshold  = inputs.getDouble("Threshold");
   Float gain = inputs.getDouble("Gain");
//

  // Make the residual image
  PagedImage<Float> PSF(PSFTable);   // Dirty beam
  PagedImage<Float> imageStep(ImageTable); // Dirty image
  PagedImage<Float> image(imageStep.shape(), imageStep.coordinates(), ModelTable); // Cleaned image
  PtrBlock<PagedImage<Float> * > mask_p;
  mask_p.resize(1);
  mask_p[0] = 0;
//
  Int nx=image.shape()(0);
  Int ny=image.shape()(1);
  Int npol=image.shape()(2);
  Int nchan=image.shape()(3);
  
  AlwaysAssert((npol==1)||(npol==2)||(npol==4), AipsError);
  
  // Loop over all channels. Read only the first plane of the
  // PSF.
  LatticeStepper psfls(PSF.shape(), IPosition(4,nx,ny,1,1),
		       IPosition(4,0,1,3,2));
  RO_LatticeIterator<Float> psfli(PSF,psfls);
  
  // Now read the mask and determine the bounding box
  Int xbeg=nx/4;
  Int ybeg=ny/4;
  
  if(mask_p[0]) {
    AlwaysAssert(mask_p[0]->shape()(0)==nx, AipsError);
    AlwaysAssert(mask_p[0]->shape()(1)==ny, AipsError);
    LatticeStepper mls(mask_p[0]->shape(),
		       IPosition(4, nx, ny, 1, 1),
		       IPosition(4, 0, 1, 3, 2));
    RO_LatticeIterator<Float> maskli(*mask_p[0], mls);
    maskli.reset();
    xbeg=nx-1;
    ybeg=ny-1;
    for (Int iy=0;iy<ny;iy++) {
      for (Int ix=0;ix<nx;ix++) {
	if(maskli.matrixCursor()(ix,iy)>0.000001) {
	  xbeg=min(xbeg,ix);
	  ybeg=min(ybeg,iy);
	}
      }
    }
    // Now have possible BLC. Make sure that we don't go over the
    // edge later
    if(xbeg>nx/2) xbeg=nx/2;
    if(ybeg>ny/2) ybeg=ny/2;
  }
  Int xend=xbeg+nx/2-1;
  Int yend=ybeg+ny/2-1;
  
  // Now we can read the image
  LatticeStepper ls(image.shape(),
		    IPosition(4, nx/2, ny/2, npol, 1), 
		    IPosition(4, 0, 1, 2, 3));
  // Set blc and trc to include only the window to be cleaned.
  ls.subSection(IPosition(4, xbeg, ybeg, 0, 0), 
		IPosition(4, xend, yend, npol-1, nchan-1));
  LatticeIterator<Float> imageli(image, ls);
  LatticeIterator<Float> imageStepli(imageStep, ls);
  
  // Now we read the mask and use only the bounding box
  Array<Float> mask;
  if(mask_p[0]) {
    Int mx=mask_p[0]->shape()(0);
    Int my=mask_p[0]->shape()(1);
    Int mpol=mask_p[0]->shape()(2);
    Int mchan=mask_p[0]->shape()(3);
    AlwaysAssert(mx==nx, AipsError);
    AlwaysAssert(my==ny, AipsError);
    AlwaysAssert(mpol==npol, AipsError);
    Int curX = mx, curY = my;
    if ((mx != 1) && (my != 1)){
      curX = mx/2;
      curY = my/2;
    }
    LatticeStepper mls(mask_p[0]->shape(),
		       IPosition(4, curX, curY, 1, 1), 
		       IPosition(4, 0, 1, 2, 3));
    if ((mx != 1) && (my != 1))
      mls.subSection(IPosition(4, xbeg, ybeg, 0, 0), 
		     IPosition(4, xend, yend, mpol-1, mchan-1));
    
    RO_LatticeIterator<Float> maskli(*mask_p[0], mls);
    maskli.reset();
    if (maskli.cursor().shape().nelements() > 1) mask=maskli.cursor();
  }
  Int chan=0;
  Int rank(0);
  ClarkCleanAlgorithm clarkClean;
  OrderedMap<Int, Int> chanNo(0);
  Bool assigned(casa::applicator.nextAvailProcess(clarkClean, rank));
  Bool allDone(False);
  for (imageStepli.reset(),imageli.reset(),psfli.reset();
       !imageStepli.atEnd();
       imageStepli++,imageli++,psfli++,chan++) {
          // Send the inputs to the casa::applicator
        while (!assigned) {
          // No free processes; wait for any Clark Clean worker
          // process to return.
          rank = casa::applicator.nextProcessDone(clarkClean, allDone);
          // Get the resulting plane and insert in the correct place
          Array<Float> af;
          casa::applicator.get(af);
          image.putSlice(af, IPosition(4, xbeg, ybeg, 0, chanNo(rank)));
          // Assign the next available process
          assigned = casa::applicator.nextAvailProcess(clarkClean, rank);
        }
	casa::applicator.put(imageStepli.cursor());
	casa::applicator.put(psfli.matrixCursor());
	casa::applicator.put(mask);
	casa::applicator.put(gain);
	casa::applicator.put(threshold);
	casa::applicator.put(numberIterations);
	casa::applicator.put(chan);
	casa::applicator.put(nchan);

        // Record the assignment of channel to process rank
        chanNo.define(rank, chan);

		  // Run the alogrithm
	casa::applicator.apply(clarkClean);
  }
     // Wait till the applicator finishes.
  // Wait for all outstanding processes to return
  rank = casa::applicator.nextProcessDone(clarkClean, allDone);
  while (!allDone) {
    // Get the resulting plane and insert in the correct place
    Array<Float> af;
    casa::applicator.get(af);
    image.putSlice(af, IPosition(4, xbeg, ybeg, 0, chanNo(rank)));
    // Wait for the next process to complete
    rank = casa::applicator.nextProcessDone(clarkClean, allDone);
  };

  return;
}
