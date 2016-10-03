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
		DSText(const casacore::Float& xPos, const casacore::Float& yPos, const casacore::String& text = "Label",
		       const casacore::Bool& hasHandles = true, const casacore::Bool& drawHandles = true);

		virtual ~DSText();
		// </group>

		// Functions to ensure consistancy with the "DisplayShape" interface.
		// <group>
		virtual void draw(PixelCanvas* pix);
		virtual void move(const casacore::Float& dX, const casacore::Float& dY);
		virtual void rotate(const casacore::Float& angle);
		virtual casacore::Bool inObject(const casacore::Float& xPos, const casacore::Float& yPos);
		virtual void changePoint(const casacore::Vector<casacore::Float>& newPos);
		virtual void changePoint(const casacore::Vector<casacore::Float>& newPos, const casacore::Int nPoint);
		virtual void scale(const casacore::Float& scaleFactor);
		virtual void setCenter(const casacore::Float& xPos, const casacore::Float& yPos);
		virtual casacore::Vector<casacore::Float> getCenter();
		// </group>

		// Get and set options
		// <group>
		virtual casacore::Record getOptions();
		virtual casacore::Bool setOptions(const casacore::Record& settings);
		// </group>


	private:
		// This is a bit messy, so seperate it out
		void makeAlignmentChoice();

		// Update handle positions
		virtual void calculateHandlePositions();

		// Cast from int to enum
		Display::TextAlign toEnum(const casacore::Int fromInt);

		// Text Parameter
		DParameterString* itsString;

		// Angle Param
		DParameterRange<casacore::Float>* itsAngle;
		DParameterMapKeyChoice* itsAlignment;
		DParameterFontChoice* itsFont;
		DParameterRange<casacore::Int>* itsFontSize;

		// Rotate about a point.. overloaded but not used
		casacore::Vector<casacore::Float> itsCenter;
		casacore::Int itsPixHeight, itsPixWidth;
		casacore::Bool itsValid;
		casacore::Bool itsHandlesMade, itsValidPositions;
		casacore::Float itsStringLength;
		casacore::Float itsStringHeight;
		casacore::Matrix<casacore::Float> itsHandleLocation;
		Display::TextAlign itsAlign;
		virtual void setDefaultOptions();


	};

} //# NAMESPACE CASA - END

#endif



