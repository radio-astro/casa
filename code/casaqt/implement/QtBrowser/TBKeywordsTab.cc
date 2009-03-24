//# TBKeywordsTab.cc: Widgets used to display table and field keywords.
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
#include <casaqt/QtBrowser/TBKeywordsTab.qo.h>
#include <casaqt/QtBrowser/QCloseableWidget.qo.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBKeyword.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/QProgressPanel.qo.h>
#include <casaqt/QtBrowser/TBTableTabs.qo.h>
#include <casaqt/QtBrowser/TBBrowser.qo.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/TBViewArray.qo.h>
#include <casaqt/QtBrowser/TBTypes.qo.h>

#include <vector>

namespace casa {

//////////////////////////////
// TBNEWKEYWORD DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBNewKeyword::TBNewKeyword(vector<String>* fields, QWidget* parent):
                                                        QDialog(parent) {
    setupUi(this);
    
    // Set up fields for field keyword
    if(fields == NULL || fields->empty()) {
        fieldLabel->close();
        fieldChooser->close();
    } else {
        for(unsigned int i = 0; i < fields->size(); i++)
            fieldChooser->addItem(fields->at(i).c_str());
    }
    
    types = TBConstants::allTypes();
    for(unsigned int i = 0; i < types->size(); i++) {
        String t = types->at(i);
        String name = TBConstants::typeName(t);
        typeChooser->addItem((name + " (" + t + ")").c_str());
    }
    
    typesWidget = new TBTypes(types->at(0));
    TBConstants::insert(valueFrame, typesWidget);
    connect(typeChooser, SIGNAL(currentIndexChanged(int)),
            this, SLOT(typeChanged(int)));
}

TBNewKeyword::~TBNewKeyword() {
    delete types;
    delete typesWidget;
}

// Private Slots //

void TBNewKeyword::typeChanged(int index) {
    if(index < 0) return;
    typesWidget->setType(types->at(index));
}

////////////////////////////////////
// TBTABLEKEYWORDSTAB DEFINITIONS //
////////////////////////////////////

// Constructors/Destructors //

TBTableKeywordsTab::TBTableKeywordsTab(TBTableTabs* tt, TBTable* t): QWidget(),
                                       ttabs(tt), table(t), arrayPanel(NULL) {
    setupUi(this);
    tableKeywords->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);

    sideWidget->close();
    rightWidget = new QCloseableWidget();
    splitter->insertWidget(1, rightWidget);
    rightWidget->hide();
    connect(rightWidget, SIGNAL(closeRequested(QWidget*)),
            this, SLOT(clearWidgetInSplitter()));
    connect(tableKeywords, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(doubleClicked(int, int)));
    connect(tableKeywords, SIGNAL(currentCellChanged(int, int, int, int)),
            this, SLOT(cellSelected(int)));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addKeyword()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(editKeyword()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeKeyword()));
    addButton->close();
    editButton->close();
    removeButton->close();
}

TBTableKeywordsTab::~TBTableKeywordsTab() {
    delete rightWidget;
}

// Accessor Methods //

QTableWidget* TBTableKeywordsTab::getTableWidget() { return tableKeywords; }

// Public Methods //

void TBTableKeywordsTab::updateTable(ProgressHelper* pp) {
    int steps = table->getTableKeywords()->size() + 2;
    if(pp != NULL) {
        pp->reset("Updating table keywords...");
        pp->setSteps(steps);
    }
    
    // Update table keywords
    vector<TBKeyword*>* keywords = table->getTableKeywords();
    tableKeywords->clearContents();
    tableKeywords->setRowCount(keywords->size());

    // put values in table keywords
    for(unsigned int i = 0; i < keywords->size(); i++) {
        String name = keywords->at(i)->getName();
        String type = keywords->at(i)->getType();
        String val = keywords->at(i)->getValue()->asString();

        if(!name.empty() && !type.empty() && !val.empty()) {
            QTableWidgetItem* item = new QTableWidgetItem(name.c_str());
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            tableKeywords->setItem(i, 0, item);

            item = new QTableWidgetItem(TBConstants::typeName(type).c_str());
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            tableKeywords->setItem(i, 1, item);
                
            item = new QTableWidgetItem(val.c_str());
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            tableKeywords->setItem(i, 2, item);

            String str;
            if(TBConstants::typeIsTable(type)) {
                int r = table->totalRowsOf(val);
                if(r > 0)
                    str = "Subtable has " + TBConstants::itoa(r) + " rows.";
                else
                    str = "Subtable has no rows.";
            }
            item = new QTableWidgetItem(str.c_str());
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            tableKeywords->setItem(i, 3, item);
        }
        if(pp != NULL) pp->step();
    }
    tableKeywords->resizeColumnsToContents();
    if(pp != NULL) pp->step();
    tableKeywords->resizeRowsToContents();
    if(pp != NULL) pp->step();

    if(pp != NULL) pp->done();
}

