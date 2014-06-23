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

#include <synthesis/MSVis/VisSet.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisibilityIterator.h>

#include <synthesis/MeasurementComponents/EVLAAperture.h>


#include <casa/namespace.h>
int main()
{
  try {

    // Form coordinate systems and test images


    CoordinateSystem coordsys;
    CoordinateSystem coordsys3;
    CoordinateSystem coordsys3Big;
    CoordinateSystem coordsys3Small;
    CoordinateSystem coordsys4;
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
				       -5.*C::pi/180.0/3600.0, 5.*C::pi/180.0/3600.0,        
				       xform,                              
				       63.5, 63.5);  
      DirectionCoordinate dirCoordsSmall(MDirection::J2000,                  
					 Projection(Projection::SIN),        
					 135*C::pi/180.0, 60*C::pi/180.0,    
					 -0.5*C::pi/180.0/3600.0, 0.5*C::pi/180.0/3600.0,        
					 xform,                              
					 63.5, 63.5);  

      Vector<String> units(2); units = "deg";                       
      dirCoords.setWorldAxisUnits(units);                               
      

      // StokesCoordinate
      Vector<Int> iquv(1);                                         
      iquv(0) = Stokes::I;
      StokesCoordinate stokesCoordsBad(iquv);	
      Vector<Int> stoks(4);
      stoks(0) = Stokes::RR;
      stoks(1) = Stokes::RL;
      stoks(2) = Stokes::LR;
      stoks(3) = Stokes::LL;
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
					 1. * 1.0E+9,                 
					 20 * 1.0E+3,                   
					 0,                             
					 1. * 1.0E+9);          
      SpectralCoordinate spectralCoords4(MFrequency::TOPO,           
 					 6.035328 * 1.0E+9,                 
 					 20 * 1.0E+3,                   
 					 0,                             
 					 6.035328 * 1.0E+9);    
      
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

      coordsys3Big.addCoordinate(dirCoordsBig);
      coordsys3Big.addCoordinate(stokesCoordsGood);
      coordsys3Big.addCoordinate(spectralCoords3);      

      coordsys3Small.addCoordinate(dirCoordsSmall);
      coordsys3Small.addCoordinate(stokesCoordsGood);
      coordsys3Small.addCoordinate(spectralCoords3);      

      coordsys4.addCoordinate(dirCoords);
      coordsys4.addCoordinate(stokesCoordsGood);
      coordsys4.addCoordinate(spectralCoords4);      

    }
    
    String name("tab1");
    TiledShape ts(IPosition(4,128,128,1,1));
    TiledShape ts2(IPosition(4,128,128,4,1));

    PagedImage<Complex> im1(ts, coordsys, name);
    PagedImage<Float> im2(ts, coordsys, "tab2");
    PagedImage<Complex> im3(ts2, coordsys3, "tab3");
    PagedImage<Float> im4(ts2, coordsys3, "tab4");

    im1.set(Complex(1.0,1.0));
    im2.set(0.0);
    im3.set(Complex(1.0,1.0));
    im4.set(0.0);
    
    ///////////////////////////////////////////

    { // begin tests

      cout << "new EVLAAperture" << endl;
      EVLAAperture aa;
      
      cout << aa.name() << endl;
      
      ////////////////////////////////////////////////
      
      {
	const char *sep=" ";
	char *aipsPath = strtok(getenv("CASAPATH"),sep);
	if (aipsPath == NULL)
	  throw(AipsError("CASAPATH not found."));
	
	String msFileName(aipsPath);
	msFileName = msFileName + "/data/regression/unittest/concat/input/A2256LC2_4.5s-1.ms";
	
	cout << "Reading " << msFileName << endl;
	
	MS ms(msFileName, Table::Old);
	
	Block<int> sort(4);
	sort[2] = MS::FIELD_ID;
	sort[3] = MS::ARRAY_ID;
	sort[1] = MS::DATA_DESC_ID;
	sort[0] = MS::TIME;
	
	ROVisibilityIterator vi(ms, sort, 0.);
	
	VisBuffer vb(vi);
	
	cout << "new EVLAAperture" << endl;
	EVLAAperture apB;
	
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
	  // five rounds is enough
	  if(count>5) break;
	}
	
	cout << "\nSecond tour through the MS, testing getVisParams and applySky" << endl;
	count=0;
	
	for(vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	  cout << "next chunk" << endl;
	  for(vi.origin();vi.more();vi++) {
	    // the following can't be tested here since getVisParams is protected
	    //cout << "band id " << apB.getVisParams(vb) << endl;

	    Int cfKey = 0;

	    try{
	      apB.applySky(im1, vb, True, cfKey);
	    } catch (AipsError x) {
	      cout << "Caught expected error: " << x.getMesg() << endl;
	    } 
	    apB.applySky(im3, vb, True, cfKey);

	  }
	  count++;
	  // five rounds is enough
	  if(count>5) break;
	}

	cout << endl << "******** ApplySky to unity image with squint and without ***********" << endl << endl;

	count = 0;
	Int count2 = 0;

	for(vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	  cout << "next chunk" << endl;
	  for(vi.origin();vi.more();vi++) {

	    if(count2==0){ // first occurence

	      Int cfKey = 0;

	      PagedImage<Complex> im5(ts2, coordsys3, "pb_squintEVLA");
	      im5.set(Complex(1.0,1.0));
	      apB.applySky(im5, vb, True, cfKey);
	      
	      PagedImage<Complex> im6(ts2, coordsys3, "pb_nosquintEVLA");
	      im6.set(Complex(1.0,1.0));
	      apB.applySky(im6, vb, False, cfKey);
	      
// 	      PagedImage<Complex> im13(ts2, coordsys3Big, "pb_squintEVLABig");
// 	      im13.set(Complex(1.0,1.0));
// 	      apB.applySky(im13, vb, True, cfKey);
	      
// 	      PagedImage<Complex> im14(ts2, coordsys3Small, "pb_squintEVLASmall");
// 	      im14.set(Complex(1.0,1.0));
// 	      apB.applySky(im14, vb, True, cfKey);
	      
	      count2++;
	      
	    }
	    else if(count2>100){ // pick later occurance (to vary PA) 
	      
	      Int cfKey = 0;
	      
	      PagedImage<Complex> im15(ts2, coordsys3, "pb2_squintEVLA");
	      im15.set(Complex(1.0,1.0));
	      apB.applySky(im15, vb, True, cfKey);
	      
// 	      PagedImage<Complex> im17(ts2, coordsys3Big, "pb2_squintEVLABig");
// 	      im17.set(Complex(1.0,1.0));
// 	      apB.applySky(im17, vb, True, cfKey);
	      
// 	      PagedImage<Complex> im18(ts2, coordsys3Small, "pb2_squintEVLASmall");
// 	      im18.set(Complex(1.0,1.0));
// 	      apB.applySky(im18, vb, True, cfKey);
	      
// 	      PagedImage<Float> im18b(ts2, coordsys3Small, "pb2_squintEVLASmall_float");
// 	      im18b.set(1.0);
// 	      apB.applySky(im18b, vb, True, cfKey);
	      
	      count2++;		
	      
	      count = -1;
	    }
	    else{
	      count2++;
	    }
	  }
	  if(count<0) break;
	  count++;
	}

      }
      ////////////////////////////////////////////////
      
      {
	const char *sep=" ";
	char *aipsPath = strtok(getenv("CASAPATH"),sep);
	if (aipsPath == NULL)
	  throw(AipsError("CASAPATH not found."));
	
	String msFileName(aipsPath);
	msFileName = msFileName + "/data/regression/cvel/input/evla-highres-sample.ms";
	
	cout << "Reading " << msFileName << endl;
	
	MS ms(msFileName, Table::Old);
	
	Block<int> sort(4);
	sort[2] = MS::FIELD_ID;
	sort[3] = MS::ARRAY_ID;
	sort[1] = MS::DATA_DESC_ID;
	sort[0] = MS::TIME;
	
	ROVisibilityIterator vi(ms, sort, 0.);
	
	VisBuffer vb(vi);
	
	cout << "new EVLAAperture" << endl;
	EVLAAperture apB;
	
	
	cout << endl << "******** ApplySky to unity image with squint and without ***********" << endl << endl;

	Int count = 0;
	Int count2 = 0;

	for(vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	  cout << "next chunk" << endl;
	  for(vi.origin();vi.more();vi++) {

	    if(count2==0){ // first occurence

	      Int cfKey = 0;

	      PagedImage<Complex> im5(ts2, coordsys4, "pb3_squintEVLA");
	      im5.set(Complex(1.0,1.0));
	      apB.applySky(im5, vb, True, cfKey);
	      
	      PagedImage<Complex> im6(ts2, coordsys4, "pb3_nosquintEVLA");
	      im6.set(Complex(1.0,1.0));
	      apB.applySky(im6, vb, False, cfKey);
	      
	      count2++;
	      
	    }
	    else if(count2>100){ // pick later occurance (to vary PA) 
	      
	      Int cfKey = 0;
	      
	      PagedImage<Complex> im15(ts2, coordsys4, "pb4_squintEVLA");
	      im15.set(Complex(1.0,1.0));
	      apB.applySky(im15, vb, True, cfKey);
	      
	      count2++;		
	      
	      count = -1;
	    }
	    else{
	      count2++;
	    }
	  }
	  if(count<0) break;
	  count++;
	}

      }

    } // end tests

  } catch (AipsError x) {
    cout << "Caught Error: " << x.getMesg() << endl;
    exit(1);
  } 

  cout << "OK" << endl;
  exit(0);
}



