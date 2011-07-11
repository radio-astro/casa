//# tVPSkyJones.cc: This program tests that VPSkyJones works
//# Copyright (C) 1998,1999,2000
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

//# Includes
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Constants.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures.h>
#include <coordinates/Coordinates.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

#include <images/Images/PagedImage.h>
#include <images/Regions/ImageRegion.h>

#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/TiledLineStepper.h>
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

#include <ms/MeasurementSets/MSSummary.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <synthesis/MeasurementComponents/VPSkyJones.h>
#include <synthesis/MeasurementComponents/PBMath.h>
#include <casa/BasicSL/String.h>


#include <casa/namespace.h>
int main()
{
  try {
    cout << "VPSkyJones" << endl;
    cout << "--------------------------------------" << endl;


    // Open the test MS
    MeasurementSet ms("3C273XC1.ms", Table::Update);

    LogIO os(LogOrigin("tVPSkyJones", "main()"));

    //   MSSummary mss(ms);
    //   mss.list(os, True);


    VPSkyJones  myVPSJ(ms, True, Quantity(0.1, "deg"), BeamSquint::GOFIGURE);
    cout << "Using 0.1 deg PA chunks for fun" << endl;


    VPSkyJones  myVPSJ2(ms, PBMath::VLA, Quantity(0.1, "deg"), BeamSquint::GOFIGURE);


    // Form guts of SkyComponents for testing  This 0.05 deg north of 3C273
    MDirection coord1(Quantity(186.63854,"deg"), Quantity((2.3286917+0.05), "deg"),   
		      MDirection::Ref(MDirection::B1950)); 
    PointShape point1(coord1);
    Flux<Double> flux1(1.0, 0.0, 0.0, 0.0);      
    flux1.convertPol(ComponentType::CIRCULAR);      
    SpectralIndex spectrum1(( MFrequency(Quantity(8.0,"GHz"))), 0.0);

    // Form an image to apply the Primary Beam to: must be
    // STOKES RR LL RL LR, multi frequency to be a good test

    
    // Form coordinate systems and test images
      Matrix<Double> xform(2,2);                                    
      xform = 0.0; xform.diagonal() = 1.0;                          
      DirectionCoordinate dirCoords(MDirection::B1950,                  
                                    Projection(Projection::SIN),        
                                    186.63854*C::pi/180.0, 2.3286917*C::pi/180.0,    
                                    -10.0*C::pi/180.0/3600.0, 10.0*C::pi/180.0/3600.0,
                                    xform,                              
                                    64, 0);  
      Vector<String> units(2); units = "deg";                       
      dirCoords.setWorldAxisUnits(units);                               
      
      // StokesCoordinate
      Vector<Int> iquv(4);                                         
      iquv(0) = Stokes::RR;
      iquv(1) = Stokes::RL;
      iquv(2) = Stokes::LR;
      iquv(3) = Stokes::LL;
      StokesCoordinate stokesCoords(iquv);                               

      // SpectralCoordinate 
      SpectralCoordinate spectralCoords(MFrequency::TOPO,           
                                        8.085 * 1.0E+9,                 
                                        0.5 *  1.0E+9,                   
                                        0,                             
                                        8.000 * 1.0E+9);          
      units.resize(1);
      units = "Hz";
      spectralCoords.setWorldAxisUnits(units);
      
      // CoordinateSystem
      CoordinateSystem coordsys;
      coordsys.addCoordinate(dirCoords);
      coordsys.addCoordinate(stokesCoords);
      coordsys.addCoordinate(spectralCoords);

      TiledShape ts(IPosition(4,128,2,4,2));
      //      TiledShape ts(IPosition(4,128,2,1,1));
      PagedImage<Complex> im1(ts, coordsys, "tab1");
      PagedImage<Complex> im2(ts, coordsys, "tab2");
    
      im1.set(1.0);
      im2.set(0.0);


    // ********************************************************************************
    // ****************   Let the Primary Beam wild rumpus begin **********************
    // ********************************************************************************

    cout << "Constructing VisSet"<<endl;
    Block<Int> bi(2); 
    bi[0]=MS::DATA_DESC_ID;
    bi[1]=MS::TIME; 
    Matrix<Int> chanSelection; // no channel selection
    // iterate in 60s chunks within each SpectralWindow
    Double interval=60.; 
    VisSet vs(ms,bi,chanSelection,interval);
    cout << "Constructed VisSet"<<endl;

    VisIter &vi = vs.iter();
    VisBuffer vb(vi);
    cout << "Constructed VisBuffer"<<endl;

    SkyComponent sc1( flux1, point1, spectrum1);
    SkyComponent sc2( flux1, point1, spectrum1);
    SkyComponent sc3( flux1, point1, spectrum1);
    Int nChunks = 0;
    int row = 0;
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      cout << "This is integration: " << nChunks++ << endl;
      for (vi.origin();vi.more();vi++) {
	if (myVPSJ.change(vb)) { 
	  cout << "State has changed: " << endl;
	  myVPSJ.showState(os);
	  myVPSJ.apply(sc1, sc2, vb, row);
	  myVPSJ.applySquare(sc1, sc3, vb, row);
	  cout << "Flux of sky comp, PB, PB2 : " << real(sc1.flux().value()(0)) << " "
	       << real(sc2.flux().value()(0)) << " "
	       << real(sc3.flux().value()(0)) << endl;	  
	}
      }      
    }
    // just apply to the image once:
    // (we already know the parallactic angle stuff works for images,
    // and the VPJskyJones correctly extracts the PA as shown above.
    myVPSJ.apply(im1, im2, vb, row);

    {
      const ImageRegion* pir = myVPSJ.extent(im2, vb, row, 0, 0, SkyJones::ANY);
      const LCSlicer lcs = pir->asLCSlicer();
      const Slicer sl = lcs.toSlicer(IPosition(4,0,0,0,0), im2.shape());
      cout << "Testing extent with ANY: " << sl.start() << sl.end() 
	    << endl;
    }
    {
      const ImageRegion* pir = myVPSJ.extent(im2, vb, row, 0, 0, SkyJones::POWEROF2);
      const LCSlicer lcs = pir->asLCSlicer();
      const Slicer sl = lcs.toSlicer(IPosition(4,0,0,0,0), im2.shape());
      cout << "Testing extent with POWEROF2: " << sl.start() << sl.end() 
	   << endl;
    }
    {
      const ImageRegion* pir = myVPSJ.extent(im2, vb, row, 0, 0, SkyJones::COMPOSITE);
      const LCSlicer lcs = pir->asLCSlicer();
      const Slicer sl = lcs.toSlicer(IPosition(4,0,0,0,0), im2.shape());
      cout << "Testing extent with COMPOSITE: " << sl.start() << sl.end() 
	   << endl;
    }


    // iterate through: we want to get just one horizontal line going through the 4 stokes, then
    // to the other frequency

    
    LatticeStepper ls(im2.shape(), IPosition(4,128,1,1,1), IPosition(4,0,2,3,1));
    LatticeIterator<Complex> lirr(im2, ls); lirr.reset();
    LatticeIterator<Complex> lirl(im2, ls); lirl.reset();
    LatticeIterator<Complex> lilr(im2, ls); lilr.reset();
    LatticeIterator<Complex> lill(im2, ls); lill.reset();
    // set them up to point to the right place for the 4 stokes
    lirl++;
    lilr++;    lilr++;
    lill++;    lill++;    lill++;
    cout << "Beam parameters for VLA_X, Freq = 8.0805 GHz, including squint for RR, LL: " << endl;
    cout << "  IX     RR      RL       LR      LL " << endl;
    uInt ix;
    for (ix=0;ix<128;ix++) {
      cout << ix 
	   << " " << real(lirr.rwVectorCursor()(ix)) 
	   << " " << real(lirl.rwVectorCursor()(ix)) 
	   << " " << real(lilr.rwVectorCursor()(ix)) 
	   << " " << real(lill.rwVectorCursor()(ix)) 
	   << endl;
    }
    // advance all iterators to the next channel
    for (ix=0;ix<4;ix++) {
      lirr++;
      lirl++;
      lilr++;  
      lill++;
    }    
    cout << "Beam parameters for VLA_X, Freq = 8.5805 GHz, including squint for RR, LL: " << endl;
    cout << "  IX     RR      RL       LR      LL " << endl;
    for (ix=0;ix<128;ix++) {
      cout << ix 
	   << " " << real(lirr.rwVectorCursor()(ix)) 
	   << " " << real(lirl.rwVectorCursor()(ix)) 
	   << " " << real(lilr.rwVectorCursor()(ix)) 
	   << " " << real(lill.rwVectorCursor()(ix)) 
	   << endl;
    }
  } catch (AipsError x) {
    cout << x.getMesg() << endl;
  } 
  
  exit(0);
}

