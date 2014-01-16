//# QtXmlRecord.h:  translations between aips++ options Records and xml.
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

#ifndef QTXMLRECORD_H
#define QTXMLRECORD_H

#include <casa/aips.h>

#include <graphics/X11/X_enter.h>
#  include <QtXml>
#  include <QDomDocument>
#include <graphics/X11/X_exit.h>


#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Utilities/DataType.h>
#include <casa/Containers/Record.h>
#include <display/Utilities/DisplayOptions.h>
#include <display/DisplayDatas/DisplayData.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class QtXmlRecord : public Record, public QObject {

	public:
		QtXmlRecord();
		~QtXmlRecord();
		bool recordToElement(const Record *rec, QDomElement *parent,
		                     QDomDocument &doc);
		Bool elementToRecord(QDomElement *ele, Record &rec, Bool useOptName=True);
		Bool domToRecord(QDomDocument *doc, Record &rec);
		Bool optsElemToRecord(QDomElement optsElem, Record &rec);
		bool readDomFrom(QDomDocument &doc, QIODevice *device);
		bool writeDomTo(QDomDocument *doc, QIODevice *device);
		bool recordToDom(Record *rec, QDomDocument &doc);
		const QString rootName;
		static QString domToString(const QDomElement &elt);
		static QDomDocument stringToDom(const QString &xml);

	private:
		void formatXml(QDomDocument &doc);
	};

} //# NAMESPACE CASA - END


#endif
