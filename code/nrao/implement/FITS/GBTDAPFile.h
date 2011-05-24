//# GBTDAPFile.h: GBTDAPFile is a FITSTable holding GBT DAP info
//# Copyright (C) 1999,2000
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

#ifndef NRAO_GBTDAPFILE_H
#define NRAO_GBTDAPFILE_H

//#! Includes go here

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/MVTime.h>
#include <casa/BasicSL/String.h>
#include <fits/FITS/FITSTable.h>
#include <casa/Containers/RecordField.h>

#include <casa/namespace.h>
//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class Record;
class CopyRecordToRecord;
} //# NAMESPACE CASA - END


// <summary>
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
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
//
// <thrown>
//#! A list of exceptions thrown if errors are discovered in the function.
//#! This tag will appear in the body of the header file, preceding the
//#! declaration of each function which throws an exception.
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

class GBTDAPFile : public FITSTable
{
public:

    GBTDAPFile(const String &dapfile, const MVTime &startTime);
    ~GBTDAPFile() {cleanup();}

    virtual Bool reopen(const String &dapfile, const MVTime &startTime, Bool resync);
    // the default reopen is a reopen with resync=True
    virtual Bool reopen(const String &dapfile) {return reopen(dapfile, Double(0.0), True);}
    virtual Bool isValid() const {return dmjd_p.isAttached();}
    virtual void next();
    virtual const Record &currentRow() const {return *currentRow_p;}
    virtual Bool pastEnd() const 
	{ return (!isValid() || FITSTable::pastEnd() && currentTime_p == endOfTime_p);}
    virtual Int rownr() const {return row_nr_p;}
    virtual void move(Int torow);

    virtual const MVTime &currentTime() const {return currentTime_p;}
    virtual const MVTime &nextTime() const {return nextTime_p;}

    // the interval between nextTime() and currentTime()
    // if the currentTime is the end of the row, the interval is the last row
    // if there is only 1 row, the interval is zero
    virtual Double interval() const {return interval_p;}

    const String& manager() const { return manager_p;}
    const String& sampler() const { return sampler_p;}

    virtual void handleField(const String &fieldName);

    virtual const Record &unhandledFields()
    {
	if (!unhandledFields_p) initUnhandledFields();
	return *unhandledFields_p;
    }

private:
    Bool timeWarningIssued_p;

    String manager_p, sampler_p;

    Record *currentRow_p;

    MVTime currentTime_p, nextTime_p, endOfTime_p;

    Double interval_p;

    RORecordFieldPtr<Double> dmjd_p;

    Int row_nr_p;

    Record *unhandledFields_p;
    CopyRecordToRecord *fieldCopier_p;
    Vector<Int> fieldMap_p;

    void cleanup();
    void init();

    void initUnhandledFields();

    // unavailable
    GBTDAPFile();
    GBTDAPFile(const GBTDAPFile &other);
    GBTDAPFile &operator=(const GBTDAPFile &other);
};

#endif


