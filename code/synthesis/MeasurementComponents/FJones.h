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
  virtual String typeName()     { return "F Jones"; };
  virtual String longTypeName() { return "F Jones (Ionosphere)"; };

  // Report current Jones type (circ=diag, lin=general)
  Jones::JonesType jonesType() { return pjonestype_; };

  // Par is freq-INdep, but matrix is freq-dep
  virtual Bool freqDepPar() { return False; };
  virtual Bool freqDepMat() { return True; };

  // We have Float parameters
  virtual VisCalEnum::VCParType parType() { return VisCalEnum::REAL; };

  // Local setapply to enforce calwt=F and spwmap=[0] for FJones
  virtual void setApply(const Record& apply);
  using SolvableVisJones::setApply;

  // Report apply-related info
  String applyinfo();

  // Local specializations for TEC generation
  virtual void setSpecify(const Record& specify);
  virtual void specify(const Record& specify);

protected:

  // FJones has one Float par per ant
  virtual Int nPar() { return 1; };

  // Jones matrix elements are NOT trivial
  virtual Bool trivialJonesElem() { return False; };

  // Access to z.a. data
  Vector<Double>& za() { return za_; };

  // Locally set which feed basis (Jones type)
  void syncJones(const Bool& doInv);

  // Calculate parameters (in this case, the z.a.)
  virtual void calcPar();

  // Calculate the FJones matrix for all ants
  virtual void calcAllJones();

private:

  // TEC specify/retrieval mode
  String tectype_;

  // Field calculation components
  MeasFrame mframe_;
  EarthMagneticMachine *emm_;

  // Fiducial ionosphere height
  Quantity ionhgt_;

  // TEC image filename
  String tecimname_;

  // zenith angle per antenna
  Vector<Double> za_;

  // The magnetic field
  Vector<Double> BlosG_;

  // Units
  Double radper_;

  // Lin (general) or Circ (diag)
  Jones::JonesType pjonestype_;

};

} //# NAMESPACE CASA - END

#endif

