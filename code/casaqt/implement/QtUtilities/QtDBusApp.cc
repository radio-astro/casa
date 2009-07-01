//# QtDBusApp.cc: Abstract parent to use the CASA DBus server.
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
#include <casaqt/QtUtilities/QtDBusApp.qo.h>

#include <QDBusConnectionInterface>
#include <QDBusInterface>

namespace casa {

///////////////////////////
// QTDBUSAPP DEFINITIONS //
///////////////////////////

// Public Static //

bool QtDBusApp::dbusNameIsRegistered(const String& objectName) {
    return DBUS_CONNECTION.isConnected() &&
           DBUS_CONNECTION.interface()->isServiceRegistered(
                   DBUS_SERVICE_BASE + objectName.c_str());
}

bool QtDBusApp::dbusCallMethodNoRet(const String& fromName,
        const String& objectName, const String& methodName,
        const Record& parameters, bool isAsync) {
    return dbusCallMethod(fromName, objectName, methodName, isAsync,
                          parameters, NULL);
}

// Helper macro for dbusCallMethod definitions.
#define DBA_CHELPER(TYPE, DTYPE, RECMETH)                                     \
bool QtDBusApp::dbusCallMethod(const String& fromName,                        \
        const String& objectName, const String& methodName,                   \
        const Record& parameters, TYPE & retValue, bool* retValueSet) {       \
    Record rec;                                                               \
    bool res = dbusCallMethod(fromName, objectName, methodName, false,        \
                              parameters, &rec);                              \
    if(rec.nfields() > 0 && rec.dataType(0) == DTYPE ) {                      \
        retValue = rec. RECMETH (0);                                          \
        if(retValueSet != NULL) *retValueSet = true;                          \
    } else if(retValueSet != NULL) *retValueSet = false;                      \
    return res;                                                               \
}

DBA_CHELPER(bool, TpBool, asBool)
DBA_CHELPER(int, TpInt, asInt)
DBA_CHELPER(uInt, TpUInt, asuInt)
DBA_CHELPER(double, TpDouble, asDouble)
DBA_CHELPER(String, TpString, asString)
DBA_CHELPER(Record, TpRecord, asRecord)


// Private Static //

const QString QtDBusApp::DBUS_SERVICE_BASE   = "edu.nrao.casa.";
const QString QtDBusApp::DBUS_OBJECT_BASE    = "/casa/";
const QString QtDBusApp::DBUS_INTERFACE_NAME = CASA_DBUS_INTERFACE;
const QString QtDBusApp::DBUS_MESSAGE_SLOT   = "messageSlot";

const QDBusConnection QtDBusApp::DBUS_CONNECTION=QDBusConnection::sessionBus();


bool QtDBusApp::dbusCallMethod(const String& from, const String& to,
        const String& methodName, bool methodIsAsync,
        const Record& parameters, Record* retValue) {
    if(!dbusNameIsRegistered(to)) return false;
    
    try {
        QtDBusXML xml = QtDBusXML::constructXML(from, to, methodName,
                                                methodIsAsync, parameters);
        
        QDBusInterface iface(
                DBUS_SERVICE_BASE + to.c_str(),
                DBUS_OBJECT_BASE + to.c_str(),
                DBUS_INTERFACE_NAME,
                DBUS_CONNECTION);

        if(methodIsAsync) {
            iface.call(QDBus::NoBlock, DBUS_MESSAGE_SLOT, xml.toXMLQString());
            
        } else {
            if(retValue == NULL)
                iface.call(DBUS_MESSAGE_SLOT, xml.toXMLQString());
            else {
                QDBusReply<QString> reply = iface.call(DBUS_MESSAGE_SLOT,
                                            xml.toXMLQString());
                if(reply.isValid())
                    *retValue = QtDBusXML::fromString(
                                reply.value()).returnedValue();
            }
        }
        return true;
        
    } catch(...) { return false; }
}


// Protected Methods //

QtDBusApp::QtDBusApp() : dbusConnection_(DBUS_CONNECTION),
        dbusRegistered_(false), dbusAdaptor_(new QtDBusAppAdaptor(*this)) { }

QtDBusApp::~QtDBusApp() {
    if(dbusRegistered_) dbusUnregisterSelf();
    delete dbusAdaptor_;
}


bool QtDBusApp::dbusRegisterSelf(const String& name) {
    if(dbusRegistered_ || (name.empty() && dbusName_.isEmpty()) ||
       dbusNameIsRegistered(name)) return false;
    
    if(!name.empty()) dbusName_ = name.c_str();
    
    try {
        // Register service and object.
        dbusRegistered_ =
            dbusConnection_.isConnected() &&
            dbusConnection_.registerService(
                    DBUS_SERVICE_BASE + dbusName_) &&
            dbusConnection_.registerObject(
                    DBUS_OBJECT_BASE + dbusName_,
                    dbusAdaptor_->itsObject_, QDBusConnection::ExportAdaptors);
        
    } catch(...) { dbusRegistered_ = false;  }
    
    return dbusRegistered_;
}

void QtDBusApp::dbusUnregisterSelf() {
    try {
        if(dbusRegistered_) {
            // Unregister object.
            dbusConnection_.unregisterObject(DBUS_OBJECT_BASE + dbusName_,
                    QDBusConnection::UnregisterNode);
            
            // Unregister service.
            dbusConnection_.unregisterService(DBUS_SERVICE_BASE + dbusName_);
        }
    } catch(...) { }
    
    dbusRegistered_ = false;
}

bool QtDBusApp::dbusSelfIsRegistered() const { return dbusRegistered_; }

String QtDBusApp::dbusSelfRegisteredName() const {
    if(dbusRegistered_) return dbusName_.toStdString();
    else                return "";
}


// Private Methods //

void QtDBusApp::dbusSlot(QtDBusXML& xml) {
    dbusXmlReceived(xml);
    
    /*
    // IMPORTANT: only proceed if the message is for this object, or sent to
    // all applications.
    QString to = xml.qto();
    if(!to.isEmpty() && to != dbusName_) return;
    */
    
    String methodName = xml.methodName(), callerName = xml.from();
    Record parameters = xml.methodParams(), retValue;
    bool isAsync = xml.methodIsAsync();
    
    dbusRunMethod(methodName, parameters, retValue, callerName, isAsync);
    if(!isAsync) xml.setReturnedValueRec(retValue);
}


//////////////////////////////////
// QTDBUSAPPADAPTOR DEFINITIONS //
//////////////////////////////////

// Public Methods //

QString QtDBusAppAdaptor::messageSlot(const QString& xmlStr) {
    QtDBusXML xml = QtDBusXML::fromString(xmlStr);
    itsApp_.dbusSlot(xml);
    return xml.toXMLQString();
}


// Private Methods //

QtDBusAppAdaptor::QtDBusAppAdaptor(QtDBusApp& app) :
        QDBusAbstractAdaptor(new QObject()), itsApp_(app),
        itsObject_(parent()) { }

QtDBusAppAdaptor::~QtDBusAppAdaptor() { }

}
