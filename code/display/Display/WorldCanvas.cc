//# WorldCanvas.cc: class for drawing in world coordinates on the PixelCanvas
//# Copyright (C) 1993,1994,1995,1996,1997,1998,1999,2000,2001,2002,2003
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

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <display/Display/WCPGFilter.h>
#include <display/DisplayCanvas/WCCoordinateHandler.h>
#include <display/DisplayCanvas/WCSizeControlHandler.h>
#include <display/DisplayCanvas/DefaultWCSizeControlHandler.h>
#include <display/DisplayCanvas/DefaultWCCoordinateHandler.h>
#include <display/DisplayCanvas/WCDataScaleHandler.h>
#include <display/DisplayCanvas/WCLinearScaleHandler.h>
#include <display/DisplayCanvas/WCResampleHandler.h>
#include <display/DisplayCanvas/WCSimpleResampleHandler.h>
#include <display/Display/AttValBase.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>
#include <cpgplot.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <display/DisplayDatas/DisplayData.h>

namespace casa { //# NAMESPACE CASA - BEGIN

const String WorldCanvas::LEFT_MARGIN_SPACE_PG = "leftmarginspacepg";
const String WorldCanvas::RIGHT_MARGIN_SPACE_PG = "rightmarginspacepg";
const String WorldCanvas::BOTTOM_MARGIN_SPACE_PG = "bottommarginspacepg";
const String WorldCanvas::TOP_MARGIN_SPACE_PG = "topmarginspacepg";

// Constructor.
WorldCanvas::WorldCanvas(PixelCanvas * pc,
		Double xOrigin, Double yOrigin,
		Double xSize, Double ySize)
: itsCanvasXOffset(0),
  itsCanvasYOffset(0),
  itsCanvasXSize(pc->width()),
  itsCanvasYSize(pc->height()),
  itsFracXOffset(0),
  itsFracYOffset(0),
  itsFracXSize(1),
  itsFracYSize(1),
  itsCanvasDrawXOffset(0),
  itsCanvasDrawYOffset(0),
  itsCanvasDrawXSize(pc->width()),
  itsCanvasDrawYSize(pc->height()),
  itsLinXMin(0),
  itsLinYMin(0),
  itsLinXMax(1),
  itsLinYMax(1),
  itsLinXMinLimit(0),
  itsLinYMinLimit(0),
  itsLinXMaxLimit(1),
  itsLinYMaxLimit(1),
  itsDataMin(0),
  itsDataMax(1),
  itsComplexToRealMethod(Display::Magnitude),
  itsWorldBackgroundColor(pc->deviceBackgroundColor()),
  itsWorldForegroundColor(pc->deviceForegroundColor()),
  itsPixelCanvas(pc),
  itsPGFilter(0),
  itsHoldCount(0),
  itsRefreshHeld(False),
  itsHeldReason(Display::BackCopiedToFront),
  itsCoordinateSystem(0),
  itsGrabbing(False),
  images_(0,16),
  itsCSmaster(0) {
	setWorldCanvasPosition(xOrigin, yOrigin, xSize, ySize);
	ctorInit();
}

// Destructor.
WorldCanvas::~WorldCanvas() {
	itsCSmaster = 0;
	itsHoldCount = 0;
	if (itsCoordinateSystem)
		delete itsCoordinateSystem;
	itsCoordinateSystem = 0;
	itsPixelCanvas->removeRefreshEventHandler(*this);
	itsPixelCanvas->removeMotionEventHandler(*this);
	itsPixelCanvas->removePositionEventHandler(*this);

	delete itsMEHListIter;
	itsMEHListIter = 0;
	delete itsPEHListIter;
	itsPEHListIter = 0;
	delete itsREHListIter;
	itsREHListIter = 0;

	if (itsOwnDataScaleHandler) {
		delete itsDataScaleHandler;
		itsDataScaleHandler = 0;
	}
	if (itsOwnResampleHandler) {
		delete itsResampleHandler;
		itsResampleHandler = 0;
	}
	if (itsOwnSizeControlHandler) {
		delete itsSizeControlHandler;
		itsSizeControlHandler = 0;
	}
	if (itsOwnCoordinateHandler) {
		delete itsCoordinateHandler;
		itsCoordinateHandler = 0;
	}
	delete itsPGFilter;
	itsPGFilter = 0;

	clearColormapChangeCache();
}

// Add event handlers.
void WorldCanvas::addRefreshEventHandler(DisplayEH &eh) {
	itsREHListIter->toEnd();
	itsREHListIter->addRight(&eh);
}
void WorldCanvas::addMotionEventHandler(WCMotionEH &eh) {
	itsMEHListIter->toEnd();
	itsMEHListIter->addRight(&eh);
}
void WorldCanvas::addPositionEventHandler(WCPositionEH &eh) {
	itsPEHListIter->toEnd();
	itsPEHListIter->addRight(&eh);
}

// Remove event handlers.
void WorldCanvas::removeRefreshEventHandler(const DisplayEH &eh) {
	itsREHListIter->toStart();
	while (!itsREHListIter->atEnd()) {
		if (itsREHListIter->getRight() == &eh) {
			itsREHListIter->removeRight();
			break;
		}
		(*itsREHListIter)++;
	}
}
void WorldCanvas::removeMotionEventHandler(const WCMotionEH &eh) {
	itsMEHListIter->toStart();
	while (!itsMEHListIter->atEnd()) {
		if (itsMEHListIter->getRight() == &eh) {
			itsMEHListIter->removeRight();
			break;
		}
		(*itsMEHListIter)++;
	}
}
void WorldCanvas::removePositionEventHandler(const WCPositionEH &eh) {
	itsPEHListIter->toStart();
	while (!itsPEHListIter->atEnd()) {
		if (itsPEHListIter->getRight() == &eh) {
			itsPEHListIter->removeRight();
			break;
		}
		(*itsPEHListIter)++;
	}
}

// Call event handlers.
void WorldCanvas::callRefreshEventHandlers(const WCRefreshEvent &ev) {

	// check if we are allowed to refresh:
	if (!(itsPixelCanvas->refreshAllowed())) {
		return;
	}
	itsREHListIter->toStart();
	while (!itsREHListIter->atEnd()) {
		// This list now contains generic DisplayEHs as well (1/02).
		// Only the true WCRefreshEHs on this list can/should handle
		// WCRefreshEvents through the old-style 'operator()' interface.
		WCRefreshEH* refEH=dynamic_cast<WCRefreshEH*>(itsREHListIter->getRight());
		if(refEH != 0) (*refEH)(ev);
		(*itsREHListIter)++;
	}
}
void WorldCanvas::callMotionEventHandlers(const WCMotionEvent &ev) {
	itsMEHListIter->toStart();
	while (!itsMEHListIter->atEnd()) {
		(*(itsMEHListIter->getRight()))(ev);
		(*itsMEHListIter)++;
	}
}
void WorldCanvas::callPositionEventHandlers(const WCPositionEvent &ev) {
	itsPEHListIter->toStart();
	while (!itsPEHListIter->atEnd()) {
		(*(itsPEHListIter->getRight()))(ev);
		(*itsPEHListIter)++;
	}
}

// Handle new-style (1/02) generic events sent to the WC.
// For now, simply dispatch the events to all the registered handlers
// (which ride, along with true WCRefreshEHs, on itsRefreshEHList).
// Filtering here, based on actual handler and event types, could
// be implemented in the future, if found to be desirable.

void WorldCanvas::handleEvent(DisplayEvent& ev) {
	ConstListIter<DisplayEH*> ehs(&itsRefreshEHList);
	// avoid itsREHListIter because of recursion issues.
	// (It might be best to can the member ListIters altogether...)
	for(ehs.toStart(); !ehs.atEnd(); ehs++) ehs.getRight()->handleEvent(ev);
}


// Handle implicit events.

void WorldCanvas::operator()(const PCRefreshEvent &pev) {
	refresh(pev.reason());
}


void WorldCanvas::operator()(const PCMotionEvent &pev) {
	Int x=pev.x(), y=pev.y();
	static const uInt mouseButtons = Display::KM_Pointer_Button_1 |
			Display::KM_Pointer_Button_2 |
			Display::KM_Pointer_Button_3 |
			Display::KM_Pointer_Button_4 |
			Display::KM_Pointer_Button_5 ;

	uInt buttonsDown = pev.modifiers() & mouseButtons;
	Bool distrib=False;	// Do we distribute this event to this WC's handlers?

			if(buttonsDown == 0) {	// no mouse buttons pressed
				itsGrabbing = False;	// insurance--should already be False.
				distrib = inWC(x,y);
			}	// who gets event depends on location...
			else distrib = itsGrabbing;	// ..or who has a grab, if buttons are down.

			// Note: WC event handlers can also use inWC(), inDrawArea() and
			// inPC() if they need to know if event is within some boundary.

			if(distrib) {

				// Distribute the event to this WC's event handlers.

				Vector<Double> pix(2), lin(2);
				pix(0) = x;
				pix(1) = y;
				pixToLin(lin, pix);		// linear coordinates
				Vector<Double> world(itsCoordinateHandler->nWorldAxes());
				linToWorld(world, lin);	// world coordinates
				WCMotionEvent wev(this, pev.modifiers(), x,y, lin(0),lin(1), world);
				callMotionEventHandlers(wev);
			}
}


void WorldCanvas::operator()(const PCPositionEvent &pev) {
	Int x=pev.x(), y=pev.y();
	Display::KeySym key=pev.key();
	Bool pressed = pev.keystate();
	static const uInt mouseButtons = Display::KM_Pointer_Button_1 |
			Display::KM_Pointer_Button_2 |
			Display::KM_Pointer_Button_3 |
			Display::KM_Pointer_Button_4 |
			Display::KM_Pointer_Button_5 ;

	uInt priorButtons = pev.modifiers() & mouseButtons;
	// NB: this is the state of mouse buttons _before_ the press/release.

	Bool kbdEvent = False;
	uInt eventButton = 0;
	switch(key) {
	case Display::K_Pointer_Button1:
		eventButton = Display::KM_Pointer_Button_1;
		break;
	case Display::K_Pointer_Button2:
		eventButton = Display::KM_Pointer_Button_2;
		break;
	case Display::K_Pointer_Button3:
		eventButton = Display::KM_Pointer_Button_3;
		break;
	case Display::K_Pointer_Button4:
		eventButton = Display::KM_Pointer_Button_4;
		break;
	case Display::K_Pointer_Button5:
		eventButton = Display::KM_Pointer_Button_5;
		break;
	default:
		kbdEvent = True;
	}

	Bool distrib;	// Do we distribute this event to this WC's handlers?

	if(kbdEvent) {

		switch(key) {
		case Display::K_Left:
		case Display::K_Right:
		case Display::K_Up:
		case Display::K_Down:
		case Display::K_Home:
		case Display::K_End:
		case Display::K_Page_Up:
		case Display::K_Page_Down:
			distrib = inWC(0,0);
			break;
			// (The above is something of a kludge; perhaps most
			// mouse-position-independent keyboard presses should be handled
			// by PCTools rather than [M]WCTools).  For now at least, we want
			// to accept at least the above keys (possibly others) on [just]
			// one of the PC's main WCs (it doesn't matter which), even if
			// the pointer is _not over_ the PixelCanvas (mouse position
			// is irrelevant for above keys).  Note that the PC _does_ have
			// kbd focus, since the keyboard event was generated.
			// (I rather wish PC had separated keyboard vs. mouse click
			// events itself...).
			// The lower-left WC will include PC position (0,0).
			//
			// The arrow/page keys are used for panning.
			// The escape key is used to reset (erase) mouse tools.

		case Display::K_Escape:
			distrib = True;
			break;
			// (Remove if you really need to make the 'escape' key (which
			// resets mouse tools) specific to a particular WC...).

		default:
			distrib = inWC(x,y);
		}
	}
	// Most keyboard events are delivered only to a WC the mouse
	// pointer is over (may need to be changed as for above keys
	// eventually).


	else {
		// mouse click events
		if(!itsGrabbing) {
			if(inWC(x,y) && priorButtons == 0) {
				// It _is_ for this WC
				// (no priorButtons means no prior grab somewhere else).
				distrib = True;
				if(pressed) itsGrabbing = True;
			}	// start grab.
			else distrib = False;
		} else {
			distrib = True;	// distribute any mouse event when grab is on.
			if(!pressed) {	// button release event
				uInt currentButtons = (priorButtons & ~eventButton);
				// button state after this button was released.
				if(currentButtons == 0) itsGrabbing = False;
			}
		}
	}
	// Last button was released: end grab.

	if(distrib) {

		// Distribute the event to this WC's event handlers.

		Vector<Double> pix(2), lin(2);
		pix(0) = x;
		pix(1) = y;
		pixToLin(lin, pix);		// linear coordinates
		Vector<Double> world(itsCoordinateHandler->nWorldAxes());
		linToWorld(world, lin);	// world coordinates
		WCPositionEvent wev(this, key, pressed, pev.modifiers(),
				x,y, lin(0),lin(1), world);
		callPositionEventHandlers(wev);
	}
}

// Refresh the WorldCanvas.
void WorldCanvas::refresh(const Display::RefreshReason &reason,
		const Bool & /*explicitrequest*/) {

	if(reason!=Display::BackCopiedToFront &&
			reason!=Display::ColorTableChange &&
			reason!=Display::ColormapChange)  clearColormapChangeCache();
	// (Cleans up this cache and prevents inappropriate reuse).

	// check if we are held, and if so, store the reason for this refresh

	if (itsHoldCount) {
		// store only first reason (except overwrite with
		// first call for 'complete' refresh)
		if (!itsRefreshHeld || itsHeldReason == Display::BackCopiedToFront) {
			itsHeldReason = reason;
		}
		itsRefreshHeld = True;

	} else if (refreshAllowed()) {
		if (reason == Display::BackCopiedToFront) {
			WCRefreshEvent wev(this, reason);
			callRefreshEventHandlers(wev);
		} else {
			// check that we are in the right place on the PixelCanvas
			updateCanvasSizesOffsets();
			// set the state of the world canvas
			itsSizeControlHandler->executeSizeControl(this);
			// acquire a PGPLOT device  (dk note: needs above stmt first)
			acquirePGPLOTdevice();
			// set the correct drawing buffer
			setDrawBuffer(Display::BackBuffer);
			// refresh all the DDs
			WCRefreshEvent wev(this, reason);
			callRefreshEventHandlers(wev);
			copyBackBufferToFrontBuffer();
			setDrawBuffer(Display::FrontBuffer);
			// refresh PC
			itsPixelCanvas->callRefreshEventHandlers(Display::BackCopiedToFront);
			// release the PGPLOT device
			releasePGPLOTdevice();
		}
	}
}

// Is a refresh allowed?
Bool WorldCanvas::refreshAllowed() {
	return itsPixelCanvas->refreshAllowed();
}

// Hold/release of refresh events.
void WorldCanvas::hold() {
	itsHoldCount++;
}

void WorldCanvas::release() {
	itsHoldCount--;
	if (itsHoldCount <= 0) {
		itsHoldCount = 0;
		if (itsRefreshHeld) {
			refresh(itsHeldReason);
		}
		itsRefreshHeld = False;
	}
}

// Set various handlers.
void WorldCanvas::setCoordinateHandler(WCCoordinateHandler *handler) {
	if (!handler) {
		if (!itsOwnCoordinateHandler) {
			itsCoordinateHandler = new DefaultWCCoordinateHandler;
			itsOwnCoordinateHandler = True;
		}
	} else {
		if (itsOwnCoordinateHandler) {
			delete itsCoordinateHandler;
			itsOwnCoordinateHandler = False;
		}
		itsCoordinateHandler = handler;
	}
}
void WorldCanvas::setSizeControlHandler(WCSizeControlHandler *handler) {
	if (!handler) {
		if (!itsOwnSizeControlHandler) {
			itsSizeControlHandler = new DefaultWCSizeControlHandler;
			itsOwnSizeControlHandler = True;
		}
	} else {
		if (itsOwnSizeControlHandler) {
			delete itsSizeControlHandler;
			itsOwnSizeControlHandler = False;
		}
		itsSizeControlHandler = handler;
	}
}
void WorldCanvas::setResampleHandler(WCResampleHandler *handler) {
	if (!handler) {
		if (!itsOwnResampleHandler) {
			itsResampleHandler = new WCSimpleResampleHandler(Interpolate2D::NEAREST);
			itsOwnResampleHandler = True;
		}
	} else {
		if (itsOwnResampleHandler) {
			delete itsResampleHandler;
			itsOwnResampleHandler = False;
		}
		itsResampleHandler = handler;
	}
}
void WorldCanvas::setDataScaleHandler(WCDataScaleHandler *handler) {
	if (!handler) {
		if (!itsOwnDataScaleHandler) {
			itsDataScaleHandler = new WCLinearScaleHandler;
			itsOwnDataScaleHandler = True;
		}
	} else {
		if (itsOwnDataScaleHandler) {
			delete itsDataScaleHandler;
			itsOwnDataScaleHandler = False;
		}
		itsDataScaleHandler = handler;
	}
}

// Set the location of the WorldCanvas.
void WorldCanvas::setWorldCanvasPosition(Double fracXOffset,
		Double fracYOffset,
		Double fracXSize, Double fracYSize) {
	itsFracXOffset = max(0., min(1., fracXOffset));
	itsFracYOffset = max(0., min(1., fracYOffset));
	itsFracXSize = max(0., min(1.-itsFracXOffset, fracXSize));
	itsFracYSize = max(0., min(1.-itsFracYOffset, fracYSize));

	updateCanvasSizesOffsets();
}

// Pixel, linear and world coordinate transformation routines.
Bool WorldCanvas::pixToLin(Vector<Double> &lin, const Vector<Double> &pix) {
	if ( Int(itsCanvasDrawXSize * (itsLinXMax - itsLinXMin)+0.5) == 0 ||
			Int(itsCanvasDrawYSize * (itsLinYMax - itsLinYMin)+0.5) == 0 ) {
		return False;
	}
	lin(0) = (pix(0) - itsCanvasDrawXOffset - itsCanvasXOffset) /
			itsCanvasDrawXSize * (itsLinXMax - itsLinXMin) + itsLinXMin;
	lin(1) = (pix(1) - itsCanvasDrawYOffset - itsCanvasYOffset) /
			itsCanvasDrawYSize * (itsLinYMax - itsLinYMin) + itsLinYMin;

	//These 4 lines will limit the points within the worldcanvas
	//if (lin(0) < itsLinXMin) lin(0) = itsLinXMin;
	//if (lin(0) > itsLinXMax) lin(0) = itsLinXMax;
	//if (lin(1) < itsLinYMin) lin(1) = itsLinYMin;
	//if (lin(1) > itsLinYMax) lin(1) = itsLinYMax;

	return True;
}
Bool WorldCanvas::pixToLin(Matrix<Double> &lin, Vector<Bool> &,
		const Matrix<Double> &pix) {
	if ( Int(itsCanvasDrawXSize * (itsLinXMax - itsLinXMin)+0.5) == 0 ||
			Int(itsCanvasDrawYSize * (itsLinYMax - itsLinYMin)+0.5) == 0 ) {
		return False;
	}
	Double tmpX = 1.0/itsCanvasDrawXSize * (itsLinXMax - itsLinXMin);
	Double tmpY = 1.0/itsCanvasDrawYSize *(itsLinYMax - itsLinYMin);
	for (Int i = 0; i < pix.shape()(0); i++) {
		lin(i, 0) = (pix(i, 0) - itsCanvasDrawXOffset - itsCanvasXOffset) *
				tmpX + itsLinXMin;
		lin(i, 1) = (pix(i, 1) - itsCanvasDrawYOffset - itsCanvasYOffset) *
				tmpY + itsLinYMin;
	}
	return True;
}
Bool WorldCanvas::linToPix(Vector<Double> &pix, const Vector<Double> &lin) {
	if ( Int((itsLinXMax - itsLinXMin)+0.5) == 0 ||
			Int((itsLinYMax - itsLinYMin)+0.5) == 0 ) {
		return False;
	}
	pix(0) = (lin(0) - itsLinXMin) / (itsLinXMax - itsLinXMin) *
			itsCanvasDrawXSize + itsCanvasDrawXOffset + itsCanvasXOffset;
	pix(1) = (lin(1) - itsLinYMin) / (itsLinYMax - itsLinYMin) *
			itsCanvasDrawYSize + itsCanvasDrawYOffset + itsCanvasYOffset;
	return True;
}
Bool WorldCanvas::linToPix(Matrix<Double> &pix, Vector<Bool> &,
		const Matrix<Double> &lin) {
	if ( Int((itsLinXMax - itsLinXMin)+0.5) == 0 ||
			Int((itsLinYMax - itsLinYMin)+0.5) == 0 ) {
		return False;
	}
	Double tmpX = 1.0/(itsLinXMax - itsLinXMin)*itsCanvasDrawXSize;
	Double tmpY = 1.0/(itsLinYMax - itsLinYMin)*itsCanvasDrawYSize;
	for (Int i = 0; i < pix.shape()(0); i++) {
		pix(i,0) = (lin(i,0) - itsLinXMin) * tmpX + itsCanvasDrawXOffset +
				itsCanvasXOffset;
		pix(i,1) = (lin(i,1) - itsLinYMin) * tmpY + itsCanvasDrawYOffset +
				itsCanvasYOffset;
	}
	return True;
}
Bool WorldCanvas::linToWorld(Vector<Double> &world,
		const Vector<Double> &lin) {
	return itsCoordinateHandler->linToWorld(world, lin);
}
Bool WorldCanvas::linToWorld(Matrix<Double> &world, Vector<Bool> &failures,
		const Matrix<Double> &lin) {
	return itsCoordinateHandler->linToWorld(world, failures, lin);
}
Bool WorldCanvas::worldToLin(Vector<Double> &lin,
		const Vector<Double> &world) {
	error_string = "";
	Bool result = itsCoordinateHandler->worldToLin(lin, world);
	if ( result == False ) {
		error_string = itsCoordinateHandler->errorMessage( );
	}
	return result;
}
Bool WorldCanvas::worldToLin(Matrix<Double> &lin, Vector<Bool> &failures,
		const Matrix<Double> &world) {
	return itsCoordinateHandler->worldToLin(lin, failures, world);
}
Bool WorldCanvas::pixToWorld(Vector<Double> &world,
		const Vector<Double> &pix) {
	static Vector<Double> lin(2);
	return (pixToLin(lin, pix) ? linToWorld(world, lin) : False);
}
Bool WorldCanvas::pixToWorld(Matrix<Double> &world, Vector<Bool> &failures,
		const Matrix<Double> &pix) {
	Matrix<Double> lin(pix.shape());
	Bool succ = pixToLin(lin, failures, pix);
	return (succ && linToWorld(world, failures, lin));
}
Bool WorldCanvas::worldToPix(Vector<Double> &pix,
		const Vector<Double> &world) {
	static Vector<Double> lin(2);
	return (worldToLin(lin, world) ? linToPix(pix, lin) : False);
}
Bool WorldCanvas::worldToPix(Matrix<Double> &pix, Vector<Bool> &failures,
		const Matrix<Double> &world) {
	Matrix<Double> lin(world.nrow(), 2);
	Bool succ = worldToLin(lin, failures, world);
	return (succ && linToPix(pix, failures, lin));
}

void WorldCanvas::registerColormap(Colormap *cmap, Float weight) {
	itsPixelCanvas->registerColormap(cmap, weight);
}

void WorldCanvas::registerColormap(Colormap *cmap, Colormap *cmapToReplace) {
	itsPixelCanvas->registerColormap(cmap, cmapToReplace);
}

void WorldCanvas::unregisterColormap(Colormap *cmap) {
	itsPixelCanvas->unregisterColormap(cmap);
}

void WorldCanvas::setColormap(Colormap *cmap) {
	if ( cmap != 0 ) itsPixelCanvas->setColormap(cmap);
}

Colormap *WorldCanvas::colormap() const {
	return itsPixelCanvas->colormap();
}

uInt WorldCanvas::newList() {
	return itsPixelCanvas->newList();
}

void WorldCanvas::endList() {
	itsPixelCanvas->endList();
}

void WorldCanvas::drawList(uInt list) {
	itsPixelCanvas->drawList(list);
}

void WorldCanvas::deleteList(uInt list) {
	itsPixelCanvas->deleteList(list);
}

void WorldCanvas::deleteLists() {
	itsPixelCanvas->deleteLists();
}

Bool WorldCanvas::validList(uInt list) {
	if (itsPixelCanvas) {
		return itsPixelCanvas->validList(list);
	}
	return False;
}

// Set color on WorldCanvas and PgPlot.
void WorldCanvas::setColor(const String &color) {
	itsPixelCanvas->setColor(color);
	Float r, g, b;
	itsPixelCanvas->getColorComponents(color, r, g, b);
	cpgscr(1, r, g, b);
	cpgsci(1);
}

void WorldCanvas::setClearColor(const String &color) {
	itsPixelCanvas->setClearColor(color);
}

Bool WorldCanvas::setFont(const String &fontName) {
	return itsPixelCanvas->setFont(fontName);
}

void WorldCanvas::setForeground(uLong color) {
	itsPixelCanvas->setForeground(color);
}

void WorldCanvas::setBackground(uLong color) {
	itsPixelCanvas->setBackground(color);
}

void WorldCanvas::setLineWidth(Float width) {
	itsPixelCanvas->setLineWidth(width);
}

void WorldCanvas::setLineStyle(Display::LineStyle style) {
	itsPixelCanvas->setLineStyle(style);
}

void WorldCanvas::setCapStyle(Display::CapStyle style) {
	itsPixelCanvas->setCapStyle(style);
}

void WorldCanvas::setJoinStyle(Display::JoinStyle style) {
	itsPixelCanvas->setJoinStyle(style);
}

void WorldCanvas::setFillStyle(Display::FillStyle style) {
	itsPixelCanvas->setFillStyle(style);
}

void WorldCanvas::setFillRule(Display::FillRule rule) {
	itsPixelCanvas->setFillRule(rule);
}

void WorldCanvas::setArcMode(Display::ArcMode mode) {
	itsPixelCanvas->setArcMode(mode);
}

// Set the WorldCanvas back/foreground colors/
Bool WorldCanvas::setWorldBackgroundColor(const String color) {
	Bool changed = itsWorldBackgroundColor != color;
	itsWorldBackgroundColor = color;
	return changed;
}
Bool WorldCanvas::setWorldForegroundColor(const String color) {
	Bool changed = itsWorldForegroundColor != color;
	itsWorldForegroundColor = color;
	return changed;
}

void WorldCanvas::setDrawBuffer(Display::DrawBuffer buf) {
	itsPixelCanvas->setDrawBuffer(buf);
}

Display::DrawBuffer WorldCanvas::drawBuffer() const {
	return itsPixelCanvas->drawBuffer();
}

void WorldCanvas::setImageCacheStrategy(Display::ImageCacheStrategy strategy) {
	itsPixelCanvas->setImageCacheStrategy(strategy);
}

Display::ImageCacheStrategy WorldCanvas::imageCacheStrategy() const {
	return itsPixelCanvas->imageCacheStrategy();
}

// Clearing functions
void WorldCanvas::clear() {
	// check if we are allowed to refresh:
	if (!itsPixelCanvas->refreshAllowed()) {
		return;
	}
	uInt store = itsPixelCanvas->clearColor();
	// may want to put this back in someday!
	// itsPixelCanvas->setClearColor(itsWorldBackgroundColor);
	itsPixelCanvas->setClearColor(itsPixelCanvas->deviceBackgroundColor());
	itsPixelCanvas->clear(Int(canvasXOffset()),
			Int(canvasYOffset()),
			Int(canvasXOffset()+canvasXSize()-1),
			Int(canvasYOffset()+canvasYSize()-1));
	itsPixelCanvas->setClearColor(store);
}
void WorldCanvas::clearNonDrawArea() {
	// check if we are allowed to refresh:
	if (!itsPixelCanvas->refreshAllowed()) {
		return;
	}
	uInt store = itsPixelCanvas->clearColor();
	// may want to put this back in someday!
	// itsPixelCanvas->setClearColor(itsWorldBackgroundColor);
	itsPixelCanvas->setClearColor(itsPixelCanvas->deviceBackgroundColor());
	// 1. left slice
	itsPixelCanvas->clear(Int(canvasXOffset()),
			Int(canvasYOffset()),
			Int(canvasXOffset() + canvasDrawXOffset()),
			Int(canvasYOffset() + canvasYSize() - 1));
	// 2. right slice
	itsPixelCanvas->clear(Int(canvasXOffset() + canvasDrawXOffset() +
			canvasDrawXSize()-1),
			Int(canvasYOffset()),
			Int(canvasXOffset() + canvasXSize()-1),
			Int(canvasYOffset() + canvasYSize() - 1));
	// 3. bottom remainder
	itsPixelCanvas->clear(Int(canvasXOffset() + canvasDrawXOffset()),
			Int(canvasYOffset()),
			Int(canvasXOffset() + canvasDrawXOffset() +
					canvasDrawXSize()-1),
					Int(canvasYOffset() + canvasDrawYOffset()));
	// 4. top remainder
	itsPixelCanvas->clear(Int(canvasXOffset() + canvasDrawXOffset()),
			Int(canvasYOffset() + canvasDrawYOffset() +
					canvasDrawYSize() -1),
					Int(canvasXOffset() + canvasDrawXOffset() +
							canvasDrawXSize()-1),
							Int(canvasYOffset() + canvasYSize() - 1));
	itsPixelCanvas->setClearColor(store);
}

// Option handling functions.
void WorldCanvas::setDefaultOptions() {
	String attString;
	AttributeBuffer attBuffer;
	attString = "leftMarginSpacePG";
	attBuffer.add(attString, Int(10));
	attString = "rightMarginSpacePG";
	attBuffer.add(attString, Int(10));
	attString = "bottomMarginSpacePG";
	attBuffer.add(attString, Int(6));
	attString = "topMarginSpacePG";
	attBuffer.add(attString, Int(6));

	setAttributes(attBuffer);
}
Bool WorldCanvas::setOptions(const Record &rec, Record &) {
	Bool ret = False, localchange = False;
	Bool error;

	Int temp;
	String attString;
	AttributeBuffer attBuffer;

	attString = "leftMarginSpacePG";
	getAttributeValue(attString, temp);
	localchange = readOptionRecord(temp, error, rec, LEFT_MARGIN_SPACE_PG)
		            		  || localchange;
	attBuffer.add(attString, temp);

	attString = "rightMarginSpacePG";
	getAttributeValue(attString, temp);
	localchange = readOptionRecord(temp, error, rec, RIGHT_MARGIN_SPACE_PG)
		            		  || localchange;
	attBuffer.add(attString, temp);

	attString = "bottomMarginSpacePG";
	getAttributeValue(attString, temp);
	localchange = readOptionRecord(temp, error, rec, BOTTOM_MARGIN_SPACE_PG)
		            		  || localchange;
	attBuffer.add(attString, temp);

	attString = "topMarginSpacePG";
	getAttributeValue(attString, temp);
	localchange = readOptionRecord(temp, error, rec, TOP_MARGIN_SPACE_PG)
		            		  || localchange;
	attBuffer.add(attString, temp);

	setAttributes(attBuffer);

	ret = ret || localchange;
	return ret;
}

Record WorldCanvas::getOptions() const {
	Record rec;

	String attString;
	Int temp;

	Record leftmarginspacepg;
	leftmarginspacepg.define("dlformat", LEFT_MARGIN_SPACE_PG);
	leftmarginspacepg.define("listname", "Left margin space (PG chars)");
	leftmarginspacepg.define("ptype", "intrange");
	leftmarginspacepg.define("pmin", 0);
	leftmarginspacepg.define("pmax", 20);
	leftmarginspacepg.define("default", Int(10));
	attString = "leftMarginSpacePG";
	getAttributeValue(attString, temp);
	leftmarginspacepg.define("value", temp);
	leftmarginspacepg.define("allowunset", False);
	leftmarginspacepg.define("context", "Margins");
	rec.defineRecord(LEFT_MARGIN_SPACE_PG, leftmarginspacepg);

	Record bottommarginspacepg;
	bottommarginspacepg.define("dlformat", BOTTOM_MARGIN_SPACE_PG);
	bottommarginspacepg.define("listname", "Bottom margin space (PG chars)");
	bottommarginspacepg.define("ptype", "intrange");
	bottommarginspacepg.define("pmin", 0);
	bottommarginspacepg.define("pmax", 20);
	bottommarginspacepg.define("default", Int(10));
	attString = "bottomMarginSpacePG";
	getAttributeValue(attString, temp);
	bottommarginspacepg.define("value", temp);
	bottommarginspacepg.define("allowunset", False);
	bottommarginspacepg.define("context", "Margins");
	rec.defineRecord(BOTTOM_MARGIN_SPACE_PG, bottommarginspacepg);

	Record rightmarginspacepg;
	rightmarginspacepg.define("dlformat", RIGHT_MARGIN_SPACE_PG);
	rightmarginspacepg.define("listname", "Right margin space (PG chars)");
	rightmarginspacepg.define("ptype", "intrange");
	rightmarginspacepg.define("pmin", 0);
	rightmarginspacepg.define("pmax", 20);
	rightmarginspacepg.define("default", Int(10));
	attString = "rightMarginSpacePG";
	getAttributeValue(attString, temp);
	rightmarginspacepg.define("value", temp);
	rightmarginspacepg.define("allowunset", False);
	rightmarginspacepg.define("context", "Margins");
	rec.defineRecord(RIGHT_MARGIN_SPACE_PG, rightmarginspacepg);

	Record topmarginspacepg;
	topmarginspacepg.define("dlformat", TOP_MARGIN_SPACE_PG);
	topmarginspacepg.define("listname", "Top margin space (PG chars)");
	topmarginspacepg.define("ptype", "intrange");
	topmarginspacepg.define("pmin", 0);
	topmarginspacepg.define("pmax", 20);
	topmarginspacepg.define("default", Int(10));
	attString = "topMarginSpacePG";
	getAttributeValue(attString, temp);
	topmarginspacepg.define("value", temp);
	topmarginspacepg.define("allowunset", False);
	topmarginspacepg.define("context", "Margins");
	rec.defineRecord( TOP_MARGIN_SPACE_PG, topmarginspacepg);

	return rec;
}

// Set Attribute/s.
void WorldCanvas::setAttribute(Attribute &at) {
	attributes.set(at);
}
void WorldCanvas::setAttributes(AttributeBuffer &at) {
	attributes.set(at);
}

// Remove an Attribute.
void WorldCanvas::removeAttribute(String& name) {
	attributes.remove(name);
}

// Get value/s from the Attribute buffer.
Bool WorldCanvas::getAttributeValue(const String& name, uInt& newValue)
const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name, Int& newValue)
const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name, Float& newValue)
const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name, Double& newValue)
const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name, Bool& newValue)
const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name, String& newValue)
const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name,
		Vector<uInt>& newValue) const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name,
		Vector<Int>& newValue) const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name,
		Vector<Float>& newValue) const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name,
		Vector<Double>& newValue) const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name,
		Vector<Bool>& newValue) const {
	return attributes.getValue(name, newValue);
}
Bool WorldCanvas::getAttributeValue(const String& name,
		Vector<String>& newValue) const {
	return attributes.getValue(name, newValue);
}

