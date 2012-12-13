//# TBViewArray.cc: Widget for viewing array data in TBArray format.
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
#include <casaqt/QtBrowser/TBViewArray.qo.h>
#include <casaqt/QtBrowser/TBTableTabs.qo.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/TBSlicer.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBFormat.qo.h>
#include <casaqt/QtBrowser/QCloseableWidget.qo.h>
#include <casaqt/QtBrowser/TBData.h>
#include <casaqt/QtBrowser/TBTableDriver.h>
#include <casaqt/QtBrowser/TBKeyword.h>

namespace casa {

//////////////////////////////
// TBVIEWARRAY DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBViewArray::TBViewArray(TBTableTabs* tt, String first, String second,
                         TBArrayData* a, int r, int c, bool e): QWidget(),
                         tTabs(tt), t(tt->getTable()), array(a), editable(e),
                         format(NULL), shouldRelease(true), row(r), col(c) {
    setup(first, second);
    slicer = NULL;
    selectedBackground = tt->getDataTab()->getSelectedCellBackground();
    unselectedBackground = tt->getDataTab()->getUnselectedCellBackground();
}

TBViewArray::~TBViewArray() {
    if(shouldRelease && array->getShape().size() > 1) array->release();
    if(slicer != NULL) delete slicer;
}

// Accessors/Mutators //

TBArrayData* TBViewArray::getArrayData() { return array; }

void TBViewArray::setShouldRelease(bool b) { shouldRelease = b; }

// Public Methods //

TBData* TBViewArray::dataAt(vector<int> d) {
    if(array->coordIsValid(d)) {
        return array->dataAt(d);
    } else return NULL;
}

void TBViewArray::setDataAt(vector<int> d, TBData& newVal, bool f) {
    if(array->coordIsValid(d)) {
        bool oldUpdate = update;
        update = false;

        int row = -1;
        int col = -1;
        bool widgetUpdate = false;
        if(currentSlice.size() > 0) { // dimension > 2
            bool inView = true;
            for(unsigned int i = 0; i < currentSlice.size(); i++) {
                int n = currentSlice.at(i);
                if(n != TBConstants::SLICER_ROW_AXIS &
                   n != TBConstants::SLICER_COL_AXIS && n != d.at(i))
                    inView = false;

                if(n == TBConstants::SLICER_ROW_AXIS) row = d.at(i);
                else if(n == TBConstants::SLICER_COL_AXIS) col = d.at(i);
            }
            if(inView && row >= 0 && col >= 0) {
                widgetUpdate = true;
            }
        } else { // dimension == 2
            row = d.at(0);
            col = d.at(1);
            widgetUpdate = true;
        }

        if(widgetUpdate) {
            if(format == NULL || !f)
                table->item(row, col)->setText(newVal.asString().c_str());
            else {
                format->applyTo(table->item(row, col), &newVal);
            }
        }
        
        array->setDataAt(d, newVal);
        update = oldUpdate;
    }
}

void TBViewArray::applyFormat(TBFormat* f) {
    format = f;
    if(f == NULL) return;

    bool oldUpdate =  update;
    update = false;
    String type = array->getType();
    type = TBConstants::arrayType(type);

    vector<int> d(currentSlice);
    int r = -1;
    int c = -1;
    for(unsigned int i = 0; i < d.size(); i++) {
        if(d.at(i) == TBConstants::SLICER_ROW_AXIS) r = i;
        else if(d.at(i) == TBConstants::SLICER_COL_AXIS) c = i;
    }

    if(currentSlice.size() > 0 && (r == -1 || c == -1)) return;

    if(currentSlice.size() == 0) {
        d.push_back(-1);
        d.push_back(-1);
        r = 0;
        c = 1;
    }

    for(int i = 0; i < table->rowCount(); i++) {
        for(int j = 0; j < table->columnCount(); j++) {
            QTableWidgetItem* item = table->item(i, j);
            d[r] = i;
            d[c] = j;
            TBData* data = array->dataAt(d);
            f->applyTo(item, array->dataAt(d));
            delete data;
        }
    }
    table->resizeColumnsToContents();
    update = oldUpdate;
}

