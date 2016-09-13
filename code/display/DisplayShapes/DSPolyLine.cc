//# DSPolyLine.cc: PolyLine implementation for "DisplayShapes"
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

#include <display/DisplayShapes/DSPolyLine.h>
#include <display/Display/PixelCanvas.h>
#include <scimath/Mathematics.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSPolyLine::DSPolyLine() :
		DSBasic() {
		setDefaultOptions();
	}

	DSPolyLine::DSPolyLine(const DSPolyLine& other) :
		DSBasic(other),
		itsPoints(other.itsPoints),
		itsCenter(other.itsCenter),
		itsValidCenter(other.itsValidCenter) {

	}


	DSPolyLine::DSPolyLine(const Matrix<Float>& points, const Bool& handles,
	                       const Bool& drawHandles) :

		DSBasic() {
		itsPoints = points;
		itsValidCenter = False;

		setHasHandles(handles);
		setDrawHandles(drawHandles);

		if(handles) {
			buildHandles(points);
		}

	}

	DSPolyLine::~DSPolyLine() {

	}

	uInt DSPolyLine::nPoints() {
		return itsPoints.nelements();
	}

	Bool DSPolyLine::setOptions(const Record& newSettings) {
		Bool localChange = False;
		if (newSettings.isDefined("polylinepoints")) {
			itsPoints = newSettings.asArrayFloat("polylinepoints");
			setHandlePositions(itsPoints);
		}

		if (DSBasic::setOptions(newSettings)) localChange = True;
		return localChange;
	}

	Record DSPolyLine::getOptions() {
		Record rec = DSBasic::getOptions();

		rec.define("type", "polyline");
		if (itsPoints.nrow()) rec.define("polylinepoints" , itsPoints);

		return rec;
	}

	void DSPolyLine::draw(PixelCanvas* pc) {
		pc->setLineWidth(getLineWidth());
		pc->setColor(getColor());

		if (itsPoints.nelements()) {
			pc->drawPolyline(itsPoints);
		}

		DSBasic::draw(pc);

	}

	void DSPolyLine::setPoints(const Matrix<Float>& points) {
		itsPoints.resize(points.nrow(), points.ncolumn());
		itsPoints = points;
		setHandlePositions(itsPoints);
	}

	void DSPolyLine::changePoint(const Vector<Float>& pos, const Int n) {
		if (itsPoints.nelements()) {
			itsPoints(n, 0) = pos(0);
			itsPoints(n,1) = pos(1);
			setHandlePositions(itsPoints);
			itsValidCenter = False;
		} else {
			/*
			cerr << "DSPolyLine.cc - Debug - Can't change points without first setting them." << endl;
			*/
		}
	}

	Matrix<Float> DSPolyLine::getPoints() {
		return itsPoints;
	}

