//# WedgeDM.h: drawing for Wedge DisplayDatas
//# Copyright (C) 2001
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

#ifndef TRIALDISPLAY_WEDGEDM_H
#define TRIALDISPLAY_WEDGEDM_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// The DisplayMethod to draw Wedges
// </summary>
//
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// </etymology>
//
// <synopsis> This class supplies the actual drawing of the wedge via
// the drawIntoList method
// </synopsis>
//
// <example>
// <srcBlock>
// </srcBlock>
// </example>
//
// <motivation>
// Users want wedges
//</motivation>
//
// <todo> 
// Transposed wedge drawing
// </todo>
//

class WedgeDM : public CachingDisplayMethod {

public:

  // Constructor.
  WedgeDM(WorldCanvas *worldCanvas, 
		AttributeBuffer *wchAttributes,
		AttributeBuffer *ddAttributes,
		CachingDisplayData *dd);
  
  // Destructor.
  virtual ~WedgeDM();
  
  // Clean up (ie. delete any existing cached display list).
  virtual void cleanup();
  
  // Draw into a cached drawing list, called by draw function.
  virtual Bool drawIntoList(Display::RefreshReason reason,
			    WorldCanvasHolder &wcHolder);
  
protected:
  
  // (Required) default constructor.
  WedgeDM();
  
  // (Required) copy constructor.
  WedgeDM(const WedgeDM &other);
  
  // (Required) copy assignment.
  void operator=(const WedgeDM &other);
  
private:

};


} //# NAMESPACE CASA - END

#endif
