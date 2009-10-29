/***
 * Framework independent implementation file for coordsys...
 *
 * Implement the coordsys component here.
 * 
 * // coordsys_cmpt.cc: defines coordsys class which implements
 * // functionality of the coordinate system component
 *
 * @author
 * @version 
 ***/

#include <iostream>

#include <casa/Arrays/Matrix.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogFilter.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/QuantumHolder.h>
//#include <casa/Utilities/DataType.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/TabularCoordinate.h>
#include <xmlcasa/images/coordsys_cmpt.h>
#include <measures/Measures/MCFrequency.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MeasureHolder.h>

#include <casa/namespace.h>

using namespace std;

namespace casac {

coordsys::coordsys() : itsParentImageName("unknown")
{
  try {
    itsCoordSys = new CoordinateSystem();
    itsLog = new LogIO();

    Vector<String> empty(0);
    addCoordinate(*itsCoordSys, False, False, empty, 0, False);

    // Give it a meaningful ObsInfo
    ObsInfo obsInfo;
    obsInfo.setTelescope(String("ATCA"));
    obsInfo.setObserver(String("Karl Jansky"));

    // It must be easier than this...  USe 0.0001
    // so that roundoff does not tick the 0 to 24
    Time time;
    time.now();
    MVTime time2(time);
    MVEpoch time4(time2);
    MEpoch date(time4);
    obsInfo.setObsDate(date);
    //
    itsCoordSys->setObsInfo(obsInfo);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
}

// private constructor for on fly components
  coordsys::coordsys(const CoordinateSystem *inCS) : itsParentImageName("unknown")
{
  try {
    itsCoordSys = new CoordinateSystem(*inCS);
    itsLog = new LogIO();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
	    << x.getMesg() << LogIO::POST;
  }
}

coordsys::~coordsys()
{
  delete itsCoordSys;
  delete itsLog;
}

Bool unset(const ::std::vector<bool> &par) {
  if (par.size() == 1 && par[0] == false) {
    return true;
  } else {
    return false;
  }
}

Bool unset(const ::std::vector<int> &par) {
  if (par.size() == 1 && par[0]==-1) {
    return true;
  } else {
    return false;
  }
}

Bool unset(const ::casac::record &theRec) {
  Bool rstat(True);
  for(::casac::rec_map::const_iterator rec_it = theRec.begin();
      rec_it != theRec.end(); rec_it++){
    rstat = False;
    break;
  }
  return rstat;
}

Bool unset(const ::casac::variant &theVar) {
  Bool rstat(False);
  if ( (theVar.type() == ::casac::variant::BOOLVEC)
       && (theVar.size() == 0) ) rstat = True;
  return rstat;
}

Bool qcompare(const casa::Quantity& v, const casa::Quantity a) {
  return ( (v.getFullUnit().getValue()) == (a.getFullUnit().getValue()) );
}

Bool checkfreq(const casa::Quantity x) {
  return (qcompare(x, casa::Quantity(1.0,"s")) ||
	  qcompare(x, casa::Quantity(1.0,"Hz")) ||
	  qcompare(x, casa::Quantity(1.0,"deg/s")) ||
	  qcompare(x, casa::Quantity(1.0,"m")) ||
	  qcompare(x, casa::Quantity(1.0,"m-1")) ||
	  qcompare(x, casa::Quantity(1.0,"(eV)")) ||
	  qcompare(x, casa::Quantity(1.0,"kg.m"))
	  );
}

casa::QuantumHolder
quantumHolderFromVar(String error, const ::casac::variant& theVar){
  casa::QuantumHolder qh;
  try {
    String error;
    if(theVar.type()== ::casac::variant::STRING ) {
      if(!qh.fromString(error, theVar.toString())) {
	//        *itsLog << LogIO::SEVERE << "Error " << error
	//      << " in converting quantity "<< LogIO::POST;
      }
    }
    if (theVar.type()== ::casac::variant::STRINGVEC){
      //      *itsLog << LogIO::WARN << "Only first vector element will be used."
      //              << LogIO::POST;
      if(!qh.fromString(error, theVar.toStringVec()[0])) {
	//        *itsLog << LogIO::SEVERE << "Error " << error
	//                << " in converting quantity "<< LogIO::POST;
      }
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!qh.fromRecord(error, *ptrRec)){
	//        *itsLog << LogIO::SEVERE << "Error " << error
	//                << " in converting quantity "<< LogIO::POST;
      }
      delete ptrRec;
    }
  } catch (AipsError x) {
    //    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
    //            << LogIO::POST;
    RETHROW(x);
  }
  return qh;
}
  
::casac::coordsys * 
coordsys::newcoordsys(const bool direction, const bool spectral,
		      const std::vector<std::string>& stokes,
		      const int linear, const bool tabular)
{
  ::casac::coordsys *newCS = 0;

  try {
    itsCoordSys = new CoordinateSystem();
    itsLog = new LogIO();
    *itsLog << LogOrigin("coordsys", "newcoordsys");

    Vector<String> Stokes;
    if (stokes.size()==1) { // just a string
      int n = sepCommaEmptyToVectorStrings(Stokes, stokes[0]);
      if ( (n==1) && (Stokes[0] == "" || Stokes[0] == " ") ) {
	Stokes.resize(0);
      }
    } else {
      Stokes = toVectorString(stokes);
    }
      
    //    Vector<String> Stokes = toVectorString(stokes);
    //    if (Stokes.size() == 1 && ((Stokes[0] == "") || (Stokes[0]==" "))) {
    //      Stokes.resize(0);
    //    }
    addCoordinate(*itsCoordSys, direction, spectral, Stokes, linear, tabular);

    // Give it a meaningful ObsInfo
    ObsInfo obsInfo;
    obsInfo.setTelescope(String("ATCA"));
    obsInfo.setObserver(String("Karl Jansky"));

    // It must be easier than this...  USe 0.0001
    // so that roundoff does not tick the 0 to 24
    Time time;
    time.now();
    MVTime time2(time);
    MVEpoch time4(time2);
    MEpoch date(time4);
    obsInfo.setObsDate(date);
    //
    itsCoordSys->setObsInfo(obsInfo);

    newCS = new ::casac::coordsys(itsCoordSys);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return newCS;
}

// Add a new default coordinate.  Only Tabular implemented presently.
bool
coordsys::addcoordinate(const bool direction, const bool spectral,
			const std::vector<std::string>& stokes,
			const int linear, const bool tabular)
{

    Vector<String> Stokes;
    if (stokes.size()==1) { // just a string
      int n = sepCommaEmptyToVectorStrings(Stokes, stokes[0]);
      if ( (n==1) && (Stokes[0] == "" || Stokes[0] == " ") ) {
	Stokes.resize(0);
      }
    } else {
      Stokes = toVectorString(stokes);
    }
    addCoordinate(*itsCoordSys, direction, spectral, Stokes, linear, tabular);
    return true;
}


std::string
coordsys::parentname()
{
  return itsParentImageName;
}

bool
coordsys::setparentname(const std::string& imagename)
{
  itsParentImageName = imagename;
  return true;
}

std::vector<int>
coordsys::axesmap(const bool toWorld)
{
  std::vector<int> rstat;
  *itsLog << LogOrigin("coordsys", "axesmap");

  try {
    Vector<Int> map;
    if (toWorld) {
      map.resize(itsCoordSys->nPixelAxes());
      for (uInt i=0; i<itsCoordSys->nPixelAxes(); i++) {
	map(i) = itsCoordSys->pixelAxisToWorldAxis(i);
	//if (map(i) >= 0) map(i)  += 1;    // 1-rel
      }
    } else {
      map.resize(itsCoordSys->nWorldAxes());
      for (uInt i=0; i<itsCoordSys->nWorldAxes(); i++) {
	map(i) = itsCoordSys->worldAxisToPixelAxis(i);
	//if (map(i) >= 0) map(i)  += 1;    // 1-rel
      }
    }
    map.tovector(rstat);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<std::string>
coordsys::axiscoordinatetypes(const bool world)
{
  std::vector<std::string> rstat;
  
  try{
    *itsLog << LogOrigin("coordsys", "axiscoordinatetypes");

    Int coord;
    Int axisInCoord;
    Vector<String> types;
    //
    if (world) {
      const uInt nAxes = itsCoordSys->nWorldAxes();
      types.resize(nAxes);
      for (uInt i=0; i<nAxes; i++) {
	itsCoordSys->findWorldAxis(coord, axisInCoord, i);
	if (coord>=0) {
	  types(i) = itsCoordSys->showType (coord);
	} else {
	  // This should never happen because we found the coordinate from
	  // a valid world axis
	  *itsLog << "World axis " << i
		  << " has been removed from the CoordinateSystem"
		  << LogIO::POST;
	}
      }
    } else {
      const uInt nAxes = itsCoordSys->nPixelAxes();
      types.resize(nAxes);
      for (uInt i=0; i<nAxes; i++) {
	itsCoordSys->findPixelAxis (coord, axisInCoord, i);
	if (coord>=0) {
	  types(i) = itsCoordSys->showType (coord);
	} else {
	  // This should never happen because we found the coordinate from
	  // a valid pixel axis
	  *itsLog << "Pixel axis " << i
		  << " has been removed from the CoordinateSystem"
		  << LogIO::POST;
	}
      }
    }
    //
    rstat = fromVectorString(types);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::string
coordsys::conversiontype(const std::string& cordtype)
{
  std::string rstat;
  rstat = getconversiontype(cordtype);
  return rstat;
}

std::vector<double>
coordsys::convert(const std::vector<double>& coordin,
		  const std::vector<bool>& absin,
		  const std::string& dopplerIn,
		  const std::vector<std::string>& unitsin,
		  const std::vector<bool>& absout,
		  const std::string& dopplerOut,
		  const std::vector<std::string>& unitsout,
		  const std::vector<int>& in_shape)
{
  std::vector<double> rstat;
  *itsLog << LogOrigin("coordsys", "convert");

  int n = naxes();

  Vector<Double> coordIn(coordin);
  Vector<Bool> absIn(absin);
  if (absIn.size()==1 && absIn[0]==true) {
    absIn.resize(n);
    for (int i = 0; i < n; i++) absIn[i]=true;
  }
  Vector<String> unitsIn = toVectorString(unitsin);
  if (unitsIn.size()==1 && unitsIn[0]=="Native") {
    unitsIn.resize(n);
    unitsIn = itsCoordSys->worldAxisUnits();
  }
  Vector<Bool> absOut(absout);
  if (absOut.size()==1 && absOut[0]==true) {
    absOut.resize(n);
    for (int i = 0; i < n; i++) absOut[i]=true;
  }
  Vector<String> unitsOut = toVectorString(unitsout);
  if (unitsOut.size()==1 && unitsOut[0]=="Native") {
    unitsOut.resize(n);
    unitsOut = itsCoordSys->worldAxisUnits();
  }
  Vector<Int> shape(in_shape);
  if (shape.size()==1 && shape[0]==-1) {
    shape.resize(0);
  }

  //
  MDoppler::Types dopIn, dopOut;
  if (!MDoppler::getType(dopIn, dopplerIn)) {
    *itsLog << "Illegal doppler" << LogIO::EXCEPTION;
  }
  if (!MDoppler::getType(dopOut, dopplerOut)) {
    *itsLog << "Illegal doppler" << LogIO::EXCEPTION;
  }
  //
  if (shape.nelements() == itsCoordSys->nPixelAxes()) {
    IPosition p(shape);
    itsCoordSys->setWorldMixRanges(p);
  }
  //
  Vector<Double> coordOut;
  if (!itsCoordSys->convert(coordOut, coordIn, absIn, unitsIn,
			    dopIn, absOut, unitsOut, dopOut,
			    0.0, 0.0)) {
    *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
  }
  coordOut.tovector(rstat);
  return rstat;
}

::casac::variant*
coordsys::convertmany(const ::casac::variant& coordin,
		      const std::vector<bool>& absin,
		      const std::string& dopplerIn,
		      const std::vector<std::string>& unitsin,
		      const std::vector<bool>& absout,
		      const std::string& dopplerOut,
		      const std::vector<std::string>& unitsout,
		      const std::vector<int>& in_shape)
{
  ::casac::variant *rstat = 0;
  *itsLog << LogOrigin("coordsys", "convertmany");

  int n = naxes();
  // form Array<Double> coordIn
  Vector<Int> coord_shape = coordin.arrayshape();
  std::vector<double> coord_vec = coordin.toDoubleVec();
  Array<Double> coordIn;
  coordIn.resize(IPosition(coord_shape));
  int i = 0;
  for (Array<Double>::iterator iter = coordIn.begin();
       iter != coordIn.end(); iter++) {
    *iter = coord_vec[i++];
  }

  Vector<Bool> absIn(absin);
  if (absIn.size()==1 && absIn[0]==true) {
    absIn.resize(n);
    for (int i = 0; i < n; i++) absIn[i]=true;
  }

  Vector<String> unitsIn = toVectorString(unitsin);
  if (unitsIn.size()==1 && unitsIn[0]=="Native") {
    unitsIn.resize(n);
    unitsIn = itsCoordSys->worldAxisUnits();
  }

  Vector<Bool> absOut(absout);
  if (absOut.size()==1 && absOut[0]==true) {
    absOut.resize(n);
    for (int i = 0; i < n; i++) absOut[i]=true;
  }

  Vector<String> unitsOut = toVectorString(unitsout);
  if (unitsOut.size()==1 && unitsOut[0]=="Native") {
    unitsOut.resize(n);
    unitsOut = itsCoordSys->worldAxisUnits();
  }

  Vector<Int> shape(in_shape);
  if (shape.size()==1 && shape[0]==-1) {
    shape.resize(0);
  }

  //
  MDoppler::Types dopIn, dopOut;
  if (!MDoppler::getType(dopIn, dopplerIn)) {
    *itsLog << "Illegal doppler" << LogIO::EXCEPTION;
  }
  if (!MDoppler::getType(dopOut, dopplerOut)) {
    *itsLog << "Illegal doppler" << LogIO::EXCEPTION;
  }
  //
  if (shape.nelements() == itsCoordSys->nPixelAxes()) {
    IPosition p(shape);
    itsCoordSys->setWorldMixRanges(p);
  }
  //
  AlwaysAssert(coordIn.shape().nelements()==2, AipsError);
  Matrix<Double> coordsOut;
  Matrix<Double> coordsIn(coordIn);
  if (!itsCoordSys->convert(coordsOut, coordsIn, absIn, unitsIn,
			    dopIn, absOut, unitsOut, dopOut, 0.0, 0.0)) {
    *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
  }
  Array<Double> coordOut(coordsOut.copy());

  // put Array<Double> coordsOut into ::casac::variant
  std::vector<int> out_shape;
  std::vector<double> rtnVec;
  coordsOut.shape().asVector().tovector(out_shape);
  coordsOut.tovector(rtnVec);
  rstat = new ::casac::variant(rtnVec, out_shape);

  return rstat;
}

std::vector<std::string>
coordsys::coordinatetype(const int which)
{
  std::vector<std::string> rstat;
  *itsLog << LogOrigin("coordsys", "coordinatetype");

  const Int n = itsCoordSys->nCoordinates();

  if (n==0) {
    *itsLog << "This CoordinateSystem is empty" << LogIO::EXCEPTION;
  }
  //
  Vector<String> types;
  Int which2 = which;
  if (which2<0) {
    types.resize(n);
    for (Int i=0; i<n; i++) types(i) = itsCoordSys->showType(i);
  } else {
    if (which2 < 0 || which2+1 > n) {
      ostringstream oss;
      oss << "There are only " << n << " coordinates available";
      *itsLog << String(oss) << LogIO::EXCEPTION;
    }
    types.resize(1);
    types(0) = itsCoordSys->showType(which2);
  }
  rstat = fromVectorString(types);
  return rstat;
}

::casac::coordsys *
coordsys::copy()
{
  ::casac::coordsys *rstat = 0;
  *itsLog << LogOrigin("coordsys", "copy");

  Record rec;
  if (!itsCoordSys->save(rec,"CoordinateSystem")) {
    *itsLog << "Could not convert to record because "
            << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
  }
  rec.define(RecordFieldId("parentName"), itsParentImageName);

  CoordinateSystem *pCS = CoordinateSystem::restore(rec,"CoordinateSystem");
  if (!pCS) {
    *itsLog << "Failed to create a CoordinateSystem" << LogIO::EXCEPTION;
  }
  if (rec.isDefined("parentName")) {
    itsParentImageName = rec.asString("parentName");
  }

  return rstat = new ::casac::coordsys(pCS);
}

bool
coordsys::done()
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "done");

  try{
    if (itsCoordSys != 0) delete itsCoordSys;
    //coordsys::coordsys();  // recreate default
    itsCoordSys = new CoordinateSystem();
    //itsLog = new LogIO();

    Vector<String> empty(0);
    addCoordinate(*itsCoordSys, False, False, empty, 0, False);

    // Give it a meaningful ObsInfo
    ObsInfo obsInfo;
    obsInfo.setTelescope(String("ATCA"));
    obsInfo.setObserver(String("Karl Jansky"));

    // It must be easier than this...  USe 0.0001
    // so that roundoff does not tick the 0 to 24
    Time time;
    time.now();
    MVTime time2(time);
    MVEpoch time4(time2);
    MEpoch date(time4);
    obsInfo.setObsDate(date);
    //
    itsCoordSys->setObsInfo(obsInfo);
    itsParentImageName = "unknown";

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }

  rstat = true;
  return rstat;
}

::casac::record *
coordsys::epoch()
{
  ::casac::record *rstat = 0;
  *itsLog << LogOrigin("coordsys", "epoch");
  //
  const ObsInfo& obsInfo = itsCoordSys->obsInfo();
  MEpoch epoch = obsInfo.obsDate();

  String error;
  Record outRec;
  if (!MeasureHolder(epoch).toRecord(error,outRec)) {
    *itsLog << LogIO::SEVERE << error << LogIO::POST;
  } else {
    rstat = fromRecord(outRec);
  }
  return rstat;
}

bool
coordsys::findaxis(int& coordinate, int& axisInCoordinate,
		   const bool isWorld, const int axis)
{
  *itsLog << LogOrigin("coordsys", "findaxis");

  //
  //axis--;               // THIS IS NO LONGER NEEDED ???
  if (isWorld) {
    itsCoordSys->findWorldAxis(coordinate, axisInCoordinate, axis);
  } else {
    itsCoordSys->findPixelAxis(coordinate, axisInCoordinate, axis);
  }
  if (coordinate >=0) {   // THIS IS NO LONGER NEEDED ???
    //coordinate++;
    //axisInCoordinate++;
    return True;
  }
  //
  return False;
}

bool
coordsys::findcoordinate(std::vector<int>& pixelaxes,
			 std::vector<int>& worldaxes,
			 const std::string& coordType, const int which)
{
  *itsLog << LogOrigin("coordsys", "findcoordinate");

  Vector<Int> pixelAxes;
  Vector<Int> worldAxes;
  //
  const Coordinate::Type type = stringToType(coordType);
  //
  Int which2 = which;
  if (which2<0) which2 = 0;
  Int after = -1;
  Int count = -1;
  //
  pixelAxes.resize(0);
  worldAxes.resize(0);
  while (True) {
    Int c = itsCoordSys->findCoordinate(type, after);
    if (c<0) {
      return False;
    } else {
      count++;
      if (count==which2) {
	pixelAxes = itsCoordSys->pixelAxes(c);
	worldAxes = itsCoordSys->worldAxes(c);
	pixelaxes.resize(pixelAxes.nelements());
	worldaxes.resize(worldAxes.nelements());
        for (uInt i=0; i<pixelAxes.nelements(); i++)
	  pixelaxes[i] = pixelAxes[i];
	for (uInt i=0; i<worldAxes.nelements(); i++)
	  worldaxes[i] = worldAxes[i];
	return True;
      }
    }
    after = c;
  }
}

std::vector<double>
coordsys::frequencytofrequency(const std::vector<double>& value,
			       const std::string& frqUnit,
			       const ::casac::variant& q_velocity,
			       const std::string& doppler)
{
  std::vector<double> rstat;
  *itsLog << LogOrigin("coordsys", "frequencytofrequency");

  Vector<Double> frequency(value);
  String freqUnit(frqUnit);
  if (freqUnit=="") {
    int ic = itsCoordSys->findCoordinate(Coordinate::SPECTRAL, -1);
    if (ic >= 0) {
      freqUnit = (itsCoordSys->spectralCoordinate(ic)).worldAxisUnits()[0];
    }
  }
  Quantum<Double> velocity = casaQuantity(q_velocity);

  //
  MDoppler::Types dopplerType;
  if (!MDoppler::getType(dopplerType, doppler)) {
    *itsLog << LogIO::WARN << "Illegal velocity doppler, using RADIO" << LogIO::POST;
    dopplerType = MDoppler::RADIO;
  }
  //
  MDoppler dop (velocity, dopplerType);
  Quantum<Vector<Double> > tmp(frequency, Unit(freqUnit));
  (dop.shiftFrequency(tmp).getValue()).tovector(rstat);

  return rstat;
}

std::vector<double>
coordsys::frequencytovelocity(const std::vector<double>& value,
			      const std::string& frqUnit,
			      const std::string& doppler,
			      const std::string& velUnit)
{
  std::vector<double> rstat;
  *itsLog << LogOrigin("coordsys", "frequencytovelocity");

  Vector<Double> frequency(value);
  String freqUnit(frqUnit);
  if (freqUnit=="") {
    int ic = itsCoordSys->findCoordinate(Coordinate::SPECTRAL, -1);
    if (ic >= 0) {
      freqUnit = (itsCoordSys->spectralCoordinate(ic)).worldAxisUnits()[0];
    }
  }

  //
  Int after = -1;
  Int c = itsCoordSys->findCoordinate(Coordinate::SPECTRAL, after);
  if (c < 0) {
    *itsLog << "There is no spectral coordinate in this CoordinateSystem"
	    << LogIO::EXCEPTION;
  }

  // Get SpectralCoordinate
  const SpectralCoordinate& sc0 = itsCoordSys->spectralCoordinate(c);
  SpectralCoordinate sc(sc0);
  Vector<String> units(sc.worldAxisUnits().copy());
  units(0) = freqUnit;
  if (!sc.setWorldAxisUnits(units)) {
    *itsLog << "Failed to set frequency units of " << freqUnit << " because "
	    << sc.errorMessage() << LogIO::EXCEPTION;
  }

  // Convert velocity type to enum
  MDoppler::Types dopplerType;
  if (!MDoppler::getType(dopplerType, doppler)) {
    *itsLog << LogIO::WARN << "Illegal velocity doppler, using RADIO"
	    << LogIO::POST;
    dopplerType = MDoppler::RADIO;
  }

  // Convert to velocity
  sc.setVelocity (velUnit, dopplerType);
  Vector<Double> velocity;
  if (!sc.frequencyToVelocity (velocity, frequency)) {
    *itsLog << "Conversion to velocity failed because " << sc.errorMessage()
	    << endl;
  }
  velocity.tovector(rstat);
  return rstat;
}

bool
coordsys::fromrecord(const ::casac::record& csys_record)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("coordsys", "fromrecord");

    //    for(::casac::rec_map::const_iterator rec_it = csys_record.begin();
    //	rec_it != csys_record.end(); rec_it++){
    //      std::cerr << (*rec_it).first <<  " " << std::endl;
    //    }
    Record *csysRecord = toRecord(csys_record);
    CoordinateSystem* pCS = CoordinateSystem::restore(*csysRecord, "");
    if (pCS==0) {
      *itsLog << "Failed to create a CoordinateSystem from this record"
	      << LogIO::EXCEPTION;
    }
    //
    if(itsCoordSys){
	    delete itsCoordSys;
    }
    itsCoordSys=new CoordinateSystem(*pCS); // memory leak here??
    delete pCS;
    //
    if (csysRecord->isDefined("parentName")) {
      itsParentImageName = csysRecord->asString("parentName");
    }
    delete csysRecord;
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "exceptions reported: " << x.getMesg()
	    << LogIO::POST;
  }
  return rstat;
}

::casac::record*
coordsys::increment(const std::string& format, const std::string& coordtype)
{
  ::casac::record* rstat = 0;
  *itsLog << LogOrigin("coordsys", "increment");

  try {

    String type(coordtype);
    Vector<Double> incr;
    Int c = -1;
    if (type.empty()) {
      incr = itsCoordSys->increment();
    } else {
      Coordinate::Type cType = stringToType(type);
      Int after = -1;
      c = itsCoordSys->findCoordinate(cType, after);
      if (c<0) {
	*itsLog << LogIO::SEVERE << "A coordinate of type " << coordtype
		<< " does not exist"	    << LogIO::EXCEPTION;
      }
      incr = (itsCoordSys->coordinate(c)).increment();
    }
    //
    Bool isAbsolute = False;
    Bool showAsAbsolute = False;
    Record rec = worldVectorToRecord (incr, c, format, isAbsolute,
				      showAsAbsolute);
    rec.define("pw_type", "world");
    rec.define("ar_type", "absolute");
    //
    rstat = fromRecord(rec);
  } catch (AipsError x) {
    *itsLog << LogOrigin("coordsys", "increment");
    *itsLog << LogIO::SEVERE << "exceptions reported: "
	    << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

::casac::variant*
coordsys::lineartransform(const std::string& cordtype)
{
  ::casac::variant *rstat = 0;
  *itsLog << LogOrigin("coordsys", "lineartransform");

  String type(cordtype);
  Array<Double> ltarray;
  //
  if (type.empty()) {
    *itsLog << "You must specify the coordinate type" << LogIO::EXCEPTION;
  }
  //
  Coordinate::Type cType = stringToType(type);
  Int after = -1;
  Int c = itsCoordSys->findCoordinate(cType, after);
  ltarray = itsCoordSys->coordinate(c).linearTransform();

  std::vector<int> v_shape;
  std::vector<double> v_ltarray;
  ltarray.shape().asVector().tovector(v_shape);
  ltarray.tovector(v_ltarray);
  rstat = new ::casac::variant(v_ltarray, v_shape);
  return rstat;
}

std::vector<std::string>
coordsys::names(const std::string& coordtype)
{
  std::vector<std::string> rstat;
  *itsLog << LogOrigin("coordsys", "names");

  Vector<String> out = itsCoordSys->worldAxisNames();
  if (coordtype=="") {
    rstat = fromVectorString(out);
  } else {
    const Coordinate::Type type = stringToType(coordtype);
    Int c = itsCoordSys->findCoordinate(type, -1);
    if (c < 0) {
      *itsLog << LogIO::WARN << "A coordinate of type "
	      << type << " does not exists" << LogIO::POST;
    } else {
      Vector<Int> worldAxes = itsCoordSys->worldAxes(c);
      rstat.resize(1);
      rstat[0] = out[worldAxes[0]];
    }
  }
  return rstat;
}

int
coordsys::naxes(const bool world)
{
  *itsLog << LogOrigin("coordsys", "naxes");

  if (world) {
    return itsCoordSys->nWorldAxes();
  } else {
    return itsCoordSys->nPixelAxes();
  }
}

int
coordsys::ncoordinates()
{
  int rstat(0);
  *itsLog << LogOrigin("coordsys", "ncoordinates");
  rstat = itsCoordSys->nCoordinates();
  return rstat;
}

std::string
coordsys::observer()
{
  std::string rstat;
  *itsLog << LogOrigin("coordsys", "observer");

  const ObsInfo& obsInfo = itsCoordSys->obsInfo();
  rstat = obsInfo.observer();

  return rstat;
}

::casac::record*
coordsys::projection(const std::string& cordtype)
{
  ::casac::record *rstat = 0;
  *itsLog << LogOrigin("coordsys", "projection");

  String name(cordtype);

  // Exception if type not found
  Int c = findCoordinate (Coordinate::DIRECTION, True);
  //
  Record rec;
  const DirectionCoordinate& dc = itsCoordSys->directionCoordinate(c);
  const Projection proj = dc.projection();
  //
  if (name.empty()) {
    // Return actual projection
    rec.define("type", proj.name());
    rec.define("parameters", proj.parameters());
  } else {
    // Return number of parameters needed for given projection
    String name2 = upcase(name);
    String name3(name2.at(0,3));
    //
    // Return all types
    if (name3==String("ALL")) {
      const Int nProj = Projection::N_PROJ;
      Vector<String> types(nProj);
      for (Int i=0; i<nProj; i++) {
	Projection::Type type = static_cast<Projection::Type>(i);
	types(i) = Projection::name(type);
      }
      rec.define("types", types);
      //rec.define("all", True);
    } else {
      Projection::Type type = Projection::type(name3);
      // Throws exception for unknown type
      const Int nP = Projection::nParameters(type);
      rec.define("nparameters", nP);
    }
  }
  //
  rstat = fromRecord(rec);
  return rstat;
}

std::vector<std::string>
coordsys::referencecode(const std::string& cordtype, const bool list)
{
  std::vector<std::string> rstat;
  *itsLog << LogOrigin("coordsys", "referencecode");

  String coordinateType(cordtype);

  if ((coordinateType != "") && (list==true)) {
    coordinateType.upcase();
    if (coordinateType.matches("DI",-2)) {
      MDirection md(MDirection::J2000);
      Int nall, nex;
      const uInt *typ;
      const String *tall = md.allTypes(nall, nex, typ);
      //      Vector<String> tcod(nall-nex);
      //      Vector<String> text(nex);
      rstat.resize(nall);
      for (Int i=0; i<nall; i++) {
	rstat[i]=tall[i];
	//if (i<nall-nex) tcod(i) = tall[i];
	//else text(i-nall+nex) = tall[i];
      };
      //return fromVectorString(tcod);
      return rstat;
    } else if (coordinateType.matches("SP",-2)) {
      MFrequency mf(casa::Quantity(1.0,"Hz"), MFrequency::LSRK);
      Int nall, nex;
      const uInt *typ;
      const String *tall = mf.allTypes(nall, nex, typ);
      //      Vector<String> tcod(nall-nex);
      //      Vector<String> text(nex);
      rstat.resize(nall);
      for (Int i=0; i<nall; i++) {
	rstat[i]=tall[i];
	//if (i<nall-nex) tcod(i) = tall[i];
	//else text(i-nall+nex) = tall[i];
      };
      //return fromVectorString(tcod);
      return rstat;
    } else {
      return rstat;
    }
  }

  const uInt nCoords = itsCoordSys->nCoordinates();
  Vector<String> codes;
  Int iStart, iEnd;
  if (coordinateType.empty()) {
    iStart = 0;
    iEnd  = nCoords-1;
  } else {
    const Coordinate::Type type = stringToType(coordinateType);
    // Exception if type not found
    if (type==Coordinate::DIRECTION) {
      iStart = findCoordinate (Coordinate::DIRECTION, True);
    } else if (type==Coordinate::SPECTRAL) {
      iStart = findCoordinate (Coordinate::SPECTRAL, True);
    } else {
      iStart = -1;
    }
    iEnd = iStart;
  }
  //
  if (iStart==-1) {
    codes.resize(1);
    codes(0) = String("");
  } else {
    codes.resize(iEnd-iStart+1);
    for (Int i=iStart,j=0; i<iEnd+1; i++,j++) {
      Coordinate::Type type = itsCoordSys->type(i);
      if (type==Coordinate::DIRECTION) {
	const DirectionCoordinate& dc = itsCoordSys->directionCoordinate(i);
	MDirection::Types dt = dc.directionType();
	codes(j) = MDirection::showType (dt);
      } else if (type==Coordinate::SPECTRAL) {
	const SpectralCoordinate& sc = itsCoordSys->spectralCoordinate(i);
	MFrequency::Types ft = sc.frequencySystem();
	codes(j) = MFrequency::showType (ft);
      } else {
	codes(j) = String("");
      }
    }
  }
  rstat = fromVectorString(codes);

  return rstat;
}

::casac::record*
coordsys::referencepixel(const std::string& cordtype)
{
  ::casac::record* rstat = 0;

  try {
    Vector<Double> crpix = itsCoordSys->referencePixel();
    Vector<Double> numeric;
    if (cordtype == "") {
      numeric = crpix;
    } else {
      const Coordinate::Type type = stringToType(String(cordtype));
      Int c = itsCoordSys->findCoordinate(type, -1);
      if (c<0) {
        *itsLog << LogIO::WARN
                << "A coordinate of type " << cordtype << " does not exist"
                << LogIO::POST;
        return rstat;
      } else {
        Vector<Int> pixelAxes = itsCoordSys->pixelAxes(c);
        numeric.resize(pixelAxes.nelements());
        for (uInt i=0; i<pixelAxes.nelements(); i++)
          numeric[i] = crpix(pixelAxes[i]);
      }
    }
    Record rec;
    rec.define("numeric",numeric);
    rec.define("pw_type","pixel");
    rec.define("ar_type","absolute");
    rstat = fromRecord(rec);

  } catch (AipsError x) {
     *itsLog << LogOrigin("coordsys", "referencepixel");
     *itsLog << LogIO::SEVERE << "exceptions reported: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

::casac::record*
coordsys::referencevalue(const std::string& format,
			 const std::string& type)
{
  ::casac::record* rstat = 0;
  *itsLog << LogOrigin("coordsys", "referencevalue");

  //
  Vector<Double> refVal;
  Int c = -1;
  if (type.empty()) {
    refVal = itsCoordSys->referenceValue();
  } else {
    Coordinate::Type cType = stringToType(type);
    Int after = -1;
    c = itsCoordSys->findCoordinate(cType, after);
    refVal = itsCoordSys->coordinate(c).referenceValue();
  }
  //
  Bool isAbsolute = True;
  Bool showAsAbsolute = True;
  Record rec =
    worldVectorToRecord (refVal, c, format, isAbsolute, showAsAbsolute);
  //
  rec.define("pw_type", "world");
  rec.define("ar_type", "absolute");
  rstat = fromRecord(rec);
  return rstat;
}

bool
coordsys::reorder(const std::vector<int>& order)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "reorder");

  //
  // This is pretty dody - if the axes have been reordered this
  // is all rubbish
  //
  Vector<Int> order2(order);
  //
  const uInt nCoord = itsCoordSys->nCoordinates();
  if (order2.nelements() != nCoord) {
    *itsLog << "order vector must be of length " << nCoord << LogIO::EXCEPTION;
  }
  //
  CoordinateSystem cSys;
  cSys.setObsInfo(itsCoordSys->obsInfo());
  for (uInt i=0; i<nCoord; i++) {
    cSys.addCoordinate(itsCoordSys->coordinate(order2(i)));
  }
  *itsCoordSys = cSys;
  rstat = true;
  return rstat;
}

bool
coordsys::replace(const ::casac::record& csys, const int in,
		  const int out)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "replace");

  Record *tmp = toRecord(csys);

  //
  CoordinateSystem* pCS = CoordinateSystem::restore(*tmp, "");
  if (!pCS) {
    *itsLog << "The supplied CoordinateSYstem is illegal" << LogIO::EXCEPTION;
  }
  //
  Int inIdx = in;
  if (inIdx<Int(0) || inIdx>Int(pCS->nCoordinates()-1)) {
    *itsLog << "Illegal index " << in << " for input coordinate" << LogIO::EXCEPTION;
  }
  Int outIdx = out;
  if (outIdx<Int(0) || outIdx>Int(itsCoordSys->nCoordinates()-1)) {
    *itsLog << "Illegal index " << out << " for output coordinate" << LogIO::EXCEPTION;
  }

  // We could implement this case by building a new CS from scratch, but any
  // axis reordering would be lost (unlikely to be common)
  if (pCS->coordinate(inIdx).nWorldAxes() !=
      itsCoordSys->coordinate(outIdx).nWorldAxes()) {
    *itsLog << "Coordinates must have the same number of axes"
	    << LogIO::EXCEPTION;
  }
  //
  const Coordinate& newCoord = pCS->coordinate (inIdx);
  Bool ok = itsCoordSys->replaceCoordinate (newCoord, outIdx);
  if (!ok) {
    *itsLog << LogIO::WARN << "Replacement incurred warning" << LogIO::POST;
    ok = True;
  }
  rstat = ok;
  delete tmp;
  delete pCS;
  pCS = 0;

  return rstat;
}

::casac::record *
coordsys::restfrequency()
{
  ::casac::record * rstat=0;

  *itsLog << LogOrigin("coordsys", "restfrequency");

  // Exception if type not found
  Int c = findCoordinate (Coordinate::SPECTRAL, True);
  //
  const SpectralCoordinate& sc = itsCoordSys->spectralCoordinate(c);
  //
  const Vector<Double> rfs = sc.restFrequencies();
  Double rf = sc.restFrequency();
  Vector<Double> rfs2(rfs.nelements());
  rfs2(0) = rf;
  uInt j = 1;
  for (uInt i=0; i<rfs.nelements(); i++) {
    if (!::near(rfs(i), rf)) {
      rfs2(j) = rfs(i);
      j++;
    }
  }
  //
  Quantum<Vector<Double> > q(rfs2, sc.worldAxisUnits()(0));
  String error;
  casa::Record R;
  if (QuantumHolder(q).toRecord(error, R)) {
    rstat = fromRecord(R);
  } else {
    *itsLog << LogIO::SEVERE << "Could not convert quantity to record."
	    << error << LogIO::POST;
  }
  return rstat;
}

bool
coordsys::setconversiontype(const std::string& direction,
			    const std::string& spectral)
{
  *itsLog << LogOrigin("coordsys", "setconversiontype");

  //
  String errorMsg;
  if (!direction.empty()) {
    if (!CoordinateUtil::setDirectionConversion (errorMsg,
						 *itsCoordSys, direction)) {
      *itsLog
	<< "Failed to set the new DirectionCoordinate reference frame because "
	<< errorMsg << LogIO::EXCEPTION;
    }
  }
  //
  if (!spectral.empty()) {
    if (!CoordinateUtil::setSpectralConversion (errorMsg,
						*itsCoordSys, spectral)) {
      *itsLog
	<< "Failed to set the new SpectralCoordinate reference frame because "
	<< errorMsg << LogIO::EXCEPTION;
    }
  }
  //
  return true;
}

std::string
coordsys::getconversiontype(const std::string& type)
{
  *itsLog << LogOrigin("coordsys", "getconversiontype");

  // 
  Coordinate::Type cType = stringToType(type);
  if (cType==Coordinate::DIRECTION) {
    Int after = -1;
    Int c = itsCoordSys->findCoordinate(Coordinate::DIRECTION, after);
    if (c >= 0) {
      const DirectionCoordinate& dCoord = itsCoordSys->directionCoordinate(c);
      MDirection::Types type;
      dCoord.getReferenceConversion(type);
      return MDirection::showType(type);
    }
  } else if (cType==Coordinate::SPECTRAL) {
    Int after = -1;
    Int c = itsCoordSys->findCoordinate(Coordinate::SPECTRAL, after);
    if (c >= 0) {
      const SpectralCoordinate& sCoord = itsCoordSys->spectralCoordinate(c);
      MFrequency::Types type;
      MEpoch epoch;
      MDirection direction;
      MPosition position;
      sCoord.getReferenceConversion(type, epoch, position, direction);
      return MFrequency::showType(type);
    }
  }
  //
  return String("");
}

bool
coordsys::setdirection(const std::string& in_ref,
		       const std::string& in_projName,
		       const std::vector<double>& projpar,
		       const std::vector<double>& refpix,
		       const ::casac::variant& iv_refval,
		       const ::casac::variant& iv_incr,
		       const ::casac::variant& iv_xform,
		       const ::casac::variant& iv_poles)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setdirection");

  //
  // Bail out if coordinate not found
  Int ic = findCoordinate (Coordinate::DIRECTION, True);
  if (ic < 0) {
    *itsLog << LogIO::WARN << "Cannot generate default values"
	    << LogIO::POST;
    return false;
  }

  const DirectionCoordinate oldDC = itsCoordSys->directionCoordinate(ic);
  const Vector<String>& oldUnits = oldDC.worldAxisUnits();

  String ref(in_ref);
  if (ref=="") {
    MDirection::Types dt = oldDC.directionType();
    ref = MDirection::showType (dt);
  }

  String projName(in_projName);
  if (projName=="") {
    const Projection proj = oldDC.projection();
    projName = proj.name();
  }

  Vector<Double> projPar(projpar);
  if (projPar.size()==1 && projPar[0]==-1) {
    const Projection proj = oldDC.projection();
    Vector<Double> tmp = proj.parameters();
    projPar.resize(tmp.size());
    for (uInt i=0; i< tmp.size(); i++) {
      projPar[i] = tmp[i];
    }
  }

  Vector<Double> refPix(refpix);
  if (refPix.size()==1 && refPix[0]==-1) {
    Vector<Int> pixelAxes = itsCoordSys->pixelAxes(ic);
    refPix.resize(pixelAxes.size());
    for (uInt i=0; i<pixelAxes.size(); i++) {
      refPix[i] = pixelAxes[i];
    }
  }

  ::casac::variant *pv_refval = 0;
  if (unset(iv_refval)) {
    ::casac::record r_refval = *referencevalue("q","direction");
    pv_refval = new ::casac::variant(r_refval);
  } else {
    pv_refval = new ::casac::variant(iv_refval);
  }
  Bool isWorld(True);  Bool isAbs(True);  Bool first(True);
  Record *refvalRec = coordinateValueToRecord(*pv_refval, isWorld, isAbs, first);
  delete pv_refval;

  ::casac::variant *pv_incr = 0;
  if (unset(iv_incr)) {
    ::casac::record r_incr = *increment("q","direction");
    pv_incr = new ::casac::variant(r_incr);
  } else {
    pv_incr = new ::casac::variant(iv_incr);
  }
  Record *incrRec = coordinateValueToRecord(*pv_incr, isWorld, isAbs, first);
  delete pv_incr;

  Vector<casa::Quantity> poleQ(2);
  if (unset(iv_poles)) {
    // Seems to allow the right increment with 180 and 0
    poleQ(0)=casa::Quantity(180.0, "deg");
    poleQ(1)=casa::Quantity(0.0,"deg");
    
  } else {
    if(!toCasaVectorQuantity(iv_poles, poleQ) || (poleQ.nelements() !=2)){
      throw(AipsError("pole parameter is not understood"));
      
    }
    
  }
  //Record *polesRec = coordinateValueToRecord(*pv_poles, isWorld, isAbs, first);

  Matrix<Double> xform(2,2);
  if (unset(iv_xform)) {
    xform(0,0) = 1.0; xform(0,1) = 0.0;
    xform(1,0) = 0.0; xform(1,1) = 1.0;
  } else {
    if (iv_xform.type() == ::casac::variant::DOUBLEVEC) {
      Vector<Int> shape = iv_xform.arrayshape();
      std::vector<double> xformVec = iv_xform.getDoubleVec();
      xform.resize(IPosition(shape));
      int i = 0;
      for (Array<Double>::iterator iter = xform.begin();
	   iter!=xform.end(); iter++) {
	*iter = xformVec[i++];
      }
    } else {
      *itsLog << LogIO::WARN << "xform paramater invalid"
	      << LogIO::POST;
      return False;
    }
  }

  // Reference Code
  String ref2 = ref;
  ref2.upcase();
  MDirection::Types refType;
  if (!MDirection::getType(refType, ref2)) {
    *itsLog << "Invalid direction code '" << ref
       << "' given. Allowed are : " << endl;
    for (uInt i=0; i<MDirection::N_Types; i++)
      *itsLog << "  " << MDirection::showType(i) << endl;
    *itsLog << LogIO::EXCEPTION;
  }

  // Projection
  Projection proj(Projection::type(projName), projPar);

  // Reference Value.  Value comes back from recordToWorld in native
  // units of itsCSys for this Coordinate.
  String dummyType;
  Vector<Double> refval;
  //  Record *refvalRec = toRecord(r_refval);
  recordToWorldVector(refval, dummyType, ic, *refvalRec);
  trim(refval, oldDC.referenceValue());

  // Increment
  String dummyType2;
  Vector<Double> incr;
  //  Record *incrRec = toRecord(r_incr);
  recordToWorldVector(incr, dummyType2, ic, *incrRec);
  trim(incr, oldDC.increment());

  // Poles
  /* String dummyType3;
  Vector<Double> poles;
  //  Record *polesRec = toRecord(r_poles);
  recordToWorldVector(poles, dummyType3, ic, *polesRec);
  Vector<Double> xx(2);
  xx = 999.0;
  trim(poles, xx);
  */

  //

  Matrix<Double> xform2(xform);
  DirectionCoordinate newDC(refType, proj,
			    Quantum<Double>(refval[0], oldUnits[0]),
			    Quantum<Double>(refval[1], oldUnits[1]),
			    Quantum<Double>(incr[0], oldUnits[0]),
			    Quantum<Double>(incr[1], oldUnits[1]),
			    //xform2, refPix[0]-1.0, refPix[1]-1.0,
			    xform2, refPix[0], refPix[1],
			    poleQ(0), poleQ(1));
			    // Quantum<Double>(poles[0], oldUnits[0]),
			    //Quantum<Double>(poles[1], oldUnits[1]));
  //
  itsCoordSys->replaceCoordinate(newDC, ic);
  delete refvalRec;
  delete incrRec;
  //delete polesRec;
  rstat = true;
  return rstat;
}

bool
coordsys::setepoch(const ::casac::record& value)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setepoch");

