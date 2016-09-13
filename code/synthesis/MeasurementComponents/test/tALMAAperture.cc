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
#include <lattices/LRegions/LCRegion.h>
#include <lattices/LRegions/LCSlicer.h>
#include <casa/Arrays/Slicer.h>

#include <components/ComponentModels/ComponentType.h>
#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/PointShape.h>
#include <components/ComponentModels/SpectralIndex.h>
#include <components/ComponentModels/SkyComponent.h>

#include <synthesis/TransformMachines/SkyJones.h>
#include <synthesis/TransformMachines/PBMath.h>
#include <casa/BasicSL/String.h>

#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>

#include <synthesis/TransformMachines/ALMAAperture.h>


#include <casa/namespace.h>
int main()
{
  try {

    // Form coordinate systems and test images


    CoordinateSystem coordsys;
    CoordinateSystem coordsys3;
    CoordinateSystem coordsys3Big;
    CoordinateSystem coordsys3BigHiRes;
    CoordinateSystem coordsys3Small;
    {
      Matrix<Double> xform(2,2);                                    
      xform = 0.0; xform.diagonal() = 1.0;                          
      DirectionCoordinate dirCoords(MDirection::J2000,                  
				    Projection(Projection::SIN),        
				    135*C::pi/180.0, 60*C::pi/180.0,    
				    -1.*C::pi/180.0/3600.0, 1.*C::pi/180.0/3600.0,        
				    xform,                              
				    63.5, 63.5);  // (128-1)/2.
      DirectionCoordinate dirCoordsBig(MDirection::J2000,                  
				       Projection(Projection::SIN),        
				       135*C::pi/180.0, 60*C::pi/180.0,    
				       -5*C::pi/180.0/3600.0, 5*C::pi/180.0/3600.0,        
				       xform,                              
				       63.5, 63.5);  
      DirectionCoordinate dirCoordsBig2(MDirection::J2000,                  
				       Projection(Projection::SIN),        
				       135*C::pi/180.0, 60*C::pi/180.0,    
				       -10*C::pi/180.0/3600.0, 10*C::pi/180.0/3600.0,        
				       xform,                              
				       63.5, 63.5);  
      DirectionCoordinate dirCoordsSmall(MDirection::J2000,                  
					 Projection(Projection::SIN),        
					 135*C::pi/180.0, 60*C::pi/180.0,    
					 -0.5*C::pi/180.0/3600.0, 0.5*C::pi/180.0/3600.0,        
					 xform,                              
					 63.5, 63.5);  

      Vector<String> units(2); 
      //units = "deg";                       
      //dirCoords.setWorldAxisUnits(units);                               
      

      // StokesCoordinate
      Vector<Int> iquv(1);                                         
      iquv(0) = Stokes::I;
      StokesCoordinate stokesCoordsBad(iquv);	
      Vector<Int> stoks(4);
      stoks(0) = Stokes::XX;
      stoks(1) = Stokes::XY;
      stoks(2) = Stokes::YX;
      stoks(3) = Stokes::YY;
      StokesCoordinate stokesCoordsGood(stoks);	
      
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
					 100. * 1.0E+9,                 
					 20 * 1.0E+3,                   
					 0,                             
					 100. * 1.0E+9);          
      units.resize(1);
      units = "Hz";
      spectralCoords3.setWorldAxisUnits(units);
      
      ObsInfo myObsInfo;
      myObsInfo.setTelescope("ALMA");

      // CoordinateSystem
      coordsys.addCoordinate(dirCoords);
      coordsys.addCoordinate(stokesCoordsBad);
      coordsys.addCoordinate(spectralCoords);
      coordsys.setObsInfo(myObsInfo);
      
      coordsys3.addCoordinate(dirCoords);
      coordsys3.addCoordinate(stokesCoordsGood);
      coordsys3.addCoordinate(spectralCoords3);      
      coordsys3.setObsInfo(myObsInfo);

      coordsys3Big.addCoordinate(dirCoordsBig);
      coordsys3Big.addCoordinate(stokesCoordsGood);
      coordsys3Big.addCoordinate(spectralCoords3);      
      coordsys3Big.setObsInfo(myObsInfo);

      coordsys3BigHiRes.addCoordinate(dirCoordsBig2);
      coordsys3BigHiRes.addCoordinate(stokesCoordsGood);
      coordsys3BigHiRes.addCoordinate(spectralCoords3);      
      coordsys3BigHiRes.setObsInfo(myObsInfo);

      coordsys3Small.addCoordinate(dirCoordsSmall);
      coordsys3Small.addCoordinate(stokesCoordsGood);
      coordsys3Small.addCoordinate(spectralCoords3);      
      coordsys3Small.setObsInfo(myObsInfo);
    }
    
    String name("tab1");
    TiledShape ts(IPosition(4,128,128,1,1));
    TiledShape ts2(IPosition(4,128,128,4,1));
    TiledShape ts3(IPosition(4,10*128,10*128,4,1));

    PagedImage<Complex> im1(ts, coordsys, name);
    PagedImage<Float> im2(ts, coordsys, "tab2");
    PagedImage<Complex> im3(ts2, coordsys3, "tab3");
    PagedImage<Float> im4(ts2, coordsys3, "tab4");
    PagedImage<Complex> im5(ts3, coordsys3, "tab5");

    im1.set(Complex(1.0,0.0));
    im2.set(0.0);
    im3.set(Complex(1.0,0.0));
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
      AlwaysAssert((Int)aa.antTypeFromName("XY01")==0,AipsError);
      AlwaysAssert((Int)aa.antTypeFromName("DA02")==1,AipsError);
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
      AlwaysAssert(aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV01"),aa.antTypeFromName("DA02"))==30002, AipsError);
      AlwaysAssert(aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DA01"),aa.antTypeFromName("XY02"))==20001, AipsError);
      cout << endl;
      Int c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV01"),aa.antTypeFromName("DV02"));
      Vector<ALMAAntennaType> a;
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      cout << (Int)a(0) << " ";
      AlwaysAssert(a.nelements()==1, AipsError);
      AlwaysAssert(a(0)==2, AipsError);

      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("CM02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      AlwaysAssert(a.nelements()==1, AipsError);
      AlwaysAssert(a(0)==3, AipsError);

      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DA01"),aa.antTypeFromName("DV02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      AlwaysAssert(a.nelements()==2, AipsError);
      AlwaysAssert(a(0)==1, AipsError);
      AlwaysAssert(a(1)==2, AipsError);
      cout << (Int)a(0) << " " << (Int)a(1) << " (1,2)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV02"),aa.antTypeFromName("DA01"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      AlwaysAssert(a.nelements()==2, AipsError);
      AlwaysAssert(a(0)==1, AipsError);
      AlwaysAssert(a(1)==2, AipsError);
      cout << (Int)a(0) << " " << (Int)a(1) << " (1,2)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("CM01"),aa.antTypeFromName("PM02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      AlwaysAssert(a.nelements()==2, AipsError);
      AlwaysAssert(a(0)==3, AipsError);
      AlwaysAssert(a(1)==4, AipsError);
      cout << (Int)a(0) << " " << (Int)a(1) << " (3,4)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("PM01"),aa.antTypeFromName("CM02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      AlwaysAssert(a.nelements()==2, AipsError);
      AlwaysAssert(a(0)==3, AipsError);
      AlwaysAssert(a(1)==4, AipsError);
      cout << (Int)a(0) << " " << (Int)a(1) << " (3,4)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("XY"),aa.antTypeFromName("DV02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      AlwaysAssert(a.nelements()==2, AipsError);
      AlwaysAssert(a(0)==0, AipsError);
      AlwaysAssert(a(1)==2, AipsError);
      cout << (Int)a(0) << " " << (Int)a(1) << " (0,2)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("DV"),aa.antTypeFromName("XY02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      AlwaysAssert(a.nelements()==2, AipsError);
      AlwaysAssert(a(0)==0, AipsError);
      AlwaysAssert(a(1)==2, AipsError);
      cout << (Int)a(0) << " " << (Int)a(1) << " (2,0)" << endl;
      c = aa.cFKeyFromAntennaTypes(aa.antTypeFromName("ZZ"),aa.antTypeFromName("XY02"));
      a.assign( ALMAAperture::antennaTypesFromCFKey(c) );
      AlwaysAssert(a.nelements()==1, AipsError);
      AlwaysAssert(a(0)==0, AipsError);
      cout << (Int)a(0) << " (0)" << endl;

      
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
	  if(count==5) apB.resetAntTypeMap();
	  // five rounds is enough
	  if(count>5) break;
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
	  if(count==5) apB.resetAntTypeMap();
	  // five rounds is enough
	  if(count>5) break;
	}

	cout << endl << "******** ApplySky to unity image with squint and without ***********" << endl << endl;

	count = 0;
	Int count2 = 0;

	for(vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	  //cout << "next chunk" << endl;
	  for(vi.origin();vi.more();vi++) {

	    if(vb.spectralWindow()<4){ // i.e. not a WVR SPW for this dataset
	      if(count2==0){ // first occurence
		Vector<ALMAAntennaType> aTs = apB.antTypeList(vb);
		Int cfKey = ALMAAperture::cFKeyFromAntennaTypes(aTs(0), aTs(0));
		Int cfKey2 = ALMAAperture::cFKeyFromAntennaTypes(aTs(0), aTs(1));
		Int cfKey3 = ALMAAperture::cFKeyFromAntennaTypes(aTs(1), aTs(1));
		Int cfKey4 = ALMAAperture::cFKeyFromAntennaTypes(ALMAAperture::antTypeFromName("DV"),ALMAAperture::antTypeFromName("DA"));
		Int cfKey5 = ALMAAperture::cFKeyFromAntennaTypes(ALMAAperture::antTypeFromName("DA"),ALMAAperture::antTypeFromName("DA"));

		PagedImage<Complex> im5(ts2, coordsys3, "pb_squintDVDV");
		im5.set(Complex(1.0,0.0));
		apB.applySky(im5, vb, True, cfKey);
		
		PagedImage<Complex> im6(ts2, coordsys3, "pb_nosquintDVDV");
		im6.set(Complex(1.0,0.0));
		apB.applySky(im6, vb, False, cfKey);
		
		PagedImage<Complex> im7(ts2, coordsys3, "pb_squintDVPM");
		im7.set(Complex(1.0,0.0));
		apB.applySky(im7, vb, True, cfKey2);
		
		PagedImage<Complex> im8(ts2, coordsys3, "pb_nosquintDVPM");
		im8.set(Complex(1.0,0.0));
		apB.applySky(im8, vb, False, cfKey2);
		
		PagedImage<Complex> im9(ts2, coordsys3, "pb_squintPMPM");
		im9.set(Complex(1.0,0.0));
		apB.applySky(im9, vb, True, cfKey3);
		
		PagedImage<Complex> im10(ts2, coordsys3, "pb_nosquintPMPM");
		im10.set(Complex(1.0,0.0));
		apB.applySky(im10, vb, False, cfKey3);
		
		PagedImage<Complex> im11(ts2, coordsys3, "pb_squintDVDA");
		im11.set(Complex(1.0,0.0));
		apB.applySky(im11, vb, True, cfKey4);
		
		PagedImage<Complex> im12(ts2, coordsys3, "pb_squintDADA");
		im12.set(Complex(1.0,0.0));
		apB.applySky(im12, vb, True, cfKey5);
		
		PagedImage<Complex> im13(ts2, coordsys3Big, "pb_squintDVDABig");
		im13.set(Complex(1.0,0.0));
		apB.applySky(im13, vb, True, cfKey4);
		
		PagedImage<Complex> im14(ts2, coordsys3Small, "pb_squintDVDASmall");
		im14.set(Complex(1.0,0.0));
		apB.applySky(im14, vb, True, cfKey4);

 		count2++;
		
	      }
	      else if(count2>200 && vb.fieldId()!=1){ // pick later occurance (to vary PA) but avoid Mars because it has invalid coords 

		Vector<ALMAAntennaType> aTs = apB.antTypeList(vb);
		Int cfKey = ALMAAperture::cFKeyFromAntennaTypes(aTs(0), aTs(0));
		Int cfKey2 = ALMAAperture::cFKeyFromAntennaTypes(aTs(0), aTs(1));
		Int cfKey3 = ALMAAperture::cFKeyFromAntennaTypes(aTs(1), aTs(1));
		Int cfKey4 = ALMAAperture::cFKeyFromAntennaTypes(ALMAAperture::antTypeFromName("DV"),ALMAAperture::antTypeFromName("DA"));
		Int cfKey5 = ALMAAperture::cFKeyFromAntennaTypes(ALMAAperture::antTypeFromName("DA"),ALMAAperture::antTypeFromName("DA"));
		Int cfKey6 = ALMAAperture::cFKeyFromAntennaTypes(ALMAAperture::antTypeFromName("PM"),ALMAAperture::antTypeFromName("PM"));
		Int cfKey7 = ALMAAperture::cFKeyFromAntennaTypes(ALMAAperture::antTypeFromName("DV"),ALMAAperture::antTypeFromName("DV"));
		Int cfKey8 = ALMAAperture::cFKeyFromAntennaTypes(ALMAAperture::antTypeFromName("CM"),ALMAAperture::antTypeFromName("CM"));
		
		PagedImage<Complex> im15(ts2, coordsys3, "pb2_squintDVDA");
		im15.set(Complex(1.0,0.0));
		apB.applySky(im15, vb, True, cfKey4);
		
		PagedImage<Complex> im16(ts2, coordsys3, "pb2_squintDADA");
		im16.set(Complex(1.0,0.0));
		apB.applySky(im16, vb, True, cfKey5);
		
		PagedImage<Complex> im17(ts2, coordsys3Big, "pb2_squintDVDABig");
		im17.set(Complex(1.0,0.0));
		apB.applySky(im17, vb, True, cfKey4);
		
		PagedImage<Complex> im18(ts2, coordsys3Small, "pb2_squintDVDASmall");
		im18.set(Complex(1.0,0.0));
		apB.applySky(im18, vb, True, cfKey4);

		PagedImage<Float> im18b(ts2, coordsys3Small, "pb2_squintDVDASmall_float");
		im18b.set(1.0);
		apB.applySky(im18b, vb, True, cfKey4);

		////////// with ray tracing

		cout << "Now using ray tracing ..." << endl;

		PagedImage<Complex> im19(ts2, coordsys3, "pb2_squintDVDVray");
		im19.set(Complex(1.0,0.0));
		apB.applySky(im19, vb, True, cfKey7, True);
		
		PagedImage<Complex> im21(ts3, coordsys3BigHiRes, "pb2_squintDVDVBigrayHiRes");
		im21.set(Complex(1.0,0.0));
		apB.applySky(im21, vb, True, cfKey7, True);
		
		PagedImage<Complex> im22(ts2, coordsys3Small, "pb2_squintDVDVSmallray");
		im22.set(Complex(1.0,0.0));
		apB.applySky(im22, vb, True, cfKey7, True);

		PagedImage<Complex> im23(ts2, coordsys3, "pb2_squintDADAray");
		im23.set(Complex(1.0,0.0));
		apB.applySky(im23, vb, True, cfKey5, True);
		
		PagedImage<Complex> im24(ts2, coordsys3Big, "pb2_squintDADABigray");
		im24.set(Complex(1.0,0.0));
		apB.applySky(im24, vb, True, cfKey5, True);
		
		PagedImage<Complex> im25(ts2, coordsys3Small, "pb2_squintDADASmallray");
		im25.set(Complex(1.0,0.0));
		apB.applySky(im25, vb, True, cfKey5, True);

		PagedImage<Complex> im26(ts2, coordsys3, "pb2_squintPMPMray");
		im26.set(Complex(1.0,0.0));
		apB.applySky(im26, vb, True, cfKey6, True);
		
		PagedImage<Complex> im27(ts2, coordsys3Big, "pb2_squintPMPMBigray");
		im27.set(Complex(1.0,0.0));
		apB.applySky(im27, vb, True, cfKey6, True);
		
		PagedImage<Complex> im28(ts2, coordsys3Small, "pb2_squintPMPMSmallray");
		im28.set(Complex(1.0,0.0));
		apB.applySky(im28, vb, True, cfKey6, True);

		PagedImage<Complex> im29(ts2, coordsys3, "pb2_squintCMCMray");
		im29.set(Complex(1.0,0.0));
		apB.applySky(im29, vb, True, cfKey8, True);
		
		PagedImage<Complex> im30(ts2, coordsys3Big, "pb2_squintCMCMBigray");
		im30.set(Complex(1.0,0.0));
		apB.applySky(im30, vb, True, cfKey8, True);
		
		PagedImage<Complex> im31(ts2, coordsys3Small, "pb2_squintCMCMSmallray");
		im31.set(Complex(1.0,0.0));
		apB.applySky(im31, vb, True, cfKey8, True);

		count2++;		

		count = -1;
	      }
	      else{
		count2++;
	      }
	    }
	  }
	  if(count<0) break;
	  count++;
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



