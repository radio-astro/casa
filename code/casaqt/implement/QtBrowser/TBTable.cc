//# Table.cc: Primary interface for the rest of the browser to a table.
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
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBKeyword.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/XMLtoken.h>
#include <casaqt/QtBrowser/QProgressPanel.qo.h>
#include <casaqt/QtBrowser/TBXMLDriver.h>
#include <casaqt/QtBrowser/TBParser.h>
#include <casaqt/QtBrowser/TBTableDriver.h>
#include <casaqt/QtBrowser/TBData.h>
#include <casaqt/QtBrowser/TBPlotter.qo.h>
#include <casaqt/QtBrowser/TBFilterRules.qo.h>

#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>

namespace casa {

/////////////////////////
// TBTABLE DEFINITIONS //
/////////////////////////

// Constructors/Destructors //

TBTable::TBTable(String f, DriverParams* dp, bool tql) : dParams(dp),
                          location(f), ready(true), insertRow(false),
                          removeRow(false), totalRows(0), loadedRows(0),
                          selectedRows(0), rowIndex(0), taql(tql) {
    if(dp == NULL) throw "Driver Parameters cannot be null.";
    if(f.empty()) throw "Table location cannot be empty.";

    TableParams* tp = new TableParams(location, insertRow, removeRow, /*data,*/
                                      data, fields, keywords, subtableRows,
                                      totalRows, loadedRows, writable, taql,
                                      dp);

    if(dp->type == DIRECT)
        driver = new TBTableDriverDirect(tp, this);
    /*else if(dp->type == XML)
        driver = new TBTableDriverXML(tp, this);
    */
    else throw ("Unsupported Driver Type: #" +
                TBConstants::itoa(dp->type)).c_str();
    delete tp;

    setPrintDebug(true);
}

TBTable::~TBTable() {   
    for(unsigned int i = 0; i < data.size(); i++) {
        vector<TBData*>* v = data.at(i);
        for(unsigned int j = 0; j < v->size(); j++) {
            delete v->at(j);
        }
        delete v;
    }
    
    for(unsigned int i = 0; i < fields.size(); i++)
        delete fields.at(i);
    
    for(unsigned int i = 0; i < keywords.size(); i++)
        delete keywords.at(i);
    
    delete driver;
    delete dParams;
}

// Accessors/Mutators //

String TBTable::getFile() {
    if(ready) return location;
    else return "";
}

String TBTable::getName() {
    if(ready) {
        if(taql) return "TaQL table";
        else return TBConstants::nameFromPath(location);
    }
    else return "";
}

DriverParams* TBTable::getDriverParams() { return dParams; }

vector<TBField*>* TBTable::getFields() { return &fields; }

TBField* TBTable::field(int i) {
    if(i >= 0 && i < (int)fields.size()) return fields.at(i);
    else return NULL;
}

TBField* TBTable::field(String name) {
    for(unsigned int i = 0; i < fields.size(); i++)
        if(fields.at(i)->getName() == name)
            return fields.at(i);
    
    return NULL;
}

vector<TBKeyword*>* TBTable::getTableKeywords() { return &keywords; }

TBKeyword* TBTable::keyword(int i) {
    if(i >= 0 && i < (int)keywords.size()) return keywords.at(i);
    else return NULL;
}

TBKeyword* TBTable::keyword(String name) {
    for(unsigned int i = 0; i < keywords.size(); i++)
        if(keywords.at(i)->getName() == name)
            return keywords.at(i);
    
    return NULL;
}

bool TBTable::isReady() { return ready; }

bool TBTable::canInsertRows() { return insertRow; }

bool TBTable::canDeleteRows() { return removeRow; }

int TBTable::getTotalRows() { return totalRows; }

int TBTable::getLoadedRows() { return loadedRows; }

int TBTable::getSelectedRows() { return selectedRows; }

int TBTable::getNumPages() {
    if(selectedRows != 0)
        return totalRows / selectedRows + (totalRows % selectedRows == 0?0:1);
    else return 0;
}

int TBTable::getPage() {
    if(selectedRows != 0) return rowIndex / selectedRows;
    else return 0;
}

int TBTable::getRowIndex() { return rowIndex; }

int TBTable::getNumFields() { return fields.size(); }

void TBTable::setPrintDebug(bool pdg) { driver->setPrintDebug(pdg); }

TBData* TBTable::dataAt(unsigned int row, unsigned int col) {
    if(row >= data.size())
        row -= rowIndex;
    
    if(ready&& 0<= row && row < data.size() && 0 <= col && col < fields.size())
        return data.at(row)->at(col);
    else return NULL;
}

vector<int> TBTable::dataDimensionsAt(unsigned int col) {
    if(ready && 0 <= col && col < fields.size())
        return driver->dimensionsOf(col);
    else return vector<int>();
}

bool TBTable::isEditable() {
    if(taql) return false;
    
    for(unsigned int i = 0; i < writable.size(); i++)
        if(!writable[i]) return false;  
    return true;
}

bool TBTable::isAnyEditable() {
    if(taql) return false;
    
    for(unsigned int i = 0; i < writable.size(); i++)
        if(writable[i]) return true;
    return false;
}

bool TBTable::isEditable(int index) {
    if(taql) return false;
    
    if(index < 0 || index >= (int)writable.size()) return false;
    else return writable[index];
}

bool TBTable::isTaQL() { return taql; }

bool TBTable::canRead() { return driver->canRead(); }

bool TBTable::canWrite() { return driver->canWrite(); }

bool TBTable::tryWriteLock() { return driver->tryWriteLock(); }

bool TBTable::releaseWriteLock() { return driver->releaseWriteLock(); }

// Public Methods //

Result TBTable::loadRows(int start, int number, bool full, vector<String>* cols,
                         bool parsedata, ProgressHelper* progressPanel) {
    if(ready) {
        rowIndex = start;
        selectedRows = number;
        Result r = driver->loadRows(start, number, full, cols, parsedata,
                                    progressPanel);
        ready = r.valid;        
        return r;
    } else return Result("Table not ready.", false);
}

vector<String> TBTable::getColumnHeaders() {
    vector<String> v(fields.size());

    for(unsigned int i = 0; i < fields.size(); i++)
        v[i] = fields.at(i)->getName();
    
    return v;
}

vector<String> TBTable::getRowHeaders() {
    vector<String> v(loadedRows);

    for(int i = 0; i < loadedRows; i++)
        v[i] = TBConstants::itoa(i + rowIndex);
    
    return v;
}

TBArrayData* TBTable::loadArray(unsigned int row, unsigned int col) {
    if(row >= data.size())
        row -= rowIndex;
    
    if(ready&& 0 <= row&& row < data.size()&& 0 <= col&& col < fields.size()){
        String type = fields.at(col)->getType();
        if(TBConstants::typeIsArray(type)) {
            TBArrayData* d = (TBArrayData*)data.at(row)->at(col);
            if(!d->isLoaded()) driver->loadArray(d, row, col);
            return d;
        } else return NULL;
    } else return NULL;
}

bool TBTable::releaseArray(unsigned int row, unsigned int col) {
    
    if(ready&& 0 <= row&& row < data.size()&& 0 <= col&& col < fields.size()){
        String type = fields.at(col)->getType();
        if(TBConstants::typeIsArray(type)) {
            TBArrayData* d = (TBArrayData*)data.at(row)->at(col);
            return d->release();
        } else return false;
    } else return false;
}

Result TBTable::editData(unsigned int row, unsigned int col, TBData* newVal) {
    if(ready && 0 <= row && (int)row < totalRows && 0 <= col &&
       col < fields.size()) {
        return driver->editData(row, col, newVal);
    } else return Result("Invalid indices.", false);
}

Result TBTable::editArrayData(unsigned int row, unsigned int col,
                             vector<int> coords, TBData* newVal, bool oneDim) {
    if(ready && 0 <= row && (int)row < totalRows && 0 <= col &&
       col < fields.size() && coords.size() > 0) {
        if(oneDim && coords.size() > 1) coords.erase(coords.begin());
        return driver->editData(row, col, newVal, &coords);
    } else return Result("Invalid indices.", false);
}

void TBTable::exportVOTable(String file, ProgressHelper* pp) {
    if(!ready) return;
    
    TBTable table(location, new DriverParams(dParams));
        
    ofstream out;
    out.open(file.c_str());

    if(!out.is_open()) return;

    int steps = totalRows + fields.size() + 2 + getNumPages();
    if(pp != NULL) pp->setSteps(steps);
        
    // Header tags
    out << "<!DOCTYPE VOTABLE SYSTEM \"http://us-vo.org/xml/VOTable.dtd\">";
    out << "<?xml version=\"1.0\" standalone=\"yes\" ?>\n\n";
    out << "<" << TBConstants::XML_VOTABLE << " version=\"1.0\">\n";
    out << "  <" << TBConstants::XML_RESOURCE << " name=\"" << location;
    out << "\">\n    <" << TBConstants::XML_TABLE << ">\n";
    if(pp != NULL) pp->step();
        
    // Fields
    for(unsigned int i = 0; i < fields.size(); i++) {
        TBField* f = fields.at(i);
        String type = f->getType();
        out << "      <" << TBConstants::XML_FIELD << " ";
        out << TBConstants::XML_ID << "=\"" << f->getName() << "\" ";
        out << TBConstants::XML_FIELD_NAME << "=\"" << f->getName() << "\" ";
        out << TBConstants::XML_FIELD_TYPE << "=\"" <<TBConstants::VOType(type);
        out << "\"";
        if(TBConstants::typeIsArray(type)) {
            out << " arraysize=\"";
            vector<int> d = dataDimensionsAt(i);
            for(unsigned int j = 0; j < d.size(); j++) {
                if(j > 0) out << "x";
                out << d.at(j);
            }
            if(type == TBConstants::TYPE_ARRAY_STRING)
                out << "x*";
            out << '"';
        } else if(type == TBConstants::TYPE_STRING ||
                  type == TBConstants::TYPE_TABLE ||
                  type == TBConstants::TYPE_RECORD) {
            out << " arraysize=\"*\"";
        }
        out << "/>\n";
        if(pp != NULL) pp->step();
    }

    // Data
    out << "      <" << TBConstants::XML_DATA << ">\n        <";
    out << TBConstants::XML_TABLEDATA << ">\n";
        
    int rowIndex = 0;
    int selectedRows = TBConstants::DEFAULT_EXPORT_NUM;
    while(table.loadedRows == 0 || table.getPage() < table.getNumPages() - 1) {
          table.loadRows(rowIndex, selectedRows, true);
        if(pp != NULL) pp->step();
        for(unsigned int i = 0; i < table.data.size(); i++) {
            vector<TBData*>* tr = table.data.at(i);
            out << "          <" << TBConstants::XML_TR << ">\n";
            for(unsigned int j = 0; j < tr->size(); j++) {
                out << "            <" << TBConstants::XML_TD << ">";

                TBData* d = table.dataAt(i, j);
                String type = d->getType();
                if(TBConstants::typeIsArray(type)) {
                    TBArrayData* d2 = (TBArrayData*)d;
                    out << d2->to1DString();
                } else if(type == TBConstants::TYPE_DATE) {
                    out << d->asDouble();
                } else out << d->asString();

                out << "</" << TBConstants::XML_TD << ">\n";
            }
            out << "          </" << TBConstants::XML_TR << ">\n";
            if(pp != NULL) pp->step();
        }

        rowIndex += TBConstants::DEFAULT_EXPORT_NUM;
    }

    // Closing Tags
    out << "        </" << TBConstants::XML_TABLEDATA << ">\n      </";
    out << TBConstants::XML_DATA << ">\n    </" << TBConstants::XML_TABLE;
    out << ">\n  </" << TBConstants::XML_RESOURCE << ">\n</";
    out << TBConstants::XML_VOTABLE << ">\n\n";
    if(pp != NULL) pp->step();

    out.close();
    if(pp != NULL) pp->done();
}

TBPlotData* TBTable::plotRows(PlotParams& x, PlotParams& y, int rowFrom,
                      int rowTo, int rowInterval, TBFilterRuleSequence* rules,
                      ProgressHelper* ph) {

    TBPlotData* data = new TBPlotData();
    if(ready && (x.rowNumbers || 0 <= x.colIndex < fields.size()) &&
       (y.rowNumbers || 0 <= y.colIndex < fields.size()) &&
       0<= rowFrom && rowFrom < totalRows && 0 <= rowTo && rowTo < totalRows &&
       rowFrom <= rowTo && 0 < rowInterval <= (rowTo - rowFrom + 1)) {

        int n = ((rowTo - rowFrom) / rowInterval) + 1;
        int steps = ((rowTo - rowFrom)/rowInterval) + 1;
        if(!x.rowNumbers || !y.rowNumbers) steps *= 2;
        steps += 1;
        if(ph != NULL) ph->setSteps(steps);
        
        if(x.rowNumbers && y.rowNumbers) {
            double* xarr = new double[n];
            double* yarr = new double[n];
            data->rows.resize(n);
            for(int i = 0; i < n; i++) {
                xarr[i] = (i * rowInterval) + rowFrom;
                yarr[i] = (i * rowInterval) + rowFrom;
                data->rows[i] = (i * rowInterval) + rowFrom;
                if(ph != NULL) ph->step();
            }
            
            data->data = new PlotPointDataImpl<double>(xarr, yarr, n);
            
            // apply filter if it exists
            if(rules != NULL) {
                TBTable table(location, new DriverParams(dParams));
                table.setPrintDebug(false);
                filter(data, table, rules, rowFrom, rowTo, rowInterval);
            }
            
            return data;
        }

        double* rowData = NULL;
        double* xarr, *yarr;
        if(x.rowNumbers || y.rowNumbers) {
            rowData = new double[n];
            for(int i = 0; i < n; i++) {
                rowData[i] = (i * rowInterval) + rowFrom;
                if(ph != NULL) ph->step();
            }
        }

        vector<String>* f = new vector<String>();
        if(x.rowNumbers) // r x C
            f->push_back(fields.at(y.colIndex)->getName());
        else if(y.rowNumbers || x.colIndex == y.colIndex) // C x r or C1 x C1
            f->push_back(fields.at(x.colIndex)->getName());
        else { // C1 x C2
            f->push_back(fields.at(x.colIndex)->getName());
            f->push_back(fields.at(y.colIndex)->getName());
        }

        TBTable table(location, new DriverParams(dParams));
        table.setPrintDebug(false);
        Result r = table.loadRows(rowFrom, rowTo - rowFrom + 1, true, f);
        delete f;
        if(ph != NULL) ph->step();

        if(r.valid) {
            if(x.rowNumbers) xarr = rowData;
            else xarr = new double[n];
                
            if(y.rowNumbers) yarr = rowData;
            else yarr = new double[n];
                
            data->rows.resize(n);
            
            String t1 = table.fields.at(0)->getType();
            int index = 0;
            for(int i = 0; i < table.loadedRows; i += rowInterval) {
                TBData* td = table.data.at(i)->at(0);
                double d;
                
                if(x.rowNumbers && !y.rowNumbers)
                    d = getDouble(td, &y.slice, y.complex, y.complexAmp);
                else
                    d = getDouble(td, &x.slice, x.complex, x.complexAmp);

                if(x.rowNumbers && !y.rowNumbers) { // r x C
                    yarr[index] = d;
                } else if(!x.rowNumbers && y.rowNumbers) { // C x r
                    xarr[index] = d;
                } else { // C x C
                    xarr[index] = d;

                    if(x.colIndex != y.colIndex) { // C1 x C2
                        td = table.data.at(i)->at(1);
                        d = getDouble(td, &y.slice, y.complex, y.complexAmp);
                        yarr[index] = d;
                        
                    } else if(TBConstants::typeIsArray(t1)) { // C1a x C1a
                        if(x.slice != y.slice || (x.complex &&
                                               x.complexAmp != y.complexAmp)) {
                            d = getDouble(td, &y.slice, y.complex, y.complexAmp);
                            yarr[index] = d;
                        } else {
                            yarr[index] = d;
                        }
                        
                        // C1c x C1c
                    } else if((TBConstants::typeIsComplex(t1)) &&
                              x.complexAmp != y.complexAmp) {
                        d = getDouble(td, &y.slice, y.complex, y.complexAmp);
                        yarr[index] = d;
                        
                        // C1 x C1
                    } else {
                        yarr[index] = d;
                    }
                }
                data->rows[index] = i + rowFrom;
                index++;
                if(ph != NULL) ph->step();
            }
            
            data->data = new PlotPointDataImpl<double>(xarr, yarr, n);
            
            // apply filter if it exists
            if(rules != NULL)
                filter(data, table, rules, rowFrom, rowTo, rowInterval);
        }
    }

    return data;
}

int TBTable::totalRowsOf(String location) {
    if(subtableRows.size() == 0) {
        return driver->totalRowsOf(location);
    } else {
        if(location[location.length() - 1] == '/')
            location.erase(location.length() - 1, 1);
        int j = -1;
        for(unsigned int i = 0; i < keywords.size(); i++) {
            TBKeyword* kw = keywords.at(i);
            if(kw->getType() == TBConstants::TYPE_TABLE) {
                j++;
                String v = kw->getValue()->asString();
                if(v[v.length() - 1] == '/') v.erase(v.length() - 1, 1);
                if(v == location) break;
            }
        }
        if(j > -1 && j < (int)subtableRows.size()) {
            if(subtableRows.at(j) != -1) return subtableRows.at(j);
            int r = driver->totalRowsOf(location);
            subtableRows[j] = r;
            return r;
        } else return driver->totalRowsOf(location); // shouldn't happen
    }
}

String TBTable::fieldToolTip(int i) {
    stringstream ss;

    if(0 <= i && i < (int)fields.size()) {
        TBField* f = fields.at(i);
        String type = f->getType();
        ss << f->getName() << ": " << TBConstants::typeName(type);

        String val = f->getUnit();
        if(!val.empty()) ss << "\nUnit: " << val;
        val = f->getUCD();
        if(!val.empty()) ss << "\nUCD: " << val;
        val = f->getRef();
        if(!val.empty()) ss << "\nRef: " << val;
        val = f->getPrecision();
        if(!val.empty()) ss << "\nPrecision: " << val;
        val = f->getWidth();
        if(!val.empty()) ss << "\nWidth: " << val;
        
        unsigned int n = f->getKeywords()->size();
        if(n > 0) ss << "\nKeywords:";
        for(unsigned int j = 0; j < n; j++) {
            ss << "\n\t";
            TBKeyword* k = f->getKeywords()->at(j);
            type = k->getType();
            val = k->getValue()->asString();

            // replace \n, \r with spaces
            for(unsigned int a = 0; a < val.length(); a++) {
                if(val[a] == '\n' || val[a] == '\r')
                    val[a] = ' ';
            }
                
            ss << k->getName() << " (" << TBConstants::typeName(type) << "): ";
            ss << val;
        }
    }
    
    return ss.str();
}

String TBTable::tableToolTip() {
    stringstream ss;

    ss << location << "\nInsert row: " << (insertRow?"yes":"no");
    ss << ", Remove row: " << (removeRow?"yes":"no") << "\nTotal rows: ";
    ss << totalRows << "\nLoaded rows: " << loadedRows << "\nFields:";

    for(unsigned int i = 0; i < fields.size(); i++) {
        TBField* f = fields.at(i);
        String type = f->getType();
        ss << "\n\t" << f->getName() << " (" << TBConstants::typeName(type);
        ss << ')';
    }

    ss << "\nKeywords:";
    for(unsigned int i = 0; i < keywords.size(); i++) {
        TBKeyword* k = keywords.at(i);
        String type = k->getType();
        String val = k->getValue()->asString();

        // replace \n, \r with spaces
        for(unsigned int a = 0; a < val.length(); a++) {
            if(val[a] == '\n' || val[a] == '\r')
                val[a] = ' ';
        }
        
        ss << "\n\t" << k->getName() << " (" << TBConstants::typeName(type);
        ss << "): " << val;
    }
    
    return ss.str();
}

Result TBTable::insertRows(int n) {
    if(n < 1) return Result("Number must be greater than 0.", false);
    else return driver->insertRows(n);
}

Result TBTable::deleteRows(vector<int> r) {
    if(r.size() == 0) return Result("", true);
    
    // sort r in descending order
    std::sort(r.begin(), r.end());
    vector<int> desc(r.size());
    int n = r.size() - 1;
    for(unsigned int i = 0; i < desc.size(); i++) {
        desc[i] = r.at(n - i);
    }

    return driver->deleteRows(desc);
}

// Private Methods //

double TBTable::getDouble(TBData* d, vector<int>* slice, bool comp, bool amp) {
    String type = d->getType();
    double val = 0;
    if(TBConstants::typeIsArray(type)) {
        TBArrayData* tad = (TBArrayData*)d;
        if(slice != NULL)
            return getDouble(tad->dataAt(*slice), slice, comp, amp);
    } else if(comp) {
        val = amp ? d->asDComplex().first : d->asDComplex().second;
    } else {
        val = d->asDouble();
    }
    return val;
}

void TBTable::filter(TBPlotData* data, TBTable& table,
                     TBFilterRuleSequence* rules, int rowFrom, int rowTo,
                     int rowInterval) {
    if(rules != NULL && data != NULL && !data->data.null()) {             
        int count = 0;

        // reload table with the necessary fields to run the rule
        vector<String>* fields = new vector<String>();
        // if any of the rules are for any field, we have to load all
        // fields
        bool any = false, found;
        for(unsigned int i = 0; i < rules->size(); i++) {
            found = false;
            if(rules->at(i)->getAnyField()) {
                any = true;
                break;
            }

            // only add fields not already in list
            String field = rules->at(i)->getField();
            for(unsigned int j = 0; j < fields->size(); j++) {
                if(fields->at(j) == field) {
                    found = true;
                    break;
                }
            }
            if(!found) fields->push_back(field);
        }

        if(any)
            table.loadRows(rowFrom, rowTo - rowFrom + 1, true);
        else
            table.loadRows(rowFrom, rowTo - rowFrom + 1, true, fields);

        delete fields;

        int n = data->data->size();
        vector<double> v1(n);
        vector<double> v2(n);
        data->rows.resize(n);

        for(int i = 0; i < table.loadedRows; i += rowInterval) {
            if(rules->rowPasses(&table, i) >= 0) {
                v1[count] = data->data->xAt(i / rowInterval);
                v2[count] = data->data->yAt(i / rowInterval);
                data->rows[count] = i + rowFrom;
                count++;
            }
        }

        n = count;

        double* xarr = new double[n];
        double* yarr = new double[n];

        for(int i = 0; i < n; i++) {
            xarr[i] = v1[i];
            yarr[i] = v2[i];
        }
        
        data->data = new PlotPointDataImpl<double>(xarr, yarr, n);
    }
}

}
