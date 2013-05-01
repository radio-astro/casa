//# SDTableIterator: an SDIterator for Tables from SDFITS data
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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
//#
//# $Id$

#ifndef DISH_SDTABLEITERATOR_H
#define DISH_SDTABLEITERATOR_H


#include <dish/SDIterators/SDRecord.h>
#include <dish/SDIterators/SDIterator.h>
#include <dish/SDIterators/SDTabRecCopier.h>

#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/Logging/LogIO.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/Table.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class CopyRecordToRecord;
class SDIterAxes;
class ROTableRow;
class TableRow;

// <summary>
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SDRecord
//   <li> DISH design
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="2000/07/19">
//   <li> FELO velocity axis isn't handled quite right, especially for large velocities
//   <li> Try and improve the speed of the copyAll() function
//   <li> Separate copyAll() into its constituent parts so that only those parts that
//        are needed when assembling vectors or returning just the header or desc are filled.
// </todo>

class SDTableIterator : public SDIterator
{
public:

    // construct an iterator from a FLAT Table
    // for now, this must be an SDFITS table
    // the selection string is currently ignored

    SDTableIterator(const String &tableName, const Record &selection,
		    Table::TableOption opt = Table::Old,
		    TableLock::LockOption lockoption = TableLock::DefaultLocking);

    // construct from another iterator
    SDTableIterator(const SDTableIterator& other);

    // construct from another with selection
    SDTableIterator(const SDTableIterator& other, const Record &selection);

    ~SDTableIterator() { cleanup(); }

    // assiment operator
    SDTableIterator &operator=(const SDTableIterator& other);

    // what type of sditerator is this ("Table")
    virtual String type() const {return "Table";}

    // Unlock this table
    Bool unlock();

    // Lock this table.  Try nattempts times.
    Bool lock(uInt nattempts=0);

    // Resets the iterator to the top
    void origin() {thisRow_ = 0;}

    // are there any more SDRecords after the current one
    Bool more();
    // step to the next one, if at end, nothing changes
    SDIterator& operator++(int) { if (more()) thisRow_++; return *this;}
    SDIterator& operator++() { if (more()) thisRow_++; return *this;}
    // step back, if at end, nothing changes
    SDIterator& operator--(int) { if (thisRow_ > 0) thisRow_--; return *this;}
    SDIterator& operator--() { if (thisRow_ > 0) thisRow_--; return *this;}

    // return the current row number
    uInt where() const { return thisRow_; }

    // how many SDRecords are there
    uInt nrecords();

    // resync with the disk contents
    void resync() {tab_->resync();}

    // flush the disk contents - a no-op if this is a read-only sditerator
    void flush() {if (isWritable()) { tab_->flush(); } }

    // re-select
    void reselect();

    // make a deep copy
    void deepCopy(const String &newName) 
    { tab_->deepCopy(newName, Table::NewNoReplace);}

    // return the current SDRecord
    const SDRecord &get() { copyAll(); return rec_;}

    // return just the data portion of the current SDRecord
    const Record &getData() { copyData(); return rec_.data();}

    // return just the header portion of the current SDRecord
    const Record &getHeader() {copyHeader(); return rec_.header();}

    // return just the other portion of the current SDRecord
    const Record &getOther() {copyOther(); return rec_.other();}

    // return just the hist portion of the current SDRecord
    const Array<String> getHist();

    // replace the current SDRecord with a new one
    Bool put(const SDRecord& rec);

    // add a new record at the end of the iterator
    Bool appendRec(const SDRecord &rec);

    // delete the record at the current location
    Bool deleteRec();

    // see if this iterator is writable
    // currently always returns False.
    Bool isWritable() const { return tab_->isWritable();}

    // get a record with structure like the current working SDRecord which
    // contains only fields which are scalar string fields - the value of those
    // fields is just a boolean.  This is necessary so that the proper
    // parsing of the selection when given as a string can be done for
    // each field without having to duplicate that code in both glish
    // and here in C++.
    const Record &stringFields() const { return stringFields_;}
	   
    // return the name of iterator, the same as the table name
    String name() const { return tab_->tableName(); }

    // other things may not work if ok == False, this needs to be cleaned up
    Bool ok() const { return ok_;}
    
private:
    Bool ok_;
    Bool hasOutputCols_;
    SDRecord rec_;
    Table *tab_;
    uInt thisRow_;

    Record selection_;

    Record stringFields_;

    // The axis info
    SDIterAxes *axes_;

    LogIO los_;

    CopyRecordToRecord *headerCopier_, *rwHeaderCopier_;
    PtrBlock<CopyRecordToRecord *> otherCopiers_, rwOtherCopiers_;
    SimpleOrderedMap<String, Int> otherCopiersMap_;

    Vector<String> ctypes_, cunits_;
    Vector<Double> crvals_, crpixs_, cdelts_, crotas_;