// Check if a certain Attribute exists.
Bool WorldCanvas::existsAttribute(String& name) const {
	return attributes.exists(name);
}

// Get the type of an Attribute.
AttValue::ValueType WorldCanvas::attributeType(String& name) const {
	return attributes.getDataType(name);
}

// Acquire/release a PGPLOT device for this WorldCanvas.
void WorldCanvas::acquirePGPLOTdevice(const Bool &linear) {
	if (!itsPGFilter) {
		itsPGFilter = new WCPGFilter(this);
	}
	itsPGFilter->ref();
	itsPGFilter->realign(linear);
}
void WorldCanvas::releasePGPLOTdevice() {
	if (!itsPGFilter) {
		return;
	}
	itsPGFilter->unref();
	if (itsPGFilter->refCount() == 0) {
		delete itsPGFilter;
		itsPGFilter = 0;
	} else {
		// need to realign the filter according to previous
		// Bool on stack...
	}
}

// Return the PGPLOT device id.
Int WorldCanvas::pgid() const {
	if (itsPGFilter) {
		return Int(itsPGFilter->pgid());
	} else {
		return Int(-1);
	}
}

// Draw unrotated text.
Bool WorldCanvas::drawText(const Vector<Double> &point, const String &text,
		Display::TextAlign alignment, const Bool &linear) {
	Vector<Int> pixelpt;
	if (castingConversion(pixelpt, point, linear)) {
		itsPixelCanvas->drawText(pixelpt(0), pixelpt(1), text, alignment);
		return True;
	} else {
		return False;
	}
}

