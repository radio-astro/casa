//# DSMarker.cc: Marker implementation for "DisplayShapes"
//# Copyright (C) 1998,1999,2000,2001,2002
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
//# $Id: 

#include <casa/aips.h>
#include <display/DisplayShapes/DSMarker.h>

#include <casa/Arrays/Vector.h>
#include <display/Display/PixelCanvas.h>

#include <display/Display/DParameterRange.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DSMarker::DSMarker() : 
  DSBasic() {
  
  itsSize = new DParameterRange<Int> ("size", "Marker Size", 
				      "Select the height in pixels of marker",
				      1,40,1,10,10);
  
  setDefaultOptions();
}

DSMarker::DSMarker(const Float& xPos, const Float& yPos, 
		   const Display::Marker& marker, const uInt pixelSize) :
  DSBasic() {
  
  DisplayShape::setHasHandles(True);
  DisplayShape::setDrawHandles(True);


  itsSize = new DParameterRange<Int> ("size", "Marker Size", 
				      "Select the height in pixels of marker",
				      1,40,1,pixelSize,10);
  itsCenter.resize(2);
  itsCenter[0] = xPos;
  itsCenter[1] = yPos;

  itsMarkerStyle = marker;
  itsValid = True;
  itsBuiltHandle = False;
  updateHandle();
}

DSMarker::DSMarker(const Record& settings) {


}

DSMarker::DSMarker(const DSMarker& other) :
  DSBasic(other),
  itsCenter(other.itsCenter),
  itsHandle(other.itsHandle),
  itsBuiltHandle(other.itsBuiltHandle),
  itsSize(other.itsSize),
  itsMarkerStyle(other.itsMarkerStyle),
  itsValid(other.itsValid) {

}

DSMarker::~DSMarker() {

  delete itsSize; itsSize =0;
}

void DSMarker::move(const Float& dX, const Float& dY) {
  if (itsValid) {
    itsCenter[0] += dX;
    itsCenter[1] += dY;
  }
  updateHandle();
}

void DSMarker::setCenter(const Float& xPos, const Float& yPos) {
  itsCenter[0] = xPos;
  itsCenter[1] = yPos;
  itsValid = True;
  updateHandle();
}

Vector<Float> DSMarker::getCenter() {
  if (itsValid) return itsCenter;

  Vector<Float> a(2); 
  a[0]=0;a[1]=0;
  return a;
}
void DSMarker::setSize(const uInt newSize) {
  itsSize->setValue(newSize);
  updateHandle();
}

void DSMarker::scale(const Float& scaleFactor) {
  if (Int(itsSize->value() * scaleFactor) > 2) {
    itsSize->setValue(Int(itsSize->value() * scaleFactor));
    updateHandle();
  }
} 

void DSMarker::draw(PixelCanvas* pc) {
  if (itsValid) {
    pc->setLineWidth(DSBasic::getLineWidth());
    pc->setColor(DisplayShape::getColor());
    pc->drawMarker(itsCenter[0], itsCenter[1], 
		   itsMarkerStyle, itsSize->value());
  } 
  DSBasic::draw(pc);
}

Bool DSMarker::inObject(const Float& xPos, const Float& yPos) {
  if (itsValid) 

    return ((xPos < itsCenter[0] + (itsSize->value()/2) ) &&
	    (xPos > itsCenter[0] - (itsSize->value()/2)) &&
	    (yPos > itsCenter[1] - (itsSize->value()/2)) &&
	    (yPos < itsCenter[1] + (itsSize->value()/2)));
  
  return False;
}

Bool DSMarker::setOptions(const Record& settings) {
  Bool localChange = False;
  
  if (settings.isDefined("center")) {
    if (!itsValid) itsValid = True;
    itsCenter = settings.asArrayFloat("center");
  }

  if (itsSize->fromRecord(settings)) localChange = True;

  if (settings.isDefined("markerstyle")) {
    Int temp = settings.asInt("markerstyle");
    Display::Marker newStyle = static_cast<Display::Marker>(temp);
    
    if (newStyle != itsMarkerStyle) {
      localChange = True;
      itsMarkerStyle = newStyle;
    }
  }
  
  if (DSBasic::setOptions(settings)) localChange = True;
  if (localChange) updateHandle();
  return localChange;
}

Record DSMarker::getOptions() {
  Record rec = DSBasic::getOptions();
  rec.removeField("type");
  
  rec.define("type", "marker");
  if (itsValid) rec.define("center", itsCenter);

  itsSize->toRecord(rec);
  rec.define("markerstyle", itsMarkerStyle);

  return rec;
}

void DSMarker::setDefaultOptions() {
  itsCenter.resize(2);

  itsMarkerStyle = Display::Square;
  itsValid = False; itsBuiltHandle = False;
}

void DSMarker::updateHandle() {
  if (itsValid) {

    itsHandle.resize(1,2);
    itsHandle(0,0) = itsCenter(0) - (0.5 * itsSize->value());
    itsHandle(0,1) = itsCenter(1) - (0.5 * itsSize->value());
    
    if(itsBuiltHandle) {
      setHandlePositions(itsHandle);
    } else {
      buildHandles(itsHandle);
      itsBuiltHandle = True;
    }
  }

}





} //# NAMESPACE CASA - END

