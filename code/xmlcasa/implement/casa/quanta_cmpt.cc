/***
 * Framework independent implementation file for quanta...
 *
 * This class makes the connection between Units and Quantity
 * classes and the distributed object system. It provides a
 * series of callable methods described in Aips++ Note 197.
 * Operations supported are mathematical (+ - * /), comparison
 * and conversion related.
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <iomanip>
#include <xmlcasa/casa/quanta_cmpt.h>

#include <casa/BasicSL/String.h>
#include <casa/Quanta/QC.h>
#include <casa/Utilities/MUString.h>
#include <casa/stdmap.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/UnitName.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/QMath.h>
#include <casa/Quanta/MVDoppler.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/MVFrequency.h>
#include <casa/Quanta/QLogical.h>

using namespace std;
using namespace casa;

namespace casac {

quanta::quanta()
{
  itsLog = new LogIO();
  UnitMap::putUser("pix",UnitVal(1.0), "pixel units");
}

quanta::~quanta()
{

}

casa::QuantumHolder
quanta::quantumHolderFromVar(const ::casac::variant& theVar){
  casa::QuantumHolder qh;
  try {
    String error;
    if(theVar.type()== ::casac::variant::STRING ) {
      if(!qh.fromString(error, theVar.toString())) {
	*itsLog << LogIO::SEVERE << "Error " << error
		<< " in converting quantity "<< LogIO::POST;
      }
    }
    if (theVar.type()== ::casac::variant::STRINGVEC){
      *itsLog << LogIO::WARN << "Only first vector element will be used."
	      << LogIO::POST;
      //      if(!qh.fromString(error, theVar.toStringVec()[0])) {
      if(!qh.fromString(error, theVar.toString())) {
	*itsLog << LogIO::SEVERE << "Error " << error
		<< " in converting quantity "<< LogIO::POST;
      }
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!qh.fromRecord(error, *ptrRec)){
	*itsLog << LogIO::SEVERE << "Error " << error
		<< " in converting quantity "<< LogIO::POST;
      }
      delete ptrRec;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return qh;
}

casac::record*
quanta::recordFromQuantity(const casa::Quantity q)
{
  ::casac::record *r=0;
  try {
    String error;
    casa::Record R;
    if (QuantumHolder(q).toRecord(error, R)) {
      r = fromRecord(R);
    } else {
      *itsLog << LogIO::SEVERE << "Could not convert quantity to record."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
	    << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

::casac::record*
quanta::recordFromQuantity(const Quantum<Vector<Double> >& q)
{
  ::casac::record *r=0;
  try {
    String error;
    casa::Record R;
    if (QuantumHolder(q).toRecord(error, R)) {
      r = fromRecord(R);
    } else {
      *itsLog << LogIO::SEVERE << "Could not convert quantity to record."
              << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
            << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

// setformat (t='', v=F) -> set specified format (e.g. 'prec') to value v
bool
quanta::setformat(const std::string& t, const std::string& v)
{
  *itsLog << LogIO::WARN << "setformat NOT IMPLEMENTED YET!" << LogIO::POST;
  return false;
}

//  getformat (t='prec') -> get specified format (e.g. 'prec')
std::string
quanta::getformat(const std::string& t)
{
  *itsLog << LogIO::WARN << "getformat NOT IMPLEMENTED YET!" << LogIO::POST;
  return "F";
}

// form.x(v) -> format value acoording to type x
std::string
quanta::formxxx(const ::casac::variant& v, const std::string& format)
{
  string out("");
  try {
    casa::Quantity quant = casaQuantity(v);

    if(!ang_as_formatted_str(out, quant, format))
      *itsLog << LogIO::WARN << "Don't understand " << format << LogIO::POST;
  }
  catch(AipsError x){
    RETHROW(x);
  }
  return out;
}

// convertfreq (v='1Hz', out='Hz') -> convert frequency to units as in out
::casac::record*
quanta::convertfreq(const ::casac::variant& v, const std::string& outunit)
{
  return recordFromQuantity(MVFrequency(casaQuantity(v)).get(Unit(outunit)));
}

// convertdop (v='0km/s', out='km/s') -> convert doppler to units as in out
::casac::record*
quanta::convertdop(const ::casac::variant& v, const std::string& outunit)
{
  return recordFromQuantity(MVDoppler(casaQuantity(v)).get(Unit(outunit)));
}

// quantity from value v and units name
::casac::record*
quanta::quantity(const ::casac::variant& v, const std::string& unitname)
{
  if(v.type()==::casac::variant::DOUBLE) {
    return recordFromQuantity(casa::Quantity(v.toDouble(),String(unitname)));
  }
  if(v.type()==::casac::variant::DOUBLEVEC) {
    return recordFromQuantity(Quantum<Vector<Double> >(v.toDoubleVec(),
						       String(unitname)));
  }
  if(v.type()==::casac::variant::INT) {
    return recordFromQuantity(casa::Quantity(v.toDouble(),String(unitname)));
  }
  if(v.type()==::casac::variant::INTVEC) {
    return recordFromQuantity(Quantum<Vector<Double> >(v.toDoubleVec(),
						       String(unitname)));
  }
  QuantumHolder qh = quantumHolderFromVar(v);
  if (qh.isQuantumVectorDouble() ) {
    Quantum<Vector<Double> > qv = qh.asQuantumVectorDouble();    
    return recordFromQuantity(qv);
  } else {
    return recordFromQuantity(casaQuantity(v));
  }
}

//::casac::record*
//quanta::quant(const double v, const std::string& name)
//{
//  return recordFromQuantity(casa::Quantity(v,String(name)));
//}

//::casac::record* 
//quanta::quantv(const std::vector<double>& v, const std::string& name)
//{
//  return recordFromQuantity(Quantum<Vector<Double> >(v,String(name)));
//}

// get value of quantity or quantity array
std::vector<double>
quanta::getvalue(const ::casac::variant& v)
{
  if(v.type()==::casac::variant::RECORD) {
    QuantumHolder qh = quantumHolderFromVar(v);
    if (qh.isQuantumVectorDouble() ) {
      Quantum<Vector<Double> > qv = qh.asQuantumVectorDouble();
      Vector<Double> vd = qv.getValue();
      int n = vd.size();
      std::vector<double> svd(n);
      for (int i=0; i < n; i++) svd[i]=vd[i];
      return svd;
    }
  }
  return casacQuantity(casaQuantity(v)).value;
  /*
  std::vector<double> rtn;
  Vector<casa::Quantity> vq;
  if(toCasaVectorQuantity(v, vq)) {
    uInt n = vq.size();
    rtn.resize(n);
    for (uInt i=0; i<n; i++) rtn[i]=vq[i].getValue();
  }
  return rtn;
  */
}

