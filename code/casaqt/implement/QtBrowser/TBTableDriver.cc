//# TBTableDriver.cc: Driver for interacting with the table on disk.
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
#include <casaqt/QtBrowser/TBTableDriver.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBKeyword.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/TBXMLDriver.h>
#include <casaqt/QtBrowser/TBParser.h>
#include <casaqt/QtBrowser/QProgressPanel.qo.h>
#include <casaqt/QtBrowser/TBData.h>

#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableColumn.h>
#include <casa/Containers/RecordField.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>

#include <sstream>

namespace casa {

///////////////////////////////
// TBTABLEDRIVER DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

TBTableDriver::TBTableDriver(TableParams* tp, TBTable* t):
               location(tp->location), insertRow(tp->insertRow),
               removeRow(tp->removeRow), data(tp->data),
               fields(tp->fields), keywords(tp->keywords),
               subtableRows(tp->subtableRows), totalRows(tp->totalRows),
               loadedRows(tp->loadedRows), writable(tp->writable),
               taql(tp->taql), dp(tp->dParams), printdebug(true), table(t) { }

TBTableDriver::~TBTableDriver() { }

// Accessors/Mutators //

void TBTableDriver::setPrintDebug(bool pdb) { printdebug = pdb; }

// Protected Methods //

/*
Result TBTableDriver::query(String type, String query) {
    String str = "send.table." + type;
        
    int l = query.length();
    int n = 0;
    if(l < 100) {
        str += " ";
        n = 1;
    }

    stringstream ss;
    ss << str << " " << l << query << "    ";
    // sometimes the last few characters are changed,
    // so add some spaces at the end so it doesn't matter

    String s = ss.str();
    const char* q = s.c_str();

    if(printdebug) TBConstants::dprint(TBConstants::DEBUG_HIGH, q);
        
    String xml = TBXMLDriver::dowork(q);

    Result r(xml);
    if((type == TBConstants::QUERY_QUERY && xml.find('<') == String::npos) ||
       (type == TBConstants::QUERY_ARRAY && xml.find('[') == String::npos))
        // there was an error
        r.valid = false;

    return r;
}
*/

/////////////////////////////////////
// TBTABLEDRIVERDIRECT DEFINITIONS //
/////////////////////////////////////

// Constructors/Destructors //

TBTableDriverDirect::TBTableDriverDirect(TableParams* tp, TBTable* t) :
                                    TBTableDriver(tp, t) {
    if(taql) m_table = Table(tableCommand(location));
    else m_table = Table(location);
}

TBTableDriverDirect::~TBTableDriverDirect() { }

// Public Methods //

bool TBTableDriverDirect::canRead() {
    return m_table.hasLock(FileLocker::Read);
}

bool TBTableDriverDirect::canWrite() {
    return m_table.hasLock(FileLocker::Write);
}

bool TBTableDriverDirect::tryWriteLock() {
    if(!m_table.lock(FileLocker::Write, 1)) return false;
    try {
        m_table.reopenRW();
        return true;
    } catch(...) {
        releaseWriteLock();
        return false;
    }
}

bool TBTableDriverDirect::releaseWriteLock() {
    if(!m_table.hasLock(FileLocker::Write)) return true; // nothing to release
    m_table.unlock(); // release write lock
    return m_table.lock(FileLocker::Read); // re-acquire read lock
}

Result TBTableDriverDirect::loadRows(int start, int num, bool full,
                                     vector<String>* f, bool parsedata,
                                     ProgressHelper* pp) {
    try {
    int steps = 1 + fields.size();
    if(parsedata && pp != NULL) {
        int n = taql ? num : totalRowsOf(location);
        if(num < n) n = num;
        steps += n / 10;
    }
    if(pp != NULL) {
        pp->reset("Loading rows...");
        pp->setSteps(steps);
    }

    Table table;
    
    // open/reference table appropriately
    if(f == NULL || f->size() == 0) {
        table = m_table;
    } else {
        Block<String> cols(f->size());
        for(unsigned int i = 0; i < f->size(); i++)
            cols[i] = f->at(i);
        table = m_table.project(cols);
        /*
        if(taql) {
            TableExprNode cols = m_table.col(f->at(0));
            
            for(unsigned int i = 1; i < f->size(); i++)
                cols = cols && m_table.col(f->at(i));
            
            table = m_table(cols);
        } else {
            stringstream ss;
            ss << "SELECT ";
            for(unsigned int i = 0; i < f->size(); i++) {
                ss << f->at(i);
                if(i < f->size() - 1) ss << ',';
            }
            ss << " FROM " << location;
            String query = ss.str();
            if(printdebug) TBConstants::dprint(TBConstants::DEBUG_HIGH, query);
            table = tableCommand(query);
        }
        */
    }
    if(pp != NULL) pp->step();

    totalRows = table.nrow();
    if(start < 0) start = 0;
    int end = start + num;
    if(num == 0 || end >= totalRows) end = totalRows;
    if(end <= start) return Result(TBConstants::ERROR_EMPTY, false);
    
    // Update table parameters
    insertRow = !taql && table.canAddRow();
    removeRow = !taql && table.canRemoveRow();

    unsigned int subtableCount = 0;
    // Update table keywords
    TableRecord kws = table.keywordSet();
    if(kws.nfields() != keywords.size()) {
        steps += kws.nfields();
        if(pp != NULL) pp->setSteps(steps);
    
        // Clear out old keywords
        for(unsigned int i = 0; i < keywords.size(); i++)
            delete keywords.at(i);
        keywords.clear();
        if(pp != NULL) pp->step();
        
        vector<TBKeyword*>* v = getKeywords(kws);
        // Put into keywords
        for(unsigned int i = 0; i < v->size(); i++) {
            TBKeyword* kw = v->at(i);
            if(kw->getType() == TBConstants::TYPE_TABLE) subtableCount++;
            keywords.push_back(kw);
            if(pp != NULL) pp->step();
        }
        delete v;
    }

    // Update table fields
    ROTableRow row(table);
    Vector<String> colNames = row.columnNames();
    TableDesc tdesc = table.tableDesc();
    void** fieldPtrs = (void**) new unsigned int*[colNames.nelements()];

    bool updateFields = colNames.nelements() != fields.size();
    if(updateFields) {
        steps -= fields.size();
        steps += 2 * colNames.nelements();
        if(pp != NULL) pp->setSteps(steps);
    
        // Clear out old fields
        for(unsigned int i = 0; i < fields.size(); i++)
            delete fields.at(i);
        fields.clear();
    }

    for(unsigned int i = 0; i < colNames.nelements(); i++) {
        ColumnDesc cdesc = tdesc.columnDesc(i);
        DataType t = row.record().type(row.record().fieldNumber(colNames(i)));
        String type = TBConstants::typeName(t);

        bool valid = true;
        if(t == TpString)
            fieldPtrs[i] = new RORecordFieldPtr<String>(row.record(),
                                                        colNames(i));
        else if(t == TpInt)
            fieldPtrs[i] = new RORecordFieldPtr<Int>(row.record(),
                                                     colNames(i));
        else if(t == TpFloat)
            fieldPtrs[i] = new RORecordFieldPtr<Float>(row.record(),
                                                       colNames(i));
        else if(t == TpDouble)
            fieldPtrs[i] = new RORecordFieldPtr<Double>(row.record(),
                                                        colNames(i));
        else if(t == TpBool)
            fieldPtrs[i] = new RORecordFieldPtr<Bool>(row.record(),
                                                      colNames(i));
        else if(t == TpUChar)
            fieldPtrs[i] = new RORecordFieldPtr<uChar>(row.record(),
                                                       colNames(i));
        else if(t == TpShort)
            fieldPtrs[i] = new RORecordFieldPtr<Short>(row.record(),
                                                       colNames(i));
        else if(t == TpUInt)
            fieldPtrs[i] = new RORecordFieldPtr<uInt>(row.record(),
                                                      colNames(i));
        else if(t == TpComplex)
            fieldPtrs[i] = new RORecordFieldPtr<Complex>(row.record(),
                                                         colNames(i));
        else if(t == TpDComplex)
            fieldPtrs[i] = new RORecordFieldPtr<DComplex>(row.record(),
                                                          colNames(i));
        else if(t == TpArrayDouble)
            fieldPtrs[i] = new RORecordFieldPtr<Array<Double> >(row.record(),
                                                                colNames(i));
        else if(t == TpArrayBool)
            fieldPtrs[i] = new RORecordFieldPtr<Array<Bool> >(row.record(),
                                                              colNames(i));
        else if(t == TpArrayUChar)
            fieldPtrs[i] = new RORecordFieldPtr<Array<uChar> >(row.record(),
                                                               colNames(i));
        else if(t == TpArrayShort)
            fieldPtrs[i] = new RORecordFieldPtr<Array<Short> >(row.record(),
                                                               colNames(i));
        else if(t == TpArrayInt)
            fieldPtrs[i] = new RORecordFieldPtr<Array<Int> >(row.record(),
                                                             colNames(i));
        else if(t == TpArrayUInt)
            fieldPtrs[i] = new RORecordFieldPtr<Array<uInt> >(row.record(),
                                                              colNames(i));
        else if(t == TpArrayFloat)
            fieldPtrs[i] = new RORecordFieldPtr<Array<Float> >(row.record(),
                                                               colNames(i));
        else if(t == TpArrayComplex)
            fieldPtrs[i] = new RORecordFieldPtr<Array<Complex> >(row.record(),
                                                                 colNames(i));
        else if(t == TpArrayDComplex)
            fieldPtrs[i] = new RORecordFieldPtr<Array<DComplex> >(row.record(),
                                                                  colNames(i));
        else if(t == TpArrayString)
            fieldPtrs[i] = new RORecordFieldPtr<Array<String> >(row.record(),
                                                                colNames(i));
        else valid = false;
                
        if(updateFields && t == TpDouble) { // Check if it's a date
            String comment = cdesc.comment();
            if(TBConstants::equalsIgnoreCase(comment,
               TBConstants::COMMENT_DATE)) {
                type = TBConstants::TYPE_DATE;
            }
        }

        if(pp != NULL) pp->step();

        if(updateFields && !valid && printdebug)
            TBConstants::dprint(TBConstants::DEBUG_HIGH,"Invalid field type.");
            
        if(updateFields) {
            TableRecord tr = cdesc.keywordSet();
            vector<TBKeyword*>* v = getKeywords(tr);

            String name = colNames[i];
        
            TBField* field = new TBField(name, type);

            // Add keywords to field
            for(unsigned int i = 0; i < v->size(); i++)
                field->addKeyword(v->at(i));
            delete v;

            fields.push_back(field);
            if(pp != NULL) pp->step();
        }
    }
    
    // Update writable vector
    writable.clear();
    writable.resize(fields.size(), false);
    try {
        table.reopenRW();
        for(unsigned int i = 0; i < fields.size(); i++) {
            writable[i] = table.isColumnWritable(fields[i]->getName());
        }
    } catch(...) { }

    // Update subtable rows
    if(subtableCount != subtableRows.size()) {
        steps += subtableCount;
        if(pp != NULL) pp->setSteps(steps);
        subtableRows.clear();
        subtableRows.resize(subtableCount);

        for(unsigned int i = 0; i < subtableRows.size(); i++)
            subtableRows[i] = -1;
        
        int j = -1;
        for(unsigned int i = 0; i < keywords.size(); i++) {
            TBKeyword* kw = keywords.at(i);
            if(kw->getType() == TBConstants::TYPE_TABLE) {
                j++;
                int r = totalRowsOf(kw->getValue()->asString());
                subtableRows[j] = r;
                if(pp != NULL) pp->step();
            }
        }
    }

    // Update data
    loadedRows = 0;
    
    for(unsigned int i = 0; i < data.size(); i++) {
        vector<TBData*>* dr = data.at(i);
        for(unsigned int j = 0; j < data.at(i)->size(); j++) {
            delete dr->at(j);
        }
        delete dr;
    }
    data.clear();
    
    if(parsedata) {
        for(int i = start; i < end; i++) {
            row.get(i);

            vector<TBData*>* r2 = new vector<TBData*>();
            for(unsigned int j = 0; j < colNames.nelements(); j++) {
                DataType t = row.record().type(j);

                TBData* val = NULL;
                if(t == TpString) {
                    val = new TBDataString(**((RORecordFieldPtr<String>*)
                                           fieldPtrs[j]));
                } else if(t == TpFloat) {
                    val = new TBDataFloat(**((RORecordFieldPtr<Float>*)
                                           fieldPtrs[j]));
                } else if(t == TpInt) {
                    val = new TBDataInt(**((RORecordFieldPtr<Int>*)
                                           fieldPtrs[j]));
                } else if(t == TpDouble) {
                    String comment = tdesc.columnDesc(j).comment();
                    double v = **((RORecordFieldPtr<Double>*)fieldPtrs[j]);
                    if(TBConstants::equalsIgnoreCase(comment,
                       TBConstants::COMMENT_DATE)) {
                        val = new TBDataDate(v);
                    } else {
                        val = new TBDataDouble(v);
                    }
                } else if(t == TpBool) {
                    val = new TBDataBool(**((RORecordFieldPtr<Bool>*)
                                           fieldPtrs[j]));
                } else if(t == TpChar) {
                    val = new TBDataChar(**((RORecordFieldPtr<Char>*)
                                           fieldPtrs[j]));
                } else if(t == TpUChar) {
                    val = new TBDataUChar(**((RORecordFieldPtr<uChar>*)
                                           fieldPtrs[j]));
                } else if(t == TpShort) {
                    val = new TBDataShort(**((RORecordFieldPtr<Short>*)
                                           fieldPtrs[j]));
                } else if(t == TpUInt) {
                    val = new TBDataUInt(**((RORecordFieldPtr<uInt>*)
                                           fieldPtrs[j]));
                } else if(t == TpComplex) {
                    val = new TBDataComplex(**((RORecordFieldPtr<Complex>*)
                                           fieldPtrs[j]));
                } else if(t == TpDComplex) {
                    val = new TBDataDComplex(**((RORecordFieldPtr<DComplex>*)
                                           fieldPtrs[j]));
                } else if(t == TpArrayDouble) {
                    val = new TBArrayDataDouble(
                              **((RORecordFieldPtr<Array<Double> >*)
                                 fieldPtrs[j]), full);
                } else if(t == TpArrayBool) {
                    val = new TBArrayDataBool(
                              **((RORecordFieldPtr<Array<Bool> >*)
                                 fieldPtrs[j]), full);
                } else if(t == TpArrayUChar) {
                    val = new TBArrayDataUChar(
                              **((RORecordFieldPtr<Array<uChar> >*)
                                      fieldPtrs[j]), full);
                } else if(t == TpArrayChar) {
                    val = new TBArrayDataChar(
                              **((RORecordFieldPtr<Array<Char> >*)
                                 fieldPtrs[j]));
                } else if(t == TpArrayShort) {
                    val = new TBArrayDataShort(
                              **((RORecordFieldPtr<Array<Short> >*)
                                      fieldPtrs[j]), full);
                } else if(t == TpArrayInt) {
                    val = new TBArrayDataInt(
                              **((RORecordFieldPtr<Array<Int> >*)
                                      fieldPtrs[j]), full);
                } else if(t == TpArrayUInt) {
                    val = new TBArrayDataUInt(
                              **((RORecordFieldPtr<Array<uInt> >*)
                                      fieldPtrs[j]), full);
                } else if(t == TpArrayFloat) {
                    val = new TBArrayDataFloat(
                              **((RORecordFieldPtr<Array<Float> >*)
                                      fieldPtrs[j]), full);
                } else if(t == TpArrayComplex) {
                    val = new TBArrayDataComplex(
                              **((RORecordFieldPtr<Array<Complex> >*)
                                      fieldPtrs[j]), full);
                } else if(t == TpArrayDComplex) {
                    val = new TBArrayDataDComplex(
                              **((RORecordFieldPtr<Array<DComplex> >*)
                                      fieldPtrs[j]), full);
                } else if(t == TpArrayString) {
                    val = new TBArrayDataString(
                              **((RORecordFieldPtr<Array<String> >*)
                                      fieldPtrs[j]), full);
                }
                
                r2->push_back(val);
            }
            data.push_back(r2);
            loadedRows++;

            if(pp != NULL && ((i - start) % 10) == 0) pp->step();
        }
    }

    delete [] fieldPtrs;
    if(pp != NULL) pp->done();
    return Result("", true);

    } catch(AipsError x) {
        return Result(x.getMesg(), false);
    } catch(...) {
        return Result("Unknown error occured during load!", false);
    }
}

void TBTableDriverDirect::loadArray(TBArrayData* d, unsigned int r,
                                    unsigned int c) {
    //checkTaqlTable();
    
    Table table = m_table;

    ROTableRow row(table);
    row.get(r);
    
    DataType t = row.record().type(c);

    if(t == TpArrayDouble) {
        RORecordFieldPtr<Array<Double> > p(row.record(), c);
        ((TBArrayDataDouble*)d)->load(*p);
    } else if(t == TpArrayBool) {
        RORecordFieldPtr<Array<Bool> > p(row.record(), c);
        ((TBArrayDataBool*)d)->load(*p);
    } else if(t == TpArrayChar) {
        RORecordFieldPtr<Array<Char> > p(row.record(), c);
        ((TBArrayDataChar*)d)->load(*p);
    } else if(t == TpArrayUChar) {
        RORecordFieldPtr<Array<uChar> > p(row.record(), c);
        ((TBArrayDataUChar*)d)->load(*p);
    } else if(t == TpArrayShort) {
        RORecordFieldPtr<Array<Short> > p(row.record(), c);
        ((TBArrayDataShort*)d)->load(*p);
    } else if(t == TpArrayInt) {
        RORecordFieldPtr<Array<Int> > p(row.record(), c);
        ((TBArrayDataInt*)d)->load(*p);
    } else if(t == TpArrayUInt) {
        RORecordFieldPtr<Array<uInt> > p(row.record(), c);
        ((TBArrayDataUInt*)d)->load(*p);
    } else if(t == TpArrayFloat) {
        RORecordFieldPtr<Array<Float> > p(row.record(), c);
        ((TBArrayDataFloat*)d)->load(*p);
    } else if(t == TpArrayComplex) {
        RORecordFieldPtr<Array<Complex> > p(row.record(), c);
        ((TBArrayDataComplex*)d)->load(*p);
    } else if(t == TpArrayDComplex) {
        RORecordFieldPtr<Array<DComplex> > p(row.record(), c);
        ((TBArrayDataDComplex*)d)->load(*p);
    } else if(t == TpArrayString) {
        RORecordFieldPtr<Array<String> > p(row.record(), c);
        ((TBArrayDataString*)d)->load(*p);
    }
}

vector<int> TBTableDriverDirect::dimensionsOf(unsigned int col) {
    vector<int> d;
    
    if(col < 0 || col >= fields.size()) return d;
    
    String type = fields.at(col)->getType();
    if(!TBConstants::typeIsArray(type)) return d;
    
    Table table = m_table;
    
    ColumnDesc acd = table.tableDesc().columnDesc(col);
    IPosition shape = acd.shape();

    // If the shape is variable, take a guess by loading the first row
    if(shape.size() == 0) {
        ROTableRow row(table);
        row.get(0);

        if(type == TBConstants::TYPE_ARRAY_DOUBLE) {
            RORecordFieldPtr<Array<Double> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_BOOL) {
            RORecordFieldPtr<Array<Bool> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_UCHAR) {
            RORecordFieldPtr<Array<uChar> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_SHORT) {
            RORecordFieldPtr<Array<Short> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_INT) {
            RORecordFieldPtr<Array<Int> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_UINT) {
            RORecordFieldPtr<Array<uInt> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_FLOAT) {
            RORecordFieldPtr<Array<Float> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_COMPLEX) {
            RORecordFieldPtr<Array<Complex> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_DCOMPLEX) {
            RORecordFieldPtr<Array<DComplex> > p(row.record(), col);
            shape = (*p).shape();
        } else if(type == TBConstants::TYPE_ARRAY_STRING) {
            RORecordFieldPtr<Array<String> > p(row.record(), col);
            shape = (*p).shape();
        }
    }

    for(unsigned int i = 0; i < shape.size(); i++)
        d.push_back(shape(i));
    
    return d;
}

Result TBTableDriverDirect::editData(unsigned int row, unsigned int col,
                                     TBData* newVal, vector<int>* d) {
    if(taql) return Result("Cannot edit TaQL tables.", false);
    
    Table table = m_table;
    table.reopenRW();
    TableColumn column(table, col);

    String fName = fields.at(col)->getName();
    String type = newVal->getType();
    bool a = d != NULL && d->size() > 0;
    IPosition* pos = NULL;
    if(a) {        
        pos = new IPosition(d->size());
        for(unsigned int i = 0; i < d->size(); i++)
            (*pos)[i] = d->at(i);
    }

    if(type == TBConstants::TYPE_STRING || type == TBConstants::TYPE_TABLE) {
        if(!a) column.putScalar(row, newVal->asString());
        else {
            ArrayColumn<String> arrayCol(table, fName);
            Array<String> array = arrayCol(row);
            array(*pos) = newVal->asString();
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_FLOAT) {
        if(!a) column.putScalar(row, newVal->asFloat());
        else {
            ArrayColumn<Float> arrayCol(table, fName);
            Array<Float> array = arrayCol(row);
            array(*pos) = newVal->asFloat();
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_DOUBLE ||
              type == TBConstants::TYPE_DATE) {
        if(!a) column.putScalar(row, newVal->asDouble());
        else {
            ArrayColumn<Double> arrayCol(table, fName);
            Array<Double> array = arrayCol(row);
            array(*pos) = newVal->asDouble();
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_INT) {
        if(!a) column.putScalar(row, newVal->asInt());
        else {
            ArrayColumn<Int> arrayCol(table, fName);
            Array<Int> array = arrayCol(row);
            array(*pos) = newVal->asInt();
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_UINT) {
        if(!a) column.putScalar(row, newVal->asUInt());
        else {
            ArrayColumn<uInt> arrayCol(table, fName);
            Array<uInt> array = arrayCol(row);
            array(*pos) = newVal->asUInt();
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_BOOL) {
        if(!a) column.putScalar(row, newVal->asBool());
        else {
            ArrayColumn<Bool> arrayCol(table, fName);
            Array<Bool> array = arrayCol(row);
            array(*pos) = newVal->asBool();
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_UCHAR) {
        if(!a) column.putScalar(row, newVal->asUChar());
        else {
            ArrayColumn<uChar> arrayCol(table, fName);
            Array<uChar> array = arrayCol(row);
            array(*pos) = newVal->asUChar();
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_CHAR) {
        if(!a) column.putScalar(row, newVal->asChar());
        else return Result("Char ArrayColumn not supported by CASA.", false);
    } else if(type == TBConstants::TYPE_SHORT) {
        if(!a) column.putScalar(row, newVal->asShort());
        else {
            ArrayColumn<Short> arrayCol(table, fName);
            Array<Short> array = arrayCol(row);
            array(*pos) = newVal->asShort();
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_COMPLEX) {
        pair<float, float> c = newVal->asComplex();
        if(!a) column.putScalar(row, Complex(c.first, c.second));
        else {
            ArrayColumn<Complex> arrayCol(table, fName);
            Array<Complex> array = arrayCol(row);
            array(*pos) = Complex(c.first, c.second);
            arrayCol.put(row, array);
        }
    } else if(type == TBConstants::TYPE_DCOMPLEX) {
        pair<double, double> c = newVal->asDComplex();
        if(!a) column.putScalar(row, DComplex(c.first, c.second));
        else {
            ArrayColumn<DComplex> arrayCol(table, fName);
            Array<DComplex> array = arrayCol(row);
            array(*pos) = DComplex(c.first, c.second);
            arrayCol.put(row, array);
        }
    } else return Result("Type not supported.", false);
    
    if(!a) {
        row %= data.size();
        data.at(row)->at(col)->setValue(*newVal);
    }
    
    if(pos != NULL) delete pos;
    
    return Result("", true);
}

int TBTableDriverDirect::totalRowsOf(String location) {
    if(location == m_table.tableName()) return m_table.nrow();
    else return Table(location).nrow();
}

Result TBTableDriverDirect::insertRows(int n) {
    if(taql) return Result("Cannot edit TaQL tables.", false);
    
    try {
        Table table = m_table;
        if(!table.canAddRow())
            return Result("Table does not support insertion of rows.", false);
        table.reopenRW();
        table.addRow(n, true);
        return Result("", true);
    } catch(AipsError er) {
        return Result(er.getMesg(), false);
    } catch(...) {
        return Result("Unknown exception.", false);
    }
}

Result TBTableDriverDirect::deleteRows(vector<int> r) {
    if(taql) return Result("Cannot edit TaQL tables.", false);
    
    try {
        Table table = m_table;
        if(!table.canRemoveRow())
            return Result("Table does not support deletion of rows.", false);
        table.reopenRW();
        for(unsigned int i = 0; i < r.size(); i++) {
            table.removeRow(r.at(i));
        }
        // update totalRows
        totalRows = table.nrow();
        return Result("", true);
    } catch(AipsError er) {
        return Result(er.getMesg(), false);
    } catch(...) {
        return Result("Unknown exception.", false);
    }
}

vector<TBKeyword*>* TBTableDriverDirect::getKeywords(RecordInterface& kws) {
    vector<TBKeyword*>* v = new vector<TBKeyword*>();
    for(unsigned int i = 0; i < kws.nfields(); i++) {
        RecordFieldId rfid(i);
        DataType t = kws.type(i);
        String type = TBConstants::typeName(t);
        String name = kws.name(rfid);

        bool valid = true;
        TBData* d = NULL;
        if(t == TpString) d = new TBDataString(kws.asString(rfid));
        else if(t == TpFloat) d = new TBDataFloat(kws.asFloat(rfid));
        else if(t == TpDouble) d = new TBDataDouble(kws.asDouble(rfid));
        else if(t == TpInt) d = new TBDataInt(kws.asInt(rfid));
        else if(t == TpUInt) d = new TBDataUInt(kws.asuInt(rfid));
        else if(t == TpBool) d = new TBDataBool(kws.asBool(rfid));
        else if(t == TpChar) d = new TBDataChar(kws.asuChar(rfid));
        else if(t == TpUChar) d = new TBDataUChar(kws.asuChar(rfid));
        else if(t == TpShort) d = new TBDataShort(kws.asShort(rfid));
        else if(t == TpComplex) d = new TBDataComplex(kws.asComplex(rfid));
        else if(t == TpDComplex) d = new TBDataDComplex(kws.asDComplex(rfid));
        else if(t == TpTable) {
            TableRecord* tr = static_cast<TableRecord*>(&kws);
            if(tr != NULL) d = new TBDataTable(tr->asTable(rfid).tableName());
            else valid = false;
        }
        else if(t == TpRecord) d = new TBDataRecord(kws.asRecord(rfid));
        else if(t == TpArrayBool) {
            Array<Bool> arr = kws.asArrayBool(rfid);
            d = new TBArrayDataBool(arr);
            ((TBArrayDataBool*)d)->load(arr);
        } else if(t == TpArrayFloat) {
            Array<Float> arr = kws.asArrayFloat(rfid);
            d = new TBArrayDataFloat(arr);
            ((TBArrayDataFloat*)d)->load(arr);
        } else if(t == TpArrayDouble) {
            Array<Double> arr = kws.asArrayDouble(rfid);
            d = new TBArrayDataDouble(arr);
            ((TBArrayDataDouble*)d)->load(arr);
        } else if(t == TpArrayUChar) {
            Array<uChar> arr = kws.asArrayuChar(rfid);
            d = new TBArrayDataUChar(arr);
            ((TBArrayDataUChar*)d)->load(arr);
        } else if(t == TpArrayShort) {
            Array<Short> arr = kws.asArrayShort(rfid);
            d = new TBArrayDataShort(arr);
            ((TBArrayDataShort*)d)->load(arr);
        } else if(t == TpArrayInt) {
            Array<Int> arr = kws.asArrayInt(rfid);
            d = new TBArrayDataInt(arr);
            ((TBArrayDataInt*)d)->load(arr);
        } else if(t == TpArrayUInt) {
            Array<uInt> arr = kws.asArrayuInt(rfid);
            d = new TBArrayDataUInt(arr);
            ((TBArrayDataUInt*)d)->load(arr);
        } else if(t == TpArrayComplex) {
            Array<Complex> arr = kws.asArrayComplex(rfid);
            d = new TBArrayDataComplex(arr);
            ((TBArrayDataComplex*)d)->load(arr);
        } else if(t == TpArrayDComplex) {
            Array<DComplex> arr = kws.asArrayDComplex(rfid);
            d = new TBArrayDataDComplex(arr);
            ((TBArrayDataDComplex*)d)->load(arr);
        } else if(t == TpArrayString) {
            Array<String> arr = kws.asArrayString(rfid);
            d = new TBArrayDataString(arr);
            ((TBArrayDataString*)d)->load(arr);
        } else valid = false;
        
        if(!valid)
            TBConstants::dprint(TBConstants::DEBUG_HIGH,
                                "Keyword type not supported.");

        if(valid && d != NULL) {
            TBKeyword* keyword = new TBKeyword(name, *d);
            delete d;
            v->push_back(keyword);
        }
    }
    return v;
}

/*
//////////////////////////////////
// TBTABLEDRIVERXML DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

TBTableDriverXML::TBTableDriverXML(TableParams* tp, TBTable* t) :
                                                    TBTableDriver(tp, t) {
    if(tp->dParams->parser == HOME) parser = new TBHomeParser(tp);
    else if(tp->dParams->parser == XERCES_DOM)
        parser =new TBXercesDOMParser(tp);
    else if(tp->dParams->parser == XERCES_SAX)
        parser = new TBXercesSAXParser(tp);
    else throw "Unsupported Parser Class.";
}

TBTableDriverXML::~TBTableDriverXML() {
    delete parser;
}

// Public Methods //

Result TBTableDriverXML::loadRows(int start, int num, bool full,
                                  vector<String>* f, bool parsedata,
                                  ProgressHelper* pp) {
    int steps = 3;
    if(pp != NULL) {
        pp->reset("Loading rows...");
        pp->setSteps(steps);
    }
    
    stringstream ss;
    ss << "SELECT";

    if(f != NULL && f->size() > 0) {
        for(unsigned int i = 0; i < f->size(); i++) {
            ss << " " << f->at(i);
            if(i < f->size() - 1) ss << ',';
        }
    }
        
    ss << " FROM " << location << " <START = " << start;
    ss << " number = " << num << " >";

    String q = full ? TBConstants::QUERY_FULL : TBConstants::QUERY_QUERY;
    Result r = query(q, ss.str());
    if(pp != NULL) pp->step();

    if(r.valid) {
        String xml = r.result;
        if(printdebug) {
            TBConstants::dprint(TBConstants::DEBUG_MED,
                                "DataDriver returned XML string.");
            TBConstants::dprint(TBConstants::DEBUG_LOW, xml);
        }
        insertRow = false;
        removeRow = false;
        for(unsigned int i = 0; i < fields.size(); i++) delete fields.at(i);
        fields.clear();
        for(unsigned int i = 0; i < keywords.size(); i++)
            delete keywords.at(i);
        keywords.clear();
        for(unsigned int i = 0; i < data.size(); i++) {
            vector<TBData*>* dr = data.at(i);
            for(unsigned int j = 0; j < dr->size(); j++)
                delete dr->at(j);
            delete dr;
        }
        data.clear();
        totalRows = 0;
        loadedRows = 0;
        bool ready;
        if(pp != NULL) pp->step();
        Result r = parser->parse(&xml, parsedata);
        if(pp != NULL) pp->step();
        ready = r.valid;
        if(!r.valid) return r;

        // Update subtableRows
        int subtableCount = 0;
        steps += keywords.size();
        if(pp != NULL) pp->setSteps(steps);
        for(unsigned int i = 0; i < keywords.size(); i++) {
            String type = keywords.at(i)->getType();
            if(TBConstants::typeIsTable(type)) subtableCount++;
            if(pp != NULL) pp->step();
        }

        if(parsedata && subtableCount != (int)subtableRows.size()) {
            steps += subtableCount;
            if(pp != NULL) pp->setSteps(steps);
            subtableRows.clear();
            subtableRows.resize(subtableCount);

            for(unsigned int i = 0; i < subtableRows.size(); i++)
                subtableRows[i] = -1;

            int j = -1;
            for(unsigned int i = 0; i < keywords.size(); i++) {
                TBKeyword* kw = keywords.at(i);
                if(kw->getType() == TBConstants::TYPE_TABLE) {
                    j++;
                    int r = totalRowsOf(kw->getValue()->asString());
                    subtableRows[j] = r;
                    if(pp != NULL) pp->step();
                }
            }
        }
        
        if(parsedata) {
            // parse data into data
            vector<vector<String>*>* data = parser->getData();
            data.resize(data->size());
            for(unsigned int i = 0; i < data->size(); i++) {
                vector<TBData*>* v = new vector<TBData*>(fields.size());
                for(unsigned int j = 0; j < fields.size(); j++) {
                    String d = data->at(i)->at(j);
                    TBData* td = TBData::create(d, fields.at(j)->getType());
                    (*v)[j] = td;
                }
                data[i] = v;
            }
        }
        
        return Result("", true);
    } else {
        return r;
    }
}

void TBTableDriverXML::loadArray(TBArrayData* d, unsigned int row,
                                    unsigned int col) {
    
}

vector<int> TBTableDriverXML::dimensionsOf(unsigned int col) {
    vector<int> d;
    String type = fields.at(col)->getType();
    
    if(TBConstants::typeIsArray(type)) {
        if(data.size() > 0) {
            TBArrayData* sd = (TBArrayData*)data.at(0)->at(col);
            return sd->getShape();
        } else {
            TBTable t(location, dp);
            vector<String> c;
            c.push_back(fields.at(col)->getName());
            Result r = t.loadRows(0, 1, true, &c);

            if(r.valid && t.getLoadedRows() == 1 && t.getNumFields() == 1) {
                TBArrayData* sd = (TBArrayData*)t.dataAt(0, 0);
                if(sd != NULL) return sd->getShape();
            }
        }
    }
    
    return d;
}

Result TBTableDriverXML::editData(unsigned int row, unsigned int col,
                               TBData* newVal, vector<int>* d) {
    stringstream ss;
    ss << "<QUERY> SELECT FROM " << location << " </QUERY>\n<COMMAND>\n";

    if(d == NULL || d->size() == 0) {
        ss << "<UPDATE row = " << row << " col = " << col << " val = \"";
        ss << newVal->asString() << "\" >";
    } else {
        ss << "<ARRAYUPDATE row = " << row << " col = " << col << " >\n";
        ss << "<ARRAYCELLUPDATE coordinates = [ ";
        for(unsigned int i = 0; i < d->size(); i++) ss << d->at(i) << ' ';
        ss << "] val = " << newVal->asString() << " >\n</ARRAYUPDATE>";
    }

    ss << "\n</COMMAND>";

    Result r = query(TBConstants::QUERY_UPDATE, ss.str());
    if(r.valid && (d == NULL || d->size() == 0)) {
        // Update internal data representation
        int r = row;
        if(r >= (int)data.size()) r -= table->getRowIndex();
        if(d == NULL || d->size() == 0) {
            TBData* dat = TBData::create(*newVal);
            (*data.at(r))[col]->setValue(*dat);
            delete dat;
        }
    }

    return r;
}

int TBTableDriverXML::totalRowsOf(String location) {
    try {
        TBTable t(location, new DriverParams(dp), false);
        t.setPrintDebug(false);
        Result r = t.loadRows(0, 1, false, NULL, false);
        
        int tr = 0;
        if(r.valid) tr = t.getTotalRows();
        return tr;
    } catch(const char* s) {
        TBConstants::dprint(TBConstants::DEBUG_HIGH,  "Table " + location +
                            " threw: " + s);
        return 0;
    } catch(...) {
        TBConstants::dprint(TBConstants::DEBUG_HIGH,
                "Unknown exception thrown.");
        return 0;
    }
}

void TBTableDriverXML::setPrintDebug(bool pdb) {
    printdebug = pdb;
    parser->setPrintDebug(pdb);
}

Result TBTableDriverXML::insertRows(int n) {
    stringstream ss;
    ss << "<QUERY> SELECT FROM " << location << " </QUERY>\n<COMMAND>\n";
    for(int i = 0; i < n; i++) ss << "<ADDROW>";
    ss << "\n</COMMAND>";
    return query(TBConstants::QUERY_UPDATE, ss.str());
}

Result TBTableDriverXML::deleteRows(vector<int> r) {
    stringstream ss;
    ss << "<QUERY> SELECT FROM " << location << " </QUERY>\n<COMMAND>\n";
    for(unsigned int i = 0; i < r.size(); i++) {
        ss << "<DELROW " << r.at(i) << " >";
    }
    ss << "\n</COMMAND>";
    Result res = query(TBConstants::QUERY_UPDATE, ss.str());
    if(res.valid) {
        totalRows = totalRowsOf(location);
    }
    return res;
}
*/

}
