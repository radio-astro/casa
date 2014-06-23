//# DBusSession.h: connection to the DBus session
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
//# $Id$

#ifndef DBUS_SESSION_H_
#define DBUS_SESSION_H_

#include <vector>
#include <string>
#include <casadbus/session/DBusSession.proxy.h>
#include <casadbus/session/Dispatcher.h>

namespace casa {

    class DBusSession :
	private org::freedesktop::DBus_proxy,
	public DBus::IntrospectableProxy,
	public DBus::ObjectProxy {
    public:

	static DBusSession &instance( );
	std::vector<std::string> listNames( ) { return ListNames( ); }
	DBus::Connection &connection( ) { return conn; }
	dbus::Dispatcher &dispatcher( ) { return *dispatcher_; }

    private:

	DBus::Connection conn;
	DBusSession( DBus::Connection & );
	DBusSession( );

	static dbus::Dispatcher *dispatcher_;

	// pure virtual functions (i.e. dbus signals)
	void NameOwnerChanged(const std::string&, const std::string&, const std::string&);
	void NameLost(const std::string&);
	void NameAcquired(const std::string&);

	friend class init_dispatcher;
    };  

	static class init_dispatcher {
    	public:
		init_dispatcher( ) {
			if ( ! initalized ) {
				initalized = true;
				DBusSession::dispatcher_ = new dbus::Dispatcher( );
				DBus::default_dispatcher = DBusSession::dispatcher_;
			}
		}
		private:
			static bool initalized;
    } init_dispatcher_;

}
#endif