void TBViewArray::clearFormat(QFontColor* f) {
    if(f == NULL) return;

    bool oldUpdate = update;
    update = false;
    vector<int> d(currentSlice);
    int r = -1;
    int c = -1;
    for(unsigned int i = 0; i < d.size(); i++) {
        if(d.at(i) == TBConstants::SLICER_ROW_AXIS) r = i;
        else if(d.at(i) == TBConstants::SLICER_COL_AXIS) c = i;
    }

    if(currentSlice.size() > 0 && (r == -1 || c == -1)) return;

    if(currentSlice.size() == 0) {
        d.push_back(-1);
        d.push_back(-1);
        r = 0;
        c = 1;
    }

    for(int i = 0; i < table->rowCount(); i++) {
        for(int j = 0; j < table->columnCount(); j++) {
            QTableWidgetItem* item = table->item(i, j);
            d[r] = i;
            d[c] = j;
            TBData* data = array->dataAt(d);
            item->setFont(f->font);
            item->setForeground(QBrush(f->color));
            item->setText(data->asString().c_str());
            delete data;
        }
    }
    table->resizeColumnsToContents();
    update = oldUpdate;
}


// Protected Methods //

void TBViewArray::contextMenuEvent(QContextMenuEvent* event) {
    // Make sure right-click is on table widget.
    QPoint pos = event->globalPos();
    QRect rect(table->mapToGlobal(QPoint(0, 0)), table->size());
    if(rect.contains(pos)) {
        // Make sure a cell is selected (shouldn't be necessary).
        if(table->currentRow() < 0 || table->currentColumn() < 0) {
            event->ignore();
            return;
        }
        
        // Show menu.
        QMenu* menu = new QMenu();
        QAction* copy = menu->addAction("Copy");
        connect(copy, SIGNAL(triggered()), SLOT(copyData()));
        menu->exec(pos);
        event->accept();
        
    } else event->ignore();
}


// Private Methods //

void TBViewArray::setup(String first, String second) {
    setupUi(this);
    update = false;
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    stringstream ss;
    ss << t->getName() << "[" << first;
    if(!second.empty()) ss << ", " << second;
    ss << ']';
        
    vector<int> shape = array->getShape();
    if(shape.size() == 1) {
        shape.resize(2);
        shape[1] = shape[0];
        shape[0] = 1;
    }           
    int n = shape.size();
    
    String t = array->getType();
    ss << " = " << TBConstants::typeName(t) << " of size [ ";
    for(unsigned int i = 0; i < shape.size(); i++) {
        ss << shape.at(i) << " ";
    }
    ss << "].";
    label->setText(ss.str().c_str());

    int rows = 0;
    int cols = 0;
    
    if(n == 2) {
        rows = shape.at(0);
        cols = shape.at(1);

        blankWidget->hide();
    } else {
        QGridLayout* gl = (QGridLayout*)layout();
        gl->removeWidget(blankWidget);

        slicer = new TBSlicer(shape);
        gl->addWidget(slicer, 2, 0, 1, 1);
        connect(slicer, SIGNAL(sliceChanged(vector<int>)),
                this, SLOT(sliceChanged(vector<int>)));
        
        rows = shape.at(n - 2);
        cols = shape.at(n - 1);

        for(int i = 0; i < n - 2; i++) {
            currentSlice.push_back(0);
        }
        currentSlice.push_back(TBConstants::SLICER_ROW_AXIS);
        currentSlice.push_back(TBConstants::SLICER_COL_AXIS);
    }

    if(rows > 0 && cols > 0) { //&& data != NULL) {
        table->setRowCount(rows);
        table->setColumnCount(cols);
        
        vector<int> d(currentSlice);
        if(d.size() == 0) {
            d.resize(2);
        }
        int x = d.size() - 2;
        int y = d.size() - 1;
        TBData* val;
        for(int r = 0; r < rows; r++) {
            d[x] = r;
            for(int c = 0; c < cols; c++) {
                d[y] = c;
                val = array->dataAt(d);
                QTableWidgetItem* item = new QTableWidgetItem(
                                             val->asString().c_str());
                if(!editable)
                    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                table->setItem(r, c, item);
                delete val;
            }
        }
            
        table->resizeColumnsToContents();
        table->resizeRowsToContents();
        relabelHeaders();
    }
    
    setWindowTitle(tTabs->getName().c_str());
    update = true;

    if(editable){
        connect(table, SIGNAL(cellChanged(int, int)),
                this, SLOT(dataChanged(int, int)));
        connect(table, SIGNAL(currentCellChanged(int, int, int, int)),
            this, SLOT(cellClicked(int, int)));
        connect(table, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(cellDoubleClicked(int, int)));
    }
}

