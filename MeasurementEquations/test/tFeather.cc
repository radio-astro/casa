//# tCEMemModel.cc:  this test CEMemModel
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
#include <casa/Arrays/ArrayLogical.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <lattices/Lattices/TempLattice.h>
#include <synthesis/MeasurementEquations/Feather.h> 
 
#include <casa/namespace.h>
int main(int argc, char **argv)
{
  cout << "Tests Feather function" << endl;
  cout << "--------------------------------------" << endl;
  try {
    if (argc<3) {
      cout <<"Usage: tFeather lowResImag highResImage"<<endl;
      exit(1);
    }
    PagedImage<Float> lowImage(argv[1]);
    PagedImage<Float> highImage(argv[2]);
    //Feather plume(lowImage, highImage);
    Feather plume;
    plume.setINTImage(highImage);
    plume.setSDImage(lowImage);
    plume.setSDScale(0.999);
    Vector<Float> ux, xamp, uy, yamp;
    plume.getFTCutSDImage(ux, xamp, uy, yamp);
    cerr << "SD: ux " << ux << endl;
    cerr << "xamp " << xamp << endl;
    plume.getFTCutIntImage(ux, xamp, uy, yamp);
    cerr << "INT ux " << ux << endl;
    cerr << "xamp " << xamp << endl;
    plume.getFeatherSD(ux, xamp, uy, yamp);
    cerr << "FeathSD ux " << ux << endl;
    cerr << "xamp " << xamp << endl;
    plume.getFeatherINT(ux, xamp, uy, yamp);
    cerr << "FeathINT ux " << ux << endl;
    cerr << "xamp " << xamp << endl;
    plume.getFeatheredCutINT(ux, xamp, uy, yamp);
    cerr << "FeatheredINT ux " << ux << endl;
    cerr << "xamp " << xamp << endl;
    plume.saveFeatheredImage("testFeather.image");

  } catch (AipsError x) {
    cout << x.getMesg() << endl;
  } 

  exit(0);

  

}
