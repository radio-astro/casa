//# GBTMSWeatherCopier.h: GBTMSWeatherCopier copies Weather DAP fields to a MSWeather table
//# Copyright (C) 2000
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

#ifndef NRAO_GBTMSWEATHERCOPIER_H
#define NRAO_GBTMSWEATHERCOPIER_H

#include <casa/aips.h>
#include <casa/Containers/RecordField.h>
#include <tables/Tables/TableColumn.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class CopyRecordToTable;
class Record;
class String;
class MVTime;
class MSWeather;
class MSWeatherColumns;
class TableDesc;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSWeatherCopier copies Weather DAP fields toa  MSWeather table
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSets
//   <li> GBT FITS files
//   <li> GBTMSWeatherFiller
// </prerequisite>
//
// <etymology>
// This class copies GBT Weather DAP into the MSWeather table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSWeather
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSWeather are made.
// Because it works with the Weather FITS files in the same way that
// the generic DAP copiers must work with multiple samplers and
// managers, this class was derived from GBTDAPCopierBase.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Writing a copier is a daunting task.  There are various dependencies
// between the subtables.  It helps to encapsulate the knowlege and
// task of filling a specific subtable to separate classes.
// </motivation>
//
//#! <thrown>
//#! A list of exceptions thrown if errors are discovered in the function.
//#! This tag will appear in the body of the header file, preceding the
//#! declaration of each function which throws an exception.
//#! </thrown>

class GBTMSWeatherCopier
{
public:
    // construct the copier given the MSWeather table and the record which
    // will contain the DAP fields to be copied.  Supply the DAP TableDesc and ID for this copier.
    GBTMSWeatherCopier(MSWeather &msWeather, const Record &weatherRecord, 
		       const TableDesc &td, Int id);

    ~GBTMSWeatherCopier();

    // copy the current contents to indicated row of the table, which must have already
    // been added to the weather table
    void copy(uInt toRow, const String &sampler, const String &manager,
	      const MVTime &time, Double interval, Int antennaId);

    // re-initialize things to use the indicate Record, TableDesc and Id
    void reinit(const Record &weatherRecord, const TableDesc &td, Int id);

private:
    MSWeather *msWeather_p;
    MSWeatherColumns *msWeatherCols_p;

    // sampler and manager
    TableColumn samplerCol_p, managerCol_p;

    Int id_p;

    // standard fields with standard MSColumns to receive them
    RORecordFieldPtr<Float> windVelField_p, windDirField_p, ambTempField_p, pressureField_p,
	dewPointField_p;

    // flags indicating which fields are actually present in this copier
    Bool hasWindVel_p, hasWindDir_p, hasAmbTemp_p, hasPressure_p, hasDewPoint_p;

    // the copier for everything else
    CopyRecordToTable *fieldCopier_p;

    void cleanup();

    void detachFields();

   // undefined and unavailable
    GBTMSWeatherCopier(const GBTMSWeatherCopier &other);
    void operator=(const GBTMSWeatherCopier &other);
};

#endif