  String error;
  MeasureHolder in;
  Record *inrec = toRecord(value);
  if (!in.fromRecord(error, *inrec)) {
    error += String("Non-measure type value in measure conversion\n");
    *itsLog << LogIO::SEVERE << error << LogIO::POST;
    return rstat;
  }
  if (!in.isMeasure()) {
    *itsLog << LogIO::SEVERE << "value is not a measure" << LogIO::POST;
    return rstat;
  }
  if (in.isMEpoch()) {
    MEpoch epoch(in.asMEpoch());
    //
    ObsInfo obsInfo = itsCoordSys->obsInfo();
    obsInfo.setObsDate(epoch);
    itsCoordSys->setObsInfo(obsInfo);
    rstat = true;
  } else {
    *itsLog << LogIO::SEVERE
	    << "Record value does not contain an Epoch measure."
	    << LogIO::POST;
  }
  return rstat;
}


void coordsys::setcoordsys(casa::CoordinateSystem &acsys) {*itsCoordSys = acsys;}

bool
coordsys::setincrement(const ::casac::variant& value,
		       const std::string& coordinateType)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setincrement");
  /*
 // Kumar's version
 casa::Vector<casa::Quantity> values;
 toCasaVectorQuantity(value, values);
 String dummyType;
 Int c;
 Vector<Double> world;
 Record *rec = 0;
 //Keeping the record compatibility with the output of increment..
 // Can go away when increment return quantas/measures or vector  of them
 Bool inputIsRec=False;
 if(value.type()== ::casac::variant::RECORD){
   inputIsRec=True;
   ::casac::variant localVar(value); //value  is const
   rec=toRecord(localVar.asRecord());
 }
 else{
////This is the only extra piece of code needed
   Vector<String> wunits = itsCoordSys->worldAxisUnits();
   world.resize(values.nelements());
   for (uInt k=0; k < world.nelements(); ++k){
     world[k]=values[k].getValue(wunits[k]);
   }
 }
 //
 if (coordinateType.empty()) {
   //Record compatibility====
   if(inputIsRec){
     c = -1;
     recordToWorldVector(world, dummyType, c, *rec);
   }
   //============
   trim(world, itsCoordSys->increment());
   if (!itsCoordSys->setIncrement(world)) {
     *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
   } else {
     rstat = true;
   }
 } else {
   const Coordinate::Type type = stringToType (coordinateType);
   Int c = findCoordinate (type, True);
   //Record compatibility
   if(inputIsRec){
     recordToWorldVector(world, dummyType, c, *rec);
   }
   //=============
   trim(world, itsCoordSys->coordinate(c).referenceValue());
   //
   Vector<Double> incAll(itsCoordSys->increment().copy());
   copyWorldAxes(incAll, world, c);
   //
   if (!itsCoordSys->setIncrement(incAll)) {
     *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
   } else {
     rstat = true;
   }
 }
 if(inputIsRec)
   delete rec;
 return rstat;
  End of Kumar's version */

  //
  Record *rec = 0;

  // Check that value is in world coordinates
  Bool isWorld;
  Bool shouldBeWorld(True);
  Bool verbose(False);
  ::casac::variant tmpv(value);
  // isWorld := its.isValueWorld (value, shouldBeWorld=T, verbose=F);
  int rtn = isValueWorld(tmpv, shouldBeWorld, verbose);
  // if (is_fail(isWorld)) fail;
  if (rtn == -1) {
    *itsLog << LogIO::SEVERE
	    << "Should be world value in coordsys.isValueWorld" << LogIO::POST;
    return rstat;
  } else {
    isWorld = (Bool)rtn;
  }

  // Check that value is in absolute coordinates
  Bool shouldBeAbs(True);
  // if (is_fail(its.checkAbsRel (value=value, shouldBeAbs=T))) fail;
  Bool rtn2 = checkAbsRel(tmpv, shouldBeAbs);
  if (!rtn2) {
    *itsLog << LogIO::SEVERE
	    << "Should be abs value in coordsys.checkAbsRel" << LogIO::POST;
    return rstat;
  }

  // Convert value to a Record
  Bool isAbs(True);
  Bool first(True);
  // its.setincrementRec.value :=
  //   its.coordinateValueToRecord (value=value, isWorld=isWorld,
  //                                isAbs=T, first=T);
  rec = coordinateValueToRecord(value, isWorld, isAbs, first);
  // if (is_fail(its.setincrementRec.value)) fail;
  if (!rec) {
    *itsLog << LogIO::SEVERE
	    << "Conversion of value to Record failed in coordinateValueToRecord"
	    << LogIO::POST;
    return rstat;
  }

  // return defaultservers.run(its.agent, its.setincrementRec, F);
  String dummyType;
  Int c;
  Vector<Double> world;
  //
  if (coordinateType.empty()) {
    c = -1;
    recordToWorldVector(world, dummyType, c, *rec);
    trim(world, itsCoordSys->increment());
    if (!itsCoordSys->setIncrement(world)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    } else {
      rstat = true;
    }
  } else {
    const Coordinate::Type type = stringToType (coordinateType);
    Int c = findCoordinate (type, True);
    recordToWorldVector(world, dummyType, c, *rec);
    trim(world, itsCoordSys->coordinate(c).referenceValue());
    //
    Vector<Double> incAll(itsCoordSys->increment().copy());
    copyWorldAxes(incAll, world, c);
    //
    if (!itsCoordSys->setIncrement(incAll)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    } else {
      rstat = true;
    }
  }
  delete rec;
  return rstat;


}

