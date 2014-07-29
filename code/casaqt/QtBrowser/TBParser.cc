//# TBParser.cc: Parses the XMLDriver-generated XML into data in a TBTable.
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
#include <casaqt/QtBrowser/TBParser.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBKeyword.h>
#include <casaqt/QtBrowser/XMLtoken.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBData.h>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

using namespace xercesc;

namespace casa {

//////////////////////////
// TBPARSER DEFINITIONS //
//////////////////////////

// Constructors/Destructors //

TBParser::TBParser(TableParams* tp): insertRow(tp->insertRow),
          removeRow(tp->removeRow), data2(tp->data), fields(tp->fields),
          keywords(tp->keywords), subtableRows(tp->subtableRows),
          totalRows(tp->totalRows), loadedRows(tp->loadedRows),
          printdebug(true) { }

TBParser::~TBParser() { }

// Accessors/Mutators //

void TBParser::setPrintDebug(bool pdb) { printdebug = pdb; }

//////////////////////////////
// TBHOMEPARSER DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBHomeParser::TBHomeParser(TableParams* tp): TBParser(tp) { }

TBHomeParser::~TBHomeParser() {
    for(unsigned int i = 0; i < xfields.size(); i++)
        delete xfields.at(i);

    for(unsigned int i = 0; i < xkeywords.size(); i++)
        delete xkeywords.at(i);

    for(map<String, vector<XMLtoken*>*>::iterator iter = xcolkeywords.begin();
        iter != xcolkeywords.end(); iter++) {
        vector<XMLtoken*>* v = iter->second;
        for(unsigned int i = 0; i < v->size(); i++)
            delete v->at(i);
        delete v;
    }
}

// Public Methods //

Result TBHomeParser::parse(String* xml, bool parsedata) {
    bool ready = false;
    try {
        // Clear out old stuff
        for(unsigned int i = 0; i < xfields.size(); i++)
            delete xfields.at(i);
        for(unsigned int i = 0; i < xkeywords.size(); i++)
            delete xkeywords.at(i);
        for(map<String, vector<XMLtoken*>*>::iterator iter =
            xcolkeywords.begin(); iter != xcolkeywords.end(); iter++) {
            vector<XMLtoken*>* v = iter->second;
            for(unsigned int i = 0; i < v->size(); i++)
                delete v->at(i);
            delete v;
        }
        xfields.clear();
        xkeywords.clear();
        xcolkeywords.clear();
            
        if(printdebug)
            TBConstants::dprint(TBConstants::DEBUG_HIGH, "Parsing XML...");
        XMLtoken* t = parseToken(xml, 0);
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_HIGH,
                                           "...done parsing XML!");

        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_HIGH,
                                    "Parsing VOtable from XML tokens...");
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "Found token with name " + t->getName(), 1);
        if(TBConstants::equalsIgnoreCase(t->getName(),
           TBConstants::XML_VOTABLE)) {
            if(t->getTags()->size() >= 1) {
                t = t->getTags()->front();

                if(printdebug)
                    TBConstants::dprint(TBConstants::DEBUG_LOW,
                            "Found token with name " + t->getName(), 1);
                if(TBConstants::equalsIgnoreCase(t->getName(),
                        TBConstants::XML_RESOURCE)) {
                    if(t->getTags()->size() >= 1) {
                        t = t->getTags()->front();

                        if(printdebug)
                            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "Found token with name " + t->getName(), 1);
                        if(TBConstants::equalsIgnoreCase(t->getName(),
                                TBConstants::XML_TABLE) && parsedata) {
                            if(printdebug)
                                TBConstants::dprint(TBConstants::DEBUG_LOW,
                                        "Parsing table from XML...", 1);
                            ready = parseXMLtable(t, parsedata);
                            if(printdebug)
                                TBConstants::dprint(TBConstants::DEBUG_LOW,
                                        "...done parsing table from XML!", 1);
                        } else { ready = false; }// error
                    } else { ready = false; } // error
                } else { ready = false; } // error
            } else { ready = false; } // error
        } else { ready = false; } // error
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_HIGH,
                "...done parsing VOtable!");
    } catch(String error) { ready = false; } // error

    return Result(ready?"":"XML syntax error!", ready);
}

// Private Methods //

