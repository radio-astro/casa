//# SDTableIterator.cc:  this defines SDTableIterator
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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

#include <dish/SDIterators/SDTableIterator.h>
#include <dish/SDIterators/SDIterAxes.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <fits/FITS/CopyRecord.h>
#include <fits/FITS/FITSDateUtil.h>
#include <fits/FITS/FITSSpectralUtil.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicMath/Math.h>
// MeasConvert.h must be included first
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MeasureHolder.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/Stokes.h>
#include <tables/Tables.h>
#include <tables/Tables/RefRows.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableParse.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Utilities/Assert.h>

namespace casa { //# NAMESPACE CASA - BEGIN

Bool SDTableIterator::isSDFITSTable(const Table& tab) {
    // check for required columns
    // DATA column
    Bool result = tab.tableDesc().isColumn("DATA") &&
	tab.tableDesc()["DATA"].trueDataType() == TpArrayFloat;
 
    // scalar columns,make sure they exist and are Scalar
    if (result) {
	Vector<String> scCols(6);
	scCols(0) = "OBJECT";
	scCols(1) = "TELESCOP";
	scCols(2) = "BANDWID";
	scCols(3) = "DATE-OBS";
	scCols(4) = "EXPOSURE";
	scCols(5) = "TSYS"; 

	for (uInt i=0;i<scCols.nelements();i++) {
	    result = result && tab.tableDesc().isColumn(scCols(i)) &&
		tab.tableDesc()[scCols(i)].isScalar();
	}
    }

    if (result) {
	// verify that there are no subtables
	for (uInt i=0;i<tab.tableDesc().ncolumn();i++) {
	    result = (result && 
			    !(tab.tableDesc().columnDesc(i).isTable()));
	}
    }
    return result;
}

// statics
Bool SDTableIterator::staticsOK_(False);
SimpleOrderedMap<String, String> SDTableIterator::fitsHeaderMap_("");
SimpleOrderedMap<String, String> SDTableIterator::itHeaderMap_("");
SimpleOrderedMap<String, String> SDTableIterator::emptyMap_("");
SimpleOrderedMap<String, Bool> SDTableIterator::fitsSpecialMap_(True);

SDTableIterator::SDTableIterator(const String &tableName, const Record &selection,
				 Table::TableOption opt,
				 TableLock::LockOption lockoption)
    : hasOutputCols_(False), tab_(0), thisRow_(0), axes_(0), headerCopier_(0),
      rwHeaderCopier_(0), otherCopiers_(0), rwOtherCopiers_(0),
      otherCopiersMap_(-1), rwTableRow_(0), oldTableRow_(0), tableRow_(0)
{
    initStatics();
    if (opt == Table::New || 
	opt == Table::NewNoReplace || 
	opt == Table::Scratch) {
	createTable(tableName, lockoption, opt);
    } else {
	tab_ = new Table(tableName, TableLock(lockoption), opt);
    }
    AlwaysAssert(tab_, AipsError);

    cleanupMost();

    if (ensureSDFITSTable()) {
	applySelection(selection);
	if (ensureSDFITSTable()) {
	    init();
	}
    }
}

SDTableIterator::SDTableIterator(const SDTableIterator& other)
    : hasOutputCols_(False), tab_(0), thisRow_(0), axes_(0), headerCopier_(0),
      rwHeaderCopier_(0), otherCopiers_(0), rwOtherCopiers_(0),
      otherCopiersMap_(-1), rwTableRow_(0), oldTableRow_(0), tableRow_(0)
{
    initStatics();
    tab_ = new Table(*other.tab_);
    AlwaysAssert(tab_, AipsError);

    // make sure this is all cleaned up
    cleanupMost();
    if (ensureSDFITSTable()) {
	init();
    }
}

SDTableIterator::SDTableIterator(const SDTableIterator& other, const Record& selection)
    : hasOutputCols_(False), tab_(0), thisRow_(0), axes_(0), headerCopier_(0),
      rwHeaderCopier_(0), otherCopiers_(0), rwOtherCopiers_(0),
      otherCopiersMap_(-1), rwTableRow_(0), oldTableRow_(0), tableRow_(0)
{
    initStatics();
    tab_ = new Table(*(other.tab_));
    AlwaysAssert(tab_, AipsError);

    // make sure this is all cleaned up
    cleanupMost();
    // axes are needed here
    
    if (ensureSDFITSTable()) {
	applySelection(selection);
	if (ensureSDFITSTable()) {
	    init();
	}
    }
}

SDTableIterator &SDTableIterator::operator=(const SDTableIterator& other)
{
    if (this == &other) return *this;

    cleanup();

    tab_ = new Table(*(other.tab_));
    AlwaysAssert(tab_, AipsError);

    // make sure this is all cleaned up
    cleanupMost();
    if (ensureSDFITSTable()) init();

    return *this;
}

Bool SDTableIterator::unlock() {
    if (tab_) tab_->unlock();
    return True;
}

Bool SDTableIterator::lock(uInt nattempts) {
    Bool result = True;
    if (tab_) {
	// hasOutputCols_ is equivalent to the table needing a write lock
	result = tab_->lock(hasOutputCols_, nattempts);
    }
    return result;
}

Bool SDTableIterator::more() {
    Bool result;
    // there must be some rows in tab and thisRow_ must be < nrows
    result = (thisRow_ < (tab_->nrow()-1));
    if (!result) {
	// it can only get here if thisRow_ == (tab_->nrows() - 1)
	DebugAssert(thisRow_ == (tab_->nrow() - 1), AipsError);
	// when we are iterating over sub rows, do something clever here
    }
    return result;
}

uInt SDTableIterator::nrecords() {
    return axes_->nsdrecords();
}

void SDTableIterator::copyAll()
{
    copyData();
    copyHeader();
    copyHist();
    copyOther();
}

void SDTableIterator::copyData()
{
    // do nothing if there is nothing in tab_
    if (tab_->nrow() == 0) return;
    
    // need to ensure the correct shape - harmless if the shape is already set
    setRowShape();
    
    // determine the shape of the data in this row
    IPosition dataShape(axes_->shape(thisRow_));
    // and the row shape we just set
    IPosition rowShape(rec_.shape());
    
    // remember what the axis descriptors are, hold these internal to this class
    if (ctypes_.nelements() != dataShape.nelements()) {
	Int newsize = dataShape.nelements();
	ctypes_.resize(newsize);
	cunits_.resize(newsize);
	crvals_.resize(newsize);
	crpixs_.resize(newsize);
	cdelts_.resize(newsize);
	crotas_.resize(newsize);
    }
    ctypes_ = axes_->ctype(thisRow_);
    cunits_ = axes_->cunit(thisRow_);
    crvals_ = axes_->crval(thisRow_);
    crpixs_ = axes_->crpix(thisRow_);
    cdelts_ = axes_->cdelt(thisRow_);
    crotas_ = axes_->crota(thisRow_);
    
    // the frequency axis is required
    Int freqAxis = axes_->freqAxis(thisRow_);
    
    // refframe
    // reference frame is in last 4 characters of CTYPE for freq axis
    String typeframe(ctypes_(freqAxis));
    MFrequency::Types reftype;
    if (FITSSpectralUtil::frameFromTag(reftype, typeframe.from(4))) {
	*refframe_ = MFrequency::showType(reftype);
    } else {
	// see if VELDEF column (velinfo_) is available
	if (!velinfo_.isNull() && velinfo_.asString(thisRow_).length() == 8) {
	    String velstuff(velinfo_.asString(thisRow_));
	    // try and get it from the tag in the last 4 characters
	    MFrequency::Types reftype;
	    if (FITSSpectralUtil::frameFromTag(reftype, velstuff(4,4))) {
		*refframe_ = MFrequency::showType(reftype);
	    } else {	
		*refframe_ = "";
	    }
	} else {
	    *refframe_ = "";
	}
    }

    // chan_freq
    Vector<Double> freqs(dataShape(freqAxis));
    // FITS starts with pixel 1 as the first pixel
    String units="pixel";
    indgen(freqs);
    freqs += 1.0;
    // unit conversion should be happening here
    freqs -= crpixs_(freqAxis);
    freqs *= cdelts_(freqAxis);
    freqs += crvals_(freqAxis);
    // the units
    units = cunits_(freqAxis);
    // if the units are empty, default as appropriate
    if (units.empty()) {
	// axis type is in the first 4 characters of CTYPE
	String freqAxisType;
	freqAxisType = typeframe.through(3);
	freqAxisType.downcase();
	if (freqAxisType == "freq") { 
	    units = "Hz";
	} else {
	    units = "m/s";
	}
    }
    // verify that the units are recognized
    UnitName name;
    if (!UnitMap::getUnit(units, name)) {
	// units are unknown - add them in to the user definition as an unknown
	// pixel-like unit
	UnitMap::putUser(units, UnitVal(), "Undefined units as found in SDFITS table"); 
    }

    // okay, we can now make chan_freq
    Quantum<Vector<Double> > chanFreq(freqs, units);
    // convert to canonical units
    chanFreq.convert();
    String holderErr;
    if (!QuantumHolder(chanFreq).toRecord(holderErr, *chanFreqRec_)) {
	String errMsg = 
	    "SDTableIterator::copyAll() unexpected problem saving chan_freq Quanta to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // reffrequency
    Quantity refFreq(crvals_(freqAxis), units);
    // convert to canonical units
    refFreq.convert();
    *refFreq_ = refFreq.getValue();

    // chan_width
    Quantity chanWidth(abs(cdelts_(freqAxis)), units);
    // convert to canonical units
    chanWidth.convert();
    *chanWidth_ = chanWidth.getValue();

    // restfrequency
    if (restFreq_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	restFreq_p.copyToField(thisRow_);
    }

    // corr_type
    // valid Stokes axis?
    Int stokesAxis = axes_->stokesAxis(thisRow_);
    if (stokesAxis >= 0) {
	// should verify that the cdelt is an integer and cunit is
	// whatever is appropriate and crota is 0
	Vector<String> stokes(dataShape(stokesAxis));
	for (uInt i=0;i<stokes.nelements();i++) {
	    // pixels are 1-relative
	    Int stokesval = Int(crvals_(stokesAxis) + 
				cdelts_(stokesAxis)*((i+1)-crpixs_(stokesAxis)));
	    stokes(i) = Stokes::name(Stokes::fromFITSValue(stokesval));
	}
	(*corrType_).resize(IPosition(1,stokes.nelements()));
	*corrType_ = stokes;
	
    } else {
	Vector<String> stokes(1);
	stokes(0) = "";
	(*corrType_).resize(IPosition(1,1));
	*corrType_ = stokes;
    }

    // arr, weights, flag, sigma
    Matrix<Float> out(rowShape);
    IPosition start(dataShape.nelements());
    start = 0;
    IPosition end(start);
    end(freqAxis) = dataShape(freqAxis) - 1;
    IPosition specShape(1,out.ncolumn());
    // behavior depends on existance of stokes axis
    if (stokesAxis >= 0) {
	// arr
	for (Int i=0;i<dataShape(stokesAxis);i++) {
	    end(stokesAxis)=i;
	    start(stokesAxis)=i;
	    out.row(i) = data_in_(thisRow_)(start,end).reform(specShape);
	}
	*arr_ = out;
	
	// weight
	if (!weight_in_.isNull()) {
	    for (Int i=0;i<dataShape(stokesAxis);i++) {
		end(stokesAxis)=i;
		start(stokesAxis)=i;
		out.row(i) = weight_in_(thisRow_)(start,end).reform(specShape);
	    }
	    *weight_ = out;
	} else {
	    // default to value of 1.0 for all weights
	    *weight_ = 1.0;
	}
	// sigma
	if (!sigma_in_.isNull()) {
	    for (Int i=0;i<dataShape(stokesAxis);i++) {
		end(stokesAxis)=i;
		start(stokesAxis)=i;
		out.row(i) = sigma_in_(thisRow_)(start,end).reform(specShape);
	    }
	    *sigma_ = out;
	} else {
	    // default to a value of 1.0 for all sigma
	    *sigma_ = 1.0;
	}
	// flag
	if (!flag_in_.isNull()) {
	    Matrix<Bool> flagOut(rowShape);
	    for (Int i=0;i<dataShape(stokesAxis);i++) {
		end(stokesAxis)=i;
		start(stokesAxis)=i;
		flagOut.row(i) = flag_in_(thisRow_)(start,end).reform(specShape);
	    }
	    *flag_ = flagOut;
	} else {
	    // default to value of F (unflagged) for all flags
	    *flag_ = False;
	}
    } else {
	// no stokes axis in initial data
	// arr
	out.row(0) = data_in_(thisRow_)(start,end).reform(specShape);
	*arr_ = out;
	
	// weight
	if (!weight_in_.isNull()) {
	    out.row(0) = weight_in_(thisRow_)(start,end).reform(specShape);
	    *weight_ = out;
	} else {
	    // default to a weight of 1 when not supplied
	    *weight_ = 1.0;
	}
	
	// sigma
	if (!sigma_in_.isNull()) {
	    out.row(0) = sigma_in_(thisRow_)(start,end).reform(specShape);
	    *sigma_ = out;
	} else {
	    // default to a sigma of 1 when not supplied
	    *sigma_ = 1.0;
	}
	
	// flag
	if (!flag_in_.isNull()) {
	    Matrix<Bool> flagOut(rowShape);
	    flagOut.row(0) = flag_in_(thisRow_)(start,end).reform(specShape);
	    *flag_ = flagOut;
	} else {
	    // default to a flag of F when not supplied
	    *flag_ = False;
	}
    }   
}
    
void SDTableIterator::copyHeader()
{
    // do nothing if there is nothing in tab_
    if (tab_->nrow() == 0) return;

    // copy this row into tableRow - get will not read if this row
    // has already been read
    tableRow_->get(thisRow_);

    // need to ensure the correct shape - harmless if the shape is already set
    setRowShape();

    // the scalar copiers (observer, project, source_name)
    headerCopier_->copy();

    // everything else is done by hand
    // this is used in several places during conversion of Quantum 
    // and Measure Holders to Records
    String holderErr;

    // time
    // axis descriptor holders are resized as one - this should be done
    // in a function instead of duplicated here and in getData
    // determine the shape of the data in this row
    IPosition dataShape(axes_->shape(thisRow_));
    if (ctypes_.nelements() != dataShape.nelements()) {
	Int newsize = dataShape.nelements();
	ctypes_.resize(newsize);
	cunits_.resize(newsize);
	crvals_.resize(newsize);
	crpixs_.resize(newsize);
	cdelts_.resize(newsize);
	crotas_.resize(newsize);
    }
    // most of these are used here, just get them all
    ctypes_ = axes_->ctype(thisRow_);
    cunits_ = axes_->cunit(thisRow_);
    crvals_ = axes_->crval(thisRow_);
    crpixs_ = axes_->crpix(thisRow_);
    cdelts_ = axes_->cdelt(thisRow_);
    crotas_ = axes_->crota(thisRow_);

    // get any time offset, first look for a TIME axis
    Int timeAxis = axes_->timeAxis(thisRow_);
    Double offset = 0.0;
    String timeUnits = "s";
    if (timeAxis >= 0) {
	// this should iterate over any non-degenerate time axes
	// for now, assume we want the value at pixel 1
	// this should also check that units are s and convert as necessary
	offset = crvals_(timeAxis) + cdelts_(timeAxis)*(crpixs_(timeAxis)-1);
	timeUnits = cunits_(timeAxis);
    } else if (!timeCol_.isNull()) {
	// otherwise use a TIME column if present
	offset = timeCol_.asdouble(thisRow_);
	if (timeCol_.keywordSet().fieldNumber("UNIT")) {
	    timeUnits = timeCol_.keywordSet().asString("UNIT"); 
	}
    }
    if (timeUnits == "") timeUnits = "s";
    Quantity qtime(offset,timeUnits);
    // convert the DATE-OBS value to a TIME and add it in, if available
    if (!dateObsCol_.isNull()) {
	MVTime mvdateObs;
	MEpoch::Types system;
	// does not check for TIMESYS keyword/column, assumes UTC
	FITSDateUtil::fromFITS(mvdateObs, system, dateObsCol_.asString(thisRow_),"");
	qtime += mvdateObs.get();
    }
    MEpoch etime(qtime, MEpoch::UTC);
    if (!MeasureHolder(etime).toRecord(holderErr, *time_)) {
	String errMsg = 
	    "SDTableIterator::copyAll() unexpected problem saving time measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // scan_number
    // subscan goes into other/sdfits record - handled in copyOther
    if (!scan_.isNull()) {
	Double scannr = scan_.asdouble(thisRow_);
	*scan_number_ = Int(scannr+0.005);
    } else {
	*scan_number_ = -1;
    }

    // direction
    // do something with crotas in this next section
    // will need to keep track of possible non-degenerate position axes
    // also do position unit conversion
    // first, get the values
    Int longAxis = axes_->longAxis(thisRow_);
    Int latAxis = axes_->latAxis(thisRow_);
    // no rotation away from the reference pixel, I think
    // if we handled that, it would be done here
    // is the longitude axis galactic 
    // a sanity check that both axes are the same type might be prudent
    Bool isGalactic = False;
    if (longAxis >= 0) {
	isGalactic = ctypes_(longAxis).matches("^G.*");
    } else if (latAxis >= 0) {
	isGalactic = ctypes_(latAxis).matches("^G.*");
    }
    MDirection::Types radecsys;
    if (isGalactic) {
	radecsys = MDirection::GALACTIC;
    } else {
	// need to get the Equinox correct, default to J2000
	radecsys = MDirection::J2000;
	// look for any EQUINOX value
	if (!equinoxCol_.isNull()) {
	    switch(equinoxCol_.columnDesc().dataType()) {
	    case TpDouble:
		{
		    Double equinox = equinoxCol_.asdouble(thisRow_);
		    // need to do something for non B1950, J2000 equinoxes/coordinates
		    if (near(equinox, 1950.0)) {
			radecsys = MDirection::B1950;
		    } else if (near(equinox, 2000.0)) {
			radecsys = MDirection::J2000;
		    }
		}
		break;
	    case TpFloat:
		{
		    Float equinox = equinoxCol_.asfloat(thisRow_);
		    // need to do something for non B1950, J2000 equinoxes/coordinates
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
		    Int equinox = equinoxCol_.asInt(thisRow_);
		    // need to do something for non B1950, J2000 equinoxes/coordinates
		    if (equinox == 1950) {
			radecsys = MDirection::B1950;
		    } else if (equinox == 2000.0) {
			radecsys = MDirection::J2000;
		    }
		}
		break;
	    default:
		radecsys = MDirection::J2000;
	    }
	}
    }
    // don't worry about any project here, we're just getting the reference pixel value
    // in the future, it should worry
    // convert from units of axis to radians
    Unit longu = "deg";
    Unit latu = "deg";
    if (longAxis >= 0 && cunits_(longAxis) != "") longu = cunits_(longAxis);
    if (latAxis >= 0 && cunits_(latAxis) != "") latu = cunits_(latAxis);
    Unit rad = "rad";
    // could do a sanity check on the axis units -> compatible with angle
    Double toRadX = longu.getValue().getFac()/rad.getValue().getFac();
    Double toRadY = latu.getValue().getFac()/rad.getValue().getFac();
    Vector<Double> pos(2);
    pos = 0.0;
    if (longAxis >= 0) {
	// this assumes we want what is at pixel 1 and crotas == 0
	pos(0) = (crvals_(longAxis) + cdelts_(longAxis)*(crpixs_(longAxis)-1))*toRadX;
    }
    if (latAxis >= 0) {
	pos(1) = (crvals_(latAxis) + cdelts_(latAxis)*(crpixs_(latAxis)-1))*toRadY;
    }
   
    MDirection dir(Quantum<Vector<Double> >(pos, "rad"), radecsys);
    if (!MeasureHolder(dir).toRecord(holderErr, *direction_)) {
	String errMsg = 
	    "SDTableIterator::copyAll() unexpected problem saving direction measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // refdirection
    MDirection refDirMeas;
    if (!refDirCol_.isNull()) {
	Vector<Double> refDir;
	refDirCol_.get(thisRow_,refDir, True);
	// use same reference code as used for the direction field
	// default to "deg" for units unless one given
	String unit("deg");
	if (refDirCol_.keywordSet().fieldNumber("UNIT") >=0 ) {
	    unit = refDirCol_.keywordSet().asString("UNIT");
	}
	refDirMeas = MDirection(Quantum<Vector<Double> >(refDir, unit), radecsys);
   } else {
	// assure that refdirection has the same reference frame as direction, with
	// default value set
       refDirMeas.setRefString(MDirection::showType(radecsys));
    }
    if (!MeasureHolder(refDirMeas).toRecord(holderErr, *refDirection_)) {
	String errMsg = 
	    "SDTableIterator::copyAll() unexpected problem saving refdirection measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // veldef
    // veldef_ is the field in the SDRecord, velinfo_ is the VELDEF column in the table
    String freqAxisType = "";
    if (!velinfo_.isNull() && velinfo_.asString(thisRow_).length() == 8) {
	String velstuff(velinfo_.asString(thisRow_));
	*veldef_ = velstuff(0,4);
	if (*veldef_ == "RADI") *veldef_ = MDoppler::showType(MDoppler::RADIO);
	else if (*veldef_ == "OPTI") *veldef_ = MDoppler::showType(MDoppler::OPTICAL);
    } else { 
	// try and determine the velocity definition from the freq axis CTYPE
	Int freqAxis = axes_->freqAxis(thisRow_);
	if (freqAxis >= 0) {
	    Int freqAxis = axes_->freqAxis(thisRow_);
	    String typeframe(ctypes_(freqAxis));
	    // the type is in the first 4 characters
	    freqAxisType = typeframe.through(3);
	    freqAxisType.downcase();
	}
	// only FELO is OPTICAL, everything else is assumed to be RADIO
	if (freqAxisType == "felo") {
	    *veldef_ = MDoppler::showType(MDoppler::OPTICAL);
	} else {
	    *veldef_ = MDoppler::showType(MDoppler::RADIO);
	}
    }

    // transition : blend of MEASURE and TRANSITI keywords
    String trans = "";
    if (!transitionCol_.isNull()) trans = transitionCol_.asString(thisRow_);
    if (!moleculeCol_.isNull() && moleculeCol_.asString(thisRow_) != "") {
	trans += ',';
	trans += moleculeCol_.asString(thisRow_);
    }
    *transition_ = trans;

    // exposure
    if (texp_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	texp_p.copyToField(thisRow_);
    }
    // duration
    if (tdur_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	tdur_p.copyToField(thisRow_);
    }

    // if there is no DURATION column, set the duration field == exposure
    if (!tdur_p.isAttached() && texp_p.isAttached()) {
	*tdurField_p = texp_p.getRecord();
    }

    // if there is no EXPOSURE column, set the exposure field == duration
    if (!texp_p.isAttached() && tdur_p.isAttached()) {
	*texpField_p = tdur_p.getRecord();
    }

    // resolution
    if (resolution_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	resolution_p.copyToField(thisRow_);
    }

    // bandwidth
    if (bw_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	bw_p.copyToField(thisRow_);
    }

    // for these vector header words, assume just one value for each
    // dummy t* values, just use zeros with appropriate dimensions - nstokes
    Vector<Float> tdummy((rec_.shape())(0), 0.0);

    // tcal - prefer the array column version then, the scalar
    // then the default value - tcal has the correct shape
    // here - only use the array tcal column if it, too, has the
    // correct shape.
    if (!arrTcalCol_.isNull() && 
	arrTcalCol_.shape(thisRow_) == (*tcal_).shape()) {
	*tcal_ = arrTcalCol_(thisRow_);
    } else if (!tcalCol_.isNull()) {
	*tcal_ = tcalCol_.asfloat(thisRow_);;
    } else {
	*tcal_ = tdummy;
    }

    // trx - prefer the array column version then, the scalar
    // then the default value - trx has the correct shape
    // here - only use the array trx column if it, too, has the
    // correct shape.
    if (!arrTrxCol_.isNull() && 
	arrTrxCol_.shape(thisRow_) == (*trx_).shape()) {
	*trx_ = arrTrxCol_(thisRow_);
    } else if (!trxCol_.isNull()) {
	*trx_ = trxCol_.asfloat(thisRow_);
    } else {
	*trx_ = tdummy;
    }

    // tsys - prefer the array column version then, the scalar
    // then the default value - tsys has the correct shape
    // here - only use the array tsys column if it, too, has the
    // correct shape.
    if (!arrTsysCol_.isNull() && 
	arrTsysCol_.shape(thisRow_) == (*tsys_).shape()) {
	*tsys_ = arrTsysCol_(thisRow_);
    } else if (!tsysCol_.isNull()) {
	*tsys_ = tsysCol_.asfloat(thisRow_);
    } else {
	*tsys_ = tdummy;
    }

    // telescope
    // remove trailing spaces from the telescope name
    (*telescope_) = (*telescope_).before(RXwhite, (*telescope_).length()-1);

    // telescope_position
    MPosition obsPos;
    if (siteLong_.isNull() || siteLat_.isNull() || siteElev_.isNull()) {
        // attempt to punt ...
	if (!MeasTable::Observatory(obsPos, *telescope_)) {
	    // apparently not in the table by that name
	    // NRAO43M is in the table as just GB
	    if (*telescope_ == "NRAO43M") {
		MeasTable::Observatory(obsPos, "GB");
	    } 
	}
    } else {
	// go with it
	// but this expects ITRF XYZ, so make a Position and convert
        obsPos = MPosition(Quantity(siteElev_.asdouble(thisRow_), "m"),
			   Quantity(siteLong_.asdouble(thisRow_), "deg"),
			   Quantity(siteLat_.asdouble(thisRow_), "deg"),
			   MPosition::WGS84);
	obsPos = MPosition::Convert(obsPos, MPosition::ITRF)();
    }
    if (!MeasureHolder(obsPos).toRecord(holderErr, *telescopePosition_)) {
	String errMsg = 
	    "SDTableIterator::copyAll() unexpected problem saving telescope position measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // pressure
    if (pressure_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	pressure_p.copyToField(thisRow_);
    }

    // dewpoint
    if (tdew_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	tdew_p.copyToField(thisRow_);
    }

    // tambient
    if (tamb_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	tamb_p.copyToField(thisRow_);
    }

    // winddir
    if (windDir_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	windDir_p.copyToField(thisRow_);
    }

    // windspeed
    if (windSpeed_p.isAttached()) {
	// it can't return a False if its attached, so ignore the return value
	windSpeed_p.copyToField(thisRow_);
    }

    // azel
    Double az, el;
    String unit = "deg";
    if (!azimuth_.isNull()) {
	az = azimuth_.asdouble(thisRow_);
	if (azimuth_.keywordSet().fieldNumber("UNIT") >= 0) {
	    unit = azimuth_.keywordSet().asString("UNIT");
	    if (unit == "") unit = "deg";
	}
    } else {
	az = 0.0;
    }
    if (!elevation_.isNull()) {
	el = elevation_.asdouble(thisRow_);
	if (elevation_.keywordSet().fieldNumber("UNIT") >= 0) {
	    String elunit = elevation_.keywordSet().asString("UNIT");
	    if (elunit != unit && elunit != "") {
		// convert el to the correct units
		el = Quantity(el,elunit).getValue(unit);
	    }
	}
    } else {
	el = 0.0;
    }
    MDirection azel(Quantity(az,unit), Quantity(el, unit), MDirection::AZEL);
    if (!MeasureHolder(azel).toRecord(holderErr, *azel_)) {
	String errMsg = 
	    "SDTableIterator::copyAll() unexpected problem saving azel measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }    
}

void SDTableIterator::copyOther()
{
    // do nothing if there is nothing in tab_
    if (tab_->nrow() == 0) return;
    // copy this row into tableRow - get will not read if this row
    // has already been read
    tableRow_->get(thisRow_);
    for (uInt i=0;i<otherCopiers_.nelements();i++) {
	if (otherCopiers_[i]) (otherCopiers_[i])->copy();
    }
    // any subscan field
    if (!scan_.isNull() && subscan_.isAttached()) {
	Double scannr = scan_.asdouble(thisRow_);
	*subscan_ = (scannr - Int(scannr+0.005)) * 100.0;
    }
}

void SDTableIterator::copyHist()
{
    // do nothing if there is nothing in tab_
    if (tab_->nrow() == 0) return;

    if (!historyCol_.isNull()) {
	rec_.hist().resize(historyCol_.shape(thisRow_));
	rec_.hist() = historyCol_(thisRow_);
    }
}
    
Bool SDTableIterator::replaceAll(const SDRecord& rec, Bool rowIsNew)
{
    LogIO os(LogOrigin("SDTableIterator","replaceAll(const SDRecord& rec)"));
    if (!isWritable()) {
	os << LogIO::SEVERE << WHERE
	   << "This iterator is not writable."
	   << LogIO::POST;
	return False;
    }
    // verify that rec appears to be a valid sdrecord
    if (!rec.isValid()) {
	os << LogIO::SEVERE << WHERE 
	   << "The record to be placed in this iterator is not a valid SDRecord."
	   << LogIO::POST;
	return False;
    }
    // verify that it is consistent
    String errmsg;
    if (!rec.isConsistent(errmsg)) {
	os << LogIO::SEVERE << WHERE
	   << "The record to be placed in this iterator is not internally consistent."
	   << LogIO::POST;
	os << LogIO::SEVERE << WHERE
	   << errmsg
	   << LogIO::POST;
	return False;
    }
    // ensure that it has the required output columns
    ensureOutputColumns();

    Bool anyNew = False;
    // watch for problems here
    Bool newok = True;
    for (uInt whichSubrec=0;whichSubrec<rec.other().nfields();whichSubrec++) {
	if (rec.other().dataType(whichSubrec) != TpRecord) continue;

	// look at contents of each other subrecord of other of rec and rec_
	// any fields in rec, not yet in rec_, need to be added
	// merge might have been used here, but that requires that matching
	// field names have exactly the same type, this only requires that
	// both are either Arrays or Scalar types (we don't allow
	// the other records to have sub records or tables)
	const Record &inRec = rec.other().subRecord(whichSubrec);
	Bool newFields = False;
	String subrecName = rec.other().name(whichSubrec);
	if (rec_.other().fieldNumber(subrecName) < 0) {
	    // rec_ needs this subrecord
	    rec_.other().defineRecord(subrecName, Record());
	    // and a place in stringFields_
	    stringFields_.rwSubRecord("other").defineRecord(subrecName, Record());
	}
	Record &outRec = rec_.other().rwSubRecord(whichSubrec);
	for (uInt i=0;i<inRec.nfields();i++) {
	    Int whichField = outRec.fieldNumber(inRec.name(i));
	    if (whichField < 0) {
		// add it in as long as it isn't a subrecord
		if (inRec.type(i) == TpRecord) {
		    os << LogIO::SEVERE << WHERE 
		       << "New other record field " << inRec.name(i) 
		       << " is a record.  Only scalar and array fields are allowed in the other records."
		       << LogIO::POST;
		    return False;
		}
		outRec.mergeField(inRec, i);
		if (inRec.type(i) == TpString) {
		    stringFields_.rwSubRecord("other").rwSubRecord(subrecName).define(inRec.name(i), True);
		}
		newFields = True;
		if (!anyNew) {
		    anyNew = True;
		    // the copiers will all need to be reset because of the new fields
		    // this makes sure they are cleaned up, but only once.
		    deleteCopiers();
		}
	    } else {
		// found it
		// Note that there may be a problem with single element
		// arrays on the glish side being turned into scalars here.
		if (inRec.description().isArray(i) != 
		    outRec.description().isArray(whichField) &&
		    inRec.description().isScalar(i) != 
		    outRec.description().isScalar(whichField)) {
		    os << LogIO::SEVERE << WHERE 
		       << "The input other." << subrecName << " field " << inRec.name(i)
		       << " is not the same scalar or array type as the field"
		       << "of the same name already in this iterator"
		       << LogIO::POST;
		    return False;
		}
		// copy this value, allow for different types
		switch (outRec.dataType(whichField)) {
		case TpBool:
		    outRec.define(inRec.name(i), inRec.asBool(i));
		    break;
		case TpUChar:
		    outRec.define(inRec.name(i), inRec.asuChar(i));
		    break;
		case TpShort:
		    outRec.define(inRec.name(i), inRec.asShort(i));
		    break;
		case TpInt:
		    outRec.define(inRec.name(i), inRec.asInt(i));
		    break;
		case TpUInt:
		    outRec.define(inRec.name(i), inRec.asuInt(i));
		    break;
		case TpFloat:
		    outRec.define(inRec.name(i), inRec.asFloat(i));
		    break;
		case TpDouble:
		    outRec.define(inRec.name(i), inRec.asDouble(i));
		    break;
		case TpComplex:
		    outRec.define(inRec.name(i), inRec.asComplex(i));
		    break;
		case TpDComplex:
		    outRec.define(inRec.name(i), inRec.asDComplex(i));
		    break;
		case TpString:
		    outRec.define(inRec.name(i), inRec.asString(i));
		    break;
		case TpArrayBool:
		    outRec.define(inRec.name(i), inRec.asArrayBool(i));
		    break;
		case TpArrayUChar:
		    outRec.define(inRec.name(i), inRec.asArrayuChar(i));
		    break;
		case TpArrayShort:
		    outRec.define(inRec.name(i), inRec.asArrayShort(i));
		    break;
		case TpArrayInt:
		    outRec.define(inRec.name(i), inRec.asArrayInt(i));
		    break;
		case TpArrayUInt:
		    outRec.define(inRec.name(i), inRec.asArrayuInt(i));
		    break;
		case TpArrayFloat:
		    outRec.define(inRec.name(i), inRec.asArrayFloat(i));
		    break;
		case TpArrayDouble:
		    outRec.define(inRec.name(i), inRec.asArrayDouble(i));
		    break;
		case TpArrayComplex:
		    outRec.define(inRec.name(i), inRec.asArrayComplex(i));
		    break;
		case TpArrayDComplex:
		    outRec.define(inRec.name(i), inRec.asArrayDComplex(i));
		    break;
		case TpArrayString:
		    outRec.define(inRec.name(i), inRec.asArrayString(i));
		    break;
		default:
		    os << LogIO::SEVERE << WHERE 
		       << "Field " << inRec.name(i) 
		       << " has an invalid type, " 
		       << Int(outRec.dataType(whichField))
		       << " -- this should never happen."
		       << LogIO::POST;
		    return False;
		    break;
		}
	    }
	}

	if (newFields) {
	    // this will also require that table columns be added
	    // at this point, everything in this other record MUST have a corresponding
	    // column in the main table (possibly as an uppercase version)
	    // so loop through each field, look for its counterpart in the table
	    // if not found, add it in, if found, do nothing, when done, set up
	    // the copier
	    Vector<String> newColNames(outRec.nfields());
	    Vector<Bool> newCols(outRec.nfields());
	    newCols = False;
	    for (uInt i=0;i<outRec.nfields();i++) {
		// The exception is any subscan number
		String name(outRec.name(i));
		if (name != "subscan") {
		    if (subrecName == "sdfits") {
			// make all upper case
			name.upcase();
		    } else {
			// signal that this came from an SDMS iterator
			// from this subrecord of other
			name = "SDMS{" + subrecName + "}" + name;
		    }
		    if (!tab_->tableDesc().isColumn(name)) {
			newCols(i) = True;
			newColNames(i) = name;
			switch (outRec.type(i)) {
			case TpBool:
			    {
				ScalarColumnDesc<Bool> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpUChar:
			    {
				ScalarColumnDesc<uChar> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpShort:
			    {
				ScalarColumnDesc<Short> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpUShort:
			    {
				ScalarColumnDesc<uShort> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpInt:
			    {
				ScalarColumnDesc<Int> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpUInt:
			    {
				ScalarColumnDesc<uInt> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpFloat:
			    {
				ScalarColumnDesc<Float> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpDouble:
			    {
				ScalarColumnDesc<Double> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpComplex:
			    {
				ScalarColumnDesc<Complex> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpDComplex:
			    {
				ScalarColumnDesc<DComplex> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpString:
			    {
				ScalarColumnDesc<String> scd(name);
				tab_->addColumn(scd);
			    }
			    break;
			case TpArrayBool:
			    tab_->addColumn(ArrayColumnDesc<Bool>(name));
			    break;
			case TpArrayUChar:
			    tab_->addColumn(ArrayColumnDesc<uChar>(name));
			    break;
			case TpArrayShort:
			    tab_->addColumn(ArrayColumnDesc<Short>(name));
			    break;
			case TpArrayUShort:
			    tab_->addColumn(ArrayColumnDesc<uShort>(name));
			    break;
			case TpArrayInt:
			    tab_->addColumn(ArrayColumnDesc<Int>(name));
			    break;
			case TpArrayUInt:
			    tab_->addColumn(ArrayColumnDesc<uInt>(name));
			    break;
			case TpArrayFloat:
			    tab_->addColumn(ArrayColumnDesc<Float>(name));
			    break;
			case TpArrayDouble:
			    tab_->addColumn(ArrayColumnDesc<Double>(name));
			    break;
			case TpArrayComplex:
			    tab_->addColumn(ArrayColumnDesc<Complex>(name));
			    break;
			case TpArrayDComplex:
			    tab_->addColumn(ArrayColumnDesc<DComplex>(name));
			    break;
			case TpArrayString:
			    tab_->addColumn(ArrayColumnDesc<String>(name));
			    break;
			default:
			    newCols(i) = False;
			    os << LogIO::SEVERE << WHERE 
			       << "Field " << name 
			       << " has an invalid type, " 
			       << Int(outRec.type(i))
			       << " -- this should never happen."
			       << LogIO::POST;
			    newok = False;
			    break;
			}
		    }
		}
	    }
	    // now fill these new columns with default values
	    for (uInt i=0;i<newCols.nelements();i++) {
		if (newCols(i)) {
		    TableColumn tcol(*tab_, newColNames(i));
		    switch (tcol.columnDesc().trueDataType()) {
		    case TpBool:
			{ 
			    ScalarColumn<Bool> scol(tcol);
			    scol.fillColumn(False); 
			}
			break;
		    case TpUChar:
			{ 
			    ScalarColumn<uChar> scol(tcol);
			    scol.fillColumn('\0'); 
			}
			break;
		    case TpShort:
			{ 
			    ScalarColumn<Short> scol(tcol);
			    scol.fillColumn(SHRT_MIN); 
			}
			break;
		    case TpUShort:
			{ 
			    ScalarColumn<uShort> scol(tcol);
			    scol.fillColumn(0); 
			}
			break;
		    case TpInt:
			{ 
			    ScalarColumn<Int> scol(tcol);
			    scol.fillColumn(LONG_MIN); 
			}
			break;
		    case TpUInt:
			{ 
			    ScalarColumn<uInt> scol(tcol);
			    scol.fillColumn(0); 
			}
			break;
		    case TpFloat:
			{ 
			    Float fnan;
			    setNaN(fnan);
			    ScalarColumn<Float> scol(tcol);
			    scol.fillColumn(fnan); 
			}
			break;
		    case TpDouble:
			{ 
			    Double dnan;
			    setNaN(dnan);
			    ScalarColumn<Double> scol(tcol);
			    scol.fillColumn(dnan); 
			}
			break;
		    case TpComplex:
			{ 
			    Float fnan;
			    setNaN(fnan);
			    ScalarColumn<Complex> scol(tcol);
			    scol.fillColumn(fnan); 
			}
			break;
		    case TpDComplex:
			{ 
			    Double dnan;
			    setNaN(dnan);
			    ScalarColumn<DComplex> scol(tcol);
			    scol.fillColumn(dnan); 
			}
			break;
		    case TpString:
			{ 
			    ScalarColumn<String> scol(tcol);
			    scol.fillColumn(String("")); 
			}
			break;
		    case TpArrayBool:
			{
			    ArrayColumn<Bool> acol(tcol);
			    acol.fillColumn(Array<Bool>());
			}
			break;
		    case TpArrayUChar:
			{
			    ArrayColumn<uChar> acol(tcol);
			    acol.fillColumn(Array<uChar>());
			}
			break;
		    case TpArrayShort:
			{
			    ArrayColumn<Short> acol(tcol);
			    acol.fillColumn(Array<Short>());
			}
			break;
		    case TpArrayUShort:
			{
			    ArrayColumn<uShort> acol(tcol);
			    acol.fillColumn(Array<uShort>());
			}
			break;
		    case TpArrayInt:
			{
			    ArrayColumn<Int> acol(tcol);
			    acol.fillColumn(Array<Int>());
			}
			break;
		    case TpArrayUInt:
			{
			    ArrayColumn<uInt> acol(tcol);
			    acol.fillColumn(Array<uInt>());
			}
			break;
		    case TpArrayFloat:
			{
			    ArrayColumn<Float> acol(tcol);
			    acol.fillColumn(Array<Float>());
			}
			break;
		    case TpArrayDouble:
			{
			    ArrayColumn<Double> acol(tcol);
			    acol.fillColumn(Array<Double>());
			}
			break;
		    case TpArrayComplex:
			{
			    ArrayColumn<Complex> acol(tcol);
			    acol.fillColumn(Array<Complex>());
			}
			break;
		    case TpArrayDComplex:
			{
			    ArrayColumn<DComplex> acol(tcol);
			    acol.fillColumn(Array<DComplex>());
			}
			break;
		    case TpArrayString:
			{
			    ArrayColumn<String> acol(tcol);
			    acol.fillColumn(Array<String>());
			}
			break;
		    default:
			os << LogIO::SEVERE << WHERE
			   << "Field " << newCols(i)
			   << " has an invalid type, " 
			   << Int(tcol.columnDesc().trueDataType())
			   << " -- this should never happen."
			   << LogIO::POST;
			newok = False;
			break;
		    }
		}
		
	    }
	}
    }
    // any new stuff
    if (anyNew) {
	// reset the copiers
	setCopiers();

	// reset the axes
	axes_->reset(*tab_);
	// if there was a problem, getting to here has reset things
	// but we still need to return False now.
	if (!newok) return False;
    }

    // resize internal rec_ and copy the fixed sub-records
    rec_.resize(rec.shape());
    rec_.data() = rec.data();
    rec_.header() = rec.header();
    rec_.hist().resize(rec.hist().shape());
    rec_.hist() = rec.hist();

    // copy the other records - these aren't fixed so a simple
    // copy would break the CopyRecordToRecord that goes from rec_
    // back to the rwTableRow_.  So, we use the copyRecord 
    // function to do that.
    if (!copyRecord(rec_.other(), rec.other())) {
	return False;
    }

    // get the current row, to be changed
    rwTableRow_->get(thisRow_,True);
    // do the copying
    // the scalar copiers
    rwHeaderCopier_->copy();
    for (uInt i=0;i<rwOtherCopiers_.nelements();i++) {
	if (rwOtherCopiers_[i]) {
	    rwOtherCopiers_[i]->copy();
	}
    }
    // and replace it
    
    rwTableRow_->put();
    // retrieve the chan_freq values from the record
    QuantumHolder cfHolder;
    String holderErr;
    if (!cfHolder.fromRecord(holderErr, *chanFreqRec_)) {
      String errMsg = 
	"SDTableIterator::replaceAll() unexpected problem retrieving chan_freq Quanta from record : " +
	holderErr;
      os << LogIO::SEVERE << WHERE 
	 << errMsg
	 << LogIO::POST;
      return False;
    }

    // this has to be done here so that one more sanity check can be done now.
    Quantum<Vector<Double> > chanFreq(cfHolder.asQuantumVectorDouble());
    Int nChanFreq = chanFreq.getValue().nelements();
    IPosition arrShape((*arr_).shape());
    // sanity check, must be same as number of pixels on second axis of data array
    if (nChanFreq != arrShape(1)) {
      os << LogIO::SEVERE << WHERE
	 << "The record to be placed in this iterator is not internally consistent."
	 << LogIO::POST;
      os << LogIO::SEVERE << WHERE
	 << "the number of channels in chan_freq is not the same as the number of pixels on the second axis of arr"
	 << LogIO::POST;
      return False;
    }

    // Do the data, weights, sigma, and flag columns next
    // the axes - put them back where we got them
    // or in their preferred location if this is a new row
    // we currently just know about the freq, stokes, ra, dec, and time axes
    // ignore any beam or rcvr axes
    // this array will hold them in the generally preferred order
    // freq, long, lat Int itsFreqAxis, longAxis, latAxis, timeAxis, stokesAxis;
    Int itsFreqAxis, itsLongAxis, itsLatAxis, itsTimeAxis, itsStokesAxis;
    itsFreqAxis = itsLongAxis = itsLatAxis = itsTimeAxis = itsStokesAxis = -1;
    if (!rowIsNew) {
      // try and get existing axes
      itsFreqAxis = axes_->freqAxis(thisRow_);
      itsLongAxis = axes_->longAxis(thisRow_);
      itsLatAxis = axes_->latAxis(thisRow_);
      itsStokesAxis = axes_->stokesAxis(thisRow_);
      itsTimeAxis = axes_->timeAxis(thisRow_);
    } else {
      // if there is more than 1 row, try and use the first as a model
      if (tab_->nrow() > 1) {
	itsFreqAxis = axes_->freqAxis(0);
	itsLongAxis = axes_->longAxis(0);
	itsLatAxis = axes_->latAxis(0);
	itsStokesAxis = axes_->stokesAxis(0);
	itsTimeAxis = axes_->timeAxis(0);
      }
    }
    Int extraAxes = 0;
    if (itsFreqAxis < 0) itsFreqAxis = axes_->maxaxes() + extraAxes++;
    if (itsLongAxis < 0) itsLongAxis = axes_->maxaxes() + extraAxes++;
    if (itsLatAxis < 0) itsLatAxis = axes_->maxaxes() + extraAxes++;
    if (itsStokesAxis < 0) itsStokesAxis = axes_->maxaxes() + extraAxes++;
    // only ADD a time axis if there is no TIME column
    if (itsTimeAxis < 0 && timeOutCol_.isNull()) itsTimeAxis = axes_->maxaxes() + extraAxes++;
    // if there is a time axis, only use it if there isn't a TIME column
    if (itsTimeAxis >= 0 && !timeOutCol_.isNull()) itsTimeAxis = -1;
	    
    Int naxes = axes_->maxaxes() + extraAxes;
    IPosition dataShape(naxes);
    // most of the axes in the output DATA column are degenerate
    dataShape = 1;
    // this assumes that arrShape has only 2 axes, stokes and freq-like
    dataShape(itsFreqAxis) = arrShape(1);
    dataShape(itsStokesAxis) = arrShape(0);
    // on output, at least here, the sdrecord always takes up
    // just a single row.  The only trick is if the output
    // DATA column can not be reshaped.  Ignore that for now.
    // The other trick is that if there is more than one stokes pixel, 
    // then the data array will need to be transformed since on output
    // the frequency axis comes first but on input it comes second.
    data_out_.setShape(thisRow_, dataShape);
    Bool doTransform = itsFreqAxis < itsStokesAxis && arrShape(0) > 1;
    if (doTransform) {
      data_out_.put(thisRow_, transpose(Matrix<Float>(*arr_)).reform(dataShape));
    } else {
      data_out_.put(thisRow_, (*arr_).reform(dataShape));
    }
    // similarly for flag, weights, and sigma. Right now, we know we can change the shape here
    flag_out_.setShape(thisRow_, dataShape);
    if (doTransform) {
	flag_out_.put(thisRow_, transpose(Matrix<Bool>(*flag_)).reform(dataShape));
    } else {
	flag_out_.put(thisRow_, (*flag_).reform(dataShape));
    }

    // weights
    weight_out_.setShape(thisRow_, dataShape);
    if (doTransform) {
	weight_out_.put(thisRow_, transpose(Matrix<Float>(*weight_)).reform(dataShape));
    } else {
	weight_out_.put(thisRow_, (*weight_).reform(dataShape));
    }

    // sigma
    sigma_out_.setShape(thisRow_, dataShape);
    if (doTransform) {
	sigma_out_.put(thisRow_, transpose(Matrix<Float>(*sigma_)).reform(dataShape));
    } else {
	sigma_out_.put(thisRow_, (*sigma_).reform(dataShape));
    }
    // set the actual axes descriptor values
	
    ctypes_.resize(naxes);
    cunits_.resize(naxes);
    cdelts_.resize(naxes);
    crvals_.resize(naxes);
    crpixs_.resize(naxes);
    crotas_.resize(naxes);
    // crotas are all always zero on ouput
    crotas_ = 0.0;

    // velocity definition
    MDoppler::Types vdef = MDoppler::RADIO;
    if (!MDoppler::getType(vdef, *veldef_)) {
      os << LogIO::WARN << WHERE
	 << "Unrecognized velocity definition : " << *veldef_ << ".  Using RADIO instead."
	 << LogIO::POST;
      vdef = MDoppler::RADIO;
    }
    // first 4 characters of CTYPE for frequency axis from the units of chanFreq
    if (chanFreq.isConform("Hz")) {
      ctypes_(itsFreqAxis) = "FREQ";
    } else if (chanFreq.isConform("m/s")) {
      ctypes_(itsFreqAxis) = "VELO";
      // unless it uses the OPTICAL radio definition
      if (vdef == MDoppler::OPTICAL) ctypes_(itsFreqAxis) = "FELO";
      // should probably check that vdef isn't something else - only support RADIO and OPTICAL here
    } else {
      // nothing else can be stored here
      os << LogIO::SEVERE << WHERE
	 << "Non frequency-like axes can not be stored in an SDFITS-based table."
	 << LogIO::POST;
      return False;
    }
    // the second 4 characters come from the refframe converted to a tag
    // the velref integer is ignored here
    MFrequency::Types rframe;
    if (*refframe_ != "" ) {
      if (!MFrequency::getType(rframe, *refframe_)) {
	os << LogIO::WARN << WHERE
	   << "Unrecognized reference frame : " << *refframe_ << ".  Using TOPO instead."
	   << LogIO::POST;
	rframe = MFrequency::TOPO;
      }
    } else {
      // nothing better to do, default to TOPO
      rframe = MFrequency::TOPO;
    }
    String tag;
    Int velref;
    if (!FITSSpectralUtil::tagFromFrame(tag, velref, rframe)) {
      os << LogIO::SEVERE << WHERE
	 << "Unexpected failure to convert reference frame " << *refframe_ << " to a FITS tag."
	 << LogIO::POST;
      return False;
    }
    ctypes_(itsFreqAxis) += tag;
	
	// get the direction
    MeasureHolder dirHolder;
    if (!dirHolder.fromRecord(holderErr, *direction_)) {
      String errMsg = 
	"SDTableIterator::replaceAll() unexpected problem retrieving direction Measure from record : " +
	holderErr;
      os << LogIO::SEVERE << WHERE 
	 << errMsg
	 << LogIO::POST;
      return False;
    }
    if (!dirHolder.isMDirection()) {
      os << LogIO::SEVERE << WHERE
	 << "SDTableIterator::replaceAll() direction field is not a direction measure."
	 << LogIO::POST;
      return False;
    }
    MDirection dir(dirHolder.asMDirection());
    // return value should always be True here, don't bother checking it
    MDirection::Types dirType;
    MDirection::getType(dirType, dir.getRefString());
    // types we can handle
    switch (dirType) {
    case MDirection::J2000:
    case MDirection::JMEAN:
    case MDirection::JTRUE:
    case MDirection::APP:
    case MDirection::B1950:
    case MDirection::BMEAN:
    case MDirection::BTRUE:
      ctypes_(itsLongAxis) = "RA";
      ctypes_(itsLatAxis) = "DEC";
      break;
    case MDirection::GALACTIC:
      ctypes_(itsLongAxis) = "GLON";
      ctypes_(itsLatAxis) = "GLAT";
      break;
    case MDirection::HADEC:
      ctypes_(itsLongAxis) = "HA";
      ctypes_(itsLatAxis) = "DEC";
      break;
    case MDirection::AZEL:
      ctypes_(itsLongAxis) = "AZ";
      ctypes_(itsLatAxis) = "EL";
      break;
    case MDirection::ECLIPTIC:
      ctypes_(itsLongAxis) = "ELON";
      ctypes_(itsLatAxis) = "ELAT";
      break;
    default:
      // types that can't be handled by SDFITS
      os << LogIO::SEVERE << WHERE
	 << "SDTableIterator::replaceAll() direction field has an unrecognized reference code: "
	 << MDirection::showType(dirType)
	 << LogIO::POST;
      return False;
    }
	    
    ctypes_(itsStokesAxis) = "STOKES";

    // appropriate units
    cunits_(itsFreqAxis) = chanFreq.getUnit();
    // always write out the position units as degrees, I think
    cunits_(itsLongAxis) = "deg";
    cunits_(itsLatAxis) =  cunits_(itsLongAxis);
    cunits_(itsStokesAxis) = "";

    if (nChanFreq > 0) {
      // figure out what the reference pixel is using reffrequency value
      Quantity refFreq(*refFreq_, cunits_(itsFreqAxis));

      Double refPix, refVal, delta;
      refPix = 1.0;
      refVal = chanFreq.getValue()(0);
      delta = 0.0;

      if (chanFreq.getValue().nelements() > 1) {
	// initial delta guess
	// chanWidth is ignored here, use the chanFreq values as they are what counts
	delta = chanFreq.getValue()(1) - refVal;
	// if reffrequency units conform with chan_freq units, use it
	if (refFreq.isConform(chanFreq.getUnit())) {
	  refVal = refFreq.getValue(chanFreq.getFullUnit());
	  // since chanFreq might be non-linear, here we want to find the
	  // reference pixel using the delta closest to the referenceValue
	  // use delta at end as initial guessm - also refines delta
	  refPix = getRefPix(chanFreq.getValue(), refVal, delta);
	  // that is zero-relative, make it 1-relative for FITS
	  refPix += 1;
	}
      } 
      crpixs_(itsFreqAxis) = refPix;
      crvals_(itsFreqAxis) = refVal;
      cdelts_(itsFreqAxis) = delta;
    } else {
      crvals_(itsFreqAxis) = 0.0;
      cdelts_(itsFreqAxis) = 1.0;
    }

    Unit deg("deg");
    crvals_(itsLongAxis) = dir.getAngle().getValue(deg)(0);
    crvals_(itsLatAxis) = dir.getAngle().getValue(deg)(1);

    Vector<String> stokes(*corrType_);
    if (stokes.nelements() > 1) {
      crvals_(itsStokesAxis) = Stokes::FITSValue(Stokes::type(stokes(0)));
    } else {
      crvals_(itsStokesAxis) = Stokes::Undefined;
    }

    // they are degenerate axes, but given them non-zero increments anyway
    cdelts_(itsLongAxis) = 1.0;
    cdelts_(itsLatAxis) = 1.0;
    if (stokes.nelements() > 1) {
      // this assumes that the appropriate delta is from
      // stokes(0) to stokes(1) and that this works for
      // generating all of the other stokes.  I need to
      // think longer and harder about the implications here.
      cdelts_(itsStokesAxis) = 
	Stokes::FITSValue(Stokes::type(stokes(1))) - crvals_(itsStokesAxis);
    } else {
      cdelts_(itsStokesAxis) = 1.0;
    }
 
    // reference pixel is always 1 at this point
    crpixs_(itsLongAxis) = 1.0;
    crpixs_(itsLatAxis) = 1.0;
    crpixs_(itsStokesAxis) = 1.0;

    MeasureHolder timeHolder;
    if (!timeHolder.fromRecord(holderErr, *time_)) {
      String errMsg = 
	"SDTableIterator::replaceAll() unexpected problem retrieving time Measure from record : " +
	holderErr;
      os << LogIO::SEVERE << WHERE 
	 << errMsg
	 << LogIO::POST;
      return False;
    }
    if (!timeHolder.isMEpoch()) {
      os << LogIO::SEVERE << WHERE
	 << "SDTableIterator::replaceAll() time field is not an epoch measure."
	 << LogIO::POST;
      return False;
    }
    MEpoch time(timeHolder.asMEpoch());
    // require UTC for now - do conversions eventually
    MEpoch::Types epoType;
    MEpoch::getType(epoType, time.getRefString());
    if (epoType != MEpoch::UTC) {
      os << LogIO::SEVERE << WHERE
	 << "SDTableIterator::replaceAll() time field is not in UTC."
	 << LogIO::POST;
      return False;
    }
    MVEpoch mvtime(time.getValue());
    // DATE_OBS gets the integer days, unless there is no time column
    // and no time axis, in which case it gets everything
    String dateString;
    String timesys;
    MVTime days;
    if (itsTimeAxis == 4 || !timeOutCol_.isNull()) {
	days = mvtime.getDay();
    } else {
	days = mvtime;
    }
    // no check on timesys, just assumes UTC
    FITSDateUtil::toFITS(dateString, timesys, days);
    dateObsOutCol_.putScalar(thisRow_, dateString);

    if (itsTimeAxis > 0) {
      ctypes_(itsTimeAxis) = "TIME";
      cunits_(itsTimeAxis) = "s";
      crvals_(itsTimeAxis) = Quantity(mvtime.getDayFraction(),"d").getValue("s");
      cdelts_(itsTimeAxis) = 1.0;
      crpixs_(itsTimeAxis) = 1.0;
    } else if (!timeOutCol_.isNull()) {
      if (dateObsOutCol_.isNull()) {
	// TIME column gets it all
	timeOutCol_.putScalar(thisRow_, mvtime.getTime().getValue("s"));
      } else {
	// just the fractional part of the day
	timeOutCol_.putScalar(thisRow_, Quantity(mvtime.getDayFraction(),"d").getValue("s"));
      }
    }

    // Now, save the axis info
    Bool added = False;
    Bool result = True;
    result = result && axes_->setctype(thisRow_, ctypes_, added);
    result = result && axes_->setcunit(thisRow_, cunits_, added);
    result = result && axes_->setcrval(thisRow_, crvals_, added);
    result = result && axes_->setcrpix(thisRow_, crpixs_, added);
    result = result && axes_->setcdelt(thisRow_, cdelts_, added);
    result = result && axes_->setcrota(thisRow_, crotas_, added);
    if (result && added) {
      // make sure this is all cleaned up
      cleanupMost();
      if (ensureSDFITSTable()) {
	init();
      } else {
	return False;
      }
    } else if (!result) {
      return False;
    }

    // special fields
	
    // rest freqency
    if (restFreq_p.isAttached()) {
      if (!restFreq_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "can not convert restfrequency to output table"
	   << LogIO::POST;
	return False;
      }
    }

    String velinfo;
    if (vdef == MDoppler::RADIO) {
	velinfo = "RADI";
    } else {
	// should probably double check this here
	velinfo = "OPTI";
    }
    velinfo += tag;
    velinfoOut_.putScalar(thisRow_, velinfo);


    // duration
    if (tdur_p.isAttached()) {
      if (!tdur_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert duration to output table"
	   << LogIO::POST;
	return False;
      }
    }

    // exposure
    if (texp_p.isAttached()) {
      if (!texp_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert exposure to output table"
	   << LogIO::POST;
	return False;
      }
    }

    // resolution
    if (resolution_p.isAttached()) {
      if (!resolution_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert resolution to output table"
	   << LogIO::POST;
	return False;
      }
    }

    // bandwidth
    if (bw_p.isAttached()) {
      if (!bw_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert bandwidth to output table"
	   << LogIO::POST;
	return False;
      }
    }

    // pressure
    if (pressure_p.isAttached()) {
      if (!pressure_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert pressure to output table"
	   << LogIO::POST;
	return False;
      }
    }

    // dewpoint
    if (tdew_p.isAttached()) {
      if (!tdew_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert dewpoint to output table"
	   << LogIO::POST;
	return False;
      }
    }

    // tambient
    if (tamb_p.isAttached()) {
      if (!tamb_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert tambient to output table"
	   << LogIO::POST;
	return False;
      }
    }

    // wind direction
    if (windDir_p.isAttached()) {
      if (!windDir_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert wind direction to output table"
	   << LogIO::POST;
	return False;
      }
    }

    // wind speed
    if (windSpeed_p.isAttached()) {
      if (!windSpeed_p.copyFromField(thisRow_)) {
	os << LogIO::SEVERE << WHERE
	   << "cannot convert wind speed to output table"
	   << LogIO::POST;
	return False;
      }
    }

    MeasureHolder azelHolder;
    if (!azelHolder.fromRecord(holderErr, *azel_)) {
      String errMsg = 
	"SDTableIterator::replaceAll() unexpected problem retrieving azel Measure from record : " +
	holderErr;
      os << LogIO::SEVERE << WHERE 
	 << errMsg
	 << LogIO::POST;
      return False;
    }
    if (!azelHolder.isMDirection()) {
      os << LogIO::SEVERE << WHERE
	 << "SDTableIterator::replaceAll() azel field is not a direction measure."
	 << LogIO::POST;
      return False;
    }
    MDirection azel(azelHolder.asMDirection());
    // make sure this really is an AZEL direction
    MDirection::Types azelType;
    MDirection::getType(azelType, azel.getRefString());
    if (azelType != MDirection::AZEL) {
      os << LogIO::SEVERE << WHERE
	 << "SDTableIterator::replaceAll() azel field is not an AZEL direction measure."
	 << LogIO::POST;
      return False;
    }
    String unit;
    if (azimuthOut_.keywordSet().fieldNumber("UNIT") >= 0) {
	unit = azimuthOut_.keywordSet().asString("UNIT");
    }
    if (unit == "") unit = "deg";
    azimuthOut_.putScalar(thisRow_, azel.getAngle().getValue(unit)(0));

    if (elevationOut_.keywordSet().fieldNumber("UNIT") >= 0) {
	unit = elevationOut_.keywordSet().asString("UNIT");
    }
    if (unit == "") unit = "deg";
    elevationOut_.putScalar(thisRow_, azel.getAngle().getValue(unit)(1));

    MeasureHolder refDirHolder;
    if (!refDirHolder.fromRecord(holderErr, *refDirection_)) {
	String errMsg = 
	    "SDTableIterator::replaceAll() unexpected problem retrieving refdirection Measure from record : " +
	    holderErr;
	os << LogIO::SEVERE << WHERE 
	   << errMsg
	   << LogIO::POST;
	return False;
    }
    if (!refDirHolder.isMDirection()) {
	os << LogIO::SEVERE << WHERE
	   << "SDTableIterator::replaceAll() refdirection field is not a direction measure."
	   << LogIO::POST;
	return False;
    }
    MDirection refDir(refDirHolder.asMDirection());
    // this must have the same type as the direction field
    MDirection::Types refDirType;
    MDirection::getType(refDirType, refDir.getRefString());
    if (dirType != refDirType) {
	os << LogIO::SEVERE << WHERE
	   << "SDTableIterator::replaceAll() refdirection must be of the same type as direction."
	   << LogIO::POST;
	return False;
    }
    if (refDirOutCol_.keywordSet().fieldNumber("UNIT") >= 0) {
	unit = refDirOutCol_.keywordSet().asString("UNIT");
    }
    if (unit == "") unit = "deg";
    refDirOutCol_.put(thisRow_,refDir.getAngle().getValue(unit));


    // this is not complete, we should handle other cases as well
    Double equinox = 2000.0;
    if (dirType == MDirection::B1950) equinox = 1950.0;
    equinoxOutCol_.putScalar(thisRow_, equinox);


    Double scannr = *scan_number_;
    if (subscan_.isAttached()) {
	scannr = scannr + *subscan_ / 100.0;
    }
    // this needs to be done throughout, as with the axes in SDIterAxes
    switch (scanOut_.columnDesc().trueDataType()) {
    case TpUShort:
	{
	    if (scannr <= USHRT_MAX && scannr >= 0.0) {
		uShort val = uShort(scannr);
		if (near(Double(val),scannr)) {
		    scanOut_.putScalar(thisRow_, val);
		} else {
		    os << LogIO::SEVERE
		       << WHERE
		       << "Unable to demote the value "
		       << scannr << " to the uShort type of the output column "
		       << scanOut_.columnDesc().name()
		       << LogIO::POST;
		    result = False;
		}
	    } else {
		os << LogIO::SEVERE
		   << WHERE
		   << "Unable to demote the value "
		   << scannr << " to the uShort type of the output column "
		   << scanOut_.columnDesc().name()
		   << LogIO::POST;
		result = False;
	    }
	}
	break;
    case TpShort:
	{
	    if (scannr <= SHRT_MAX && scannr >= SHRT_MIN) {
		Short val = Short(scannr);
		if (near(Double(val),scannr)) {
		    scanOut_.putScalar(thisRow_, val);
		} else {
		    os << LogIO::SEVERE
		       << WHERE
		       << "Unable to demote the value "
		       << scannr << " to the Short type of the output column "
		       << scanOut_.columnDesc().name()
		       << LogIO::POST;
		    result = False;
		}
	    } else {
		os << LogIO::SEVERE
		   << WHERE
		   << "Unable to demote the value "
		   << scannr << " to the Short type of the output column "
		   << scanOut_.columnDesc().name()
		   << LogIO::POST;
		result = False;
	    }
	    break;
	}
    case TpUInt:
	{
	    if (scannr <= ULONG_MAX && scannr >= 0.0) {
		uInt val = uInt(scannr);
		if (near(Double(val),scannr)) {
		    scanOut_.putScalar(thisRow_, val);
		} else {
		    os << LogIO::SEVERE
		       << WHERE
		       << "Unable to demote the value "
		       << scannr << " to the uInt type of the output column "
		       << scanOut_.columnDesc().name()
		       << LogIO::POST;
		    result = False;
		}
	    } else {
		os << LogIO::SEVERE
		   << WHERE
		   << "Unable to demote the value "
		   << scannr << " to the uInt type of the output column "
		   << scanOut_.columnDesc().name()
		   << LogIO::POST;
		result = False;
	    }
	}
	break;
    case TpInt:
	{
	    if (scannr <= LONG_MAX && scannr >= LONG_MIN) {
		Int val = Int(scannr);
		if (near(Double(val),scannr)) {
		    scanOut_.putScalar(thisRow_, val);
		} else {
		    os << LogIO::SEVERE
		       << WHERE
		       << "Unable to demote the value "
		       << scannr << " to the Int type of the output column "
		       << scanOut_.columnDesc().name()
		       << LogIO::POST;
		    result = False;
		}
	    } else {
		os << LogIO::SEVERE
		   << WHERE
		   << "Unable to demote the value "
		   << scannr << " to the Int type of the output column "
		   << scanOut_.columnDesc().name()
		   << LogIO::POST;
		result = False;
	    }
	}
	break;
    case TpFloat:
    case TpDouble:
	scanOut_.putScalar(thisRow_, scannr);
	break;
    default:
	os << LogIO::SEVERE
	   << WHERE
	   << "Unable to store the value "
	   << scannr << " to the "
	   << Int(scanOut_.columnDesc().trueDataType())
	   << " type of the output column "
	   << scanOut_.columnDesc().name()
	   << LogIO::POST;
	result = False;
	break;
    }

    // telescope information
    MeasureHolder telPosHolder;
    if (!telPosHolder.fromRecord(holderErr, *telescopePosition_)) {
	String errMsg = 
	    "SDTableIterator::replaceAll() unexpected problem retrieving telescope_position Measure from record : " +
	    holderErr;
	os << LogIO::SEVERE << WHERE 
	   << errMsg
	   << LogIO::POST;
	return False;
    }
    if (!telPosHolder.isMPosition()) {
	os << LogIO::SEVERE << WHERE
	   << "SDTableIterator::replaceAll() telescope_position field is not a position measure."
	   << LogIO::POST;
	return False;
    }
    MPosition telPos(telPosHolder.asMPosition());
    MPosition::Convert towgs(telPos, MPosition::WGS84);
    MPosition wgspos(towgs());
    MVPosition mvtpos(wgspos.getValue());
    unit = "m";
    // and put them in place, converting units as necessary
    if (siteElevOut_.keywordSet().fieldNumber("UNIT") >= 0) {
	unit = siteElevOut_.keywordSet().asString("UNIT");
    }
    if (unit == "") {
	unit = "m";
    }
    if (!mvtpos.getLength().isConform(unit)) {
	os << LogIO::SEVERE << WHERE
	   << "Telescope position elevation can not be converted to output units of " << unit
	   << LogIO::POST;
	return False;
    }
    siteElevOut_.putScalar(thisRow_,mvtpos.getLength().getValue(unit));
    unit = "deg";
    if (siteLongOut_.keywordSet().fieldNumber("UNIT") >= 0) {
	unit = siteLongOut_.keywordSet().asString("UNIT");
    }
    if (unit == "") unit = "deg";
    Quantity qrad(0.0,"rad");
    if (!qrad.isConform(unit)) {
	os << LogIO::SEVERE << WHERE
	   << "Telescope position longitude can not be converted to output units of " << unit
	   << LogIO::POST;
	return False;
    }
    siteLongOut_.putScalar(thisRow_,mvtpos.getLong(unit).getValue());
    unit = "deg";
    if (siteLatOut_.keywordSet().fieldNumber("UNIT") >= 0) {
	unit = siteLatOut_.keywordSet().asString("UNIT");
    }
    if (unit == "") unit = "deg";
    if (!qrad.isConform(unit)) {
	os << LogIO::SEVERE << WHERE
	   << "Telescope position latitude can not be converted to output units of " << unit
	   << LogIO::POST;
	return False;
    }
    siteLatOut_.putScalar(thisRow_,mvtpos.getLat(unit).getValue());


    // molecule and transition are blended
    // last "," delimits transition from molecule
    String trans = *transition_;
    String molOut, transOut;
    molOut = "";
    transOut = "";
    uInt nr = trans.freq(',');
    if (nr == 0) {
      // no delimiter, everything into molecule
      molOut = trans;
    } else {
      uInt count = 0;
      uInt i;
      for (i=0;i<trans.length();i++) {
	if (trans[i] == ',') count++;
	if (count == nr) break;
      }
      // i == location of last ','
      if (i > 0) {
	transOut = trans(0,(i-1));
      } // else comma comes first, there is no transition here, leave blank
      if (i < (trans.length()-1)) {
	molOut = trans((i+1),(trans.length()-1));
      } // else comma comes in last place, there is no molecule here, leave blank
    }
    moleculeOutCol_.putScalar(thisRow_, molOut);

    transitionOutCol_.putScalar(thisRow_, transOut);

    // For these vector header words, just put one value into the scalar
    // column and the whole thing into the array column, if it exists
    Vector<Float> tmp;
    tmp = *tcal_;
    tcalOutCol_.putScalar(thisRow_, mean(tmp));
    if (!arrTcalOutCol_.isNull()) arrTcalOutCol_.put(thisRow_, tmp);

    tmp = *trx_;
    trxOutCol_.putScalar(thisRow_, mean(tmp));
    if (!arrTrxOutCol_.isNull()) arrTrxOutCol_.put(thisRow_, tmp);

    tmp = *tsys_;
    tsysOutCol_.putScalar(thisRow_, mean(tmp));
    if (!arrTsysOutCol_.isNull()) arrTsysOutCol_.put(thisRow_, tmp);

    // the history output column, always present at this point
    if (!historyOutCol_.isNull()) {
      historyOutCol_.put(thisRow_, 
			 rec_.hist().reform(IPosition(1,rec_.hist().nelements())));
    }
    // okay
    return True;
}

// everthing except tab_ needs to be cleaned up each time init()
// is called
void SDTableIterator::cleanupMost()
{
    delete axes_;
    axes_ = 0;

    deleteCopiers();
}

void SDTableIterator::cleanup()
{
    cleanupMost();
    // do tab_ last
    if (tab_) {
	// if tab_ has no rows, mark it for deletion
	if (tab_->nrow() == 0) tab_->markForDelete();
	delete tab_;
    }
    tab_ = 0;
}

void SDTableIterator::init()
{
    // it is expected that cleanupMost() will have been called first

    // this is all the tricky stuff

    // turn on the FITS unit map
    UnitMap::addFITS();

    // recognize "M/SEC" as velocity (dishdemo1 uses that, others may as well)
    UnitMap::putUser("M/SEC",UnitVal(1.0,"m/s"),"Some FITS files use this instead of m/s");

    // recognize "DEGREE" and "degree" as deg
    UnitMap::putUser("DEGREE",UnitVal(1.0,"deg"),"degree");
    UnitMap::putUser("degree",UnitVal(1.0,"deg"),"degree");

    if (axes_) {
	delete axes_;
	axes_ = 0;
    }
    axes_ = new SDIterAxes(*tab_);
    AlwaysAssert(axes_, AipsError);

    // if tab is writable, add the history column if not already present
    if (isWritable() && !tab_->tableDesc().isColumn("HISTORY")) {
    	tab_->addColumn(ArrayColumnDesc<String>("HISTORY", "", 1));
	ArrayColumn<String> hist(*tab_,"HISTORY");
	hist.fillColumn(Vector<String>());
    }
    
    // simple copiers (this also does any necessary rw copiers
    // as well as reset rec_ other fields as necessary and 
    // initializes the stringFields_ record
    setCopiers();

    // SDTabReqCopiers
    if (tab_->tableDesc().isColumn("RESTFREQ")) {
	restFreq_p.init("RESTFREQ", *tab_, "restfrequency", rec_.desc(), "Hz", !isWritable());
    }
    if (tab_->tableDesc().isColumn("DURATION")) {
	tdur_p.init("DURATION", *tab_, "duration", rec_.header(), "s", !isWritable());
    }
    if (tab_->tableDesc().isColumn("EXPOSURE")) {
	texp_p.init("EXPOSURE", *tab_, "exposure", rec_.header(), "s", !isWritable());
    }
    if (tab_->tableDesc().isColumn("FREQRES")) {
	resolution_p.init("FREQRES", *tab_, "resolution", rec_.header(), "Hz", !isWritable());
    }
    if (tab_->tableDesc().isColumn("BANDWID")) {
	bw_p.init("BANDWID", *tab_, "bandwidth", rec_.header(), "Hz", !isWritable());
    } else if (tab_->tableDesc().isColumn("BANDWIDTH")) {
	// recognize a previously used spelling in the SDFITS convention
	bw_p.init("BANDWID", *tab_, "bandwidth", rec_.header(), "Hz", !isWritable());
    }
    if (tab_->tableDesc().isColumn("PRESSURE")) {
	pressure_p.init("PRESSURE", *tab_, "pressure", rec_.header(), "hPa", !isWritable());
    }
    if (tab_->tableDesc().isColumn("DEWPOINT")) {
	pressure_p.init("DEWPOINT", *tab_, "dewpoint", rec_.header(), "K", !isWritable());
    }
    if (tab_->tableDesc().isColumn("TAMBIENT")) {
	pressure_p.init("TAMBIENT", *tab_, "tambient", rec_.header(), "K", !isWritable());
    }
    if (tab_->tableDesc().isColumn("WINDDIRE")) {
	pressure_p.init("WINDDIRE", *tab_, "wind_dir", rec_.header(), "rad", !isWritable());
    }
    if (tab_->tableDesc().isColumn("WINDSPEE")) {
	pressure_p.init("WINDSPEE", *tab_, "wind_speed", rec_.header(), "m/s", !isWritable());
    }

    chanFreqRec_.attachToRecord(rec_.desc(),"chan_freq");
    refframe_.attachToRecord(rec_.desc(),"refframe");
    refFreq_.attachToRecord(rec_.desc(),"reffrequency");
    chanWidth_.attachToRecord(rec_.desc(),"chan_width");
    corrType_.attachToRecord(rec_.desc(),"corr_type");

    // deal with DATA column and other axes here
    // we can have one of two names for the data column
    if (tab_->tableDesc().isColumn("DATA")) {
	data_in_.attach(*tab_,"DATA");
    } 
    
    arr_.attachToRecord(rec_.data(),"arr");
    if (tab_->tableDesc().isColumn("FLAG")) {
	flag_in_.attach(*tab_,"FLAG");
    }
    flag_.attachToRecord(rec_.data(),"flag");
    if (tab_->tableDesc().isColumn("WEIGHT")) {
	weight_in_.attach(*tab_,"WEIGHT");
    }
    weight_.attachToRecord(rec_.data(),"weight");
    if (tab_->tableDesc().isColumn("SIGMA")) {
	sigma_in_.attach(*tab_,"SIGMA");
    }
    sigma_.attachToRecord(rec_.data(),"sigma");
    veldef_.attachToRecord(rec_.header(),"veldef");
    azel_.attachToRecord(rec_.header(),"azel");
    tdurField_p.attachToRecord(rec_.header(),"duration");
    texpField_p.attachToRecord(rec_.header(),"exposure");
    scan_number_.attachToRecord(rec_.header(),"scan_number");
    if (rec_.other().fieldNumber("subscan") >= 0 &&
	rec_.other().rwSubRecord("sdfits").fieldNumber("subscan") >= 0) {
	subscan_.attachToRecord(rec_.other().rwSubRecord("sdfits"),"subscan");
    }
    tsys_.attachToRecord(rec_.header(),"tsys");
    trx_.attachToRecord(rec_.header(),"trx");
    tcal_.attachToRecord(rec_.header(),"tcal");
    refDirection_.attachToRecord(rec_.header(),"refdirection");
    direction_.attachToRecord(rec_.header(),"direction");
    time_.attachToRecord(rec_.header(),"time");
    telescope_.attachToRecord(rec_.header(),"telescope");
    telescopePosition_.attachToRecord(rec_.header(),"telescope_position");
    transition_.attachToRecord(rec_.header(),"transition");
    if (tab_->tableDesc().isColumn("SCAN")) scan_.attach(*tab_, "SCAN");
    if (tab_->tableDesc().isColumn("EQUINOX")) equinoxCol_.attach(*tab_, "EQUINOX");
    if (tab_->tableDesc().isColumn("ELEVATIO")) elevation_.attach(*tab_, "ELEVATIO");
    if (tab_->tableDesc().isColumn("AZIMUTH")) azimuth_.attach(*tab_, "AZIMUTH");
    if (tab_->tableDesc().isColumn("VELDEF")) velinfo_.attach(*tab_, "VELDEF");
    // watch for vector columns of the same type as used in ms2sdfits for tcal et al
    if (tab_->tableDesc().isColumn("SYSCAL_TSYS") &&
	tab_->tableDesc().columnDesc("SYSCAL_TSYS").dataType() == TpFloat) {
	arrTsysCol_.attach(*tab_, "SYSCAL_TSYS");
    }
    if (tab_->tableDesc().isColumn("SYSCAL_TRX") &&
	tab_->tableDesc().columnDesc("SYSCAL_TRX").dataType() == TpFloat) {
	arrTrxCol_.attach(*tab_, "SYSCAL_TRX");
    }
    if (tab_->tableDesc().isColumn("SYSCAL_TCAL") &&
	tab_->tableDesc().columnDesc("SYSCAL_TCAL").dataType() == TpFloat) {
	arrTcalCol_.attach(*tab_, "SYSCAL_TCAL");
    }
    if (tab_->tableDesc().isColumn("TSYS")) tsysCol_.attach(*tab_, "TSYS");
    if (tab_->tableDesc().isColumn("TRX")) trxCol_.attach(*tab_, "TRX");
    if (tab_->tableDesc().isColumn("TCAL")) tcalCol_.attach(*tab_, "TCAL");
    if (tab_->tableDesc().isColumn("REFDIRECTION")) {
	refDirCol_.attach(*tab_,"REFDIRECTION");
    }
    if (tab_->tableDesc().isColumn("SITEELEV")) {
        siteElev_.attach(*tab_, "SITEELEV");
    } 
    if (tab_->tableDesc().isColumn("SITELONG")) {
        siteLong_.attach(*tab_, "SITELONG");
    } 
    if (tab_->tableDesc().isColumn("SITELAT")) {
        siteLat_.attach(*tab_, "SITELAT");
    } 
    if (tab_->tableDesc().isColumn("MOLECULE")) moleculeCol_.attach(*tab_,"MOLECULE");
    if (tab_->tableDesc().isColumn("TRANSITI")) transitionCol_.attach(*tab_,"TRANSITI");
    if (tab_->tableDesc().isColumn("DATE-OBS")) dateObsCol_.attach(*tab_,"DATE-OBS");
    if (tab_->tableDesc().isColumn("TIME")) timeCol_.attach(*tab_,"TIME");
    if (tab_->tableDesc().isColumn("HISTORY")) historyCol_.attach(*tab_,"HISTORY");
    // now do the same things for the case where the table is writable
    if (isWritable()) {
	if (tab_->tableDesc().isColumn("DATA")) {
	    data_out_.attach(*tab_,"DATA");
	} 
	if (tab_->tableDesc().isColumn("FLAG")) {
	    flag_out_.attach(*tab_,"FLAG");
	}
	if (tab_->tableDesc().isColumn("WEIGHT")) {
	    weight_out_.attach(*tab_,"WEIGHT");
	}
	if (tab_->tableDesc().isColumn("SIGMA")) {
	    sigma_out_.attach(*tab_,"SIGMA");
	}
	if (tab_->tableDesc().isColumn("SCAN")) scanOut_.attach(*tab_, "SCAN");
	if (tab_->tableDesc().isColumn("EQUINOX")) equinoxOutCol_.attach(*tab_, "EQUINOX");
	if (tab_->tableDesc().isColumn("ELEVATIO")) elevationOut_.attach(*tab_, "ELEVATIO");
	if (tab_->tableDesc().isColumn("AZIMUTH")) azimuthOut_.attach(*tab_, "AZIMUTH");
	if (tab_->tableDesc().isColumn("VELDEF")) velinfoOut_.attach(*tab_, "VELDEF");
	if (tab_->tableDesc().isColumn("TSYS")) tsysOutCol_.attach(*tab_, "TSYS");
	if (tab_->tableDesc().isColumn("TRX")) trxOutCol_.attach(*tab_, "TRX");
	if (tab_->tableDesc().isColumn("TCAL")) tcalOutCol_.attach(*tab_, "TCAL");
	// duplicate any array columns found above
	if (!arrTsysCol_.isNull()) {
	    arrTsysOutCol_.attach(*tab_, "SYSCAL_TSYS");
	}
	if (!arrTrxCol_.isNull()) {
	    arrTrxOutCol_.attach(*tab_, "SYSCAL_TRX");
	}
	if (!arrTcalCol_.isNull()) {
	    arrTcalOutCol_.attach(*tab_, "SYSCAL_TCAL");
	}
	if (tab_->tableDesc().isColumn("REFDIRECTION")) {
	    refDirOutCol_.attach(*tab_,"REFDIRECTION");
	}
	if (tab_->tableDesc().isColumn("SITEELEV")) {
	    siteElevOut_.attach(*tab_, "SITEELEV");
	} 
	if (tab_->tableDesc().isColumn("SITELONG")) {
	    siteLongOut_.attach(*tab_, "SITELONG");
	} 
	if (tab_->tableDesc().isColumn("SITELAT")) {
	    siteLatOut_.attach(*tab_, "SITELAT");
	} 
	if (tab_->tableDesc().isColumn("MOLECULE")) moleculeOutCol_.attach(*tab_,"MOLECULE");
	if (tab_->tableDesc().isColumn("TRANSITI")) transitionOutCol_.attach(*tab_,"TRANSITI");
	if (tab_->tableDesc().isColumn("DATE-OBS")) dateObsOutCol_.attach(*tab_,"DATE-OBS");
	if (tab_->tableDesc().isColumn("TIME")) timeOutCol_.attach(*tab_,"TIME");
	if (tab_->tableDesc().isColumn("HISTORY")) historyOutCol_.attach(*tab_,"HISTORY");
	// use the knowlege of the other axes as above
	// we'll need to test for this on output or otherwise make this
	// more general
    }
}

String replaceSpecial(const String& string)
{
    String result;
    result.alloc(2*string.length());
    int len = 0;
    for (int i=0; i<Int(string.length());i++) {
	char c = string[i];
	// Escape special characters
	// I don't know what the full list is, it would be nice if
	// this was available from Ger
	switch (c) {
	case '-':
	    result[len++] = '\\';
	}
	result[len++] = c;
    }
    return String(result.chars(),len);
}

Bool SDTableIterator::parseDescRecord(const Record& selection, String& result)
{
    LogIO os(LogOrigin("SDTableIterator","parseDescRecord"));
    uInt nfields = selection.nfields();
    Int nout = 0;
    result = "";

    // the four types of selection - reffrequency, restfrequency, refframe, chan_width
    // For Tables, this is : 
    //    reffrequency -> crval on freq axis
    //    restfrequency -> RESTFREQ
    //    refframe -> tag on ctype on freq axis
    //    chan_width -> cdelt on freq axis
    String ctypeSel, crvalSel, cdeltSel;
    Int restFreqField = -1;

    for (uInt i=0;i<nfields;i++) {
	String fieldResult;
	// is this one of the five handled fields
	String name=selection.name(i);
	// translate names to fields in table
	if (name == "reffrequency") name = "crval";
	else if (name == "restfrequency") name = "RESTFREQ";
	else if (name == "refframe") name = "ctype";
	else if (name == "chan_width") name = "cdelt";
	if (name != "crval" && name != "cdelt" && name != "ctype" && name != "RESTFREQ") {
	    os << LogIO::WARN
	       << "Selection on data.desc." << name << " is not supported."
	       << "\nThis field will be ignored."
	       << LogIO::POST;
	} else {
	    // first do some sanity checks on the column
	    if (name == "RESTFREQ") {
		// remember this for later
		restFreqField = i;
	    } else if (name == "ctype") {
		// a string selection
		Vector<String> list;
		if (selection.type(i) == TpString) {
		    // single value
		    list.resize(1);
		    selection.get(i, list(0));
		} else if (selection.type(i) == TpArrayString) {
		    // vector of values
		    selection.get(i, list);
		} else {
		    os << LogIO::WARN
		       << "Selections must consist of vectors of strings for "
		       << "fields which contain strings."
		       << "\nSelection field=" << selection.name(i)
		       << " violates this rule."
		       << "\nThis field will be ignored."
		       << LogIO::POST;
		    continue;
		}
		// translate the list of reference frames into FITS tags
		for (uInt i=0;i<list.nelements();i++) {
		    MFrequency::Types rframe;
		    if (!MFrequency::getType(rframe, list(i))) {
			os << LogIO::WARN
			   << "Unrecognized reference frame, " << list(i) << ", in selection record."
			   << "\nThis field will be ignored."
			   << LogIO::POST;
		    }
		    String tag;
		    Int velref;
		    if (!FITSSpectralUtil::tagFromFrame(tag, velref, rframe)) {
			os << LogIO::SEVERE
			   << "Unexpected failure to convert reference frame " << list(i) << " to a FITS tag."
			   << "\nPlease report this failure using the bug() facility."
			   << LogIO::POST;
		    }
		    // full pattern
		    list(i) = "*" + tag;
		}
		// assemble these discrete values into a selection for this column
		// these will be patterns so treat them as such
		for (uInt j=0;j<list.nelements();j++) {
		    if (list(j).length() != 0) {
			if (fieldResult.length() > 0) 
			    fieldResult = fieldResult + " || ";
			// the __CTOK__ token will be replaced later
			fieldResult = fieldResult + "__CTOK__" + " == " + 
			    "pattern(" + '"' + list(j) + '"' + ")";
		    }
		}
	    } else {
		// it must be one of the non-string fields
		if (selection.type(i) != TpArrayDouble) {
		    os << LogIO::WARN
		       << "Selections must consist of a matrix of doubles for "
		       << "fields which contain non-string values.\n"
		       << "Selection field=" << selection.name(i)
		       << " violates this rule."
		       << "\nThis field will be ignored."
		       << LogIO::POST;
		    continue;
		}
		Matrix<Double> ranges;
		// is the input glish Record a matrix
		if (selection.shape(i).nelements() == 2) {
		    selection.get(i, ranges);
		} else if (selection.shape(i).nelements() == 1 &&
			   selection.shape(i)(0) % 2 == 0) {
		    // if this is a vector with an even number of elements then
		    // assume its a 2 x nranges Matrix
		    Vector<Double> vranges;
		    selection.get(i, vranges);
		    IPosition mshape(2);
		    mshape(0) = 2;
		    mshape(1) = vranges.nelements()/2;
		    ranges = vranges.reform(mshape);
		}
		if (ranges.nrow() != 2) {
		    os << LogIO::WARN
		       << "The shape of the selection Matrix must be (2,nranges)\n"
		       << "Selection field=" << selection.name(i)
		       << " violates this rule."
		       << "\nThis field will be ignored."
		       << LogIO::POST;
		    continue;
		}
		// the __CTOK__ tokens will be replaced later
		for (uInt j=0;j<ranges.ncolumn();j++) {
		    String tag = "";
		    Double rmin, rmax;
		    rmin = ranges(0,j);
		    rmax = ranges(1,j);
		    if (rmin == -C::dbl_max && rmax == C::dbl_max) {
			// entire range is specified, no selection necessary
			continue;
		    } else if (rmin == rmax) {
			// single value, use identity selection
			tag = "__CTOK__  == " + doubleToString(rmin);
		    } else if (rmin == -C::dbl_max) {
			// upper limit
			tag = "__CTOK__ IN [ := " + 
			doubleToString(rmax) + " ] ";
		    } else if (rmax == C::dbl_max) {
			// lower limit
			tag = "__CTOK__ IN [ " + doubleToString(rmin) +
			    " =: ] ";
		    } else {
			// a true range, assume order is ok
			tag = "__CTOK__ IN [ " + doubleToString(rmin) +
			    " =:= " + doubleToString(rmax) + " ] ";
		    }
		    if (tag.length() == 0) continue;
		    if (fieldResult.length() > 0) fieldResult = fieldResult + " || ";
		    fieldResult = fieldResult + tag;
		}
	    }
	    if (fieldResult.length() > 0) {
		// The "__AXISTOK__" token is replaced later
		fieldResult = 
		    " ( __AXISTOK__ == REGEX('((^FREQ|^[VF]ELO)).*') ) AND ( " +
		    fieldResult + " ) ";
		if (name == "crval") {
		    crvalSel = fieldResult;
		} else if (name == "cdelt") {
		    cdeltSel = fieldResult;
		} else {
		    // must be ctype
		    ctypeSel = fieldResult;
		}
		nout++;
	    }
	}
    }
    if (nout > 0) {
	Bool firstResult = True;
	// axes may not yet exist.  However, it is safe to make them here if necessary.
	if (!axes_) {
	    axes_ = new SDIterAxes(*tab_);
	    AlwaysAssert(axes_, AipsError);
	}
	Vector<String> ctypeCols(axes_->ctypeColNames());
	if (!crvalSel.empty()) {
	    String crvalResult = makeFullAxisSel(crvalSel, ctypeCols, axes_->crvalColNames());
	    if (!crvalResult.empty()) {
		// we actually have something to add here
		if (!firstResult) result = result + " && ";
		else {
		    result = result + " ( ";
		    firstResult = False;
		}
		result = result + crvalResult;
	    }
	}
	if (!cdeltSel.empty()) {
	    String cdeltResult = makeFullAxisSel(cdeltSel, ctypeCols, axes_->cdeltColNames());
	    if (!cdeltResult.empty()) {
		// we actually have something to add here
		if (!firstResult) result = result + " && ";
		else {
		    result = result + " ( ";
		    firstResult = False;
		}
		result = result + cdeltResult;
	    }
	}
	if (!ctypeSel.empty()) {
	    String ctypeResult = makeFullAxisSel(ctypeSel, ctypeCols, axes_->ctypeColNames());
	    if (!ctypeResult.empty()) {
		// we actually have something to add here
		if (!firstResult) result = result + " && ";
		else {
		    result = result + " ( ";
		    firstResult = False;
		}
		result = result + ctypeResult;
	    }
	}
	if (!firstResult) result = result + " ) ";
    }

    // it should be safe to add any RESTFREQ selection here
    // this is lifted from parseRecord, which can't be used fully here
    if (restFreqField >= 0) {
	if (selection.type(restFreqField) != TpArrayDouble) {
	    os << LogIO::WARN
	       << "Selections must be a matrix of doubles for "
	       << "fields which contain non-string values.\n"
	       << "Selection field=" << selection.name(restFreqField)
	       << " violates this rule."
	       << "\nThis field will be ignored."
	       << LogIO::POST;
	} else {
	    String fieldResult;
	    Matrix<Double> ranges;
	    // is the input glish Record a matrix
	    if (selection.shape(restFreqField).nelements() == 2) {
		selection.get(restFreqField, ranges);
	    } else if (selection.shape(restFreqField).nelements() == 1 &&
		       selection.shape(restFreqField)(0) % 2 == 0) {
		// if this is a vector with an even number of elements then
		// assume its a 2 x nranges Matrix
		Vector<Double> vranges;
		selection.get(restFreqField, vranges);
		IPosition mshape(2);
		mshape(0) = 2;
		mshape(1) = vranges.nelements()/2;
		ranges = vranges.reform(mshape);
	    }
	    if (ranges.nrow() != 2) {
		os << LogIO::WARN
		   << "The shape of the selection Matrix must be (2,nranges)\n"
		   << "Selection field=" << selection.name(restFreqField)
		   << " violates this rule."
		   << "\nThis field will be ignored."
		   << LogIO::POST;
	    } else {
		for (uInt j=0;j<ranges.ncolumn();j++) {
		    String tag = "";
		    Double rmin, rmax;
		    rmin = ranges(0,j);
		    rmax = ranges(1,j);
		    if (rmin == -C::dbl_max && rmax == C::dbl_max) {
			// entire range is specified, no selection necessary
			continue;
		    } else if (rmin == rmax) {
			// single value, use identity selection
			tag = "RESTFREQ == " + doubleToString(rmin);
		    } else if (rmin == -C::dbl_max) {
			// upper limit
			tag = "RESTFREQ IN [ := " + 
			    doubleToString(rmax) + " ] ";
		    } else if (rmax == C::dbl_max) {
			// lower limit
			tag = "RESTFREQ IN [ " + doubleToString(rmin) +
			    " =: ] ";
		    } else {
			// a true range, assume order is ok
			tag = "RESTFREQ IN [ " + doubleToString(rmin) +
			    " =:= " + doubleToString(rmax) + " ] ";
		    }
		    if (tag.length() == 0) continue;
		    if (fieldResult.length() > 0) fieldResult = fieldResult + " || ";
		    fieldResult = fieldResult + tag;
		}
		
		if (fieldResult.length() > 0) {
		    fieldResult = " ( " + fieldResult + " ) ";
		    if (nout == 0) result = result + " ( ";
		    else result = result + " && ";
		    result = result + fieldResult;
		    nout ++;
		}
		if (nout > 0) result = result + " ) ";
	    }
	}
    }

    return True;
}

Bool SDTableIterator::parseRecord(const Record& selection, 
				  const SimpleOrderedMap<String, String>& map, 
				  const TableDesc& layout, String& result)
{
    LogIO os(LogOrigin("SDTableIterator","parseRecord"));
    uInt nfields = selection.nfields();
    String tag;
    Int nout = 0;
    result = "";

    for (uInt i=0;i<nfields;i++) {
	String fieldResult;
	// first do some sanity checks on the column
	String tag;
	String tableName("");
	// is the field name in the map, if so, just use that
	// ignore these special fields
	String selName(selection.name(i));
	if (selName == "ut" || selName == "date") continue;
	else if (map.isDefined(selName)) {
	    tableName = map(selName);
	} else if (map.ndefined() == 0) {
	    // in this case, this is the ns map and just upcase the field name
	    tableName = selName;
	    tableName.upcase();
	} else {
	    // its not there, emit a warning and go on
	    os << LogIO::WARN
	       << "field " << selName 
	       << " can not currently be selected on - "
	       << "this field will be ignored"
	       << LogIO::POST;
	    continue;
	}
	// special characters in tableName need to be escaped
	String queryName = replaceSpecial(tableName);

	// SCAN is a special case
	if (tableName == "SCAN") queryName = "FLOOR(SCAN)";

	// SUBSCAN is a special case
	if (tableName == "SUBSCAN" && subscan_.isAttached()) {
	    tableName = "SCAN";
	    queryName = "ROUND((SCAN - FLOOR(SCAN))*100.0)";
	}

	// if not in layout, move on
	if (!layout.isColumn(tableName)) continue;

	// if an Array column, warn and move on
	if (layout.columnDesc(tableName).isArray()) {
	    os << LogIO::WARN
	       << "The selection field " << selName << " is an array field."
	       << "\nArray fields can not currently be selected on."
	       << "\nThis field will be ignored."
	       << LogIO::POST;
	    continue;
	}

	if (layout.columnDesc(tableName).dataType() == TpString) {
	    // a string column
	    Vector<String> list;
	    if (selection.type(i) == TpString) {
		// single value
		list.resize(1);
		selection.get(i, list(0));
	    } else if (selection.type(i) == TpArrayString) {
		// vector of values
		selection.get(i, list);
	    } else {
		os << LogIO::WARN
		   << "Selections must be string vectors only for "
		   << "fields which contain strings."
		   << "\nSelection field=" << selName
		   << " violates this rule."
		   << "\nThis field will be ignored."
		   << LogIO::POST;
		continue;
	    }
	    // assemble these discrete values into a selection for this column
	    // veldef is a special case
	    if (queryName == "VELDEF") {
		// only two types are supported here, everything else is just ignored
		// RADIO -> FREQ* or VELO*
		// OPTICAL -> FELO*
		// this rather complex pattern is because we don't know which axis in any
		// given row is actually the frequency-like axis.
		// assemble these discrete values into a selection for this column
		// these will be patterns so treat them as such
		String ctypeResult;
		Int ctout = 0;
		for (uInt j=0;j<list.nelements();j++) {
		    if (list(j).length() != 0) {
			if (list(j) == MDoppler::showType(MDoppler::RADIO)) {
			    // match radio axes
			    if (ctout > 0) ctypeResult = ctypeResult + " || ";
			    ctout++;
			    // __AXISTOK__ is replaced later
			    ctypeResult = ctypeResult + " __AXISTOK__ == REGEX('(^FREQ|^VELO).*')";
			} else if (list(j) == MDoppler::showType(MDoppler::OPTICAL)) {
			    // match optical axes
			    if (ctout > 0) ctypeResult = ctypeResult + " || ";
			    ctout++;
			    // __AXISTOK__ is replaced later
			    ctypeResult = ctypeResult + " __AXISTOK__ == REGEX('^FELO.*')";
			}
		    }
		    if (ctout > 0) {
			// axes may not yet exist.  However, it is safe to make them here if necessary.
			if (!axes_) {
			    axes_ = new SDIterAxes(*tab_);
			    AlwaysAssert(axes_, AipsError);
			}
			// the 3rd argument here is not important in this context
			ctypeResult = makeFullAxisSel(ctypeResult, axes_->ctypeColNames(), axes_->ctypeColNames());
			if (!ctypeResult.empty()) {
			    // finally add it to the bigger picture
			    fieldResult = fieldResult + ctypeResult;
			}
		    }
		}
	    } else {
		// these may be patterns so treat them as such
		fieldResult = fieldResult + parseList(list, queryName);
	    }
	} else {
	    // a scalar column of some other type
	    if (selection.type(i) != TpArrayDouble) {
		os << LogIO::WARN
		   << "Selections must be a matrix of doubles for "
		   << "fields which contain non-string values.\n"
		   << "Selection field=" << selName
		   << " violates this rule."
		   << "\nThis field will be ignored."
		   << LogIO::POST;
		continue;
	    }
	    Matrix<Double> ranges;
	    // is the input glish Record a matrix
	    if (selection.shape(i).nelements() == 2) {
		selection.get(i, ranges);
	    } else if (selection.shape(i).nelements() == 1 &&
		       selection.shape(i)(0) % 2 == 0) {
		// if this is a vector with an even number of elements then
		// assume its a 2 x nranges Matrix
		Vector<Double> vranges;
		selection.get(i, vranges);
		IPosition mshape(2);
		mshape(0) = 2;
		mshape(1) = vranges.nelements()/2;
		ranges = vranges.reform(mshape);
	    }
	    if (ranges.nrow() != 2) {
		os << LogIO::WARN
		   << "The shape of the selection Matrix must be (2,nranges)\n"
		   << "Selection field=" << selName
		   << " violates this rule."
		   << "\nThis field will be ignored."
		   << LogIO::POST;
		continue;
	    }
	    fieldResult = fieldResult + parseRanges(ranges, queryName);
	}
	if (fieldResult.length() > 0) {
	    fieldResult = " ( " + fieldResult + " ) ";
	    if (nout == 0) result = result + " ( ";
	    else result = result + " && ";
	    result = result + fieldResult;
	    nout ++;
	}
    }
    if (nout > 0) result = result + " ) ";
    return True;
}

String SDTableIterator::parseSelection(const Record& selection, const TableDesc& layout,
				       Vector<uInt> &specialRows, Bool &hasSpecial)
{
    LogIO os(LogOrigin("SDTableIterator","parseSelection(const Record& selection, const TableDesc &layout)"));
    String result = "";
    Int nout = 0;

    // watch for the empty record
    if (selection.nfields() == 1 && selection.name(0) == "empty") return result;

    Bool hasHeaderField, hasSDFITSHeaderField, hasDescField;
    hasHeaderField=hasSDFITSHeaderField=hasDescField=False;

    for (uInt i=0;i<selection.nfields();i++) {
	// we've already handled the row selection
	if (selection.name(i) != "row") {
	    if (selection.name(i) == "header") {
		if (selection.dataType(i) == TpRecord) {
		    hasHeaderField = True;
		} else {
		    os << LogIO::WARN
		       << "The header selection field must be a record"
		       << "\nThis field will be ignored."
		       << LogIO::POST;
		}
	    } else if(selection.name(i) == "other") {
		if (selection.dataType(i) == TpRecord) {
		    // only deal with sdfits subrecords here
		    const Record otherRecord(selection.subRecord(i));
		    for (uInt j=0;j<otherRecord.nfields();j++) {
			if (otherRecord.name(j) == "sdfits") {
			    if (otherRecord.dataType(j) == TpRecord) {
				hasSDFITSHeaderField=True;
			    } else {
				os << LogIO::WARN
				   << "The sdfits record selection field must be a record"
				   << "\nThis field will be ignored."
				   << LogIO::POST;
			    }
			} else {
			    os << LogIO::WARN
			       << "Selection on field=other." << otherRecord.name(j)
			       << " is not supported when the iterator is SDFITS-based."
			       << "\nThis field will be ignored."
			       << LogIO::POST;
			}
		    }
		} else {
		    os << LogIO::WARN
		       << "The other selection field must be a record"
		       << "\nThis field will be ignored."
		       << LogIO::POST;
		}
	    } else if (selection.name(i) == "data") {
		if (selection.dataType(i) == TpRecord) {
		    const Record dataRecord(selection.subRecord(i));
		    for (uInt j=0;j<dataRecord.nfields();j++) {
			if (dataRecord.name(j) == "desc") {
			    if (dataRecord.dataType(j) == TpRecord) {
				hasDescField=True;
			    } else {
				os << LogIO::WARN
				   << "The data.desc selection field must be a record"
				   << "\nThis field will be ignored."
				   << LogIO::POST;
			    }
			} else {
			    os << LogIO::WARN
			       << "Selection on field=data." << dataRecord.name(j) << " is not suported."
			       << "\nThis field will be ignored."
			       << LogIO::POST;
			}
		    }
		} else {
		    os << LogIO::WARN
		       << "The data selection field must be a record"
		       << "\nThis field will be ignored."
		       << LogIO::POST;
		}
	    } else {
		os << LogIO::WARN
		   << "Selection on field=" << selection.name(i) << " is not suported."
		   << "\nThis field will be ignored."
		   << LogIO::POST;
	    }
	}
    }

    if (hasHeaderField) {
	String hresult = "";
	if (!parseRecord(selection.subRecord("header"), itHeaderMap_, layout, hresult)) {
	    throw(AipsError(result));
	}
	if (hresult.length() > 0) {
	    // nout must be 0 here
	    result = " ( " + hresult;
	    nout++;
	}
	hasSpecial = parseSpecialFields(selection.subRecord("header"), specialRows);
    }
    if (hasSDFITSHeaderField) {
	String nsresult = "";
	if (!parseRecord(selection.subRecord("other").subRecord("sdfits"), emptyMap_, layout, nsresult)) {
	    throw(AipsError(nsresult));
	}
	if (nsresult.length() > 0) {
	    if (nout == 0) result = " ( ";
	    else result = result + " && ";
	    result = result + nsresult;
	    nout++;
	}
    }
    if (hasDescField) {
	String descresult = "";
	if (!parseDescRecord(selection.subRecord("data").subRecord("desc"), descresult)) {
	    throw(AipsError(descresult));
	}
	if (descresult.length() > 0) {
	    if (nout == 0) result = " ( ";
	    else result = result + " && ";
	    result = result + descresult;
	    nout++;
	}
    }
    if (nout != 0) result = result + " ) ";
    return result;
}

Vector<uInt> SDTableIterator::parseRow(const Record& selection, Int maxRow)
{
    // row is already known to exist as a field in rowValue
    // any error in parsing simply cause the returned vector having 0 elements
    Vector<uInt> result;
    IPosition shape(selection.shape("row"));
    // the value must be a matrix, otherwise it is ignored
    if (shape.nelements() == 2 || 
	(shape.nelements() == 1 && shape(0) % 2 == 0)) {
	// make sure its a double, else ignore it
	// treat even element vectors as doubles of shape [2,x]
	Matrix<Double> ranges;
	Matrix<Int> iranges;
	Int rmin, rmax;
	if (selection.dataType("row") == TpArrayDouble) {
	    if (shape.nelements() == 2) {
		selection.get("row",ranges);
	    } else {
		Vector<Double> vranges;
		selection.get("row", vranges);
		IPosition mshape(2);
		mshape(0) = 2;
		mshape(1) = vranges.nelements()/2;
		ranges = vranges.reform(mshape);
	    }
	    iranges.resize(3,ranges.ncolumn());
	    if (ranges.nrow() == 2) {
		// count the number of elements in result we need
		Int nrows = 0;
		uInt nvalidRanges = 0;
		Int len;
		for (uInt i=0;i<ranges.ncolumn();i++) {
		    if (ranges(0,i) <= 0) ranges(0,i) = 1.0;
		    if (ranges(1,i) > double(maxRow)) ranges(1,i) = double(maxRow);
		    rmin = Int(ranges(0,i));
		    rmax = Int(ranges(1,i));
		    if (rmax <= 0 || rmax < rmin) continue;
		    if (rmin > maxRow) continue;
		    len = rmax - rmin + 1;
		    iranges(0,nvalidRanges) = nrows;
		    iranges(1,nvalidRanges) = rmin;
		    iranges(2,nvalidRanges) = len;
 		    nrows += len;
		    nvalidRanges++;
		}
		// now fill it in
		result.resize(nrows);
		for (uInt i=0;i<nvalidRanges;i++) {
		    // copy constuctor makes a reference
		    Vector<uInt> sliceOfResult(result(Slice(iranges(0,i), 
							    iranges(2,i))));
		    indgen(sliceOfResult, uInt(iranges(1,i)));
		}
	    }
	}
    }
    // make result 0-relative
    result = result - uInt(1);
    return result;
}
		
void SDTableIterator::initStatics()
{
    if (!staticsOK_) {
	// these are the things which are simple copies
	// and which exist in the standard header
	fitsHeaderMap_.define("TELESCOP","telescope");
	fitsHeaderMap_.define("OBSERVER","observer");
	fitsHeaderMap_.define("PROJID","project");
	fitsHeaderMap_.define("OBJECT","source_name");

	// special columns, delt with on an individual basis
	// but which do not go to the sdfits record
	fitsSpecialMap_.define("CRPIX1",True);
	fitsSpecialMap_.define("CRPIX2",True);
	fitsSpecialMap_.define("CRPIX3",True);
	fitsSpecialMap_.define("CRPIX4",True);
	fitsSpecialMap_.define("CRPIX5",True);
	fitsSpecialMap_.define("CRVAL1",True);
	fitsSpecialMap_.define("CRVAL2",True);
	fitsSpecialMap_.define("CRVAL3",True);
	fitsSpecialMap_.define("CRVAL4",True);
	fitsSpecialMap_.define("CRVAL5",True);
	fitsSpecialMap_.define("CDELT1",True);
	fitsSpecialMap_.define("CDELT2",True);
	fitsSpecialMap_.define("CDELT3",True);
	fitsSpecialMap_.define("CDELT4",True);
	fitsSpecialMap_.define("CDELT5",True);
	fitsSpecialMap_.define("CTYPE1",True);
	fitsSpecialMap_.define("CTYPE2",True);
	fitsSpecialMap_.define("CTYPE3",True);
	fitsSpecialMap_.define("CTYPE4",True);
	fitsSpecialMap_.define("CTYPE5",True);
	fitsSpecialMap_.define("CUNIT1",True);
	fitsSpecialMap_.define("CUNIT2",True);
	fitsSpecialMap_.define("CUNIT3",True);
	fitsSpecialMap_.define("CUNIT4",True);
	fitsSpecialMap_.define("CUNIT5",True);
	fitsSpecialMap_.define("CROTA1",True);
	fitsSpecialMap_.define("CROTA2",True);
	fitsSpecialMap_.define("CROTA3",True);
	fitsSpecialMap_.define("CROTA4",True);
	fitsSpecialMap_.define("CROTA5",True);
	fitsSpecialMap_.define("RADECSYS",True);
	fitsSpecialMap_.define("EQUINOX",True);
	fitsSpecialMap_.define("VELDEF",True);
	fitsSpecialMap_.define("AZIMUTH",True);
	fitsSpecialMap_.define("ELEVATIO",True);
	fitsSpecialMap_.define("SCAN",True);
	fitsSpecialMap_.define("TIME",True);
	fitsSpecialMap_.define("TSYS",True);
	fitsSpecialMap_.define("TCAL",True);
	fitsSpecialMap_.define("TRX",True);
	fitsSpecialMap_.define("SYSCAL_TRX",True);
	fitsSpecialMap_.define("SYSCAL_TSYS",True);
	fitsSpecialMap_.define("SYSCAL_TCAL",True);
	fitsSpecialMap_.define("SIGMA",True);
	fitsSpecialMap_.define("SITELONG",True);
	fitsSpecialMap_.define("SITELAT",True);
	fitsSpecialMap_.define("SITEELEV",True);
	fitsSpecialMap_.define("DATAMIN",True);
	fitsSpecialMap_.define("DATAMAX",True);
	fitsSpecialMap_.define("DATA",True);
	fitsSpecialMap_.define("HISTORY", True);
	fitsSpecialMap_.define("FLAG",True);
	fitsSpecialMap_.define("WEIGHT",True);
	fitsSpecialMap_.define("DURATION",True);
	fitsSpecialMap_.define("EXPOSURE",True);
	fitsSpecialMap_.define("RESTFREQ",True);
	fitsSpecialMap_.define("FREQRES",True);
	fitsSpecialMap_.define("BANDWID",True);
	fitsSpecialMap_.define("BANDWIDTH",True);
	fitsSpecialMap_.define("PRESSURE",True);
	fitsSpecialMap_.define("TAMBIENT",True);
	fitsSpecialMap_.define("WINDDIRE",True);
	fitsSpecialMap_.define("WINDSPEE",True);
	fitsSpecialMap_.define("DEWPOINT",True);
	fitsSpecialMap_.define("DATE-OBS",True);
	fitsSpecialMap_.define("LST",True);
	fitsSpecialMap_.define("MOLECULE",True);
	fitsSpecialMap_.define("TRANSITI",True);
	fitsSpecialMap_.define("REFDIRECTION",True);

	// actually, here I need the reverse mapping of the above
	itHeaderMap_.define("exposure","EXPOSURE");
	// these first two are special cases, but for selection purposes this works for now.
	itHeaderMap_.define("scan_number","SCAN");
	itHeaderMap_.define("veldef","VELDEF");
	itHeaderMap_.define("telescope","TELESCOP");
	itHeaderMap_.define("observer","OBSERVER");
	itHeaderMap_.define("project","PROJID");
	itHeaderMap_.define("source_name","OBJECT");
	itHeaderMap_.define("resolution","FREQRES");
	itHeaderMap_.define("bandwidth","BANDWID");
	itHeaderMap_.define("pressure","PRESSURE");
	itHeaderMap_.define("tambient","TAMBIENT");
	itHeaderMap_.define("wind_dir","WINDDIRE");
	itHeaderMap_.define("wind_speed","WINDSPEE");
	itHeaderMap_.define("dewpoint","DEWPOINT");

	staticsOK_ = True;
    }
}

void SDTableIterator::applySelection(const Record &selection)
{
    // first, if there is a row selection indicated, do that first
    if (selection.isDefined("row")) {
	*tab_ = (*tab_)(parseRow(selection,tab_->nrow()));
    }
    Vector<uInt> specialRows;
    Bool hasSpecial = False;
    String selCmd(parseSelection(selection, tab_->tableDesc(), specialRows, hasSpecial)); 
    if (hasSpecial) {
	*tab_ = (*tab_)(specialRows);
    }
    if (selCmd.length() != 0) {
	selCmd = String("SELECT FROM $1 WHERE ") +  selCmd;
	*tab_ = tableCommand(selCmd, *tab_);
    }
    selection_ = selection;
}

void SDTableIterator::createTable(const String &tableName, 
				  TableLock::LockOption lockoption, 
				  Table::TableOption opt)
{
    // assemble the TableDesc
    // need to also add appropriate units
    // don't forget to also make changes in ensureOutputColumns if columns
    // are added or removed from this list - it would better to share code.
    TableDesc td;
    td.comment() = "SDFITS-like table for use with preliminary SDTableIterator";
    td.addColumn(ArrayColumnDesc<Float>("DATA", "The DATA column"));
    // the SDIterAxes class will take care of the axis information as necessary
    td.addColumn(ScalarColumnDesc<String>("OBJECT"));
    td.addColumn(ScalarColumnDesc<String>("TELESCOP"));
    td.addColumn(ScalarColumnDesc<Double>("BANDWID"));
    td.addColumn(ScalarColumnDesc<Double>("EQUINOX"));
    td.addColumn(ScalarColumnDesc<String>("DATE-OBS"));
    td.addColumn(ScalarColumnDesc<Float>("TSYS"));
    td.addColumn(ScalarColumnDesc<String>("OBSERVER"));
    td.addColumn(ScalarColumnDesc<String>("PROJID"));
    td.addColumn(ScalarColumnDesc<Double>("SCAN"));
    td.addColumn(ScalarColumnDesc<String>("MOLECULE"));
    td.addColumn(ScalarColumnDesc<String>("TRANSITI"));
    td.addColumn(ScalarColumnDesc<Float>("TCAL"));
    td.addColumn(ScalarColumnDesc<Float>("TRX"));
    td.addColumn(ScalarColumnDesc<Double>("FREQRES"));
    td.addColumn(ScalarColumnDesc<String>("VELDEF"));
    td.addColumn(ScalarColumnDesc<Double>("LST"));
    td.addColumn(ScalarColumnDesc<Double>("AZIMUTH"));
    td.addColumn(ScalarColumnDesc<Double>("ELEVATIO"));
    td.addColumn(ScalarColumnDesc<Float>("TAMBIENT"));
    td.addColumn(ScalarColumnDesc<Float>("DEWPOINT"));
    td.addColumn(ScalarColumnDesc<Float>("PRESSURE"));
    td.addColumn(ScalarColumnDesc<Float>("WINDSPEE"));
    td.addColumn(ScalarColumnDesc<Float>("WINDDIRE"));
    td.addColumn(ScalarColumnDesc<Double>("SITELONG"));
    td.addColumn(ScalarColumnDesc<Double>("SITELAT"));
    td.addColumn(ScalarColumnDesc<Double>("SITEELEV"));
    td.addColumn(ScalarColumnDesc<Double>("RESTFREQ"));
    td.addColumn(ArrayColumnDesc<Bool>("FLAG", "The DATA flags"));
    td.addColumn(ArrayColumnDesc<Float>("WEIGHT", "The DATA weights"));
    td.addColumn(ArrayColumnDesc<Float>("SIGMA", "The DATA sigmas"));
    td.addColumn(ScalarColumnDesc<Float>("DURATION"));
    td.addColumn(ScalarColumnDesc<Float>("EXPOSURE"));
    td.addColumn(ArrayColumnDesc<Double>("REFDIRECTION",IPosition(1, 2),
					 ColumnDesc::FixedShape));
    td.addColumn(ArrayColumnDesc<String>("HISTORY", "", 1));
    // these are pretty much always needed - SDFITS needs a true solution
    // to this problem
    td.addColumn(ArrayColumnDesc<Float>("SYSCAL_TSYS"));
    td.addColumn(ArrayColumnDesc<Float>("SYSCAL_TCAL"));
    td.addColumn(ArrayColumnDesc<Float>("SYSCAL_TRX"));


    // Setup a new table from the description
    SetupNewTable newtab(tableName, td, opt);

    // bind everything to StandardStMan
    StandardStMan ssm;
    newtab.bindAll(ssm);

    // create the table
    tab_ = new Table(newtab, lockoption);
}

Bool SDTableIterator::put(const SDRecord& rec)
{
    // remember current row, in case something goes wrong here
    oldTableRow_->get(thisRow_, True);
    Bool result = replaceAll(rec);
    if (!result) oldTableRow_->put();
    return result;
}

Bool SDTableIterator::appendRec(const SDRecord& rec)
{
    Bool result = False;
    LogIO os(LogOrigin("SDTableIterator","append(const SDRecord& rec)"));
    if (!isWritable()) {
	os << LogIO::SEVERE << WHERE
	   << "This iterator is not writable."
	   << LogIO::POST;
    } else {
	uInt oldRow = thisRow_;
	tab_->addRow(1,True);
	thisRow_ = tab_->nrow() - 1;
	result = replaceAll(rec, True);
	if (!result) {
	    // clean up by deleting this row
	    tab_->removeRow(thisRow_);
	    thisRow_ = oldRow;
	    copyAll();
	}
    }
    return result;
}

Bool SDTableIterator::deleteRec()
{
    Bool result = False;
    LogIO os(LogOrigin("SDTableIterator","delete()"));
    if (!isWritable()) {
	os << LogIO::SEVERE << WHERE
	   << "This iterator is not writable."
	   << LogIO::POST;
    } else {
	if (tab_->nrow() > 0) {
	    // there really is a row there, this avoids the error where
	    // the table has no rows, thisRow points to 0
	    tab_->removeRow(thisRow_);
	    if (thisRow_ >= tab_->nrow() && thisRow_ != 0) thisRow_--;
	    copyAll();
	} 
	if (tab_->nrow() == 0) {
	    SDRecord emptyRec;
	    rec_ = emptyRec;
	}
	result = True;
    }
    return result;
}

Bool SDTableIterator::ensureSDFITSTable() {
    // see if tab is ok?
    ok_ = isSDFITSTable(*tab_);
    if (!ok_) {
	// ensure that tab is deleted first, in case tab is actually
	// the log table that this thing might be written to!
	String tabname = tab_->tableName();
	delete tab_;
	tab_ = 0;
	tab_ = new Table;
	LogIO os;
	os << LogOrigin("SDTableIterator","init()") 
	   << LogIO::SEVERE
	   << tabname << " is not a valid single dish FITS table\n"
	   << LogIO::POST;
    }
    return ok_;
}

void SDTableIterator::initStringFields(Record &fieldRec, const Record &modelRec)
{
    RecordDesc modelDesc = modelRec.description();
    for (uInt i=0;i<modelDesc.nfields();i++) {
	if (modelDesc.isSubRecord(i)) {
	    fieldRec.defineRecord(modelDesc.name(i), Record());
	    initStringFields(fieldRec.rwSubRecord(modelDesc.name(i)),
			    modelRec.subRecord(modelDesc.name(i)));
	} else if (modelDesc.type(i) == TpString &&
		   modelDesc.isScalar(i)) {
	    fieldRec.define(modelDesc.name(i), True);
	}
    }
    // and add in the date field IF there is a source_name at this level
    if (fieldRec.fieldNumber("source_name") >= 0) {
	fieldRec.define("date", True);
    }
}

String SDTableIterator::makeFullAxisSel(const String &axisSel, const Vector<String> &ctypeCols,
					const Vector<String> &axisCols)
{
    Bool first = True;
    String result;
    for (uInt i=0;i<axisCols.nelements();i++) {
	if (!axisCols(i).empty()) {
	    if (!first) result = result + " || ";
	    else {
		result = " ( ";
		first = False;
	    }
	    String parsed = axisSel;
	    parsed.gsub("__AXISTOK__", ctypeCols(i));
	    parsed.gsub("__CTOK__", axisCols(i));
	    result = result + parsed;
	}
    }
    if (!first) result = result + " ) ";
    return result;
}

// this returns it zero-based
Double SDTableIterator::getRefPix(const Vector<Double> &chanVals, Double refVal, Double &delta)
{
    Double newDelta, result;

    result = (refVal - chanVals(0)) / delta;

    // find delta at nearest pixel to result
    if (result < 0) {
	newDelta = (chanVals(1) - chanVals(0));
    } else {
	uInt iresult = Int(result + 0.5);
	if (iresult == 0) {
	    newDelta = (chanVals(1) - chanVals(0));
	} else if (iresult >= (chanVals.nelements()-1)) {
	    uInt last = chanVals.nelements()-1;
	    newDelta = chanVals(last) - chanVals(last-1);
	} else {
	    newDelta = (chanVals(iresult+1) - chanVals(iresult-1)) / 2.0;
	}
    }
    if (!near(newDelta, delta)) {
	// iterate
	result = getRefPix(chanVals, refVal, newDelta);
    }
    delta = newDelta;
    return result;
}

Record SDTableIterator::getVectorShortCuts(const Record &recTemplate)
{
    Record result;
    // special fields are: 
    //    header: scan_number, telescope_position, time, source_name, date, ut
    //    data/desc: restfrequency

    // if this is called, then at least one of those must be present
    // construct the vector of rows/per row
    Vector<uInt> rowsPerRow(tab_->nrow(),0u);
    uInt *rprPtr, *tmp;
    uInt totalRows;
    Bool deleteIt;
    rprPtr = rowsPerRow.getStorage(deleteIt);
    tmp = rprPtr;
    totalRows = 0;
    for (uInt i=0;i<rowsPerRow.nelements();i++) {
	*tmp = axes_->nsdrecords(i);
	totalRows += *tmp;
	tmp++;
    }

    Vector<uInt> rows(totalRows);
    uInt *rowPtr;
    Bool deleteItRow;
    rowPtr = rows.getStorage(deleteItRow);
    tmp = rprPtr;
    uInt thisRow = 0;
    for (uInt i=0;i<rowsPerRow.nelements();i++) {
	for (uInt j=0;j<*tmp;j++) {
	    *(rowPtr+thisRow) = i;
	    thisRow++;
	}
	tmp++;
    }
    rows.putStorage(rowPtr, deleteItRow);
    rowsPerRow.putStorage(rprPtr, deleteIt);
    RefRows refRows(rows);
    // header fields
    if (recTemplate.fieldNumber("header") >= 0) {
	Record headerRecord;
	// scan_number
	if (recTemplate.subRecord("header").
	    fieldNumber("scan_number") >= 0) {
	    if (!scan_.isNull()) {
		// watch for a few types
		switch (scan_.columnDesc().dataType()) {
		case TpInt:
		    {
			ROScalarColumn<Int> scans(scan_);
			headerRecord.define("scan_number",
					    scans.getColumnCells(refRows));
		    }
		    break;
		case TpFloat:
		    {
			ROScalarColumn<Float> scans(scan_);
			Vector<Float> fscans(scans.getColumnCells(refRows));
			Vector<Int> iscans(fscans.nelements());
			const Float *fptr, *ftmp;
			Int *iptr, *itmp;
			Bool ideleteIt, fdeleteIt;
			fptr = fscans.getStorage(fdeleteIt);
			iptr = iscans.getStorage(ideleteIt);
			ftmp = fptr;
			itmp = iptr;
			for (uInt row=0;row<fscans.nelements();row++) {
			    *(itmp++) = Int(*(ftmp++)+0.005);
			}
			fscans.freeStorage(fptr, fdeleteIt);
			iscans.putStorage(iptr, ideleteIt);
			headerRecord.define("scan_number", iscans);
		    }
		    break;
		default:
		    {
			Vector<Int> iscans(totalRows);
			Int *iptr, *itmp;
			Bool deleteIt;
			iptr = iscans.getStorage(deleteIt);
			itmp = iptr;
			for (uInt k=0;k<iscans.nelements();k++) {
			    Double scannr = scan_.asdouble(rows(k));
			    *(itmp++) = Int(scannr+0.005);
			}
			iscans.putStorage(iptr, deleteIt);
			headerRecord.define("scan_number", iscans);
		    }
		    break;
		}
	    } else {
		// no scan_number column present
		headerRecord.define("scan_number",
				    Vector<Int>(totalRows, -1));
	    }
	}
	if (recTemplate.subRecord("header").
	    fieldNumber("telescope_position") >= 0) {
	    Matrix<Double> telPos(3, totalRows);
	    Vector<Double> emptyPos(3,0.0);
	    // assumes telescope with the same name doesn't move
	    String lastKnownTelescope = "";
	    Vector<Double> thisTelPos(3,0.0);
	    MPosition obsPos;
	    // SITE keywords
	    if (!siteLong_.isNull() && !siteLat_.isNull() && !siteElev_.isNull()) {
		ROScalarColumn<Double> siteLongCol(*tab_,"SITELONG"),
		    siteLatCol(*tab_,"SITELAT"), siteElevCol(*tab_, "SITEELEV");
		Vector<Double> siteLongs, siteLats, siteElevs;
		siteLongs = siteLongCol.getColumnCells(refRows);
		siteLats = siteLatCol.getColumnCells(refRows);
		siteElevs = siteElevCol.getColumnCells(refRows);
		Double lastLong, lastLat, lastElev;
		lastLong = lastLat = lastElev = 0.0;
		const Double *siteLongPtr, *siteLatPtr, *siteElevPtr;
		const Double *thisLong, *thisLat, *thisElev;
		Bool delLong, delLat, delElev;
		siteLongPtr = siteLongs.getStorage(delLong);
		siteLatPtr = siteLats.getStorage(delLat);
		siteElevPtr = siteElevs.getStorage(delElev);
		thisLong = siteLongPtr;
		thisLat = siteLatPtr;
		thisElev = siteElevPtr;
		Unit meter("m");
		Unit deg("deg");
		for (uInt i=0;i<siteLongs.nelements();i++) {
		    if (lastLong != *thisLong && lastLat != *thisLat && lastElev != *thisElev) {
			lastLong = *thisLong;
			lastLat = *thisLat;
			lastElev = *thisElev;
			obsPos = MPosition(Quantity(lastElev,meter),
					   Quantity(lastLong,deg),
					   Quantity(lastLat,deg),
					   MPosition::WGS84);
			thisTelPos = obsPos.get(meter).getValue();
		    }
		    telPos.column(i) = thisTelPos;
		    thisLong++;
		    thisLat++;
		    thisElev++;
		}
		siteLongs.freeStorage(siteLongPtr, delLong);
		siteLats.freeStorage(siteLatPtr, delLat);
		siteElevs.freeStorage(siteElevPtr, delElev);
	    } else {
		// telescope name
		ROScalarColumn<String> telCol(*tab_, "TELESCOP");
		Vector<String> telescopes(telCol.getColumnCells(refRows));
		const String *telPtr, *thisTel;
		Bool deleteIt;
		telPtr = telescopes.getStorage(deleteIt);
		thisTel = telPtr;
		Unit meter("m");
		for (uInt i=0;i<telescopes.nelements();i++) {
		    String tel = *(thisTel++);
		    tel = tel.before(RXwhite, tel.length()-1);
		    if (lastKnownTelescope != tel) {
			lastKnownTelescope = tel;
			// NRAO43M == GB
			if (tel == "NRAO43M") tel = "GB";
			if (!MeasTable::Observatory(obsPos, tel)) {
			    // apparently not in the table - give up
			    obsPos = MPosition();
			}
			thisTelPos = obsPos.get(meter).getValue();
		    }
		    telPos.column(i) = thisTelPos;
		}
		telescopes.freeStorage(telPtr, deleteIt);
	    }
	    headerRecord.define("telescope_position",telPos);
	}
	if (recTemplate.subRecord("header").
	    fieldNumber("source_name") >= 0) {
	    // OBJECT must exist
	    ROScalarColumn<String> objCol(*tab_,"OBJECT");
	    headerRecord.define("source_name", objCol.getColumnCells(refRows));
	}
	if (recTemplate.subRecord("header").fieldNumber("time") >= 0 || 
	    recTemplate.subRecord("header").fieldNumber("ut") >= 0 || 
	    recTemplate.subRecord("header").fieldNumber("date") >= 0) {
	    // get the vector of times
	    MEpoch time;
	    MVTime timeInDays;
	    MVEpoch etime;
	    String timesys;
	    String dateRow;
	    Unit secs("s");
	    Unit days("d");
	    Bool doTime = recTemplate.subRecord("header").fieldNumber("time") >= 0;
	    Bool doUt = recTemplate.subRecord("header").fieldNumber("ut") >= 0;
	    Bool doDate = recTemplate.subRecord("header").fieldNumber("date") >= 0;
	    Vector<Double> times, uts;
	    Vector<String> dates;
	    Double *timePtr, *thisTime, *utPtr, *thisUt;
	    String *datePtr, *thisDate;
	    timePtr = thisTime = utPtr = thisUt = 0;
	    datePtr = thisDate = 0;
	    const uInt *rPerRowPtr, *nrowsThisRow;
	    Bool timeDeleteIt, utDeleteIt, dateDeleteIt, rowDeleteIt;
	    if (doTime) {
		times.resize(totalRows);
		timePtr = times.getStorage(timeDeleteIt);
		thisTime = timePtr;
	    }
	    if (doUt) {
		uts.resize(totalRows);
		utPtr = uts.getStorage(utDeleteIt);
		thisUt = utPtr;
	    }
	    if (doDate) {
		dates.resize(totalRows);
		datePtr = dates.getStorage(dateDeleteIt);
		thisDate = datePtr;
	    }
	    rPerRowPtr = rowsPerRow.getStorage(rowDeleteIt);
	    nrowsThisRow = rPerRowPtr;
	    Double rowTime, rowUt;
	    rowTime = rowUt = 0;
	    String rowDate;
	    for (uInt i=0;i<rowsPerRow.nelements();i++) {
		time = axes_->time(i);
		if (doUt || doDate) etime = time.getValue();
		if (doTime) rowTime = time.get(secs).getValue();
		if (doUt) rowUt = Quantity(etime.getDayFraction(),days).getValue(secs);
		if (doDate) {
		    timeInDays = etime.getDay();
		    FITSDateUtil::toFITS(rowDate, timesys, timeInDays, MEpoch::UTC, FITSDateUtil::NEW_DATEONLY);
		}
		for (uInt k=0;k<*nrowsThisRow;k++){
		    if (doTime) *(thisTime++) = rowTime;
		    if (doUt) *(thisUt++) = rowUt;
		    if (doDate) *(thisDate++) = rowDate;
		}
		nrowsThisRow++;
	    }
	    rowsPerRow.freeStorage(rPerRowPtr, rowDeleteIt);
	    if (doTime) {
		times.putStorage(timePtr, timeDeleteIt);
		headerRecord.define("time",times);
	    }
	    if (doUt) {
		uts.putStorage(utPtr, utDeleteIt);
		headerRecord.define("ut",uts);
	    }
	    if (doDate) {
		dates.putStorage(datePtr, dateDeleteIt);
		headerRecord.define("date",dates);
	    }
	}
	result.defineRecord("header", headerRecord);
    }
    
    if (recTemplate.fieldNumber("data") >= 0 &&
	recTemplate.subRecord("data").fieldNumber("desc") >= 0 &&
	recTemplate.subRecord("data").subRecord("desc").
	fieldNumber("restfrequency") >= 0) {
	// data/desc/restfrequency
	result.defineRecord("data",Record());
	result.rwSubRecord("data").defineRecord("desc", Record());
	if (restFreq_p.isAttached()) {
	    ROScalarColumn<Double> restFreqCol(*tab_,"RESTFREQ");
	    result.rwSubRecord("data").rwSubRecord("desc").
		define("restfrequency",restFreqCol.getColumnCells(refRows));
	} else {
	    // no RESTFREQ column
	    result.rwSubRecord("data").rwSubRecord("desc").
		define("restfrequency",Vector<Double>(totalRows, 0.0));
	}
    }
    return result;
}

Bool SDTableIterator::parseSpecialFields(const Record &selection,
					 Vector<uInt> &specialRows)
{
    LogIO os(LogOrigin("SDTableIterator","parseSpecialFields"));
    Record hvec;
    Bool hasUT, hasDate;
    hasUT = hasDate = False;
    if (selection.fieldNumber("ut") >= 0) {
	hvec.define("ut",0);
	hasUT = True;
    }
    if (selection.fieldNumber("date") >= 0) {
	hvec.define("date","");
	hasDate = True;
    }

    if (hasUT || hasDate) {
	// the only part of this object we can rely on is the existence of tab_
	SDIterAxes axes(*tab_);
	Vector<Double> uts;
	Vector<String> dates;
	getSpecialVectors(axes, uts, dates,
			  axes.nsdrecords(), tab_->nrow(), hasUT, hasDate);
	uInt nels = max(uts.nelements(), dates.nelements());
	if (nels > 0) {
	    Vector<Bool> okRows(nels, True);
	    Bool *okPtr, *okPtr0;
	    Bool deleteIt;
	    okPtr0 = okRows.getStorage(deleteIt);
	    okPtr = okPtr0;
	    if (hasDate) {
		// should be a string column
		Vector<String> list;
		Int dateField = selection.fieldNumber("date");
		if (selection.type(dateField) == TpString) {
		    // single value, once we find it we stop
		    list.resize(1);
		    selection.get(dateField, list(0));
		} else if (selection.type(dateField) == TpArrayString) {
		    // vector of values
		    selection.get(dateField, list);
		} else {
		    os << LogIO::WARN
		       << "Selection on the header.date field must be string vectors only."
		       << "\nThis selection violates that rule and the date field will be ignored here."
		       << LogIO::POST;
		    hasDate = False;
		}
		// make all of the okRows false first
		for (uInt i=0; i<nels; i++) *(okPtr++) = False;
		// now turn them on one by one for each field that matches
		String *datePtr, *datePtr0;
		Bool deleteDatePtr;
		datePtr0 = dates.getStorage(deleteDatePtr);
		for (uInt i=0;i<list.nelements();i++) {
		    // they may be patterns so treat them as such
		    Regex toMatch = Regex::fromPattern(list(i));
		    okPtr = okPtr0;
		    datePtr = datePtr0;
		    for (uInt j=0;j<nels;j++) {
			if (!(*okPtr) && (*datePtr++).matches(toMatch)) {
			    *okPtr = True;
			}
			okPtr++;
		    }
		}
		dates.putStorage(datePtr0, deleteDatePtr);
	    }
	    if (hasUT) {
		// should be an ArrayDouble column
		Matrix<Double> ranges;
		Int utField = selection.fieldNumber("ut");
		if (selection.type(utField) != TpArrayDouble) {
		    os << LogIO::WARN
		       << "Selections must be a matrix of doubles for "
		       << "fields which contain non-string values.\n"
		       << "Selection field=ut violates this rule."
		       << "\nThis field will be ignored."
		       << LogIO::POST;
		    hasUT = False;
		}
		if (selection.shape(utField).nelements() == 2) {
		    selection.get(utField, ranges);
		} else if (selection.shape(utField).nelements() == 1 &&
			   selection.shape(utField)(0) % 2 == 0) {
		    // if this is a vector with an even number of elements then
		    // assume its a 2 x nranges Matrix
		    Vector<Double> vranges;
		    selection.get("ut", vranges);
		    IPosition mshape(2);
		    mshape(0) = 2;
		    mshape(1) = vranges.nelements()/2;
		    ranges = vranges.reform(mshape);
		}
		if (ranges.nrow() > 0 && ranges.nrow() != 2) {
		    os << LogIO::WARN
		       << "The shape of the selection Matrix must be (2,nranges)\n"
		       << "Selection field=ut"
		       << " violates this rule."
		       << "\nThis field will be ignored."
		       << LogIO::POST;
		    hasUT = False;
		}
		Double *utPtr, *utPtr0;
		Bool deleteUtPtr;
		utPtr0 = uts.getStorage(deleteUtPtr);
		for (uInt j=0;j<ranges.ncolumn();j++) {
		    okPtr = okPtr0;
		    utPtr = utPtr0;
		    Double rmin, rmax;
		    rmin = ranges(0,j);
		    rmax = ranges(1,j);
		    if (rmin == -C::dbl_max && rmax == C::dbl_max) {
			// entire range is specified, no selection necessary
			continue;
		    } else {
			for (uInt k=0;k<nels;k++) {
			    // if its already False, no need to recheck it
			    Double val = *utPtr;
			    *okPtr = *okPtr && val >= rmin && val <= rmax;
			    okPtr++;
			    utPtr++;
			}
		    }
		}
		uts.putStorage(utPtr0, deleteUtPtr);
	    }
	    // count how many are still True
	    uInt nTrue = 0;
	    okPtr = okPtr0;
	    for (uInt i=0;i<nels;i++) {
		if (*okPtr++) nTrue++;
	    }
	    // resize output array
	    specialRows.resize(nTrue);
	    nTrue = 0;
	    okPtr = okPtr0;
	    for (uInt i=0;i<nels;i++) {
		if (*okPtr++) specialRows(nTrue++) = i;
	    }
	    okRows.putStorage(okPtr0, deleteIt);
	}
    }
    return (hasDate || hasUT);
}

void SDTableIterator::getSpecialVectors(SDIterAxes &axes, Vector<Double> &uts, 
					Vector<String> &dates, uInt nrecs, uInt nrow,
					Bool doUT, Bool doDate)
{
    String timesys;
    String dateRow;
    Double utRow;
    MEpoch time;
    MVTime days;
    MVEpoch etime;
    Double *utPtr0, *utPtr;
    utPtr0 = 0;
    String *datePtr0, *datePtr;
    datePtr0 = 0;
    Bool deleteItDate, deleteItUt;
    deleteItDate = deleteItUt = False;
    utRow = 0.0;
    if (doDate) {
	dates.resize(nrecs);
	datePtr0 = dates.getStorage(deleteItDate);
    }
    if (doUT) {
	uts.resize(nrecs);
	utPtr0 = uts.getStorage(deleteItUt);
    }
    datePtr = datePtr0;
    utPtr = utPtr0;
    for (uInt i=0;i<nrow;i++) {
	time = axes.time(i);
	etime = time.getValue();
	if (doUT) {
	    utRow = Quantity(etime.getDayFraction(),"d").getValue("s");
	}
	if (doDate) {
	    days = etime.getDay();
	    FITSDateUtil::toFITS(dateRow, timesys, days, MEpoch::UTC, FITSDateUtil::NEW_DATEONLY);
	}
	for (uInt j=0;j<axes.nsdrecords(i);j++) {
	    if (datePtr) *(datePtr++) = dateRow;
	    if (utPtr) *(utPtr++) = utRow;
	}
    }
    if (doUT) dates.putStorage(datePtr0, deleteItDate);
    if (doDate) uts.putStorage(utPtr0, deleteItUt);
}

void SDTableIterator::reselect()
{
    if (ensureSDFITSTable()) {
	applySelection(selection_);
	if (ensureSDFITSTable()) {
	    init();
	}
    }
}

void SDTableIterator::ensureOutputColumns()
{
    if (!hasOutputCols_ && isWritable()) {
	// add in any missing fields from the header not in the table
	// assume that the CORE keywords already exist
	// BANDWID, DATE-OBS, EXPOSURE, OBJECT, TELESCOP, TSYS 
	// Also assume that DATA and its coordinate system exists.
	// HISTORY will also have been added at some point elsewhere.
	TableDesc td;
	TableDesc itsTd(tab_->tableDesc());
	if (!itsTd.isColumn("EQUINOX"))
	    td.addColumn(ScalarColumnDesc<Double>("EQUINOX"));
	if (!itsTd.isColumn("OBSERVER"))
	    td.addColumn(ScalarColumnDesc<String>("OBSERVER"));
	if (!itsTd.isColumn("PROJID"))
	    td.addColumn(ScalarColumnDesc<String>("PROJID"));
	if (!itsTd.isColumn("SCAN"))
	    td.addColumn(ScalarColumnDesc<Double>("SCAN"));
	if (!itsTd.isColumn("MOLECULE"))
	    td.addColumn(ScalarColumnDesc<String>("MOLECULE"));
	if (!itsTd.isColumn("TRANSITI"))
	    td.addColumn(ScalarColumnDesc<String>("TRANSITI"));
	if (!itsTd.isColumn("TCAL"))
	    td.addColumn(ScalarColumnDesc<Float>("TCAL"));
	if (!itsTd.isColumn("TRX"))
	    td.addColumn(ScalarColumnDesc<Float>("TRX"));
	if (!itsTd.isColumn("FREQRES"))
	    td.addColumn(ScalarColumnDesc<Double>("FREQRES"));
	if (!itsTd.isColumn("VELDEF"))
	    td.addColumn(ScalarColumnDesc<String>("VELDEF"));
	if (!itsTd.isColumn("LST"))
	    td.addColumn(ScalarColumnDesc<Double>("LST"));
	if (!itsTd.isColumn("AZIMUTH"))
	    td.addColumn(ScalarColumnDesc<Double>("AZIMUTH"));
	if (!itsTd.isColumn("ELEVATIO"))
	    td.addColumn(ScalarColumnDesc<Double>("ELEVATIO"));
	if (!itsTd.isColumn("TAMBIENT"))
	    td.addColumn(ScalarColumnDesc<Float>("TAMBIENT"));
	if (!itsTd.isColumn("DEWPOINT"))
	    td.addColumn(ScalarColumnDesc<Float>("DEWPOINT"));
	if (!itsTd.isColumn("PRESSURE"))
	    td.addColumn(ScalarColumnDesc<Float>("PRESSURE"));
	if (!itsTd.isColumn("WINDSPEE"))
	    td.addColumn(ScalarColumnDesc<Float>("WINDSPEE"));
	if (!itsTd.isColumn("WINDDIRE"))
	    td.addColumn(ScalarColumnDesc<Float>("WINDDIRE"));
	if (!itsTd.isColumn("SITELONG"))
	    td.addColumn(ScalarColumnDesc<Double>("SITELONG"));
	if (!itsTd.isColumn("SITELAT"))
	    td.addColumn(ScalarColumnDesc<Double>("SITELAT"));
	if (!itsTd.isColumn("SITEELEV"))
	    td.addColumn(ScalarColumnDesc<Double>("SITEELEV"));
	if (!itsTd.isColumn("RESTFREQ"))
	    td.addColumn(ScalarColumnDesc<Double>("RESTFREQ"));
	if (!itsTd.isColumn("FLAG"))
	    td.addColumn(ArrayColumnDesc<Bool>("FLAG", "The DATA flags"));
	if (!itsTd.isColumn("WEIGHT"))
	    td.addColumn(ArrayColumnDesc<Float>("WEIGHT", "The DATA weights"));
	if (!itsTd.isColumn("SIGMA"))
	    td.addColumn(ArrayColumnDesc<Float>("SIGMA", "The DATA sigmas"));
	if (!itsTd.isColumn("DURATION"))
	    td.addColumn(ScalarColumnDesc<Float>("DURATION"));
	if (!itsTd.isColumn("EXPOSURE"))
	    td.addColumn(ScalarColumnDesc<Float>("EXPOSURE"));
	if (!itsTd.isColumn("REFDIRECTION"))
	    td.addColumn(ArrayColumnDesc<Double>("REFDIRECTION",IPosition(1, 2),
						 ColumnDesc::FixedShape));
	if (!itsTd.isColumn("SYSCAL_TSYS"))
	    td.addColumn(ArrayColumnDesc<Float>("SYSCAL_TSYS"));
	if (!itsTd.isColumn("SYSCAL_TRX"))
	    td.addColumn(ArrayColumnDesc<Float>("SYSCAL_TRX"));
	if (!itsTd.isColumn("SYSCAL_TCAL"))
	    td.addColumn(ArrayColumnDesc<Float>("SYSCAL_TCAL"));

	// were any columns actually added
	// only the types actually possible here are looked for - i.e. this isn't general
	if (td.ncolumn() > 0) {
	    for (uInt i=0;i<td.ncolumn();i++) {
		tab_->addColumn(td[i]);
	    }
	    // now fill them with the default values
	    Vector<String> newCols(td.columnNames());
	    for (uInt i=0;i<newCols.nelements();i++) {
		TableColumn tcol(*tab_, newCols(i));
		switch (tcol.columnDesc().trueDataType()) {
		case TpFloat:
		    { 
			Float fnan;
			setNaN(fnan);
			ScalarColumn<Float> scol(tcol);
			scol.fillColumn(fnan); 
		    }
		    break;
		case TpDouble:
		    { 
			Double dnan;
			setNaN(dnan);
			ScalarColumn<Double> scol(tcol);
			scol.fillColumn(dnan); 
		    }
		    break;
		case TpString:
		    { 
			ScalarColumn<String> scol(tcol);
			scol.fillColumn(String("")); 
		    }
		    break;
		default:
		    // do nothing
		    break;
		}
	    }
	    if (td.isColumn("REFDIRECTION")) {
		// new REFDIRECTION column, fixed shape, can do in one pass
		Vector<Double> emptyDir(2);
		emptyDir = 0.0;
		ArrayColumn<Double> refdir(*tab_, "REFDIRECTION");
		refdir.fillColumn(emptyDir);
	    }
	    // do these array columns by hand - need to resize, do each cell one at a time
	    ArrayColumn<Bool> flagCol;
	    ArrayColumn<Float> sigmaCol;
	    ArrayColumn<Float> weightCol;
	    // attach these as appropriate
	    if (td.isColumn("FLAG")) flagCol.attach(*tab_,"FLAG");
	    if (td.isColumn("SIGMA")) sigmaCol.attach(*tab_,"SIGMA");
	    if (td.isColumn("WEIGHT")) weightCol.attach(*tab_, "WEIGHT");
	    if (!flagCol.isNull() || !sigmaCol.isNull() || !weightCol.isNull()) {
		// I think this is most efficient - use each size once, make multiple
		// passes through the table
		Vector<Bool> finishedRow(tab_->nrow(), False);
		Array<Bool> flagTmp;
		Array<Float> fltTmp;
		Bool moreToDo = True;
		Bool shapeSet;
		IPosition shape;
		while (moreToDo) {
		    shapeSet = False;
		    moreToDo = False;
		    for (uInt i=0;i<finishedRow.nelements();i++) {
			// skip rows which are do not yet have defined DATA values
			if (!finishedRow(i) && data_in_.isDefined(i)) {
			    IPosition rowShape = data_in_.shape(i);
			    if (!shapeSet) {
				if (!flagCol.isNull()) {
				    flagTmp.resize(rowShape);
				    flagTmp = False;
				}
				if (!sigmaCol.isNull() || !weightCol.isNull()) {
				    fltTmp.resize(rowShape);
				    // default value for weight and sigma is 1.0
				    fltTmp = 1.0;
				}
				shape = rowShape;
				shapeSet = True;
			    }
			    if (rowShape.isEqual(shape)) {
				finishedRow(i) = True;
				if (!flagCol.isNull()) {
				    flagCol.put(i, flagTmp);
				}
				if (!sigmaCol.isNull()) {
				    sigmaCol.put(i, fltTmp);
				}
				if (!weightCol.isNull()) {
				    weightCol.put(i, fltTmp);
				}
			    } else {
				moreToDo = True;
			    }
			}
		    }
		}
	    }
	    // I think its just best to cleanup and re-init at this point
	    // just to be sure
	    cleanupMost();
	    init();
	}
	hasOutputCols_ = True;
    }
}

void SDTableIterator::setRowShape()
{
    // determine the shape of the data in this row
    IPosition shape(axes_->shape(thisRow_));
    // the frequency axis is required
    Int freqAxis = axes_->freqAxis(thisRow_);
    // the stokes axis is optional
    Int stokesAxis = axes_->stokesAxis(thisRow_);

    IPosition arrShape(2);
    if (stokesAxis >= 0) arrShape(0) = shape(stokesAxis);
    else arrShape(0) = 1;
    arrShape(1) = shape(freqAxis);

    // and resize the record
    rec_.resize(arrShape);
}

const Array<String> SDTableIterator::getHist()
{
    Array<String> hist;
    rec_.get("hist", hist);
    return hist;
}

void SDTableIterator::deleteCopiers()
{
    delete headerCopier_;
    headerCopier_ = 0;
    delete rwHeaderCopier_;
    rwHeaderCopier_ = 0;
    for (uInt i=0;i<otherCopiers_.nelements();i++) {
	if (otherCopiers_[i]) {
	    delete otherCopiers_[i];
	    otherCopiers_[i] = 0;
	}
	if (rwOtherCopiers_[i]) {
	    delete rwOtherCopiers_[i];
	    rwOtherCopiers_[i] = 0;
	}
    }
    delete rwTableRow_;
    delete oldTableRow_;
    delete tableRow_;
    rwTableRow_ = oldTableRow_ = 0;
    tableRow_ = 0;
}

void SDTableIterator::setCopiers()
{
    // assumes things have already been deleted, cleared
    // this also sets the contents of the other records

    // set up a copier for the simple scalar types
    // first pass, all column names likely to be copied somewhere
    Vector<String> allCols(tab_->tableDesc().columnNames());
    Vector<String> usedCols(allCols.nelements());
    // remember which column goes to which other copier
    Vector<String> whichOther(allCols.nelements());

    uInt usedCount = 0;
    Regex sdms("^SDMS{.*}.*");
    for (uInt i=0;i<allCols.nelements();i++) {
	if (fitsHeaderMap_.isDefined(allCols(i))) {
	    usedCols(usedCount++) = allCols(i);
	} else if (!fitsSpecialMap_.isDefined(allCols(i))) {
	    usedCols(usedCount) = allCols(i);
	    // try and determine which other record this goes into
	    // if column name begins matchs SDMS{.*} then it goes into
	    // an "other" record where the name of the other is
	    // SDMS{other}columnname
	    // otherwise this is assumed to be an sdfits column
	    // and goes into the sdfits record
	    String otherName = "sdfits";
	    if (allCols(i).matches(sdms)) {
		otherName = allCols(i).after('{');
		otherName = otherName.before('}');
	    }
	    if (!otherCopiersMap_.isDefined(otherName)) {
		otherCopiersMap_.define(otherName,otherCopiersMap_.ndefined());
	    }
	    whichOther(usedCount) = otherName;
	    usedCount++;
	}
    }
    usedCols.resize(usedCount, True);
    tableRow_ = new ROTableRow(*tab_, usedCols);
    AlwaysAssert(tableRow_, AipsError);

    // completely reset the other records
    for (uInt i=0;i<otherCopiersMap_.ndefined();i++) {
	String thisOther = otherCopiersMap_.getKey(i);
	RecordDesc desc;
	Bool isSDFITS = False;
	if (thisOther == "sdfits") {
	    // add in subscan if appropriate
	    if (tab_->tableDesc().isColumn("SCAN") && 
		tab_->tableDesc().columnDesc("SCAN").dataType() != TpInt) {
		desc.addField("subscan",TpFloat);
	    }
	    isSDFITS = True;
	}
	for (uInt j=0;j<usedCols.nelements();j++) {
	    if (whichOther(j) == thisOther) {
		// this column belongs to this record
		// it should be safe to assume it isn't already in this record
		DataType dtype = tableRow_->record().dataType(usedCols(j));
		String name(usedCols(j));
		if (isSDFITS) {
		    name.downcase();
		} else {
		    name = name.after('}');
		}
		desc.addField(name, dtype);
	    }
	}
	// use this to restructure this record
	rec_.other().defineRecord(thisOther, Record(desc));
    }

    // rec_ has everything its going to have - initialize the stringFields_ record
    initStringFields(stringFields_, rec_);

    // ensure block of other copiers is large enough
    if (otherCopiers_.nelements() < otherCopiersMap_.ndefined()) {
	otherCopiers_.resize(otherCopiersMap_.ndefined());
	rwOtherCopiers_.resize(otherCopiersMap_.ndefined());
	for (uInt i=0;i<otherCopiers_.nelements();i++) {
	    otherCopiers_[i] = 0;
	    rwOtherCopiers_[i] = 0;
	}
    }

    Vector<String> colNames(tableRow_->columnNames());
    Vector<Int> headerFields(colNames.nelements());
    headerFields = -1;
    Block<Vector<Int> > otherFields(otherCopiers_.nelements());
    for (uInt i=0;i<otherFields.nelements();i++) {
	otherFields[i].resize(colNames.nelements());
	otherFields[i] = -1;
    }

    for (uInt i=0;i<colNames.nelements();i++) {
	if (fitsHeaderMap_.isDefined(colNames(i))) {
	    headerFields(i) = rec_.header().fieldNumber(fitsHeaderMap_(colNames(i)));
	} else if (!fitsSpecialMap_.isDefined(colNames(i))) {
	    // we already know what other copier this will belong to
	    String otherName = whichOther(i);
	    Int otherIndex = otherCopiersMap_(otherName);
	    String otherField = colNames(i);
	    if (otherName == "sdfits") {
		otherField.downcase();
	    } else {
		otherField = otherField.after('}');
	    } 
	    (otherFields[otherIndex])(i) = 
		rec_.other().subRecord(otherName).fieldNumber(otherField);
	}

    }

    headerCopier_ = new CopyRecordToRecord(rec_.header(), tableRow_->record(), headerFields);
    AlwaysAssert(headerCopier_, AipsError);
    for (uInt i=0;i<otherCopiers_.nelements();i++) {
	// don't bother unless some of the otherFields for this copier are != -1
	Int thisIndex = otherCopiersMap_.getVal(i);
	if (anyNE(otherFields[thisIndex],-1)) {
	    String thisOther = otherCopiersMap_.getKey(i);
	    otherCopiers_[thisIndex] = new CopyRecordToRecord(rec_.other().rwSubRecord(thisOther),
							      tableRow_->record(), otherFields[thisIndex]);
	    AlwaysAssert(otherCopiers_[thisIndex], AipsError);
	}
    }

    if (isWritable()) {
	// oldTableRow_ has everything - used in replaceAll to guard against
	// failures.
	oldTableRow_ = new TableRow(*tab_);
	AlwaysAssert(oldTableRow_, AipsError);

	// rwTableRow_ should have the same fields as tableRow_
	rwTableRow_ = new TableRow(*tab_,tableRow_->columnNames());
	AlwaysAssert(rwTableRow_, AipsError);

	// reverse sense of headerFields
	Vector<Int> rwHeaderFields(rec_.header().nfields());
	rwHeaderFields = -1;
	for (uInt i=0;i<headerFields.nelements();i++) {
	    if (headerFields(i)>=0) {
		rwHeaderFields(headerFields(i)) = i;
	    }
	}
	rwHeaderCopier_ = new CopyRecordToRecord(rwTableRow_->record(), rec_.header(), rwHeaderFields);
	AlwaysAssert(headerCopier_, AipsError);
	for (uInt i=0;i<otherCopiers_.nelements();i++) {
	    Int thisIndex = otherCopiersMap_.getVal(i);
	    if (otherCopiers_[thisIndex]) {
		String thisOther = otherCopiersMap_.getKey(i);
		Vector<Int> rwOtherFields(rec_.other().rwSubRecord(thisOther).nfields());
		rwOtherFields = -1;
		for (uInt j=0;j<otherFields[thisIndex].nelements();j++) {
		    if (otherFields[thisIndex](j)>=0) {
			rwOtherFields(otherFields[thisIndex](j)) = j;
		    }
		}
		rwOtherCopiers_[thisIndex] = new CopyRecordToRecord(rwTableRow_->record(),
							    rec_.other().rwSubRecord(thisOther),
							    rwOtherFields);
		AlwaysAssert(rwOtherCopiers_[thisIndex], AipsError);
	    }
	}
    }
}

Bool SDTableIterator::copyRecord(Record &outRec, const Record &inRec)
{
    // assume that outRec has already been checked so that
    // anything in inRec is present in outRec
    for (uInt i=0;i<inRec.nfields();i++) {
	switch (inRec.dataType(i)) {
	case TpBool:
	    outRec.define(inRec.name(i), inRec.asBool(i));
	    break;
	case TpUChar:
	    outRec.define(inRec.name(i), inRec.asuChar(i));
	    break;
	case TpShort:
	    outRec.define(inRec.name(i), inRec.asShort(i));
	    break;
	case TpInt:
	    outRec.define(inRec.name(i), inRec.asInt(i));
	    break;
	case TpUInt:
	    outRec.define(inRec.name(i), inRec.asuInt(i));
	    break;
	case TpFloat:
	    outRec.define(inRec.name(i), inRec.asFloat(i));
	    break;
	case TpDouble:
	    outRec.define(inRec.name(i), inRec.asDouble(i));
	    break;
	case TpComplex:
	    outRec.define(inRec.name(i), inRec.asComplex(i));
	    break;
	case TpDComplex:
	    outRec.define(inRec.name(i), inRec.asDComplex(i));
	    break;
	case TpString:
	    outRec.define(inRec.name(i), inRec.asString(i));
	    break;
	case TpArrayBool:
	    outRec.define(inRec.name(i), inRec.asArrayBool(i));
	    break;
	case TpArrayUChar:
	    outRec.define(inRec.name(i), inRec.asArrayuChar(i));
	    break;
	case TpArrayShort:
	    outRec.define(inRec.name(i), inRec.asArrayShort(i));
	    break;
	case TpArrayInt:
	    outRec.define(inRec.name(i), inRec.asArrayInt(i));
	    break;
	case TpArrayUInt:
	    outRec.define(inRec.name(i), inRec.asArrayuInt(i));
	    break;
	case TpArrayFloat:
	    outRec.define(inRec.name(i), inRec.asArrayFloat(i));
	    break;
	case TpArrayDouble:
	    outRec.define(inRec.name(i), inRec.asArrayDouble(i));
	    break;
	case TpArrayComplex:
	    outRec.define(inRec.name(i), inRec.asArrayComplex(i));
	    break;
	case TpArrayDComplex:
	    outRec.define(inRec.name(i), inRec.asArrayDComplex(i));
	    break;
	case TpArrayString:
	    outRec.define(inRec.name(i), inRec.asArrayString(i));
	    break;
	case TpRecord:
	    if (!copyRecord(outRec.rwSubRecord(inRec.name(i)),
			    inRec.subRecord(i))) {
		return False;
	    }
	    break;
	default:
	    LogIO os(LogOrigin("SDTableIterator","replaceAll(const SDRecord& rec)"));
	    os << LogIO::SEVERE << WHERE 
	       << "Field " << inRec.name(i) 
	       << " has an invalid type, " 
	       << Int(inRec.dataType(i))
	       << " -- this should never happen."
	       << LogIO::POST;
	    return False;
	    break;
	}
    }
    return True;
}

} //# NAMESPACE CASA - END

