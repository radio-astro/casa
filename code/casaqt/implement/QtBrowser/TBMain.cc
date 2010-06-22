//# TBMain.cc: Main window which is mainly responsible for menu actions.
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
#include <casaqt/QtBrowser/TBMain.qo.h>

#include <casaqt/QtBrowser/TBBrowser.qo.h>
#include <casaqt/QtBrowser/TBAction.h>
#include <casaqt/QtBrowser/TBTableTabs.qo.h>
#include <casaqt/QtBrowser/TBValidityChecker.qo.h>
#include <casaqt/QtBrowser/TBPlotter.qo.h>
#include <casaqt/QtBrowser/TBFormat.qo.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBConnection.qo.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBHelp.qo.h>
#include <casaqt/QtBrowser/TBView.h>
#include <casaqt/QtBrowser/TBSorter.qo.h>
#include <casaqt/QtBrowser/TBOptions.qo.h>
#include <casaqt/QtBrowser/TBTaQL.qo.h>
#include <casaqt/PlotterImplementations/PlotterImplementations.h>
#include <casaqt/QtUtilities/QtFileDialog.qo.h>

namespace casa {

/////////////////////////
// TBMAIN DEFINITIONS //
/////////////////////////

// Constructors/Destructors //

TBMain::TBMain() : QMainWindow(), browser(), waiting(false),
                   separator(NULL), saveView(true) {
    setup();
    setupKeyboardShortcuts();
    show();    
    QCoreApplication::processEvents();
    QString myapp("Table Browser");
    QCoreApplication::setApplicationName(myapp);

    
    // Try loading the last saved view
    TBView* view = TBView::loadFromDefaultFile();
    if(view != NULL) {
        QtFileDialog::setNextDirectory(view->getLastOpenedDirectory().c_str());
        QtFileDialog::setHistoryLimit(view->chooserHistoryLimit());
        
        browser.showView(view);
        delete view;

        if(browser.isAvailable()) {
            tabChanged(browser.getTabWidget()->currentIndex());
            statusBar->showMessage("Opened saved view.");
        }
    }
}

TBMain::~TBMain() {
    browser.disconnect();
    delete statusBar;
    delete closeMenu;
    delete editTablesMenu;
    delete separator;
    delete formatMenu;
    delete fileBar;
    delete editBar;
    delete viewBar;
    delete toolBar;
}

// Public Methods //

void TBMain::openTable(String filename, DriverParams* dp, int start, int num) {
    if(filename.empty()) return;

    QCoreApplication::processEvents();
    browser.openTable(filename, dp, start, num);
}

// Protected Methods //

void TBMain::closeEvent(QCloseEvent* event) {
    bool accept = true;
    if(browser.openedTables() > 0 && browser.bgTaskIsRunning()) {
        QMessageBox::StandardButton response = QMessageBox::question(this,
            "Quit Confirmation",
            "There are tasks still running.  Do you really want to quit?",
            QMessageBox::Yes | QMessageBox::No);
        accept = response == QMessageBox::Yes;
    }

    if(accept) {
        if(saveView) {
            TBConstants::dprint(TBConstants::DEBUG_HIGH,
                                "Saving current view to default location.");
            TBView* view = browser.view();
            view->setLastOpenedDirectory(
                    QtFileDialog::lastDirectory().toStdString());
            view->setChooserHistoryLimit(QtFileDialog::historyLimit());
            view->saveToDefaultFile();
            delete view;
        }
        event->accept();
    }
    else event->ignore();
}

// Private Methods //

void TBMain::setup() {
    setupUi(this);
    
    // Connect widgets
    connect(&browser, SIGNAL(tabChanged(int)), this, SLOT(tabChanged(int)));
    connect(&browser, SIGNAL(actionPerformed(TBAction*)),
            this, SLOT(actionPerformed(TBAction*)));
    connect(&browser, SIGNAL(actionUndone()), this, SLOT(actionUndone()));
    connect(&browser, SIGNAL(actionRedone()), this, SLOT(actionRedone()));
    connect(&browser, SIGNAL(threadIsFinished()),this, SLOT(threadFinished()));
    connect(&browser, SIGNAL(findRuleAvailable()),
            this, SLOT(findRuleAvailable()));
    connect(&browser, SIGNAL(tableOpened(String)),
            this, SLOT(tableOpened(String)));
    connect(&browser, SIGNAL(tableClosed(String)),
            this, SLOT(tableClosed(String)));
    connect(&browser, SIGNAL(filterRuleAvailable(int)),
            this, SLOT(filterRuleAvailable(int)));
    connect(&browser, SIGNAL(filterRuleCleared(int)),
            this, SLOT(filterRuleCleared(int)));
    connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(actionCloseAndExit, SIGNAL(triggered()),
            this, SLOT(closeAllAndExit()));
    connect(actionOpenTable, SIGNAL(triggered()), this, SLOT(openTable()));
    //connect(actionOpenTableWithOptions, SIGNAL(triggered()),
    //        this, SLOT(openTableWithOptions()));
    connect(actionOpenTaQL, SIGNAL(triggered()), this, SLOT(openTaQL()));
    connect(actionCloseTable, SIGNAL(triggered()), this, SLOT(closeTable()));
    connect(actionCloseAll, SIGNAL(triggered()), this, SLOT(closeAll()));
    connect(actionUndo, SIGNAL(triggered()), this, SLOT(undoAction()));
    connect(actionRedo, SIGNAL(triggered()), this, SLOT(redoAction()));
    connect(actionExportVOTable, SIGNAL(triggered()),
            this, SLOT(exportVOTable()));
    connect(actionPlot2D, SIGNAL(triggered()), this, SLOT(plot2D()));
    connect(actionEditTable, SIGNAL(toggled(bool)), SLOT(editCurrentTable()));
    connect(actionCheckValidity, SIGNAL(triggered()),
            this, SLOT(checkValidity()));
    connect(actionFilter, SIGNAL(triggered()), this, SLOT(filterOnFields()));
    connect(actionClearFilter, SIGNAL(triggered()),
            this, SLOT(clearFilterOnFields()));
    connect(actionFind, SIGNAL(triggered()), this, SLOT(find()));
    connect(actionFindNext, SIGNAL(triggered()), this, SLOT(findNext()));
    connect(actionFindPrevious, SIGNAL(triggered()), this, SLOT(findPrev()));
    connect(actionTableInformation, SIGNAL(triggered()),
            this, SLOT(viewTableInfo()));
    connect(actionInsertRows, SIGNAL(triggered()), this, SLOT(insertRows()));
    connect(actionDeleteRows, SIGNAL(triggered()), this, SLOT(deleteRows()));
    connect(actionSort, SIGNAL(triggered()), this, SLOT(sort()));
    connect(actionOptions, SIGNAL(triggered()), this, SLOT(options()));
    connect(actionClearSort, SIGNAL(triggered()), this, SLOT(clearSort()));
    
    setCentralWidget(&browser);
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    
    // Set up menus and toolbars
    closeMenu = new QMenu("Close...");
    connect(closeMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(closeTable(QAction*)));
    columnsMenu = new QMenu("Columns");
    connect(columnsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(viewColumn(QAction*)));
    editTablesMenu = new QMenu("Edit Table...");
    connect(editTablesMenu, SIGNAL(triggered(QAction*)),
            SLOT(editTable(QAction*)));
    separator = menuEdit->insertSeparator(actionOptions);

    formatMenu = new QMenu("Format Display...");
    connect(formatMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(formatDisplay(QAction*)));

    connect(actionHelp, SIGNAL(triggered()), this, SLOT(openHelp()));

    fileBar = addToolBar("&File");
    fileBar->addAction(actionOpenTable);
    fileBar->addAction(actionCloseTable);

    editBar = addToolBar("&Edit");
    editBar->addAction(actionUndo);
    editBar->addAction(actionRedo);
    editBar->addAction(actionEditTable);

    viewBar = addToolBar("&View");
    viewBar->addAction(actionTableInformation);
    viewBar->addAction(actionFilter);
    
    toolBar = addToolBar("&Tools");
    toolBar->addAction(actionFind);
    toolBar->addAction(actionSort);
    toolBar->addAction(actionPlot2D);
    toolBar->addAction(actionHelp);

    // hide import menu until it's been implemented
    menuBar()->removeAction(menuImport->menuAction());
    // hide plot3d action until it's been implemented
    menuTools->removeAction(actionPlot3D);
}

void TBMain::setupKeyboardShortcuts() {
    actionOpenTable->setShortcut(QKeySequence("Ctrl+O"));
    actionOpenTaQL->setShortcut(QKeySequence("Ctrl+Shift+O"));
    //actionOpenTableWithOptions->setShortcut(QKeySequence("Ctrl+Shift+O"));
    actionCloseTable->setShortcut(QKeySequence("Ctrl+W"));
    actionCloseAll->setShortcut(QKeySequence("Ctrl+A"));
    actionUndo->setShortcut(QKeySequence("Ctrl+Z"));
    actionRedo->setShortcut(QKeySequence("Ctrl+Y"));
    (void) new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z,
            this, SLOT(redoAction()));
    actionEditTable->setShortcut(QKeySequence("Ctrl+E"));
    actionExit->setShortcut(QKeySequence("Ctrl+Q"));
    actionCloseAndExit->setShortcut(QKeySequence("Ctrl+Shift+Q"));
    actionPlot2D->setShortcut(QKeySequence("Ctrl+P"));
    actionFilter->setShortcut(QKeySequence("Ctrl+Shift+F"));
    actionClearFilter->setShortcut(QKeySequence("Ctrl+Shift+C"));
    actionSort->setShortcut(QKeySequence("Ctrl+S"));
    actionFind->setShortcut(QKeySequence("Ctrl+F"));
    actionFindNext->setShortcut(QKeySequence("F3"));
    actionFindPrevious->setShortcut(QKeySequence("Shift+F3"));
    actionTableInformation->setShortcut(QKeySequence("Ctrl+I"));
    actionHelp->setShortcut(QKeySequence("Ctrl+H"));
}

void TBMain::enableMenus(bool en) {
    actionCloseTable->setEnabled(en);
    actionCloseAll->setEnabled(en);
    actionEditTable->setEnabled(en);
    actionFilter->setEnabled(en);
    //actionClearFilter->setEnabled(en);
    actionExportVOTable->setEnabled(en);
    actionPlot2D->setEnabled(en);
    actionCheckValidity->setEnabled(en);
    actionFind->setEnabled(en);
    actionSort->setEnabled(en);
    actionTableInformation->setEnabled(en);

    if(!en) {
        menuFile->removeAction(closeMenu->menuAction());
        menuEdit->removeAction(editTablesMenu->menuAction());
        menuView->removeAction(columnsMenu->menuAction());
        menuView->removeAction(formatMenu->menuAction());
        actionUndo->setEnabled(false);
        actionUndo->setText("&Undo");
        actionRedo->setEnabled(false);
        actionRedo->setText("&Redo");
        actionFindNext->setEnabled(false);
        actionFindPrevious->setEnabled(false);
    }
}

void TBMain::editTable(TBTableTabs* ttabs, bool edit) {
    if(waiting || ttabs == NULL) return;
    
    TBTable* table = ttabs->getTable();
    bool editToggle = edit;
    
    if(edit && !table->canWrite()) {
        // if editing is turned on, get write lock
        bool tryAgain = true;
        while(tryAgain && !table->tryWriteLock()) {
            QMessageBox::StandardButton resp = QMessageBox::question(this,
                    "Write Lock Unavailable", "The write lock for this "
                    "table is current unavailable!  Would you like to try "
                    "again?", QMessageBox::Retry | QMessageBox::Abort);
            tryAgain = resp == QMessageBox::Retry;
        }
    }
    
    if(edit && !table->canWrite()) {
        // write lock did not succeed
        editToggle = false;
    } else if(!edit) {
        // we're not editing, so relinquish the write lock
        table->releaseWriteLock();
    }
    
    // toggle editing in table tabs
    ttabs->setEditable(editToggle);
    
    // update edit action
    String name = table->getName();
    QAction* a;
    for(int i = 0; i < editTablesMenu->actions().size(); i++) {
        a = editTablesMenu->actions().at(i);
        if(name == String(a->text().toStdString())) {
            a->blockSignals(true);
            a->setChecked(editToggle);
            a->blockSignals(false);
            break;
        }
    }
    
    // update the edit button if this is the current table tab
    if(ttabs == browser.currentlySelectedTableTabs()) {
        actionEditTable->blockSignals(true);
        actionEditTable->setChecked(editToggle);
        actionEditTable->blockSignals(false);
    }
}


