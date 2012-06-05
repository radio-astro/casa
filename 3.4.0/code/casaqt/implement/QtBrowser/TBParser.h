//# TBParser.h: Parses the XMLDriver-generated XML into data in a TBTable.
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
//# $Id: $
#ifndef TBPARSER_H_
#define TBPARSER_H_

#include <vector>
#include <sstream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <casaqt/QtBrowser/TBConstants.h>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace xercesc;
using namespace std;

namespace casa {

//# Forward Declarations
class TBField;
class TBKeyword;
class XMLtoken;
class TableParams;
class TBData;

// <summary>
// Parses the XMLDriver-generated XML into data in a TBTable.
// </summary>
//
// <synopsis>
// TBParser is an abstract superclass for any implementing subclass to parse
// a String containing XML.  A TBParser keeps a reference to the table
// parameters so that the table data can be directly imported.  Note: the
// TBParser is not used for the "Direct" table driver, which is the default.
// Currently the user is unable to selected an "XML" table driver, which means
// TBParsers are not used.
// </synopsis>

class TBParser {
public:
    // Constructor which takes a TableParams argument to store references to
    // the table parameters.
    TBParser(TableParams* tp);

    virtual ~TBParser();

    
    vector<vector<String>*>* getData() { return &data; }
    
    // Set whether the TBParser should print debug information or not.
    void setPrintDebug(bool pdb);
        
    
    // Any subclass must implement the parse() method.  Parses the given String
    // into the table parameters and returns a Result indicating whether the
    // parsing succeeded or not.  If parsedata is true, the table data is
    // parsed, otherwise just table meta-data like keywords is parsed.
    virtual Result parse(String* xml, bool parsedata = true) = 0;

protected:
    // Is true if this table allows for the insertion of rows, false otherwise.
    bool& insertRow;

    // Is true if this table allows for the deletion of rows, false otherwise.
    bool& removeRow;

    // Holds the table data.
    vector<vector<String>*> data;

    // Holds the "real" table data.
    vector<vector<TBData*>*>& data2;
    
    // Holds the table fields.
    vector<TBField*>& fields;

    // Holds the table keywords.
    vector<TBKeyword*>& keywords;

    // Holds the list of the number of rows for each subtable.
    vector<int>& subtableRows;

    // Holds the total number of rows in the table.
    int& totalRows;

    // Holds the number of rows currently loaded in the table.
    int& loadedRows;

    // Is true if debug information should be printed, false otherwise.
    bool printdebug;
};

// <summary>
// TBParser subclass that uses a "home" parsing method.
// </summary>
//
// <synopsis>
// TBHomeParser is a subclass of TBParser that implements all the parsing
// methods itself using String methods.  It is somewhat slow and its use is
// not recommended.
// </synopsis>

class TBHomeParser : public TBParser {
public:
    // Constructor that take the table parameters.
    TBHomeParser(TableParams* tp);

    virtual ~TBHomeParser();

    
    // Implements TBParser::parse().  Parses the String into XMLtokens and then
    // parses the table information from the XMLtokens.
    Result parse(String* xml, bool parsedata = true);

private:
    // All parsed XMLtokens that had a tag name of TBConstants::XML_FIELD.
    vector<XMLtoken*> xfields;

    // All parsed XMLtokens that had a tag name of TBConstants::XML_KEYWORD.
    vector<XMLtoken*> xkeywords;

    // All parsed XMLtokens that had a tag name of TBConstants::XML_COLUMNKW.
    map<String, vector<XMLtoken*>*> xcolkeywords;

    
    // Recursively parses a XMLtoken from the given String.  The level
    // parameter is used to properly add tabs to the debug information.
    XMLtoken* parseToken(String* xml, int level);

    // Parses XML attributes from the given String into the given token.  The
    // level parameter is used to properly add tabs to the debug information.
    void parseAttributes(XMLtoken* token, String* attrPtr, int level);

    // Parses XML content (<tag>content</tag>) from the given String into the
    // given token.  The level parameter is used to properly add tabs to the
    // debug information.
    void parseContent(XMLtoken* token, String* contentPtr, int level);

    // Given an XMLtoken tree, parse the table information from it.  If
    // parsedata is true the table data is parsed, otherwise just the
    // meta-information like keywords is parsed.
    bool parseXMLtable(XMLtoken* t, bool parsedata);
};

// <summary>
// TBParser subclass that uses a DOM parser in the XERCES library.
// </summary>
//
// <synopsis>
// TBXercesDOMParser is a subclass of TBParser that implements all the parsing
// methods using a XERCES DOM parser.  Although the actual parsing happens
// quickly, deciphering table data from the parsed XML is somewhat slow and
// thus the use of TBXercesDOMParser is not recommended.
// </synopsis>

class TBXercesDOMParser : public TBParser {
public:
    // Constructor that takes the table parameters.
    TBXercesDOMParser(TableParams* tp);

    virtual ~TBXercesDOMParser();

    
    // Implements TBParser::parse().  The String is parsed into DOMElements and
    // then the table information is parsed from the DOMElements.
    Result parse(String* xml, bool parsedata = true);

private:
    // First level parsing method that takes the top-level element and
    // parses it.
    Result parseXML(const DOMElement* element, bool parsedata);

    // Second level parsing method that takes the TABLE element and parses
    // the table out of it.
    Result parseTable(const DOMElement* element, bool parsedata);

    // Third level parsing method that takes the TABLEDATA element and
    // parses the table data out of it.
    Result parseTableData(const DOMElement* element);
};

// <summary>
// TBParser subclass that uses a SAX parser in the XERCES library.
// </summary>
//
// <synopsis>
// TBXercesSAXParser is a subclass of TBParser that implements all the parsing
// methods using a XERCES SAX parser.  If XML parsing is required, the
// TBXercesSAXParser is recommended for its (relative) speed.
// TBXercesSAXParser also implements xerces::DefaultHandler since SAX uses
// event-driven parsing.
// </synopsis>

class TBXercesSAXParser : public TBParser, public DefaultHandler {
public:
    // Constructor that takes the table parameters.
    TBXercesSAXParser(TableParams* tp);

    virtual ~TBXercesSAXParser();

    
    // Implements TBParser::parse().  Parses the String into the table data
    // serially using event-driven SAX parsing.
    Result parse(String* xml, bool parsedata = true);

    // Implements DefaultHandler::startDocument().
    void startDocument();

    // Implements DefaultHandler::endDocument().
    void endDocument();

    // Implements DefaultHandler::startElement().
    void startElement(const XMLCh* const uri, const XMLCh* const localname,
                      const XMLCh* const qname, const Attributes& attrs);

    // Implements DefaultHandler::endElement().
    void endElement(const XMLCh* const uri, const XMLCh* const localname,
                    const XMLCh* const qname);

    // Implements DefaultHandler::characters().
    void characters(const XMLCh* const chars, const unsigned int length);

private:
    // SAX reader.
    SAX2XMLReader* reader;

    // Flag indicating whether the parsing is currently in a <TD> tag or not.
    bool inTD;

    // The current row of table data being parsed.
    vector<String>* row;

    // Keeps all non-XML or extra text.
    stringstream extraText;

    // Indicates whether the parsing is valid or not.
    bool valid;

    // Keep all parsed column keywords.
    map<int, vector<TBKeyword*>*> colkws;

    // Is true if the table data should be parsed, false otherwise.
    bool parsedata;
};

}

#endif /* TBPARSER_H_ */
