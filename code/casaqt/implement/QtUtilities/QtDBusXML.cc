//# QtDBusXML.cc: XML scheme to be used with CASA's Qt DBus communication.
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
#include <casaqt/QtUtilities/QtDBusXML.h>

#include <QDateTime>

namespace casa {

///////////////////////////
// QTDBUSXML DEFINITIONS //
///////////////////////////

// Public Static //

QtDBusXML QtDBusXML::constructXML(const QString& from, const QString& to,
        const QString& methodName, bool methodIsAsync,
        const Record& methodParams, const Record& retValue) {
    QtDBusXML xml;
    if(!from.isEmpty()) xml.setFrom(from);
    if(!to.isEmpty()) xml.setTo(to);
    if(!methodName.isEmpty()) xml.setMethodName(methodName, methodIsAsync);
    if(!methodParams.empty()) xml.setMethodParams(methodParams);
    if(!retValue.empty()) xml.setReturnedValueRec(retValue);
    return xml;
}

QtDBusXML QtDBusXML::fromString(const QString& xmlStr) {
    QtDBusXML xml;
    xml.fromXMLString(xmlStr);
    return xml;
}

void QtDBusXML::extractXML(const QtDBusXML& xml, QString* time, QString* from,
        QString* to, QString* methodName, Record* methodParams,
        Record* retValue) {
    if(time != NULL) *time = xml.qtime();
    if(from != NULL) *from = xml.qfrom();
    if(to != NULL) *to = xml.qto();
    if(methodName != NULL) *methodName = xml.qmethodName();
    if(methodParams != NULL) *methodParams = xml.methodParams();
    if(retValue != NULL) *retValue = xml.returnedValue();
}


// Private Static //

#define XML_DOCNAME       "casa-dbus"
#define XML_TIME          "time"
#define XML_FROM          "from"
#define XML_TO            "to"
#define XML_METHOD        "method"
#define XML_METHOD_NAME   "name"
#define XML_METHOD_ASYNC  "async"
#define XML_METHOD_PARAM  "param"
#define XML_METHOD_TYPE   "type"
#define XML_RETURNED      "returned"
#define XML_RETURNED_TYPE "type"
#define XML_RECORD_ENTRY  "entry"
#define XML_RECORD_KEY    "key"
#define XML_RECORD_TYPE   "Type"

#define TYPE_BOOL         "bool"
#define TYPE_INT          "int"
#define TYPE_UINT         "uint"
#define TYPE_DOUBLE       "double"
#define TYPE_STRING       "string"
#define TYPE_RECORD       "record"



bool QtDBusXML::methodParamIsBool(const String& paramName) const {
        return qmethodParamType(QString(paramName.c_str())) == TYPE_BOOL; }
bool QtDBusXML::methodParamIsInt(const String& paramName) const {
        return qmethodParamType(QString(paramName.c_str())) == TYPE_INT; }
bool QtDBusXML::methodParamIsUInt(const String& paramName) const {
        return qmethodParamType(QString(paramName.c_str())) == TYPE_UINT; }
bool QtDBusXML::methodParamIsDouble(const String& paramName) const {
        return qmethodParamType(QString(paramName.c_str())) == TYPE_DOUBLE; }
bool QtDBusXML::methodParamIsString(const String& paramName) const {
        return qmethodParamType(QString(paramName.c_str())) == TYPE_STRING; }
bool QtDBusXML::methodParamIsRecord(const String& paramName) const {
        return qmethodParamType(QString(paramName.c_str())) == TYPE_RECORD; }



bool QtDBusXML::returnedIsBool() const { return qreturnedType() == TYPE_BOOL; }
bool QtDBusXML::returnedIsInt() const { return qreturnedType() == TYPE_INT; }
bool QtDBusXML::returnedIsUInt() const { return qreturnedType() == TYPE_UINT; }
bool QtDBusXML::returnedIsDouble() const { return qreturnedType() == TYPE_DOUBLE; }
bool QtDBusXML::returnedIsString() const { return qreturnedType() == TYPE_STRING; }
bool QtDBusXML::returnedIsRecord() const { return qreturnedType() == TYPE_RECORD; }




bool QtDBusXML::qstringToBool(const QString& value) {
    return !(value == "0" || value.compare("f", Qt::CaseInsensitive) == 0 ||
             value.compare("false", Qt::CaseInsensitive) == 0); }
QString QtDBusXML::qstringFromBool(bool value) { return value ? "1" : "0"; }

QDomElement QtDBusXML::elemChild(QDomElement elem, const QString& name,
        bool createIfAbsent) {
    if(elem.isNull()) return QDomElement();
    QDomElement chElem = elem.firstChildElement(name);
    if(chElem.isNull() && createIfAbsent) {
        chElem = elem.ownerDocument().createElement(name);
        elem.appendChild(chElem);
    }
    return chElem;
}

void QtDBusXML::setElemText(QDomElement elem, const QString& text) {
    if(elem.isNull()) return;
    elem.normalize(); // consolidate all QDomTexts into one
    // Remove current text if it exists.
    for(QDomNode n = elem.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomText t = n.toText();
        if(!t.isNull()) {
            elem.removeChild(t);
            break;
        }
    }
    if(!text.isEmpty()) 
        elem.appendChild(elem.ownerDocument().createTextNode(text));
}

Record QtDBusXML::elemToRecord(QDomElement value) {
    Record rec;
    QDomNodeList children = value.childNodes();
    QDomElement elem;
    for(int i = 0; i < children.size(); i++) {
        elem = children.at(i).toElement();
        if(elem.isNull() || elem.tagName() != XML_RECORD_ENTRY) continue;
        elemToRecord(rec, elem);
    }
    return rec;
}

void QtDBusXML::elemToRecord(Record& rec, QDomElement value) {
    if(value.isNull() || value.attribute(XML_RECORD_KEY).isEmpty() ||
       value.attribute(XML_RECORD_TYPE).isEmpty()) return;
    String key = value.attribute(XML_RECORD_KEY).toStdString();
    QString type = value.attribute(XML_RECORD_TYPE);
    if(type == TYPE_BOOL)
        rec.define(key, qstringToBool(value.text()));
    else if(type == TYPE_INT)
        rec.define(key, value.text().toInt());
    else if(type == TYPE_UINT)
        rec.define(key, value.text().toUInt());
    else if(type == TYPE_DOUBLE)
        rec.define(key, value.text().toDouble());
    else if(type == TYPE_STRING)
        rec.define(key, value.text().toStdString());
    else if(type == TYPE_RECORD)
        rec.defineRecord(key, elemToRecord(value));
}

void QtDBusXML::elemFromRecord(QDomElement elem, const Record& value) {
    if(elem.isNull()) return;
    DataType t;
    QDomElement entry;
    for(unsigned int i = 0; i < value.nfields(); i++) {
        t = value.dataType(i);
        entry = elem.ownerDocument().createElement(XML_RECORD_ENTRY);
        entry.setAttribute(XML_RECORD_KEY, value.name(i).c_str());
        
        switch(t) {
        case TpBool:            
            entry.setAttribute(XML_RECORD_TYPE, TYPE_BOOL);
            setElemText(entry, qstringFromBool(value.asBool(i)));
            break;
            
        case TpInt:
            entry.setAttribute(XML_RECORD_TYPE, TYPE_INT);
            setElemText(entry, QString::number(value.asInt(i)));
            break;
            
        case TpUInt:
            entry.setAttribute(XML_RECORD_TYPE, TYPE_UINT);
            setElemText(entry, QString::number(value.asuInt(i)));
            break;
            
        case TpDouble:
            entry.setAttribute(XML_RECORD_TYPE, TYPE_DOUBLE);
            setElemText(entry, QString::number(value.asDouble(i)));
            break;
            
        case TpString:
            entry.setAttribute(XML_RECORD_TYPE, TYPE_STRING);
            setElemText(entry, value.asString(i).c_str());
            break;
            
        case TpRecord:
            entry.setAttribute(XML_RECORD_TYPE, TYPE_RECORD);
            elemFromRecord(entry, value.asRecord(i));
            break;
            
        default: continue;
        }
        
        elem.appendChild(entry);
    }
}


// Constructors/Destructors //

QtDBusXML::QtDBusXML() {
    initialize();
}

QtDBusXML::QtDBusXML(const QtDBusXML& copy) {
    initialize();
    operator=(copy);
}

QtDBusXML::~QtDBusXML() { }


// Public Methods //

QString QtDBusXML::qtime() const { return elemChildText(XML_TIME); }
void QtDBusXML::setTime() {
    setElemChildText(XML_TIME, QDateTime::currentDateTime().toString()); }

QString QtDBusXML::qfrom() const { return elemChildText(XML_FROM); }
void QtDBusXML::setFrom(const QString& value) {
    setElemChildText(XML_FROM, value); }

QString QtDBusXML::qto() const { return elemChildText(XML_TO); }
void QtDBusXML::setTo(const QString& value) { setElemChildText(XML_TO, value);}

QString QtDBusXML::qmethodName() const {
    QDomElement elem = itsXML_.documentElement().firstChildElement(XML_METHOD);
    if(elem.isNull()) return QString();
    else return elem.attribute(XML_METHOD_NAME, QString());
}

bool QtDBusXML::methodIsAsync() const {
    QDomElement elem = itsXML_.documentElement().firstChildElement(XML_METHOD);
    if(elem.isNull()) return false;
    else return qstringToBool(elem.attribute(XML_METHOD_ASYNC,
                                             qstringFromBool(false)));
}

void QtDBusXML::setMethodName(const QString& value, bool isAsync) {
    QDomElement elem = itsXML_.documentElement().firstChildElement(XML_METHOD);
    if(elem.isNull()) {
        elem = itsXML_.createElement(XML_METHOD);
        itsXML_.documentElement().appendChild(elem);
    }
    elem.setAttribute(XML_METHOD_NAME, value);
    elem.setAttribute(XML_METHOD_ASYNC, qstringFromBool(isAsync));
}

void QtDBusXML::setMethodIsAsync(bool value) {
    QDomElement elem = itsXML_.documentElement().firstChildElement(XML_METHOD);
    if(elem.isNull()) {
        elem = itsXML_.createElement(XML_METHOD);
        itsXML_.documentElement().appendChild(elem);
    }
    elem.setAttribute(XML_METHOD_ASYNC, qstringFromBool(value));
}

QString QtDBusXML::qmethodParamType(const QString& paramName) const {
    QDomElement elem = methodParam(paramName);
    if(elem.isNull()) return QString();
    else return elem.attribute(XML_METHOD_TYPE, QString());
}

bool QtDBusXML::methodParamBool(const QString& paramName) const {
    QDomElement elem = methodParam(paramName);
    if(elem.isNull() || elem.attribute(XML_METHOD_TYPE) != TYPE_BOOL)
        return false;
    QString value = elem.text();
    if(value.isEmpty()) return false; // shouldn't happen
    else return qstringToBool(value);
}

int QtDBusXML::methodParamInt(const QString& paramName) const {
    QDomElement elem = methodParam(paramName);
    if(elem.isNull() || elem.attribute(XML_METHOD_TYPE) != TYPE_INT) return 0;
    QString value = elem.text();
    if(value.isEmpty()) return 0; // shouldn't happen
    else return value.toInt();
}

uInt QtDBusXML::methodParamUInt(const QString& paramName) const {
    QDomElement elem = methodParam(paramName);
    if(elem.isNull() || elem.attribute(XML_METHOD_TYPE) != TYPE_UINT) return 0;
    QString value = elem.text();
    if(value.isEmpty()) return 0; // shouldn't happen
    else return value.toUInt();
}

double QtDBusXML::methodParamDouble(const QString& paramName) const {
    QDomElement elem = methodParam(paramName);
    if(elem.isNull() || elem.attribute(XML_METHOD_TYPE)!=TYPE_DOUBLE) return 0;
    QString value = elem.text();
    if(value.isEmpty()) return 0; // shouldn't happen
    else return value.toDouble();
}

QString QtDBusXML::methodParamQString(const QString& paramName) const {
    QDomElement elem = methodParam(paramName);
    if(elem.isNull() || elem.attribute(XML_METHOD_TYPE) != TYPE_STRING)
        return QString();
    else return elem.text();
}

Record QtDBusXML::methodParamRecord(const QString& paramName) const {
    QDomElement elem = methodParam(paramName);
    if(!elem.isNull() && elem.attribute(XML_METHOD_TYPE) == TYPE_RECORD)
        return elemToRecord(elem);
    else return Record();
}

void QtDBusXML::setMethodParam(const QString& paramName, bool value) {
    setMethodParam(paramName, TYPE_BOOL, qstringFromBool(value)); }

void QtDBusXML::setMethodParam(const QString& paramName, int value) {
    setMethodParam(paramName, TYPE_INT, QString::number(value)); }

void QtDBusXML::setMethodParam(const QString& paramName, uInt value) {
    setMethodParam(paramName, TYPE_UINT, QString::number(value)); }

void QtDBusXML::setMethodParam(const QString& paramName, double value){
    setMethodParam(paramName, TYPE_DOUBLE, QString::number(value)); }

void QtDBusXML::setMethodParam(const QString& paramName, const QString& value){
    setMethodParam(paramName, TYPE_STRING, value); }

void QtDBusXML::setMethodParam(const QString& paramName, const Record& value) {
    QDomElement elem = methodParam(paramName, true);
    elem.setAttribute(XML_METHOD_TYPE, TYPE_RECORD);
    elemFromRecord(elem, value);
}

Record QtDBusXML::methodParams() const {
    Record rec;
    
    QDomElement elem = itsXML_.documentElement().firstChildElement(XML_METHOD);
    if(elem.isNull()) return rec;
    
    QDomNodeList params = elem.elementsByTagName(XML_METHOD_PARAM);
    QString name, type;
    for(int i = 0; i < params.size(); i++) {
        elem = params.at(i).toElement();
        if(!elem.isNull()) {
            name = elem.attribute(XML_METHOD_NAME);
            type = elem.attribute(XML_METHOD_TYPE);
            if(type == TYPE_BOOL)
                rec.define(name.toStdString(), methodParamBool(name));
            else if(type == TYPE_INT)
                rec.define(name.toStdString(), methodParamInt(name));
            else if(type == TYPE_UINT)
                rec.define(name.toStdString(), methodParamUInt(name));
            else if(type == TYPE_DOUBLE)
                rec.define(name.toStdString(), methodParamDouble(name));
            else if(type == TYPE_STRING)
                rec.define(name.toStdString(),
                           methodParamString(name.toStdString()));
            else if(type == TYPE_RECORD)
                rec.defineRecord(name.toStdString(),
                        elemToRecord(elem));
        }
    }
    
    return rec;    
}

void QtDBusXML::setMethodParams(const Record& parameters) {
    QDomElement elem = itsXML_.documentElement().firstChildElement(XML_METHOD);
    if(elem.isNull()) {
        elem = itsXML_.createElement(XML_METHOD);
        itsXML_.appendChild(elem);
    }
    
    // Set up method parameters for supported types.
    String name;
    for(uInt i = 0; i < parameters.nfields(); i++) {
        name = parameters.name(i);
        switch(parameters.dataType(i)) {
        case TpBool:   setMethodParam(name, parameters.asBool(i)); break;
        case TpInt:    setMethodParam(name, parameters.asInt(i)); break;
        case TpUInt:   setMethodParam(name, parameters.asuInt(i)); break;
        case TpDouble: setMethodParam(name, parameters.asDouble(i)); break;
        case TpString: setMethodParam(name, parameters.asString(i)); break;
        case TpRecord: setMethodParam(name, parameters.asRecord(i)); break;
            
        default: break;
        }
    }
}

QString QtDBusXML::qreturnedType() const {
    QDomElement elem=itsXML_.documentElement().firstChildElement(XML_RETURNED);
    if(elem.isNull()) return QString();
    else return elem.attribute(XML_RETURNED_TYPE, QString());
}

bool QtDBusXML::returnedBool() const {
    if(!returnedIsBool()) return false;
    else return qstringToBool(returnedQString());
}

int QtDBusXML::returnedInt() const {
    if(!returnedIsInt()) return 0;
    else return returnedQString().toInt();
}

uInt QtDBusXML::returnedUInt() const {
    if(!returnedIsUInt()) return 0;
    else return returnedQString().toUInt();
}

double QtDBusXML::returnedDouble() const {
    if(!returnedIsDouble()) return 0;
    else return returnedQString().toDouble();
}

QString QtDBusXML::returnedQString() const {
    QDomElement elem=itsXML_.documentElement().firstChildElement(XML_RETURNED);
    if(elem.isNull()) return QString();
    else return elem.text();
}

Record QtDBusXML::returnedRecord() const {
    return elemToRecord(itsXML_.documentElement().firstChildElement(
                        XML_RETURNED));
}

void QtDBusXML::setReturnedValue(bool value) {
    setReturnedValue(TYPE_BOOL, qstringFromBool(value)); }
void QtDBusXML::setReturnedValue(int value) {
    setReturnedValue(TYPE_INT, QString::number(value)); }
void QtDBusXML::setReturnedValue(uInt value) {
    setReturnedValue(TYPE_UINT, QString::number(value)); }
void QtDBusXML::setReturnedValue(double value) {
    setReturnedValue(TYPE_DOUBLE, QString::number(value)); }
void QtDBusXML::setReturnedValue(const QString& value) {
    setReturnedValue(TYPE_STRING, value); }
void QtDBusXML::setReturnedValue(const Record& value) {
    QDomElement elem = elemChild(itsXML_, XML_RETURNED, true);
    elem.setAttribute(XML_RETURNED_TYPE, TYPE_RECORD);
    elemFromRecord(elem, value);
}

Record QtDBusXML::returnedValue() const {
    Record ret;
    QDomElement elem=itsXML_.documentElement().firstChildElement(XML_RETURNED);
    if(!elem.isNull()) {
        QString type = elem.attribute(XML_RETURNED_TYPE);
        if(type == TYPE_BOOL)        ret.define("", returnedBool());
        else if(type == TYPE_INT)    ret.define("", returnedInt());
        else if(type == TYPE_UINT)   ret.define("", returnedUInt());
        else if(type == TYPE_DOUBLE) ret.define("", returnedDouble());
        else if(type == TYPE_STRING) ret.define("", returnedString());
        else if(type == TYPE_RECORD) ret.defineRecord("", returnedRecord());
    }
    return ret;
}

void QtDBusXML::setReturnedValueRec(const Record& retValue) {
    if(retValue.nfields() > 0) {
        switch(retValue.dataType(0)) {
        case TpBool:   setReturnedValue(retValue.asBool(0)); break;
        case TpInt:    setReturnedValue(retValue.asInt(0)); break;
        case TpUInt:   setReturnedValue(retValue.asuInt(0)); break;
        case TpDouble: setReturnedValue(retValue.asDouble(0)); break;
        case TpString: setReturnedValue(retValue.asString(0)); break;
        case TpRecord: setReturnedValue(retValue.asRecord(0)); break;
            
        default: break;
        }
    }
}


QString QtDBusXML::toXMLQString() const { return itsXML_.toString(); }
bool QtDBusXML::fromXMLString(const QString& value) {
    QString error;
    itsXML_.setContent(value, &error);
    return !error.isEmpty();
}

QDomDocument& QtDBusXML::domDocument() { return itsXML_; }
const QDomDocument& QtDBusXML::domDocument() const { return itsXML_; }


QtDBusXML& QtDBusXML::operator=(const QtDBusXML& copy) {
    itsXML_ = copy.itsXML_;
    return *this;
}


// Private Methods //

void QtDBusXML::initialize() {
    itsXML_.appendChild(itsXML_.createElement(XML_DOCNAME));
    setTime();
}

QDomElement QtDBusXML::methodParam(const QString& paramName,
        bool createIfAbsent) const {
    QDomElement elem = elemChild(itsXML_, XML_METHOD, createIfAbsent);
    if(elem.isNull()) return QDomElement();

    QDomNodeList params = elem.elementsByTagName(XML_METHOD_PARAM);
    bool found = false;
    QDomElement pelem;
    for(int i = 0; !found && i < params.size(); i++) {
        pelem = params.at(i).toElement();
        if(!pelem.isNull() && pelem.attribute(XML_METHOD_NAME) == paramName)
            found = true;
    }
    
    if(found) return pelem;
    else if(createIfAbsent) {
        pelem = const_cast<QDomDocument&>(itsXML_).createElement(
                XML_METHOD_PARAM);
        pelem.setAttribute(XML_METHOD_NAME, paramName);
        elem.appendChild(pelem);
        return pelem;
    } else return QDomElement();
}

void QtDBusXML::setMethodParam(const QString& name, const QString& type,
        const QString& value) {
    QDomElement elem = methodParam(name, true);
    elem.setAttribute(XML_METHOD_TYPE, type);
    setElemText(elem, value);
}

void QtDBusXML::setReturnedValue(const QString& type, const QString& value) {
    QDomElement elem = elemChild(itsXML_, XML_RETURNED, true);
    elem.setAttribute(XML_RETURNED_TYPE, type);
    setElemText(elem, value);
}

}
