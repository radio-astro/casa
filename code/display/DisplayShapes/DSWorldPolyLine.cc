//# DSWorldPolyLine.cc : Implementation of a world coords DSPolyLine
//# Copyright (C) 1998,1999,2000,2001,2002
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

#include <display/DisplayShapes/DSWorldPolyLine.h>
#include <display/DisplayShapes/DSPixelPolyLine.h>
#include <display/DisplayShapes/DSScreenPolyLine.h>

#include <casa/Quanta/UnitMap.h>
#include <casa/Containers/List.h>
#include <display/Display/WorldCanvas.h>
#include <casa/Arrays/ArrayMath.h>

#include <display/Display/DisplayCoordinateSystem.h>
namespace casa { //# NAMESPACE CASA - BEGIN

	DSWorldPolyLine::DSWorldPolyLine() :
		DSPolyLine(),
		itsPD(0),
		itsWC(0),
		itsWorldXPoints(0),
		itsWorldYPoints(0) {

		// Default cons. We know nothing about anything.

	}

	DSWorldPolyLine::DSWorldPolyLine(const Record& settings, PanelDisplay* pd) :
		DSPolyLine(),
		itsPD(pd),
		itsWC(0),
		itsWorldXPoints(0),
		itsWorldYPoints(0) {
		setOptions(settings);
	}

	DSWorldPolyLine::DSWorldPolyLine(DSScreenPolyLine& other, PanelDisplay* pd) :
		DSPolyLine(),
		itsPD(pd),
		itsWC(0),
		itsWorldXPoints(0),
		itsWorldYPoints(0) {

		Record shapeSettings = other.getRawOptions();
		DSPolyLine::setOptions(shapeSettings);
		updateWCoords();

	}

	DSWorldPolyLine::DSWorldPolyLine(DSPixelPolyLine& other, PanelDisplay* pd) :
		DSPolyLine(),
		itsPD(pd),
		itsWC(0),
		itsWorldXPoints(0),
		itsWorldYPoints(0) {

		Record shapeSettings = other.getRawOptions();
		DSPolyLine::setOptions(shapeSettings);
		updateWCoords();
	}

	DSWorldPolyLine::~DSWorldPolyLine() {

	}

	void DSWorldPolyLine::recalculateScreenPosition() {
		if (!itsWC) {
			Matrix<Float> points = getPoints();
			if (points.nelements()) {

				// Slightly dodgy... but probably more appropriate than a throw.
				// We will be using "old" screen pos i.e. before a "recalculation"
				// to find our WC.

				itsWC = chooseWCFromPixPoints(points, itsPD);
				if (!itsWC) {
					throw(AipsError("Couldn't recalculate screen pos of polygon "
					                "as I couldn't find a valid WC to use"));
				}
			} else {
				return;
			}
		}

		Matrix<Float> pointsToSet(itsWorldXPoints.nelements(), 2);

		// TODO Units here..

		for (uInt i=0; i<itsWorldXPoints.nelements(); i++) {
			Vector<Double> worldPoint(2);
			Vector<Double> pixPoint(2);
			worldPoint(0) = itsWorldXPoints(i).getValue();
			worldPoint(1) = itsWorldYPoints(i).getValue();
			itsWC->worldToPix(pixPoint, worldPoint);
			pointsToSet(i,0) = Float(pixPoint(0));
			pointsToSet(i,1) = Float(pixPoint(1));
		}
		DSPolyLine::setPoints(pointsToSet);

	}

	void DSWorldPolyLine::draw(PixelCanvas* pc) {

		if (itsWC && getPoints().nelements()) {

			Matrix<Float> pnts = getPoints();
			if (inWorldCanvasDrawArea(pnts, itsWC)) {
				DSPolyLine::draw(pc);
			}
		}
	}


	Bool DSWorldPolyLine::setOptions(const Record& settings) {
		Bool localChange = False;
		Record toSet = settings;

		if (settings.isDefined("coords")) {
			if (settings.asString("coords") != "world") {
				throw(AipsError("I (DSWorldPolyLine) was expecting an option record which"
				                " had coords == \'world\'. Please use a \'lock\' or"
				                " \'revert\' function"
				                " to change my co-ord system"));
			}
		}


		if (settings.isDefined("polylinepoints")) {
			if (!itsWC) {
				cerr << "NYI - DSWorldPolyLine.cc - #152" << endl;
				throw (AipsError("NYI"));
			}

			// TODO fix this
			matrixFloatFromQuant(toSet, "polylinepoints", "?");

			Matrix<Float> worldPoints = toSet.asArrayFloat("polylinepoints");
			Matrix<Float> pixelPoints(worldPoints.nrow(), worldPoints.ncolumn());

			for (uInt i=0 ; i < worldPoints.nrow() ; i ++) {
				Vector<Double> wrld(2);
				Vector<Double> pix(2);
				wrld(0) = Double(worldPoints(i, 0));
				wrld(1) = Double(worldPoints(i, 1));
				itsWC->worldToPix(pix, wrld);

				pixelPoints(i,0) = Float(pix(0));
				pixelPoints(i,1) = Float(pix(1));
			}
			toSet.removeField("polylinepoints");
			toSet.define("polylinepoints", pixelPoints);

		}

		if (DSPolyLine::setOptions(toSet)) {
			localChange = True;
		}

		return localChange;
	}

