//# WCPowerScaleHandler.cc: linear, log and exp scaling of data values
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
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

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <display/DisplayCanvas/WCPowerScaleHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	const String WCPowerScaleHandler::POWER_CYCLES="powercycles";

// Default Constructor Required
	WCPowerScaleHandler::WCPowerScaleHandler() :
		WCDataScaleHandler() {
		setDefaultOptions();
	}

// Destructor
	WCPowerScaleHandler::~WCPowerScaleHandler() {
	}

// Install the default options for this object.
	void WCPowerScaleHandler::setDefaultOptions() {
		WCDataScaleHandler::setDefaultOptions();
		itsOptionsPowerCycles = 0.0;
		itsOptionsHistoEqualisation = False;
	}

// Apply new options to this object.
	Bool WCPowerScaleHandler::setOptions(Record &rec, Record &recOut) {
		Bool ret = WCDataScaleHandler::setOptions(rec, recOut);
		Bool localchange = False;
		Bool error;

		localchange = (readOptionRecord(itsOptionsPowerCycles, error, rec ,POWER_CYCLES) || localchange);
		localchange = (readOptionRecord(itsOptionsHistoEqualisation, error,
		                                rec, "histoequalisation") ||
		               localchange);

		ret = (ret || localchange);
		return ret;
	}

// Retrieve the current and default options for this object.
	Record WCPowerScaleHandler::getOptions( bool scrub ) const {
		Record rec = WCDataScaleHandler::getOptions(scrub);

		Record histoequalisation;
		histoequalisation.define("dlformat", "histoequalisation");
		histoequalisation.define("listname", "Histogram equalisation?");
		histoequalisation.define("ptype", "boolean");
		histoequalisation.define("default", Bool(False));
		histoequalisation.define("value", itsOptionsHistoEqualisation);
		histoequalisation.define("allowunset", False);
		rec.defineRecord("histoequalisation", histoequalisation);

		Record powercycles;
		powercycles.define("dlformat", POWER_CYCLES);
		powercycles.define("listname", "Scaling Power Cycles");
		powercycles.define("ptype", "floatrange");
		powercycles.define("pmin", Float(-5.0));
		powercycles.define("pmax", Float(5.0));
		powercycles.define("presolution", Float(0.1));
		powercycles.define("default", Float(0));
		powercycles.define("value", itsOptionsPowerCycles);
		powercycles.define("provideentry", True);
		powercycles.define("allowunset", False);
		rec.defineRecord(POWER_CYCLES, powercycles);

		return rec;
	}

