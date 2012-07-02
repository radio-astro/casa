//# TBBrowser.cc: Browser widget for managing opened tables.
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
#include <casaqt/QtBrowser/TBBrowser.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBValidityChecker.qo.h>
#include <casaqt/QtBrowser/TBTableTabs.qo.h>
#include <casaqt/QtBrowser/TBDataTab.qo.h>
#include <casaqt/QtBrowser/TBKeywordsTab.qo.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBAction.h>
#include <casaqt/QtBrowser/QProgressPanel.qo.h>
#include <casaqt/QtBrowser/TBExportThread.h>
#include <casaqt/QtBrowser/TBViewArray.qo.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBFilterRules.qo.h>
#include <casaqt/QtBrowser/TBTableInfo.qo.h>
#include <casaqt/QtBrowser/TBKeyword.h>
#include <casaqt/QtBrowser/TBFormat.qo.h>
#include <casaqt/QtBrowser/TBView.h>

#include <sstream>

namespace casa {

////////////////////////////
// TBBROWSER DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

TBBrowser::TBBrowser() : QWidget(), available(true), tables(),
                         actions(), progressPanel(NULL), bgTaskRunning(false),
                         exportHidden(false) {
    setupUi(this);
    errorDialog = new QErrorMessage(this);
    errorDialog->setWindowTitle("Table Browser Error");
    tabWidget->removeTab(0);
    connect(tabWidget, SIGNAL(currentChanged(int)),this, SLOT(tabChange(int)));
    disableProgressFrame();
    updateEnabled();
    
    // Set up close tab button
    QToolButton* b = new QToolButton(this);
    b->setIcon(QIcon(":/images/close.png"));
    tabWidget->setCornerWidget(b);
    connect(b, SIGNAL(clicked()), this, SLOT(doCloseTable()));
}

TBBrowser::~TBBrowser() {
    for(unsigned int i = 0; i < tables.size(); i++)
        delete tables.at(i);
    
    delete errorDialog;
    if(progressPanel != NULL) delete progressPanel;

    /*
    if(exportThread != NULL) {
        exportThread->terminate();
        //delete exportThread;
    }
    */
     
    for(unsigned int i = 0; i < currentFindRules.size(); i++)
        if(currentFindRules.at(i) != NULL) delete currentFindRules.at(i);
    for(unsigned int i = 0; i < currentFilters.size(); i++)
        if(currentFilters.at(i) != NULL) delete currentFilters.at(i);
}

// Public Methods //

/* Accessors/Mutators */

bool TBBrowser::isAvailable() { return available; }

int TBBrowser::openedTables() { return tables.size(); }

QTabWidget* TBBrowser::getTabWidget() { return tabWidget; }

int TBBrowser::getNumActions() { return actions.size(); }

int TBBrowser::getNumUndoneActions() { return actions.undoneSize(); }

String TBBrowser::lastActionName() {
    if(actions.size() > 0) return actions.lastActionName();
    else return "";
}

String TBBrowser::lastUndoneActionName() {
    if(actions.undoneSize() > 0) return actions.lastUndoneActionName();
    else return "";
}


/* Table Access Methods */

String TBBrowser::currentlySelectedTableName() {
    if(tables.size() > 0)
        return tables.at(tabWidget->currentIndex())->getName();
    else return "";
}

TBTable* TBBrowser::currentlySelectedTable() {
    if(tables.size() > 0) {
        return tables.at(tabWidget->currentIndex())->getTable();
    } else return NULL;
}

TBTableTabs* TBBrowser::currentlySelectedTableTabs() {
    if(tables.size() > 0) {
        return tables.at(tabWidget->currentIndex());
    } else return NULL;
}

vector<String> TBBrowser::openedTableNames() {
    vector<String> tbls;
    for(unsigned int i = 0; i < tables.size(); i++) {
        tbls.push_back(tables.at(i)->getName());
    }
    return tbls;
}

TBTableTabs* TBBrowser::table(String name) {
    if(available) {
        for(unsigned int i = 0; i < tables.size(); i++) {
            if(tables.at(i)->getName() == name) {
                return tables.at(i);
            }
        }
    }
    return NULL;
}

int TBBrowser::indexOf(TBTableTabs* tt) {
    for(unsigned int i = 0; i < tables.size(); i++) {
        if(tables.at(i) == tt) return i;
    }
    return -1;
}

TBTableTabs* TBBrowser::tableAt(int index) {
    if(index >= 0 && index < (int)tables.size()) {
        return tables.at(index);
    } else return NULL;
}


/* Table Operation Methods */

int TBBrowser::openTable(String f, DriverParams* dp, int s, int n) {
    return addTable(f, false, dp, s, n);
}

int TBBrowser::openTable(String f, bool t, DriverParams* dp, int s, int n) {
    return addTable(f, t, dp, s, n);
}

String TBBrowser::closeTable() {
    if(available) {
        if(tabWidget->currentIndex() < 0) return "";
        return closeTable(tabWidget->currentIndex());
    } else return "";
}

String TBBrowser::closeTable(int n) {
    if(available && 0 <= n && n < tabWidget->count()) {
        tabWidget->removeTab(n);

        TBTableTabs* tt = tables.at(n);
        tables.erase(tables.begin() + n);
        currentFindRules.erase(currentFindRules.begin() + n);
        currentFilters.erase(currentFilters.begin() + n);
        String name = String(tt->getName());

        removeActionsAssociatedWith(tt);
        
        delete tt;

        if(tables.size() == 0) available = false;

        updateEnabled();
        emit tableClosed(name);
        return name;
    } else return "";
}

bool TBBrowser::closeTable(String name) {
    if(available) {
        for(unsigned int i = 0; i < tables.size(); i++) {
            if(tables.at(i)->getName() == name) {
                return closeTable(i) != "";
            }
        }
    }
    return false;
}

QStringList TBBrowser::getColumnsAt(unsigned int index) {
    QStringList cols;
    if(available && 0 <= index && index < tables.size()) {
        vector<String> fields=tables.at(index)->getTable()->getColumnHeaders();
        for(unsigned int i = 0; i < fields.size(); i++) {
            cols << fields.at(i).c_str();
        }
    }
    return cols;
}

bool TBBrowser::columnIsHidden(unsigned int index, int col) {
    if(available && 0 <= index && index < tables.size())
        return tables.at(index)->getTableWidget()->isColumnHidden(col);
    else return false;
}

void TBBrowser::setColumnHidden(unsigned int index, int col, bool hidden) {
    if(available && 0 <= index && index < tables.size()) {
        tables.at(index)->getDataTab()->hideColumn(col, hidden);
    }
}

void TBBrowser::setColumnHidden(int col, bool hidden) {
    setColumnHidden(tabWidget->currentIndex(), col, hidden);
}

void TBBrowser::exportVOTable(String file) {
    if(tables.size() > 0) {
        // Do the work in its own thread
        progressPanel = addProgressPanel("Exporting to VOTable...",
                                         true, false);
        connect(progressPanel,SIGNAL(hideRequested()),
                this, SLOT(hideThread()));
        
        exportThread = new TBExportThread(tables.at(
                                tabWidget->currentIndex())->getTable(), file,
                                new ProgressHelper(progressPanel));
        connect(exportThread, SIGNAL(finished()), this,SLOT(threadFinished()));

        exportThread->start();
    }
}

bool TBBrowser::findRuleAvailable(int index) {
    if(index < 0 || index >= tabWidget->count()) return false;
    else return currentFindRules.at(index) != NULL;
}

void TBBrowser::find() {
    TBFilterRules* findDialog = new TBFilterRules(currentlySelectedTable(),
                                                  this);
    findDialog->renameForSearch(true);
    connect(findDialog, SIGNAL(runRequested(
            TBFilterRuleSequence*, TBFilterRules*)),
            this,SLOT(findRulesEntered(TBFilterRuleSequence*,TBFilterRules*)));
    findDialog->setWindowModality(Qt::WindowModal);
    findDialog->exec();
}

void TBBrowser::findNext(bool second) {
    int i = tabWidget->currentIndex();

    TBFilterRuleSequence* rules = currentFindRules.at(i);
    if(rules == NULL) {
        find();
        return;
    }

    QTableWidget* tw = tables.at(i)->getTableWidget();
    TBDataTab* dt = tables.at(i)->getDataTab();
    TBTable* tb = tables.at(i)->getTable();
    
    // set up the starting row number
    if(!second && tw->currentRow() > -1) i = tw->currentRow() + 1;
    else i = 0;
    
    for(; i < tw->rowCount(); i++) {
        int j = rules->rowPasses(tb, dt->logicalIndex(i));
        if(j > -1) {
            tw->setCurrentCell(i, j);
            break;
        }
    }

    // didn't find anything
    if(i == tw->rowCount()) {
        if(second) {
            QMessageBox::warning(this, "Find",
                     "Could not find any matching rows on this page.");
        } else if(QMessageBox::question(this, "Find",
              "Reached end of page.  Continue search at top?",
              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            findNext(true);            
        }
    }
}

void TBBrowser::findPrev(bool second) {
    int i = tabWidget->currentIndex();

    TBFilterRuleSequence* rules = currentFindRules.at(i);
    if(rules == NULL) {
        find();
        return;
    }

    QTableWidget* tw = tables.at(i)->getTableWidget();
        TBDataTab* dt = tables.at(i)->getDataTab();
        TBTable* tb = tables.at(i)->getTable();
    
    // set up the starting row number
    if(!second && tw->currentRow() > -1) i = tw->currentRow() - 1;
    else i = tw->rowCount() - 1;
    
    for(; i >= 0; i--) {
        int j = rules->rowPasses(tb, dt->logicalIndex(i));
        if(j > -1) {
            tw->setCurrentCell(i, j);
            break;
        }
    }

    // didn't find anything
    if(i == -1) {
        if(second) {
            QMessageBox::warning(this, "Find",
                     "Could not find any matching rows on this page.");
        } else if(QMessageBox::question(this, "Find",
              "Reached top of page.  Continue search at bottom?",
              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            findPrev(true);
            
        }
    }
}

bool TBBrowser::filterAvailable(int index) {
    if(index < 0 || index >= tabWidget->count()) return false;
    else return currentFilters.at(index) != NULL;
}

void TBBrowser::filterOnFields() {
    int i = tabWidget->currentIndex();
    TBTableTabs* tt = tables.at(i);
    TBFilterRules* fr = new TBFilterRules(tt->getTable());
    tt->showWidgetInTableSplitter(fr);
    connect(fr, SIGNAL(runRequested(TBFilterRuleSequence*, TBFilterRules*)),
            this, SLOT(filterRulesEntered(TBFilterRuleSequence*)));
}

void TBBrowser::clearFilterOnFields() {
    int i = tabWidget->currentIndex();
    QTableWidget* tw = tables.at(i)->getTableWidget();
    if(currentFilters[i] != NULL) delete currentFilters[i];
    currentFilters[i] = NULL;
    tables.at(i)->getDataTab()->setFilter(NULL);
    for(int i = 0; i < tw->rowCount(); i++)
        tw->showRow(i);
    emit filterRuleCleared(tabWidget->currentIndex());
}

TBFilterRuleSequence* TBBrowser::filterAt(int index) {
    if(index < 0 || index >= (int)tables.size()) return NULL;
    return currentFilters.at(index);
}

void TBBrowser::formatDisplay(int i) {
    tables[tabWidget->currentIndex()]->getDataTab()->formatField(i);
}

void TBBrowser::viewTableInfo() {
    int i = tabWidget->currentIndex();
    TBTableTabs* tt = tables.at(i);
    TBTableInfo* info = new TBTableInfo(tt->getTable());
    tt->showWidgetInCurrentSplitter(info);
}

void TBBrowser::sort(vector<pair<String, bool> >& s) {
    sort(tabWidget->currentIndex(), s);
}

void TBBrowser::sort(int index, vector<pair<String, bool> >& s) {
    if(index < 0 || index >= (int)tables.size()) return;
    else tables.at(index)->sort(s);
}


/* Action Methods */

Result TBBrowser::doAction(TBAction* action) {
    Result r = actions.doAction(action);
    if(r.valid) emit actionPerformed(action);
    return r;
}

Result TBBrowser::undoAction() {
    Result r = actions.undoAction();
    if(r.valid) emit actionUndone();
    return r;
}

Result TBBrowser::redoAction() {
    Result r = actions.redoAction();
    if(r.valid) emit actionRedone();
    return r;
}


/* Progress Methods */

QProgressPanel* TBBrowser::addProgressPanel(String label, bool h, bool c) {
    tabWidget->setEnabled(false);
    QProgressPanel* qpp = new QProgressPanel(label, h, c);
    gridLayout->addWidget(qpp, 0, 0, -1, -1);
    qpp->show();

    // force a repaint
    QCoreApplication::processEvents();
    
    bgTaskRunning = true;
    return qpp;
}

void TBBrowser::removeProgressPanel(QProgressPanel* panel) {
    gridLayout->removeWidget(panel);
    tabWidget->setEnabled(true);
    bgTaskRunning = false;
}

ProgressHelper* TBBrowser::enableProgressFrame(String l) {
    label->setText(l.c_str());
    frame->show();
    tabWidget->setEnabled(false);
    repaint();
    bgTaskRunning = true;
    return new ProgressHelper(label, progress);
}

void TBBrowser::disableProgressFrame() {
    tabWidget->setEnabled(true);
    label->setText("");
    frame->hide();
    bgTaskRunning = false;
}


/* Miscellaneous Browser Methods */

bool TBBrowser::bgTaskIsRunning() {
    return bgTaskRunning;
}

TBView* TBBrowser::view() {
    TBView* view = new TBView();

    int selInd = tabWidget->currentIndex();
    for(unsigned int i = 0; i < tables.size(); i++) {
        // create TBTableView
        TBTableView* tv = new TBTableView();
        TBTableTabs* tt = tables.at(i);
        TBTable* table = tt->getTable();
        tv->location = tt->getFileName();
        tv->selected = (int)i == selInd;
        tv->taql = table->isTaQL();

        if(tt->getTableWidget()->horizontalHeader()->sectionsHidden()) {
            for(int j = 0; j < table->getNumFields(); j++) {
                tv->hidden.push_back(columnIsHidden(i, j));
            }
        }
            
        if(tt->getTableWidget()->horizontalHeader()->sectionsMoved()) {
            QHeaderView* qhv = tt->getTableWidget()->horizontalHeader();
            bool changed = false;
            for(int j = 0; j < tt->getTableWidget()->columnCount(); j++) {
                if(qhv->visualIndex(j) != j) {
                    changed = true;
                    break;
                }
            }
            
            if(changed)
                for(int j = 0; j < tt->getTableWidget()->columnCount(); j++)
                    tv->visInd.push_back(qhv->visualIndex(j));
        }

        tv->loadedFrom = table->getRowIndex();
        tv->loadedNum = table->getSelectedRows();
        tv->filter = tt->getDataTab()->getFilter();

        for(int j = 0; j < table->getNumFields(); j++) {
            tv->formats.push_back(tt->getDataTab()->formatAt(j));
        }

        if(tt->getSortFields() != NULL && tt->getSortFields()->size() > 0) {
            for(unsigned int j = 0; j < tt->getSortFields()->size(); j++) {
                tv->sort.push_back(tt->getSortFields()->at(j));
            }
        }
        
        TBConstants::dprint(TBConstants::DEBUG_MED,
                     "Added table view for " + tv->location + ".");
        view->addTableView(tv);
    }
    
    return view;
}

void TBBrowser::showView(TBView* view) {
    if(view == NULL) return;
    
    vector<TBTableView*>* views = view->getTableViews();
    bool filter = false;
    int selInd = 0;
    
    for(unsigned int i = 0; i < views->size(); i++) {
        // open table
        TBTableView* v = views->at(i);
        int ind = openTable(v->location, v->taql, NULL, v->loadedFrom,
                            v->loadedNum);
        if(ind < 0) continue;
        
        TBTableTabs* tt = tables.at(ind);
        TBTable* tb = tt->getTable();
        
        // show table view
        if(v->sort.size() > 0) {
            // make sure fields exist
            bool found = false;
            for(unsigned int j = 0; j < v->sort.size(); j++) {
                String field = v->sort.at(j).first;
                found = false;
                for(unsigned int k = 0; k < tb->getFields()->size(); k++) {
                    if(tb->getFields()->at(k)->getName() == field) {
                        found = true;
                        break;
                    }
                }
                
                if(!found) {
                    v->sort.erase(v->sort.begin() + j);
                }
            }
            
            if(v->sort.size() > 0)
                tables.at(ind)->sort(v->sort);
        }
        
        for(unsigned int j = 0; j < v->hidden.size(); j++)
            setColumnHidden(ind, j, v->hidden.at(j));
        
        if(v->filter != NULL && v->filter->size() > 0) {
            // make sure filter is valid
            
            bool found = false;
            for(unsigned int j = 0; j < v->filter->size(); j++) {
                TBFilterRule* r = v->filter->at(j);
                found = false;
                String field = r->getField();
                for(unsigned int k = 0; k < tb->getFields()->size(); k++) {
                    if(tb->getFields()->at(k)->getName() == field) {
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    delete r;
                    v->filter->getRules()->erase(v->filter->getRules()->begin()
                                                 + j);
                }
            }
            
            if(v->filter->size() > 0) {
                currentFilters[ind] = v->filter;
                filter = true;
            }
        }
        
        if(v->selected) selInd = ind;
        
        for(unsigned int j = 0; j < v->formats.size(); j++) {
            if(v->formats.at(j) != NULL) {
                // make sure format is valid
                TBFormat* f = v->formats.at(j);
                
                tables.at(ind)->getDataTab()->applyFormat(j, f);
            }
        }
        
        QHeaderView* qhv= tables.at(ind)->getTableWidget()->horizontalHeader();
        for(unsigned int j = 0; j < v->visInd.size(); j++) {
            int index = v->visInd.at(j);
            if(index != qhv->visualIndex(j)) {
                qhv->moveSection(qhv->visualIndex(j), index);
            }
        }
    }

    if(filter) {
        //progressPanel = addProgressPanel("Filtering shown rows...",
        //                               false, false);
        //ProgressHelper ph(progressPanel);
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        //int sum = 0;
        //for(unsigned int i = 0; i < tables.size(); i++)
        //    sum += tables.at(i)->getTable()->getLoadedRows();
        //ph.setSteps(sum / 10);
    
        for(unsigned int i = 0; i < tables.size(); i++) {
            if(currentFilters.at(i) != NULL) {
                tabWidget->setCurrentIndex(i);
                TBFilterRuleSequence* rules = currentFilters.at(i);
                tables.at(i)->getDataTab()->setFilter(rules);
            }
        }
        //ph.done();
        //removeProgressPanel(progressPanel);
        //delete progressPanel;
        //progressPanel = NULL;
        QApplication::restoreOverrideCursor();
    }

    tabWidget->setCurrentIndex(selInd);
}

// Public Slots //

void TBBrowser::displayError(String message) {
     errorDialog->showMessage(tr(message.c_str()));
}

void TBBrowser::openTaQL(String command) {
    openTable(command, true);
}

void TBBrowser::followReference(String subtable, int index) {
    vector<TBKeyword*>* kws = currentlySelectedTable()->getTableKeywords();
    for(unsigned int i = 0; i < kws->size(); i++) {
        TBKeyword* kw = kws->at(i);
        if(kw->getName() == subtable) {
            int j = openTable(kw->getValue()->asString());
            if(j > -1) {
                TBTable* t = tables.at(j)->getTable();
                if(0 <= index && index < t->getTotalRows()) {
                    int x = t->getRowIndex();
                    int y = x + (t->getLoadedRows() - 1);

                    // load correct page
                    if(index < x || index > y) {
                        int p = index / t->getSelectedRows();
                        tables.at(j)->getDataTab()->loadPage(p);
                        x = t->getRowIndex();
                    }

                    QTableWidget* tw = tables.at(j)->getTableWidget();
                    tw->setCurrentCell(index - x, 0);
                } else {
                    String msg = "Index " + TBConstants::itoa(index) +
                                 " is not valid for subtable "+ subtable + ".";
                    displayError(msg);
                }
            }
            break;
        }
    }
}

void TBBrowser::removeActionsAssociatedWithWidget(QWidget* widget) {
    removeActionsAssociatedWith(widget);
}

void TBBrowser::removeActionsAssociatedWith(void* widget) {
    vector<TBAction*> toRemove;
    for(int i = 0; i < actions.size(); i++) {
        TBAction* a = actions.at(i);
        if(a->isAssociatedWith(widget)) {
            toRemove.push_back(a);
        }
    }

    for(unsigned int i = 0; i < toRemove.size(); i++) {
        actions.remove(toRemove.at(i));
        delete toRemove.at(i);
    }
    if(toRemove.size() > 0) emit actionUndone();

    toRemove.clear();
    for(int i = 0; i < actions.undoneSize(); i++) {
        TBAction* a = actions.undoneAt(i);
        if(a->isAssociatedWith(widget))
            toRemove.push_back(a);
    }

    for(unsigned int i = 0; i < toRemove.size(); i++) {
        actions.removeUndone(toRemove.at(i));
        delete toRemove.at(i);
    }
    if(toRemove.size() > 0) emit actionRedone();
}

// Private Methods //

void TBBrowser::updateEnabled() {
    tabWidget->setEnabled(available);
}

int TBBrowser::addTable(String f, bool taql, DriverParams* dp, int start,
                        int num) {
    // if f is already open, select that tab
    for(unsigned int i = 0; i < tables.size(); i++) {
        if(tables.at(i)->getTable()->isTaQL() == taql &&
           tables.at(i)->getFileName() == f) {
            tabWidget->setCurrentIndex(i);
            return i;
        }
    }

    if(dp == NULL) {
        // use defaults
        dp = new DriverParams();
    }

    if(start < 0) start = 0;
    if(num < 1) num = TBConstants::DEFAULT_SELECT_NUM;
    
    // If it's not a valid table, an exception will be thrown in the
    // TBTableTabs constructor (annoyingly), so try to catch that here.
    TBTableTabs* t = NULL;
    try {
        t = new TBTableTabs(this, f, dp, taql);
        t->loadRows(start, num);
        if(t->isAvailable()) {
            tables.push_back(t);
            currentFindRules.push_back(NULL);
            currentFilters.push_back(NULL);
            available = true;
            tabWidget->addTab(t, t->getName().c_str());
            updateEnabled();
            tabWidget->setCurrentIndex(tables.size() - 1);
            tabWidget->setTabToolTip(tables.size() - 1,
                                     t->getTable()->tableToolTip().c_str());
            QCoreApplication::processEvents();
            emit tableOpened(t->getName());
            return tables.size() - 1;
        } else {
            delete t;
            displayError("Could not open table " + f + "!");
            return -1;
        }
    } catch(...) {
        if(t != NULL) delete t;
        displayError("Could not open table " + f + "!");
        return -1;
    }
}

// Private Slots //

void TBBrowser::doCloseTable() {
    closeTable();
}

void TBBrowser::threadFinished() {
    if(!exportHidden) {
        removeProgressPanel(progressPanel);
        emit threadIsFinished();
    } else { // the thread has been hidden
        QMessageBox::information(this, "Export to VOTable",
                                 "Export thread has completed.");
        label->setText("");
        frame->hide();
    }
    delete exportThread;
    exportThread = NULL;
    delete progressPanel;
    progressPanel = NULL;
    exportHidden = false;
    bgTaskRunning = false;
}

void TBBrowser::hideThread() {
    tabWidget->setEnabled(true);
    gridLayout->removeWidget(progressPanel);
    progressPanel->setVisible(false);

    frame->show();
    label->setText(progressPanel->getLabel()->text());
    progress->setMaximum(progressPanel->getProgressBar()->maximum());
    progress->setValue(progressPanel->getProgressBar()->value());
    connect(progressPanel->getProgressBar(), SIGNAL(valueChanged(int)),
            progress, SLOT(setValue(int)));
    
    exportHidden = true;
    emit threadIsFinished();
}

void TBBrowser::filterRulesEntered(TBFilterRuleSequence* rules) {
    int i = tabWidget->currentIndex();
    tables.at(i)->clearWidgetInTableSplitter();

    if(rules != NULL && rules->size() > 0) {
        if(currentFilters.at(i) != NULL) delete currentFilters.at(i);
        currentFilters[i] = rules;
    
        //progressPanel = addProgressPanel("Filtering shown rows...",
        //                                false, false);
        //ProgressHelper ph(progressPanel);
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        
        tables.at(i)->getDataTab()->setFilter(rules);

        //removeProgressPanel(progressPanel);
        //delete progressPanel;
        //progressPanel = NULL;
        QApplication::restoreOverrideCursor();
        emit filterRuleAvailable(tabWidget->currentIndex());
    } else {
        if(currentFilters.at(i) != NULL) delete currentFilters.at(i);
        currentFilters[i] = NULL;
        tables.at(i)->getDataTab()->setFilter(NULL);
        if(rules != NULL) delete rules;
        emit filterRuleCleared(tabWidget->currentIndex());
    }
}

void TBBrowser::findRulesEntered(TBFilterRuleSequence* rules,
                                 TBFilterRules* rDialog){
    rDialog->close();
    if(rules == NULL || rules->size() < 1) return;

    int i = tabWidget->currentIndex();
    
    if(currentFindRules.at(i) != NULL) delete currentFindRules.at(i);
    else emit findRuleAvailable();
    currentFindRules[i] = rules;

    QTableWidget* tw = tables.at(i)->getTableWidget();
    TBDataTab* dt = tables.at(i)->getDataTab();
    TBTable* tb = tables.at(i)->getTable();
    
    for(i = 0; i < tw->rowCount(); i++) {
        int j = rules->rowPasses(tb, dt->logicalIndex(i));
        if(j > -1) {
            tw->setCurrentCell(i, j);
            break;
        }
    }

    if(i == tw->rowCount())
        QMessageBox::warning(this, "Find",
                     "Could not find any matching rows on this page.");
}

void TBBrowser::tabChange(int index) {
    if(index >= 0) emit tabChanged(index);
}

}
