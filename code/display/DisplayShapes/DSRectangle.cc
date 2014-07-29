//# DSRectangle: Rectangle implementation for "DisplayShapes"
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
#include <display/DisplayShapes/DSRectangle.h>
#include <casa/iostream.h>
#include <casa/Exceptions.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSRectangle::DSRectangle() :
		DSPoly() {
		setDefaultOptions();
	}

	DSRectangle::DSRectangle(const Float& xPos, const Float& yPos,
	                         const Float& width, const Float& height,
	                         const Bool& handles, const Bool& drawHandles) :
		DSPoly() {
		setHasHandles(handles);
		setDrawHandles(drawHandles);
		buildHandles(makeAsPoly(xPos, yPos, width, height));
		setPoints(makeAsPoly(xPos, yPos, width, height));
		itsAngle = 0;
		itsValid = True;
	}

	DSRectangle::DSRectangle(const DSRectangle& other) :
		DSPoly(other),
		itsAngle(other.itsAngle),
		itsValid(other.itsValid),
		itsCenter(other.itsCenter),
		itsWidth(other.itsWidth),
		itsHeight(other.itsHeight) {

	}

	DSRectangle::~DSRectangle() {

	}

	void DSRectangle::setCenter(const Float& xPos, const Float& yPos) {
		if (itsValid) DSPoly::setCenter(xPos, yPos);
		else {
			// The center should be all we need to make a rectangle, so:
			buildHandles(makeAsPoly(xPos, yPos, itsWidth, itsHeight));
			setPoints(makeAsPoly(xPos, yPos, itsWidth, itsHeight));
			itsValid = True;
		}
	}

	void DSRectangle::move(const Float& dX, const Float& dY) {
		if (itsValid) DSPoly::move(dX, dY);
	}

	Float DSRectangle::getHeight() {
		if (itsValid) {
			Matrix<Float> currentPoints(getPoints());
			return hypot(currentPoints(1,0) - currentPoints(0,0),
			             currentPoints(1,1) - currentPoints(0,1));
		} else {
			return itsHeight;
		}

		return 0;
	}

	Float DSRectangle::getWidth() {
		if (itsValid) {
			Matrix<Float> currentPoints(getPoints());
			return hypot(currentPoints(3,0) - currentPoints(0,0),
			             currentPoints(3,1) - currentPoints(0,1));
		} else {
			return itsWidth;
		}

		return 0;
	}

	void DSRectangle::changePoint(const Vector<Float>& point) {

		if(itsValid) {
			Matrix<Float> currentPoints(getPoints());
			Vector<Float> currentCenter(getCenter());
			Matrix<Float> unRotated(rotatePolygon(currentPoints,
			                                      -(toRadians(itsAngle)),
			                                      currentCenter[0], currentCenter[1]));
			Vector<Float> clickPoint(rotatePoint(point, -(toRadians(itsAngle)),
			                                     currentCenter[0], currentCenter[1]));
			Int movingPoint(0);
			closestPoint(unRotated, clickPoint[0], clickPoint[1], movingPoint);
			changePoint(point, movingPoint);
		}
	}

	void DSRectangle::changePoint(const Vector<Float>& point,
	                              const Int whichPoint) {

		// Should put this in for speed reasons
		//if !(itsAngle == 0 || itsAngle == C::pi)
		// {
		// else
		// Do for unrotated.

		if(itsValid) {
			Matrix<Float> currentPoints(getPoints());
			Vector<Float> currentCenter(getCenter());
			Matrix<Float> unRotated(rotatePolygon(currentPoints
			                                      , -(toRadians(itsAngle)),
			                                      currentCenter[0], currentCenter[1]));
			Vector<Float> clickPoint(rotatePoint(point,
			                                     -(toRadians(itsAngle)),
			                                     currentCenter[0], currentCenter[1]));

			if (whichPoint == 0) {

				unRotated(0,0) = clickPoint[0];
				unRotated(0,1) = clickPoint[1];

				unRotated(1,0) = clickPoint[0];
				unRotated(3,1) = clickPoint[1];

			} else if (whichPoint == 1) {

				unRotated(1,0) = clickPoint[0];
				unRotated(1,1) = clickPoint[1];

				unRotated(0,0) = clickPoint[0];
				unRotated(2,1) = clickPoint[1];

			} else if (whichPoint == 2) {

				unRotated(2,0) = clickPoint[0];
				unRotated(2,1) = clickPoint[1];

				unRotated(1,1) = clickPoint[1];
				unRotated(3,0) = clickPoint[0];

			} else if (whichPoint == 3) {

				unRotated(3,0) = clickPoint[0];
				unRotated(3,1) = clickPoint[1];

				unRotated(0,1) = clickPoint[1];
				unRotated(2,0) = clickPoint[0];

			} else {
				throw(AipsError("DSRectangle.cc - Can't change point > 3"));
			}

			currentPoints = rotatePolygon(unRotated, toRadians(itsAngle),
			                              currentCenter[0],
			                              currentCenter[1]);
			setPoints(currentPoints);

		}
	}

	Float DSRectangle::getAngle() {
		return itsAngle;
	}

	void DSRectangle::addPoint(const Vector<Float>& /*toAdd*/) {
		throw(AipsError("DSRectangle.cc - Can't add points to a rectangle."));
	}

	void DSRectangle::setWidth(const Float& width) {
		if (itsValid) {

			Matrix<Float> currentPoints(getPoints());
			Vector<Float> currentCenter(getCenter());
			Matrix<Float> unRotated(rotatePolygon(currentPoints, -toRadians(itsAngle),
			                                      currentCenter[0], currentCenter[1]));

			unRotated(0,0) = currentCenter[0] - (0.5 * width);
			unRotated(1,0) = currentCenter[0] - (0.5 * width);
			unRotated(2,0) = currentCenter[0] + (0.5 * width);
			unRotated(3,0) = currentCenter[0] + (0.5 * width);

			currentPoints = rotatePolygon(unRotated, toRadians(itsAngle),
			                              currentCenter[0],
			                              currentCenter[1]);
			setPoints(currentPoints);

		} else {
			itsWidth = width;
		}
	}

	void DSRectangle::setHeight(const Float& height) {
		if (itsValid) {
			Matrix<Float> currentPoints(getPoints());
			Vector<Float> currentCenter(getCenter());
			Matrix<Float> unRotated(rotatePolygon(currentPoints,
			                                      -toRadians(itsAngle),
			                                      currentCenter[0], currentCenter[1]));

			unRotated(0,1) = currentCenter[1] - (0.5 * height);
			unRotated(1,1) = currentCenter[1] + (0.5 * height);
			unRotated(2,1) = currentCenter[1] + (0.5 * height);
			unRotated(3,1) = currentCenter[1] - (0.5 * height);

			currentPoints = rotatePolygon(unRotated, toRadians(itsAngle),
			                              currentCenter[0], currentCenter[1]);
			setPoints(currentPoints);

		} else itsHeight = height;

	}

