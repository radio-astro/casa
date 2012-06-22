//# TBKeyword.cc: Representation of a keyword attached to a table or field.
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
#include <casaqt/QtBrowser/TBKeyword.h>

namespace casa {
///////////////////////////
// TBKEYWORD DEFINITIONS //
///////////////////////////

// Constructors/Destructors //

/*
TBKeyword::TBKeyword(String n, String t, String v, String d): name(n), type(t),
                     value(v), displayValue(d) { }
*/

TBKeyword::TBKeyword(String n, TBData& d): name(n),
                                           data(TBData::create(d)) {
    if(data == NULL) data = new TBDataString("ERROR");
}

TBKeyword::~TBKeyword() { }

// Accessor Methods //

String TBKeyword::getName() { return name; }
    
String TBKeyword::getType() { return data->getType(); }
    
TBData* TBKeyword::getValue() { return data; }

String TBKeyword::getDisplayValue() {
    return data->asString();
}

}