// Public Slots //

void TBTableKeywordsTab::clearWidgetInSplitter() {
    QWidget* widget = rightWidget->getCurrentWidget();
    if(arrayPanel != NULL) {
        arrayPanel->removeActionsAssociatedWithArrays();
        arrayPanel = NULL;
    }
    rightWidget->clearWidget();
    rightWidget->hide();
    rightWidget->setCloseButtonText("Close");
    emit rightWidgetClosed(widget);
}

void TBTableKeywordsTab::showWidgetInSplitter(QWidget* widget) {
    rightWidget->setWidget(widget, true);
    rightWidget->show();
}

// Private Slots //

void TBTableKeywordsTab::doubleClicked(int row, int col) {
    String type = table->keyword(row)->getType();
    if(TBConstants::typeIsTable(type)) {
        ttabs->getBrowser()->openTable(table->keyword(
                             row)->getValue()->asString());
    } else if(TBConstants::typeIsArray(type)) {
        //TBArray* a = new TBArray(row, type, table->keyword(row)->getValue());
        //if(a != NULL && a->isValid()) {
            bool show = false;
            if(arrayPanel == NULL) {
                arrayPanel = new TBArrayPanel(ttabs);
                connect(arrayPanel, SIGNAL(allArraysClosed()),
                        this, SLOT(clearWidgetInSplitter()));
                rightWidget->setCloseButtonText("Close All");
                show = true;
            }
            
            TBArrayData* d = (TBArrayData*)table->keyword(row)->getValue();
            TBViewArray* va = new TBViewArray(ttabs,
                              table->keyword(row)->getName(), "", d, -1, -1,
                              false);
            bool b = arrayPanel->addArray(va, -1);
            if(!b) delete va;
            if(show) showWidgetInSplitter(arrayPanel);
        //}
    } else if(type == TBConstants::TYPE_RECORD) {
        TBDataRecord* r = (TBDataRecord*)table->keyword(row)->getValue();
        TBViewRecord* vr = new TBViewRecord(ttabs, r, table->keyword(row)->getName());
        showWidgetInSplitter(vr);
    }
}

void TBTableKeywordsTab::addKeyword() {
    TBNewKeyword* key = new TBNewKeyword(NULL, this);
    key->exec();
}

void TBTableKeywordsTab::editKeyword() {
    // not implemented
}

void TBTableKeywordsTab::removeKeyword() {
    // not implemented
}

void TBTableKeywordsTab::cellSelected(int row) {
    bool en = row >= 0 && row < tableKeywords->rowCount();
    removeButton->setEnabled(en);
    editButton->setEnabled(en);
}

////////////////////////////////////
// TBFIELDKEYWORDSTAB DEFINITIONS //
////////////////////////////////////

// Constructors/Destructors //

TBFieldKeywordsTab::TBFieldKeywordsTab(TBTableTabs* tt, TBTable* t): QWidget(),
                                       ttabs(tt), table(t), arrayPanel(NULL) {
    setupUi(this);
    widget->close();
    rightWidget = new QCloseableWidget();
    splitter->insertWidget(1, rightWidget);
    rightWidget->hide();
    connect(rightWidget, SIGNAL(closeRequested(QWidget*)),
            this, SLOT(clearWidgetInSplitter()));
    fieldKeywords->header()->setMovable(false);
    connect(fieldKeywords, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this, SLOT(doubleClicked(QTreeWidgetItem*, int)));
}

TBFieldKeywordsTab::~TBFieldKeywordsTab() {
    delete rightWidget;
}

// Accessor Methods //

QTreeWidget* TBFieldKeywordsTab::getTreeWidget() { return fieldKeywords; }

// Public Methods //