// Draw a single point.
Bool WorldCanvas::drawPoint(const Vector<Double> &point, const Bool &linear) {
	Vector<Int> pixelpt;
	if (castingConversion(pixelpt, point, linear)) {
		itsPixelCanvas->drawPoint(pixelpt(0), pixelpt(1));
		return True;
	} else {
		return False;
	}
}

// Draw a single line.
Bool WorldCanvas::drawLine(const Vector<Double> &a, const Vector<Double> &b,
		const Bool &linear) {
	Vector<Int> pixa, pixb;
	if (castingConversion(pixa, a, linear) &&
			castingConversion(pixb, b, linear)) {
		itsPixelCanvas->drawLine(pixa(0), pixa(1), pixb(0), pixb(1));
		return True;
	} else {
		return False;
	}
}

// Draw a set of points.
Bool WorldCanvas::drawPoints(const Matrix<Double> &points,
		const Bool &linear) {
	Matrix<Int> pixelpts;
	if (castingConversion(pixelpts, points, linear)) {
		itsPixelCanvas->drawPoints(pixelpts);
		return True;
	} else {
		return False;
	}
}

// Draw a set of points.
Bool WorldCanvas::drawPoints(const Vector<Float> &px, const Vector<Float> &py,
		const Bool linear) {
	Vector<Int> pixelx, pixely;
	Vector<Bool> validconversions;
	if (castingClippingConversion(pixelx, pixely, validconversions,
			px, py, linear)) {
		itsPixelCanvas->drawPoints(pixelx, pixely);
		return True;
	} else {
		return False;
	}
}

// Draw a set of text strings.
Bool WorldCanvas::drawTextStrings(const Vector<Float> &px,
		const Vector<Float> &py,
		const Vector<String> &strings,
		const Float rotation,
		const Float xoffset, const Float yoffset,
		const Bool linear) {
	if (px.nelements() != py.nelements()) {
		throw(AipsError("Non-conformant input to WorldCanvas::"
				"drawTextStrings"));
	}
	Vector<Int> pixelx, pixely;
	Vector<Bool> validconversions;
	if (castingClippingConversion(pixelx, pixely, validconversions,
			px, py, linear)) {
		if (pixelx.nelements() != pixely.nelements()) {
			throw(AipsError("Non-conformant conversion result in WorldCanvas::"
					"drawTextStrings"));
		}
		MaskedArray<String> mask(strings, LogicalArray(validconversions == True));
		Vector<String> vstrings(mask.getCompressedArray());
		if (vstrings.nelements() != pixelx.nelements()) {
			throw(AipsError("Failure parsing output of conversion in "
					"WorldCanvas::drawTextStrings"));
		}

		// calculate character height and deduce linear offsets
		Float dx, dy;
		cpgqcs(3, &dx, &dy);
		dx = dx * xoffset;
		dy = dy * yoffset;

		// loop over all strings
		for (uInt i = 0; i < vstrings.nelements(); i++) {
			// Do with pgplot for time being...
			Vector<Double> pix(2), lin(2);
			pix(0) = pixelx(i) + dx;
			pix(1) = pixely(i) + dy;
			if (pixToLin(lin, pix)) {
				cpgptxt(Float(lin(0)), Float(lin(1)), rotation, 0.5,
						vstrings(i).chars());
			}
		}
		return True;
	} else {
		return False;
	}
}

// Draw a set of markers.
Bool WorldCanvas::drawMarkers(const Vector<Float> &px, const Vector<Float> &py,
		const Display::Marker markertype,
		const Int size, const Bool& linear) {
	if (px.nelements() != py.nelements()) {
		throw(AipsError("Non-conformant input to WorldCanvas::"
				"drawMarkers"));
	}
	Vector<Int> pixelx, pixely;
	Vector<Bool> validconversions;
	if (castingClippingConversion(pixelx, pixely, validconversions,
			px, py, linear)) {
		if (pixelx.nelements() != pixely.nelements()) {
			throw(AipsError("Non-conformant conversion result in WorldCanvas::"
					"drawMarkers"));
		}
		for (uInt i = 0; i < pixelx.nelements(); i++) {
			itsPixelCanvas->drawMarker(pixelx(i), pixely(i), markertype, size);
		}
	} else {
		return False;
	}
	return True;
}

