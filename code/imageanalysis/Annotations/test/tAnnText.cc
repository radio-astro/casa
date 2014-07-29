//# Copyright (C) 2000,2001,2003
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
#include <imageanalysis/Annotations/AnnText.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <casa/namespace.h>

int main () {
	LogIO log;
	try {
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		AnnotationBase::unitInit();
		{
			log << LogIO::NORMAL
				<< "mixed world and pixel coordinates throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity x(0.01, "pix");
			Quantity y(0, "deg");
			String t = "blah";

			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			try {
				AnnText text(
					x, y,
					csys, t, Vector<Stokes::StokesTypes>(0)
				);
				thrown = False;
			} catch (const AipsError& x) {
				log << LogIO::NORMAL
					<< "Exception thrown as expected: " << x.getMesg()
					<< LogIO::POST;
			}
			AlwaysAssert(thrown, AipsError);
		}

		{
			log << LogIO::NORMAL
				<< "Test bad quantity for world direction coordinate throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity x(0.01, "km/s");
			Quantity y(0, "deg");
			String t = "blah";
			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			try {
				AnnText text(
					x, y,
					csys, t, Vector<Stokes::StokesTypes>(0)
				);
				thrown = False;
			} catch (const AipsError& x) {
				log << LogIO::NORMAL
					<< "Exception thrown as expected: "
					<< x.getMesg() << LogIO::POST;
			}
			AlwaysAssert(thrown, AipsError);
		}
		{
			log << LogIO::NORMAL
				<< "Test coordinates with no conversion"
				<< LogIO::POST;
			Quantity x(0.05, "deg");
			Quantity y(0, "deg");
			String t = "blah";
			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			AnnText text(
				x, y,
				csys, t, Vector<Stokes::StokesTypes>(0)
			);

			MDirection point = text.getDirection();
			AlwaysAssert(
				near(
					point.getAngle("deg").getValue("deg")[0],
					x.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					point.getAngle("deg").getValue("deg")[1],
					y.getValue("deg")
				), AipsError
			);
		}
		{
			log << LogIO::NORMAL
				<< "Test precessing from B1950 to J2000"
				<< LogIO::POST;
			Quantity x(0.05, "deg");
			Quantity y(0, "deg");
			String t = "blah";
			String dirTypeString = "B1950";
			AnnText text(
				x, y, dirTypeString,
				csys, t, Quantity(0, "Hz"), Quantity(0, "Hz"), "", "", Quantity(0, "Hz"),
				Vector<Stokes::StokesTypes>(0)
			);
			cout << text << endl;
			MDirection point = text.getDirection();
			AlwaysAssert(
				near(
					point.getAngle("rad").getValue("rad")[0],
					0.012055422536187882
				), AipsError
			);
			AlwaysAssert(
				near(
					point.getAngle("rad").getValue("rad")[1],
					0.00485808148440817
				), AipsError
			);
		}
	} catch (AipsError x) {
		log << LogIO::SEVERE
			<< "Caught exception: " << x.getMesg()
			<< LogIO::POST;
		return 1;
	}

	cout << "OK" << endl;
	return 0;
}
