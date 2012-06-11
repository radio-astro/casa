//# VPSkyJones.cc: Implementation for VPSkyJones
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/TransformMachines/PBMath.h>
#include <synthesis/TransformMachines/VPSkyJones.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables.h>
#include <tables/Tables/TableRecord.h>
#include <ms/MeasurementSets/MSColumns.h>

#include <measures/Measures/Stokes.h>

#include <casa/BasicSL/Constants.h>

#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/ComponentShape.h>

#include <synthesis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>

#include <casa/Utilities/Assert.h>

namespace casa { //# NAMESPACE CASA - BEGIN

VPSkyJones::VPSkyJones(const ROMSColumns& msc, Table& tab,
		       const Quantity &parAngleInc,
		       BeamSquint::SquintType doSquint,
		       const Quantity &skyPositionThreshold) 
  : BeamSkyJones(parAngleInc, doSquint,skyPositionThreshold)
{ 
  LogIO os(LogOrigin("VPSkyJones", "VPSkyJones"));

  const uInt nrow = tab.nrow();

  ROScalarColumn<String> telCol(tab, "telescope");
  ROScalarColumn<Int> antCol(tab, "antenna");
  ROScalarColumn<TableRecord> recCol(tab, "pbdescription");

    
  for (uInt i=0; i < nrow; ++i) {
    // if-operator condition checks that the names are not redundant
    if (indexTelescope(telCol(i))<0) {
   
    
	 
  
      String name;
      String commonpb;
      if(recCol(i).isDefined("name") && (recCol(i).asString("name") != "REFERENCE")){

	const Int nameFieldNumber=recCol(i).fieldNumber("name");
	if (nameFieldNumber!=-1)
          recCol(i).get(nameFieldNumber,name);      
	
	if (name == "COMMONPB") {
	  const Int commonPBFieldNumber=recCol(i).fieldNumber("commonpb");
	  AlwaysAssert(commonPBFieldNumber!=-1,AipsError);
	  recCol(i).get(commonPBFieldNumber, commonpb );
	}      
	
	if (commonpb == "DEFAULT") {
	  String band;
	  PBMath::CommonPB whichPB;
	  String commonPBName;
	  ROScalarColumn<String> telescopesCol(msc.observation().telescopeName());
	  Quantity freq( msc.spectralWindow().refFrequency()(0), "Hz");	
	  String tele =  telCol(i);
	  if(tele=="") {
	    os  << "Telescope name for row " << i << " of " << tab.tableName()
		<< " is blank : cannot find correct primary beam model" << LogIO::EXCEPTION;
	  }
	  else {
	    PBMath::whichCommonPBtoUse( tele, freq, band, whichPB, commonPBName );
	  }
	  
	  PBMath  myPBMath(tele, False, freq );
	  setPBMath (telCol(i), myPBMath);	
	  
	} else {        
	  PBMath  myPBMath( recCol(i));
	setPBMath (telCol(i), myPBMath);
	}
      }

    }

  }
};


VPSkyJones::VPSkyJones(const ROMSColumns& msc,
		       Bool makePBs,
		       const Quantity &parAngleInc,
		       BeamSquint::SquintType doSquint,
		       const Quantity &skyPositionThreshold) 
  : BeamSkyJones(parAngleInc, doSquint, skyPositionThreshold)
{ 
  LogIO os(LogOrigin("VPSkyJones", "VPSkyJones"));

  if (makePBs) {
    ROScalarColumn<String> telescopesCol(msc.observation().telescopeName());
    

    for (uInt i=0; i < telescopesCol.nrow(); ++i) {
      String telescope_p = telescopesCol(i); 
      // if-operator condition checks that the names are not redundant
      if (indexTelescope(telescope_p)<0) {
	// Ultimately, we need to get the Beam parameters from the MS.beamSubTable,
	// but until then we will use this: figure it out from Telescope and Freq
	String band;
	PBMath::CommonPB whichPB;
	String commonPBName;
	// This frequency is ONLY required to determine which PB model to use:
	// The VLA, the ATNF, and WSRT have frequency - dependent PB models
	Quantity freq( msc.spectralWindow().refFrequency()(0), "Hz");
      
	if((telescope_p==" ") || (telescope_p=="")) {
	  whichPB=PBMath::UNKNOWN;
	}
	else {
	  PBMath::whichCommonPBtoUse( telescope_p, freq, band, whichPB, commonPBName );
	}
	
	if(whichPB != PBMath::UNKNOWN){
	  os << "PB used " << commonPBName << LogIO::POST;
	    PBMath  myPBMath(telescope_p, False, freq );
	    setPBMath (telescope_p, myPBMath);
	}
	else{
	  //lets do it by diameter
	  os << "PB used determined from dish-diameter" << LogIO::POST;
	  Double diam=msc.antenna().dishDiameter()(0);
	  PBMath myPBMath(diam, False, freq);
	  setPBMath(telescope_p, myPBMath);

	}



      }
    }
  }
};


VPSkyJones::VPSkyJones(const String& telescope,
		       PBMath::CommonPB commonPBType,
		       const Quantity &parAngleInc,
		       BeamSquint::SquintType doSquint,
		       const Quantity &skyPositionThreshold) 
  : BeamSkyJones(parAngleInc, doSquint,skyPositionThreshold)
{
  LogIO os(LogOrigin("VPSkyJones", "VPSkyJones"));
   

  // we need a way to do this for multiple telescope cases
  String telescope_p = telescope;
  PBMath  myPBMath(commonPBType);
  setPBMath (telescope_p, myPBMath);

};

 


VPSkyJones::VPSkyJones(const String& telescope,
		       PBMath& myPBMath,
		       const Quantity &parAngleInc,
		       BeamSquint::SquintType doSquint,
		       const Quantity &skyPositionThreshold) 
  : BeamSkyJones(parAngleInc, doSquint,skyPositionThreshold)
{ 
  LogIO os(LogOrigin("VPSkyJones", "VPSkyJones"));
   

  // we need a way to do this for multiple telescope cases
  String telescope_p = telescope;
  setPBMath (telescope_p, myPBMath);

};

 

} //# NAMESPACE CASA - END

