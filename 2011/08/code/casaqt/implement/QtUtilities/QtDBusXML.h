//# QtDBusXML.h: XML scheme to be used with CASA's Qt DBus communication.
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
#ifndef QTDBUSXML_H_
#define QTDBUSXML_H_

#include <casa/Containers/Record.h>

#include <QDomDocument>
#include <QDBusArgument>
#include <QMetaType>

#include <casa/namespace.h>

namespace casa {

// Subclass of QDomDocument that represents an XML scheme that is to be used
// with CASA's Qt dbus communication.
// <casa-dbus>
//   <time>[TIMESTAMP]</time>
//   <from>[NAME]</from>
//   <to>[NAME]</to>
//   <method name="[METHOD]" async="[ISASYNC]">
//     <param name="[NAME]" type="[TYPE]">[VALUE]</param>
//     ...
//   </method>
//   <returned type="[TYPE]">[VALUE]</returned>
// </casa-dbus>
// Currently supported types for parameters and returned values:
//   bool, int, uInt, double, String, Records with these types.
// For a discussion of what these fields mean, see the documentation for
// QtDBusXmlApp.
class QtDBusXML {
public:
    // Static Methods //
    
    // Constructs and returns an XML message using the given parameters.  Only
    // uses the non-empty values.  Sets the time to the current.
    // <group>
    static QtDBusXML constructXML(const String& from = "",
            const String& to = "", const String& methodName = "",
            bool methodIsAsync = false, const Record& methodParams = Record(),
            const Record& retValue = Record()) {
        return constructXML(QString(from.c_str()), QString(to.c_str()),
                QString(methodName.c_str()), methodIsAsync, methodParams,
                retValue); }
    static QtDBusXML constructXML(const QString& from = "",
            const QString& to = "", const QString& methodName = "",
            bool methodIsAsync = false, const Record& methodParams = Record(),
            const Record& retValue = Record());
    // </group>
    
    // Constructs and returns an XML message from the given XML string.
    // <group>
    static QtDBusXML fromString(const String& xmlStr) {
        return fromString(QString(xmlStr.c_str())); }
    static QtDBusXML fromString(const QString& xmlStr);
    // </group>
    
    // Reads the values from the given XML message into the given parameters
    // which are not NULL.
    // <group>
    static void extractXML(const QtDBusXML& xml, String* time = NULL,
            String* from = NULL, String* to = NULL, String* methodName = NULL,
            Record* methodParams = NULL, Record* retValue = NULL) {
        QString* qtime = NULL, *qfrom = NULL, *qto = NULL, *qmethodName = NULL;
        if(time != NULL)       qtime       = new QString();
        if(from != NULL)       qfrom       = new QString();
        if(to != NULL)         qto         = new QString();
        if(methodName != NULL) qmethodName = new QString();
        extractXML(xml, qtime, qfrom, qto, qmethodName, methodParams,retValue);
        if(time != NULL)       { *time = qtime->toStdString(); delete qtime; }
        if(from != NULL)       { *from = qfrom->toStdString(); delete qfrom; }
        if(to != NULL)         { *to = qto->toStdString(); delete qto; }
        if(methodName != NULL) { *methodName = qmethodName->toStdString();
                                 delete qmethodName; }
    }
    static void extractXML(const QtDBusXML& xml, QString* time = NULL,
            QString* from = NULL, QString* to = NULL, QString* methodName=NULL,
            Record* methodParams = NULL, Record* retValue = NULL);
    // </group>
    
    
    // Non-Static Methods //
    
    // Constructor.
    QtDBusXML();
    
    // Copy constructor, see operator=().
    QtDBusXML(const QtDBusXML& copy);
    
    // Destructor.
    virtual ~QtDBusXML();
    
    
    // Gets the value of the time tag, or an empty string if there is none.
    // <group>
    String time() const { return qtime().toStdString(); }
    QString qtime() const;
    // </group>
    
    // Sets the time tag to the current time.
    void setTime();
    
    // Gets/Sets the from tag.
    // <group>
    String from() const { return qfrom().toStdString(); }
    QString qfrom() const;
    void setFrom(const String& value) { setFrom(QString(value.c_str())); }
    void setFrom(const QString& value);
    // </group>
    
    // Gets/Sets the to tag.
    // <group>
    String to() const { return qto().toStdString(); }
    QString qto() const;
    void setTo(const String& value) { setTo(QString(value.c_str())); }
    void setTo(const QString& value);
    // </group>
    
