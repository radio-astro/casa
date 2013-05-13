//# DSArrow.h: Arrow implementation for "DisplayShapes"
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

#ifndef TRIALDISPLAY_DSARROW_H
#define TRIALDISPLAY_DSARROW_H

#include <casa/aips.h>

#include <display/DisplayShapes/DSLine.h>
#include <display/DisplayShapes/DSPoly.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;
	template <class T> class DParameterRange;

// <summary>
// Arrow implementation;adds a polygon to a DSLine to represent the arrow head
// </summary>
//
// <prerequisite>
// <li> <linkto class="DSLine">DSLine</linkto>
// <li> <linkto class="DSPoly">DSPoly</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSArrow is a method of managing the drawing of a line and polygon onto a
// PixelCanvas.
// </etymology>
//
// <synopsis>
// DSArrow simply extends from DSLine, and adds the management of a polygon
// to it. The polygon is used to represent the arrow head, and various options
// regarding its apppearance can be changed.
//
// There are generally two ways to make DisplayShape(s); To create them in
// "one hit" by providing arguments to the constructor, or by using the
// default constructor and then the "setOptions" method. A simple interface
// for all classes inheriting from the
// <linkto class="DisplayShape">DisplayShape</linkto> class is provided by
// <linkto class="DisplayShapeInterface">DisplayShapeInterface</linkto>.
// </synopsis>
//
// <motivation>
// It was thought, especially in an annotations context, that an Arrow would be
// a usefull drawing tool.
// </motivation>
//
// <example>
// <srcblock>
// Vector<Float> startPoint(2); startPoint[0] = 100; startPoint[1] = 100;
// Vector<Float> endPoint(2);   endPoint[0] = 200;   endPoint[1] = 200;
//
// DSArrow* myArrow =
// new DSArrow(startPoint, endPoint, DSArrow::Filled_Triangle, 10);
//
// myArrow->move(10,10);
//
// Vector<Float> newStart(2); newStart[0] = 50; newStart[1] = 50;
// myArrow->setStartPoint(newStart);
//
// Record newArrow;
// newArrow.define("arrowheadsize", 6);
//
// myArrow->setOptions(newArrow);
// myArrow->draw(myPixelCanvas);
// etc..
// </srcblock>
// </example>

	class DSArrow : public DSLine {

	public:

		// Supported arrow head types
		enum ArrowHead { Open_Triangle, Filled_Triangle , Cool_Triangle};

		// For checking setoptions(arrowheadstyle = ?)
		static const int numArrowHeads = 3;


		// Default constructor. In order to make the arrow "valid", it must
		// be supplied with a valid start and end point.
		DSArrow();

		// The start and end point are in pixel co-ordinates.
		DSArrow(const Vector<Float> &startPoint, const Vector<Float> &endPoint,
		        DSArrow::ArrowHead style = DSArrow::Cool_Triangle,
		        const Int arrowHeadSize = 8);

		// Copy constructor
		DSArrow(const DSArrow& other);

		// Destructor
		virtual ~DSArrow();

		// Standard Display Shape functions
		// <group>
		virtual void draw(PixelCanvas *pc);
		virtual void move(const Float& dX, const Float& dY);
		virtual void rotate(const Float& angle);
		virtual void scale(const Float& scaleFactor);
		virtual void setCenter(const Float& xPos, const Float& yPos);
		// </group>

		// Does nothing currently
		virtual void rotateAbout(const Float& angle, const Float& aboutX,
		                         const Float& aboutY);

		// Standard (changePoint) and specific funtions for altering the arrow. the
		// changePoint method with only one argument will move the closest point to
		// the specified new point. When speciffyin 'n', in this case only 0 or
		// 1 are valid. The 'startpoint' is the one with the arrow head on it.
		// <group>
		virtual void setStartPoint(const Vector<Float>& startPoint);
		virtual void setEndPoint(const Vector<Float>& endPoint);
		virtual void changePoint(const Vector<Float>&pos, const Int n);
		virtual void changePoint(const Vector<Float>& pos);
		// </group>

		// These are to enable the use of arrows more easily in vector plots
		// <group>
		virtual void setLength(const Float& pixelLength);
		virtual void setAngle(const Float& angle);
		// </group>

		// Return an option record describing the shape
		virtual Record getOptions();

		// Set options.
		virtual Bool setOptions(const Record& newSettings);

	protected:

	private:
		// If someone calls "setLength" before "setCenter", we can store our
		// length until they DO call setLength;
		Float itsLength;

		// Make a new arrow head (which is simply a DSPoly)
		virtual void buildArrowHead();

		// Caluclate angle etc.
		virtual void updateArrowHead();

		// Set default options
		virtual void setDefaultOptions();

		// Construct the line segment of the arrow when we have
		// valid start / end points
		virtual void make();

		DParameterRange<Int>* itsHeadSize;

		DSPoly* itsArrowHead;

		// Temp storage while I am being set up
		// <group>
		Vector<Float> itsStartPoint;
		Vector<Float> itsEndPoint;
		// </group>

		// A Matrix of the unRotated arrow head
		Matrix<Float> itsUnrotatedHead;

		// Offset needed to make sure the tip of the arrow is at the end of
		// the line.
		Float itsOffset;

		// Style of arrow head
		DSArrow::ArrowHead itsArrowHeadStyle;

		// Have I been supplied with valid points yet
		Bool itsValidStart,itsValidEnd;
	};


} //# NAMESPACE CASA - END

#endif













