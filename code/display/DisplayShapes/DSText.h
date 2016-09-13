//# Text.h: Text implementation for "DisplayShapes"
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

#ifndef TRIALDISPLAY_DSTEXT_H
#define TRIALDISPLAY_DSTEXT_H

#include <casa/aips.h>

#include <display/DisplayShapes/DisplayShape.h>
#include <display/Display/DisplayEnums.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class DParameterString;
	class DParameterFontChoice;
	class DParameterMapKeyChoice;
	template <class T> class DParameterRange;

// <summary>
// Implementation of text.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSText is a method of managing the drawing of text onto a PixelCanvas.
// </etymology>
//
// <synopsis>
// DSText is designed to have the same interface as any other 'shape'
// extending DisplayShape. Much of the functionality is exactly the same,
// and hence provides a usefull wrapper for text in an 'annotations' context.
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
// To create a text wrapper which behaved in the same was as any other
// DisplayShape
// </motivation>
//
// <example>
// <srcblock>
// </srcblock>
// </example>


	class DSText : public DisplayShape {

	public:

		// Constructors and destructors
		// <group>
		DSText();
		DSText(const DSText& other);
		DSText(const Float& xPos, const Float& yPos, const String& text = "Label",
		       const Bool& hasHandles = True, const Bool& drawHandles = True);

		virtual ~DSText();
		// </group>

		// Functions to ensure consistancy with the "DisplayShape" interface.
		// <group>
		virtual void draw(PixelCanvas* pix);
		virtual void move(const Float& dX, const Float& dY);
		virtual void rotate(const Float& angle);
		virtual Bool inObject(const Float& xPos, const Float& yPos);
		virtual void changePoint(const Vector<Float>& newPos);
		virtual void changePoint(const Vector<Float>& newPos, const Int nPoint);
		virtual void scale(const Float& scaleFactor);
		virtual void setCenter(const Float& xPos, const Float& yPos);
		virtual Vector<Float> getCenter();
		// </group>

		// Get and set options
		// <group>
		virtual Record getOptions();
		virtual Bool setOptions(const Record& settings);
		// </group>


	private:
		// This is a bit messy, so seperate it out
		void makeAlignmentChoice();

		// Update handle positions
		virtual void calculateHandlePositions();

		// Cast from int to enum
		Display::TextAlign toEnum(const Int fromInt);

		// Text Parameter
		DParameterString* itsString;

		// Angle Param
		DParameterRange<Float>* itsAngle;
		DParameterMapKeyChoice* itsAlignment;
		DParameterFontChoice* itsFont;
		DParameterRange<Int>* itsFontSize;

		// Rotate about a point.. overloaded but not used
		Vector<Float> itsCenter;
		Int itsPixHeight, itsPixWidth;
		Bool itsValid;
		Bool itsHandlesMade, itsValidPositions;
		Float itsStringLength;
		Float itsStringHeight;
		Matrix<Float> itsHandleLocation;
		Display::TextAlign itsAlign;
		virtual void setDefaultOptions();


	};

} //# NAMESPACE CASA - END

#endif



