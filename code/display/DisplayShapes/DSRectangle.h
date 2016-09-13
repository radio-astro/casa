//# DSRectangle.h: Rectangle implementation for "DisplayShapes"
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

#ifndef TRIALDISPLAY_DSRECTANGLE_H
#define TRIALDISPLAY_DSRECTANGLE_H

#include <casa/aips.h>

#include <display/DisplayShapes/DSPoly.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of a rectangle.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DSPoly">DSPoly</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSRectangle is a method of managing the drawing of a rectangle onto a
// PixelCanvas.
// </etymology>
//
// <synopsis>
// DSRectangle is simply a DSPoly with only four points. It has overloaded
// certain functions however, which cause it to behave as a rectangle when
// having its points moved etc. It calculates its width and height from
// points returned from DSPoly when needed.
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
// The need to draw rectangles on screen.
// </motivation>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <todo asof="2002/05/13">
// <li> Modify functions like 'changePoint' to allow a special case when
// angle = 0 for speed reasons.
// </todo>


	class DSRectangle : public DSPoly {

	public:

		// Constructors and Destructors
		// <group>
		DSRectangle();
		DSRectangle(const casacore::Float& xPos, const casacore::Float& yPos, const casacore::Float& width,
		            const casacore::Float& height, const casacore::Bool& handles = false,
		            const casacore::Bool& drawHandles = false);
		DSRectangle(const DSRectangle& other);
		virtual ~DSRectangle();
		// </group>

		// Standard DisplayShape functions
		// <group>
		virtual void move(const casacore::Float& dX, const casacore::Float& dY);
		virtual void rotate(const casacore::Float& angle);
		virtual void rotateAbout(const casacore::Float& angle, const casacore::Float& aboutX,
		                         const casacore::Float& aboutY);
		virtual void setCenter(const casacore::Float& xPos, const casacore::Float& yPos);
		virtual void changePoint(const casacore::Vector<casacore::Float>& pos);
		virtual void changePoint(const casacore::Vector<casacore::Float>& newPoints, const casacore::Int whichOne);
		virtual void addPoint(const casacore::Vector<casacore::Float>& toAdd);
		// </group>


		// Rectangle specific. Get / set width and height in pixels.
		// <group>
		virtual void setHeight(const casacore::Float& height);
		virtual void setWidth(const casacore::Float& width);
		virtual casacore::Float getHeight();
		virtual casacore::Float getWidth();
		// </group>

		// Get and set options
		// <group>
		virtual casacore::Bool setOptions(const casacore::Record& settings);
		virtual casacore::Record getOptions();
		// </group>

		// Using supplied parameters, make a polygon of four points representing the
		// rectangle. This can then be used to create / alter a DSPoly.
		virtual casacore::Matrix<casacore::Float> makeAsPoly(const casacore::Float& xPos, const casacore::Float& yPos,
		                                 const casacore::Float& width, const casacore::Float& height);

	private:

		// Always valid
		casacore::Float itsAngle;
		casacore::Bool itsValid;

		//Ony used during the set-up period
		casacore::Float itsCenter, itsWidth, itsHeight;

		virtual void setDefaultOptions();
		virtual void setAngle(const casacore::Float& newAngle);

	protected:
		virtual casacore::Float getAngle();
	};


} //# NAMESPACE CASA - END

#endif




