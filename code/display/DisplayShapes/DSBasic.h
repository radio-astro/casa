//# DSBasic.h: Basically, a base class for any shape which can be drawn with
//# primitive lines.
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

#ifndef TRIALDISPLAY_DSBASIC_H
#define TRIALDISPLAY_DSBASIC_H

#include <casa/aips.h>

#include <display/DisplayShapes/DisplayShape.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T> class DParameterRange;

// <summary>
// Base class for all "basic" shapes
// </summary>
//
// <prerequisite>
// <li> <linkto class="DisplayShape">DisplayShape</linkto>
// </prerequisite>
//
// <etymology>
// DSBasic is a base for all basic classes, it manages linewidth.
// </etymology>
//
// <synopsis>
// DSBasic is a base class for any shape using basic lines to draw it self,
// i.e. shapes such as lines, circles and polygons should extend this class.
// More complex DisplayShapes, e.g. text should not.
//
// <motivation>
// A common interface for the setting of linewidths
// </motivation>
//
// <example>
// <srcblock>
//
// </srcblock>
// </example>


	class DSBasic : public DisplayShape {

	public:

		// Default constructor
		DSBasic();

		// Copy constructor
		DSBasic(const DSBasic& other);

		// Destructor
		virtual ~DSBasic();

		// Manage line width
		// <group>
		virtual void setLineWidth(const Int pixels);
		virtual Int getLineWidth();
		// </group>

		// Standard fns
		// <group>
		virtual void draw(PixelCanvas* pc);
		virtual void rotateAbout(const Float& angle, const Float& aboutX,
		                         const Float& aboutY);
		virtual void move(const Float& dX, const Float& dY);
		virtual Record getOptions();
		virtual Bool setOptions(const Record& settings);
		// </group>

	private:

		DParameterRange<Int>* itsLineWidth;

	};


} //# NAMESPACE CASA - END

#endif