//
	void DSPolyLine::changePoint(const Vector<Float>& pos) {
		Int whichPoint(-1);
		if (closestPoint(itsPoints, pos(0), pos(1), whichPoint)) {
			itsPoints(whichPoint, 0) = pos(0);
			itsPoints(whichPoint, 1) = pos(1);
			setHandlePositions(itsPoints);
			itsValidCenter = False;
		}
	}


	void DSPolyLine::addPoint(const Vector<Float>& newPos) {

		if (itsPoints.nelements()) {
			itsValidCenter = False;
			Matrix<Float> newPoints(itsPoints.nrow() + 1, itsPoints.ncolumn());

			for (uInt i=0; i<itsPoints.nrow(); i++) {
				newPoints(i, 0) = itsPoints(i,0);
				newPoints(i,1) = itsPoints(i,1);
			}
			newPoints(itsPoints.nrow(), 0) = newPos[0];
			newPoints(itsPoints.nrow(), 1) = newPos[1];

			itsPoints.resize(newPoints.nrow(), newPoints.ncolumn());
			itsPoints = newPoints;
			addHandle(newPos);

		} else {

			// This is our first point
			itsPoints.resize(1,2);
			itsPoints(0,0) = newPos(0);
			itsPoints(0,1) = newPos(1);
			buildHandles(itsPoints);

		}
	}

	void DSPolyLine::scale(const Float& /*scaleFactor*/) {

	}


	Bool DSPolyLine::inObject(const Float& xPos, const Float& yPos) {
		Bool onLine = False;

		// Calculate distance from point - line
		// This might be too slow?

		for(uInt i=0; i< (itsPoints.nrow() - 1); i++) {
			Float u, x, y;

			// Calculate where a tangent to the line meets with the point
			u = (((xPos-itsPoints(i,0))*(itsPoints(i+1,0)-itsPoints(i,0))) + ((yPos-itsPoints(i,1))*(itsPoints(i+1,1)-itsPoints(i,1)))) / (( square( itsPoints(i+1,0) - itsPoints(i,0) ) ) + ( square(itsPoints(i+1,1)-itsPoints(i,1)) ));
			x = itsPoints(i,0) + u * ( itsPoints(i+1,0) - itsPoints(i,0) );
			y = itsPoints(i,1) + u * ( itsPoints(i+1,1) - itsPoints(i,1) );

			// Now check that the tangent hits the line in between the two points
			// This might not be needed for this purpose - although might be good when we get thick lines
			Float P2P(hypot(itsPoints(i,0)-itsPoints(i+1,0) , itsPoints(i,1)-itsPoints(i+1,1)));
			if ((P2P > hypot(itsPoints(i,0)-x, itsPoints(i,1)-y))
			        && (P2P > hypot(itsPoints(i+1,0)-x, itsPoints(i+1,1)-y))) {

				Float distance(hypot(xPos - x, yPos - y));
				// x,y is the point on the line where the tanget from the point meets it

				if (distance <= getLineWidth() + 1) {
					// This should be itsThreshold or something I guess
					onLine = True;
					break;
				}
			}
		}

		return onLine;
	}

	void DSPolyLine::move(const Float& dX, const Float& dY) {
		itsPoints = translateMatrix(itsPoints, dX, dY);
		DSBasic::move(dX, dY);

		if (itsValidCenter) {
			itsCenter(0) += dX;
			itsCenter(1) += dY;
		}


	}

	Vector<Float> DSPolyLine::getCenter() {
		if(itsValidCenter) {
			return itsCenter;
		} else {
			if (itsPoints.nrow() % 2 ==1) {
				Int nCenter((itsPoints.nrow() / 2));
				Vector<Float> center(2);
				center = itsPoints.row(nCenter);
				itsCenter = center;
			} else {
				Int nCenter((itsPoints.nrow() / 2));
				Vector<Float> center(2);
				center(0) = itsPoints(nCenter-1,0) +
				            (0.5 * (itsPoints(nCenter , 0) - itsPoints(nCenter-1, 0)));
				center(1) = itsPoints(nCenter-1,1) +
				            (0.5 * (itsPoints(nCenter , 1) - itsPoints(nCenter-1, 1)));
				itsCenter = center;
			}
			itsValidCenter = True;
		}
		return itsCenter;

	}

	void DSPolyLine::setCenter(const Float& xPos, const Float& yPos) {
		Vector<Float> currentCenter(getCenter());
		itsPoints = translateMatrix(itsPoints, (xPos - currentCenter[0]),
		                            (yPos - currentCenter[1]));

		// New center.
		itsValidCenter = True;
		itsCenter[0] = xPos;
		itsCenter[1] = yPos;

		setHandlePositions(itsPoints);

	}

	void DSPolyLine::rotateAbout(const Float& angle, const Float& aboutX,
	                             const Float& aboutY) {
		DSBasic::rotateAbout(angle, aboutX, aboutY);
	}

	void DSPolyLine::rotate(const Float& angle) {
		//Rotate the polyline about its center
		Vector<Float> temp(getCenter());
		itsPoints = DisplayShape::rotatePolygon(itsPoints,
		                                        toRadians(angle),
		                                        temp[0], temp[1]);
		setHandlePositions(itsPoints);

		DSBasic::rotateAbout(angle, temp[0], temp[1]);


	}

	void DSPolyLine::setDefaultOptions() {
		itsValidCenter = False;
		setDrawHandles(True);
	}









} //# NAMESPACE CASA - END

