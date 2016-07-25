//# MSMetaInfoForCal.h: MS Meta info services in Calibraiton context
//# Copyright (C) 2016
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#ifndef SYNTHESIS_MSMETAINFOFORCAL_H
#define SYNTHESIS_MSMETAINFOFORCAL_H

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>

class MSMetaData;

namespace casa { //# NAMESPACE CASA - BEGIN

class MSMetaInfoForCal {
  
public:

  // Construct from the filename of an MS
  //  (If the MS doesn't exist, services will be trivialized)
  MSMetaInfoForCal(String msname);

  // Construct from a supplied MS object
  MSMetaInfoForCal(const MeasurementSet& ms);

  // Construct from nAnt,nSpw
  MSMetaInfoForCal(uInt nAnt,uInt nSpw,uInt nFld);
  
  // Dtor
  ~MSMetaInfoForCal();

  // The MS name
  String msname() const { return msname_; };

  // Signal if a real MS was specified in ctor
  Bool msOk() const { return msOk_; };

  // Return access to MSMetaData object (if avail)
  const MSMetaData& msmd() const;

  // The number of antennas
  uInt nAnt() const { return nAnt_;};

  // Antenna name, by index
  String antennaName(uInt iant) const;

  // All antenna names, as vector
  void antennaNames(Vector<String>& antnames) const;

  // The number of spws
  uInt nSpw() const { return nSpw_;};

  // Spw name, by index
  String spwName(uInt ispw) const;

  // The number of fields
  uInt nFld() const { return nFld_;};

  // Field name, by index
  String fieldName(uInt ifld) const;

  // All field names, as vector
  void fieldNames(Vector<String>& fldnames) const;

  // Field id at time
  Int fieldIdAtTime(Double time) const;

  // Scan number at time
  Int scanNumberAtTime(Double time) const;


private:

  // The supplied MS name
  String msname_;

  // Is the MS available (else spoof)
  Bool msOk_;

  // Remember basic shapes
  uInt nAnt_, nSpw_, nFld_;

  // MS pointer
  MeasurementSet *ms_;

  // MSMetaData pointer
  MSMetaData *msmd_;


};


} //# NAMESPACE CASA - END

#endif


