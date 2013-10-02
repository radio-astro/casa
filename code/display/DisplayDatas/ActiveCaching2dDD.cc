//# ActiveCaching2dDD.cc: active 2d implementation of a CachingDisplayData
//# Copyright (C) 1999,2000,2001,2002,2003,2004
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDoppler.h>
#include <casa/BasicMath/Math.h>
#include <coordinates/Coordinates.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/ActiveCaching2dDD.h>
#include <casa/stdio.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	ActiveCaching2dDD::ActiveCaching2dDD(const DisplayCoordinateSystem &coordsys,
	                                     const Vector<Double> &pixblc,
	                                     const Vector<Double> &pixtrc) :
		CachingDisplayData(),
		itsCoordinateSystem(coordsys),
		itsPixelBlc(pixblc),
		itsPixelTrc(pixtrc),
		itsOptionsAspect("flexible"),
		itsSpectralUnit("km/s"),
		itsVelocityType("radio"),
		itsAbsolute(True),
		itsShowWorldCoordinate(True),
		itsFractionalPixels(False) {

		setCoordinateSystem(coordsys, pixblc, pixtrc);
	}

	ActiveCaching2dDD::ActiveCaching2dDD() :
		CachingDisplayData(),
		itsOptionsAspect("flexible"),
		itsSpectralUnit("km/s"),
		itsVelocityType("radio"),
		itsAbsolute(True),
		itsShowWorldCoordinate(True),
		itsFractionalPixels(False) {
		// MUST IMPLEMENT
	}

	ActiveCaching2dDD::ActiveCaching2dDD(const ActiveCaching2dDD &o) : CachingDisplayData(o) {
		// MUST IMPLEMENT
	}

	void ActiveCaching2dDD::operator=(const ActiveCaching2dDD &) {
		// MUST IMPLEMENT
	}


	ActiveCaching2dDD::~ActiveCaching2dDD() {
	}

	Bool ActiveCaching2dDD::linToWorld(Vector<Double> &world,
	                                   const Vector<Double> &lin) {
		return itsCoordinateSystem.toWorld(world, lin);
	}

	Bool ActiveCaching2dDD::worldToLin(Vector<Double> &lin,
	                                   const Vector<Double> &world) {
		return itsCoordinateSystem.toPixel(lin, world);
	}

	String ActiveCaching2dDD::showPosition(const Vector<Double> &world,
	                                       const Bool &) {
		String retval;
		String units;
		if (itsShowWorldCoordinate) {
			for (uInt i = 0; i < 2; i++) {
				units = "";
				if (i > 0) {
					retval = retval + String(" ");
				}
				retval += itsCoordinateSystem.format(units, Coordinate::DEFAULT,
				                                     world(i), i, True, itsAbsolute);
				if ((units != String("")) && (units != String(" "))) {
					retval = retval + String(" ") + units;
				}
			}
		} else {
			static Vector<Double> pixel;
			if (!itsCoordinateSystem.toPixel(pixel, world)) {
				throw(AipsError(itsCoordinateSystem.errorMessage()));
			}
//
			Int offset = uiBase();         // make 1-rel for display
			// (uiBase() is 1 by default, but you can now call setUIBase(0)
			// to make it return 0, so that this method returns 0-based values).

			if (!itsAbsolute) {
				itsCoordinateSystem.makePixelRelative(pixel);
				offset = 0;
			}
			ostringstream oss;
			if (itsFractionalPixels) {
				oss << pixel(0)+offset << " " << pixel(1)+offset << " pixels";
			} else {
				oss << Int(pixel(0)+0.5+offset) << " " << Int(pixel(1)+0.5+offset)
				    << " pixels";
			}
			retval = String(oss);
		}
//
		return retval;
	}

	Vector<String> ActiveCaching2dDD::worldAxisNames() const {
		return itsCoordinateSystem.worldAxisNames();
	}

	Vector<String> ActiveCaching2dDD::worldAxisUnits() const {
		return itsCoordinateSystem.worldAxisUnits();
	}

	void ActiveCaching2dDD::setDefaultOptions() {
		CachingDisplayData::setDefaultOptions();
	}

	Bool ActiveCaching2dDD::setOptions(Record &rec, Record &recOut) {
		Bool ret = CachingDisplayData::setOptions(rec, recOut);

// Deal with local things.  These don't issue a refresh, but
// next time you look at the position, they  will be active

		Bool error;

// Position Tracking things.

		if (readOptionRecord(itsVelocityType, error, rec,  "velocitytype") ||
		        readOptionRecord(itsSpectralUnit, error, rec,  "spectralunit")) {
			setSpectralFormatting (itsCoordinateSystem, itsVelocityType, itsSpectralUnit);
		}
//
		String value;
		if (readOptionRecord(value, error, rec,  "absrel")) {
			itsAbsolute = (value=="absolute");
		}
		if (readOptionRecord(value, error, rec,  "pixelworld")) {
			itsShowWorldCoordinate = (value=="world");
		}
		if (readOptionRecord(value, error, rec,  "fracpix")) {
			itsFractionalPixels = (value=="fractional");
		}
//
		Bool localchange;

		localchange = readOptionRecord(itsOptionsAspect, error, rec, "aspect");
		ret =  (ret || localchange);

		return ret;
	}

	Record ActiveCaching2dDD::getOptions() {
		Record rec = CachingDisplayData::getOptions();
//
		Record aspect;
		aspect.define("dlformat", "aspect");
		aspect.define("listname", "aspect ratio");
		aspect.define("ptype", "choice");
		Vector<String> vaspect(3);
		vaspect(0) = "fixed lattice";
		vaspect(1) = "fixed world";
		vaspect(2) = "flexible";
		aspect.define("popt", vaspect);
		aspect.define("default", vaspect(1));
		aspect.define("value", itsOptionsAspect);
		aspect.define("allowunset", False);
		rec.defineRecord("aspect", aspect);
//
//
		Record absrel;
		absrel.define("context", "position_tracking");
		absrel.define("dlformat", "absrel");
		absrel.define("listname", "absolute or relative");
		absrel.define("ptype", "choice");
		Vector<String> v(2);
		v(0) = "absolute";
		v(1) = "relative";
		absrel.define("popt", v);
		absrel.define("default", "absolute");
		if (itsAbsolute) {
			absrel.define("value", "absolute");
		} else {
			absrel.define("value", "relative");
		}
		absrel.define("allowunset", False);
		rec.defineRecord("absrel", absrel);
//
		Record pixworld;
		pixworld.define("context", "position_tracking");
		pixworld.define("dlformat", "pixelworld");
		pixworld.define("listname", "world or pixel coordinates");
		pixworld.define("ptype", "choice");
		v.resize(2);
		v(0) = "world";
		v(1) = "pixel";
		pixworld.define("popt", v);
		pixworld.define("default", "world");
		if (itsShowWorldCoordinate) {
			pixworld.define("value", "world");
		} else {
			pixworld.define("value", "pixel");
		}
		pixworld.define("allowunset", False);
		rec.defineRecord("pixelworld", pixworld);
//
		Record fracpix;
		fracpix.define("context", "position_tracking");
		fracpix.define("dlformat", "fracpix");
		fracpix.define("listname", "fractional or integral pixel coordinates");
		fracpix.define("ptype", "choice");
		v.resize(2);
		v(0) = "fractional";
		v(1) = "integral";
		fracpix.define("popt", v);
		fracpix.define("default", "integral");
		if (itsFractionalPixels) {
			fracpix.define("value", "fractional");
		} else {
			fracpix.define("value", "integral");
		}
		fracpix.define("allowunset", False);
		rec.defineRecord("fracpix", fracpix);
//
		Int after = -1;
		Int iS = itsCoordinateSystem.findCoordinate(Coordinate::SPECTRAL, after);
		if (iS>=0) {
			Record spectralunit;
			spectralunit.define("context", "position_tracking");
			spectralunit.define("dlformat", "spectralunit");
			spectralunit.define("listname", "spectral unit");
			spectralunit.define("ptype", "userchoice");
			Vector<String> vunits(4);
			vunits(0) = "km/s";
			vunits(1) = "m/s";
			vunits(2) = "GHz";
			vunits(3) = "Hz";
			spectralunit.define("popt", vunits);
			spectralunit.define("default", "km/s");
			spectralunit.define("value", itsSpectralUnit);
			spectralunit.define("allowunset", False);
			rec.defineRecord("spectralunit", spectralunit);
//
			Record veltype;
			veltype.define("context", "position_tracking");
			veltype.define("dlformat", "velocitytype");
			veltype.define("listname", "velocity type");
			veltype.define("ptype", "choice");
			vunits.resize(3);
			vunits(0) = "optical";
			vunits(1) = "radio";
			vunits(2) = "true";
			veltype.define("popt", vunits);
			veltype.define("default", "radio");
			veltype.define("value", itsVelocityType);
			veltype.define("allowunset", False);
			rec.defineRecord("veltype", veltype);
		}
//
		return rec;
	}


	Bool ActiveCaching2dDD::sizeControl(WorldCanvasHolder &wch,
	                                    AttributeBuffer &holderBuf) {
		// Set WC CS, draw area and zoom window.  The first 'Active' DD on the
		// WC's DD list handles it.


		// Return if another DD has already done size control.

		if(!wch.isCSmaster(this)) return False;

		if(itsCoordinateSystem.nPixelAxes()<2u) return False;
		// (the AC2dDD constructors and methods should have assured
		// that this is invariantly True after construction themselves
		// for internal object consistency, but so far they do not...)

		// Otherwise, assume 'CSMaster' role: take charge of setting the
		// WC CS (and its 'axis codes'), the canvas draw area , zoom window,
		// and maixmum zoom extents.
		// The CSMaster will also be called upon to perform coordinate
		// conversions for the WC.  (Eventually, the WC CS should really
		// eventually do that; that's what it's there for, after all...).

		WorldCanvas *wCanvas = wch.worldCanvas();

		wCanvas->setCoordinateSystem(itsCoordinateSystem);

		// dk note (5/03): X and Y axis codes are WC Attributes which describe
		// the WC CS's world coordinate types, e.g.  RA J2000 on X, VEL(TOPO) on Y.
		// (They also encode the 'axis in coordinate', which is probably irrelevant).
		//
		// This is still not enough to distinguish, e.g., two Linear or
		// TabularCoordinates.  (It's a good idea for a Caching DD to add axis
		// names as well (at least), in optionsAsAttributes()...).
		//
		// Axis codes need improvement.  They should encode everything needed by
		// a DD in most cases to decide whether it is compatible with the WC's
		// current world coordinates (although if necessary a DD can look at
		// WC CS info directly as well).   If the DD is not compatible, it should
		// return False from conformsToCS(), and should not draw.
		// Ideally, axis codes would not be limited to two world coordinates as
		// they are now.  (In the example above, world space is 3-dimensional, and
		// axis codes should include a Declination world coordinate as well).
		// Neither should it be necessary in all cases to identify one world
		// coordinate with linear X and one with linear Y.

		String xAxis = "xaxiscode (required match)",
		       yAxis = "yaxiscode (required match)";
		Attribute xAxisCode(xAxis, codeWorldAxis(0));
		Attribute yAxisCode(yAxis, codeWorldAxis(1));

		wCanvas->setAttribute(xAxisCode);
		wCanvas->setAttribute(yAxisCode);


		// add info on world coordinate units

		Attribute xAxisUnits("xaxisunits", worldAxisUnits()[0]);
		Attribute yAxisUnits("yaxisunits", worldAxisUnits()[1]);
		wCanvas->setAttribute(xAxisUnits);
		wCanvas->setAttribute(yAxisUnits);


		// The 'zoom window' consists of 'linear' (which usually means
		// data pixel) coordinates for the corners of the desired viewing
		// area: lin{X,Y}{Min,Max} (in other words, the region of data
		// currently on view).  By convention, the _center_ of the data's
		// blc pixel has coordinate 0, its bottom edge is at -.5.
		// The window is determined by one of four sources (in increasing
		// order of priority):

		// 1) The values already on the WC.

		// 2) An order to 'unzoom', or zoom-to-extent (the "resetCoordinates"
		//    Attribute.  This is all this attribute does now; the CSMaster DD's
		//    sizeControl always assumes that the WC CS and corresponding
		//    axis codes are to be [re]set as it chooses).

		// 3) Attributes set through the WC::setZoomRectangleLCS() method (usually
		//    by the mouse zoomer) to order a zoom ("manualZoom{Blc,Trc}").

		// 4) A zoom-to-extent will also occur, overriding any zoom order above,
		//    under the true 'reset' condition, shown below.

		// Any such 'command Attributes' from cases 2) and 3) are removed
		// from the WC after they have been processed.

		Bool reset = !wch.wasCSmaster(this);
		// 'reset' condition: this is the first sizeControl since this
		// dd was made CS master.
		// (If same master is changing axis codes, it should already
		// have set zoom-order attributes appropriately elsewhere).

		static String unZoom = "resetCoordinates",
		              zoomB = "manualZoomBlc", zoomT = "manualZoomTrc";

		Bool unzoom = False;
		if(!reset && wCanvas->existsAttribute(unZoom)) {
			wCanvas->getAttributeValue(unZoom, unzoom);
		}


		Double linXMin=wCanvas->linXMin(), linXMax=wCanvas->linXMax(),
		       linYMin=wCanvas->linYMin(), linYMax=wCanvas->linYMax();
		// Existing settings are the default.

		if (reset || unzoom) {
			linXMin = itsPixelBlc[0];
			linYMin = itsPixelBlc[1];
			linXMax = itsPixelTrc[0];
			linYMax = itsPixelTrc[1];
		}
		// Set zoom window to maximum extents.

		if(!reset &&  wCanvas->existsAttribute(zoomB) &&
		        wCanvas->existsAttribute(zoomT)) {
			Vector<Double> zoomBlc, zoomTrc;
			wCanvas->getAttributeValue(zoomB, zoomBlc);
			wCanvas->getAttributeValue(zoomT, zoomTrc);
			if (zoomBlc.nelements() >= 2 && zoomTrc.nelements() >= 2) {
				linXMin = zoomBlc[0];
				linXMax = zoomTrc[0];
				linYMin = zoomBlc[1];
				linYMax = zoomTrc[1];
			}
		}
		// 'manual' zoom order -- overrides
		// 'unzoom order' (but not 'reset').

		wCanvas->removeAttribute(zoomB);	// '[Un]zoom order' attributes
		wCanvas->removeAttribute(zoomT);	// have been acted upon as
		wCanvas->removeAttribute(unZoom);	// needed--remove them.


		// Make minor zoom window adjustments to assure that the window is not
		// beyond the DD's maximum extents, nor smaller than 1 data pixel.  (In
		// the long run these should not be necessary, but at present at least
		// the latter constraint is required by WorldAxesDD and WC::drawImage()).

		// If requested window is within a single data pixel, use that whole pixel.
		// Otherwise create a window that includes at least the pixel surrounding
		// the requested zoom center (but always keeping within maximum zoom
		// extents).

		Double mn, mx, ctr;

		mn=floor(linXMin+.5);
		mx=floor(linXMax+.5);
		if(mn==mx) ctr=mn;
		else ctr=(linXMin+linXMax)/2.;
		linXMin = max(itsPixelBlc[0], min(itsPixelTrc[0]-1., min(ctr-.5, linXMin)));
		linXMax = max(linXMin+1.,     min(itsPixelTrc[0],    max(ctr+.5, linXMax)));

		mn=floor(linYMin+.5);
		mx=floor(linYMax+.5);
		if(mn==mx) ctr=mn;
		else ctr=(linYMin+linYMax)/2.;
		linYMin = max(itsPixelBlc[1], min(itsPixelTrc[1]-1., min(ctr-.5, linYMin)));
		linYMax = max(linYMin+1.,     min(itsPixelTrc[1],    max(ctr+.5, linYMax)));


		// Adjust draw area size for requested data pixel aspect ratio.

		Double origDrawXsize = Double(wCanvas->canvasDrawXSize()),
		       origDrawYsize = Double(wCanvas->canvasDrawYSize());
		Double screenPixX=origDrawXsize, screenPixY=origDrawYsize;
		// Initial allowed size (won't change for 'flexible' aspect).

		Double dataPixX = linXMax - linXMin,
		       dataPixY = linYMax - linYMin;

		Double screenPixPerDataPixX = screenPixX/dataPixX,
		       screenPixPerDataPixY = screenPixY/dataPixY;


		if( itsOptionsAspect=="fixed world" &&
		        worldAxisUnits()[0]==worldAxisUnits()[1] ) {

			// same number of screen pixels per world increment on X and Y

			Double worldPerDataPixX = ::fabs(worldAxisIncrements()[0]),
			       worldPerDataPixY = ::fabs(worldAxisIncrements()[1]);
			if(worldPerDataPixX>0 && worldPerDataPixY>0) {
				Double screenPixPerWorld = min(screenPixPerDataPixX/worldPerDataPixX,
				                               screenPixPerDataPixY/worldPerDataPixY);
				screenPixX = screenPixPerWorld * worldPerDataPixX * dataPixX;
				screenPixY = screenPixPerWorld * worldPerDataPixY * dataPixY;
			}
		}


		else if (itsOptionsAspect == "fixed lattice") {

			// square data pixels.

			Double screenPixPerDataPix = min(screenPixPerDataPixX,
			                                 screenPixPerDataPixY);
			screenPixX = screenPixPerDataPix*dataPixX;
			screenPixY = screenPixPerDataPix*dataPixY;
		}


		screenPixX = floor(max(3., screenPixX) +.5);	// Provide at least 3 screen
		screenPixY = floor(max(3., screenPixY) +.5);	// pixels to draw into...

		uInt drawXSize = uInt(screenPixX),
		     drawYSize = uInt(screenPixY);


		// Re-center draw area, given its new size.

		uInt drawXOffset = wCanvas->canvasDrawXOffset() +
		                   Int((origDrawXsize-screenPixX)/2. +.5);
		uInt drawYOffset = wCanvas->canvasDrawYOffset() +
		                   Int((origDrawYsize-screenPixY)/2. +.5);


		// WCH will eventually place these onto the WC, defining its zoom window,
		// draw area, and maximum extents.

		holderBuf.add("linXMin", linXMin);
		holderBuf.add("linYMin", linYMin);
		holderBuf.add("linXMax", linXMax);
		holderBuf.add("linYMax", linYMax);
		holderBuf.add("canvasDrawXSize", drawXSize);
		holderBuf.add("canvasDrawYSize", drawYSize);
		holderBuf.add("canvasDrawXOffset", drawXOffset);
		holderBuf.add("canvasDrawYOffset", drawYOffset);
		holderBuf.add("linXMinLimit", itsPixelBlc[0]);
		holderBuf.add("linYMinLimit", itsPixelBlc[1]);
		holderBuf.add("linXMaxLimit", itsPixelTrc[0]);
		holderBuf.add("linYMaxLimit", itsPixelTrc[1]);


		return True;
	}


	Bool ActiveCaching2dDD::conformsToCS(const WorldCanvas& wc) {
		// Determine whether DD can draw on the current coordinate system of the
		// given WC[H].  This implementation simply to compares 'axis
		// codes' for equality if they exist on the WC.  (More sophisticated
		// future approaches may consider, e.g., WC CS projections and reference
		// pixels, as well as the DD's own ability to reproject or transform
		// reference frames, if any).

		String xAxis = "xaxiscode (required match)",
		       yAxis = "yaxiscode (required match)";
		String xcode, ycode;

		return csConformed_ =
		           (!wc.getAttributeValue(xAxis, xcode) || xcode==codeWorldAxis(0))
		           && (!wc.getAttributeValue(yAxis, ycode) || ycode==codeWorldAxis(1));
	}


	void ActiveCaching2dDD::cleanup() {
	}

	AttributeBuffer ActiveCaching2dDD::optionsAsAttributes() {
		AttributeBuffer buffer = CachingDisplayData::optionsAsAttributes();
		return buffer;
	}


	void ActiveCaching2dDD::setCoordinateSystem(const DisplayCoordinateSystem &coordsys,
	        const Vector<Double> &pixblc,
	        const Vector<Double> &pixtrc) {
		itsCoordinateSystem = coordsys;
		itsPixelBlc = pixblc;
		itsPixelTrc = pixtrc;

		// Sanity check
		if ((itsCoordinateSystem.nWorldAxes() != 2) ||
		        (itsCoordinateSystem.nPixelAxes() != 2) ||
		        (pixblc.nelements() != 2) ||
		        (pixtrc.nelements() != 2)) {
			throw(AipsError("Non-2d CoordinateSystem given to "
			                "ActiveCaching2dDD constructor"));
		}

		// convert DisplayCoordinateSystem to canonical units
		// SHOULD NOT DO THIS FOR LINEAR AXES!!!
		/*
		Vector<String> waxisunits = itsCoordinateSystem.worldAxisUnits();
		for (uInt i = 0; i < 2; i++) {
		  Quantity q(1.0, waxisunits(i));
		  q.convert();
		  waxisunits(i) = q.getUnit();
		}
		itsCoordinateSystem.setWorldAxisUnits(waxisunits);
		*/

// Set up formatting used in position tracking

		setSpectralFormatting (itsCoordinateSystem, itsVelocityType, itsSpectralUnit);
	}

	void ActiveCaching2dDD::identifyWorldAxis(Coordinate::Type &type,
	        Int &coordinate,
	        Int &axisincoord,
	        const uInt worldaxisnum) {
		itsCoordinateSystem.findWorldAxis(coordinate, axisincoord, worldaxisnum);
		if ((coordinate < 0) || (axisincoord < 0)) {
			throw(AipsError("Couldn't find requested axis in "
			                "ActiveCaching2dDD::identifyWorldAxis"));
		}
		type = itsCoordinateSystem.type(coordinate);
	}

	String ActiveCaching2dDD::codeWorldAxis(const uInt worldaxisnum) {
		Coordinate::Type ctype;
		Int coordinate, axisincoord;
		identifyWorldAxis(ctype, coordinate, axisincoord, worldaxisnum);
		String retval;
		switch (ctype) {
		case Coordinate::LINEAR: {
			LinearCoordinate lcoord =
			    itsCoordinateSystem.linearCoordinate(coordinate);
			retval = lcoord.showType();
		}
		break;
		case Coordinate::DIRECTION: {
			DirectionCoordinate dcoord =
			    itsCoordinateSystem.directionCoordinate(coordinate);
			retval = dcoord.showType() +
			         MDirection::showType(dcoord.directionType());
		}
		break;
		case Coordinate::SPECTRAL: {
			SpectralCoordinate scoord =
			    itsCoordinateSystem.spectralCoordinate(coordinate);
			retval = scoord.showType() +
			         MFrequency::showType(scoord.frequencySystem());
		}
		break;
		case Coordinate::STOKES: {
			StokesCoordinate scoord =
			    itsCoordinateSystem.stokesCoordinate(coordinate);
			retval = scoord.showType();
		}
		break;
		case Coordinate::TABULAR: {
			TabularCoordinate tcoord =
			    itsCoordinateSystem.tabularCoordinate(coordinate);
			retval = tcoord.showType();
		}
		break;
		default:
			// nothing special
			break;
		}
		char chnum[20]; // axisincoord surely will never be more than 10^19 !
		sprintf(chnum, "%d", axisincoord);
		retval = retval + String(chnum);
		return retval;
	}

	Vector<Double> ActiveCaching2dDD::worldAxisIncrements() const {
		return itsCoordinateSystem.increment();
	}

	void ActiveCaching2dDD::setSpectralFormatting  (DisplayCoordinateSystem& cSys,
	        const String& doppler,
	        const String& unit)
//
// Position tracking is formatted via the CS::format function. Here we
// set up the units for the formatter to use.  If the unit is velocity,
// the Doppler is needed as well.
//
	{
		static LogIO os(LogOrigin("ActiveCaching2dDD", "setSpectralFormatting", WHERE));
		String errorMsg;
		if ( ! cSys.setSpectralFormatting( errorMsg, unit, doppler ) ) {
			os << LogIO::WARN << "Failed to update SpectralCoordinate formatting because" << LogIO::POST;
			os << errorMsg << LogIO::POST;
		}
	}


} //# NAMESPACE CASA - END

