//# WCPositionEvent.h: class which stores WorldCanvas position event info
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2002
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

#ifndef TRIALDISPLAY_WCPOSITIONEVENT_H
#define TRIALDISPLAY_WCPOSITIONEVENT_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayEvents/WCMotionEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;

// <summary>
// Class which stores WorldCanvas position event information.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "WCPositionEvent" is a contraction and concatenation of "WorldCanvas",
// "Position" and "Event", and describes position events occuring on
// WorldCanvases.
// </etymology>

// <prerequisite>
// <li> <linkto class=WorldCanvasEvent>WorldCanvasEvent</linkto>
// </prerequisite>

// <synopsis>
// This class adds to the information stored in the <linkto
// class=WCMotionEvent>WCMotionEvent</linkto> class.  It adds
// information describing key that has been pressed or released, and
// the state of that key.
// </synopsis>

// <example>
// </example>

// <motivation>
// A compact way of passing passing position event information around
// the WorldCanvas-oriented display classes was needed, with a
// functional but tight and efficient interface.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/18">
// None.
// </todo>

	class WCPositionEvent : public WCMotionEvent {

	public:

		// Constructor taking a pointer to the WorldCanvas for which the
		// event is valid, the key which is pressed and its state
		// (ie. pressed or released), the state of the keyboard and pointer
		// modifiers, and the pixel, linear and world coordinates of the
		// event.
		WCPositionEvent(WorldCanvas * wc,
		                const Display::KeySym &key,
		                const Bool &keyState,
		                const uInt &modifiers,
		                const Int &pixX, const Int &pixY,
		                const Double &linX, const Double &linY,
		                const Vector<Double> &world);

		// Destructor.
		virtual ~WCPositionEvent();

		// Which key was pressed or released?
		virtual Display::KeySym key() const {
			return itsKey;
		}

		// What state is the key in: <src>True</src> for pressed,
		// <src>False</src> for released.
		virtual Bool keystate() const {
			return itsKeyState;
		}

	protected:

		// (Required) default constructor.
		WCPositionEvent();

		// (Required) copy constructor.
		WCPositionEvent(const WCPositionEvent &other);

		// (Required) copy assignment.
		WCPositionEvent &operator=(const WCPositionEvent &other);

	private:

		// Store the key that was pressed or released here.
		Display::KeySym itsKey;

		// Store the state of that key here.
		Bool itsKeyState;

	};


} //# NAMESPACE CASA - END

#endif


