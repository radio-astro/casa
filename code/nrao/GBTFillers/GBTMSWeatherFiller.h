//# GBTMSWeatherFiller.h: GBTMSWeatherFiller fills the MSWeather table for GBT fillers
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

#ifndef NRAO_GBTMSWEATHERFILLER_H
#define NRAO_GBTMSWEATHERFILLER_H

#include <casa/aips.h>
#include <casa/Containers/Block.h>
#include <nrao/GBTFillers/GBTDAPFillerBase.h>
#include <tables/Tables/TableColumn.h>
#include <ms/MeasurementSets/MSWeather.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSWeather;
class MVTime;
class String;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

class GBTMSWeatherCopier;

// <summary>
// GBTMSWeatherFiller fills the MSWeather table for GBT fillers
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
// This class puts GBT data (fills) into the MSWeather table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSWeather
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSWeather are made.
// Because it works with the Weather FITS files in the same way that
// the generic DAP fillers must work with multiple samplers and
// managers, this class was derived from GBTDAPFillerBase.
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

class GBTMSWeatherFiller : public GBTDAPFillerBase
{
public:

    // The default ctor.  No MSWeather table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSWeather has been created.
    GBTMSWeatherFiller();

    // A filler attached to an MSWeather table
    GBTMSWeatherFiller(MSWeather &msWeather);

    ~GBTMSWeatherFiller();

    const String &device() const {return device_p;}

    // attach it to an MSWeather
    void attach(MSWeather &msWeather);

    // prepare for filling the indicated weather file
    Int prepare(const String &fileName, const String &manager,
		const String &sampler, const MVTime &startTime);

    // set the antennaId - defaults to -1 if not explicitly set
    void setAntennaId(Int antennaId) {antennaId_p = antennaId;}

    // fill what has been prepared
    void fill();

    // flush the underlying MS subtable
    void flush() {msWeather_p->flush();}

private:
    String device_p;

    MSWeather *msWeather_p;

    // sampler and manager
    TableColumn samplerCol_p, managerCol_p;

    // copiers - one for each 
    PtrBlock<GBTMSWeatherCopier *> copiers_p;
    // and how many of them are really in use at any given point
    Int ndap_p;

    Int antennaId_p;

    // initialize the above for the first time
    void init(MSWeather &msWeather);

    // undefined and unavailable
    GBTMSWeatherFiller(const GBTMSWeatherFiller &other);
    void operator=(const GBTMSWeatherFiller &other);
};

#endif