bool
coordsys::setlineartransform(const std::string& coordinateType,
			     const ::casac::variant& v_value)
{
  *itsLog << LogOrigin("coordsys", "setlineartransform");

  //
  if (coordinateType.empty()) {
    *itsLog << "You must specify the coordinate type" << LogIO::EXCEPTION;
  }

  Vector<Int> shape = v_value.arrayshape();
  if (shape.size() == 0) {
    *itsLog << "The value array is empty" << LogIO::EXCEPTION;
  }

  // Get the data into a CASA array of double values.
  Array<Double> value;
  value.resize(IPosition(shape));

  if ( v_value.type() == ::casac::variant::DOUBLEVEC ) {
      
      std::vector<double> valueVector;
      valueVector = v_value.getDoubleVec();
      int i = 0;
      for (Array<Double>::iterator iter = value.begin();
	   iter!=value.end(); iter++) {
	  *iter = valueVector[i++];
      }
  } else if ( v_value.type() == ::casac::variant::INTVEC ) {
      std::vector<int> valueVector;
      valueVector = v_value.getIntVec();
      int i = 0;
      for (Array<Double>::iterator iter = value.begin();
	   iter!=value.end(); iter++) {
	  *iter = (Double)valueVector[i++];
      }
  } else
      *itsLog << "The value array is not a double or integer array" << LogIO::EXCEPTION;

  //
  const Coordinate::Type type = stringToType (coordinateType);
  Int c = findCoordinate (type, True);
  if (type==Coordinate::LINEAR) {
    LinearCoordinate lc = itsCoordSys->linearCoordinate(c);
    lc.setLinearTransform(value);
    itsCoordSys->replaceCoordinate(lc, c);
  } else if (type==Coordinate::DIRECTION) {
    DirectionCoordinate lc = itsCoordSys->directionCoordinate(c);
    lc.setLinearTransform(value);
    itsCoordSys->replaceCoordinate(lc, c);
  } else if (type==Coordinate::SPECTRAL) {
    SpectralCoordinate lc = itsCoordSys->spectralCoordinate(c);
    lc.setLinearTransform(value);
    itsCoordSys->replaceCoordinate(lc, c);
  } else if (type==Coordinate::STOKES) {
    StokesCoordinate lc = itsCoordSys->stokesCoordinate(c);
    lc.setLinearTransform(value);
    itsCoordSys->replaceCoordinate(lc, c);
  } else if (type==Coordinate::TABULAR) {
    TabularCoordinate lc = itsCoordSys->tabularCoordinate(c);
    lc.setLinearTransform(value);
    itsCoordSys->replaceCoordinate(lc, c);
  } else {
    *itsLog << "Coordinate type not yet handled " << LogIO::EXCEPTION;
    }

  return true;
}

bool
coordsys::setnames(const std::vector<std::string>& value,
		   const std::string& coordinateType)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setnames");

  Vector<String> vnames = toVectorString(value);
  Vector<String> names;
  if (vnames.size()==1) {
    sepCommaEmptyToVectorStrings(names, vnames[0]);
  } else {
    names = vnames;
  }

  //
  if (coordinateType.empty()) {
    if (!itsCoordSys->setWorldAxisNames(names)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    }
  } else {
    const Coordinate::Type type = stringToType (coordinateType);
    Int c = findCoordinate (type, True);
    Vector<Int> worldAxes = itsCoordSys->worldAxes(c);
    if (names.nelements() != worldAxes.nelements()) {
      *itsLog << "Supplied axis names vector must be of length "
	      << worldAxes.nelements() << LogIO::EXCEPTION;
    }
    //
    Vector<String> namesAll(itsCoordSys->worldAxisNames().copy());
    for (uInt i=0; i<worldAxes.nelements(); i++) {
      namesAll(worldAxes(i)) = names(i);
    }
    //
    if (!itsCoordSys->setWorldAxisNames(namesAll)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    }
  }
  rstat = true;

  return rstat;
}

bool
coordsys::setobserver(const std::string& observer)
{
  *itsLog << LogOrigin("coordsys", "setobserver");
  ObsInfo obsInfo = itsCoordSys->obsInfo();
  obsInfo.setObserver(observer);
  itsCoordSys->setObsInfo(obsInfo);
  return true;
}

