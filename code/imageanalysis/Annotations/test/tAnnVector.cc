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
#include <imageanalysis/Annotations/AnnVector.h>

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
			Quantity blcx(0.01, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0, "deg");
			Quantity trcy(0.01, "pix");

			Quantity beginFreq, endFreq;
			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			String freqRefFrameString = MFrequency::showType(
				csys.spectralCoordinate().frequencySystem()
			);
			String dopplerString = MDoppler::showType(
				csys.spectralCoordinate().velocityDoppler()
			);
			Quantity restfreq(
				csys.spectralCoordinate().restFrequency(), "Hz"
			);
			Vector<Stokes::StokesTypes> stokes(0);
			try {
				AnnVector vector(
					blcx, blcy, trcx, trcy,
					csys, Vector<Stokes::StokesTypes>(0)
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
				<< "Test bad quantity for world direction coordinate throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity blcx(0.01, "km/s");
			Quantity blcy(0, "deg");
			Quantity trcx(0, "deg");
			Quantity trcy(0.01, "deg");
			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			try {
				AnnVector vector(
					blcx, blcy, trcx, trcy,
					csys, Vector<Stokes::StokesTypes>(0)
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
			Quantity blcx(0.05, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0.015, "deg");
			Quantity trcy(0.01, "deg");
			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			AnnVector vector(
				blcx, blcy, trcx, trcy, csys,
				Vector<Stokes::StokesTypes>(0)
			);

			Vector<MDirection> points = vector.getEndPoints();
			AlwaysAssert(
				near(
					points[0].getAngle("deg").getValue("deg")[0],
					blcx.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					points[0].getAngle("deg").getValue("deg")[1],
					blcy.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					points[1].getAngle("deg").getValue("deg")[0],
					trcx.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					points[1].getAngle("deg").getValue("deg")[1],
					trcy.getValue("deg")
				), AipsError
			);
		}
		{
			log << LogIO::NORMAL
				<< "Test precessing from B1950 to J2000"
				<< LogIO::POST;
			Quantity blcx(0.05, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0.015, "deg");
			Quantity trcy(0.01, "deg");
			String dirTypeString = "B1950";
			AnnVector vector(
				blcx, blcy, trcx, trcy, dirTypeString,
				csys, Quantity(0, "Hz"), Quantity(0, "Hz"),
				"", "", Quantity(0, "Hz"), Vector<Stokes::StokesTypes>(0)
			);
			cout << vector << endl;
			Vector<MDirection> points = vector.getEndPoints();
			AlwaysAssert(
				near(
					points[0].getAngle("rad").getValue("rad")[0],
					0.012055422536187882
				), AipsError
			);
			AlwaysAssert(
				near(
					points[0].getAngle("rad").getValue("rad")[1],
					0.00485808148440817
				), AipsError
			);
			AlwaysAssert(
				near(
					points[1].getAngle("rad").getValue("rad")[0],
					0.011444556041464694
				), AipsError
			);
			AlwaysAssert(
				near(
					points[1].getAngle("rad").getValue("rad")[1],
					0.0050326323941514792
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
