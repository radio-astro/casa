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

namespace casa {

// <summary>
// Representation of a single XML token.
// </summary>
//
// <synopsis>
// XMLtoken encapsulates an XML token which includes a name, zero or more
// attributes, and optional content which can be a casacore::String or one or more
// XMLtokens.
// </synopsis>

class XMLtoken {
public:
	// Constructor that takes the tag name.
    XMLtoken(casacore::String n);

    ~XMLtoken();

    
    // Returns the attributes for this tag.
    std::map<casacore::String, casacore::String>* getAttributes();

    // Returns the list of content tags, or an empty list if there are none.
    std::vector<XMLtoken*>* getTags();

    // Returns this tag's name.
    const casacore::String getName();

    // Sets the casacore::String content of this tag to the given value.
    void setContent(casacore::String c);

    // Returns the casacore::String content of this tag, or blank if there is none.
    const casacore::String getContent();

    
    // Returns the value for the given attribute, or blank if the attribute
    // is invalid.
    casacore::String getAttribute(casacore::String attr);
    
private:
	// Tag name.
    const casacore::String name;
    
    // casacore::String content (or blank for none).
    casacore::String content;

    // Token content (empty list for none).
    std::vector<XMLtoken*> tags;
    
    // Tag attributes.
    std::map<casacore::String, casacore::String> attributes;
};

}

#endif /* XMLTOKEN_H_ */
