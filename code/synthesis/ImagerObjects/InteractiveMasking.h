// -*- C++ -*-
//# InteractiveMasking.h: Definition of the InteractiveMasking class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
#ifndef SYNTHESIS_INTERACTIVEMASKING_H
#define SYNTHESIS_INTERACTIVEMASKING_H
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>

#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  Bool clone(const String& imageName, const String& newImageName);
  class ViewerProxy;
  
  class new_interactive_clean_callback 
  {
  public:
    new_interactive_clean_callback( ) { }
    casa::dbus::variant result( ) { return casa::dbus::toVariant(result_); }
    bool callback( const DBus::Message & msg );
  private:
    DBus::Variant result_;
  };
  
  class InteractiveMasking 
  {
  public:
    InteractiveMasking() :
      viewer_p(0), clean_panel_p(0), image_id_p(0), mask_id_p(0),
      prev_image_id_p(0), prev_mask_id_p(0)
    {};
    
    ~InteractiveMasking()
    {
      if ( viewer_p ) 
	{
	  // viewer_p->close( clean_panel_p );
	  viewer_p->done();
	  delete viewer_p;
	}
    };
    /*
    Int interactivemask(const String& image, const String& mask, 
			Int& niter, Int& ncycles, String& thresh, 
			const Bool forceReload=False);
    */
    Int interactivemask(const String& image, const String& mask, 
			Int& niter, Int& cycleniter, String& thresh, String& cyclethresh, 
			const Bool forceReload=False);
  private:
    ViewerProxy *viewer_p;
    int clean_panel_p;
    int image_id_p;
    int mask_id_p;
    int prev_image_id_p;
    int prev_mask_id_p;
  };
  
}

#endif
