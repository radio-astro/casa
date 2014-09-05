//# TsysGainCal.h: Declaration of Tsys calibration
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

#ifndef SYNTHESIS_TSYSGAINCAL_H
#define SYNTHESIS_TSYSGAINCAL_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
// not yet:#include <synthesis/MeasurementComponents/CalCorruptor.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class VisEquation;
//not yet: class TJonesCorruptor;


// **********************************************************
//  Standard Tsys Spectrum from SYSCAL table
//

class StandardTsys : public BJones {
public:

  // Constructor
  StandardTsys(VisSet& vs);
  StandardTsys(const Int& nAnt);

  virtual ~StandardTsys();

  // Return the type enum (for now, pretend we are B)
  virtual Type type() { return VisCal::B; };

  // Return type name as string (ditto)
  virtual String typeName()     { return "B TSYS"; };
  virtual String longTypeName() { return "B TSYS (freq-dep Tsys)"; };

  // Tsys are Float parameters
  virtual VisCalEnum::VCParType parType() { return VisCalEnum::REAL; };

  // Local setSpecify
  using BJones::setSpecify;
  virtual void setSpecify(const Record& specify);

  // Specific specify() that reads the SYSCAL subtable
  virtual void specify(const Record& specify);

  // In general, we are freq-dep
  virtual Bool freqDepPar() { return True; };


protected:

  // The Jones matrix elements are not the parameters
  //  ( j = sqrt(p) )
  virtual Bool trivialJonesElem() { return False; };

  // Specialized calcPar that does some sanity checking
  virtual void calcPar();

  // Invert doInv for Tsys corrections
  virtual void syncJones(const Bool& doInv) { BJones::syncJones(!doInv); };
  
  // Calculate Jones matrix elements from Tsys (j = sqrt(p) )
  virtual void calcAllJones();

  // Calculate weight scale
  virtual void calcWtScale();

private:

  // The name of the SYSCAL table
  String sysCalTabName_;

  // <nothing>


};

} //# NAMESPACE CASA - END

#endif