    RecordFieldPtr<Record> chanFreqRec_;
    RecordFieldPtr<String> refframe_;

    // strictly Float data columns at this point
    ROArrayColumn<Float> data_in_;
    ArrayColumn<Float> data_out_;
    RecordFieldPtr<Array<Float> > arr_;

    ROArrayColumn<Bool> flag_in_;
    ArrayColumn<Bool> flag_out_;
    RecordFieldPtr<Array<Bool> > flag_;

    ROArrayColumn<Float> weight_in_;
    ArrayColumn<Float> weight_out_;
    RecordFieldPtr<Array<Float> > weight_;

    ROArrayColumn<Float> sigma_in_;
    ArrayColumn<Float> sigma_out_;
    RecordFieldPtr<Array<Float> > sigma_;

    RecordFieldPtr<String> veldef_, transition_;
    RecordFieldPtr<Int> scan_number_;
    RecordFieldPtr<Float> subscan_;
    RecordFieldPtr<Double> tdurField_p, texpField_p, refFreq_, chanWidth_;
    RecordFieldPtr<Record> azel_, direction_, refDirection_, time_,
	telescopePosition_;
    RecordFieldPtr<String> telescope_;
    RecordFieldPtr<Array<String> > corrType_;

    SDTabRecCopier restFreq_p, tdur_p, texp_p, resolution_p, bw_p, pressure_p,
	tdew_p, tamb_p, windDir_p, windSpeed_p;

    ROTableColumn scan_, equinoxCol_, elevation_, azimuth_, velinfo_,
	tsysCol_, trxCol_, tcalCol_, timeCol_, dateObsCol_,
	siteLat_, siteLong_, siteElev_, transitionCol_, moleculeCol_;
    ROArrayColumn<Float> arrTsysCol_, arrTrxCol_, arrTcalCol_;
    TableColumn scanOut_, equinoxOutCol_, elevationOut_, azimuthOut_, velinfoOut_,
	tsysOutCol_, trxOutCol_, tcalOutCol_, timeOutCol_, dateObsOutCol_,
	siteLatOut_, siteLongOut_, siteElevOut_, transitionOutCol_, moleculeOutCol_;
    ArrayColumn<Float> arrTsysOutCol_, arrTrxOutCol_, arrTcalOutCol_;
    TableRow *rwTableRow_, *oldTableRow_;
    ROTableRow *tableRow_;
    
    RecordFieldPtr<Array<Float> > tcal_, trx_, tsys_;

    ROArrayColumn<String> historyCol_;
    ArrayColumn<String> historyOutCol_;

    ROArrayColumn<Double> refDirCol_;
    ArrayColumn<Double> refDirOutCol_;

    // static maps
    static SimpleOrderedMap<String, String> fitsHeaderMap_, itHeaderMap_, emptyMap_;
    static SimpleOrderedMap<String, Bool> fitsSpecialMap_;
    static Bool staticsOK_;

    void cleanup();
    void cleanupMost();
    void copyAll();
    void copyData();
    void copyHeader();
    void copyHist();
    void copyOther();
    Bool replaceAll(const SDRecord& rec, Bool rowIsNew = False);
    Bool ensureSDFITSTable();
    void init();
    void initStatics();
    void applySelection(const Record &selection);
    void createTable(const String& tableName, TableLock::LockOption lockoption,
		     Table::TableOption opt);
    void ensureOutputColumns();
    void setRowShape();
    void initStringFields(Record &fieldsRec, const Record &modelRec);

    String makeFullAxisSel(const String &axisSel, const Vector<String> &ctypeCols, 
			   const Vector<String> &axisCols);

    String parseSelection(const Record& selection, const TableDesc& layout,
			  Vector<uInt> &specalRows, Bool &hasSpecial);
    Bool parseSpecialFields(const Record& selection, Vector<uInt> &specialRows);
    Vector<uInt> parseRow(const Record& selection, Int maxRow);
    Bool parseRecord(const Record& selection, 
		     const SimpleOrderedMap<String, String>& map, 
		     const TableDesc& layout, String& result);
    Bool parseDescRecord(const Record& selection, String& result);
    Double getRefPix(const Vector<Double> &chanVals, Double refVal, Double &delta);
    static void getSpecialVectors(SDIterAxes &axes, Vector<Double> &uts, Vector<String> &dates, 
				  uInt nrecs, uInt nrow, Bool doUT, Bool doDate);
    Record getVectorShortCuts(const Record &recTemplate);
    // This will likely be more useful elsewhere eventually
    // Does the table appear to be from an SDFITS file
    Bool isSDFITSTable(const Table& tab);
    void deleteCopiers();
    void setCopiers();
    Bool copyRecord(Record &outRec, const Record &inRec);
};


} //# NAMESPACE CASA - END

#endif