Bool WorldCanvas::drawMappedMarkers(const Vector<Float> &px,
		const Vector<Float> &py,
		const Vector<Float> &values,
		const Int sizemin, const Int sizemax,
		const Display::Marker markertype,
		const Bool& linear) {
	if (px.nelements() != py.nelements()) {
		throw(AipsError("Non-conformant input to WorldCanvas::"
				"drawMarkers"));
	}
	Int size;
	Int deltasize = sizemax-sizemin;
	Float min,max;

	minMax(min,max,values);
	Float delta = max-min;
	Vector<Int> ppos(2);
	Vector<Double> wpos(2);
	for (uInt i = 0; i < px.nelements(); i++) {
		wpos(0) = px(i);
		wpos(1) = py(i);
		Bool success = False;
		success = castingConversion(ppos, wpos, linear);
		if ( success && inDrawArea(ppos(0),ppos(1)) ) {
			size = Int(ceil(sizemin + fabs(min-values(i))/delta*deltasize)+0.5);
			itsPixelCanvas->drawMarker(ppos(0),ppos(1), markertype, size);
		}
	}
	clearNonDrawArea();
	return True;
}

// Draw a set of lines.
Bool WorldCanvas::drawLines(const Matrix<Double> &vertices,
		const Bool &linear) {
	Matrix<Int> pixelpts;
	if (castingConversion(pixelpts, vertices, linear)) {
		itsPixelCanvas->drawLines(pixelpts);
		return True;
	} else {
		return False;
	}
}

// Draw a polyline (connected line).
Bool WorldCanvas::drawPolyline(const Matrix<Double> &vertices,
		const Bool &linear) {
	Matrix<Int> pixelpts;
	if (castingConversion(pixelpts, vertices, linear)) {
		itsPixelCanvas->drawPolyline(pixelpts);
		return True;
	} else {
		return False;
	}
}

// Draw a polygon.
Bool WorldCanvas::drawPolygon(const Matrix<Double> &vertices,
		const Bool &linear) {
	Matrix<Int> pixelpts;
	if (castingConversion(pixelpts, vertices, linear)) {
		itsPixelCanvas->drawPolygon(pixelpts);
		return True;
	} else {
		return False;
	}
}

// Draw points in colors from the Colormap.
Bool WorldCanvas::drawColormappedPoints(const Matrix<Double> &points,
		const Vector<Float> &values,
		const Bool &linear) {
	// 1. rescale the values to colormap indices
	Vector<uInt> scaledValues(values.shape());
	uInt rMax = itsPixelCanvas->getColormapSize() - 1;
	itsDataScaleHandler->setRangeMax(rMax);
	itsDataScaleHandler->setDomainMinMax(itsDataMin, itsDataMax);
	(*itsDataScaleHandler)(scaledValues, values);

	// 2. draw the points
	return mapToColorAndDrawPoints(points, scaledValues, linear);
}


Bool WorldCanvas::drawBeamEllipse(Float major, Float minor,  Float pa,
		String majunit, String minunit,  String paunit,
		Float cx, Float cy, Bool outline) {
	// This routine is specialized for drawing image restoring-beam ellipses.
	// Its parameters are defined so as to require as little conversion as
	// possible of restoring beam information as stored in an image header.
	// (6/07).
	//
	// It does nothing unless the axes on display map to the two axes of a
	// DirectionCoordinate within the WC CS (WorldCanvas::itsCoordinateSystem).
	// Center location cx,cy is specified as a fraction of WC draw area:
	// (0,0) is blc, (1,1) is trc; they default to (.1, .1).
	//
	// The unit strings for major,minor are given in in majunit, minunit, and
	// should be valid angular units (they default to "arcsec").  major/minor
	// are the _full_ major and minor axis sizes in terms of relative direction
	// world coordinates.
	//
	// pa specifies "position angle", in the angular units specified by paunit.
	// pa uses the image header convention "North-to-East"; more precisely,
	// 0 degrees aligns the major axis along increasing DirCoord(1) (commonly
	// Dec), 90 degrees aligns it along increasing DirCoord(0) (commonly RA).
	// (NB: increasing RA commonly means decreasing pixel/Lattice coordinates).
	// In the common case, this means that pa increases counterclockwise from
	// vertical.  Note that this is _not_ the pa convention in some other
	// PixelCanvas/WorldCanvas drawEllipse() routines (where pa is always
	// counterclockwise from horizontal).
	//
	// Also note: this routine attempts to do the right thing in oddball cases
	// such as displaying Dec on the horizontal axis (pa 0 would also be
	// horizontal in that case), distorted ('flexible') aspect ratio (ellipse
	// also distorted appropriately) or all-sky images in which the beam may be
	// displayed in a canvas area where absolute world coordinates do not exist.
	// It should even take care of uneven coordinate 'increments' (non-square
	// image data pixels).
	// (So far, it does _not_ correctly handle non-identity transformation
	// matrices in the DirectionCoordinate (e.g. rotated "North")-- which I
	// think is true in many other places as well... -- for someone's to-do list
	// (not mine, I hope)).

	Unit rad("rad");
	if(rad!=majunit || rad!=minunit || rad!=paunit) return False;
	// Check that passed units are angular.

	DisplayCoordinateSystem* cs = itsCoordinateSystem;
	if(cs==0) return False;
	if(cs->nPixelAxes()<2) return False;	// (shouldn't happen).

	Int dcno = cs->findCoordinate(Coordinate::DIRECTION);
	if(dcno<0) return False;	// We need a direction coordinate.
	if(cs->findCoordinate(Coordinate::DIRECTION, dcno)!=-1) return False;
	// (too weird to deal with...).

	const DirectionCoordinate& dc = cs->directionCoordinate(dcno);
	Vector<Double> incr = dc.increment();
	// relative World unit increase per Lattice element, on zeroth
	// ('East', 'RA') and first ('North', Dec') Direction coordinates.


	// The following are indexed by ax, which is 0 for display
	// horizontal (X), 1 for vertical(Y).

	Vector<Int> dirAxNo(2);	// (described below).
	Vector<Double> stretch(2);
	// maj, min need to be passed to  PC::drawPolygon() in common
	// units, for position angle to be drawn correctly; radians has
	// been chosen arbitrarily.  stretch then applies current X and y
	// conversions from radians to screen pixels.
	Vector<Double> screenPerLin(2);
	// Current number of screen pixels per 'linear coordinate unit'
	// (data Lattice element) on the WC


	if(itsLinXMax<=itsLinXMin || itsLinYMax<=itsLinYMin) return False;
	// (shouldn't happen).

	screenPerLin[0] = itsCanvasDrawXSize / (itsLinXMax - itsLinXMin);
	screenPerLin[1] = itsCanvasDrawYSize / (itsLinYMax - itsLinYMin);


	for(Int ax=0; ax<2; ax++) {
		Int coordno, dirax;
		cs->findPixelAxis(coordno, dirax, ax);
		if(coordno!=dcno) return False;
		// Direction Coordinatess (sky) must be on display.
		dirAxNo[ax] = dirax;
		// dirax:  0 means the Long/Ra-like axis,  1 the Lat/Dec-like axis.

		Int wax = cs->pixelAxisToWorldAxis(ax);
		// (I dislike the philosophy of requiring a particular pixel
		// axis to be identified with a particular sky axis, actually...).
		if(wax<0) return False;
		String csunit = cs->worldAxisUnits()[wax];

		if(rad!=csunit) return False;	// (cs units should be angular).

		Double radincr = Quantum<Double>(incr[dirax], csunit).getValue(rad);
		// relevant axis's world coordinate increment in radians.
		if(radincr==0.) return False;
		stretch[ax] = (1./radincr) *  screenPerLin[ax];
	}
	// On each canvas axis, the current conversion factor from relative
	// world coordinate radians to screen (PixelCanvas) pixels.


	if( (dirAxNo[0] != 0 && dirAxNo[0] !=1 ) ||
			(dirAxNo[1] != 1-dirAxNo[0]) ) return False;
	// (Will usually be (0,1), but will be (1,0) if the sky coordinates
	// are transposed from their normal order on the WorldCanvas.
	// It must be one or the other -- _both_ direction axes must
	// be on display).

	Double degpa = Quantum<Double>(pa, paunit).getValue("deg");
	// PixelCanvas::drawEllipse() wants pos angle in degrees.

	// Factor in possible rotation of sky coordinate North away from vertical.
	// ("Getting the sign right"...  grr...).  I spent days trying to do this
	// correctly from the 'documents' -- good luck.
	// The old (sensible) ROTA (sky rotation) angle was depricated in favor
	// of a 'PC' (pixel coordinate) matrix, which rotates pixels instead (and
	// may do more).  ROTA was 'deprecated' at least seven years ago but is
	// still used (e.g., in AIPS rotation, aips++ image-to-FITS, etc).  In
	// radio, at least, you usually want to rotate the sky, not the pixels, but
	// someone in the optical mafia put the fix in.  So we have PC, which is
	// commonly misused when image pixels are not square on the sky (abs
	// CDELTS not equal).  ia.rotate of such an image produces erroneous
	// results, e.g.
	// The PC matrix is usually set erroneously in that case: it should not
	// really be a rotation matrix then (it doesn't commute with the CDELT
	// matrix), but is commonly just a copy of the sky rotation matrix
	// corresponding to CROTA, which is what is assumed here...).
	// Also note: PC is all the gods of DirectionCoordinate ever deign to let
	// us see (they keep the wcsprm to themselves, the greedy darlins...).
	//
	// Anyway, I give up trying to do it right, for now.  This works empirically
	// at least for a rotation or for non-square pixels (if perhaps not for both
	// at once -- when, as I say, other code fails as well...).


	Matrix<Double> pc = dc.linearTransform();	// ('PC Matrix')

	if(pc.nrow()==2 && pc.ncolumn()==2) {		// (should be True)
		if( abs(pc(0,0)*pc(0,1) + pc(1,0)*pc(1,1)) < 1.e-5 &&
				near(pc(0,0)*pc(0,0) + pc(1,0)*pc(1,0) , 1.)   &&
				near(pc(0,1)*pc(0,1) + pc(1,1)*pc(1,1) , 1.) ) {
			// Check orthogonality of pc (I'm not going to attempt
			// to deal with ant other PC species....  As I say, a sky
			// rotation with non-square pixels is _supposed_ to mean that
			// pc is _not_ orthogonal, but at present it will be...)).
			Double rho = atan2(pc(1,0), pc(0,0)) * 180. / C::pi;
			// Recover the angle of DirCoord rotation (old 'CROTA')
			// (essentially atan(sin(rho)/cos(rho)) ).

			degpa += rho;
		}
	}


	Bool transposed = dirAxNo[0]==1;
	if(!transposed) degpa = 90. - degpa;
	// Applies proper pos angle transformation: On the sky it is
	// from "North" (positive dirAxNo 1) increasing toward "East"
	// (positive dirAxNo 0).  In PixelCanvas, it is from rightward
	// (positive X (ax 0)) increasing toward upward (positive Y (ax 1)).


	Double radsmaj = Quantum<Double>(major, majunit).getValue(rad) / 2.;
	Double radsmin = Quantum<Double>(minor, minunit).getValue(rad) / 2.;
	// These ellipse parameters, passed to PixelCanvas::drawEllipse()
	// below, are relative world coordinate (sky) radians; the computed
	// ellipse (x,y) loci will then be scaled (in most cases equally)
	// to (x,y) screen pixels relative to the ellipse center.
	// Also note that full major, minor axes were passed to this routine,
	// whereas PixelCanvas::drawEllipse() uses semi-major and semi-minor
	// axes.

	Double scrcx = cx * itsCanvasDrawXSize
			+ itsCanvasXOffset + itsCanvasDrawXOffset;
	Double scrcy = cy * itsCanvasDrawYSize
			+ itsCanvasYOffset + itsCanvasDrawYOffset;
	// The center was passed to this routine as a fractional cx, cy
	// within the WC image drawing area (0,0=blc, 1,1=trc); Here they
	// are transformed and passed below as absolute PixelCanvas (screen
	// pixel) x and y coordinates ('stretch' scaling factors will _not_
	// affect this center location).



	itsPixelCanvas->drawEllipse(scrcx, scrcy,  radsmaj, radsmin,  degpa,
			outline,    stretch[0], stretch[1]);

	return True;
}





// Draw ellipses in colors from the Colormap.
Bool WorldCanvas::drawColormappedEllipses(const Matrix<Double> &centres,
		const Vector<Float> &smajor,
		const Vector<Float> &sminor,
		const Vector<Float> &pangle,
		const Vector<Float> &values,
		const Float &scale,
		const Bool &outline,
		const Bool &linear) {

	// 1. rescale the values to colormap indices
	Vector<uInt> scaledValues(values.shape());
	uInt rMax = itsPixelCanvas->getColormapSize() - 1;
	itsDataScaleHandler->setRangeMax(rMax);
	itsDataScaleHandler->setDomainMinMax(itsDataMin, itsDataMax);
	(*itsDataScaleHandler)(scaledValues, values);

	// 2. draw the ellipses
	return mapToColorAndDrawEllipses(centres, smajor, sminor, pangle,
			scaledValues, scale, outline,
			linear);
}

// Draw contour map.
bool WorldCanvas::drawContourMap(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Vector<Float> &levels,
		const Bool usePixelEdges) {
	Vector<Double> actWorldBlc, actWorldTrc;
	Matrix<Float> sampledImage;
	trimAndResampleImage(actWorldBlc, actWorldTrc, sampledImage,
			blc, trc, data, usePixelEdges);

	// scaling - must finish at itsLinXMax, itsLinYMax:

	Vector<Double> linBlc(2), linTrc(2);

	if (!worldToLin(linBlc, actWorldBlc)) {
		return false;
	}

	if (!worldToLin(linTrc, actWorldTrc)) {
		return false;
	}
	//
	Matrix<Float> tr(3,2);
	tr(1,0) = (linTrc(0) - linBlc(0)) / (Float)(sampledImage.shape()(0) - 0);
	tr(2,1) = (linTrc(1) - linBlc(1)) / (Float)(sampledImage.shape()(1) - 0);

	// start here:

	tr(0,0) = linBlc(0) - tr(1,0);
	tr(0,1) = linBlc(1) - tr(2,1);

	// no cross terms

	tr(1,1) = tr(2,0) = 0.0;

	// draw the contours

	itsPGFilter->cont(sampledImage, levels, tr);
	return true;
}

