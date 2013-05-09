//# DisplayEvents.h: The DisplayEvents module - display event handling
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

#ifndef TRIALDISPLAY_DISPLAYEVENTS_H
#define TRIALDISPLAY_DISPLAYEVENTS_H

#include <display/DisplayEvents/WCRTZoomer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <module>
//
// <summary>
// A module providing event handling for the DisplayCanvases
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="None yet" date="yyyy/mm/dd" demos="">
// </reviewed>
//
// <prerequisite>
// <li> DisplayCanvas
// </prerequisite>
//
// <etymology>
// The module provides various classes that are responsible for
// "handling" "events"
// </etymology>
//
// <synopsis>
// This module contains classes which are used to handle
// position, motion and refresh events that occur on PixelCanvases and
// WorldCanvases.  Position events occur when the user presses or
// releases a keyboard key or a mouse button when the window input
// focus is over a PixelCanvas or WorldCanvas.  Motion events occur
// when the user moves the mouse over a PixelCanvas or a WorldCanvas.
// Refresh events occur for a variety of reasons, and arise when
// PixelCanvases or WorldCanvases need to redraw their contents.
//
// The lowest level of event classes are those which describe the
// various events on the PixelCanvas.  These comprise PCPositionEvent,
// PCMotionEvent and PCRefreshEvent.  All of the information
// pertaining to a particular event will be stored in one of these
// classes.  To receive these type of events, the programmer must
// register a class derived from PCPositionEH, PCMotionEH or
// PCRefreshEH (where the "EH" stands for Event Handler) with the
// appropriate PixelCanvas.  Then any events of that type (Position,
// Motion or Refresh) will be distributed to the class that the
// programmer registered.  The WorldCanvas has equivalent event
// storage classes in WCPositionEvent, WCMotionEvent and
// WCRefreshEvent, and equivalent event handling classes in
// WCPositionEH, WCMotionEH and WCRefreshEH.
//
// It is important to note that none of the event handlers "consume"
// events.  This means that all PixelCanvas events occuring on a
// particular PixelCanvas wil be distributed to all the event handlers
// registered on that PixelCanvas.  Likewise, all WorldCanvas events
// for a particular WorldCanvas are distributed to all WorldCanvas
// event handlers registered on that WorldCanvas.
//
// It is also critical for the programmer to be aware that in general,
// the WorldCanvas registers one instance of each of the PCPositionEH,
// PCMotionEH and PCRefreshEH type handler classes for its own use.
// These event handlers are responsible for constructing WorldCanvas
// event information objects (WCPositionEvent, WCMotionEvent and
// WCRefreshEvent) from the corresponding PixelCanvas events, and then
// call the appropriate handlers on the WorldCanvas.  So the
// programmer must be careful that they handle each event once and
// only once - either on the PixelCanvas, or on the WorldCanvas, but
// not on both.  There will be rare instances where handling events on
// both the WorldCanvas and its underlying PixelCanvas is necessary.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Any interactive display library must provide methods for the
// programmer to respond to user input.  That is the reason for this
// module.
// </motivation>
//
// <todo asof="1999/02/10">
//   <li> None known
// </todo>
//
// </module>


} //# NAMESPACE CASA - END

#endif