XMLtoken* TBHomeParser::parseToken(String* xml, int level) {
    //int n = xml.length();
    String name, attr, content;
    
    unsigned int i = xml->find('<');

    if(i >= xml->length()) { throw "No tags found."; } // error

    while(!isalpha(xml->at(size_t(i + 1)))) {
        // get rid of comments and meta-info
        i = xml->find('<', i + 1);
    }

    unsigned int j = xml->find('>', i);
    unsigned int k = TBConstants::findWS(*xml, i);

    if(j >= xml->length()) { throw "No ending >."; } // error
    
    if(k < j) { // tag has attributes
        name = xml->substr(i + 1, (k - i - 1));
        attr = xml->substr(k + 1, (j - k - 1));
    } else { // tag is just name
        name = xml->substr(i + 1, (j - i - 1));
        attr = "";
    }

    if(name[name.length() - 1] == '/') {
        name = name.erase(name.length() - 1);
    }
    if(!attr.empty() && attr[attr.length() - 1] == '/') {
        attr = attr.erase(attr.length() - 1);
    }

    XMLtoken* token = new XMLtoken(name);
    
    if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
            "Found tag name " + name, level);
    if(!attr.empty() && printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
            "Found attributes.", level + 1);

    String str = xml->substr(j + 1);
    TBConstants::strtrim(str);
    content = str;

    str = "</" + name + ">";
    unsigned int l = TBConstants::findIgnoreCase(content, str);

    if(l == 0) { // empty tag
        content = "";
    } else if(l < content.length()) {
        str = content.substr(0, l - 1);
        TBConstants::strtrim(str);
        content = str;
        if(printdebug)
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                    "Removed ending tag from remaining string.", level + 1);
    }

    // parse attributes
    if(!attr.empty()) {
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
                "Parsing attributes...", level + 1);
        parseAttributes(token, &attr, level);
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
                "...done parsing attributes!", level + 1);
    }

    // parse content
    if(!content.empty()) {
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
                "Parsing content...", level + 1);
        parseContent(token, &content, level);
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
                "...done parsing content!", level + 1);
    }
    
    return token;
}

void TBHomeParser::parseAttributes(XMLtoken* token,String* attrPtr,int /*level*/) {
    String attr = *attrPtr;
    while(attr.length() > 0) {
        unsigned int i = attr.find('=');

        if(i >= attr.length()) return; // invalid
        String str = attr.substr(0, i);
        TBConstants::strtrim(str);
        String name = str;

        str = attr.substr(i + 1);
        TBConstants::strtrim(str);
        attr = str;
        
        i = 0;
        unsigned int j = 0;
        if(attr[i] == '"') {
            i = 1;
            j = attr.find('"', i);
            if(j >= attr.length()) return; // invalid
        } else {
            j = TBConstants::findWS(attr);
            if(j > attr.length()) j = attr.length();
        }

        String value = attr.substr(i, j - i);

        token->getAttributes()->insert(make_pair(name, value));

        str = attr.substr(j + 1);
        if(j < attr.length()) {
            TBConstants::strtrim(str);
            attr = str;
        }
        else return; // done
    }
}

void TBHomeParser::parseContent(XMLtoken* token,String* contentPtr,int level) {
    String content = *contentPtr;
    if(content[0] == '<') { // contents are tokens
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
                "Contents are tokens.", level + 1);

        while(content.length() > 0) {
            if(content[0] == '<') {
                unsigned int i = content.find('>');
                String nextTag = content.substr(0, i + 1);
                String str = content.substr(i + 1);
                TBConstants::strtrim(str);
                content = str;

                //if(nextTag[nextTag.length() - 2] != '/') {
                    unsigned int j = nextTag.find('>');
                    unsigned int k = TBConstants::findWS(nextTag);

                    if(j >= nextTag.length()) { } // error

                    String name;
                    if(k < j) { // tag has attributes
                        name = nextTag.substr(1, (k - 1));
                    } else { // tag is just name
                        name = nextTag.substr(1, (j - 1));
                    }

                    if(name[name.length() - 1] == '/')
                        name.erase(name.length() - 1);

                    str = "</" + name + ">";
                    j = TBConstants::findIgnoreCase(content, str);

                    if(j < content.length()) {
                        nextTag += content.substr(0, j + 3 + name.length());
                        j = content.find('<', j + 1);

                        if(j > content.length()) j = content.length();
                    
                        content = content.substr(j);
                    }

                XMLtoken* t = parseToken(&nextTag, level + 1);
                token->getTags()->push_back(t);
            } else {
                return; // invalid?
            }
        }
    } else { // content is string
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
                "Content is string.", level + 1);
        token->setContent(content);
    }
}