bool
coordsys::setprojection(const std::string& name,
			const std::vector<double>& in_parameters)
{
  *itsLog << LogOrigin("coordsys", "setprojection");

  Vector<Double> parameters;
  if ( !(in_parameters.size()==1 && in_parameters[0]==-1) ) {
    parameters = in_parameters;
  }

  // Exception if type not found
  Int ic = findCoordinate (Coordinate::DIRECTION, True);
  //
  DirectionCoordinate
    dirCoordFrom(itsCoordSys->directionCoordinate(ic));             // Copy
  Vector<String> unitsFrom = dirCoordFrom.worldAxisUnits().copy();

  // Set radian units so we can copy constructor parameters over
  Vector<String> radUnits(2);
  radUnits = String("rad");
  if (!dirCoordFrom.setWorldAxisUnits(radUnits)) {
    *itsLog << "Failed to set radian units for DirectionCoordinate"
	    << LogIO::EXCEPTION;
  }

  // Create output DirectionCoordinate
  Projection::Type type = Projection::type(name);
  Projection projTo(type, parameters);
  //
  Vector<Double> refValFrom = dirCoordFrom.referenceValue();
  Vector<Double> refPixFrom = dirCoordFrom.referencePixel();
  Vector<Double> incrFrom = dirCoordFrom.increment();
  DirectionCoordinate dirCoordTo (dirCoordFrom.directionType(), projTo,
				  refValFrom(0), refValFrom(1),
				  incrFrom(0), incrFrom(1),
				  dirCoordFrom.linearTransform(),
				  refPixFrom(0), refPixFrom(1));

  // Set original units
  if (!dirCoordTo.setWorldAxisUnits(unitsFrom)) {
    *itsLog << dirCoordTo.errorMessage() << LogIO::EXCEPTION;
  }

  // Replace in Coordinate System
  itsCoordSys->replaceCoordinate(dirCoordTo, ic);
  return true;
}

bool
coordsys::setreferencecode(const std::string& code,
			   const std::string& coordinateType,
			   const bool adjust)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setreferencecode");

  const Coordinate::Type type = stringToType (coordinateType);
  //
  if (type==Coordinate::DIRECTION) {
    setDirectionCode(code, adjust);
    rstat = true;
  } else if (type==Coordinate::SPECTRAL) {
    setSpectralCode(code, adjust);
    rstat = true;
  } else {
    *itsLog << "Coordinate type must be 'Direction' or 'Spectral'"
	    << LogIO::EXCEPTION;
  }
  return rstat;
}

bool
coordsys::setreferencelocation(const std::vector<int>& pixel,
			       const ::casac::variant& world,
			       const std::vector<bool>& imask)
{

  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setreferencelocation");

  //Checks  
  std::vector<double> dpixel;
  if (unset(pixel)) {
    Vector<Double> rp = itsCoordSys->referencePixel();
    rp.tovector(dpixel);
  } else {
    int n = pixel.size();
    dpixel.resize(n);
    for (int i=0; i < n; i++) dpixel[i] =  pixel[i];
  }
  uInt nPixelAxes = naxes(False);
  if (dpixel.size() != nPixelAxes) {
    *itsLog << LogIO::WARN << "Parameter pixel must be of length "
	    << nPixelAxes << LogIO::POST;
    return rstat;
  }
  std::vector<bool> mask(imask);
  if (unset(mask)) {
    mask.resize(nPixelAxes);
    for (uInt i=0; i < nPixelAxes; i++) mask[i]=True;
  }
  if (mask.size() != pixel.size()) {
    *itsLog << LogIO::WARN << "shape and mask must be the same length"
	    << LogIO::POST;
    return false;
  }
    
  // Convert world to numeric world format, adding/trimming
  // missing/extra axes in the process
  std::vector<double> p = toPixel(world);
  if (p.size() != pixel.size()) {
    *itsLog << LogIO::WARN << "pixel and world must be the same length"
	    << LogIO::POST;
    return false;
  }
  ::casac::record *w = toworld(p, "n");
  if (!w) {
    *itsLog << LogIO::WARN << "failed to create numeric world record"
	    << LogIO::POST;
    return false;
  }

  // Eliminate Stokes and masked values
  std::vector<int> p2w = axesmap(true);
  std::vector<double> rp;
  (itsCoordSys->referencePixel()).tovector(rp);
  if (rp.size() != nPixelAxes) {
    *itsLog << LogIO::WARN << "Failed to get referencePixel"
	    << LogIO::POST;
    return false;
  }
  ::casac::record* rv = referencevalue("n");
  if (!rv) {
    *itsLog << LogIO::WARN << "failed to get referencevalue"
	    << LogIO::POST;
    return false;
  }
  std::vector<std::string> types = axiscoordinatetypes(false);
  if (types.size()==0) {
    *itsLog << LogIO::WARN << "failed to get axiscoordinatetypes"
	    << LogIO::POST;
    return false;
  }
  std::vector<double> dp(dpixel);
  for (uInt i=0 ; i < nPixelAxes; i++) {
    if (mask[i] && types[i]=="Stokes") {
      *itsLog << LogIO::WARN
	      << "Cannot change Stokes reference pixel, setting mask to F"
	      << LogIO::POST;
      mask[i]=false;
    }
    if (!mask[i]) {
      dp[i] = rp[i];
      w[p2w[i]] = rv[p2w[i]];
    }
  }

  // Set new values
  if (!setreferencepixel(dp, "")) {
    *itsLog << LogIO::WARN << "failed to set referencepixel"
	    << LogIO::POST;
    return false;
  }
  if (!setreferencevalue(w, "")) {
    *itsLog << LogIO::WARN << "failed to set referencevalue"
	    << LogIO::POST;
    return false;
  }
  rstat = true;
  return rstat;
}

bool
coordsys::setreferencepixel(const std::vector<double>& refpix,
			    const std::string& coordinateType)
{
  bool rstat(false);

  *itsLog << LogOrigin("coordsys", "setreferencepixel");

  //
  Vector<Double> refPix2(refpix);
  //   refPix2 = refPix - 1.0;                     // 0-rel
  //
  if (coordinateType.empty()) {
    trim(refPix2, itsCoordSys->referencePixel());
    if (!itsCoordSys->setReferencePixel(refPix2)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    } else {
      rstat = true;
    }
  } else {
    const Coordinate::Type type = stringToType (coordinateType);
    Int c = findCoordinate (type, True);
    trim(refPix2, itsCoordSys->coordinate(c).referencePixel());
    //
    Vector<Int> pixelAxes = itsCoordSys->pixelAxes(c);
    Vector<Double> refPixAll = itsCoordSys->referencePixel();
    for (uInt i=0; i<pixelAxes.nelements(); i++) {
      refPixAll(pixelAxes(i)) = refPix2(i);
    }
    //
    if (!itsCoordSys->setReferencePixel(refPixAll)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    } else {
      rstat = true;
    }
  }
  return rstat;
}

bool
coordsys::setreferencevalue(const ::casac::variant& value,
			    const std::string& coordinateType)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setreferencevalue");

  Bool isWorld;
  Bool shouldBeWorld(True);
  Bool verbose(False);
  ::casac::variant tmpv(value);
  int rtn = isValueWorld(tmpv, shouldBeWorld, verbose);
  if (rtn == -1) {
    *itsLog << LogIO::SEVERE
            << "Should be world value in coordsys.isValueWorld" << LogIO::POST;
    return rstat;
  } else {
    isWorld = (Bool)rtn;
  }

  Bool shouldBeAbs(True);
  Bool rtn2 = checkAbsRel(tmpv, shouldBeAbs);
  if (!rtn2) {
    *itsLog << LogIO::SEVERE
            << "Should be abs value in coordsys.checkAbsRel" << LogIO::POST;
    return rstat;
  }

  Bool isAbs(True);
  Bool first(True);
  Record *rec = coordinateValueToRecord(value, isWorld, isAbs, first);
  if (!rec) {
    *itsLog << LogIO::SEVERE
            << "Conversion of value to Record failed in coordinateValueToRecord"            << LogIO::POST;
    return rstat;
  }

  String dummyType;
  Int c;
  //
  Vector<Double> world;
  if (coordinateType.empty()) {
    c = -1;
    recordToWorldVector(world, dummyType, c, *rec);
    trim(world, itsCoordSys->referenceValue());
    if (!itsCoordSys->setReferenceValue(world)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    } else {
      rstat = true;
    }
  } else {
    const Coordinate::Type type = stringToType (coordinateType);
    c = findCoordinate (type, True);
    recordToWorldVector(world, dummyType, c, *rec);
    trim(world, itsCoordSys->coordinate(c).referenceValue());
    //
    Vector<Double> refValAll(itsCoordSys->referenceValue().copy());
    copyWorldAxes(refValAll, world, c);
    //
    if (!itsCoordSys->setReferenceValue(refValAll)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    } else {
      rstat = true;
    }
  }
  return rstat;
}

bool
coordsys::setrestfrequency(const ::casac::variant& vfvalue, const int which,
			   const bool append)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setrestfrequency");

  Int c = findCoordinate (Coordinate::SPECTRAL, True);
  if (c < 0) return false;  // Avoid exception if type not found
  SpectralCoordinate sc = itsCoordSys->spectralCoordinate(c);

  QuantumHolder qh;
  Quantum<Vector<Double> > restFrequency;
  Vector<casa::Quantity> fvalue;
  if (vfvalue.type() == ::casac::variant::STRING ||
      vfvalue.type() == ::casac::variant::STRINGVEC) {
    if (!toCasaVectorQuantity(vfvalue, fvalue)) {
      *itsLog << LogIO::WARN << "Bad input parameter" << LogIO::POST;
      return False;
    }
    int len = fvalue.size();
    Vector<Double> tmp(len);
    Unit u(fvalue[0].getUnit());
    for (int i = 0; i < len; i++) {
      tmp[i] = fvalue[i].getValue(u);
    }
    Quantum<Vector<Double> > tmpqv(tmp,u);
    restFrequency=tmpqv;
  } else if (vfvalue.type() == ::casac::variant::RECORD) {
    //NOW the record has to be compatible with QuantumHolder::toRecord
    ::casac::variant localvar(vfvalue); //cause its const
    Record * ptrRec = toRecord(localvar.asRecord());
    String error;
    if(!qh.fromRecord(error, *ptrRec)){
      *itsLog << LogIO::WARN << "Error " << error
	      << " in converting quantity "<< LogIO::POST;
      return False;
    }
    delete ptrRec;
    if (qh.isScalar() && qh.isQuantity()) {
      casa::Quantity q=qh.asQuantity();
      Vector<Double> tmp(1);
      tmp[0]=q.getValue();
      Quantum<Vector<Double> > tmpqv(tmp,q.getUnit());
      restFrequency = tmpqv;
    } else  if (qh.isVector() && qh.isQuantumVectorDouble()) {
      restFrequency = qh.asQuantumVectorDouble();
    } else {
      *itsLog << LogIO::WARN
	      << "Cannot convert QuantumHolder to Quantum<Vector<Double> > "
	      << LogIO::POST;
      return False;
    }
  } else if (vfvalue.type() == ::casac::variant::DOUBLE) {
    Vector<Double> rf(1);
    rf[0]=vfvalue.toDouble();
    restFrequency = Quantum<Vector<Double> >(rf,sc.worldAxisUnits()(0));
  } else if (vfvalue.type() == ::casac::variant::DOUBLEVEC) {
    Vector<Double> rf;
    rf=vfvalue.toDoubleVec();
    restFrequency = Quantum<Vector<Double> >(rf,sc.worldAxisUnits()(0));
  } else {
    *itsLog << LogIO::WARN << "Bad input parameter" << LogIO::POST;
    return False;
  }

  //
  casa::Quantity
    theUnit = casa::Quantity(1.0, restFrequency.getFullUnit().getName());
  if (!checkfreq(theUnit)) {
    *itsLog << "Value is not a valid frequency" <<LogIO::EXCEPTION;
  }
  Vector<Double> rf = restFrequency.getValue(Unit(sc.worldAxisUnits()(0)));
  //
  if (which >= 0) {
    sc.setRestFrequencies(rf, which, append);
  } else {
    *itsLog << "Illegal index '" << which
	    << "' into restfrequency vector" << LogIO::EXCEPTION;
  }
  //
  itsCoordSys->replaceCoordinate(sc, c);
  rstat = true;

  return rstat;
}

Bool
qvdFromVar(String &error, Quantum<Vector<Double> > &rtn,
	   const ::casac::variant &vfvalue)
{
  QuantumHolder qh;
  Vector<casa::Quantity> fvalue;
  if (vfvalue.type() == ::casac::variant::BOOLVEC) { //unset
    Vector<Double> tmp;
    tmp.resize(0);
    Unit u("");
    Quantum<Vector<Double> > tmpqv(tmp,u);
    rtn = tmpqv;
  } else if (vfvalue.type() == ::casac::variant::STRING ||
	     vfvalue.type() == ::casac::variant::STRINGVEC) {
    if (!toCasaVectorQuantity(vfvalue, fvalue)) {
      error = "Bad input parameter";
      return False;
    }
    int len = fvalue.size();
    Vector<Double> tmp(len);
    Unit u(fvalue[0].getUnit());
    for (int i = 0; i < len; i++) {
      tmp[i] = fvalue[i].getValue(u);
    }
    Quantum<Vector<Double> > tmpqv(tmp,u);
    rtn=tmpqv;
  } else if (vfvalue.type() == ::casac::variant::RECORD) {
    ::casac::variant localvar(vfvalue);
    Record * ptrRec = toRecord(localvar.asRecord());
    if(!qh.fromRecord(error, *ptrRec)){
      return False;
    }
    delete ptrRec;
    if (qh.isScalar() && qh.isQuantity()) {
      casa::Quantity q=qh.asQuantity();
      Vector<Double> tmp(1);
      tmp[0]=q.getValue();
      Quantum<Vector<Double> > tmpqv(tmp,q.getUnit());
      rtn = tmpqv;
    } else  if (qh.isVector() && qh.isQuantumVectorDouble()) {
      rtn = qh.asQuantumVectorDouble();
    } else {
      error = "Cannot convert QuantumHolder to Quantum<Vector<Double> > ";
      return False;
    }
  } else {
    error = "Unrecognized input parameter";
    return False;
  }
  return True;
}

bool
coordsys::setspectral(const std::string& ref, const ::casac::variant& restfreq,
		      const ::casac::variant& v_frequencies,
		      const std::string& doppler,
		      const ::casac::variant& v_velocities)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setspectral");

  Bool dofreq(False);
  Bool dovel(False);
  Quantum<Double> restFrequency = casa::Quantity(-1.0,"GHz");
  if (!unset(restfreq)) {
    restFrequency = casaQuantity(restfreq);
  }
  //
  String error;
  Quantum<Vector<Double> > frequencies;
  if (!qvdFromVar(error, frequencies, v_frequencies)) {
    *itsLog << LogIO::WARN << "Error in frequencies parameter.  "
	    << error << LogIO::POST;
  } else {
    if (frequencies.getValue().size() > 0) dofreq = True;
  }
  Quantum<Vector<Double> > velocities;
  if (!qvdFromVar(error, velocities, v_velocities)) {
    *itsLog << LogIO::WARN << "Error in velocities parameter.  "
	    << error << LogIO::POST;
  } else {
    if (velocities.getValue().size() > 0) dovel = True;
  }

  //
  if (dofreq && dovel) {
    *itsLog << LogIO::SEVERE
	    << "You cannot give both frequencies and velocities"
	    << LogIO::POST;
    return rstat;
  }
  if (!(ref.size()==0) && !(doppler.size()==0)
      && !(restFrequency.getValue()==-1)
      && !dofreq && !dovel) {
    *itsLog << LogIO::WARN << "Nothing to do" << LogIO::POST;
    return true;
  }

  // Exception if coordinate not found
  Int ic = findCoordinate (Coordinate::SPECTRAL, True);
  SpectralCoordinate oldSpecCoord(itsCoordSys->spectralCoordinate(ic));
  const Vector<String>& names = oldSpecCoord.worldAxisNames();

  // Frequency system
  if (!ref.empty()) {
    MFrequency::Types freqType;
    String code = ref;
    code.upcase();
    if (!MFrequency::getType(freqType, code)) {
      *itsLog << "Invalid frequency reference '" << code
	      << "'" << LogIO::EXCEPTION;
    }
    oldSpecCoord.setFrequencySystem(freqType);
  }

  // Rest frequency
  if (restFrequency.getValue() > 0) {
    Quantum<Double> t(restFrequency);
    t.convert(Unit(oldSpecCoord.worldAxisUnits()(0)));
    oldSpecCoord.setRestFrequency(t.getValue(), False);
  }

  // Frequencies
  Bool doneFreq = False;
  if (dofreq) {
    if (frequencies.getFullUnit() == Unit(String("Hz"))) {
      /*
       *itsLog << LogIO::NORMAL << "Creating tabular SpectralCoordinate";
       *itsLog << " with " << frequencies.getValue().nelements() << " frequency elements" << LogIO::POST;
       */
      SpectralCoordinate sc(oldSpecCoord.frequencySystem(),
			    frequencies.getValue(Unit(String("Hz"))),
			    oldSpecCoord.restFrequency());
      sc.setWorldAxisNames(names);
      //
      itsCoordSys->replaceCoordinate(sc, ic);
      doneFreq = True;
    } else {
      *itsLog << "Illegal unit for frequencies" << LogIO::EXCEPTION;
    }
  }

  // Velocities
  Bool doneVel = False;
  if (dovel) {
    if (velocities.getFullUnit() == Unit(String("km/s"))) {
      if (doneFreq) {
	*itsLog << "You cannot specify frequencies and velocities"
		<< LogIO::EXCEPTION;
      }
      //
      MDoppler::Types dopplerType;
      if (doppler.empty()) {
	*itsLog << "You must specify the doppler type" << LogIO::EXCEPTION;
      }
      if (!MDoppler::getType(dopplerType, doppler)) {
	*itsLog << "Invalid doppler '" << doppler << "'" << LogIO::EXCEPTION;
      }
      //
      /*
       *itsLog << LogIO::NORMAL << "Creating tabular SpectralCoordinate";
       *itsLog << " with " << velocities.getValue().nelements() << " velocity elements" << LogIO::POST;
       */
      SpectralCoordinate sc(oldSpecCoord.frequencySystem(),
			    dopplerType,
			    velocities.getValue(),
			    velocities.getFullUnit().getName(),
			    oldSpecCoord.restFrequency());
      sc.setWorldAxisNames(names);
      //
      itsCoordSys->replaceCoordinate(sc, ic);
      doneVel = True;
    } else {
      *itsLog << "Illegal unit for velocities" << LogIO::EXCEPTION;
    }
  }
  //
  if (!doneFreq && !doneVel) {
    itsCoordSys->replaceCoordinate(oldSpecCoord, ic);
  }
  rstat = true;

  return rstat;
}

bool
coordsys::setstokes(const std::vector<std::string>& in_stokes)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setstokes");

  Vector<String> vstokes = toVectorString(in_stokes);
  Vector<String> stokes;
  if (vstokes.size()==1) {
    sepCommaEmptyToVectorStrings(stokes, vstokes[0]);
  } else {
    stokes = vstokes;
  }

  // Exception if type not found
  Int c = findCoordinate (Coordinate::STOKES, True);

  //
  if (stokes.nelements()>0) {
    Vector<Int> which(stokes.nelements());
    for (uInt i=0; i<stokes.nelements(); i++) {
      String tmp = upcase(stokes(i));
      which(i) = Stokes::type(tmp);
    }
    //
    const StokesCoordinate& sc = itsCoordSys->stokesCoordinate(c);
    StokesCoordinate sc2(sc);
    sc2.setStokes(which);
    itsCoordSys->replaceCoordinate(sc2, c);
    rstat = true;
  } else {
    *itsLog << "You did not specify any new Stokes values" << LogIO::EXCEPTION;
  }
  return rstat;
}

