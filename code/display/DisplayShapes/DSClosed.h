//# DSClosed.h: Base class for all 'closed' DisplayShapes
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

#ifndef TRIALDISPLAY_DSCLOSED_H
#define TRIALDISPLAY_DSCLOSED_H


#include <casa/aips.h>

#include <display/DisplayShapes/DSBasic.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// DSClosed provides common functionality for all "closed" shapes.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DSBasic">DSBasic</linkto>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSClosed is used to provide a common interface for all closed shapes
// </etymology>
//
// <synopsis>
// DSClosed simply manages the fill options of any closed display shapes.
// </synopsis>
//
// <motivation>
// A desire for a common interface for all closed shapes.
// </motivation>
//
// <example>
// <srcblock>
// </srcblock>
// </example>

	class DSClosed : public DSBasic {

	public:

		// Type of fill to use.
		enum FillStyle {No_Fill, Full_Fill};

		// Constructors and destructors
		// <group>
		DSClosed();
		DSClosed(const DSClosed& other);

		virtual ~DSClosed();
		// </group>

		// Get / Set fill options
		// <group>
		virtual void setFillStyle(DSClosed::FillStyle fill);
		virtual DSClosed::FillStyle getFillStyle();
		virtual void setFillColor(String color);
		virtual String getFillColor();
		// </group>


		// General DisplayShape functions. The non abstract functions in this group
		// simply pass on calls up the class tree.
		// <group>
		virtual void draw(PixelCanvas *pc);
		virtual void rotateAbout(const Float& angle, const Float& aboutX,
		                         const Float& aboutY);
		virtual void move(const Float& dX, const Float& dY);
		// </group>

		// Get and set options
		// <group>
		virtual Record getOptions();
		virtual Bool setOptions(const Record& settings);
		// </group>

	private:

		// Used to set up the class when default constructor called
		virtual void setDefaultOptions();
		DSClosed::FillStyle itsFillStyle;
		String itsFillColor;

	};

} //# NAMESPACE CASA - END

#endif




