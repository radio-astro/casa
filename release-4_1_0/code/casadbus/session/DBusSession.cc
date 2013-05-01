//# DBusSession.cc: provide a common connection to dbus for casa
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
#include <casadbus/session/DBusSession.h>

namespace casa {

    static const char *DBUS_SERVER_NAME = "org.freedesktop.DBus";
    static const char *DBUS_SERVER_PATH = "/org/freedesktop/DBus";

    bool init_dispatcher::initalized = false;
    dbus::Dispatcher *DBusSession::dispatcher_;

    DBusSession &DBusSession::instance( ) {
	static DBus::Connection bus = DBus::Connection::SessionBus();
	static DBusSession the_session(bus);
	return the_session;
    }

    DBusSession::DBusSession( DBus::Connection &c )
    : ::DBus::ObjectProxy( c, DBUS_SERVER_PATH, DBUS_SERVER_NAME), conn(c)  { }
    void DBusSession::NameOwnerChanged(const std::string&, const std::string&, const std::string&) { }
    void DBusSession::NameLost(const std::string&) { } 
    void DBusSession::NameAcquired(const std::string&) { }

}
