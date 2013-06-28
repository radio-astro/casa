/*
 * tSynthesisImager.cc
 *SynthesisImager.cc: test of SynthesisImager
//# Copyright (C) 2013
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
#include <lattices/Lattices/LatticeConcat.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageConcat.h>
#include <casa/namespace.h>
#include <images/Images/TempImage.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <ms/MeasurementSets/MSSourceIndex.h>

int main(int argc, char **argv)
{
  using namespace std;
  using namespace casa;
  try{



    String msname=String(argv[1]);

    //MeasurementSet ms( "/home/rrusk/testing/3C273XC1.ms", Table::Update );
    cout << "--MeasurementSet created." << endl;

    SynthesisImager* imgr = new SynthesisImager();
    imgr->selectData(msname, /*spw=*/"0", /*field=*/"0", /*taql=*/"", /*antenna=*/"",  /*uvdist*/"", /*scan*/"", /*obs*/"",
    		/*timestr*/"", /*usescratch*/False, /*readonly*/False);
    cout <<"--Imager created for MeasurementSet object. " << endl;
    MeasurementSet tab(msname);
    int nx = 100;
    int ny = 100;
    Quantity cellx( 30, "arcsec" );
    Quantity celly( 30, "arcsec" );
    Vector<Int> spwids(2);
    String stokes="I";
    Int nchan=1;
    MDirection phasecenter=MSFieldColumns(tab.field()).phaseDirMeas(0,0.0);
    Quantity freqBeg=MSSpWindowColumns(tab.spectralWindow()).chanFreqQuant()(0)(IPosition(1,0));
    Int ndataChan=MSSpWindowColumns(tab.spectralWindow()).numChan()(0);
    Quantity freqWidth=MSSpWindowColumns(tab.spectralWindow()).chanFreqQuant()(0)(IPosition(1,ndataChan-1));
    freqWidth-=freqBeg;
    imgr->defineImage(/*imagename*/"test_image", nx, ny, cellx, celly,
			   stokes,phasecenter, nchan,
			   freqBeg, freqWidth, Vector<Quantity>(1,Quantity(1.420, "GHz")));
			   /*const Int facets=1,
			   const String& ftmachine="GridFT",
			   const Projection& projection=Projection::SIN,
			   const Quantity& distance=Quantity(0,"m"),
			   const MFrequency::Types& freqFrame=MFrequency::LSRK,
			   const Bool trackSource=False, const MDirection&
			   trackDir=MDirection(Quantity(0.0, "deg"),
					       Quantity(90.0, "deg")))
    */
    cerr << "nx=" << nx << " ny=" << ny
       << " cellx='" << cellx.getValue() << cellx.getUnit()
       << "' celly='" << celly.getValue() << celly.getUnit()
       << "' spwids=" << 0
       << " field=" <<   0 << endl;
    imgr->weight("natural");
    Record rec;
    imgr->executeMajorCycle(rec);
    CountedPtr<SIImageStore> images=imgr->imageStore(0);
    LatticeExprNode LEN = max( *(images->residual()) );
    cerr << "Max of residual=" << LEN.getFloat() << endl;




  }catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
  }
};
