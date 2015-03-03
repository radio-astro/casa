//# VisCalGlobals.h: Implementation of VisCalGlobals
//# Copyright (C) 1996,1997,2000,2001,2002,2003
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

#include <synthesis/MeasurementComponents/VisCalGlobals.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/DJones.h>
#include <synthesis/MeasurementComponents/GSpline.h>
#include <synthesis/MeasurementComponents/BPoly.h>
#include <synthesis/MeasurementComponents/EJones.h>
#include <synthesis/MeasurementComponents/EPJones.h>
#include <synthesis/MeasurementComponents/FJones.h>
#include <synthesis/MeasurementComponents/KJones.h>
#include <synthesis/MeasurementComponents/LJJones.h>
#include <synthesis/MeasurementComponents/AMueller.h>
#include <synthesis/MeasurementComponents/TsysGainCal.h>
#include <synthesis/MeasurementComponents/EVLASwPow.h>
#include <synthesis/MeasurementComponents/SingleDishSkyCal.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Create a specialized VisCal from VisSet
VisCal* createVisCal(const String& type, VisSet& vs) {

  String uptype=type;
  uptype.upcase();

  if (uptype=="P" || uptype=="P JONES") 
    return new PJones(vs);

  else
    // Try request as a solvable type
    return createSolvableVisCal(type,vs);
};

// Create a specialized VisCal from msname, nAnt, nSpw
VisCal* createVisCal(const String& type, String msname, Int MSnAnt, Int MSnSpw) {

  String uptype=type;
  uptype.upcase();

  if (uptype=="P" || uptype=="P JONES") 
    return new PJones(msname,MSnAnt,MSnSpw);

  else
    // Try request as a solvable type
    return createSolvableVisCal(type,msname,MSnAnt,MSnSpw);
};


// Create a specialized VisCal from nAnt
VisCal* createVisCal(const String& type, const Int& nAnt) {

  // TBD: handle P w/out an MS?
  //  if (type=="P") 
  //    return new PJones(nAnt);
  //  else
    // Try request as a solvable type
    return createSolvableVisCal(type,nAnt);
};


// Create a specialized SolvableVisCal from VisSet
SolvableVisCal* createSolvableVisCal(const String& type, VisSet& vs) {

  String uptype=type;
  uptype.upcase();

  if      (uptype=="B" || uptype=="B JONES") 
    return new BJones(vs);

  else if (uptype=="BPOLY") 
    return new BJonesPoly(vs);

  else if (uptype=="G" || uptype=="G JONES") 
    return new GJones(vs);

  else if (uptype=="GSPLINE") 
    return new GJonesSpline(vs);
  
  else if (uptype=="TF" || uptype=="TF JONES" || uptype=="TF NOISE") 
    return new TfJones(vs);

  else if (uptype=="T" || uptype=="T JONES" || uptype=="T NOISE") 
    return new TJones(vs);

  else if (uptype.before('+')=="DLIN" ||
	   uptype.before('+')=="D" || 
	   uptype=="D JONES") 
    return new DlinJones(vs);

  else if (uptype.before('+')=="DFLIN" || 
	   uptype.before('+')=="DF" || 
	   uptype=="DF JONES") 
    return new DflinJones(vs);

  else if (uptype.before('+')=="DLLS" || 
	   uptype=="DLLS JONES")
    return new DllsJones(vs);

  else if (uptype.before('+')=="DFLLS" || 
	   uptype=="DFLLS JONES")
    return new DfllsJones(vs);

  else if (uptype.before('+')=="DGEN" ||
	   uptype.before('+')=="DGENERAL" || 
	   uptype=="DGEN JONES") 
    return new DJones(vs);

  else if (uptype.before('+')=="DFGEN" || 
	   uptype.before('+')=="DFGENERAL" || 
	   uptype=="DFGEN JONES") 
    return new DfJones(vs);

  else if (uptype=="J" || uptype=="J JONES") 
    return new JJones(vs);

  else if (uptype == "EP" || uptype == "EP JONES")
    return new EPJones(vs);

  else if (uptype == "LJ" || uptype == "LJ JONES")
    return new LJJones(vs);

  else if (uptype=="M" || uptype=="M MUELLER")
    return new MMueller(vs);

  else if (uptype=="A" || uptype=="A MUELLER")
    return new AMueller(vs);

  else if (uptype=="N" || uptype=="A NOISE")
    return new ANoise(vs);

  else if (uptype=="MF" || uptype=="MF MUELLER")
    return new MfMueller(vs);
     
  else if (uptype=="X" || uptype=="X MUELLER")
    return new XMueller(vs);

  else if (uptype=="XJ" || uptype=="X JONES")
    return new XJones(vs);

  else if (uptype=="XF" || uptype=="XF JONES")
    return new XfJones(vs);

  else if (uptype=="K" || uptype=="K JONES")
    return new KJones(vs);

  else if (uptype=="KCROSS" || uptype=="KCROSS JONES")
    return new KcrossJones(vs);

  else if (uptype=="GLINXPH" || uptype=="GLINXPH JONES" ||
	   uptype=="XY+QU")
    return new GlinXphJones(vs);

  else if (uptype=="GLINXPHF" || uptype=="GLINXPHF JONES" ||
	   uptype=="XYF+QU")
    return new GlinXphfJones(vs);

  else if (uptype=="KMBD" || uptype=="KMBD JONES")
    return new KMBDJones(vs);

  else if (uptype.contains("KANTPOS") || uptype.contains("KANTPOS JONES"))
    return new KAntPosJones(vs);

  else if (uptype.contains("TSYS"))
    return new StandardTsys(vs);

  else if (uptype.contains("EVLASWP"))
    return new EVLASwPow(vs);

  else if (uptype=="TFOPAC")  // Not yet solvable (even though an SVJ)
    return new TfOpac(vs);

  else if (uptype=="TOPAC")  // Not yet solvable (even though an SVJ)
    return new TOpac(vs);

  else if (uptype.contains("GAINCURVE"))  // Not yet solvable (even though an SVJ)
    return new EGainCurve(vs);
  
  else if (uptype.contains("EVLAGAIN"))
    throw(AipsError("Please regenerate EVLA Sw Pow table using gencal."));

  else if (uptype.contains("TEC") || uptype.contains("F JONES") )  // Ionosphere
    return new FJones(vs);

  else if (uptype.contains("SDSKY_PS"))
    return new SingleDishPositionSwitchCal(vs);

  else if (uptype.contains("SDSKY_RASTER"))
    return new SingleDishRasterCal(vs);

  else if (uptype.contains("SDSKY_OTF"))
    return new SingleDishOtfCal(vs);
  
  else {
    cout << "attempted type = " << type << endl;
    throw(AipsError("Unknown calibration type."));
  }
};


