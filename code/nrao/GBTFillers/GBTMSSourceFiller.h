//# GBTMSSourceFiller.h: GBTMSSourceFiller fills the MSSource table for GBT fillers
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

#ifndef NRAO_GBTMSSOURCEFILLER_H
#define NRAO_GBTMSSOURCEFILLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/RecordField.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/Constants.h>
#include <ms/MeasurementSets/MSSource.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
class MSSource;
class MSSourceColumns;
class Table;
class ColumnsIndex;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSSourceFiller fills the MSSource table for GBT fillers
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
// This class puts GBT data (fills) into the MSSource table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSSource
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSSource are made.
// This filler keeps returns a unique ID for each set of source information.
// Existing information is kept for later indexing.  The spectral window ID
// is used in addition to the source ID to set this information into the
// appropriate row of the MSSource table.
//
// The sourceId is available to be used by 
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

class GBTMSSourceFiller
{
public:

    // The default ctor.  No MSSource table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSSource has been created.
    GBTMSSourceFiller();

    // A filler attached to an MSSource table
    GBTMSSourceFiller(MSSource &msSource);

    ~GBTMSSourceFiller();

    // attach it to an MSSource
    void attach(MSSource &msSource);

    // fill given the indicated source information and spectral window ID
    void fill(const Vector<Int> &spectralWindowIds,
	      const String &name, Double restFrequency = 0.0,
	      Double sysvel = 0.0, const String &code="",
	      Double time = 0.0, Double interval=-1,
	      Int calibrationGroup=-1,
	      const Vector<Double> &direction=Vector<Double>(2,0.0),
	      const Vector<Double> &position=Vector<Double>(3,0.0));

    // What is the most recently filled source ID.  Returns -1 if none have been filled.
    Int sourceId() const {return sourceId_p;}

    // flush the underlying MS subtable
    void flush() {msSource_p->flush();}

private:
    MSSource *msSource_p;
    MSSourceColumns *msSourceCols_p;

    // The current Id
    Int sourceId_p;

    // the table holding the cache, it is temporary
    Table *theCache_p;

    // the columns in the cache
    ScalarColumn<Int> calGroupCol_p;
    ScalarColumn<String> codeCol_p, nameCol_p;
    ScalarColumn<Double> timeCol_p, intervalCol_p, restFreqCol_p, sysvelCol_p;
    // the vector cols are stored in the cache as scalar columns so that they can be indexed
    ScalarColumn<Double> dirCol0_p, dirCol1_p, posCol0_p, posCol1_p, posCol2_p;

    // The ColumnsIndex for the cache
    ColumnsIndex *cacheIndx_p;
    // ColumnsIndex for the MSSource table
    ColumnsIndex *sourceIndx_p;

    // The pointers to the fields in the index key
    RecordFieldPtr<Int> calGroupKey_p;
    RecordFieldPtr<String> codeKey_p, nameKey_p; 
    RecordFieldPtr<Double> timeKey_p, intervalKey_p, restFreqKey_p, sysvelKey_p;
    RecordFieldPtr<Double> dirKey0_p, dirKey1_p, posKey0_p, posKey1_p, posKey2_p;

    RecordFieldPtr<Int> sourceIdKey_p, spWinIdKey_p;

    // initialize the above for the first time
    void init(MSSource &msSource);

    // clean things up
    void cleanup();

    // undefined and unavailable
    GBTMSSourceFiller(const GBTMSSourceFiller &other);
    void operator=(const GBTMSSourceFiller &other);
};

#endif


