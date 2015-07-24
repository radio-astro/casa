//# PlotMSCalibration.cc: Calibration parameters.
//# Copyright (C) 2015
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
#include <plotms/PlotMS/PlotMSCalibration.h>
#include <synthesis/CalLibrary/CalLibraryTools.h>

namespace casa {

///////////////////////////////////////
// PLOTMSCALIBRATION DEFINITIONS //
///////////////////////////////////////


// Non-Static //

PlotMSCalibration::PlotMSCalibration() { setDefaults(); }

PlotMSCalibration::PlotMSCalibration(const PlotMSCalibration& copy) {
    operator=(copy); }

PlotMSCalibration::~PlotMSCalibration() { }

void PlotMSCalibration::fromRecord(const RecordInterface& record) {    

  // Set from Record 
  if (record.isDefined("UseCallib"))
    setUseCallib(record.asBool("UseCallib"));

  if (record.isDefined("CalLibrary"))
    setCalLibrary(record.asString("CalLibrary"));
}

Record PlotMSCalibration::toRecord() const {

  // Fill and return record 
  Record rec(Record::Variable);
  rec.define("UseCallib", useCallib());
  rec.define("CalLibrary", calLibrary());
  return rec;
}

PlotMSCalibration& PlotMSCalibration::operator=(const PlotMSCalibration& copy) {
    itsCallibUse_ = copy.itsCallibUse_;
    itsCalLibrary_ = copy.itsCalLibrary_;
    return *this;
}

bool PlotMSCalibration::operator==(const PlotMSCalibration& other) const {
  return (itsCallibUse_  == other.itsCallibUse_ &&
	      itsCalLibrary_ == other.itsCalLibrary_);
}

void PlotMSCalibration::setDefaults() {
  setUseCallib(False);
  setCalLibrary("");
}

/*
Record PlotMSCalibration::callibRec() const {
    // Parse the cal library file/string
    Record callibRec = Record();
    if (itsCalLibrary_.length() > 0) {
        callibRec = callibSetParams(itsCalLibrary_);
    }
    return callibRec;
}
*/

String PlotMSCalibration::summary() const {

  stringstream ss;
  ss.precision(12);

  ss << "Plot Data Calibration:" << endl;
  ss << " Use Calibration    = " << useCallib() << endl;
  ss << " Calibration Library   = " << calLibrary() << endl;
  //ss << " Calibration Record = " << callibRec() << endl;
  return ss.str();
}

}