// Get unit of quanta or quanta array
std::string
quanta::getunit(const ::casac::variant& v)
{
  if(v.type()==::casac::variant::RECORD) {
    QuantumHolder qh = quantumHolderFromVar(v);
    if (qh.isQuantumVectorDouble() ) {
      Quantum<Vector<Double> > qv = qh.asQuantumVectorDouble();
      return qv.getFullUnit().getName();
    }
  }
  return casacQuantity(casaQuantity(v)).units;
  //return casacQuantity(casaQuantity(v)).units;
}

// canonical/canon (v='1') -> canonical value of v
::casac::record*
quanta::canonical(const ::casac::variant& v)
{
  return recordFromQuantity(casaQuantity(v).get());
}

::casac::record*
quanta::canon(const ::casac::variant& v)
{
  //  return recordFromQuantity(casa::Quantity(1.,String(v)));
  return recordFromQuantity(casaQuantity(v).get());
}

// convert (v='1', out='') -> convert unit v to units as in out
::casac::record*
quanta::convert(const ::casac::variant& v, const ::casac::variant& outunit)
{
  // Strangely, this cannot be declared const because of the possible
  // .asQuantumVectorDouble() calls.
  QuantumHolder qh(quantumHolderFromVar(v));

  const Unit outU(casaQuantity(outunit).getUnit());

  if (qh.isQuantumVectorDouble()) {
    return recordFromQuantity(outU.empty() ? qh.asQuantumVectorDouble().get() :
                              qh.asQuantumVectorDouble().get(outU));
  } else {
    return recordFromQuantity(outU.empty() ? casaQuantity(v).get() :
			      casaQuantity(v).get(outU));
  }
}

