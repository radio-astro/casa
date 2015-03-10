//# LatticeAsVector.cc: Class to display lattice objects as vector fields
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/aips.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>
#include <lattices/LEL/LatticeExprNode.h>
#include <lattices/Lattices/LatticeLocker.h>
#include <lattices/Lattices/MaskedLattice.h>

#include <display/DisplayDatas/LatticeAsVector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <>
	Float LatticeAsVector<Complex>::getVariance() {
		SHARED_PTR<MaskedLattice<Complex> > ml = maskedLattice();
		Float clip = 10.0;
		Float realVar = 0.0;
		Float imagVar = 0.0;
		{
			LatticeExprNode n1(real(*ml));
			LatticeExprNode n2(n1[abs(n1-mean(n1)) < clip*stddev(n1)]);
			LatticeExprNode n3(variance(n2));
			realVar = n3.getFloat();
		}
		{
			LatticeExprNode n1(imag(*ml));
			LatticeExprNode n2(n1[abs(n1-mean(n1)) < clip*stddev(n1)]);
			LatticeExprNode n3(variance(n2));
			imagVar = n3.getFloat();
		}
		return (realVar + imagVar) / 2.0;
	}

	template <>
	Float LatticeAsVector<Float>::getVariance()
//
// Never called
//
	{
		return 0.0;
	}

	template <>
	const Float LatticeAsVector<Float>::dataValue(IPosition pos) {

		// Return the value of the Lattice at a particular position.
		// Overrides the base method to take itsRotation (user-specified
		// addition to position angle) into account as well.

		Float val = LatticePADisplayData<Float>::dataValue(pos);
		if(itsRotation==0.) return val;

		Float rotation = itsRotation*C::pi/180.;
		// Computation is done in radians (itsRotation is in degrees).

		Float toRadians = (itsUnits==Unit("rad"))?
		                  itsUnits.getValue().getFac() : C::pi/180.;
		// Assume Float data is in degrees unless an angle Unit is provided.

		if(toRadians!=0.) val = (val*toRadians + rotation)/toRadians;

		return remainder(val, 360.);
		// remainder (from math.h) actually assures that val is in the
		// range [-180,+180], (not [0,360]).
	}


	template <>
	const Complex LatticeAsVector<Complex>::dataValue(IPosition pos) {

		// This is only a half-hearted stab (there are no Image<Complex>'s yet).
		// When they're implemented, showValue() should also be modified to
		// display this Complex value in polar coordinates....

		Complex val = LatticePADisplayData<Complex>::dataValue(pos);
		if(itsRotation==0.) return val;

		Float rotation = itsRotation*C::pi/180.;
		// Computation is done in radians (itsRotation is in degrees).

		val = polar(abs(val), arg(val)+rotation);

		return val;
	}


} //# NAMESPACE CASA - END

