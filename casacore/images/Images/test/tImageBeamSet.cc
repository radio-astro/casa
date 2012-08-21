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
		{
			cout << "*** Test constructors" << endl;
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
		{
			cout << "*** test setBeam()" << endl;
			GaussianBeam beam0(Quantity(4, "arcsec"), Quantity(3, "arcsec"), Quantity(20, "deg"));
			IPosition shape(2, 3, 4);
			Vector<ImageBeamSet::AxisType> types(2);
			types[0] = ImageBeamSet::SPECTRAL;
			types[1] = ImageBeamSet::POLARIZATION;
			ImageBeamSet x(beam0, shape, types);
			GaussianBeam beam1(Quantity(5, "arcsec"), Quantity(4, "arcsec"), Quantity(20, "deg"));
			IPosition pos2(2, 1, 2);
			x.setBeam(beam1, pos2);
			for (IPosition pos(2,0,0); pos != shape; pos.next(shape)) {
				GaussianBeam beam = x.getBeam(pos);
				if (pos == pos2) {
					AlwaysAssert(beam == beam1, AipsError);
				}
				else {
					AlwaysAssert(beam == beam0, AipsError);
				}
			}
			{
				cout << "*** test getting max and min area beams" << endl;
				GaussianBeam init(
					Quantity(4, "arcsec"), Quantity(2, "arcsec"),
					Quantity(0, "deg")
				);
				IPosition shape(2, 3, 4);
				Vector<ImageBeamSet::AxisType> types(2);
				types[0] = ImageBeamSet::SPECTRAL;
				types[1] = ImageBeamSet::POLARIZATION;
				ImageBeamSet x(init, shape, types);
				AlwaysAssert(x.getMaxAreaBeam() == init, AipsError);
				AlwaysAssert(x.getMinAreaBeam() == init, AipsError);

				GaussianBeam maxBeam(
					Quantity(10, "arcsec"), Quantity(8, "arcsec"),
					Quantity(0, "deg")
				);
				GaussianBeam minBeam(
					Quantity(1, "arcsec"), Quantity(1, "arcsec"),
					Quantity(0, "deg")
				);
				IPosition maxBeamPos(2, 2, 1);
				IPosition minBeamPos(2, 2, 3);
				x.setBeam(maxBeam, maxBeamPos);
				x.setBeam(minBeam, minBeamPos);
				AlwaysAssert(x.getMaxAreaBeam() == maxBeam, AipsError);
				AlwaysAssert(x.getMinAreaBeam() == minBeam, AipsError);
				AlwaysAssert(x.getMaxAreaBeamPosition() == maxBeamPos, AipsError);
				AlwaysAssert(x.getMinAreaBeamPosition() == minBeamPos, AipsError);
			}
		}
	}
	catch (const AipsError& x) {
		cout << x.getMesg() << endl;
		cout << "FAIL" << endl;
		return 1;
	}
	cout << "OK" << endl;
	return 0;
}

