//# GBTMSSysCalFiller.h: GBTMSSysCalFiller fills the MSSysCal table for GBT fillers
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

#ifndef NRAO_GBTMSSYSCALFILLER_H
#define NRAO_GBTMSSYSCALFILLER_H

#include <casa/aips.h>
#include <casa/Containers/RecordField.h>
#include <ms/MeasurementSets/MSSysCal.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSSysCalColumns;
class ColumnsIndex;
template <class T> class Matrix;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSSysCalFiller fills the MSSysCal table for GBT fillers
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
// This class puts GBT data (fills) into the MSSysCal table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSSysCal
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSSysCal are made.
// Each call to fill generates a new row.
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

class GBTMSSysCalFiller
{
public:

    // The default ctor.  No MSSysCal table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSSysCal has been created.
    GBTMSSysCalFiller();

    // A filler attached to an MSSysCal table
    GBTMSSysCalFiller(MSSysCal &msSysCal);

    ~GBTMSSysCalFiller();
    
    // attach it to an MSSysCal
    void attach(MSSysCal &msSysCal);

    // Pre-fill the TCAL and TRX values.  This associates
    // those input values with antennaId, feedId and spectralWindowId.
    // and a date string from the receiver CAL table.  Subsequent
    // calls to fill use these pre-filled values.  Since these
    // values don't change very often with time (generally never during
    // an observing run, for example) the calls to fill simply
    // involve a time and interval matching that in the main table
    // so that the TSYS cell can be prepared for use by the calibration
    // routines (which requires one cell for each cell in the main table).
    // If this combination of keys has already been used, these values
    // of tcal and trx will replace the ones already stored.
    void prefill(const Matrix<Float> &tcal, const Matrix<Float> &trx,
		 Int antennaId, Int feedId, Int spectralWindowId);

    // Fill the SYSCAL table.  A new row is added to the SYSCAL table.
    void fill(Double time, Double interval, Int antennaId, Int feedId,
	      Int spectralWindowId);

    // flush the underlying MS subtable
    void flush() {msSysCal_p->flush();}

private:
    // the actual syscal table and columns
    MSSysCal *msSysCal_p;
    MSSysCalColumns *msSysCalCols_p;

    // the dummy ones used to hold the cached TCAL and TRX values
    MSSysCal *dummySysCal_p;
    MSSysCalColumns *dummyCols_p;

    // Table index into the dummy SYSCAL table
    ColumnsIndex *dummyIndex_p;
    // And the index keys
    RecordFieldPtr<Int> antIdKey_p, feedIdKey_p, spwIdKey_p;

    // row number matching these keys
    Int lastIndex_p;

    // initialize the above for the first time
    void init(MSSysCal &msSysCal);

    // undefined and unavailable
    GBTMSSysCalFiller(const GBTMSSysCalFiller &other);
    void operator=(const GBTMSSysCalFiller &other);
};

#endif