bool TBHomeParser::parseXMLtable(XMLtoken* t, bool parsedata) {
    vector<XMLtoken*> colkws;
    for(unsigned int i = 0; i < t->getTags()->size(); i++) {
        XMLtoken* tag = t->getTags()->at(i);
        String name = tag->getName();
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_LOW,
                "Found token with name " + name, 2);

        if(TBConstants::equalsIgnoreCase(name, TBConstants::XML_TOTAL)) {
            String val = tag->getAttribute(TBConstants::XML_ROW);
            if(!val.empty()) {
                int v = TBConstants::atoi(val, &totalRows);
                if(v == EOF || v == 0)
                    totalRows = 0;
            }
            
        } else if(TBConstants::equalsIgnoreCase(name,TBConstants::XML_FIELD)) {
            xfields.push_back(tag);
        } else if(TBConstants::equalsIgnoreCase(name,
                TBConstants::XML_KEYWORD)) {
            xkeywords.push_back(tag);
        } else if(TBConstants::equalsIgnoreCase(name, 
                TBConstants::XML_COLUMNKW)) {
            colkws.push_back(tag);
        } else if(TBConstants::equalsIgnoreCase(name, 
                TBConstants::XML_RWINFO)) {
            String val = tag->getAttribute(TBConstants::XML_INSERTROW);
            if(!val.empty() && TBConstants::equalsIgnoreCase(val, 
                    TBConstants::XML_TRUE))
                insertRow = true;

            val = tag->getAttribute(TBConstants::XML_REMOVEROW);
            if(!val.empty() && TBConstants::equalsIgnoreCase(val, 
                    TBConstants::XML_TRUE))
                removeRow = true;
                
        } else if(TBConstants::equalsIgnoreCase(name, TBConstants::XML_DATA)) {
            if(!parsedata) continue;
            tag = tag->getTags()->front();

            if(TBConstants::equalsIgnoreCase(tag->getName(),
                    TBConstants::XML_TABLEDATA)) {
                for(unsigned int j = 0; j < tag->getTags()->size(); j++) {
                    // each row
                    vector<String>* dv = new vector<String>();
                    XMLtoken* row = tag->getTags()->at(j);

                    for(unsigned int k = 0; k < row->getTags()->size(); k++) {                      
                        dv->push_back(row->getTags()->at(k)->getContent());
                    }
                                
                    data.push_back(dv);
                    loadedRows++;
                }
            } else { return false; }
        } else { return false; } // error
    }

    // Set up column keywords
    vector<String> cols;

    for(unsigned int i = 0; i < xfields.size(); i++) {
        String name = xfields.at(i)->getAttribute(TBConstants::XML_FIELD_NAME);
        if(!name.empty()) cols.push_back(name);
        else return false;
    }
    
    for(unsigned int i = 0; i < colkws.size(); i++) {
        XMLtoken* t = colkws.at(i);
        String val = t->getAttribute(TBConstants::XML_COLKW_COL);
        if(!val.empty()) {
            int n;
            TBConstants::atoi(val, &n);
            String c = cols.at(n);

            map<String,vector<XMLtoken*>*>::iterator iter=xcolkeywords.find(c);

            if(iter == xcolkeywords.end()) {
                vector<XMLtoken*>* v = new vector<XMLtoken*>();
                v->push_back(t);
                xcolkeywords[c] = v;
            } else {
                vector<XMLtoken*>* v = iter->second;
                v->push_back(t);
            }
        }
    }

    // Finally, collect information into the "real" fields and keywords
    for(unsigned int i = 0; i < xkeywords.size(); i++) {
        String name = xkeywords.at(i)->getAttribute(
                TBConstants::XML_KEYWORD_NAME);
        String type = xkeywords.at(i)->getAttribute(
                TBConstants::XML_KEYWORD_TYPE);
            
        if(name.empty() || type.empty()) return false;

        String value = xkeywords.at(i)->getAttribute(
                TBConstants::XML_KEYWORD_VAL);
        TBData* d = TBData::create(value, type);
        if(d != NULL) {
            TBKeyword* keyword = new TBKeyword(name, *d);
            keywords.push_back(keyword);
            delete d;
        }
    }
    
    // Fields
    for(unsigned int i = 0; i < xfields.size(); i++) {
        String name = xfields.at(i)->getAttribute(TBConstants::XML_FIELD_NAME);
        String type = xfields.at(i)->getAttribute(TBConstants::XML_FIELD_TYPE);
        String unit = xfields.at(i)->getAttribute(TBConstants::XML_FIELD_UNIT);
        String ucd = xfields.at(i)->getAttribute(TBConstants::XML_FIELD_UCD);
        String ref = xfields.at(i)->getAttribute(TBConstants::XML_FIELD_REF);
        String precision = xfields.at(i)->getAttribute(
                TBConstants::XML_FIELD_PRECISION);
        String width = xfields.at(i)->getAttribute(
                TBConstants::XML_FIELD_WIDTH);
            
        if(name.empty() || type.empty()) return false;

        TBField* field = new TBField(name, type);
        fields.push_back(field);

        if(!unit.empty()) field->setUnit(unit);
        if(!ucd.empty()) field->setUCD(ucd);
        if(!ref.empty()) field->setRef(ref);
        if(!precision.empty()) field->setPrecision(precision);
        if(!width.empty()) field->setWidth(width);

        map<String,vector<XMLtoken*>*>::iterator iter=xcolkeywords.find(name);
        if(iter != xcolkeywords.end()) {
            vector<XMLtoken*>* kws = iter->second;
            
            for(unsigned int j = 0; j < kws->size(); j++) {
                name = kws->at(j)->getAttribute(TBConstants::XML_KEYWORD_NAME);
                type = kws->at(j)->getAttribute(TBConstants::XML_KEYWORD_TYPE);
                
                if(!name.empty() && !type.empty()) {
                    String value = kws->at(j)->getAttribute(
                            TBConstants::XML_KEYWORD_VAL);
                    TBData* d = TBData::create(value, type);
                    if(d != NULL) {
                        TBKeyword* kw = new TBKeyword(name, *d);
                        field->addKeyword(kw);
                        delete d;
                    }
                }
            }
        }
    }
    
    return true;
}

