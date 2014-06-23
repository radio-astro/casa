//# DSClosed.cc: Base class for all 'closed' DisplayShapes
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

#include <casa/aips.h>

#include <display/DisplayShapes/DSClosed.h>
#include <display/Display/PixelCanvas.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSClosed::DSClosed() :
		DSBasic() {
		setDefaultOptions();
	}

	DSClosed::DSClosed(const DSClosed& other) :
		DSBasic(other),
		itsFillStyle(other.itsFillStyle),
		itsFillColor(other.itsFillColor) {
	}

	DSClosed::~DSClosed() {
	}

	void DSClosed::setFillStyle(DSClosed::FillStyle fill) {
		itsFillStyle = fill;
	}

	DSClosed::FillStyle DSClosed::getFillStyle() {
		return itsFillStyle;
	}

	void DSClosed::setFillColor(String color) {
		itsFillColor = color;
	}

	String DSClosed::getFillColor() {
		return itsFillColor;
	}

	void DSClosed::draw(PixelCanvas *pix) {
		DSBasic::draw(pix);
	}

	void DSClosed::rotateAbout(const Float& angle, const Float& aboutX,
	                           const Float& aboutY) {
		DSBasic::rotateAbout(angle, aboutX, aboutY);
	}

	void DSClosed::move(const Float& dX, const Float& dY) {
		DSBasic::move(dX, dY);
	}

	Record DSClosed::getOptions() {
		Record rec = DSBasic::getOptions();

		rec.define("type", "closedshape");
		rec.define("fillStyle", itsFillStyle);

		rec.define("fillColor", itsFillColor); // Probably not needed.
		//I think color = fillColor, imposed by pixelCanvas?

		return rec;
	}

	Bool DSClosed::setOptions(const Record& settings) {
		Bool localChange = False;
		if (settings.isDefined("fillstyle")) {
			Int temp(settings.asInt("fillstyle"));
			itsFillStyle = static_cast<DSClosed::FillStyle>(temp);
		}
		if (settings.isDefined("fillcolor"))
			itsFillColor = settings.asString("fillcolor");

		if (DSBasic::setOptions(settings)) localChange = True;

		return localChange;
	}

	void DSClosed::setDefaultOptions() {
		itsFillStyle = DSClosed::No_Fill;
		itsFillColor = "white";
	}





} //# NAMESPACE CASA - END