bool WorldCanvas::drawContourMap(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Complex> &data,
		const Vector<Float> &levels,
		const Bool usePixelEdges) {
	switch (itsComplexToRealMethod) {
	case Display::Phase:
		return drawContourMap(blc, trc, phase(data), levels, usePixelEdges);
	case Display::Real:
		return drawContourMap(blc, trc, real(data), levels, usePixelEdges);
	case Display::Imaginary:
		return drawContourMap(blc, trc, imag(data), levels, usePixelEdges);
	default:
		return drawContourMap(blc, trc, amplitude(data), levels, usePixelEdges);
	}
}

// Draw contour map.
bool WorldCanvas::drawContourMap(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Matrix<Bool> &mask,
		const Vector<Float> &levels,
		const Bool usePixelEdges) {
	static Float blank = -12345.;
	Vector<Double> actWorldBlc, actWorldTrc;
	Matrix<Float> sampledImage;
	Matrix<Bool> sampledMask;
	trimAndResampleImage(actWorldBlc, actWorldTrc, sampledImage,
			sampledMask, blc, trc, data, mask,
			usePixelEdges);

	// Set values of sampledImage to magic value where blanked

	MaskedArray<Float> t(sampledImage, sampledMask == False);
	t = blank;

	// scaling - must finish at itsLinXMax, itsLinYMax:

	Vector<Double> linBlc(2), linTrc(2);
	if (!worldToLin(linBlc, actWorldBlc)) {
		return false;
	}
	if (!worldToLin(linTrc, actWorldTrc)) {
		return false;
	}
	//
	Matrix<Float> tr(3,2);
	tr(1,0) = (linTrc(0) - linBlc(0)) / (Float)(sampledImage.shape()(0) - 0);
	tr(2,1) = (linTrc(1) - linBlc(1)) / (Float)(sampledImage.shape()(1) - 0);

	// start here:

	tr(0,0) = linBlc(0) - tr(1,0);
	tr(0,1) = linBlc(1) - tr(2,1);

	// no cross terms

	tr(1,1) = tr(2,0) = 0.0;


	// draw the contours

	itsPGFilter->conb(sampledImage, levels, tr, blank);
	return true;
}

bool WorldCanvas::drawContourMap(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Complex> &data,
		const Matrix<Bool> &/*mask*/,
		const Vector<Float> &levels,
		const Bool usePixelEdges) {
	switch (itsComplexToRealMethod) {
	case Display::Phase:
		return drawContourMap(blc, trc, phase(data), levels, usePixelEdges);
	case Display::Real:
		return drawContourMap(blc, trc, real(data), levels, usePixelEdges);
	case Display::Imaginary:
		return drawContourMap(blc, trc, imag(data), levels, usePixelEdges);
	default:
		return drawContourMap(blc, trc, amplitude(data), levels, usePixelEdges);
	}
}
// Draw an image.
// Optionally saves color-indexed image for reuse (under drawObj key).
void WorldCanvas::drawImage(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Matrix<Float> &dataRed,
		const Matrix<Float> &dataGreen,
		const Matrix<Float> &dataBlue,
		const Bool usePixelEdges,
		void* drawObj) {
	ColorIndexedImage_* imRed = NULL;
	if ( dataRed.nelements() > 0 ){
		imRed = makeColorIndexedImage( blc, trc, dataRed, usePixelEdges, NULL );
	}
	ColorIndexedImage_* imGreen = NULL;
	if ( dataGreen.nelements() > 0 ){
		imGreen = makeColorIndexedImage( blc, trc, dataGreen, usePixelEdges, NULL );
	}
	ColorIndexedImage_* imBlue = NULL;
	if ( dataBlue.nelements() > 0 ){
		imBlue = makeColorIndexedImage( blc, trc, dataBlue, usePixelEdges, NULL );
	}
	ColorIndexedImage_* im = makeColorIndexedImage( blc, trc, data, usePixelEdges, drawObj );

	//Get colormap indices for all of the data.
	Matrix<uInt> redColors;
	Matrix<uInt> blueColors;
	Matrix<uInt> greenColors;
	if ( imRed != NULL ){
		redColors = imRed->data;
	}
	if ( imBlue != NULL ){
		blueColors = imBlue->data;
	}
	if ( imGreen != NULL ){
		greenColors = imGreen->data;
	}
	Matrix<uInt> combinedColors;
	itsPixelCanvas->mapToColor(combinedColors, redColors, greenColors, blueColors);
	Vector<int> blcVector = im->blc;
	itsPixelCanvas->drawImage(combinedColors, blcVector(0), blcVector(1));
	delete imRed;
	delete imBlue;
	delete imGreen;
	if ( drawObj == 0 ){
		delete im;
	}
}

void WorldCanvas::drawImage(const Vector<Double> &blc, const Vector<Double> &trc,
		const Matrix<Complex> &data,
		const Matrix<Complex> &dataRed,
		const Matrix<Complex> &dataGreen,
		const Matrix<Complex> &dataBlue,
		const Bool usePixelEdges,
		void* drawObj) {
	switch (itsComplexToRealMethod) {
	case Display::Phase:
		drawImage(blc, trc, phase(data), phase(dataRed), phase(dataGreen), phase(dataBlue),
				usePixelEdges, drawObj);
		break;
	case Display::Real:
		drawImage(blc, trc, real(data), real(dataRed), real(dataGreen), real(dataBlue),
				usePixelEdges, drawObj);
		break;
	case Display::Imaginary:
		drawImage(blc, trc, imag(data), imag(dataRed), imag(dataGreen), imag(dataBlue),
				usePixelEdges, drawObj);
		break;
	default:
		drawImage(blc, trc, amplitude(data), amplitude(dataRed), amplitude(dataGreen), amplitude(dataBlue),
				usePixelEdges, drawObj);
		break;
	}
}

// Draw an image.
// Optionally saves color-indexed image for reuse (under drawObj key).
void WorldCanvas::drawImage(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Bool usePixelEdges,
		void* drawObj) {

	ColorIndexedImage_* im = makeColorIndexedImage( blc, trc, data, usePixelEdges, drawObj);
	mapToColorAndDrawImage(im->blc, im->data);
	if(drawObj==0) delete im;
	// discard indexed image if caching not desired.
}

WorldCanvas::ColorIndexedImage_* WorldCanvas::makeColorIndexedImage(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Bool usePixelEdges, void* drawObj){
	Vector<Double> wBlc(2), wTrc(2), pBlc(2);
	Matrix<Float> sampledImage;
	trimAndResampleImage(wBlc, wTrc, sampledImage,
			blc, trc, data, usePixelEdges);
	if (!worldToPix(pBlc, wBlc)) return NULL;


	ColorIndexedImage_* im = getClearedColorIndexedImage(drawObj);
	// 'im' is cleared for [re]use.
	// It will be cached in images_ iff drawObj!=0.


	// scale the resampled image to color indices.

	im->data.resize(sampledImage.shape());
	im->colormapSize = itsPixelCanvas->getColormapSize();

	itsDataScaleHandler->setRangeMax(im->colormapSize - 1u);
	itsDataScaleHandler->setDomainMinMax(itsDataMin, itsDataMax);

	(*itsDataScaleHandler)(im->data, sampledImage);	// fill indexed image.

	im->blc[0] = Int(pBlc[0]+.5);
	im->blc[1] = Int(pBlc[1]+.5);
	return im;
}

void WorldCanvas::drawImage(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Complex> &data,
		const Bool usePixelEdges,
		void* drawObj) {

	switch (itsComplexToRealMethod) {
	case Display::Phase:
		drawImage(blc, trc, phase(data), usePixelEdges, drawObj);
		break;
	case Display::Real:
		drawImage(blc, trc, real(data), usePixelEdges, drawObj);
		break;
	case Display::Imaginary:
		drawImage(blc, trc, imag(data), usePixelEdges, drawObj);
		break;
	default:
		drawImage(blc, trc, amplitude(data), usePixelEdges, drawObj);
		break;
	}
}




// Draw  image (masked version).
// Optionally saves color-indexed image for reuse (under drawObj key).

void WorldCanvas::drawImage(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Matrix<Bool> &mask,
		const Bool usePixelEdges,
		void* drawObj, Bool opaqueMask) {


	ColorIndexedImage_* im = getClearedColorIndexedImage(drawObj);
	// 'im' is cleared for [re]use.
	// It will be cached in images_ iff drawObj!=0.


	Vector<Double> wBlc(2), wTrc(2), pBlc(2);
	Matrix<Float> sampledImage;
	trimAndResampleImage(wBlc, wTrc, sampledImage, im->mask,
			blc, trc, data, mask, usePixelEdges);

	if (!worldToPix(pBlc, wBlc)) {	// (Shouldn't happen)
		delete im;
		if(drawObj!=0) images_.remove(drawObj);
		return;
	}


	// scale the resampled image to color indices.

	im->data.resize(sampledImage.shape());
	im->colormapSize = itsPixelCanvas->getColormapSize();

	itsDataScaleHandler->setRangeMax(im->colormapSize - 1u);
	itsDataScaleHandler->setDomainMinMax(itsDataMin, itsDataMax);

	(*itsDataScaleHandler)(im->data, sampledImage);	// fill indexed image.

	im->blc[0] = Int(pBlc[0]+.5);
	im->blc[1] = Int(pBlc[1]+.5);


	mapToColorAndDrawImage(im->blc, im->data, im->mask, opaqueMask);


	if(drawObj==0) delete im;
	// discard indexed image if caching not desired.
}


void WorldCanvas::drawImage(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Complex> &data,
		const Matrix<Bool> &mask,
		const Bool usePixelEdges,
		void* drawObj, Bool opaqueMask) {

	switch (itsComplexToRealMethod) {
	case Display::Phase:
		drawImage(blc, trc, phase(data), mask, usePixelEdges, drawObj,
				opaqueMask);
		break;
	case Display::Real:
		drawImage(blc, trc, real(data), mask, usePixelEdges, drawObj, opaqueMask);
		break;
	case Display::Imaginary:
		drawImage(blc, trc, imag(data), mask, usePixelEdges, drawObj, opaqueMask);
		break;
	default:
		drawImage(blc, trc, amplitude(data), mask, usePixelEdges, drawObj,
				opaqueMask);
		break;
	}
}



WorldCanvas::ColorIndexedImage_* WorldCanvas::getClearedColorIndexedImage(
		void* drawObj) {
	// Retrieve an indexed image to write onto.  Used (exclusively) by
	// WC::drawImage().  If one exists in the cache under this objId key,
	// clear it for reuse; otherwise return a new one. If a (non-zero) objId
	// was provided, it will be cached under that key; otherwise the caller
	// must delete it himself when finished.

	ColorIndexedImage_* im;

	if(drawObj!=0 && images_.isDefined(drawObj)) {
		im = images_(drawObj);		// Retrieve existing cached image
		im->clear();
	}			// and clear it for reuse.
	else {
		im = new ColorIndexedImage_;	// create new color-indexed image.
		if(drawObj!=0) images_.define(drawObj, im);
	}
	// cache it, if caller wishes.
	return im;
}



Bool WorldCanvas::redrawIndexedImage(void* drawObj,
		Display::RefreshReason reason,
		Bool opaqueMask) {
	// Redraw from color-indexed image cache, if possible (speeds up
	// colormap-only changes considerably).

	if(!images_.isDefined(drawObj)) return False;	// No such image.

	ColorIndexedImage_* im = images_(drawObj);

	if( reason != Display::ColorTableChange ||
			im->colormapSize != itsPixelCanvas->getColormapSize() ) {
		removeIndexedImage(drawObj);
		return False;
	}
	// not a colormap-only refresh, or colormap size has changed.
	// Old color-indexed image can't be reused -- remove it.

	if(im->maskValid()) mapToColorAndDrawImage(im->blc, im->data,
			im->mask, opaqueMask);
	else		      mapToColorAndDrawImage(im->blc, im->data);
	return True;
}	// Successful redraw.



Bool WorldCanvas::removeIndexedImage(void* drawObj) {
	// Remove a color-indexed image from the cache, if any.  Return value
	// indicates whether there was anything to remove.

	if(!images_.isDefined(drawObj)) return False;
	delete images_(drawObj);
	images_.remove(drawObj);
	return True;
}


void WorldCanvas::clearColormapChangeCache() {
	// Clear the whole colormap change cache (see images_, below).

	for(uInt i=0; i<images_.ndefined(); i++) delete images_.getVal(i);
	images_.clear();
}