///////////////////////////////////
// TBXERCESDOMPARSER DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

TBXercesDOMParser::TBXercesDOMParser(TableParams* tp) : TBParser(tp) { }

TBXercesDOMParser::~TBXercesDOMParser() { }

// Public Methods //

Result TBXercesDOMParser::parse(String* xml, bool parsedata) {
    XercesDOMParser* parser = new XercesDOMParser();

    // convert the string into a parseable format
    MemBufInputSource* memBuf = new MemBufInputSource((const XMLByte*)
                    xml->c_str(), strlen(xml->c_str()), "xml_input", false);
    if(printdebug)
        TBConstants::dprint(TBConstants::DEBUG_HIGH, "Parsing XML...");
    parser->parse(*memBuf);
    if(printdebug)
        TBConstants::dprint(TBConstants::DEBUG_HIGH, "...done parsing XML!");

    if(printdebug)
        TBConstants::dprint(TBConstants::DEBUG_HIGH,
                "Parsing table from XML tokens...");
    Result r= parseXML(parser->getDocument()->getDocumentElement(), parsedata);
    if(printdebug)
        TBConstants::dprint(TBConstants::DEBUG_HIGH, "...done parsing table!");
    
    delete parser;
    delete memBuf;
    
    return r;
}

// Private Methods //

Result TBXercesDOMParser::parseXML(const DOMElement* element, bool parsedata) {
    if(element != NULL) {
        char* chst = XMLString::transcode(element->getTagName());
        String name(chst);
        XMLString::release(&chst);
        if(TBConstants::equalsIgnoreCase(name, TBConstants::XML_VOTABLE)) {
            DOMNodeList* nodes = element->getElementsByTagName(
                    XMLString::transcode(TBConstants::XML_RESOURCE.c_str()));
            if(nodes != NULL && nodes->getLength() > 0) {
                // should only be one <RESOURCE>
                element = (DOMElement*)nodes->item(0);
                nodes = element->getElementsByTagName(XMLString::transcode(
                        TBConstants::XML_TABLE.c_str()));
                if(nodes != NULL && nodes->getLength() > 0) {
                    // should only be one <TABLE>
                    element = (DOMElement*)nodes->item(0);
                    if(element != NULL) return parseTable(element, parsedata);
                    else return Result("Invalid XML structure.", false);
                } else return Result("Missing " + TBConstants::XML_TABLE + 
                        "tag.", false);
            } else return Result("Missing " + TBConstants::XML_RESOURCE + 
                    "tag.", false);
        } else return Result("Missing " + TBConstants::XML_VOTABLE + "tag.", 
                false);
    } else return Result("Invalid XML structure.", false);
}

