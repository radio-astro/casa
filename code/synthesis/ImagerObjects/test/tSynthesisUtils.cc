/*
 * tSynthesisUtils.cc
 *demo of SynthesisUtil functionality
//# Copyright (C) 2013-2014
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
 *  Created on: Jun 27, 2013
 *      Author: kgolap
 */





#include <casa/iostream.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MRadialVelocity.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Logging/LogIO.h>
#include <synthesis/ImagerObjects/SynthesisImager.h>
#include <synthesis/ImagerObjects/SIImageStore.h>
#include <imageanalysis/Utilities/SpectralImageUtil.h>
#include <lattices/Lattices/LatticeConcat.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageConcat.h>
#include <images/Images/SubImage.h>
#include <casa/namespace.h>
#include <images/Images/TempImage.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <ms/MeasurementSets/MSSourceIndex.h>
#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>
#include <measures/Measures/VelocityMachine.h>

int main(int argc, char **argv)
{
  using namespace std;
  using namespace casa;
  try{



    String msname(argv[1]);
    Record msrec;
    msrec.define("msname", msname);
    msrec.define("field", "0");
    msrec.define("spw", "0:5~51");
    Record parsrec;
    parsrec.defineRecord("ms0", msrec);
    //Vector<Double> start(20);
    //Vector<Double> end(20);
    //start(0)=1.412787e9;
    //end(0)=1.412787e9+2.5e4;
    //for (Int k=1; k < 20; ++k){
    //  start(k)=start(k-1)+2.5e4;
    //  end(k)=end(k-1)+2.5e4;
    //}
	  
    SynthesisParamsImage impars;
    Vector<Int> ims(2);ims[0]=1000; ims[1]=1000;
    impars.imsize=ims;
    Vector<Quantity> cells(2); cells[0]=Quantity(1, "arcsec"), cells[1]=Quantity(2,"arcsec");
    impars.cellsize=cells;
    impars.stokes="I";
    impars.phaseCenter=MDirection(Quantity(20.0, "deg"), Quantity(40.0, "deg"), MDirection::J2000);
    impars.nchan=1000;
    //impars.freqStart=freqStart;
    //impars.freqStep=freqStep;
    impars.restFreq=Quantity(1.420, "GHz");
    //impars.nTaylorTerms=nTaylorTerms;
    //impars.refFreq=refFreq;
    impars.projection=Projection::SIN;
    impars.freqFrame=MFrequency::LSRK;
    //impars.overwrite=overwrite;
    //impars.startModel=startmodel;
    impars.velStart=Quantity(1320.0, "km/s");
    impars.velStep=Quantity(0.3, "km/s");
    impars.veltype="OPTICAL";

    MeasurementSet elms(msname, Table::Old);
    Block<Int> sort;
    ROVisibilityIterator vi(elms, sort);

    CoordinateSystem cs=impars.buildCoordinateSystem(&vi);
    
    ////////////////
    SpectralCoordinate spCoord=cs.spectralCoordinate(2);
    cerr << "Returned tabular values " << spCoord.worldValues() << endl;
    VelocityMachine vm(MFrequency::Ref(MFrequency::LSRK),  Unit("Hz"),  MVFrequency(Quantity(1.420, "GHz")), MDoppler::Ref(MDoppler::OPTICAL), Unit("km/s"));
    Vector<Double> worldFreq(impars.nchan);
    for (Int k=0; k< impars.nchan; ++k)
      worldFreq[k]=vm.makeFrequency(impars.velStart.getValue() + impars.velStep.getValue()*Double(k)).getValue();
    //cerr << "should be these values " << worldFreq << endl;
    

    SpectralCoordinate newSpec(impars.freqFrame,  worldFreq,  spCoord.restFrequency());
    // Fixing the coordinateSystem for OPTICAL (see cas-6658)
    cs.replaceCoordinate(newSpec, 2);






    ///////////////////
  
  

    Vector<CoordinateSystem> oCs;
    Vector<Int> oNchan;
    
    Record rec=SynthesisUtilMethods::cubeDataImagePartition(parsrec, cs, 10, impars.nchan, oCs, oNchan);
    cerr << rec << endl;


    //	  cerr << SynthesisUtilMethods::cubeDataPartition(parsrec, start, end, MFrequency::LSRK) << endl;
    
	  



  }catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
  }
  return 0;
};
