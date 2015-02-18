#ifndef _CASA_SDMSMANAGER_H_
#define _CASA_SDMSMANAGER_H_

#include <iostream>
#include <string>

#include <libsakura/sakura.h>

#include <casa/aipstype.h>
#include <casa/Containers/Record.h>
#include <casa_sakura/SakuraAlignedArray.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <mstransform/MSTransform/MSTransformManager.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SDMSManager : public MSTransformManager {
public:
  // Default constructor
  SDMSManager();
  // Construct from MS name string
  explicit SDMSManager(string const& ms_name);

  // Set user defined Sort columns
  void setSortColumns(Block<Int> sortColumns,
		      bool addDefaultSortCols=false);

  //SDMSManager &operator=(SDMSManager const &other);
  // Destructor
  ~SDMSManager();

  void fillCubeToOutputMs(vi::VisBuffer2 *vb,
			  Cube<Float> const &data_cube);

protected:

  void fillCubeToDataCols(vi::VisBuffer2 *vb,RefRows &rowRef,
		    Cube<Float> const &data_cube);

  void setIterationApproach();

  int getBlockId(Block<Int> const &data, Int const value);

private:
  Block<Int> userSortCols_;

}; // class SingleDishMS -END


} //# NAMESPACE CASA - END
  
#endif /* _CASA_SDMSMANAGER_H_ */
