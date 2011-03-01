//# tALMAAperture.cc: This program tests the ALMAAperture class
//# Copyright (C) 1998,1999,2000
//# Associated Universities, Inc. Washington DC, USA.
//# Copyright (C) 2011 ESO (in the framework of the ALMA collaboration)
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

//# Includes
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Constants.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures.h>
#include <coordinates/Coordinates.h>
#include <casa/Arrays/Matrix.h>

#include <images/Images/PagedImage.h>
#include <images/Regions/ImageRegion.h>

#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <casa/Arrays/IPosition.h>
#include <lattices/Lattices/TiledShape.h>
#include <lattices/Lattices/LCRegion.h>
#include <lattices/Lattices/LCSlicer.h>
#include <casa/Arrays/Slicer.h>

#include <components/ComponentModels/ComponentType.h>
#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/PointShape.h>
#include <components/ComponentModels/SpectralIndex.h>
#include <components/ComponentModels/SkyComponent.h>

#include <synthesis/MeasurementComponents/SkyJones.h>
#include <synthesis/MeasurementComponents/PBMath.h>
#include <casa/BasicSL/String.h>

#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>

#include <synthesis/MeasurementComponents/ALMAAperture.h>


#include <casa/namespace.h>
int main()
{
  try {

    // Form coordinate systems and test images


    CoordinateSystem coordsys;
    CoordinateSystem coordsys3;
    {
      Matrix<Double> xform(2,2);                                    
      xform = 0.0; xform.diagonal() = 1.0;                          
      DirectionCoordinate dirCoords(MDirection::J2000,                  
				    Projection(Projection::SIN),        
				    135*C::pi/180.0, 60*C::pi/180.0,    
				    -1*C::pi/180.0/3600.0, 1*C::pi/180.0/3600.0,        
				    xform,                              
				    0, 0);  
      // reference position is in lower left corner; easier for beam inspection!
      
      Vector<String> units(2); units = "deg";                       
      dirCoords.setWorldAxisUnits(units);                               
      

      // StokesCoordinate
      Vector<Int> iquv(1);                                         
      iquv(0) = Stokes::I;
      StokesCoordinate stokesCoordsBad(iquv);	
      iquv(0) = Stokes::XX;
      StokesCoordinate stokesCoordsGood(iquv);	
      
      // SpectralCoordinate
      SpectralCoordinate spectralCoords(MFrequency::TOPO,           
					2000 * 1.0E+6,                 
					20 * 1.0E+3,                   
					0,                             
					2000.40575 * 1.0E+6);          
      units.resize(1);
      units = "Hz";
      spectralCoords.setWorldAxisUnits(units);
      
      // SpectralCoordinate 3
      SpectralCoordinate spectralCoords3(MFrequency::TOPO,           
					 43 * 1.0E+9,                 
					 20 * 1.0E+3,                   
					 0,                             
					 43 * 1.0E+9);          
      units.resize(1);
      units = "Hz";
      spectralCoords3.setWorldAxisUnits(units);
      
      // CoordinateSystem
      coordsys.addCoordinate(dirCoords);
      coordsys.addCoordinate(stokesCoordsBad);
      coordsys.addCoordinate(spectralCoords);
      
      coordsys3.addCoordinate(dirCoords);
      coordsys3.addCoordinate(stokesCoordsGood);
      coordsys3.addCoordinate(spectralCoords3);      
    }
    
    String name("tab1");
    TiledShape ts(IPosition(4,128,128,1,1));

    PagedImage<Complex> im1(ts, coordsys, name);
    PagedImage<Float> im2(ts, coordsys, "tab2");
    PagedImage<Float> im3(ts, coordsys3, "tab3");
    PagedImage<Float> im4(ts, coordsys3, "tab4");

    im1.set(Complex(1.0,1.0));
    im2.set(0.0);
    im3.set(1.0);
    im4.set(0.0);
    
    ///////////////////////////////////////////

    { // begin tests

      cout << "new ALMAAperture" << endl;
      ALMAAperture aa;
      
      cout << aa.name() << endl;
      
      cout << (Int)aa.antTypeFromName("DV01") << endl;
      cout << (Int)aa.antTypeFromName("DA01") << endl;
      cout << (Int)aa.antTypeFromName("PM01") << endl;
      cout << (Int)aa.antTypeFromName("CM01") << endl;
      cout << (Int)aa.antTypeFromName("XY01") << endl;
      cout << endl;   
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV01"),aa.antTypeFromName("DV02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV01"),aa.antTypeFromName("DA02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV01"),aa.antTypeFromName("PM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV01"),aa.antTypeFromName("CM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV01"),aa.antTypeFromName("XY02")) << endl;
      cout << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DA01"),aa.antTypeFromName("DV02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DA01"),aa.antTypeFromName("DA02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DA01"),aa.antTypeFromName("PM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DA01"),aa.antTypeFromName("CM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DA01"),aa.antTypeFromName("XY02")) << endl;
      cout << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("PM01"),aa.antTypeFromName("DV02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("PM01"),aa.antTypeFromName("DA02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("PM01"),aa.antTypeFromName("PM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("PM01"),aa.antTypeFromName("CM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("PM01"),aa.antTypeFromName("XY02")) << endl;
      cout << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("DV02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("DA02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("PM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("CM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("XY02")) << endl;
      cout << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("XY01"),aa.antTypeFromName("DV02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("XY01"),aa.antTypeFromName("DA02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("XY01"),aa.antTypeFromName("PM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("XY01"),aa.antTypeFromName("CM02")) << endl;
      cout << aa.cFKeyFromAntennaTypes(aa.antTypeFromName("XY01"),aa.antTypeFromName("XY02")) << endl;
      cout << endl;
      Int c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV01"),aa.antTypeFromName("DV02"));
      Vector<ALMAAntennaType> a;
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      cout << (Int)a(0) << " " << (Int)a(1) << " (0,0)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("CM02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      cout << (Int)a(0) << " " << (Int)a(1) << " (3,3)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DA01"),aa.antTypeFromName("DV02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      cout << (Int)a(0) << " " << (Int)a(1) << " (0,1)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("PM02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      cout << (Int)a(0) << " " << (Int)a(1) << " (2,3)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV"),aa.antTypeFromName("XY02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      cout << (Int)a(0) << " " << (Int)a(1) << " (-1,0)" << endl;
      
      cout << endl;
      cout << ALMAAperture::antTypeStrFromType(aa.antTypeFromName("XY01")) << endl;
      cout << ALMAAperture::antTypeStrFromType(aa.antTypeFromName("DV01")) << endl;
      cout << ALMAAperture::antTypeStrFromType(aa.antTypeFromName("DA02")) << endl;
      cout << ALMAAperture::antTypeStrFromType(aa.antTypeFromName("PM01")) << endl;
      cout << ALMAAperture::antTypeStrFromType(aa.antTypeFromName("CM01")) << endl;

      ////////////////////////////////////////////////
      
      {
	const char *sep=" ";
	char *aipsPath = strtok(getenv("CASAPATH"),sep);
	if (aipsPath == NULL)
	  throw(AipsError("CASAPATH not found."));
	
	String msFileName(aipsPath);
	msFileName = msFileName + "/data/regression/exportasdm/input/uid___X02_X56142_X1.ms";
	
	cout << "Reading " << msFileName << endl;
	
	MS ms(msFileName, Table::Old);
	
	Block<int> sort(4);
	sort[2] = MS::FIELD_ID;
	sort[3] = MS::ARRAY_ID;
	sort[1] = MS::DATA_DESC_ID;
	sort[0] = MS::TIME;
	
	ROVisibilityIterator vi(ms, sort, 0.);
	
	VisBuffer vb(vi);
	
	cout << "new ALMAAperture" << endl;
	ALMAAperture apB;
	
	Int count = 0;
	
	for(vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	  cout << "next chunk" << endl;
	  for(vi.origin();vi.more();vi++) {
	    Int nUnique = 0;
	    Vector<Int> map = apB.vbRow2CFKeyMap(vb, nUnique);
	    cout << "map " << map << endl;
	    cout << "Unique " << nUnique << endl;
	  }
	  count++;
	  // test reset
	  if(count==10) apB.resetAntTypeMap();
	  // ten rounds is enough
	  if(count>10) break;
	}
	
	cout << "\nSecond tour through the MS, testing getVisParams and applySky" << endl;
	count=0;
	
	for(vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	  cout << "next chunk" << endl;
	  for(vi.origin();vi.more();vi++) {
	    cout << "band id " << apB.getVisParams(vb) << endl;
	    cout << "ant type map " << apB.antTypeMap(vb) << endl;
	    Vector<ALMAAntennaType> aTs = apB.antTypeList(vb);
	    cout << "ant type list " << aTs << endl;
	    Int cfKey = ALMAAperture::cFKeyFromAntennaTypes(aTs(0), aTs(0));

	    if(vb.spectralWindow()<4){ // i.e. not a WVR SPW for this dataset
	      try{
		apB.applySky(im1, vb, True, cfKey);
	      } catch (AipsError x) {
		cout << "Caught expected error: " << x.getMesg() << endl;
	      } 
	      apB.applySky(im3, vb, True, cfKey);
	    }
	    else{
	      try{
		apB.applySky(im1, vb, True, cfKey);
	      } catch (AipsError x) {
		cout << "Caught expected error: " << x.getMesg() << endl;
	      } 
	    }
	  }
	  count++;
	  // test reset
	  if(count==10) apB.resetAntTypeMap();
	  // ten rounds is enough
	  if(count>10) break;
	}
      }

      aa.destroyAntResp();      
      cout << "new ALMAAperture, should initialise the response table again" << endl;
      ALMAAperture apC;
      cout << apC.name() << endl;
      
    } // end tests

  } catch (AipsError x) {
    cout << "Caught Error: " << x.getMesg() << endl;
    exit(1);
  } 

  cout << "OK" << endl;
  exit(0);
}



