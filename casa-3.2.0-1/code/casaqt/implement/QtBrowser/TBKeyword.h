//# TBKeyword.h: Representation of a keyword attached to a table or field.
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
#ifndef TBKEYWORD_H_
#define TBKEYWORD_H_

#include <casaqt/QtBrowser/TBData.h>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

// <summary>
// Representation of a keyword attached to a table or field.
// <summary>
//
// <synopsis>
// A TBKeyword can be attached either to an entire table or to a specific field
// in a table.  Keywords consist of a name, type, and value.
// </synopsis>

class TBKeyword {
public:
    // Constructor which takes the name, type, and value of the keyword.
    // Optionally, a display value can also be entered.  (For example, array
    // keywords may not want to display their whole value.)
    // TBKeyword(String name, String type, String value, String dispValue = "");
    
    TBKeyword(String name, TBData& data);

    ~TBKeyword();

    
    // Returns the name of this keyword.
    String getName();

    // Returns the type of this keyword.
    String getType();

    // Returns the value of this keyword.
    TBData* getValue();

    // Returns the display value of this keyword.  If no display value has been
    // defined, the normal value is returned.
    String getDisplayValue();
    
private:
    // Keyword name.
    String name;

    // Keyword value.
    TBData* data;
};

}

#endif /* TBKEYWORD_H_ */