// define name as new unit with value v
bool
quanta::define(const std::string& name, const ::casac::variant& v)
{
  const casa::Quantity q(casaQuantity(v));  // Don't repeatedly parse v.
  
  UnitMap::removeUser(name);
  UnitMap::putUser(name, UnitVal(q.getValue(), q.getUnit()), "User defined");
  return true;
}

// list known units
// map (v='all') produce record of strings of specified type of units
std::string
quanta::map(const std::string& v)
{
  ostringstream oss;

  if (v == "Constants" || v == "constants" || v == "const" ||
          v == "Const") {
    oss << "== Constants ====\n";
    const uInt N = 20;
    static String types[N] = {
      "pi    3.14..           ",
      "ee    2.71..           ",
      "c     light vel.       ",
      "G     grav. const      ",
      "h     Planck const     ",
      "HI    HI line          ",
      "R     gas const        ",
      "NA    Avogadro #       ",
      "e     electron charge  ",
      "mp    proton mass      ",
      "mp_me mp/me            ",
      "mu0   permeability vac.",
      "eps0  permittivity vac.",
      "k     Boltzmann const  ",
      "F     Faraday const    ",
      "me    electron mass    ",
      "re    electron radius  ",
      "a0    Bohr\'s radius   ",
      "R0    solar radius     ",
      "k2    IAU grav. const^2"
    };
    static casa::Quantity res[N] = {
      casa::Quantity(C::pi,""), casa::Quantity(C::e,""),
      QC::c, QC::G, QC::h, QC::HI, QC::R, QC::NA, QC::e, QC::mp,
      QC::mp_me, QC::mu0, QC::epsilon0, QC::k, QC::F, QC::me, QC::re, QC::a0,
      QC::R0, QC::k2
    };
    for (uInt i = 0; i < N; i++)
      oss << types[i] << "\t\t" << res[i] << "\n";
  } else {
    const uInt N = 5;
    static String types[N] = {"all", "Prefix", "SI", "Custom", "User"};

    uInt p = MUString::minimaxNC(v, N, types);

    if (p >= N) {
      oss << "Unknown mapping requested" << endl;
    } else {
      if (p == 0 || p == 1) {
	oss << "== Prefix ====\n";
	UnitMap::listPref(oss);
      }
      if (p == 0 || p == 2) {
	oss << "== SI ====\n";
	UnitMap::listSI(oss);
      }
      if (p == 0 || p == 3) {
	oss << "== Custom ====\n";
	UnitMap::listCust(oss);
      }
      if (p == 0 || p == 4) {
	oss << "== User ====\n";
	UnitMap::listUser(oss);
      }
    };
  }
  return oss.str();
}

// list known units
// map (v='all') produce record of strings of specified type of units
::casac::record*
quanta::maprec(const std::string& v)
{
  return fromRecord(quanta::mapit(v));
}

// define FITS related units
bool
quanta::fits()
{
  UnitMap::addFITS();
  return true;
}

// output in angle format
// angle (v, prec=0, form="") -> formatted string of angle/time unit
std::vector<std::string>
quanta::angle(const ::casac::variant& v, const int prec, const std::vector<std::string>& form, const bool showform)
{
  //  Quantum<Vector<Double> > val(Vector<Double>(v.value), String(v.units));
  casa::Quantity q = casaQuantity(v);
  Vector<Double> vd(1);
  vd[0]=q.getValue();
  Quantum<Vector<Double> > val( vd, q.getUnit());

  Vector<Int> shape(1); // need to simplify code if we can assume 1-dim!!!
  shape[0] = (val.getValue()).size();
  Vector<String> fmt = toVectorString(form);
  Vector<String> returnval;

  Int fm = quanta::makeFormA(fmt);
  Int nelem = val.getValue().nelements();
  if (nelem > 0) {
    Int nrow = shape(shape.nelements()-1);
    Int ncol = nelem/nrow;
    returnval.resize(nrow);
    Int k = 0;
    for (Int i=0; i<nrow; i++) {
      ostringstream oss;
      if (ncol > 1 && showform) oss << '[';
      for (Int j=0; j<ncol; j++) {
	if (j>0) {
	  if (showform) oss << ", ";
	  else oss << " ";
	};
	oss << MVAngle(casa::Quantity(val.getValue()(k), val.getFullUnit())).string(fm, prec);
	k++;
      };
      if (ncol > 1 && showform) oss << ']';
      returnval(i) = oss.str();
    };
  } else {
    returnval.resize(0);
  };
  return fromVectorString(returnval);
}

