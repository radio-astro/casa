//# MultiWCTool.cc: base class for MultiWorldCanvas event-based tools
//# Copyright (C) 2000,2001,2002
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
#include <display/Display/PanelDisplay.h>
#include <display/DisplayEvents/MultiWCTool.h>
#include <display/Display/PixelCanvas.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// (Required) default constructor.
MultiWCTool::MultiWCTool() :
  DisplayTool(),
  itsWCListIter(0),
  itsCurrentWC(0),
  itsEventHandlersRegistered(False) {
  itsWCListIter = new ListIter<WorldCanvas *>(&itsWCList);
}

MultiWCTool::MultiWCTool(const Display::KeySym &keysym) :
  DisplayTool(keysym),
  itsWCListIter(0),
  itsCurrentWC(0),
  itsEventHandlersRegistered(False) {
  itsWCListIter = new ListIter<WorldCanvas *>(&itsWCList);
  enable();
}

MultiWCTool::~MultiWCTool() {
  MultiWCTool::disable();
  if (itsWCListIter) {
    delete itsWCListIter;
  }
}

// (Required) copy constructor.
MultiWCTool::MultiWCTool(const MultiWCTool &other) :
  DisplayTool(other) {  }

// (Required) copy assignment.
MultiWCTool &MultiWCTool::operator=(const MultiWCTool &other) {
  if (this != &other) DisplayTool::operator=(other);
  return *this;
}

void MultiWCTool::addWorldCanvas(WorldCanvas &worldcanvas) {
  itsWCListIter->toEnd();
  itsWCListIter->addRight(&worldcanvas);
  if (itsEventHandlersRegistered) {
    worldcanvas.addPositionEventHandler(*this);
    worldcanvas.addMotionEventHandler(*this);
    worldcanvas.addRefreshEventHandler(*this);
  }
}

void MultiWCTool::removeWorldCanvas(WorldCanvas &worldcanvas) {
  itsWCListIter->toStart();
  while (!itsWCListIter->atEnd()) {
    if (itsWCListIter->getRight() == &worldcanvas) {
      itsWCListIter->removeRight();
      if (itsEventHandlersRegistered) {
	worldcanvas.removePositionEventHandler(*this);
	worldcanvas.removeMotionEventHandler(*this);
	worldcanvas.removeRefreshEventHandler(*this);
      }
      break;
    }
    (*itsWCListIter)++;
  }
}

void MultiWCTool::addWorldCanvases(PanelDisplay* pdisp) {
  itsWCListIter->toEnd();
  pdisp->myWCLI->toStart();
  while (!(pdisp->myWCLI->atEnd())) {
    WorldCanvas* wcanvas = pdisp->myWCLI->getRight();
    itsWCListIter->addRight(wcanvas);
    if (itsEventHandlersRegistered) {
      wcanvas->addPositionEventHandler(*this);
      wcanvas->addMotionEventHandler(*this);
      wcanvas->addRefreshEventHandler(*this);
    }    
    (*(pdisp->myWCLI))++;
  }
}

void MultiWCTool::removeWorldCanvases(PanelDisplay* pdisp) {
  disable();

  itsWCListIter->toStart();
  while (!itsWCListIter->atEnd()) {
    Bool found=False;
    pdisp->myWCLI->toStart();
    while (!pdisp->myWCLI->atEnd()) {      
      WorldCanvas* wcanvas = pdisp->myWCLI->getRight();    
      if (itsWCListIter->getRight() == wcanvas) {
	found=True;
	break;  }
      (*(pdisp->myWCLI))++;  }
    if(found) itsWCListIter->removeRight();
    else (*itsWCListIter)++;  }

  enable();
}

void MultiWCTool::enable() {
  if (!itsEventHandlersRegistered) {
    itsEventHandlersRegistered = True;
    itsWCListIter->toStart();
    while (!itsWCListIter->atEnd()) {
      WorldCanvas *wc = itsWCListIter->getRight();
      wc->addPositionEventHandler(*this);
      wc->addMotionEventHandler(*this);
      wc->addRefreshEventHandler(*this);
      (*itsWCListIter)++;
    }
  }
}

void MultiWCTool::disable() {
  if (itsEventHandlersRegistered) {
    itsEventHandlersRegistered = False;
    itsWCListIter->toStart();
    while (!itsWCListIter->atEnd()) {
      WorldCanvas *wc = itsWCListIter->getRight();
      wc->removePositionEventHandler(*this);
      wc->removeMotionEventHandler(*this);
      wc->removeRefreshEventHandler(*this);
      (*itsWCListIter)++;
    }
  }
}

void MultiWCTool::operator()(const WCPositionEvent &ev) {
  if (ev.key() != getKey()) {
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

void MultiWCTool::refresh() {
  if(itsCurrentWC==0) return;
  PixelCanvas *pc = itsCurrentWC->pixelCanvas();
  if(pc==0) return;
  pc->copyBackBufferToFrontBuffer();
  pc->setDrawBuffer(Display::FrontBuffer);
  pc->callRefreshEventHandlers(Display::BackCopiedToFront);  }


void MultiWCTool::operator()(const WCRefreshEvent &ev) {
  if (	itsCurrentWC != 0 &&
	ev.worldCanvas() == itsCurrentWC &&
	ev.reason() == Display::BackCopiedToFront &&
	itsCurrentWC->pixelCanvas()->drawBuffer()==Display::FrontBuffer  )
  draw(ev);  }

void MultiWCTool::operator()(const WCMotionEvent &ev) {
  moved(ev);
}

void MultiWCTool::setClipToDrawArea() {
  WorldCanvas *wc = itsCurrentWC; if(wc==0) return;
  PixelCanvas *pc = wc->pixelCanvas(); if(pc==0) return;
  Int x0 = wc->canvasXOffset() + wc->canvasDrawXOffset();
  Int x1 = x0 + wc->canvasDrawXSize() - 1;
  Int y0 = wc->canvasYOffset() + wc->canvasDrawYOffset();
  Int y1 = y0 + wc->canvasDrawYSize() - 1;
  pc->setClipWindow(x0,y0, x1,y1);
  pc->enable(Display::ClipWindow);  }

void MultiWCTool::setClipToWC() {
  WorldCanvas *wc = itsCurrentWC; if(wc==0) return;
  PixelCanvas *pc = wc->pixelCanvas(); if(pc==0) return;
  Int x0 = wc->canvasXOffset();  Int x1 = x0 + wc->canvasXSize() - 1;
  Int y0 = wc->canvasYOffset();  Int y1 = y0 + wc->canvasYSize() - 1;
  pc->setClipWindow(x0,y0, x1,y1);
  pc->enable(Display::ClipWindow);  }

void MultiWCTool::resetClip() {
  WorldCanvas *wc = itsCurrentWC; if(wc==0) return;
  PixelCanvas *pc = wc->pixelCanvas(); if(pc==0) return;
  pc->disable(Display::ClipWindow);  }

// Callbacks: responses to events.  To be implemented by derived classes
// as needed.

void MultiWCTool::keyPressed(const WCPositionEvent &ev) {  }
void MultiWCTool::keyReleased(const WCPositionEvent &) {  }
void MultiWCTool::otherKeyPressed(const WCPositionEvent &) {  }
void MultiWCTool::otherKeyReleased(const WCPositionEvent &) {  }
void MultiWCTool::moved(const WCMotionEvent &) {  }
void MultiWCTool::draw(const WCRefreshEvent &) {  }



} //# NAMESPACE CASA - END

