//# GBTMSAntennaFiller.h: GBTMSAntennaFiller fills the MSAntenna table for GBT fillers
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

#ifndef NRAO_GBTMSANTENNAFILLER_H
#define NRAO_GBTMSANTENNAFILLER_H

#include <casa/aips.h>
#include <measures/Measures/MPosition.h>
#include <ms/MeasurementSets/MSAntenna.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSAntennaColumns;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

class GBTAntennaFile;

// <summary>
// GBTMSAntennaFiller fills the MSAntenna table for GBT fillers
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
// This class puts GBT data (fills) into the MSAntenna table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSAntenna
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSAntenna are made.  
// For GBT data at there will usually only be one row in this table.   This
// row is added using information in the Antenna file when available or
// the NRAO_GBT position from the Measures observatory table.  A new row
// is added when the telescope information in the Antenna file changes.
//
// The antennaId is available to be used by 
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

class GBTMSAntennaFiller
{
public:

    // The default ctor.  No MSAntenna table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSAntenna has been created.
    GBTMSAntennaFiller();

    // A filler attached to an MSAntenna table
    GBTMSAntennaFiller(MSAntenna &msAntenna);

    ~GBTMSAntennaFiller();

    // attach it to an MSAntenna
    void attach(MSAntenna &msAntenna);

    // fill using the supplied Antenna file if attached or the
    // "GBT" entry in the Measures observatory table.
    // rows are reused if the name and position match
    void fill(const GBTAntennaFile &antennaFile);

    // this is the row number of the most recently "filled" MSAntenna row
    Int antennaId() const {return antId_p;}

    // flush the underlying MS subtable
    void flush() {msAntenna_p->flush();}

private:
    MSAntenna *msAntenna_p;
    MSAntennaColumns *msAntCols_p;

    MPosition position_p;
    String name_p;

    Int antId_p;

    // initialize the above for the first time
    void init(MSAntenna &msAntenna);

    // undefined and unavailable
    GBTMSAntennaFiller(const GBTMSAntennaFiller &other);
    void operator=(const GBTMSAntennaFiller &other);
};

#endif


