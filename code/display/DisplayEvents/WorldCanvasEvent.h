//# WorldCanvasEvent.h: base class for event information for WorldCanvases
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

#ifndef TRIALDISPLAY_WORLDCANVASEVENT_H
#define TRIALDISPLAY_WORLDCANVASEVENT_H

#include <casa/aips.h>
#include <display/DisplayEvents/DisplayEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvas;

// <summary>
// Base class describing event information for WorldCanvases.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "WorldCanvasEvents" describes "Events" (ie. things which happen at a 
// measurable time) which occur on WorldCanvases.
// </etymology>

// <prerequisite>
// <li> <linkto class=DisplayEvent>DisplayEvent</linkto>
// </prerequisite>

// <synopsis>
// This class adds to the information stored in the <linkto
// class=DisplayEvent>DisplayEvent</linkto> class.  It adds
// information specific to events occuring on WorldCanvases, viz.
// a pointer to the WorldCanvas itself.
// </synopsis>

// <example>
// </example>

// <motivation>
// It is desirable to locate in a single place the information which
// is common to all events occuring on WorldCanvases.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/15">
// None.
// </todo>

class WorldCanvasEvent : public DisplayEvent {

 public:

  // Constructor, taking a pointer to a WorldCanvas.
  WorldCanvasEvent(WorldCanvas *wc);

  // Destructor.
  virtual ~WorldCanvasEvent();

  // Return a pointer to the WorldCanvas on which the event occurred.
  virtual WorldCanvas *worldCanvas() const
    { return itsWorldCanvas; }

 protected:

  // (Required) default constructor.
  WorldCanvasEvent();

  // (Required) copy constructor.
  WorldCanvasEvent(const WorldCanvasEvent &other);
  
  // (Required) copy assignment.
  WorldCanvasEvent &operator=(const WorldCanvasEvent &other);

 private:

  // Store the WorldCanvas of the event here at construction.
  WorldCanvas *itsWorldCanvas;

};


} //# NAMESPACE CASA - END

#endif
