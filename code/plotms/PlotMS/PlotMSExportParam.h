//# PlotMSExportParam.h: Export parameters container
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
#ifndef PLOTMSEXPORTPARAM_H_
#define PLOTMSEXPORTPARAM_H_

#include <plotms/PlotMS/PlotMSConstants.h>

#include <casa/namespace.h>

namespace casa {

// Specifies export parameters for plotms
class PlotMSExportParam {
public:
    
  // Constructor, which uses default values.
  PlotMSExportParam();
    
  // Destructor.
  ~PlotMSExportParam();
    
    
  // Converts this object to/from a record.  Each field will have a key that
  // is its enum name
  // <group>
  void fromRecord(const RecordInterface& record);
  Record toRecord() const;
  // </group>


  // Convenience methods for returning the standard field values.
  // <group>
  PMS::ExportRange getExportRange() const { return exportRange_; };
  // </group>
  
  
  // Convenience methods for setting the standard field values.
  // <group>
  void setExportRange(PMS::ExportRange exportRange) {
	  exportRange_ = exportRange;
  };
  void setExportRange(String exportRangeStr);
  // </group>
  
  
  // Equality operators.
  // <group>
  bool operator==(const PlotMSExportParam& other) const;
  bool operator!=(const PlotMSExportParam& other) const {
    return !(operator==(other)); }
  // </group>

  // Print out a summary
  String summary() const;

  // Sets the default values.
  void setDefaults();
  
private:

  // The export range
  PMS::ExportRange exportRange_;
  static const String EXPORT_RANGE;
};

}

#endif /* PLOTMSEXPORTPARAM_H_ */
