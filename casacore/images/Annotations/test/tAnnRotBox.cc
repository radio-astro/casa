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
#include <images/Annotations/AnnRotBox.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <casa/namespace.h>

int main () {
	try {
		LogIO log;
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
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
					csys, beginFreq, endFreq, freqRefFrameString,
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
					csys, beginFreq, endFreq, freqRefFrameString,
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
					csys, beginFreq, endFreq, freqRefFrameString,
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
				<< "Test corners with no conversions"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
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

			AnnRotBox box(
				centerx, centery, widthx, widthy, pa, dirTypeString,
				csys, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<Bool> found(4, False);
			Vector<MDirection> corners = box.getCorners();
			Double cosp = cos(pa.getValue("rad"));
			Double sinp = sin(pa.getValue("rad"));
			for (uInt i=0; i<4; i++) {
				Double ra = corners[i].getAngle("deg").getValue("deg")[0];
				Double dec = corners[i].getAngle("deg").getValue("deg")[1];
				cout << "*** ra dec " << ra << " " << dec << endl;

				if (
					near(
						ra,
						centerx.getValue("deg") + sinp*widthx.getValue("deg")/2
					)
				) {
					AlwaysAssert(! found[0], AipsError);
					AlwaysAssert(
						near(
							dec,
							centery.getValue("deg") + cosp*widthy.getValue("deg")/2
						), AipsError
					);
					found[0] = True;
				}

				else if (
					near(
						ra,
						centerx.getValue("deg") + cosp*widthx.getValue("deg")/2
					)
				) {
					AlwaysAssert(! found[1], AipsError);
					AlwaysAssert(
						near(
							dec,
							centery.getValue("deg") - sinp*widthy.getValue("deg")/2
						), AipsError
					);
					found[1] = True;
				}
				else if (
					near(
						ra,
						centerx.getValue("deg") - sinp*widthx.getValue("deg")/2
					)
				) {
					AlwaysAssert(! found[2], AipsError);
					AlwaysAssert(
						near(
							dec,
							centery.getValue("deg") - cosp*widthy.getValue("deg")/2
						), AipsError
					);
					found[2] = True;
				}
				else if (
					near(
						ra,
						centerx.getValue("deg") - cosp*widthx.getValue("deg")/2
					)
				) {
					AlwaysAssert(! found[3], AipsError);
					AlwaysAssert(
						near(
							dec,
							centery.getValue("deg") + sinp*widthy.getValue("deg")/2
						), AipsError
					);
					found[3] = True;
				}
				else {
					log << "Corner could not be identified" << LogIO::EXCEPTION;
				}
			}

		}
		{
			log << LogIO::NORMAL
				<< "Test widths specified in pixels"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(0.5, "pix");
			Quantity widthy(0.75, "pix");
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
				csys, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<Bool> found(4, False);
			Vector<MDirection> corners = box.getCorners();
			Double cosp = cos(pa.getValue("rad"));
			Double sinp = sin(pa.getValue("rad"));
			for (uInt i=0; i<4; i++) {
				Double ra = corners[i].getAngle("deg").getValue("deg")[0];
				Double dec = corners[i].getAngle("deg").getValue("deg")[1];
                cout << "ra dec 2 " << ra << " " << dec << endl;
				if (
					near(
						ra,
						centerx.getValue("deg") + sinp*widthx.getValue()/60/2
					)
				) {
					AlwaysAssert(! found[0], AipsError);
					AlwaysAssert(
						near(
							dec,
							centery.getValue("deg") + cosp*widthy.getValue()/60/2
						), AipsError
					);
					found[0] = True;
				}

				else if (
					near(
						ra,
						centerx.getValue("deg") + cosp*widthx.getValue()/120
					)
				) {
					AlwaysAssert(! found[1], AipsError);
					AlwaysAssert(
						near(
							dec,
							centery.getValue("deg") - sinp*widthy.getValue()/120
						), AipsError
					);
					found[1] = True;
				}
				else if (
					near(
						ra,
						centerx.getValue("deg") - sinp*widthx.getValue()/120
					)
				) {
					AlwaysAssert(! found[2], AipsError);
					AlwaysAssert(
						near(
							dec,
							centery.getValue("deg") - cosp*widthy.getValue()/120
						), AipsError
					);
					found[2] = True;
				}
				else if (
					near(
						ra,
						centerx.getValue("deg") - cosp*widthx.getValue()/120
					)
				) {
					AlwaysAssert(! found[3], AipsError);
					AlwaysAssert(
						near(
							dec,
							centery.getValue("deg") + sinp*widthy.getValue()/120
						), AipsError
					);
					found[3] = True;
				}
				else {
					log << "This corner could not be identified" << LogIO::EXCEPTION;
				}
			}


		}
		{
			// check unmodified frequencies
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
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
				csys, beginFreq, endFreq, freqRefFrameString,
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
			// check frequencies GALACTO -> LSRK
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
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
				csys, beginFreq, endFreq, freqRefFrameString,
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
			// check unmodified frequencies when specifying relativistic velocities
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
			Quantity pa(30, "deg");

			Quantity beginFreq(-250000, "km/s");
			Quantity endFreq(250000000, "m/s");

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
				csys, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 2604896650.3078709),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 235914853.26413003),
				AipsError
			);
		}
		{
			// check unmodified frequencies when specifying velocities
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
			Quantity pa(30, "deg");

			Quantity beginFreq(-20, "km/s");
			Quantity endFreq(20000, "m/s");

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
				csys, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 1420500511.0578821),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 1420310992.5141187),
				AipsError
			);
		}
		{
			// check unmodified frequencies when specifying relativistic velocities
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
			Quantity pa(30, "deg");

			Quantity beginFreq(-250000, "km/s");
			Quantity endFreq(250000000, "m/s");

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
				csys, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = box.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 2604896650.3078709),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 235914853.26413003),
				AipsError
			);
		}
		{
			// check modified doppler definitions
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity widthx(30, "arcsec");
			Quantity widthy(45, "arcsec");
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
				csys, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

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
