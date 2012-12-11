//
// C++ Interface: PlotHelper
//
// Description:
//    A small helper class to handle direction coordinate in asapplotter
//
// Author: Kana Sugimoto <kana.sugi@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//

// casacore
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <measures/Measures/MDirection.h>
#include <casa/Logging/LogIO.h>

#include "PlotHelper.h"

//#define KS_DEBUG

using namespace std ;
using namespace casa ;
using namespace asap ;

namespace asap {

PlotHelper::PlotHelper() : dircoord_(0) 
{
#ifdef KS_DEBUG
  cout << "Default constructor nrow = " << data_.nrow() << endl;
#endif
};

PlotHelper::PlotHelper( const ScantableWrapper &s) : dircoord_(0)
{
#ifdef KS_DEBUG
  cout << "Constructing PlotHelper with scantable wrapper" << endl;
#endif
  setScantable(s);
};

PlotHelper::~PlotHelper(){
#ifdef KS_DEBUG
  cout << "Called PlotHelper destructor" << endl;
#endif
  if (dircoord_){
#ifdef KS_DEBUG
    cout << "Destructing dircoord_" << endl;
#endif
    delete dircoord_;
    dircoord_ = 0;
  }
};

void PlotHelper::setScantable( const ScantableWrapper &s )
{
#ifdef KS_DEBUG
  cout << "Setting scantable" << endl;
#endif
  data_ = s ;
};

//   void PlotHelper::setGridParam(const int nx, const int ny, const string cellx, const string celly, string center, const string projection){
//   // Value check of nx and ny
//   if (nx < 1)
//     throw(AipsError("nx should be > 0"));
//   if (ny < 1)
//     throw(AipsError("ny should be > 0"));
//   // Destroy old coord
//   if (dircoord_){
//     cout << "Destructing dircoord_" << endl;
//     delete dircoord_;
//     dircoord_ = 0;
//   }

//   //
//   Double incX, incY;
//   Double centX, centY;
//   MDirection::Types mdt;
//   // projection
//   Projection::Type projType(Projection::type(String(projname)));
//   ///
//   incX = cellx;
//   incY = celly;
//   centX = centx;
//   centY = centy;
//   MDirection::getType(mdt, String(epoch));
//   ///
//   // Get map extent
//   Double xmax, xmin, ymax, ymin;
//   ROArrayColumn<Double> dirCol;
//   MDirection::Types stdt;
//   const bool stset = (data_ ? true : false);

//   if (stset) {
//     dirCol_.attach( data_, "DIRECTION" );
//     Matrix<Double> direction = dirCol.getColumn();
//     minMax(xmin, xmax, direction.row( 0 ));
//     minMax(ymin, ymax, direction.row( 1 ));
//     if (!MDirection::getType(stdt, data_.getDirectionRefString()))
//       throw AipsError("Failed to get direction reference from scantable.");
//   }
//   // center
//   if (center.size() == 0){
//     if (!stset)
//       throw AipsError("Scantable is not set. Could not resolve map center.");
//     cenX = 0.5 * (xmin + xmax);
//     cenY = 0.5 * (ymin + ymax);
//     mdt = stdt;
//   } else {
//     if (!MDirection::getType(mdt,center))
//       throw AipsError("Invalid direction reference in center");
//     if (stset && mdt != stdt)
//       throw AipsError("Direction reference of center should be the same as input scantable");
//     MDirection centdir;
//     centdir.setRefString(String(center));
//     ///
//     Vector<Double> centrad;
//     centrad = centdir.getAngle("rad").getValue();
//     cenX = centrad[0];
//     cenY = centrad[1];
//     // rotaion
//     if (stset) {
//       Double rotnum = round(abs(cenX - )/(C::))
//     }
//   }
//   // cell
//   Quantum<Double> qcellx, qcelly;
//   if (cellx.size() == 0 && celly.size() == 0){
//       // Need resolution
//     if (!stset)
//       throw AipsError("Scantable is not set. Could not resolve cell size.");
//   } else {
//     if (cellx.size() != 0){
//       readQuantity(qcellx, String(cellx));
//       qcellx.convert("rad");
//     } else if (celly.size() == 0) {
//       celly = cellx;
//    
//   }

void PlotHelper::setGridParamVal(const int nx, const int ny, const double cellx, const double celly, const double centx, const double centy, const string epoch, const string projname){
  // Value check of nx and ny
  if (nx < 1)
    throw(AipsError("nx should be > 0"));
  if (ny < 1)
    throw(AipsError("ny should be > 0"));
  // Destroy old coord
  if (dircoord_){
#ifdef KS_DEBUG
    cout << "Destructing dircoord_" << endl;
#endif
    delete dircoord_;
    dircoord_ = 0;
  }

  // center (in rad)
  Double centX(centx), centY(centy);
  // cell size (in rad)
  Double incX(cellx), incY(celly);
  // epoch
  MDirection::Types mdt;
  MDirection::getType(mdt, String(epoch));
  // projection
  Projection::Type projType(Projection::type(String(projname)));

  Matrix<Double> xform(2,2) ;
  xform = 0.0 ;
  xform.diagonal() = 1.0 ;
  dircoord_ = new DirectionCoordinate(mdt, projType,
				      centX, centY, incX, incY,
				      xform,
				      0.5*Double(nx), 
				      0.5*Double(ny)) ; // pixel at center
// 				      0.5*Double(nx-1), 
// 				      0.5*Double(ny-1)) ; // pixel at grid
#ifdef KS_DEBUG
  {//Debug outputs
  cout << "Direction coordinate is set: " << endl;
  Vector<String> units = dircoord_->worldAxisUnits();
  Vector<Double> refv = dircoord_->referenceValue();
  cout <<"Reference: " << MDirection::showType(dircoord_->directionType()) << " " << refv[0] << units[0] << " " << refv[1] << units[1]  << endl;
  Vector<Double> refpix = dircoord_->referencePixel();
  cout <<"Reference Pixel: [" << refpix[0] << ", " << refpix[1] << "]" << endl;
  Vector<Double> inc = dircoord_->increment();
  cout <<"Increments: [" << inc[0] << ", " << inc[1] << "]" << endl;
  cout <<"Projection: " << dircoord_->projection().name() << endl;
  }
#endif
};

vector<double>  PlotHelper::getGridPixel(const int whichrow){
  if (data_.nrow() < 1)
    throw AipsError("Scantable is not set. Could not get direction.");
  else if (whichrow > int(data_.nrow()) - 1)
    throw AipsError("Row index out of range.");
  if (!dircoord_)
    throw AipsError("Direction coordinate is not defined.");

  Vector<Double> pixel;
  MDirection world;
  vector<double> outvec;
  world = data_.getCP()->getDirection(whichrow);
#ifdef KS_DEBUG
  cout << "searching pixel position (world = " << data_.getCP()->getDirectionString(whichrow) << " = [" << world.getAngle("rad").getValue()[0] << ", " << world.getAngle("rad").getValue()[1] << "])" << endl;
#endif
  dircoord_->toPixel(pixel, world);
#ifdef KS_DEBUG
  cout << "got pixel = [" << pixel[0] << ", " << pixel[1] << "]" << endl;
#endif
  // convert pixel to std vector
  pixel.tovector(outvec);
  return outvec;
};

} //namespace asap
