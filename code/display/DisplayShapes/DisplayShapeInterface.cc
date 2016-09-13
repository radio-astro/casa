//# DisplayShapeInterface.cc: Simple wrapper for DisplayShapes
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

#include <casa/Exceptions/Error.h>
#include <display/DisplayShapes/DisplayShapeInterface.h>
#include <display/DisplayShapes/DSRectangle.h>
#include <display/DisplayShapes/DSEllipse.h>
#include <display/DisplayShapes/DSCircle.h>
#include <display/DisplayShapes/DSSquare.h>
#include <display/DisplayShapes/DSText.h>
#include <display/DisplayShapes/DSArrow.h>
#include <display/DisplayShapes/DSPolyLine.h>
#include <display/DisplayShapes/DSMarker.h>
#include <scimath/Mathematics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DisplayShapeInterface::DisplayShapeInterface(const Record& shapeInfo) :
		itsShape(0),
		itsPc(0),
		itsWc(0) {

		itsCoords = DisplayShapeInterface::Pixels;
		makeShape(shapeInfo);
	}

	DisplayShapeInterface::DisplayShapeInterface(const Record& shapeInfo,
	        const PixelCanvas* pc,
	        const Bool scale) :
		itsShape(0),
		itsPc(pc),
		itsWc(0) {

		itsCoords = DisplayShapeInterface::Relative;
		itsScale = scale;
		makeShape(shapeInfo);
	}

	DisplayShapeInterface::DisplayShapeInterface(const Record shapeInfo,
	        const WorldCanvas* wc) :
		itsShape(0),
		itsPc(0),
		itsWc(wc) {

		itsCoords = DisplayShapeInterface::World;
		makeShape(shapeInfo);
	}

	DisplayShapeInterface::DisplayShapeInterface(const DisplayShapeInterface&
	        other) :
		itsShape(other.itsShape),
		itsCoords(other.itsCoords),
		itsPc(other.itsPc),
		itsWc(other.itsWc),
		itsScale(other.itsScale),
		altCoords(other.altCoords) {
	}

	DisplayShapeInterface::~DisplayShapeInterface() {
		if (itsShape) {
			delete itsShape;
			itsShape = 0;
		}
	}

	void DisplayShapeInterface::makeShape(const Record& shapeInfo) {

		if (!shapeInfo.isDefined("type"))
			throw(AipsError("DisplayShapeInterface.cc - No \"Type\" specified. Can't make a shape of unknown type."));

		String type(shapeInfo.asString("type"));
		type.downcase();

		if (type == "rectangle") {
			itsShape = new DSRectangle();
		} else if (type == "ellipse") {
			itsShape = new DSEllipse();
		} else if (type == "circle") {
			itsShape = new DSCircle();
		} else if (type == "square") {
			itsShape = new DSSquare();
		} else if (type == "text") {
			itsShape = new DSText();
		} else if (type == "arrow") {
			itsShape = new DSArrow();
		} else if (type == "polyline") {
			itsShape = new DSPolyLine();
		} else if (type == "marker") {
			itsShape = new DSMarker();
		} else {
			throw(AipsError("DisplayShapeInterface.cc - Unkown Shape"));
		}

		// Turn all Options into pixel values and set them
		itsShape->setOptions(toPixOpts(shapeInfo));

		// We just copy instead of using updateNonPixelPosition() in case
		// we don't yet have a valid canvas to work with.
		altCoords = shapeInfo;

	}

	Record DisplayShapeInterface::toPixOpts(const Record& settings) {
		Record toReturn = settings;

		switch (itsCoords) {

		case DisplayShapeInterface::Pixels :
			break;
		case DisplayShapeInterface::Relative :
			toReturn = relToPixOpts(settings);
			break;
		case DisplayShapeInterface::World :
			toReturn = worldToPixOpts(settings);
			break;
		}

		return toReturn;
	}


	Record DisplayShapeInterface::fromPixOpts(const Record& from) {
		Record toReturn = from;
		switch (itsCoords) {
		case DisplayShapeInterface::Pixels :
			toReturn = itsShape->getOptions();
			break;
		case DisplayShapeInterface::Relative :
			toReturn = pixToRelOpts(itsShape->getOptions());
			break;
		case DisplayShapeInterface::World :
			toReturn = pixToWorldOpts(itsShape->getOptions());
			break;
		}
		return toReturn;

	}

	/*
	 *  Save the shape in its non pixel form
	 */
	Bool DisplayShapeInterface::updateNonPixelPosition() {
		altCoords = fromPixOpts(itsShape->getOptions());
		return True;
	}

	/*
	 *  Update the pixel display of the shape to reflect it's non pixel coords
	 */
	Bool DisplayShapeInterface::updatePixelPosition() {
		itsShape->setOptions(toPixOpts(altCoords));
		return True;
	}


	/*
	 *  Turn an option record from pixel to relative
	 */
	Record DisplayShapeInterface::pixToRelOpts(const Record& pixel) {
		Record toReturn = pixel;

		if (pixel.isDefined("center")) {
			toReturn.removeField("center");
			toReturn.define("center", pixToRel(pixel.asArrayFloat("center")));
		}

		if (pixel.isDefined("startpoint")) {
			toReturn.removeField("startpoint");
			toReturn.define("startpoint",
			                pixToRel(pixel.asArrayFloat("startpoint")));
		}

		if (pixel.isDefined("endpoint")) {
			toReturn.removeField("endpoint");
			toReturn.define("endpoint", pixToRel(pixel.asArrayFloat("endpoint")));
		}

		if (pixel.isDefined("width") && itsScale ) {
			toReturn.removeField("width");
			toReturn.define("width", pixToRelWidth(pixel.asFloat("width")));
		}

		if (pixel.isDefined("height") && itsScale) {
			toReturn.removeField("height");
			toReturn.define("height", pixToRelHeight(pixel.asFloat("height")));
		}

		if (pixel.isDefined("minoraxis") && itsScale) {
			toReturn.removeField("minoraxis");
			toReturn.define("minoraxis",
			                pixToRelHeight(pixel.asFloat("minoraxis")));
		}

		if (pixel.isDefined("majoraxis") && itsScale) {
			toReturn.removeField("majoraxis");
			toReturn.define("majoraxis",
			                pixToRelHeight(pixel.asFloat("majoraxis")));
		}

		return toReturn;

	}

	/*
	 *  Turn an option record from relative to pixel
	 */
	Record DisplayShapeInterface::relToPixOpts(const Record& relative) {
		Record toReturn = relative;

		if (relative.isDefined("center")) {
			toReturn.removeField("center");
			toReturn.define("center", relToPix(relative.asArrayFloat("center")));
		}

		if (relative.isDefined("startpoint")) {
			toReturn.removeField("startpoint");
			toReturn.define("startpoint",
			                relToPix(relative.asArrayFloat("startpoint")));
		}

		if (relative.isDefined("endpoint")) {
			toReturn.removeField("endpoint");
			toReturn.define("endpoint", relToPix(relative.asArrayFloat("endpoint")));
		}

		/*
		if (relative.isDefined("width")
		    && relative.isDefined("height")
		    && itsScale) {

		  // Get the current pixel dimensions
		  Record temp = itsShape->getOptions();
		  if (!(temp.isDefined("width") && temp.isDefined("height")))
		throw("Error converting to pixel co-ords");

		  Float oldPixHeight = temp.asFloat("height");
		  Float oldPixWidth = temp.asFloat("width");

		  cerr << "Obtained old height as: " << oldPixHeight << endl;
		  cerr << "Obtained old width as: " << oldPixWidth << endl;

		  cerr << "Aspect ratio of rectangle (height / width) : " <<
		    oldPixHeight / oldPixWidth << endl;
		  Float aspect = oldPixHeight / oldPixWidth;

		  Float newPixHeight = relToPixHeight(relative.asFloat("height"));
		  Float newPixWidth = relToPixWidth(relative.asFloat("width"));

		  if ((newPixWidth * aspect) > newPixHeight) {
		    newPixWidth = newPixHeight / aspect;
		  } else {
		    newPixHeight = newPixWidth * aspect;
		  }

		  cerr << "Defining new height as: " << newPixHeight << endl;
		  cerr << "Defining new width as: " << newPixWidth << endl;

		  toReturn.removeField("width");
		  toReturn.define("width", newPixWidth);
		  toReturn.define("height", newPixHeight);

		}
		*/

		if (relative.isDefined("width") && itsScale) {
			toReturn.removeField("width");
			toReturn.define("width", relToPixWidth(relative.asFloat("width")));
		}

		if (relative.isDefined("height") && itsScale) {
			toReturn.removeField("height");
			toReturn.define("height", relToPixHeight(relative.asFloat("height")));
		}



		if (relative.isDefined("minoraxis") && itsScale) {
			toReturn.removeField("minoraxis");
			toReturn.define("minoraxis",
			                relToPixHeight(relative.asFloat("minoraxis")));
		}

		if (relative.isDefined("majoraxis") && itsScale) {
			toReturn.removeField("majoraxis");
			toReturn.define("majoraxis", relToPixWidth(relative.asFloat("majoraxis")));
		}

		return toReturn;
	}

	Vector<Float> DisplayShapeInterface::pixToRel(const Vector<Float>& pix) {
		if (pix.nelements() != 2)
			throw(AipsError("DisplayShapeInterface.cc - Bad arg to pixToRel()"));

		Vector<Float> relative(2);

		relative[0] = pix[0] / itsPc->width();
		relative[1] = pix[1] / itsPc->height();

		if (     isNaN(relative[0]) || isInf(relative[0])
		         || isNaN(relative[1]) || isInf(relative[1]) ) {
			relative[0] = 0.0;
			relative[1] = 0.0;
		}
		return relative;
	}


	Vector<Float> DisplayShapeInterface::relToPix(const Vector<Float>& rel) {
		if (rel.nelements() != 2)
			throw(AipsError("DisplayShapeInterface.cc - Bad arg to relToPix()"));

		Vector<Float> pixels(2);
		pixels[0] = itsPc->width() * rel[0];
		pixels[1] = itsPc->height() * rel[1];

		if ((     isNaN(pixels[0]) || isInf(pixels[0])
		          || isNaN(pixels[1]) || isInf(pixels[1]) )) {
			// Maybe the PC wasn't ready for out call to ->width and ->height
			// The next refresh should recaculate this.
			pixels[0]=0;
			pixels[1]=0;
		}

		return pixels;
	}

	Float DisplayShapeInterface::relToPixWidth(const Float rel) {
		Float width;
		width = rel * itsPc->width();
		if (isNaN(width) || isInf(width))
			width = 1;

		return width;
	}

	Float DisplayShapeInterface::relToPixHeight(const Float rel) {
		Float height;
		height = rel * itsPc->height();

		if (isNaN(height) || isInf(height))
			height = 1;

		return height;
	}

	Float DisplayShapeInterface::pixToRelWidth(const Float pix) {
		Float width;
		width = pix / itsPc->width();

		if (isNaN(width) || isInf(width))
			width = 1;

		return width;
	}

	Float DisplayShapeInterface::pixToRelHeight(const Float pix) {
		Float height;
		height = pix / itsPc->height();

		if (isNaN(height) || isInf(height))
			height = 1;

		return height;
	}


	Record DisplayShapeInterface::pixToWorldOpts(const Record& /*pix*/) {
		//NYI
		Record null;
		return null;
	}

	Record DisplayShapeInterface::worldToPixOpts(const Record& /*world*/) {
		Record null;
		return null;
	}

	void DisplayShapeInterface::setDrawHandles(const Bool& draw) {
		itsShape->setDrawHandles(draw);
	}

	Bool DisplayShapeInterface::whichHandle(const Float xPos, const Float yPos,
	                                        Int& out) {
		return itsShape->whichHandle(xPos, yPos, out);
	}

	Vector<Float> DisplayShapeInterface::getCenter() const {
		return itsShape->getCenter();
	}

	void DisplayShapeInterface::addPoint(const Vector<Float>& newPoint) {
		itsShape->addPoint(newPoint);
	}

	void DisplayShapeInterface::draw(PixelCanvas *toDrawTo) {
		itsShape->draw(toDrawTo);
	}

	Bool DisplayShapeInterface::inObject(const Float xPos, const Float yPos) {
		return itsShape->inObject(xPos, yPos);
	}

	Bool DisplayShapeInterface::onHandles(const Float xPos, const Float yPos) {
		return itsShape->onHandles(xPos, yPos);
	}

	DisplayShape* DisplayShapeInterface::getObject() {
		return itsShape;
	}

	void DisplayShapeInterface::addLocked(DisplayShapeInterface* toLock) {
		itsShape->addLocked(toLock->getObject());
	}

	void DisplayShapeInterface::removeLocked(DisplayShapeInterface* removeLock) {
		itsShape->removeLocked(removeLock->getObject());
	}

	void DisplayShapeInterface::setCenter(const Float xPos, const Float yPos) {
		itsShape->setCenter(xPos, yPos);
	}

	void DisplayShapeInterface::changePoint(const Vector<Float>& newPos) {
		itsShape->changePoint(newPos);
	}

	void DisplayShapeInterface::changePoint(const Vector<Float>& newPoint,
	                                        const Int nPoint) {
		itsShape->changePoint(newPoint, nPoint);
	}

	void DisplayShapeInterface::rotate(const Float angle) {
		itsShape->rotate(angle);
	}

	void DisplayShapeInterface::scale(const Float scale) {
		itsShape->scale(scale);
	}

	void DisplayShapeInterface::move(const Float dX, const Float dY) {
		itsShape->move(dX, dY);
	}

	Bool DisplayShapeInterface::setOptions(const Record& newOptions) {
		try {
			return itsShape->setOptions(toPixOpts(newOptions));
		} catch (const AipsError &x) {
			throw(x);
		}

		return False;
	}

	Record DisplayShapeInterface::getOptions() {
		return fromPixOpts(itsShape->getOptions());
	}

	void DisplayShapeInterface::toPolygon() {
		DisplayShape* asPolygon;

		if ((itsShape->getOptions()).asString("type") != "polyline")
			throw(AipsError("I can only make shapes of type \"polyline\" into a polygon"));
		else {
			// Manipulate the settings record.
			Record polyline = itsShape->getOptions();
			polyline.removeField("type");
			polyline.define("type", "polygon");

			Matrix<Float> thePoints = polyline.asArrayFloat("polylinepoints");

			polyline.removeField("polylinepoints");
			polyline.define("polygonpoints", thePoints);

			asPolygon = new DSPoly;
			asPolygon->setOptions(polyline);

			delete itsShape;
			itsShape = asPolygon;

		}
		updateNonPixelPosition();

	}




} //# NAMESPACE CASA - END

