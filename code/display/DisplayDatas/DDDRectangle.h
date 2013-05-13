//# DDDRectangle.h: implementation of rectangular DDDObject
//# Copyright (C) 1999,2000,2002
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
//# $Id$

#ifndef TRIALDISPLAY_DDDRECTANGLE_H
#define TRIALDISPLAY_DDDRECTANGLE_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <display/DisplayDatas/DDDObject.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of a rectangular object for DrawingDisplayData class.
// </summary>

// <synopsis>
// This class implements the interface defined by DDDObject, to provide
// a rectangle object for registration with DrawingDisplayData objects.
// </synopsis>

	class DDDRectangle : public DDDObject {

	public:

		// Constructor taking a Record description.  Fields in the record,
		// on top of what is consumed by the DDDObject constructor, are:
		// <src>blc</src> and <src>trc</src>.
		DDDRectangle(const Record &description, DrawingDisplayData *owner);

		// Destructor.
		virtual ~DDDRectangle();


		// Draw this rectangle object for the given reason on the provided
		// WorldCanvas.
		virtual void draw(const Display::RefreshReason &reason,
		                  WorldCanvas *worldcanvas);

		// Return a record describing this object.
		virtual Record description();

		// Update this object based on the information in the provided
		// Record.
		virtual void setDescription(const Record &rec);

		// Event handlers.  The parent DrawingDisplayData will distribute
		// events as necessary to the various DDDObjects which comprise it.
		// <group>
		virtual void operator()(const WCRefreshEvent &/*ev*/) {
			;
		};
		virtual void operator()(const WCPositionEvent &ev);
		virtual void operator()(const WCMotionEvent &ev);
		// </group>

	protected:

		// (Required) default constructor.
		DDDRectangle();

		// (Required) copy constructor.
		DDDRectangle(const DDDRectangle &other);

		// (Required) copy assignment.
		void operator=(const DDDRectangle &other);

	private:

		// Blc and trc of rectangle in world coordinates.
		Vector<Double> itsBlc, itsTrc;

		// Handle coordinates.
		Vector<Int> itsHX, itsHY;

		// Mode.
		DDDObject::Mode itsMode;

		// Store for movement bases.
		Int itsBaseMoveX, itsBaseMoveY;

		// is it a left handle?  a bottom handle?
		Bool itsLeftHandle, itsBottomHandle;

	};


} //# NAMESPACE CASA - END

#endif
