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
#include <imageanalysis/Annotations/AnnAnnulus.h>

#include <casa/Quanta/QuantumHolder.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <casa/namespace.h>

#include <vector>

int main () {
	try {
		LogIO log;
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		Vector<Double> refpix = csys.referencePixel();
		refpix[0] = 200;
		csys.setReferencePixel(refpix);
		IPosition shape(4, 400,400,1,5500);
		AnnRegion::unitInit();
		{
			log << LogIO::NORMAL
				<<"Test mixed world and pixel coordinates throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(-0.01, "deg");
			Quantity centery(0.01, "pix");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");
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
				AnnAnnulus annulus(
					centerx, centery, inner, outer, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
				);
				thrown = False;
			} catch (AipsError x) {
				log << LogIO::NORMAL
					<< "Exception thrown as expected: " << x.getMesg()
					<< LogIO::POST;
			}
			AlwaysAssert(thrown, AipsError);
		}
		{
			log << LogIO::NORMAL
				<< "Test that bad quantity for world direction coordinate throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(-0.01, "deg");
			Quantity centery(0.01, "cm");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");

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
				AnnAnnulus annulus(
					centerx, centery, inner, outer, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
				);
				thrown = False;
			} catch (AipsError x) {
				log << LogIO::NORMAL
					<< "Exception thrown as expected: " << x.getMesg()
					<< LogIO::POST;
			}
			AlwaysAssert(thrown, AipsError);
		}
		{
			log << LogIO::NORMAL
				<< "Test that inner radius larger than outer radius throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(-0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(20, "arcsec");

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
				AnnAnnulus annulus(
					centerx, centery, inner, outer, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
				);
				thrown = False;
			} catch (AipsError x) {
				log << LogIO::NORMAL
					<< "Exception thrown as expected: " << x.getMesg()
					<< LogIO::POST;
			}
			AlwaysAssert(thrown, AipsError);
		}
		{
			log << LogIO::NORMAL
				<< "Test that shape and coordinate system mismatch throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(-0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");

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
				AnnAnnulus annulus(
					centerx, centery, inner, outer, dirTypeString,
					csys, IPosition(3, 40, 40, 2), beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
				);
				thrown = False;
			}
			catch (AipsError x) {
				log << LogIO::NORMAL
					<< "Exception thrown as expected: " << x.getMesg()
					<< LogIO::POST;
			}
			AlwaysAssert(thrown, AipsError);
		}
		{
			log << LogIO::NORMAL << "Test precision"
				<< LogIO::POST;
			Quantity centerx(-0.01, "deg");
			Quantity centery(0.02, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");
			Vector<Stokes::StokesTypes> stokes(0);
			AnnAnnulus ann(
				centerx, centery, inner, outer,
				csys, shape, stokes
			);
			cout << ann << endl;
		}

		{
			log << LogIO::NORMAL
				<< "Test center with no conversion"
				<< LogIO::POST;
			Quantity centerx(-0.01, "deg");
			Quantity centery(0.02, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");

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
			AnnAnnulus annulus(
				centerx, centery, inner, outer, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			MDirection center = annulus.getCenter();

			AlwaysAssert(
				near(
					center.getAngle("deg").getValue("deg")[0],
					centerx.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					center.getAngle("deg").getValue("deg")[1],
					centery.getValue("deg")
				), AipsError
			);
			TableRecord regionRec = annulus.asRecord();
			QuantumHolder qh;
			String err;
			for (uInt i=0; i<2; i++) {
				qh.fromRecord(
					err,
					regionRec.asRecord("regions").asRecord(i).asRecord("center").asRecord(0)
				);
				AlwaysAssert(
					near(
						qh.asQuantity().getValue("rad"),
						centerx.getValue("rad")
					), AipsError
				);
				qh.fromRecord(
					err,
					regionRec.asRecord("regions").asRecord(i).asRecord("center").asRecord(1)
				);
				AlwaysAssert(
					near(
						qh.asQuantity().getValue("rad"),
						centery.getValue("rad")
					), AipsError
				);
			}
		}

		{
			log << LogIO::NORMAL
				<< "Test precessing from B1950 to J2000"
				<< LogIO::POST;
			Quantity centerx(0.05, "deg");
			Quantity centery(0, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");

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
			AnnAnnulus annulus(
				centerx, centery, inner, outer, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			MDirection center = annulus.getCenter();
			Double xnew = 0.012055422536187882;
			Double ynew = 0.00485808148440817;
			AlwaysAssert(
				near(
					center.getAngle("rad").getValue("rad")[0],
					xnew
				), AipsError
			);
			AlwaysAssert(
				near(
					center.getAngle("rad").getValue("rad")[1],
					ynew
				), AipsError
			);
			TableRecord regionRec = annulus.asRecord();
			QuantumHolder qh;
			String err;
			for (uInt i=0; i<2; i++) {
				qh.fromRecord(
					err,
					regionRec.asRecord("regions").asRecord(i).asRecord("center").asRecord(0)
				);
				AlwaysAssert(
					near(
						qh.asQuantity().getValue("rad"),
						xnew
					), AipsError
				);
				qh.fromRecord(
					err,
					regionRec.asRecord("regions").asRecord(i).asRecord("center").asRecord(1)
				);
				AlwaysAssert(
					near(
						qh.asQuantity().getValue("rad"),
						ynew
					), AipsError
				);
			}
		}

		{
			log << LogIO::NORMAL
				<< "Test radius specified in pixels"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity inner(30, "pix");
			Quantity outer(40, "pix");

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

			AnnAnnulus annulus(
				centerx, centery, inner, outer, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<Quantity> radq = annulus.getRadii();
			AlwaysAssert(
				near(
					radq[0].getValue("deg"),
					Quantity(1800, "arcsec").getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					radq[1].getValue("deg"),
					Quantity(2400, "arcsec").getValue("deg")
				), AipsError
			);
		}

		{
			log << LogIO::NORMAL << "Test unmodified frequencies" << LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity inner(30, "pix");
			Quantity outer(40, "pix");

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
			std::vector<AnnAnnulus> annulus(2);
			annulus[0] = AnnAnnulus(
				centerx, centery, inner, outer,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			annulus[1] = AnnAnnulus(
				centerx, centery, inner, outer,
				csys, shape, stokes
			);
			annulus[1].setFrequencyLimits(
				beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq
			);
			for (uInt i=0; i<2; i++) {
				Vector<MFrequency> freqs = annulus[i].getFrequencyLimits();
				AlwaysAssert(
					near(freqs[0].get("Hz").getValue(), beginFreq.getValue("Hz")),
					AipsError
				);
				AlwaysAssert(
					near(freqs[1].get("Hz").getValue(), endFreq.getValue("Hz")),
					AipsError
				);
			}
		}

		{
			log << LogIO::NORMAL
				<< "Test frequencies GALACTO -> LSRK"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity inner(30, "pix");
			Quantity outer(40, "pix");

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
			vector<AnnAnnulus> annulus(2);
			Vector<Stokes::StokesTypes> stokes(0);
			annulus[0] = AnnAnnulus(
				centerx, centery, inner, outer,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			annulus[1] = AnnAnnulus(
				centerx, centery, inner, outer,
				csys, shape, stokes
			);
			annulus[1].setFrequencyLimits(
				beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq
			);
			annulus[1].setAnnotationOnly(False);
			AlwaysAssert(annulus[0] == annulus[1], AipsError);
			vector<String> s(2);
			ostringstream os;
			os << annulus[0];
			s[0] = os.str();
			os.str("");
			os << annulus[1];
			s[1] = os.str();
			cout << "'" << s[0] << "'" << endl;
			cout << "'" << s[1] << "'" << endl;

			AlwaysAssert(s[0] == s[1], AipsError);
			for (uInt i=0; i<2; i++) {
				Vector<MFrequency> freqs = annulus[i].getFrequencyLimits();
				AlwaysAssert(
					near(freqs[0].get("Hz").getValue(), 1415508785.4853702),
					AipsError
				);
				AlwaysAssert(
					near(freqs[1].get("Hz").getValue(), 1450521370.2853618),
					AipsError
				);
			}
		}

		{
			log << LogIO::NORMAL
				<< "Test unmodified frequencies when specifying relativistic velocities"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");

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
			AnnAnnulus annulus(
				centerx, centery, inner, outer,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = annulus.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 235914853.26413003),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 2604896650.3078709),
				AipsError
			);
		}

		{
			log << LogIO::NORMAL
				<< "Test unmodified frequencies when specifying velocities"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");

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
			AnnAnnulus annulus(
				centerx, centery, inner, outer,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = annulus.getFrequencyLimits();
			AlwaysAssert(
				near(freqs[0].get("Hz").getValue(), 1420310992.5141187),
				AipsError
			);
			AlwaysAssert(
				near(freqs[1].get("Hz").getValue(), 1420500511.0578821),
				AipsError
			);
		}
		{
			log << LogIO::NORMAL
				<< "Test modified doppler definitions"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");

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
			AnnAnnulus annulus(
				centerx, centery, inner, outer, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			cout << "annulus " << annulus << endl;
			Vector<MFrequency> freqs = annulus.getFrequencyLimits();
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
				<< "Test colors"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.02, "deg");
			Quantity inner(30, "arcsec");
			Quantity outer(40, "arcsec");

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

			AnnAnnulus annulus(
				centerx, centery, inner, outer, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			{
				// we don't support this color name
				Bool thrown = False;
				try {
					annulus.setColor("purple");
					AlwaysAssert(False, AipsError);
				}
				catch (AipsError x) {
					log << LogIO::NORMAL << "Exception caught as expected: "
						<< x.getMesg() << LogIO::POST;
					thrown = True;
				}
				AlwaysAssert(thrown, AipsError);
			}
			{
				// bad hex spec
				Bool thrown = False;
				try {
					annulus.setColor("GG0022");
					AlwaysAssert(False, AipsError);
				}
				catch (AipsError x) {
					log << LogIO::NORMAL << "Exception caught as expected: "
						<< x.getMesg() << LogIO::POST;
					thrown = True;
				}
				AlwaysAssert(thrown, AipsError);
			}
			annulus.setColor("FF0000");
			AlwaysAssert(annulus.getColorString() == "red", AipsError);
			cout << "ann 1st color " << annulus << endl;
			annulus.setColor("Ff15De");
			AlwaysAssert(annulus.getColorString() == "ff15de", AipsError);
			cout << "ann 2nd color " << annulus << endl;
			annulus.setColor(" bLue" );
			AlwaysAssert(annulus.getColorString() == "blue", AipsError);
			cout << "ann 3rd color " << annulus << endl;
			AlwaysAssert(AnnotationBase::colorChoices().size() == 10, AipsError);
			cout << annulus << endl;
			annulus.setFont("Lucida Blueberry");
			cout << annulus << endl;
			annulus.setLabel("beeblebrox");
			cout << annulus << endl;
			annulus.setLabel("marklar beeblebrox");
			cout << annulus << endl;
			annulus.setLabelColor("yellow");
			cout << annulus << endl;
			annulus.setLabel("");
			cout << annulus << endl;
			//int font_style = annulus.getFontStyle();
			annulus.setFontStyle(AnnotationBase::ITALIC_BOLD);
			ostringstream oss;
			annulus.print(oss);
			cout << oss.str() << endl;
		}
	}
	catch (AipsError x) {
		cerr << "Caught exception: " << x.getMesg() << endl;
		return 1;
	}

	cout << "OK" << endl;
	return 0;
}
