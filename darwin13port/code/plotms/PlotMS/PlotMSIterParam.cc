//# PlotMSIterateParams.cc: Iteration paremters container implementation
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
#include <plotms/PlotMS/PlotMSIterParam.h>

namespace casa {

///////////////////////////////////////
// PLOTMSITERATIONPARM DEFINITIONS //
///////////////////////////////////////


// Non-Static //

PlotMSIterParam::PlotMSIterParam() { setDefaults(); }
PlotMSIterParam::~PlotMSIterParam() { }


void PlotMSIterParam::fromRecord(const RecordInterface& record) {    

  // Set defaults first (in case any missing bits in record)
  setDefaults();

  // Set from the Record 
  if (record.isDefined("iterAxis"))
    setIterAxis(record.asString("iterAxis"));

  if (record.isDefined("xSelfScale"))
    setXSelfScale(record.asBool("xSelfScale"));

  if (record.isDefined("ySelfScale"))
    setYSelfScale(record.asBool("ySelfScale"));

  if (record.isDefined("Nx"))
    setNx(record.asInt("Nx"));

  if (record.isDefined("Ny"))
    setNy(record.asInt("Ny"));

}

Record PlotMSIterParam::toRecord() const {

  // Fill a record 
  Record rec(Record::Variable);
  rec.define("iterAxis",iterAxisStr());  // as String
  rec.define("xSelfScale",xSelfScale_);
  rec.define("ySelfScale",ySelfScale_);
  rec.define("Nx",Nx_);
  rec.define("Ny",Ny_);

  // Return it
  return rec;

}



bool PlotMSIterParam::operator==(const PlotMSIterParam& other) const {

  return (iterAxis_ == other.iterAxis_ &&
	  xSelfScale_ == other.xSelfScale_ &&
	  ySelfScale_ == other.ySelfScale_ &&
	  Nx_ == other.Nx_ && 
	  Ny_ == other.Ny_);
}


void PlotMSIterParam::setDefaults() {    

  setIterAxis(PMS::NONE);  // No iteration
  setXSelfScale(False);
  setYSelfScale(False);
  setNx(1);
  setNy(1);

}

String PlotMSIterParam::summary() const {

  stringstream ss;
  ss << boolalpha;

  ss << "Iteration parameters:" << endl;
  ss << " Iteration Axis = " << iterAxisStr() << endl;
  ss << " X self-scale   = " << xSelfScale_ << endl;
  ss << " Y self-scale   = " << ySelfScale_ << endl;
  ss << " Nx             = " << Nx_ << endl;
  ss << " Ny             = " << Ny_ << endl;
  return ss.str();
}
  

}
