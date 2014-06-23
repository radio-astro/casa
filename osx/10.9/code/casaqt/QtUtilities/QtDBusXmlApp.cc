//# QtDBusXmlApp.cc: parent to use with XML message based DBus servers.
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
#include <casaqt/QtUtilities/QtDBusXmlApp.qo.h>

#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDebug>

namespace casa {

//////////////////////////////
// QTDBUSXMLAPP DEFINITIONS //
//////////////////////////////

bool QtDBusXmlApp::dbusXmlCallNoRet(const String& fromName,
        const String& objectName, const String& methodName,
        const Record& parameters, bool isAsync) {
    return dbusXmlCall(fromName, objectName, methodName, isAsync,
                          parameters, NULL);
}

// Helper macro for dbusXmlCall definitions.
#define DBA_CHELPER(TYPE, DTYPE, RECMETH)                                     \
bool QtDBusXmlApp::dbusXmlCall(const String& fromName,                        \
        const String& objectName, const String& methodName,                   \
        const Record& parameters, TYPE & retValue, bool* retValueSet) {       \
    Record rec;                                                               \
    bool res = dbusXmlCall(fromName, objectName, methodName, false,           \
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

#define CASA_DBUS_XML_SLOT "xmlSlot"

bool QtDBusXmlApp::dbusXmlCall(const String& from, const String& to,
        const String& methodName, bool methodIsAsync,
        const Record& parameters, Record* retValue) {
    if(!serviceIsAvailable(to)) return false;

    try {
        QtDBusXML xml = QtDBusXML::constructXML(from, to, methodName,
                                                methodIsAsync, parameters);

        QDBusInterface iface(to.c_str(),serviceOwner(to),CASA_DBUS_XML_INTERFACE,connection());

        Bool timeoutSet = False;
        Bool qtTripped = False;
#if QT_VERSION >= 0x040805
    if ( !methodIsAsync ){
    	iface.setTimeout (12000000); //Time out in milliseconds (Essentially infinite)
    	timeoutSet = True;
    }
    qtTripped = True;
#endif

        if(methodIsAsync) {
            iface.call(QDBus::NoBlock, CASA_DBUS_XML_SLOT, xml.toXMLQString());

        } else {
            if(retValue == NULL){
            	QDBusReply<QString> reply = iface.call(CASA_DBUS_XML_SLOT, xml.toXMLQString());
            	qDebug () << reply.error ();
            	qDebug() << "Type="<<reply.error().type();
            	qDebug() << "Timeout="<<iface.timeout()<<timeoutSet<<QT_VERSION <<" (A)";
            }
            else {
		QString slot(CASA_DBUS_XML_SLOT);
                QDBusReply<QString> reply = iface.call(slot,xml.toXMLQString());
                if(reply.isValid())
                    *retValue = QtDBusXML::fromString(
                                reply.value()).returnedValue();
                else {
                	qDebug () << reply.error ();
                	qDebug() << "Type="<<reply.error().type();
                	qDebug() << "Timeout="<<iface.timeout() << timeoutSet << " (B)";
                	qDebug() << "QtTripped="<<qtTripped<<" version="<<QT_VERSION;
                }
            }
        }
        return true;

    } catch(...) { return false; }
}


// Protected Methods //

QtDBusXmlApp::QtDBusXmlApp() : dbusRegistered_(false), dbusAdaptor_(new QtDBusXmlAppAdaptor(*this)) { }

QtDBusXmlApp::~QtDBusXmlApp() {
    if(dbusRegistered_) dbusUnregisterSelf();
    delete dbusAdaptor_;
}


bool QtDBusXmlApp::dbusRegisterSelf(const String& name) {
    if( dbusRegistered_ || (name.empty() && dbusName_.isEmpty()) || serviceIsAvailable(name) )
	return false;

    if(!name.empty()) dbusName_ = name.c_str();

    try {
        // Register service and object.
	dbusRegistered_ = connection().isConnected() &&
			  connection().registerService(dbusServiceName()) &&
			  connection().registerObject(dbusObjectName(),
						      dbusAdaptor_->itsObject_, 
						      QDBusConnection::ExportAdaptors);

    } catch(...) { dbusRegistered_ = false;  }

    return dbusRegistered_;
}

void QtDBusXmlApp::dbusUnregisterSelf() {
    try {
        if(dbusRegistered_) {
            // Unregister object.
	    connection().unregisterObject(dbusObjectName(),QDBusConnection::UnregisterNode);

            // Unregister service.
            connection().unregisterService(dbusServiceName());
        }
    } catch(...) { }

    dbusRegistered_ = false;
}

bool QtDBusXmlApp::dbusSelfIsRegistered() const { return dbusRegistered_; }

String QtDBusXmlApp::dbusSelfRegisteredName() const {
    if(dbusRegistered_) return dbusName_.toStdString();
    else                return "";
}


// Private Methods //

void QtDBusXmlApp::dbusSlot(QtDBusXML& xml) {
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

    dbusRunXmlMethod(methodName, parameters, retValue, callerName, isAsync);
    if(!isAsync) xml.setReturnedValueRec(retValue);
}


//////////////////////////////////
// QTDBUSAPPADAPTOR DEFINITIONS //
//////////////////////////////////

// Public Methods //

QString QtDBusXmlAppAdaptor::xmlSlot(const QString& xmlStr) {
    QtDBusXML xml = QtDBusXML::fromString(xmlStr);
    itsApp_.dbusSlot(xml);
    return xml.toXMLQString();
}


// Private Methods //

QtDBusXmlAppAdaptor::QtDBusXmlAppAdaptor(QtDBusXmlApp& app) :
        QDBusAbstractAdaptor(new QObject()), itsApp_(app),
        itsObject_(parent()) { }

QtDBusXmlAppAdaptor::~QtDBusXmlAppAdaptor() { }

}
