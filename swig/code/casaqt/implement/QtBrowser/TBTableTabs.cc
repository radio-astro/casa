//# TBTableTabs.cc: Collection of table backend and display tabs.
//# Copyright (C) 2007-2008
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
#include <casaqt/QtBrowser/TBTableTabs.qo.h>
#include <casaqt/QtBrowser/TBBrowser.qo.h>
#include <casaqt/QtBrowser/TBDataTab.qo.h>
#include <casaqt/QtBrowser/TBKeywordsTab.qo.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/QProgressPanel.qo.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBData.h>

namespace casa {

//////////////////////////////
// TBTABLETABS DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBTableTabs::TBTableTabs(TBBrowser* b, String f, DriverParams* dp, bool taql):
                   QWidget(), editable(false), filename(f), table(f, dp, taql),
                   dataTab(this), keywordsTab(this, &table),
                   fieldKeywordsTab(this, &table), browser(b) {
    setupUi(this);
    
    tabWidget->removeTab(0);
    tabWidget->addTab(&dataTab, "table data");
    tabWidget->addTab(&keywordsTab, "table keywords");
    tabWidget->addTab(&fieldKeywordsTab, "field keywords");
    
    available = table.isReady();
    name = table.getName();
    
    // Connect widgets
    connect(&dataTab, SIGNAL(dataChanged(int, int, String)),
            this, SLOT(dataChanged(int, int, String)));
    connect(&dataTab, SIGNAL(rightWidgetClosed(QWidget*)),
            browser, SLOT(removeActionsAssociatedWithWidget(QWidget*)));
    connect(&keywordsTab, SIGNAL(rightWidgetClosed(QWidget*)),
            browser, SLOT(removeActionsAssociatedWithWidget(QWidget*)));
    connect(&fieldKeywordsTab, SIGNAL(rightWidgetClosed(QWidget*)),
            browser, SLOT(removeActionsAssociatedWithWidget(QWidget*)));
    connect(&dataTab, SIGNAL(followReferenceRequested(String, int)),
            browser, SLOT(followReference(String, int)));
}

TBTableTabs::~TBTableTabs() { }

// Accessors/Mutators //

bool TBTableTabs::isAvailable() { return available; }

bool TBTableTabs::isEditable() { return editable; }

String TBTableTabs::getName() { return name; }

String TBTableTabs::getFileName() { return filename; }

TBBrowser* TBTableTabs::getBrowser() { return browser; }

TBDataTab* TBTableTabs::getDataTab() { return &dataTab; }

TBTableKeywordsTab* TBTableTabs::getKeywordsTab() { return &keywordsTab; }

TBFieldKeywordsTab* TBTableTabs::getFieldKeywordsTab() {
    return &fieldKeywordsTab;
}

QTableWidget* TBTableTabs::getTableWidget() {
    return dataTab.getTableWidget();
}

QTableWidget* TBTableTabs::getTableKeywords() {
    return keywordsTab.getTableWidget();
}

QTreeWidget* TBTableTabs::getFieldKeywords() {
    return fieldKeywordsTab.getTreeWidget();
}

TBTable* TBTableTabs::getTable() { return &table; }

vector<pair<String, bool> >* TBTableTabs::getSortFields() {
    return dataTab.getSortFields();
}

// Public Methods //

bool TBTableTabs::loadRows(int start, int num) {
    while(!table.canRead()) {
        QMessageBox::StandardButton resp = QMessageBox::question(this,
                "Read Lock Unavailable", "The read lock for this table is "
                "currently unavailable!  Would you like to try again?",
                QMessageBox::Retry | QMessageBox::Abort);
        if(resp == QMessageBox::Abort) return false;
    }
    
    //QProgressPanel* pp = browser->addProgressPanel("Loading Rows...", false,
    //                                               false);
    //ProgressHelper ph(pp);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    
    // Load rows in backend
    Result r = table.loadRows(start, num, false, NULL, true);//, &ph);
    available = r.valid;
    if(available) {
        // ..then update GUI
        dataTab.updateTable();//&ph);
        keywordsTab.updateTable();//&ph);
        fieldKeywordsTab.updateTable();//&ph);
    } else {
        TBConstants::strtrim(r.result);
        if(TBConstants::equalsIgnoreCase(r.result, TBConstants::ERROR_EMPTY))
            browser->displayError("This table has no data.");
        else
            browser->displayError(r.result);
    }
    
    //browser->removeProgressPanel(pp);
    //delete pp;
    QApplication::restoreOverrideCursor();
    return available;
}

void TBTableTabs::insertRows(int n) {
    if(n < 1) return;
    
    // Insert rows in backend
    Result r = table.insertRows(n);
    if(r.valid) {
        // ...then update GUI
        // go to last page
        int pages = table.getNumPages();
        int start = (pages - 1) * table.getSelectedRows();
        loadRows(start, table.getSelectedRows());
    } else {
        browser->displayError(r.result);
    }
}

void TBTableTabs::deleteRows(vector<int> r) {
    if(r.size() == 0) return;

    Result res = table.deleteRows(r);
    // make sure rowIndex is still within table bounds
    int rowIndex = table.getRowIndex();
    if(rowIndex < table.getTotalRows()) {
        // see if current page needs to be refreshed
        int last = table.getRowIndex() + table.getLoadedRows();
        bool refresh = false;
        for(unsigned int i = 0; i < r.size(); i++) {
            if(r.at(i) < last) {
                refresh = true;
                break;
            }
        }
        if(refresh) loadRows(rowIndex, table.getSelectedRows());
    } else {
        // load last page
        int pages = table.getNumPages();
        int start = (pages - 1) * table.getSelectedRows();
        loadRows(start, table.getSelectedRows());
    }
    if(!res.valid) {
        browser->displayError(res.result);
    }
}

void TBTableTabs::clearSort() {
    vector<pair<String, bool> > v;
    sort(v);
}

void TBTableTabs::sort(vector<pair<String, bool> >& s) {
    dataTab.sortBy(s);
}

void TBTableTabs::highlight(int row) {  
    if(row >= table.getRowIndex() &&
       row < table.getRowIndex() + dataTab.getTableWidget()->rowCount()) {
        // row is in range
        int r = row - table.getRowIndex();
        int tabIndex = browser->indexOf(this);
        browser->getTabWidget()->setCurrentIndex(tabIndex);
        dataTab.highlight(r);
        
    } else {
        // prompt for page load
        String message = "This row is not currently loaded.  Would you like";
        message += " to load the relevant page?"; 
        QMessageBox::StandardButton sb = QMessageBox::question(browser,
                            "Load rows?", message.c_str(),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(sb == QMessageBox::Yes) {
            int sel = table.getSelectedRows();
            int page = row / sel;
            loadRows(sel * page, sel);
            highlight(row);
        }
    }
}

// Public Slots //

void TBTableTabs::dataChanged(int row, int col, String newVal) {
    TBData* oldVal = table.dataAt(row, col);
    if(editable) {
        if(oldVal->asString() == newVal) return;
        
        String type = oldVal->getType();
        newVal = TBConstants::formatValue(newVal, type);
        if(TBConstants::valueIsValid(newVal, type)) {
            TBData* d = TBData::create(newVal, type);
            TBEditDataAction* action = new TBEditDataAction(this, row, col, d);
            Result r = browser->doAction(action);
            if(!r.valid) {
                browser->displayError(r.result);

                // restore old value
                dataTab.setData(row, col, oldVal);
            }
        } else {
            browser->displayError(newVal + " is not a valid value for type " +
                                  TBConstants::typeName(type) + ".");

            dataTab.setData(row, col, oldVal);
        }
    } else {
        // restore old value
        dataTab.setData(row, col, oldVal);
    }
}

void TBTableTabs::setEditable(bool e) {
    editable = e;
    TBConstants::dprint(TBConstants::DEBUG_MED, "Table " + name + " is now " +
                        (e?"":" not") + "editable.");
}

void TBTableTabs::showWidgetInTableSplitter(QWidget* widget, bool isArray) {
    dataTab.showWidgetInSplitter(widget, isArray);
}

void TBTableTabs::showWidgetInKeywordSplitter(QWidget* widget) {
    keywordsTab.showWidgetInSplitter(widget);
}

void TBTableTabs::showWidgetInFieldKeywordSplitter(QWidget* widget) {
    fieldKeywordsTab.showWidgetInSplitter(widget);
}

void TBTableTabs::showWidgetInCurrentSplitter(QWidget* widget) {
    int n = tabWidget->currentIndex();
    if(n == 0) showWidgetInTableSplitter(widget);
    else if(n == 1) showWidgetInKeywordSplitter(widget);
    else if(n == 2) showWidgetInFieldKeywordSplitter(widget);
}

void TBTableTabs::clearWidgetInTableSplitter() {
    dataTab.clearWidgetInSplitter();
}

void TBTableTabs::clearWidgetInKeywordSplitter() {
    keywordsTab.clearWidgetInSplitter();
}

void TBTableTabs::clearWidgetInFieldKeywordSplitter() {
    fieldKeywordsTab.clearWidgetInSplitter();
}

void TBTableTabs::clearWidgetInCurrentSplitter() {
    int n = tabWidget->currentIndex();
    if(n == 0) clearWidgetInTableSplitter();
    else if(n == 1) clearWidgetInKeywordSplitter();
    else if(n == 2) clearWidgetInFieldKeywordSplitter();
}

}
