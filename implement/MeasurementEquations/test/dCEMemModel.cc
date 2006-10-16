//# dCEMemModel.cc:  this test CEMemModel
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
#include <lattices/Lattices/LCSlicer.h>
#include <images/Images/ImageRegion.h>
#include <images/Images/SubImage.h>
#include <lattices/Lattices/PagedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayLogical.h>
#include <synthesis/MeasurementEquations/LatConvEquation.h>
#include <synthesis/MeasurementEquations/CEMemModel.h>
#include <casa/BasicMath/Random.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <lattices/Lattices/LatticeConvolver.h>
#include <lattices/Lattices/TempLattice.h>
#include <casa/Exceptions/Error.h>


 
#include <casa/namespace.h>
int main()
{


  cout << "Tests MEM with a 2-D image deconvolution" << endl;
  cout << "--------------------------------------" << endl;
  try {

    /* 
    PagedImage<Float> dirty("3C273XC1.ms.dirty");
    PagedImage<Float> prior("3C273XC1.ms.mem.prior");
    PagedImage<Float> psf("3C273XC1.ms.psf");
    */

    // /*

    PagedImage<Float> dirty0("M31A.DIRTY");
    PagedImage<Float> psf("M31A.PSF");
    PagedImage<Float> prior0("M31A.BIAS");
    uInt dim = dirty0.ndim();
    IPosition blc(dim, 0);
    IPosition trc(dim, 0);
    blc(0) = dirty0.shape()(0) / 4;
    blc(1) = dirty0.shape()(1) / 4;
    trc(0) = dirty0.shape()(0) * 3 / 4 - 1;
    trc(1) = dirty0.shape()(1) * 3 / 4 - 1;    
    cout << "BLC: " << blc << "   TRC: " << trc << endl;
    LCSlicer quarter(blc, trc);
    SubImage<Float> prior(prior0, ImageRegion(quarter), True);
    SubImage<Float> dirty(dirty0, ImageRegion(quarter), True);
    //  */


    PagedImage<Float> model(dirty.shape(), dirty.coordinates(), "TEST.mem");
    model.set(0.1);

    cout << "dirty shape = " << dirty.shape() << endl;
    cout << "psf shape = " << psf.shape() << endl;
    cout << "model shape = " << model.shape() << endl;
    LatConvEquation convEqn(psf, dirty); 
    
    EntropyI  ent;
    cout << "Test of Information Entropy" << endl;

    CEMemModel memImage(ent, model, prior, 20, 0.01, 20.0 );
    memImage.solve(convEqn);

    dirty0.table().unlock();
    psf.table().unlock();
    model.table().unlock();


  } catch (AipsError x) {
    cout << x.getMesg() << endl;
  } 

  exit(0);

  
}