bool
coordsys::settabular(const std::vector<double>& in_pixel, const std::vector<double>& in_world, const int which)
{

  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "settabular");

  Vector<Double> pixel(in_pixel);
  Vector<Double> world(in_world);
  if (in_pixel.size()==1 && in_pixel[0]==-1) {
    pixel.resize(0);
  }
  if (in_world.size()==1 && in_world[0]==-1) {
    world.resize(0);
  }

  if (pixel.size()==0 && world.size()==0) {
    *itsLog << LogIO::WARN << "Nothing to do" << LogIO::POST;
    return false;
  }

  // Exception if coordinate not found
  Int idx = which;
  if (idx < 0) {
    *itsLog << "The specified TabularCoordinate number must be >= 0"
	    << LogIO::EXCEPTION;
  }
  //
  Int ic = -1;
  for (Int i=0,j=0; i<Int(itsCoordSys->nCoordinates()); i++) {
    if (itsCoordSys->type(i)==Coordinate::TABULAR) {
      if (j==idx) {
	ic = i;
	break;
      } else {
	j++;
      }
    }
  }
  if (ic==-1) {
    *itsLog << "Specified TabularCoordinate could not be found"
	    << LogIO::EXCEPTION;
  }
  //
  TabularCoordinate oldTabularCoord(itsCoordSys->tabularCoordinate(ic));
  const String  name = oldTabularCoord.worldAxisNames()(0);
  const String  unit = oldTabularCoord.worldAxisUnits()(0);
  //
  Vector<Double> oldPixel = oldTabularCoord.pixelValues();
  Vector<Double> oldWorld = oldTabularCoord.worldValues();
  //
  uInt nPixel = pixel.nelements();
  uInt nWorld = world.nelements();
  //
  if (nPixel==0 && nWorld==0) {
    *itsLog << "You must give at least one of the pixel or world vectors"
	    << LogIO::EXCEPTION;
  }
  if (nPixel!=0 && nWorld!=0 && nPixel!=nWorld) {
    *itsLog << "Pixel and world vectors must be the same length"
	    << LogIO::EXCEPTION;
  }
  //
  Vector<Double> p = oldPixel.copy();
  if (nPixel > 0) {
    p.resize(0);
    p = pixel; // - 1.0;
  } else {
    *itsLog << "Old pixel vector length = " << oldPixel.nelements()
	    << LogIO::POST;   }
  nPixel = p.nelements();
  //
  Vector<Double> w = oldWorld.copy();
  if (nWorld > 0) {
    w.resize(0);
    w = world;
  } else {
    *itsLog << "Old world vector length = " << oldWorld.nelements()
	    << LogIO::POST;   }
  nWorld = w.nelements();
  //
  if (nPixel != nWorld) {
    *itsLog << "Pixel and world vectors must be the same length"
	    << LogIO::EXCEPTION;
  }
  //
  TabularCoordinate tc(p, w, unit, name);
  itsCoordSys->replaceCoordinate(tc, ic);
  rstat = true;

  return rstat;
}

bool
coordsys::settelescope(const std::string& telescope)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "settelescope");

  ObsInfo obsInfo = itsCoordSys->obsInfo();
  obsInfo.setTelescope(telescope);
  itsCoordSys->setObsInfo(obsInfo);
  //
  MPosition pos;
  if (!MeasTable::Observatory(pos, telescope)) {
    *itsLog << LogIO::WARN
	    << "This telescope is not known to the casapy system" << endl;
    *itsLog << "You can request that it be added" << LogIO::POST;
  }
  rstat = true;

  return rstat;
}

bool
coordsys::setunits(const std::vector<std::string>& value,
		   const std::string& coordinateType, const bool overwrite,
		   const int which)
{
  bool rstat(false);
  *itsLog << LogOrigin("coordsys", "setunits");
  Vector<String> vunits = toVectorString(value);
  Vector<String> units;
  if (vunits.size()==1) {
    sepCommaEmptyToVectorStrings(units, vunits[0]);
  } else {
    units = vunits;
  }

  //
  if (coordinateType.empty()) {
    if (!itsCoordSys->setWorldAxisUnits(units)) {
      *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    }
    rstat = true;
  } else {
    const Coordinate::Type type = stringToType (coordinateType);
    Int c = which;
    if (c < 0) {
      c = findCoordinate (type, False);
    }
    //
    Vector<Int> worldAxes = itsCoordSys->worldAxes(c);
    if (units.nelements() != worldAxes.nelements()) {
      *itsLog << "Supplied axis units vector must be of length "
	      << worldAxes.nelements() << LogIO::EXCEPTION;
    }
    //
    if (overwrite && type==Coordinate::LINEAR) {
      const LinearCoordinate& lc = itsCoordSys->linearCoordinate(c);
      LinearCoordinate lc2(lc);
      if (!lc2.overwriteWorldAxisUnits(units)) {
	*itsLog << lc2.errorMessage() << LogIO::EXCEPTION;
      }
      itsCoordSys->replaceCoordinate(lc2, uInt(c));
    } else if (overwrite && type==Coordinate::TABULAR) {
      const TabularCoordinate& tc = itsCoordSys->tabularCoordinate(c);
      TabularCoordinate tc2(tc);
      if (!tc2.overwriteWorldAxisUnits(units)) {
	*itsLog << tc2.errorMessage() << LogIO::EXCEPTION;
      }
      itsCoordSys->replaceCoordinate(tc2, uInt(c));
    } else {
      Vector<String> unitsAll(itsCoordSys->worldAxisUnits().copy());
      for (uInt i=0; i<worldAxes.nelements(); i++) {
	unitsAll(worldAxes(i)) = units(i);
      }
      //
      if (!itsCoordSys->setWorldAxisUnits(unitsAll)) {
	*itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
      }
    }
    rstat = true;
  }

  return rstat;
}

std::vector<std::string>
coordsys::stokes()
{
  std::vector<std::string> rstat;
  *itsLog << LogOrigin("coordsys", "stokes");

  // Exception if type not found
  Int c = findCoordinate (Coordinate::STOKES, True);
  //
  StokesCoordinate sc = itsCoordSys->stokesCoordinate(c);
  Vector<Int> stokes = sc.stokes();
  //
  Vector<String> t(stokes.nelements());
  for (uInt i=0; i<t.nelements(); i++) {
    t(i) = Stokes::name(Stokes::StokesTypes(stokes(i)));
  }
  //
  rstat = fromVectorString(t);
  return rstat;
}

std::vector<std::string>
coordsys::summary(const std::string& dopplerType, const bool list)
{
  std::vector<std::string> rstat;
  *itsLog << LogOrigin("coordsys", "summary");

  MDoppler::Types velType;
  if (!MDoppler::getType(velType, dopplerType)) {
    *itsLog << LogIO::WARN << "Illegal doppler type, using RADIO"
	    << LogIO::POST;
    velType = MDoppler::RADIO;
  }
  //
  IPosition latticeShape, tileShape;
  Vector<String> messages;
  if (!list) {
    // Only write to  local sink so we can fish the messages out
    LogFilter filter;
    LogSink sink(filter, False);
    LogIO osl(sink);
    //
    messages = itsCoordSys->list(osl, velType, latticeShape, tileShape, True);
  } else {
    messages =
      itsCoordSys->list(*itsLog, velType, latticeShape, tileShape, False);
  }
  if (messages.size() == 0) {
    rstat.resize(1);
    rstat[0]="T";
  } else {
    rstat = fromVectorString(messages);
  }
  return rstat;
}

std::string
coordsys::telescope()
{
  std::string rstat;
  *itsLog << LogOrigin("coordsys", "telescope");

  const ObsInfo& obsInfo = itsCoordSys->obsInfo();
  rstat = obsInfo.telescope();

  return rstat;
}

::casac::record*
coordsys::toabs(const ::casac::variant& value, const int isworld)
{
  ::casac::record *rstat = 0;
  *itsLog << LogOrigin("coordsys", "toabs");

  int shouldBeWorld(isworld);
  Bool verbose(True);
  Bool isWorld;
  ::casac::variant tmpv(value);
  int rtn = isValueWorld(tmpv, shouldBeWorld, verbose);
  if (rtn == -1) {
    *itsLog << LogIO::SEVERE
	    << "Should be world value in coordsys.isValueWorld" << LogIO::POST;
    return rstat;
  } else {
    isWorld = (Bool)rtn;
  }

  // Check that value is in absolute coordinates
  Bool shouldBeAbs(False);
  Bool rtn2 = checkAbsRel(tmpv, shouldBeAbs);
  if (!rtn2) {
    *itsLog << LogIO::SEVERE
	    << "Should be rel value in coordsys.checkAbsRel" << LogIO::POST;
    return rstat;
  }
  // Convert value to a Record
  Bool isAbs(False);
  Bool first(False);
  Record *rec = coordinateValueToRecord(value, isWorld, isAbs, first);
  if (!rec) {
    *itsLog << LogIO::SEVERE
	    << "Conversion of value to Record failed in coordinateValueToRecord"
	    << LogIO::POST;
    return rstat;
  }

  Bool absToRel = False;
  //  Record *rec = toRecord(value);
  
  Record rectmp = absRelRecord(*itsLog, *rec, isWorld, absToRel);
  delete rec;
  if (isWorld) {
    rectmp.define("pw_type", "world");
  } else {
    rectmp.define("pw_type", "pixel");
  }
  rectmp.define("ar_type", "absolute");

  rstat = fromRecord(rectmp);
  //
  return rstat;
}

::casac::record*
coordsys::toabsmany(const ::casac::variant& value, const int isworld)
{

  ::casac::record *rstat=0;
  *itsLog << LogOrigin("coordsys", "toabsmany");

  int shouldBeWorld(isworld);
  Bool verbose(True);
  Bool isWorld;
  ::casac::variant tmpv(value);

  int rtn = isValueWorld(tmpv, shouldBeWorld, verbose);
  if (rtn == -1) {
    *itsLog << LogIO::SEVERE
	    << "Should be world value in coordsys.isValueWorld" << LogIO::POST;
    return rstat;
  } else {
    isWorld = (Bool)rtn;
  }

  // Check that value is in absolute coordinates
  Bool shouldBeAbs(False);
  Bool rtn2 = checkAbsRel(tmpv, shouldBeAbs);
  if (!rtn2) {
    *itsLog << LogIO::SEVERE
	    << "Should be rel value in coordsys.checkAbsRel" << LogIO::POST;
    return rstat;
  }

  Array<Double> valueIn;

  if (value.type() == ::casac::variant::DOUBLEVEC) {
    Vector<Int> value_shape = value.arrayshape();
    std::vector<double> value_vec = value.getDoubleVec();
    valueIn.resize(IPosition(value_shape));
    int i = 0;
    for (Array<Double>::iterator iter = valueIn.begin();
	 iter != valueIn.end(); iter++) {
      *iter = value_vec[i++];
    }
  } else if (value.type() == ::casac::variant::RECORD) {
    ::casac::variant localvar(value);
    Record *tmp = toRecord(localvar.asRecord());
    if (tmp->isDefined("numeric")) {
      valueIn = tmp->asArrayDouble("numeric");
    } else {
      *itsLog << LogIO::SEVERE << "unsupported record type for value"
	      << LogIO::EXCEPTION;
      return rstat;
    }
    delete tmp;
  } else {
    *itsLog << LogIO::SEVERE << "unsupported data type for value"
	    << LogIO::EXCEPTION;
  }

  AlwaysAssert(valueIn.shape().nelements()==2, AipsError);
  Matrix<Double> values(valueIn);
  Double offset = 0.0;
  if (isWorld) {
    itsCoordSys->makeWorldAbsoluteMany(values);
  } else {
    itsCoordSys->makePixelAbsoluteMany(values);
    //offset = 1.0;                       // Make 1-rel
  }
  Array<Double> valueOut(values.copy() + offset);

  /*
  // put Array<Double> valueOut into ::casac::variant
  std::vector<int> shape;
  std::vector<double> rtnVec;
  valueOut.shape().asVector().tovector(shape);
  valueOut.tovector(rtnVec);
  rstat = new ::casac::variant(rtnVec, shape);
     if (its.toAbsManyRec.isworld) {
           rv::pw_type := 'world';
        } else {
           rv::pw_type := 'pixel';
        }
        rv::ar_type := 'absolute';
#
        return rv;  */

  Record tmpRec;
  tmpRec.define("numeric", valueOut);
  if (isWorld) {
    tmpRec.define("pw_type", "world");
  } else {
    tmpRec.define("pw_type", "pixel");
  }
  tmpRec.define("ar_type","absolute");
  rstat = fromRecord(tmpRec);

  return rstat;
}

::casac::record *
coordsys::topixel(const ::casac::variant& value)
{
  ::casac::record *rtnrec = 0;
  std::vector<double> rstat;
  *itsLog << LogOrigin("coordsys", "topixel");

  //
  Bool shouldBeWorld(True);
  Bool verbose(False);
  Bool isWorld;
  ::casac::variant tmpv(value);
  int rtn = isValueWorld(tmpv, shouldBeWorld, verbose);
  if (rtn == -1) {
    *itsLog << LogIO::SEVERE
	    << "Should be world value in coordsys.isValueWorld" << LogIO::POST;
    return rtnrec;
  } else {
    isWorld = (Bool)rtn;
  }

  // Check that value is in absolute coordinates
  Bool shouldBeAbs(True);
  Bool rtn2 = checkAbsRel(tmpv, shouldBeAbs);
  if (!rtn2) {
    *itsLog << LogIO::SEVERE
	    << "Should be abs value in coordsys.checkAbsRel" << LogIO::POST;
    return rtnrec;
  }

  // Convert value to a Record
  Bool isAbs(True);
  Bool first(True);
  Record *rec = coordinateValueToRecord(value, isWorld, isAbs, first);
  if (!rec) {
    *itsLog << LogIO::SEVERE
	    << "Conversion of value to Record failed in coordinateValueToRecord"
	    << LogIO::POST;
    return rtnrec;
  }

  String dummyType;
  Int c = -1;
  Vector<Double> world;
  recordToWorldVector(world, dummyType, c, *rec);
  trim(world, itsCoordSys->referenceValue());
  //
  Vector<Double> pixel;
  if (!itsCoordSys->toPixel (pixel, world)) {
    *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
  }
  //
  delete rec;

  Record tmpRec;
  tmpRec.define("numeric",pixel);
  tmpRec.define("pw_type","pixel");
  tmpRec.define("ar_type","absolute");

  rtnrec = fromRecord(tmpRec);
  return rtnrec;
}

std::vector<double>
coordsys::toPixel(const ::casac::variant& value)
{
  std::vector<double> rstat;
  *itsLog << LogOrigin("coordsys", "topixel");

  //
  Bool shouldBeWorld(True);
  Bool verbose(False);
  Bool isWorld;
  ::casac::variant tmpv(value);
  int rtn = isValueWorld(tmpv, shouldBeWorld, verbose);
  if (rtn == -1) {
    *itsLog << LogIO::SEVERE
            << "Should be world value in coordsys.isValueWorld" << LogIO::POST;
    return rstat;
  } else {
    isWorld = (Bool)rtn;
  }

  // Check that value is in absolute coordinates
  Bool shouldBeAbs(True);
  Bool rtn2 = checkAbsRel(tmpv, shouldBeAbs);
  if (!rtn2) {
    *itsLog << LogIO::SEVERE
            << "Should be abs value in coordsys.checkAbsRel" << LogIO::POST;
    return rstat;
  }

  // Convert value to a Record
  Bool isAbs(True);
  Bool first(True);
  Record *rec = coordinateValueToRecord(value, isWorld, isAbs, first);
  if (!rec) {
    *itsLog << LogIO::SEVERE
            << "Conversion of value to Record failed in coordinateValueToRecord"            << LogIO::POST;
    return rstat;
  }

  String dummyType;
  Int c = -1;
  Vector<Double> world;
  recordToWorldVector(world, dummyType, c, *rec);
  trim(world, itsCoordSys->referenceValue());
  //

  Vector<Double> pixel;
  if (!itsCoordSys->toPixel (pixel, world)) {
    *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
  }
  //
  delete rec;

  pixel.tovector(rstat);
  /*    rv::pw_type := 'pixel';
        rv::ar_type := 'absolute';
        return rv; */

  return rstat;
}

::casac::record*
coordsys::topixelmany(const ::casac::variant& value)
{
  ::casac::record *rstat = 0;
  *itsLog << LogOrigin("coordsys", "topixelmany");

  //
  Vector<Int> value_shape = value.arrayshape();
  std::vector<double> value_vec = value.getDoubleVec();
  Array<Double> world;
  world.resize(IPosition(value_shape));
  int i = 0;
  for (Array<Double>::iterator iter = world.begin();
       iter != world.end(); iter++) {
    *iter = value_vec[i++];
  }

  //
  AlwaysAssert(world.shape().nelements()==2, AipsError);
  Matrix<Double> pixels;
  Matrix<Double> worlds(world);
  Vector<Bool> failures;
  if (!itsCoordSys->toPixelMany(pixels, worlds, failures)) {
    *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
  }
  Array<Double> pixel(pixels.copy());

  //
  /*
  std::vector<int> shape;
  std::vector<double> rtnVec;
  pixel.shape().asVector().tovector(shape);
  pixel.tovector(rtnVec);
  rstat = new ::casac::variant(rtnVec, shape);
  */
  Record tmpRec;
  tmpRec.define("numeric", pixel);
  tmpRec.define("pw_type", "pixel");
  tmpRec.define("ar_type", "absolute");
  rstat = fromRecord(tmpRec);

  return rstat;
}

