//# DisplayShapeInterface.h: Simple wrapper for DisplayShapes
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

#ifndef TRIALDISPLAY_DISPLAYSHAPEINTERFACE_H
#define TRIALDISPLAY_DISPLAYSHAPEINTERFACE_H


#include <casa/aips.h>

#include <display/DisplayShapes/DisplayShape.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/WorldCanvas.h>
#include <casa/Containers/Record.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//class PixelCanvas;
//class WorldCanvas;

// <summary>
// Wrapper for DisplayShapes;provides easier/unified constr. of DisplayShapes
// </summary>
//
// <prerequisite>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DisplayShapeInterface makes it easier to work with DisplayShapes.
// </etymology>
//
// <synopsis>
// DisplayShapeInterface simply contains a pointer to a DisplayShape object.
// It allows all DisplayShapes to be constructed via a single constructor. i.e.
// the DisplayShapeInterface constructor actually decides which shape to
// create based on the information in the casacore::Record passed to it. It makes
// creating new shapes look a little nice, since:
// <srcblock>
// record1.define("type", "square");
// and
// record2.define("type", "circle");
// </srcblock>
// can both be simply made into a new DisplayShapeInterface, e.g.
// <srcblock>
// DisplayShapeInterface* myShape = new DisplayShapeInterface(record1);
// or
// DisplayShapeInterface* myShape = new DisplayShapeInterface(record2);
// </srcblock>
// It also guarantees consistant interfaces between shapes.
// </synopsis>
//
// <motivation>
// The creation of an interface to DisplayShapes makes use of the simpler.
// </motivation>
//
// <example>
// </example>




	class DisplayShapeInterface {

	public:
		enum Coord {Pixels, Relative, World};

		// Constructor for a standard shape. The shape will be totally based on
		// pixel sizes, both for it's position and sizes.
		DisplayShapeInterface(const casacore::Record& shapeInfo);

		// Constructor for shape based on relative screen positions. if
		// scaled is true, relative screen positions will be used for
		// height etc also. If not, positions only will be based on relative
		// screen pos, heights will be based on an absolute pixel value.
		DisplayShapeInterface(const casacore::Record& shapeInfo, const PixelCanvas* pc,
		                      const casacore::Bool scale = true);

		// Constructor for shape based on world co-ordinates (NYI)
		DisplayShapeInterface(const casacore::Record shapeInfo, const WorldCanvas* wc);

		// Copy cons
		DisplayShapeInterface(const DisplayShapeInterface& other);

		virtual ~DisplayShapeInterface();


		// DisplayShape functionality. All of these functions simply wrap those
		// of the underlying DisplayShape (and add conversion functionality).
		// Please see <linkto class="DisplayShape">DisplayShape</linkto>
		// for a full description of these functions.
		// <group>
		virtual casacore::Bool inObject(const casacore::Float xPos, const casacore::Float yPos);
		virtual casacore::Bool onHandles(const casacore::Float xPos, const casacore::Float yPos);
		virtual casacore::Vector<casacore::Float> getCenter() const;
		virtual void setCenter(const casacore::Float xPos, const casacore::Float yPos);
		virtual casacore::Bool whichHandle(const casacore::Float xPos, const casacore::Float yPos, casacore::Int& out);
		virtual void changePoint(const casacore::Vector<casacore::Float>& newPos);
		virtual void changePoint(const casacore::Vector<casacore::Float>& newPoint, const casacore::Int nPoint);
		virtual void addPoint(const casacore::Vector<casacore::Float>& newPoint);
		virtual void rotate(const casacore::Float angle);
		virtual void scale(const casacore::Float angle);
		virtual void draw(PixelCanvas* toDrawTo);
		virtual void move(const casacore::Float dX, const casacore::Float dY);
		virtual void addLocked(DisplayShapeInterface* toLock);
		virtual void removeLocked(DisplayShapeInterface* removeLock);
		virtual void setDrawHandles(const casacore::Bool& draw);
		virtual casacore::Bool setOptions(const casacore::Record& newOptions);
		virtual casacore::Record getOptions() ;
		// </group>

		// Update the pixel location of this shape, based on the stored information
		// on its location. If the shape is being used in pixel mode this will do
		// nothing. If being used in relative screen mode or WC mode, this will
		// update its location. Returns false if a conversion to pixels fail.
		virtual casacore::Bool updatePixelPosition();

		// If the shape is being used in absolute pixel co-ordinates, this will do
		// nothing. If being used in relative screen or WC mode, this will update
		// the position based on its current pixel location. Returns false if the
		// conversion to the other co-ord type fails.
		virtual casacore::Bool updateNonPixelPosition();

		// Returns the pointer to the underlying object
		virtual DisplayShape* getObject();

		// If the shape is a PolyLine, this function will change it to a polygon.
		// This is designed for use in the creation of polygon by the user clicking
		// out a polyline, for example.
		virtual void toPolygon();


	private:
		casacore::Record toPixOpts(const casacore::Record& notPix);

		// Will return an option set with whatever the current
		// coords method is.
		casacore::Record fromPixOpts(const casacore::Record& pix) ;

		casacore::Record pixToRelOpts(const casacore::Record& pixel);
		casacore::Record relToPixOpts(const casacore::Record& relative);
		casacore::Record pixToWorldOpts(const casacore::Record& pix);
		casacore::Record worldToPixOpts(const casacore::Record& world);

		casacore::Vector<casacore::Float> pixToRel(const casacore::Vector<casacore::Float>& pix);
		casacore::Vector<casacore::Float> relToPix(const casacore::Vector<casacore::Float>& rel);
		casacore::Float relToPixWidth(const casacore::Float rel);
		casacore::Float relToPixHeight(const casacore::Float rel);
		casacore::Float pixToRelWidth(const casacore::Float pix);
		casacore::Float pixToRelHeight(const casacore::Float pix);

		DisplayShape* itsShape;
		DisplayShapeInterface::Coord itsCoords;
		const PixelCanvas* itsPc;
		const WorldCanvas* itsWc;

		void makeShape(const casacore::Record& shapeInfo);

		// Scale widths and heights in relative mode?
		casacore::Bool itsScale;

		// Copy of shape, however in its alternate coord form
		casacore::Record altCoords;
	};



} //# NAMESPACE CASA - END

#endif
