//# CachingDisplayMethod.h: assistance for auto-caching DisplayData objects
//# Copyright (C) 1999,2000,2001
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

#ifndef TRIALDISPLAY_CACHINGDISPLAYMETHOD_H
#define TRIALDISPLAY_CACHINGDISPLAYMETHOD_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayDatas/DisplayMethod.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class AttributeBuffer;
	class WorldCanvas;
	class WorldCanvasHolder;
	class CachingDisplayData;

// <summary>
// Assistance class for auto-caching of DisplayData objects.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" data="" tests="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DisplayMethod> DisplayMethod </linkto>
// <li> <linkto class=CachingDisplayData> CachingDisplayData </linkto>
// </prerequisite>

// <etymology>
// "CachingDisplayMethod" is an implementation of a <linkto
// class=DisplayMethod> DisplayMethod </linkto> which provides a
// single element in a cache of individual depictions of some data.
// </etymology>

// <synopsis>
// </synopsis>


	class CachingDisplayMethod : public DisplayMethod {

	public:

		// Constructor.  A single instance of a CachingDisplayMethod is
		// setup to draw on a particular WorldCanvas, and only when
		// particular conditions, as stored in the provided
		// AttributeBuffers, are met.  The parent DisplayData is given
		// in <src>parentDisplayData</src>.
		CachingDisplayMethod(WorldCanvas *worldCanvas,
		                     AttributeBuffer *wchAttributes,
		                     AttributeBuffer *ddAttributes,
		                     CachingDisplayData *parentDisplayData);

		// Destructor.
		virtual ~CachingDisplayMethod();

		// Return the WorldCanvas recorded in this CachingDisplayMethod.
		virtual WorldCanvas *worldCanvas() {
			return itsWorldCanvas;
		}

		// Draw this slice of data on the supplied WorldCanvasHolder.
		virtual void draw(Display::RefreshReason reason,
		                  WorldCanvasHolder &wcHolder);

	protected:

		// (Required) default constructor.
		CachingDisplayMethod();

		// (Required) copy constructor.
		CachingDisplayMethod(const CachingDisplayMethod &other);

		// (Required) copy assignment.
		void operator=(const CachingDisplayMethod &other);

		// Draw into a cached drawing list, called by draw function.
		// The return value is set False if the DM could not draw or
		// does not wish a drawlist to be saved.
		virtual Bool drawIntoList(Display::RefreshReason reason,
		                          WorldCanvasHolder &wcHolder) = 0;

	private:

		// Store for the WorldCanvas which this has previously drawn upon.
		WorldCanvas *itsWorldCanvas;

		// Store for WorldCanvasHolder- and DisplayData-specific
		// AttributeBuffers.
		AttributeBuffer *itsWCHAttributes, *itsDDAttributes;

		// Do we have a cached drawing list?
		Bool itsHasList;

		// Store for cached drawing list number.
		uInt itsCachedDrawingList;

	};


} //# NAMESPACE CASA - END

#endif
