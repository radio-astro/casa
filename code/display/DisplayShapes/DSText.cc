//# Text.cc: Text implementation for "DisplayShapes"
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
#include <scimath/Mathematics.h>

#include <display/Display/PixelCanvas.h>
#include <display/DisplayShapes/DSText.h>
#include <casa/Exceptions.h>

#include <display/Display/DParameterString.h>
#include <display/Display/DParameterRange.h>
#include <display/Display/DParameterFontChoice.h>
#include <display/Display/DParameterMapKeyChoice.h>
#include <display/Display/DLFont.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSText::DSText() :
		DisplayShape()

	{
		// Set up user options
		itsString = new DParameterString("text", "Text Label", "Enter text here",
		                                 "Text String", "Text String") ;
		itsAngle = new DParameterRange<Float>("angle", "Text Angle",
		                                      "Select angle for drawing of text",
		                                      -180,180, 1, 0,0);
		itsFontSize = new DParameterRange<Int>("fontsize", "Font Size",
		                                       "Select the font size for text",
		                                       6, 64,1,12,12);
		itsFont = new DParameterFontChoice("font", "Font",
		                                   "Select desired text font");

		makeAlignmentChoice();

		setDefaultOptions();
	}

	Bool DSText::inObject(const Float& xPos, const Float& yPos) {
		if (itsValid && !itsHandlesMade) {
			return (hypot(xPos - itsCenter[0], yPos - itsCenter[1]) < 5);
		} else if (itsValid) {
			return (inPolygon(itsHandleLocation, xPos, yPos));
		}
		return False;
	}

	void DSText::setCenter(const Float& xPos, const Float& yPos) {
		if (itsValid) {
			itsCenter[0] = xPos;
			itsCenter[1] = yPos;
		} else {
			itsCenter[0] = xPos;
			itsCenter[1] = yPos;
			itsValid = True;
		}
		itsValidPositions = False;
	}

	DSText::DSText(const Float& xPos, const Float& yPos, const String& text,
	               const Bool& hasHandles, const Bool& drawHandles) :
		DisplayShape() {

		// Set up user options
		itsString = new DParameterString("text", "Text Label", "Enter text here",
		                                 text, "Text String") ;

		itsAngle = new DParameterRange<Float>("angle", "Text Angle",
		                                      "Select angle for drawing of text",
		                                      -180,180, 1, 0,0);

		itsFontSize = new DParameterRange<Int>("fontsize", "Font Size",
		                                       "Select the font size for text",
		                                       6, 64,1,12,12);

		itsFont = new DParameterFontChoice("font", "Font",
		                                   "Select desired text font");

		makeAlignmentChoice();
		//

		itsCenter.resize(2);
		itsCenter[0] = xPos;
		itsCenter[1] = yPos;

		itsAlign = Display::AlignBottomLeft;
		itsValid = True;
		setHasHandles(hasHandles);
		setDrawHandles(drawHandles);

		itsHandlesMade = False;
		itsValidPositions = False;
	}


	DSText::~DSText() {
		delete itsString;
		itsString = 0;
		delete itsAngle;
		itsAngle = 0;
		delete itsFontSize;
		itsFontSize = 0;
		delete itsFont;
		itsFont = 0;
		delete itsAlignment;
		itsAlignment = 0;
	}

	DSText::DSText(const DSText& other) :
		DisplayShape(other),
		itsString(other.itsString),
		itsAngle(other.itsAngle),
		itsAlignment(other.itsAlignment),
		itsFont(other.itsFont),
		itsFontSize(other.itsFontSize),
		itsCenter(other.itsCenter),
		itsPixHeight(other.itsPixHeight),
		itsPixWidth(other.itsPixWidth),
		itsValid(other.itsValid),
		itsHandlesMade(other.itsHandlesMade),
		itsValidPositions(other.itsValidPositions),
		itsStringLength(other.itsStringLength),
		itsStringHeight(other.itsStringHeight),
		itsHandleLocation(other.itsHandleLocation),
		itsAlign(other.itsAlign) {

	}

	void DSText::draw(PixelCanvas *pix) {

		if (itsValid) {
			pix->setColor(getColor());


			DLFont* tempFont = new DLFont(itsFont->value(), DLFont::Name,
			                              itsFontSize->value());

			pix->setFont(tempFont);
			delete tempFont;
			tempFont = 0;

			itsPixHeight = pix->textHeight(itsString->value());
			itsPixWidth = pix->textWidth(itsString->value());

			if (itsPixHeight != -1 && itsPixHeight != -1) {
				calculateHandlePositions();
				if (itsHandlesMade) {
					setHandlePositions(itsHandleLocation);
				} else {
					buildHandles(itsHandleLocation);
					itsHandlesMade = True;
				}
			}
			pix->drawText(itsCenter[0], itsCenter[1], itsString->value(),
			              itsAngle->value(), toEnum(itsAlignment->keyValue()));

			DisplayShape::draw(pix);
		}
	}

	void DSText::move(const Float& dX, const Float& dY) {
		itsCenter[0] += dX;
		itsCenter[1] += dY;
		DisplayShape::move(dX, dY);
		itsValidPositions = False;
	}

	void DSText::rotate(const Float& angle) {

		itsAngle->setValue(itsAngle->value() + angle);

		Vector<Float> cent(2);
		cent=getCenter();
		DisplayShape::rotateAbout(angle, cent[0], cent[1]);
		itsValidPositions = False;
	}

	Bool DSText::setOptions(const Record& settings) {
		Bool localChange = False;

		if (settings.isDefined("center")) {
			Vector<Float> cent(settings.asArrayFloat("center"));
			setCenter(cent[0], cent[1]);
		}

		if (itsFontSize->fromRecord(settings)) localChange = True;
		if (itsFont->fromRecord(settings)) localChange = True;
		if (itsString->fromRecord(settings)) localChange = True;
		if (itsAngle->fromRecord(settings)) localChange = True;
		if (itsAlignment->fromRecord(settings)) localChange = True;

		if (DisplayShape::setOptions(settings)) localChange = True;
		if (localChange) itsValidPositions = False;

		return localChange;
	}

	Vector<Float> DSText::getCenter() {
		if (itsValid) return itsCenter;

		Vector<Float> a(2);
		a[0] = 0;
		a[1] = 0;
		return a;
	}

	Record DSText::getOptions() {
		Record rec = DisplayShape::getOptions();

		itsString->toRecord(rec, True, True);
		itsAngle->toRecord(rec, True, True);
		itsFontSize->toRecord(rec, True, True);
		itsFont->toRecord(rec, True, True);
		itsAlignment->toRecord(rec, True, True);

		if (rec.isDefined("type")) rec.removeField("type");
		rec.define("type", "text");

		if (itsValid) rec.define("center", itsCenter);

		return rec;
	}

	void DSText::makeAlignmentChoice() {

		Vector<String> aligns(9);
		aligns(0) = "AlignCenter";
		aligns(1) = "AlignLeft";
		aligns(2) = "AlignTop";
		aligns(3) = "AlignRight";
		aligns(4) = "AlignBottom";
		aligns(5) = "AlignTopLeft";
		aligns(6) = "AlignTopRight";
		aligns(7) = "AlignBottomLeft";
		aligns(8) = "AlignBottomRight";

		Vector<Int> enums(9);
		for (Int i=0; i<9; i++) enums(i) = i;

		itsAlignment = new DParameterMapKeyChoice("align", "Text Alignment",
		        "Select how text is positioned on screen",
		        aligns, enums, aligns(7),
		        aligns(7));

	}

	void DSText::changePoint(const Vector<Float>& newPos) {
	}

	void DSText::changePoint(const Vector<Float>& newPos, const Int nPoint) {
	}

	void DSText::scale(const Float& scaleFactor) {
		if ((itsFontSize->value() * scaleFactor) > 4)
			itsFontSize->setValue(itsFontSize->value() * scaleFactor);

		itsValidPositions = False;
	}

	Display::TextAlign DSText::toEnum(const Int fromInt) {
		Display::TextAlign caster;

		if (fromInt >= 0)
			caster = static_cast<Display::TextAlign>(fromInt);
		else caster = Display::AlignBottomLeft;

		return caster;
	}

	void DSText::calculateHandlePositions() {
		if (itsValidPositions) return;

		itsHandleLocation.resize(4,2);

		Float radAngle(itsAngle->value() * (C::pi/180));
		Display::TextAlign align = toEnum(itsAlignment->keyValue());

		Float xOff = 0, yOff = 0;
		const static Int buffer = 5;

		switch(align) {
		case Display::AlignLeft :
			xOff = - buffer;
			yOff = - ((0.5 * itsPixHeight) + buffer);
			break;
		case Display::AlignCenter :
			xOff = - ((0.5 * itsPixWidth) + buffer);
			yOff = - ((0.5 * itsPixHeight) + buffer);
			break;
		case Display::AlignTop :
			xOff = - ((0.5 * itsPixWidth) + buffer);
			yOff = - ((itsPixHeight) + buffer);
			break;
		case Display::AlignRight :
			xOff = - (itsPixWidth + buffer);
			yOff = - ((0.5 * itsPixHeight) + buffer);
			break;
		case Display::AlignBottom :
			xOff = -((0.5 * itsPixWidth) + buffer);
			yOff = -buffer;
			break;
		case Display::AlignTopLeft :
			xOff = -buffer;
			yOff = -(itsPixHeight + buffer);
			break;
		case Display::AlignTopRight :
			xOff =  - (itsPixWidth + buffer);
			yOff =  - (itsPixHeight + buffer);
			break;
		case Display::AlignBottomLeft :
			xOff = - buffer;
			yOff = - buffer;
			break;
		case Display::AlignBottomRight :
			xOff = - (itsPixWidth + buffer);
			yOff = - buffer;
			break;
		}

		itsHandleLocation(0,0) = itsCenter[0] + xOff;
		itsHandleLocation(0,1) = itsCenter[1] + yOff;
		itsHandleLocation(1,0) = itsHandleLocation(0,0);
		itsHandleLocation(1,1) = itsHandleLocation(0,1) + itsPixHeight + (2*buffer);
		itsHandleLocation(2,0) = itsHandleLocation(1,0) + itsPixWidth + (2*buffer);
		itsHandleLocation(2,1) = itsHandleLocation(1,1);
		itsHandleLocation(3,0) = itsHandleLocation(2,0);
		itsHandleLocation(3,1) = itsHandleLocation(2,1) - itsPixHeight - (2*buffer);

		itsHandleLocation = rotatePolygon(itsHandleLocation, radAngle,
		                                  itsCenter[0], itsCenter[1]);
		itsValidPositions = True;

	}

	void DSText::setDefaultOptions() {
		setHasHandles(True);
		setDrawHandles(True);

		itsCenter.resize(2);
		itsAlign = Display::AlignBottomLeft;
		itsValid = False;
		itsHandlesMade = False;
		itsValidPositions = False;
	}













} //# NAMESPACE CASA - END

