//# SDFITSDataIterator.cc:  iterates through an SDFITS DATA field
//# Copyright (C) 2000,2001,2002,2003
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

//# Includes

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Containers/Record.h>
#include <coordinates/Coordinates/Projection.h>
#include <casa/Exceptions/Error.h>
#include <fits/FITS/FITSDateUtil.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <casa/Logging/LogIO.h>
#include <dish/SDIterators/SDFITSDataIterator.h>
#include <measures/Measures/Stokes.h>
#include <casa/Quanta/Unit.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Utilities/Assert.h>

#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SDFITSDataIterator::SDFITSDataIterator()
    : freqRegex_p("^FREQ.*"), velRegex_p("^[FV]ELO.*"),
      longRegex_p("^((RA)|(.LON)).*"), latRegex_p("^((DEC)|(.LON)).*"),
      timeRegex_p("^TIME$"), stokesRegex_p("^STOKES$"),
      beamRegex_p("^BEAM$"), receiverRegex_p("^RECEIVER$"),
      freqAxis_p(-1), stokesAxis_p(-1), longAxis_p(-1), latAxis_p(-1),
      timeAxis_p(-1), beamAxis_p(-1), receiverAxis_p(-1),
      obsfreqId_p(-1), lstId_p(-1), restfreqId_p(-1), equinoxId_p(-1),
      timeId_p(-1), exposureId_p(-1), iter_p(0), arrLattice_p(0),
      refFreq_p(0.0), fdelt_p(0.0), timeRange_p(2)
{
    // ensure that the FITS units are in the unit map
    UnitMap::addFITS();
}

SDFITSDataIterator::~SDFITSDataIterator()
{
    delete iter_p;
    iter_p = 0;
    delete arrLattice_p;
    arrLattice_p = 0;
}

