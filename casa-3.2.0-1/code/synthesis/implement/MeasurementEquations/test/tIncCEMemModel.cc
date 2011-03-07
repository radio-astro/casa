//# tIncCEMemModel.cc:  this test IncCEMemModel
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
 
#include <casa/iostream.h>
#include <casa/aips.h>
#include <images/Images/PagedImage.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayLogical.h>
#include <synthesis/MeasurementEquations/LatConvEquation.h>
#include <synthesis/MeasurementEquations/IncCEMemModel.h>
#include <synthesis/MeasurementEquations/CEMemModel.h>
#include <casa/BasicMath/Random.h>
#include <casa/BasicMath/Math.h>
#include <casa/Exceptions/Error.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeConvolver.h>
#include <lattices/Lattices/TempLattice.h>
 
 
#include <casa/namespace.h>
int main()
{


  cout << "Tests Incremental MEM with a simple 1-D deconvolution" << endl;
  cout << "--------------------------------------" << endl;
  try {
 
    Int npsf = 256;
    Int ndirty = 128;
    const IPosition arrayShape(1, ndirty);
    const IPosition psfShape(1, npsf);
    
    TempLattice<Float> psf(psfShape);
    psf.set(0.0);
    {
      // make (almost) Gaussian beam to test deconvolution
      LatticeIterator<Float> 
	li(psf, LatticeStepper(psf.shape(), psf.niceCursorShape()));
      li.reset();    

      Int ii;
      for (ii=0; ii< npsf; ii++) {
	li.rwVectorCursor()(ii) = exp( - square( ((Float)(ii - npsf/2)) / 2.0 ) );
      }
    }

    TempLattice<Float> prior(arrayShape);
    prior.set(0.1);
    TempLattice<Float> truth(arrayShape);
    truth.set(0.0);
    {
      // make the "truth" image
      LatticeIterator<Float> 
	li(truth, LatticeStepper(truth.shape(), truth.niceCursorShape()));
      li.reset(); 

      li.rwVectorCursor()(41) =         1;
      li.rwVectorCursor()(43) =         1;
      li.rwVectorCursor()(46) =         1;
      li.rwVectorCursor()(50) =         3;
      li.rwVectorCursor()(58) =       0.2;
      li.rwVectorCursor()(59) =       0.5;
      li.rwVectorCursor()(60) =         1;
      li.rwVectorCursor()(61) =       0.7;
      li.rwVectorCursor()(62) =       0.5;
      li.rwVectorCursor()(63) =      0.25;
      li.rwVectorCursor()(64) =      0.25;
      li.rwVectorCursor()(65) =       0.3;
      li.rwVectorCursor()(66) =       0.4;
      li.rwVectorCursor()(67) =       0.5;
      li.rwVectorCursor()(68) =       0.4;
      li.rwVectorCursor()(69) =       0.3;
      li.rwVectorCursor()(70) =       0.2;
      li.rwVectorCursor()(71) =      0.15;
      li.rwVectorCursor()(72) =       0.1;
      li.rwVectorCursor()(73) =      0.05;
      li.rwVectorCursor()(74) =      0.01;
    }

    // Make the dirty image
    TempLattice<Float> dirty(arrayShape);
    dirty.set(0.0);
    LatticeConvolver<Float> latConvolver(psf, dirty.shape());
    latConvolver.linear(dirty, truth);

    TempLattice<Float> model(arrayShape);
    model.set(0.0);
    TempLattice<Float> delta(arrayShape);
    delta.set(0.0);
    LatConvEquation convEqn(psf, dirty); 
    
    EntropyI  ent;
    IncEntropyI  incEnt;
    cout << "Test of Information Entropy" << endl;

    // this one tests with just a default level
    {
      IncCEMemModel memImage(incEnt, model, delta, 400, 0.001, 0.2 );
      memImage.solve(convEqn);
    }

    {
      CEMemModel memImage(ent, model, 400, 0.001, 0.2 );
      memImage.solve(convEqn);
    }

    {
      RO_LatticeIterator<Float> 
	dli(dirty, LatticeStepper(dirty.shape(), dirty.niceCursorShape()));
      RO_LatticeIterator<Float> 
	mli(model, LatticeStepper(model.shape(), model.niceCursorShape()));
      RO_LatticeIterator<Float> 
	dmli(delta, LatticeStepper(delta.shape(), delta.niceCursorShape()));
      RO_LatticeIterator<Float> 
	tli(truth, LatticeStepper(truth.shape(), truth.niceCursorShape()));
      dli.reset(); 
      mli.reset(); 
      dmli.reset(); 
      tli.reset(); 
      uInt i;
      cout << "Deconvolution Results: " << endl;
      cout << "Pix   Dirty     Deconv1     Deconv2     Truth: " << endl;
      for (i=30; i< 80; i++) {
	cout << i 
	     << "  " <<  dli.vectorCursor()(i) 
	     << "  " <<  mli.vectorCursor()(i) 
	     << "  " <<  dmli.vectorCursor()(i) 
	     << "  " <<  tli.vectorCursor()(i) << endl;
      }
    }
  } catch (AipsError x) {
    cout << x.getMesg() << endl;
  }

  exit(0);

  

}
