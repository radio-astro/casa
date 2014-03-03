//# FJones.h:  Declaration of Ionospheric corrections
//# Copyright (C) 1996,1997,2000,2001,2002,2003,2011,2014
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

#ifndef SYNTHESIS_FJONES_H
#define SYNTHESIS_FJONES_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
//#include <synthesis/MeasurementComponents/CalCorruptor.h>

#include <measures/Measures/EarthMagneticMachine.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class FJones : public SolvableVisJones {
public:

  // Constructor
  FJones(VisSet& vs);
  //  FJones(const Int& nAnt);  // NYI

  virtual ~FJones();

  // Return the type enum
  virtual Type type() { return VisCal::F; };

  // Return type name as string
  virtual String typeName()     { return "FJones"; };
  virtual String longTypeName() { return "FJones (Ionosphere)"; };

  // Type of Jones matrix according to nPar()
  Jones::JonesType jonesType() { return Jones::Diagonal; };

  // Par is freq-INdep, but matrix is freq-dep
  virtual Bool freqDepPar() { return False; };
  virtual Bool freqDepMat() { return True; };

  // We have Float parameters
  virtual VisCalEnum::VCParType parType() { return VisCalEnum::REAL; };

  // Report apply-related info
  String applyinfo();

protected:

  // FJones has one Float par per ant
  virtual Int nPar() { return 1; };

  // Jones matrix elements are NOT trivial
  virtual Bool trivialJonesElem() { return False; };

  // Access to z.a. data
  Vector<Double>& za() { return za_; };

  // Calculate parameters (in this case, the z.a.)
  virtual void calcPar();

  // Calculate the FJones matrix for all ants
  virtual void calcAllJones();

private:

  // Field calculation components
  MeasFrame mframe_;
  EarthMagneticMachine *emm_;

  // Fiducial ionosphere height
  Quantity ionhgt_;

  // zenith angle per antenna
  Vector<Double> za_;

  // The magnetic field
  Vector<Double> BlosG_;

  // Units
  Double radper_;

};

} //# NAMESPACE CASA - END

#endif