// Draw a color component of a multi-channel image.
void WorldCanvas::drawImage(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Display::ColorComponent &colorcomponent,
		const Bool usePixelEdges) {

	// 0. check that we are the right colormodel
	itsPixelCanvas->setColorModel(itsPixelCanvas->pcctbl()->colorModel());
	switch(itsPixelCanvas->colorModel()) {
	case Display::RGB: {
		if ((colorcomponent != Display::Red) &&
				(colorcomponent != Display::Green) &&
				(colorcomponent != Display::Blue)) {
			LogIO os;
			os << LogIO::WARN << LogOrigin("WorldCanvas", "drawImage", WHERE)
						   << "WorldCanvas cannot draw non-RGB images on RGB PixelCanvases"
						   << LogIO::POST;
		}
		break;
	}
	case Display::HSV: {
		if ((colorcomponent != Display::Hue) &&
				(colorcomponent != Display::Saturation) &&
				(colorcomponent != Display::Value)) {
			LogIO os;
			os << LogIO::WARN << LogOrigin("WorldCanvas", "drawImage", WHERE)
						   << "WorldCanvas cannot draw non-HSV images on HSV PixelCanvases"
						   << LogIO::POST;
		}
		break;
	}
	default: {
		LogIO os;
		os << LogIO::WARN << LogOrigin("WorldCanvas", "drawImage", WHERE)
					   << "WorldCanvas cannot draw component images on PixelCanvases "
					   << "unless they are RGB- or HSV-based."
					   << LogIO::POST;
	}
	}

	// 2. get component-specific information: max colors, ptr to image.
	uInt rMax = 0;
	uInt n1, n2, n3;
	itsPixelCanvas->pcctbl()->nColors(n1, n2, n3);

	switch (colorcomponent) {
	case Display::Red:
	case Display::Hue: {
		rMax = n1 - 1;
	}
	break;
	case Display::Green:
	case Display::Saturation: {
		rMax = n2 - 1;
	}
	break;
	case Display::Blue:
	case Display::Value: {
		rMax = n3 - 1;
	}
	break;
	default: {
		throw(AipsError("Unknown component to draw in WorldCanvas::drawImage"));
	}
	}
	Vector<Double> reqPixBlc(2), reqPixTrc(2);
	worldToPix(reqPixBlc, blc);
	worldToPix(reqPixTrc, trc);


	// 3. Clip input to draw area (and round to whole screen pixels)

	// 'req' = corners of input array, in pixelcanvas ('screen') coordinates.
	// 'act' (or 'sampled', 'scaled') = corners of output array, originally
	// in pixelcanvas coordinates (the actual region of the pixel canvas that
	// will be drawn).

	Vector<Int> actPixBlc(2), actPixTrc(2);
	Vector<Int> canvblc(2), canvtrc(2);
	canvblc(0)=itsCanvasXOffset + itsCanvasDrawXOffset;
	canvblc(1)=itsCanvasYOffset + itsCanvasDrawYOffset;
	canvtrc(0)=canvblc(0) + itsCanvasDrawXSize - 1;
	canvtrc(1)=canvblc(1) + itsCanvasDrawYSize - 1;
	for (Int i=0; i<2; i++) {
		actPixBlc(i)= max(canvblc(i),min(canvtrc(i),Int( reqPixBlc(i) +.5  )));
		actPixTrc(i)= max(canvblc(i),min(canvtrc(i),Int( reqPixTrc(i) +.5  )));
	}


	// 4. resample the image

	// dk note (5/04): All the WC's scaling is clumsy and awkward, partly
	// because the parameters are awkwardly defined.
	// I've changed the resampler interface so that it is passed
	// the location of corners of its _output_ within the  _input data_
	// (as the interpolaters need), instead of vice versa.
	// This also fixes the bug which was occurring when the input data was
	// only 1 pixel wide (blc==trc, 0 dividing 0), which was the original
	// object of this exercise.
	// Other similar revisions (including eliminating 'trc' everywhere, in
	// favor of 1-beyond-trc, or size) will have to wait for some other day.


	Int smplSh0 = actPixTrc(0)+1 - actPixBlc(0),
			smplSh1 = actPixTrc(1)+1 - actPixBlc(1);
	if(smplSh0<=0 || smplSh1<=0) return;		// no pixels to draw.


	Matrix<Float> sampledImage(smplSh0, smplSh1);


	// Linear scaling: transform the corners of the output (sampled screen
	// pixel) matrix, from pixelcanvas (screen) coordinates, to locations
	// within the input data matrix, for the resampler/interpolator.

	Vector<Float> smplBlc(2);	// location of sampledImage(0,0) and
	Vector<Float> smplTrc(2);	// sampledImage(trc) within data Matrix.

	for (Int i=0; i<2; i++) {
		Double s1 = reqPixTrc[i],        s0 = reqPixBlc[i];
		Double d1 = data.shape()(i)-1.,  d0 = 0.;
		if(usePixelEdges) {
			d1 +=.5;
			d0 -=.5;
		};
		// (Which data corner coordinates do we have here:
		// centers or edges? (jeez I hate that stuff...)).
		Double dPerS = (d1-d0) / (s1-s0);
		// 'increment' within input data, per screen pixel.
		smplBlc[i] = (actPixBlc[i] - s0)*dPerS + d0;
		smplTrc[i] = (actPixTrc[i] - s0)*dPerS + d0;
	}


	(*itsResampleHandler)(sampledImage, data, smplBlc, smplTrc);


	// 5. rescale the image
	Matrix<uInt> scaledImage(sampledImage.shape());
	itsDataScaleHandler->setRangeMax(rMax);
	itsDataScaleHandler->setDomainMinMax(itsDataMin, itsDataMax);
	(*itsDataScaleHandler)(scaledImage, sampledImage);

	// 6. drop the resampled, rescaled image into the component buffer
	for (Int i=0; i<2; i++) {
		actPixBlc(i) -= canvblc(i);
		actPixTrc(i) -= canvblc(i);
	}

	Matrix<uInt> target(itsCanvasDrawXSize, itsCanvasDrawYSize);
	target = 0;

	target( IPosition(actPixBlc), IPosition(actPixTrc) ) =  scaledImage;

	// 7. store this in the PixelCanvas...
	itsPixelCanvas->drawImage(target, canvblc(0), canvblc(1), colorcomponent);

}


void WorldCanvas::drawImage(const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Complex> &data,
		const Display::ColorComponent &colorcomponent,
		const Bool usePixelEdges) {
	switch (itsComplexToRealMethod) {
	case Display::Phase:
		drawImage(blc, trc, phase(data), colorcomponent, usePixelEdges);
		break;
	case Display::Real:
		drawImage(blc, trc, real(data), colorcomponent, usePixelEdges);
		break;
	case Display::Imaginary:
		drawImage(blc, trc, imag(data), colorcomponent, usePixelEdges);
		break;
	default:
		drawImage(blc, trc, amplitude(data), colorcomponent, usePixelEdges);
		break;
	}
}


bool WorldCanvas::drawVectorMap(const Vector<Double>& blc,
		const Vector<Double>& trc,
		const Matrix<Complex>& data,
		const Matrix<Bool>& mask,
		Float angleConversionFactor,
		Float phasePolarity,
		Bool debias, Float variance,
		Int xPixelInc, Int yPixelInc,
		Float scale, Bool arrow,
		Float barb, Float rotation,
		Double xWorldInc, Double yWorldInc,
		const Bool usePixelEdges) {
	Matrix<Float> a(amplitude(data));
	Matrix<Float> p(phase(data));
	//
	angleConversionFactor = 1.0;
	return drawVectorMap(blc, trc, a, p, mask, angleConversionFactor,
			phasePolarity, debias,
			variance, xPixelInc, yPixelInc, scale, arrow,
			barb, rotation, xWorldInc, yWorldInc, usePixelEdges);
}

bool WorldCanvas::drawVectorMap(const Vector<Double>& blc,
		const Vector<Double>& trc,
		const Matrix<Float>& data,
		const Matrix<Bool>& mask,
		Float angleConversionFactor,
		Float phasePolarity,
		Bool debias, Float variance,
		Int xPixelInc, Int yPixelInc,
		Float scale, Bool arrow,
		Float barb, Float rotation,
		Double xWorldInc, Double yWorldInc,
		const Bool usePixelEdges) {
	Matrix<Float> amp;
	return drawVectorMap(blc, trc, amp, data, mask, angleConversionFactor,
			phasePolarity, debias,
			variance, xPixelInc, yPixelInc, scale, arrow,
			barb, rotation, xWorldInc, yWorldInc, usePixelEdges);
}

bool WorldCanvas::drawVectorMap(const Vector<Double>& blc,
		const Vector<Double>& trc,
		const Matrix<Float>& amp,
		const Matrix<Float>& posang,
		const Matrix<Bool>& mask,
		Float angleConversionFactor,
		Float phasePolarity,
		Bool debias, Float variance,
		Int xPixelInc, Int yPixelInc,
		Float scale, Bool arrow,
		Float barb, Float rotation,
		Double xWorldInc, Double yWorldInc,
		const Bool usePixelEdges)
//
// If amplitude matrix is of shape 0, amplitude is unity
// position angle in radians = posang * angleConversionFactor
// rotation in radians
//
//
{
	Bool noAmp = amp.shape()==IPosition(2,0,0);
	AlwaysAssert(noAmp || (amp.shape()==posang.shape()), AipsError);
	//
	IPosition shp = posang.shape();
	const uInt nWorld = blc.nelements();
	//
	Vector<Double> linBlc(2), linTrc(2);
	if (!worldToLin(linBlc, blc)) {
		return false;
	}
	//
	if (!worldToLin(linTrc, trc)) {
		return false;
	}

	// Find maximum amplitude in array
	// Autoscale means longest vector is 5% size of smallest window dimension.
	// The user has an extra scale factor they can apply (scale)

	Bool useMask = mask.nelements()>0;
	Float dMax = 0.0;
	if (noAmp) {
		dMax = 1.0;
	} else {
		if (useMask) {
			MaskedArray<Float> mData(amp, mask, True);
			dMax = max(mData);
		} else {
			dMax = max(amp);
		}
	}
	Float maxLength  = min(abs(xWorldInc*shp(0)), abs(yWorldInc*shp(1))) / 20.0;
	Float scale2 = abs(scale) * maxLength / dMax;


	// Deal with pixel treatement and generate increment and offset to
	// map from Lattice pixel coordinate to linear coordinate. There
	// is no DL method to do this otherwise that I can find.

	Float dx = 0.0;
	Float dy  = 0.0;
	Float xOff = 0.0;
	Float yOff = 0.0;
	if (usePixelEdges) {

		// All of edge pixels is seen

		dx  = (linTrc(0) - linBlc(0)) / Float(shp(0));
		dy  = (linTrc(1) - linBlc(1)) / Float(shp(1));
		xOff = dx / 2.0;
		yOff = dy / 2.0;
	} else {

		// Edge pixels are only seen from centre inwards

		dx  = (linTrc(0) - linBlc(0)) / Float(shp(0) - 1);
		dy  = (linTrc(1) - linBlc(1)) / Float(shp(1) - 1);
	}
	//
	Double fac = abs(yWorldInc / xWorldInc * dy / dx);
	scale2 /= 2.0;
	Float ddx, ddy;
	Float a, pa, t;
	uInt countFail = 0;
	//
	Vector<Double> worldPos(nWorld);
	Vector<Double> worldHead(nWorld);
	Vector<Double> worldTail(nWorld);
	Vector<Double> linPos(2);
	Vector<Double> linHead(2);
	Vector<Double> linTail(2);
	//
	if (arrow) cpgsah(2, 45.0, barb);

	// Loop through array.  Find linear world coordinate, and offsets
	// for head and tail.

	for (Int j=0; j<shp(1); j+=xPixelInc) {
		for (Int i=0; i<shp(0); i+=yPixelInc) {
			if (!useMask || (useMask && mask(i,j))) {

				// Amplitude and phase

				if (noAmp) {
					a = 1.0;
				} else {
					a = amp(i,j);
					if (debias) {
						t = a * a;
						if (t-variance >=0) {
							a = sqrt(t-variance);
						} else {
							countFail++;
							a = abs(t);
						}
					}
				}
				pa = angleConversionFactor * posang(i,j) / phasePolarity;

				// Linear coordinate of array pixel

				linPos(0) = xOff + linBlc(0) + dx*i;
				linPos(1) = yOff + linBlc(1) + dy*j;

				// Convert to world coordinate.

				if (!linToWorld(worldPos, linPos)) {
					return false;
				}

				// Work out vector start and end in world coordinates.  This ensures
				// we preserve position angles correctly in the translation from
				// world to linear coordinates.

				ddx = scale2 * a * sin(pa+rotation) * fac;
				ddy = scale2 * a * cos(pa+rotation);
				//
				worldTail(0) = worldPos(0) - ddx;
				worldTail(1) = worldPos(1) - ddy;
				worldHead(0) = worldPos(0) + ddx;
				worldHead(1) = worldPos(1) + ddy;

				// Convert back to linear

				if (!worldToLin(linHead, worldHead)) {
					return false;
				}
				if (!worldToLin(linTail, worldTail)) {
					return false;
				}

				// Draw

				if (arrow) {
					cpgarro (Float(linTail(0)), Float(linTail(1)), Float(linHead(0)), Float(linHead(1)));
				} else {
					cpgmove (Float(linTail(0)), Float(linTail(1)));
					cpgdraw(Float(linHead(0)), Float(linHead(1)));
				}
			}
		}
	}
	//
	if (countFail > 0) {
		LogIO os(LogOrigin("WorldCanvas", "drawVectorMap", WHERE));
		os << LogIO::WARN << "Problems were encountered debiasing the amplitude. This" << endl;
		os << "means the variance was too large (which would make imaginary amplitudes)" << endl;
		os << "This happened for " << countFail << " pixels, for which the undebiased amplitude was subsituted" << endl;
		os.post();
	}
	return true;
}


bool WorldCanvas::drawMarkerMap(const Vector<Double>& blc,
		const Vector<Double>& trc,
		const Matrix<Complex>& data,
		const Matrix<Bool>& mask,
		Int xPixelInc, Int yPixelInc,
		Float scale, Double xWorldInc, Double yWorldInc,
		const String& markerType, Bool usePixelEdges) {
	switch (itsComplexToRealMethod) {
	case Display::Phase:
		return drawMarkerMap(blc, trc, phase(data), mask, xPixelInc, yPixelInc,
				scale, xWorldInc, yWorldInc, markerType, usePixelEdges);
		break;
	case Display::Real:
		return drawMarkerMap(blc, trc, real(data), mask, xPixelInc, yPixelInc,
				scale, xWorldInc, yWorldInc, markerType, usePixelEdges);

		break;
	case Display::Imaginary:
		return drawMarkerMap(blc, trc, imag(data), mask, xPixelInc, yPixelInc,
				scale, xWorldInc, yWorldInc, markerType, usePixelEdges);

		break;
	default:
		return drawMarkerMap(blc, trc, amplitude(data), mask, xPixelInc, yPixelInc,
				scale, xWorldInc, yWorldInc, markerType, usePixelEdges);
		break;
	}
}

