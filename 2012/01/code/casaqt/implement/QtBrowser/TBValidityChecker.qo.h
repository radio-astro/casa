//# TBValidityChecker.qo.h: Widget for running TBTests on opened tables.
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
#ifndef TBVALIDITYCHECKER_H_
#define TBVALIDITYCHECKER_H_

#include <casaqt/QtBrowser/TBValidityChecker.ui.h>

#include <casa/BasicSL/String.h>

#include <vector>

#include <QtGui>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBBrowser;
class TBTest;

// <summary>
// Widget for running TBTests on opened tables.
// </summary>
//
// <synopsis>
// A TBValidityChecker allows the user to select a table and then run any
// defined TBTests on it.  The pass/fail results are then shown to the user.
// </synopsis>

class TBValidityChecker : public QWidget, Ui::ValidityChecker {
    Q_OBJECT
    
public:
	// Constructor that takes the browser parent.
    TBValidityChecker(TBBrowser* browser);

    ~TBValidityChecker();

private:
	// Browser parent.
    TBBrowser* browser;
    
    // List of tests that can be run.
    vector<TBTest*> tests;

    
    // Sets up the tests in the GUI.
    void setupTests();

private slots:
	// Slot for when the user selects a new table.
    void tableChanged(QString newTable);

    // Slot for when the user selects a new test.  Loads the checks in the
    // right side of the widget.
    void loadTest(int index);

    // Slot to run all checks in the selected test on the selected table.
    void runChecks();

    // Slot to run the selected check in the selected test on the selected
    // table.
    void runCheck();

    // Slot for when a new table is opened in the browser.  Adds the name to
    // the list of opened tables.
    void tableOpened(String table);

    // Slot for when a table is closed in the browser.  Removes the name from
    // the list of opened tables. 
    void tableClosed(String table);
};

}

#endif /* TBVALIDITYCHECKER_H_ */
