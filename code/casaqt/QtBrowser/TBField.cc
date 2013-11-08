//# TBField.cc: Representation of a table field.
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
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBKeyword.h>

namespace casa {

/////////////////////////
// TBFIELD DEFINITIONS //
/////////////////////////

// Constructors/Destructors //

TBField::TBField(String n, String t): name(n), type(t), keywords() { }

TBField::~TBField() {
    for(unsigned int i = 0; i < keywords.size(); i++)
        delete keywords.at(i);
}

// Accessor Methods //

String TBField::getName() { return name; }

String TBField::getType() { return type; }

vector<TBKeyword*>* TBField::getKeywords() { return &keywords; }

TBKeyword* TBField::keyword(int i) {
    if(i >= 0 && i < (int)keywords.size()) return keywords.at(i);
    else return NULL;
}

TBKeyword* TBField::keyword(String name) {
    for(unsigned int i = 0; i < keywords.size(); i++)
        if(keywords.at(i)->getName() == name)
            return keywords.at(i);
    
    return NULL;
}

unsigned int TBField::numKeywords() { return keywords.size(); }

void TBField::setUCD(String u) { ucd = u; }

String TBField::getUCD() { return ucd; }

void TBField::setRef(String r) { ref = r; }

String TBField::getRef() { return ref; }

void TBField::setUnit(String u) { unit = u; }

String TBField::getUnit() { return unit; }

void TBField::setPrecision(String p) { precision = p; }

String TBField::getPrecision() { return precision; }

void TBField::setWidth(String w) { width = w; }

String TBField::getWidth() { return width; }

// Public Methods //

TBKeyword* TBField::keywordAt(unsigned int i) {
    if(i < keywords.size())
        return keywords.at(i);
    else
        return NULL;
}

void TBField::addKeyword(TBKeyword* keyword) {
    if(keyword != NULL)
        keywords.push_back(keyword);
}

}