void TBFieldKeywordsTab::updateTable(ProgressHelper* pp) {
    int steps = table->getNumFields();
    if(pp != NULL) {
        pp->reset("Updating field keywords...");
        pp->setSteps(steps);
    }

    // Update field keywords
    fieldKeywords->clear();
    vector<TBField*>* fields = table->getFields();

    vector<TBKeyword*>* keywords = NULL;
    // put values in field keywords
    for(unsigned int i = 0; i < fields->size(); i++) {
        keywords = fields->at(i)->getKeywords();

        if(keywords->size() > 0) {
            steps += keywords->size();
            if(pp != NULL) pp->setSteps(steps);
            
            QTreeWidgetItem* kwItem = new QTreeWidgetItem(fieldKeywords);
            kwItem->setFlags(kwItem->flags() & (~Qt::ItemIsEditable));
            kwItem->setText(0, fields->at(i)->getName().c_str());
        
            for(unsigned int j = 0; j < keywords->size(); j++) {
                String name = keywords->at(j)->getName();
                String type = keywords->at(j)->getType();
                String val = keywords->at(j)->getDisplayValue();

                unsigned int k = val.find('\n');
                while(k < val.length()) {
                    val.replace(k, 1, "\t");
                    k = val.find('\n', k + 1);
                }

                if(!name.empty() && !type.empty() && !val.empty()) {
                    QTreeWidgetItem* item = new QTreeWidgetItem(kwItem);
                    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                    item->setText(0, name.c_str());
                    item->setText(1, TBConstants::typeName(type).c_str());
                    item->setText(2, val.c_str());

                    if(TBConstants::typeIsTable(type)) {
                        int r = table->totalRowsOf(val);
                        String str;
                        if(r > 0)
                            str = "Subtable has " + TBConstants::itoa(r) +
                                    " rows.";
                        else
                            str = "Subtable has no rows.";
                        item->setText(3, str.c_str());
                    }
                }

                if(pp != NULL) pp->step();
            }

            fieldKeywords->setItemExpanded(kwItem, true);
        }
        if(pp != NULL) pp->step();
    }

    for(int i = 0; i < fieldKeywords->columnCount(); i++)
        fieldKeywords->resizeColumnToContents(i);

    if(pp != NULL) pp->done();
}

// Public Slots //

void TBFieldKeywordsTab::clearWidgetInSplitter() {
    QWidget* widget = rightWidget->getCurrentWidget();
    if(arrayPanel != NULL) {
        arrayPanel->removeActionsAssociatedWithArrays();
        arrayPanel = NULL;
    }
    rightWidget->clearWidget();
    rightWidget->hide();
    rightWidget->setCloseButtonText("Close");
    emit rightWidgetClosed(widget);
}

void TBFieldKeywordsTab::showWidgetInSplitter(QWidget* widget) {
    rightWidget->setWidget(widget, true);
    rightWidget->show();
}

void TBFieldKeywordsTab::doubleClicked(QTreeWidgetItem* item, int col) {
    QTreeWidgetItem* parent = item->parent();
    if(parent == NULL) return;
    int row = parent->indexOfChild(item);
    String field = qPrintable(parent->text(0));

    TBField* f = table->field(field);
    String type = f->keyword(row)->getType();
    if(TBConstants::typeIsTable(type)) {
        ttabs->getBrowser()->openTable(f->keyword(
                             row)->getValue()->asString());
    } else if(TBConstants::typeIsArray(type)) {
        //TBArray* a= new TBArray(field, row, type, f->keyword(row)->getValue());
        //if(a != NULL && a->isValid()) {
            bool show = false;
            if(arrayPanel == NULL) {
                arrayPanel = new TBArrayPanel(ttabs);
                connect(arrayPanel, SIGNAL(allArraysClosed()),
                        this, SLOT(clearWidgetInSplitter()));
                rightWidget->setCloseButtonText("Close All");
                show = true;
            }
            
            TBArrayData* d = (TBArrayData*)f->keyword(row)->getValue();
            TBViewArray* va = new TBViewArray(ttabs, f->getName(),
                                     TBConstants::itoa(row), d, -1, -1, false);
            bool b = arrayPanel->addArray(va, -1);
            if(!b) delete va;
            if(show) showWidgetInSplitter(arrayPanel);
        //}
    } else if(type == TBConstants::TYPE_RECORD) {
        TBDataRecord* r = (TBDataRecord*)f->keyword(row)->getValue();
        TBViewRecord* vr = new TBViewRecord(ttabs, r, f->getName(), TBConstants::itoa(row));
        showWidgetInSplitter(vr);
    }
}

}
