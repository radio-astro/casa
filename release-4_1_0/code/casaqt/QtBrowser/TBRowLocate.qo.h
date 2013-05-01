//# TBRowLocate.qo.h: Displays results of a row locate.
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
#ifndef TBROWLOCATE_H_
#define TBROWLOCATE_H_

#include <casaqt/QtBrowser/TBRowLocate.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBLocatedRows;
class TBTableTabs;

// <summary>
// Displays results of a row locate.
// </summary>
//
// <synopsis>
// A TBRowLocate is basically a tab widget, where each tab holds a table that
// lists the given row data.  The TBRowLocate also has buttons to perform
// actions on the located rows.  Currently these actions include highlighting
// the row in the table browser.
// </synopsis>

class TBRowLocate : public QDialog, Ui::RowLocate {
    Q_OBJECT
    
public:
    // Constructor which takes the row data to display.
    TBRowLocate(TBLocatedRows* rows);
    
    ~TBRowLocate();
    
private:
    // Displayed row data.
    TBLocatedRows* rows;

    // List of tables.
    vector<TBTableTabs*> tables;
    
private slots:
    // Slot for when the user clicks a cell.
    void cellClicked(int row);
    
    // Slot for when the user clicks the "Highlight" button.
    void highlight();
    
    // Slot for when the user clicks the "Next" button.
    void next();
    
    // Slot for when the user changes the tab.
    void tabChanged(int newIndex);
};

}

#endif /* TBROWLOCATE_H_ */
