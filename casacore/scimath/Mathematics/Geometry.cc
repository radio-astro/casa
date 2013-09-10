//# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
//# Copyright (C) 1995,1996,1997,1998,1999,2000,2001,2002
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
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: Combinatorics.cc 21100 2011-06-28 12:49:00Z gervandiepen $
//   

#include <scimath/Mathematics/Geometry.h>

#include <utility>

namespace casa {

	std::pair<Double, Double> Geometry::rotate2D(
		Double x, Double y, const Quantity& theta
	) {
		Double thetaRad = theta.getValue("rad");
		Double c, s;
		sincos(thetaRad, &s, &c);
		return std::make_pair(x*c - y*s, x*s + y*c);
	}

} //# NAMESPACE CASA - END