// Private Slots //

void TBMain::openTable() {
    if(waiting) return;

    QString f = QtFileDialog::qgetExistingDir(this, tr("Find Data"));
    
    if(!f.isEmpty()) {
        openTable(qPrintable(f));
    }
}

void TBMain::openTaQL() {
    if(waiting) return;
    
    TBTaQL* taql = new TBTaQL(NULL, TBTaQL::SELECT);
    connect(taql, SIGNAL(command(String)), &browser, SLOT(openTaQL(String)));
    taql->exec();
}

/*
void TBMain::openTableWithOptions() {
    if(waiting) return;

    TBConnection* con = new TBConnection(this, this);
    con->setWindowModality(Qt::WindowModal);
    con->exec();
}
*/

void TBMain::tableOpened(String name) {
    statusBar->showMessage(tr(("Browsing table: " + name).c_str()));
    
    // add table to menus
    closeMenu->addAction(name.c_str());
    if(closeMenu->actions().size() == 1) {
        menuFile->insertMenu(actionCloseAll, closeMenu);
    }

    TBTableTabs* tt = browser.table(name);
    connect(tt->getDataTab(), SIGNAL(columnHidden(int, bool)),
            this, SLOT(columnHidden(int, bool)));
    
    QAction* a = editTablesMenu->addAction(name.c_str());
    a->setCheckable(true);
    a->setEnabled(tt->getTable()->isAnyEditable());
    if(editTablesMenu->actions().size() == 1)
        menuEdit->insertMenu(separator, editTablesMenu);
    
    enableMenus(true);
    actionClearFilter->setEnabled(browser.filterAvailable(browser.indexOf(
                                  browser.table(name))));
    
    TBDataTab* dt = tt->getDataTab();
    connect(dt, SIGNAL(sortEntered()), this, SLOT(sortEntered()));
    connect(dt, SIGNAL(sortCleared()), this, SLOT(sortCleared()));
    
    tabChanged(browser.getTabWidget()->currentIndex());
}

