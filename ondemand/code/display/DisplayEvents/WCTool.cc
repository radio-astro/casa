//# WCTool.cc: base class for WorldCanvas event-based tools
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

#include <display/Display/WorldCanvas.h>
#include <display/DisplayEvents/WCTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	WCToolPosEH::WCToolPosEH(WCTool *tool) :
		itsTool(tool) { }
	void WCToolPosEH::operator()(const WCPositionEvent& ev) {
		(*itsTool)(ev);
	}
	WCToolMotEH::WCToolMotEH(WCTool *tool) :
		itsTool(tool) { }
	void WCToolMotEH::operator()(const WCMotionEvent& ev) {
		(*itsTool)(ev);
	}
	WCToolRefEH::WCToolRefEH(WCTool *tool) :
		itsTool(tool) { }
	void WCToolRefEH::operator()(const WCRefreshEvent& ev) {
		(*itsTool)(ev);
	}

	WCTool::WCTool(WorldCanvas *wcanvas,
	               const Display::KeySym &keysym) :
		DisplayTool(keysym),
		itsWorldCanvas(wcanvas),
		itsPixelCanvas(itsWorldCanvas->pixelCanvas()),
		itsEventHandlersRegistered(False) {
		itsPositionEH = new WCToolPosEH(this);
		itsMotionEH = new WCToolMotEH(this);
		itsRefreshEH = new WCToolRefEH(this);
		enable();
	}

	WCTool::~WCTool() {
		disable();
		delete itsRefreshEH;
		delete itsMotionEH;
		delete itsPositionEH;
	}

	void WCTool::enable() {
		if (!itsEventHandlersRegistered) {
			itsEventHandlersRegistered = True;
			itsWorldCanvas->addPositionEventHandler(*itsPositionEH);
			itsWorldCanvas->addMotionEventHandler(*itsMotionEH);
			itsWorldCanvas->addRefreshEventHandler(*itsRefreshEH);
		}
	}

	void WCTool::disable() {
		if (itsEventHandlersRegistered) {
			itsEventHandlersRegistered = False;
			itsWorldCanvas->removePositionEventHandler(*itsPositionEH);
			itsWorldCanvas->removeMotionEventHandler(*itsMotionEH);
			itsWorldCanvas->removeRefreshEventHandler(*itsRefreshEH);
		}
	}

	void WCTool::operator()(const WCPositionEvent &ev) {
		if (ev.key() != getKey() || getKey( ) == Display::K_None) {
			if (ev.keystate()) {
				otherKeyPressed(ev);
			} else {
				otherKeyReleased(ev);
			}
		} else {
			if (ev.keystate()) {
				keyPressed(ev);
			} else {
				keyReleased(ev);
			}
		}
	}

	void WCTool::operator()(const WCMotionEvent &ev) {
		static viewer::region::region_list_type empty;
		moved(ev,empty);
	}

	void WCTool::operator()(const WCRefreshEvent &ev) {
		refresh(ev);
	}

	void WCTool::keyPressed(const WCPositionEvent &) {
	}

	void WCTool::keyReleased(const WCPositionEvent &) {
	}

	void WCTool::otherKeyPressed(const WCPositionEvent &) {
	}

	void WCTool::otherKeyReleased(const WCPositionEvent &) {
	}

	void WCTool::moved(const WCMotionEvent &, const viewer::region::region_list_type &) {
	}

	void WCTool::refresh(const WCRefreshEvent &) {
	}

// (Required) default constructor.
	WCTool::WCTool() :
		DisplayTool(),
		itsWorldCanvas(0),
		itsPixelCanvas(0),
		itsEventHandlersRegistered(False),
		itsPositionEH(0),
		itsMotionEH(0),
		itsRefreshEH(0) {
	}

// (Required) copy constructor.
	WCTool::WCTool(const WCTool &other) :
		DisplayTool(other) {
		// NOT YET IMPLEMENTED
	}

// (Required) copy assignment.
	WCTool &WCTool::operator=(const WCTool &other) {
		if (this != &other) {
			DisplayTool::operator=(other);
			// NOT YET IMPLEMENTED
		}
		return *this;
	}



} //# NAMESPACE CASA - END

