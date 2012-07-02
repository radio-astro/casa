//# TBTableTabs.qo.h: Collection of table backend and display tabs.
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
#ifndef TBTABLETABS_H_
#define TBTABLETABS_H_

#include <casaqt/QtBrowser/TBTableTabs.ui.h>

#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBDataTab.qo.h>
#include <casaqt/QtBrowser/TBKeywordsTab.qo.h>
#include <casaqt/QtBrowser/TBAction.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class TBBrowser;
class DriverParams;

// <summary>
// Collection of table backend and display tabs.
// </summary>
//
// <synopsis>
// TBTableTabs keeps track of the underlying TBTable object as well as the
// three tabs associated with its view (data, table keywords, and field
// keywords).  It is the unifying interface over the GUI and non-GUI aspects
// of a table.  Methods called on a TBTableTabs object tend to update the
// backend via the TBTable and then update the GUIs with the new data.
// </synopsis>

class TBTableTabs : public QWidget, Ui::TableTabs {
    Q_OBJECT
    
public:
    // Constructor that takes the browser parent, the location of the table,
    // the driver parameters for opening the table, and whether the table is
    // from a TaQL command or not.
    TBTableTabs(TBBrowser* b, String filename, DriverParams* dp, bool taql);

    ~TBTableTabs();
    

    // Returns true if the table is available, false otherwise.
    bool isAvailable();
    
    // Returns true if the table is currently in editing mode, false otherwise.
    bool isEditable();

    // Returns the "name" of this table (which is the last part of the
    // filename).
    String getName();
    
    // Returns the location of this table.
    String getFileName();

    // Returns the browser associated with this table.
    TBBrowser* getBrowser();

    // Returns the data tab associated with this table.
    TBDataTab* getDataTab();
    
    // Returns the table keywords tab associated with this table.
    TBTableKeywordsTab* getKeywordsTab();
    
    // Returns the field keywords tab assicated with this table.
    TBFieldKeywordsTab* getFieldKeywordsTab();

    // Returns the QTableWidget used to display table data.
    QTableWidget* getTableWidget();
    
    // Returns the QTableWidget used to display table keywords.
    QTableWidget* getTableKeywords();
    
    // Returns the QTreeWidget used to display the field keywords.
    QTreeWidget* getFieldKeywords();

    // Returns the underlying table backend.
    TBTable* getTable();
    
    // Returns the current sort on the table data tab, or an empty list
    // if there is none.
    vector<pair<String, bool> >* getSortFields();
    

    // Loads rows into the table backend and, if successful, updates the three
    // GUI displays to reflect the new table data.  Returns whether the
    // loading was successful or not.  If the loading was not successful an
    // error message is displayed.
    bool loadRows(int start = 0, int num = TBConstants::DEFAULT_SELECT_NUM);

    // If the table allows for insertion of rows, the given number of rows
    // is inserted at the end of the table and, if successful, the GUI display
    // is updated accordingly.  If the insertion was not successful an error
    // message is displayed.
    void insertRows(int n);

    // Deletes the given rows from the table, if the table allows for deletion
    // of rows.  Each element in the vector should be a row number.  If
    // successful, the GUI display is updated; otherwise an error message is
    // displayed.
    void deleteRows(vector<int> r);

    // Clears the current sort on the table data tab.
    void clearSort();

    // Sorts the table data tab with the given sort order.  See
    // TBDataTab::sortBy().
    void sort(vector<pair<String, bool> >& s);

    // Selects and highlights the given row in the data tab.  If the given
    // row is not loaded in the table, the user is prompted on whether they
    // want to load the appropriate page or not.
    void highlight(int row);
    
public slots:
    // Updates the underlying table with the new data and then updates the
    // GUI displays if successful.  The new value is first checked for
    // validity.  If the update fails, an error message is displayed.
    void dataChanged(int row, int col, String newVal);

    // Sets whether this table is currently in editing mode or not.
    void setEditable(bool e);

    // Shows the given widget in the side panel on the table data tab.
    void showWidgetInTableSplitter(QWidget* widget, bool isArray = false);

    // Shows the given widget in the side panel on the table keywords tab.
    void showWidgetInKeywordSplitter(QWidget* widget);

    // Shows the given widget in the side panel on the field keywords tab.
    void showWidgetInFieldKeywordSplitter(QWidget* widget);

    // Shows the given widget in the side panel of the tab that is currently
    // selected.
    void showWidgetInCurrentSplitter(QWidget* widget);

    // Clears the side panel on the table data tab.
    void clearWidgetInTableSplitter();

    // Clears the side panel on the table keywords tab.
    void clearWidgetInKeywordSplitter();

    // Clears the side panel on the field keywords tab.
    void clearWidgetInFieldKeywordSplitter();

    // Clears the side panel on the currently selected tab.
    void clearWidgetInCurrentSplitter();
    
private:
    // Indicates whether this table is currently available.
    bool available;
    
    // Indicates whether this table is currently in editing mode or not.
    bool editable;
    
    // This table's location.
    String filename;
    
    // This table's name.
    String name;
    
    // Table backend.
    TBTable table;
    
    // Table data tab.
    TBDataTab dataTab;
    
    // Table keywords tab.
    TBTableKeywordsTab keywordsTab;
    
    // Field keywords tab.
    TBFieldKeywordsTab fieldKeywordsTab;
    
    // Reference to parent.
    TBBrowser* browser;
};

}

#endif /* TBTABLETABS_H_ */
