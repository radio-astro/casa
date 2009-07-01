//# dbus_cmpt.cc: dbus component tool.
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
#include <xmlcasa/utils/dbus_cmpt.h>

#include <casaqt/QtUtilities/QtDBusApp.qo.h>
#include <xmlcasa/StdCasa/CasacSupport.h>

namespace casac {

//////////////////////
// DBUS DEFINITIONS //
//////////////////////

// Static //

const String dbus::FROM_NAME = "casapy";


bool dbus::asyncCallMethod_(const string& objectName, const string& methodName,
        const record& parameters) {
    Record* params = toRecord(parameters);
    bool res = QtDBusApp::dbusCallMethodNoRet(FROM_NAME, objectName,
            methodName, *params, true);
    delete params;
    return res;
}


// Non-Static //

dbus::dbus() { }

dbus::~dbus() { }

bool dbus::asyncCallMethod(const string& objectName, const string& methodName,
        const record& parameters) {
    return asyncCallMethod_(objectName, methodName, parameters); }

}