Bool SDFITSDataIterator::setrow(const Record &row, String &errMsg) 
{
    errMsg = "";

    // reset some things to default values
    freqAxis_p = stokesAxis_p = longAxis_p = latAxis_p = timeAxis_p = 
	beamAxis_p = receiverAxis_p = -1;

    Bool result = True;
    // make sure that at least one of the data pointers is attached
    if (!fdataPtr_p.isAttached() && !idataPtr_p.isAttached() &&
	!sdataPtr_p.isAttached() && !ddataPtr_p.isAttached()) {
	result = init(row, errMsg);
    }
    if (!result) return result;

    // set the data array values
    if (fdataPtr_p.isAttached()) {
	// nothing complicated to be done here
	arrLattice_p = new ArrayLattice<Float>(*fdataPtr_p);
	AlwaysAssert(arrLattice_p, AipsError);
    } else if (idataPtr_p.isAttached()) {
	// make the ArrayLattice of the right shape
	arrLattice_p = new ArrayLattice<Float>((*idataPtr_p).shape());
	AlwaysAssert(arrLattice_p, AipsError);
	// and convert from one to another
	convertArray(arrLattice_p->asArray(), *idataPtr_p);
    } else if (sdataPtr_p.isAttached()) {
	// make the ArrayLattice of the right shape
	arrLattice_p = new ArrayLattice<Float>((*sdataPtr_p).shape());
	AlwaysAssert(arrLattice_p, AipsError);
	// and convert from one to another
	convertArray(arrLattice_p->asArray(), *sdataPtr_p);
    } else if (ddataPtr_p.isAttached()) {
	// make the ArrayLattice of the right shape
	arrLattice_p = new ArrayLattice<Float>((*ddataPtr_p).shape());
	AlwaysAssert(arrLattice_p, AipsError);
	// and convert from one to another
	convertArray(arrLattice_p->asArray(), *ddataPtr_p);
    } else {
	// this should never happen
	LogIO los;
	los << LogOrigin("SDFITSDataIterator","setrow");
	los << WHERE
	    << "An impossible error occurred, no DATA pointers are attached."
	    << "\nPlease report this serious error."
	    << LogIO::EXCEPTION;
    }

    IPosition dataShape(arrLattice_p->shape());

    // fill the axis descriptors as appropriate
    uInt naxes = dataShape.nelements();
    if (naxes > ctypeIds_p.nelements()) {
	errMsg = "More axes indicated in TDIM than are available in the CTYPE keywords/columns";
	return False;
    }

    crval_p.resize(naxes);
    crpix_p.resize(naxes);
    cdelt_p.resize(naxes);
    crota_p.resize(naxes);
    ctype_p.resize(naxes);
    cunit_p.resize(naxes);

    // crval
    for (uInt i=0;i<naxes;i++) {
	if (crvalIds_p(i) >= 0) {
	    crval_p(i) = row.asDouble(crvalIds_p(i));
	} else {
	    crval_p(i) = 0.0;
	}
    }

    // crpix
    for (uInt i=0;i<naxes;i++) {
	if (crpixIds_p(i) >= 0) {
	    crpix_p(i) = row.asDouble(crpixIds_p(i));
	} else {
	    crpix_p(i) = 1.0;
	}
    }
    // make these 0-relative instead of 1-relative
    crpix_p -= 1.0;

    // eventually need to be more sophisticated regarding cdelt, crota, CD, and PC and WCS
    // cdelt
    for (uInt i=0;i<naxes;i++) {
	if (cdeltIds_p(i) >= 0) {
	    cdelt_p(i) = row.asDouble(cdeltIds_p(i));
	    // cdelts of zero cause problems, setting it to 1.0 should be
	    // okay so long as there is only one pixel on this axis and
	    // that pixel is the reference pixel
	    if (cdelt_p(i) == 0.0) {
		cdelt_p(i) = 1.0;
		if (dataShape(i) != 1 || crpix_p(i) != 0.0) {
		    ostringstream os;
		    os << "CDELT" << (i+1) 
		       << " == 0.0 and there is more than one pixel or reference pixel is not 1.0"
		       << "\nUnable to parse this axis coordinate system";
		    errMsg = String(os);
		    // it might be better to just use cell number here instead of giving up
		    return False;
		}
	    }
	} else {
	    cdelt_p(i) = 1.0;
	}
    }

    // crota
    for (uInt i=0;i<naxes;i++) {
	if (crotaIds_p(i) >= 0) {
	    crota_p(i) = row.asDouble(crotaIds_p(i));
	} else {
	    crota_p(i) = 0.0;
	}
    }

    // ctype
    Bool isGalactic = False;
    for (uInt i=0;i<naxes;i++) {
	// these MUST all be present
	String ctype = row.asString(ctypeIds_p(i));
	ctype_p(i) = ctype;
	// what type of axis is it
	if (ctype.matches(freqRegex_p) || ctype.matches(velRegex_p)) {
	    if (freqAxis_p < 0) freqAxis_p = i;
	    else {
		errMsg = "multiple frequency-like axes";
		return False;
	    }
	} else if (ctype.matches(longRegex_p)) {
	    if (longAxis_p < 0) longAxis_p = i;
	    else {
		errMsg = "multiple longitude-like axes";
		return False;
	    }
	    if (ctype.matches("^G.*")) isGalactic = True;
	    // worth sanity check that lat axis is also galactic?
	} else if (ctype.matches(latRegex_p)) {
	    if (latAxis_p < 0) latAxis_p = i;
	    else {
		errMsg = "multiple lattitude-like axes";
		return False;
	    }
	} else if (ctype.matches(timeRegex_p)) {
	    if (timeAxis_p < 0) timeAxis_p = i;
	    else {
		errMsg = "multiple time axes";
		return False;
	    }
	} else if (ctype.matches(stokesRegex_p)) {
	    if (stokesAxis_p < 0) stokesAxis_p = i;
	    else {
		errMsg = "multiple stokes axes";
		return False;
	    }
	} else if (ctype.matches(beamRegex_p)) {
	    if (beamAxis_p < 0) beamAxis_p = i;
	    else {
		errMsg = "multiple beam axes";
		return False;
	    }
	} else if (ctype.matches(receiverRegex_p)) {
	    if (receiverAxis_p < 0) receiverAxis_p = i;
	    else {
		errMsg = "multiple receiver axes";
		return False;
	    }
	} else {
	    errMsg = "unrecognized axis type : " + ctype;
	    return False;
	}
    }

    // ensure that required axes are there
    if (freqAxis_p < 0 || longAxis_p < 0 || latAxis_p < 0) {
	errMsg = "one of the required SDFITS axes is missing";
	return False;
    }

    // cunit
    for (uInt i=0;i<naxes;i++) {
	if (cunitIds_p(i) >= 0) {
	    cunit_p(i) = row.asString(cunitIds_p(i));
	} else {
	    cunit_p(i) = "";
	}
    }

    cursorShape_p = IPosition(2, 1);
    IPosition cursorAxes = IPosition(2,0,1);
    // preferred ordering is stokes and then frequency
    cursorShape_p(0) = 1;
    cursorShape_p(1) = dataShape(freqAxis_p);
    if (stokesAxis_p >= 0) {
	if (stokesAxis_p < freqAxis_p) {
	    cursorShape_p(0) = dataShape(stokesAxis_p);
	    cursorShape_p(1) = dataShape(freqAxis_p);
	    needsToBeTransposed_p = False;
	} else {
	    cursorShape_p(0) = dataShape(freqAxis_p);
	    cursorShape_p(1) = dataShape(stokesAxis_p);
	    needsToBeTransposed_p = True;
	}
    } else {
	if (freqAxis_p == 0) {
	    // frequency axis comes first
	    cursorShape_p(0) = dataShape(freqAxis_p);
	    cursorShape_p(1) = 1;
	    needsToBeTransposed_p = True;
	} else {
	    cursorShape_p(0) = 1;
	    cursorShape_p(1) = dataShape(freqAxis_p);
	    needsToBeTransposed_p = False;
	}
    }
    IPosition axisPath(naxes);
    // this should cover everything, step in time last so things
    // remain time-sorted when filled.
    uInt aptr = 0;
    if (stokesAxis_p >= 0) {
	if (stokesAxis_p > freqAxis_p) {
	    axisPath(aptr++) = freqAxis_p;
	    axisPath(aptr++) = stokesAxis_p;
	} else {
	    axisPath(aptr++) = stokesAxis_p;
	    axisPath(aptr++) = freqAxis_p;
	}
    } else {
	axisPath(aptr++) = freqAxis_p;
    }
    if (receiverAxis_p >= 0) axisPath(aptr++) = receiverAxis_p;
    if (beamAxis_p >= 0) axisPath(aptr++) = beamAxis_p;
    axisPath(aptr++) = latAxis_p;
    axisPath(aptr++) = longAxis_p;
    if (timeAxis_p >= 0) axisPath(aptr++) = timeAxis_p;

    LatticeStepper stepper(dataShape, cursorShape_p, cursorAxes, axisPath);
    iter_p = new RO_LatticeIterator<Float>(*arrLattice_p, stepper);
    AlwaysAssert(iter_p, AipsError);

    if (needsToBeTransposed_p) {
	// need to reverse the order
	IPosition dataShape(2);
	dataShape(0) = cursorShape_p(1);
	dataShape(1) = cursorShape_p(0);
	floatData_p.resize(dataShape);
    } else {
	// cursorShape is the data shape
	floatData_p.resize(cursorShape_p);
    }

    String veldef;
    if (veldefField_p.isAttached()) veldef = *veldefField_p;
    // fudge for UniPOPS data - not sure how best to do this, perhaps this
    // should be done in the uni2sdfits utility
    if (row.fieldNumber("RVSYS") >= 0 && restfreqId_p >= 0) {
	Double rvsys = row.asDouble("RVSYS");
	Double factor;
	if (veldef.contains("RADI")) {
	    factor = 1.0/(1.0-rvsys/C::c);
	} else {
	    factor = 1.0 + rvsys/C::c;
	}
	crval_p(freqAxis_p) *= factor;
	cdelt_p(freqAxis_p) *= factor;
    }

    // frequency axis values are set when the row starts out
    // need to worry about frequency axis units
    freqs_p.resize(dataShape(freqAxis_p));
    fdelt_p = cdelt_p(freqAxis_p);
    refFreq_p = crval_p(freqAxis_p);

    indgen(freqs_p);
    freqs_p -= crpix_p(freqAxis_p);
    String ftype = ctype_p(freqAxis_p);

    // velocity convention defaults to RADIO
    dopplerType_p = MDoppler::RADIO;
    if (ftype.matches(freqRegex_p)) {
	// frequency values stay as is, velocity convention might be overruled by veldef value
	freqs_p *= fdelt_p;
	freqs_p += crval_p(freqAxis_p);
	if (veldef != "" && veldef.contains("OPTI")) dopplerType_p = MDoppler::OPTICAL;
    } else {
	// these require a rest frequency
	if (restfreqId_p < 0) {
	    errMsg = "Velocity axis requires a RESTFREQ keyword/column for frequency conversion";
	    return False;
	}
	Double restfreq = row.asDouble(restfreqId_p);
	// unipops gets this wrong, rely on VELDEF
	if (ftype.contains("VELO") || veldef.contains("RADI")) {
	    // true velocity axis, use VELDEF if available to set the doppler type
	    if (veldef != "") {
		if (veldef.contains("OPTI")) dopplerType_p = MDoppler::OPTICAL;
		// ignore RELA for now
	    }
	    freqs_p *= fdelt_p;
	    freqs_p += crval_p(freqAxis_p);
	    // convert these velocities to frequencies
	    freqs_p /= C::c;
	    refFreq_p /= C::c;
	    if (dopplerType_p == MDoppler::RADIO) {
		freqs_p = restfreq * ( 1.0 - freqs_p);
		refFreq_p = restfreq * (1.0 - refFreq_p);
		fdelt_p = - restfreq * ( fdelt_p/C::c);
	    } else {
		// must be optical
		freqs_p = restfreq / (1.0 + freqs_p);
		refFreq_p = restfreq / (1.0 + refFreq_p);
		fdelt_p = -(fdelt_p/C::c) * (refFreq_p/restfreq) * refFreq_p;
	    }
	} else {
	    // must be FELO - implies OPTICAL convention in V, but linear in Freq
	    // convert V and deltV and ref channel to F and deltF at reference channel
	    dopplerType_p = MDoppler::OPTICAL;
	    Double vRval = crval_p(freqAxis_p);
	    Double denom = (1.0+vRval/C::c);
	    fdelt_p = -restfreq * (fdelt_p / C::c) / denom / denom;
	    Double freqRval = restfreq / denom;
	    freqs_p *= fdelt_p;
	    freqs_p += freqRval;
	    Int rpix = Int(crpix_p(freqAxis_p)+0.5);
	    if (rpix < 0) rpix = 0;
	    if (uInt(rpix) >= freqs_p.nelements()) rpix = freqs_p.nelements()-1;
	    refFreq_p = freqs_p(rpix);
	}
    }
    // get the reference frame
    String tag;
    for (uInt i=4;i<ftype.length();i++) {
	if (ftype[i] != '-' && ftype[i] != ' ') {
	    tag += ftype[i];
	}
    }
    if (tag == "" && veldef != "") {
	// if there is a VELDEF, try the tag found there
	for (uInt i=4;i<veldef.length();i++) {
	    if (veldef[i] != '-' && veldef[i] != ' ') {
		tag += veldef[i];
	    }
	}
    }
    // default reference frame
    if (tag == "LSR" || tag == "LSRK") {
	freqRefType_p = MFrequency::LSRK;
    } else if (tag == "HEL") {
	freqRefType_p = MFrequency::BARY;
    } else if (tag == "OBS") {
	freqRefType_p = MFrequency::TOPO;
    } else if (tag == "LSD") {
	freqRefType_p = MFrequency::LSRD;
    } else if (tag == "GEO") {
	freqRefType_p = MFrequency::GEO;
    } else if (tag == "SOU" || tag == "REST") {
	freqRefType_p = MFrequency::REST;
    } else if (tag == "GAL") {
	freqRefType_p = MFrequency::GALACTO;
    } else if (tag == "") {
	// just use the default
	freqRefType_p = MFrequency::TOPO;
    } else {
	errMsg = "Unrecognized frequency reference frame " + tag;
	return False;
    }

    // stokes axis values are set when the row starts out
    if (stokesAxis_p >= 0) {
	stokes_p.resize(dataShape(stokesAxis_p));
	// always a simple linear axis - first, set them based on
	// the FITS values for that axis
	indgen(stokes_p);
	// need to convert axis descriptor to integers, remembering that they might be negative
	Int icrpix, icdelt, icrval;
	if (crpix_p(stokesAxis_p) > 0) icrpix = Int(crpix_p(stokesAxis_p) + 0.5);
	else icrpix = Int(crpix_p(stokesAxis_p) - 0.5);
	if (cdelt_p(stokesAxis_p) > 0) icdelt = Int(cdelt_p(stokesAxis_p) + 0.5);
	else icdelt = Int(cdelt_p(stokesAxis_p) - 0.5);
	if (crval_p(stokesAxis_p) > 0) icrval = Int(crval_p(stokesAxis_p) + 0.5);
	else icrval = Int(crval_p(stokesAxis_p) - 0.5);
	stokes_p -= icrpix;
	stokes_p *= icdelt;
	stokes_p += icrval;
	// now convert these to the AIPS++ values
	for (Int k=0;k<Int(stokes_p.nelements());k++) {
	    switch(stokes_p(k)) {
	    case 1: stokes_p(k) = Stokes::I; break;
	    case 2: stokes_p(k) = Stokes::Q; break;
	    case 3: stokes_p(k) = Stokes::U; break;
	    case 4: stokes_p(k) = Stokes::V; break;
	    case -1: stokes_p(k) = Stokes::RR; break;
	    case -2: stokes_p(k) = Stokes::LL; break;
	    case -3: stokes_p(k) = Stokes::RL; break;
	    case -4: stokes_p(k) = Stokes::LR; break;
	    case -5: stokes_p(k) = Stokes::XX; break;
	    case -6: stokes_p(k) = Stokes::YY; break;
	    case -7: stokes_p(k) = Stokes::XY; break;
	    case -8: stokes_p(k) = Stokes::YX; break;
	    case 0: break;
	    default:
		// everything else is unsupported
		// make it undefined - should probably issue a warning
		stokes_p(k) = Stokes::Undefined; break;
	    }
	}
    } else {
	stokes_p.resize(1);
	stokes_p(0) = 0;
    }

    // set the zero-time values
    String timesys("UTC");
    if (timesysField_p.isAttached()) timesys = *timesysField_p;
    if (!FITSDateUtil::fromFITS(dateObs_p, timesys_p, *dateObsField_p, timesys)) {
	Bool badDate = True;
	String dateObs = *dateObsField_p;
	if (dateObs.matches(Regex("^..../../..$"))) {
	    // special case, seen in early samples from Parkes YYYY/MM/DD
	    // reformat this to the new FITS style : YYYY-MM-DD
	    String dateObs = *dateObsField_p;
	    dateObs.gsub("/","-");
	    // and try again
	    badDate = !FITSDateUtil::fromFITS(dateObs_p, timesys_p, dateObs, timesys);
	}
	if (badDate) {
	    errMsg = "Problem converting DATE-OBS string : " + *dateObsField_p;
	    return False;
	}
    }
    if (timeId_p >= 0) {
	timeFieldVal_p = Quantity(row.asDouble(timeId_p),"s");
    }

    // exposure
    if (exposureId_p >= 0) {
	// assumes seconds, could probably check units though
	exposure_p = Quantity(row.asDouble(exposureId_p),"s");
    } else {
	// set it to zero, strictly speaking exposure is required, but don't be 
	// unnecessarily pedantic here
	exposure_p = Quantity(0.0,"s");
    }

    // default beam and receiver values
    beamId_p = 1;
    receiverId_p = 1;

    // direction coordinate
    Matrix<Double> xform(2,2);
    xform = 0.0; xform.diagonal() = 1.0;
    // handle crota here, should perhaps check that only one crota value is non-zero
    if (!near(crota_p(latAxis_p), 0.0)) {
	// rotation about second axis
	xform.diagonal() = cos(crota_p(latAxis_p));
	xform(0,1) = -sin(crota_p(latAxis_p));
	xform(1,0) = sin(crota_p(latAxis_p));
    } else if (!near(crota_p(longAxis_p),0.0)) {
	// rotation about first axis
	xform.diagonal() = cos(crota_p(longAxis_p));
	xform(1,0) = -sin(crota_p(longAxis_p));
	xform(0,1) = sin(crota_p(longAxis_p));
    }
    // are these galactic coordinates
    MDirection::Types radecsys;
    if (isGalactic) {
	radecsys = MDirection::GALACTIC;
    } else {
	// need to get the Epoch/Equinox correctly, default to J2000
	radecsys = MDirection::J2000;
	// give preference to EQUINOX
	if (equinoxId_p >= 0) {
	    // need to do something for non B1950, J2000 equinoxes
	    switch (equinoxType_p) {
	    case TpDouble:
		{
		    Double equinox = row.asDouble(equinoxId_p);
		    if (near(equinox, 1950.0)) {
			radecsys = MDirection::B1950;
		    } else if (near(equinox, 2000.0)) {
			radecsys = MDirection::J2000;
		    }
		}
		break;
	    case TpFloat:
		{
		    Float equinox = row.asFloat(equinoxId_p);
		    if (near(equinox, Float(1950.0))) {
			radecsys = MDirection::B1950;
		    } else if (near(equinox, Float(2000.0))) {
			radecsys = MDirection::J2000;
		    }
		}
		break;
	    case TpInt:
	    case TpShort:
		{
		    Int equinox = row.asInt(equinoxId_p);
		    if (equinox == 1950) {
			radecsys = MDirection::B1950;
		    } else if (equinox == 2000) {
			radecsys = MDirection::J2000;
		    }
		}
		break;
	    default:
		errMsg = "The EQUINOX or EPOCH keyword/column is an impossible type.  This should never happen here.";
		return False;
	    }
	} 
    }
    // figure the projection using the long axis (assume same as lat axis)
    String proj = ctype_p(longAxis_p);
    if (proj.length() > 4) {
	proj.gsub(Regex("^...."),"");
	proj.gsub(Regex("^-*"),"");
	proj.gsub(Regex(" *"),"");
    } else {
	proj = "";
    }
    if (proj == "") {
	// assume CAR projectin
	proj = "CAR";
    } 
    Projection::Type ptype;
    // worry about "PROJP" parameters eventually, just punt for now
    Vector<Double> projp;
    ptype = Projection::type(proj);
    if (ptype == Projection::N_PROJ) {
	errMsg = "Unknown projection : " + proj;
	return False;
    }
    Projection projn;
    try {
	projn = Projection(ptype, projp);
    } catch (AipsError x) {
	errMsg = "Error forming projection : " + x.getMesg();;
	return False;
    } 

    // convert from units of axis to radians
    if (cunit_p(longAxis_p) == "") cunit_p(longAxis_p) = "deg";
    if (cunit_p(latAxis_p) == "") cunit_p(latAxis_p) = "deg";
    Unit longu = cunit_p(longAxis_p);
    Unit latu = cunit_p(latAxis_p);
    Unit rad = "rad";
    if (longu.getValue() != rad.getValue() ||
	latu.getValue() != rad.getValue()) {
	errMsg = "Longitude or latitude axis units are incompatible with angle";
	return False;
    }
    Double toRadX = longu.getValue().getFac()/rad.getValue().getFac();
    Double toRadY = latu.getValue().getFac()/rad.getValue().getFac();
    dirCoord_p = DirectionCoordinate(radecsys, 
				     projn,
				     crval_p(longAxis_p)*toRadX, crval_p(latAxis_p)*toRadY,
				     cdelt_p(longAxis_p)*toRadX, cdelt_p(latAxis_p)*toRadY,
				     xform,
				     crpix_p(longAxis_p), crpix_p(latAxis_p));					 

    setValues();
    
    return result;
}

