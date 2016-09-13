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
#include <imageanalysis/Annotations/AnnRotBox.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <casa/namespace.h>

#include <iomanip>

void testCorners(const Vector<MDirection>& corners) {
	Vector<Bool> found(4, False);
	for (uInt i=0; i<4; i++) {
		Double ra = corners[i].getAngle("arcmin").getValue("arcmin")[0];
		Double dec = corners[i].getAngle("arcmin").getValue("arcmin")[1];
		cout << "*** ra dec " << std::setprecision(19) << ra << " " << dec << endl;
		if (
			near(
				ra, -1.140437089542486016
			)
		) {
			AlwaysAssert(! found[0], AipsError);
			AlwaysAssert(
				near(
					dec, 27.58556805004556267
				), AipsError
			);
			found[0] = True;
		}
		else if (
			near(
				ra, 24.8405434977460402
			)
		) {
			AlwaysAssert(! found[1], AipsError);
			AlwaysAssert(
				near(
					dec, 12.58525870773907762
				), AipsError
			);
			found[1] = True;
		}
		else if (
			near(
				ra, 2.340432320494556961
			)
		) {
			AlwaysAssert(! found[2], AipsError);
			AlwaysAssert(
				near(
					dec, -26.38556820383231738
				), AipsError
			);
			found[2] = True;
		}
		else if (
			near(
				ra, -23.64051399776834117
			)
		) {
			AlwaysAssert(! found[3], AipsError);
			AlwaysAssert(
				near(
					dec, -11.38528853997836165
				), AipsError
			);
			found[3] = True;
		}
		else {
			throw AipsError("Corner could not be identified");
		}
	}
}

int main () {
	try {
		LogIO log;
		IPosition imShape(4,500,500,1,6000);
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		Vector<Double> refVal = csys.referenceValue();
		refVal[0] = 300;
		csys.setReferenceValue(refVal);
		AnnRegion::unitInit();
		{
			log << LogIO::NORMAL
				<<"Test mixed world and pixel coordinates throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "pix");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
			Quantity pa(30, "deg");

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
				AnnRotBox box(
					centerx, centery, widthx, widthy, pa, dirTypeString,
					csys, imShape, beginFreq, endFreq, freqRefFrameString,
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
				<< "Test that bad quantity for world direction coordinate throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "cm");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
			Quantity pa(30, "deg");

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
				AnnRotBox box(
					centerx, centery, widthx, widthy, pa, dirTypeString,
					csys, imShape, beginFreq, endFreq, freqRefFrameString,
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
				<< "Test that bad quantity for position angle throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "cm");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
			Quantity pa(30, "kg");

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
				AnnRotBox box(
					centerx, centery, widthx, widthy, pa, dirTypeString,
					csys, imShape, beginFreq, endFreq, freqRefFrameString,
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
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
			Quantity pa(30, "deg");

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
			AnnRotBox box(
				centerx, centery, widthx, widthy, pa, dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			Vector<MDirection> corners = box.getCorners();
			for (uInt i=0; i<4; i++) {
				cout << corners[i].getAngle("arcmin") << endl;
			}
			vector<Quantity> wblc, wtrc;
			box.worldBoundingBox(wblc, wtrc);

			AlwaysAssert(
				near(
					wblc[0].getValue("arcmin"), 24.84038105676658503
				),
				AipsError
			);
			AlwaysAssert(
				near(
					wblc[1].getValue("arcmin"),-26.38557158514987577
				), AipsError
			);
			AlwaysAssert(
				near(
					wtrc[0].getValue("arcmin"), -23.64038105676658219
				), AipsError
			);
			AlwaysAssert(
				near(
					wtrc[1].getValue("arcmin"), 27.58557158514987862
				), AipsError
			);

			vector<Double> pblc, ptrc;
			box.pixelBoundingBox(pblc, ptrc);
			AlwaysAssert(pblc[0] < ptrc[0], AipsError);
			AlwaysAssert(pblc[1] < ptrc[1], AipsError);
			cout << std::setprecision(19) << pblc[0] << " " << pblc[1] << endl;
			cout << std::setprecision(19) << ptrc[0] << " " << ptrc[1] <<  endl;
			AlwaysAssert(
				near(
					pblc[0], 274.8578147498681687
				),
				AipsError
			);
			AlwaysAssert(
				near(
					pblc[1], -26.38531252582800235
				), AipsError
			);
			AlwaysAssert(
				near(
					ptrc[0], 323.1521213950262563
				), AipsError
			);
			AlwaysAssert(
				near(
					ptrc[1], 27.58527554845477781
				), AipsError
			);
		}
		*/
		{
			log << LogIO::NORMAL
				<< "Test corners with no conversions"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
			Quantity pa(30, "deg");

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

			AnnRotBox box(
				centerx, centery, widthx, widthy, pa, dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			testCorners(box.getCorners());
		}
		{
			log << LogIO::NORMAL
				<< "Test widths specified in pixels"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "pix");
			Quantity widthy(45, "pix");
			Quantity pa(30, "deg");
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

			AnnRotBox box(
				centerx, centery, widthx, widthy, pa, dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			testCorners(box.getCorners());
		}
		{
			log << LogIO::NORMAL
				<< "check unmodified frequencies"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
			Quantity pa(30, "deg");

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
			AnnRotBox box(
				centerx, centery, widthx, widthy, pa,
				dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
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
		}
		{
			log << LogIO::NORMAL
				<< "check frequencies GALACTO -> LSRK"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
			Quantity pa(30, "deg");

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
			AnnRotBox box(
				centerx, centery, widthx, widthy, pa,
				dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 1415467701.708973169),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 1450479270.302481413),
				AipsError
			);
		}
		{
			log << LogIO::NORMAL
				<< "check unmodified frequencies when specifying relativistic velocities"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
			Quantity pa(30, "deg");

			Quantity endFreq(-250000, "km/s");
			Quantity beginFreq(250000000, "m/s");

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
			AnnRotBox box(
				centerx, centery, widthx, widthy, pa,
				dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
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
				<< "check unmodified frequencies when specifying velocities"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
			Quantity pa(30, "deg");

			Quantity endFreq(-20, "km/s");
			Quantity beginFreq(20000, "m/s");

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
			AnnRotBox box(
				centerx, centery, widthx, widthy, pa,
				dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
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
				<< "check unmodified frequencies when specifying relativistic velocities"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
			Quantity pa(30, "deg");

			Quantity endFreq(-250000, "km/s");
			Quantity beginFreq(250000000, "m/s");

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
			AnnRotBox box(
				centerx, centery, widthx, widthy, pa,
				dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
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
				<< "test modified doppler definitions"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcmin");
			Quantity widthy(45, "arcmin");
			Quantity pa(30, "deg");

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
			AnnRotBox box(
				centerx, centery, widthx, widthy, pa,
				dirTypeString,
				csys, imShape, beginFreq, endFreq, freqRefFrameString,
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

	} catch (AipsError x) {
		cerr << "Caught exception: " << x.getMesg() << endl;
		return 1;
	}

	cout << "OK" << endl;
	return 0;
}
