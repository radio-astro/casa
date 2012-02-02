//# XMLtoken.cc: Representation of a single XML token.
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
#include <casaqt/QtBrowser/XMLtoken.h>

namespace casa {

//////////////////////////
// XMLTOKEN DEFINITIONS //
//////////////////////////

// Constructors/Destructors //

XMLtoken::XMLtoken(String n): name(n), tags(), attributes() { }

XMLtoken::~XMLtoken() {
    for(unsigned int i = 0; i < tags.size(); i++)
        delete tags.at(i);
}

// Accessors/Mutators //

map<String, String>* XMLtoken::getAttributes() { return &attributes; }

vector<XMLtoken*>* XMLtoken::getTags() { return &tags; }

const String XMLtoken::getName() { return name; }
    
void XMLtoken::setContent(String c) { content = c; }

const String XMLtoken::getContent() { return content; }

// Public Methods //

String XMLtoken::getAttribute(String attr) {
    map<String, String>::iterator iter = attributes.find(attr);
            
    if(iter != attributes.end()) return iter->second.c_str();
    else return "";
}

}