bool WorldCanvas::drawMarkerMap(const Vector<Double>& blc,
		const Vector<Double>& trc,
		const Matrix<Float>& data,
		const Matrix<Bool>& mask,
		Int xPixelInc, Int yPixelInc,
		Float scale, Double /*xWorldInc*/, Double /*yWorldInc*/,
		const String& markerType, Bool usePixelEdges) {
	const IPosition& shp = data.shape();
	const uInt nWorld = blc.nelements();
	//
	Vector<Double> linBlc(2), linTrc(2);
	if (!worldToLin(linBlc, blc)) {
		return false;
	}
	//
	if (!worldToLin(linTrc, trc)) {
		return false;
	}


	// Deal with pixel treatement and generate increment and offset to
	// map from Lattice pixel coordinate to linear coordinate. There
	// is no DL method to do this otherwise that I can find.

	Float dx = 0.0;
	Float dy  = 0.0;
	Float xOff = 0.0;
	Float yOff = 0.0;
	if (usePixelEdges) {

		// All of edge pixels is seen

		dx  = (linTrc(0) - linBlc(0)) / Float(shp(0));
		dy  = (linTrc(1) - linBlc(1)) / Float(shp(1));
		xOff = dx / 2.0;
		yOff = dy / 2.0;
	} else {

		// Edge pixels are only seen from centre inwards

		dx  = (linTrc(0) - linBlc(0)) / Float(shp(0) - 1);
		dy  = (linTrc(1) - linBlc(1)) / Float(shp(1) - 1);
	}

	// We autoscale so that the width of the shape is 5% of the smallest
	// window dimension.   The shape stays fixed in pixel canvas space
	// so that a square is always a square...
	// The user has an extra scale factor they can apply (scale)

	uInt xSize = canvasDrawXSize();
	uInt ySize = canvasDrawYSize();
	//
	Float pix = 0.05 * Float(min(xSize,ySize));              // Number of screen pixels for default size
	Float linX = pix * abs(linTrc(0) - linBlc(0)) / xSize;   // Linear coordinate size corresponding to
	Float linY = pix * abs(linTrc(1) - linBlc(1)) / ySize;   // pix screen pixels
	//
	Float dMax = max(abs(itsDataMin), abs(itsDataMax));
	linX /= dMax;                                            // Scale max value to this size
	linY /= dMax;
	//
	Float a, linPosX, linPosY, ddx, ddy;
	Vector<Double> worldPos(nWorld);
	Vector<Double> linPos(2);
	Bool useMask = mask.nelements()>0;

	// When we have other shape, make these with new according to required size

	Float linPolyX[4], linPolyY[4];

	// Loop through array and draw markers.

	for (Int j=0; j<shp(1); j+=xPixelInc) {
		for (Int i=0; i<shp(0); i+=yPixelInc) {
			if (!useMask || (useMask && mask(i,j))) {
				a = scale*data(i,j);

				// Linear coordinate of array pixel

				linPosX = xOff + linBlc(0) + dx*i;
				linPosY = yOff + linBlc(1) + dy*j;

				// Work out polygon in linear coordinates for plotting. Regardless of the
				// aspect ratio, we want to see symmetric shapes

				ddx = a * linX / 2.0;
				ddy = a * linY / 2.0;

				// Draw
				if (markerType=="square") {

					// Presently only squares are implemented.

					linPolyX[0] = linPosX - ddx;
					linPolyX[1] = linPosX + ddx;
					linPolyX[2] = linPolyX[1];
					linPolyX[3] = linPolyX[0];
					//
					linPolyY[0] = linPosY - ddy;
					linPolyY[1] = linPolyY[0];
					linPolyY[2] = linPosY + ddy;
					linPolyY[3] = linPolyY[2];
					//
					if (data(i,j)>=0) {
						cpgsfs(1);                    // Filled
					} else {
						cpgsfs(2);                    // Outline
					}
					cpgpoly(4, linPolyX, linPolyY);
				}
			}
		}
	}
	return true;
}



// Flush the component images.
void WorldCanvas::flushComponentImages() {
	itsPixelCanvas->flushComponentBuffers();
}

// Buffer memory exchanges.
void WorldCanvas::copyBackBufferToFrontBuffer() {
	itsPixelCanvas->
	copyBackBufferToFrontBuffer((Int)canvasXOffset(),
			(Int)canvasYOffset(),
			(Int)canvasXOffset() + canvasXSize()-1,
			(Int)canvasYOffset() + canvasYSize()-1);
}
void WorldCanvas::copyFrontBufferToBackBuffer() {
	itsPixelCanvas->
	copyFrontBufferToBackBuffer((Int)canvasXOffset(),
			(Int)canvasYOffset(),
			(Int)canvasXOffset()+canvasXSize()-1,
			(Int)canvasYOffset()+canvasYSize()-1);
}
void WorldCanvas::swapBuffers() {
	itsPixelCanvas->
	swapBuffers((Int)canvasXOffset(),
			(Int)canvasYOffset(),
			(Int)canvasXOffset()+canvasXSize()-1,
			(Int)canvasYOffset()+canvasYSize()-1);
}

// Set the zoom rectangle.
void WorldCanvas::setZoomRectangleLCS(const Vector<Double> &blc,
		const Vector<Double> &trc) {
	String attString("manualZoomBlc");
	Attribute blcatt(attString, blc);
	setAttribute(blcatt);
	attString = "manualZoomTrc";
	Attribute trcatt(attString, trc);
	setAttribute(trcatt);
}

// Move the zoom rectangle, ie. pan.
void WorldCanvas::moveZoomRectangleLCS(Double dx, Double dy) {
	itsLinXMin += dx;
	itsLinYMin += dy;
	itsLinXMax += dx;
	itsLinYMax += dy;
}

// Reset zoom to 1:1.
void WorldCanvas::resetZoomRectangle() {
	itsLinXMin = itsLinXMinLimit;
	itsLinYMin = itsLinYMinLimit;
	itsLinXMax = itsLinXMaxLimit;
	itsLinYMax = itsLinYMaxLimit;
}

// Set the range of the linear coordinate system.
void WorldCanvas::setLinearCoordinateSystem(const Vector<Double> &blc,
		const Vector<Double> &trc,
		Bool resetZoom) {
	itsLinXMinLimit = blc(0);
	itsLinYMinLimit = blc(1);
	itsLinXMaxLimit = trc(0);
	itsLinYMaxLimit = trc(1);
	if (resetZoom) {
		resetZoomRectangle();
	}
}

// Set and retrieve the DisplayCoordinateSystem of this WorldCanvas.
void WorldCanvas::setCoordinateSystem(const DisplayCoordinateSystem &csys) {
	if (itsCoordinateSystem) {
		delete itsCoordinateSystem;
	}
	if (&csys) {
		itsCoordinateSystem = new DisplayCoordinateSystem(csys);
	} else {
		itsCoordinateSystem = 0;
	}
}
const DisplayCoordinateSystem &WorldCanvas::coordinateSystem() const {
	return *itsCoordinateSystem;
}

// (private) Support for construction.
void WorldCanvas::ctorInit() {
	itsSizeControlHandler = new DefaultWCSizeControlHandler;
	itsCoordinateHandler = new DefaultWCCoordinateHandler;
	itsResampleHandler = new WCSimpleResampleHandler(Interpolate2D::NEAREST);
	itsDataScaleHandler = new WCLinearScaleHandler;

	itsOwnSizeControlHandler = True;
	itsOwnCoordinateHandler = True;
	itsOwnResampleHandler = True;
	itsOwnDataScaleHandler = True;

	itsREHListIter = new ListIter<DisplayEH *>(&itsRefreshEHList);
	itsPEHListIter = new ListIter<WCPositionEH *>(&itsPositionEHList);
	itsMEHListIter = new ListIter<WCMotionEH *>(&itsMotionEHList);

	itsPixelCanvas->addRefreshEventHandler(*this);
	itsPixelCanvas->addMotionEventHandler(*this);
	itsPixelCanvas->addPositionEventHandler(*this);

	// These are inter-dependent attributes!!!  Can't use
	// attributes for these variables.
	attributes.add("canvasDrawXOffset", &itsCanvasDrawXOffset, 0, False, True);
	attributes.add("canvasDrawYOffset", &itsCanvasDrawYOffset, 0, False, True);
	attributes.add("canvasDrawXSize", &itsCanvasDrawXSize, 0, False, True);
	attributes.add("canvasDrawYSize", &itsCanvasDrawYSize, 0, False, True);

	attributes.add("linXMin", &itsLinXMin, 0.0, False, True);
	attributes.add("linXMax", &itsLinXMax, 0.0, False, True);
	attributes.add("linYMin", &itsLinYMin, 0.0, False, True);
	attributes.add("linYMax", &itsLinYMax, 0.0, False, True);

	attributes.add("linXMinLimit", &itsLinXMinLimit, 0.0, False, True);
	attributes.add("linXMaxLimit", &itsLinXMaxLimit, 0.0, False, True);
	attributes.add("linYMinLimit", &itsLinYMinLimit, 0.0, False, True);
	attributes.add("linYMaxLimit", &itsLinYMaxLimit, 0.0, False, True);

	attributes.add("dataMin", &itsDataMin, 0.0, False, True);
	attributes.add("dataMax", &itsDataMax, 0.0, False, True);

	setDefaultOptions();
}

// (private) Update canvas sizes and offsets.
void WorldCanvas::updateCanvasSizesOffsets() {
	Int pw=pixelCanvas()->width(), ph=pixelCanvas()->height();


	itsCanvasXOffset = uInt(max(0., .5+ itsFracXOffset*pw ));
	itsCanvasYOffset = uInt(max(0., .5+ itsFracYOffset*ph ));
	itsCanvasXSize   = uInt(max(0., .5+ (itsFracXOffset+itsFracXSize)*pw
			-itsCanvasXOffset ));
	itsCanvasYSize   = uInt(max(0., .5+ (itsFracYOffset+itsFracYSize)*ph
			-itsCanvasYOffset ));

	itsCanvasDrawXSize = itsCanvasXSize;
	itsCanvasDrawYSize = itsCanvasYSize;
}

void WorldCanvas::updateFracSizesOffsets() {
	itsFracXSize = Double(itsCanvasXSize) / Double(pixelCanvas()->width());
	itsFracYSize = Double(itsCanvasYSize) / Double(pixelCanvas()->height());
	itsFracXOffset = Double(itsCanvasXOffset) / Double(pixelCanvas()->width());
	itsFracYOffset = Double(itsCanvasYOffset) / Double(pixelCanvas()->height());
}

// (private) Convert the given coordinate/s to pixel coordinate/s.
Bool WorldCanvas::castingConversion(Vector<Int> &pixelpt,
		const Vector<Double> &worldpt,
		const Bool &linear) {
	Vector<Double> pixelLoc(2);
	Bool succ;
	if (linear) {
		succ = linToPix(pixelLoc, worldpt);
	} else {
		succ = worldToPix(pixelLoc, worldpt);
	}
	if (succ) {
		pixelpt.resize(pixelLoc.shape());
		ArrayPositionIterator iter(pixelLoc.shape(), 0);
		while (!iter.pastEnd()) {
			pixelpt(iter.pos()) = Int(pixelLoc(iter.pos()) + 0.5);
			iter.next();
		}
		return True;
	} else {
		return False;
	}
}
Bool WorldCanvas::castingConversion(Matrix<Int> &pixelpts,
		const Matrix<Double> &worldpts,
		const Bool &linear) {
	Vector<Bool> failures(worldpts.nrow(), False);
	Matrix<Double> pixelLocs(worldpts.nrow(), 2);
	Bool succ;
	if (linear) {
		succ = linToPix(pixelLocs, failures, worldpts);
	} else {
		succ = worldToPix(pixelLocs, failures, worldpts);
	}
	if (succ) {
		pixelpts.resize(pixelLocs.shape());
		ArrayPositionIterator iter(pixelLocs.shape(), 0);
		while (!iter.pastEnd()) {
			pixelpts(iter.pos()) = Int(pixelLocs(iter.pos()) + 0.5);
			iter.next();
		}
		return True;
	} else {
		// partial success is possible:
		uInt count = 0;
		for (uInt i = 0; i < failures.nelements(); i++) {
			if (!failures(i)) {
				count++;
			}
		}
		if (count) {
			pixelpts.resize(count, 2);
			uInt j = 0;
			for (uInt i = 0; (i < failures.nelements()) && (j < count); i++) {
				if (!failures(i)) {
					pixelpts(j, 0) = Int(pixelLocs(i, 0) + 0.5);
					pixelpts(j, 1) = Int(pixelLocs(i, 1) + 0.5);
					j++;
				}
			}
			return True;
		} else {
			// no successful conversions at all!
			return False;
		}
	}
}
Bool WorldCanvas::castingConversion(Matrix<Float> &pixelpts,
		const Matrix<Double> &worldpts,
		const Bool &linear) {
	Vector<Bool> failures(worldpts.nrow(), False);
	Matrix<Double> pixelLocs(worldpts.nrow(), 2);
	Bool succ;
	if (linear) {
		succ = linToPix(pixelLocs, failures, worldpts);
	} else {
		succ = worldToPix(pixelLocs, failures, worldpts);
	}
	if (succ) {
		pixelpts.resize(pixelLocs.shape());
		ArrayPositionIterator iter(pixelLocs.shape(), 0);
		while (!iter.pastEnd()) {
			pixelpts(iter.pos()) = Float(pixelLocs(iter.pos()));
			iter.next();
		}
		return True;
	} else {
		return False;
	}
}