Result TBXercesDOMParser::parseTable(const DOMElement* element,bool parsedata){
    DOMNodeList* nodes;
    DOMElement* e;
    char* chst;

    // Parse TOTAL
    nodes = element->getElementsByTagName(XMLString::transcode(
            TBConstants::XML_TOTAL.c_str()));
    for(unsigned int i = 0; i < nodes->getLength(); i++) {
        e = (DOMElement*)nodes->item(i);

        // TOTAL row
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_ROW.c_str())));
        String attr(chst);
        TBConstants::strtrim(attr);
        XMLString::release(&chst);
        if(!attr.empty()) {
            if(TBConstants::atoi(attr, &totalRows) != 1) totalRows = 0;
            // error parsing total rows
        }
    }
    
    // Parse FIELDs
    nodes = element->getElementsByTagName(XMLString::transcode(
            TBConstants::XML_FIELD.c_str()));
    for(unsigned int i = 0; i < nodes->getLength(); i++) {
        e = (DOMElement*)nodes->item(i);

        // FIELD name
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_FIELD_NAME.c_str())));
        String name(chst);
        XMLString::release(&chst);
        if(name.empty()) continue;
        
        // FIELD type
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_FIELD_TYPE.c_str())));
        String type(chst);
        XMLString::release(&chst);
        if(type.empty()) continue;
        
        TBField* field = new TBField(name, type);

        // FIELD ucd
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_FIELD_UCD.c_str())));
        String attr(chst);
        XMLString::release(&chst);
        if(!attr.empty()) field->setUCD(attr);
                
        // FIELD ref
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_FIELD_REF.c_str())));
        attr = String(chst);
        XMLString::release(&chst);
        if(!attr.empty()) field->setRef(attr);

        // FIELD unit
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_FIELD_UNIT.c_str())));
        attr = String(chst);
        XMLString::release(&chst);
        if(!attr.empty()) field->setUnit(attr);

        // FIELD precision
        chst = XMLString::transcode(e->getAttribute(
              XMLString::transcode(TBConstants::XML_FIELD_PRECISION.c_str())));
        attr = String(chst);
        XMLString::release(&chst);
        if(!attr.empty()) field->setPrecision(attr);
                
        // FIELD width
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_FIELD_WIDTH.c_str())));
        attr = String(chst);
        XMLString::release(&chst);
        if(!attr.empty()) field->setWidth(attr);

        fields.push_back(field);
    }
    
    // Parse KEYWORDs
    nodes = element->getElementsByTagName(XMLString::transcode(
            TBConstants::XML_KEYWORD.c_str()));
    for(unsigned int i = 0; i < nodes->getLength(); i++) {
        e = (DOMElement*)nodes->item(i);

        // KEYWORD name
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_KEYWORD_NAME.c_str())));
        String name(chst);
        XMLString::release(&chst);
        if(name.empty()) continue;
        
        // KEYWORD type
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_KEYWORD_TYPE.c_str())));
        String type(chst);
        XMLString::release(&chst);
        if(type.empty()) continue;

        // KEYWORD val
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_KEYWORD_VAL.c_str())));
        String val(chst);
        XMLString::release(&chst);

        TBData* d = TBData::create(val, type);
        if(d != NULL) {
            TBKeyword* keyword = new TBKeyword(name, *d);
            keywords.push_back(keyword);
            delete d;
        }
    }
    
    // Parse COLUMNKWs
    nodes = element->getElementsByTagName(XMLString::transcode(
            TBConstants::XML_COLUMNKW.c_str()));
    for(unsigned int i = 0; i < nodes->getLength(); i++) {
        e = (DOMElement*)nodes->item(i);

        // COLUMNKW col
        // find corresponding field
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_COLKW_COL.c_str())));
        String str(chst);
        XMLString::release(&chst);
        if(str.empty()) continue;
        int col;
        if(TBConstants::atoi(str, &col) < 1 || col < 0 ||
                col >= (int)fields.size()) continue;
        TBField* field = fields.at(col);
        if(field == NULL) continue;
        
        // COLUMNKW name
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_KEYWORD_NAME.c_str())));
        String name(chst);
        XMLString::release(&chst);
        if(name.empty()) continue;
        
        // COLUMNKW type
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_KEYWORD_TYPE.c_str())));
        String type(chst);
        XMLString::release(&chst);
        if(type.empty()) continue;

        // COLUMNKW val
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_KEYWORD_VAL.c_str())));
        String val(chst);
        XMLString::release(&chst); 

        TBData* d = TBData::create(val, type);
        if(d != NULL) {
            TBKeyword* keyword = new TBKeyword(name, *d);
            field->addKeyword(keyword);
            delete d;
        }
    }

    // Parse RWINFO
    nodes = element->getElementsByTagName(XMLString::transcode(
            TBConstants::XML_RWINFO.c_str()));
    for(unsigned int i = 0; i < nodes->getLength(); i++) {
        e = (DOMElement*)nodes->item(i);

        // RWINFO insertRow
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_INSERTROW.c_str())));
        String attr(chst);
        XMLString::release(&chst);
        if(!attr.empty() && TBConstants::equalsIgnoreCase(attr, 
                TBConstants::XML_TRUE)) insertRow = true;
        else insertRow = false;

        // RWINFO removeRow
        chst = XMLString::transcode(e->getAttribute(
               XMLString::transcode(TBConstants::XML_REMOVEROW.c_str())));
        attr = String(chst);
        XMLString::release(&chst);
        if(!attr.empty() && TBConstants::equalsIgnoreCase(attr, 
                TBConstants::XML_TRUE)) removeRow = true;
        else removeRow = false;
    }
    
    // Parse DATA
    if(parsedata) {
        nodes = element->getElementsByTagName(XMLString::transcode(
                TBConstants::XML_DATA.c_str()));
        if(nodes != NULL && nodes->getLength() > 0) {
            // first tag should be TABLEDATA
            e = (DOMElement*)nodes->item(0);
            return parseTableData(e);
        }
    }

    return Result("", true);
}

