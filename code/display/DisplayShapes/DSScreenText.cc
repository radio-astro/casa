//# DSScreenText.cc : Implementation of a relative screen pos. DSText
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

#include <display/DisplayShapes/DSScreenText.h>
#include <display/DisplayShapes/DSWorldText.h>
#include <display/DisplayShapes/DSPixelText.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitMap.h>

#include <casa/Containers/Record.h>

#include <display/Display/PixelCanvas.h>
#include <display/Display/PanelDisplay.h>

#include <scimath/Mathematics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSScreenText::DSScreenText() :
		DSText(),
		itsPC(0),
		itsRelativeCenter(0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
	}

	DSScreenText::DSScreenText(const Record& settings, PixelCanvas* pc) :
		DSText(),
		itsPC(pc),
		itsRelativeCenter(0) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
		setOptions(settings);

	}

	DSScreenText::DSScreenText(DSPixelText& other, PixelCanvas* pc) :
		DSText(),
		itsPC(pc) {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");
		DSText::setOptions(other.getRawOptions());
		updateRC();
	}

	DSScreenText::DSScreenText(DSWorldText& other) :
		DSText() {

		UnitMap::putUser("frac", UnitVal(1.0), "fractional units");

		itsPC = other.panelDisplay()->pixelCanvas();

		DSText::setOptions(other.getRawOptions());
		updateRC();
	}

	DSScreenText::~DSScreenText() {

	}



	Bool DSScreenText::setOptions(const Record& settings) {
		Bool localChange = False;
		Record toSet = settings;

		if (settings.isDefined("coords")) {
			if (settings.asString("coords") != "frac") {
				throw(AipsError("I (DSScreenText) was expecting an option record which"
				                " had coords == \'frac\'. Please use a \'lock\' function"
				                " to change my co-ord system"));
			}
		}

		if (settings.isDefined("center")) {
			if (settings.dataType("center") == TpRecord) {
				Record centerField = settings.subRecord("center");
				if (centerField.nfields() != 2) {
					throw(AipsError("Bad record to setoptions() of DSScreenText, since "
					                "the "
					                "field \'center\' must be composed of two subfields"
					                " (x, y)"));
				}

				Record x = centerField.subRecord(0);
				Record y = centerField.subRecord(1);

				QuantumHolder xh, yh;
				String error;

				xh.fromRecord(error,x);
				if (error.length() != 0) {
					throw(AipsError("Bad record to setoptions() of DSScreenText, since "
					                "extracting the \'x\' (first element) from the center"
					                " field caused an error: " + error));
				}

				yh.fromRecord(error,y);
				if (error.length() != 0) {
					throw(AipsError("Bad record to setoptions() of DSScreenText, since "
					                "extracting the \'y\' (second element) from the center"
					                " field caused an error: " + error));
				}

				if ((xh.asQuantumFloat().getFullUnit().getName() != "frac") ||
				        (yh.asQuantumFloat().getFullUnit().getName() != "frac")) {
					throw(AipsError("Bad record to setoptions() of DSScreenText, since "
					                "the units were incorrect for the field center ("
					                "I was expecting \'frac\' units!)"));

				}

				Float relX = xh.asQuantumFloat().getValue();
				Float relY = yh.asQuantumFloat().getValue();

				itsRelativeCenter.resize(2);
				itsRelativeCenter(0) = relX;
				itsRelativeCenter(1) = relY;

				Vector<Float> screen(2);
				screen = relToScreen(itsRelativeCenter);

				toSet.removeField("center");


				DSText::setCenter(screen(0), screen(1));

			} else if (settings.dataType("center") == TpArrayFloat) {

				throw(AipsError("Bad record to setoptions() of DSScreenText, since "
				                "the field"
				                " \'center\' must be of type quanta, not an array"));
			} else {

				throw(AipsError("Bad record to setoptions() of DSScreenText, since "
				                "the field"
				                " \'center\' was of an unknown type"));
			}
		}

		if (DSText::setOptions(toSet)) {
			localChange = True;
		}

		return localChange;
	}

	Vector<Float> DSScreenText::relToScreen(const Vector<Float>& rel) {

		if (rel(0) < 0 || rel(0) > 1 || rel(1) < 0 || rel(1) > 1 ) {
			throw(AipsError("Relative co-ordinates are only valid of they are between"
			                " 0 and 1."));

		}

		if (!itsPC) {
			throw(AipsError("Can't do relative to screen if I don't have a valid"
			                " pixelCanvas!"));
		}


		Vector<Float> pix(2);

		pix(0) = itsPC->width() * rel(0);
		pix(1) = itsPC->height() * rel(1);

		if ((     isNaN(pix(0)) || isInf(pix(0))
		          || isNaN(pix(1)) || isInf(pix(1)) )) {

			pix(0) = 0;
			pix(1) = 0;
		}


		return pix;
	}


	void DSScreenText::recalculateScreenPosition() {
		if (!itsPC || itsRelativeCenter.nelements()==0) {
			throw(AipsError("Can't do recalculateScreenPosition() since I don't have"
			                " a valid pixelCanvas, or a relative center saved!"));
		}

		Vector<Float> screen;

		screen = relToScreen(itsRelativeCenter);

		DSText::setCenter(screen(0), screen(1));
	}

	void DSScreenText::move(const Float& dX, const Float& dY) {
		DSText::move(dX, dY);
		updateRC();
	}

	void DSScreenText::setCenter(const Float& xPos, const Float& yPos) {
		DSText::setCenter(xPos, yPos);
		updateRC();
	}


	void DSScreenText::updateRC() {

		itsRelativeCenter = screenToRel(DSText::getCenter());

	}


	Vector<Float> DSScreenText::screenToRel(const Vector<Float>& screen) {

		if (!itsPC) {
			throw(AipsError("Can't do screen to rel if I don't have a valid"
			                " pixelCanvas!"));
		}

		Vector<Float> rel(2);

		rel(0) = screen(0) / itsPC->width();
		rel(1) = screen(1) / itsPC->height();

		if ((     isNaN(screen(0)) || isInf(screen(0))
		          || isNaN(screen(1)) || isInf(screen(1)) )) {

			rel(0) = 0;
			rel(1) = 0;
		}


		return rel;

	}

	Record DSScreenText::getOptions() {

		Record toReturn;

		toReturn = DSText::getOptions();

		if (toReturn.isDefined("center")) {
			if (toReturn.dataType("center") != TpArrayFloat) {
				throw (AipsError("I (DSScreenText) received a bad option record from "
				                 "DSText: the field \'center\' was in an unexpected"
				                 " format"));
			}

			Vector<Float> center = toReturn.asArrayFloat("center");

			if (center.nelements() !=2) {
				throw (AipsError("I (DSScreenText) received a bad option record from "
				                 "DSText: the field \'center\' had an unexpected"
				                 " number of elements"));
			}

			itsRelativeCenter = screenToRel(center);


			Quantity x(itsRelativeCenter(0), "frac");
			Quantity y(itsRelativeCenter(1), "frac");

			QuantumHolder xh(x);
			QuantumHolder yh(y);

			Record subx, suby, sub;
			String error;

			xh.toRecord(error, subx);
			if (error.length() != 0) {
				throw(AipsError("Couldn\'t create field \'center\' (1st element) as a "
				                "quantity in "
				                "DSScreenText, error occured: " + error));
			}

			yh.toRecord(error, suby);
			if (error.length() != 0) {
				throw(AipsError("Couldn\'t create field \'center\' (2nd element) as a "
				                "quantity in "
				                "DSScreenText, error occured: " + error));
			}

			sub.defineRecord("x", subx);
			sub.defineRecord("y", suby);

			toReturn.removeField("center");
			toReturn.defineRecord("center", sub);

		}

		// Shouldn't happen (should never be defined) .. but why not
		if (toReturn.isDefined("coords")) {
			toReturn.removeField("coords");
		}

		toReturn.define("coords", "frac");

		return toReturn;

	}



} //# NAMESPACE CASA - END