::casac::record*
coordsys::torecord()
{
  ::casac::record *rstat = new ::casac::record();
  *itsLog << LogOrigin("coordsys", "torecord");

  try {
    Record rec;
    if (!itsCoordSys->save(rec,"CoordinateSystem")) {
      *itsLog << "Could not convert to record because "
	      << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
    }

    rec.define(RecordFieldId("parentName"), itsParentImageName);

    // Put it in a ::casac::record
    rstat = fromRecord(rec.asRecord("CoordinateSystem"));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
    	    << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
coordsys::torel(const ::casac::variant& value, const int isworld)
{
  ::casac::record *rstat = 0;
  *itsLog << LogOrigin("coordsys", "torel");

  int shouldBeWorld(isworld);
  Bool verbose(True);
  Bool isWorld;
  ::casac::variant tmpv(value);
  int rtn = isValueWorld(tmpv, shouldBeWorld, verbose);
  if (rtn == -1) {
    *itsLog << LogIO::SEVERE
	    << "Should be world value in coordsys.isValueWorld"
	    << LogIO::EXCEPTION;
    return rstat;
  } else {
    isWorld = (Bool)rtn;
  }

  // Check that value is in absolute coordinates
  Bool shouldBeAbs(True);
  Bool rtn2 = checkAbsRel(tmpv, shouldBeAbs);
  if (!rtn2) {
    *itsLog << LogIO::SEVERE
	    << "Should be abs value in coordsys.torel" << LogIO::EXCEPTION;
    return rstat;
  }
  // Convert value to a Record
  Bool isAbs(True);
  Bool first(False);
  Record *rec = coordinateValueToRecord(value, isWorld, isAbs, first);
  if (!rec) {
    *itsLog << LogIO::SEVERE
	    << "Conversion of value to Record failed in coordinateValueToRecord"
	    << LogIO::EXCEPTION;
    return rstat;
  }

  //
  Bool absToRel = True;
  Record tmpRec = absRelRecord(*itsLog, *rec, isWorld, absToRel);
  delete rec;
  //
  if (isWorld) {
    tmpRec.define("pw_type","world");
  } else {
    tmpRec.define("pw_type","pixel");
  }
  tmpRec.define("ar_type","relative");

  rstat = fromRecord(tmpRec);
  return rstat;
}

::casac::record *
coordsys::torelmany(const ::casac::variant& value, const int isWorld)
{
  ::casac::record *rstat = 0;
  *itsLog << LogOrigin("coordsys", "torelmany");

  // form Array<Double> valueIn
  Vector<Int> value_shape = value.arrayshape();
  std::vector<double> value_vec = value.getDoubleVec();
  Array<Double> valueIn;
  valueIn.resize(IPosition(value_shape));
  int i = 0;
  for (Array<Double>::iterator iter = valueIn.begin();
       iter != valueIn.end(); iter++) {
    *iter = value_vec[i++];
  }

  AlwaysAssert(valueIn.shape().nelements()==2, AipsError);
  Double offset = 0.0;
  //if (!isWorld) offset = -1.0;            // Make 0-rel
  Matrix<Double> values(valueIn + offset);
  if (isWorld) {
    itsCoordSys->makeWorldRelativeMany(values);
  } else {
    itsCoordSys->makePixelRelativeMany(values);
  }
  Array<Double> valueOut(values.copy());

  /*
  // put Array<Double> valueOut into ::casac::variant
  std::vector<int> shape;
  std::vector<double> rtnVec;
  valueOut.shape().asVector().tovector(shape);
  valueOut.tovector(rtnVec);
  rstat = new ::casac::variant(rtnVec, shape);
  */

  Record tmpRec;
  tmpRec.define("numeric", valueOut);
  if (isWorld) {
    tmpRec.define("pw_type", "world");
  } else {
    tmpRec.define("pw_type", "pixel");
  }
  tmpRec.define("ar_type","relative");
  rstat = fromRecord(tmpRec);

  return rstat;
}

::casac::record*
coordsys::toworld(const ::casac::variant& value, const std::string& format)
{
  ::casac::record *rstat = 0;
  *itsLog << LogOrigin("coordsys", "toworld");

  Vector<Double> pixel;
  if (unset(value)) {
    Vector<Double> refpix = itsCoordSys->referencePixel();
    pixel.resize(refpix.size());
    for (uInt i=0; i < refpix.size(); i++) pixel[i]=refpix[i];
  } else if (value.type() == ::casac::variant::DOUBLEVEC) {
    pixel = value.getDoubleVec();
  } else if (value.type() == ::casac::variant::INTVEC) {
    Vector<Int> ipixel = value.getIntVec();
    Int n = ipixel.size();
    pixel.resize(n);
    for (int i=0 ; i < n; i++) pixel[i]=ipixel[i];
  } else if (value.type() == ::casac::variant::RECORD) {
    ::casac::variant localvar(value);
    Record *tmp = toRecord(localvar.asRecord());
    if (tmp->isDefined("numeric")) {
      pixel = tmp->asArrayDouble("numeric");
    } else {
      *itsLog << LogIO::SEVERE << "unsupported record type for pixel"
	      << LogIO::EXCEPTION;
      return rstat;
    }
    delete tmp;
  } else {
    *itsLog << LogIO::SEVERE << "unsupported data type for pixel"
	    << LogIO::EXCEPTION;
    return rstat;
  }

  Record rec = toWorldRecord (pixel, format);
  rec.define("pw_type","world");
  rec.define("ar_type","absolute");

  rstat = fromRecord(rec);

  /*    rv2 := its.recordToCoordinateValue(rv);
	rv2::pw_type := 'world';
        rv2::ar_type := 'absolute';*/
  return rstat;
}

::casac::record*
coordsys::toworldmany(const ::casac::variant& value)
{
  ::casac::record * rstat = 0;
  *itsLog << LogOrigin("coordsys", "toworldmany");

  // form Array<Double> pixel
  Vector<Int> value_shape = value.arrayshape();
  std::vector<double> value_vec = value.getDoubleVec();
  Array<Double> pixel;
  pixel.resize(IPosition(value_shape));
  int i = 0;
  for (Array<Double>::iterator iter = pixel.begin();
       iter != pixel.end(); iter++) {
    *iter = value_vec[i++];
  }

  //
  AlwaysAssert(pixel.shape().nelements()==2, AipsError);
  Matrix<Double> worlds;
  Matrix<Double> pixels(pixel);
  Vector<Bool> failures;
  if (!itsCoordSys->toWorldMany(worlds, pixels, failures)) {
    *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
  }
  Array<Double> world(worlds.copy());

  // put Array<Double> world into ::casac::variant
  /*
  std::vector<int> shape;
  std::vector<double> rtnVec;
  world.shape().asVector().tovector(shape);
  world.tovector(rtnVec);
  rstat = new ::casac::variant(rtnVec, shape);
  */
  Record tmpRec;
  tmpRec.define("numeric", world);
  tmpRec.define("pw_type", "world");
  tmpRec.define("ar_type","absolute");
  rstat = fromRecord(tmpRec);

  return rstat;
}

std::string
coordsys::type()
{
  string rstat;
  *itsLog << LogOrigin("coordsys", "type");
  rstat = "coordsys";
  return rstat;
}

std::vector<std::string>
coordsys::units(const std::string& cordtype)
{
  std::vector<string> rstat;
  *itsLog << LogOrigin("coordsys", "units");

  Vector<String> units = itsCoordSys->worldAxisUnits();
  if (cordtype=="") {
    rstat = fromVectorString(units);
  } else {
    std::vector<int> pixelaxes;
    std::vector<int> worldaxes;
    if (findcoordinate (pixelaxes, worldaxes, cordtype, 0)) {
      int n = pixelaxes.size();
      for (int i = 0; i < n; i++) {
	rstat.push_back(units[pixelaxes[i]]);
      }
    } else {
      *itsLog << LogIO::WARN << "A coordinate of type ["
	      << cordtype << "] does not exist" << LogIO::POST;
    }
  }
  return rstat;
}

std::vector<double>
coordsys::velocitytofrequency(const std::vector<double>& value,
			      const std::string& frequnit,
			      const std::string& dopplerType,
			      const std::string& velUnit)
{
  std::vector<double> rstat;
  *itsLog << LogOrigin("coordsys", "velocitytofrequency");

  Vector<Double> velocity(value);
  String freqUnit(frequnit);
  if (freqUnit=="") freqUnit=units("spectral")[0];

  //
  Int after = -1;
  Int c = itsCoordSys->findCoordinate(Coordinate::SPECTRAL, after);
  if (c < 0) {
    *itsLog << "There is no spectral coordinate in this CoordinateSystem"
	    << LogIO::EXCEPTION;
   }

  // Get SpectralCoordinate
  const SpectralCoordinate& sc0 = itsCoordSys->spectralCoordinate(c);
  SpectralCoordinate sc(sc0);
  Vector<String> units(sc.worldAxisUnits().copy());
  units(0) = freqUnit;
  if (!sc.setWorldAxisUnits(units)) {
    *itsLog << "Failed to set frequency units of " << freqUnit
	    << " because " << sc.errorMessage() << LogIO::EXCEPTION;
  }

  // Convert velocity type to enum
  MDoppler::Types velType;
  if (!MDoppler::getType(velType, dopplerType)) {
    *itsLog << LogIO::WARN << "Illegal velocity type, using RADIO"
	    << LogIO::POST;
    velType = MDoppler::RADIO;
  }

  // Convert to fequency
  sc.setVelocity (velUnit, velType);
  Vector<Double> frequency;
  if (!sc.velocityToFrequency(frequency, velocity)) {
    *itsLog << "Conversion to frequency failed because "
	    << sc.errorMessage() << endl;
  }
  frequency.tovector(rstat);
  return rstat;
}

void 
coordsys::addCoordinate (CoordinateSystem& cSys, Bool direction, Bool spectral,
			 const Vector<String>& stokes, Int linear,
			 Bool tabular)
{
  if (direction) CoordinateUtil::addDirAxes(cSys);
  //
  if (stokes.nelements()>0) {
    Vector<Int> which(stokes.nelements());
    for (uInt i=0; i<stokes.nelements(); i++) {
      String tmp = upcase(stokes(i));
      which(i) = Stokes::type(tmp);
    }
    StokesCoordinate sc(which);
    cSys.addCoordinate(sc);
  }
  //
  if (spectral) CoordinateUtil::addFreqAxis(cSys);
  //
  if (linear > 0) {
    Vector<String> names(linear);
    Vector<String> units(linear);
    Vector<Double> refVal(linear);
    Vector<Double> refPix(linear);
    Vector<Double> incr(linear);
    for (Int i=0; i<linear; i++) {
      ostringstream oss;
      oss << i; //+1;
      names(i) = "LinAxis" + String(oss);
      units(i) = "km";
      refVal(i) = 0.0;
      refPix(i) = 0.0;
      incr(i) = 1.0;
    }
    //
    Matrix<Double> pc(linear,linear);
    pc.set(0.0);
    pc.diagonal() = 1.0;
    LinearCoordinate lc(names, units, refVal, incr, pc, refPix);
    cSys.addCoordinate(lc);
  }
  //
  if (tabular) {
    Double refVal = 0.0;
    Double refPix = 0.0;
    Double inc = 1.0;
    String unit("km");
    String name("TabAxis1");
    TabularCoordinate tc(refVal, inc, refPix, unit, name);
    cSys.addCoordinate(tc);
  }
}

Coordinate::Type coordsys::stringToType(const String& typeIn) const
  //
  // Convert the users string to a Coordinate type.
  // We don't allow Tabular coordinates as the user
  // does not interact with them directly.
  //
{
  String ct= upcase(typeIn);
  String ct1(ct.at(0,1));
  String ct2(ct.at(0,2));
  //
  if (ct1==String("L")) return Coordinate::LINEAR;
  if (ct1==String("D")) return Coordinate::DIRECTION;
  if (ct1==String("T")) return Coordinate::TABULAR;
  //
  if (ct2==String("ST")) return Coordinate::STOKES;
  if (ct2==String("SP")) return Coordinate::SPECTRAL;
  //
  *itsLog << LogOrigin("coordsys", "stringToType");
  *itsLog << "Unknown coordinate type" << LogIO::EXCEPTION;
  //
  Coordinate::Type t(Coordinate::LINEAR);
  return t;
}

Record coordsys::worldVectorToRecord (const Vector<Double>& world,
                                      Int c, const String& format,
                                      Bool isAbsolute, Bool showAsAbsolute)
  //
  // World vector must be in the native units of cSys
  // c = -1 means world must be length cSys.nWorldAxes
  // c > 0 means world must be length cSys.coordinate(c).nWorldAxes()
  // format from 'n,q,s,m'
  //
{
  *itsLog << LogOrigin("coordsys", "worldVectorToRecord");
  String ct= upcase(format);
  Vector<String> units;
  if (c < 0) {
    units = itsCoordSys->worldAxisUnits();
  } else {
    units = itsCoordSys->coordinate(c).worldAxisUnits();
  }
  //  AlwaysAssert(world.nelements()==units.nelements(),AipsError);//
  Record rec;
  if (ct.contains(String("N"))) {
    rec.define("numeric", world);
  }
  //
  if (ct.contains(String("Q"))) {
    String error;
    Record recQ1, recQ2;
    //
    for (uInt i=0; i<world.nelements(); i++) {
      Quantum<Double> worldQ(world(i), Unit(units(i)));
      recQ1 = quantumToRecord (*itsLog, worldQ);
      recQ2.defineRecord(i, recQ1);
    }
    rec.defineRecord("quantity", recQ2);
  }
  //
  if (ct.contains(String("S"))) {
    Vector<Int> worldAxes;
    if (c <0) {
      worldAxes.resize(world.nelements());
      indgen(worldAxes);
    } else {
      worldAxes = itsCoordSys->worldAxes(c);
    }
    //
    Coordinate::formatType fType = Coordinate::SCIENTIFIC;
    Int prec = 8;
    String u;
    Int coord, axisInCoord;
    Vector<String> fs(world.nelements());
    for (uInt i=0; i<world.nelements(); i++) {
      itsCoordSys->findWorldAxis(coord, axisInCoord, i);
      if (itsCoordSys->type(coord)==Coordinate::DIRECTION ||
	  itsCoordSys->type(coord)==Coordinate::STOKES) {
	fType = Coordinate::DEFAULT;
      } else {
	fType = Coordinate::SCIENTIFIC;
      }
      //
      u = "";
      fs(i) = itsCoordSys->format (u, fType, world(i), worldAxes(i),
				   isAbsolute, showAsAbsolute, prec);
      if ((u != String("")) && (u != String(" "))) {
	fs(i) += String(" ") + u;
      }
    }

    rec.define("string", fs);
  }
  //
  if (ct.contains(String("M"))) {
    Record recM = worldVectorToMeasures(world, c, isAbsolute);      rec.defineRecord("measure", recM);
  }
  //
  return rec;
}

Record 
coordsys::quantumToRecord (LogIO& os, const Quantum<Double>& value) const
{
  Record rec;
  QuantumHolder h(value);
  String error;
  if (!h.toRecord(error, rec)) os << error << LogIO::EXCEPTION;   return rec;
}

Record coordsys::worldVectorToMeasures(const Vector<Double>& world,
                                       Int c, Bool abs) const
{
  *itsLog << LogOrigin("coordsys", "worldVectorToMeasures(...)");

  //
  uInt directionCount, spectralCount, linearCount, stokesCount, tabularCount;
  directionCount = spectralCount = linearCount = stokesCount = tabularCount
    = 0;

  // Loop over desired Coordinates
  Record rec;
  String error;
  uInt s,  e;
  if (c < 0) {
    //AlwaysAssert(world.nelements()==itsCoordSys->nWorldAxes(), AipsError);
    s = 0;
    e = itsCoordSys->nCoordinates();
  } else {
    //AlwaysAssert(world.nelements()==itsCoordSys->coordinate(c).nWorldAxes(), AipsError);
    s = c;
    e = c+1;
  }
  //
  for (uInt i=s; i<e; i++) {
    // Find the world axes in the CoordinateSystem that this
    // coordinate belongs to
    const Vector<Int>& worldAxes = itsCoordSys->worldAxes(i);
    const uInt nWorldAxes = worldAxes.nelements();
    Vector<Double> world2(nWorldAxes);
    const Coordinate& coord = itsCoordSys->coordinate(i);
    Vector<String> units = coord.worldAxisUnits();
    Bool none = True;

    // Fill in missing world axes if all coordinates specified
    if (c < 0) {
      for (uInt j=0; j<nWorldAxes; j++) {
	if (worldAxes(j)<0) {
	  world2(j) = coord.referenceValue()(j);
	} else {
	  world2(j) = world(worldAxes(j));
	  none = False;
	}
      }
    } else {
      world2 = world;
      none = False;
    }
    //
    if (itsCoordSys->type(i) == Coordinate::LINEAR ||
	itsCoordSys->type(i) == Coordinate::TABULAR) {
      if (!none) {
	Record linRec1, linRec2;
	for (uInt k=0; k<world2.nelements(); k++) {
	  Quantum<Double> value(world2(k), units(k));
	  linRec1 = quantumToRecord (*itsLog, value);
	  linRec2.defineRecord(k, linRec1);
	}
	//
	if (itsCoordSys->type(i) == Coordinate::LINEAR) {
	  rec.defineRecord("linear", linRec2);
	} else if (itsCoordSys->type(i) == Coordinate::TABULAR) {
	  rec.defineRecord("tabular", linRec2);
	}
      }
      //
      if (itsCoordSys->type(i) == Coordinate::LINEAR) linearCount++;       if (itsCoordSys->type(i) == Coordinate::TABULAR) tabularCount++;
    } else if (itsCoordSys->type(i) == Coordinate::DIRECTION) {
      if (!abs) {
	*itsLog << "It is not possible to have a relative MDirection measure" << LogIO::EXCEPTION;
      }
      //AlwaysAssert(worldAxes.nelements()==2,AipsError);
      //
      if (!none) {
	// Make an MDirection and stick in record
	Quantum<Double> t1(world2(0), units(0));
	Quantum<Double> t2(world2(1), units(1));
	MDirection direction(t1, t2, itsCoordSys->directionCoordinate(i).directionType());
	//
	MeasureHolder h(direction);
	Record dirRec;
	if (!h.toRecord(error, dirRec)) {
	  *itsLog << error << LogIO::EXCEPTION;
	} else {
	  rec.defineRecord("direction", dirRec);
	}
      }
      directionCount++;
    } else if (itsCoordSys->type(i) == Coordinate::SPECTRAL) {
      if (!abs) {
	*itsLog << "It is not possible to have a relative MFrequency measure" << LogIO::EXCEPTION;
      }
      //AlwaysAssert(worldAxes.nelements()==1,AipsError);
      //
      if (!none) {
	// Make an MFrequency and stick in record
	Record specRec, specRec1;
	Quantum<Double> t1(world2(0), units(0));
	const SpectralCoordinate& sc0 = itsCoordSys->spectralCoordinate(i);
	MFrequency frequency(t1, sc0.frequencySystem());
	//
	MeasureHolder h(frequency);
	if (!h.toRecord(error, specRec1)) {
	  *itsLog << error << LogIO::EXCEPTION;
	} else {
	  specRec.defineRecord("frequency", specRec1);
	}
	//
	SpectralCoordinate sc(sc0);
	// Do velocity conversions and stick in MDOppler
	// Radio
	sc.setVelocity (String("km/s"), MDoppler::RADIO);
	Quantum<Double> velocity;
	if (!sc.frequencyToVelocity(velocity, frequency)) {
	  *itsLog << sc.errorMessage() << LogIO::EXCEPTION;
	} else {
	  MDoppler v(velocity, MDoppler::RADIO);
	  MeasureHolder h(v);
	  if (!h.toRecord(error, specRec1)) {
	    *itsLog << error << LogIO::EXCEPTION;
	  } else {
	    specRec.defineRecord("radiovelocity", specRec1);
	  }
	}

	// Optical
	sc.setVelocity (String("km/s"), MDoppler::OPTICAL);
	if (!sc.frequencyToVelocity(velocity, frequency)) {
	  *itsLog << sc.errorMessage() << LogIO::EXCEPTION;
	} else {
	  MDoppler v(velocity, MDoppler::OPTICAL);
	  MeasureHolder h(v);
	  if (!h.toRecord(error, specRec1)) {
	    *itsLog << error << LogIO::EXCEPTION;
	  } else {
	    specRec.defineRecord("opticalvelocity", specRec1);
	  }
	}

	// beta (relativistic/true)
	sc.setVelocity (String("km/s"), MDoppler::BETA);
	if (!sc.frequencyToVelocity(velocity, frequency)) {
	  *itsLog << sc.errorMessage() << LogIO::EXCEPTION;
	} else {
	  MDoppler v(velocity, MDoppler::BETA);
	  MeasureHolder h(v);
	  if (!h.toRecord(error, specRec1)) {
	    *itsLog << error << LogIO::EXCEPTION;
	  } else {
	    specRec.defineRecord("betavelocity", specRec1);              }
	}

	// Fill spectral record
	rec.defineRecord("spectral", specRec);
      }
      spectralCount++;
    } else if (itsCoordSys->type(i) == Coordinate::STOKES) {
      if (!abs) {
	*itsLog << "It makes no sense to have a relative Stokes measure" << LogIO::EXCEPTION;
      }
      //AlwaysAssert(worldAxes.nelements()==1,AipsError);
      //
      if (!none) {
	const StokesCoordinate& coord0 = itsCoordSys->stokesCoordinate(i);
	StokesCoordinate coord(coord0);             // non-const
	String u;
	String s = coord.format(u, Coordinate::DEFAULT, world2(0),
				0, True, True, -1);
	rec.define("stokes", s);
      }
      stokesCount++;
    } else {
      *itsLog << "Cannot handle Coordinates of type " << itsCoordSys->showType(i) << LogIO::EXCEPTION;
    }
  }
  //
  if (directionCount > 1) {
    *itsLog << LogIO::WARN << "There was more than one DirectionCoordinate in the " << LogIO::POST;
    *itsLog << LogIO::WARN << "CoordinateSystem.  Only the last one is returned" << LogIO::POST;
  }
  if (spectralCount > 1) {
    *itsLog << LogIO::WARN << "There was more than one SpectralCoordinate in the " << LogIO::POST;
    *itsLog << LogIO::WARN << "CoordinateSystem.  Only the last one is returned" << LogIO::POST;
  }
  if (stokesCount > 1) {
    *itsLog << LogIO::WARN << "There was more than one StokesCoordinate in the " << LogIO::POST;
    *itsLog << LogIO::WARN << "CoordinateSystem.  Only the last one is returned" << LogIO::POST;
  }
  if (linearCount > 1) {
    *itsLog << LogIO::WARN << "There was more than one LinearCoordinate in the " << LogIO::POST;
    *itsLog << LogIO::WARN << "CoordinateSystem.  Only the last one is returned" << LogIO::POST;
  }
  if (tabularCount > 1) {
    *itsLog << LogIO::WARN << "There was more than one TabularCoordinate in the " << LogIO::POST;
    *itsLog << LogIO::WARN << "CoordinateSystem.  Only the last one is returned" << LogIO::POST;
  }
  //
  return rec;
}

Int coordsys::findCoordinate (Coordinate::Type type, Bool warn)
const
{
  *itsLog << LogOrigin("coordsys", "findCoordinate()");

  Int afterCoord = -1;
  Int c = itsCoordSys->findCoordinate(type, afterCoord);
  if (c<0) {
    *itsLog << "No coordinate of type " << Coordinate::typeToString(type)
	    << " in this CoordinateSystem" << LogIO::EXCEPTION;
  }
  //
  afterCoord = c;
  Int c2 = itsCoordSys->findCoordinate(type, afterCoord);
  if (warn && c2 >= 0) {
    *itsLog << LogIO::WARN
       << "This CoordinateSystem has more than one coordinate of type "
       << Coordinate::typeToString(type) << LogIO::POST;
  }
  return c;
}

void coordsys::recordToWorldVector (Vector<Double>& out, String& type,
                                    Int c, const RecordInterface& rec) const
  //
  // The Record may hold any combination of "numeric", "quantity",
  // "measure" and "string".  They are all representations of the same
  // thing.  So we only need convert from one type to world double in
  // native units 
  //
{
  *itsLog << LogOrigin("coordsys", "recordToWorldVector(...)");
  //
  Bool done = False;
  if (rec.isDefined("numeric")) {
    out.resize(0);
    out = rec.asArrayDouble("numeric");     // Assumed native units
    type += "n";
    done = True;
  }
  //
  Vector<String> units;
  if (c < 0) {
    units = itsCoordSys->worldAxisUnits();
  } else {
    units = itsCoordSys->coordinate(c).worldAxisUnits();
  }
  //
  if (rec.isDefined("quantity")) {
    if (!done) {
      const RecordInterface& recQ = rec.asRecord("quantity");
      out.resize(0);
      out = quantumVectorRecordToVectorDouble (recQ, units);
      done = True;
    }
    type += "q";
  }
  //
  if (rec.isDefined("measure")) {
    if (!done) {
      const RecordInterface& recM = rec.asRecord("measure");
      Vector<Double> tmp = measuresToWorldVector (recM);
      if (c < 0) {
	out.resize(0);
	out = tmp;
      } else {
	Vector<Int> worldAxes = itsCoordSys->worldAxes(c);
	out.resize(worldAxes.nelements());
	for (uInt i=0; i<worldAxes.nelements(); i++) {
	  out(i) = tmp(worldAxes(i));
	}
      }
      done = True;
    }
    type += "m";
  }
  //
  if (rec.isDefined("string")) {
    if (!done) {
      Vector<String> world = rec.asArrayString("string");
      out.resize(0);
      out = stringToWorldVector (*itsLog, world, units);
      done = True;
    }
    type += "s";
  }
  //
  if (rec.isDefined("quantum")) {
    if (!done) {
      const RecordInterface& recQ = rec.asRecord("quantum");
      out.resize(0);
      out = quantumRecordToVectorDouble (recQ, units);
      done = True;
    }
    type += "q";
  }
  //
  if (rec.isDefined("qstring")) {
    if (!done) {
      String world = rec.asString("qstring");
      out.resize(1);
      QuantumHolder qh;
      String error;
      if (!qh.fromString(error, world)) {
	*itsLog << LogIO::SEVERE << "Error " << error
		<< " converting string quantity" << LogIO::POST;
      }
      casa::Quantity q = qh.asQuantity();
      out[0]=q.getValue(Unit(units(0))); // always gives the correct unit?
      done = True;
    }
    type += "s";
  }

  //
  if (!done) {
    ostringstream os;
    os << rec;
    *itsLog << "Unrecognized format for world coordinate " << endl
	    << "Rec=[" << os.str() << "]" << LogIO::EXCEPTION;
  }
}

Vector<Double> 
coordsys::quantumVectorRecordToVectorDouble (const RecordInterface& recQ,
					     const Vector<String>& units) const
  //
  // Convert vector to world double in native units
  //
{
  *itsLog << LogOrigin("coordsys", "quantumVectorRecordToVectorDouble");
  Record recQ2;
  QuantumHolder h;
  String error;
  Quantum<Double> q;
  const uInt n = recQ.nfields();
  Vector<Double> worldIn(n);
  if (n != units.size()) {
    *itsLog << "Number of axes must equal number of fields in record!"
	    << LogIO::EXCEPTION;
  }
  //
  for (uInt i=0; i<n; i++) {
    recQ2 = recQ.asRecord(i);
    if (!h.fromRecord(error, recQ2)) {
      *itsLog << error << LogIO::EXCEPTION;
    }
    q = h.asQuantumDouble();
    worldIn(i) = q.getValue(Unit(units(i)));
  }
  return worldIn;
}

Vector<Double> 
coordsys::quantumRecordToVectorDouble (const RecordInterface& recQ,
				       const Vector<String>& units) const
  // Convert quantum to world double in native units
{
  // Vector<Double> worldIn(1);
  //  casa::Quantity q(recQ.asDouble("value"), recQ.asString("unit"));
  //  worldIn[0]=q.getValue(Unit(units(0)));
  //  return worldIn;
  Vector<Double> worldIn;
  QuantumHolder qh;
  String error;
  if (!qh.fromRecord(error, recQ)) {
    *itsLog << LogIO::WARN
	    << "Failed to get quantity from input record because of "
	    << error << LogIO::POST;
    return worldIn;
  }
  if (qh.isQuantity()) {
    worldIn.resize(1);
    casa::Quantity q(qh.asQuantity());
    //worldIn[0]=q.getValue(q.getUnit());
    worldIn[0]=q.getValue(Unit(units(0)));
    return worldIn;
  } else {
    if (qh.isQuantumVectorDouble()) {
      Quantum<Vector<Double> > q(qh.asQuantumVectorDouble());
      uInt n = q.getValue().size();
      worldIn.resize(n);
      //for (uInt i=0; i < n; i++) worldIn[i]=(q.getValue(q.getUnit()))[i];
      for (uInt i=0; i < n; i++) worldIn[i]=(q.getValue(Unit(units(i))))[i];
      return worldIn;
    } else {
      *itsLog << LogIO::WARN
	      << "Unexpected record input to quantumRecordToVectorDouble()"
	      << LogIO::POST;
    }
  }
  return worldIn;
}


Vector<Double> 
coordsys::measuresToWorldVector (const RecordInterface& rec) const
  //
  // Units are converted to those of the CoordinateSystem
  // The record may contain one or more measures
  // Missing values are given the referenceValue
  //
{
  *itsLog << LogOrigin("coordsys", "measuresToVector(...)");

  // The record will have fields from 'direction', 'spectral',
  // 'stokes', 'linear', 'tabular'
  Int ic, afterCoord;
  Vector<Double> world(itsCoordSys->referenceValue().copy());
  String error;
  //
  if (rec.isDefined("direction")) {
    afterCoord = -1;
    ic = itsCoordSys->findCoordinate(Coordinate::DIRECTION, afterCoord);
    if (ic >=0) {
      Vector<Int> worldAxes = itsCoordSys->worldAxes(ic);
      const RecordInterface& rec2 = rec.asRecord("direction");
      MeasureHolder h;
      if (!h.fromRecord(error, rec2)) {
	*itsLog << error << LogIO::EXCEPTION;
      }
      //
      MDirection d = h.asMDirection();
      const DirectionCoordinate dc = itsCoordSys->directionCoordinate (ic);
      Vector<String> units = dc.worldAxisUnits();
      const MVDirection mvd = d.getValue();
      Quantum<Double> lon = mvd.getLong(Unit(units(0)));
      Quantum<Double> lat = mvd.getLat(Unit(units(1)));
      // Fill output
      world(worldAxes(0)) = lon.getValue();
      world(worldAxes(1)) = lat.getValue();
    } else {
      *itsLog << LogIO::WARN
	      << "There is no direction coordinate in this Coordinate System"
	      << endl;
      *itsLog << LogIO::WARN
	      << "However, the world record you are converting contains "
	      << endl;
      *itsLog << LogIO::WARN << "a direction field.  " << LogIO::POST;
    }
  }
  //
  if (rec.isDefined("spectral")) {
    afterCoord = -1;
    ic = itsCoordSys->findCoordinate(Coordinate::SPECTRAL, afterCoord);
    if (ic >=0) {
      Vector<Int> worldAxes = itsCoordSys->worldAxes(ic);
      const RecordInterface& rec2 = rec.asRecord("spectral");
      if (rec2.isDefined("frequency")) {
	const RecordInterface& rec3 = rec2.asRecord("frequency");
	MeasureHolder h;
	if (!h.fromRecord(error, rec3)) {
	  *itsLog << error << LogIO::EXCEPTION;
	}
	//
	MFrequency f = h.asMFrequency();
	const SpectralCoordinate sc = itsCoordSys->spectralCoordinate (ic);
	Vector<String> units = sc.worldAxisUnits();
	world(worldAxes(0)) = f.get(units(0)).getValue();
      } else {
	*itsLog << "This spectral record does not contain a frequency field"
		<< LogIO::EXCEPTION;
      }
    } else {
      *itsLog << LogIO::WARN
	      << "There is no spectral coordinate in this Coordinate System"
	      << endl;
      *itsLog << LogIO::WARN
	      << "However, the world record you are converting contains "
	      << endl;
      *itsLog << LogIO::WARN << "a spectral field.  " << LogIO::POST;
    }
  }
  //
  if (rec.isDefined("stokes")) {
    afterCoord = -1;
    ic = itsCoordSys->findCoordinate(Coordinate::STOKES, afterCoord);
    if (ic >=0) {
      Vector<Int> worldAxes = itsCoordSys->worldAxes(ic);
      Stokes::StokesTypes type = Stokes::type(rec.asString("stokes"));
      const StokesCoordinate sc = itsCoordSys->stokesCoordinate (ic);
      //
      Int pix;
      Vector<Double> p(1), w(1);
      if (!sc.toPixel (pix, type)) {
	*itsLog << sc.errorMessage() << LogIO::EXCEPTION;
      } else {
	p(0) = pix;
	if (!sc.toWorld (w, p)) {
	  *itsLog << sc.errorMessage() << LogIO::EXCEPTION;
	}
      }
      //
      world(worldAxes(0)) = w(0);
    } else {
      *itsLog << LogIO::WARN
	      << "There is no stokes coordinate in this CoordinateSystem"
	      << endl;
      *itsLog << LogIO::WARN
	      << "However, the world record you are converting contains "
	      << endl;
      *itsLog << LogIO::WARN << "a stokes field.  " << LogIO::POST;
    }
  }
  //
  if (rec.isDefined("linear")) {
    afterCoord = -1;
    ic = itsCoordSys->findCoordinate(Coordinate::LINEAR, afterCoord);
    if (ic >=0) {
      Vector<Int> worldAxes = itsCoordSys->worldAxes(ic);
      const LinearCoordinate lc = itsCoordSys->linearCoordinate (ic);
      Vector<Double> w =
	quantumVectorRecordToVectorDouble (rec.asRecord("linear"),
					   lc.worldAxisUnits());
      //
      for (uInt i=0; i<w.nelements(); i++) {
	world(worldAxes(i)) = w(i);
      }
    } else {
      *itsLog << LogIO::WARN
	      << "There is no linear coordinate in this CoordinateSystem"
	      << endl;
      *itsLog << LogIO::WARN
	      << "However, the world record you are converting contains "
	      << endl;
      *itsLog << LogIO::WARN << "a linear field.  " << LogIO::POST;
    }
  }
  //
  if (rec.isDefined("tabular")) {
    afterCoord = -1;
    ic = itsCoordSys->findCoordinate(Coordinate::TABULAR, afterCoord);
    if (ic >=0) {
      Vector<Int> worldAxes = itsCoordSys->worldAxes(ic);
      QuantumHolder h;
      String error;
      if (!h.fromRecord(error, rec.asRecord("tabular"))) {
	*itsLog << error << LogIO::EXCEPTION;
      }
      //
      const TabularCoordinate tc = itsCoordSys->tabularCoordinate (ic);
      String units = tc.worldAxisUnits()(0);
      Quantum<Double> q = h.asQuantumDouble();
      //
      world(worldAxes(0)) = q.getValue(Unit(units));
    } else {
      *itsLog << LogIO::WARN
	      << "There is no tabular coordinate in this Coordinate System"
	      << endl;
      *itsLog << LogIO::WARN
	      << "However, the world record you are converting contains "
	      << endl;
      *itsLog << LogIO::WARN << "a tabular field.  " << LogIO::POST;
    }
  }
  //
  //
  return world;
}

Vector<Double> 
coordsys::stringToWorldVector (LogIO& os, const Vector<String>& world,
			       const Vector<String>& units) const{
  Vector<Double> world2 = itsCoordSys->referenceValue();
  Int coordinate, axisInCoordinate;
  const uInt nIn = world.nelements();
  for (uInt i=0; i<nIn; i++) {
    itsCoordSys->findWorldAxis(coordinate, axisInCoordinate, i);
    Coordinate::Type type = itsCoordSys->type(coordinate);
    //
    if (type==Coordinate::DIRECTION) {
      Quantum<Double> val;
      if (!MVAngle::read(val, world(i))) {
	os << "Failed to convert string formatted world direction "
	   << world << " to double"  << LogIO::EXCEPTION;
      }
      world2(i) = val.getValue(Unit(units(i)));
    } else if (type==Coordinate::STOKES) {
      Stokes::StokesTypes type2 = Stokes::type(world(i));
      world2(i) = StokesCoordinate::toWorld(type2);
    } else {
      Quantum<Double> val;
      if (!Quantum<Double>::read(val, world(i))) {
	os << "Failed to convert string formatted world "
	   << world << " to double"  << LogIO::EXCEPTION;
      }
      world2(i) = val.getValue(Unit(units(i)));
    }
  }
  //
  return world2;
}

void coordsys::copyWorldAxes (Vector<Double>& out,
			      const Vector<Double>& in, Int c) const
{
  Vector<Int> worldAxes = itsCoordSys->worldAxes(c);
  for (uInt i=0; i<worldAxes.nelements(); i++) {
    out(worldAxes(i)) = in(i);
  }
}

void coordsys::trim (Vector<Double>& inout,
                     const Vector<Double>& replace) const
{
  const Int nIn = inout.nelements();
  const Int nOut = replace.nelements();
  Vector<Double> out(nOut);
  for (Int i=0; i<nOut; i++) {
    if (i > nIn-1) {
      out(i) = replace(i);
    } else {
      out(i) = inout(i);
    }
  }
  inout.resize(nOut);
  inout = out;
}

void coordsys::setDirectionCode (const String& code, Bool adjust)
{
  *itsLog << LogOrigin("coordsys", "setDirectionCode");

  // Exception if type not found
  Int ic = findCoordinate (Coordinate::DIRECTION, True);
  // Convert type
  String code2 = code;
  MDirection::Types typeTo;
  code2.upcase();
  if (!MDirection::getType(typeTo, code2)) {
    *itsLog << "Invalid direction code '" << code
	    << "' given. Allowed are : " << endl;
    for (uInt i=0; i<MDirection::N_Types; i++)
      *itsLog << "  " << MDirection::showType(i) << endl;
    *itsLog << LogIO::EXCEPTION;
  }

  // Bug out if nothing to do
  DirectionCoordinate
    dirCoordFrom(itsCoordSys->directionCoordinate(ic));  // Copy
  if (dirCoordFrom.directionType() == typeTo) return;
  Vector<String> unitsFrom = dirCoordFrom.worldAxisUnits();
  //
  Vector<String> radUnits(2);
  radUnits = String("rad");
  if (!dirCoordFrom.setWorldAxisUnits(radUnits)) {
    *itsLog << "Failed to set radian units for DirectionCoordinate"
	    << LogIO::EXCEPTION;
  }

  // Create output DirectionCoordinate
  Vector<Double> refValFrom = dirCoordFrom.referenceValue();
  Vector<Double> refPixFrom = dirCoordFrom.referencePixel();
  Vector<Double> incrFrom = dirCoordFrom.increment();
  DirectionCoordinate dirCoordTo (typeTo, dirCoordFrom.projection(),
				  refValFrom(0), refValFrom(1),
				  incrFrom(0), incrFrom(1),
				  dirCoordFrom.linearTransform(),
				  refPixFrom(0), refPixFrom(1));
  //
  if (adjust) {
    MDirection::Convert machine;
    const ObsInfo& obsInfo = itsCoordSys->obsInfo();
    Bool madeMachine =
      CoordinateUtil::makeDirectionMachine(*itsLog, machine, dirCoordTo,
					   dirCoordFrom, obsInfo, obsInfo);
    //      cerr << "made DirectionMachine = " << madeMachine << endl;
    //
    if (madeMachine) {
      MVDirection mvdTo, mvdFrom;
      Bool ok = dirCoordFrom.toWorld (mvdFrom, refPixFrom);
      if (ok) {
	mvdTo = machine(mvdFrom).getValue();
	Vector<Double> referenceValueTo(2);
	referenceValueTo(0) = mvdTo.getLong();
	referenceValueTo(1) = mvdTo.getLat();
	if (!dirCoordTo.setReferenceValue(referenceValueTo)) {
	  *itsLog << dirCoordTo.errorMessage() << LogIO::EXCEPTION;
	}
	if (!dirCoordTo.setWorldAxisUnits(unitsFrom)) {
	  *itsLog << dirCoordTo.errorMessage() << LogIO::EXCEPTION;
	}
      }
    }
  }
  //
  itsCoordSys->replaceCoordinate(dirCoordTo, ic);
}

void coordsys::setSpectralCode (const String& code, Bool adjust)
{
  // Exception if type not found
  Int ic = findCoordinate (Coordinate::SPECTRAL, True);
  // Convert type String to enum
  *itsLog << LogOrigin("coordsys", "setSpectralCode");
  MFrequency::Types typeTo;
  String code2 = code;
  code2.upcase();
  if (!MFrequency::getType(typeTo, code2)) {
    *itsLog << "Invalid frequency code '" << code
	    << "' given. Allowed are : " << endl;
    for (uInt i=0; i<MFrequency::N_Types; i++)
      *itsLog << "  " << MFrequency::showType(i) << endl;
    *itsLog << LogIO::EXCEPTION;
  }

  // Get Spectral Coordinate
  SpectralCoordinate specCoordTo(itsCoordSys->spectralCoordinate(ic));  // Copy

  // Bug out if nothing to do
  if (specCoordTo.frequencySystem() == typeTo) return;

  // Set new value
  specCoordTo.setFrequencySystem(typeTo);

  // Now adjust reference value if adjust is required
  if (adjust) {
    // Generate to/from Coordinate and CoordinateSystem and set new type
    const CoordinateSystem& cSysFrom = *itsCoordSys;
    const SpectralCoordinate specCoordFrom(cSysFrom.spectralCoordinate(ic));
    //
    CoordinateSystem cSysTo(cSysFrom);
    cSysTo.replaceCoordinate(specCoordTo, ic);
    //
    MFrequency::Convert machine;
    CoordinateUtil::makeFrequencyMachine(*itsLog, machine, ic, ic,
					 cSysTo, cSysFrom);
    //
    if (!machine.isNOP()) {
      MVFrequency mvfTo, mvfFrom;
      Vector<Double> refPixFrom = specCoordFrom.referencePixel();
      Bool ok = specCoordFrom.toWorld (mvfFrom, refPixFrom(0));
      if (ok) {
	mvfTo = machine(mvfFrom).getValue();
	Vector<Double> refValTo = specCoordTo.referenceValue();
	refValTo(0) = mvfTo.getValue();
	//
	Vector<String> unitsTo(1);
	unitsTo = String("Hz");
	if (!specCoordTo.setWorldAxisUnits(unitsTo)) {
	  *itsLog << specCoordTo.errorMessage() << LogIO::EXCEPTION;
	}
	if (!specCoordTo.setReferenceValue(refValTo)) {
	  *itsLog << specCoordTo.errorMessage() << LogIO::EXCEPTION;
	}
	if (!specCoordTo.setWorldAxisUnits(specCoordFrom.worldAxisUnits()))
	  {
	    *itsLog << specCoordTo.errorMessage() << LogIO::EXCEPTION;
	  }
      }
    }
  }

  // Replace coordinate in CoordinateSystem
  itsCoordSys->replaceCoordinate(specCoordTo, ic);
}

Record coordsys::toWorldRecord (const Vector<Double>& pixel,
                                const String& format)
{
  *itsLog << LogOrigin("coordsys", "toWorld");
  //
  Vector<Double> pixel2 = pixel.copy();
  //  if (pixel2.nelements()>0) pixel2 -= 1.0;        // 0-rel
  trim(pixel2, itsCoordSys->referencePixel());

  // Convert to world
  Vector<Double> world;
  Record rec;
  if (itsCoordSys->toWorld (world, pixel2)) {
    Bool isAbsolute = True;
    Bool showAsAbsolute = True;
    Int c = -1;
    rec = worldVectorToRecord (world, c, format, isAbsolute, showAsAbsolute);
  } else {
    *itsLog << itsCoordSys->errorMessage() << LogIO::EXCEPTION;
  }
  return rec;
}

Record coordsys::absRelRecord (LogIO& os, const RecordInterface& recIn,
			       Bool isWorld, Bool absToRel)
{
  Record recIn2;
  Vector<Double> value, value2;
  if (isWorld) {
    String format;
    Int c = -1;
    recordToWorldVector (value, format, c, recIn);
    Bool isAbsolute = False;
    if (absToRel) {
      trim(value, itsCoordSys->referenceValue());
      itsCoordSys->makeWorldRelative (value);
      isAbsolute = False;
    } else {
      Vector<Double> zero(itsCoordSys->nWorldAxes(),0.0);
      trim(value, zero);
      itsCoordSys->makeWorldAbsolute (value);
      isAbsolute = True;
    }
    //
    Bool showAsAbsolute = isAbsolute;
    recIn2 = worldVectorToRecord (value, c, format,
				  isAbsolute, showAsAbsolute);
  } else {
    if (recIn.isDefined("numeric")) {
      value = recIn.asArrayDouble("numeric");
    } else {
      os << "Input does not appear to be a pixel coordinate" << LogIO::EXCEPTION;
    }
    if (absToRel) {
      //value -= 1.0;                      // make 0-rel
      trim(value, itsCoordSys->referencePixel());
      itsCoordSys->makePixelRelative (value);
    } else {
      Vector<Double> zero(itsCoordSys->nPixelAxes(),0.0);
      trim(value, zero);
      itsCoordSys->makePixelAbsolute (value);
      //value += 1.0;                      // make 1-rel
    }
    recIn2.define("numeric", value);
  }
  //
  return recIn2;
}

int
coordsys::isValueWorld(casac::variant& value, int shouldBeWorld,
		       Bool verbose)
{
  // value         - the value, may have attribute 'pw_type' to tell us
  //                 whether its world or pixel.  If not, we might be
  //                 able to work it out from the type
  // shouldBeWorld - the value is expected to be a world value. if unset (-1)
  //                 it means we rely on the attribute
  // return values 1:True, 0:False, -1:Fail
  {
    *itsLog << LogOrigin("coordsys", "isValueWorld");

    int fail = -1;

    if (value.type() == ::casac::variant::BOOLVEC) { // value is unset
      if (shouldBeWorld == -1) { // shouldBeWorld is unset
	*itsLog << LogIO::SEVERE
		<< "Cannot discern whether value is pixel or world"
		<< LogIO::POST;
	return fail;
      } else {
	return shouldBeWorld;
      }
    }

    Record *rec = 0;
    if(value.type() == ::casac::variant::RECORD) {
      rec = toRecord(value.asRecord());
    }

    //
    if (shouldBeWorld == -1) { // shouldBeWorld is unset
      if (rec && rec->isDefined("pw_type")) {
	if (rec->asString("pw_type") == "world") {
	  return 1;
	} else {
	  return 0;
	}
      } else {
	if (value.type() == ::casac::variant::STRING ||
	    value.type() == ::casac::variant::RECORD) {
	  return 1;
	} else {
	  *itsLog << LogIO::SEVERE
		  << "Cannot discern whether value is pixel or world"
		  << LogIO::POST;
	  return fail;
	}
      }
    } else {
      Bool sbw = shouldBeWorld;
      if (rec && rec->isDefined("pw_type")) {
	if (sbw && (rec->asString("pw_type")!="world")) {
	  if (verbose) {
	    *itsLog << LogIO::WARN
		    << "Value appears to be pixel but world over-ride will be honoured"
		    << LogIO::POST;
	  }
	} else if (!sbw  && (rec->asString("pw_type")!="pixel")) {
	  if (value.type() == ::casac::variant::STRING ||
	      value.type() == ::casac::variant::RECORD) {
	    *itsLog << LogIO::SEVERE
		<< "Value must be of numeric type to be a pixel coordinate"
		<< LogIO::POST;
	    return fail;
	  } else {
	    *itsLog << LogIO::WARN
		    << "Value appears to be world but pixel over-ride will be honoured."
		    << LogIO::POST;
	  }
	}
      } else {
	if (!sbw) {
	  if (value.type() == ::casac::variant::STRING) {
	    // We may be able to convert a string to numeric (as needed for pixel)
	    if (verbose) {
	      *itsLog << LogIO::WARN
		      << "Value appears to be world but pixel over-ride will be honoured"
		      << LogIO::POST;
	    }
	  } else if (value.type() == ::casac::variant::RECORD) {
	    // We can't convert these to numeric
	    *itsLog << LogIO::SEVERE
		    << "Value is a world coordinate (quantity/record), but pixel (numeric) expected"
		    << LogIO::POST;
	    return fail;
	  }
	}
      }
      return sbw;
    }
  }
}

Bool
coordsys::checkAbsRel(casac::variant& value, casa::Bool shouldBeAbs)
{
  *itsLog << LogOrigin("coordsys", "checkAbsRel");

  if (value.type() == ::casac::variant::RECORD) {
    //    ::casac::record crec = value.asRecord();
    Record *rec = toRecord(value.asRecord());
    if(rec->isDefined("ar_type")) {
      String s;
      Int fn = rec->fieldNumber("ar_type");
      if (rec->dataType(fn) == TpString) {
	rec->get(fn, s);
      }
      if (shouldBeAbs && !s.empty()) {
	if (s == "relative") {
	  *itsLog << LogIO::SEVERE
		  << "The value is relative, not absolute', origin='coordsys.checkAbsRel"
		  << LogIO::EXCEPTION;
	  return False;
	}
      } else {
	if (s == "absolute") {
	  *itsLog << LogIO::SEVERE
		  << "The value is absolute, not relative', origin='coordsys.checkAbsRel"
		  << LogIO::EXCEPTION;
	  return False;
	}
      }
    }
  }
  return True;
}

Record *
coordsys::coordinateValueToRecord(const ::casac::variant& value, Bool isWorld,
				  Bool isAbs, Bool first)
  //
  // This function looks at the type of value and fills a record
  // holding 'numeric', 'string', 'quantity', 'measure' with that
  // value.  The value may itself be a record holding any/all of these
  // fields (they must all be representations of the same world
  // coordinate)
  //
  // You can choose to return just the first value found (first=T) or
  // all of them in the case that value is a record
  //
  // isWorld says the value has been checked to be a world value, or
  // user over-ride specifies that it is world and can be converted to
  // world
  //
  // isAbs says the value is expected to be absolute, else relative.
  //
  // If value is unset, the reference pixel/value is used.  Otherwise,
  // missing axes are padded in the C++.
  //
{
  *itsLog << LogOrigin("coordsys", "coordinateValueToRecord");

  Record *rec = new Record();

  if (value.type() == ::casac::variant::BOOLVEC) { // value is unset
    if (isWorld) {
      if (isAbs) {
	Vector<Double> refVal;
	refVal = itsCoordSys->referenceValue();
	Int c = -1;
	String format("n");
	Bool isAbsolute = True;
	Bool showAsAbsolute = True;
	*rec = worldVectorToRecord (refVal, c, format, isAbsolute, showAsAbsolute);
	rec->define("pw_type", "world");
	rec->define("ar_type", "absolute");
      } else {
	int naxes = itsCoordSys->nWorldAxes();
	Array<Float> arr(IPosition(naxes),0.0);
	rec->define(RecordFieldId("numeric"), arr);
      }
    } else {
      if (isAbs) {
	Vector<Double> crpix = itsCoordSys->referencePixel();
	rec->define(RecordFieldId("numeric"), crpix);
	rec->define("pw_type","pixel");
	rec->define("ar_type","absolute");
      } else {
	int naxes = itsCoordSys->nPixelAxes();
	Array<Float> arr(IPosition(naxes),0.0);
	rec->define(RecordFieldId("numeric"), arr);
      }
    }
    return rec;
  }
  ::casac::variant tmpv(value);
  if (value.type() == ::casac::variant::DOUBLE) {
    rec->define(RecordFieldId("numeric"), tmpv.asDouble());
    return rec;
  }
  if (value.type() == ::casac::variant::DOUBLEVEC) {
    Vector<Double> d = tmpv.asDoubleVec();
    rec->define(RecordFieldId("numeric"), d);
    return rec;
  }
  if (value.type() == ::casac::variant::INTVEC) {
    Vector<Double> d = tmpv.asDoubleVec();
    rec->define(RecordFieldId("numeric"), d);
    return rec;
  }
  if (value.type() == ::casac::variant::STRING) {
    if (isWorld) {
      // Don't split e.g. '1 km 20 m' into a vector.
      String s = tmpv.asString();
      Int maxn=50;
      String sa[maxn];
      String sep = " ";
      int n = split(s, sa, maxn, sep);
      if (n==1) {
	rec->define("qstring", s); // quantum string such as '4kHz'
      } else {
	Vector<String> sarr(n);
	for (int i=0; i < n; i++) sarr[i]=sa[i];
	RecordDesc rd;
	rd.addField("string", TpArrayString, IPosition(1,n));
	rec = new Record(rd);
	rec->define("string", sarr);
      }
    } else {
      // Convert to numeric e.g. "1 20" -> [1,20]
      //tovector(value, 'double');  NEEDS WORK HERE!!!
      //rec->define(RecordFieldId("numeric"),value);
    }
    return rec;
  }
  if (value.type() == ::casac::variant::STRINGVEC) {
    Vector<String> sarr = toVectorString(value.toStringVec());
    int n = sarr.size();
    RecordDesc rd;
    rd.addField("string", TpArrayString, IPosition(1,n));
    rec = new Record(rd);
    rec->define("string", sarr);
    return rec;
  }
  // variant doesn't support quantity yet
  //if (value.type() == ::casac::variant::QUANTITY) {
  //  if (!isWorld) {
  //    *itsLog << LogIO::SEVERE
  //            << "Pixel coordinate must be numeric" << LogIO::POST;
  //   return rec;
  //  }
  //  rec->define(RecordFieldId("quantity"),
  //    defaultcoordsyssupport.valuetovectorquantum(value, singlevector=T));
  //  if (is_fail(rec.quantity)) fail;
  //  return rec;
  //}
  if (value.type() == ::casac::variant::RECORD) {
    // Catch r := cs.toworld (value, 'nqms') style where r is a record
    // with fields including 'measure'
    Bool none = True;
    rec = toRecord(tmpv.asRecord());
    if (rec->isDefined("numeric")) {
      if (first) return rec;
      none = False;
    }
    if (rec->isDefined("measure")) {
      if (first) return rec;
      if (!isWorld) {
	*itsLog << LogIO::SEVERE
		<< "Pixel coordinate must be numeric not a measure"
		<< LogIO::POST;
	delete rec;
	rec = 0;
	return rec;
      }
      none = False;
    }
    if (rec->isDefined("quantity")) {
      if (first) return rec;
      if (!isWorld) {
	*itsLog << LogIO::SEVERE
		<< "Pixel coordinate must be numeric not a quantity"
		<< LogIO::POST;
	delete rec;
	rec = 0;
	return rec;
      }
      none = False;
    }
    if (rec->isDefined("string")) {
      if (first) return rec;
      if (!isWorld) {
	//           rec.numeric := dms.tovector(value.string, 'double');
	// NEEDS WORK HERE
      }
      none = False;
    }

    // Assumes the record is a measure...
    if (none) {
      if (!isWorld) {
	*itsLog << LogIO::WARN
		<< "Pixel coordinate must be numeric not a measure"
		<< LogIO::POST;
	delete rec;
	rec = 0;
	return rec;
      }
      RecordDesc rd;
      rd.addField("measure", TpRecord);
      rec = new Record(rd);
      rec->defineRecord("measure", *toRecord(tmpv.asRecord()));
    }
    return rec;

    /*
    // Assumes the record is a quantity
    Record tmpR = *toRecord(tmpv.asRecord());
    if (none) {
      if (!isWorld) {
	*itsLog << LogIO::SEVERE
		<< "Pixel coordinate must be numeric" << LogIO::POST;
	delete rec;
	rec = 0;
	return rec;
      }
      RecordDesc rd;
      rd.addField("quantum", TpRecord);
      rec = new Record(rd);
      rec->defineRecord("quantum", *toRecord(tmpv.asRecord()));
    }
    return rec;
    */
  }
  return rec;
}

} // casac namespace