vector<int> TBViewArray::currentCell(int row, int col) {
    vector<int> d;
    if(currentSlice.size() > 0) {
        for(unsigned int i = 0; i < d.size(); i++) {
            if(d.at(i) == TBConstants::SLICER_ROW_AXIS) d[i] = row;
            else if(d.at(i) == TBConstants::SLICER_COL_AXIS) d[i] = col;
        }
    } else {
        d.push_back(row);
        d.push_back(col);
    }
    return d;
}

void TBViewArray::relabelHeaders() {
    // set up headers to be 0-based rather than 1-based
    QStringList headers;
    int rows = table->rowCount();
    for(int i = 0; i < rows; i++)
        headers << TBConstants::itoa(i).c_str();
    table->setVerticalHeaderLabels(headers);

    headers.clear();
    int cols = table->columnCount();
    for(int i = 0; i < cols; i++)
        headers << TBConstants::itoa(i).c_str();
    table->setHorizontalHeaderLabels(headers);
}

// Private Slots //

void TBViewArray::sliceChanged(vector<int> newSlice) {
    int rowIndex = -1;
    int colIndex = -1;

    for(unsigned int i = 0; i < newSlice.size(); i++) {
        if(newSlice.at(i) == TBConstants::SLICER_ROW_AXIS) {
            // ensure no more than one ROW_AXIS
            if(rowIndex > -1) return;
            rowIndex = i;
        }
        else if(newSlice.at(i) == TBConstants::SLICER_COL_AXIS) {
            // ensure no more than one COL_AXIS
            if(colIndex > -1) return;
            colIndex = i;
        }
    }

    // ensure exactly one ROW_AXIS and COL_AXIS
    if(rowIndex > -1 && colIndex > -1) {
        bool oldUpdate = update;
        update = false;
        table->clear();
        int x = array->getShape().at(rowIndex);
        int y = array->getShape().at(colIndex);
        
        table->setRowCount(x);
        table->setColumnCount(y);
        
        vector<int> d(newSlice);
        TBData* data;
        for(int r = 0; r < x; r++) {
            for(int c = 0; c < y; c++) {
                d[rowIndex] = r;
                d[colIndex] = c;
                
                data = array->dataAt(d);
                
                QTableWidgetItem* item = new QTableWidgetItem(
                                             data->asString().c_str());
                if(!editable)
                    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                table->setItem(r, c, item);
                delete data;
            }
        }
        currentSlice = vector<int>(newSlice);

        if(format != NULL) applyFormat(format);
        
        table->resizeRowsToContents();
        table->resizeColumnsToContents();
        relabelHeaders();
        
        update = oldUpdate;
    }
}

void TBViewArray::dataChanged(int r, int c) {   
    
    if(!update || r == -1 || c == -1) return;

    vector<int> d;
    if(currentSlice.size() > 0) { // dimension > 2
        for(unsigned int i = 0; i < currentSlice.size(); i++) {
            int n = currentSlice.at(i);
            if(n == TBConstants::SLICER_ROW_AXIS) d.push_back(r);
            else if(n == TBConstants::SLICER_COL_AXIS) d.push_back(c);
            else d.push_back(n);
        }
    } else {
        d.push_back(r);
        d.push_back(c);
    }
        
    TBData* oldVal = array->dataAt(d);
    String newVal = qPrintable(table->item(r, c)->text());
    stringstream ss;
    ss << tTabs->getFileName() << ": editing (" << row << "," << col << ")[";
    for(unsigned int i = 0; i < d.size(); i++)
        ss << d[i] << (i < d.size() - 1 ? " " : "");
    ss << "] from \"" << oldVal->asString() << "\" to \"" << newVal << "\"...";

    Result* result = NULL;
    if(tTabs->isEditable()) {
        String at = array->getType();
        at = TBConstants::arrayType(at);
        newVal = TBConstants::formatValue(newVal, at);
        if(TBConstants::valueIsValid(newVal, at)) {
            TBData* nd = TBData::create(newVal, at);
            TBEditArrayDataAction* action = new TBEditArrayDataAction(tTabs,
                this, row, col, d, nd);
            Result rTemp = tTabs->getBrowser()->doAction(action);
            result = new Result(String(rTemp.result), rTemp.valid);
            
        } else {
            result = new Result(newVal + " is not a valid value for type " +
                                TBConstants::typeName(at) + ".", false);
        }
    }

    if(result != NULL && !result->valid) {
        tTabs->getBrowser()->displayError(result->result);

        bool oldUpdate = update;
        update = false;
        if(format == NULL)
            table->item(r, c)->setText(oldVal->asString().c_str());
        else {
            String t = array->getType();
            t = TBConstants::arrayType(t);
            format->applyTo(table->item(r, c), oldVal);
        }
        update = oldUpdate;
    }
    
    ss << (result == NULL || !result->valid ? "failed!" : "succeeded!");
    TBConstants::dprint(TBConstants::DEBUG_HIGH, ss.str());
    cerr << ss.str() << endl;
    if(result != NULL) delete result;
    delete oldVal;
}