Bool WorldCanvas::castingClippingConversion(Vector<Int> &pixelx,
		Vector<Int> &pixely,
		Vector<Bool> &validConversions,
		const Vector<Float> &worldx,
		const Vector<Float> &worldy,
		const Bool /*linear*/) {
	if (worldx.nelements() != worldy.nelements()) {
		throw(AipsError("Non-conformant input to WorldCanvas::castingConversion"));
		//return False;
	}

	Vector<Double> worldco(2), linearco(2), pixelco(2);
	Double clinxmin = min(itsLinXMin, itsLinXMax);
	Double clinxmax = max(itsLinXMin, itsLinXMax);
	Double clinymin = min(itsLinYMin, itsLinYMax);
	Double clinymax = max(itsLinYMin, itsLinYMax);

	uInt count = 0;
	pixelx.resize(worldx.shape());
	pixely.resize(worldy.shape());
	validConversions.resize(worldx.shape());
	validConversions = False;

	for (uInt i = 0; i < worldx.nelements(); i++) {
		worldco(0) = worldx(i);
		worldco(1) = worldy(i);
		if (!worldToLin(linearco, worldco)) {
			// this conversion failed, continue with next
			continue;
		}
		if ((linearco(0) < clinxmin) || (linearco(0) > clinxmax) ||
				(linearco(1) < clinymin) || (linearco(1) > clinymax)) {
			// conversion is outside linear extent of WorldCanvas, continue with next
			continue;
		}
		if (!linToPix(pixelco, linearco)) {
			// this conversion failed, continue with next
			continue;
		}
		// ok - so lets cast to Int and store it
		pixelx(count) = Int(pixelco(0) + 0.5);
		pixely(count) = Int(pixelco(1) + 0.5);
		validConversions(i) = True;
		count++;
	}
	pixelx.resize(count, True);
	pixely.resize(count, True);
	if (count>0) {
		return True;
	} else {
		return False;
	}
}


// Trim and resample an image.
void WorldCanvas::trimAndResampleImage(Vector<Double> &drawBlc,
		Vector<Double> &drawTrc,
		Matrix<Float> &sampledImage,
		const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Bool &usePixelEdges) {

	Vector<Double> reqPixBlc(2), reqPixTrc(2);
	worldToPix(reqPixBlc, blc);
	worldToPix(reqPixTrc, trc);


	// 1. Clip input to draw area (and round to whole screen pixels)

	// 'req' = corners of input array, in pixelcanvas ('screen') coordinates.
	// 'act' (or 'sampled', 'draw') = corners of output array, originally
	// in pixelcanvas coordinates (the actual region of the pixel canvas that
	// will be drawn).

	Vector<Int> actPixBlc(2), actPixTrc(2);
	Vector<Int> canvblc(2), canvtrc(2);
	canvblc(0)=itsCanvasXOffset + itsCanvasDrawXOffset;
	canvblc(1)=itsCanvasYOffset + itsCanvasDrawYOffset;
	canvtrc(0)=canvblc(0) + itsCanvasDrawXSize - 1;
	canvtrc(1)=canvblc(1) + itsCanvasDrawYSize - 1;

	for (Int i=0; i<2; i++) {
		actPixBlc(i)= max(canvblc(i),min(canvtrc(i),Int( reqPixBlc(i) +.5  )));
		actPixTrc(i)= max(canvblc(i),min(canvtrc(i),Int( reqPixTrc(i) +.5  )));
	}

	// (Return parameters: Corners of clipped area actually to be drawn).

	drawBlc.resize(2);	// (silly to translate to world here...)
	drawTrc.resize(2);	// (they are translated right back by caller...)
	Vector<Double> outblc(2), outtrc(2);
	for (Int i=0; i<2; i++) {
		outblc(i)=Double(actPixBlc(i));
		outtrc(i)=Double(actPixTrc(i));
	}
	pixToWorld(drawBlc, outblc);
	pixToWorld(drawTrc, outtrc);


	// 2. resample the image

	Int smplSh0 = actPixTrc(0)+1 - actPixBlc(0),
			smplSh1 = actPixTrc(1)+1 - actPixBlc(1);
	if(smplSh0<=0 || smplSh1<=0) {
		sampledImage.resize(0, 0);
		return;
	}		// no pixels to draw.


	sampledImage.resize(smplSh0, smplSh1);


	// Linear scaling: transform the corners of the output (sampled screen
	// pixel) matrix, from pixelcanvas (screen) coordinates, to locations
	// within the input data matrix, for the resampler/interpolator.

	Vector<Float> smplBlc(2);	// location of sampledImage(0,0) and
	Vector<Float> smplTrc(2);	// sampledImage(trc) within data Matrix.

	for (Int i=0; i<2; i++) {
		Double s1 = reqPixTrc[i],        s0 = reqPixBlc[i];
		Double d1 = data.shape()(i)-1.,  d0 = 0.;
		if(usePixelEdges) {
			d1 +=.5;
			d0 -=.5;
		};
		Double dPerS = (d1-d0) / (s1-s0);
		// 'increment' within input data, per screen pixel.
		smplBlc[i] = (actPixBlc[i] - s0)*dPerS + d0;
		smplTrc[i] = (actPixTrc[i] - s0)*dPerS + d0;
	}

	// Resample data

	(*itsResampleHandler)(sampledImage, data, smplBlc, smplTrc);

}

// Trim and resample an image and mask.
void WorldCanvas::trimAndResampleImage(Vector<Double> &drawBlc,
		Vector<Double> &drawTrc,
		Matrix<Float> &sampledImage,
		Matrix<Bool> &sampledMask,
		const Vector<Double> &blc,
		const Vector<Double> &trc,
		const Matrix<Float> &data,
		const Matrix<Bool> &mask,
		const Bool &usePixelEdges) {

	Vector<Double> reqPixBlc(2), reqPixTrc(2);
	worldToPix(reqPixBlc, blc);
	worldToPix(reqPixTrc, trc);


	// 1. Clip input to draw area (and round to whole screen pixels)

	// 'req' = corners of input array, in pixelcanvas ('screen') coordinates.
	// 'act' (or 'sampled', 'scaled') = corners of output array, originally
	// in pixelcanvas coordinates (the actual region of the pixel canvas that
	// will be drawn).

	Vector<Int> actPixBlc(2), actPixTrc(2);
	Vector<Int> canvblc(2), canvtrc(2);
	canvblc(0)=itsCanvasXOffset + itsCanvasDrawXOffset;
	canvblc(1)=itsCanvasYOffset + itsCanvasDrawYOffset;
	canvtrc(0)=canvblc(0) + itsCanvasDrawXSize - 1;
	canvtrc(1)=canvblc(1) + itsCanvasDrawYSize - 1;
	for (Int i=0; i<2; i++) {
		actPixBlc(i)= max(canvblc(i),min(canvtrc(i),Int( reqPixBlc(i) +.5  )));
		actPixTrc(i)= max(canvblc(i),min(canvtrc(i),Int( reqPixTrc(i) +.5  )));
	}

	// (Return parameters: Corners of clipped area actually to be drawn).

	drawBlc.resize(2);	// (silly to translate to world here...)
	drawTrc.resize(2);	// (they are translated right back by caller...)
	Vector<Double> outblc(2), outtrc(2);
	for (Int i=0; i<2; i++) {
		outblc(i)=Double(actPixBlc(i));
		outtrc(i)=Double(actPixTrc(i));
	}
	pixToWorld(drawBlc, outblc);
	pixToWorld(drawTrc, outtrc);


	// 2. resample the image and mask

	Int smplSh0 = actPixTrc(0)+1 - actPixBlc(0),
			smplSh1 = actPixTrc(1)+1 - actPixBlc(1);
	if(smplSh0<=0 || smplSh1<=0) {
		sampledImage.resize(0, 0);
		sampledMask.resize(0, 0);
		return;
	}		// no pixels to draw.


	sampledImage.resize(smplSh0, smplSh1);
	sampledMask.resize(smplSh0, smplSh1);


	// Linear scaling: transform the corners of the output (sampled screen
	// pixel) matrix, from pixelcanvas (screen) coordinates, to locations
	// within the input data matrix, for the resampler/interpolator.

	Vector<Float> smplBlc(2);	// location of sampledImage(0,0) and
	Vector<Float> smplTrc(2);	// sampledImage(trc) within data Matrix.

	for (Int i=0; i<2; i++) {
		Double s1 = reqPixTrc[i],        s0 = reqPixBlc[i];
		Double d1 = data.shape()(i)-1.,  d0 = 0.;
		if(usePixelEdges) {
			d1 +=.5;
			d0 -=.5;
		};
		Double dPerS = (d1-d0) / (s1-s0);
		// 'increment' within input data, per screen pixel.
		smplBlc[i] = (actPixBlc[i] - s0)*dPerS + d0;
		smplTrc[i] = (actPixTrc[i] - s0)*dPerS + d0;
	}

	// Resample data & mask

	(*itsResampleHandler)(sampledImage, sampledMask, data, mask, smplBlc, smplTrc);
}


// Map color-indexed image to colors, and draw on PC.
void WorldCanvas::mapToColorAndDrawImage(const Vector<Int> &blc,
		const Matrix<uInt> &scaledImage) {

	Matrix<uInt> colorImage(scaledImage.shape());
	itsPixelCanvas->mapToColor(colorImage, scaledImage, True);
	itsPixelCanvas->drawImage(colorImage, blc(0), blc(1));
}

Matrix<uInt> WorldCanvas::mapToColor( const Matrix<uInt> & scaledImage ){
	Matrix<uInt> colorImage(scaledImage.shape());
	itsPixelCanvas->mapToColor(colorImage, scaledImage, True);
	return colorImage;
}



// Map color-indexed image to colors, and draw on PC  (masked version).
void WorldCanvas::mapToColorAndDrawImage(const Vector<Int> &blc,
		const Matrix<uInt> &scaledImage,
		const Matrix<Bool> &mask,
		Bool opaqueMask) {

	Matrix<uInt> colorImage(scaledImage.shape());
	itsPixelCanvas->mapToColor(colorImage, scaledImage, True);
	itsPixelCanvas->drawImage(blc(0), blc(1), colorImage, mask, opaqueMask);
}


// Draw a set of Colormapped points.
Bool WorldCanvas::mapToColorAndDrawPoints(const Matrix<Double> &points,
		const Vector<uInt> &scaledValues,
		const Bool &linear) {
	// 1. convert world/linear to pixel
	Matrix<Int> pixelpts;
	if (!castingConversion(pixelpts, points, linear)) {
		return False;
	}
	Vector<uInt> colorValues(scaledValues.shape());
	itsPixelCanvas->mapToColor(colorValues, scaledValues, True);
	itsPixelCanvas->drawColoredPoints(pixelpts, colorValues);
	return True;
}

// Draw a set of Colormapped ellipses.
Bool WorldCanvas::mapToColorAndDrawEllipses(const Matrix<Double> &centres,
		const Vector<Float> &smajor,
		const Vector<Float> &sminor,
		const Vector<Float> &pangle,
		const Vector<uInt> scaledValues,
		const Float &scale,
		const Bool &outline,
		const Bool &linear) {
	// 1. convert world/linear to pixel
	Matrix<Float> pixelpts;
	if (!castingConversion(pixelpts, centres, linear)) {
		return False;
	}

	// 2. get true pixelcanvas color values
	Vector<uInt> colorValues(scaledValues.shape());
	itsPixelCanvas->mapToColor(colorValues, scaledValues, True);

	// 3. draw the ellipses
	itsPixelCanvas->drawColoredEllipses(pixelpts, smajor, sminor,
			pangle, colorValues,
			scale, outline);
	return True;
}

Bool WorldCanvas::inPC(Int x, Int y) {
	return ( itsPixelCanvas!=0 &&
			x >= 0 && x < Int(itsPixelCanvas->width()) &&
			y >= 0 && y < Int(itsPixelCanvas->height()) );
}

// Adding, removing and querying restrictions
void WorldCanvas::setRestriction(const Attribute &restriction) {
	itsRestrictions.set(restriction);
}
void WorldCanvas::setRestrictions(const AttributeBuffer &resBuff) {
	itsRestrictions.set(resBuff);
}
Bool WorldCanvas::existRestriction(const String &name) const {
	return itsRestrictions.exists(name);
}
void WorldCanvas::removeRestriction(const String &restrictionName) {
	itsRestrictions.remove(restrictionName);
}
void WorldCanvas::removeRestrictions() {
	itsRestrictions.clear();
}
Bool WorldCanvas::matchesRestriction(const Attribute
		&restriction) const {
	return itsRestrictions.matches(restriction);
}
Bool WorldCanvas::matchesRestrictions(const AttributeBuffer
		&buffer) const {
	return itsRestrictions.matches(buffer);
}
const AttributeBuffer *WorldCanvas::restrictionBuffer() const {
	return &itsRestrictions;
}

int WorldCanvas::zIndex( ) const {
	int result = 0;
	if ( itsRestrictions.exists("zIndex")) {
		itsRestrictions.getValue("zIndex", result);
	}
	return result;
}

Vector<String> WorldCanvas::worldAxisNames() const {
	Vector<String> axisNames;
	if (csMaster() != 0) axisNames = csMaster()->worldAxisNames();
	return axisNames;
}


Vector<String> WorldCanvas::worldAxisUnits() const {
	Vector<String> axisUnits;
	if (csMaster() != 0) axisUnits = csMaster()->worldAxisUnits();
	return axisUnits;
}

const std::list<DisplayData*> &WorldCanvas::displaylist( ) const {
	static std::list<DisplayData*> empty;
	if ( csMaster( ) == 0 ) return empty;
	const WorldCanvasHolder *wch = csMaster( )->findHolder(this);
	if ( wch == 0 ) return empty;
	return wch->displaylist( );
}



} //# NAMESPACE CASA - END