void TBMain::closeTable() {
    if(waiting) return;

    if(browser.openedTables() > 0) {
        browser.closeTable();
    }
}

void TBMain::closeTable(QAction* which) {
    if(waiting) return;

    String name = qPrintable(which->text());
    browser.closeTable(name);
}

void TBMain::closeAll() {
    if(waiting) return;

    while(browser.openedTables() > 0)
        closeTable();
}

void TBMain::closeAllAndExit() {
    bool accept = true;
    if(browser.bgTaskIsRunning()) {
        QMessageBox::StandardButton response = QMessageBox::question(this,
            "Quit Confirmation",
            "There are tasks still running.  Do you really want to quit?",
            QMessageBox::Yes | QMessageBox::No);
        accept = response == QMessageBox::Yes;
    }

    if(accept) {
        closeAll();
        close();
    }
}

void TBMain::tableClosed(String name) {
    statusBar->showMessage(tr(("Closed table: " + name).c_str()));
    
    for(int i = 0; i < closeMenu->actions().size(); i++) {
        QAction* a = closeMenu->actions().at(i);
        String n = qPrintable(a->text());
        if(n == name) {
            closeMenu->removeAction(a);
            break;
        }
    }
    
    for(int i = 0; i < editTablesMenu->actions().size(); i++) {
        QAction* a = editTablesMenu->actions().at(i);
        String n = qPrintable(a->text());
        if(n == name) {
            editTablesMenu->removeAction(a);
            break;
        }
    }

    if(browser.openedTables() == 0) {
        enableMenus(false);
    }
}