void TBViewArray::cellClicked(int row, int col) {
    bool oldUpdate = update;
    update = false;
    for(unsigned int i = 0; i < selectedCells.size(); i++)
        selectedCells.at(i)->setBackground(unselectedBackground);
    selectedCells.clear();

    if(row == -1 || col == -1) {
        update = oldUpdate;
        return;
    }

    for(int r = 0; r < table->rowCount(); r++) {
        if(r != row) {
            QTableWidgetItem* item = table->item(r, col);
            item->setBackground(selectedBackground);
            selectedCells.push_back(item);
        }
    }

    for(int c = 0; c < table->columnCount(); c++) {
        if(c != col) {
            QTableWidgetItem* item = table->item(row, c);
            item->setBackground(selectedBackground);
            selectedCells.push_back(item);
        }
    }
    update = oldUpdate;
}

void TBViewArray::cellDoubleClicked(int row, int col) {
    if(tTabs->getTable()->isEditable(this->col)) {  
        if(tTabs->isEditable()) {
            // clear formatting for editing
            vector<int> cell = currentCell(row, col);
            TBData* d = array->dataAt(cell);
            setDataAt(cell, *d, true);
            delete d;
        } else if(editable) {
            String message = "Browser is not in editor mode.\n";
            message += "Toggle this table in Edit->Table if you wish to edit.";
            tTabs->getBrowser()->displayError(message);
        }
    } else {
        String message = tTabs->getTable()->isAnyEditable() ?
                         "This column does not allow editing." :
                         "This table does not allow editing.";
        tTabs->getBrowser()->displayError(message);
    }
}

void TBViewArray::copyData() {
    QList<QTableWidgetItem*> sel = table->selectedItems();
    if(sel.size() < 1) return;
    QApplication::clipboard()->setText(sel[0]->text());
}


//////////////////////////////
// TBARRAYPANEL DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBArrayPanel::TBArrayPanel(TBTableTabs* tt): ttabs(tt), splitter(Qt::Vertical) {
    setupUi(this);
    gridLayout->addWidget(&splitter, 0, 0, 1, 1);
}

TBArrayPanel::~TBArrayPanel() {
    // QCloseableWidget will delete the TBViewArrays
    for(unsigned int i = 0; i < widgets.size(); i++) {
        delete widgets.at(i);
    }
}

// Public Methods //

bool TBArrayPanel::addArray(TBViewArray* array, int colIndex) {
    for(unsigned int i = 0; i < arrays.size(); i++)
        if(arrays.at(i)->getArrayData() == array->getArrayData())
            return false;

    arrays.push_back(array);
    QCloseableWidget* w = new QCloseableWidget(array);
    connect(w, SIGNAL(closeRequested(QWidget*)),
            this, SLOT(closeRequested(QWidget*)));
    widgets.push_back(w);
    indices.push_back(colIndex);
    splitter.addWidget(w);
    return true;
}

void TBArrayPanel::setShouldRelease(bool b) {
    for(unsigned int i = 0; i < arrays.size(); i++)
        arrays.at(i)->setShouldRelease(b);
}

void TBArrayPanel::applyFormat(TBFormat* format, int index) {
    for(unsigned int i = 0; i < indices.size(); i++) {
        if(indices[i] == index) {
            arrays.at(i)->applyFormat(format);
        }
    }
}

// Public Slots //

