//# DSPixelPoly.cc : Implementation of an absolute pixel DSPoly
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

#include <display/DisplayShapes/DSPixelPoly.h>
#include <display/DisplayShapes/DSScreenPoly.h>
#include <display/DisplayShapes/DSWorldPoly.h>

#include <display/DisplayShapes/DSPixelPolyLine.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSPixelPoly::DSPixelPoly() :
		DSPoly() {
		UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

	}

	DSPixelPoly::DSPixelPoly(const Record& settings) :
		DSPoly() {
		UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

		setOptions(settings);
	}

	DSPixelPoly::DSPixelPoly(DSScreenPoly& other) :
		DSPoly() {

		UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

		DSPoly::setOptions(other.getRawOptions());


	}

	DSPixelPoly::DSPixelPoly(DSWorldPoly& other) :
		DSPoly() {

		UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

		DSPoly::setOptions(other.getRawOptions());

	}

	DSPixelPoly::DSPixelPoly(DSPixelPolyLine& other) :
		DSPoly() {

		UnitMap::putUser("pix", UnitVal(1.0), "absolute pixels");

		Record otherOptions = other.getOptions();
		if (otherOptions.isDefined("polylinepoints")) {
			Record points = otherOptions.subRecord("polylinepoints");
			otherOptions.removeField("polylinepoints");
			otherOptions.defineRecord("polygonpoints", points);
		}

		setOptions(otherOptions);

	}

	DSPixelPoly::~DSPixelPoly() {

	}

	Record DSPixelPoly::getOptions() {
		Record toReturn;

		toReturn = DSPoly::getOptions();

		matrixFloatToQuant(toReturn, "polylinepoints", "pix");

		if (toReturn.isDefined("coords")) {
			toReturn.removeField("coords");
		}
		toReturn.define("coords", "pix");

		return toReturn;
	}

	Bool DSPixelPoly::setOptions(const Record& settings) {

		Bool localChange = False;
		Record toSet = settings;

		if (settings.isDefined("coords")) {
			if (settings.asString("coords") != "pix") {
				throw(AipsError("I (DSPixelPoly) was expecting an "
				                "option record which"
				                " had coords == \'pix\'. Please use a \'lock\' function"
				                " to change my co-ord system"));
			}
		}

		matrixFloatFromQuant(toSet, "polygonpoints", "pix");

		if (DSPoly::setOptions(toSet)) {
			localChange = True;
		}

		return localChange;
	}

} //# NAMESPACE CASA - END

