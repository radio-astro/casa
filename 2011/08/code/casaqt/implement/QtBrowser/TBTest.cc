//# TBTest.cc: Tests to check the validity of a table.
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
#include <casaqt/QtBrowser/TBTest.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBBrowser.qo.h>
#include <casaqt/QtBrowser/TBTableTabs.qo.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBKeyword.h>

#include <fstream>

namespace casa {

////////////////////////
// TBTEST DEFINITIONS //
////////////////////////

// Constructors/Destructors //

TBTest::TBTest(TBBrowser* b, String n) : browser(b), name(n) { }

TBTest::~TBTest() { }

// Accessor Methods //

String TBTest::getName() { return name; }

////////////////////////////////
// TBIDFIELDSTEST DEFINITIONS //
////////////////////////////////

// Constructors/Destructors //

TBIDFieldsTest::TBIDFieldsTest(TBBrowser* b):
                                    TBTest(b, "ID Fields are Valid") { }

TBIDFieldsTest::~TBIDFieldsTest() { }

// Public Methods //

vector<String> TBIDFieldsTest::checks(String t) {
    vector<String> c;

    TBTable* table = browser->table(t)->getTable();
    vector<TBField*>* fields = table->getFields();

    for(unsigned int i = 0; i < fields->size(); i++) {
        String name = fields->at(i)->getName();
        int n = name.length();
        if(n > 3 && name[n - 3] == '_' && (name[n - 2] == 'I' ||
           name[n - 2] == 'i') && (name[n - 1] == 'D' || name[n - 1] == 'd')) {
            c.push_back("Field " + name + " has corresponding table keyword " +
                        name.substr(0, n - 3) + ".");
        }
    }

    return c;
}

bool TBIDFieldsTest::runCheck(String t, int id) {
    TBTable* table = browser->table(t)->getTable();
    vector<TBField*>* fields = table->getFields();
    vector<TBKeyword*>* keywords = table->getTableKeywords();

    int count = 0;
    String name;
    for(unsigned int i = 0; i < fields->size(); i++) {
        name = fields->at(i)->getName();
        int n = name.length();
        if(n > 3 && name[n - 3] == '_' && (name[n - 2] == 'I' ||
           name[n - 2] == 'i') && (name[n - 1] == 'D' || name[n - 1] == 'd')) {
            if(count != id) {
                count++;
                continue;
            } else break;
        }
    }
            
    name = name.substr(0, name.length() - 3);
    for(unsigned int j = 0; j < keywords->size(); j++) {
        if(name == keywords->at(j)->getName())
            return true;
    }

    return false;
}

/////////////////////////////////
// TBSUBTABLESTEST DEFINITIONS //
/////////////////////////////////

// Constructors/Destructors //

TBSubtablesTest::TBSubtablesTest(TBBrowser* b): TBTest(b,"Subtables Exist") { }

TBSubtablesTest::~TBSubtablesTest() { }

// Public Methods //

vector<String> TBSubtablesTest::checks(String t) {
    vector<String> c;

    TBTable* table = browser->table(t)->getTable();
    vector<TBKeyword*>* keywords = table->getTableKeywords();

    for(unsigned int i = 0; i < keywords->size(); i++) {
        String type = keywords->at(i)->getType();
        if(TBConstants::typeIsTable(type)) {
            String name = keywords->at(i)->getName();
            c.push_back("Subtable " + name + " exists on disk at: " +
                        keywords->at(i)->getValue()->asString() + ".");
        }
    }
    
    return c;
}

bool TBSubtablesTest::runCheck(String t, int id) {
    TBTable* table = browser->table(t)->getTable();
    vector<TBKeyword*>* keywords = table->getTableKeywords();

    int count = 0;

    for(unsigned int i = 0; i < keywords->size(); i++) {
        String type = keywords->at(i)->getType();
        if(TBConstants::typeIsTable(type)) {
            if(count != id) {
                count++;
                continue;
            }
            
            String loc = keywords->at(i)->getValue()->asString();

            fstream fin;
            fin.open(loc.c_str(),ios::in);
            bool open = fin.is_open();
            fin.close();
            if(!open) return false;
        }
    }
    
    return true;
}

//////////////////////////////////////
// TBVALIDSUBTABLESTEST DEFINITIONS //
//////////////////////////////////////

// Constructors/Destructors //

TBValidSubtablesTest::TBValidSubtablesTest(TBBrowser* b):
                                        TBTest(b, "Subtables Have Data") { }

TBValidSubtablesTest::~TBValidSubtablesTest() { }

// Public Methods //

vector<String> TBValidSubtablesTest::checks(String t) {
    vector<String> c;

    TBTable* table = browser->table(t)->getTable();
    vector<TBKeyword*>* keywords = table->getTableKeywords();

    for(unsigned int i = 0; i < keywords->size(); i++) {
        String type = keywords->at(i)->getType();
        if(TBConstants::typeIsTable(type)) {
            String name = keywords->at(i)->getName();
            c.push_back("Subtable " + name +
                    " can be opened and has at least one row of data.");
        }
    }
    
    return c;
}

bool TBValidSubtablesTest::runCheck(String t, int id) {
    TBTable* tb = browser->table(t)->getTable();
    vector<TBKeyword*>* keywords = tb->getTableKeywords();

    int count = 0;

    for(unsigned int i = 0; i < keywords->size(); i++) {
        String type = keywords->at(i)->getType();
        if(TBConstants::typeIsTable(type)) {
            if(count != id) {
                count++;
                continue;
            }
            
            String loc = keywords->at(i)->getValue()->asString();
            int r = tb->totalRowsOf(loc);
            return r > 0;
        }
    }
    
    return false;
}

}
