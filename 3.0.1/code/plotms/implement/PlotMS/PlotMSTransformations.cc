//# PlotMSAveraging.cc: Averaging parameters.
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
#include <plotms/PlotMS/PlotMSTransformations.h>

namespace casa {

///////////////////////////////////////
// PLOTMSTRANSFORMATIONS DEFINITIONS //
///////////////////////////////////////


// Non-Static //

PlotMSTransformations::PlotMSTransformations() { setDefaults(); }
PlotMSTransformations::~PlotMSTransformations() { }


void PlotMSTransformations::fromRecord(const RecordInterface& record) {    

  // Set from Record 
  if (record.isDefined("Frame"))
    setFrame(record.asString("Frame"));

  if (record.isDefined("Veldef"))
    setVelDef(record.asString("Veldef"));

  if (record.isDefined("RestFreq"))
    setRestFreq(record.asDouble("RestFreq"));

  if (record.isDefined("XpcOffset"))
    setXpcOffset(record.asDouble("XpcOffset"));

  if (record.isDefined("YpcOffset"))
    setYpcOffset(record.asDouble("YpcOffset"));


}

Record PlotMSTransformations::toRecord() const {

  // Fill record 
  Record rec(Record::Variable);
  rec.define("Frame",frameStr());    // as String
  rec.define("Veldef",veldefStr());  // as String
  rec.define("RestFreq",restFreq());
  rec.define("XpcOffset",xpcOffset());
  rec.define("YpcOffset",ypcOffset());

  // Return it
  return rec;

}



bool PlotMSTransformations::operator==(const PlotMSTransformations& other) const {

  return (mfreqType_ == other.mfreqType_ &&
	  mdoppType_ == other.mdoppType_ &&
	  restFreq_  == other.restFreq_  &&
	  XpcOffset_ == other.XpcOffset_ &&
	  YpcOffset_ == other.YpcOffset_);

}


void PlotMSTransformations::setDefaults() {    

  setFrame(MFrequency::N_Types);  // this means adopt native MS freq frame (spw)
  setVelDef(MDoppler::RADIO);
  setRestFreq(0.0);               // Means generate relative velocity axis
  setXpcOffset(0.0);
  setYpcOffset(0.0);

}

String PlotMSTransformations::summary() const {

  stringstream ss;
  ss.precision(12);

  ss << "Plot Data Transformations:" << endl;
  ss << " Frame    = " << frameStr() << endl;
  ss << " VelDef   = " << veldefStr() << endl;
  ss << " RestFreq = " << restFreq() << " MHz" << endl;
  ss << " dX       = " << xpcOffset() << " arcsec" << endl;
  ss << " dY       = " << ypcOffset() << " arcsec" << endl;

  return ss.str();
}
  

}