Result TBXercesDOMParser::parseTableData(const DOMElement* element) {
    DOMNodeList* nodes, *dataNodes;
    DOMElement* e;

    if(printdebug) TBConstants::dprint(TBConstants::DEBUG_MED,
            "Parsing table data...", 1);
    // Parse TRs
    nodes = element->getElementsByTagName(XMLString::transcode(
            TBConstants::XML_TR.c_str()));
    char* chst;
    for(unsigned int i = 0; i < nodes->getLength(); i++) {
        e = (DOMElement*)nodes->item(i);

        // Parse TDs
        dataNodes = e->getElementsByTagName(XMLString::transcode(
                TBConstants::XML_TD.c_str()));
        if(dataNodes->getLength() == 0)
            return Result("Malformed table data.", false);

        vector<String>* row = new vector<String>();

        for(unsigned int j = 0; j < dataNodes->getLength(); j++) {
            e = (DOMElement*) dataNodes->item(j);
            chst = XMLString::transcode(e->getTextContent());
            String data(chst);
            TBConstants::strtrim(data);
            XMLString::release(&chst);
            row->push_back(data);
        }
        data.push_back(row);
        loadedRows++;
    }

    if(printdebug)TBConstants::dprint(TBConstants::DEBUG_MED,
            "...done parsing table data!", 1);
    return Result("", true);
}

///////////////////////////////////
// TBXERCESSAXPARSER DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

TBXercesSAXParser::TBXercesSAXParser(TableParams* tp) : TBParser(tp),
                   DefaultHandler(), inTD(false), row(NULL), extraText(),
                   valid(true) {
    reader = XMLReaderFactory::createXMLReader();
    reader->setContentHandler(this);
    reader->setErrorHandler(this);
}

TBXercesSAXParser::~TBXercesSAXParser() {
    delete reader;
}

// Public Methods //

Result TBXercesSAXParser::parse(String* xml, bool parsedata) {
    this->parsedata = parsedata;
    // convert the string into a parseable format
    MemBufInputSource* memBuf = new MemBufInputSource((const XMLByte*)
                    xml->c_str(), strlen(xml->c_str()), "xml_input", false);
    reader->parse(*memBuf);
    delete memBuf;
    
    return Result(valid?"":"XML syntax error", valid);
}

