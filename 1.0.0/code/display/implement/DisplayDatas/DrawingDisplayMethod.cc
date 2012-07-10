//# DrawingDisplayMethod.cc: actual drawing for interactive DrawingDisplayData
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

#include <casa/aips.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/DrawingDisplayData.h>
#include <display/DisplayDatas/DrawingDisplayMethod.h>
#include <display/DisplayDatas/DDDObject.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DrawingDisplayMethod::DrawingDisplayMethod(WorldCanvas *worldCanvas, 
					   AttributeBuffer *wchAttributes,
					   AttributeBuffer *ddAttributes,
					   CachingDisplayData *dd) :
  CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
}

DrawingDisplayMethod::~DrawingDisplayMethod() {
  cleanup();
}

Bool DrawingDisplayMethod::drawIntoList(Display::RefreshReason reason,
					WorldCanvasHolder &wcHolder) {
  // use friendship to access list of DDDObjects
  ListIter<void *> *listIter = 
    ((DrawingDisplayData *)parentDisplayData())->itsDDDOListIter;
  DDDObject *dddObject;
  listIter->toStart();
  while (!listIter->atEnd()) {
    dddObject = (DDDObject *)listIter->getRight();
    dddObject->draw(reason, wcHolder.worldCanvas());
    (*listIter)++;
  }
  return True;
}

void DrawingDisplayMethod::cleanup() {
}

DrawingDisplayMethod::DrawingDisplayMethod() {
}

DrawingDisplayMethod::DrawingDisplayMethod(const DrawingDisplayMethod &other) :
  CachingDisplayMethod(other) {
}

void DrawingDisplayMethod::operator=(const DrawingDisplayMethod &) {
}

} //# NAMESPACE CASA - END

