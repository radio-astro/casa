//# PlotMSCalibration.h: (On The Fly) Calibration parameters.
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
#ifndef PLOTMSCALIBRATION_H_
#define PLOTMSCALIBRATION_H_

#include <plotms/PlotMS/PlotMSConstants.h>
#include <casa/Containers/Record.h>

namespace casa {

// Specifies calibration parameters for an MS.
class PlotMSCalibration {
public:
  // Constructor, which sets default values.
  PlotMSCalibration();

  // Copy constructor
  PlotMSCalibration(const PlotMSCalibration& copy);
    
  // Destructor.
  ~PlotMSCalibration();
    
    
  // Converts this object to/from a record.  Each field will have a key that
  // is its enum name
  // <group>
  void fromRecord(const casacore::RecordInterface& record);
  casacore::Record toRecord() const;
  // </group>


  // Convenience methods for returning the standard field values.
  // <group>
  casacore::Bool useCallib() const { return itsCallibUse_; };
  casacore::String calLibrary() const  { return itsCalLibrary_; }

  /*
  // Returns parsed cal library as a Record
  casacore::Record callibRec() const;
  */
  // </group>
  
  
  // Convenience methods for setting the standard field values.
  // <group>
  void setUseCallib(const casacore::Bool use) { itsCallibUse_ = use; }
  void setCalLibrary(const casacore::String& callib) { itsCalLibrary_ = callib;}
  // </group>
  
  
  // Equality operators.
  // <group>
  bool operator==(const PlotMSCalibration& other) const;
  bool operator!=(const PlotMSCalibration& other) const {
    return !(operator==(other)); }
  // </group>

  // Copy operator.
  PlotMSCalibration& operator=(const PlotMSCalibration& copy);
  // Print out a summary
  casacore::String summary() const;
  
private:
  casacore::Bool itsCallibUse_;
  casacore::String itsCalLibrary_;
  //casacore::Record itsCallibRec_;

  // Sets the default values.
  void setDefaults();
    
};

}

#endif /* PLOTMSCALIBRATION_H_ */
