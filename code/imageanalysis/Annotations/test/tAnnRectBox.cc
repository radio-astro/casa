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
#include <imageanalysis/Annotations/AnnRectBox.h>
#include <imageanalysis/Annotations/RegionTextList.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

#include <images/Images/PagedImage.h>

#include <casa/namespace.h>

#include <iomanip>

int main () {
	/*
	PagedImage<Float> pim("g35_sma_usb_12co.image");
	CoordinateSystem csys1 = pim.coordinates();
	IPosition shape1 = pim.shape();
	/*
	Vector<Double> refVal = csys1.referenceValue();
	refVal[0] = 4.94827;
	refVal[1] = 0.035055;
	Vector<Double> refPix = csys1.referencePixel();
	refPix[0] = 40;
	refPix[1] = 40;
	csys1.setReferencePixel(refPix);
	Vector<String> units = csys1.worldAxisUnits();
	units[0] = "arcsec";
	units[1] = "arcsec";
	csys1.setWorldAxisUnits(units);

 	RegionTextList rlist( "txy.crtf", csys1, shape1 );
	Vector<AsciiAnnotationFileLine> aaregions = rlist.getLines( );
	for ( unsigned int i=0; i < aaregions.size( ); ++i ) {
		if ( aaregions[i].getType( ) != AsciiAnnotationFileLine::ANNOTATION ) continue;
		const AnnotationBase* ann = aaregions[i].getAnnotationBase();
		cout << " >>>==>> " << ann->getLabel( ) << endl;
		cout << " >>>==>> " << ann->getFontSize( ) << endl;

	}

	return 0;
	*/

	try {
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		AnnRegion::unitInit();
		LogIO log;
		IPosition shape(4, 400,400,1,6000);
		Vector<Double> refVal = csys.referenceValue();
		refVal[0] = 300;
		csys.setReferenceValue(refVal);

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
				AnnRectBox box(
						blcx, blcy, trcx, trcy,
						dirTypeString,
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
			Quantity blcx(0.01, "km/s");
			Quantity blcy(0, "deg");
			Quantity trcx(0, "deg");
			Quantity trcy(0.01, "deg");
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
				AnnRectBox box(
						blcx, blcy, trcx, trcy,
						dirTypeString,
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
			Quantity blcx(10, "arcmin");
			Quantity blcy(5, "arcmin");
			Quantity trcx(2, "arcmin");
			Quantity trcy(12, "arcmin");

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
			AnnRectBox box(
				blcx, blcy, trcx, trcy,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			/*
			vector<Quantity> wblc, wtrc;
			box.worldBoundingBox(wblc, wtrc);
			AlwaysAssert(
				near(wblc[0].getValue("arcmin"), blcx.getValue("arcmin")),
				AipsError
			);

			AlwaysAssert(
				near(wblc[1].getValue("arcmin"), blcy.getValue("arcmin")),
				AipsError
			);
			AlwaysAssert(
				near(wtrc[0].getValue("arcmin"), trcx.getValue("arcmin")),
				AipsError
			);
			AlwaysAssert(
				near(wtrc[1].getValue("arcmin"), trcy.getValue("arcmin")),
				AipsError
			);
			/*
			vector<Double> pblc, ptrc;
			box.pixelBoundingBox(pblc, ptrc);

			AlwaysAssert(pblc[0] < ptrc[0], AipsError);
			AlwaysAssert(pblc[1] < ptrc[1], AipsError);

			AlwaysAssert(near(pblc[0], (-1)*wblc[0].getValue("arcmin") + 300, .4), AipsError);
			AlwaysAssert(near(pblc[1], wblc[1].getValue("arcmin"), 3e-6), AipsError);
			AlwaysAssert(near(ptrc[0], (-1)*wtrc[0].getValue("arcmin") + 300, .4), AipsError);
			AlwaysAssert(near(ptrc[1], wtrc[1].getValue("arcmin"), 3e-6), AipsError);
			*/
		}
		{
			log << LogIO::NORMAL
				<< "Test coordinates with no conversion"
				<< LogIO::POST;
			Quantity blcx(0.05, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0.015, "deg");
			Quantity trcy(0.01, "deg");
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
			AnnRectBox box(
				blcx, blcy, trcx, trcy,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);

			Vector<MDirection> corners = box.getCorners();
			AlwaysAssert(
				near(
					corners[0].getAngle("deg").getValue("deg")[0],
					blcx.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[0].getAngle("deg").getValue("deg")[1],
					blcy.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[1].getAngle("deg").getValue("deg")[0],
					trcx.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[1].getAngle("deg").getValue("deg")[1],
					blcy.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[2].getAngle("deg").getValue("deg")[0],
					trcx.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[2].getAngle("deg").getValue("deg")[1],
					trcy.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[3].getAngle("deg").getValue("deg")[0],
					blcx.getValue("deg")
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[3].getAngle("deg").getValue("deg")[1],
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
			AnnRectBox box(
				blcx, blcy, trcx, trcy, dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
				dopplerString, restfreq, stokes, False
			);
			Vector<MDirection> corners = box.getCorners();
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
					corners[2].getAngle("rad").getValue("rad")[0],
					0.011444556041464694
				), AipsError
			);
			AlwaysAssert(
				near(
					corners[2].getAngle("rad").getValue("rad")[1],
					0.0050326323941514792
				), AipsError
			);
			cout << box.getCorners() << endl;
		}
		{
			log << LogIO::NORMAL
				<< "Test unmodified frequencies"
				<< LogIO::POST;
			Quantity blcx(0.05, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0.015, "deg");
			Quantity trcy(0.01, "deg");
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
			AnnRectBox box(
				blcx, blcy, trcx, trcy,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
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
				<< "Test frequencies GALACTO -> LSRK"
				<< LogIO::POST;
			Quantity blcx(0.05, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0.015, "deg");
			Quantity trcy(0.01, "deg");
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
			AnnRectBox box(
				blcx, blcy, trcx, trcy,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
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
				<< "Test unmodified frequencies when specifying relativistic velocities"
				<< LogIO::POST;
			Quantity blcx(0.05, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0.015, "deg");
			Quantity trcy(0.01, "deg");
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
			AnnRectBox box(
				blcx, blcy, trcx, trcy,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
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
				<< "Test unmodified frequencies when specifying velocities"
				<< LogIO::POST;
			Quantity blcx(0.05, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0.015, "deg");
			Quantity trcy(0.01, "deg");
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
			AnnRectBox box(
				blcx, blcy, trcx, trcy,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
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
				<< "Test modified doppler definitions"
				<< LogIO::POST;
			Quantity blcx(0.05, "deg");
			Quantity blcy(0, "deg");
			Quantity trcx(0.015, "deg");
			Quantity trcy(0.01, "deg");
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
			AnnRectBox box(
				blcx, blcy, trcx, trcy,
				dirTypeString,
				csys, shape, beginFreq, endFreq, freqRefFrameString,
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
		/*
        {
            Quantity blcx (4.94852990, "rad");
            Quantity blcy (3.47999219e-02, "rad");
            Quantity trcx(4.94804479,"rad");
            Quantity trcy(3.52847397e-02, "rad");
            PagedImage<Float> im("g35.03_II_nh3_11.hline.image");
            Vector<Stokes::StokesTypes> stokes(1, Stokes::I);
            Int polaxis = CoordinateUtil::findStokesAxis(stokes, im.coordinates());
            cout << "stokes " << stokes << endl;
            AnnRectBox *box = new AnnRectBox( blcx, blcy, trcx, trcy, im.coordinates(), shape, stokes );
        }
        */

	} catch (AipsError x) {
		cerr << "Caught exception: " << x.getMesg() << endl;
		return 1;
	}

	cout << "OK" << endl;
	return 0;
}
