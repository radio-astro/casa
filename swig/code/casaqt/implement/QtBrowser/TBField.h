//# TBField.h: Representation of a table field.
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
#ifndef TBFIELD_H_
#define TBFIELD_H_

#include <vector>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBKeyword;

// <summary>
// Representation of a table field.
// <summary>
//
// <synopsis>
// A TBField consists of a name, a type, a potentially empty list of keywords,
// and some other potentially empty properties.
// </synopsis>

class TBField {
public:
    // Constructor to take the name and type.  Other properties must be added
    // via method calls.
    TBField(String name, String type);

    ~TBField();

    
    // Returns this field's name.
    String getName();

    // Returns this field's type.
    String getType();

    // Returns this field's keywords.
    vector<TBKeyword*>* getKeywords();
    
    // Returns the keyword at index i, or NULL if there is none.
    TBKeyword* keyword(int i);
    
    // Returns the keyword with name i, or NULL if there is none.
    TBKeyword* keyword(String name);

    // Returns the number of keywords this field has.
    unsigned int numKeywords();

    // Returns the keyword at index i.
    TBKeyword* keywordAt(unsigned int i);

    // Adds the given keyword to this field's keyword list.
    void addKeyword(TBKeyword* keyword);

    // Sets this field's UCD property.
    void setUCD(String ucd);

    // Returns this field's UCD property, or an empty String if it is not set.
    String getUCD();

    // Sets this field's ref property.
    void setRef(String ref);

    // Returns this field's ref property, or an empty String if it is not set.
    String getRef();

    // Sets this field's unit property.
    void setUnit(String unit);
    
    // Returns this field's unit property, or an empty String if it is not set.
    String getUnit();

    // Sets this field's precision property.
    void setPrecision(String precision);
    
    // Returns this field's precision property, or an empty String if it is not
    // set.
    String getPrecision();

    // Sets this field's width property.
    void setWidth(String width);
    
    // Returns this field's width property, or an empty String if it is not
    // set.
    String getWidth();

private:
    // Field name.
    String name;

    // Field type.
    String type;

    // Field keywords.
    vector<TBKeyword*> keywords;

    // Field UCD.
    String ucd;

    // Field ref.
    String ref;

    // Field unit.
    String unit;

    // Field precision.
    String precision;

    // Field width.
    String width;
};

}

#endif /* TBFIELD_H_ */
