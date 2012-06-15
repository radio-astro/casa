//# Profile2dDM.h: Drawing Method for 2d Profile DisplayDatas
//# Copyright (C) 2003
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

#ifndef TRIALDISPLAY_PROFILE2DDM_H
#define TRIALDISPLAY_PROFILE2DDM_H

#include <casa/aips.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>
#include <display/DisplayDatas/Profile2dDD.h>
namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// A DisplayMethod to draw Profiles
// </summary>
//
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <prerequisite>
//   <li> DisplayMethod
//   <li> CachingDisplayMethod
// </prerequisite>
//
// <etymology>
// </etymology>
// <synopsis> 
// 
// </synopsis> 

class Profile2dDM : public CachingDisplayMethod {

  public:

  // Constructor.
  Profile2dDM(WorldCanvas *worldCanvas, 
		AttributeBuffer *wchAttributes,
		AttributeBuffer *ddAttributes,
		CachingDisplayData *dd);
  
  // Destructor.
  virtual ~Profile2dDM();
  
  // Clean up (ie. delete any existing cached display list).
  virtual void cleanup();
  
  // Draw into a cached drawing list, called by draw function.
  virtual Bool drawIntoList(Display::RefreshReason reason,
			    WorldCanvasHolder &wcHolder);
  
  protected:
  
  // (Required) default constructor.
  Profile2dDM();
  
  // (Required) copy constructor.
  Profile2dDM(const Profile2dDM &other);
  
  // (Required) copy assignment.
  void operator=(const Profile2dDM &other);
  
  private:
  // <group>
  // Set the appropriate styes and rules for drawing the profile
  void setStyles(WorldCanvas *wc, Profile2dDD *parent);
  // Restore the appropriate styles and rules after drawing
  // is completed
  void restoreStyles(WorldCanvas *wc);
  // </group>
};


} //# NAMESPACE CASA - END

#endif
