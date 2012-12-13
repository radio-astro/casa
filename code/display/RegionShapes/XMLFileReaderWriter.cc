//# XMLFileReaderWriter.cc: Implementation for reading/writing XML files.
//# Copyright (C) 2008
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
#include <QLabel>

#include <display/RegionShapes/XMLFileReaderWriter.h>

namespace casa {

// RSXMLRECORD DEFINITIONS //

QDomElement RSXML::recordToXML(const RecordInterface& r, String name) {
    QDomElement topElem = DOCUMENT.createElement(name.c_str());
    
    QString s("%1"), type, text;
    for(unsigned int i = 0; i < r.nfields(); i++) {
        text = "";
        QDomElement elem = DOCUMENT.createElement(FIELD);
        switch(r.type(i)) {
        case TpBool:   type = "bool"; text = r.asBool(i) ? "1" : "0"; break;
        case TpUChar:  type = "uchar"; text = r.asuChar(i); break;
        case TpShort:  type = "short"; text = QString::number(r.asShort(i));
                       break;
        case TpInt:    type = "int"; text = QString::number(r.asInt(i)); break;
        case TpUInt:   type = "uint"; text=QString::number(r.asuInt(i)); break;
        case TpFloat:  type = "float"; text = s.arg(r.asFloat(i)); break;
        case TpDouble: type = "double"; text = s.arg(r.asDouble(i)); break;
        case TpComplex:type = "complex"; text = s.arg(r.asComplex(i).real()) +
                       " " + s.arg(r.asComplex(i).imag()); break;
        case TpDComplex:type = "dcomplex"; text = s.arg(r.asDComplex(i).real())
                        + " " + s.arg(r.asDComplex(i).imag()); break;
        case TpString: type = "string"; text = r.asString(i).c_str(); break;
        case TpArrayBool: { type = "boolArray";
            QDomElement array = DOCUMENT.createElement(ARRAY);
            array.setAttribute(SHAPE, shapeToString(r.shape(i)));
            elem.appendChild(array);
            for(Array<Bool>::ConstIteratorSTL it = r.asArrayBool(i).begin();
                it != r.asArrayBool(i).end(); it++) {
                QDomElement val = DOCUMENT.createElement(VAL);
                val.appendChild(DOCUMENT.createTextNode(*it ? "1": "0"));
                array.appendChild(val);
            } break; }
        case TpArrayUChar: { type = "ucharArray";
            QDomElement array = DOCUMENT.createElement(ARRAY);
            array.setAttribute(SHAPE, shapeToString(r.shape(i)));
            elem.appendChild(array);
            for(Array<uChar>::ConstIteratorSTL it = r.asArrayuChar(i).begin();
                it != r.asArrayuChar(i).end(); it++) {
                QDomElement val = DOCUMENT.createElement(VAL);
                val.appendChild(DOCUMENT.createTextNode(QString(*it)));
                array.appendChild(val);
            } break; }
        case TpArrayShort: { type = "shortArray";
            QDomElement array = DOCUMENT.createElement(ARRAY);
            array.setAttribute(SHAPE, shapeToString(r.shape(i)));
            elem.appendChild(array);
            for(Array<Short>::ConstIteratorSTL it = r.asArrayShort(i).begin();
                it != r.asArrayShort(i).end(); it++) {
                QDomElement val = DOCUMENT.createElement(VAL);
                val.appendChild(DOCUMENT.createTextNode(QString::number(*it)));
                array.appendChild(val);
            } break; }
        case TpArrayInt: { type = "intArray";
            QDomElement array = DOCUMENT.createElement(ARRAY);
            array.setAttribute(SHAPE, shapeToString(r.shape(i)));
            elem.appendChild(array);
            for(Array<Int>::ConstIteratorSTL it = r.asArrayInt(i).begin();
                it != r.asArrayInt(i).end(); it++) {
                QDomElement val = DOCUMENT.createElement(VAL);
                val.appendChild(DOCUMENT.createTextNode(QString::number(*it)));
                array.appendChild(val);
            } break; }
        case TpArrayUInt: { type = "uintArray";
            QDomElement array = DOCUMENT.createElement(ARRAY);
            array.setAttribute(SHAPE, shapeToString(r.shape(i)));
            elem.appendChild(array);
            for(Array<uInt>::ConstIteratorSTL it = r.asArrayuInt(i).begin();
                it != r.asArrayuInt(i).end(); it++) {
                QDomElement val = DOCUMENT.createElement(VAL);
                val.appendChild(DOCUMENT.createTextNode(QString::number(*it)));
                array.appendChild(val);
            } break; }
        case TpArrayFloat: { type = "floatArray";
            QDomElement array = DOCUMENT.createElement(ARRAY);
            array.setAttribute(SHAPE, shapeToString(r.shape(i)));
            elem.appendChild(array);
            for(Array<Float>::ConstIteratorSTL it = r.asArrayFloat(i).begin();
                it != r.asArrayFloat(i).end(); it++) {
                QDomElement val = DOCUMENT.createElement(VAL);
                val.appendChild(DOCUMENT.createTextNode(s.arg(*it)));
                array.appendChild(val);
            } break; }
        case TpArrayDouble: { type = "doubleArray";
            QDomElement array = DOCUMENT.createElement(ARRAY);
            array.setAttribute(SHAPE, shapeToString(r.shape(i)));
            elem.appendChild(array);
            for(Array<Double>::ConstIteratorSTL it= r.asArrayDouble(i).begin();
                it != r.asArrayDouble(i).end(); it++) {
                QDomElement val = DOCUMENT.createElement(VAL);
                val.appendChild(DOCUMENT.createTextNode(s.arg(*it)));
                array.appendChild(val);
            } break; }
        case TpArrayString: { type = "stringArray";
            QDomElement array = DOCUMENT.createElement(ARRAY);
            array.setAttribute(SHAPE, shapeToString(r.shape(i)));
            elem.appendChild(array);
            for(Array<String>::ConstIteratorSTL it= r.asArrayString(i).begin();
                it != r.asArrayString(i).end(); it++) {
                QDomElement val = DOCUMENT.createElement(VAL);
                val.appendChild(DOCUMENT.createTextNode((*it).c_str()));
                array.appendChild(val);
            } break; }
        case TpRecord: elem = recordToXML(r.asRecord(i), FIELD.toStdString());
                       type = "record"; break;
        
        default: continue;
        }
        
        elem.setAttribute(NAME, r.name(i).c_str());
        elem.setAttribute(TYPE, type);
        if(!text.isEmpty()) elem.appendChild(DOCUMENT.createTextNode(text));
        topElem.appendChild(elem);
    }
    
    return topElem;
}

Record RSXML::XMLToRecord(const QDomElement& element) {
    Record r;
    
    QDomNode node = element.firstChild();
    while(!node.isNull()) {
        QDomElement f = node.toElement();
        if(f.isNull() || !f.hasAttribute(NAME) || !f.hasAttribute(TYPE)) {
            node = node.nextSibling(); continue; }
        String n = f.attribute(NAME).toStdString();
        QString type = f.attribute(TYPE).toLower();
        QString t = f.text();
        bool ok;
        QDomElement array = f.firstChildElement(ARRAY), val;
        if(type == "bool") {
            t = t.toLower();
            r.define(n, t != "0" && t != "false" && t != "f");
        } else if(type == "uchar") {
            if(t.size() > 0) r.define(n, t[0].cell());
        } else if(type == "short") {
            short s = t.toShort(&ok);
            if(ok) r.define(n, s);
        } else if(type == "int") {
            int i = t.toInt(&ok);
            if(ok) r.define(n, i);
        } else if(type == "uint") {
            unsigned int i = t.toUInt(&ok);
            if(ok) r.define(n, i);
        } else if(type == "float") {
            float f = t.toFloat(&ok);
            if(ok) r.define(n, f);
        } else if(type == "double") {
            double d = t.toDouble(&ok);
            if(ok) r.define(n, d);
        } else if(type == "complex") {
            QStringList l = t.split(QRegExp("\\s+"));
            if(l.size() >= 2) {
                bool ok2;
                float re = l[0].toFloat(&ok), im = l[1].toFloat(&ok2);
                if(ok && ok2) r.define(n, Complex(re, im));
            }
        } else if(type == "dcomplex") {
            QStringList l = t.split(QRegExp("\\s+"));
            if(l.size() >= 2) {
                bool ok2;
                double re = l[0].toDouble(&ok), im = l[1].toDouble(&ok2);
                if(ok && ok2) r.define(n, DComplex(re, im));
            }
        } else if(type == "string") r.define(n, t.toStdString());
        else if(type == "boolarray") {
            if(!array.isNull() && array.hasAttribute(SHAPE)) {
                Array<bool> a(stringToShape(array.attribute(SHAPE)));
                Array<bool>::IteratorSTL it = a.begin();
                QDomNodeList vals = array.childNodes();
                for(int i = 0; it != a.end() && i < vals.size(); i++) {
                    val = vals.at(i).toElement();
                    if(!val.isNull()) {
                        t = val.text().toLower();
                        if(t.size() > 0) {
                            *it = t != "0" && t != "false" && t != "f";
                            it++;
                        }
                    }
                }
                r.define(n, a);
            }
        } else if(type == "uchararray") {
            if(!array.isNull() && array.hasAttribute(SHAPE)) {
                Array<uChar> a(stringToShape(array.attribute(SHAPE)));
                Array<uChar>::IteratorSTL it = a.begin();
                QDomNodeList vals = array.childNodes();
                for(int i = 0; it != a.end() && i < vals.size(); i++) {
                    val = vals.at(i).toElement();
                    if(!val.isNull()) {
                        t = val.text();
                        if(t.size() > 0) {
                            *it = t[0].cell();
                            it++;
                        }
                    }
                }
                r.define(n, a);
            }
        } else if(type == "shortarray") {
            if(!array.isNull() && array.hasAttribute(SHAPE)) {
                Array<short> a(stringToShape(array.attribute(SHAPE)));
                Array<short>::IteratorSTL it = a.begin();
                QDomNodeList vals = array.childNodes();
                short s;
                for(int i = 0; it != a.end() && i < vals.size(); i++) {
                    val = vals.at(i).toElement();
                    if(!val.isNull()) {
                        s = val.text().toShort(&ok);
                        if(ok) {
                            *it = s;
                            it++;
                        }
                    }
                }
                r.define(n, a);
            }
        } else if(type == "intarray") {
            if(!array.isNull() && array.hasAttribute(SHAPE)) {
                Array<int> a(stringToShape(array.attribute(SHAPE)));
                Array<int>::IteratorSTL it = a.begin();
                QDomNodeList vals = array.childNodes();
                int v;
                for(int i = 0; it != a.end() && i < vals.size(); i++) {
                    val = vals.at(i).toElement();
                    if(!val.isNull()) {
                        v = val.text().toInt(&ok);
                        if(ok) {
                            *it = v;
                            it++;
                        }
                    }
                }
                r.define(n, a);
            }
        } else if(type == "uintarray") {
            if(!array.isNull() && array.hasAttribute(SHAPE)) {
                Array<uInt> a(stringToShape(array.attribute(SHAPE)));
                Array<uInt>::IteratorSTL it = a.begin();
                QDomNodeList vals = array.childNodes();
                unsigned int v;
                for(int i = 0; it != a.end() && i < vals.size(); i++) {
                    val = vals.at(i).toElement();
                    if(!val.isNull()) {
                        v = val.text().toUInt(&ok);
                        if(ok) {
                            *it = v;
                            it++;
                        }
                    }
                }
                r.define(n, a);
            }
        } else if(type == "floatarray") {
            if(!array.isNull() && array.hasAttribute(SHAPE)) {
                Array<float> a(stringToShape(array.attribute(SHAPE)));
                Array<float>::IteratorSTL it = a.begin();
                QDomNodeList vals = array.childNodes();
                float v;
                for(int i = 0; it != a.end() && i < vals.size(); i++) {
                    val = vals.at(i).toElement();
                    if(!val.isNull()) {
                        v = val.text().toFloat(&ok);
                        if(ok) {
                            *it = v;
                            it++;
                        }
                    }
                }
                r.define(n, a);
            }
        } else if(type == "doublearray") {
            if(!array.isNull() && array.hasAttribute(SHAPE)) {
                Array<double> a(stringToShape(array.attribute(SHAPE)));
                Array<double>::IteratorSTL it = a.begin();
                QDomNodeList vals = array.childNodes();
                double v;
                for(int i = 0; it != a.end() && i < vals.size(); i++) {
                    val = vals.at(i).toElement();
                    if(!val.isNull()) {
                        v = val.text().toDouble(&ok);
                        if(ok) {
                            *it = v;
                            it++;
                        }
                    }
                }
                r.define(n, a);
            }
        } else if(type == "stringarray") {
            if(!array.isNull() && array.hasAttribute(SHAPE)) {
                Array<String> a(stringToShape(array.attribute(SHAPE)));
                Array<String>::IteratorSTL it = a.begin();
                QDomNodeList vals = array.childNodes();
                for(int i = 0; it != a.end() && i < vals.size(); i++) {
                    val = vals.at(i).toElement();
                    if(!val.isNull()) {
                        *it = val.text().toStdString();
                        it++;
                    }
                }
                r.define(n, a);
            }
        } else if(type == "record") r.defineRecord(n, XMLToRecord(f));
        
        node = node.nextSibling();
    }
    
    return r;
}

QDomDocument RSXML::DOCUMENT = QDomDocument();

const QString RSXML::ARRAY   = "array";
const QString RSXML::FIELD   = "field";
const QString RSXML::NAME    = "name";
const QString RSXML::SHAPE   = "shape";
const QString RSXML::TYPE    = "type";
const QString RSXML::VAL     = "val";

QString RSXML::shapeToString(const IPosition& shape) {
    stringstream ss;
    for(unsigned int i = 0; i < shape.size(); i++) {
        ss << shape[i];
        if(i < shape.size() - 1) ss << ' ';
    }
    return ss.str().c_str();
}

IPosition RSXML::stringToShape(const QString& shape) {
    QStringList list = shape.split(QRegExp("\\s+"));
    vector<int> v;
    bool ok;
    int val;
    for(int i = 0; i < list.size(); i++) {
        val = list[i].toInt(&ok);
        if(ok) v.push_back(val);
    }
    IPosition ipos(v.size());
    for(unsigned int i = 0; i < v.size(); i++) ipos[i] = v[i];
    return ipos;
}


// XMLFILEREADERWRITER DEFINITIONS //

// Static Definitions //

vector<RegionShape*> XMLFileReaderWriter::DOMToShapes(QDomElement& el) {
    vector<RegionShape*> v;
    if(el.isNull() || el.tagName() != SHAPE_OPTIONS) return v;
    
    QDomNodeList l = el.elementsByTagName(SHAPE);
    QDomElement e;
    String str;
    RegionShape* shape;
    for(int i = 0; i < l.size(); i++) {
        e = l.at(i).toElement();
        if(e.isNull()) continue;
        shape = RegionShape::shapeFromRecord(RSXML::XMLToRecord(e), str);
        if(shape != NULL) v.push_back(shape);
    }
    
    return v;
}

QDomElement XMLFileReaderWriter::shapesToDOM(QDomDocument& document,
        const vector<RegionShape*>& shapes) {
    QDomElement shapeOpts = document.createElement(SHAPE_OPTIONS);    
    for(unsigned int i = 0; i < shapes.size(); i++) {
        shapeOpts.appendChild(RSXML::recordToXML(shapes[i]->toRecord(),
                              SHAPE.toStdString()));
    }
    return shapeOpts;
}

const QString XMLFileReaderWriter::DOCROOT_NAME  = "casaviewer-shapes";
const pair<QString, QString> XMLFileReaderWriter::VERSION("version", "1.0");
const QString XMLFileReaderWriter::SHAPE_OPTIONS = "shape-options";
const QString XMLFileReaderWriter::SHAPE         = "shape";


// Non-Static Definitions //

XMLFileReaderWriter::XMLFileReaderWriter() { }

XMLFileReaderWriter::~XMLFileReaderWriter() { }

bool XMLFileReaderWriter::read(vector<RegionShape*>& shapes) {
    QFile file(m_filename.c_str());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setError("Could not open file " + m_filename, true);
        return false;
    }
    QDomDocument doc;
    if(!doc.setContent(&file)) {
        setError("Could not read XML file " + m_filename, true);
        file.close();
        return false;
    }
    
