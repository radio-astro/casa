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
#include <casa/Quanta/MVAngle.h>
#include <casa/Logging/LogIO.h>

#include <measures/Measures/MDirection.h>
#include <tables/Tables/TableRecord.h>


#include "PlotHelper.h"

#define SMALL_ANGLE 1.0e-7

// #ifndef KS_DEBUG
// #define KS_DEBUG
// #endif

using namespace std ;
using namespace casa ;
using namespace asap ;

namespace asap {

PlotHelper::PlotHelper() : dircoord_p(0) 
{
#ifdef KS_DEBUG
  cout << "Default constructor nrow = " << data_p->nrow() << endl;
#endif
};

PlotHelper::PlotHelper( const ScantableWrapper &s) : dircoord_p(0)
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
  if (dircoord_p){
#ifdef KS_DEBUG
    cout << "Destructing dircoord_p" << endl;
#endif
    delete dircoord_p;
    dircoord_p = 0;
  }
};

void PlotHelper::setScantable( const ScantableWrapper &s )
{
#ifdef KS_DEBUG
  cout << "Setting scantable" << endl;
#endif
  data_p = s.getCP();
};


DirectionCoordinate PlotHelper::getSTCoord(const int nx, const int ny,
					   const Projection::Type ptype)
{
  LogIO os(LogOrigin("PlotHelper","getSTCoord()", WHERE));
  os << "Getting pointing information of the scantable." << LogIO::POST;
  if (data_p->nrow() < 1)
    throw AipsError("Scantable is not set. Please set a scantable first.");

  // First, generate rough direction coordinate.
  DirectionCoordinate coord;
  Double incx, incy;
  MDirection::Types mdt;
  ROArrayColumn<Double> dircol;
  Double xmax, xmin, ymax, ymin;
  Double centx, centy;
  Matrix<Double> xform(2,2);
  xform = 0.0;
  xform.diagonal() = 1.0;
  // Rough estimates of center and cell from scantable DIRECTIONs.
  dircol.attach( data_p->table(), "DIRECTION" );
  const Vector<String> udir = dircol.keywordSet().asArrayString("QuantumUnits");
  const Matrix<Double> direction = dircol.getColumn();
  minMax(xmin, xmax, direction.row(0));
  minMax(ymin, ymax, direction.row(1));
  if (!MDirection::getType(mdt, data_p->getDirectionRefString()))
    throw AipsError("Failed to get direction reference from scantable.");
  centx = 0.5 * (xmin + xmax);
  centy = 0.5 * (ymin + ymax);
  incx = abs(xmax - xmin) / (double) nx * cos(centy);
  incy = abs(ymax - ymin) / (double) ny;
  // Direction coordinate seems not work well with inc=0. set very small value.
  if (incx == 0.) incx = SMALL_ANGLE;
  if (incy == 0.) incy = SMALL_ANGLE;
  // Generate a temporal direction coordinte
  coord = DirectionCoordinate(mdt, ptype,
				centx, centy, incx, incy, xform,
				0.5*Double(nx), 0.5*Double(ny));
  coord.setWorldAxisUnits(udir);
#ifdef KS_DEBUG
  {//Debug outputs
    cout << "Generated a temporal direction coordinate of a scantable: " << endl;
    Vector<String> units = coord.worldAxisUnits();
    Vector<Double> refv = coord.referenceValue();
    cout <<"- Reference: " << MDirection::showType(coord.directionType()) << " " << refv[0] << units[0] << " " << refv[1] << units[1]  << endl;
    Vector<Double> refpix = coord.referencePixel();
    cout <<"- Reference Pixel: [" << refpix[0] << ", " << refpix[1] << "]" << endl;
    Vector<Double> inc = coord.increment();
    cout <<"- Increments: [" << inc[0] << ", " << inc[1] << "]" << endl;
    cout <<"- Projection: " << coord.projection().name() << endl;
  }
#endif
  return coord;
}