// Absolute angle
	void DSRectangle::setAngle(const Float& angle) {
		itsAngle = angle;
		if (itsValid) {
			Vector<Float> center(getCenter());
			setPoints(makeAsPoly(center[0], center[1], getWidth(), getHeight()));
			DSPoly::rotate(itsAngle);
		}
	}

// This function rotates relatively
	void DSRectangle::rotate(const Float& angle) {
		itsAngle += angle;
		if (itsValid) DSPoly::rotate(angle);
	}

	void DSRectangle::rotateAbout(const Float& angle, const Float& aboutX,
	                              const Float& aboutY) {
		if (itsValid) DSPoly::rotateAbout(angle, aboutX, aboutY);
	}


	void DSRectangle::setDefaultOptions() {
		itsValid = False;
		itsHeight = 0;
		itsWidth = 0;
		itsAngle = 0;
	}

	Bool DSRectangle::setOptions(const Record& settings) {
		Bool localChange = False;

		if (settings.isDefined("height")) setHeight(settings.asFloat("height"));
		if (settings.isDefined("width")) setWidth(settings.asFloat("width"));
		if (settings.isDefined("center")) {
			Vector<Float> newCent(settings.asArrayFloat("center"));
			setCenter(newCent[0], newCent[1]);
		}

		if (settings.isDefined("angle")) {
			itsAngle = settings.asFloat("angle");
			setAngle(itsAngle);
		}

		if (DSPoly::setOptions(settings)) localChange = True;
		return localChange;
	}

	Record DSRectangle::getOptions() {
		Record rec(DSPoly::getOptions());

		rec.define("type", "rectangle");
		rec.define("angle", itsAngle);
		rec.define("width", getWidth());
		rec.define("height", getHeight());

		if (rec.isDefined("polygonpoints")) rec.removeField("polygonpoints");

		return rec;
	}

	Matrix<Float> DSRectangle::makeAsPoly(const Float& xPos, const Float& yPos,
	                                      const Float& width, const Float& height) {
		Matrix<Float> toReturn(4,2);

		toReturn(0,0) = xPos + (0.5 * width);
		toReturn(1,0) = xPos + (0.5 * width);
		toReturn(2,0) = xPos - (0.5 * width);
		toReturn(3,0) = xPos - (0.5 * width);

		toReturn(0,1) = yPos + (0.5 * height);
		toReturn(1,1) = yPos - (0.5 * height);
		toReturn(2,1) = yPos - (0.5 * height);
		toReturn(3,1) = yPos + (0.5 * height);

		return toReturn;
	}








} //# NAMESPACE CASA - END

