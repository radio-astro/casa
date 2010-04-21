//# QtDBusApp.cc: base class for casa dbus applications
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

#include <stdlib.h>
#include <QTextStream>
#include <QDBusConnectionInterface>
#include <casaqt/QtUtilities/QtDBusApp.h>

#define service_base "edu.nrao.casa."
#define object_base "/casa/"

namespace casa {

  QtDBusApp::QtDBusApp( ) : service_name(0), object_name(0) { }

    QDBusConnection &QtDBusApp::connection( ) {
	static QDBusConnection bus(QDBusConnection::sessionBus());
	return bus;
    }

    QString QtDBusApp::generateServiceName( const QString &name, const pid_t pid ) {
	QString result;
	QTextStream(&result) << service_base << name << "_" << pid;
	return result;
    }

    QString QtDBusApp::dbusServiceName( const QString &name, const pid_t pid ) {

	if ( service_name )
	    return *service_name;

	service_name = new QString( generateServiceName(name,pid) );
	return *service_name;
    }

    QString QtDBusApp::dbusObjectName( const QString &name, const pid_t pid ) {

	if ( object_name )
	    return *object_name;

	object_name = new QString( );
	QTextStream(object_name) << object_base << name << "_" << pid;
	return *object_name;
    }

    QString QtDBusApp::serviceOwner( const QString &service ) {
        char *str = strdup(service.toAscii().data());
	char *cur = strtok(str,".");
	char *prev = cur;
	while ( (cur = strtok(0,".")) ) {
	    prev = cur;
	}
	QString result(object_base);
	QTextStream(&result) << prev;
	return result;
    }

    QString QtDBusApp::serviceOwner( const std::string &service ) {
	return serviceOwner(QString(service.c_str()));
    }

    const QString &QtDBusApp::serviceBase( ) {
	static QString base(service_base);
	return base;
    }

    const QString &QtDBusApp::objectBase( ) {
	static QString base(object_base);
	return base;
    }

    bool QtDBusApp::serviceIsAvailable(const QString &service) {
	return connection().isConnected() &&
	  connection().interface()->isServiceRegistered(service);
    }

    bool QtDBusApp::serviceIsAvailable(const std::string &service) {
	return serviceIsAvailable(QString(service.c_str()));
    }

    QtDBusApp::~QtDBusApp( ) {
	if ( service_name ) { delete service_name; }
	if ( object_name ) { delete object_name; }
    }

    const QString &QtDBusApp::dbusServiceName( const QString &name ) {

	if ( service_name )
	    return *service_name;


	if ( name.size( ) > 0 ) {
	    service_name = new QString( );
	    QTextStream(service_name) << service_base << name;
	} else {
	    service_name = new QString( generateServiceName(dbusName(), getpid( )) );
	}

	return *service_name;
    }

    const QString &QtDBusApp::dbusObjectName( const QString &name ) {

	if ( object_name )
	    return *object_name;

	object_name = new QString( );

	if ( name.size( ) > 0 )
	    QTextStream(object_name) << object_base << name;
	else
	    QTextStream(object_name) << object_base << dbusName( ) << "_" << getpid( );

	return *object_name;
    }

    int QtDBusApp::get_id( ) {
	static bool initialized = false;
#if defined(__APPLE___)
	if ( ! initialized ) {
	    initialized = true;
	    srandomdev( );
	}
#else
	if ( ! initialized ) {
	    initialized = true;
	    union {
	      void *foo;
	      unsigned bar;
	    };
	    foo = &initialized;
	    srandom(bar);
	}
#endif
	int rn = (int) random( );
	while ( rn <= 0 || rn == INT_MAX || rn == SHRT_MAX || rn == CHAR_MAX ||
		std::find(used_ids.begin( ), used_ids.end( ), rn) != used_ids.end() ) {
// 		used_ids.find(rn) != used_ids.end() ) {
	    rn = (int) random( );
	}
// 	used_ids.insert(rn);
	used_ids.push_back(rn);
	return rn;
    }

    bool QtDBusApp::connectToDBus( QObject *object,  const QString &dbus_name ) {

	bool dbusRegistered = false;

	if ( dbusRegistered || serviceIsAvailable(dbusServiceName(dbus_name)) )
	    return false;

	try {
	    // Register service and object.
	    dbusRegistered = connection().isConnected() &&
			     connection().registerService(dbusServiceName(dbus_name)) &&
			     connection().registerObject(dbusObjectName(dbus_name), object,
							 QDBusConnection::ExportAdaptors);

	} catch(...) { dbusRegistered = false; }

	return dbusRegistered;
    }


}

