//# TBBrowser.qo.h: Browser widget for managing opened tables.
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
#ifndef TBBROWSER_H_
#define TBBROWSER_H_

#include <casaqt/QtBrowser/TBBrowser.ui.h>
#include <casaqt/QtBrowser/TBAction.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBTableTabs;
class QProgressPanel;
class ProgressHelper;
class TBExportThread;
class TBTable;
class TBFilterRuleSequence;
class TBFilterRules;
class DriverParams;
class TBView;

// <summary>
// Browser widget for managing opened tables.
// <summary>
//
// <synopsis>
// TBBrowser is a container for multiple TBTableTabs.  Its main responsibility
// is to keep these tabs up-to-date and handle their opening and closing.
// TBBrowser is the main interface for the GUI of the table browser and thus
// also provides public methods to operate on opened tables.
// </synopsis>

class TBBrowser : public QWidget, Ui::Browser {
    Q_OBJECT
    
public:
    // Default Constructor.  Sets up GUI with no tabs or opened tables.
    TBBrowser();

    ~TBBrowser();
    
    
    /* Accessors/Mutators */

    // Returns true if the browser is available, false otherwise.
    bool isAvailable();

    // Returns the number of tables that are currently opened.
    int openedTables();

    // Returns the widget managing the tabs (QTabWidget).
    QTabWidget* getTabWidget();

    // Returns the number of performed actions.  See TBActionList::size().
    int getNumActions();

    // Returns the number of undone actions.  See TBActionList::undoneSize().
    int getNumUndoneActions();

    // Returns the name of the last performed action, or blank if there is
    // none.  See TBActionList::lastActionName().
    String lastActionName();

    // Returns the name of the last undone action, or blank if there is none.
    // See TBActionList::lastUndoneActionName().
    String lastUndoneActionName();
    
    
    /* Table Access Methods */

    // Returns the name of the currently selected table, or blank if there is
    // none.
    String currentlySelectedTableName();
    
    // Returns the currently selected table.
    TBTable* currentlySelectedTable();
    
    // Returns the currently selected table tab.
    TBTableTabs* currentlySelectedTableTabs();

    // Returns the names of all opened tables.
    vector<String> openedTableNames();
    
    // Returns the TBTableTabs for the table with the given name.
    TBTableTabs* table(String name);
    
    // Returns the index of the indicated table tabs, or -1 if the pointer is
    // invalid.
    int indexOf(TBTableTabs* tt);
    
    // Return the table at index i, or NULL if the index is invalid.
    TBTableTabs* tableAt(int i);
    
    
    /* Table Operation Methods */
    
    // Opens a table tab from the given file, with the given parameters.  Loads
    // rows from start to (start + num).  If the DriverParameters are NULL,
    // the default is used.
    int openTable(String filename, DriverParams* dp = NULL, int start = 0,
                  int num = TBConstants::DEFAULT_SELECT_NUM);
    
    // Opens a table tab from the given file, with the given parameters.  Loads
    // rows from start to (start + num).  If the DriverParameters are NULL,
    // the default is used.
    int openTable(String filename, bool taql, DriverParams* dp = NULL,
                  int start = 0, int num = TBConstants::DEFAULT_SELECT_NUM);
    
    // Closes the currently selected table and returns its name.
    String closeTable();

    // Closes the table at the given tab index and returns its name.
    String closeTable(int index);
    
    // Closes the table with the given name and returns whether the close
    // succeeded or not.
    bool closeTable(String name);

    // Returns the column (field) names for the table at the given index.
    QStringList getColumnsAt(unsigned int index);

    // Returns true if the indicated column on the indicated tab is hidden,
    // false otherwise.
    bool columnIsHidden(unsigned int index, int col);

    // Sets whether the indicated column on the currently selected tab is
    // hidden.
    void setColumnHidden(int col, bool hidden);
    
    // Sets whether the indicated column on the indicated tab is hidden.
    void setColumnHidden(unsigned int index, int col, bool hidden);
    
    // Exports the currently selected table to VOTable XML format to the given
    // filename.  See TBTable::exportVOTable().
    void exportVOTable(String file);
    
    // Returns true if the table at the given index has a find rule entered,
    // false otherwise.
    bool findRuleAvailable(int index);
    
    // Shows a find dialog, and connects the signal to run the find rules as
    // indicated.
    void find();

    // If the currently selected table has a find rule, finds the next row that
    // passes the rule.  The second parameter is used to control the
    // wrap-around search such that the search will go through each row exactly
    // once.
    void findNext(bool second = false);

    // If the currently selected table has a find rule, finds the previous row
    // that passes the rule.  The second parameter is used to control the
    // wrap-around search such that the search will go through each row exactly
    // once.
    void findPrev(bool second = false);

    // Returns true if the table at the given index has a filter, false
    // otherwise.
    bool filterAvailable(int index);

    // Shows a filter rules dialog in the side panel of the currently selected
    // table and connects its signal such that when the user clicks the
    // "Filter" button, the filter is run on the table.
    void filterOnFields();

    // Clears the filter on the currently selected table.
    void clearFilterOnFields();
    
    // Returns the filter for the table at the indicated index, or NULL if
    // there is none.
    TBFilterRuleSequence* filterAt(int index);
    
    // Displays a format dialog for the currently selected table for the field
    // at index i, and connects the signal to format the display accordingly.
    void formatDisplay(int i);