// output in time format
// time (v, prec=0, form="") -> formatted string of time/angle unit
std::vector<std::string>
quanta::time(const ::casac::variant& v, const int prec,
	     const std::vector<std::string>& form, const bool showform)
{
  //  Quantum<Vector<Double> > val(Vector<Double>(v.value), String(v.units));
  casa::Quantity q = casaQuantity(v);
  Vector<Double> vd(1);
  vd[0]=q.getValue();
  Quantum<Vector<Double> > val( vd, q.getUnit());

  Vector<Int> shape(1);   // need to simplify code if we can assume 1-dim!!!
  shape[0] = (val.getValue()).size();
  Vector<String> fmt = toVectorString(form);
  Vector<String> returnval;

  Int fm = quanta::makeFormT(fmt);
  Int nelem = val.getValue().nelements();
  if (nelem > 0) {
    Int nrow = shape(shape.nelements()-1);
    Int ncol = nelem/nrow;
    returnval.resize(nrow);
    Int k = 0;
    for (Int i=0; i<nrow; i++) {
      ostringstream oss;
      if (ncol > 1 && showform) oss << '[';
      for (Int j=0; j<ncol; j++) {
	if (j>0) {
	  if (showform) oss << ", ";
	  else oss << " ";
	};
	oss << MVTime(casa::Quantity(val.getValue()(k),
				     val.getFullUnit())).
	  string(fm, prec);
	k++;
      };
      if (ncol > 1 && showform) oss << ']';
      returnval(i) = oss.str();
    };
  } else {
    returnval.resize(0);
  };
  return fromVectorString(returnval);
}

::casac::record*
quanta::add(const ::casac::variant& v, const ::casac::variant& a)
{
  return recordFromQuantity(casaQuantity(v) + casaQuantity(a));
}

::casac::record*
quanta::sub(const ::casac::variant& v, const ::casac::variant& a)
{
  return recordFromQuantity(casaQuantity(v) - casaQuantity(a));
}

::casac::record*
quanta::mul(const ::casac::variant& v, const ::casac::variant& a)
{
  return recordFromQuantity(casaQuantity(v) * casaQuantity(a));
}

::casac::record*
quanta::div(const ::casac::variant& v, const ::casac::variant& a)
{
  return recordFromQuantity(casaQuantity(v) / casaQuantity(a));
}

// negate
::casac::record*
quanta::neg(const ::casac::variant& v)
{
  return recordFromQuantity(casa::Quantity(-casaQuantity(v).getValue(),
					   casaQuantity(v).getUnit()));
}

// normalise angle
::casac::record*
quanta::norm(const ::casac::variant& v, const double a)
{
  return recordFromQuantity(casa::Quantity(MVAngle(casaQuantity(v))(a).degree(), "deg"));
}

bool
quanta::le(const ::casac::variant& v, const ::casac::variant& a)
{
  return (casaQuantity(v) <= casaQuantity(a));
}

bool
quanta::lt(const ::casac::variant& v, const ::casac::variant& a)
{
  return (casaQuantity(v) < casaQuantity(a));
}

bool
quanta::eq(const ::casac::variant& v, const ::casac::variant& a)
{
  return (casaQuantity(v) == casaQuantity(a));
}

bool
quanta::ne(const ::casac::variant& v, const ::casac::variant& a)
{
  return (casaQuantity(v) != casaQuantity(a));
}

bool
quanta::gt(const ::casac::variant& v, const ::casac::variant& a)
{
  return (casaQuantity(v) > casaQuantity(a));
}

bool
quanta::ge(const ::casac::variant& v, const ::casac::variant& a)
{
  return (casaQuantity(v) >= casaQuantity(a));
}

::casac::record*
quanta::sin(const ::casac::variant& v)
{
  return recordFromQuantity(casa::sin(casaQuantity(v)));
}

::casac::record*
quanta::cos(const ::casac::variant& v)
{
  return recordFromQuantity(casa::cos(casaQuantity(v)));
}

::casac::record*
quanta::tan(const ::casac::variant& v)
{
  return recordFromQuantity(casa::tan(casaQuantity(v)));
}

