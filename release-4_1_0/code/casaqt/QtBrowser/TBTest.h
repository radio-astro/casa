//# TBTest.h: Tests to check the validity of a table.
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
#ifndef TBTEST_H_
#define TBTEST_H_

#include <vector>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBBrowser;

// <summary>
// Tests to check the validity of a table.
// </summary>
//
// <synopsis>
// TBTest is an abstract superclass for any class that wants to test the
// validity of a table.  A "test" can be thought of as a series of checks, or
// smaller tests.  Although currently only basic tests are implemented, more
// complex tests could be added in the future; for example, a check to make
// sure the table has a valid structure if it is a Measurement Set.
// </synopsis>

class TBTest {
public:
	// Constructor that takes the browser parent and the name of the test.
    TBTest(TBBrowser* browser, String name);

    virtual ~TBTest();

    
    // Returns the name of the test.
    String getName();
    
    
    // checks() must be implemented by any subclass.
    // Returns a list of the names of the checks in this test, given the name
    // of the table on which the test will be run.
    virtual vector<String> checks(String table) = 0;

    // runCheck() must be implemented by any subclass.
    // Runs the given check on the given table and returns the result.
    virtual bool runCheck(String table, int i) = 0;

protected:
	// Browser parent.
	TBBrowser* browser;
	
	// Name of the test.
    String name;
};

// <summary>
// Tests whether fields ending in _ID have a corresponding subtable.
// </summary>

class TBIDFieldsTest : public TBTest {
public:
	// Constructor that takes the browser parent.
    TBIDFieldsTest(TBBrowser* browser);

    ~TBIDFieldsTest();

    // Implements TBTest::checks().
    // For each field ending in _ID: "Field [name]_ID has corresponding table
    // keyword [name]."
    vector<String> checks(String table);

    // Implements TBTest::runCheck().
    bool runCheck(String table, int i);
};

// <summary>
// Tests whether subtables exist on disk.
// </summary>

class TBSubtablesTest : public TBTest {
public:
	// Constructor that takes the browser parent.
    TBSubtablesTest(TBBrowser* browser);

    ~TBSubtablesTest();

    // Implements TBTest::checks().
    // For each subtable in the keywords: "Subtable [name] exists on disk at 
    // [location]."
    vector<String> checks(String table);

    // Implements TBTest::runCheck().
    bool runCheck(String table, int i);
};

// <summary>
// Tests whether subtables can be opened and have data.
// </summary>

class TBValidSubtablesTest : public TBTest {
public:
	// Constructor that takes the browser parent.
    TBValidSubtablesTest(TBBrowser* browser);

    ~TBValidSubtablesTest();

    // Implements TBTest::checks().
    // For each subtable in the keywords: "Subtable [name] can be opened and
    // has at least one row of data."
    vector<String> checks(String table);

    // Implements TBTest::runCheck().
    bool runCheck(String table, int i);
};

}

#endif /* TBTEST_H_ */
