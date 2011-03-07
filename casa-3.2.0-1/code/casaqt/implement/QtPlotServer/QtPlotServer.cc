//# QtPlotServer.cc: Qt implementation of main 2D plot server display window.
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

#include <casaqt/QtPlotServer/QtPlotServer.qo.h>
#include <casaqt/QtPlotServer/QtPlotSvrPanel.qo.h>
#include <casaqt/QtPlotServer/QtDBusPlotSvrAdaptor.qo.h>


namespace casa {

    QString QtPlotServer::name_;

    const QString &QtPlotServer::name( ) {
	static bool initialized = false;
	if ( ! initialized ) {
	    name_ = "plot_server";
	}
	return name_;
    }

    QtPlotServer::QtPlotServer( const char *dbus_name ) {
	dbus_name_ = (dbus_name ? dbus_name : 0);
	dbus_ = new QtDBusPlotSvrAdaptor(this);
	dbus_->connectToDBus(dbus_name_);
    }

    QtPlotServer::~QtPlotServer( ) {
	// wonder if we need to delete dbus adaptor...
    }

    QtPlotSvrPanel *QtPlotServer::panel( const QString &title, const QString &xlabel, const QString &ylabel,
					 const QString &window_title, const QString &legend ) {
	QtPlotSvrPanel *result = new QtPlotSvrPanel(title,xlabel,ylabel,window_title,legend);
	return result;
    }

}
