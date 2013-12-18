//# DSWorldRectangle.cc : Implementation of a world coords DSRectangle
//# Copyright (C) 1998,1999,2000,2001,2002,2003
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
//# $Id:
#include <display/DisplayShapes/DSWorldRectangle.h>
#include <display/DisplayShapes/DSPixelRectangle.h>
#include <display/DisplayShapes/DSScreenRectangle.h>

#include <casa/Quanta/UnitMap.h>
#include <casa/Containers/List.h>
#include <display/Display/WorldCanvas.h>
#include <casa/Arrays/ArrayMath.h>

#include <images/Images/ImageUtilities.h>
#include <display/Display/DisplayCoordinateSystem.h>

#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSWorldRectangle::DSWorldRectangle() :
		DSRectangle(),
		itsPD(0),
		itsWC(0),
		itsWorldParameters(0) {

		// Default cons. We know nothing about anything.

	}

	DSWorldRectangle::DSWorldRectangle(const Record& settings, PanelDisplay* pd) :
		DSRectangle(),
		itsPD(pd),
		itsWC(0),
		itsWorldParameters(0) {

		setOptions(settings);
		recalculateScreenPosition();

	}

	DSWorldRectangle::DSWorldRectangle(DSScreenRectangle& other, PanelDisplay* pd) :
		DSRectangle(),
		itsPD(pd),
		itsWC(0),
		itsWorldParameters(0) {

		Record shapeSettings = other.getRawOptions();
		DSRectangle::setOptions(shapeSettings);
		updateWCoords();

	}

	DSWorldRectangle::DSWorldRectangle(DSPixelRectangle& other, PanelDisplay* pd) :
		DSRectangle(),
		itsPD(pd),
		itsWC(0),
		itsWorldParameters(0) {

		Record shapeSettings = other.getRawOptions();
		DSRectangle::setOptions(shapeSettings);
		updateWCoords();

	}

	DSWorldRectangle::~DSWorldRectangle() {

	}


	void DSWorldRectangle::recalculateScreenPosition() {

		if (!itsWC) {
			if (getOptions().isDefined("center")) {
				throw (AipsError("Couldn't recalculate screen position for WCRectangle "
				                 "since I didn't have a valid worldcanvas"));
			} else return;
		}

		Vector<Double> pixelParams(3);
		LogOrigin itsLogO("DisplayShapeInterface", " ");
		LogIO itsLogger(itsLogO);

		static IPosition pixelAxes(2, 0, 1);

		ImageUtilities::worldWidthsToPixel(pixelParams,
		                                   itsWorldParameters,
		                                   itsWC->coordinateSystem(),
		                                   pixelAxes);

		// get x and y world center;
		Vector<Double> wcCenter(2);
		Vector<Double> pixelDbl(2);

		wcCenter(0) = itsWorldParameters(0).getValue();
		wcCenter(1) = itsWorldParameters(1).getValue();

		if (!itsWC->worldToPix(pixelDbl, wcCenter)) {
			throw (AipsError("Couldn't turn the center of the ellipse into "
			                 "pixel co-ordinates"));
		}

		DSRectangle::setCenter(Float(pixelDbl(0)), Float(pixelDbl(1)));
		DSRectangle::setWidth(pixelParams(0));
		DSRectangle::setHeight(pixelParams(1));

		// No other way to do this I don't think;
		Record rec;

		rec.define("angle", (pixelParams(2)*(180 / C::pi)));

		DSRectangle::setOptions(rec);
	}


	Bool DSWorldRectangle::setOptions(const Record& settings) {

		Bool localChange = False;
		Record toSet = settings;

		if (settings.isDefined("coords")) {
			if (settings.asString("coords") != "world") {
				throw(AipsError("I (DSWorldRectangle) was expecting an option record which"
				                " had coords == \'world\'. Please use a \'lock\' or"
				                " \'revert\' function"
				                " to change my co-ord system"));
			}
		}

		if (settings.isDefined("center")) {
			toSet.removeField("center");
			localChange = True;
		}
		if (settings.isDefined("majoraxis")) {
			toSet.removeField("majoraxis");
			localChange = True;
		}
		if (settings.isDefined("minoraxis")) {
			toSet.removeField("minoraxis");
			localChange = True;
		}
		if (settings.isDefined("angle")) {
			toSet.removeField("angle");
			localChange = True;
		}


		// TODO :: Must do a world to pix....

		if (DSRectangle::setOptions(toSet)) {
			localChange = True;
		}

		return localChange;
	}

	Record DSWorldRectangle::getOptions() {

		Record toReturn;
		toReturn = DSRectangle::getOptions();

		// Plus lots of stuff to go here!

		// Shouldn't happen (should never be defined) .. but why not
		if (toReturn.isDefined("coords")) {
			toReturn.removeField("coords");
		}

		toReturn.define("coords", "world");
		return toReturn;
	}

	/*
	Bool DSWorldRectangle::chooseWC(const Float& centerX, const Float& centerY,
				 PanelDisplay* pd,
				 WorldCanvas* wc) {
	  Bool success = False;

	  // Look for ones where the point is in WC and in draw area
	  ListIter<WorldCanvas* >* wcs = pd->wcs();

	  wcs->toStart();

	  while( !wcs->atEnd() && !success) {

	  if (wcs->getRight()->inWC(Int(centerX), Int(centerY)) &&
		wcs->getRight()->inDrawArea(Int(centerX), Int(centerY))) {

	      itsWC = wcs->getRight();
	      success = True;


	    } else {
	      wcs->step();
	    }
	  }

	  // if that returns nothing, look for one just in WC..
	  if (!success) {
	    wcs->toStart();
	    while(!wcs->atEnd() && !success) {
	      if (wcs->getRight()->inWC(centerX, centerY)) {
		itsWC = wcs->getRight();
		success = True;
	      } else {
		wcs++;
	      }
	    }
	  }

	  return success;
	}

	*/

	void DSWorldRectangle::updateWCoords() {

		Vector<Float> pixelCenter;

		// Get the pixel options.
		Record settings = getRawOptions();

		if (!itsWC) {
			if (settings.isDefined("center")) {
				if (settings.dataType("center") == TpArrayFloat) {
					pixelCenter = settings.asArrayFloat("center");
					itsWC = chooseWCFromPixPoint(pixelCenter(0), pixelCenter(1),
					                             itsPD);
					if (!itsWC) {
						throw(AipsError("Couldn't find a world canvas based on "
						                "info returned from shape" ));
					}
				} else {
					throw(AipsError("Bad data format for \'center\' returned from"
					                " shape." ));
				}
			} else {
				throw(AipsError("No \'center\' field defined and no valid world "
				                "canvas"));
			}
		}

		if (!(settings.isDefined("majoraxis") && settings.isDefined("minoraxis")
		        && settings.isDefined("angle"))) {
			throw(AipsError("Not enough information to do a pixel->world conversion"
			                " of an ellipse"));
		}

		pixelCenter = settings.asArrayFloat("center");
		Float majorAxis = settings.asFloat("majoraxis");
		Float minorAxis = settings.asFloat("minoraxis");

		// Shape (deg) -> imageutils (rad)
		Float angle = (C::pi / 180) * settings.asFloat("angle");


		Vector<Double> pixelParams(5);

		pixelParams(0) = Double(pixelCenter(0));
		pixelParams(1) = Double(pixelCenter(1));
		pixelParams(2) = Double(majorAxis);
		pixelParams(3) = Double(minorAxis);
		pixelParams(4) = Double(angle);


		GaussianBeam mamipa;
		LogOrigin itsLogO("DisplayShapeInterface", " ");
		LogIO itsLogger(itsLogO);

		static IPosition pixelAxes(2, 0, 1);

		ImageUtilities::pixelWidthsToWorld(mamipa, pixelParams,
		                                   itsWC->coordinateSystem(),
		                                   pixelAxes);

		Vector<Double> centerpix(2);
		centerpix(0) = Double(pixelParams(0));
		centerpix(1) = Double(pixelParams(1));
		Vector<Double> worldcent(2);

		if (!itsWC->pixToWorld(worldcent, centerpix)) {
			throw(AipsError("Failure on DSWorldRectangle pix->world"));
		}

		// TODO : UNITS!!!!
		Quantity x(worldcent(0), "rad");
		Quantity y(worldcent(1), "rad");

		itsWorldParameters.resize(5);
		itsWorldParameters(0) = x;
		itsWorldParameters(1) = y;
		itsWorldParameters(2) = mamipa.getMajor();
		itsWorldParameters(3) = mamipa.getMinor();
		itsWorldParameters(4) = mamipa.getPA();

	}


	void DSWorldRectangle::move(const Float& dX, const Float& dY) {
		DSRectangle::move(dX, dY);
		updateWCoords();
		recalculateScreenPosition();
	}

	void DSWorldRectangle::setCenter(const Float& xPos, const Float& yPos) {

		DSRectangle::setCenter(xPos, yPos);
		updateWCoords();

	}

	void DSWorldRectangle::draw(PixelCanvas* pc) {
		if (itsWC) {
			Vector<Float> cent = getCenter();

			if ( itsWC->inDrawArea(Int(cent(0)+0.5), Int(cent(1)+0.5)) ) {
				DSRectangle::draw(pc);
			}

		}
	}

	void DSWorldRectangle::rotate(const Float& angle) {

		DSRectangle::rotate(angle);
		updateWCoords();

	}


	void DSWorldRectangle::changePoint(const Vector<Float>&pos, const Int n) {
		DSRectangle::changePoint(pos, n);
		updateWCoords();
	}

	void DSWorldRectangle::changePoint(const Vector<Float>& pos) {

		DSRectangle::changePoint(pos);
		updateWCoords();
	}

} //# NAMESPACE CASA - END

