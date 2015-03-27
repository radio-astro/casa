//# SDMSIterator: an SDIterator for single dish data in a MeasurementSet.
//# Copyright (C) 2000,2001,2002
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

#ifndef DISH_SDMSITERATOR_H
#define DISH_SDMSITERATOR_H


#include <dish/SDIterators/SDRecord.h>
#include <dish/SDIterators/SDIterator.h>

#include <casa/Arrays/Array.h>
#include <casa/Containers/RecordField.h>
#include <casa/BasicSL/Complex.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MSOper/MSValidIds.h>
#include <ms/MSSel/MSDopplerIndex.h>
#include <ms/MSSel/MSFeedIndex.h>
#include <ms/MSSel/MSFreqOffIndex.h>
#include <ms/MSSel/MSPointingIndex.h>
#include <ms/MSSel/MSSourceIndex.h>
#include <ms/MSSel/MSSysCalIndex.h>
#include <ms/MSSel/MSWeatherIndex.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRow.h>
#include <casa/BasicSL/String.h>
#include <casa/Quanta/Unit.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class ROMSColumns;
class SDIterFieldCopier;
class ColumnsIndex;
class TableRow;
template <class T> class Block;
template <class T> class Vector;

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
// <todo asof="yyyy/mm/dd">
//   <li>
// </todo>

class SDMSIterator : public SDIterator
{
public:
    SDMSIterator(const String &msName, const Record &selection,
		 Table::TableOption opt = Table::Old,
		 TableLock::LockOption lockoptions = TableLock::DefaultLocking,
		 Bool useCorrectedData = False);

    // construct from another MS iterator
    SDMSIterator(const SDMSIterator& other);

    // construct from another with selection
    SDMSIterator(const SDMSIterator& other, const Record &selection);

    ~SDMSIterator() { cleanup(); }

    // assiment operator
    SDMSIterator &operator=(const SDMSIterator& other);

    // what type of sditerator is this ("MeasurementSet")
    virtual String type() const {return "MeasurementSet";}

    // Unlock this table
    Bool unlock();

    // Lock this table.  Try nattempts times.
    Bool lock(uInt nattempts=0);

    // reset the iterator to the top
    void origin() {thisRow_p = 0;}

    // are there any more SDRecords after the current one
    Bool more() { return (thisRow_p < (nrecords()-1)); }
    // step to the next one, if at end, nothing changes
    SDIterator& operator++(int) { if (more()) thisRow_p++; return *this;}
    SDIterator& operator++() { if (more()) thisRow_p++; return *this;}
    // step back, if at end, nothing changes
    SDIterator& operator--(int) { if (thisRow_p > 0) thisRow_p--; return *this;}
    SDIterator& operator--() { if (thisRow_p > 0) thisRow_p--; return *this;}

    // return the current row number
    uInt where() const { return thisRow_p; }

    // how many SDRecords are there
    uInt nrecords() {return ms_p->nrow();}

    // resync with the disk contents
    void resync();

    // flush to disk - a no-op for read-only sditerators (all MS-based
    // ones are read-only)
    void flush() {;}

    // re-select
    void reselect();

    // make a deep copy
    void deepCopy(const String &newName) 
    { ms_p->deepCopy(newName, Table::NewNoReplace);}

    // return the current SDRecord
    const SDRecord &get() { copyAll(); return *rec_p;}

    // return just the data portion of the current SDRecord
    const Record &getData() { copyData(); return rec_p->data();}

    // return just the header portion of the current SDRecord
    const Record &getHeader() {copyHeader(); return rec_p->header();}

    // return just the other portion of the current SDRecord
    const Record &getOther() {copyOther(); return rec_p->other();}

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
    Bool isWritable() const { return ms_p->isWritable();}

    // get a record with structure like the current working SDRecord which
    // contains only fields which are scalar string fields - the value of those
    // fields is just a boolean.  This is necessary so that the proper
    // parsing of the selection when given as a string can be done for
    // each field without having to duplicate that code in both glish
    // and here in C++.
    const Record &stringFields() const { return stringFields_p;}
	   
    // return the name of iterator, the same as the table name
    String name() const { return ms_p->tableName(); }

    // other things may not work if ok == False, this needs to be cleaned up
    Bool ok() const { return ok_p;}

    // Toggle use of corrected data if available.
    virtual Bool useCorrectedData(Bool correctedData);

    // query whether corrected data is being used.
    virtual Bool correctedData() {return correctedData_p;}
private:
    Bool ok_p;
    SDRecord *rec_p;
    MeasurementSet *ms_p;
    uInt thisRow_p;
    Record selection_p;

    Bool noData_p, floatData_p, correctedData_p, weightSpectrum_p, sigmaSpectrum_p;

    ROMSColumns *romsCols_p;
    MSValidIds ids_p;

    RecordFieldPtr<String> sourceName_p,  refframe_p, telescope_p, veldef_p, transition_p,
	observer_p, project_p;
    RecordFieldPtr<Int> scan_number_p;
    RecordFieldPtr<Array<Float> > arr_p, weight_p, sigma_p, tcal_p, trx_p, tsys_p;
    RecordFieldPtr<Array<Complex> > carr_p;
    RecordFieldPtr<Array<Bool> > flag_p;
    RecordFieldPtr<Array<String> > corrType_p;

    RecordFieldPtr<Double> duration_p, exposure_p, refFreq_p, chanWidth_p,
	resolution_p, bandwidth_p, pressure_p, dewpoint_p, tambient_p,
	winddir_p, windspeed_p, restFreq_p;
    RecordFieldPtr<Record> azel_p, direction_p, refDirection_p, time_p, 
	telescopePosition_p, chanFreq_p;

    Record stringFields_p;

    MSDopplerIndex dopplerIndex_p;
    MSFeedIndex feedIndex_p;
    MSFreqOffIndex freqOffsetIndex_p;
    MSPointingIndex pointingIndex_p;
    MSSourceIndex sourceIndex_p;
    MSSysCalIndex syscalIndex_p;
    MSWeatherIndex weatherIndex_p;

    Block<ROTableRow *> stdTabRows_p;
    Int mainRow_p, antRow_p, dataDescRow_p, dopplerRow_p, feedRow_p, fieldRow_p,
	observationRow_p, pointingRow_p, polRow_p, procRow_p, sourceRow_p, 
	spwRow_p, stateRow_p, syscalRow_p, weatherRow_p;
    Block<ROTableRow *> nsTabRows_p;
    Block<String> nsOtherNames_p;
    Block<MSTableIndex *> nsIndexes_p;
    PtrBlock<Table *> nsTables_p;

    Unit sec_p;

    MeasFrame frame_p;
    MDirection::Convert *toAzEl_p;
    MPosition obsPos_p;

    void cleanup();
    void cleanRec();
    void copyAll();
    void copyData();
    void copyHeader();
    void copyHist();
    void copyOther();
    Bool replaceAll(const SDRecord& rec, Bool rowIsNew = False);
    void init();
    void initRec();
    void resetDataType();

    void applySelection(const Record &selection);
    Vector<uInt> parseRow(const Record& selection, Int maxRow);
    void createMS(const String& msName, TableLock::LockOption lockoption,
		  Table::TableOption opt);
    Double getRefPix(const Vector<Double> &chanVals, Double refVal, Double &delta, Int count=0);
    void initStringFields(Record &fieldRec, const Record &modelRec);
    String asStringVec(const Vector<Int> &vec);
    Record getVectorShortCuts(const Record &recTemplate);
    String dataUnits(const MSMainEnums::PredefinedColumns whichCol);

};


} //# NAMESPACE CASA - END

#endif

