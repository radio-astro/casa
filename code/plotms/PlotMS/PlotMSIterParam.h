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

#include <casa/namespace.h>

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
  void fromRecord(const RecordInterface& record);
  Record toRecord() const;
  // </group>


  // Convenience methods for returning the standard field values.
  // <group>
  PMS::Axis iterAxis() const { return iterAxis_; };
  String iterAxisStr() const { return PMS::axis(iterAxis_); };
  Bool xSelfScale() const { return xSelfScale_; };
  Bool ySelfScale() const { return ySelfScale_; };
  Bool isCommonAxisX() const { return commonAxisX_; };
  Bool isCommonAxisY() const {return commonAxisY_; };
  Int Nx() const { return Nx_;};
  Int Ny() const { return Ny_;};

  // </group>
  
  
  // Convenience methods for setting the standard field values.
  // <group>
  void setIterAxis(PMS::Axis iterAxis) { iterAxis_ = iterAxis; };
  void setIterAxis(String iterAxisStr) { iterAxis_ = PMS::axis(iterAxisStr=="" ? "None" : iterAxisStr); };
  void setXSelfScale(Bool xSelfScale) { xSelfScale_ = xSelfScale; };
  void setYSelfScale(Bool ySelfScale) { ySelfScale_ = ySelfScale; };
  void setCommonAxisX(Bool commonAxisX ){commonAxisX_ = commonAxisX; };
  void setCommonAxisY(Bool commonAxisY ){commonAxisY_ = commonAxisY; };
  void setNx(Int nx) { Nx_ = max(nx,1); };
  void setNy(Int ny) { Ny_ = max(ny,1); };
  // </group>
  
  
  // Equality operators.
  // <group>
  bool operator==(const PlotMSIterParam& other) const;
  bool operator!=(const PlotMSIterParam& other) const {
    return !(operator==(other)); }
  // </group>

  // Print out a summary
  String summary() const;

  // Sets the default values.
  void setDefaults();
  
private:

  // The Iteration axis
  PMS::Axis iterAxis_;

  // Self-scale the X and Y axes
  Bool xSelfScale_, ySelfScale_;

  //Whether to use a common axis when the grid consists of
  //multiple plots.
  Bool commonAxisX_, commonAxisY_;

  // The number of plots in X and Y
  Int Nx_,Ny_;

    
};

}

#endif /* PLOTMSITERPARAM_H_ */
