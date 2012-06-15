//# XMLtoken.h: Representation of a single XML token.
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
#ifndef XMLTOKEN_H_
#define XMLTOKEN_H_

#include <vector>
#include <map>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// <summary>
// Representation of a single XML token.
// </summary>
//
// <synopsis>
// XMLtoken encapsulates an XML token which includes a name, zero or more
// attributes, and optional content which can be a String or one or more
// XMLtokens.
// </synopsis>

class XMLtoken {
public:
	// Constructor that takes the tag name.
    XMLtoken(String n);

    ~XMLtoken();

    
    // Returns the attributes for this tag.
    map<String, String>* getAttributes();

    // Returns the list of content tags, or an empty list if there are none.
    vector<XMLtoken*>* getTags();

    // Returns this tag's name.
    const String getName();

    // Sets the String content of this tag to the given value.
    void setContent(String c);

    // Returns the String content of this tag, or blank if there is none.
    const String getContent();

    
    // Returns the value for the given attribute, or blank if the attribute
    // is invalid.
    String getAttribute(String attr);
    
private:
	// Tag name.
    const String name;
    
    // String content (or blank for none).
    String content;

    // Token content (empty list for none).
    vector<XMLtoken*> tags;
    
    // Tag attributes.
    map<String, String> attributes;
};

}

#endif /* XMLTOKEN_H_ */
