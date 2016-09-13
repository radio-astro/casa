//# QtXmlRecord.cc:  translations between aips++ options Records and xml.
//# Copyright (C) 2005
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

#include <graphics/X11/X_enter.h>
#  include <QtXml>
#  include <QMessageBox>
#include <graphics/X11/X_exit.h>

#include "QtXmlRecord.h"
#include <casa/IO/AipsIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	QtXmlRecord::QtXmlRecord() : rootName("casa-Record")
	{}
	QtXmlRecord::~QtXmlRecord()
	{}

	bool QtXmlRecord::recordToDom(Record *rec, QDomDocument &doc) {
		QDomElement root = doc.createElement(rootName);
		recordToElement(rec, &root, doc);
		doc.appendChild(root);
		formatXml(doc);
		return true;
	}


	void QtXmlRecord::formatXml(QDomDocument &domDocument) {
		QDomElement root = domDocument.documentElement();
		QDomNode rootb = domDocument.removeChild(root);
		root = domDocument.createElement(rootName);
		domDocument.appendChild(root);
		QDomNode n = rootb.firstChild();
		while(!n.isNull()) {
			QDomElement e = n.toElement();
			if(!e.isNull()) {
				QString attr = e.attribute("context", "basic_settings");
				if (root.elementsByTagName(attr).count() == 0) {
					root.appendChild(domDocument.createElement(attr));
				}

				QString eName = e.tagName();  //#dk
				//#dk  QString eName = e.attribute("dlformat", "no-name");

				QDomElement elem = (QDomElement)e.cloneNode().toElement();
				elem.removeAttribute("context");
				elem.removeAttribute("dlformat");
				elem.setTagName(eName);
				/*
				QString help = e.attribute("help");
				QDomElement helpElement = domDocument.createElement("help");
				QDomText helpElementText =domDocument.createTextNode(help);
				helpElement.appendChild(helpElementText);
				elem.appendChild(helpElement);
				elem.removeAttribute("help");

				QString value = e.attribute("value");
				QDomElement valueElement = domDocument.createElement("value");
				   QDomText valueElementText =domDocument.createTextNode(value);
				valueElement.appendChild(valueElementText);
				elem.appendChild(valueElement);
				elem.removeAttribute("value");
				 */

				elem.setAttribute("saved", e.attribute("value"));
				root.elementsByTagName(attr).item(0).appendChild(elem);
			}
			n = n.nextSibling();
		}

	}


	bool QtXmlRecord::recordToElement(const Record *rec, QDomElement *parent,
	                                  QDomDocument &doc) {

		Int nFields = rec->nfields();

		for (Int i = 0; i < nFields; i++) {
			if (rec->type(i) == TpRecord &&
			        rec->name(i)!="value" && rec->name(i)!="default") {
				//Axis label rest value has been moved to the image manager.
				if ( rec->name(i) == "axislabelrestvalue"){
					continue;
				}
				QDomElement subRec = doc.createElement(rec->name(i).chars());
				//#dk QDomElement subRec = doc.createElement("record");
				parent->appendChild(subRec);
				recordToElement( &(rec->subRecord(i)), &subRec, doc);
			} else {
				ostringstream oss;
				QString datatype;

				switch (rec->type(i)) {

					{
					case TpRecord:
						// (handle 'unset' value or default as null String.
						// Record values/defaults not supported at present).
						oss << "";
						datatype = "String";
						break;
					}

					{
					case TpBool:
						Bool value = rec->asBool(i);
						oss << value;
						datatype = "Bool";
						break;
					}
					{
					case TpUChar:
						uChar value = rec->asuChar(i);
						oss << value;
						datatype = "uChar";
						break;
					}
					{
					case TpShort:
						Short value = rec->asShort(i);
						oss << value;
						datatype = "Short";
						break;
					}
					{
					case TpInt:
						Int value = rec->asInt(i);
						oss << value;
						datatype = "Int";
						break;
					}
					{
					case TpUInt:
						uInt value = rec->asuInt(i);
						oss << value;
						datatype = "uInt";
						break;
					}
					{
					case TpFloat:
						Float value = rec->asFloat(i);
						oss << value;
						datatype = "Float";
						break;
					}
					{
					case TpDouble:
						Double value = rec->asDouble(i);
						oss << value;
						datatype = "Double";
						break;
					}
					{
					case TpComplex:
						Complex value = rec->asComplex(i);
						oss << value;
						datatype = "Complex";
						break;
					}
					{
					case TpDComplex:
						DComplex value = rec->asDComplex(i);
						oss << value;
						datatype = "DComplex";
						break;
					}
					{
					case TpString:
						String value = rec->asString(i);
						oss << value;
						datatype = "String";
						break;
					}
					{
					case TpArrayBool:
						Array<Bool> value = rec->asArrayBool(i);
						oss << value;
						datatype = "ArrayBool";
						break;
					}
					{
					case TpArrayUChar:
						Array<uChar> value = rec->asArrayuChar(i);
						oss << value;
						datatype = "ArrayuChar";
						break;
					}
					{
					case TpArrayShort:
						Array<Short> value = rec->asArrayShort(i);
						oss << value;
						datatype = "ArrayShort";
						break;
					}
					{
					case TpArrayInt:
						Array<Int> value = rec->asArrayInt(i);
						oss << value;
						datatype = "ArrayInt";
						break;
					}
					{
					case TpArrayUInt:
						Array<uInt> value = rec->asArrayuInt(i);
						oss << value;
						datatype = "ArrayuInt";
						break;
					}
					{
					case TpArrayFloat:
						Array<Float> value = rec->asArrayFloat(i);
						oss << value;
						datatype = "ArrayFloat";
						break;
					}
					{
					case TpArrayDouble:
						Array<Double> value = rec->asArrayDouble(i);
						oss << value;
						datatype = "ArrayDouble";
						break;
					}
					{
					case TpArrayComplex:
						Array<Complex> value = rec->asArrayComplex(i);
						oss << value;
						datatype = "ArrayComplex";
						break;
					}
					{
					case TpArrayDComplex:
						Array<DComplex> value = rec->asArrayDComplex(i);
						oss << value;
						datatype = "ArrayDComplex";
						break;
					}
					{
					case TpArrayString:
						Array<String> value = rec->asArrayString(i);
						oss << value;
						datatype = "ArrayString";
						break;
					}
					{
					case TpChar:
						;
					}
					{
					case TpUShort:
						;
					}
					{
					case TpTable:
						;
					}
					{
					case TpArrayChar:
						;
					}
					{
					case TpArrayUShort:
						;
					}
					{
					case TpOther:
						;
					}
					{
					case TpQuantity:
						;
					}
					{
					case TpArrayQuantity:
						;
					}
					{
					case TpNumberOfTypes:
						;
					}
					{
					default:
						Array<String>value = rec->asArrayString(i);
						oss << value;
						datatype = "Other";
					}
				}
				//the datatype may have a use for some complex items.
				parent->setAttribute("datatype", datatype);
				String value(oss);
				String name = rec->name(i);
				parent->setAttribute(name.chars(), value.chars());
			}
		}

		return true;
	}


	bool QtXmlRecord::elementToRecord(QDomElement *ele, Record &rec,
	                                  Bool useOptName) {
		// Translates the value attribute of ele to a field in rec of
		// form  nm=value.  The type of value is determined by the element's
		// ptype attribute.  (ele is intended to represent a single AutoGui
		// option, such as yaxistext).  If useOptName is True (the default),
		// nm will be the element tagName (option name), suitable for sending
		// only an option's value (e.g. [xaxistext="mytext"]) to setOptions.
		// If false, nm will be "value" so that rec can be used (possibly with
		// additional meta-information) as a subrecord to the option name, as in
		// [xaxistext=[value="mytext", label="mylabel", ...]]; this is the other
		// form acceptable to setOptions.
		if (!ele->isNull()) {
			String name = ele->tagName().toStdString();
			String fldnm = (useOptName? name : "value");
			String ptype = ele->attribute("ptype").toStdString();
			String datatype = ele->attribute("datatype").toStdString();
			QString value = ele->attribute("value");


			if (ptype == "choice" || ptype == "userchoice") {
				rec.define(fldnm, value.toStdString());
			} else if(ptype == "floatrange") {
				rec.define(fldnm, value.toFloat());
			} else if(ptype == "intrange") {
				rec.define(fldnm, value.toInt());
			} else if(ptype == "intpair") {
				vector<int> v(2);
				if(value.contains(' ')) {
					QTextStream ss(&value);
					ss >> v[0];
					ss >> v[1];

				} else {
					v[0] = value.toInt();
					v[1] = v[0];
				}
				rec.define(fldnm, Vector<int>(v));
			} else if(ptype == "doublepair") {
				vector<double> v(2);
				if(value.contains(' ')) {
					QTextStream ss(&value);
					ss >> v[0];
					ss >> v[1];

				} else {
					v[0] = value.toDouble();
					v[1] = v[0];
				}
				rec.define(fldnm, Vector<double>(v));
			} else if(ptype == "int") {
				rec.define(fldnm, value.toInt());
			} else if(ptype == "double") {
				rec.define(fldnm, value.toDouble());
			} else if(ptype == "string") {
				if (name == "mask") {
					//#dk  Don't process this type of entry for now --
					//#dk  this is a more complex data type, which needs
					//#dk  further support; see LPADD.cc "mask" & vdd.g 'mask'.
				} else
					rec.define(fldnm, value.toStdString());
			} else if ( ptype == "boolean" || ptype =="boolCheck" ) {
				rec.define(fldnm, (value == "1"));
			} else if (ptype == "minmaxhist" ) {
				value.remove("[").remove("]");
				QStringList list = value.split(",");
				if (list.size() == 2) {
					bool ok1, ok2;
					double d1 = list[0].toDouble(&ok1) ;
					double d2 = list[1].toDouble(&ok2);
					if (ok1 == true && ok2 == true && d1 < d2) {
						Vector<Float> tempinsert(2);
						tempinsert(0) = d1;
						tempinsert(1) = d2;
						rec.define(fldnm, tempinsert);
					}
				} else if ( list.size() == 1 ) {
					rec.define(list[0].toStdString(), true);
				}
			} else if (ptype == "button") {
				//#dk cout << " set " << name << endl;

				rec.define(fldnm, True);
				//#dk rec.define(fldnm, "T");
				//#dk AipsIO ios("temp", ByteIO::New);
				//#dk ios << rec;
			}


			else if (ptype == "array") {

				QRegExp rx("[,\\s\\[\\]]");
				// separate on commas, whitespace or brackets.

				QStringList vals = value.split(rx, QString::SkipEmptyParts);

				Vector<Double> dVals(vals.size());
				for(Int i=0; i<vals.size(); i++) dVals[i] = vals.at(i).toDouble();


				// Consistent with older glish interface, 'array's may return
				// Vectors of Ints _or_ Doubles.  (This should be changed, since
				// it's a pain to figure out what you have on the receiving end;
				// also, you usually want to restrict to one or the other...).

				Bool allInt=True;
				for(Int i=0; i<vals.size(); i++) if(dVals[i]!=Int(dVals[i])) {
						allInt=False;
						break;
					}

				if(allInt) {
					Vector<Int> iVals(vals.size());
					for(Int i=0; i<vals.size(); i++) iVals[i] = Int(dVals[i]);

					rec.define(fldnm, iVals);
				}		// (Ints will do).

				else {
					rec.define(fldnm, dVals);
				}		// (must use Doubles).

			}


			else if (ptype == "scalar") {
				// Single Float value.
				rec.define(fldnm, value.trimmed().toFloat());
			}


			else if (ptype == "check") {
				//#dk cout << " set " << name << endl;
				value.remove("[").remove("]");
				QStringList list = value.split(", ");
				Vector<String> axisNames(list.size());
				for(int i = 0; i < list.size(); i++)
					axisNames(i) = list.at(i).toStdString();
				rec.define(fldnm, axisNames);
			} else if (ptype == "region") {
				Record unset;
				unset.define("i_am_unset", "i_am_unset");
				rec.defineRecord("value", unset);
				//#dk region gui elements not supported yet
				// -- for now, send 'unset' value in all cases.
			}
		}
		return true;
	}



	Bool QtXmlRecord::domToRecord(QDomDocument *doc, Record &rec)  {
		return optsElemToRecord(doc->firstChildElement(), rec);
	}


	Bool QtXmlRecord::optsElemToRecord(QDomElement optsElem, Record &rec) {
		// (optsElem's children should be the option groups).

		QDomElement grp = optsElem.firstChildElement();
		for (; !grp.isNull(); grp = grp.nextSiblingElement()) {
			QString grpName = grp.tagName();
			QDomElement ele = grp.firstChildElement();
			for (; !ele.isNull(); ele = ele.nextSiblingElement()) {

				Record rcd;
				QDomNamedNodeMap map = ele.attributes();
				for (int i = 0; i < (int)(map.count()); i++) {
					QDomNode attr = map.item (i);
					if (attr.nodeName() == "value")
						elementToRecord(&ele, rcd, False);
					// ("False" because rcd will be a subrecord
					// to the option field name in rec, below).
					else
						rcd.define(String(attr.nodeName().toStdString()),
						           String(attr.nodeValue().toStdString()));
				}

				if( (rcd.isDefined("ptype") && rcd.dataType("ptype")==TpString &&
				        // (the above repetitive, mandatory ass-kissing is due to poor
				        // design in Records, imo.... "Not found" or "not this type"
				        // is _not_ exceptional, and should be handled by appropriate
				        // feedback (a default, redefinable null return value, in
				        // this case) -- _not_ by an exception!   grr...).
				        rcd.asString("ptype")=="region")  ||
				        ele.tagName()=="mask" )  continue;
				// (region and mask interface elements not supported
				// at present -- don't pass them on...)

				rcd.define("context", String(grpName.toStdString()));
				QString name = ele.tagName();
				rec.defineRecord(String(name.toStdString()), rcd);
			}
		}
		return true;
	}


	bool QtXmlRecord::readDomFrom(QDomDocument &doc, QIODevice *device) {
		QString errorStr;
		int errorLine;
		int errorColumn;

		if (!doc.setContent(device, true, &errorStr, &errorLine,
		                    &errorColumn)) {
			QMessageBox::information(0, tr("Qt Options"),
			                         tr("Parse error at line %1, column%2:\n%3")
			                         .arg(errorLine)
			                         .arg(errorColumn)
			                         .arg(errorStr));
			return false;
		}

		QDomElement root = doc.documentElement();
		if (root.tagName() != rootName) {
			QMessageBox::information(0, tr("Qt Options"),
			                         tr("The file is not an Options file."));
			return false;
		} else if (root.hasAttribute("version")
		           && root.attribute("version") != "1.0") {
			QMessageBox::information(0, tr("Qt Options"),
			                         tr("The file is not an Options "
			                            "version 1.0 file."));
			return false;
		}

		return true;

	}
	bool QtXmlRecord::writeDomTo(QDomDocument *doc, QIODevice *device) {
		const int IndentSize = 4;
		QTextStream out(device);
		doc->save(out, IndentSize);
		return true;
	}

	QString QtXmlRecord::domToString(const QDomElement &elt) {
		QString result;
		QTextStream stream(&result, QIODevice::WriteOnly);
		elt.save(stream, 2);
		stream.flush();
		return result;
	}

	QDomDocument QtXmlRecord::stringToDom(const QString &xml) {
		QDomDocument result;
		result.setContent(xml);
		return result;
	}

} //# NAMESPACE CASA - END