    QDomElement docElem = doc.documentElement();
    if(docElem.isNull()) {
        setError("XML file " + m_filename + " is empty.", true);
        file.close();
        return false;
    }
    QDomElement shapesElem = docElem;
    if(shapesElem.tagName() != SHAPE_OPTIONS) {
        QDomNodeList elems = docElem.elementsByTagName(SHAPE_OPTIONS);
        if(elems.size() == 0) {
            setError("No " + SHAPE_OPTIONS.toStdString() + " element found.",
                     true);
            file.close();
            return false;
        }
        int i = 0;
        for(; i < elems.size(); i++) {
            shapesElem = elems.at(i).toElement();
            if(!shapesElem.isNull()) break;
        }
        if(i == elems.size()) {
            setError("No " + SHAPE_OPTIONS.toStdString() + " element found.",
                     true);
            file.close();
            return false;
        }
    }
    
    shapes.clear();
    QDomNodeList list = shapesElem.elementsByTagName(SHAPE);
    QDomElement shapeElem;
    Record shapeRec;
    RegionShape* shape;
    bool success = true;
    String error;
    stringstream ss;
    for(int i = 0; i < list.size(); i++) {
        shapeElem = list.at(i).toElement();
        if(!shapeElem.isNull()) {
            shapeRec = RSXML::XMLToRecord(shapeElem);
            error = "";
            shape = RegionShape::shapeFromRecord(shapeRec, error);
            if(shape == NULL) {
                success = false;
                RSUtils::appendUniqueMessage(ss, error);
            } else shapes.push_back(shape);
        }
    }
    
    file.close();
    setError(ss.str());
    return success;
}

QWidget* XMLFileReaderWriter::optionsWidget() const {
    return new QLabel("[no additional options]");
}

void XMLFileReaderWriter::setOptions(const QWidget* widget) { }

bool XMLFileReaderWriter::write(const vector<RegionShape*>& s) const {
    QFile file(m_filename.c_str());
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setError("Could not open file " + m_filename, true);
        return false;
    }
    QTextStream out(&file);
    
    QDomDocument writeDoc;
    QDomElement writeElem = writeDoc.createElement(DOCROOT_NAME);
    writeDoc.appendChild(writeElem);
    writeElem.setAttribute(VERSION.first, VERSION.second);
    
    QDomElement shapeOpts = shapesToDOM(writeDoc, s);
    writeElem.appendChild(shapeOpts);
    
    out << writeDoc.toString();
    out.flush();
    file.close();    

    return true;
}

}
