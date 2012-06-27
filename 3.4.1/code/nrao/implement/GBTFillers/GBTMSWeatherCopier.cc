//# GBTMSWeatherCopier.cc: copies Weather DAP fields to a MSWeather table
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
//# $Id$

//# Includes

#include <nrao/GBTFillers/GBTMSWeatherCopier.h>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>
#include <fits/FITS/CopyRecord.h>
#include <casa/BasicSL/Constants.h>
#include <ms/MeasurementSets/MSWeather.h>
#include <ms/MeasurementSets/MSWeatherColumns.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableDesc.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>


GBTMSWeatherCopier::GBTMSWeatherCopier(MSWeather &msWeather, const Record &weatherRecord, 
				       const TableDesc &td, Int id)
    : msWeather_p(0), msWeatherCols_p(0), id_p(id), hasWindVel_p(False), hasWindDir_p(False),
      hasAmbTemp_p(False), hasPressure_p(False), hasDewPoint_p(False), fieldCopier_p(0)
{
    msWeather_p = new MSWeather(msWeather);
    AlwaysAssert(msWeather_p, AipsError);

    msWeatherCols_p = new MSWeatherColumns(*msWeather_p);
    AlwaysAssert(msWeatherCols_p, AipsError);

    reinit(weatherRecord, td, id);
}

void GBTMSWeatherCopier::reinit(const Record &weatherRecord, const TableDesc &td, Int id) 
{
    detachFields();

    id_p = id;

    TableDesc localtd(td);
    // set up the fields to be copied
    Vector<String> colNames = msWeather_p->tableDesc().columnNames();
    Vector<Int> copyMap(weatherRecord.nfields());
    for (uInt i=0;i<copyMap.nelements();i++) {
	String thisField = weatherRecord.name(i);
	copyMap(i) = -1;
	if (thisField == "WINDVEL" && weatherRecord.type(i) == TpFloat) {
	    windVelField_p.attachToRecord(weatherRecord, i);
	    hasWindVel_p = True;
	} else if (thisField == "WINDDIR" && weatherRecord.type(i) == TpFloat) {
	    windDirField_p.attachToRecord(weatherRecord, i);
	    hasWindDir_p = True;
	} else if (thisField == "AMB_TEMP" && weatherRecord.type(i) == TpFloat) {
	    ambTempField_p.attachToRecord(weatherRecord, i);
	    hasAmbTemp_p = True;
	} else if (thisField == "PRESSURE" && weatherRecord.type(i) == TpFloat) {
	    pressureField_p.attachToRecord(weatherRecord, i);
	    hasPressure_p = True;
	} else if ((thisField == "DEW_POINT" || thisField == "DEWP") && 
		   weatherRecord.type(i) == TpFloat) {
	    dewPointField_p.attachToRecord(weatherRecord, i);
	    hasDewPoint_p = True;
	} else {
	    // the non-standard MS GBT columns should be GBT_field_name
	    // ignore DMJD
	    if (thisField != "DMJD") {
		String thisFieldAsColumn = "GBT_" + thisField;
		localtd.renameColumn(thisFieldAsColumn, thisField);
		ColumnDesc thisColDesc = localtd[thisFieldAsColumn];
		if (!msWeather_p->tableDesc().isColumn(thisFieldAsColumn)) {
		    msWeather_p->addColumn(thisColDesc);
		}
		if (thisColDesc.trueDataType() == msWeather_p->tableDesc().columnDesc(thisFieldAsColumn).trueDataType()) {
		    // add this one to the fields to be copied
		    // need to find its column number in colNames
		    for (uInt j=0;j<colNames.nelements();j++) {
			if (colNames(j) == thisFieldAsColumn) {
			    copyMap(i) = j;
			    break;
			}
		    }
		}
	    }
	}
    }
    // add any standard columns as necessary
    Block<Int> newCols(10);
    Int newColsCount = 0;
    if (hasWindDir_p && msWeatherCols_p->windDirection().isNull()) {
	newCols[newColsCount++] = Int(MSWeather::WIND_DIRECTION);
	newCols[newColsCount++] = Int(MSWeather::WIND_DIRECTION_FLAG);
    }
    if (hasWindVel_p && msWeatherCols_p->windSpeed().isNull()) {
	newCols[newColsCount++] = Int(MSWeather::WIND_SPEED);
	newCols[newColsCount++] = Int(MSWeather::WIND_SPEED_FLAG);
    }
    if (hasAmbTemp_p && msWeatherCols_p->temperature().isNull()) {
	newCols[newColsCount++] = Int(MSWeather::TEMPERATURE);
	newCols[newColsCount++] = Int(MSWeather::TEMPERATURE_FLAG);
    }
    if (hasPressure_p && msWeatherCols_p->pressure().isNull()) {
	newCols[newColsCount++] = Int(MSWeather::PRESSURE);
	newCols[newColsCount++] = Int(MSWeather::PRESSURE_FLAG);
    }
    if (hasDewPoint_p && msWeatherCols_p->dewPoint().isNull()) {
	newCols[newColsCount++] = Int(MSWeather::DEW_POINT);
	newCols[newColsCount++] = Int(MSWeather::DEW_POINT_FLAG);
    }
    if (newColsCount > 0) {
	delete msWeatherCols_p;
	msWeatherCols_p = 0;
	TableDesc newtd;
	// one at a time
	for (Int i=0;i<newColsCount;i++) {
	    MSWeather::addColumnToDesc(newtd, MSWeather::PredefinedColumns(newCols[i]));
	}
	for (uInt j=0;j<newtd.ncolumn();j++) {
	    msWeather_p->addColumn(newtd[j]);
	}
	msWeatherCols_p = new MSWeatherColumns(*msWeather_p);
	AlwaysAssert(msWeatherCols_p, AipsError);
    }
    // ensure that the SAMPLER and MANAGER columns exist
    if (!msWeather_p->tableDesc().isColumn("SAMPLER")) {
	msWeather_p->addColumn(ScalarColumnDesc<String>("SAMPLER"));
    }
    if (samplerCol_p.isNull()) {
	samplerCol_p.attach(*msWeather_p, "SAMPLER");
    }
    if (!msWeather_p->tableDesc().isColumn("MANAGER")) {
	msWeather_p->addColumn(ScalarColumnDesc<String>("MANAGER"));
    }
    if (managerCol_p.isNull()) {
	managerCol_p.attach(*msWeather_p, "MANAGER");
    }

    fieldCopier_p = new CopyRecordToTable(*msWeather_p, weatherRecord, copyMap);
    AlwaysAssert(fieldCopier_p, AipsError);
}

