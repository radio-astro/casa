//# DSScreenEllipse.cc : Implementation of a relative screen pos. DSEllipse
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

#include <display/DisplayShapes/DSScreenEllipse.h>
#include <display/DisplayShapes/DSWorldEllipse.h>
#include <display/DisplayShapes/DSPixelEllipse.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/PanelDisplay.h>

#include <scimath/Mathematics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSScreenEllipse::DSScreenEllipse() :
		DSEllipse(),
		itsPC(0),
		itsRelativeCenter() {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
	}

	DSScreenEllipse::DSScreenEllipse(const Record& settings, PixelCanvas* pc) :
		DSEllipse(),
		itsPC(pc),
		itsRelativeCenter(0) {


		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		setOptions(settings);

	}

	DSScreenEllipse::DSScreenEllipse(DSPixelEllipse& other, PixelCanvas* pc) :
		DSEllipse(),
		itsPC(pc),
		itsRelativeCenter(0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		DSEllipse::setOptions(other.getRawOptions());
		updateRelative();

	}

	DSScreenEllipse::DSScreenEllipse(DSWorldEllipse& other) :
		DSEllipse(),
		itsPC(0),
		itsRelativeCenter(0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		itsPC = other.panelDisplay()->pixelCanvas();

		DSEllipse::setOptions(other.getRawOptions());
		updateRelative();
	}

	DSScreenEllipse::~DSScreenEllipse() {

	}


	Bool DSScreenEllipse::setOptions(const Record& settings) {
		Bool localChange = False;
		Record toSet = settings;

		if (settings.isDefined("coords")) {
			if (settings.asString("coords") != "frac") {
				throw(AipsError("I (DSScreenEllipse) was expecting an option "
				                "record which"
				                " had coords == \'frac\'. Please use a \'lock\' function"
				                " to change my co-ord system"));
			}
		}

		if (DSEllipse::setOptions(toSet)) {
			localChange = True;
		}

		return localChange;
	}

	void DSScreenEllipse::recalculateScreenPosition() {
		if (!itsPC) {
			throw(AipsError("Can't do recalculateScreenPosition() since I don't have"
			                " a valid pixelCanvas, a relative start or a relative "
			                "end saved!"));
		}



	}

	void DSScreenEllipse::move(const Float& dX, const Float& dY) {
		DSEllipse::move(dX, dY);
		updateRelative();
	}

	void DSScreenEllipse::setCenter(const Float& xPos, const Float& yPos) {
		DSEllipse::setCenter(xPos, yPos);
		updateRelative();
	}

	void DSScreenEllipse::rotate(const Float& angle) {
		DSEllipse::rotate(angle);
		updateRelative();
	}

	void DSScreenEllipse::changePoint(const Vector<Float>&pos, const Int n) {
		DSEllipse::changePoint(pos, n);
		updateRelative();
	}

	void DSScreenEllipse::changePoint(const Vector<Float>& pos) {
		DSEllipse::changePoint(pos);
		updateRelative();
	}


	void DSScreenEllipse::updateRelative() {


	}


	Record DSScreenEllipse::getOptions() {

		Record toReturn;

		toReturn = DSEllipse::getOptions();

		// Shouldn't happen (should never be defined) .. but why not
		if (toReturn.isDefined("coords")) {
			toReturn.removeField("coords");
		}

		toReturn.define("coords", "frac");

		return toReturn;

	}







} //# NAMESPACE CASA - END