void PlotHelper::setGridParam(const int nx, const int ny,
			      const string cellx, const string celly,
			      string center, const string projname)
{
  LogIO os(LogOrigin("PlotHelper","setGridParam()", WHERE));
  // Value check of nx and ny
  if (nx < 1)
    throw(AipsError("nx should be > 0"));
  if (ny < 1)
    throw(AipsError("ny should be > 0"));
  // Destroy old coord
  if (dircoord_p){
#ifdef KS_DEBUG
    cout << "Destructing old dircoord_p" << endl;
#endif
    delete dircoord_p;
    dircoord_p = 0;
  }

  // Check for availability of data_p
  const bool stset = ((data_p->nrow() > 0) ? true : false);
  const bool needst = center.empty() || (cellx.empty() && celly.empty());
  if (needst && !stset)
    throw AipsError("Could not resolve grid parameters. Please set a scantable first.");

  // projection
  Projection::Type projtype(Projection::type(String(projname)));

  // Calculate projected map center (in world coordinate)
  // and extent (in pixel coordinate) from scantable DIRECIONs (if necessary).
  MDirection stcent;
  Double stxmax, stxmin, stymax, stymin;
  MDirection::Types stdt;
  DirectionCoordinate stcoord;
  Quantum<Double> stincx, stincy;
  if (needst && stset) {
    stcoord = getSTCoord(nx, ny, projtype);
    Vector<Double> inc = stcoord.increment();
    Vector<String> units = stcoord.worldAxisUnits();
    stincx = Quantum<Double>(inc[0], units[0]);
    stincy = Quantum<Double>(inc[1], units[0]);
    stdt = stcoord.directionType();
    // Get the extent of directions in Pixel coordinate
    ROArrayColumn<Double> dircol;
    dircol.attach( data_p->table(), "DIRECTION" );
    const Matrix<Double> direction = dircol.getColumn();
    Matrix<Double> dirpix;
    Vector<Bool> failures;
    if (!stcoord.toPixelMany(dirpix, direction, failures))
      throw AipsError("Failed to get directions in pixel coordinate.");
    minMax(stxmin, stxmax, dirpix.row(0));
    minMax(stymin, stymax, dirpix.row(1));
    // Get the direction center in World coordinate.
    Vector<Double> centpix(2);
    centpix[0] = 0.5 * (stxmin + stxmax);
    centpix[1] = 0.5 * (stymin + stymax);
    stcoord.toWorld(stcent, centpix);
#ifdef KS_DEBUG
    {//Debug output
      Quantum< Vector<Double> > qcent;
      cout << "Got the center and map extent of scantable." << endl;
      qcent = stcent.getAngle();
      cout << "- Center of DIRECTIONs: " << stcent.getRefString() << " "
	   << qcent.getValue()[0] << qcent.getUnit() << " "
	   << qcent.getValue()[1] << qcent.getUnit() << endl;
      cout << "- Map extent: [" << (stxmax-stxmin)*stincx.getValue() << ", "
	   << (stymax-stymin)*stincy.getValue() << "] ( " << stincx.getUnit() << ")" << endl;
    }
#endif
  }

  os << "Setting grid parameters" << LogIO::POST;
  // Now, define direction coordinate from input parameters (in radian)
  Double incx, incy;
  Double centx, centy;
  MDirection::Types mdt;
  // center
  if (center.empty()){
    os << "center is not specified. Using pointing center of the scantable." << LogIO::POST;
    if (!stset)
      throw AipsError("Scantable is not set. Could not resolve map center.");

    centx = stcent.getAngle("rad").getValue()[0];
    centy = stcent.getAngle("rad").getValue()[1];
    mdt = stdt;
  } else {
    os << "Using user defined center" << LogIO::POST;
    // Parse center string
    string::size_type pos0 = center.find(" ");
    
    if (pos0 == string::npos)
      throw AipsError("bad string format in center direction");

    string::size_type pos1 = center.find(" ", pos0+1);
    String sepoch, sra, sdec;
    if (pos1 != string::npos) {
      sepoch = center.substr(0, pos0);
      sra = center.substr(pos0+1, pos1-pos0);
      sdec = center.substr(pos1+1);
    } else {
      sepoch = "J2000";
      sra = center.substr(0, pos0);
      sdec = center.substr(pos0+1);
    }
    if (!MDirection::getType(mdt,sepoch))
      throw AipsError("Invalid direction reference in center");
    if (stset){
      if ( !needst &&
	   !MDirection::getType( stdt, data_p->getDirectionRefString() ) )
	throw AipsError("Failed to get direction reference from scantable.");
      if (mdt != stdt)
	throw AipsError("Direction reference of center should be the same as input scantable");
    }
    QuantumHolder qh ;
    String err ;
    qh.fromString(err, sra);
    Quantum<Double> ra  = qh.asQuantumDouble();
    qh.fromString(err, sdec) ;
    Quantum<Double> dec = qh.asQuantumDouble();
    centx = ra.getValue("rad");
    centy = dec.getValue("rad");
    // rotaion
    if (stset) {
      Double stcentx = stcent.getAngle("rad").getValue()[0];
      Double rotnum = round( abs(centx - stcentx) / (C::_2pi) );
      if (centx < stcentx) rotnum *= -1;
      centx -= (rotnum * C::_2pi);
    }
  }
  os << "Grid center: ( "  << centx  << " rad , " << centy << " rad ) " << LogIO::POST;

  // cell
  if (cellx.empty() && celly.empty()){
    os << "cell size is not specified. Using cell size to cover all pointings in the scantable." << LogIO::POST;
    if (!stset)
      throw AipsError("Scantable is not set. Could not resolve cell size.");

    Vector<Double> centpix;
    MDirection centmd = MDirection(Quantum<Double>(centx, "rad"),
				   Quantum<Double>(centy, "rad"), mdt);
    stcoord.toPixel(centpix, centmd);
#ifdef KS_DEBUG
    cout << "- got centpix [" << centpix[0] << ", " << centpix[1] << "]" <<endl;
#endif
    // Direction coordinate seems not work well with inc=0. set very small value. 
    Double wx = max( max( abs(stxmax-centpix[0]), abs(stxmin-centpix[0]) ), 0.5 )
      * 2 * stincx.getValue("rad");
    Double wy = max( max( abs(stymax-centpix[1]), abs(stymin-centpix[1]) ), 0.5 )
      * 2 * stincy.getValue("rad");
    incx = wx / max(nx - 1., 1.);
    incy = wy / max(ny - 1., 1.);
  } else {
    os << "Using user defined cell size" << LogIO::POST;
    Quantum<Double> qcellx, qcelly;
    if (!cellx.empty() && !celly.empty()){
      readQuantity(qcellx, String(cellx));
      readQuantity(qcelly, String(celly));
    } else if (celly.empty()) {
      readQuantity(qcellx, String(cellx));
      qcelly = qcellx;
    } else { //only celly is defined
      readQuantity(qcelly, String(celly));
      qcellx = qcelly;
    }
    incx = qcellx.getValue("rad");
    incy = qcelly.getValue("rad");
  }
  // inc should be negative to transpose plot
  incx = -abs(incx);
  incy = -abs(incy);

  os << "Spacing: ( " << abs(incx) << " rad , " << abs(incy) << " rad )" <<endl;

  setupCoord(mdt, projtype, centx, centy, incx, incy,
	     0.5*Double(nx), 0.5*Double(ny)) ; // pixel at center)

};