	Record DSWorldPolyLine::getOptions() {
		Record toReturn;
		toReturn = DSPolyLine::getOptions();

		if (toReturn.isDefined("polylinepoints")) {
			if(toReturn.dataType("polylinepoints") != TpArrayFloat) {
				throw(AipsError("Bad data type returned for field \'polylinepoints\'"));
			}

			if (!itsWC) {
				itsWC = chooseWCFromPixPoints(toReturn.asArrayFloat("polylinepoints"),
				                              itsPD);

				if (!itsWC) {
					throw(AipsError("DSWorldPolyLine couldn't convert to world  position "
					                "since "
					                "it couldn't find a valid worldcanvas"));
				}

			}


			// -> World options.
			/*
			Vector<String> units = itsWC->coordinateSystem().worldAxisUnits();
			String encodeTo = units(0);

			for (uInt i=0; i < units.nelements(); i++) {
			if (units(i) != encodeTo) {
			  throw(AipsError("Not sure yet how to deal with the sitatuion that "
				      "arose in DSWorldPoly.cc #199!?!"));
				      }
			}

			//  matrixFloatToQuant(toReturn, "polylinepoints", encodeTo);
			}
			*/

			pixelToWorldPoints(toReturn, "polylinepoints", itsWC);

		}
		// Shouldn't happen (should never be defined) .. but why not
		if (toReturn.isDefined("coords")) {
			toReturn.removeField("coords");
		}

		toReturn.define("coords", "world");
		return toReturn;
	}


	void DSWorldPolyLine::updateWCoords() {
		Matrix<Float> pixelPoints = getPoints();

		if (pixelPoints.nelements()) {

			if (!itsWC) {
				itsWC = chooseWCFromPixPoints(pixelPoints, itsPD);
				if (!itsWC) {
					throw(AipsError("Couldn't update world coordinates of polyline "
					                "as I couldn't find a valid WC to use"));
				}
			}
			// Ok, valid itsWC and points

			Vector<String> units = itsWC->coordinateSystem().worldAxisUnits();
			String encodeTo = units(0);

			for (uInt i=0; i < units.nelements(); i++) {
				if (units(i) != encodeTo) {
					throw(AipsError("Not sure *yet* how to deal with the sitatuion that "
					                "arose in DSWorldPolyLine.cc #252!?!"));
				}
			}

			itsWorldXPoints.resize(pixelPoints.nrow());
			itsWorldYPoints.resize(pixelPoints.nrow());

			for (uInt i=0; i < pixelPoints.nrow() ; i ++) {
				Vector<Double> pixPoint(2);
				Vector<Double> worldPoint(2);
				pixPoint(0) = Double(pixelPoints(i,0));
				pixPoint(1) = Double(pixelPoints(i,1));

				cerr << "DSWOrldPolyLine update WCoords... converting " <<
				     pixPoint(0) << "," << pixPoint(1) << " to world" << endl;

				if (!itsWC->pixToWorld(worldPoint, pixPoint)) {
					throw(AipsError("Couldn't convert pix->world"));
				}

				itsWorldXPoints(i) = Quantity(worldPoint(0), encodeTo);
				itsWorldYPoints(i) = Quantity(worldPoint(1), encodeTo);
			}

		}
	}


	void DSWorldPolyLine::move(const Float& dX, const Float& dY) {
		DSPolyLine::move(dX, dY);
		updateWCoords();
	}

	void DSWorldPolyLine::setCenter(const Float& xPos, const Float& yPos) {
		DSPolyLine::setCenter(xPos, yPos);
		updateWCoords();
	}

	void DSWorldPolyLine::rotate(const Float& angle) {
		DSPolyLine::rotate(angle);
		updateWCoords();
	}

	void DSWorldPolyLine::scale(const Float& scaleFactor) {
		DSPolyLine::scale(scaleFactor);
		updateWCoords();
	}

	void DSWorldPolyLine::addPoint(const Vector<Float>& newPos) {
		DSPolyLine::addPoint(newPos);
		updateWCoords();
	}

	void DSWorldPolyLine::setPoints(const Matrix<Float>& points) {
		DSPolyLine::setPoints(points);
		updateWCoords();
	}

	void DSWorldPolyLine::changePoint(const Vector<Float>&pos, const Int n) {
		DSPolyLine::changePoint(pos, n);
		updateWCoords();
	}

	void DSWorldPolyLine::changePoint(const Vector<Float>& pos) {
		DSPolyLine::changePoint(pos);
		updateWCoords();
	}








} //# NAMESPACE CASA - END

