//# TBSorter.cc: Widget to configure a multi-field sort ordering.
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
#include <casaqt/QtBrowser/TBSorter.qo.h>

namespace casa {

//////////////////////////
// TBSORTER DEFINITIONS //
//////////////////////////

// Constructors/Destructors //

TBSorter::TBSorter(vector<String>& c, vector<pair<String, bool> >* s,
                   QWidget* parent): QDialog(parent), cols(c) {
    setupUi(this);

    // Connect widgets
    connect(addButton, SIGNAL(clicked()), this, SLOT(add()));
    connect(addAllButton, SIGNAL(clicked()), this, SLOT(addAll()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
    connect(removeAllButton, SIGNAL(clicked()), this, SLOT(removeAll()));
    connect(moveUpButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(moveDownButton, SIGNAL(clicked()), this, SLOT(moveDown()));
    connect(tableWidget, SIGNAL(currentCellChanged(int, int, int, int)),
            this, SLOT(tableIndexChanged(int)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptButton()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // Add columns to field list
    for(unsigned int i = 0; i < cols.size(); i++) {
        listWidget->addItem(cols.at(i).c_str());
    }
    
    tableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);

    // Move any fields from the field list to the sort list if they were
    // given in the constructor parameter
    if(s != NULL && s->size() != 0) {
        for(unsigned int i = 0; i < s->size(); i++) {
            String f = s->at(i).first;
            for(unsigned int j = 0; j < cols.size(); j++) {
                if(f == cols.at(j)) {
                    add(j, s->at(i).second);
                    break;
                }
            }
        }
    }
}

TBSorter::~TBSorter() {

}

// Private Methods //

void TBSorter::add(int row, bool asc) {
	// Remove from field list
	String field = qPrintable(listWidget->takeItem(row)->text());

	// Add to sort list
    int n = tableWidget->rowCount();
    tableWidget->setRowCount(n + 1);
    QTableWidgetItem* item = new QTableWidgetItem(field.c_str());
    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    tableWidget->setItem(n, 0, item);

    QCheckBox* box = new QCheckBox("Ascending");
    box->setChecked(asc);
    tableWidget->setCellWidget(n, 1, box);
    
    // Update buttons
    removeButton->setEnabled(true);
    removeAllButton->setEnabled(true);
    if(listWidget->count() == 0) {
        addButton->setEnabled(false);
        addAllButton->setEnabled(false);
    }
}

void TBSorter::remove(int row) {
	// Remove from sort list
    String field = qPrintable(tableWidget->takeItem(row, 0)->text());
    tableWidget->removeRow(row);
    
    int index = 0;
    int n = listWidget->count();

    // Find where in the field list it was originally and put it there
    if(n == 0) {
        listWidget->addItem(field.c_str());
    } else {
        // insert in proper place in list
        for(unsigned int i = 0; i < cols.size(); i++) {
            if(cols.at(i) == field || index >= n) {
                listWidget->insertItem(index, field.c_str());
                break;
            }
            String item = qPrintable(listWidget->item(index)->text());
            if(cols.at(i) == item)
                index++;
        }
    }

    // Update buttons
    addButton->setEnabled(true);
    addAllButton->setEnabled(true);
    if(tableWidget->rowCount() == 0) {
        removeButton->setEnabled(false);
        removeAllButton->setEnabled(false);
    }
}

void TBSorter::swapRows(int r1, int r2) {
	// Pull out the two rows...
    QList<QTableWidgetItem*> row1, row2;
    for (int col = 0; col < tableWidget->columnCount(); col++) {
        row1 << tableWidget->takeItem(r1, col);
        row2 << tableWidget->takeItem(r2, col);
    }

    // ..then put them in their new places
    for (int col = 0; col < tableWidget->columnCount(); ++col) {
        tableWidget->setItem(r2, col, row1.at(col));
        tableWidget->setItem(r1, col, row2.at(col));
    }
}

// Private Slots //

void TBSorter::add() {
    int i = listWidget->currentRow();
    if(i != -1) {
        add(i);
        if(i >= listWidget->count()) i = listWidget->count() - 1;
        listWidget->setCurrentRow(i);
        tableWidget->setCurrentCell(tableWidget->rowCount() - 1, 0);
    }
}

void TBSorter::addAll() {
    while(listWidget->count() != 0) {
        add(0);
        tableWidget->setCurrentCell(tableWidget->rowCount() - 1, 0);
    }
}

void TBSorter::remove() {
    int i = tableWidget->currentRow();
    if(i != -1) {
        int j = tableWidget->currentColumn();
        remove(i);
        int r = i + 1;
        if(r >= tableWidget->rowCount()) r = tableWidget->rowCount() - 1;
        tableWidget->setCurrentCell(r, j);
    }
}

void TBSorter::removeAll() {
    tableWidget->clearContents();
    tableWidget->setRowCount(0);
    listWidget->clear();
    for(unsigned int i = 0; i < cols.size(); i++)
        listWidget->addItem(cols.at(i).c_str());

    removeAllButton->setEnabled(false);
    removeButton->setEnabled(false);
    addButton->setEnabled(true);
    addAllButton->setEnabled(true);
}

void TBSorter::tableIndexChanged(int row) {
    moveUpButton->setEnabled(row >= 1 && row < tableWidget->rowCount());
    moveDownButton->setEnabled(row >= 0 && row < tableWidget->rowCount() - 1);
}

void TBSorter::moveUp() {
    int i = tableWidget->currentRow();
    if(i <= 0 || i >= tableWidget->rowCount()) return;
    swapRows(i, i - 1);
    tableWidget->setCurrentCell(i - 1, tableWidget->currentColumn());
}

void TBSorter::moveDown() {
    int i = tableWidget->currentRow();
    if(i < 0 || i >= tableWidget->rowCount() - 1) return;
    swapRows(i, i + 1);
    tableWidget->setCurrentCell(i + 1, tableWidget->currentColumn());
}

void TBSorter::acceptButton() {
    if(tableWidget->rowCount() > 0) {
    	// Collect sort list into vector
        vector<pair<String, bool> > v(tableWidget->rowCount());

        for(int i = 0; i < tableWidget->rowCount(); i++) {
            String field = qPrintable(tableWidget->item(i, 0)->text());
            bool asc= ((QCheckBox*)tableWidget->cellWidget(i, 1))->isChecked();
            pair<String, bool> p(field, asc);
            v[i] = p;
        }
        
        emit sortEntered(v);
    }

    accept();
}

}
