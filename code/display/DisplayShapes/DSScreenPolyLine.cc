//# DSScreenPolyLine.cc : Implementation of a relative scren pos. DSPolyLine
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

#include <display/DisplayShapes/DSScreenPolyLine.h>
#include <display/DisplayShapes/DSWorldPolyLine.h>
#include <display/DisplayShapes/DSPixelPolyLine.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/PanelDisplay.h>

#include <scimath/Mathematics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSScreenPolyLine::DSScreenPolyLine() :
		DSPolyLine(),
		itsPC(0),
		itsRelativePoints(0,0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
	}

	DSScreenPolyLine::DSScreenPolyLine(const Record& settings, PixelCanvas* pc) :
		DSPolyLine(),
		itsPC(pc),
		itsRelativePoints(0,0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		setOptions(settings);

	}

	DSScreenPolyLine::DSScreenPolyLine(DSPixelPolyLine& other, PixelCanvas* pc) :
		DSPolyLine(),
		itsPC(pc),
		itsRelativePoints(0,0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		DSPolyLine::setOptions(other.getRawOptions());
		updateRelative();
	}

	DSScreenPolyLine::DSScreenPolyLine(DSWorldPolyLine& other) :
		DSPolyLine(),
		itsPC(0),
		itsRelativePoints(0,0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		itsPC = other.panelDisplay()->pixelCanvas();

		DSPolyLine::setOptions(other.getRawOptions());
		updateRelative();
	}

	DSScreenPolyLine::~DSScreenPolyLine() {

	}


	Bool DSScreenPolyLine::setOptions(const Record& settings) {
		Bool localChange = False;
		Record toSet = settings;

		if (settings.isDefined("coords")) {
			if (settings.asString("coords") != "frac") {
				throw(AipsError("I (DSScreenPolyLine) was expecting an option "
				                "record which"
				                " had coords != \'frac\'. Please use a \'lock\' function"
				                " to change my co-ord system"));
			}
		}

		matrixFloatFromQuant(toSet, "polylinepoints", "frac");
		if (toSet.isDefined("polylinepoints")) {

			Matrix<Float> rel = toSet.asArrayFloat("polylinepoints");
			Matrix<Float> screen = relToScreen(rel, itsPC);

			toSet.removeField("polylinepoints");
			toSet.define("polylinepoints", screen);
		}


		if (DSPolyLine::setOptions(toSet)) {
			localChange = True;
		}

		return localChange;
	}



	void DSScreenPolyLine::recalculateScreenPosition() {
		if (itsRelativePoints.nelements() > 0) {
			if (!itsPC) {
				throw(AipsError("No pixel canvas available to update screen pos for"
				                " ScreenPolyLine"));
			}

			if (itsRelativePoints.ncolumn() != 2) {
				throw(AipsError("Bad format for relative Points Matrix"));
			}

			Matrix<Float> pixel = relToScreen(itsRelativePoints, itsPC);
			setPoints(pixel);
		}
	}

	void DSScreenPolyLine::move(const Float& dX, const Float& dY) {
		DSPolyLine::move(dX, dY);
		updateRelative();
	}

	void DSScreenPolyLine::setCenter(const Float& xPos, const Float& yPos) {
		DSPolyLine::setCenter(xPos, yPos);
		updateRelative();
	}

	void DSScreenPolyLine::rotate(const Float& angle) {
		DSPolyLine::rotate(angle);
		updateRelative();
	}

	void DSScreenPolyLine::scale(const Float& scaleFactor) {
		DSPolyLine::scale(scaleFactor);
		updateRelative();
	}

	void DSScreenPolyLine::changePoint(const Vector<Float>&pos, const Int n) {
		DSPolyLine::changePoint(pos, n);
		updateRelative();
	}

	void DSScreenPolyLine::changePoint(const Vector<Float>& pos) {
		DSPolyLine::changePoint(pos);
		updateRelative();
	}


	void DSScreenPolyLine::updateRelative() {
		Matrix<Float> pixel = getPoints();

		if (pixel.nelements() > 0) {
			if (!itsPC) {
				throw(AipsError("No pixel canvas available to update screen pos for"
				                " ScreenPolyLine"));
			}

			if (pixel.ncolumn() != 2) {
				throw(AipsError("Bad format for pixel Points Matrix returned "
				                "from shape"));
			}

			itsRelativePoints.resize(pixel.nrow(), pixel.ncolumn());
			itsRelativePoints = screenToRel(pixel, itsPC);

		}
	}

	void DSScreenPolyLine::addPoint(const Vector<Float>& newPos) {
		DSPolyLine::addPoint(newPos);
		updateRelative();
	}

	void DSScreenPolyLine::setPoints(const Matrix<Float>& points) {
		DSPolyLine::setPoints(points);
		updateRelative();
	}


	Record DSScreenPolyLine::getOptions() {

		Record toReturn;

		toReturn = DSPolyLine::getOptions();

		if (toReturn.isDefined("polylinepoints")) {
			Matrix<Float> pixel = toReturn.asArrayFloat("polylinepoints");
			Matrix<Float> relative = screenToRel(pixel, itsPC);
			toReturn.removeField("polylinepoints");
			toReturn.define("polylinepoints", relative);

			matrixFloatToQuant(toReturn, "polylinepoints", "frac");
		}

		// Shouldn't happen (should never be defined) .. but why not
		if (toReturn.isDefined("coords")) {
			toReturn.removeField("coords");
		}

		toReturn.define("coords", "frac");

		return toReturn;

	}


} //# NAMESPACE CASA - END