    // Gets/Sets the method name and whether the method call is asynchronous or
    // not (default is false).
    // <group>
    String methodName() const { return qmethodName().toStdString(); }
    QString qmethodName() const;
    bool methodIsAsync() const;
    void setMethodName(const String& value, bool isAsync = false) {
        setMethodName(QString(value.c_str()), isAsync); }
    void setMethodName(const QString& value, bool isAsync = false);
    void setMethodIsAsync(bool value);
    // </group>
    
    // Returns the type of the method parameter with the given name, or an
    // empty string if there is none.
    // <group>
    String methodParamType(const String& paramName) const {
        return qmethodParamType(QString(paramName.c_str())).toStdString(); }
    QString qmethodParamType(const QString& paramName) const;
    // </group>
    
    // Returns whether the method parameter with the given name is the
    // specified type or not.
    // <group>
    bool methodParamIsBool(const String& paramName) const;
    bool methodParamIsInt(const String& paramName) const;
    bool methodParamIsUInt(const String& paramName) const;
    bool methodParamIsDouble(const String& paramName) const;
    bool methodParamIsString(const String& paramName) const;
    bool methodParamIsRecord(const String& paramName) const;
    // </group>
    
    // Returns the value of the method parameter with the given name as the
    // specified type.  Is invalid if that parameter is not of the requested
    // type.  Note: the value can always be returned as a string
    // representation.
    // <group>
    bool methodParamBool(const String& paramName) const {
        return methodParamBool(QString(paramName.c_str())); }
    bool methodParamBool(const QString& paramName) const;
    int methodParamInt(const String& paramName) const {
        return methodParamInt(QString(paramName.c_str())); }
    int methodParamInt(const QString& paramName) const;
    uInt methodParamUInt(const String& paramName) const {
        return methodParamUInt(QString(paramName.c_str())); }
    uInt methodParamUInt(const QString& paramName) const;
    double methodParamDouble(const String& paramName) const {
        return methodParamDouble(QString(paramName.c_str())); }
    double methodParamDouble(const QString& paramName) const;
    String methodParamString(const String& paramName) const {
        return methodParamQString(QString(paramName.c_str())).toStdString(); }
    QString methodParamQString(const QString& paramName) const;
    Record methodParamRecord(const String& paramName) const {
        return methodParamRecord(QString(paramName.c_str())); }
    Record methodParamRecord(const QString& paramName) const;
    // </group>
    
    // Sets the parameter with the given name to the given value (and
    // associated type).
    // <group>
    void setMethodParam(const String& paramName, bool value) {
        setMethodParam(QString(paramName.c_str()), value); }
    void setMethodParam(const QString& paramName, bool value);
    void setMethodParam(const String& paramName, int value) {
        setMethodParam(QString(paramName.c_str()), value); }
    void setMethodParam(const QString& paramName, int value);
    void setMethodParam(const String& paramName, uInt value) {
        setMethodParam(QString(paramName.c_str()), value); }
    void setMethodParam(const QString& paramName, uInt value);
    void setMethodParam(const String& paramName, double value) {
        setMethodParam(QString(paramName.c_str()), value); }
    void setMethodParam(const QString& paramName, double value);
    void setMethodParam(const String& paramName, const String& value) {
        setMethodParam(QString(paramName.c_str()), QString(value.c_str())); }
    void setMethodParam(const QString& paramName, const QString& value);
    void setMethodParam(const String& paramName, const Record& value) {
        setMethodParam(QString(paramName.c_str()), value); }
    void setMethodParam(const QString& paramName, const Record& value);
    // </group>
    
    // Gets/Sets all method parameter values as a Record.
    // <group>
    Record methodParams() const;
    void setMethodParams(const Record& parameters);
    // </group>
    
    // Returns whether or not a returned value was set.
    bool returnedSet() const { return !qreturnedType().isEmpty(); }
    
    // Returns the type of the returned value, or empty string for none.
    // <group>
    String returnedType() const { return qreturnedType().toStdString(); }
    QString qreturnedType() const;
    // </group>
    
    // Returns whether the returned value is the specified type or not.
    // <group>
    bool returnedIsBool() const;
    bool returnedIsInt() const;
    bool returnedIsUInt() const;
    bool returnedIsDouble() const;
    bool returnedIsString() const;
    bool returnedIsRecord() const;
    // </group>
    