void TBArrayPanel::removeActionsAssociatedWithArrays() {
    TBBrowser* browser = ttabs->getBrowser();
    for(unsigned int i = 0; i < arrays.size(); i++) {
        browser->removeActionsAssociatedWithWidget(arrays.at(i));
    }
}

// Private Methods //

void TBArrayPanel::removeActionsAssociatedWithArray(TBViewArray* va) {
    if(va == NULL) return;
    TBBrowser* browser = ttabs->getBrowser();
    browser->removeActionsAssociatedWithWidget(va);
}

// Private Slots //

void TBArrayPanel::closeRequested(QWidget* widget) {
    TBViewArray* va = (TBViewArray*)widget;
    
    unsigned int i = 0;
    for(; i < arrays.size(); i++) {
        if(arrays.at(i) == va) {
            break;
        }
    }
    
    if(i < arrays.size()) {
        removeActionsAssociatedWithArray(va);
        QCloseableWidget* w = widgets.at(i);
        w->close();
        delete w;
        //delete va;
        arrays.erase(arrays.begin() + i);
        widgets.erase(widgets.begin() + i);
        indices.erase(indices.begin() + i);
        
        if(arrays.size() == 0) emit allArraysClosed();
    }
}


//////////////////////////////
// TBVIEWRECORD DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBViewRecord::TBViewRecord(TBTableTabs* t, TBDataRecord* r, String first,
                           String second) : tt(t), record(*r->asRecord()) {
    setupUi(this);
    stringstream ss;
    ss << t->getName() << "[" << first;
    if(!second.empty()) ss << ", " << second;
    ss << "] = Record of size " << record.nfields() << ".";
    label->setText(ss.str().c_str());
    
    blankWidget->close();
    fill(*table, record, first, second);
}

TBViewRecord::~TBViewRecord() {
    
}

// Private Methods //

void TBViewRecord::fill(QTableWidget& t, Record& r, String f, String s) {
    QStringList cols;
    t.setColumnCount(3);
    t.setSelectionBehavior(QAbstractItemView::SelectRows);
    cols << "Name" << "Type" << "Value";
    t.setHorizontalHeaderLabels(cols);
    t.horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
    
    vector<TBKeyword*>* rfields = TBTableDriverDirect::getKeywords(r);
    if(rfields != NULL) {
        t.setRowCount(rfields->size());
        
        for(unsigned int i = 0; i < rfields->size(); i++) {
            TBKeyword* k = rfields->at(i);
            
            String name = k->getName();
            String type = k->getType();
            TBData* data = k->getValue();
            if(name.empty() || type.empty() || data == NULL)
                continue;
            
            QTableWidgetItem* item = new QTableWidgetItem(name.c_str());
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            t.setItem(i, 0, item);
            
            item = new QTableWidgetItem((TBConstants::typeName(type)).c_str());
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            t.setItem(i, 1, item);
            
            if(TBConstants::typeIsArray(type)) {
                TBArrayData* ad = (TBArrayData*)data;
                vector<int> shape = ad->getShape();
                if(shape.size() >= 2 && ad->isLoaded()) {
                    TBViewArray* va = new TBViewArray(tt, f, s, ad,
                                                      -1, -1, false);
                    va->blockSignals(true);
                    t.setCellWidget(i, 2, va);
                } else {
                    item = new QTableWidgetItem(data->asString().c_str());
                    item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                    t.setItem(i, 2, item);
                }
            } else if(type == TBConstants::TYPE_RECORD) {
                QTableWidget* tw = new QTableWidget();
                fill(*tw, *data->asRecord(), f, s);
                
                // Make sure some record rows are visible.
                unsigned int row = data->asRecord()->nfields();
                if(row > TBConstants::DEFAULT_RECORD_VISIBLE_ROWS)
                    row = TBConstants::DEFAULT_RECORD_VISIBLE_ROWS;
                int height = tw->rowViewportPosition(row);
                if(height < 0) {
                    height = tw->rowViewportPosition(row - 1);
                    height += tw->rowHeight(row - 1);
                }
                height += tw->horizontalHeader()->height() + 5;
                
                t.setCellWidget(i, 2, tw);
                t.setRowHeight(i, height);
                
            } else {
                item = new QTableWidgetItem(data->asString().c_str());
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
                t.setItem(i, 2, item);
            }
        }
        
        for(unsigned int i = 0; i < rfields->size(); i++)
            delete rfields->at(i);
        delete rfields;
    }
}

}
