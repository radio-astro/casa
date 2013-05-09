//# PCPositionEH.h: PixelCanvas position event handler
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

#ifndef TRIALDISPLAY_PCPOSITIONEH_H
#define TRIALDISPLAY_PCPOSITIONEH_H

#include <casa/aips.h>
#include <display/DisplayEvents/PCPositionEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;

//
// <summary>
// PixelCanvas Event Handler for managing keyboard and mouse button
// events
// </summary>
//
// <prerequisite>
// <li> <linkto class="PCPositionEvent">PCPositionEvent</linkto>
// </prerequisite>
//
// <etymology>
// PCPositionEH : PixelCanvas Position Event Handler
// </etymology>
//
// <synopsis>
// Designed for derivation, this class is the base class which
// is communicated to when an X button or keyboard event is sent
// to the PixelCanvas.  The application programmer should derive
// from this class, add any data fields needed, and override
// operator () to respond properly to keyboard and mouse button
// events.
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
// see the <linkto class="WorldCanvas">WorldCanvas</linkto> code
// see the Display test directory
// </example>
//

	class PCPositionEH {

	public:

		// Default Constructor Required
		PCPositionEH();

		// Default just prints the event to cout
		virtual void operator ()(const PCPositionEvent & eh);

		// Destructor
		virtual ~PCPositionEH();

	};


} //# NAMESPACE CASA - END

#endif