// Create a specialized VisCal from msname, nAnt, nSpw
SolvableVisCal* createSolvableVisCal(const String& type, String msname, Int MSnAnt, Int MSnSpw) {
  String uptype=type;
  uptype.upcase();

  if      (uptype=="B" || uptype=="B JONES") 
    return new BJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="BPOLY") 
    throw(AipsError(uptype+" not yet supported via BJonesPoly(msname,MSnAnt,MSnSpw)"));
  //    return new BJonesPoly(msname,MSnAnt,MSnSpw);

  else if (uptype=="G" || uptype=="G JONES") 
    return new GJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="GSPLINE") 
    throw(AipsError(uptype+" not yet supported via BJonesPoly(msname,MSnAnt,MSnSpw)"));
  //    return new GJonesSpline(msname,MSnAnt,MSnSpw);
  
  else if (uptype=="TF" || uptype=="TF JONES" || uptype=="TF NOISE") 
    return new TfJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="T" || uptype=="T JONES" || uptype=="T NOISE") 
    return new TJones(msname,MSnAnt,MSnSpw);

  else if (uptype.before('+')=="DLIN" ||
	   uptype.before('+')=="D" || 
	   uptype=="D JONES") 
    return new DlinJones(msname,MSnAnt,MSnSpw);

  else if (uptype.before('+')=="DFLIN" || 
	   uptype.before('+')=="DF" || 
	   uptype=="DF JONES") 
    return new DflinJones(msname,MSnAnt,MSnSpw);

  else if (uptype.before('+')=="DLLS" || 
	   uptype=="DLLS JONES")
    return new DllsJones(msname,MSnAnt,MSnSpw);

  else if (uptype.before('+')=="DFLLS" || 
	   uptype=="DFLLS JONES")
    return new DfllsJones(msname,MSnAnt,MSnSpw);

  else if (uptype.before('+')=="DGEN" ||
	   uptype.before('+')=="DGENERAL" || 
	   uptype=="DGEN JONES") 
    return new DJones(msname,MSnAnt,MSnSpw);

  else if (uptype.before('+')=="DFGEN" || 
	   uptype.before('+')=="DFGENERAL" || 
	   uptype=="DFGEN JONES") 
    return new DfJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="J" || uptype=="J JONES") 
    return new JJones(msname,MSnAnt,MSnSpw);

  else if (uptype == "EP" || uptype == "EP JONES")
    throw(AipsError(uptype+" not yet supported via BJonesPoly(msname,MSnAnt,MSnSpw)"));
  //    return new EPJones(msname,MSnAnt,MSnSpw);

  else if (uptype == "LJ" || uptype == "LJ JONES")
    throw(AipsError(uptype+" not yet supported via BJonesPoly(msname,MSnAnt,MSnSpw)"));
  //    return new LJJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="M" || uptype=="M MUELLER")
    return new MMueller(msname,MSnAnt,MSnSpw);

  else if (uptype=="A" || uptype=="A MUELLER")
    return new AMueller(msname,MSnAnt,MSnSpw);

  else if (uptype=="N" || uptype=="A NOISE")
    return new ANoise(msname,MSnAnt,MSnSpw);

  else if (uptype=="MF" || uptype=="MF MUELLER")
    return new MfMueller(msname,MSnAnt,MSnSpw);
     
  else if (uptype=="X" || uptype=="X MUELLER")
    return new XMueller(msname,MSnAnt,MSnSpw);

  else if (uptype=="XJ" || uptype=="X JONES")
    return new XJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="XF" || uptype=="XF JONES")
    return new XfJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="K" || uptype=="K JONES")
    return new KJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="KCROSS" || uptype=="KCROSS JONES")
    return new KcrossJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="GLINXPH" || uptype=="GLINXPH JONES" ||
	   uptype=="XY+QU")
    return new GlinXphJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="GLINXPHF" || uptype=="GLINXPHF JONES" ||
	   uptype=="XYF+QU")
    return new GlinXphfJones(msname,MSnAnt,MSnSpw);

  else if (uptype=="KMBD" || uptype=="KMBD JONES")
    return new KMBDJones(msname,MSnAnt,MSnSpw);

  else if (uptype.contains("KANTPOS") || uptype.contains("KANTPOS JONES"))
    return new KAntPosJones(msname,MSnAnt,MSnSpw);

  else if (uptype.contains("TSYS"))
    return new StandardTsys(msname,MSnAnt,MSnSpw);

  else if (uptype.contains("EVLASWP"))
    return new EVLASwPow(msname,MSnAnt,MSnSpw);

  else if (uptype=="TFOPAC")  // Not yet solvable (even though an SVJ)
    return new TfOpac(msname,MSnAnt,MSnSpw);

  else if (uptype=="TOPAC")  // Not yet solvable (even though an SVJ)
    return new TOpac(msname,MSnAnt,MSnSpw);

  else if (uptype.contains("GAINCURVE"))  // Not yet solvable (even though an SVJ)
    return new EGainCurve(msname,MSnAnt,MSnSpw);
  
  else if (uptype.contains("EVLAGAIN"))
    throw(AipsError("Please regenerate EVLA Sw Pow table using gencal."));

  else if (uptype.contains("TEC") || uptype.contains("F JONES") )  // Ionosphere
    return new FJones(msname,MSnAnt,MSnSpw);

  else if (uptype.contains("SDSKY_PS"))
    throw(AipsError(uptype+" not yet supported via BJonesPoly(msname,MSnAnt,MSnSpw)"));
  //    return new SingleDishPositionSwitchCal(msname,MSnAnt,MSnSpw);

  else if (uptype.contains("SDSKY_RASTER"))
    throw(AipsError(uptype+" not yet supported via BJonesPoly(msname,MSnAnt,MSnSpw)"));
  //    return new SingleDishRasterCal(msname,MSnAnt,MSnSpw);

  else if (uptype.contains("SDSKY_OTF"))
    throw(AipsError(uptype+" not yet supported via BJonesPoly(msname,MSnAnt,MSnSpw)"));
  //    return new SingleDishOtfCal(msname,MSnAnt,MSnSpw);
  
  else {
    cout << "attempted type = " << type << endl;
    throw(AipsError("Unknown calibration type."));
  }
};

// Create a specialized SolvableVisCal from nAnt
SolvableVisCal* createSolvableVisCal(const String& type, const Int& nAnt) {

  if      (type=="B") 
    return new BJones(nAnt);

  else if (type=="G") 
    return new GJones(nAnt);

  else if (type=="T") 
    return new TJones(nAnt);
  
  else if (type=="D") 
    return new DJones(nAnt);

  else if (type=="Df") 
    return new DfJones(nAnt);

  else if (type=="J") 
    return new JJones(nAnt);
     
  else {
    cout << "attempted type = " << type << endl;
    throw(AipsError("Unknown calibration type."));
  }
};

} //# NAMESPACE CASA - END