    // Shows a table information panel in the side panel of the currently
    // selected table.
    void viewTableInfo();
    
    // Sort the currently selected table by the indicated fields.
    void sort(vector<pair<String, bool> >& s);

    // Sort the table at index i with the fields in s.  Each entry in s should
    // be a field name paired with a bool indicating whether the sort order is
    // ascending.  See TBTableTabs::sort().
    void sort(int i, vector<pair<String, bool> >& s);

    
    /* Action Methods */
    
    // Does the specified action and returns the result.  See
    // TBActionList::doAction().
    Result doAction(TBAction* action);

    // Undoes the last performed action and returns the result.  See
    // TBActionList::undoAction().
    Result undoAction();

    // Redoes the last undone action and returns the result.  See
    // TBActionList::redoAction().
    Result redoAction();


    /* Progress Methods */

    // Adds a QProgressPanel to the front of the browser and dims the GUI
    // behind the progress panel, then returns the QProgressPanel.
    QProgressPanel* addProgressPanel(String label, bool hideable,
                                     bool cancelable);

    // Removes the given QProgressPanel from the browser and restores it to
    // its fully-enabled state.
    void removeProgressPanel(QProgressPanel* panel);

    // Shows the progress frame on the bottom of the browser and returns a
    // ProgressHelper encapsulating it.
    ProgressHelper* enableProgressFrame(String label);

    // Hides the progress frame on the bottom of the browser.
    void disableProgressFrame();
    
    
    /* Miscellaneous Browser Methods */

    // Returns true if a background task is currently running, false otherwise.
    bool bgTaskIsRunning();

    // Returns the current view of the browser.  See TBView.
    TBView* view();

    // Shows the given view in the browser.  See TBView.
    void showView(TBView* view);

public slots:
    // Displays the given error in a dialog to the user.
    void displayError(String message);
    
    // Opens a TaQL table with the given command.  See openTable().
    void openTaQL(String command);

    // Opens the subtable located at the given filename, and load the rows such
    // that row r can be selected.
    void followReference(String subtable, int r);

    // Removes all actions associated with the given widget.
    void removeActionsAssociatedWithWidget(QWidget* widget);

    // Removes all actions associated with the given object.
    void removeActionsAssociatedWith(void* widget);

signals:
    // This signal is emitted when a new table is opened and its tab added.
    // The String parameter holds the name of the newly opened table.
    void tableOpened(String name, String fullpath);

    // This signal is emitted when a table is closed and its tab removed.
    // The String parameter holds the name of the table that was closed.
    void tableClosed(String name);

    // This signal is emitted whenever an action is performed.  The
    // TBAction parameter points to the action that was performed.
    void actionPerformed(TBAction* action);

    // This signal is emitted whenever the latest action was undone.
    void actionUndone();

    // This signal is emitted whenever the latest undone action was redone.
    void actionRedone();

    // This signal is emitted whenever a task thread has completed.
    void threadIsFinished();

    // This signal is emitted whenever the user enters a find rule for the
    // currently selected table.
    void findRuleAvailable();
    
    // This signal is emitted whenever the user enters a filter rule for the
    // table with the given index.
    void filterRuleAvailable(int index);
    
    // This signal is emitted whenever the user clears the filter rule for the
    // table with the given index.
    void filterRuleCleared(int index);

    // This signal is emitted whenever the tab is changed.  The int parameter
    // indicates the new current tab index.
    void tabChanged(int tableIndex);
    
private:
    // Indicates whether the browser is available.
    bool available;

    // Holds the opened tables.
    vector<TBTableTabs*> tables;

    // Error dialog.
    QErrorMessage* errorDialog;

    // Actions, both performed and undone.
    TBActionList actions;

    // The current progress panel, or NULL if there is none.
    QProgressPanel* progressPanel;

    // The current export thread, or NULL if there is none.
    TBExportThread* exportThread;

    // Find rules for all opened tables.
    vector<TBFilterRuleSequence*> currentFindRules;

    // Filters for all opened tables.
    vector<TBFilterRuleSequence*> currentFilters;

    // Indicates whether a background task is currently running or not.
    bool bgTaskRunning;

    // Indicates whether an export thread is both active and hidden or not.
    bool exportHidden;

    
    // Enable or disable the widgets based upon availability.
    void updateEnabled();
    
    // Adds a table tab from the given file, with the given parameters.  Loads
    // rows from start to (start + num).  If the DriverParameters are NULL,
    // the default is used.
    int addTable(String filename, bool taql, DriverParams* dp, int start,
                  int num);
    
private slots:
    // Slot to close the currently opened table.
    void doCloseTable();

    // Slot for when a task thread has finished.
    void threadFinished();

    // Slot for hiding a task thread in the background.
    void hideThread();

    // Slot for when the user has entered a filter rule.  Applies the given
    // TBFilterRulesSequence to the currently selected table.
    void filterRulesEntered(TBFilterRuleSequence* rules);

    // Slot for when the user has entered a search rule.  Searches the
    // currently selected table for a row that passes and, if one is found
    // on the currently loaded pages, selects that row.
    void findRulesEntered(TBFilterRuleSequence* rules, TBFilterRules* rDialog);

    // Slot for when the QTabWidget detects a change in tab.  Emits the
    // tabChanged() signal.
    void tabChange(int index);
};

}

#endif /* TBBROWSER_H_*/
