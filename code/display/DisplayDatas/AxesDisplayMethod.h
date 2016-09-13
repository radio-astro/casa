//# AxesDisplayMethod.h: axis label drawing for AxesDisplayData
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

#ifndef TRIALDISPLAY_AXESDISPLAYMETHOD_H
#define TRIALDISPLAY_AXESDISPLAYMETHOD_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;
	class AxesDisplayData;

// <summary>
// Class to draw a single set of axis labels for AxesDisplayData.
// </summary>
//
// <synopsis>
// This class adds to the interface defined by DisplayMethod to
// provide the necessary infrastructure to draw a single set of
// axis labels when requested by the AxesDisplayData class.
// </synopsis>

	class AxesDisplayMethod : public CachingDisplayMethod {

	public:

		// Constructor.
		AxesDisplayMethod(WorldCanvas *worldCanvas,
		                  AttributeBuffer *wchAttributes,
		                  AttributeBuffer *ddAttributes,
		                  CachingDisplayData *dd);

		// Destructor.
		virtual ~AxesDisplayMethod();

		// Clean up (ie. delete any existing cached display list).
		virtual void cleanup();

		// Draw into a cached drawing list, called by draw function.
		virtual Bool drawIntoList(Display::RefreshReason reason,
		                          WorldCanvasHolder &wcHolder);

	protected:

		// (Required) default constructor.
		AxesDisplayMethod();

		// (Required) copy constructor.
		AxesDisplayMethod(const AxesDisplayMethod &other);

		// (Required) copy assignment.
		void operator=(const AxesDisplayMethod &other);

	private:

	};


} //# NAMESPACE CASA - END

#endif