::casac::record*
quanta::asin(const ::casac::variant& v)
{
  return recordFromQuantity(casa::asin(casaQuantity(v)));
}

::casac::record*
quanta::acos(const ::casac::variant& v)
{
  return recordFromQuantity(casa::acos(casaQuantity(v)));
}

::casac::record*
quanta::atan(const ::casac::variant& v)
{
  return recordFromQuantity(casa::atan(casaQuantity(v)));
}

::casac::record*
quanta::atan2(const ::casac::variant& v, const ::casac::variant& a)
{
  return recordFromQuantity(casa::atan2(casaQuantity(v), casaQuantity(a)));
}

::casac::record*
quanta::abs(const ::casac::variant& v)
{
  return recordFromQuantity(casa::abs(casaQuantity(v)));
}

::casac::record*
quanta::ceil(const ::casac::variant& v)
{
  return recordFromQuantity(casa::ceil(casaQuantity(v)));
}

::casac::record*
quanta::floor(const ::casac::variant& v)
{
  return recordFromQuantity(casa::floor(casaQuantity(v)));
}

::casac::record*
quanta::log(const ::casac::variant& v)
{
  return recordFromQuantity(casa::log(casaQuantity(v)));
}

::casac::record*
quanta::log10(const ::casac::variant& v)
{
  return recordFromQuantity(casa::log10(casaQuantity(v)));
}

::casac::record*
quanta::exp(const ::casac::variant& v)
{
  return recordFromQuantity(casa::exp(casaQuantity(v)));
}

::casac::record*
quanta::sqrt(const ::casac::variant& v)
{
  return recordFromQuantity(casa::sqrt(casaQuantity(v)));
}

// compare units for conformance
// T if unit dimensions of v and a equal
bool
quanta::compare(const ::casac::variant& v, const ::casac::variant& a)
{
  return ( (casaQuantity(v).getFullUnit().getValue()) ==
	   (casaQuantity(a).getFullUnit().getValue()) );
}

bool
quanta::qcompare(const ::casac::variant& v, const casa::Quantity a)
{
  return ( (casaQuantity(v).getFullUnit().getValue()) ==
	   (a.getFullUnit().getValue()) );
}


// check if correct units
bool
quanta::check(const std::string& v)
{
  bool retval(false);
  casa::Quantity res;
  if (casa::Quantity::read(res, v)) {
    retval = true;
  }
  return retval;
}

// checkfreq - true if cm is proper frequency unit definition
bool
quanta::checkfreq(const ::casac::variant& cm)
{
  return (qcompare(cm, casa::Quantity(1.0,"s")) ||
	  qcompare(cm, casa::Quantity(1.0,"Hz")) ||
	  qcompare(cm, casa::Quantity(1.0,"deg/s")) ||
	  qcompare(cm, casa::Quantity(1.0,"m")) ||
	  qcompare(cm, casa::Quantity(1.0,"m-1")) ||
	  qcompare(cm, casa::Quantity(1.0,"(eV)")) ||
	  qcompare(cm, casa::Quantity(1.0,"kg.m"))
	  );
}

// raise to power
::casac::record*
quanta::pow(const ::casac::variant& v, const int a)
{
  return recordFromQuantity(casa::pow(casaQuantity(v),a));
}

// get a constant
::casac::record*
quanta::constants(const std::string& v)
{
  String in(v);
  const uInt N = 20;
  String str;
  static String types[N] = {
    "pi", "ee", "c", "G", "h", "HI", "R", "NA", "e", "mp",
    "mp_me", "mu0", "epsilon0", "k", "F", "me", "re", "a0",
    "R0", "k2"
  };
  static casa::Quantity res[N] = {
    casa::Quantity(C::pi,""), casa::Quantity(C::e,""),
    QC::c, QC::G, QC::h, QC::HI, QC::R, QC::NA, QC::e, QC::mp,
    QC::mp_me, QC::mu0, QC::epsilon0, QC::k, QC::F, QC::me, QC::re, QC::a0,
    QC::R0, QC::k2
  };
  uInt p = MUString::minimaxNC(in, N, types);
  if (p >= N ) return recordFromQuantity(casa::Quantity(0.,""));
  return recordFromQuantity(res[p]);
}

