//# WCRefreshEvent.h: class which stores WorldCanvas refresh event information
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

#ifndef TRIALDISPLAY_WCREFRESHEVENT_H
#define TRIALDISPLAY_WCREFRESHEVENT_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <display/DisplayEvents/WorldCanvasEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvas;

// <summary>
// Class which stores WorldCanvas refresh event information.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "WCRefreshEvent" is a contraction and concatenation of "WorldCanvas",
// "Refresh" and "Event", and describes refresh events occuring on 
// WorldCanvases.
// </etymology>

// <prerequisite>
// <li> <linkto class=WorldCanvasEvent>WorldCanvasEvent</linkto>
// </prerequisite>

// <synopsis> 
// This class adds to the information stored in the <linkto
// class=WorldCanvasEvent>WorldCanvasEvent</linkto> class.  It adds
// information describing the reason a refresh event has occurred.  An
// object of this class is generated and distributed to all registered
// WCRefreshEvent handlers whenever a refresh is explicitly or
// implicitly generated on a WorldCanvas.
//
// The reason field can be used to minimize the computation required
// to redraw the screen, or it can be ignored, forcing all elements to
// be redrawn.  The reason for the refresh can be one of the
// following:
//
// <dd> Display::UserCommand 
// <dt> The refresh event was caused by the user explicitly calling
// the <linkto class=WorldCanvas>WorldCanvas</linkto> refresh
// function.
//
// <dd> Display::ColorTableChange
// <dt> The refresh event was caused by a change in the distribution
// of colors on the <linkto class=PixelCanvas>PixelCanvas</linkto>.
// Graphical elements that are dependent on colormaps need to be
// re-drawn.  Cached display lists that include colormap-dependent
// graphical elements must be rebuilt.
//
// <dd> Display::ColormapChange
// <dt> This is a less severe version of the above refresh reason
// (Display::ColorTableChange), and means that a single Colormap has
// been modified, but that the overall distribution of colors remains
// unchanged, ie. other Colormaps in this WorldCanvas' ColorTable are
// unaffected. 
//
// <dd> Display::PixelCoordinateChange
// <dt> The <linkto class=PixelCanvas>PixelCanvas</linkto> has changed
// size.  Usually this means the graphics drawn must be translated to
// a new position (recentered) which can be done by translating all
// primitives and display lists.
//
// <dd> Display::LinearCoordinateChange
// <dt> The linear coordinate system which is overlaid on the
// PixelCanvas (normally by a <linkto
// class=WorldCanvas>WorldCanvas</linkto> has changed.
//
// <dd> Display::WorldCoordinateChange
// <dt> The world coordinate system which is overlaid on the 
// PixelCanvas (normally by a <linkto
// class=WorldCanvas>WorldCanvas</linkto> has changed.
//
// <dd> Display::BackCopiedToFront
// <dt> The back buffer has been written to the screen.  Only
// transient graphics drawers should be interested in this refresh
// reason.  Any graphics which are required to be visible over
// everything else on the PixelCanvas (or WorldCanvas) should catch 
// and respond to this refresh reason.  Examples include interactively 
// constructed regions, eg. a zooming box.
// </synopsis>

// <example>
// </example>

// <motivation> 
// A compact way of passing refresh event information around the
// display classes was needed, with a functional but tight and
// efficient interface.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/15">
// None.
// </todo>

class WCRefreshEvent : public WorldCanvasEvent {

 public:

  // Constructor taking a pointer to the WorldCanvas for which the
  // event is valid, and the reason for the refresh.
  WCRefreshEvent(WorldCanvas *wc, const Display::RefreshReason &reason);
  
  // Destructor.
  virtual ~WCRefreshEvent();

  // Why did the refresh occur?
  virtual Display::RefreshReason reason() const 
    { return itsRefreshReason; }

 protected:

  // (Required) default constructor.
  WCRefreshEvent();

  // (Required) copy constructor.
  WCRefreshEvent(const WCRefreshEvent &other);

  // (Required) copy assignment.
  WCRefreshEvent &operator=(const WCRefreshEvent &other);

 private:

  // Store the reason for the refresh here at construction.
  Display::RefreshReason itsRefreshReason;

};


} //# NAMESPACE CASA - END

#endif


