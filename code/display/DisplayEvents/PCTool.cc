//# PCTool.cc: base class for PixelCanvas event-based tools
//# Copyright (C) 1999,2000,2001
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

#include <casa/BasicSL/String.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/PCTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	PCToolPosEH::PCToolPosEH(PCTool *tool) :
		itsTool(tool) { }
	void PCToolPosEH::operator()(const PCPositionEvent& ev) {
		(*itsTool)(ev);
	}
	PCToolMotEH::PCToolMotEH(PCTool *tool) :
		itsTool(tool) { }
	void PCToolMotEH::operator()(const PCMotionEvent& ev) {
		(*itsTool)(ev);
	}
	PCToolRefEH::PCToolRefEH(PCTool *tool) :
		itsTool(tool) { }
	void PCToolRefEH::operator()(const PCRefreshEvent& ev) {
		(*itsTool)(ev);
	}

// Constructor.
	PCTool::PCTool(PixelCanvas *pcanvas, Display::KeySym keysym) :
		itsPixelCanvas(pcanvas),
		itsKeySym(keysym),
		itsEventHandlersRegistered(False) {
		try {
			itsKeyModifier = Display::keyModifierFromKeySym(itsKeySym);
		} catch (AipsError x) {
			if (&x) {
				itsKeyModifier = (Display::KeyModifier)0;
			}
		}
		itsPositionEH = new PCToolPosEH(this);
		itsMotionEH = new PCToolMotEH(this);
		itsRefreshEH = new PCToolRefEH(this);
		enable();
	}

// Destructor.
	PCTool::~PCTool() {
		disable();
		if (itsRefreshEH) {
			delete itsRefreshEH;
		}
		if (itsMotionEH) {
			delete itsMotionEH;
		}
		if (itsPositionEH) {
			delete itsPositionEH;
		}
	}

// Switch the tool on/off.
	void PCTool::enable() {
		if (!itsEventHandlersRegistered) {
			itsEventHandlersRegistered = True;
			itsPixelCanvas->addPositionEventHandler(*itsPositionEH);
			itsPixelCanvas->addMotionEventHandler(*itsMotionEH);
			itsPixelCanvas->addRefreshEventHandler(*itsRefreshEH);
		}
	}
	void PCTool::disable() {
		if (itsEventHandlersRegistered) {
			itsEventHandlersRegistered = False;
			itsPixelCanvas->removePositionEventHandler(*itsPositionEH);
			itsPixelCanvas->removeMotionEventHandler(*itsMotionEH);
			itsPixelCanvas->removeRefreshEventHandler(*itsRefreshEH);
		}
	}

// Required operators for event handling.
	void PCTool::operator()(const PCPositionEvent &ev) {
		if (ev.key() != itsKeySym || getKey( ) == Display::K_None) {
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
	void PCTool::operator()(const PCMotionEvent &ev) {
		moved(ev);
	}
	void PCTool::operator()(const PCRefreshEvent &ev) {
		refresh(ev);
	}

// Functions called by the local event handling operators.
	void PCTool::keyPressed(const PCPositionEvent &) {
	}
	void PCTool::keyReleased(const PCPositionEvent &) {
	}
	void PCTool::otherKeyPressed(const PCPositionEvent &) {
	}
	void PCTool::otherKeyReleased(const PCPositionEvent &) {
	}
	void PCTool::moved(const PCMotionEvent &) {
	}
	void PCTool::refresh(const PCRefreshEvent &) {
	}

	void PCTool::setKey(const Display::KeySym &keysym) {
		itsKeySym = keysym;
		try {
			itsKeyModifier = Display::keyModifierFromKeySym(itsKeySym);
		} catch (AipsError x) {
			if (&x) {
				itsKeyModifier = (Display::KeyModifier)0;
			}
		}
	}

	Bool PCTool::keyPresentlyDown(const PCMotionEvent &ev) {
		return (ev.modifiers() & itsKeyModifier);
	}

} //# NAMESPACE CASA - END