void TBXercesSAXParser::startDocument() {
    if(printdebug) TBConstants::dprint(TBConstants::DEBUG_HIGH,
            "Parsing table from XML...");
}

void TBXercesSAXParser::endDocument() {
    // clear/delete stuff in colkws
    for(map<int, vector<TBKeyword*>*>::iterator iter = colkws.begin();
        iter != colkws.end(); iter++) {
        int c = iter->first;
        vector<TBKeyword*>* v = iter->second;
        if(0 <= c && c < (int)fields.size()) {
            TBField* f = fields.at(c);
            for(unsigned int i = 0; i < v->size(); i++)
                f->addKeyword(v->at(i));
        }
        delete v;
    }
    colkws.clear();

    String str = extraText.str();
    extraText.str("");
    if(printdebug) {
        if(!str.empty()) TBConstants::dprint(TBConstants::DEBUG_MED,
                "Extra text during SAX parse: " + str);
        TBConstants::dprint(TBConstants::DEBUG_HIGH,
                "...done parsing table from XML!");
    }
}
    
void TBXercesSAXParser::startElement(const XMLCh* const /*uri*/,
                                     const XMLCh* const /*localname*/,
                                     const XMLCh* const qname,
                                     const Attributes& attrs) {
    char* chst = XMLString::transcode(qname);
    String name(chst);
    XMLString::release(&chst);
    if(TBConstants::equalsIgnoreCase(name, TBConstants::XML_TOTAL)) {
        chst = XMLString::transcode(attrs.getValue(
               XMLString::transcode(TBConstants::XML_ROW.c_str())));
        String r(chst);
        XMLString::release(&chst);
        if(!r.empty()) {
            int i;
            if(TBConstants::atoi(r, &i) >= 1) totalRows = i;
        }
    } else if(TBConstants::equalsIgnoreCase(name, TBConstants::XML_FIELD)) {
        chst = XMLString::transcode(attrs.getValue(
               XMLString::transcode(TBConstants::XML_FIELD_NAME.c_str())));
        String name(chst);
        XMLString::release(&chst);
        chst = XMLString::transcode(attrs.getValue(
                XMLString::transcode(TBConstants::XML_FIELD_TYPE.c_str())));
        String type(chst);
        XMLString::release(&chst);

        if(!name.empty() && !type.empty()) {
            TBField* field = new TBField(name, type);

            chst = XMLString::transcode(attrs.getValue(
                   XMLString::transcode(TBConstants::XML_FIELD_UCD.c_str())));
            String v(chst);
            XMLString::release(&chst);
            if(!v.empty()) field->setUCD(v);

            chst = XMLString::transcode(attrs.getValue(
                   XMLString::transcode(TBConstants::XML_FIELD_REF.c_str())));
            v = String(chst);
            XMLString::release(&chst);
            if(!v.empty()) field->setRef(v);

            chst = XMLString::transcode(attrs.getValue(
                  XMLString::transcode(TBConstants::XML_FIELD_UNIT.c_str())));
            v = String(chst);
            XMLString::release(&chst);
            if(!v.empty()) field->setUnit(v);

            chst = XMLString::transcode(attrs.getValue(
             XMLString::transcode(TBConstants::XML_FIELD_PRECISION.c_str())));
            v = String(chst);
            XMLString::release(&chst);
            if(!v.empty()) field->setPrecision(v);

            chst = XMLString::transcode(attrs.getValue(
                 XMLString::transcode(TBConstants::XML_FIELD_WIDTH.c_str())));
            v = String(chst);
            XMLString::release(&chst);
            if(!v.empty()) field->setWidth(v);
            
            fields.push_back(field);
        }
    } else if(TBConstants::equalsIgnoreCase(name, TBConstants::XML_KEYWORD)) {
        chst = XMLString::transcode(attrs.getValue(
               XMLString::transcode(TBConstants::XML_KEYWORD_NAME.c_str())));
        String name(chst);
        XMLString::release(&chst);
        chst = XMLString::transcode(attrs.getValue(
               XMLString::transcode(TBConstants::XML_KEYWORD_TYPE.c_str())));
        String type(chst);
        XMLString::release(&chst);
        
        if(!name.empty() && !type.empty()) {
            chst = XMLString::transcode(attrs.getValue(
                 XMLString::transcode(TBConstants::XML_KEYWORD_VAL.c_str())));
            String val(chst);
            XMLString::release(&chst);
            TBData* d = TBData::create(val, type);
            if(d != NULL) {
                TBKeyword* keyword = new TBKeyword(name, *d);
                keywords.push_back(keyword);
                delete d;
            }
        }
    } else if(TBConstants::equalsIgnoreCase(name,TBConstants::XML_COLUMNKW)) {
        chst = XMLString::transcode(attrs.getValue(
               XMLString::transcode(TBConstants::XML_KEYWORD_NAME.c_str())));
        String name(chst);
        XMLString::release(&chst);
        chst = XMLString::transcode(attrs.getValue(
               XMLString::transcode(TBConstants::XML_KEYWORD_TYPE.c_str())));
        String type(chst);
        XMLString::release(&chst);
        if(!name.empty() && !type.empty()) {
            chst = XMLString::transcode(attrs.getValue(
                 XMLString::transcode(TBConstants::XML_KEYWORD_VAL.c_str())));
            String val(chst);
            XMLString::release(&chst);
            TBData* d = TBData::create(val, type);
            if(d != NULL) {
                TBKeyword* keyword = new TBKeyword(name, *d);
                delete d;

                chst = XMLString::transcode(attrs.getValue(
                        XMLString::transcode(TBConstants::XML_COLKW_COL.c_str())));
                String cs(chst);
                XMLString::release(&chst);
                int c;
                if(!cs.empty() && TBConstants::atoi(cs, &c) >= 1) {
                    map<int, vector<TBKeyword*>*>::iterator iter = colkws.find(c);
                    if(iter == colkws.end()) {
                        vector<TBKeyword*>* v = new vector<TBKeyword*>();
                        v->push_back(keyword);
                        colkws[c] = v;
                    } else {
                        vector<TBKeyword*>* v = iter->second;
                        v->push_back(keyword);
                    }
                }
            }
        }
    } else if(TBConstants::equalsIgnoreCase(name, TBConstants::XML_RWINFO)) {
        chst = XMLString::transcode(attrs.getValue(
               XMLString::transcode(TBConstants::XML_INSERTROW.c_str())));
        String v(chst);
        XMLString::release(&chst);
        if(v == "0" || TBConstants::equalsIgnoreCase(v,
                TBConstants::XML_FALSE)) insertRow = false;
        else insertRow = true;

        chst = XMLString::transcode(attrs.getValue(
               XMLString::transcode(TBConstants::XML_REMOVEROW.c_str())));
        v = String(chst);
        XMLString::release(&chst);
        if(v == "0" || TBConstants::equalsIgnoreCase(v,
                TBConstants::XML_FALSE)) removeRow = false;
        else removeRow = true;
    } else if(parsedata && TBConstants::equalsIgnoreCase(name,
            TBConstants::XML_TR)) {
        row = new vector<String>();
    } else if(parsedata && TBConstants::equalsIgnoreCase(name,
            TBConstants::XML_TD)) {
        inTD = true;
    } else if(parsedata && TBConstants::equalsIgnoreCase(name,
            TBConstants::XML_TABLEDATA)) {
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_HIGH,
                "Parsing table data...");
    }
}
    