void PlotHelper::setGridParamVal(const int nx, const int ny,
				 const double cellx, const double celly,
				 const double centx, const double centy,
				 const string epoch, const string projname){
  LogIO os(LogOrigin("PlotHelper","setGridParamVal()", WHERE));
  // Value check of nx and ny
  if (nx < 1)
    throw(AipsError("nx should be > 0"));
  if (ny < 1)
    throw(AipsError("ny should be > 0"));
  // Destroy old coord
  if (dircoord_p){
#ifdef KS_DEBUG
    cout << "Destructing old dircoord_p" << endl;
#endif
    delete dircoord_p;
    dircoord_p = 0;
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

  setupCoord(mdt, projType, centX, centY, incX, incY,
	     0.5*Double(nx), 0.5*Double(ny)) ; // pixel at center
// 	     0.5*Double(nx-1), 0.5*Double(ny-1)) ; // pixel at grid

};


void PlotHelper::setupCoord(const MDirection::Types mdt,
			    const Projection::Type pjt,
			    const Double centx, const Double centy,
			    const Double incx, const Double incy,
			    const Double refx, const Double refy)
{
  LogIO os(LogOrigin("PlotHelper","setupCoord()", WHERE));
  // Destroy old coord
  if (dircoord_p){
#ifdef KS_DEBUG
    cout << "Destructing old dircoord_p" << endl;
#endif
    delete dircoord_p;
    dircoord_p = 0;
  }

  Matrix<Double> xform(2,2) ;
  xform = 0.0 ;
  xform.diagonal() = 1.0 ;
  dircoord_p = new DirectionCoordinate(mdt, pjt, centx, centy, incx, incy,
				      xform, refx, refy);
  {//Summary
    os << "Successfully generated grid coordinate:" << LogIO::POST;
    Vector<String> units = dircoord_p->worldAxisUnits();
    Vector<Double> refv = dircoord_p->referenceValue();
    os <<"- Reference Direction : "
       << MDirection::showType(dircoord_p->directionType())
       << " " << refv[0] << units[0] << " " << refv[1] << units[1] << LogIO::POST;
    Vector<Double> refpix = dircoord_p->referencePixel();
    os <<"- Reference Pixel     : [" << refpix[0] << ", " << refpix[1] << "]" << LogIO::POST;
    Vector<Double> inc = dircoord_p->increment();
    os <<"- Increments          : [" << inc[0] << ", " << inc[1] << "]" << LogIO::POST;
    os <<"- Projection Type     : " << dircoord_p->projection().name() << LogIO::POST;
  }
};

vector<double>  PlotHelper::getGridPixel(const int whichrow)
{
  if (data_p->nrow() < 1)
    throw AipsError("Scantable is not set. Could not get direction.");
  else if (whichrow > int(data_p->nrow()) - 1)
    throw AipsError("Row index out of range.");
  if (!dircoord_p)
    throw AipsError("Direction coordinate is not defined.");

  Vector<Double> pixel;
  MDirection world;
  vector<double> outvec;
  world = data_p->getDirection(whichrow);
#ifdef KS_DEBUG
  cout << "searching pixel position (world = " << data_p->getDirectionString(whichrow) << " = [" << world.getAngle("rad").getValue()[0] << ", " << world.getAngle("rad").getValue()[1] << "])" << endl;
#endif
  dircoord_p->toPixel(pixel, world);
#ifdef KS_DEBUG
  cout << "got pixel = [" << pixel[0] << ", " << pixel[1] << "]" << endl;
#endif
  // convert pixel to std vector
  pixel.tovector(outvec);
  return outvec;
};

string PlotHelper::getGridRef()
{
  if (!dircoord_p)
    throw AipsError("Direction coordinate is not defined. Please set it first.");

  string outref;

  Vector<String> units = dircoord_p->worldAxisUnits();
  Vector<Double> refv = dircoord_p->referenceValue();
  MVAngle lon( Quantum<Double>(refv[0], units[0]) );
  MVAngle lat ( Quantum<Double>(refv[1], units[1]) );
  outref = MDirection::showType(dircoord_p->directionType()) + " "
    + lon(0.0).string(MVAngle::TIME, 9) + " "
    + lat.string(MVAngle::ANGLE+MVAngle::DIG2, 9);

  return outref;
};

vector<double>  PlotHelper::getGridCellVal()
{
  if (!dircoord_p)
    throw AipsError("Direction coordinate is not defined. Please set it first.");

  vector<double> outinc(2);
  Vector<Double> inc = dircoord_p->increment();
  Vector<String> units = dircoord_p->worldAxisUnits();
  MVAngle qincx( Quantum<Double>(inc[0], units[0]) );
  MVAngle qincy( Quantum<Double>(inc[1], units[1]) );
  outinc[0] = (double) abs(qincx.radian());
  outinc[1] = (double) abs(qincy.radian());

  return outinc;
};


} //namespace asap
