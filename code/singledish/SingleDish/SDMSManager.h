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
  // Copy constructor
  explicit SDMSManager(SDMSManager const &other);

  SDMSManager &operator=(SDMSManager const &other);
  // Destructor
  ~SDMSManager();

  void fillCubeToOutputMs(vi::VisBuffer2 *vb,
			  Cube<Float> const &data_cube);

protected:

  void fillCubeToDataCols(vi::VisBuffer2 *vb,RefRows &rowRef,
		    Cube<Float> const &data_cube);

}; // class SingleDishMS -END


} //# NAMESPACE CASA - END
  
#endif /* _CASA_SDMSMANAGER_H_ */
