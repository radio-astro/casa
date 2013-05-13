//# PCMotionEvent.h: class which stores PixelCanvas motion event information
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

#ifndef TRIALDISPLAY_PCMOTIONEVENT_H
#define TRIALDISPLAY_PCMOTIONEVENT_H

#include <casa/aips.h>
#include <display/DisplayEvents/PixelCanvasEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class PixelCanvas;

// <summary>
// Class which stores PixelCanvas motion event information.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "PCMotionEvent" is a contraction and concatenation of "PixelCanvas",
// "Motion" and "Event", and describes motion events occuring on
// PixelCanvases.
// </etymology>

// <prerequisite>
// <li> <linkto class=PixelCanvasEvent>PixelCanvasEvent</linkto>
// </prerequisite>

// <synopsis>
// This class adds to the information stored in the <linkto
// class=PixelCanvasEvent>PixelCanvasEvent</linkto> class.  It adds
// information describing the current position of the mouse or
// pointing device, and the state of the keyboard modifiers (including
// the mouse buttons).
// </synopsis>

// <example>
// </example>

// <motivation>
// A compact way of passing motion event information around the
// PixelCanvas-oriented display classes was needed, with a functional
// but tight and efficient interface.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/15">
// None.
// </todo>

	class PCMotionEvent : public PixelCanvasEvent {

	public:

		// Constructor taking a pointer to the PixelCanvas for which the
		// event is valid, the position of the event, and the keyboard and
		// pointer modifiers.
		PCMotionEvent(PixelCanvas *pc, const Int &x, const Int &y,
		              const uInt &modifiers);

		// Destructor.
		virtual ~PCMotionEvent();

		// The x and y pixel position of the pointer when the event occured.
		// <group>
		virtual Int x() const {
			return itsX;
		}
		virtual Int y() const {
			return itsY;
		}
		// </group>

		// Return the state of the "modifiers": this is made up of mask
		// bits referring to various keys on the keyboard (eg. Control,
		// Shift, etc.) and the mouse buttons.
		virtual uInt modifiers() const {
			return itsModifiers;
		}

	protected:

		// (Required) default constructor.
		PCMotionEvent();

		// (Required) copy constructor.
		PCMotionEvent(const PCMotionEvent &other);

		// (Required) copy assignment.
		PCMotionEvent &operator=(const PCMotionEvent &other);

	private:

		// Store the position of the event here.
		Int itsX, itsY;

		// Store the button and keyboard modifier masks here.
		uInt itsModifiers;

	};


} //# NAMESPACE CASA - END

#endif