void SDFITSDataIterator::next()
{
    // move the cursor and set the values
    if (!iter_p->atEnd()) {
	(*iter_p)++;
	setValues();
    }
}

Bool SDFITSDataIterator::init(const Record &row, String &errMsg)
{
    errMsg = "";

    // reset the handled columns
    handledCols_p.resize(row.description().nfields());
    handledCols_p = False;

    // DATA must be present
    Int dataField = row.fieldNumber("DATA");
    if (dataField < 0) {
	errMsg = "Required DATA keyword/column not found";
	return False;
    }
    // what type is that field
    switch (row.type(dataField)) {
    case TpArrayFloat:
	fdataPtr_p.attachToRecord(row, dataField);
	break;
    case TpArrayInt:
	idataPtr_p.attachToRecord(row, dataField);
	break;
    case TpArrayShort:
	sdataPtr_p.attachToRecord(row, dataField);
	break;
    case TpArrayDouble:
	ddataPtr_p.attachToRecord(row, dataField);
	// issue warning on loss of precision
	{
	    LogIO os;
	    os << LogOrigin("SDFITSDataIterator","init");
	    os << WHERE
	       << LogIO::WARN
	       << "DATA column will be converted from double to float"
	       << " with resulting loss of information."
	       << LogIO::POST;
	}
	break;
    default:
	errMsg = "Unrecognized type for DATA column";
	break;
    }
    handledCols_p(dataField) = True;

    // axis pointers
    // count up how many axes there can be, at most, by counting up
    // all of the CTYPEnnn fields
    Int naxes = -1;
    Bool more = True;
    while (more) {
	naxes++;
	more = indexedFieldNumber(row, "CTYPE", naxes+1) >= 0;
    }
    // FITS is 1-rel
    if (naxes == 0) {
	errMsg = "There are no CTYPEnnn keywords/columns - can not construct coordinate information";
	return False;
    }

    // and now set the appropriate field numbers
    ctypeIds_p.resize(naxes);
    crvalIds_p.resize(naxes);
    crpixIds_p.resize(naxes);
    cdeltIds_p.resize(naxes);
    crotaIds_p.resize(naxes);
    cunitIds_p.resize(naxes);
    // default to -1
    ctypeIds_p = -1;
    crvalIds_p = -1;
    crpixIds_p = -1;
    cdeltIds_p = -1;
    crotaIds_p = -1;
    cunitIds_p = -1;
    // and actually set the field numbers
    for (Int i=0;i<naxes;i++) {
	ctypeIds_p(i) = indexedFieldNumber(row, "CTYPE", i+1);
	// that must exist
	handledCols_p(ctypeIds_p(i)) = True;
	// the rest might not exist
	crvalIds_p(i) = indexedFieldNumber(row, "CRVAL", i+1);
	if (crvalIds_p(i) >= 0) handledCols_p(crvalIds_p(i)) = True;
	crpixIds_p(i) = indexedFieldNumber(row, "CRPIX", i+1);
	if (crpixIds_p(i) >= 0) handledCols_p(crpixIds_p(i)) = True;
	cdeltIds_p(i) = indexedFieldNumber(row, "CDELT", i+1);
	if (cdeltIds_p(i) >= 0) handledCols_p(cdeltIds_p(i)) = True;
	crotaIds_p(i) = indexedFieldNumber(row, "CROTA", i+1);
	if (crotaIds_p(i) >= 0) handledCols_p(crotaIds_p(i)) = True;
	cunitIds_p(i) = indexedFieldNumber(row, "CUNIT", i+1);
	if (cunitIds_p(i) >= 0) handledCols_p(cunitIds_p(i)) = True;
    }

    // and other fields - these are used here but not handled here
    obsfreqId_p = row.fieldNumber("OBSFREQ");
    restfreqId_p = row.fieldNumber("RESTFREQ");

    // these are used here and handled here
    lstId_p = row.fieldNumber("LST");
    if (lstId_p >= 0) handledCols_p(lstId_p) = True;
    equinoxId_p = row.fieldNumber("EQUINOX");
    if (equinoxId_p >= 0) {
	handledCols_p(equinoxId_p) = True;
	equinoxType_p = row.dataType(equinoxId_p);
	// if there is an EPOCH, ignore it
	if (row.fieldNumber("EPOCH") >= 0) handledCols_p(row.fieldNumber("EPOCH"));
    } else {
	// use EPOCH instead, if available
	equinoxId_p = row.fieldNumber("EPOCH");
	if (equinoxId_p >= 0) {
	    handledCols_p(equinoxId_p) = True;
	    equinoxType_p = row.dataType(equinoxId_p);
	}
    }
    if (row.fieldNumber("DATE-OBS") >= 0) {
	dateObsField_p.attachToRecord(row, "DATE-OBS");
	handledCols_p(row.fieldNumber("DATE-OBS")) = True;
    } else {
	errMsg = "Required DATE-OBS keyword/column is missing.";
	return False;
    }
    if (row.fieldNumber("TIMESYS") >= 0) {
	timesysField_p.attachToRecord(row, "TIMESYS");
	handledCols_p(row.fieldNumber("TIMESYS")) = True;
    }
    exposureId_p = row.fieldNumber("EXPOSURE");
    if (exposureId_p >= 0) {
	handledCols_p(exposureId_p) = True;
    }
    // a warning message should be issued here if EXPOSURE isn't found
    if (row.fieldNumber("VELDEF") >= 0) {
	veldefField_p.attachToRecord(row, "VELDEF");
	handledCols_p(row.fieldNumber("VELDEF")) = True;
    }
    // Some SDFITS have a TIME field instead of a TIME axis
    timeId_p = row.fieldNumber("TIME");
    if (timeId_p >= 0) handledCols_p(row.fieldNumber("TIME")) = True;

    return True;
}

