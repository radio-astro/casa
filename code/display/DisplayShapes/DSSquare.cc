//# DSSquare.cc: Square implementation for "DisplayShapes"
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

#include <display/DisplayShapes/DSSquare.h>
#include <casa/BasicSL/Constants.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSSquare::DSSquare() :
		DSRectangle() {
	}

	DSSquare::DSSquare(const DSSquare& other) :
		DSRectangle(other) {

	}

	DSSquare::~DSSquare() {

	}

	DSSquare::DSSquare(const Float& xPos, const Float& yPos, const Float& size,
	                   const Bool& handles, const Bool& drawHandles) :
		DSRectangle(xPos,yPos,size,size,handles,drawHandles) {
	}

	void DSSquare::setSize(const Float& size) {
		DSRectangle::setWidth(size);
		DSRectangle::setHeight(size);
	}

	Bool DSSquare::setOptions(const Record& settings) {
		Bool localChange = False;
		Record newsettings(settings);

		if (newsettings.isDefined("size")) {

			if (newsettings.isDefined("height")) newsettings.removeField("height");
			if (newsettings.isDefined("width")) newsettings.removeField("width");

			newsettings.define("height", newsettings.asFloat("size"));
			newsettings.define("width", newsettings.asFloat("size"));

		}

		if (DSRectangle::setOptions(newsettings)) localChange = True;
		return localChange;

	}

	Record DSSquare::getOptions() {
		Record rec(DSRectangle::getOptions());
		rec.define("type", "square");
		rec.define("size",getHeight());

		if (rec.isDefined("width")) rec.removeField("width");
		if (rec.isDefined("height")) rec.removeField("height");

		return rec;
	}

	void DSSquare::changePoint(const Vector<Float>& pos, const Int nPoint) {
		Float itsAngle(getAngle());
		Matrix<Float> currentPoints(getPoints());
		Vector<Float> currentCenter(getCenter());

		Matrix<Float> unRotated(rotatePolygon(currentPoints,
		                                      -(toRadians(itsAngle)),
		                                      currentCenter[0], currentCenter[1]));

		Vector<Float> clickPoint(rotatePoint(pos,
		                                     -(toRadians(itsAngle)),
		                                     currentCenter[0], currentCenter[1]));

		Float distance(hypot(clickPoint[0]-currentCenter[0],
		                     clickPoint[1]-currentCenter[1]));

		Float angle(std::atan2(clickPoint[1] -  currentCenter[1],
		                       clickPoint[0] - currentCenter[0]));
		Float compDist(cos(angle) * distance);

		if (nPoint == 0) {

			unRotated(0,0) = currentCenter[0] + compDist;
			unRotated(0,1) = currentCenter[1] + compDist;

			unRotated(1,0) = unRotated(0,0);
			unRotated(3,1) = unRotated(0,1);

		} else if (nPoint == 1) {

			unRotated(1,0) = currentCenter[0] + compDist;
			unRotated(1,1) = currentCenter[1] - compDist;

			unRotated(0,0) = unRotated(1,0);
			unRotated(2,1) = unRotated(1,1);

		} else if (nPoint == 2) {

			unRotated(2,0) = currentCenter[0] + compDist;
			unRotated(2,1) = currentCenter[1] + compDist;

			unRotated(1,1) = unRotated(2,1);
			unRotated(3,0) = unRotated(2,0);

		} else if (nPoint == 3) {

			unRotated(3,0) = currentCenter[0] + compDist;
			unRotated(3,1) = currentCenter[1] - compDist;

			unRotated(0,1) = unRotated(3,1);
			unRotated(2,0) = unRotated(3,0);

		} else {
			//cerr << "Debug - Error" << endl;
		}

		currentPoints = rotatePolygon(unRotated, toRadians(itsAngle),
		                              currentCenter[0],
		                              currentCenter[1]);
		setPoints(currentPoints);
	}

	void DSSquare::changePoint(const Vector<Float>& pos) {
		Matrix<Float> currentPoints(getPoints());

		Int movingPoint(0);
		closestPoint(currentPoints, pos[0], pos[1], movingPoint);

		changePoint(pos, movingPoint);

	}










} //# NAMESPACE CASA - END

