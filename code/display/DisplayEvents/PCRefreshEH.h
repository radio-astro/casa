//# PCRefreshEH.h: PixelCanvas refresh event handler
//# Copyright (C) 1993,1994,1995,1996,1999,2000
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

#ifndef TRIALDISPLAY_PCREFRESHEH_H
#define TRIALDISPLAY_PCREFRESHEH_H

#include <casa/aips.h>
#include <display/DisplayEvents/PCRefreshEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;

//
// <summary>
// WorldCanvas Event Handler for managing events
// </summary>
//
// <etymology>
// WCMotionEH : WorldCanvas Motion Event Handler
// </etymology>
//
// <synopsis>
// Designed for derivation, this class is the base class which
// is communicated to when the it is necessary to refresh the
// display.  The application programmer should derive
// from this class, add any data fields needed, and override
// operator () to redraw the screen properly.
// </synopsis>
//
// <motivation>
// Wanted a standard method for event management.  It was felt
// that a class might be more flexible than callback methods
// because a class can contain data and other functions as
// well.
// </motivation>
//
// <example>
// <ul>
// <li>see the <linkto class="WorldCanvas">WorldCanvas</linkto> class
// code.
// <li>see the Display test directory
// </ul>
//
// </example>
//
// <todo>
// <li> write and test!
// </todo>
//

	class PCRefreshEH {

	public:

		// Default Constructor Required
		PCRefreshEH();

		PCRefreshEH( const PCRefreshEH& other );

		// Default just prints the event to cout
		virtual void operator ()(const PCRefreshEvent & eh);

		// Destructor
		virtual ~PCRefreshEH();

	};


} //# NAMESPACE CASA - END

#endif


