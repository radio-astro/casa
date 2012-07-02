//# GBTMSHistoryFiller.h: GBTMSHistoryFiller fills the MSHistory table for GBT fillers
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

#ifndef NRAO_GBTMSHISTORYFILLER_H
#define NRAO_GBTMSHISTORYFILLER_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MSHistory.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSHistoryColumns;
class MVTime;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSHistoryFiller fills the MSHistory table for GBT fillers
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
// This class puts GBT data (fills) into the MSHistory table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSHistory
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSHistory are made.  
// Each fill() always adds a new row to the MSHistory table.
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

class GBTMSHistoryFiller
{
public:

    // The default ctor.  No MSHistory table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSHistory has been created.
    GBTMSHistoryFiller();

    // A filler attached to an MSHistory table
    GBTMSHistoryFiller(MSHistory &msHistory);

    ~GBTMSHistoryFiller();

    // attach it to an MSHistory
    void attach(MSHistory &msHistory);

    // fill - always adds a new row to the obsLog table
    void fill(Int observationId, const String &message, 
	      const String &priority, const String &origin,
	      const MVTime &timestamp);

    // flush the underlying MS subtable
    void flush() {msHistory_p->flush();}

private:
    MSHistory *msHistory_p;
    MSHistoryColumns *msHistoryCols_p;

    // initialize the above for the first time
    void init(MSHistory &msHistory);

    // undefined and unavailable
    GBTMSHistoryFiller(const GBTMSHistoryFiller &other);
    void operator=(const GBTMSHistoryFiller &other);
};

#endif