void TBMain::tabChanged(int tableIndex) {
    if(tableIndex < 0 || tableIndex >= browser.openedTables())
        return;

    TBTableTabs* tb = browser.tableAt(tableIndex);
    actionEditTable->setChecked(tb->isEditable());

    // update menus
    QStringList cols = browser.getColumnsAt(tableIndex);
    columnsMenu->clear();
    formatMenu->clear();
    for(int i = 0; i < cols.size(); i++) {
        QAction* a = columnsMenu->addAction(cols.at(i));
        a->setCheckable(true);
        a->setChecked(!browser.columnIsHidden(tableIndex, i));

        formatMenu->addAction(cols.at(i));
    }
    if(cols.size() > 0) {
        QAction* a = columnsMenu->addAction("&Show All Columns");
        connect(a, SIGNAL(triggered()), this, SLOT(showAllColumns()));
        a = columnsMenu->addAction("&Hide All Columns");
        connect(a, SIGNAL(triggered()), this, SLOT(hideAllColumns()));

        a = formatMenu->addAction("&Clear All Formats");
        connect(a, SIGNAL(triggered()),
                browser.currentlySelectedTableTabs()->getDataTab(),
                SLOT(clearAllFormats()));

        if(!menuView->actions().contains(columnsMenu->menuAction()))
            menuView->addMenu(columnsMenu);

        if(!menuView->actions().contains(formatMenu->menuAction()))
            menuView->addMenu(formatMenu);
    }

    bool b = browser.findRuleAvailable(tableIndex);
    actionFindNext->setEnabled(b);
    actionFindPrevious->setEnabled(b);

    actionInsertRows->setEnabled(tb->getTable()->canInsertRows());
    actionDeleteRows->setEnabled(tb->getTable()->canDeleteRows());
    
    actionClearFilter->setEnabled(browser.filterAvailable(tableIndex));
    
    vector<pair<String, bool> >* sorted = tb->getSortFields();
    actionClearSort->setEnabled(sorted != NULL && sorted->size() > 0);
    
    b = browser.currentlySelectedTableTabs()->getTable()->isAnyEditable();
    actionEditTable->setEnabled(b);
}

void TBMain::viewColumn(QAction* which) {
    if(waiting) return;

    QString name = which->text();
    for(int i = 0; i < columnsMenu->actions().size(); i++) {
        QString n = columnsMenu->actions().at(i)->text();
        if(n == name || (name == "Field" && n == "Field / Keyword")) {
            browser.setColumnHidden(i, !which->isChecked());
            break;
        }
    }
}

void TBMain::columnHidden(int index, bool hidden) {
    columnsMenu->actions().at(index)->setChecked(!hidden);
}

void TBMain::actionPerformed(TBAction* action) {
    actionUndo->setEnabled(true);
    actionRedo->setEnabled(false);
    actionUndo->setText(("&Undo: " + action->name()).c_str());
    actionRedo->setText("&Redo");
}

void TBMain::actionUndone() {
    if(browser.getNumActions() == 0) {
        actionUndo->setEnabled(false);
        actionUndo->setText("&Undo");
    }
    else {
        actionUndo->setText(("&Undo: " + browser.lastActionName()).c_str());
    }
    
    if(browser.getNumUndoneActions() > 0) {
        actionRedo->setEnabled(true);
       actionRedo->setText(("&Redo: "+browser.lastUndoneActionName()).c_str());
    }
}

void TBMain::actionRedone() {
    if(browser.getNumUndoneActions() == 0) {
        actionRedo->setEnabled(false);
        actionRedo->setText("&Redo");
    } else {
       actionRedo->setText(("&Redo: "+browser.lastUndoneActionName()).c_str());
    }
    
    if(browser.getNumActions() > 0) {
        actionUndo->setEnabled(true);
        actionUndo->setText(("&Undo: " + browser.lastActionName()).c_str());
    }
}

void TBMain::undoAction() {
    if(waiting) return;

    if(browser.getNumActions() > 0) {
        Result r = browser.undoAction();
        if(!r.valid)
            browser.displayError(r.result);
    }
}

void TBMain::redoAction() {
    if(waiting) return;

    if(browser.getNumUndoneActions() > 0) {
        Result r = browser.redoAction();
        if(!r.valid)
            browser.displayError(r.result);
    }
}

void TBMain::editCurrentTable() {
    if(waiting) return;

    editTable(browser.currentlySelectedTableTabs(),
              actionEditTable->isChecked());
}

void TBMain::editTable(QAction* action) {
    if(waiting) return;
    
    editTable(browser.table(action->text().toStdString()),
              action->isChecked());
}