void TBXercesSAXParser::endElement(const XMLCh* const /*uri*/,
                                   const XMLCh* const /*localname*/,
                                   const XMLCh* const qname) {
    char* chst = XMLString::transcode(qname);
    String name(chst);
    XMLString::release(&chst);
    if(parsedata && TBConstants::equalsIgnoreCase(name, TBConstants::XML_TR)){
        data.push_back(row);
        loadedRows++;
    } else if(parsedata && TBConstants::equalsIgnoreCase(name,
            TBConstants::XML_TD)) {
        inTD = false;
    } else if(parsedata && TBConstants::equalsIgnoreCase(name,
            TBConstants::XML_TABLEDATA)) {
        if(printdebug) TBConstants::dprint(TBConstants::DEBUG_HIGH,
                "...done parsing table data!");
    }
}

void TBXercesSAXParser::characters(const XMLCh* const chars,
                                   const unsigned int /*length*/) {
    char* chst = XMLString::transcode(chars);
    String str(chst);
    XMLString::release(&chst);
    if(inTD) {
        if(row != NULL) {
            TBConstants::strtrim(str);
            row->push_back(str);
        }
        else valid = false;
    } else {
        TBConstants::strtrim(str);
        if(!str.empty()) extraText << '"' << str << "\"\n";
    }
}

}
