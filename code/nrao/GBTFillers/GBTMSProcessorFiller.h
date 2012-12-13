//# GBTMSProcessorFiller.h: GBTMSProcessorFiller fills the MSProcessor table for GBT fillers
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

#ifndef NRAO_GBTMSPROCESSORFILLER_H
#define NRAO_GBTMSPROCESSORFILLER_H

#include <casa/aips.h>
#include <casa/Containers/RecordField.h>
#include <ms/MeasurementSets/MSProcessor.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class ColumnsIndex;
class MSProcessorColumns;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSProcessorFiller fills the MSProcessor table for GBT fillers
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
// This class puts GBT data (fills) into the MSProcessor table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSProcessor
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSProcessor are made.  
//
// The processorId is available to be used by 
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

class GBTMSProcessorFiller
{
public:

    // The default ctor.  No MSProcessor table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSProcessor has been created.
    GBTMSProcessorFiller();

    // A filler attached to an MSProcessor table
    GBTMSProcessorFiller(MSProcessor &msProcessor);

    ~GBTMSProcessorFiller();

    // attach it to an MSProcessor
    void attach(MSProcessor &msProcessor);

    // fill - sub_type is always GBT, mode_id is not yet used
    void fill(const String &type, Int typeId);

    // this is the row number of the most recently "filled" MSProcessor row
    Int processorId() {return procId_p;}

    // flush the underlying MS subtable
    void flush() {msProcessor_p->flush();}
private:
    MSProcessor *msProcessor_p;
    MSProcessorColumns *msProcCols_p;

    ColumnsIndex *index_p;

    RecordFieldPtr<String> typeKey_p;
    RecordFieldPtr<Int> typeIdKey_p;

    Int procId_p;

    // initialize the above for the first time
    void init(MSProcessor &msProcessor);

    // undefined and unavailable
    GBTMSProcessorFiller(const GBTMSProcessorFiller &other);
    void operator=(const GBTMSProcessorFiller &other);
};

#endif


