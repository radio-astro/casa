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
    TBField(casacore::String name, casacore::String type);

    ~TBField();

    
    // Returns this field's name.
    casacore::String getName();

    // Returns this field's type.
    casacore::String getType();

    // Returns this field's keywords.
    std::vector<TBKeyword*>* getKeywords();
    
    // Returns the keyword at index i, or NULL if there is none.
    TBKeyword* keyword(int i);
    
    // Returns the keyword with name i, or NULL if there is none.
    TBKeyword* keyword(casacore::String name);

    // Returns the number of keywords this field has.
    unsigned int numKeywords();

    // Returns the keyword at index i.
    TBKeyword* keywordAt(unsigned int i);

    // Adds the given keyword to this field's keyword list.
    void addKeyword(TBKeyword* keyword);

    // Sets this field's UCD property.
    void setUCD(casacore::String ucd);

    // Returns this field's UCD property, or an empty casacore::String if it is not set.
    casacore::String getUCD();

    // Sets this field's ref property.
    void setRef(casacore::String ref);

    // Returns this field's ref property, or an empty casacore::String if it is not set.
    casacore::String getRef();

    // Sets this field's unit property.
    void setUnit(casacore::String unit);
    
    // Returns this field's unit property, or an empty casacore::String if it is not set.
    casacore::String getUnit();

    // Sets this field's precision property.
    void setPrecision(casacore::String precision);
    
    // Returns this field's precision property, or an empty casacore::String if it is not
    // set.
    casacore::String getPrecision();

    // Sets this field's width property.
    void setWidth(casacore::String width);
    
    // Returns this field's width property, or an empty casacore::String if it is not
    // set.
    casacore::String getWidth();

private:
    // Field name.
    casacore::String name;

    // Field type.
    casacore::String type;

    // Field keywords.
    std::vector<TBKeyword*> keywords;

    // Field UCD.
    casacore::String ucd;

    // Field ref.
    casacore::String ref;

    // Field unit.
    casacore::String unit;

    // Field precision.
    casacore::String precision;

    // Field width.
    casacore::String width;
};

}

#endif /* TBFIELD_H_ */
