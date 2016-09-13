//# tPBMath.cc: This program tests the PBMath objects
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
#include <casa/Arrays/ArrayMath.h>

#include <images/Images/PagedImage.h>
#include <images/Images/SubImage.h>
#include <images/Images/TempImage.h>
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

#include <synthesis/MeasurementComponents/SkyJones.h>
#include <synthesis/MeasurementComponents/PBMath.h>
#include <casa/BasicSL/String.h>


#include <casa/namespace.h>
int main()
{
  try {
    cout << "Tests Apply" << endl;
    cout << "--------------------------------------" << endl;

    // Form coordinate systems and test images
    Vector<Double> imsize(2);
    imsize=512.0;

   LogIO os; 
    CoordinateSystem coordsys;
    {
      
      Matrix<Double> xform(2,2);                                    
      xform = 0.0; xform.diagonal() = 1.0;                          
      DirectionCoordinate dirCoords(MDirection::AZELGEO,                  
				    Projection(Projection::SIN),        
				    0.0,0.0,    
				    -1*C::pi/180.0/3600.0, 1*C::pi/180.0/3600.0,        
				    xform,                              
				    imsize(0)/2.0, imsize(1)/2.0);  
      /* (MDirection::AZELGEO, dc_p.projection(), 0.0, 
				      0.0, dc_p.increment()(0), dc_p.increment()(1), xform, unitVec(0),
				      unitVec(1));*/
      // reference position is in lower left corner; easier for beam inspection!
      
      Vector<String> units(2); units = "deg";                       
      dirCoords.setWorldAxisUnits(units);                               
      Vector<Int> stokes(4);
      stokes(0)=Stokes::XX; stokes(1)=Stokes::XY; stokes(2)=Stokes::YX;
      stokes(3)=Stokes::YY;
      StokesCoordinate stokesCoords(stokes);	
      
      // SpectralCoordinate
      SpectralCoordinate spectralCoords(MFrequency::TOPO,           
					200 * 1.0E+9,                 
					20 * 1.0E+6,                   
					0,                             
					200. * 1.0E+9);          
      units.resize(1);
      units = "Hz";
      spectralCoords.setWorldAxisUnits(units);
      
       
      // CoordinateSystem
      coordsys.addCoordinate(dirCoords);
      coordsys.addCoordinate(stokesCoords);
      coordsys.addCoordinate(spectralCoords);
      IPosition latticeShape, tileShape;
      coordsys.list(os, MDoppler::RADIO, latticeShape, tileShape, False);
     
    }
    
    TiledShape ts(IPosition(4,Int(imsize(0)),Int(imsize(1)),4,1));

    PagedImage<Complex> im1(ts, coordsys,"ALMA_12M.VP");
    PagedImage<Complex> im2(ts, coordsys,"ALMA_7M.VP");
    im1.set(Complex(1.0,0.0));
    im2.set(Complex(1.0,0.0));
    {
      IPosition blc(4, 0, 0, 1, 0);
      IPosition trc=im1.shape()-1;
      trc(2)=2;
      Slicer sl(blc, trc, Slicer::endIsLast);
      SubImage<Complex> sub1(im1, sl, True);
      SubImage<Complex> sub2(im2, sl, True);
      sub1.set(Complex(0.0, 0.0));
      sub2.set(Complex(0.0, 0.0));
    }
    PBMath1DAiry mypb12(12.0, 12.0/25.0*2.0,  
						    Quantity(200,"arcsec"), 
		      Quantity(100.0,"GHz"));
    PBMath1DAiry mypb7(7.0, 7.0/25.0*2.0,  
						    Quantity(300,"arcsec"), 
		      Quantity(100.0,"GHz"));
    MDirection coord1(Quantity(0.0,"deg"), Quantity(0.0, "deg"),   
		      MDirection::Ref(MDirection::AZELGEO));
    
    mypb12.applyVP(im1, im1, coord1);
    mypb7.applyVP(im2, im2, coord1);

    /*****************now we test*****************************/
    
     PBMath2DImage impb12 (PagedImage<Complex>("ALMA_12M.VP"));
     PBMath2DImage impb7 (PagedImage<Complex>("ALMA_7M.VP"));

     Vector<Int> stokesI(1);
     stokesI(0)=Stokes::I;
     StokesCoordinate stokesCoordsI(stokesI);
     coordsys.replaceCoordinate(stokesCoordsI, 1);
     TiledShape tsreal(IPosition(4,Int(imsize(0)),Int(imsize(1)),1,1));
     PagedImage<Float> someimage(tsreal, coordsys, "imbeam");
     PagedImage<Float> otherimage(tsreal, coordsys, "cannedbeam");
     someimage.set(1.0);
     otherimage.set(1.0);
     impb12.applyPB( someimage, someimage, coord1,  
		     Quantity(0.0, "deg"), BeamSquint::NONE);
     mypb12.applyPB( otherimage, otherimage, coord1,  
		     Quantity(0.0, "deg"), BeamSquint::NONE);			   
     
     cerr << "Maxes " << max(someimage.get()) << "   " << max(otherimage.get()) << "   " << max( someimage.get() - otherimage.get()) << endl;
     if(abs(max( someimage.get() - otherimage.get())) > 1e-7)
       throw(AipsError("Failed in 12 m image beaming"));
     someimage.set(1.0);
     otherimage.set(1.0);
     impb7.applyPB( someimage, someimage, coord1,  
		     Quantity(0.0, "deg"), BeamSquint::NONE);
     mypb7.applyPB( otherimage, otherimage, coord1,  
		     Quantity(0.0, "deg"), BeamSquint::NONE);
     if(abs(max( someimage.get() - otherimage.get())) > 1e-7)
       throw(AipsError("Failed in 7 m image beaming"));

  } catch (AipsError x) {
    cout << x.getMesg() << endl;
  } 

  cout << "OK" << endl;
  exit(0);
}



