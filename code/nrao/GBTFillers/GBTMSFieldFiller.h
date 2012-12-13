//# GBTMSFieldFiller.h: fills the FIELD table for GBT fillers
//# Copyright (C) 2000,2001
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

#ifndef NRAO_GBTMSFIELDFILLER_H
#define NRAO_GBTMSFIELDFILLER_H

#include <casa/aips.h>
#include <casa/Containers/RecordField.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MSField.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSFieldColumns;
class MDirection;
class MeasFrame;
class ColumnsIndex;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSFieldFiller fills the MSField table for GBT fillers
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSets
//   <li> GBT FITS files
// </prerequisite>
//
// <etymology>
// This class puts GBT data (fills) into the MSField table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSField
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSField are made.
// This is fully indexed.  New rows are only added for each new NAME,
// CODE, NUM_POLY, SOURCE_ID combination or when the directions change 
// for an existing such combination.
// 
// The fieldId is available to be used by 
// other GBTMS*Fillers as other subtables are filled in turn.
//
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Writing a filler is a daunting task.  There are various dependencies
// between the subtables.  It helps to encapsulate the knowlege and
// task of filling a specific subtable to separate classes.
// </motivation>
//
//#! <thrown>
//#! A list of exceptions thrown if errors are discovered in the function.
//#! This tag will appear in the body of the header file, preceding the
//#! declaration of each function which throws an exception.
//#! </thrown>

class GBTMSFieldFiller
{
public:

    // The default ctor.  No MSField table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSField has been created.
    GBTMSFieldFiller();

    // A filler attached to an MSField table
    GBTMSFieldFiller(MSField &msField);

    ~GBTMSFieldFiller();

    // attach it to an MSField
    void attach(MSField &msField);

    // fill, adding rows as necessary, fieldDir is the nominal pointing direction
    // at equinox.  DELAY_DIR, PHASE_DIR, and REFERENCE_DIR are
    // set to the fieldDir value.  These are always in J2000 in the MS.
    void fill(Int sourceId, const String &name, const MDirection &fieldDir);

    // this is the row numbers of the most recently filled MSField row
    // it returns -1 if nothing has been filled yet
    const Int &fieldId() const {return fieldId_p;}

    // flush the underlying MS subtable
    void flush() {msField_p->flush();}

private:
    ColumnsIndex *index_p;
    RecordFieldPtr<String> nameKey_p;
    RecordFieldPtr<Int> sourceIdKey_p;
    RecordFieldPtr<Double> timeKey_p;
    MSField *msField_p;
    MSFieldColumns *msFieldCols_p;

    Int fieldId_p;

    // initialize the above for the first time
    void init(MSField &msField);

    // undefined and unavailable
    GBTMSFieldFiller(const GBTMSFieldFiller &other);
    void operator=(const GBTMSFieldFiller &other);
};

#endif


