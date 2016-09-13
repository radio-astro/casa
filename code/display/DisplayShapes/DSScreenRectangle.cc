//# DSScreenRectangle.cc : Implementation of a relative screen pos. DSRectangle
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

#include <display/DisplayShapes/DSScreenRectangle.h>
#include <display/DisplayShapes/DSWorldRectangle.h>
#include <display/DisplayShapes/DSPixelRectangle.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/PanelDisplay.h>

#include <scimath/Mathematics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSScreenRectangle::DSScreenRectangle() :
		DSRectangle(),
		itsPC(0),
		itsRelativeCenter() {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
	}

	DSScreenRectangle::DSScreenRectangle(const Record& settings, PixelCanvas* pc) :
		DSRectangle(),
		itsPC(pc),
		itsRelativeCenter(0) {


		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		setOptions(settings);

	}

	DSScreenRectangle::DSScreenRectangle(DSPixelRectangle& other, PixelCanvas* pc) :
		DSRectangle(),
		itsPC(pc),
		itsRelativeCenter(0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		DSRectangle::setOptions(other.getRawOptions());
		updateRelative();

	}

	DSScreenRectangle::DSScreenRectangle(DSWorldRectangle& other) :
		DSRectangle(),
		itsPC(0),
		itsRelativeCenter(0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		itsPC = other.panelDisplay()->pixelCanvas();

		DSRectangle::setOptions(other.getRawOptions());
		updateRelative();
	}

	DSScreenRectangle::~DSScreenRectangle() {

	}


	Bool DSScreenRectangle::setOptions(const Record& settings) {
		Bool localChange = False;
		Record toSet = settings;

		if (settings.isDefined("coords")) {
			if (settings.asString("coords") != "frac") {
				throw(AipsError("I (DSScreenRectangle) was expecting an option "
				                "record which"
				                " had coords == \'frac\'. Please use a \'lock\' function"
				                " to change my co-ord system"));
			}
		}

		if (DSRectangle::setOptions(toSet)) {
			localChange = True;
		}

		return localChange;
	}

	void DSScreenRectangle::recalculateScreenPosition() {
		if (!itsPC) {
			throw(AipsError("Can't do recalculateScreenPosition() since I don't have"
			                " a valid pixelCanvas, a relative start or a relative "
			                "end saved!"));
		}



	}

	void DSScreenRectangle::move(const Float& dX, const Float& dY) {
		DSRectangle::move(dX, dY);
		updateRelative();
	}

	void DSScreenRectangle::setCenter(const Float& xPos, const Float& yPos) {
		DSRectangle::setCenter(xPos, yPos);
		updateRelative();
	}

	void DSScreenRectangle::rotate(const Float& angle) {
		DSRectangle::rotate(angle);
		updateRelative();
	}

	void DSScreenRectangle::changePoint(const Vector<Float>&pos, const Int n) {
		DSRectangle::changePoint(pos, n);
		updateRelative();
	}

	void DSScreenRectangle::changePoint(const Vector<Float>& pos) {
		DSRectangle::changePoint(pos);
		updateRelative();
	}


	void DSScreenRectangle::updateRelative() {


	}


	Record DSScreenRectangle::getOptions() {

		Record toReturn;

		toReturn = DSRectangle::getOptions();

		// Shouldn't happen (should never be defined) .. but why not
		if (toReturn.isDefined("coords")) {
			toReturn.removeField("coords");
		}

		toReturn.define("coords", "frac");

		return toReturn;

	}

} //# NAMESPACE CASA - END