// isangle - check if angle
bool
quanta::isangle(const ::casac::variant& v)
{
  return (//check(v) && 
	  (qcompare(v, casa::Quantity(1.0,"s")) ||
	   qcompare(v, casa::Quantity(1.0,"rad"))));
}

// convert angle to time
::casac::record*
quanta::totime(const ::casac::variant& v)
{
  ::casac::record* retval = 0;
  if (casaQuantity(v).check(UnitVal::TIME)) {
    retval = recordFromQuantity(casaQuantity(v));
  } else {
    retval = recordFromQuantity(MVTime(casaQuantity(v)).get());
  };
  return retval;
}

// convert time to angle
::casac::record*
quanta::toangle(const ::casac::variant& v)
{
  ::casac::record* retval = 0;
  if (casaQuantity(v).check(UnitVal::ANGLE)) {
    retval = recordFromQuantity(casaQuantity(v));
  } else {
    retval = recordFromQuantity(MVAngle(casaQuantity(v)).get());
  };
  return retval;
}

// split time into many fields
::casac::record*
quanta::splitdate(const ::casac::variant& v)
{
  MVTime x(casaQuantity(v));
  Record rval;
  rval.define(RecordFieldId("mjd"), x.day());
  rval.define(RecordFieldId("year"), x.year());
  rval.define(RecordFieldId("yearday"), static_cast<Int>(x.yearday()));
  rval.define(RecordFieldId("month"),static_cast<Int>(x.month()));
  rval.define(RecordFieldId("monthday"),static_cast<Int>(x.monthday()));
  rval.define(RecordFieldId("week"),static_cast<Int>(x.yearweek()));
  rval.define(RecordFieldId("weekday"),static_cast<Int>(x.weekday()));
  Double y(fmod(x.day(), 1.0));
  rval.define(RecordFieldId("hour"),static_cast<Int>(y*24.0));
  y = fmod(y*24.0, 1.0);
  rval.define(RecordFieldId("min"),static_cast<Int>(y*60.0));
  y = fmod(y*60.0, 1.0);
  rval.define(RecordFieldId("sec"),static_cast<Int>(y*60.0));
  rval.define(RecordFieldId("s"),static_cast<Double>(y*60.0));
  y = fmod(y*60.0, 1.0);
  rval.define(RecordFieldId("msec"),static_cast<Int>(y*1000.0));
  rval.define(RecordFieldId("usec"),static_cast<Int>(y*1.0e6));
  return fromRecord(rval);
}

// Quantum to String
std::string
quanta::tos(const ::casac::variant& v, const int prec)
{
  std::string retval;

  ostringstream oss;
  oss << fixed << setprecision(prec)
      << casaQuantity(v).getValue() << casaQuantity(v).getUnit();
  retval = oss.str();

  return retval;
}

std::string
quanta::type()
{
  return "quanta";
}

