//# PlotMSExportParams.cc: Iteration paremters container implementation
//# Copyright (C) 2009
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
//# $Id: $
#include <plotms/PlotMS/PlotMSExportParam.h>
#include <QDebug>

namespace casa {

///////////////////////////////////////
// PLOTMSEXPORTPARM DEFINITIONS //
///////////////////////////////////////
const String PlotMSExportParam::EXPORT_RANGE="Export Range";

PlotMSExportParam::PlotMSExportParam()
	{ setDefaults(); }
PlotMSExportParam::~PlotMSExportParam() { }


void PlotMSExportParam::fromRecord(const RecordInterface& record) {

  // Set defaults first (in case any missing bits in record)
  setDefaults();

  // Set from the Record 
  if (record.isDefined(EXPORT_RANGE)){
    setExportRange(static_cast<PMS::ExportRange>(record.asInt(EXPORT_RANGE)));
  }
}

Record PlotMSExportParam::toRecord() const {

  // Fill a record 
  Record rec(Record::Variable);
  rec.define(EXPORT_RANGE,getExportRange());

  // Return it
  return rec;
}

void PlotMSExportParam::setExportRange(String exportRangeStr) {
	if ( exportRangeStr == ""){
		exportRange_ = PMS::PAGE_CURRENT;
	}
	else if ( exportRangeStr == "current"){
		exportRange_ = PMS::PAGE_CURRENT;
	}
	else if ( exportRangeStr == "all"){
		exportRange_ = PMS::PAGE_ALL;
	}
	else {
	  exportRange_ = PMS::exportRange(exportRangeStr);
	}

 };

bool PlotMSExportParam::operator==(const PlotMSExportParam& other) const {
	bool equalExports = false;
	if (exportRange_ == other.exportRange_ ){
		equalExports = true;
	}
	return equalExports;
}


void PlotMSExportParam::setDefaults() {
  setExportRange(PMS::PAGE_CURRENT);
}

String PlotMSExportParam::summary() const {

  stringstream ss;
  ss << boolalpha;

  ss << "Export parameters:" << endl;
  ss << " Export Range = " << getExportRange() << endl;


  return ss.str();
}
  

}
