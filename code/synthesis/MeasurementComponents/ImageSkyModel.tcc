#include <casa/OS/HostInfo.h>
#include <synthesis/MeasurementComponents/ImageSkyModel.h>

namespace casa { //# NAMESPACE CASA - BEGIN

#define MEMFACTOR 8.0
#if !(defined (AIPS_64B))
#  undef  MEMFACTOR
#  define MEMFACTOR 18.0
#endif

template<class M>
TempImage<M>* ImageSkyModel::getTempImage(const TiledShape& imgShp,
                                          const CoordinateSystem& imgCoords,
                                          const uInt nMouthsToFeed)
{
  TempImage<M>* timgptr = NULL;
  Double memoryMB = HostInfo::memoryTotal() / 1024.0 / MEMFACTOR;

  if(nMouthsToFeed > 1) // esp. !0
    memoryMB /= nMouthsToFeed;

  if(memoryMB > 0.0){
    while(!timgptr && memoryMB > 1.0){
      try{
        timgptr = new TempImage<M> (imgShp, imgCoords, memoryMB);
        AlwaysAssert(timgptr, AipsError);
      }
      catch(...){
        memoryMB *= 0.5;
      }
    }
  }
  else{
    timgptr = new TempImage<M> (imgShp, imgCoords, 0.0);
  }
  AlwaysAssert(timgptr, AipsError);
  return timgptr;
}

} //# NAMESPACE CASA - END

