//# DDDRectangle.h: implementation of rectangular DDDObject
//# Copyright (C) 1999,2000,2001,2002
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
#include <casa/Exceptions.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/DrawingDisplayData.h>
#include <display/DisplayDatas/DDDRectangle.h>
#include <cpgplot.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DDDRectangle::DDDRectangle(const Record &description, 
			   DrawingDisplayData *owner) :
  DDDObject(description, owner),
  itsHX(4), 
  itsHY(4),
  itsMode(DDDObject::None) {
  if (!(description.isDefined("blc") && description.isDefined("trc"))) {
    throw(AipsError("Invalid record for constructing DDDRectangle"));
  }
  description.get("blc", itsBlc);
  description.get("trc", itsTrc);
}

DDDRectangle::~DDDRectangle() {
}

void DDDRectangle::draw(const Display::RefreshReason &reason,
			WorldCanvas *worldcanvas) {
  Vector<Double> pblc(2), ptrc(2);
  worldcanvas->worldToPix(pblc, itsBlc);
  worldcanvas->worldToPix(ptrc, itsTrc);
  Int x1 = Int(min(pblc(0), ptrc(0)) + 0.5);
  Int x2 = Int(max(pblc(0), ptrc(0)) + 0.5);
  Int y1 = Int(min(pblc(1), ptrc(1)) + 0.5);
  Int y2 = Int(max(pblc(1), ptrc(1)) + 0.5);
  PixelCanvas *pCanvas = worldcanvas->pixelCanvas();
  pCanvas->setColor(color());
  pCanvas->setLineWidth(lineWidth());
  pCanvas->drawRectangle(x1, y1, x2, y2);
  if (showingHandles()) {
    Int w = x2 - x1;
    Int h = y2 - y1;
    Int h1 = 0;
    if (w>=35 && h>=35) { 
      h1 = 6; 
    } else if (w>=20 && h>=20) { 
      h1 = 4; 
    } else if (w>= 9 && h>= 9) { 
      h1 = 2; 
    }
    itsHX(0) = x1 + 1;
    itsHX(1) = x1 + 1 + h1 - 1;
    itsHX(2) = x1 - 1 + w - h1 + 1;
    itsHX(3) = x1 - 1 + w - 1 + 1;
    itsHY(0) = y1 + 1 + 1 - 1;
    itsHY(1) = y1 + 1 + h1 - 1;
    itsHY(2) = y1 - 1 + h - h1 + 1;
    itsHY(3) = y1 - 1 + h;    
    if (h1) {
      // (+1 in drawing because of strange X behaviour!)
      pCanvas->drawFilledRectangle(itsHX(0), itsHY(0) - 1, itsHX(1) + 1, 
				  itsHY(1) + 0);
      pCanvas->drawFilledRectangle(itsHX(2), itsHY(0) - 1, itsHX(3) + 1, 
				  itsHY(1) + 0);
      pCanvas->drawFilledRectangle(itsHX(0), itsHY(2) - 1, itsHX(1) + 1,
				  itsHY(3) + 0);
      pCanvas->drawFilledRectangle(itsHX(2), itsHY(2) - 1, itsHX(3) + 1, 
				  itsHY(3) + 0);
    }
  }

  // should allow DDDObject::draw(...) to draw the label since
  // it has the options to control that...
  if (owner()->labelPosition() == "centre") {
    worldcanvas->worldToLin(pblc, itsBlc);
    worldcanvas->worldToLin(ptrc, itsTrc);
    cpgsci(1);
    cpgtext((pblc(0) + ptrc(0)) / 2, (pblc(1) + ptrc(1)) / 2, 
	    label().chars());
  }
}

Record DDDRectangle::description()  {
  Record rec = DDDObject::description();
  rec.define("blc", itsBlc);
  rec.define("trc", itsTrc);
  rec.define ("type", String("rectangle"));
  return rec;
} 

void DDDRectangle::setDescription(const Record &rec)  {
  DDDObject::setDescription(rec);
  itsBlc = rec.asArrayDouble("blc");
  itsTrc = rec.asArrayDouble("trc");
  owner()->refresh();
}