bool
quanta::done(const bool kill)
{
  bool rstat(false);
  try {
    //    delete itsLog;
    //    itsLog=0;
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

// generate a quantity
// unit (v=1., name='') -> unit from string v; or from units name and value v
  //::casac::record* quanta::unit(const double v, const std::string& unitname)
::casac::record*
quanta::unit(const ::casac::variant& v, const std::string& unitname)
{
  return quantity(v, unitname);
}

bool
quanta::isquantity(const ::casac::variant& v)
{
  bool retval = false;
  try {
    casa::QuantumHolder qh;
    String error;
    if(v.type()== ::casac::variant::STRING ||
       v.type()== ::casac::variant::STRINGVEC){
      if(qh.fromString(error, v.toString())){
	retval=qh.isQuantity();
      }
    }
    if(v.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(v);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(qh.fromRecord(error, *ptrRec)){
	retval=qh.isQuantity();
      }
      delete ptrRec;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}


// Make list of known units
void quanta::mapInsert(Record &out,
                       const String &type,
                       const std::map<String, UnitName> &mp) {
  ostringstream osn;
  osn <<  mp.size();
  out.define(RecordFieldId(String("== ") + type + String(" ==")),
	     String("== ") + type +
	     String(" ==== ") + String(osn) + String(" ===="));
  for (std::map<String, UnitName>::const_iterator i=mp.begin();
       i != mp.end(); ++i) {
    ostringstream oss;
    oss << i->second;
    String str = oss.str();
    out.define(RecordFieldId(type + "_" + i->first), str);
  };
}

Record quanta::mapit(const String &tp) {
  const uInt N = 5;
  Record tmp;
  String str;
  static String types[N] = {
    "all", "Prefix", "SI", "Custom", "User"};

  uInt p = MUString::minimaxNC(tp, N, types);

  if (p >= N) {
    tmp.define(RecordFieldId(String("Error")),
	       String("Unknown mapping requested"));
  } else {
    if (p == 0 || p == 1)
      quanta::mapInsert(tmp, types[1], UnitMap::givePref());
    if (p == 0 || p == 2)
      quanta::mapInsert(tmp, types[2], UnitMap::giveSI());
    if (p == 0 || p == 3)
      quanta::mapInsert(tmp, types[3], UnitMap::giveCust());
    if (p == 0 || p == 4)
      quanta::mapInsert(tmp, types[4], UnitMap::giveUser());
  };
  return tmp;
}

Int quanta::makeFormT(const Vector<String> &in) {
  Int res = MVTime::giveMe("time");
  for (uInt i = 0; i<in.size(); i++) res |= MVTime::giveMe(in(i));
  return res;
}

Int quanta::makeFormA(const Vector<String> &in) {
  Int res = MVAngle::giveMe("angle");
  for (uInt i = 0; i<in.size(); i++) res |= MVAngle::giveMe(in(i));
  return res;
}

// dopcv - doppler value conversion
casa::Quantity
quanta::dopcv(const casa::Quantity val, const casa::Quantity arg) {
  casa::Quantity retval;
  try {
    retval = MVDoppler(val).get(arg.getFullUnit());
  } catch (AipsError (x)) {
    *itsLog << LogIO::SEVERE
	    << "Exception Reports: " << x.getMesg()
	    << "\nIllegal doppler type units specified"
	    << LogIO::POST;
  }
  return retval;
};

// frqcv - freq converter
casa::Quantity
quanta::frqcv(const casa::Quantity val, const casa::Quantity arg) {
  casa::Quantity retval;
  try {
    retval = MVFrequency(val).get(arg.getFullUnit());
  } catch (AipsError (x)) {
    *itsLog << LogIO::SEVERE
	    << "Exception Reports: " << x.getMesg()
	    << "\nIllegal doppler type units specified"
	    << LogIO::POST;
  }
  return retval;
};

// tfreq - table freq formatter
Vector<String>
quanta:: tfreq(const Quantum<Vector<Double> > &val, const Vector<Int> &arg,
	       const String &form, const Bool form2) {
  Vector<String> retval;

  Int nelem = val.getValue().nelements();
  Vector<Double> x(val.getValue());
  casa::Quantity y;
  Unit inun(val.getFullUnit());
  try {
    Unit outun(form);
    for (Int i=0; i<nelem; i++) {
      y = casa::Quantity(x(i), inun);
      x(i)= MVFrequency(y).get(outun).getValue();
    };
  } catch (AipsError (x)) {
    *itsLog << LogIO::SEVERE
	    << "Exception Reports: " << x.getMesg()
	    << "\nIllegal frequency type units specified"
	    << LogIO::POST;
  }
  if (nelem > 0) {
    Int nrow = arg(arg.nelements()-1);
    Int ncol = nelem/nrow;
    retval.resize(nrow);
    Int k = 0;
    for (Int i=0; i<nrow; i++) {
      ostringstream oss;
      if (ncol > 1) oss << '[';
      for (Int j=0; j<ncol; j++) {
	if (j>0) oss << ", ";
	oss << x(k);
	k++;
      };
      if (ncol > 1) oss << ']';
      if (form2) oss << " " << form;
      retval(i) = oss.str();
    };
  } else {
    retval.resize(0);
  };
  return retval;
}

//// Two useless(?) classes from DOquanta.cc
// unit(vector)
Quantum<Array<Double> >
quanta::unitv(const Array<Double> v, const String& unitname) {
  // returnval().getValue().resize(IPosition());
  // returnval() = Quantum<Array<Double> >(val(), arg());
  return Quantum<Array<Double> >(v, unitname);
}

casa::Array<casa::QuantumHolder>
quant(const casa::Array<casa::QuantumHolder> a) {
  return a;
}

} // casac namespace
