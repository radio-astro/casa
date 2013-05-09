//# WCMotionEH.h: WorldCanvas motion event handler
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000
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

#ifndef TRIALDISPLAY_WCMOTIONEH_H
#define TRIALDISPLAY_WCMOTIONEH_H

#include <casa/aips.h>
#include <display/DisplayEvents/WCMotionEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//
// <summary>
// Base class for handling WorldCanvas motion events.
// </summary>
//
// <prerequisite>
// <li> <linkto class="WCMotionEvent">WCMotionEvent</linkto>
// <li> Understanding of Display library event-handling methodology
// </prerequisite>
//
// <etymology>
// WCMotionEH : WorldCanvas motion event-handler.
// </etymology>
//
// <synopsis>
// This is a base class intended to be derived from in applications.  The user must
// write a derived class and override the operator() function.  The operator() function
// is called whenever the pointer moves across the world canvas.
//
// This base class does have a concrete implementation, namely to report
// to stdout events it receives.  To effect this behaviour, create an instance
// this class and register it as an event-handler on a WorldCanvas.
// </synopsis>
//
// <motivation>
// Provide the user with an object-oriented approach to event handling and
// include world-coordinate position information.
// </motivation>
//
// <example>
// see the test programs in Display/test.
// </example>
//

	class WCMotionEH {

	public:

		// Default Constructor Required
		WCMotionEH();

		// default sends event to cout
		virtual void operator ()(const WCMotionEvent & ev) = 0;

		// Destructor
		virtual ~WCMotionEH();

	};


} //# NAMESPACE CASA - END

#endif