void DDDRectangle::operator()(const WCPositionEvent &ev) {
  if ((ev.key() != owner()->key()) || 
      // new phrase in expression on next line...
      //!(ev.modifiers() & owner()->keyModifier())) {
      False) {
    return;
  }
  Int x = ev.pixX();
  Int y = ev.pixY();
  Vector<Double> pblc(2), ptrc(2);
  ev.worldCanvas()->worldToPix(pblc, itsBlc);
  ev.worldCanvas()->worldToPix(ptrc, itsTrc);
  Int x1 = Int(min(pblc(0), ptrc(0)) + 0.5);
  Int x2 = Int(max(pblc(0), ptrc(0)) + 0.5);
  Int y1 = Int(min(pblc(1), ptrc(1)) + 0.5);
  Int y2 = Int(max(pblc(1), ptrc(1)) + 0.5);
  if (showingHandles()) {
    // put back in when cursor control on X11PixelCanvas
    //Bool overhandle = False;
    if (((x < x1) || (x > x2) || (y < y1) || (y > y2)) && ev.keystate()) {
      // press button outside rectangle, switch to handles invisible
	clearClickBuffer();
	showHandles(False);
    } else if (ev.keystate()) {
	// first update handle locations since this might be 
	// on another canvas!
	Int w = x2 - x1;
	Int h = y2 - y1;
	Int h1 = 0;
	if (w>=35 && h>=35) { 
	  h1 = 6; 
	} else if (w>=20 && h>=20) { 
	  h1 = 4; 
	} else if (w>= 9 && h>= 9) { 
	  h1 = 2; 
	}
	itsHX(0) = x1 + 1;
	itsHX(1) = x1 + 1 + h1 - 1;
	itsHX(2) = x1 - 1 + w - h1 + 1;
	itsHX(3) = x1 - 1 + w - 1 + 1;
	itsHY(0) = y1 + 1 + 1 - 1;
	itsHY(1) = y1 + 1 + h1 - 1;
	itsHY(2) = y1 - 1 + h - h1 + 1;
	itsHY(3) = y1 - 1 + h;

	Bool left = ((x >= min(itsHX(0), itsHX(1))) && 
			   (x <= max(itsHX(0), itsHX(1))));
	Bool right = ((x >= min(itsHX(2), itsHX(3))) && 
			    (x <= max(itsHX(2), itsHX(3))));
	Bool top = ((y >= min(itsHY(0), itsHY(1))) &&
			  (y <= max(itsHY(0), itsHY(1))));
	Bool bottom = ((y >= min(itsHY(2), itsHY(3))) &&
			     (y <= max(itsHY(2), itsHY(3))));
	if ((left || right) && (top || bottom)) {
	  clearClickBuffer();
	  if (isEditable()) itsMode = DDDObject::Handle;
	  itsLeftHandle = left;
	  itsBottomHandle = top;
	  //overhandle = True;
	} else {
	  // clicked inside rectangle, and not on handles.
	  if (!storeClick(ev)) {
	    if (isMovable()) itsMode = DDDObject::Move;
	    itsBaseMoveX = x;
	    itsBaseMoveY = y;
	  }
	}
	
	// if on handle, resize
	// if in rectangle, move
    }
    /*
      if (overhandle) {
      ev.worldCanvas()->pixelCanvas()->setCursor(120);
      } else {
      ev.worldCanvas()->pixelCanvas()->unsetCursor();
    }
    */
  } else {
    // if on rectangle edge (tolerance 1 pixel) switch to visible

    if ((((x>=x1-1 && x<=x1+1) || (x>=x2-1 && x<=x2+1)) && (y1 <= y) && (y <= y2)) ||
	(((y>=y1-1 && y<=y1+1) || (y>=y2-1 && y<=y2+1)) && (x1 <= x) && (x <= x2))) {
      showHandles(True);
      clearClickBuffer();
      itsMode = DDDObject::None;
    }
  }    
}

void DDDRectangle::operator()(const WCMotionEvent &ev) {
  if (!(ev.modifiers() & owner()->keyModifier())) {
    return;
  }
  Int x = ev.pixX();
  Int y = ev.pixY();
  Vector<Double> pblc(2), ptrc(2);
  ev.worldCanvas()->worldToPix(pblc, itsBlc);
  ev.worldCanvas()->worldToPix(ptrc, itsTrc);
  switch (itsMode) {
  case DDDObject::Move:
    {
      Double dx = Double(x - itsBaseMoveX);
      Double dy = Double(y - itsBaseMoveY);
      itsBaseMoveX = x;
      itsBaseMoveY = y;
      pblc(0) = pblc(0) + dx;
      pblc(1) = pblc(1) + dy;
      ptrc(0) = ptrc(0) + dx;
      ptrc(1) = ptrc(1) + dy;
      ev.worldCanvas()->pixToWorld(itsBlc, pblc);
      ev.worldCanvas()->pixToWorld(itsTrc, ptrc);
      owner()->refresh();
      break;
    }
  case DDDObject::Handle:
    {
      //ev.worldCanvas()->pixelCanvas()->setCursor(120);
      Double x1 = Int(min(pblc(0), ptrc(0)) + 0.5);
      Double x2 = Int(max(pblc(0), ptrc(0)) + 0.5);
      Double y1 = Int(min(pblc(1), ptrc(1)) + 0.5);
      Double y2 = Int(max(pblc(1), ptrc(1)) + 0.5);
      if (itsLeftHandle) {
	pblc(0) = x;
	ptrc(0) = x2;
      } else {
	pblc(0) = x1;
	ptrc(0) = x;
      }
      if (itsBottomHandle) {
	pblc(1) = y;
	ptrc(1) = y2;
      } else {
	pblc(1) = y1;
	ptrc(1) = y;
      }
      ev.worldCanvas()->pixToWorld(itsBlc, pblc);
      ev.worldCanvas()->pixToWorld(itsTrc, ptrc);
      owner()->refresh();
      break;
    }
  default:
    {

      break;
    }
  }
}

DDDRectangle::DDDRectangle() {
}

DDDRectangle::DDDRectangle(const DDDRectangle &) {
}

void DDDRectangle::operator=(const DDDRectangle &) {
}

} //# NAMESPACE CASA - END