Int SDFITSDataIterator::indexedFieldNumber(const Record &row,
					   const String &field, Int index)
{
    ostringstream ostr;
    ostr << index;
    String name(field);
    name += String(ostr);
    return row.fieldNumber(name);
}

void SDFITSDataIterator::setValues()
{
    // to swap axes or not
    if (needsToBeTransposed_p) {
	floatData_p = transpose(iter_p->matrixCursor());
    } else {
	floatData_p = iter_p->matrixCursor();
    }

    IPosition position(iter_p->position());

    // TIME axis takes precedence over any TIME column
    MVTime toff(Quantity(0.0,"s"));
    if (timeAxis_p >= 0) {
	Double pix = Double(position(timeAxis_p));
	Double offset = (pix - crpix_p(timeAxis_p)) * cdelt_p(timeAxis_p) + crval_p(timeAxis_p);
	toff = Quantity(offset,"s");
    } else if (timeId_p >= 0) {
	toff = timeFieldVal_p;;
    }
    MVTime thisTime = dateObs_p + toff;
    time_p = MEpoch(thisTime.get(), timesys_p);
    
    // set the time range using exposure
    Double dTime = time_p.get(Unit("s")).getValue();
    Double halfInterval = exposure_p.get("s").getValue()/2.0;
    timeRange_p(0) = dTime-halfInterval;
    timeRange_p(1) = dTime+halfInterval;

    // beam ID and receiver ID, if present
    beamId_p = 1;
    if (beamAxis_p >= 0) {
	Int pix = position(beamAxis_p);
	beamId_p = Int((pix - crval_p(beamAxis_p))*cdelt_p(beamAxis_p) + crval_p(beamAxis_p) + 0.5);
    }
    receiverId_p = 1;
    if (receiverAxis_p >= 0) {
	Int pix = position(receiverAxis_p);
	receiverId_p = Int((pix - crval_p(receiverAxis_p))*cdelt_p(receiverAxis_p) + 
			   crval_p(receiverAxis_p) + 0.5);
    }

    // the position axes
    Vector<Double> pixels(2);
    pixels(0) = position(longAxis_p);
    pixels(1) = position(latAxis_p);
    dirCoord_p.toWorld(direction_p, pixels);
}

} //# NAMESPACE CASA - END

