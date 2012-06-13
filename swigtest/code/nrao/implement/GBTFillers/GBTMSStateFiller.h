//# GBTMSStateFiller.h: GBTMSStateFiller fills the MSState table for GBT fillers
//# Copyright (C) 2002,2003
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

#ifndef NRAO_GBTMSSTATEFILLER_H
#define NRAO_GBTMSSTATEFILLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/RecordField.h>
#include <ms/MeasurementSets/MSState.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/String.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class ColumnsIndex;
class MSStateColumns;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSStateFiller fills the MSState table for GBT fillers
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
// This class puts GBT data into (fills) the MSState table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSState
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSState are made.  
// Existing rows are re-used whenever possible.
//
// The stateIds are available to be used by the
// other GBTMS*Fillers as other subtables are filled in turn.
//
// It is assumed that the non-standard columns NRAO_GBT_PROCNAME,
// NRAO_GBT_PROCSIZE, and NRAO_GBT_PROCSEQN already exist in the 
// MSState table when this class attempts to attach to the MSState
// table.
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

class GBTMSStateFiller
{
public:

    // The default ctor.  No MSState table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSState has been created.
    GBTMSStateFiller();

    // A filler attached to an MSState table
    GBTMSStateFiller(MSState &msState);

    ~GBTMSStateFiller();

    // attach it to an MSState
    void attach(MSState &msState);

    // fill for all of the states given.  sigref and cal must have
    // the same length.  New rows are only added when needed (existing
    // rows with the same values are re-used). The scalar values are
    // constant for all the input states.  Some interpretation is
    // done here for the special 2-state case.  In that case, if
    // cal is changing then sigref is assumed to not change.  That
    // is to accomodate the TPWCALSP mode where the sigref column
    // is used trigger lo blanking while doppler tracking.  Due to
    // different user interfaces, it is sometimes possible to
    // get an unexpected TPWCALSP state table even in other modes.
    // This assumption, that sigref=True when cal changes and
    // there are only 2 states, should be valid in all cases.
    void fill(const Vector<Bool> &sigref, const Vector<Bool> &cal,
	      const String &procname, Int procsize, Int procseqn,
	      const String &swstate, const String &swtchsig);

    // The vector state ids most recently filled.
    const Vector<Int> &stateIds() const {return stateIds_p;}

    // Return a vector of the SIG column for the states just filled
    const Vector<Bool> &sigs() const {return sigs_p;}

    // flush the underlying MS subtable
    void flush() {msState_p->flush();}

private:
    ColumnsIndex *index_p;
    RecordFieldPtr<Bool> sigKey_p;
    RecordFieldPtr<Double> calKey_p;
    RecordFieldPtr<Int> subScanKey_p, procSizeKey_p;
    RecordFieldPtr<String> obsModeKey_p;
    MSState *msState_p;
    MSStateColumns *msStateCols_p;
    ScalarColumn<String> procnameCol_p;
    ScalarColumn<Int> procsizeCol_p, procseqnCol_p;

    Vector<Int> stateIds_p;
    Vector<Bool> sigs_p;

    // initialize the above for the first time
    void init(MSState &msState);

    // undefined and unavailable
    GBTMSStateFiller(const GBTMSStateFiller &other);
    void operator=(const GBTMSStateFiller &other);
};

#endif


