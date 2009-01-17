//# AxisUnit.h: Visibility plotting object.
//# Copyright (C) 2008
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#
//# $Id:  $
#ifndef AXISUNIT_H_
#define AXISUNIT_H_

#include <casa/Exceptions.h>
#include <casa/BasicSL/String.h>

#include <map>

#include <casa/namespace.h>

namespace casa {

// Rob stuff, not currently used. //

// <summary>Maps quantities plottable by PlotMS into basic unit types, for
// checking whether they can be plotted on the same plot.</summary>

enum AxisUnitEnum {Angle,	      // Not Dimensionless because it can
		                      // include degrees, mas, radians, etc.
		   Dimensionless,     // Things that you'd have to stretch to
		                      // tack a unit onto.
		   FluxDensity,       // (m, u)Jy
		   Freq,	      // VisSet.h typedefs Frequency to Double.
		   Intensity,         // Jy/beam, Jy/sq. arcsec, etc.
		   Time,
		   Velocity,
                   Wavelengths};      // (u, v, w) distances.

class AxisUnit
{
public:
  AxisUnitEnum operator[](const String& quant);

  // You want the AxisUnit, you go through this.  Call like AxisUnit::lookup().['time'].
  static AxisUnit& lookup();
  
private:
  // Default constructor.  Because it is private, instantiation of AxisUnits is suppressed.
  AxisUnit();

  // There can only be one AxisUnit, so there is no copy constructor or
  // assignment operator.

  // Destructor
  ~AxisUnit() {}

  typedef std::map<String, AxisUnitEnum> S2UMap;
  S2UMap quant2u;
};
  
class AxisUnitException: public AipsError {
public:
  //
  // Creates an AxisUnitException and initializes the error message from
  // the parameter
  // <group>
  AxisUnitException(const Char *str) : AipsError(str, AipsError::INVALID_ARGUMENT) {}
  AxisUnitException(const String &str) : AipsError(str,
						   AipsError::INVALID_ARGUMENT) {}
  
  // </group>

  // Destructor which does nothing.
  ~AxisUnitException() throw() {}
};
  
}

#endif /* AXISUNIT_H_ */
