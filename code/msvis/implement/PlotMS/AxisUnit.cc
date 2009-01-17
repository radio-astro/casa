//# AxisUnit.cc: Visibility plotting object.
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
#include <msvis/PlotMS/AxisUnit.h>

namespace casa { //# NAMESPACE CASA - BEGIN

AxisUnit::AxisUnit()
{
  quant2u["amp"]	      = FluxDensity;
  quant2u["azimuth"]	      = Angle;
  quant2u["baseline"]	      = Dimensionless;
  quant2u["channel"]	      = Dimensionless;
  quant2u["corr"]	      = Dimensionless;
  quant2u["elevation"]	      = Angle;
  quant2u["frequency"]	      = Freq;
  quant2u["hourangle"]	      = Angle;
  quant2u["imag"]	      = FluxDensity;
  quant2u["parallacticangle"] = Angle;
  quant2u["phase"]	      = Angle;
  quant2u["real"]	      = FluxDensity;
  quant2u["time"]	      = Time;  
  quant2u["u"]		      = Wavelengths;
  quant2u["uvdist"]	      = Wavelengths;
  quant2u["v"]		      = Wavelengths;
  quant2u["velocity"]	      = Velocity;
  quant2u["w"]		      = Wavelengths;
  quant2u["weight"]	      = Dimensionless;	// May not be right.
}
  
AxisUnitEnum AxisUnit::operator[](const String& quant)
{
  S2UMap::iterator it = quant2u.find(quant);
  
  if(it == quant2u.end())
    throw AxisUnitException(quant);
  
  return it->second;  
}

// You want the AxisUnit, you go through this.  Call like AxisUnit::lookup().['time'].
AxisUnit& AxisUnit::lookup()
{
  static AxisUnit au;  // THE instantiation of AxisUnit.
  return au;
}

} // End of casa namespace.
