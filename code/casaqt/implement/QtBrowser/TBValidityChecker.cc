//# TBValidityChecker.cc: Widget for running TBTests on opened tables.
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
#include <casaqt/QtBrowser/TBValidityChecker.qo.h>
#include <casaqt/QtBrowser/TBBrowser.qo.h>
#include <casaqt/QtBrowser/TBTest.h>

namespace casa {

///////////////////////////////////
// TBVALIDITYCHECKER DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

TBValidityChecker::TBValidityChecker(TBBrowser* b) : QWidget(), browser(b) {
    setupUi(this);
    
    vector<String> strs = browser->openedTableNames();
    for(unsigned int i = 0; i < strs.size(); i++) {
        tableChooser->addItem(strs.at(i).c_str());
    }
    if(strs.size() <= 1) tableChooser->setEnabled(false);

    // Connect widgets
    setupTests();
    connect(testList, SIGNAL(currentRowChanged(int)),
    		this, SLOT(loadTest(int)));

    connect(runButton, SIGNAL(clicked()), this, SLOT(runCheck()));
    connect(runAllButton, SIGNAL(clicked()), this, SLOT(runChecks()));

    connect(tableChooser, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(tableChanged(QString)));
    tableChooser->setCurrentIndex(0);

    connect(browser, SIGNAL(tableOpened(String)),
    		this, SLOT(tableOpened(String)));
    connect(browser, SIGNAL(tableClosed(String)),
    		this, SLOT(tableClosed(String)));
}

TBValidityChecker::~TBValidityChecker() {
    for(unsigned int i = 0; i < tests.size(); i++)
        delete tests.at(i);
}

// Private Methods //

void TBValidityChecker::setupTests() {
	// All known tests are added here
    TBTest* t = new TBIDFieldsTest(browser);
    tests.push_back(t);
    testList->addItem(t->getName().c_str());
    
    t = new TBSubtablesTest(browser);
    tests.push_back(t);
    testList->addItem(t->getName().c_str());

    t = new TBValidSubtablesTest(browser);
    tests.push_back(t);
    testList->addItem(t->getName().c_str());
}

// Private Slots //

void TBValidityChecker::tableChanged(QString nt) {
    testTable->clear();
    testTable->setRowCount(0);
    testTable->setColumnCount(0);
    testList->clearSelection();
}

void TBValidityChecker::loadTest(int index) {
    if(testList->currentRow() < 0) return;

    String t = qPrintable(tableChooser->currentText());
    vector<String> checks = tests.at(index)->checks(t);

    testTable->clear();
    testTable->setColumnCount(2);
    QStringList cols;
    cols << "Result" << "Check";
    testTable->setHorizontalHeaderLabels(cols);
    testTable->setRowCount(checks.size());
    for(unsigned int i = 0; i < checks.size(); i++) {
        QTableWidgetItem* item = new QTableWidgetItem(checks.at(i).c_str());
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        testTable->setItem(i, 1, item);
        item = new QTableWidgetItem("not run  ");
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        testTable->setItem(i, 0, item);
    }
    testTable->resizeColumnsToContents();
}

void TBValidityChecker::runChecks() {
    String table = qPrintable(tableChooser->currentText());
    int t = testList->currentRow();
    if(t < 0) return;
    
    for(int i = 0; i < testTable->rowCount(); i++) {
        if(tests.at(t)->runCheck(table, i))
            testTable->item(i, 0)->setText("passed");
        else
            testTable->item(i, 0)->setText("FAILED");
    }
}

void TBValidityChecker::runCheck() {
    String table = qPrintable(tableChooser->currentText());
    int t = testList->currentRow();
    int c = testTable->currentRow();
    if(t < 0 || c < 0) return;

    if(tests.at(t)->runCheck(table, c))
        testTable->item(c, 0)->setText("passed");
    else
        testTable->item(c, 0)->setText("FAILED");
}

void TBValidityChecker::tableOpened(String table) {
    tableChooser->addItem(table.c_str());
    tableChooser->setEnabled(true);
}

void TBValidityChecker::tableClosed(String table) {
    for(int i = 0; i < tableChooser->count(); i++) {
        String t = qPrintable(tableChooser->itemText(i));
        if(t == table) {
            tableChooser->removeItem(i);
            if(tableChooser->count() <= 1)
                tableChooser->setEnabled(false);
        }
    }
}

}