void TBMain::exportVOTable() {
    if(waiting) return;

    String dir = qPrintable(QDir::currentPath());

    QString f = QFileDialog::getSaveFileName(this,"Save VOTable", dir.c_str());

    if(!f.isEmpty()) {
        String str = qPrintable(f);
        TBConstants::strtrim(str);

        menubar->setEnabled(false);
        fileBar->setEnabled(false);
        editBar->setEnabled(false);
        toolBar->setEnabled(false);
        viewBar->setEnabled(false);
        waiting = true;
        browser.exportVOTable(str);
    }
}

void TBMain::threadFinished() {
    menubar->setEnabled(true);
    fileBar->setEnabled(true);
    editBar->setEnabled(true);
    toolBar->setEnabled(true);
    viewBar->setEnabled(true);
    waiting = false;
}

void TBMain::checkValidity() {
    if(waiting) return;

    if(browser.openedTables() > 0) {
        TBValidityChecker* vc = new TBValidityChecker(&browser);
        vc->show();
    }
}

void TBMain::plot2D() {
    if(waiting) return;

    if(browser.openedTables() > 0) {
        TBPlotter* plotter = new TBPlotter(&browser, plotterImplementation(
                                 TBConstants::defaultPlotterImplementation));
        plotter->show();
    }
}

void TBMain::showAllColumns() {
    if(waiting) return;

    QList<QAction*> actions = columnsMenu->actions();
    for(int i = 0; i < actions.size() - 2; i++) {
        QAction* a = actions.at(i);
        if(a->isCheckable() && !a->isChecked()) {
            a->setChecked(true);
            viewColumn(a);
        }
    }
}

void TBMain::hideAllColumns() {
    if(waiting) return;

    QList<QAction*> actions = columnsMenu->actions();
    for(int i = 0; i < actions.size() - 2; i++) {
        QAction* a = actions.at(i);
        if(a->isCheckable() && a->isChecked()) {
            a->setChecked(false);
            viewColumn(a);
        }
    }
}

void TBMain::filterOnFields() {
    if(waiting) return;

    browser.filterOnFields();
}

void TBMain::clearFilterOnFields() {
    if(waiting) return;

    browser.clearFilterOnFields();
}

void TBMain::filterRuleAvailable(int i) {
    if(i == browser.getTabWidget()->currentIndex())
        actionClearFilter->setEnabled(true);
}

void TBMain::filterRuleCleared(int i) {
    if(i == browser.getTabWidget()->currentIndex())
        actionClearFilter->setEnabled(false);
}

void TBMain::find() {
    if(waiting) return;

    browser.find();
}

void TBMain::findNext() {
    if(waiting) return;

    browser.findNext();
}

void TBMain::findPrev() {
    if(waiting) return;

    browser.findPrev();
}

void TBMain::findRuleAvailable() {
    actionFindNext->setEnabled(true);
    actionFindPrevious->setEnabled(true);
}

void TBMain::sortEntered() {
    actionClearSort->setEnabled(true);
}

void TBMain::sortCleared() {
    actionClearSort->setEnabled(false);
}

void TBMain::viewTableInfo() {
    if(waiting) return;

    browser.viewTableInfo();
}

void TBMain::formatDisplay(QAction* which) {
    if(waiting) return;

    QString name = which->text();
    int i = 0;
    for(; i < formatMenu->actions().size() - 1; i++) {
        QString n = formatMenu->actions().at(i)->text();
        if(n == name) break;
    }

    if(i == formatMenu->actions().size() - 1) return;
    else browser.formatDisplay(i);
}

void TBMain::openHelp() {
    if(waiting) return;

    TBHelp* help = new TBHelp();
    help->show();
}

void TBMain::insertRows() {
    bool ok;
    int n = QInputDialog::getInteger(this, "Insert Rows",
            "How many rows do you want to insert at the end of the table?", 1,
            1, 100, 1, &ok);
    while(ok && n < 1) {
        browser.displayError("Number of rows must be greater than 0.");
        n = QInputDialog::getInteger(this, "Insert Rows",
                "How many rows do you want to insert at the end of the table?",
                1, 1, 100, 1, &ok);
    }

    if(ok) {
        browser.currentlySelectedTableTabs()->insertRows(n);
    }
}

