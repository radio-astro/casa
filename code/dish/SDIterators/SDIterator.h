//# SDIterator: base class which iterates through a data set - serving SDRecords
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
//# with along this library; if not, write to the Free Software Foundation,
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

#ifndef DISH_SDITERATOR_H
#define DISH_SDITERATOR_H

#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class SDRecord;
class Record;
class RecordDesc;
class SDIterFieldCopier;

template<class T> class Array;
template<class T> class Matrix;
template<class T> class Vector;


// <summary>
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SDRecord
//   <li> SDCalc design
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

class SDIterator
{
public:
    // this just has the common functions, no data
    SDIterator() {;}

    virtual ~SDIterator() {;}

    // what type of sditerator is this ("MeasurementSet, Table, Image")
    virtual String type() const = 0;

    // reset the iterator to start
    virtual void origin() = 0;

    // are there any more SDRecords after the current one
    virtual Bool more() = 0;

    // unlock this sditerator and all of its sub-tables, if any
    virtual Bool unlock() = 0;

    // lock this sditerator and all of its sub-tables.
    // The original lock type is used here.  It will try
    // and acquire the lock nattempt times - that is roughly
    // the time in seconds until it gives up trying to acquire the lock.
    virtual Bool lock(uInt nattempts=0) = 0;

    // step to the next one, if at end, nothing changes
    // <group>
    virtual SDIterator& operator++(int) = 0;
    virtual SDIterator& operator++() = 0;
    // </group>

    // step back, if at end, nothing changes
    // <group>
    virtual SDIterator& operator--(int) = 0;
    virtual SDIterator& operator--() = 0;
    // </group>

    // return the current location
    virtual uInt where() const = 0;

    // how many SDRecords are there
    virtual uInt nrecords() = 0;

    // resync with the disk contents
    virtual void resync() = 0;

    // flush the contents to disk (a no-op for a read-only MS)
    virtual void flush() = 0;

    // re-select
    virtual void reselect() = 0;

    // make a deep copy of the main table and any subtables.
    // An exception is thrown if the newName can not be created or
    // a file having that name already exists.
    virtual void deepCopy(const String &newName) = 0;

    // return the current SDRecord
    virtual const SDRecord &get() = 0;

    // return just the data portion of the current SDRecord
    virtual const Record &getData() = 0;

    // return just the header portion of the current SDRecord
    virtual const Record &getHeader() = 0;

    // return just the other portion of the current SDRecord
    virtual const Record &getOther() = 0;

    // return just the hist portion of the current SDRecord
    virtual const Array<String> getHist() = 0;

    // replace the current SDRecord with a new one
    virtual Bool put(const SDRecord& rec) = 0;

    // add a new record at the end of the iterator
    virtual Bool appendRec(const SDRecord &rec) = 0;

    // delete the record at the current location
    virtual Bool deleteRec() = 0;

    // see if this iterator is writable
    virtual Bool isWritable() const = 0;

    // get all values from the SDIterator for the fields 
    // specified in the template as vectors or arrays
    virtual Record get_vectors(const Record& recTemplate);

    // get a record with structure like the current working SDRecord which
    // contains only fields which are scalar string fields - the value of those
    // fields is just a boolean.  This is necessary so that the proper
    // parsing of the selection when given as a string can be done for
    // each field without having to duplicate that code in both glish
    // and here in C++.
    virtual const Record &stringFields() const = 0;
	   
    // return the name of iterator
    virtual String name() const = 0;

    // Is this iterator ok
    virtual Bool ok() const = 0;

    // Toggle use of corrected data if available.  Default is a no-op,
    // that returns False.
    virtual Bool useCorrectedData(Bool correctedData)
    { return False;}

    // query whether corrected data is being used.  Default is False.
    virtual Bool correctedData() {return False;}
protected:
    // current supported short-cuts are: time, scan_number, source_name, telescope_position,
    // rest_frequency, date, ut - default just uses getVectors, may be two times through
    virtual Record getVectorShortCuts(const Record &recTemplate);
    // get them by brute force
    Record getVectors(const Record &recTemplate);
    void makeDescFromTemplate(const Record& recTemplate, const Record& sdrec,
			      RecordDesc& target, uInt nrows);
    RecordDesc cloneDesc(const Record& rec);
    void addFieldsToCopier(SDIterFieldCopier& copier, const Record& sdrec,
			   Record& result, const String &parentName);
    String parseRanges(const Matrix<Double> &ranges, const String &name,
		       Bool colIsArray = False);
    String parseList(const Vector<String> &list, const String &name);
    String doubleToString(Double dbl);
    void makeVariableSubRecs(Record &rec);
private:
    uInt countFields(const RecordDesc &rec);
};


} //# NAMESPACE CASA - END

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <dish/SDIterators/SDIterator.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif


