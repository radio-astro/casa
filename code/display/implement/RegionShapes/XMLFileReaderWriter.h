//# XMLFileReaderWriter.h: Implementation for reading/writing XML files.
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
#ifndef XMLFILEREADERWRITIER_H_
#define XMLFILEREADERWRITIER_H_

#include <QtXml>

#include <display/RegionShapes/RSFileReaderWriter.h>

#include <casa/namespace.h>

namespace casa {

// Constants and methods for reading/writing RS XML files.
class RSXML {
public:
    // Converts a Record to/from a QDomElement.
    // <group>
    static QDomElement recordToXML(const RecordInterface& record,
                                   String elementName = "casa-record");    
    static Record XMLToRecord(const QDomElement& element);
    // </group>
    
private:
    // Document used for creating nodes.
    static QDomDocument DOCUMENT;
    
    // Constants.
    // <group>
    static const QString ARRAY;
    static const QString FIELD;
    static const QString NAME;
    static const QString SHAPE;
    static const QString TYPE;
    static const QString VAL;
    // </group>
    
    // Converts an IPosition to/from a QString.
    // <group>
    static QString shapeToString(const IPosition& shape);
    static IPosition stringToShape(const QString& shape);
    // </group>
};


// Implementation of RegionFileReader/RegionFileWriter for reading/writing
// CASA-XML files.
class XMLFileReaderWriter : public RSFileReader, public RSFileWriter {
public:
    // Static Members //
    
    // Returns the shapes represented by the given QDomElement.  The top-level
    // element MUST have tag name SHAPE_OPTIONS, and will have zero or more
    // children elements, each representing a RegionShape, with tag name
    // SHAPE.
    static vector<RegionShape*> DOMToShapes(QDomElement& element);
    
    // Returns a QDomElement that represents the given shapes.  The top-level
    // element will have tag name SHAPE_OPTIONS, and will have zero or more
    // children elements, each representing a RegionShape, with tag name SHAPE.
    // The given document is used to create the element, but the element is NOT
    // added to the document.
    static QDomElement shapesToDOM(QDomDocument& document,
                                   const vector<RegionShape*>& shapes);
    
    // XML file constants.
    // <group>
    static const QString DOCROOT_NAME;
    static const pair<QString, QString> VERSION;
    static const QString SHAPE_OPTIONS;
    static const QString SHAPE;
    // </group>
    
    
    // Non-Static Members //    
    
    // Constructor.
    XMLFileReaderWriter();
    
    // Destructor.
    ~XMLFileReaderWriter();
    
    // RSFileReader methods //

    // Implements RSFileReader::read.
    bool read(vector<RegionShape*>& shapes);
    
    // RSFileWriter methods //
    
    // Implements RSFileWriter::optionsWidget.
    QWidget* optionsWidget() const;
    
    // Implements RSFileWriter::setOptions.
    void setOptions(const QWidget* widget);

    // Implements RSFileWriter::write.
    bool write(const vector<RegionShape*>& shapes) const;
};

}

#endif /* XMLFILEREADERWRITIER_H_ */
