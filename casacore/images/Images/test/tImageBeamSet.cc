//# tImageConcat.cc: This program tests the ImageConcat class
//# Copyright (C) 1996,1997,1999,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#


#include <casa/aips.h>

#include <images/Images/ImageBeamSet.h>

#include <casa/iostream.h>



#include <casa/namespace.h>

int main() {
	try {
		// empty beam set
		ImageBeamSet x;
		AlwaysAssert(x.empty(), AipsError);
		AlwaysAssert(x.size() == 0, AipsError);
		AlwaysAssert(x.nelements() == 0, AipsError);
		AlwaysAssert(x.getAxes().size() == 0, AipsError);
		GaussianBeam beam(
			Quantity(4, "arcsec"), Quantity(3, "arcsec"),
			Quantity(40, "deg")
		);
		Vector<ImageBeamSet::AxisType> types(2);
		types[0] = ImageBeamSet::SPECTRAL;
		types[1] = ImageBeamSet::POLARIZATION;
		ImageBeamSet b(IPosition(2, 20, 4), types);

		b.set(beam);
		ImageBeamSet c = b;
		AlwaysAssert(b == b, AipsError);
		AlwaysAssert(c == b, AipsError);
		ImageBeamSet d(b);
		AlwaysAssert(d == b, AipsError);
		c = x;
		x = b;
		AlwaysAssert(x == b, AipsError);

        ImageBeamSet k(beam);
	}
	catch (AipsError x) {
		cout << x.getMesg() << endl;
		cout << "FAIL" << endl;
		return 1;
	}
	cout << "OK" << endl;
	return 0;
}

