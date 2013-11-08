//# DSArrow.cc: Arrow implementation for "DisplayShapes"
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
#include <casa/Exceptions.h>

#include <display/DisplayShapes/DSArrow.h>
#include <display/DisplayShapes/DSPoly.h>

#include <display/Display/DParameterRange.h>

#include <display/Display/PixelCanvas.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSArrow::DSArrow() :
		DSLine(),
		itsArrowHead(0),
		itsOffset(0) {

		itsHeadSize = new DParameterRange<Int>("arrowheadsize", "Arrow Head Size",
		                                       "Select the desired arrow head size",
		                                       3,36, 1, 8,8);

		setDefaultOptions();
	}

	DSArrow::~DSArrow() {
		if (itsArrowHead) delete itsArrowHead;
		itsArrowHead = 0;
		delete itsHeadSize;
		itsHeadSize = 0;
	}

	DSArrow::DSArrow(const Vector<Float> &startPoint,
	                 const Vector<Float> &endPoint,
	                 DSArrow::ArrowHead style,
	                 const Int arrowHeadSize) :

		DSLine(startPoint, endPoint),
		itsArrowHead(0),
		itsOffset(0) {

		itsHeadSize = new DParameterRange<Int>("arrowheadsize",
		                                       "Arrow Head Size",
		                                       "Select the desired arrow head size",
		                                       3,36,1,arrowHeadSize,
		                                       8);
		itsArrowHeadStyle = style;
		buildArrowHead();

	}

	DSArrow::DSArrow(const DSArrow& other) :
		DSLine(other),
		itsHeadSize(other.itsHeadSize),
		itsArrowHead(other.itsArrowHead),
		itsStartPoint(other.itsStartPoint),
		itsEndPoint(other.itsEndPoint),
		itsUnrotatedHead(other.itsUnrotatedHead),
		itsOffset(0),
		itsArrowHeadStyle(other.itsArrowHeadStyle) {

	}

	void DSArrow::setDefaultOptions() {
		itsArrowHeadStyle = DSArrow::Cool_Triangle;
		itsStartPoint.resize(2);
		itsEndPoint.resize(2);
		itsLength = -1;
	}

	void DSArrow::buildArrowHead() {
		if (isValid()) {
			if (itsArrowHead) delete itsArrowHead;
			itsArrowHead = 0;
			Float maxX = 0;
			Vector<Float> cent(DSPolyLine::getCenter());
			Matrix<Float> ends(DSLine::getEnds());
			Float angle(std::atan2(ends(0,1) - ends(1,1) , ends(0,0) - ends(1,0)));

			if (itsArrowHeadStyle == DSArrow::Filled_Triangle ||
			        itsArrowHeadStyle == DSArrow::Open_Triangle) {

				Matrix<Float> newHead(3,2);
				newHead(0,0) = 1;
				newHead(0,1) = 1 + Float(itsHeadSize->value());
				newHead(1,0) = 1 + Float(3 * itsHeadSize->value());
				newHead(1,1) = 1;
				newHead(2,0) = 1;
				newHead(2,1) = 1 - Float(itsHeadSize->value());
				maxX = 1 + Float(3 * itsHeadSize->value());
				itsUnrotatedHead = newHead;

				itsArrowHead = new DSPoly(newHead, False, False);
				if (itsArrowHeadStyle == DSArrow::Filled_Triangle)
					itsArrowHead->setFillStyle(DSClosed::Full_Fill);
				else itsArrowHead->setFillStyle(DSClosed::No_Fill);

			} else if (itsArrowHeadStyle == DSArrow::Cool_Triangle) {

				Matrix<Float> newHead(4,2);
				newHead(0,0) = 1;
				newHead(0,1) = 1;
				newHead(1,0) = 1 + Float(itsHeadSize->value());
				newHead(1,1) = 1 + Float(itsHeadSize->value());
				newHead(2,0) = 1;
				newHead(2,1) = 1 + Float(2 * itsHeadSize->value());
				newHead(3,0) = 1 + Float(3 * itsHeadSize->value());
				newHead(3,1) = 1 + Float(itsHeadSize->value());
				maxX = 1 + Float(3 * itsHeadSize->value());
				itsUnrotatedHead.resize(newHead.nrow(), newHead.ncolumn());
				itsUnrotatedHead = newHead;

				itsArrowHead = new DSPoly(newHead, False, False);
				itsArrowHead->setFillStyle(DSClosed::Full_Fill);

			} else {
				throw(AipsError("That Arrow head style is NYI sorry"));
				return;
			}



			Vector<Float> centerNow = itsArrowHead->getCenter();
			itsOffset = maxX - centerNow(0);

			itsArrowHead->rotate(toDegrees(angle));
			itsArrowHead->setCenter(ends(0,0), ends(0,1));
			itsArrowHead->move(itsOffset * - cos(angle), itsOffset * - sin(angle));

		} else throw(AipsError("DSArrow.cc - Can't build arrow head without valid end"));

	}

	void DSArrow::setCenter(const Float& xPos, const Float& yPos) {
		if (!isValid() && itsLength != -1) {
			DSLine::setCenter(xPos, yPos);
			setLength(itsLength);
			updateArrowHead();
		} else {
			DSLine::setCenter(xPos, yPos);
			updateArrowHead();
		}
	}

	Bool DSArrow::setOptions(const Record& newSettings) {
		// Update required?
		Bool localChange = False;

		// Do we need a new arrow head?
		Bool arrowHead = False;

		if (itsHeadSize->fromRecord(newSettings)) {
			localChange = True;
			arrowHead = True;
		}

		if (newSettings.isDefined("arrowheadstyle")) {
			DSArrow::ArrowHead cast;

			Int temp = newSettings.asInt("arrowheadstyle");

			if (temp < 0 || temp >= numArrowHeads) {
				throw (AipsError ("Bad setOptions - Couldn't cast Arrow Head Style"));
			}

			try {
				cast = static_cast<DSArrow::ArrowHead>(temp);
			} catch (...) {
				throw(AipsError ("Bad setOptions - Couldn't cast Arrow Head Style"));
			}

			if (cast !=itsArrowHeadStyle) {
				arrowHead = True;
				localChange = True;
				itsArrowHeadStyle = cast;
			}

		}

		if (DSLine::setOptions(newSettings)) {
			localChange = True;
		}

		if (arrowHead) buildArrowHead();

		return localChange;
	}

	void DSArrow::make() {

		DSLine::make();
		buildArrowHead();
	}

	Record DSArrow::getOptions() {
		Record rec = DSLine::getOptions();

		if (rec.isDefined("type")) rec.removeField("type");
		rec.define("type", "arrow");
		rec.define("arrowheadstyle", itsArrowHeadStyle);

		itsHeadSize->toRecord(rec);

		return rec;
	}

	void DSArrow::draw(PixelCanvas* pc) {
		// Redraw arrow head
		if (isValid()) {

			// This is here so that if we are not drawing handles (e.g.
			// postscript), then the arrow head is drawn last
			// If we are drawing handles, we want the line last so that the arrow
			// head doesn't cover any of the handle.
			if (drawingHandles()) {
				itsArrowHead->setColor(DisplayShape::getColor());
				itsArrowHead->draw(pc);
				DSLine::draw(pc);
			} else {
				DSLine::draw(pc);
				itsArrowHead->setColor(DisplayShape::getColor());
				itsArrowHead->draw(pc);
			}

		}
	}

	void DSArrow::move(const Float& dX, const Float& dY) {
		if (isValid()) {
			itsArrowHead->move(dX,dY);
			DSLine::move(dX,dY);
		}
	}

	void DSArrow::rotate(const Float& angle) {
		if (isValid()) {
			DSLine::rotate(angle);
			Vector<Float> lineCent(getCenter());
			itsArrowHead->rotateAbout(angle, lineCent[0], lineCent[1]);
		}
	}

	void DSArrow::updateArrowHead() {
		if (isValid()) {

			if (itsArrowHead) {
				Matrix<Float> ends(DSLine::getEnds());
				Float angle(std::atan2(ends(0,1) - ends(1,1) , ends(0,0) - ends(1,0)));

				itsArrowHead->setPoints(itsUnrotatedHead);
				itsArrowHead->rotate(toDegrees(angle));
				itsArrowHead->setCenter(ends(0,0), ends(0,1));
				itsArrowHead->move(itsOffset * - cos(angle), itsOffset * - sin(angle));

			} else {
				buildArrowHead();
			}

		}
	}

	void DSArrow::rotateAbout(const Float& /*angle*/, const Float& /*aboutX*/,
	                          const Float& /*aboutY*/) {

	}


	void DSArrow::setStartPoint(const Vector<Float> &startPoint) {
		DSLine::setStartPoint(startPoint);
		updateArrowHead();
	}

	void DSArrow::changePoint(const Vector<Float> &pos, const Int n) {
		if (n == 0) {
			setStartPoint(pos);
		} else if (n==1) {
			setEndPoint(pos);
		} else throw (AipsError("DSArrow.cc :: changePoint - Error - Bad point"));
	}

	void DSArrow::changePoint(const Vector<Float> &pos) {
		if (isValid()) {
			DSPolyLine::changePoint(pos);
			updateArrowHead();
		} else throw(AipsError("I can't move the nearest point since my points aren't valid yet"));
	}

	void DSArrow::setEndPoint(const Vector<Float> &endPoint) {
		DSLine::setEndPoint(endPoint);
		updateArrowHead();
	}

	void DSArrow::scale(const Float& /*scaleFactor*/) {
		throw(AipsError("DSArrow.cc - Scale NYI so far"));
	}

	void DSArrow::setLength(const Float& length) {
		if (isValid()) {

			Vector<Float> center = getCenter();
			Matrix<Float> ends(DSLine::getEnds());

			Float angle(std::atan2(ends(0,1) - ends(1,1) , ends(0,0) - ends(1,0)));

			Vector<Float> sp(2);
			Vector<Float> ep(2);

			sp(0) = center(0) + (length * cos(angle));
			sp(1) = center(1) + (length * sin(angle));

			setStartPoint(sp);

			ep(0) = center(0) - (length * cos(angle));
			ep(1) = center(1) - (length * sin(angle));

			setEndPoint(ep);

			updateArrowHead();
		} else {
			itsLength = length;
		}
	}

	void DSArrow::setAngle(const Float& angle) {
		if (isValid()) {
			Matrix<Float> ends(DSLine::getEnds());
			Float currentAngle(std::atan2(ends(0,1) - ends(1,1) , ends(0,0) - ends(1,0)));
			rotate(angle-currentAngle);
		}
	}






} //# NAMESPACE CASA - END

