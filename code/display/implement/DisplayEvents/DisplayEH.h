//# DisplayEH.h: WorldCanvas refresh event handler
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

#ifndef TRIALDISPLAY_DISPLAYEH_H
#define TRIALDISPLAY_DISPLAYEH_H

#include <casa/aips.h>
//# (Note: below is one of just 3 DL classes not in trialdisplay/...)
#include <display/DisplayEvents/DisplayEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// class for handling any type of DisplayEvent.
// </summary>
//
// <prerequisite>
// <li> <linkto class="DisplayEvent">DisplayEvent</linkto>
// <li> Understanding of Display library event-handling methodology
// </prerequisite>
//
// <etymology>
// DisplayEH : generic display event handler--A handler for any type of 
// display library event.
// </etymology>
//
// <synopsis>
// DisplayEH has an overridable callback that allows it to handle any type
// of base class DisplayEvent.  It is intended to allow different types
// of handlers (for different types of event) to reside on the same callback
// list.
// </synopsis>
//
// <motivation>
// Different types of events (and event-handling routines) are needed in the
// display library, and more are likely to be needed in future.  However,
// creating still more specialized handler lists in places like WC for
// each new type of event seems impractical.  WorldCanvas and PixelCanvas
// already maintain 3 such lists each.  This generic event handler
// interface will allow more general use of existing event dispatching lists.
//
// The immediate use for DisplayEH will be to enable more general
// exchange of control messages between objects already attached 
// (directly or indirectly) to WorldCanvas as handlers, in particular
// between DisplayDatas and MultiWCTools.  WorldCanvas and WorldCanvasHolder
// will add this simple interface in order to distribute the new events;
// DisplayData will inherit it in order to handle them.  WCRefreshEH will
// also be altered to inherit from this class, and WorldCanvas's
// existing refresh event handler list will be altered to accept any
// DisplayEH (not just WCRefreshEH objects).
//
// To listen for an event on this new WC 'channel', an object derived
// from this interface would be attached to the WC's RefreshEHList
// (as the MultiWCTools are) or WCH's DD list (in the DD case).  The
// object would then implement handleEvent() to respond (only) to the
// events of interest.  A sender would create a new DisplayEvent type (or
// use an existing one for its intended purpose), and send the
// event by calling WC::handleEvent(event).
//
// Except for the accommodations mentioned above, the current 
// interface and functionality of WCRefreshEHs and all other
// existing event handlers is left as is.  Note especially that
// the older events (Position, Motion, Refresh) are still sent 
// using the old 'operator()' interface, which remains in place, at
// least for now.
//
// In the future, it may be useful to derive all existing handlers
// from DisplayEH, to consolidate to just one generic EH list each
// in PC, WC and WCH (and anywhere else event dispatching is needed),
// and to distinguish handler and event types only during event
// dispatching or processing.  Then any object which inherits this
// interface could be placed on any such dispatching list in order
// to respond to events of interest.
// </motivation>

class DisplayEH {

 public:

  // Classes that contain this interface (i.e., derive from it) can
  // override this method to implement actions for whatever types of
  // DisplayEvent they're interested in.
  // It was thought best at this point to move away from the old
  // 'operator()' style of handling interface, for clarity.

  virtual void handleEvent(DisplayEvent&) {  } 
  
  virtual ~DisplayEH() {  }

};


} //# NAMESPACE CASA - END

#endif



