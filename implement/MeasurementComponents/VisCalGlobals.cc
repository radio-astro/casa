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
#include <synthesis/MeasurementComponents/GSpline.h>
#include <synthesis/MeasurementComponents/BPoly.h>
#include <synthesis/MeasurementComponents/EJones.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Create a specialized VisCal from VisSet
VisCal* createVisCal(const String& type, VisSet& vs) {

  String uptype=type;
  uptype.upcase();

  if (type=="P" || type=="P JONES") 
    return new PJones(vs);

  else if (type=="TOPAC")  // Not yet solvable (even though an SVJ)
    return new TOpac(vs);

  else if (type=="GAINCURVE")  // Not yet solvable (even though an SVJ)
    return new EGainCurve(vs);
  
  else
    // Try request as a solvable type
    return createSolvableVisCal(type,vs);
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

  if      (type=="B" || type=="B JONES") 
    return new BJones(vs);

  else if (type=="BPOLY") 
    return new BJonesPoly(vs);

  else if (type=="G" || type=="G JONES") 
    return new GJones(vs);

  else if (type=="GSPLINE") 
    return new GJonesSpline(vs);
  
  else if (type=="T" || type=="T JONES") 
    return new TJones(vs);

  else if (type=="D" || type=="D JONES") 
    return new DJones(vs);

  else if (type=="DF" || type=="DF JONES") 
    return new DfJones(vs);

  else if (type=="J" || type=="J JONES") 
    return new JJones(vs);

  else if (type=="M" || type=="M MUELLER")
    return new MMueller(vs);

  else if (type=="MF" || type=="MF MUELLER")
    return new MfMueller(vs);
     
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
