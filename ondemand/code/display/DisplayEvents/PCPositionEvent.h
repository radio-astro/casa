//# PCPositionEvent.h: class which stores PixelCanvas position event info
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

#ifndef TRIALDISPLAY_PCPOSITIONEVENT_H
#define TRIALDISPLAY_PCPOSITIONEVENT_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayEvents/PCMotionEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;

// <summary>
// Class which stores PixelCanvas position event information.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "PCPositionEvent" is a contraction and concatenation of "PixelCanvas",
// "Position" and "Event", and describes position events occuring on
// PixelCanvases.
// </etymology>

// <prerequisite>
// <li> <linkto class=PixelCanvasEvent>PixelCanvasEvent</linkto>
// </prerequisite>

// <synopsis>
// This class adds to the information stored in the <linkto
// class=PCMotionEvent>PCMotionEvent</linkto> class.  It adds
// information describing key that has been pressed or released, and
// the state of that key.
// </synopsis>

// <example>
// </example>

// <motivation>
// A compact way of passing passing position event information around
// the PixelCanvas-oriented display classes was needed, with a
// functional but tight and efficient interface.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/15">
// None.
// </todo>

	class PCPositionEvent : public PCMotionEvent {

	public:

		// Constructor taking a pointer to the PixelCanvas for which the
		// event is valid, the symbol of the key, the state of the key,
		// the position of the pointer, and the input "modifiers".
		// User Constructor
		PCPositionEvent(PixelCanvas *pc, const Display::KeySym &key,
		                const Bool &keystate, const Int &x, const Int &y,
		                const uInt &modifiers);

		// Destructor.
		virtual ~PCPositionEvent();

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
		PCPositionEvent();

		// (Required) copy constructor.
		PCPositionEvent(const PCPositionEvent &other);

		// (Required) copy assignment.
		PCPositionEvent &operator=(const PCPositionEvent &other);

	private:

		// Store the key that was pressed or released here.
		Display::KeySym itsKey;

		// Store the state of that key here.
		Bool itsKeyState;

	};


} //# NAMESPACE CASA - END

#endif


