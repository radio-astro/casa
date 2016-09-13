//# DisplayEvent.h: class for basic event information in the display classes
//# Copyright (C) 1999,2000
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

#ifndef DISPLAY_DISPLAYEVENT_H
#define DISPLAY_DISPLAYEVENT_H

#include <casa/aips.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class describing the most basic event information in the display classes.
// </summary>

// <use visibility=local>

// <reviewed reviewer="Ralph Marson" date="2000/04/07" tests="tDisplayEvent">
// </reviewed>

// <etymology>
// "DisplayEvent" describes "Events" (ie. things which happen at a
// measurable time) which various "Display" class objects would like
// to know about.
// </etymology>

// <synopsis>
// This class is a simple class which provides the base for all event
// information in the display classes.  It simply records the one
// thing common to all events: the time the event occured.  The time
// recorded is the Julian date at the time of construction of a
// DisplayEvent object, stored and returned in seconds.
// <synopsis>

// <example>
// The following example constructs two DisplayEvent instances, and
// checks that they have stored event times that are ordered correctly
// by the order in which they were constructed:
// <srcblock>
// DisplayEvent *de1, *de2;
// de1 = new DisplayEvent();
// de2 = new DisplayEvent();
// if (de1.timeOfEvent() >= de2.timeOfEvent()) {
//     throw(AipsError("I have invented a time machine!"));
// }
// </srcblock>
// </example>

// <motivation>
// It is desirable to locate in a single place the information common
// to all events of interest to the display classes.  At the very
// lowest level, the only such common information is time data.
// </motivation>

// <todo asof="1999/10/15">
// Nothing known.
// </todo>

	class DisplayEvent {

	public:

		// Constructor.  The Julian date at construction is recorded as the
		// event time of this DisplayEvent.
		DisplayEvent();

		// Copy constructor - construct a new DisplayEvent from
		// <src>other</src>.
		DisplayEvent(const DisplayEvent &other);

		// Destructor.
		virtual ~DisplayEvent();

		// Copy assignment using copy semantics.
		DisplayEvent &operator=(const DisplayEvent &other);

		// Return the Julian date (in fractional seconds) that this event
		// occured.
		virtual Double timeOfEvent() const;

	private:

		// Store the time of the event here at construction.
		Double itsTimeOfEvent;

	};


} //# NAMESPACE CASA - END

#endif