    // Returns the returned value as the specified type.  Is invalid if that
    // parameter is not of the requested type.  Note: the value can always be
    // returned as a string representation.
    // <group>
    bool returnedBool() const;
    int returnedInt() const;
    uInt returnedUInt() const;
    double returnedDouble() const;
    String returnedString() const { return returnedQString().toStdString(); }
    QString returnedQString() const;
    Record returnedRecord() const;
    // </group>
    
    // Sets the returned value to the given value (and associated type).
    // <group>
    void setReturnedValue(bool value);
    void setReturnedValue(int value);
    void setReturnedValue(uInt value);
    void setReturnedValue(double value);
    void setReturnedValue(const String& value) {
        setReturnedValue(QString(value.c_str())); }
    void setReturnedValue(const QString& value);
    void setReturnedValue(const Record& value);
    // </group>
    
    // Gets/Sets the returned value as a record.  ONLY the first field is used.
    // <group>
    Record returnedValue() const;
    void setReturnedValueRec(const Record& retValue);
    // </group>
    
    
    // Returns the whole XML as a string.
    // <group>
    String toXMLString() const { return toXMLQString().toStdString(); }
    QString toXMLQString() const;
    // </group>
    
    // Sets the whole XML as a string, and returns whether the operation
    /// succeeded or not.
    // <group>
    bool fromXMLString(const String& value) {
        return fromXMLString(QString(value.c_str())); }
    bool fromXMLString(const QString& value);
    // </group>
    
    // Returns the underlying QDomDocument.
    // <group>
    QDomDocument& domDocument();
    const QDomDocument& domDocument() const;
    // </group>
    
    
    // Copy operator.
    QtDBusXML& operator=(const QtDBusXML& copy);
    
private:
    // XML document.
    QDomDocument itsXML_;
    
    
    // Initialize object; meant to be called from constructor.
    void initialize();
    
    // Helper method for elemChildText().
    QString elemChildText(const QString& name) const {
        return elemChildText(itsXML_, name, false); }
    
    // Helper method for setElemChildText().
    void setElemChildText(const QString& name, const QString& value) {
        setElemChildText(itsXML_, name, value, true); }
    
    // Helper method that returns the element for the method parameter with the
    // given name, or a null element if it is not.  See elemChild().
    QDomElement methodParam(const QString& paramName,
            bool createIfAbsent = false) const;
    
    // Helper method for setting the method parameter values.
    void setMethodParam(const QString& name, const QString& type,
            const QString& value);
    
    // Helper method for setting the returned value.
    void setReturnedValue(const QString& type, const QString& value);
    
    
    // Static //
    
    // Converts between QStrings and bools.
    // <group>
    static bool qstringToBool(const QString& value);
    static QString qstringFromBool(bool value);
    // </group>
    
    // Returns the child of the given element with the given tag name.  If
    // createIfAbsent is true, then the element will be created and appended if
    // it is not present; otherwise, the returned element will be null if not
    // present.  If the given element is null, a null element is returned.
    // <group>
    static QDomElement elemChild(QDomDocument doc, const QString& name,
            bool createIfAbsent = false) {
        return elemChild(doc.documentElement(), name, createIfAbsent); }
    static QDomElement elemChild(QDomElement elem, const QString& name,
            bool createIfAbsent = false);
    // </group>
    
    // Returns the text value of the child of the given element with the given
    // tag name.  See elemChild().
    // <group>
    static QString elemChildText(QDomDocument doc, const QString& name,
            bool createIfAbsent = false) {
        return elemChildText(doc.documentElement(), name, createIfAbsent); }
    static QString elemChildText(QDomElement elem, const QString& name,
            bool createIfAbsent = false) {
        return elemChild(elem, name, createIfAbsent).text();
    }
    // </group>
    
    // Sets the text value of the child of the given element with the given tag
    // name to the given value.  See elemChild().
    // <group>
    static void setElemChildText(QDomDocument doc, const QString& name,
            const QString& value, bool createIfAbsent = false) {
        setElemChildText(doc.documentElement(), name, value, createIfAbsent); }
    static void setElemChildText(QDomElement elem, const QString& name,
            const QString& value, bool createIfAbsent = false) {
        setElemText(elemChild(elem, name, createIfAbsent), value); }
    // </group>
    
    // Sets the text value of the given element (if it is not null) to the
    // given text.
    static void setElemText(QDomElement elem, const QString& text);
    
    // Converts between a QDomElement and Record for values.
    // <group>
    static Record elemToRecord(QDomElement value);
    static void elemToRecord(Record& rec, QDomElement value);
    static void elemFromRecord(QDomElement elem, const Record& value);
    // </group>
};

}

#endif /* QTDBUSXML_H_ */
