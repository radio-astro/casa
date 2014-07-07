/*
//#MSBin functionality on the command line
//# Copyright (C) 2014
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
 *  Created on: Jan 16, 2014
 *      Author: kgolap
 */
#include <casa/Quanta/QuantumHolder.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <measures/Measures/MDirection.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <mstransform/MSTransform/MSTransformDataHandler.h>
#include <mstransform/MSTransform/MSUVBin.h>
#include <tables/Tables/TableIter.h>
#include <casa/Inputs/Input.h>
#include <casa/namespace.h>

using namespace std;

int main(int argc, char **argv) {
  Input inp;
  inp.version("2014/06/18 by CM MLLN, HTST");
  // Title of CM  i.e Code Monkey is
  //Master Lead Lion Ninja and Honcho Tiger Samurai Team 
  inp.create("vis", "ngc5921.ms", "MS to be binned");
  inp.create("outvis", "OutMS.ms", "Output MS");
  inp.create("field", "*", "fields to use from input ms");
  inp.create("spw", "*", "spw selection to use on input ms");
  inp.create("phasecenter", "e.g J2000 19h20m00 -5d00m00 or fieldid", "phasecenter of gridded vis");
  inp.create("nx", "200", "number of pixel along x direction" );
  inp.create("ny", "200", "number of pixel along y direction" );
  inp.create("cellx", "1arcsec", "x-increment");
  inp.create("celly", "1arcsec", "y-increment");
  inp.create("ncorr", "1", "number of pol-correlations 1 or 2 or 4");
  inp.create("nchan","1", "number of output channel");
  inp.create("fstart", "1.420GHz", "frequency of first channel in LSRK");
  inp.create("fstep", "1KHz", "channel width");
  inp.create("fdb", "False", "Force to go through disk and not use ram");
  inp.readArguments(argc, argv);
  String msname=inp.getString("vis");
  String outMS=inp.getString("outvis");
  String elphcen=inp.getString("phasecenter");
  MDirection phaseCenter;
  if(!MSUVBin::String2MDirection(elphcen, phaseCenter, msname)){
    cerr << "Could not interprete phasecenter param : "+elphcen << endl;
    return -1;
  }
  QuantumHolder qh;
  Int nx, ny;
  nx=inp.getInt("nx");
  ny=inp.getInt("ny");
  String err, tmpstr;
  tmpstr=inp.getString("cellx");
  if(!qh.fromString(err, tmpstr)){
    cerr << "could not read cellx:  " << err << endl;
    return -1;
  }
  Quantity cellx=qh.asQuantity();
  tmpstr=inp.getString("celly");
  if(!qh.fromString(err, tmpstr)){
    cerr << "could not read celly:  " << err << endl;
    return -1;
  }
  Quantity celly=qh.asQuantity();
  Int ncorr=inp.getInt("ncorr");
  Int nchan=inp.getInt("nchan");
  tmpstr=inp.getString("fstart");
  if(!qh.fromString(err, tmpstr)){
    cerr << "could not read fstart:  " << err << endl;
    return -1;
  }
  Quantity fstart=qh.asQuantity();
  tmpstr=inp.getString("fstep");
  if(!qh.fromString(err, tmpstr)){
    cerr << "could not read fstep:  " << err << endl;
    return -1;
  }
  Quantity fstep=qh.asQuantity();
  String field=inp.getString("field");
  String spw=inp.getString("spw");
  
  Bool forceDisk=inp.getBool("fdb");
  
  MSUVBin binner(phaseCenter, nx,
		 ny, nchan, ncorr, cellx, celly, fstart, fstep);
  binner.selectData(msname, field, spw);
  binner.setOutputMS(outMS);
  binner.fillOutputMS(forceDisk);
  

  return 0;
} 
