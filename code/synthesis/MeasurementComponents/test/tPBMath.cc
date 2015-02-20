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
      StokesCoordinate stokesCoords(iquv);	
      
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
      coordsys.addCoordinate(stokesCoords);
      coordsys.addCoordinate(spectralCoords);
      
      coordsys3.addCoordinate(dirCoords);
      coordsys3.addCoordinate(stokesCoords);
      coordsys3.addCoordinate(spectralCoords3);      
    }
    
    String name("tab1");
    TiledShape ts(IPosition(4,128,128,1,1));

    PagedImage<Float> im1(ts, coordsys, name);
    PagedImage<Float> im2(ts, coordsys, "tab2");
    PagedImage<Float> im3(ts, coordsys3, "tab3");
    PagedImage<Float> im4(ts, coordsys3, "tab4");

    im1.set(1.0);
    im2.set(0.0);
    im3.set(1.0);
    im4.set(0.0);
    
    LatticeStepper ls2(im2.shape(), IPosition(4, 128, 1, 1, 1),
                      IPosition(4, 0, 1, 2, 3));
    LatticeStepper ls4(im4.shape(), IPosition(4, 128, 1, 1, 1),
                      IPosition(4, 0, 1, 2, 3));
    RO_LatticeIterator<Float> li2(im2, ls2);
    RO_LatticeIterator<Float> li4(im4, ls4);
 
    // Form guts of SkyComponents for testing
    MDirection coord1(Quantity(135.0,"deg"), Quantity(60.01, "deg"),   
		      MDirection::Ref(MDirection::J2000));
    PointShape point1(coord1);
    Flux<Double> flux1(1.0, 0.0, 0.0, 0.0);      
    flux1.convertPol(ComponentType::CIRCULAR);      
    Flux<Double> flux2(1.0, 0.0, 0.0, 0.0);      
    MFrequency mfr(Quantity(43.0, "GHz"));
    SpectralIndex spectrum1(mfr, 0.0);

    // ********************************************************************************
    // ****************   Let the Primary Beam wild rumpus begin **********************
    // ********************************************************************************

    MDirection pointingDir ( Quantity(135.0, "deg"), Quantity(60.0, "deg"), 
			      MDirection::Ref(MDirection::J2000) );

    cout << "Pointing direction -1 = " << pointingDir.getAngle().getValue("deg")(0) << "  " <<
      pointingDir.getAngle().getValue("deg")(1) << endl;
    // VLA_Q
    {
      cout << "Pointing direction 0 = " << pointingDir.getAngle().getValue("deg")(0) << "  " <<
	pointingDir.getAngle().getValue("deg")(1) << endl;

     PBMath myPB(PBMath::VLA_Q);
      myPB.summary();
      myPB.applyPB2( im3, im4, pointingDir, Quantity(0.0, "deg"), BeamSquint::NONE);
      li4.reset();
      cout << "Airy VLA Q, no squint, at 43 GHz Apply" << endl;
      uInt ii;
      for (ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
      myPB.applyPB2( im3, im4, pointingDir, Quantity(0.0, "deg"), BeamSquint::RR);
      li4.reset();
      cout << "Airy VLA Q, RR Squint, at 43 GHz Apply" << endl;
      for (ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
      myPB.applyPB2( im3, im4, pointingDir, Quantity(0.0, "deg"), BeamSquint::LL);
      li4.reset();
      cout << "Airy VLA Q, LL Squint, at 43 GHz Apply" << endl;
      for (ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
      myPB.applyPB2( im3, im4, pointingDir, Quantity(0.0, "deg"), BeamSquint::GOFIGURE);
      li4.reset();
      cout << "Airy VLA Q, GOFIGURE Squint (none), at 43 GHz Apply" << endl;
      for (ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }

      Vector<Double> xPA(36);
      Vector<Double> yFluxRR(36);
      Vector<Double> yFluxLL(36);
      Quantity Freq(43.0e+9, "Hz");

      // test the beam squint
      Int i;
      for (i = 0; i < 36; i++) {
	SkyComponent sc1( flux1, point1, spectrum1);
	SkyComponent sc2( flux1, point1, spectrum1);
	Quantity parAngle( (Double)10.0*i, "deg");
	myPB.applyPB( sc1, sc2, pointingDir, Freq, 
			 parAngle, BeamSquint::GOFIGURE);
	xPA(i) =  (Double)10.0*i;
	
	yFluxRR(i) = real(sc2.flux().value()(0));
	yFluxLL(i) = real(sc2.flux().value()(3));

      }
      cout << " The following illustrates beam squint for a 1 Jy SkyComponent point source " << endl;
      cout << " 0.01 deg straight north of the pointing center at 43 GHz, with the VLA Q Band PB Model  " <<
	endl;
      cout << " PA[deg]    RR    LL " << endl;
      for ( i=0; i< 36; i++) {
	cout <<  xPA(i) << "   " << yFluxRR(i) << "   " << yFluxLL(i) << endl;
      }
    }

    // test out "extent"
    {
      PBMath1DGauss myGauss( Quantity(0.5,"'"), Quantity(1.0, "'"),
			     Quantity(43, "GHz") );
      {      
	const ImageRegion* pir = myGauss.extent(im3, pointingDir, 0, 0, 0, SkyJones::ANY);
	const LCSlicer lcs = pir->asLCSlicer();
	const Slicer sl = lcs.toSlicer(IPosition(4,0,0,0,0), im3.shape());
	cout << "Testing extent with ANY: " << sl.start() << sl.end() 
	     <<  endl;
      }
      {      
	const ImageRegion* pir = myGauss.extent(im3, pointingDir, 0, 0, 0, SkyJones::POWEROF2);
	const LCSlicer lcs = pir->asLCSlicer();
	const Slicer sl = lcs.toSlicer(IPosition(4,0,0,0,0), im3.shape());
	cout << "Testing extent with POWEROF2: " << sl.start() << sl.end() 
	     <<  endl;
      }
      {      
	const ImageRegion* pir = myGauss.extent(im3, pointingDir, 0, 0, 0, SkyJones::COMPOSITE);
	const LCSlicer lcs = pir->asLCSlicer();
	const Slicer sl = lcs.toSlicer(IPosition(4,0,0,0,0), im3.shape());
	cout << "Testing extent with COMPOSITE: " << sl.start() << sl.end() 
	     <<  endl;
      }
    }


    // VLA
    {
      PBMath myPB(PBMath::VLA);
      myPB.summary();
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Poly VLA at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }


    // VLA_INVERSE
    {
      PBMath myPB(PBMath::VLA_INVERSE);
      myPB.summary();
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Inverse Poly VLA at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }


    /********************************************************
       cannot use this yet! VLA_NVSS is Airy disk, see bug in PBMath1DAiry
    // VLA_NVSS
    {
      PBMath myPB(PBMath::VLA_NVSS);
      myPB.summary();
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Airy VLA at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }
    *******************************************************/

   

    // Hat Creek
    {
      PBMath myPB(PBMath::HATCREEK);
      myPB.summary(10);
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Gaussian HATCREEK,  at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }

    // explicit Gaussian constructor
    {
      PBMath myPB(PBMathInterface::GAUSS, 
		  Quantity(191.67/2.0,"'"), Quantity(215.0, "'"),
		  Quantity(1.0, "GHz") );
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Gaussian HATCREEK through explicit Gaussian constructor, at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }


    // TestCosPoly: WSRT
    {
      PBMath myPB(PBMath::WSRT);
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Cos Poly WSRT at 43 GHz (dream on!) Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }

    // ATCA_L1
    {
      PBMath myPB(PBMath::ATCA_L1);
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Poly ATCA L1,  at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }

    // ATCA_L2
    {
      PBMath myPB(PBMath::ATCA_L2);
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Poly ATCA L2,  at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }

     // ATCA_X
    {
      PBMath myPB(PBMath::ATCA_X);
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "Poly ATCA X,  at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }


     // ALMA
    {
      PBMath myPB(PBMath::ALMA);
      myPB.applyPB2( im3, im4, pointingDir);
      li4.reset();
      cout << "ALMA,  at 43 GHz Apply" << endl;
      for (uInt ii=0;ii<128;ii++) {
	cout << ii << " " << li4.vectorCursor()(ii) << endl;
      }
    }


    // static functions
    String pbString;
    PBMath::nameCommonPB(PBMath::VLA_L, pbString);
    cout << "PBString should be VLA_L, and it is: " << pbString << endl;

    PBMath::CommonPB ipb;
    PBMath::enumerateCommonPB("VLA_L", ipb);
    cout << "PBnumber should be 20, and it is: " << ipb << endl;

    PBMath::nameCommonPB(PBMath::ALMA, pbString);
    cout << "PBString should be ALMA, and it is: " << pbString << endl;

    PBMath::enumerateCommonPB("ALMA", ipb);
    cout << "PBnumber should be 28, and it is: " << ipb << endl;


  } catch (AipsError x) {
    cout << x.getMesg() << endl;
  } 

  cout << "OK" << endl;
  exit(0);
}



