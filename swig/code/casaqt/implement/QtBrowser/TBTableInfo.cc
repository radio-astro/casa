//# TBTableInfo.cc: Displays general information about a table.
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
#include <casaqt/QtBrowser/TBTableInfo.qo.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBTable.h>

namespace casa {

/////////////////////////////
// TBTABLEINFO DEFINITIONS //
/////////////////////////////

// Constructors/Destructors //

TBTableInfo::TBTableInfo(TBTable* t): QWidget(), table(t) {
    setupUi(this);
    tableWidget->horizontalHeader()->setResizeMode(6, QHeaderView::Stretch);

    if(t != NULL) {
        nameLabel->setText(("<b>" + t->getName() + "</b>").c_str());
        if(t->isTaQL()) locTaqlLabel->setText("<b>TaQL</b>:");
        locationLabel->setText(t->getFile().c_str());
        rowsLabel->setText(TBConstants::itoa(t->getTotalRows()).c_str());
        insertLabel->setText((t->canInsertRows())?"yes":"no");
        deleteLabel->setText((t->canDeleteRows())?"yes":"no");
        setFields(t->getFields());
    }
}

TBTableInfo::~TBTableInfo() { }

// Private Methods //

void TBTableInfo::setFields(vector<TBField*>* fields) {
    tableWidget->clearContents();
    tableWidget->setRowCount(fields->size());
    
    bool unit = false, ucd = false, ref = false, prec = false, width = false;

    for(unsigned int i = 0; i < fields->size(); i++) {
        TBField* field = fields->at(i);

        String str = field->getName();
        QTableWidgetItem* item = new QTableWidgetItem(str.c_str());
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        tableWidget->setItem(i, 0, item);
        
        str = field->getType();
        str = TBConstants::typeName(str);
        item = new QTableWidgetItem(str.c_str());
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        tableWidget->setItem(i, 1, item);

        str = field->getUnit();
        if(!str.empty()) unit = true;
        item = new QTableWidgetItem(str.c_str());
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        tableWidget->setItem(i, 2, item);

        str = field->getUCD();
        if(!str.empty()) ucd = true;
        item = new QTableWidgetItem(str.c_str());
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        tableWidget->setItem(i, 3, item);

        str = field->getRef();
        if(!str.empty()) ref = true;
        item = new QTableWidgetItem(str.c_str());
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        tableWidget->setItem(i, 4, item);

        str = field->getPrecision();
        if(!str.empty()) prec = true;
        item = new QTableWidgetItem(str.c_str());
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        tableWidget->setItem(i, 5, item);

        str = field->getWidth();
        if(!str.empty()) width = true;
        item = new QTableWidgetItem(str.c_str());
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        tableWidget->setItem(i, 6, item);
    }
    
    if(!unit) tableWidget->hideColumn(2);
    if(!ucd) tableWidget->hideColumn(3);
    if(!ref) tableWidget->hideColumn(4);
    if(!prec) tableWidget->hideColumn(5);
    if(!width) tableWidget->hideColumn(6);
    
    // relabel headers to be 0-based
    QStringList headers;
    for(int i = 0; i < tableWidget->columnCount(); i++)
        headers << TBConstants::itoa(i).c_str();
    tableWidget->setVerticalHeaderLabels(headers);
    
    tableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    tableWidget->resizeColumnsToContents();
}

}
