/***
 * Framework independent implementation file for measures...
 *
 * Implement the measures component here.
 * 
 * // The measures framework component is a re-wrappering of the thin
 * // C++-glish DO interface implementing the interface of the former
 * // Glish measures tool.
 *
 * @author
 * @version 
 ***/

#include <iostream>

#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <measures/Measures.h>
#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/MCDoppler.h>
#include <measures/Measures/MCEarthMagnetic.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MCFrequency.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MCRadialVelocity.h>
#include <measures/Measures/MCuvw.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasComet.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MeasureHolder.h>
#include <measures_cmpt.h>

using namespace std;
using namespace casa;

namespace casac {

measures::measures() : pcomet_p(0)
{
  itsLog = new LogIO();
  MEpoch tim;
  MPosition pos;
  MDirection dir;
  frame_p = new MeasFrame(tim,pos,dir);
}

measures::~measures()
{
  delete frame_p;
  if (pcomet_p) delete pcomet_p;
}

// Show a direction formatted
std::string
measures::dirshow(const ::casac::record& v)
{
  std::string outStr("");
  try {
    MeasureHolder mh_dir;
    if (casacRec2MeasureHolder(mh_dir,v)) {
      // return [dq.form.long(v.m0), dq.form.lat(v.m1), v.refer];
      ostringstream os;
      if (mh_dir.isMDirection()) {
	os << (mh_dir.asMDirection()).getValue().getAngle("deg");
      } else {
	os << mh_dir.asMeasure();
      }
      os << "  " << (mh_dir.asMeasure()).getRefString();
      outStr = os.str();
    }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return outStr;
}

// Show a measure formatted (with reference code)
std::string
measures::show(const ::casac::record& v, const bool refcode)
{
  std::string outStr("");
  String error;
  try {
    MeasureHolder mh_v;
    Record *inrec = toRecord(v);
    if ((mh_v.fromRecord(error,*inrec)) && mh_v.isMeasure()) {
      /*
      const public.show := function(v, refcode=T) {
	z := "";
	if (ismeasure(v)) {
	  x := dm.gettype(v);
	  y := dm.getvalue(v);
	  if (x ~ m/^dir/i) {
	    z := [dq.form.long(y[1]), dq.form.lat(y[2])];
	  } else if (x ~ m/^pos/i) {
	    z := [dq.form.long(y[1]), dq.form.lat(y[2]),
		  dq.form.len(y[3])];
	  } else if (x ~ m/^epo/i) {
	    z := dq.form.dtime(y[1]);
	  } else if (x ~ m/^radial/i || x ~ m/^dopp/i) {
	    z := dq.form.vel(y[1]);
	  } else if (x ~ m/^freq/i) {
	    z := dq.form.freq(y[1]);
	  } else if (x ~ m/^earth/i) {
	    z := [dq.tos(y[1]), dq.tos(y[2]),
		  dq.tos(y[3])];
	  } else if (x ~ m/^base/i || x ~ m/^uvw/i) {
	    y := dm.addxvalue(v);
	    z := [dq.form.len(y[1]), dq.form.len(y[2]),
		  dq.form.len(y[3])];
	  } else {
	    return '';
	  };
	  if (refcode) return [z, dm.getref(v)];
	};
	return z;
      }
      */
      ostringstream os;
      if (mh_v.isMDirection()) {
	os << (mh_v.asMDirection()).getValue().get();
      } else if (mh_v.isMDoppler()) {
	os << (mh_v.asMDoppler()).getValue().get();
      } else if (mh_v.isMEpoch()) {
	os << (mh_v.asMEpoch()).getValue().get();
      } else if (mh_v.isMFrequency()) {
	os << (mh_v.asMFrequency()).getValue().get();
      } else if (mh_v.isMPosition()) {
	os << (mh_v.asMPosition()).getValue().get();
      } else if (mh_v.isMRadialVelocity()) {
	os << (mh_v.asMRadialVelocity()).getValue().get();
      } else if (mh_v.isMBaseline()) {
	os << (mh_v.asMBaseline()).getValue().get();
      } else if (mh_v.isMuvw()) {
	os << (mh_v.asMuvw()).getValue().get();
      } else if (mh_v.isMEarthMagnetic()) {
	os << (mh_v.asMEarthMagnetic()).getValue().get();
      } else {
	os << mh_v.asMeasure()
	   << ((mh_v.asMeasure()).getUnit()).getName(); // units not printing?
      }
      if (refcode) os << " " << (mh_v.asMeasure()).getRefString();
      outStr = os.str();
    } else {
      error += String ("Non-measure input\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
    }
    delete inrec;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
  }
  return outStr;
}

// Kludge until measures::measure() is refactored.
bool
is_MEpoch(const ::casac::record& a)
{
  bool rstat(false);
  try{
    Record *p_a = toRecord(a);
    if (p_a->isDefined("type")) {
      String error;
      MeasureHolder mh;
      if (mh.fromRecord(error,*p_a))
	rstat = (mh.isMeasure() && mh.isMEpoch());
    }
    delete p_a;
  } catch (AipsError(x)) {
    // no nothing
  }
  return rstat;
}

// epoch from rf, time v0 and optional offset
::casac::record*
measures::epoch(const std::string& rf, const ::casac::variant& v0, const ::casac::record& off)
{
  ::casac::record *retval(0);
  try {
    String error("");

    casa::Quantity q0(casaQuantityFromVar(v0));
    //kludge since cannot pass meaningful default paramters
    if (q0.getValue() == 0 && q0.getUnit() == "") {
      q0 = casa::Quantity(0.0,"d");
    }
    MEpoch me(q0);
    me.setRefString(rf);
    MeasureHolder in(me);
    MeasureHolder out;

    Record *pOffset;
    if (is_MEpoch(off)) {
      pOffset = toRecord(off);
    } else {
      pOffset = new Record();
    }

    if (!measures::measure(error, out, in, rf, *pOffset)) {
      error += "Epoch definition failed!";
      *itsLog << LogIO::WARN << error << LogIO::POST;
      error = "";
    }

    delete pOffset;

    Record outRec;
    out.toRecord(error, outRec);
    retval=fromRecord(outRec);
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
  }
  return retval;
}

// direction from rf, long/lat v0,v1 and optional offset
::casac::record*
measures::direction(const std::string& rf, const ::casac::variant& v0, const ::casac::variant& v1, const ::casac::record& off)
{
  ::casac::record *retval = 0;
  try{
    String error;
    casa::Quantity q0(casaQuantityFromVar(v0));
    casa::Quantity q1(casaQuantityFromVar(v1));
    //kludge since cannot pass meaningful default paramters
    if (q0.getValue() == 0 && q0.getUnit() == "" &&
	q1.getValue() == 0 && q1.getUnit() == "") {
      q0 = casa::Quantity(0.0,"deg");
      q1 = casa::Quantity(90.0,"deg");
    }
    MDirection d(q0,q1);
    if (!d.setRefString(rf)) {
      *itsLog << LogIO::WARN << "Illegal reference frame string.  Reference string set to DEFAULT" << LogIO::POST;
    }

    Record *pOff = toRecord(off);
    if (pOff->nfields() > 0) {
      MeasureHolder doff;
      if ((doff.fromRecord(error,*pOff)) && doff.isMeasure()) {
	if (!d.setOffset(doff.asMeasure())) {
	  error += String ("Non-matching measure offset\n");
	  *itsLog << LogIO::WARN << error << LogIO::POST;
	  error = "";
	}
      } else {
	error += String ("Non-measure type offset in measure conversion\n");
	*itsLog << LogIO::WARN << error << LogIO::POST;
	error = "";
      }
    }

    MeasureHolder out;
    MeasureHolder in(d);
    if (! measures::measure(error, out, in, rf, *pOff)) {
      error += String("Call to measures::measure() failed\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
      delete pOff;
      return retval;
    }
    delete pOff;

    Record outRec;
    if (out.toRecord(error, outRec)) {
      retval = fromRecord(outRec);
    } else {
      error += String("Failed to generate direction return value.\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// get value of measure or measure[array]
::casac::record*
measures::getvalue(const ::casac::record& v)
{
  casac::record* retval=0;
  String error;

  try {
    MeasureHolder mh;
    Record *pV = toRecord(v);
    Record tmpRec;
    if (pV->isDefined("return")) {
      tmpRec = pV->subRecord(pV->fieldNumber("return"));
    } else {
      tmpRec = *pV;
    }
    if ((mh.fromRecord(error,tmpRec)) && mh.isMeasure()) {
      if (tmpRec.isDefined("type")) {
	tmpRec.removeField(tmpRec.fieldNumber("type"));
      }
      if (tmpRec.isDefined("refer")) {
	tmpRec.removeField(tmpRec.fieldNumber("refer"));
      }
      retval = fromRecord(tmpRec);
    } else {
      error += "Incorrect input type for getvalue()";
      *itsLog << LogIO::WARN << error << LogIO::POST;
      error = "";
    }
    delete pV;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
  }
  return retval;
}

std::string
measures::gettype(const ::casac::record& v)
{
  std::string retval("");

  try {
    String error;
    MeasureHolder mh;
    Record *pV = toRecord(v);
    if ((mh.fromRecord(error,*pV)) && mh.isMeasure()) {
      retval = (mh.asMeasure()).tellMe();
    } else {
      error += "Incorrect input type for gettype()";
      *itsLog << LogIO::WARN << error << LogIO::POST;
      error = "";
    }
    delete pV;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
  }
  return retval;
}

std::string
measures::getref(const ::casac::record& v)
{
  std::string retval("");

  try {
    String error;
    MeasureHolder mh;
    Record *pV = toRecord(v);
    if ((mh.fromRecord(error,*pV)) && mh.isMeasure()) {
      retval = (mh.asMeasure()).getRefString();
    } else {
      error += "Incorrect input type for getref()";
      *itsLog << LogIO::WARN << error << LogIO::POST;
      error = "";
    }
    delete pV;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
  }
  return retval;
}

::casac::record*
measures::getoffset(const ::casac::record& v)
{
  ::casac::record *retval(0);

  try {
    String error;
    MeasureHolder mh;
    Record *pV = toRecord(v);
    if (! (mh.fromRecord(error, *pV) && mh.isMeasure()) ) {
      error += "Incorrect input type for getoffset()";
      *itsLog << LogIO::WARN << error << LogIO::POST;
      error = "";
    } else {
      if (pV->isDefined("offset")) {
	Int offsetField = pV->fieldNumber("offset");
	Record offrec;
	offrec = pV->subRecord(offsetField);
	retval = fromRecord(offrec);
      } else {
	*itsLog << "Measures does not have an offset" << LogIO::POST;
      }
    }
    delete pV;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
  }
  return retval;
}

// get the comet name
std::string
measures::cometname()
{
  std::string emptyStr("");
  try {
    if (pcomet_p) {
      return pcomet_p->getName();
    } else {
      *itsLog << LogIO::WARN << "Method cometname fails! "
	      << "No Comet table present" << LogIO::POST;
      return emptyStr;
    }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return emptyStr;
}

// get the type of comet table
std::string
measures::comettype()
{
  std::string emptyStr("");
  try {
    if (pcomet_p) {
      if (pcomet_p->getType() == MDirection::TOPO) {
	return String("TOPO");
      } else {
	return String("APP");
      };
    } else {
      return String("none");
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return emptyStr;
}

// get the topocentre position of current comet
::casac::record*
measures::comettopo()
{
  std::vector<double> retval(1);
  retval[0]=0;
  String unit("");

  try {
    Vector<Double> returnval;
    if (pcomet_p && pcomet_p->getType() == MDirection::TOPO) {
      returnval = pcomet_p->getTopo().getValue();
      returnval.tovector(retval);
      unit = "m";
    } else {
      *itsLog << LogIO::WARN << "Method comettopo fails!  "
	      << "No Topocentric Comet table present\n" << LogIO::POST;
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
}

// set a comet frame
bool
measures::framecomet(const std::string& v)
{
  try {
    return doframe(v);
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports from framecomet: " << x.getMesg() << LogIO::POST;
  }
  return false;
}

// Position from rf, long v1, lat v2, height v0 (or x,y,z) and optional offset
//   rf='', v0='0..', v1='90..', v2='0m', off=F
::casac::record*
measures::position(const std::string& rf, const ::casac::variant& v0, const ::casac::variant& v1, const ::casac::variant& v2, const ::casac::record& off)
{
  ::casac::record *retval(0);
 
  try{
    
    String error;
    Quantum<Vector<Double> > q0v=casaQuantumVector(v0);
    Record mpos_rec;
    mpos_rec.define("type", "position");
    mpos_rec.define("refer", String(rf));
     String err;
    //If its  quanta vectors
    if(q0v.getValue().nelements() != 0){
      Quantum<Vector<Double> > q1v=casaQuantumVector(v1);
      Quantum<Vector<Double> > q2v=casaQuantumVector(v2);
      if((q1v.getValue().nelements() != q0v.getValue().nelements()) ||  (q2v.getValue().nelements() != q0v.getValue().nelements()))
	throw(AipsError("The length of the 3 quantities are not conformant \n"));
      Record QuantRec;
     
      QuantumHolder(q0v).toRecord(err, QuantRec);
      mpos_rec.defineRecord("m0", QuantRec);
      QuantumHolder(q1v).toRecord(err, QuantRec);
      mpos_rec.defineRecord("m1", QuantRec);
      QuantumHolder(q2v).toRecord(err, QuantRec);
      mpos_rec.defineRecord("m2", QuantRec);
    }
    else{
      casa::Quantity q0(casaQuantityFromVar(v0));
      casa::Quantity q1(casaQuantityFromVar(v1));
      casa::Quantity q2(casaQuantityFromVar(v2));
      //kludge since cannot pass meaningful default paramters
      if (q0.getValue() == 0 && q0.getUnit() == "" &&
	  q1.getValue() == 0 && q1.getUnit() == "" &&
	  q2.getValue() == 0 && q2.getUnit() == "") {
	q0 = casa::Quantity(0.0,"deg");
	q1 = casa::Quantity(90.0,"deg");
	q2 = casa::Quantity(0.0,"m");
      }
      casa::Quantity q(1.0,"rad");
      Record QuantRec;
      if (q.isConform(q1)) {
	// Note reordering of input quantities
	QuantumHolder(q2).toRecord(err, QuantRec);
	mpos_rec.defineRecord("m0", QuantRec);
	QuantumHolder(q0).toRecord(err, QuantRec);
	mpos_rec.defineRecord("m1", QuantRec);
	QuantumHolder(q1).toRecord(err, QuantRec);
	mpos_rec.defineRecord("m2", QuantRec);
	
      } else {
	QuantumHolder(q0).toRecord(err, QuantRec);
	mpos_rec.defineRecord("m0", QuantRec);
	QuantumHolder(q1).toRecord(err, QuantRec);
	mpos_rec.defineRecord("m1", QuantRec);
	QuantumHolder(q2).toRecord(err, QuantRec);
	mpos_rec.defineRecord("m2", QuantRec);
	//mvp = new MVPosition(q0.getBaseValue(),
	//		     q1.getBaseValue(),
	//		     q2.getBaseValue());
      }
    }
    MeasureHolder out;
    MeasureHolder in;
    in.fromRecord(err, mpos_rec);
    if(!in.isMPosition())
      throw(AipsError("Could not convert input into a Position Measure \n" + err));
    Record *pOff = toRecord(off);
    if (! measures::measure(error, out, in, rf, *pOff)) {
      error += String("Call to measures::measure() failed\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
      delete pOff;
      return retval;
    }
    delete pOff;

    Record outRec;
    if (out.toRecord(error, outRec)) {
      retval = fromRecord(outRec);
    } else {
      error += String("Failed to generate position measure return value.\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      error = "";
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// get position of observatory by name
::casac::record*
measures::observatory(const std::string& name)
{
  ::casac::record *retval(0);
  try {
    String error;
    MPosition returnval;

    if (!MeasTable::Observatory(returnval, String(name))) {
      *itsLog << LogIO::SEVERE << "Unknown observatory asked for\n"
	      << LogIO::POST;
      return retval;
    };

    MeasureHolder out(returnval);
    Record outRec;
    if (!out.toRecord(error, outRec)) {
      error += "Failed to generate observatory return value.\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return retval;
    };
    retval = fromRecord(outRec);
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// get list of observatories
std::string
measures::obslist()
{
  std::string emptyStr("");
  try {
    String returnval;
    const Vector<String> &lst = MeasTable::Observatories();
    returnval = String();
    if (lst.nelements() > 0) {
      // Note in next one the const throw away, since join does not accept
      // const String src[]
      Bool deleteIt; 
      String *storage = const_cast<String *>(lst.getStorage(deleteIt));
      const String *cstorage = storage;
      returnval = join(storage, lst.nelements(), String(" "));
      lst.freeStorage(cstorage, deleteIt);
    };
    return returnval;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    return emptyStr;
  }
}

// spectral line list
std::string
measures::linelist()
{
  std::string emptyStr("");
  *itsLog << LogIO::WARN << "DEPRECATED. The " << __FUNCTION__ << " method has been deprecated and will be removed "
	<< "in the near future. Please use the spectral line (sl) tool and/or related tasks instead"
	<< LogIO::POST;
  try {

    const Vector<String> &lst = MeasTable::Lines();
    String returnval = String();
    if (lst.nelements() > 0) {
      // Note in next one the const throw away, since join does not accept
      // const String src[]
      Bool deleteIt; 
      String *storage = const_cast<String *>(lst.getStorage(deleteIt));
      const String *cstorage = storage;
      returnval = join(storage, lst.nelements(), String(" "));
      lst.freeStorage(cstorage, deleteIt);
    };
    return returnval;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    return emptyStr;
  }
}

// a spectral line
::casac::record *
measures::spectralline(const std::string& name)
{
	*itsLog << LogIO::WARN << "DEPRECATED. The " << __FUNCTION__ << " method has been deprecated and will be removed "
		<< "in the near future. Please use the spectral line (sl) tool and/or related tasks instead"
		<< LogIO::POST;
  ::casac::record *retval = 0;
  String error;

  try {
    MFrequency returnval;

    if (!MeasTable::Line(returnval, String(name))) {
      *itsLog << LogIO::SEVERE <<"Unknown spectral line asked for\n" << LogIO::POST;
      return retval;
    };

    MeasureHolder out(returnval);
    Record outRec;
    if (!out.toRecord(error, outRec)) {
      error += "Failed to generate spectralline return value.\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return retval;
    };
    retval = fromRecord(outRec);
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// source list
std::string
measures::sourcelist()
{
  std::string emptyStr("");
  try {
    String returnval;
    const Vector<String> &lst = MeasTable::Sources();
    returnval = String();
    if (lst.nelements() > 0) {
      // Note in next one the const throw away, since join does not accept
      // const String src[]
      Bool deleteIt; 
      String *storage = const_cast<String *>(lst.getStorage(deleteIt));
      const String *cstorage = storage;
      returnval = join(storage, lst.nelements(), String(" "));
      lst.freeStorage(cstorage, deleteIt);
    };
    return returnval;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    return emptyStr;
  }
};

// a source
::casac::record*
measures::source(const ::casac::variant& name)
{
  ::casac::record *retval(0);
  try {
    String error;
    MDirection returnval;

    if (!casaMDirection(name, returnval)) {
      *itsLog << LogIO::SEVERE << "Unknown source asked for\n"
    	      << LogIO::POST;
      return retval;
    };
    

    MeasureHolder out(returnval);
    Record outRec;
    if (!out.toRecord(error, outRec)) {
      error += "Failed to generate source return value.\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return retval;
    };
    retval = fromRecord(outRec);
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// frequency from rf, value v0 and optional offset
//   rf='', v0='0Hz', off=F
::casac::record*
measures::frequency(const std::string& rf, const ::casac::variant& v0, const ::casac::record& off)
{
  ::casac::record *retval(0);
  try{
    String error;
    casa::Quantity q0(casaQuantityFromVar(v0));
    //kludge since cannot pass meaningful default paramters
    if (q0.getValue() == 0 && q0.getUnit() == "") {
      q0 = casa::Quantity(0.0,"Hz");
    }
    MFrequency f(q0);
    if (!f.setRefString(rf)) {
      *itsLog << LogIO::WARN << "Illegal reference frame string." << LogIO::POST;
    }

    Record *pOff = toRecord(off);
    if (pOff->nfields() > 0) {
      MeasureHolder foff;
      if ((foff.fromRecord(error,*pOff)) && foff.isMeasure()) {
	if (!f.setOffset(foff.asMeasure())) {
	  error += String ("Illegal offset type specified, not used\n");
	  *itsLog << LogIO::WARN << error << LogIO::POST;
	  error = "";
	}
      } else {
	error += String ("Non-measure type offset in measure conversion\n");
	*itsLog << LogIO::WARN << error << LogIO::POST;
	error = "";
      }
    }

    MeasureHolder out;
    MeasureHolder in(f);
    if (! measures::measure(error, out, in, rf, *pOff)) {
      error += String("Call to measures::measure() failed\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
      delete pOff;
      return retval;
    }
    delete pOff;

    Record outRec;
    if (out.toRecord(error, outRec)) {
      retval = fromRecord(outRec);
    } else {
      error += String("Failed to generate frequency measure return value.\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      error = "";
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// doppler from rf, doppler v0 and optional offset
//   rf='', v0='0', off=F
::casac::record*
measures::doppler(const std::string& rf, const ::casac::variant& v0, const ::casac::record& off)
{
  ::casac::record *retval(0);
  try{
    String error;
    casa::Quantity q0(casaQuantityFromVar(v0));
    //kludge since cannot pass meaningful default paramters
    if (q0.getValue() == 0 && q0.getUnit() == "") {
      q0 = casa::Quantity(0.0,"m/s");
    }
    MDoppler mq(q0);
    if (!mq.setRefString(rf)) {
      *itsLog << LogIO::WARN << "Illegal reference frame string." << LogIO::POST;
    }

    Record *pOff = toRecord(off);
    if (pOff->nfields() > 0) {
      MeasureHolder mqoff;
      if ((mqoff.fromRecord(error,*pOff)) && mqoff.isMeasure()) {
	if (!mq.setOffset(mqoff.asMeasure())) {
	  error += String ("Illegal offset type specified, not used\n");
	  *itsLog << LogIO::WARN << error << LogIO::POST;
	  error = "";
	}
      } else {
	error += String ("Non-measure type offset in measure conversion\n");
	*itsLog << LogIO::WARN << error << LogIO::POST;
	error = "";
      }
    }

    MeasureHolder out;
    MeasureHolder in(mq);
    if (! measures::measure(error, out, in, rf, *pOff)) {
      error += String("Call to measures::measure() failed\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
      delete pOff;
      return retval;
    }
    delete pOff;

    Record outRec;
    if (out.toRecord(error, outRec)) {
      retval = fromRecord(outRec);
    } else {
      error += String("Failed to generate doppler measure return value.\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      error = "";
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    }
  return retval;
}

// radialvelocity from rf, velocity v0 and optional offset
//   rf='', v0='0m/s', off=F
::casac::record*
measures::radialvelocity(const std::string& rf, const ::casac::variant& v0, const ::casac::record& off)
{
  ::casac::record *retval(0);
  try{
    String error;
    casa::Quantity q0(casaQuantityFromVar(v0));
    //kludge since cannot pass meaningful default paramters
    if (q0.getValue() == 0 && q0.getUnit() == "") {
      q0 = casa::Quantity(0.0,"m/s");
    }
    MRadialVelocity mq(q0);
    if (!mq.setRefString(rf)) {
      *itsLog << LogIO::WARN << "Illegal reference frame string." << LogIO::POST;
    }

    Record *pOff = toRecord(off);
    if (pOff->nfields() > 0) {
      MeasureHolder mqoff;
      if ((mqoff.fromRecord(error,*pOff)) && mqoff.isMeasure()) {
	if (!mq.setOffset(mqoff.asMeasure())) {
	  error += String ("Illegal offset type specified, not used\n");
	  *itsLog << LogIO::WARN << error << LogIO::POST;
	  error = "";
	}
      } else {
	error += String ("Non-measure type offset in measure conversion\n");
	*itsLog << LogIO::WARN << error << LogIO::POST;
	error = "";
      }
    }
    delete pOff;

    MeasureHolder mh(mq);
    Record outRec;
    if (mh.toRecord(error, outRec)) {
      retval = fromRecord(outRec);
    } else {
      error += String("Failed to generate radialvelocity measure return value.\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      error = "";
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// Calculate uvw from rf, long v1, lat v2, height v0 (or x,y,z)
// and optional offset
//   rf='', v0='0..', v1='', v2='', off=F
::casac::record*
measures::uvw(const std::string& rf, const ::casac::variant& v0, const ::casac::variant& v1, const ::casac::variant& v2, const ::casac::record& off)
{
  ::casac::record *retval(0);
  try{
    String error;
    

    Quantum<Vector<Double> > q0v=casaQuantumVector(v0);
    Record muvw_rec;
    muvw_rec.define("type", "uvw");
    muvw_rec.define("refer", String(rf));
    String err;
    //If its  quanta vectors
    if(q0v.getValue().nelements() != 0){
      Quantum<Vector<Double> > q1v=casaQuantumVector(v1);
      Quantum<Vector<Double> > q2v=casaQuantumVector(v2);
      if((q1v.getValue().nelements() != q0v.getValue().nelements()) ||  (q2v.getValue().nelements() != q0v.getValue().nelements()))
	throw(AipsError("The length of the 3 quantities are not conformant \n"));
      Record QuantRec;
     
      QuantumHolder(q0v).toRecord(err, QuantRec);
      muvw_rec.defineRecord("m0", QuantRec);
      QuantumHolder(q1v).toRecord(err, QuantRec);
      muvw_rec.defineRecord("m1", QuantRec);
      QuantumHolder(q2v).toRecord(err, QuantRec);
      muvw_rec.defineRecord("m2", QuantRec);
    }
    else{
      casa::Quantity q0(casaQuantity(v0));
      casa::Quantity q1(casaQuantity(v1));
      casa::Quantity q2(casaQuantity(v2));
      //kludge since cannot pass meaningful default paramters
      if (q0.getValue() == 0 && q0.getUnit() == "" &&
	  q1.getValue() == 0 && q1.getUnit() == "" &&
	  q2.getValue() == 0 && q2.getUnit() == "") {
	q0 = casa::Quantity(0.0,"deg");
	q1 = casa::Quantity(0.0,"deg");
	q2 = casa::Quantity(0.0,"m");
      }
      casa::Quantity q(1.0,"rad");
      Record QuantRec;
      if (q.isConform(q1)) {
	// Note reordering of input quantities
	QuantumHolder(q2).toRecord(err, QuantRec);
	muvw_rec.defineRecord("m0", QuantRec);
	QuantumHolder(q0).toRecord(err, QuantRec);
	muvw_rec.defineRecord("m1", QuantRec);
	QuantumHolder(q1).toRecord(err, QuantRec);
	muvw_rec.defineRecord("m2", QuantRec);
	
      } else {
	QuantumHolder(q0).toRecord(err, QuantRec);
	muvw_rec.defineRecord("m0", QuantRec);
	QuantumHolder(q1).toRecord(err, QuantRec);
	muvw_rec.defineRecord("m1", QuantRec);
	QuantumHolder(q2).toRecord(err, QuantRec);
	muvw_rec.defineRecord("m2", QuantRec);
      }
    }






    Record *pOff = toRecord(off);
    if (pOff->nfields() > 0) {
      MeasureHolder mqoff;
      if ((mqoff.fromRecord(error,*pOff)) && mqoff.isMuvw()) {
	  error += String ("Illegal offset type specified, not used\n");
	  *itsLog << LogIO::WARN << error << LogIO::POST;
	  delete pOff;
	  pOff=new Record();
      }
    } 
    

    MeasureHolder out;
    MeasureHolder in;
    in.fromRecord(err, muvw_rec);
    if(!in.isMuvw())
      throw(AipsError("Could not convert input into a uvw Measure \n" + err));
    if (! measures::measure(error, out, in, rf, *pOff)) {
      error += String("Call to measures::measure() failed\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
      delete pOff;
      return retval;
    }
    delete pOff;

    Record outRec;
    if (out.toRecord(error, outRec)) {
      retval = fromRecord(outRec);
    } else {
      error += String("Failed to generate uvw measure return value.\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      error = "";
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

/*
bool
record2MeasureHolder(String& error, MeasureHolder& mh, const ::casac::record &r)
{
  bool rstat(false);
  Record *pRec = toRecord(r);
  if (!mh.fromRecord(error,*pRec)) {
    error += String("  Non-measure record in record2MeasureHolder conversion\n");
  } else {
    rstat = true;
  }
  delete pRec;
  return rstat;
}
*/

// Baseline v converted to uvw and uvwdot
::casac::record*
measures::touvw(::casac::record& dotOut, ::casac::record& xyzOut, const ::casac::record& v)
{
  String error;
  ::casac::record *retval(0);
  try {
    MeasureHolder in;
    Record *pV = toRecord(v);
    if (!in.fromRecord(error,*pV)) {
      error += String ("Non-measure type v in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete pV;
      return retval;
    }
    delete pV;
    if (!in.isMBaseline()) {
      error += "Trying to convert non-baseline to uvw\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return retval;
    };
    in.asMeasure().getRefPtr()->set(*frame_p);   // attach frame
    MBaseline::Convert mcvt(in.asMeasure(), MBaseline::J2000);
    const MVBaseline &bas2000 = mcvt().getValue();
    MVDirection dir2000;
    Double dec2000;
    if (!frame_p->getJ2000(dir2000) || !frame_p->getJ2000Lat(dec2000)) {
      error += "No direction in frame for uvw calculation\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return retval;
    };
    MVuvw uvw2000 = MVuvw(bas2000, dir2000);
    MeasureHolder out(Muvw(uvw2000, Muvw::J2000));
    uInt nel = in.nelements() == 0 ? 1 : in.nelements();
    out.makeMV(in.nelements());
    Double sd = sin(dec2000);
    Double cd = cos(dec2000);

    Vector<Double> xyz;
    Vector<Double> dot;
    dot.resize(3*nel);
    xyz.resize(3*nel);
    if (in.nelements() == 0) {
      xyz = uvw2000.getValue();
      dot[0] = -sd*xyz[1] + cd*xyz[2];
      dot[1] = +sd*xyz[0];
      dot[2] = -cd*xyz[0];
    };
    for (uInt i=0; i<3*in.nelements(); i+=3) {
      const MVuvw &mv = MVuvw(mcvt(dynamic_cast<const MVBaseline &>
				   (*in.getMV(i/3))).getValue(), dir2000);
      if (!out.setMV(i/3, mv)) {
	error += "Cannot get extra baseline value in measures::touvw\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
	return retval;
      };
      for (uInt j=0; j<3; ++j) xyz[i+j] = mv.getValue()[j];
      dot[i+0] = -sd*xyz[i+1] + cd*xyz[i+2];
      dot[i+1] = +sd*xyz[i+0];
      dot[i+2] = -cd*xyz[i+0];
    };
    for (uInt j=0; j<3*nel; ++j) {
      dot[j] *= C::pi/180/240./1.002737909350795;
    };

    xyzOut=*recordFromQuantity(Quantum<Vector<Double> >(xyz,"m"));
    dotOut=*recordFromQuantity(Quantum<Vector<Double> >(dot,"m/s"));

    Record outRec;
    if (!out.toRecord(error, outRec)) {
      error += "Failed to generate return value.\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return retval;
    };
    retval = fromRecord(outRec);
  } catch (AipsError(x)) {
    error += "Cannot convert baseline to uvw: frame "
      "information missing";
    *itsLog << LogIO::SEVERE << error << LogIO::POST;
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    };
  return retval;
}

// Expand positions to baselines
Bool
expand2(String &error, MeasureHolder &out, Vector<Double> &xyz,
       const MeasureHolder &in) {
  if (!in.isMuvw()) {
    error += "Trying to expand non-baseline type\n";
    return False;
  };
  const MVuvw &uvw2000 = in.asMuvw().getValue();
  if (in.nelements() < 2) {
    xyz.resize(3);
    xyz = uvw2000.getValue();
    out = MeasureHolder(Muvw(uvw2000, Muvw::J2000));
  } else {
    uInt nel = (in.nelements() * (in.nelements()-1))/2;
    xyz.resize(3*nel);
    uInt k=0;
    for (uInt i=0; i<in.nelements(); ++i) {
      for (uInt j=i+1; j<in.nelements(); ++j) {
        MVuvw mv = (dynamic_cast<const MVuvw &>(*in.getMV(j))).getValue();
        mv -= (dynamic_cast<const MVuvw &>(*in.getMV(i))).getValue();
        if (k == 0) {
          out = MeasureHolder(Muvw(mv, Muvw::J2000));
          out.makeMV(nel);
        };
        if (!out.setMV(k, mv)) {
          error += "Cannot expand baseline value in DOmeasures::expand\n";
          return False;
        };
        for (uInt j=0; j<3; ++j) xyz[3*k+j] = mv.getValue()[j];
        ++k;
      };
    };
  };
  return True;
}

MeasureHolder
expand(Quantum<Vector<Double> > &xyzres, MeasureHolder &val) {
  MeasureHolder returnval;
  Vector<Double> xres;
  String err;
  if (expand2(err, returnval, xres, val)) {
    xyzres = Quantum<Vector<Double> >(xres, "m");
  }
  return returnval;
}

// expand to baselines from positions v
::casac::record*
measures::expand(::casac::record& xyzres, const ::casac::record& v)
{
  String error;
  ::casac::record *retval= 0;

  try {
    Record *pRec = toRecord(v);
    String origType="uvw";
    String origRefer="J2000";
    if (pRec->isDefined("type") && pRec->isDefined("refer")) {
      pRec->get("type", origType);
      pRec->get("refer", origRefer);
      /*Int fldnum = pRec->fieldNumber("type");
      Int fldnum2 = pRec->fieldNumber("refer");
      if ( (pRec->dataType(fldnum)) == TpString &&
	   (pRec->dataType(fldnum2)) == TpString) {
	String uvw;
	pRec->get(fldnum, uvw);
	if (uvw == "baseline" || uvw == "position") {
	  pRec->removeField(fldnum); //can't use fldnum2 now
	  pRec->define("type","uvw");
	}
	pRec->removeField(pRec->fieldNumber("refer"));
      
      
      pRec->define("refer","J2000");
    }
      */
      pRec->define("refer","J2000");
      pRec->define("type","uvw");
    }

    MeasureHolder in;
    if (!in.fromRecord(error, *pRec)) {
      error += String("Non-measure input to expand()\n");
      error += String("Can only expand baselines, positions, or uvw\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
      delete pRec;
      return retval;
    };
    delete pRec;

    MeasureHolder out;
    Vector<Double> xyz;
    if ( expand2(error, out, xyz, in) ) {
      xyzres = *recordFromQuantity(Quantum<Vector<Double> >(xyz,"m"));

      Record outRec;
      if (out.toRecord(error, outRec)) {
	outRec.define("type", origType);
	outRec.define("refer", origRefer);
	retval = fromRecord(outRec);
      } else {
	error += "Failed to generate valid return value.\n";
	*itsLog << LogIO::SEVERE << error << LogIO::POST;
      };
    }
  } catch (AipsError(x)) {
    error += "Cannot convert baseline to uvw: frame "
      "information missing";
    *itsLog << LogIO::SEVERE << error << LogIO::POST;
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  };
  return retval;
}

// earthmagnetic from rf, long v1, lat v2, height v0 (or x,y,z)
// and optional offset
//   rf='', v0='0G', v1='0..', v2='90..', off=F
::casac::record*
measures::earthmagnetic(const std::string& rf, const ::casac::variant& v0, const ::casac::variant& v1, const ::casac::variant& v2, const ::casac::record& off)
{
  ::casac::record *retval(0);
  try{
    String error;
    casa::Quantity q0(casaQuantityFromVar(v0));
    casa::Quantity q1(casaQuantityFromVar(v1));
    casa::Quantity q2(casaQuantityFromVar(v2));
    //kludge since cannot pass meaningful default paramters
    if (q0.getValue() == 0 && q0.getUnit() == "" &&
	q1.getValue() == 0 && q1.getUnit() == "" &&
	q2.getValue() == 0 && q2.getUnit() == "") {
      q0 = casa::Quantity( 0.0,"G");
      q1 = casa::Quantity( 0.0,"deg");
      q2 = casa::Quantity(90.0,"deg");
    }
    MVEarthMagnetic mvq(q0, q1, q2);
    MEarthMagnetic mq(mvq);
    if (!mq.setRefString(rf)) {
      *itsLog << LogIO::WARN << "Illegal reference frame string."
	      << LogIO::POST;
    }

    Record *pOff = toRecord(off);
    if (pOff->nfields() > 0) {
      MeasureHolder mqoff;
      if ((mqoff.fromRecord(error,*pOff)) && mqoff.isMeasure()) {
	if (!mq.setOffset(mqoff.asMeasure())) {
	  error += String ("Illegal offset type specified, not used\n");
	  *itsLog << LogIO::WARN << error << LogIO::POST;
	  error = "";
	}
      } else {
	error += String ("Non-measure type offset in measure conversion, not used\n");
	*itsLog << LogIO::WARN << error << LogIO::POST;
	error = "";
      }
    }

    MeasureHolder out;
    MeasureHolder in(mq);
    if (! measures::measure(error, out, in, rf, *pOff)) {
      error += String("Call to measures::measure() failed\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
      delete pOff;
      return retval;
    }
    delete pOff;

    Record outRec;
    if (out.toRecord(error, outRec)) {
      retval = fromRecord(outRec);
    } else {
      error += String("Failed to generate EarthMagnetic measure return value.\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      error = "";
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// baseline from rf, long v1, lat v2, height v0 (or x,y,z) and optional offset
//   rf='', v0='0..', v1='', v2='', off=F
::casac::record*
measures::baseline(const std::string& rf, const ::casac::variant& v0, const ::casac::variant& v1, const ::casac::variant& v2, const ::casac::record& off)
{
  ::casac::record *retval(0);
  try{



    String error;
    Quantum<Vector<Double> > q0v=casaQuantumVector(v0);
    Record mbas_rec;
    mbas_rec.define("type", "baseline");
    mbas_rec.define("refer", String(rf));
     String err;
    //If its  quanta vectors
    if(q0v.getValue().nelements() != 0){
      Quantum<Vector<Double> > q1v=casaQuantumVector(v1);
      Quantum<Vector<Double> > q2v=casaQuantumVector(v2);
      if((q1v.getValue().nelements() != q0v.getValue().nelements()) ||  (q2v.getValue().nelements() != q0v.getValue().nelements()))
	throw(AipsError("The length of the 3 quantities are not conformant \n"));
      Record QuantRec;
     
      QuantumHolder(q0v).toRecord(err, QuantRec);
      mbas_rec.defineRecord("m0", QuantRec);
      QuantumHolder(q1v).toRecord(err, QuantRec);
      mbas_rec.defineRecord("m1", QuantRec);
      QuantumHolder(q2v).toRecord(err, QuantRec);
      mbas_rec.defineRecord("m2", QuantRec);
    }
    else{
      casa::Quantity q0(casaQuantity(v0));
      casa::Quantity q1(casaQuantity(v1));
      casa::Quantity q2(casaQuantity(v2));
      //kludge since cannot pass meaningful default paramters
      if (q0.getValue() == 0 && q0.getUnit() == "" &&
	  q1.getValue() == 0 && q1.getUnit() == "" &&
	  q2.getValue() == 0 && q2.getUnit() == "") {
	q0 = casa::Quantity(0.0,"deg");
	q1 = casa::Quantity(0.0,"deg");
	q2 = casa::Quantity(0.0,"m");
      }
      casa::Quantity q(1.0,"rad");
      Record QuantRec;
      if (q.isConform(q1)) {
	// Note reordering of input quantities
	QuantumHolder(q2).toRecord(err, QuantRec);
	mbas_rec.defineRecord("m0", QuantRec);
	QuantumHolder(q0).toRecord(err, QuantRec);
	mbas_rec.defineRecord("m1", QuantRec);
	QuantumHolder(q1).toRecord(err, QuantRec);
	mbas_rec.defineRecord("m2", QuantRec);
	
      } else {
	QuantumHolder(q0).toRecord(err, QuantRec);
	mbas_rec.defineRecord("m0", QuantRec);
	QuantumHolder(q1).toRecord(err, QuantRec);
	mbas_rec.defineRecord("m1", QuantRec);
	QuantumHolder(q2).toRecord(err, QuantRec);
	mbas_rec.defineRecord("m2", QuantRec);
      }
    }



    Record *pOff = toRecord(off);
    if (pOff->nfields() > 0) {
      MeasureHolder mqoff;
      if (!((mqoff.fromRecord(error,*pOff)) && mqoff.isMBaseline())) {
	  error += String ("Illegal offset type specified, not used\n");
	  *itsLog << LogIO::WARN << error << LogIO::POST;
	  error = "";	
	  delete pOff;
	  pOff=new Record();
      }
    }

    MeasureHolder mhin;
    mhin.fromRecord(err, mbas_rec); 
    if(!mhin.isMBaseline()){
      throw(AipsError("values or reference are invalid to make a Baseline measure \n" + err));
    }

    MeasureHolder mhout;
    if (!measures::measure(error,mhout,mhin,rf,*pOff)) {
      error += String("Failed to convert to Baseline measure.\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      error = "";
    } else {
      Record outRec;
      if (mhout.toRecord(error, outRec)) {
	retval = fromRecord(outRec);
      } else {
	error += String("Failed to generate Baseline measure return value.\n");
	*itsLog << LogIO::SEVERE << error << LogIO::POST;
	error = "";
      };
    }
    delete pOff;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

#include <casa/Containers/RecordFieldId.h>

// make a baseline from position
::casac::record*
measures::asbaseline(const ::casac::record& pos)
{
  /*
    const public.asbaseline := function(pos) {
      if (!ismeasure(pos) || (pos.type != 'position' && 
			       pos.type != 'baseline')) {
	fail('Non-position type for asbaseline input');
      };
      if (pos.type == 'position') {
	loc := public.measure(pos, 'itrf');
	loc.type := 'baseline';
	if (!ismeasure(loc)) fail('Cannot convert position to baseline');
	loc := public.measure(loc, 'j2000');
	if (!ismeasure(loc)) fail('Cannot convert baseline to J2000');
	return loc;
      };
      return pos;
    }
  */
  ::casac::record *retval = 0;
  try {
    String error("");
    Record *pPos = toRecord(pos);
    String tp;
    Record outRec;
    pPos->get(RecordFieldId("type"), tp);
    tp.downcase();
    if ((tp != downcase(MPosition::showMe())) && (tp != downcase(MBaseline::showMe()))) {
      *itsLog << LogIO::WARN << "Non-position type for asbaseline input"
	      << LogIO::POST;
      delete pPos;
      return retval;
    }

    MeasureHolder mh_pos;
    if (mh_pos.fromRecord(error,*pPos) && mh_pos.isMeasure()) {
      if (! mh_pos.isMPosition()  &&  !mh_pos.isMBaseline()) {
	*itsLog << LogIO::SEVERE << "Input must be a position measure"
		<< LogIO::POST;
	delete pPos;
	return retval;
      }
      String err;
      if(mh_pos.isMPosition()){
	Record tempRec, off;
	MeasureHolder tmpmh;
	if(!measures::measure(err, tmpmh, mh_pos, "ITRF", off))
	  *itsLog << LogIO::SEVERE << "Error in position conversion: " << err
		  << LogIO::POST;
	tmpmh.toRecord(err, tempRec);
	tempRec.define("type", String("baseline"));
	mh_pos.fromRecord(err, tempRec);
	MeasureHolder mh_out; 
	if(!measures::measure(err, mh_out, mh_pos, "j2000", off))
	   *itsLog << LogIO::SEVERE << "Error in baseline conversion: " << err
		  << LogIO::POST;
	///Weird if using = operator for MeasureHolder it won't allow all elements
	///saved to Record
	mh_out.toRecord(err, outRec);
 
      }
      else{
	//Most probably an MBaseline already
	mh_pos.toRecord(err, outRec);
      }
      
       retval=fromRecord(outRec);
      return retval;
    } else {
      error += String ("Non-measure type for asbaseline input\n");
      *itsLog << LogIO::WARN << error << LogIO::POST;
      delete pPos;
      return retval;
    }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
    return retval;
  }
}

::casac::record*
measures::listcodes(const ::casac::record& ms)
{
  ::casac::record *retval(0);
  try {
    String error;

    Record *p_ms = toRecord(ms);
    MeasureHolder mh_ms;
    if (!mh_ms.fromRecord(error,*p_ms)) {
      error += String ("Non-measure type ms in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_ms;
      return retval;
    }
    delete p_ms;

    Int nall, nex;
    const uInt *typ;
    const String *tall = mh_ms.asMeasure().allTypes(nall, nex, typ);
    Vector<String> tcod(nall-nex);
    Vector<String> text(nex);
    for (Int i=0; i<nall; i++) {
      if (i<nall-nex) tcod(i) = tall[i];
      else text(i-nall+nex) = tall[i];
    };

    RecordDesc rd;
    rd.addField("normal",TpArrayString);
    rd.addField("extra",TpArrayString);
    Record returnval(rd);
    RecordFieldPtr<Array<String> > normal(returnval, "normal");
    RecordFieldPtr<Array<String> > extra(returnval, "extra");
    normal.define(tcod);
    extra.define(text);
    retval = fromRecord(returnval);
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

// Convert measures
Bool measures::measure(String &error, MeasureHolder &out,
		       const MeasureHolder &in, const String &outref,
		       const Record &off) {
  MeasureHolder mo;
  try {
    if (off.nfields()> 0) {
      if (!mo.fromRecord(error, off)) {
	//	error += String("Non-measure type offset in measure conversion\n");
	//	*itsLog << LogIO::WARN << error << LogIO::POST;
	//	error = "";
      } else {
	if (mo.isMeasure()) {
	  mo.asMeasure().getRefPtr()->set(*frame_p);
	}
      }
    }
    in.asMeasure().getRefPtr()->set(*frame_p);

    if (in.isMEpoch()) {
      MEpoch::Ref outRef;
      MEpoch::Types tp;
      String x = outref;
      Bool raze = False;
      if (x.before(2) == "r_" || x.before(2) == "R_") {
	raze = True;
	x = x.from(2);
      };
      if (MEpoch::getType(tp, x)) {
	if (raze) outRef.setType(tp | MEpoch::RAZE);
	else outRef.setType(tp);
      } else outRef.setType(MEpoch::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMEpoch()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      MEpoch::Convert mcvt(MEpoch::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
        if (!out.setMV(i, mcvt(dynamic_cast<const MVEpoch &>
                               (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
    } else if (in.isMPosition()) {
      MPosition::Ref outRef;
      MPosition::Types tp;
      if (MPosition::getType(tp, outref)) outRef.setType(tp);
      else outRef.setType(MPosition::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMPosition()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      MPosition::Convert mcvt(MPosition::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
        if (!out.setMV(i, mcvt(dynamic_cast<const MVPosition &>
                               (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
    } else if (in.isMDirection()) {
      MDirection::Ref outRef;
      MDirection::Types tp;
      if (MDirection::getType(tp, outref)) outRef.setType(tp);
      else outRef.setType(MDirection::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMDirection()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      MDirection::Convert mcvt(MDirection::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
	if (!out.setMV(i, mcvt(dynamic_cast<const MVDirection &>
			       (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
   } else if (in.isMFrequency()) {
      MFrequency::Ref outRef;
      MFrequency::Types tp;
      if (MFrequency::getType(tp, outref)) outRef.setType(tp);
      else outRef.setType(MFrequency::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMFrequency()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      MFrequency::Convert mcvt(MFrequency::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
	if (!out.setMV(i, mcvt(dynamic_cast<const MVFrequency &>
			       (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
    } else if (in.isMDoppler()) {
      MDoppler::Ref outRef;
      MDoppler::Types tp;
      if (MDoppler::getType(tp, outref)) outRef.setType(tp);
      else outRef.setType(MDoppler::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMDoppler()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      MDoppler::Convert mcvt(MDoppler::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
        if (!out.setMV(i, mcvt(dynamic_cast<const MVDoppler &>
                               (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
    } else if (in.isMRadialVelocity()) {
      MRadialVelocity::Ref outRef;
      MRadialVelocity::Types tp;
      if (MRadialVelocity::getType(tp, outref)) outRef.setType(tp);
      else outRef.setType(MRadialVelocity::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMRadialVelocity()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      MRadialVelocity::Convert
	mcvt(MRadialVelocity::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
        if (!out.setMV(i, mcvt(dynamic_cast<const MVRadialVelocity &>
                               (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
    } else if (in.isMBaseline()) {
      MBaseline::Ref outRef;
      MBaseline::Types tp;
      if (MBaseline::getType(tp, outref)) outRef.setType(tp);
      else outRef.setType(MBaseline::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMBaseline()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      MBaseline::Convert mcvt(MBaseline::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
        if (!out.setMV(i, mcvt(dynamic_cast<const MVBaseline &>
                               (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
    } else if (in.isMuvw()) {
      Muvw::Ref outRef;
      Muvw::Types tp;
      if (Muvw::getType(tp, outref)) outRef.setType(tp);
      else outRef.setType(Muvw::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMuvw()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      Muvw::Convert mcvt(Muvw::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
        if (!out.setMV(i, mcvt(dynamic_cast<const MVuvw &>
                               (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
    } else if (in.isMEarthMagnetic()) {
      MEarthMagnetic::Ref outRef;
      MEarthMagnetic::Types tp;
      if (MEarthMagnetic::getType(tp, outref)) outRef.setType(tp);
      else outRef.setType(MEarthMagnetic::DEFAULT);
      outRef.set(*frame_p);
      if (!mo.isEmpty()) {
	if (mo.isMEarthMagnetic()) outRef.set(mo.asMeasure());
	else {
	  error += "Non-conforming offset measure type\n";
	  return False;
	};
      };
      MEarthMagnetic::Convert
	mcvt(MEarthMagnetic::Convert(in.asMeasure(), outRef));
      out = MeasureHolder(mcvt());
      out.makeMV(in.nelements());
      for (uInt i=0; i<in.nelements(); i++) {
        if (!out.setMV(i, mcvt(dynamic_cast<const MVEarthMagnetic &>
                               (*in.getMV(i))).getValue())) {
	  error += "Cannot get extra measure value in DOmeasures::measures\n";
	  return False;
	};
      };
    };
    if (out.isEmpty()) {
      error += "No measure created; probably unknow measure type\n";
      return False;
    };
  } catch (AipsError (x)) {
    error += "Cannot convert due to missing frame information\n";
    return False;
  };
  return True;
}


// measure v converted to rf with optional offset
::casac::record*
measures::measure(const ::casac::record& v, const std::string& rf, const ::casac::record& off)
{
  String error;
  ::casac::record *retval(0);
  try {
    String outref(rf);
    String error;


    Record *pOff = toRecord(off);
    if (pOff->nfields() > 0) {
      MeasureHolder mo;
      if (pOff->isDefined("offset")) {
	if (!mo.fromRecord(error, *pOff)) {
	  error += String("Non-measure type offset in measure conversion\n");
	  *itsLog << LogIO::WARN << error << LogIO::POST;
	  error = "";
	} else {
	  mo.asMeasure().getRefPtr()->set(*frame_p);
	}
      }
    }

    MeasureHolder in;
    Record *pV = toRecord(v);
    if (! (in.fromRecord(error, *pV) && in.isMeasure()) ) {
      error += String("Non-measure type in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete pV;
      delete pOff;
      return retval;
    };
    delete pV;
    //    in.asMeasure().getRefPtr()->set(*frame_p);
  
    MeasureHolder out;

    if (!measures::measure(error, out, in, outref, *pOff)) {
	error += "Cannot convert measure";
	*itsLog << LogIO::WARN << error << LogIO::POST;
	error = "";
        delete pOff;
	return retval;
    }
    delete pOff;

    Record outRec;
    if (!out.toRecord(error, outRec)) {
      error += "Failed to generate return value.\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return 0;
    };
    retval = fromRecord(outRec);
    return retval;
  } catch (AipsError x) {
    error += "Cannot convert due to missing frame information\n";
    *itsLog << LogIO::SEVERE << error << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return 0;
}


bool 
measures::doframe(const String &in) {
  String error;
  try {
    delete pcomet_p; pcomet_p = 0;
    if (in.empty()) {
      pcomet_p = new MeasComet;
    } else {
      pcomet_p = new MeasComet(in);
    };
    if (!pcomet_p->ok()) {
      delete pcomet_p; pcomet_p = 0;
      return false;
    };
    frame_p->set(*pcomet_p);
  } catch (AipsError (x)) {
    *itsLog << LogIO::SEVERE << error << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
    return false;
  }
  return true;
}

// set a frame element
// specify an element of the measurement frame; v as measure
bool
measures::doframe(const ::casac::record& v)
{
  String error;
  try {

  /*  if (ismeasure(v)) {
	doframeRec.val := v;
	if ((v.type == 'frequency' && (v.refer=='rest' ||
				       v.refer == 'REST')) || 
	    defaultservers.run(private.agent, doframeRec)) {
	  private.framestack[v.type] := v;
	  if (dq.testbf()) public.showframe();
	  return T;
	};
      };  */
    MeasureHolder in;
    Record *pV = toRecord(v);
    if (!in.fromRecord(error,*pV)) {
      error += String ("Non-measure type v in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete pV;
      return false;
    }
    delete pV;
    if (!in.isMeasure()) {
      *itsLog << LogIO::SEVERE << "Illegal or unnecessary measure specified for frame" << LogIO::POST;
      return false;
    }
    if (in.isMPosition() || in.isMDirection() ||
	in.isMEpoch() || in.isMRadialVelocity()) {
      ostringstream tmpos;
      tmpos << in.asMeasure();
      *itsLog << LogIO::NORMAL << tmpos.str() << endl;
      frame_p->set(in.asMeasure());
      return true;;
    };
    return false;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return false;
}

bool 
measures::doframe(const MeasureHolder &in) {
  try {
    if (in.isMPosition() || in.isMDirection() ||
	in.isMEpoch() || in.isMRadialVelocity()) {
      frame_p->set(in.asMeasure());
      return True;
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return False;
}

// framenow specifies a frame time of now
bool
measures::framenow()
{
  try {
    MEpoch now((casa::Quantity(Time().modifiedJulianDay(), "d")), MEpoch::UTC);
    measures::doframe(now);
    return true;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    return false;
  }
}

// show specified frame elements
std::string
measures::showframe()
{
  std::string s("False");
  try {
    if (frame_p) {
      ostringstream oss;
      oss << *frame_p;
      //*itsLog << LogIO::NORMAL << oss.str() << endl << LogIO::POST;
      s = oss.str();
    }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return s;
}

casa::MeasureHolder
measures::doptorv(const casa::String &rf, const casa::MeasureHolder &v)
{
  MeasureHolder returnval;
  try {
    MRadialVelocity::Ref outRef;
    MRadialVelocity tout;
    tout.giveMe(outRef, rf);
    returnval = MeasureHolder(MRadialVelocity::
			      fromDoppler(v.asMDoppler(), 
					  static_cast<MRadialVelocity::Types>
					  (outRef.getType())));
    uInt nel(v.nelements());
    if (nel>0) {
      returnval.makeMV(nel);
      MDoppler::Convert mfcv(v.asMDoppler(),
			     v.asMDoppler().getRef());
      for (uInt i=0; i<nel; i++) {
	returnval.setMV(i, MRadialVelocity::
			fromDoppler(mfcv(v.getMV(i)),
				    static_cast<MRadialVelocity::Types>
				    (outRef.getType())).getValue());
      };
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: "
	    << x.getMesg() << LogIO::POST;
  }
  return returnval;
};

// toradialvelocity (rf, v0) -> radialvelocity as rf, doppler v0
::casac::record*
measures::toradialvelocity(const std::string& rf, const ::casac::record& v0)
{
  try {
    ::casac::record *retval(0);
    String error;

    MeasureHolder in;
    Record *pV = toRecord(v0);
    if (!in.fromRecord(error,*pV)) {
      error += String ("Non-measure type v0 in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete pV;
      return 0;
    }
    delete pV;
    MeasureHolder out;
    if (in.isMeasure() && in.isMDoppler()) {
      out = measures::doptorv(rf,in);
    } else {
      *itsLog << LogIO::SEVERE << "Illegal Doppler specified" << LogIO::POST;
      return 0;
    }
    if (out.isMeasure()) {
      Record outRec;
      if (!out.toRecord(error, outRec)) {
	error += "Failed to generate return value.\n";
	*itsLog << LogIO::SEVERE << error << LogIO::POST;
	return 0;
      };
      retval = fromRecord(outRec);
      return retval;
    }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  };
  return 0;
}

casa::MeasureHolder
measures::doptofreq(const casa::String &rf, const casa::MeasureHolder &v,
		    const casa::Quantity &rfq)
{
  MeasureHolder returnval;

  try {
    MFrequency::Ref outRef;
    MFrequency tout;
    tout.giveMe(outRef, rf);
    returnval =
      MeasureHolder(MFrequency::fromDoppler(v.asMDoppler(),
					    MVFrequency(rfq),
					    static_cast<MFrequency::Types>
					    (outRef.getType())));
    uInt nel(v.nelements());
    if (nel>0) {
      returnval.makeMV(nel);
      MDoppler::Convert mfcv(v.asMDoppler(),
			     v.asMDoppler().getRef());
      for (uInt i=0; i<nel; i++) {
	returnval.
	  setMV(i, MFrequency::
		fromDoppler(mfcv(v.getMV(i)),
			    MVFrequency(rfq),
			    static_cast<MFrequency::Types>
			    (outRef.getType())).getValue());
      };
    };
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return returnval;
};

// tofrequency (rf, v0, rfq) -> frequency as rf, doppler v0, rest frequency rfq
::casac::record*
measures::tofrequency(const std::string& rf, const ::casac::record& v0, const ::casac::record& rfq)
{
  try {
    ::casac::record *retval(0);
    String error;

    //deal with rest frequency given as frequency measure or frequency quantity
    Record *p_rfq = toRecord(rfq);
    QuantumHolder qh_rfq;
    MeasureHolder mh_rfq;
    casa::Quantity q_rfq;
    if (qh_rfq.fromRecord(error, *p_rfq)) {
      q_rfq = qh_rfq.asQuantity();
    } else if ( (mh_rfq.fromRecord(error,*p_rfq)) &&
		mh_rfq.isMeasure() && mh_rfq.isMFrequency() &&
		(qh_rfq.fromRecord(error, p_rfq->
				   asRecord(RecordFieldId("m0")))) ) {
      q_rfq = qh_rfq.asQuantity();
    } else {
      *itsLog << LogIO::SEVERE << "Illegal rest frequency specified"
	      << LogIO::POST;
      delete p_rfq;
      return retval;
    }
    delete p_rfq;

    // deal with input doppler measure
    MeasureHolder mh_v0;
    Record *p_v0 = toRecord(v0);
    if (!mh_v0.fromRecord(error,*p_v0)) {
      error += String ("Non-measure type v0 in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_v0;
      return retval;
    }
    delete p_v0;

    MeasureHolder out;
    // if (ismeasure(v0) && v0.type == 'doppler' && is_quantity(rfq) &&
    //	  dq.compare(rfq, dq.quantity(1.,'Hz'))) {
    casa::Quantity qcomp(1.0,"Hz");// used to compare dimensionality of units
    if (mh_v0.isMeasure() && mh_v0.isMDoppler() && qh_rfq.isQuantum() &&
	(q_rfq.getFullUnit().getValue() == qcomp.getFullUnit().getValue())) {
      out = measures::doptofreq(rf,mh_v0,q_rfq);
    } else {
      *itsLog << LogIO::SEVERE << "Illegal Doppler or rest frequency specified" << LogIO::POST;
      return 0;
    }
    if (out.isMeasure()) {
      Record outRec;
      if (!out.toRecord(error, outRec)) {
	error += "Failed to generate return value.\n";
	*itsLog << LogIO::SEVERE << error << LogIO::POST;
	return 0;
      };
      retval = fromRecord(outRec);
      return retval;
    }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return 0;
}


MeasureHolder
todop(String error, MeasureHolder v0, casa::Quantity rfq) {
  MeasureHolder returnval;
  error = "";

  if (v0.isMRadialVelocity()) {
    returnval = MRadialVelocity::toDoppler(v0.asMeasure());
    uInt nel(v0.nelements());
    if (nel>0) {
      returnval.makeMV(nel);
      MRadialVelocity::Convert mfcv(v0.asMRadialVelocity(),
				    v0.asMRadialVelocity().getRef());
      for (uInt i=0; i<nel; i++) {
	returnval.setMV(i, MRadialVelocity::
			  toDoppler(mfcv(v0.getMV(i))).
			  getValue());
      };
    };
  } else if (v0.isMFrequency()) {
    returnval = MFrequency::toDoppler(v0.asMeasure(),
					MVFrequency(rfq));
    uInt nel(v0.nelements());
    if (nel>0) {
      returnval.makeMV(nel);
      MFrequency::Convert mfcv(v0.asMFrequency(),
			       v0.asMFrequency().getRef());
      for (uInt i=0; i<nel; i++) {
	returnval.setMV(i, MFrequency::
			  toDoppler(mfcv(v0.getMV(i)),
				    MVFrequency(rfq)).
			  getValue());
      };
    };
  } else {
    error = "todop can only convert MFrequency or MRadialVelocity";
  };
  return returnval;
};


// Doppler as rf, radvel/freq v0, rest frequency rfq
::casac::record*
measures::todoppler(const std::string& rf, const ::casac::record& v0, const ::casac::variant& rfq)
{
  ::casac::record *retval(0);

  try {

  String error;
  casa::Quantity q_rfq;
  casa::MFrequency m_rfq;
  try{
    casaMFrequency(rfq, m_rfq);
    q_rfq=m_rfq.get("Hz");
  }
  catch(...){
    q_rfq=casaQuantity(rfq);
  }
  /*
  Record *pRfq = toRecord(rfq);

  QuantumHolder qh_rfq;
  MeasureHolder mh_rfq;
  casa::Quantity q_rfq;
  if (qh_rfq.fromRecord(error, *pRfq)) {
    q_rfq = qh_rfq.asQuantity();
  } else if ( (mh_rfq.fromRecord(error,*pRfq)) &&
	      mh_rfq.isMeasure() && mh_rfq.isMFrequency() &&
	      (qh_rfq.fromRecord(error, pRfq->
				 asRecord(RecordFieldId("m0")))) ) {
    q_rfq = qh_rfq.asQuantity();
  } else {
    *itsLog << LogIO::SEVERE << "Illegal rest frequency specified"
	    << LogIO::POST;
    delete pRfq;
    return retval;
  }
  delete pRfq;
  */

  MeasureHolder mh_v;
  Record *p_v0 = toRecord(v0);
  if (!mh_v.fromRecord(error,*p_v0)) {
    error += String ("Non-measure type radvel/freq in measure conversion\n");
    *itsLog << LogIO::SEVERE << error << LogIO::POST;
    delete p_v0;
    return retval;
  }
  delete p_v0;

  MeasureHolder returnval;

  casa::Quantity qcomp(1.0,"Hz");// used to compare dimensionality of units

  // if (ismeasure(v0)) {
  //   if (v0.type == 'radialvelocity') {
  //     loc := private.todop(v0, dq.quantity(1.,'Hz'));
  if (mh_v.isMRadialVelocity()) {
    returnval = todop(error, mh_v, casa::Quantity(1.,"Hz"));
  // } else if (v0.type == 'frequency' && is_quantity(rfq) &&
  //	        dq.compare(rfq, dq.quantity(1.,'Hz'))) {
  //   loc := private.todop(v0, rfq);
  } else if (mh_v.isMFrequency() && 
	     (q_rfq.getFullUnit().getValue() ==
	      qcomp.getFullUnit().getValue())) {
    returnval = todop(error, mh_v, q_rfq);
  } else {
    error += "Illegal Doppler or rest frequency specified."
      "  todoppler can only convert MFrequency or MRadialVelocity";
    *itsLog << LogIO::SEVERE << error << LogIO::POST;
  };

  if (returnval.isMeasure()) {
    MeasureHolder returnval2;
    Record off;
    measures::measure(error, returnval2, returnval, rf, off);

    Record outRec;
    if (!returnval2.toRecord(error, outRec)) {
      error += "Failed to generate return value.\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return retval;
    };
    retval = fromRecord(outRec);
  }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    return retval;
  }
  return retval;
}

// torestfrequency (v0, d0) -> rest frequency from freq v0, doppler d0
::casac::record*
measures::torestfrequency(const ::casac::record& v0, const ::casac::record& d0)
{
  try {
    String error;
    ::casac::record *retval(0);

    MeasureHolder mh_v0;
    Record *p_v0 = toRecord(v0);
    if (!mh_v0.fromRecord(error,*p_v0)) {
      error += String ("Non-measure type v0 in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_v0;
      return 0;
    }
    delete p_v0;

    Record *p_d0 = toRecord(d0);
    MeasureHolder mh_d0;
    if (!mh_d0.fromRecord(error,*p_d0)) {
      error += String ("Non-measure type d0 in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_d0;
      return 0;
    }
    delete p_d0;

    MeasureHolder returnval;

    if (mh_v0.isMeasure() && mh_v0.isMFrequency() &&
	mh_d0.isMeasure() && mh_d0.isMDoppler()) {
      returnval = 
	MeasureHolder(MFrequency::toRest(mh_v0.asMFrequency(),
					 mh_d0.asMDoppler()));
      uInt nel(mh_v0.nelements());
      if (nel != mh_d0.nelements()) {
	*itsLog << LogIO::SEVERE 
		<< "Incorrect length of doppler or frequency in torest"
		<< LogIO::POST;
	return 0;
      };
      if (nel>0) {
	returnval.makeMV(nel);
	MFrequency::Convert mfcv(mh_v0.asMFrequency(),
				 mh_v0.asMFrequency().getRef());
	MDoppler::Convert mdcv(mh_d0.asMDoppler(),
			       mh_d0.asMDoppler().getRef());
	for (uInt i=0; i<nel; i++) {
	  returnval.setMV(i, MFrequency::
			  toRest(mfcv(mh_v0.getMV(i)),
				 mdcv(mh_d0.getMV(i))).
			  getValue());
	};
      };
    } else {
      *itsLog << LogIO::SEVERE <<"Illegal Doppler or rest frequency specified"
	      << LogIO::POST;
      return 0;
    }
    Record outRec;
    if (!returnval.toRecord(error, outRec)) {
      error += "Failed to generate return value.\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return 0;
    };
    retval = fromRecord(outRec);
    return retval;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    return 0;
  }
}

// Rise/set sidereal time(coord, elev)
::casac::record*
measures::rise(const ::casac::variant& crd, const ::casac::variant& ev)
{
  ::casac::record * rstat = new ::casac::record();
  String error;
  try {
    Record outrec;
    casa::Quantity elev;
    if(toCasaString(ev)==String("")){
      elev=casa::Quantity(0.0, "deg");
    }
    else{
	elev=casaQuantity(ev);
    }
    MDirection srcDir;
    if(casaMDirection(crd, srcDir)){
	MPosition pos=frame_p->position();
      
	MDirection hd(MDirection::Convert(srcDir, 
					  MDirection::Ref(MDirection::HADEC,
							  *frame_p))());
	MDirection c(MDirection::Convert(srcDir, 
					 MDirection::Ref(MDirection::APP,
							 *frame_p))());
	
	Double longi;
	Double lati;
	frame_p->getLong(longi);
	frame_p->getLat(lati);
      
      
	Double ct= (sin(elev.get("rad").getValue())-
		    sin(hd.getAngle("rad").getValue()(1))*sin(lati))/
	  (cos(hd.getAngle("rad").getValue()(1))*cos(lati));
	if(ct >= 1.0){
	  outrec.define("rise", "below");
	  outrec.define("set", "below");
	}
	else if(ct <= -1.0){
	  outrec.define("rise", "above");
	  outrec.define("set", "above");
	}
	else{
	  Double a= acos(ct);
	  //	Double appRa=MVAngle(c.getAngle("rad").getValue()(0)).binorm(0.0).get("rad").getValue();
	  Double appRa=c.getAngle("rad").getValue()(0);
	  String err;
	  Record riserec;
       	  QuantumHolder(casa::Quantity(appRa-a, "rad")).toRecord(err, riserec);
	  outrec.defineRecord("rise", riserec);
	  QuantumHolder(casa::Quantity(appRa+a, "rad")).toRecord(err, riserec);
	  outrec.defineRecord("set", riserec);			
	  ///
	}
	
    }
    else{
      *itsLog << LogIO::SEVERE << "No rise/set coordinates specified"
	      << LogIO::POST;

    }
    if(rstat)
      delete rstat;
    rstat=fromRecord(outrec);  

    // TODO : IMPLEMENT ME HERE !
  /*
#
    const public.rise := function(crd, ev='5deg') {
      if (!ismeasure(crd)) fail('No rise/set coordinates specified');
      if (!ismeasure(private.getwhere())) {
	dq.errorgui('Specify where you are in Frame');
	fail('No rise/set Frame->Where specified');
      };
      private.fillnow();
      hd := public.measure(crd, 'hadec');
      c := public.measure(crd, 'app');
      if (!ismeasure(hd) || !ismeasure(c)) fail('Cannot get HA for rise/set');
      ps := private.getwhere();
      ct := dq.div(dq.sub(dq.sin(ev),
			  dq.mul(dq.sin(hd.m1),
				 dq.sin(ps.m1))),
		   dq.mul(dq.cos(hd.m1), dq.cos(ps.m1)));
      if (ct.value >= 1) return "below below"; 
      if (ct.value <= -1) return "above above"; 
      a := dq.acos(ct);
      return [rise=dq.sub(dq.norm(c.m0, 0), a),
              set=dq.add(dq.norm(c.m0, 0), a)]
    }
#
  */
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Failed to calculate rise/set;\n may be the frames have not been initialized properly" << LogIO::POST;
  }
  return rstat;
}

// Rise/set times(coord, elev)
::casac::record*
measures::riseset(const ::casac::variant& crd, const ::casac::variant& ev)
{
  ::casac::record *rstat = 0;
  String error;
  try {
    
    
      ::casac::record* risevals=rise(crd, ev);
      Record *recRise=toRecord(*risevals);
      if((recRise->nfields() == 0) || (recRise->dataType(0) == TpString)){
      
	return risevals;
      }
      Double riseLAST;
      Record tmprec;
      String err;
      tmprec=recRise->asRecord("rise");
      QuantumHolder qh;
      qh.fromRecord(err, tmprec);
      riseLAST=MVAngle(qh.asQuantity())().radian();
      //if(riseLAST <0.0)
      //	riseLAST+=C::circle;
      Double setLAST;
      tmprec=recRise->asRecord("set");
      qh.fromRecord(err, tmprec);
      setLAST=MVAngle(qh.asQuantity())().radian();
      if(setLAST < riseLAST)
      	setLAST+=C::circle;
      MEpoch newEp(frame_p->epoch());
      newEp.setRefString("R_UTC");
      newEp.getRefPtr()->set(*frame_p);
      MEpoch refepoch=MEpoch::Convert(newEp, MEpoch::Ref(MEpoch::LAST, *frame_p))();
      Double refval=refepoch.get("d").getValue();
      *itsLog << "LAST of rise= " 
	      << MVAngle(riseLAST).string(MVAngle::TIME, 8) << LogIO::POST;
      refepoch.set(MVEpoch(casa::Quantity(refval+ riseLAST/C::circle, "d")), 
		   MEpoch::Ref(MEpoch::LAST, *frame_p));
      MeasureHolder mh(MEpoch::Convert(refepoch, MEpoch::Ref(MEpoch::UTC, *frame_p))());
      ostringstream os;
      os  << "UTC of rise= " 
      	      << MVTime(MEpoch::Convert(refepoch, MEpoch::Ref(MEpoch::UTC, *frame_p))().getValue()).string(MVTime::YMD)	      << endl;
      Record riseRec;
      Record riseRecUTC;
      mh.toRecord(err, riseRecUTC);
      riseRec.defineRecord("utc", riseRecUTC);
      Record riseRecLAST;
      MeasureHolder(refepoch).toRecord(err, riseRecLAST);
      riseRec.defineRecord("last", riseRecLAST);

      *itsLog << "LAST of  set= " 
	      << MVAngle(setLAST).string(MVAngle::TIME, 8) << LogIO::POST;
      refepoch.set(MVEpoch(casa::Quantity(refval+ setLAST/C::circle, "d")));
      
      Record setRec;
      Record setRecUTC;
      MeasureHolder(MEpoch::Convert(refepoch, MEpoch::Ref(MEpoch::UTC, *frame_p))()).toRecord(err, setRecUTC);

      os  << "UTC of  set= " 
      	      << MVTime(MEpoch::Convert(refepoch, MEpoch::Ref(MEpoch::UTC, *frame_p))().getValue()).string(MVTime::YMD)	      << endl;
      *itsLog << os.str() << LogIO::POST;
      setRec.defineRecord("utc", setRecUTC);
      Record setRecLAST;
      MeasureHolder(refepoch).toRecord(err, setRecLAST);
      setRec.defineRecord("last", setRecLAST);
      Record outrec;
      outrec.defineRecord("rise", riseRec);
      outrec.defineRecord("set", setRec);
      rstat=fromRecord(outrec);
      if(recRise)
	delete recRise;
      if(risevals)
	delete risevals;

    
    

    // TODO : IMPLEMENT ME HERE !
  /*
# Rise/set times(coord, elev)
#
    const public.riseset := function(crd, ev='5deg') {
	a := public.rise(crd, ev);
	if (is_fail(a)) fail;
	if (is_string(a)) {
	  return [solved=F,
		 rise=[last=a[1], utc=a[1]],
		 set=[last=a[2], utc=a[2]]];
	};
	x := a;
	ofe := public.measure(private.framestack['epoch'], 'utc');
	if (!ismeasure(ofe)) ofe := public.epoch('utc', 'today');
	for (i in 1:2) {
	  x[i] :=
	      public.measure(public.epoch('last',
					  dq.totime(a[i]),
					  off=public.epoch('r_utc',
							   dq.add(ofe.m0,
								  '0.5d'))),
			     'utc');
	};
	return [solved=T,
	       rise=[last=public.epoch('last', dq.totime(a[1])),
		    utc=x[1]],
	       set=[last=public.epoch('last', dq.totime(a[2])),
		   utc=x[2]]];
    }
#
  */
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return rstat;
}

// posangle
casa::Quantity
measures::posangle (const MDirection& md1, const MDirection& md2) {
  MDirection x(md1);
  MDirection y(md2);
  x.getRefPtr()->set(*frame_p);
  y.getRefPtr()->set(*frame_p);
  if (x.isModel()) x = MDirection::Convert(x, MDirection::DEFAULT)();
  if (y.isModel()) y = MDirection::Convert(y, MDirection::DEFAULT)();
  if (x.getRefPtr()->getType() != y.getRefPtr()->getType()) {
    y = MDirection::Convert(y, MDirection::castType
			    (x.getRefPtr()->getType()))();
  };
  return x.getValue().positionAngle(y.getValue(), "deg");
};

// get postion angle from direction m1 and m2
::casac::record*
measures::posangle(const ::casac::record& m1, const ::casac::record& m2)
{
  std::vector<double> retval(1);
  String unit("deg");
  retval[0]=0;
  String error;

  try {
    MeasureHolder in1;
    Record *p_m1 = toRecord(m1);
    if (!in1.fromRecord(error,*p_m1)) {
      error += String("Non-measure type m1 in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_m1;
      return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
    }
    delete p_m1;
    if (!in1.isMDirection()) {
      error += "Trying to convert non-direction to direction\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
    };
    MDirection x = in1.asMDirection();

    MeasureHolder in2;
    Record *p_m2 = toRecord(m2);
    if (!in2.fromRecord(error,*p_m2)) {
      error += String("Non-measure type m2 in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_m2;
      return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
    }
    delete p_m2;
    if (!in2.isMDirection()) {
      error += "Trying to convert non-direction to direction\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
    };
    MDirection y = in2.asMDirection();
    /*
    x.getRefPtr()->set(*frame_p);
    y.getRefPtr()->set(*frame_p);
    if (x.isModel()) x = MDirection::Convert(x, MDirection::DEFAULT)();
    if (y.isModel()) y = MDirection::Convert(y, MDirection::DEFAULT)();
    if (x.getRefPtr()->getType() != y.getRefPtr()->getType()) {
      y = MDirection::Convert(y, MDirection::castType
			      (x.getRefPtr()->getType()))();
    };
    return recordFromQuantity(casa::Quantity(x.getValue().positionAngle(y.getValue(), "deg")));
    */
    return recordFromQuantity(posangle(x,y));
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
    return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
  }
}

// get separation between directions m1 and m2
::casac::record*
measures::separation(const ::casac::record& m1, const ::casac::record& m2)
{
  std::vector<double> retval(1);
  String unit("deg");
  retval[0]=0;
  String error;

  try {
    MeasureHolder in1;
    Record *p_m1 = toRecord(m1);
    if (!in1.fromRecord(error,*p_m1)) {
      error += String("Non-measure type m1 in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_m1;
      return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
    }
    delete p_m1;
    if (!in1.isMDirection()) {
      error += "Trying to convert non-direction to direction\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
    };
    MDirection x = in1.asMDirection();

    MeasureHolder in2;
    Record *p_m2 = toRecord(m2);
    if (!in2.fromRecord(error,*p_m2)) {
      error += String("Non-measure type m2 in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_m2;
      return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
    }
    delete p_m2;

    if (!in2.isMDirection()) {
      error += "Trying to convert non-direction to direction\n";
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
    };
    MDirection y = in2.asMDirection();

    x.getRefPtr()->set(*frame_p);
    y.getRefPtr()->set(*frame_p);
    if (x.isModel()) x = MDirection::Convert(x, MDirection::DEFAULT)();
    if (y.isModel()) y = MDirection::Convert(y, MDirection::DEFAULT)();
    if (x.getRefPtr()->getType() != y.getRefPtr()->getType()) {
      y = MDirection::Convert(y, MDirection::castType
			      (x.getRefPtr()->getType()))();
    };
    return recordFromQuantity(x.getValue().separation(y.getValue(), "deg"));
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
	    << LogIO::POST;
    return recordFromQuantity(Quantum<Vector<Double> >(retval,unit));
  }
}

// get extra values (for direction and position mainly) as 3D vector
::casac::record*
measures::addxvalue(const ::casac::record& a)
{
  ::casac::record* retval=0;

  try{
    String error;
    MeasureHolder val;

    Record *p_a = toRecord(a);
    if (!val.fromRecord(error,*p_a)) {
      error += String ("Non-measure type a in measure conversion\n");
      *itsLog << LogIO::SEVERE << error << LogIO::POST;
      delete p_a;
      return retval;
    }
    delete p_a;

    if (!val.isMeasure()) {
      *itsLog << LogIO::SEVERE << "Non-measure for addxvalue()"
	      << LogIO::POST;
      return retval;
    }

    Vector<Quantum<Double> > res =
      val.asMeasure().getData()->getXRecordValue();
    uInt len = res.size();
    if (len > 0) {
      std::vector<double> arg0(len);
      for (uInt i=0 ; i < len; i++) {
	arg0[i]=res[i].getValue();
      } //no support for array of quantity
      return recordFromQuantity(Quantum<Vector<Double> >(arg0,res[0].getUnit()));
    }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

bool
measures::done()
{
  try {
    if (pcomet_p) delete pcomet_p;
    pcomet_p=0;
    delete frame_p;
    frame_p=new MeasFrame();
    return true;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    return false;
  }
}

bool
measures::ismeasure(const ::casac::record& a)
{
  bool retval(false);

  try{
    String error;
    MeasureHolder val;

    Record *p_a = toRecord(a);
    if (val.fromRecord(error,*p_a)) {
      retval = val.isMeasure();
    }
    delete p_a;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
  }
  return retval;
}

std::string
measures::type()
{
  return "measures";
}


casa::Quantity
measures::casaQuantityFromVar(const ::casac::variant& theVar){
  casa::Quantity retval;
  try {
    casa::QuantumHolder qh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!qh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting quantity from string "<< LogIO::POST;
      }
      retval=qh.asQuantity();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      //NOW the record has to be compatible with QuantumHolder::toRecord
      ::casac::variant localvar(theVar); //cause its const
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!qh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting quantity from record "<< LogIO::POST;
      }
      delete ptrRec;
      retval=qh.asQuantity();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::MeasureHolder
measures::casaMeasureHolderFromVar(const ::casac::variant& theVar){
  casa::MeasureHolder mh;
  try {
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return mh;
}

bool
measures::casacRec2MeasureHolder(casa::MeasureHolder& mh,
				   const ::casac::record& theRec){
  bool rstat(false);
  try {
    String error;
    Record * ptrRec = toRecord(theRec);
    if(!mh.fromRecord(error, *ptrRec)){
      *itsLog << LogIO::SEVERE << "Error [" << error
	      << "] in converting record to measure "<< LogIO::POST;
    }
    delete ptrRec;
    rstat = mh.isMeasure();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
return rstat;
}

casa::MDirection
measures::casaMDirectionFromVar(const ::casac::variant& theVar){
  casa::MDirection retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMDirection();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMDirection();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::MDoppler
measures::casaMDopplerFromVar(const ::casac::variant& theVar){
  casa::MDoppler retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMDoppler();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMDoppler();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::MEpoch
measures::casaMEpochFromVar(const ::casac::variant& theVar){
  casa::MEpoch retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMEpoch();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMEpoch();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::MFrequency
measures::casaMFrequencyFromVar(const ::casac::variant& theVar){
  casa::MFrequency retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMFrequency();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMFrequency();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::MPosition
measures::casaMPositionFromVar(const ::casac::variant& theVar){
  casa::MPosition retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMPosition();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMPosition();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::MRadialVelocity
measures::casaMRadialVelocityFromVar(const ::casac::variant& theVar){
  casa::MRadialVelocity retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMRadialVelocity();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMRadialVelocity();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::MBaseline
measures::casaMBaselineFromVar(const ::casac::variant& theVar){
  casa::MBaseline retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMBaseline();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMBaseline();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::Muvw
measures::casaMuvwFromVar(const ::casac::variant& theVar){
  casa::Muvw retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMuvw();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMuvw();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

casa::MEarthMagnetic
measures::casaMEarthMagneticFromVar(const ::casac::variant& theVar){
  casa::MEarthMagnetic retval;
  try {
    casa::MeasureHolder mh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!mh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting string to measure "<< LogIO::POST;
      }
      retval = mh.asMEarthMagnetic();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(theVar);
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!mh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting record to measure "<< LogIO::POST;
      }
      delete ptrRec;
      retval = mh.asMEarthMagnetic();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

/*
bool
measures::selftest()
{
  try {
    ::casac::record *rtn = 0;
    std::vector<double> v(1);
    v[0]=30.0;
    Quantity v0(v,"deg");
    v[0]=40.0;
    Quantity v1(v,"deg");
    ::casac::record empty;
    //    rtn = measures::direction("J2000",v0,v1,empty);
    cout << measures::dirshow(*rtn) << endl;
    return true;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
    return false;
  }
}

::casac::record*
measures::echo(const ::casac::record& v, const bool godeep)
{
  ::casac::record *retval(0);
  try {
    Record *inrec = toRecord(v);
    if (godeep) {
      String error("");
      MeasureHolder mh;
      if (! mh.fromRecord(error,*inrec)) {
	*itsLog << LogIO::WARN << error
		<< "\nInput record cannot be stored in a MeasureHolder"
		<< LogIO::POST;
      } else {
	Record outRec;
	if (! mh.toRecord(error, outRec) ) {
	  *itsLog << LogIO::WARN << error
		  << "\nOutput record cannot be generated from MeasureHolder"
		  << LogIO::POST;
	} else {
	  retval=fromRecord(outRec);
	}
      }
    } else {
      retval = fromRecord(*inrec);
    }
    delete inrec;
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reports: " << x.getMesg()
            << LogIO::POST;
  }
  return retval;
}
*/

/*
#
# Subsidiary gui methods
#

#
# Recalculate Radial velocity from frequency
#
    private.getrv := function(d, ref b, tp='true') {
      if (is_string(d)) {
	val b := d;
      } else {
	if (!has_field(private.framestack, 'frequency') ||
	    !ismeasure(private.framestack.frequency)) {
	  dq.errorgui('No rest frequency in frame for conversion');
	  fail;
	};
	c := public.toradialvelocity(d.refer,
				     public.todoppler('beta', d,
					    private.framestack.frequency));
	if (!ismeasure(c)) {
	  dq.errorgui('Error in conversion frequency to radial velocity');
	  fail;
	};
	if (tp != 'true') c := public.todoppler(tp, c);
	val b := dq.form.vel(c.m0);
      };
      return T;
    }
#
# Recalculate frequency from Radial velocity
#
    private.getfrq := function(d, ref b) {
      if (is_string(d)) {
	val b := d;
      } else {
	if (!has_field(private.framestack, 'frequency') ||
	    !ismeasure(private.framestack.frequency)) {
	  dq.errorgui('No rest frequency in frame for conversion');
	  fail;
	};
	c := public.tofrequency(d.refer,
				public.todoppler('beta', d),
				private.framestack.frequency);
	if (!ismeasure(c)) {
	  dq.errorgui('Error in conversion radial velocity to frequency');
	  fail;
	};
	val b := dq.form.freq(c.m0);
      };
      return T;
    }
#
# Recalculate rise/set
#
    private.getrs := function(d, ref b) {
      if (is_string(d)) {
	val b := d;
      } else {
	x := d;
	if (dq.getformat('dtime') != 'last') {
	  ofe := public.measure(private.framestack['epoch'], 'utc');
	  if (!ismeasure(ofe)) ofe := public.epoch('utc', 'today');
	  ofe := ofe.m0;
	  for (i in 1:2) {
	    x[i] := public.measure(public.epoch('last',
						dq.totime(d[i]),
						off=public.epoch('r_utc',
								 ofe)),
				   'utc').m0;
	    if (dq.getformat('dtime') == 'solar') {
	      x[i] := dq.add(x[i],
			     dq.totime(private.framestack['position'].m0));
	    };
	  };
	};
	for (i in 1:2) b[i] := dq.form.dtime(x[i]);
      };
    }
#
*/

} // casac namespace