void TBMain::deleteRows() {
    bool ok;
    String message = "Enter comma-separated row indicies to delete:\n";
    message += "(Example: 1-5, 7, 10 - 14)";
    String text = qPrintable(QInputDialog::getText(this, "Delete Rows",
                    message.c_str(), QLineEdit::Normal, "", &ok));
    TBConstants::strtrim(text);

    if(ok) {
        String msg = "WARNING: Row deletion cannot be undone.  Are you sure";
        msg += " you want to continue?";
        QMessageBox::StandardButton response = QMessageBox::question(this,
                    "Deletion Confirmation", msg.c_str(),
                    QMessageBox::Yes | QMessageBox::No);
        if(response != QMessageBox::Yes) return;
        
        int totalRows = browser.currentlySelectedTable()->getTotalRows();
        vector<int> rows;

        // Parse the row numbers from the text
        unsigned int i = 0, j = text.find(','), k = 0;
        int n, m, temp;
        bool valid = true;
        while(i < text.length()) {
            String str = text.substr(i, j - i);
            TBConstants::strtrim(str);

            k = str.find('-');
            if(k >= str.length()) { // one number
                temp = TBConstants::atoi(str, &n);
                if(temp != 1) valid = false;
                if(temp >= 1) {
                    if(n >= 0 && n < totalRows) rows.push_back(n);
                    else valid = false;
                }
            } else { // range
                temp = sscanf(str.c_str(), "%d - %d", &n, &m);
                if(temp != 2) valid = false;
                if(temp >= 2) {
                    if(n > m) {
                        temp = n;
                        n = m;
                        m = temp;
                    }
                    for(int a = n; a <= m; a++) {
                        if(a >= 0 && a < totalRows) rows.push_back(a);
                        else valid = false;
                    }
                }
            }
            
            i = j;
            if(i < text.length()) j = text.find(',', ++i);
        }

        if(!text.empty() && !valid) {
            String msg= "Invalid format; one or more entered row indices may ";
            msg += "not have been deleted";
            QMessageBox::warning(this, "Delete Rows", msg.c_str());
        }
        
        browser.currentlySelectedTableTabs()->deleteRows(rows);
    }
}

void TBMain::sort() {
    TBTableTabs* tt = browser.currentlySelectedTableTabs();
    TBTable* table = browser.currentlySelectedTable();
    if(table == NULL) return;
    
    vector<TBField*>* fields = table->getFields();
    if(fields == NULL || fields->size() == 0) return;

    vector<String> f(fields->size());
    for(unsigned int i = 0; i < fields->size(); i++) {
        f[i] = fields->at(i)->getName();
    }
    TBSorter* sorter = new TBSorter(f, tt->getSortFields(), this);
    connect(sorter, SIGNAL(sortEntered(vector<pair<String, bool> >&)),
            this, SLOT(sort(vector<pair<String, bool> >&)));
    sorter->setWindowModality(Qt::WindowModal);
    sorter->exec();
}

void TBMain::clearSort() {
    browser.currentlySelectedTableTabs()->clearSort();
}

void TBMain::sort(vector<pair<String, bool> >& sort) {
    browser.sort(sort);
}

void TBMain::options() {
    TBOptions* options = new TBOptions();
    options->setSaveView(saveView);
    options->setDebugLevel(TBConstants::debugThreshold);
    options->setChooserHistoryLimit(QtFileDialog::historyLimit());
    connect(options, SIGNAL(saveOptions(TBOptions*)),
            this, SLOT(saveOptions(TBOptions*)));
    options->setWindowModality(Qt::WindowModal);
    options->exec();
}

void TBMain::saveOptions(TBOptions* opt) {
    if(opt == NULL) return;

    TBConstants::debugThreshold = opt->debugLevel();
    saveView = opt->saveView();
    QtFileDialog::setHistoryLimit(opt->chooserHistoryLimit());
    if(!saveView) {
        // if a view file exists, delete it
        String viewLoc = TBView::defaultFile();
        if(remove(viewLoc.c_str()) == 0) {
            TBConstants::dprint(TBConstants::DEBUG_MED,
                    "Old saved view was deleted.");
        }
    }
    
    delete opt;
}

}