#define WCPowerScaleHandlerSCALE(Type, TypeEpsilon) \
  out.resize(in.shape()); \
  Bool inDel; \
  const Type *inp = in.getStorage(inDel); \
  Bool outDel; \
  uInt *outp = out.getStorage(outDel); \
  uInt *p = outp; \
  uInt *endp = outp + out.nelements(); \
  const Type *q = inp; \
  Double precalc0, precalc1, precalc2, precalc3; \
  precalc0 = (Double)(rangeMax()); \
  if (itsOptionsPowerCycles < -0.001) { \
    precalc1 = pow((Double)10.0, (Double)(-itsOptionsPowerCycles)) - (Double)1.0; \
    precalc2 = (precalc0 + (Double)1.0) / (Double)(-itsOptionsPowerCycles); \
  } else if (itsOptionsPowerCycles > 0.001) { \
    precalc1 = (precalc0 + (Double)1.0) / (pow((Double)10.0, (Double)itsOptionsPowerCycles) - \
				       (Double)1.0); \
    precalc2 = (Double)0.0; \
  } else { \
    precalc1 = precalc0 + (Double)1.0; \
    precalc2 = (Double)0.0; \
  } \
  precalc3 = (Double)1.0 / ((Double)(domainMax()) - (Double)(domainMin())); \
  Type precalc4 = (Type)(domainMin()); \
  Double frac; \
  if (itsOptionsPowerCycles < -0.001) { \
    while (p < endp) { \
      frac = (Double)(*q++ - precalc4) * precalc3; \
      if (frac < 0.0) { \
	frac = 0.0; \
      } else if (frac > 1.0) { \
	frac = 1.0; \
      } \
      *p = (uInt)(precalc2 * log10(frac * precalc1 + 1.0)); \
      p++; \
    } \
  } else if (itsOptionsPowerCycles > 0.001) { \
    while (p < endp) { \
      frac = (Double)(*q++ - precalc4) * precalc3; \
      if (frac < 0.0) { \
	frac = 0.0; \
      } else if (frac > 1.0) { \
	frac = 1.0; \
      } \
      *p = (uInt)(precalc1 * (pow(10.0, frac * (Double)itsOptionsPowerCycles) - 1.0)); \
      p++; \
    } \
  } else { \
    while (p < endp) { \
      frac = (Double)(*q++ - precalc4) * precalc3; \
      if (frac < 0.0) { \
	frac = 0.0; \
      } else if (frac > 1.0) { \
	frac = 1.0; \
      } \
      *p = (uInt)(precalc0 * frac + 0.5); \
      p++; \
    } \
  } \
  in.freeStorage(inp, inDel); \
  out.putStorage(outp, outDel);


	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<Bool> &in) {
		out.resize(in.shape());
		Bool inDel;
		const Bool * inp = in.getStorage(inDel);
		Bool outDel;
		uInt * outp = out.getStorage(outDel);
		uInt * endp = outp + out.nelements();
		uInt * p = outp;
		const Bool * q = inp;
		while (p < endp)
			*p++ = (*q++) ? rangeMax() : 0;
		in.freeStorage(inp, inDel);
		out.putStorage(outp, outDel);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<uChar> &in) {
		WCPowerScaleHandlerSCALE(uChar, 1);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<Char> &in) {
		WCPowerScaleHandlerSCALE(Char, 1);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<uShort> &in) {
		WCPowerScaleHandlerSCALE(uShort, 1);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<Short> &in) {
		WCPowerScaleHandlerSCALE(Short, 1);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<uInt> &in) {
		WCPowerScaleHandlerSCALE(uInt, 1);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<Int> &in) {
		WCPowerScaleHandlerSCALE(Int, 1);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<uLong> &in) {
		WCPowerScaleHandlerSCALE(uLong, 1);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<Long> &in) {
		WCPowerScaleHandlerSCALE(Long, 1);
		return True;
	}
	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<Float> &inn) {
		Array<Float> in(inn.shape());
		if (itsOptionsHistoEqualisation) {
			histoEqualise(in, inn);
		} else {
			in = inn;
		}
		WCPowerScaleHandlerSCALE(Float, FLT_EPSILON);
		return True;
	}

	Bool WCPowerScaleHandler::operator()(Array<uInt> &out, const Array<Double> &in) {
		WCPowerScaleHandlerSCALE(Double, DBL_EPSILON);
		return True;
	}

	Bool WCPowerScaleHandler::operator()(Array<uInt> & out, const Array<Complex> & in) {
		return WCDataScaleHandler::operator()(out, in);
	}

	Bool WCPowerScaleHandler::operator()(Array<uInt> & out, const Array<DComplex> & in) {
		return WCDataScaleHandler::operator()(out, in);
	}


	Bool WCPowerScaleHandler::histoEqualise(Array<Float> &out,
	                                        const Array<Float> &in) {
		// generate cumulative histogram
		static uInt nbins = 200;
		uInt i;
		Float dmin = min(in), dmax = max(in);
		Float dval;
		Vector<Float> histo(nbins);
		for (i = 0; i < nbins; i++) {
			dval = dmin + (Float)i / (Float)(nbins - 1) * (dmax - dmin);
			MaskedArray<Float> xx(in, in < dval);
			histo(i) = xx.nelementsValid();
		}

		// equalise based on histogram
		out.resize(in.shape());
		Bool inDel;
		const Float *inp = in.getStorage(inDel);
		Bool outDel;
		Float *outp = out.getStorage(outDel);
		Float *p = outp;
		Float *endp = outp + out.nelements();
		const Float *q = inp;
		Float frac;
		Float on_n = Float(1.0) / Float(in.nelements());
		Float multiplier = on_n * (max(in) - dmin);
		while (p < endp) {
			//MaskedArray<Float> xx(in, in < *q);
			//frac = (Float)(xx.nelementsValid());
			i = uInt(Float(*q - dmin) / Float(dmax - dmin) * Float(nbins - 1) + 0.5);
			frac = histo(i);
			*p = (Float)(dmin + frac * multiplier);
			q++;
			p++;
		}
		in.freeStorage(inp, inDel);
		\
		out.putStorage(outp, outDel);
		\
		return True;
	}




} //# NAMESPACE CASA - END

