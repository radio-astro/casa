//# DSPoly.h: Polygon implementation for "DisplayShapes"
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
#ifndef TRIALDISPLAY_DSPOLY_H
#define TRIALDISPLAY_DSPOLY_H

#include <casa/aips.h>

#include <display/DisplayShapes/DSClosed.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of a Polygon.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DSClosed">DSClosed</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSPoly is a method of managing the drawing of a polygon onto a PixelCanvas.
// </etymology>
//
// <synopsis>
// DSPoly allows for the management of a polygon, including the addition and
// deletion of points.
//
// There are generally two ways to make DisplayShape(s); To create them in
// "one hit" by providing arguments to the constructor, or by using the
// default constructor and then the "setOptions" method. A simple interface
// for all classes inheriting from the
// <linkto class="DisplayShape">DisplayShape</linkto> class is provided
// by <linkto class="DisplayShapeInterface">DisplayShapeInterface</linkto>.
// </synopsis>
//
// <motivation>
// To enable display of circles onto a pixel canvas.
// </motivation>
//
// <example>
// <srcblock>
// </srcblock>
// </example>


	class DSPoly : public DSClosed {

	public:

		// Constructors and destructor
		// <group>
		DSPoly();
		DSPoly(const DSPoly& other);
		DSPoly(const Matrix<Float>& points, const Bool& handles = False,
		       const Bool& drawHandles = False);

		virtual ~DSPoly();
		// </group>

		// General DisplayShape functions
		// <group>
		virtual void draw(PixelCanvas *pc);
		virtual void rotate(const Float& angle);
		virtual Bool inObject(const Float& xPos, const Float& yPos);
		virtual void scale(const Float& scale);
		virtual void move (const Float& dX, const Float& dY);
		virtual void changePoint(const Vector<Float>& newLocation);
		virtual void changePoint(const Vector<Float>& newLocation, const Int point);
		virtual void setCenter(const Float& xPos, const Float& yPos);
		virtual Vector<Float> getCenter();
		virtual void rotateAbout(const Float& angle, const Float& aboutX,
		                         const Float& aboutY);
		// </group>

		// Polygon specific functions, add, delete and set points, all in pixels.
		// Add point decides the best place in the array to insert the new point.
		// Similarly, deletePoints(const Vector<Float>& remove) deletes the point
		// closest to that provided.
		// <group>
		virtual void addPoint(const Vector<Float>& newPoint);
		virtual void deletePoint(const Vector<Float>& removePoint);
		virtual void deletePoint(const Int nPoint);
		virtual void setPoints(const Matrix<Float>& newPoints);
		// </group>


		// Get and set options
		// <group>
		virtual Record getOptions();
		virtual Bool setOptions(const Record& settings);
		// </group>

	private:

		// Points used to draw (with scaling + rotation)
		Matrix<Float> itsPoints;
		Vector<Float> itsPolyCenter;
		Bool itsValidPoints;
		Bool itsValidCenter;

		virtual void setDefaultOptions();
		virtual Float getArea();

	protected:
		virtual Matrix<Float> getPoints();

	};

} //# NAMESPACE CASA - END

#endif
