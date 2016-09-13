//# DSEllipse.h: Ellipse implementation for "DisplayShapes"
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

#ifndef TRIALDISPLAY_DSELLIPSE_H
#define TRIALDISPLAY_DSELLIPSE_H

#include <casa/aips.h>

#include <display/DisplayShapes/DSClosed.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of a ellipse.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSEllipse is a method of managing the drawing of a ellipse onto a
// PixelCanvas.
// </etymology>
//
// <synopsis>
// DSEllipse is the DisplayShape implementation of the primitive ellipse.
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
// To allow the management of ellipses being draw to a pixel canvas.
// </motivation>
//
// <example>
// </example>



	class DSEllipse : public DSClosed {

	public:
		// Constructors and destructors. the 'onlyShowOneHandle' flag is primarily
		// for use if the ellipse is intended to remain symmetrical i.e. a circle.
		// <group>
		DSEllipse(const Bool& onlyShowOneHandle = False);
		DSEllipse(const DSEllipse& other);
		DSEllipse(const Float& xPos, const Float& yPos, const Float& major,
		          const Float& minor,
		          const Bool& hasHandles = False, const Bool& drawHandles = False,
		          const Bool& onlyShowOneHandle = False);
		virtual ~DSEllipse();
		// </group>

		// General DisplayShape functions.
		// <group>
		virtual void draw(PixelCanvas *pix);
		virtual void move(const Float& dX, const Float& dY);
		virtual Bool inObject(const Float& xPos, const Float& yPos);
		virtual void rotate(const Float& angle);
		virtual void rotateAbout(const Float& angle, const Float& aboutX,
		                         const Float& aboutY);
		virtual void setCenter(const Float& xPos, const Float& yPos);
		virtual Vector<Float> getCenter();
		virtual void changePoint(const Vector<Float>& newPoint);
		virtual void changePoint(const Vector<Float>& newPoint, const Int nPoint);
		virtual void scale(const Float& scaleFactor);
		// </group>

		// Functions to set / get the minor and major axis of the ellipse
		// (in pixels).
		// <group>
		virtual Float getMinorAxis();
		virtual Float getMajorAxis();
		virtual void setMajorAxis(const Float& newMajor);
		virtual void setMinorAxis(const Float& newMinor);
		// </group>

		// Get and set options
		// <group>
		virtual Record getOptions();
		virtual Bool setOptions(const Record& settings);
		// </group>

	protected:
		// Required on update
		virtual void calculateHandlePositions();
	private:

		Vector<Float> itsCenter;
		Float itsAngle;
		Float itsMajorAxis, itsMinorAxis;
		Bool itsValid, itsOneHandle;

		Matrix<Float> itsHandleLocation;

		// Called when def. constructor used
		virtual void setDefaultOptions();


	};


} //# NAMESPACE CASA - END

#endif


