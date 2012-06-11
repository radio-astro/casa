//# TBRowLocate.cc: Displays results of a row locate.
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
#include <casaqt/QtBrowser/TBRowLocate.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBTableTabs.qo.h>

namespace casa {

/////////////////////////////
// TBROWLOCATE DEFINITIONS //
/////////////////////////////

// Constructors/Destructors //

TBRowLocate::TBRowLocate(TBLocatedRows* r) : QDialog(NULL), rows(r),
                                             tables(r->tables()) {
    setupUi(this);    
    tabWidget->removeTab(0);
    
    QStringList cols;
    cols << "Row #";
    
    vector<int>* rv;
    QTableWidgetItem* item;
    for(unsigned int i = 0; i < tables.size(); i++) {
        rv = r->results[tables[i]];
        
        QTableWidget* table = new QTableWidget();
        table->setColumnCount(1);
        table->setHorizontalHeaderLabels(cols);
        table->setRowCount(rv->size());
        
        for(unsigned int j = 0; j < rv->size(); j++) {
            item = new QTableWidgetItem(TBConstants::itoa(rv->at(j)).c_str());
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            table->setItem(j, 0, item);
        }
        
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setCurrentCell(-1, -1);
        table->verticalHeader()->hide();
        
        connect(table, SIGNAL(currentCellChanged(int, int, int, int)),
                this, SLOT(cellClicked(int)));
        connect(table, SIGNAL(cellDoubleClicked(int, int)),
                this, SLOT(highlight()));
        
        tabWidget->addTab(table, tables[i]->getName().c_str());
    }
    
    if(tabWidget->count() > 0) tabChanged(0);
    
    connect(highlightButton, SIGNAL(clicked()), this, SLOT(highlight()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(next()));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
}

TBRowLocate::~TBRowLocate() {
    delete rows;
}

// Private Slots //

void TBRowLocate::cellClicked(int row) {
    if(tabWidget->currentIndex() == -1) return;
    
    highlightButton->setEnabled(row >= 0);
}

void TBRowLocate::highlight() {
    if(tabWidget->currentIndex() == -1) return;
    
    QTableWidget* tw = (QTableWidget*)tabWidget->currentWidget();   
    QTableWidgetItem* item = tw->currentItem();
    if(item == NULL) return;
    
    String val = qPrintable(item->text());
    int row;
    if(TBConstants::atoi(val, &row) < 1) return;

    TBTableTabs* tt = tables[tabWidget->currentIndex()];
    tt->highlight(row);
    
    nextButton->setEnabled(tw->currentRow() < tw->rowCount() - 1);
}

void TBRowLocate::next() {
    if(tabWidget->currentIndex() == -1) return;
    
    QTableWidget* tw = (QTableWidget*)tabWidget->currentWidget();
    int selRow = tw->currentRow();
    if(selRow == -1 || selRow >= tw->rowCount() - 1) return;
    
    tw->setCurrentCell(selRow + 1, tw->currentColumn());
    highlight();
}

void TBRowLocate::tabChanged(int index) {
    if(index == -1) return;
    
    QTableWidget* tw = (QTableWidget*)tabWidget->widget(index); 
    highlightButton->setEnabled(tw->currentRow() >= 0);
    nextButton->setEnabled(false);
}

}
