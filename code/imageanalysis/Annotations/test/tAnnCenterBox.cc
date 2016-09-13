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
#include <imageanalysis/Annotations/AnnCenterBox.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <casa/namespace.h>

#include <iomanip>

int main () {
	try {
		LogIO log;
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		AnnRegion::unitInit();
		IPosition shape(4, 400,400,1,5500);
		{
			log << LogIO::NORMAL
				<<"Test mixed world and pixel coordinates throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "pix");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");

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
				AnnCenterBox box(
					centerx, centery, widthx, widthy, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
				);

				thrown = False;
			} catch (AipsError x) {
				log << LogIO::NORMAL << "Exception thrown as expected: "
					<< x.getMesg() << LogIO::POST;
			}
			AlwaysAssert(thrown, AipsError);
		}
		{
			log << LogIO::NORMAL
				<< "Test that bad quantity for world direction coordinate throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "cm");
			Quantity widthx(30, "arcsec");
						Quantity widthy(45, "arcsec");
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
				AnnCenterBox box(
					centerx, centery, widthx, widthy, dirTypeString,
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
			Quantity centerx(0.6, "arcmin");
			Quantity centery(1.2, "arcmin");
			Quantity widthx(6, "arcmin");
			Quantity widthy(8, "arcmin");

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
			AnnCenterBox box(
				centerx, centery, widthx, widthy, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			/*
			vector<Quantity> wblc, wtrc;
			box.worldBoundingBox(wblc, wtrc);
			AlwaysAssert(
				near(wblc[0].getValue("arcmin"), (centerx+widthx/2).getValue("arcmin")),
				AipsError
			);

			AlwaysAssert(
				near(wblc[1].getValue("arcmin"), (centery-widthy/2).getValue("arcmin")),
				AipsError
			);
			AlwaysAssert(
				near(wtrc[0].getValue("arcmin"), (centerx-widthx/2).getValue("arcmin")),
				AipsError
			);
			AlwaysAssert(
				near(wtrc[1].getValue("arcmin"), (centery+widthy/2).getValue("arcmin")),
				AipsError
			);

			/*
			vector<Double> pblc, ptrc;
			box.pixelBoundingBox(pblc, ptrc);
			AlwaysAssert(pblc[0] < ptrc[0], AipsError);
			AlwaysAssert(pblc[1] < ptrc[1], AipsError);

			AlwaysAssert(near(pblc[0], (-1)*wblc[0].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(pblc[1], wblc[1].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(ptrc[0], (-1)*wtrc[0].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(ptrc[1], wtrc[1].getValue("arcmin"), 3e-6), AipsError);
			*/
		}
		{
			log << LogIO::NORMAL
				<< "Test corners with no conversions"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.02, "deg");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
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
			AnnCenterBox box(
				centerx, centery, widthx, widthy, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			Vector<MDirection> corners = box.getCorners();
			Double got = corners[0].getAngle("deg").getValue("deg")[0];
			Double expec = (centerx + widthx/2).getValue("deg");
			Double tol = 1e-7;
			AlwaysAssert(near(got, expec, tol), AipsError);
			got = corners[0].getAngle("deg").getValue("deg")[1];
			expec = (centery - widthy/2).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);

			got = corners[1].getAngle("deg").getValue("deg")[0];
			expec = (centerx - widthx/2).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);
			got = corners[1].getAngle("deg").getValue("deg")[1];
			expec = (centery - widthy/2).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);

			got = corners[2].getAngle("deg").getValue("deg")[0];
			expec = (centerx - widthx/2).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);
			got = corners[2].getAngle("deg").getValue("deg")[1];
			expec = (centery + widthy/2).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);

			got = corners[3].getAngle("deg").getValue("deg")[0];
			expec = (centerx + widthx/2).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);
			got = corners[3].getAngle("deg").getValue("deg")[1];
			expec = (centery + widthy/2).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);
		}
		{

			log << LogIO::NORMAL
				<< "Test widths specified in pixels"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "pix");
			Quantity widthy(45, "pix");
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
			AnnCenterBox box(
				centerx, centery, widthx, widthy, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			Vector<MDirection> corners = box.getCorners();
			Double got = corners[0].getAngle("deg").getValue("deg")[0];
			Double expec = (centerx + Quantity(900, "arcsec")).getValue("deg");
			Double tol = 1e-4;
			AlwaysAssert(near(got, expec, tol), AipsError);
			got = corners[0].getAngle("deg").getValue("deg")[1];
			expec = (centerx - Quantity(1350, "arcsec")).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);

			got = corners[1].getAngle("deg").getValue("deg")[0];
			expec = (centerx - Quantity(900, "arcsec")).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);
			got = corners[1].getAngle("deg").getValue("deg")[1];
			expec = (centerx - Quantity(1350, "arcsec")).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);

			got = corners[2].getAngle("deg").getValue("deg")[0];
			expec = (centerx - Quantity(900, "arcsec")).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);
			got = corners[2].getAngle("deg").getValue("deg")[1];
			expec = (centerx + Quantity(1350, "arcsec")).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);

			got = corners[3].getAngle("deg").getValue("deg")[0];
			expec = (centerx + Quantity(900, "arcsec")).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);
			got = corners[3].getAngle("deg").getValue("deg")[1];
			expec = (centerx + Quantity(1350, "arcsec")).getValue("deg");
			AlwaysAssert(near(got, expec, tol), AipsError);
		}
		{
			log << LogIO::NORMAL << "Check unmodified frequencies" << LogIO::POST;
			Quantity centerx(-0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
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
			AnnCenterBox box(
				centerx, centery, widthx, widthy,
				dirTypeString, csys, shape, beginFreq,
				endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), beginFreq.getValue("Hz")),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), endFreq.getValue("Hz")),
				AipsError
			);
			log << LogIO::NORMAL << "*** Test freq in pixels" << LogIO::POST;
			beginFreq = Quantity(0, "pix");
			endFreq = Quantity(100, "pix");
			box = AnnCenterBox(
				centerx, centery, widthx, widthy,
				dirTypeString, csys, shape, beginFreq,
				endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			SpectralCoordinate sp = csys.spectralCoordinate();
			Double expBeg, expEnd;
			sp.toWorld(expBeg, beginFreq.getValue());
			sp.toWorld(expEnd, endFreq.getValue());

			freqs = box.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), expBeg),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), expEnd),
				AipsError
			);

			log << LogIO::NORMAL << "*** Test freq in channels" << LogIO::POST;
			beginFreq = Quantity(0, "channel");
			endFreq = Quantity(100, "channel");
			box = AnnCenterBox(
				centerx, centery, widthx, widthy,
				dirTypeString, csys, shape, beginFreq,
				endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			freqs = box.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), expBeg),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), expEnd),
				AipsError
			);
		}
		{
			log << LogIO::NORMAL << "Test frequencies GALACTO -> LSRK" << LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");

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
			AnnCenterBox box(
				centerx, centery, widthx, widthy,
				dirTypeString, csys, shape, beginFreq,
				endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
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
				<< "Test unmodified frequencies when specifying relativistic velocities" << LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");

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
			AnnCenterBox box(
				centerx, centery, widthx, widthy,
				dirTypeString, csys, shape, beginFreq,
				endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
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
				<< "Test unmodified frequencies when specifying velocities" << LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");

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
			AnnCenterBox box(
				centerx, centery, widthx, widthy,
				dirTypeString, csys, shape, beginFreq,
				endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
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
				<< "Test unmodified frequencies when specifying relativistic velocities"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");

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
			AnnCenterBox box(
				centerx, centery, widthx, widthy,
				dirTypeString, csys, shape, beginFreq,
				endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
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
			log << LogIO::NORMAL << "Test modified doppler definitions"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");

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
			AnnCenterBox box(
				centerx, centery, widthx, widthy,
				dirTypeString, csys, shape, beginFreq,
				endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			cout << box << endl;

			Vector<MFrequency> freqs = box.getFrequencyLimits();
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

	} catch (const AipsError& x) {
		cerr << "Caught exception: " << x.getMesg() << endl;
		return 1;
	}

	cout << "OK" << endl;
	return 0;
}
