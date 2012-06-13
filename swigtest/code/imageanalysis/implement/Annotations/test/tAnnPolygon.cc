//# tWCLELMask.cc:  mechanical test of the WCLELMask class
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
//# $Id: tWCLELMask.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#include <casa/aips.h>
#include <imageanalysis/Annotations/AnnPolygon.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <casa/namespace.h>
#include <iomanip>


int main () {
	try {
		IPosition shape(4, 4000, 4000, 1, 1420601);
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		AnnRegion::unitInit();
		LogIO log;

		{
			log << LogIO::NORMAL
				<< "Test mixed world and pixel coordinates throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Vector<Quantity> x(3);
			Vector<Quantity> y(3);

			x[0] = Quantity(0.01, "deg");
			y[0] = Quantity(0, "deg");
			x[1] = Quantity(0, "deg");
			y[1] = Quantity(0.01, "pix");
			x[2] = Quantity(0, "deg");
			y[2] = Quantity(0, "deg");

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
				AnnPolygon poly(
					x, y, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
				);
				thrown = False;
			} catch (AipsError x) {
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
			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(0.01, "km/s");
			y[0] = Quantity(0, "deg");
			x[1] = Quantity(0, "deg");
			y[1] = Quantity(0.01, "deg");
			x[2] = Quantity(0, "deg");
			y[2] = Quantity(0, "deg");

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
				AnnPolygon poly(
					x, y, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
				);
				thrown = False;
			} catch (AipsError x) {
				log << LogIO::NORMAL
					<< "Exception thrown as expected: "
					<< x.getMesg() << LogIO::POST;
			}
			AlwaysAssert(thrown, AipsError);

		}
		/*
		{
			log << LogIO::NORMAL << "Test getBoundingBox and getPixelBox"
				<< LogIO::POST;
			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(-3, "arcmin");
			y[0] = Quantity(3, "arcmin");
			x[1] = Quantity(-0.9, "arcmin");
			y[1] = Quantity(0.6, "arcmin");
			x[2] = Quantity(-0.9, "arcmin");
			y[2] = Quantity(0.9, "arcmin");

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
			AnnPolygon poly(
				x, y, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			vector<Quantity> wblc, wtrc;
			poly.worldBoundingBox(wblc, wtrc);
			AlwaysAssert(near(wblc[0].getValue("arcmin"), -0.9, 1e-6), AipsError);
			AlwaysAssert(near(wblc[1].getValue("arcmin"), 0.6, 1e-6), AipsError);
			AlwaysAssert(near(wtrc[0].getValue("arcmin"), -3.0, 1e-6), AipsError);
			AlwaysAssert(near(wtrc[1].getValue("arcmin"), 3.0, 1e-6), AipsError);

			vector<Double> pblc, ptrc;
			poly.pixelBoundingBox(pblc, ptrc);
			AlwaysAssert(pblc[0] < ptrc[0], AipsError);
			AlwaysAssert(pblc[1] < ptrc[1], AipsError);

			AlwaysAssert(near(pblc[0], (-1)*wblc[0].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(pblc[1], wblc[1].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(ptrc[0], (-1)*wtrc[0].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(ptrc[1], wtrc[1].getValue("arcmin"), 3e-6), AipsError);

			vector<Quantity> wx, wy;
			poly.worldVertices(wx, wy);
			AlwaysAssert(near(wx[0].getValue("arcmin"), -3.0), AipsError);
			AlwaysAssert(near(wy[0].getValue("arcmin"), 3.0), AipsError);
			AlwaysAssert(near(wx[1].getValue("arcmin"), -0.9), AipsError);
			AlwaysAssert(near(wy[1].getValue("arcmin"), 0.6), AipsError);
			AlwaysAssert(near(wx[2].getValue("arcmin"), -0.9), AipsError);
			AlwaysAssert(near(wy[2].getValue("arcmin"), 0.9), AipsError);

			vector<Double> px, py;
			poly.pixelVertices(px, py);
			AlwaysAssert(near(px[0], (-1)*wx[0].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(py[0], wy[0].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(px[1], (-1)*wx[1].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(py[1], wy[1].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(px[2], (-1)*wx[2].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(py[2], wy[2].getValue("arcmin"), 3e-6), AipsError);
		}
		*/
		{
			log << LogIO::NORMAL << "Verify corners"
				<< LogIO::POST;
			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(-0.05, "deg");
			y[0] = Quantity(0.05, "deg");
			x[1] = Quantity(-0.015, "deg");
			y[1] = Quantity(0.01, "deg");
			x[2] = Quantity(-0.015, "deg");
			y[2] = Quantity(0.05, "deg");

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
			AnnPolygon poly(
				x, y, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MDirection> corners = poly.getCorners();
			AlwaysAssert(corners.size() == 3, AipsError);

			AlwaysAssert(
				near(
					corners[0].getAngle("deg").getValue("deg")[0],
					x[0].getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[0].getAngle("deg").getValue("deg")[1],
					y[0].getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[1].getAngle("deg").getValue("deg")[0],
					x[1].getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[1].getAngle("deg").getValue("deg")[1],
					y[1].getValue("deg")
				), AipsError
			);
			cout << poly.getCorners() << endl;
		}
		Vector<Double> refVal = csys.referenceValue();
		refVal[0] = 100;
		refVal[1] = 0.0;
		csys.setReferenceValue(refVal);
		{
			log << LogIO::NORMAL
				<< "Test precessing from B1950 to J2000"
				<< LogIO::POST;

			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(0.05, "deg");
			y[0] = Quantity(0, "deg");
			x[1] = Quantity(0.015, "deg");
			y[1] = Quantity(0.01, "deg");
			x[2] = Quantity(0.015, "deg");
			y[2] = Quantity(0, "deg");

			Quantity beginFreq, endFreq;
			String dirTypeString = "B1950";
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
			AnnPolygon poly(
				x, y, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			Vector<MDirection> corners = poly.getCorners();
			AlwaysAssert(corners.size() == 3, AipsError);
			AlwaysAssert(
				near(
					corners[0].getAngle("rad").getValue("rad")[0],
					0.012055422536187882
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[0].getAngle("rad").getValue("rad")[1],
					0.00485808148440817
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[1].getAngle("rad").getValue("rad")[0],
					0.011444556041464694
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[1].getAngle("rad").getValue("rad")[1],
					0.0050326323941514792
				), AipsError
			);
			cout << poly.getCorners() << endl;
		}

		{
			log << LogIO::NORMAL
				<< "Test unmodified frequencies"
				<< LogIO::POST;

			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(0.05, "deg");
			y[0] = Quantity(0, "deg");
			x[1] = Quantity(0.015, "deg");
			y[1] = Quantity(0.01, "deg");
			x[2] = Quantity(0.015, "deg");
			y[2] = Quantity(0, "deg");

			Quantity beginFreq(1415, "MHz");
			Quantity endFreq(1450e6, "Hz");

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
			AnnPolygon poly(
				x, y, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = poly.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), beginFreq.getValue("Hz")),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), endFreq.getValue("Hz")),
				AipsError
			);
		}
		csys.setReferenceValue(Vector<Double>(4, 0));

		{
			log << LogIO::NORMAL
				<< "Test frequencies GALACTO -> LSRK"
				<< LogIO::POST;

			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(-0.05, "deg");
			y[0] = Quantity(0, "deg");
			x[1] = Quantity(-0.015, "deg");
			y[1] = Quantity(0.01, "deg");
			x[2] = Quantity(-0.015, "deg");
			y[2] = Quantity(0, "deg");

			Quantity beginFreq(1415, "MHz");
			Quantity endFreq(1450e6, "Hz");

			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			String freqRefFrameString = "GALACTO";
			String dopplerString = MDoppler::showType(
				csys.spectralCoordinate().velocityDoppler()
			);
			Quantity restfreq(
				csys.spectralCoordinate().restFrequency(), "Hz"
			);
			Vector<Stokes::StokesTypes> stokes(0);
			AnnPolygon poly(
				x, y, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = poly.getFrequencyLimits();
			cout << std::setprecision(10) << "freqs " << freqs[0].get("Hz").getValue() << endl;
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 1415508785.4853702),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 1450521370.2853618),
				AipsError
			);
		}

		{
			log << LogIO::NORMAL
				<< "Test unmodified frequencies when specifying relativistic velocities"
				<< LogIO::POST;

			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(-0.05, "deg");
			y[0] = Quantity(0, "deg");
			x[1] = Quantity(-0.015, "deg");
			y[1] = Quantity(0.01, "deg");
			x[2] = Quantity(-0.015, "deg");
			y[2] = Quantity(0, "deg");

			Quantity beginFreq(250000000, "m/s");
			Quantity endFreq(-250000, "km/s");

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
			AnnPolygon poly(
				x, y, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = poly.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 2604896650.3078709),
				AipsError
			);
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 235914853.26413003),
				AipsError
			);
		}

		{
			log << LogIO::NORMAL
				<< "Test unmodified frequencies when specifying velocities"
				<< LogIO::POST;

			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(-0.05, "deg");
			y[0] = Quantity(0, "deg");
			x[1] = Quantity(-0.015, "deg");
			y[1] = Quantity(0.01, "deg");
			x[2] = Quantity(-0.015, "deg");
			y[2] = Quantity(0, "deg");

			Quantity beginFreq(20000, "m/s");
			Quantity endFreq(-20, "km/s");

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
			AnnPolygon poly(
				x, y, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			Vector<MFrequency> freqs = poly.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 1420500511.0578821),
				AipsError
			);
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 1420310992.5141187),
				AipsError
			);
		}

		{
			log << LogIO::NORMAL
				<< "Test modified doppler definitions"
				<< LogIO::POST;
			Vector<Quantity> x(3);
			Vector<Quantity> y(3);
			x[0] = Quantity(-0.05, "deg");
			y[0] = Quantity(0, "deg");
			x[1] = Quantity(-0.015, "deg");
			y[1] = Quantity(0.01, "deg");
			x[2] = Quantity(-0.015, "deg");
			y[2] = Quantity(0, "deg");

			Quantity beginFreq(2013432.1736247784, "m/s");
			Quantity endFreq(-1986.7458583077, "km/s");

			String dirTypeString = MDirection::showType(
				csys.directionCoordinate().directionType(False)
			);
			String freqRefFrameString = MFrequency::showType(
				csys.spectralCoordinate().frequencySystem()
			);
			String dopplerString = "OPTICAL";
			Quantity restfreq(
				csys.spectralCoordinate().restFrequency(), "Hz"
			);
			Vector<Stokes::StokesTypes> stokes(0);
			AnnPolygon poly(
				x, y, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			cout << poly << endl;

			Vector<MFrequency> freqs = poly.getFrequencyLimits();
			cout << freqs[0].get("Hz").getValue() << endl;
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 1410929824.5978253),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 1429881678.974175),
				AipsError
			);
		}
		{
			log << LogIO::NORMAL
				<< "Test that object construction using Darrell's parameters no longer segfaults"
				<< LogIO::POST;
			Vector<Quantity> x(5);
			Vector<Quantity> y(5);
			IPosition shape1(4, 400, 400, 1, 1);
			x[0] = Quantity(4.94799, "rad"); y[0] = Quantity(0.0352309, "rad");
			x[1] = Quantity(4.94806, "rad"); y[1] = Quantity(0.0352925, "rad");
			x[2] = Quantity(4.94798, "rad"); y[2] = Quantity(0.0353659, "rad");
			x[3] = Quantity(4.94786, "rad"); y[3] = Quantity(0.0353345, "rad");
			x[4] = Quantity(4.94800, "rad"); y[4] = Quantity(0.0353174, "rad");
			Vector<Stokes::StokesTypes> stokes(1);
			stokes[0] = Stokes::I;
			try {

				AnnPolygon poly(
					x, y,
					csys, shape1, stokes
				);
			}
			catch (AipsError) {
				// this will throw an exception but we don't care
				// we just don't want it to segfault
			}

		}

	} catch (AipsError x) {
		cerr << "Caught exception: " << x.getMesg() << endl;
		return 1;
	}

	cout << "OK" << endl;
	return 0;
}
