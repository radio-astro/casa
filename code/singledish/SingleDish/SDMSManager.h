//# SDMSManager.h: this defines single dish MS transform manager 
//#                inheriting MSTransformManager.
//#
//# Copyright (C) 2015
//# National Astronomical Observatory of Japan
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
#ifndef _CASA_SDMSMANAGER_H_
#define _CASA_SDMSMANAGER_H_

#include <iostream>
#include <string>
#include <map>

#include <libsakura/sakura.h>

#include <casa/aipstype.h>
#include <casa/Containers/Record.h>
#include <casa_sakura/SakuraAlignedArray.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <mstransform/MSTransform/MSTransformManager.h>
#include <scimath/Mathematics/Convolver.h>
#include <scimath/Mathematics/VectorKernel.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SDMSManager : public MSTransformManager {
public:
  // Default constructor
  SDMSManager();
  // Construct from MS name string
  explicit SDMSManager(string const& ms_name);

  // Set user defined Sort columns
  void setSortColumns(Block<Int> sortColumns,
		      bool addDefaultSortCols=false,
		      Double timebin=0.0);

  // Set/unset smoothing parameter
  void setSmoothing(string const &kernelType, float const &kernelWidth);
  void unsetSmoothing();

  // Initialize smoothing operation
  void initializeSmoothing();

  Record getSelRec(string const &spw);
  //MeasurementSet getMS();

  //SDMSManager &operator=(SDMSManager const &other);
  // Destructor
  ~SDMSManager();

  void fillCubeToOutputMs(vi::VisBuffer2 *vb,
			  Cube<Float> const &data_cube);

  void fillCubeToOutputMs(vi::VisBuffer2 *vb,
			  Cube<Float> const &data_cube,
			  Cube<Bool> const *flag_cube);

protected:

  void fillCubeToDataCols(vi::VisBuffer2 *vb,RefRows &rowRef,
			  Cube<Float> const &data_cube,
			  Cube<Bool> const *flag_cube);

  void setIterationApproach();

  int getBlockId(Block<Int> const &data, Int const value);

  // Inspection for smoothing operation
  Vector<Int> inspectNumChan();

private:
  Block<Int> userSortCols_;

  // for Gaussian smoothing
  Bool doSmoothing_;
  VectorKernel::KernelTypes kernelType_;
  float kernelWidth_;

}; // class SDMSManager -END


} //# NAMESPACE CASA - END
  
#endif /* _CASA_SDMSMANAGER_H_ */