GBTMSWeatherCopier::~GBTMSWeatherCopier() {
    cleanup();
}

void GBTMSWeatherCopier::cleanup() {
    delete msWeather_p;
    msWeather_p = 0;

    delete msWeatherCols_p;
    msWeatherCols_p = 0;

    detachFields();
}

void GBTMSWeatherCopier::detachFields() {
    windVelField_p.detach();
    windDirField_p.detach();
    ambTempField_p.detach();
    pressureField_p.detach();
    dewPointField_p.detach();

    hasWindVel_p = hasWindDir_p = hasAmbTemp_p = hasPressure_p = hasDewPoint_p = False;

    delete fieldCopier_p;
    fieldCopier_p = 0;
}

void GBTMSWeatherCopier::copy(uInt toRow, const String &sampler, const String &manager,
			      const MVTime &time, Double interval, Int antennaId) {
    samplerCol_p.putScalar(toRow, sampler);
    managerCol_p.putScalar(toRow, manager);
    msWeatherCols_p->time().put(toRow, time.second());
    msWeatherCols_p->interval().put(toRow, interval);
    msWeatherCols_p->antennaId().put(toRow, antennaId);

    if (!msWeatherCols_p->windSpeed().isNull()) {
	if (hasWindVel_p) {
	    msWeatherCols_p->windSpeed().put(toRow, *windVelField_p);
	} 
	msWeatherCols_p->windSpeedFlag().put(toRow, !hasWindVel_p);
    }
    if (!msWeatherCols_p->windDirection().isNull()) {
	if (hasWindDir_p) {
	    msWeatherCols_p->windDirection().put(toRow, *windDirField_p);
	}
	msWeatherCols_p->windDirectionFlag().put(toRow, !hasWindDir_p);
    }
    if (!msWeatherCols_p->temperature().isNull()) {
	if (hasAmbTemp_p) {
	    // celcius -> kelvin
	    msWeatherCols_p->temperature().put(toRow, *ambTempField_p + 273.15);
	}
	msWeatherCols_p->temperatureFlag().put(toRow, !hasAmbTemp_p);
    }
    if (!msWeatherCols_p->pressure().isNull()) {
	if (hasPressure_p) {
	    // mbar -> Pa
	    Quantity pressure(*pressureField_p, "bar");
	    msWeatherCols_p->pressure().put(toRow, pressure.getValue("Pa"));
	}
	msWeatherCols_p->pressureFlag().put(toRow, !hasAmbTemp_p);
    }
    if (!msWeatherCols_p->dewPoint().isNull()) {
	if (hasDewPoint_p) {
	    // celcius -> kelvin
	    msWeatherCols_p->dewPoint().put(toRow, *dewPointField_p + 273.15);
	}
	msWeatherCols_p->dewPointFlag().put(toRow, !hasAmbTemp_p);
    }
    // should we attempt to derive the relative humidity from the dew point?

    // and everything else
    fieldCopier_p->copy(toRow);
}
