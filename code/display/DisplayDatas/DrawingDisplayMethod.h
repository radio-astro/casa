//# DrawingDisplayMethod.h: actual drawing for interactive DrawingDisplayData
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

#ifndef TRIALDISPLAY_DRAWINGDISPLAYMETHOD_H
#define TRIALDISPLAY_DRAWINGDISPLAYMETHOD_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvas;
class DrawingDisplayData;

// <summary>
// DisplayMethod which provides actual drawing for DrawingDisplayData class.
// </summary>

// <synopsis>
// This class provides the necessary implementation for the
// actual drawing of the objects contained within a 
// DrawingDisplayData object. 
// </synopsis>

class DrawingDisplayMethod : public CachingDisplayMethod {

 public:

  // Constructor.
  DrawingDisplayMethod(WorldCanvas *worldCanvas, 
		       AttributeBuffer *wchAttributes,
		       AttributeBuffer *ddAttributes,
		       CachingDisplayData *dd);
  
  // Destructor.
  virtual ~DrawingDisplayMethod();

  // Clean up (ie. delete any existing cached display list).
  virtual void cleanup();

  // Draw into a cached drawing list, called by draw function.
  virtual Bool drawIntoList(Display::RefreshReason reason,
			    WorldCanvasHolder &wcHolder);
  
 protected:

  // (Required) default constructor.
  DrawingDisplayMethod();

  // (Required) copy constructor.
  DrawingDisplayMethod(const DrawingDisplayMethod &other);

  // (Required) copy assignment.
  void operator=(const DrawingDisplayMethod &other);

 private:

};


} //# NAMESPACE CASA - END

#endif
