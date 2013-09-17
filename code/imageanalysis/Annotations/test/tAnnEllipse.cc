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
#include <imageanalysis/Annotations/AnnEllipse.h>

#include <casa/Quanta/QuantumHolder.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <casa/namespace.h>

int main () {
	try {
		LogIO log;
		IPosition shape(4, 400, 400, 1, 5500);
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		AnnRegion::unitInit();
		Vector<Double> refpix = csys.referencePixel();
		refpix[0] = 200;
		csys.setReferencePixel(refpix);
		{
			log << LogIO::NORMAL
				<<"Test mixed world and pixel coordinates throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "pix");
			Quantity mi(30, "arcsec");
			Quantity ma(40, "arcsec");
			Quantity pa(0, "deg");
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
				AnnEllipse ellipse(
					centerx, centery, ma, mi, pa, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
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
				<< "Test that bad quantity for world direction coordinate throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "cm");
			Quantity mi(30, "arcsec");
			Quantity ma(40, "arcsec");
			Quantity pa(0, "deg");

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
				AnnEllipse ellipse(
					centerx, centery, ma, mi, pa, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
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
				<< "Test that minor axis larger than major axis throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity mi(30, "arcsec");
			Quantity ma(20, "arcsec");
			Quantity pa(0, "deg");

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
				AnnEllipse ellipse(
					centerx, centery, ma, mi, pa, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
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
				<< "Test that position angle not in angular units throws exception"
				<< LogIO::POST;
			Bool thrown = True;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity mi(30, "arcsec");
			Quantity ma(40, "arcsec");
			Quantity pa(0, "km");

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
				AnnEllipse ellipse(
					centerx, centery, ma, mi, pa, dirTypeString,
					csys, shape, beginFreq, endFreq, freqRefFrameString,
					dopplerString, restfreq, stokes, False
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
			Quantity centerx(-0.6, "arcmin");
			Quantity centery(1.2, "arcmin");
			Quantity mi(3, "arcmin");
			Quantity ma(4, "arcmin");
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
			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			cout << ellipse << endl;

			Quantity wDelX(4.964101615, "arcmin");
			Quantity wDelY(4.598076211, "arcmin");
			/*
			vector<Quantity> wblc, wtrc;
			ellipse.worldBoundingBox(wblc, wtrc);
			AlwaysAssert(near(wblc[0].getValue("arcmin"), (centerx+wDelX).getValue("arcmin"), 1e-6), AipsError);
			AlwaysAssert(near(wblc[1].getValue("arcmin"), (centery-wDelY).getValue("arcmin"), 1e-6), AipsError);
			AlwaysAssert(near(wtrc[0].getValue("arcmin"), (centerx-wDelX).getValue("arcmin"), 1e-6), AipsError);
			AlwaysAssert(near(wtrc[1].getValue("arcmin"), (centery+wDelY).getValue("arcmin"), 1e-6), AipsError);
			/*
			vector<Double> pblc, ptrc;
			ellipse.pixelBoundingBox(pblc, ptrc);
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
				<< "Test center with no conversion"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.02, "deg");
			Quantity mi(30, "arcsec");
			Quantity ma(40, "arcsec");
			Quantity pa(0, "deg");

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

			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			MDirection center = ellipse.getCenter();

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
			TableRecord regionRec = ellipse.asRecord();
			QuantumHolder qh;
			String err;
			for (uInt i=0; i<2; i++) {
				qh.fromRecord(
					err,
					regionRec.asRecord("center").asRecord(0)
				);
				AlwaysAssert(
					near(
						qh.asQuantity().getValue("rad"),
						centerx.getValue("rad")
					), AipsError
				);
				qh.fromRecord(
					err,
					regionRec.asRecord("center").asRecord(1)
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
			Quantity mi(30, "arcsec");
			Quantity ma(40, "arcsec");
			Quantity pa(0, "deg");

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
			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			MDirection center = ellipse.getCenter();
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
			TableRecord regionRec = ellipse.asRecord();
			QuantumHolder qh;
			String err;
			for (uInt i=0; i<2; i++) {
				qh.fromRecord(
					err,
					regionRec.asRecord("center").asRecord(0)
				);
				AlwaysAssert(
					near(
						qh.asQuantity().getValue("rad"),
						xnew
					), AipsError
				);
				qh.fromRecord(
					err,
					regionRec.asRecord("center").asRecord(1)
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
				<< "Test axes specified in pixels"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity mi(30, "pix");
			Quantity ma(40, "pix");
			Quantity pa(0, "deg");

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

			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Quantity qma = ellipse.getSemiMajorAxis();
			Quantity qmi = ellipse.getSemiMinorAxis();
			AlwaysAssert(
				near(
					qmi.getValue("deg"),
					Quantity(1800, "arcsec").getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					qma.getValue("deg"),
					Quantity(2400, "arcsec").getValue("deg")
				), AipsError
			);
		}

		{
			log << LogIO::NORMAL << "Test unmodified frequencies" << LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity mi(30, "pix");
			Quantity ma(40, "pix");
			Quantity pa(0, "deg");

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
			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = ellipse.getFrequencyLimits();
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
				<< "Test frequencies GALACTO -> LSRK"
				<< LogIO::POST;
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity mi(30, "pix");
			Quantity ma(40, "pix");
			Quantity pa(0, "deg");

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
			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = ellipse.getFrequencyLimits();
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
			Quantity centerx(0.01, "deg");
			Quantity centery(0.01, "deg");
			Quantity mi(30, "pix");
			Quantity ma(40, "pix");
			Quantity pa(0, "deg");

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
			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = ellipse.getFrequencyLimits();
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
			Quantity mi(30, "pix");
			Quantity ma(40, "pix");
			Quantity pa(0, "deg");

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
			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MFrequency> freqs = ellipse.getFrequencyLimits();
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
			Quantity mi(30, "pix");
			Quantity ma(40, "pix");
			Quantity pa(0, "deg");

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
			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			cout << ellipse << endl;
			Vector<MFrequency> freqs = ellipse.getFrequencyLimits();
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
				<< "Test center outside lattice with no conversion"
				<< LogIO::POST;
			Quantity centerx(1.0, "deg");
			Quantity centery(1.0, "deg");
			Quantity mi(30, "arcsec");
			Quantity ma(40, "arcsec");
			Quantity pa(0, "deg");

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

			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			MDirection center = ellipse.getCenter();

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
			TableRecord regionRec = ellipse.asRecord();
			cout << regionRec << endl;
			QuantumHolder qh;
			String err;
			for (uInt i=0; i<2; i++) {
				qh.fromRecord(
					err,
					regionRec.asRecord("center").asRecord(0)
				);
				AlwaysAssert(
					near(
						qh.asQuantity().getValue("rad"),
						centerx.getValue("rad")
					), AipsError
				);
				qh.fromRecord(
					err,
					regionRec.asRecord("center").asRecord(1)
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
				<< "Test non-square pixels"
				<< LogIO::POST;
			Quantity centerx(50, "pix");
			Quantity centery(50, "pix");
			Quantity mi(20, "pix");
			Quantity ma(20, "pix");
			Quantity pa(0, "deg");
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
			Vector<Double> inc =csys.increment();
			inc[0] = -1.4;
			csys.setIncrement(inc);
			AnnEllipse ellipse(
				centerx, centery, ma, mi, pa, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			AlwaysAssert(ellipse.getSemiMajorAxis().getValue() == 28, AipsError);
			AlwaysAssert(ellipse.getSemiMinorAxis().getValue() == 20, AipsError);
			AlwaysAssert(near(ellipse.getPositionAngle().getValue(), 90.0), AipsError);
			inc[0] = -1;
			inc[1] = 1.4;
			csys.setIncrement(inc);
			ellipse = 	AnnEllipse(
				centerx, centery, ma, mi, pa, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			AlwaysAssert(ellipse.getSemiMajorAxis().getValue() == 28, AipsError);
			AlwaysAssert(ellipse.getSemiMinorAxis().getValue() == 20, AipsError);
			AlwaysAssert(near(ellipse.getPositionAngle().getValue(), 0.0), AipsError);

		}
	} catch (const AipsError& x) {
		cerr << "Caught exception: " << x.getMesg() << endl;
		return 1;
	}

	cout << "OK" << endl;
	return 0;
}
