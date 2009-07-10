//# TBMain.qo.h: Main window which is mainly responsible for menu actions.
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
#ifndef TBMAIN_H_
#define TBMAIN_H_

#include <casaqt/QtBrowser/TBMain.ui.h>
#include <casaqt/QtBrowser/TBBrowser.qo.h>

//#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class TBBrowser;
class TBAction;
class DriverParams;
class TBOptions;

// <summary>
// Main window which is mainly responsible for menu actions.
// <summary>
//
// <synopsis>
// TBMain is a QMainWindow subclass that wraps about a TBBrowser object.  Its
// main responsibility is to provide menu commands and interact with the
// TBBrowser.
// </synopsis>

class TBMain : public QMainWindow, Ui::MainWindow {
    Q_OBJECT
    
public:
    // Default Constructor.
    TBMain();

    ~TBMain();

    
    // Opens a table with the given file name and driver parameters.  If the
    // driver parameters are NULL, the defaults are used.  If a start and num
    // are provided, those rows are loaded into the new table; otherwise
    // the default number of rows are loaded.
    void openTable(String filename, DriverParams* dp = NULL, int start = 0,
                   int num = TBConstants::DEFAULT_SELECT_NUM);

signals:
    // This signal is emitted when a close menu command is given with
    // a table name.
    void closeTableTriggered(String name);

protected:
    // Catches a close event for the window.  If a background task is running,
    // a confirmation dialog is presented to the user.  If the "save view on
    // close" option is turned on, the current view is saved to the default
    // location.
    void closeEvent(QCloseEvent* event);
    
private:
    // Browser object.
    TBBrowser browser;

    // The window's status bar.
    QStatusBar* statusBar;

    // Close menu that lists all currently opened tables.
    QMenu* closeMenu;

    // Columns menu that lists the table fields in the currently selected
    // table.
    QMenu* columnsMenu;

    // Edit menu that lists all currently opened tables.
    QMenu* editTablesMenu;

    // Indicates whether the main window is waiting for some other task to
    // complete or not.  If the window is waiting, menu commands will not
    // respond.
    bool waiting;

    // Separator added in the Edit menu before the Options command.
    QAction* separator;

    // Format menu that lists the table fields in the currently selected
    // table.
    QMenu* formatMenu;

    // Toolbar holding "file"-related menu commands.
    QToolBar* fileBar;

    // Toolbar holding "edit"-related menu commands.
    QToolBar* editBar;

    // Toolbar holding "view"-related menu commands.
    QToolBar* viewBar;

    // Toolbar holding "tool"-related menu commands.
    QToolBar* toolBar;

    // Indicates whether the TBMain should save the current view on close or
    // not.
    bool saveView;

    
    // Sets up GUI components and signal/slot connections.
    void setup();

    // Sets up the keyboard shortcuts.
    void setupKeyboardShortcuts();

    // Calls setEnabled() on all menus with the given parameter.
    void enableMenus(bool en);
    
    // Turns on/off editing, if write lock is available, on the given table.
    void editTable(TBTableTabs* table, bool edit);

private slots:
    // Opens a filechooser dialog to allow the user to open a table.
    void openTable();
    
    // Opens a TBTaQL dialog to allow the user to open a table using a TaQL
    // command.
    void openTaQL();
    
    // Connects with TBBrowser::tableOpened() signal.  Updates dynamic menus
    // as necessary.
    void tableOpened(String name);

    //void openTableWithOptions();
    
    // Closes the currently selected table.
    void closeTable();

    // Closes the table corresponding to the given action's name.
    void closeTable(QAction* which);

    // Closes all opened tables.
    void closeAll();
    
    // Closes all currently opened tables and closes the window.
    void closeAllAndExit();
    
    // Connects with TBBrowser::tableClosed() signal.  Updates dynamic menus
    // as necessary.
    void tableClosed(String name);
    
    // Slot for when the tab is changed in the browser.  Updates all necessary
    // menus.
    void tabChanged(int tableIndex);

    // Shows or hides the column on the current table that corresponds to
    // the name of the given action.
    void viewColumn(QAction* which);
    
    // Updates the column menu to indicate that the given column has been
    // shown/hidden.
    void columnHidden(int index, bool hidden);

    // Updates display to indicate the last action performed.
    void actionPerformed(TBAction* action);

    // Updates display to indicate that the last action was undone.
    void actionUndone();

    // Update display to indicate that the last undone action was redone.
    void actionRedone();
    
    // Undoes the last performed action.  See TBBrowser::undoAction().
    void undoAction();

    // Redoes the last undone action.  See TBBrowser::redoAction().
    void redoAction();

    // Enters the currently selected table into editing mode.
    void editCurrentTable();
    
    // Enters the table corresponding to the given edit action into editing
    // mode.
    void editTable(QAction* action);

    // Exports the current table to VOTable format.  See
    // TBBrowser::exportVOTable().
    void exportVOTable();

    // Reenables menus after a task thread has completed.
    void threadFinished();

    // Opens a new TBValidityChecker widget.
    void checkValidity();

    // Opens a new TBPlotter window.
    void plot2D();

    // Shows all the columns in the currently selected table.
    void showAllColumns();

    // Hides all the columns in the currently selected table.
    void hideAllColumns();

    // Opens widget to enter filter rules for the current table.  See
    // TBBrowser::filterOnFields().
    void filterOnFields();

    // Clears the current filter on the current table.  See
    // TBBrowser::clearFilterOnFields().
    void clearFilterOnFields();
    
    // Enables the "clear filter" action.
    void filterRuleAvailable(int i);
    
    // Disables the "clear filter" action.
    void filterRuleCleared(int i);

    // Opens widget to enter search rules for the current table.  See
    // TBBrowser::find().
    void find();

    // Searches for the next row matching the search rule for the current
    // table.  See TBBrowser::findNext().
    void findNext();

    // Searches for the previous row matching the search rule for the current
    // table.  See TBBrowser::findPrev().
    void findPrev();

    // Slot for when the current table has a find rule associated with it.
    void findRuleAvailable();
    
    // Slot for when a sort has been entered for the current table.
    void sortEntered();
    
    // Slot for when a sort has been cleared for the current table.
    void sortCleared();

    // Displays table information for the currently selected table.
    void viewTableInfo();

    // Formats the field with the name in the given QAction.
    void formatDisplay(QAction* which);

    // Opens the help window.  See TBHelp.
    void openHelp();

    // Shows an insert rows dialog.
    void insertRows();

    // Shows a delete rows dialog.
    void deleteRows();

    // Shows a sort dialog.  See TBSorter.
    void sort();
    
    // Clears the current sort.
    void clearSort();

    // Sort the current table with the given order.  See TBBrowser::sort().
    void sort(vector<pair<String, bool> >& sort);

    // Allows the user to edit table options.  See TBOptions.
    void options();

    // Applies the user-provided options and deletes the TBOptions widget.
    void saveOptions(TBOptions* opt);
};

}

#endif /* TBMAIN_H_ */
