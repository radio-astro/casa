//# TBDataTab.cc: Widget used to display table data.
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
#include <casaqt/QtBrowser/TBDataTab.qo.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/QCloseableWidget.qo.h>
#include <casaqt/QtBrowser/TBKeyword.h>
#include <casaqt/QtBrowser/TBFilterRules.qo.h>
#include <casaqt/QtBrowser/TBFormat.qo.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/TBViewArray.qo.h>
#include <casaqt/QtBrowser/TBTableTabs.qo.h>
#include <casaqt/QtBrowser/QProgressPanel.qo.h>
#include <casaqt/QtBrowser/TBData.h>

namespace casa {

////////////////////////////
// TBDATAITEM DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

TBDataItem::TBDataItem(TBData* d): QTableWidgetItem(),
                                      arrayDimension(false), tdata(d) {
    setText(d->asString().c_str());
    String type = d->getType();
    if(TBConstants::typeIsArray(type)) {
        TBArrayData* d2 = (TBArrayData*)d;
        if(d2->isLoaded()) {
            TBData* d3 = d2->firstItem();
            if(d3 != NULL) {
                tdata = d3;
                arrayDimension = true;
            }
        } else {
            vector<int> s = d2->getShape();
            if(s.size() > 0) {
                tdata = new TBDataInt(s[0]);
                arrayDimension = true;
            }
        }
    }
}

TBDataItem::~TBDataItem() {
    if(arrayDimension) delete tdata;
}

// Operators //

bool TBDataItem::operator<(const QTableWidgetItem& other) const {
    TBData* o = ((TBDataItem*)&other)->tdata;
    
    String type = tdata->getType();
    
    if(type == TBConstants::TYPE_DOUBLE || type == TBConstants::TYPE_DATE) {
        return tdata->asDouble() < o->asDouble();
    } else if(type == TBConstants::TYPE_FLOAT) {
        return tdata->asFloat() < o->asFloat();
    } else if(type == TBConstants::TYPE_INT) {
        return tdata->asInt() < o->asInt();
    } else if(type == TBConstants::TYPE_UINT) {
        return tdata->asUInt() < o->asUInt();
    } else if(type == TBConstants::TYPE_BOOL) {
        return tdata->asBool() < o->asBool();
    } else if(type == TBConstants::TYPE_CHAR) {
        return tdata->asChar() < o->asChar();
    } else if(type == TBConstants::TYPE_UCHAR) {
        return tdata->asUChar() < o->asUChar();
    } else if(type == TBConstants::TYPE_SHORT) {
        return tdata->asShort() < o->asShort();
    } else if(type == TBConstants::TYPE_COMPLEX) {
        pair<float, float> c1 = tdata->asComplex(), c2 = o->asComplex();
        bool b = c1.first < c2.first;
        if(b || c1.first != c2.first) return b;
        else return c1.second < c2.second;      
    } else if(type == TBConstants::TYPE_DCOMPLEX) {
        pair<double, double> c1 = tdata->asDComplex(), c2 = o->asDComplex();
                bool b = c1.first < c2.first;
                if(b || c1.first != c2.first) return b;
                else return c1.second < c2.second;  
    } else return text() < other.text();
}

// Private Methods //

void TBDataItem::set(String value, bool text) {
    String t = type;
    if(TBConstants::typeIsArray(t)) {
        // Check if the value is an array element (one-dimensional arrays) or
        // a dimension (all other arrays).
        
        int dim;
        if(TBConstants::atoi(value, &dim) == 1) {
            // the value is a dimension, so set it as an int
            arrayDimension = true;
            setData(Qt::UserRole, dim);
            return;
        }
        
        t = TBConstants::arrayType(t);
    }
    
    if(t == TBConstants::TYPE_INT || t == TBConstants::TYPE_UINT ||
       t == TBConstants::TYPE_SHORT) {
        double d = TBConstants::valueToDouble(value, t);
        if(t == TBConstants::TYPE_UINT) {
            unsigned int i = (unsigned int)d;
            if(text) setData(Qt::DisplayRole, i);
            setData(Qt::UserRole, i);
        } else {
            int i = (int)d;
            if(text) setData(Qt::DisplayRole, i);
            setData(Qt::UserRole, i);
        }
        
    } else if(t == TBConstants::TYPE_FLOAT || t == TBConstants::TYPE_DOUBLE) {
        double d = TBConstants::valueToDouble(value, t);
        if(text) setData(Qt::DisplayRole, d);
        setData(Qt::UserRole, d);
        
    } else if(t == TBConstants::TYPE_DATE || t == TBConstants::TYPE_BOOL) {
        if(text) setText(value.c_str());
        setData(Qt::UserRole, TBConstants::valueToDouble(value, t));
        
    } else if(t== TBConstants::TYPE_COMPLEX || t== TBConstants::TYPE_DCOMPLEX){
        if(text) setText(value.c_str());
        pair<double, double> p = TBConstants::toComplex(value);
        setData(Qt::UserRole, p.first);
        
    } else {
        if(text) setText(value.c_str());
        setData(Qt::UserRole, value.c_str());
    }
}

///////////////////////////
// TBDATATAB DEFINITIONS //
///////////////////////////

// Constructors/Destructors //

TBDataTab::TBDataTab(TBTableTabs* tt) : QWidget(), ttabs(tt),
                    table(tt->getTable()), page(0),
                    loadRows(TBConstants::DEFAULT_SELECT_NUM), filter(NULL),
                    defaultFormat(NULL), arrayOpened(false), arrayPanel(NULL) {
    setupUi(this);
    
    // Connect widgets
    connect(backButton, SIGNAL(clicked()), this, SLOT(pageBack()));
    connect(forwardButton, SIGNAL(clicked()), this, SLOT(pageForward()));
    connect(firstButton, SIGNAL(clicked()), this, SLOT(pageFirst()));
    connect(lastButton, SIGNAL(clicked()), this, SLOT(pageLast()));
    connect(goButton, SIGNAL(clicked()), this, SLOT(pageGo()));
    connect(tableWidget, SIGNAL(cellChanged(int, int)),
            this, SLOT(notifyDataChanged(int, int)));
    connect(tableWidget, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(doubleClicked(int, int)));         
    connect(tableWidget, SIGNAL(currentCellChanged(int, int, int, int)),
            this, SLOT(cellClicked(int, int)));
    connect(resizeButton, SIGNAL(clicked()), this, SLOT(resizeHeaders()));
    connect(tableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(headerClicked(int)));
    connect(restoreColsButton, SIGNAL(clicked()),this, SLOT(restoreColumns()));
    connect(tableWidget->horizontalHeader(),SIGNAL(sectionMoved(int,int, int)),
            this, SLOT(headerMoved()));
    
    // Set up GUI components
    loadEdit->setText(TBConstants::itoa(loadRows).c_str());
    loadEdit->setValidator(new QIntValidator(1, TBConstants::MAX_SELECT_NUM,
                           loadEdit));

    sideWidget->close();
    rightWidget = new QCloseableWidget();
    splitter->insertWidget(1, rightWidget);
    rightWidget->hide();
    connect(rightWidget, SIGNAL(closeRequested(QWidget*)),
            this, SLOT(clearWidgetInSplitter()));

    selectedBackground = QBrush(tableWidget->palette().alternateBase());

    tableWidget->horizontalHeader()->setMovable(true);
    tableWidget->horizontalHeader()->setClickable(true);
    clearSortIndicator();
}

TBDataTab::~TBDataTab() {
    delete rightWidget;
    
    for(unsigned int i = 0; i < formats.size(); i++)
        if(formats.at(i) != NULL) delete formats.at(i);
        
    if(defaultFormat != NULL) delete defaultFormat;
}

// Accessors/Mutators //

QTableWidget* TBDataTab::getTableWidget() { return tableWidget; }

QBrush& TBDataTab::getSelectedCellBackground() { return selectedBackground; }

QBrush& TBDataTab::getUnselectedCellBackground() {
    return unselectedBackground;
}

TBFilterRuleSequence* TBDataTab::getFilter() { return filter; }

void TBDataTab::setFilter(TBFilterRuleSequence* f) {
    bool hadFilter = filter != NULL;
    filter = f;
    if(filter != NULL && filter->size() > 0) {
        // Update label
        vector<String> fields;
        bool found;
        for(unsigned int i = 0; i < filter->size(); i++) {
            found = false;
            String field = filter->at(i)->getField();
            for(unsigned int j = 0; j < fields.size(); j++) {
                if(fields[j] == field) {
                    found = true;
                    break;
                }
            }
            if(!found) fields.push_back(field);
        }
        
        stringstream ss;
        ss << "Filtering rows on: ";
        for(unsigned int i = 0; i < fields.size(); i++) {
            ss << fields[i];
            if(i < fields.size() - 1) ss << ", ";
        }
        ss << ".";
        filterLabel->setText(ss.str().c_str());
        
        applyFilter();
    } else {
        if(hadFilter) applyFilter();        
        filterLabel->setText("");
    }
}

TBFormat* TBDataTab::formatAt(int index) {
    if(index < 0 || (unsigned int)index >= formats.size()) return NULL;
    else return formats.at(index);
}

vector<pair<String, bool> >* TBDataTab::getSortFields() { return &currSort; }

// Public Methods //

void TBDataTab::updateTable(ProgressHelper* pp) {
    if(pp == NULL) QCoreApplication::processEvents();
    if(pp != NULL) pp->reset("Updating data display...");
    int steps = (table->getLoadedRows() / 10) + 2 + table->getNumFields();
    if(filter != NULL) steps += table->getLoadedRows();
    if(pp != NULL) pp->setSteps(steps);

    stringstream ss;
    ss << "\tTotal rows: " << table->getTotalRows() << "\nTotal cols: ";
    ss << table->getNumFields() << "\nLoaded rows: " << table->getLoadedRows();
    ss << "\nTotal pages: " << table->getNumPages() << '\n';
    TBConstants::dprint(TBConstants::DEBUG_MED, ss.str());

    page = table->getPage();

    tableWidget->blockSignals(true);
    if(pp == NULL) ttabs->setEnabled(false);
    
    // update table parameters
    QStringList cols;
    vector<String> fields = table->getColumnHeaders();
    for(unsigned int i = 0; i < fields.size(); i++)
        cols << fields.at(i).c_str();
    
    tableWidget->setCurrentCell(-1, -1);
    tableWidget->clear();
        
    vector<pair<String, bool> > sortCopy(currSort);
    currSort.clear();
    clearSortIndicator();
    
    tableWidget->setRowCount(table->getLoadedRows());
    tableWidget->setColumnCount(table->getNumFields());
    tableWidget->setHorizontalHeaderLabels(cols);
    
    // Set column tooltips
    for(int i = 0; i < tableWidget->columnCount(); i++)
        tableWidget->horizontalHeaderItem(i)->setToolTip(
                table->fieldToolTip(i).c_str());

    for(int i = 0; i < tableWidget->rowCount(); i++)
        tableWidget->showRow(i);
    if(pp != NULL) pp->step(); 
    
    QStringList rows;
    fields = table->getRowHeaders();
    for(unsigned int i = 0; i < fields.size(); i++)
        rows << fields.at(i).c_str();
    tableWidget->setVerticalHeaderLabels(rows);
    if(pp != NULL) pp->step();

    rowItems.resize(0);
    rowIndices.clear();

    // put values in table
    for(int i = 0; i < table->getLoadedRows(); i++) {
        for(int j = 0; j < table->getNumFields(); j++) {
            TBData* val = table->dataAt(i, j);
            String type = val->getType();
            QTableWidgetItem* item = new TBDataItem(val);
            if(TBConstants::typeIsArray(type) ||
               TBConstants::typeIsTable(type) ||
               type == TBConstants::TYPE_RECORD)
                item->setFlags(item->flags() & (~Qt::ItemIsEditable));
            tableWidget->setItem(i, j, item);
        }
        rowItems.push_back(tableWidget->item(i, 0));
        if(pp != NULL && (i % 10 == 0)) pp->step();
        
        // refresh gui
        if(pp == NULL && i % 100 == 99) QCoreApplication::processEvents();
    }
    tableWidget->setEnabled(true);
    if(tableWidget->rowCount() > 0 && tableWidget->columnCount() > 0)
        unselectedBackground = QBrush(tableWidget->item(0, 0)->background());

    updatePageTrackers();
    loadRows = table->getSelectedRows();
    loadEdit->setText(TBConstants::itoa(loadRows).c_str());
    clearWidgetInSplitter();
    arrayOpened = false;

    pageEdit->setValidator(new QIntValidator(1,table->getNumPages(),pageEdit));

    // sort and filter
    if(!sortCopy.empty()) sortBy(sortCopy);
    if(filter != NULL) {
        applyFilter();
    }

    if((unsigned int)tableWidget->columnCount() != formats.size()) {
        for(unsigned int i = 0; i < formats.size(); i++)
            delete formats.at(i);
        formats.clear();
        for(int i = 0; i < tableWidget->columnCount(); i++) {
            formats.push_back(NULL);
            if(pp != NULL) pp->step();
        }
    } else {
        for(int i = 0; i < tableWidget->columnCount(); i++) {
            if(formats.at(i) != NULL) applyFormat(i, formats.at(i));
            if(pp != NULL) pp->step();
        }
    }
    
    if(defaultFormat == NULL && tableWidget->rowCount() > 0 &&
       tableWidget->columnCount() > 0) {
        QTableWidgetItem* i = tableWidget->item(0, 0);
        QFont f = i->font();
        QColor c = i->foreground().color();
        defaultFormat = new QFontColor(f, c);
    }

    highlightedCells.clear();
    tableWidget->setCurrentCell(-1, -1);
    
    if(pp != NULL) pp->done();
    
    tableWidget->blockSignals(false);
    if(pp == NULL) ttabs->setEnabled(true);
}

void TBDataTab::setData(int row, int col, TBData* newVal, bool format) {
    if(newVal == NULL) return;
    
    if(row >= tableWidget->rowCount()) row -= table->getRowIndex();
    if(row < 0 || row >= tableWidget->rowCount()) return;

    tableWidget->blockSignals(true);
    
    if(rowIndices.size() > 0)
        row = rowItems.at(row)->row();
    
    if(format && formats.at(col) != NULL) {
        formats.at(col)->applyTo(tableWidget->item(row, col), newVal);
    } else {
        tableWidget->item(row, col)->setText(newVal->asString().c_str());
    }
    tableWidget->blockSignals(false);
}

bool TBDataTab::loadPage(int p) {
    // check loadRows
    int l;
    String str = qPrintable(loadEdit->text());
    int v = TBConstants::atoi(str, &l);
    if(v == EOF || v == 0 || l <= 0 || l == loadRows) {
        loadEdit->setText(TBConstants::itoa(loadRows).c_str());
        l = loadRows;
    }
        
    if((p != page || l != loadRows) && 0 <= p && p < table->getNumPages()) {
        highlightedCells.clear();
        if(arrayPanel != NULL) {
            arrayPanel->setShouldRelease(false);
        }
        bool b = ttabs->loadRows(p * loadRows, l);
        loadRows = l;
        updatePageTrackers();
        return b;
    }
    return false;
}

void TBDataTab::refresh(int row, int col, bool format) {
    setData(row, col, table->dataAt(row, col), format);
}

void TBDataTab::sortBy(vector<pair<String, bool> >& s) {
    currSort.clear();
    for(unsigned int i = 0; i < s.size(); i++)
        currSort.push_back(s.at(i));
        
    if(s.empty()) {
        if(!rowIndices.empty()) clearSort();
        return;
    }

    for(int i = s.size() - 1; i >= 0; i--) {
        String f = s.at(i).first;
        for(unsigned int j = 0; j < table->getFields()->size(); j++) {
            if(f == table->getFields()->at(j)->getName()) {
                sort(j, s.at(i).second);
                break;
            }
        }
    }
    emit sortEntered();
}

void TBDataTab::setSortIndicator(int index, bool asc) {
    tableWidget->horizontalHeader()->setSortIndicatorShown(true);
    tableWidget->horizontalHeader()->setSortIndicator(index,
                        (asc?Qt::AscendingOrder:Qt::DescendingOrder));
}

void TBDataTab::clearSortIndicator() {
    tableWidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
}

void TBDataTab::highlight(int row) {
    if(row >= tableWidget->rowCount()) row -= table->getRowIndex();
    
    int visRow = visibleIndex(row);
    int col = tableWidget->currentColumn();
    if(col == -1) col = 0;
    tableWidget->setCurrentCell(visRow, col);
}

int TBDataTab::visibleIndex(int logicalIndex) {
    if(logicalIndex < 0 || logicalIndex >= (int)rowIndices.size() ||
       rowIndices.size() == 0)
        return logicalIndex;
    else return rowItems.at(logicalIndex)->row();
}

int TBDataTab::logicalIndex(int visibleIndex) {
    if(visibleIndex < 0 || visibleIndex >= (int)rowIndices.size() ||
       rowIndices.size() == 0)
        return visibleIndex;
    else return rowIndices.at(visibleIndex);
}

void TBDataTab::hideColumn(int index, bool hidden) {
    if(index < 0 || index > tableWidget->columnCount()) return;
    tableWidget->setColumnHidden(index, hidden);
    emit columnHidden(index, hidden);
}

void TBDataTab::formatField(int index) {
    if(index < 0 || index > tableWidget->columnCount()) return;
    TBField* field = table->field(index);
    TBFormatter* f = new TBFormatter(field->getName(), field->getType(), index,
                                QFontColor(font(), palette().color(
                                foregroundRole())), NULL);

    TBFormat* fm = formatAt(index);
    if(fm != NULL) f->setFormat(fm);
    connect(f, SIGNAL(clearRequested(int)), this, SLOT(clearFormat(int)));
    connect(f, SIGNAL(setRequested(int, TBFormat*)),
            this, SLOT(applyFormat(int, TBFormat*)));
    
    //f->setWindowModality(Qt::WindowModal);
    f->exec();
}

// Public Slots //

void TBDataTab::showWidgetInSplitter(QWidget* widget, bool isArray) {
    rightWidget->setWidget(widget, true);
    rightWidget->show();
    arrayOpened = isArray;
}

void TBDataTab::clearWidgetInSplitter() {
    QWidget* widget = rightWidget->getCurrentWidget();
    if(arrayPanel != NULL) {
        arrayPanel->removeActionsAssociatedWithArrays();
        arrayPanel = NULL;
    }
    rightWidget->clearWidget();
    rightWidget->hide();
    rightWidget->setCloseButtonText("Close");
    arrayOpened = false;
    emit rightWidgetClosed(widget);
}

void TBDataTab::applyFormat(int index, TBFormat* format) {
    if(format == NULL) {
        clearFormat(index);
        return;
    }
    if(index < 0 || (unsigned int)index >= formats.size()) return;

    formats[index] = format;
    
    tableWidget->blockSignals(true);
    for(int i = 0; i < tableWidget->rowCount(); i++) {
        QTableWidgetItem* item = tableWidget->item(i, index);
        format->applyTo(item, table->dataAt(i, index));
    }
    
    int r = tableWidget->currentRow();
    if(r < 0) r = 0;
    tableWidget->setCurrentCell(r, index);
    
    // check if a viewing array is open or not
    if(arrayOpened) {
        ((TBArrayPanel*)rightWidget->getCurrentWidget())->applyFormat(format,
                                                                      index);
    }
    tableWidget->resizeColumnToContents(index);
    
    tableWidget->blockSignals(false);
}

void TBDataTab::clearFormat(int index) {
    if(0 <= index && (unsigned int)index < formats.size()) {
        if(formats.at(index) != NULL) delete formats.at(index);
        formats[index] = NULL;

        if(defaultFormat == NULL) return; // shouldn't happen

        tableWidget->blockSignals(true);
        for(int i = 0; i < tableWidget->rowCount(); i++) {
            QTableWidgetItem* item = tableWidget->item(i, index);
            item->setFont(defaultFormat->font);
            item->setForeground(QBrush(defaultFormat->color));
            String data = table->dataAt(i, index)->asString();
            item->setText(data.c_str());
        }
        if(arrayOpened) {
           ((TBViewArray*)rightWidget->getCurrentWidget())->clearFormat(
                   defaultFormat);
        }
        tableWidget->resizeColumnToContents(index);
        tableWidget->blockSignals(false);
    }
}

void TBDataTab::clearAllFormats() {
    for(unsigned int i = 0; i < formats.size(); i++)
        clearFormat(i);
}

// Protected Methods //

void TBDataTab::contextMenuEvent(QContextMenuEvent* event) {    
    QPoint pos = event->pos();
    
    // should be able to just do, for example, tableWidget->horizontalHeader()
    // ->rect().contains(pos), but Qt doesn't correctly report the size of
    // the header as of 4.3.2, so do it using x/y coordinates instead.
    
    bool hx = pos.x() >= tableWidget->verticalHeader()->width();
    bool tx = pos.x() <= tableWidget->width();
    bool hy = pos.y() <= tableWidget->horizontalHeader()->height();
    
    // right click on table column headers
    if(hx && tx && hy) {
        int col = tableWidget->horizontalHeader()->visualIndexAt(event->x());
        col = tableWidget->horizontalHeader()->logicalIndex(col);
        if(col < 0) return;
        
        clickedHeader = col;
        QMenu* menu = new QMenu();
        QAction* a = menu->addAction("Field Information");
        connect(a, SIGNAL(triggered()), this, SLOT(displayFieldInfo()));
        
        String type = table->field(clickedHeader)->getType();
        if(TBConstants::typeIsNumberable(type)) {
            a = menu->addAction("Statistics");
            connect(a, SIGNAL(triggered()),
                    this, SLOT(displayFieldStatistics()));
        }
        
        a = menu->addAction("Format Field");
        connect(a, SIGNAL(triggered()), this, SLOT(formatField()));
        
        a = menu->addAction("Hide Column");
        connect(a, SIGNAL(triggered()), this, SLOT(hideColumn()));
        
        menu->exec(event->globalPos());
        
        event->accept();
        
    // right click on table data
    } else if(hx && tx) {
        int x = tableWidget->currentRow();
        int y = tableWidget->currentColumn();
        
        if(x < 0 || y < 0) return;

        String type = table->field(y)->getType();
        if(TBConstants::typeIsIndexable(type)) {
            double d = table->dataAt(x, y)->asDouble();
            if(d < 0) return;
            
            vector<String> subtables;
            for(unsigned int i = 0; i< table->getTableKeywords()->size(); i++){
                TBKeyword* kw = table->getTableKeywords()->at(i);
                String type = kw->getType();
                if(TBConstants::typeIsTable(type)) {
                    subtables.push_back(kw->getName());
                }
            }

            if(subtables.size() == 0) return;
            
            QMenu* menu = new QMenu();
            QMenu* follow = new QMenu("Follow subtable index reference");

            for(unsigned int i = 0; i < subtables.size(); i++)
                follow->addAction(subtables.at(i).c_str());
                
            connect(follow, SIGNAL(triggered(QAction*)),
                    this, SLOT(referenceMenuClicked(QAction*)));

            menu->addMenu(follow);
            menu->exec(event->globalPos());
            event->accept();
        }
    }
    
    if(!event->isAccepted()) event->ignore();
}

// Private Methods //

void TBDataTab::updatePageTrackers() {
    pageLabel->setText(("[ " + TBConstants::itoa(page + 1) + " / " +
                      TBConstants::itoa(table->getNumPages()) + " ]").c_str());
    pageEdit->setText(TBConstants::itoa(page + 1).c_str());
    int n = table->getNumPages() - 1;
    forwardButton->setEnabled(page < n);
    lastButton->setEnabled(page < n);
    backButton->setEnabled(page != 0);
    firstButton->setEnabled(page != 0);
}

void TBDataTab::sort(int index, bool asc) {
    tableWidget->blockSignals(true);
    tableWidget->sortItems(index,(asc?Qt::AscendingOrder:Qt::DescendingOrder));
    setSortIndicator(index, asc);

    // Rename row headers
    int ind = table->getRowIndex();
    rowIndices.clear();
    for(unsigned int i = 0; i < rowItems.size(); i++) {
        int visRow = rowItems.at(i)->row();
        tableWidget->verticalHeaderItem(visRow)->setText(
                TBConstants::itoa(ind + i).c_str());
    }
    for(int i = 0; i < tableWidget->rowCount(); i++) {
        int n = 0;
        String str = qPrintable(tableWidget->verticalHeaderItem(i)->text());
        TBConstants::atoi(str, &n);
        rowIndices.push_back(n);
    }

    tableWidget->blockSignals(false);
}

void TBDataTab::applyFilter() {
    if(filter == NULL) {
        for(int i = 0; i < tableWidget->rowCount(); i++)
            tableWidget->showRow(i);
    } else {
        for(int i = 0; i < tableWidget->rowCount(); i++) {
            int ind = logicalIndex(i);
            
            if(filter->rowPasses(table, ind) < 0) tableWidget->hideRow(i);
            else tableWidget->showRow(i);
        }
    }
}

// Private Slots //

void TBDataTab::pageBack() {
    loadPage(page - 1);
}

void TBDataTab::pageForward() {
    loadPage(page + 1);
}

void TBDataTab::pageFirst() {
    loadPage(0);
}

void TBDataTab::pageLast() {
    loadPage(table->getNumPages() - 1);
}

void TBDataTab::pageGo() {
    if(!pageEdit->text().isEmpty()) {
        int p;
        String str = qPrintable(pageEdit->text());
        TBConstants::atoi(str, &p);
        if(p == 0) p = 1; // why does the validator allow this? :(
        loadPage(p - 1);
    }
}

void TBDataTab::notifyDataChanged(int row, int col) {
    if(row > -1 && col > -1) {
        String data = qPrintable(tableWidget->item(row, col)->text());
        if(rowIndices.size() > 0)
            row = rowIndices.at(row);
        emit dataChanged(row + table->getRowIndex(), col, data);
    }
}

void TBDataTab::cellClicked(int row, int col) {
    tableWidget->blockSignals(true);
    for(unsigned int i = 0; i < highlightedCells.size(); i++) {
        highlightedCells.at(i)->setBackground(unselectedBackground);
    }
    highlightedCells.clear();
    
    if(row < 0 || row >= tableWidget->rowCount() &&
       col < 0 || col >= tableWidget->columnCount()) {
        tableWidget->blockSignals(false);
        return;
    }
    
    for(int r = 0; r < tableWidget->rowCount(); r++) {
        if(r != row) {
            QTableWidgetItem* item = tableWidget->item(r, col);
            item->setBackground(selectedBackground);
            highlightedCells.push_back(item);
        }
    }
    for(int c = 0; c < tableWidget->columnCount(); c++) {
        if(c != col) {
            QTableWidgetItem* item = tableWidget->item(row, c);
            item->setBackground(selectedBackground);
            highlightedCells.push_back(item);
        }
    }
    
    tableWidget->blockSignals(false);
}

void TBDataTab::referenceMenuClicked(QAction* which) {
    int x = tableWidget->currentRow();
    int y = tableWidget->currentColumn();
    if(x < 0 || y < 0) return;

    String type = table->field(y)->getType();
    if(TBConstants::typeIsIndexable(type)) {
        String data = table->dataAt(x, y)->asString();
        int index;
        if(TBConstants::atoi(data, &index) != 1) return;

        String subtable = qPrintable(which->text());
        emit followReferenceRequested(subtable, index);
    }
}

void TBDataTab::doubleClicked(int row, int col) {
    if(row < 0 || col < 0) return;

    if(rowIndices.size() > 0)
        row = rowIndices.at(row);
    String type = table->field(col)->getType();
    
    if(TBConstants::typeIsTable(type)) {
        ttabs->getBrowser()->openTable(table->dataAt(row, col)->asString());
    } else if(TBConstants::typeIsArray(type)) {     
        TBArrayData* d = table->loadArray(row + table->getRowIndex(), col);
        if(d != NULL && d->isLoaded() && d->isEmpty())
            d->release();

        if(d != NULL && d->isLoaded()) {
            bool show = false;
            if(arrayPanel == NULL) {
                arrayPanel = new TBArrayPanel(ttabs);
                connect(arrayPanel, SIGNAL(allArraysClosed()),
                        this, SLOT(clearWidgetInSplitter()));
                rightWidget->setCloseButtonText("Close All");
                show = true;
            }
            TBViewArray* va = new TBViewArray(ttabs, TBConstants::itoa(row +
                              table->getRowIndex()), TBConstants::itoa(col),
                              d, row + table->getRowIndex(), col, true);
            TBFormat* f = formatAt(col);
            if(f != NULL) va->applyFormat(f);
            bool b = arrayPanel->addArray(va, col);
            if(!b) {
                va->setShouldRelease(false);
                delete va;
            }
            if(show) showWidgetInSplitter(arrayPanel, true);
        } else {
            ttabs->getBrowser()->displayError(
                                 "This cell has invalid or no data.");
        }
    } else if(type == TBConstants::TYPE_RECORD) {
        TBDataRecord* r = (TBDataRecord*)table->keyword(row)->getValue();
        TBViewRecord* vr = new TBViewRecord(ttabs, r, table->getName(), TBConstants::itoa(row));
        showWidgetInSplitter(vr);
    } else {
        if(table->isEditable(col)) {
            if(ttabs->isEditable()) {
                // clear formatting for editing
                setData(row + table->getRowIndex(), col,
                        table->dataAt(row, col), false);
            } else {
                String message = "Browser is not in editor mode.\nToggle this";
                message += " table in Edit->Table if you wish to edit.";
                ttabs->getBrowser()->displayError(message);
            }
        } else {
            String message = table->isAnyEditable() ?
                             "This column does not allow editing." :
                             "This table does not allow editing.";
            ttabs->getBrowser()->displayError(message);
        }
    }
}

void TBDataTab::displayFieldInfo() {
    String str = table->fieldToolTip(clickedHeader);
    if(!str.empty()) {
        String fieldName = table->field(clickedHeader)->getName();
        QMessageBox::information(this, (table->getName() + " :: " +
                                fieldName).c_str(), str.c_str());
    }
}

void TBDataTab::displayFieldStatistics() {
    String type = table->field(clickedHeader)->getType();
    if(!TBConstants::typeIsNumberable(type)) return;
    
    double v = table->dataAt(0, clickedHeader)->asDouble();
    double min = v, max = v, sum = v;
    
    for(int i = 1; i < tableWidget->rowCount(); i++) {
        v = table->dataAt(i, clickedHeader)->asDouble();
        if(v < min) min = v;
        if(v > max) max = v;
        sum += v;
    }
    sum /= tableWidget->rowCount();
    
    stringstream ss;
    ss << "Rows:\t" << table->getRowIndex() << " - ";
    ss << table->getRowIndex() + tableWidget->rowCount() - 1 << "\nMinimum:\t";
    ss << min << "\nMaximum:\t" << max << "\nMean:\t" << sum;
    String fieldName = table->field(clickedHeader)->getName();
    QMessageBox::information(this, (table->getName() + " :: " + 
                             fieldName).c_str(), ss.str().c_str());
}

void TBDataTab::hideColumn() {
    hideColumn(clickedHeader, true);
}

void TBDataTab::formatField() {
    formatField(clickedHeader);
}

void TBDataTab::headerMoved() {
    restoreColsButton->setEnabled(true);
}

void TBDataTab::restoreColumns() {
    if(tableWidget->horizontalHeader()->sectionsMoved()) {
        QHeaderView* qhv = tableWidget->horizontalHeader();
        for(int i = 0; i < tableWidget->columnCount(); i++) {
            if(qhv->visualIndex(i) != i) {
                qhv->moveSection(qhv->visualIndex(i), i);
            }
        }
    }
    restoreColsButton->setEnabled(false);
}

void TBDataTab::resizeHeaders() {
    tableWidget->resizeColumnsToContents();
    tableWidget->resizeRowsToContents();
}

void TBDataTab::headerClicked(int index) {
    if(index < 0 || index >= table->getNumFields()) return;

    bool asc = true;
    if(index == tableWidget->horizontalHeader()->sortIndicatorSection())
        asc = tableWidget->horizontalHeader()->sortIndicatorOrder() ==
                Qt::AscendingOrder;

    sort(index, asc);
    currSort.clear();
    String field = table->getFields()->at(index)->getName();
    currSort.push_back(pair<String, bool>(field, asc));
    emit sortEntered();
}

void TBDataTab::clearSort() {
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    tableWidget->blockSignals(true);

    vector<QList<QTableWidgetItem*> > copy(tableWidget->rowCount());
    for(int row = 0; row < tableWidget->rowCount(); row++) {
        int r = rowItems.at(row)->row();
        if(row != r) {
            for(int col = 0; col < tableWidget->columnCount(); col++) {
                copy[row] << tableWidget->takeItem(row, col);
            }
        }
    }

    for(int r = 0; r < tableWidget->rowCount(); r++) {
        if(copy[r].size() > 0) {
            int row = rowIndices.at(r);
            row -= table->getRowIndex();
            for(int col = 0; col < tableWidget->columnCount(); col++) {
                tableWidget->setItem(row, col, copy.at(r).at(col));
            }
        }
    }

    int ind = table->getRowIndex();
    for(int i = 0; i < tableWidget->rowCount(); i++)
        tableWidget->verticalHeaderItem(i)->setText(
                TBConstants::itoa(ind + i).c_str());

    rowIndices.clear();
    currSort.clear();
    clearSortIndicator();
    tableWidget->blockSignals(false);
    QApplication::restoreOverrideCursor();
    emit sortCleared();
}

}
