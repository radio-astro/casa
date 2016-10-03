//# PlotMSIterateParam.h: Iteration parameters container
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
#ifndef PLOTMSITERPARAM_H_
#define PLOTMSITERPARAM_H_

#include <plotms/PlotMS/PlotMSConstants.h>

namespace casa {

// Specifies iteration parameters for plotms
class PlotMSIterParam {
public:
  // Static //

  // None
  
  // Non-Static //
    
  // Constructor, which uses default values.
  PlotMSIterParam();
    
  // Destructor.
  ~PlotMSIterParam();
    
    
  // Converts this object to/from a record.  Each field will have a key that
  // is its enum name
  // <group>
  void fromRecord(const casacore::RecordInterface& record);
  casacore::Record toRecord() const;
  // </group>


  // Convenience methods for returning the standard field values.
  // <group>
  PMS::Axis iterAxis() const { return iterAxis_; };
  casacore::String iterAxisStr() const { return PMS::axis(iterAxis_); };
  casacore::Bool isCommonAxisX() const { return commonAxisX_; };
  casacore::Bool isCommonAxisY() const {return commonAxisY_; };
  casacore::Bool isGlobalAxisX() const { return globalScaleX_; };
  casacore::Bool isGlobalAxisY() const { return globalScaleY_; };
  casacore::Int getGridRow() const { return gridRow;};
  casacore::Int getGridCol() const { return gridCol;};

  // </group>
  
  
  // Convenience methods for setting the standard field values.
  // <group>
  void setIterAxis(PMS::Axis iterAxis) { iterAxis_ = iterAxis; };
  void setIterAxis(casacore::String iterAxisStr) { iterAxis_ = PMS::axis(iterAxisStr=="" ? "None" : iterAxisStr); };
  void setCommonAxisX(casacore::Bool commonAxisX ){commonAxisX_ = commonAxisX; };
  void setCommonAxisY(casacore::Bool commonAxisY ){commonAxisY_ = commonAxisY; };
  void setGlobalScaleX(casacore::Bool globalAxisX ){globalScaleX_ = globalAxisX; };
  void setGlobalScaleY(casacore::Bool globalAxisY ){globalScaleY_ = globalAxisY; };
  void setGridRow(casacore::Int nx) { gridRow = casacore::max(nx,-1); };
  void setGridCol(casacore::Int ny) { gridCol = casacore::max(ny,-1); };
  bool isIteration() const;
  // </group>
  
  
  // Equality operators.
  // <group>
  bool operator==(const PlotMSIterParam& other) const;
  bool operator!=(const PlotMSIterParam& other) const {
    return !(operator==(other)); }
  // </group>

  // Print out a summary
  casacore::String summary() const;

  // Sets the default values.
  void setDefaults();
  
private:

  // The Iteration axis
  PMS::Axis iterAxis_;
  static const casacore::String ITER_AXIS;

  // global scale X and Y axes
  casacore::Bool globalScaleX_, globalScaleY_;
  static const casacore::String GLOBAL_SCALE_X;
  static const casacore::String GLOBAL_SCALE_Y;

  //Whether to use a common axis when the grid consists of
  //multiple plots and the axis has a global scale.
  casacore::Bool commonAxisX_, commonAxisY_;
  static const casacore::String COMMON_AXIS_X;
  static const casacore::String COMMON_AXIS_Y;


  // The location of the plot in rows and columns
  casacore::Int gridRow;
  casacore::Int gridCol;
  static const casacore::String ROW_INDEX;
  static const casacore::String COL_INDEX;


    
};

}

#endif /* PLOTMSITERPARAM_H_ */
