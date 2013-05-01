//# PlotServerProxy.cc: provide a viewer dbus proxy for C++
//# Copyright (C) 2009
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
//# $Id: $
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/utilities/Conversion.h>

namespace casa {


    std::list<std::string> PlotServerProxy::execArgs( const std::list<std::string> &args ) {
	std::list<std::string> result(args);
	result.push_front("casaplotserver");
	return result;
    }

  PlotServerProxy::PlotServerProxy( const std::string &name ) : 
		DBus::ObjectProxy( DBusSession::instance().connection( ), dbus::object(name).c_str(), dbus::path(name).c_str() ) { }

}
