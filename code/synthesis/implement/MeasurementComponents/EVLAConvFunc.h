#ifndef SYNTHESIS_EVLACONVFUNC_H
#define SYNTHESIS_EVLACONVFUNC_H
//
//TEMPS
#include <images/Images/ImageInterface.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/BeamCalc.h>
#include <synthesis/MeasurementComponents/CFStore.h>
#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>
//TEMPS
//
// Temp. function for staged re-factoring
//    
namespace casa { //# NAMESPACE CASA - BEGIN
  class EVLAConvFunc
  {
  public:
    EVLAConvFunc():bandID_p(-1){};
    Int getVLABandID(Double& freq,String&telescopeName);
    Bool findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R);
    void makeConvFunction(const ImageInterface<Complex>& image,
			  const Int wConvSize,
			  const VisBuffer& vb,Float pa,
			  const Vector<Int>& polMap,
			  Vector<Int>& cfStokes,
			  CFStore& cfs,
			  CFStore& cfwts);
    int getVisParams(const VisBuffer& vb);
    Int makePBPolnCoords(CoordinateSystem& squintCoord,
			 Vector<Int>& cfStokes,
			 const VisBuffer&vb,
			 const Vector<Int>& polMap);
    Int bandID_p;
    Float Diameter_p, Nant_p, HPBW, sigma;
    
    LogIO& logIO() {return logIO_p;}
    LogIO logIO_p;

  };
};
#endif
