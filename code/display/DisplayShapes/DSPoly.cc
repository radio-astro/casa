//# DSPoly.cc: Polygon implementation for "DisplayShapes"
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
#include <display/DisplayShapes/DSPoly.h>
#include <display/DisplayShapes/DSBasic.h>
#include <display/Display/PixelCanvas.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DSPoly::DSPoly() :
		DSClosed() {
		setDefaultOptions();
	}

	DSPoly::DSPoly(const DSPoly& other) :
		DSClosed(other),
		itsPoints(other.itsPoints),
		itsPolyCenter(other.itsPolyCenter),
		itsValidPoints(other.itsValidPoints),
		itsValidCenter(other.itsValidCenter) {

	}



	DSPoly::DSPoly(const Matrix<Float>& points, const Bool& handles,
	               const Bool& drawHandles) :
		DSClosed() {

		setHasHandles(handles);
		setDrawHandles(drawHandles);
		itsValidCenter = False;
		itsPoints = points;
		itsPolyCenter.resize(2);
		if (handles) {
			buildHandles(itsPoints);
		}
	}

	DSPoly::~DSPoly() {
	}

	void DSPoly::setCenter(const Float& xPos, const Float& yPos) {

		Vector<Float> currentCenter(getCenter());
		itsPoints = translateMatrix(itsPoints, (xPos - currentCenter[0]),
		                            (yPos - currentCenter[1]));

		// New center.
		itsValidCenter = True;
		itsPolyCenter[0] = xPos;
		itsPolyCenter[1] = yPos;

		setHandlePositions(itsPoints);
	}


	void DSPoly::draw(PixelCanvas *pix) {

		pix->setLineWidth(getLineWidth());
		pix->setColor(getColor());
		if (itsPoints.nelements()) {
			if (getFillStyle() == DSClosed::No_Fill) {
				pix->drawPolygon(itsPoints.column(0), itsPoints.column(1));
			} else if (getFillStyle() == DSClosed::Full_Fill) {
				pix->drawFilledPolygon(itsPoints.column(0), itsPoints.column(1));
			} else {
				//cerr << "DSPoly - Debug - Fill style NYI sorry" << endl;
				pix->drawPolygon(itsPoints.column(0), itsPoints.column(1));
			}
		}
		DSClosed::draw(pix);

	}


	Record DSPoly::getOptions() {
		Record rec = DSClosed::getOptions();

		rec.define("type", "polygon");
		if(itsPoints.nrow()) rec.define("polygonpoints", itsPoints);
		if (itsPoints.nrow()) rec.define("center", getCenter());

		return rec;
	}

	Bool DSPoly::setOptions(const Record& settings) {
		Bool localChange = False;
		// Maybe this first one, "validPoints" isnt needed?
		if (settings.isDefined("polygonpoints")) {
			itsPoints = settings.asArrayFloat("polygonpoints");
			setHandlePositions(itsPoints);
		}


		if(DSClosed::setOptions(settings)) localChange = True;
		return localChange;
	}

	void DSPoly::rotate(const Float& angle) {

		//Rotate the polygon about its center
		Vector<Float> temp(getCenter());
		itsPoints = DisplayShape::rotatePolygon(itsPoints, toRadians(angle),
		                                        temp[0], temp[1]);
		setHandlePositions(itsPoints);

		DSClosed::rotateAbout(angle, temp[0], temp[1]);
	}

	void DSPoly::rotateAbout(const Float& angle, const Float& aboutX,
	                         const Float& aboutY) {

		//Rotate the polygon about some point
		itsValidCenter = False;
		itsPoints = DisplayShape::rotatePolygon(itsPoints, toRadians(angle),
		                                        aboutX, aboutY);
		DSClosed::rotateAbout(angle, aboutX, aboutY);
	}

	Bool DSPoly::inObject(const Float& xPos, const Float& yPos) {

		// Use the standard method
		return DisplayShape::inPolygon(itsPoints, xPos, yPos);

	}
	void DSPoly::scale(const Float& scale) {

		Vector<Float> temp(getCenter());

		if (scale < 1 && getArea() < 10) return;

		for (uInt i=0; i<itsPoints.nrow() ; i++) {
			itsPoints(i,0) = temp[0] + (scale * (itsPoints(i,0) - temp[0]));
			itsPoints(i,1) = temp[1] + (scale * (itsPoints(i,1) - temp[1]));
		}

		//Rebuild the thePoints and handles..
		setHandlePositions(itsPoints);
		//
	}

	void DSPoly::changePoint(const Vector<Float> &newLocation) {
		Int whichPoint(-1);
		if (closestPoint(itsPoints, newLocation[0], newLocation[1], whichPoint)) {
			itsPoints(whichPoint, 0) = newLocation[0];
			itsPoints(whichPoint, 1) = newLocation[1];
			setHandlePositions(itsPoints);
			itsValidCenter = False;
		}
	}

	void DSPoly::changePoint(const Vector<Float> &newLocation, const Int point) {
		itsPoints(point, 0) = newLocation[0];
		itsPoints(point, 1) = newLocation[1];
		setHandlePositions(itsPoints);
		itsValidCenter = False;
	}

	void DSPoly::addPoint(const Vector<Float> &newPoint) {
		//Need to think about this one.

		//Find the points it is closest two, and insert it in between them.
		itsValidCenter = False;
		Int firstPoint = -1;
		Int secondPoint = -1;
		closestPoints(itsPoints, newPoint[0], newPoint[1], firstPoint, secondPoint);
		Int position;
		if (firstPoint > secondPoint) position = firstPoint;
		else position = secondPoint;

		//Insert into position.
		Matrix<Float> newPoints(itsPoints.nrow() + 1, itsPoints.ncolumn());
		Bool inserted = False;

		for (uInt i=0; i<newPoints.nrow(); i++) {
			if(i == uInt(position)) {
				newPoints(i,0) = newPoint[0];
				newPoints(i,1) = newPoint[1];
				inserted = True;
			} else {
				if (!inserted) {
					newPoints(i,0) = itsPoints(i,0);
					newPoints(i,1) = itsPoints(i,1);
				} else {
					newPoints(i,0) = itsPoints(i-1,0);
					newPoints(i,1) = itsPoints(i-1,1);
				}
			}

		}
		itsPoints.resize();
		itsPoints = newPoints;
		addHandle(newPoint, False, position);
	}

	void DSPoly::deletePoint(const Vector<Float> & /*removePoint*/) {
		//NYI
		//itsValidCenter = False;
		//cerr << "Sorry. I haven't been implemented yet." << endl;
	}

	void DSPoly::deletePoint(const Int /*nPoint*/) {
		//NYI
		//itsValidCenter = False;
		//cerr << "Sorry. I haven't been implemented yet." << endl;
	}

	void DSPoly::move(const Float& dX, const Float& dY) {
		itsPoints = translateMatrix(itsPoints, dX, dY);
		DSClosed::move(dX, dY);
		//
		if (itsValidCenter) {
			itsPolyCenter[0] += dX;
			itsPolyCenter[1] += dY;
		}
		//

	}

	Vector<Float> DSPoly::getCenter() {
		if (itsValidCenter) {
			return itsPolyCenter;
		}

		Vector<Float> newCenter(2);
		Float centerX(0), centerY(0), common(0);
		Float area(getArea());
		uInt n(itsPoints.nrow());

		for (uInt i=0; i<(n-1); i++) {
			common = ((  itsPoints(i,0) * itsPoints(i+1,1)   ) - ( itsPoints(i+1, 0) * itsPoints(i,1)    ));
			centerX += ( itsPoints(i,0)+itsPoints(i+1,0) ) * common;
			centerY += ( itsPoints(i,1)+itsPoints(i+1,1) ) * common;
		}
		//Do the "wrap" around one
		common = ((  itsPoints(n-1,0) * itsPoints(0,1)   ) - ( itsPoints(0, 0) * itsPoints(n-1,1)    ));
		centerX += ( itsPoints(n-1,0)+itsPoints(0,0) ) * common;
		centerY += ( itsPoints(n-1,1)+itsPoints(0,1) ) * common;

		newCenter[0] = abs((centerX / (6 * area)));
		newCenter[1] = abs((centerY/ (6 * area)));

		if (isNaN(newCenter[0]) || isNaN(newCenter[1]) || isInf(newCenter[0])
		        || isInf(newCenter[1])  ) {
			//Assume shape is simply too small to calc.
			newCenter[0] = itsPoints(0,0);
			newCenter[1] = itsPoints(0,1);
		}
		//
		itsValidCenter = True;
		itsPolyCenter = newCenter;
		//


		return newCenter;

	}

	Float DSPoly::getArea() {
		Int n(itsPoints.nrow());
		Int i;
		Int j(0);

		Float area = 0;

		for (i=0 ; i<n ; i++) {
			j = (i + 1) % n;
			area += itsPoints(i,0) * itsPoints(j,1); //polygon[i].x * polygon[j].y;
			area -= itsPoints(i,1) * itsPoints(j,0); //polygon[i].y * polygon[j].x;
		}
		area /= 2;
		return(area < 0 ? -area : area);

	}

	void DSPoly::setPoints(const Matrix<Float> &newPoints) {

		itsPoints = newPoints;
		itsValidCenter = False;
		setHandlePositions(itsPoints);

	}

	Matrix<Float> DSPoly::getPoints() {
		return itsPoints;
	}

	void DSPoly::setDefaultOptions() {
		itsPolyCenter.resize(2);
		itsPoints.resize();
		itsValidCenter = False;
		setHasHandles(True);
		setDrawHandles(False);
	}









} //# NAMESPACE CASA - END

