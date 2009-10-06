//# QtDBusXmlApp.qo.h: Abstract parent to use the CASA DBus server.
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
#ifndef QTDBUSAPP_QO_H_
#define QTDBUSAPP_QO_H_

#include <set>
#include <string>
#include <QString>
#include <QDBusConnection>

namespace casa {

    class QtDBusApp {
	public:
	    static QDBusConnection &connection( );
	    static QString dbusServiceName( const QString &name, const pid_t pid );
	    static QString dbusObjectName( const QString &name, const pid_t pid );
	    static QString serviceOwner( const QString &service );
	    static QString serviceOwner( const std::string &service );
	    const QString &dbusServiceName( );
	    const QString &dbusObjectName( );
	    static bool serviceIsAvailable(const QString &service);
	    static bool serviceIsAvailable(const std::string &service);
	    virtual const QString &getName( ) const = 0;
	    QtDBusApp( ) : service_name(0), object_name(0) { }
	    virtual ~QtDBusApp( );

	protected:
	    // returns a new "id" to be used for things such as data, widget,
	    // or object handles on both ends of the dbus connection...
	    virtual int get_id( );

	private:
	    static const QString &serviceBase( );
	    static const QString &objectBase( );
	    QString *service_name;
	    QString *object_name;

	    std::set<int> used_ids;
    };

    inline std::string to_string(const QString &other) { return std::string((const char*) other.toAscii().data()); }

}

#endif
