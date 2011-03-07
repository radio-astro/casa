//# GBTMSObservationFiller.h: GBTMSObservationFiller fills the MSObservation table for GBT fillers
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

#ifndef NRAO_GBTMSOBSERVATIONFILLER_H
#define NRAO_GBTMSOBSERVATIONFILLER_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MSObservation.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSObservationColumns;
class MVTime;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSObservationFiller fills the MSObservation table for GBT fillers
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
// This class puts GBT data (fills) into the MSObservation table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSObservation
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSObservation are made.  
// Each fill() adds a new row to the MSObservation table.
// 
// The observationId is available to be used by 
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

class GBTMSObservationFiller
{
public:

    // The default ctor.  No MSObservation table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSObservation has been created.
    GBTMSObservationFiller();

    // A filler attached to an MSObservation table
    GBTMSObservationFiller(MSObservation &msObservation);

    ~GBTMSObservationFiller();

    // attach it to an MSObservation
    void attach(MSObservation &msObservation);

    // fill - adds a new row to the observation table whenever project,
    // observer or telescope, changes.
    // other content to add will be added here as it becomes available
    // e.g. OBS_SCHEDULE
    // The timestamp and duration are used to adjust that TIME_RANGE so that
    // the current project row extends at least from timestamp to 
    // timestamp+duration.
    void fill(const String &project, const String &observer,
	      const MVTime &timestamp, Double duration, 
	      const String &telescope);

    // this is the row number of the most recently "filled" MSObservation row
    // it returns -1 if nothing has been filled yet
    Int observationId() const {return observationId_p;}

    // its easier to just cache the most recent project here
    const String &project() const {return project_p;}

    // flush the underlying MS subtable
    void flush() {msObservation_p->flush();}

private:
    MSObservation *msObservation_p;
    MSObservationColumns *msObsCols_p;

    Int observationId_p;
    String project_p;
    String observer_p;
    String telescope_p;

    // initialize the above for the first time
    void init(MSObservation &msObservation);

    // undefined and unavailable
    GBTMSObservationFiller(const GBTMSObservationFiller &other);
    void operator=(const GBTMSObservationFiller &other);
};

#endif


