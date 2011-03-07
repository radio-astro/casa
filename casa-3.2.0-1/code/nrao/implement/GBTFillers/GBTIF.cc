//# GBTIF.cc:  this describes a GBT IF
//# Copyright (C) 2003
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

#include <nrao/GBTFillers/GBTIF.h>

#include <nrao/FITS/GBTLO1File.h>
#include <nrao/GBTFillers/GBTCorrelation.h>
#include <nrao/GBTFillers/GBTIFFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicMath/Math.h>
#include <casa/Quanta/QLogical.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

GBTIF::GBTIF(uInt nchan, uInt nstate)
    : itsNchan(nchan), itsNstate(nstate), itsRefFreq(nchan/2.0),
      itsRefChan(nchan/2.0), itsDeltaFreq(1.0), itsTol(10.0), itsVsource(0.0),
      itsRefFrame(MFrequency::TOPO), itsDopType(MDoppler::RADIO),
      itsOffsets(nstate, 0.0), itsSpwId(nstate, -1), itsIFrows(0),
      itsNcorr(0), itsCorrs(1, 0)
{
    if (nchan == 1) itsRefChan = 0.0;
    itsCorrs[0] = new GBTCorrelation(nstate, nchan);
    AlwaysAssert(itsCorrs[0], AipsError);
}

GBTIF::GBTIF(uInt nchan, uInt nstate, const String &bank, Int port,
	     Double ifFreq, Double bw, Bool increases,
	     const GBTIFFiller &iffiller,
	     const GBTLO1File &lo1aFile, const GBTLO1File &lo1bFile,
	     Bool continuum)
    : itsNchan(nchan), itsNstate(nstate), itsRefFreq(nchan/2.0),
      itsRefChan(nchan/2.0), itsDeltaFreq(1.0), itsTol(10.0), itsVsource(0.0),
      itsRefFrame(MFrequency::TOPO), itsDopType(MDoppler::RADIO),
      itsOffsets(nstate, 0.0), itsSpwId(nstate, -1), itsIFrows(0),
      itsNcorr(0), itsCorrs(0)
{
    if (nchan == 1) {
	itsRefChan = 0.0;
	itsRefFreq = 0.0;
    }
    // does this bank,port have a row in iffiller?
    Int ifRow = iffiller.isValid() ? iffiller.whichRow(bank, port) : -1;
    if (ifRow >= 0) {
	// does the correct LO1A file exist here?
	String lo1Name;
	if (!iffiller.loCircuit().isNull()) {
	    lo1Name = iffiller.loCircuit()(ifRow);
	}
	Vector<Double> lo1Offsets;
	Double frameFactor, vsource, tol;
	frameFactor = vsource = tol = 0;
	MFrequency::Types refFrame = MFrequency::TOPO;
	MDoppler::Types dopType = MDoppler::RADIO;
	MVFrequency restfrq;
	Bool hasLO1 = False;
	if (lo1Name == "LO1B") {
	    if (lo1bFile.isAttached()) {
		hasLO1 = True;
		restfrq = lo1bFile.restfrq();
		tol = lo1bFile.reqdptol();
		lo1Offsets = lo1bFile.firstLO1(frameFactor, refFrame,
					       dopType, vsource);
	    }
	} else {
	    // assume A
	    if (lo1aFile.isAttached()) {
		hasLO1 = True;
		restfrq = lo1aFile.restfrq();
		tol = lo1aFile.reqdptol();
		lo1Offsets = lo1aFile.firstLO1(frameFactor, refFrame,
					       dopType, vsource);
	    }
	}
	if (hasLO1) {
	    itsTol = tol;
	    if (!continuum) {
		itsVsource = vsource;
		itsRefFrame = refFrame;
		itsDopType = dopType;
		itsRestFreq = restfrq;
	    }
	    if (lo1Offsets.nelements() != itsOffsets.nelements()) {
		// should do something more clever hear.  Need
		// some way to identify which states are really
		// in play.  For now, set offsets to value of
		// first field in lo1Offsets - essentially this assumes
		// that there is no frequency switching going on.
		itsOffsets = lo1Offsets[0];
	    } else {
		itsOffsets = lo1Offsets;
	    }
	    Double initialLO1 = itsOffsets[0];
	    itsOffsets -= initialLO1;
	    // use SFF to get observed frequency at the indicated IF
	    // which always corresponds to channel 0 here
	    if (bw <= 0.0) { 
		itsDeltaFreq = iffiller.bandwidth()(ifRow)/itsNchan;
	    } else {
		itsDeltaFreq = bw/itsNchan;
	    }
	    // this should get the sign correct
	    itsDeltaFreq *= iffiller.sffSideband()(ifRow);
	    // correct for !increases
	    if (!increases) itsDeltaFreq = -itsDeltaFreq;
	    Double refChan = 0.0;
	    if (ifFreq < 0.0) {
		ifFreq = iffiller.centerIF()(ifRow);
		// if this happens, centerIF is at the center of the band
		if (nchan > 1) {
		    refChan = nchan/2.0;
		 }
	    }
	    itsRefFreq = iffiller.sffSideband()(ifRow)*ifFreq +
		iffiller.sffMultiplier()(ifRow)*initialLO1 + 
		iffiller.sffOffset()(ifRow);
	    // multiplying by frameFactor shifts things to the
	    // desired reference frame.  This is valid at
	    // the start of the scan and should be valid
	    // to within this tolerance IF doppler tracking
	    // is really taking place.
	    // delta freq, the frequency shift offsets also need to be
	    // similarly scaled
	    itsRefFreq *= frameFactor;
	    itsDeltaFreq *= frameFactor;
	    itsOffsets *= frameFactor;
	    // shift from refChan to itsRefChan
	    itsRefFreq -= itsDeltaFreq*(refChan-itsRefChan);
	    if (isNaN(itsRefFreq) || isNaN(itsDeltaFreq)) {
		// the row was just plain bad - reset to defaults
		itsDeltaFreq = 1.0;
		itsRefFreq = itsRefChan;
		itsOffsets = 0.0;
	    }
	}
	itsIFrows.resize(1);
	itsIFrows[0] = ifRow;
    } else {
	itsIFrows.resize(1);
	itsIFrows[0] = -1;
    }
}

GBTIF::GBTIF(const GBTIF &other)
    : itsNchan(other.itsNchan), itsNstate(other.itsNstate), 
      itsRefFreq(other.itsRefFreq), itsRefChan(other.itsRefChan),
      itsDeltaFreq(other.itsDeltaFreq), itsTol(other.itsTol), 
      itsVsource(other.itsVsource), itsRefFrame(other.itsRefFrame),
      itsRestFreq(other.itsRestFreq),
      itsDopType(other.itsDopType), itsOffsets(other.itsOffsets.nelements()), 
      itsSpwId(other.itsSpwId.nelements()),
      itsIFrows(other.itsIFrows.nelements()), itsNcorr(other.itsNcorr), 
      itsCorrs(other.itsCorrs.nelements())
{
    itsOffsets = other.itsOffsets;
    itsSpwId = other.itsSpwId;
    itsIFrows = other.itsIFrows;
    for (uInt i=0;i<itsNcorr;i++) {
	itsCorrs[i] = new GBTCorrelation(*(other.itsCorrs[i]));
	AlwaysAssert(itsCorrs[i], AipsError);
    }
}

GBTIF::~GBTIF()
{ clearCorrs();}

GBTIF &GBTIF::operator=(const GBTIF &other)
{
    if (this != &other) {
	clearCorrs();
	itsNchan = other.itsNchan;
	itsNstate = other.itsNstate;
	itsRefFreq = other.itsRefFreq; 
	itsRefChan = other.itsRefChan;
	itsDeltaFreq = other.itsDeltaFreq; 
	itsTol = other.itsTol; 
	itsVsource = other.itsVsource; 
	itsRefFrame = other.itsRefFrame;
	itsRestFreq = other.itsRestFreq;
	itsDopType = other.itsDopType; 
	itsOffsets.resize(other.itsOffsets.nelements());
	itsOffsets = other.itsOffsets;
	itsSpwId.resize(other.itsSpwId.nelements());
	itsSpwId = other.itsSpwId;
	itsIFrows.resize(other.itsIFrows.nelements());
	itsIFrows = other.itsIFrows;
	itsNcorr = other.itsNcorr; 
	itsCorrs.resize(other.itsCorrs.nelements(), True, False);
	itsCorrs = 0;
	for (uInt i=0;i<itsNcorr;i++) {
	    itsCorrs[i] = new GBTCorrelation(*(other.itsCorrs[i]));
	    AlwaysAssert(itsCorrs[i], AipsError);
	}
    }
    return *this;
}

Bool GBTIF::operator==(const GBTIF &other) const
{
    Bool result = True;
    if (this != &other) {
	Double tol = max(itsTol, other.itsTol);
	result = itsNchan == other.itsNchan &&
	    itsNstate == other.itsNstate &&
	    itsRefFrame == other.itsRefFrame &&
	    itsRestFreq.nearAbs(other.itsRestFreq,tol) &&
	    itsDopType == other.itsDopType &&
	    allNearAbs(itsOffsets, other.itsOffsets, tol) &&
	    nearAbs(itsRefFreq, other.itsRefFreq,tol) &&
	    nearAbs(itsDeltaFreq, other.itsDeltaFreq,tol) &&
	    near(itsVsource, other.itsVsource);
	// the comparison does not involve either the associated
	// IF rows or GBTCorrelations.
    }
    return result;
}

void GBTIF::addIFRow(Int whichRow) {
    Int n = itsIFrows.nelements();
    itsIFrows.resize(n+1, True);
    itsIFrows[n] = whichRow;
}

void GBTIF::addCorr(const GBTCorrelation &corr)
{
    if (itsNcorr >= itsCorrs.nelements()) {
	uInt oldSize = itsCorrs.nelements();
	itsCorrs.resize(max(uInt(1),oldSize*2));
	for (uInt i=oldSize;i<itsCorrs.nelements();i++) {
	    itsCorrs[i] = 0;
	}
    }
    itsCorrs[itsNcorr] = new GBTCorrelation(corr);
    AlwaysAssert(itsCorrs[itsNcorr], AipsError);
    itsNcorr++;
}

void GBTIF::clearCorrs() 
{
    for (uInt i=0;i<itsNcorr;i++) {
	delete itsCorrs[i];
    }
    itsNcorr = 0;
    itsCorrs = 0;
}

// Needed to avoid namespace conflicts
#include <casa/Utilities/Copy.tcc>
//template void objcopy<GBTIF *>(GBTIF **, GBTIF * const *, uInt);
//template void objset<GBTIF *>(GBTIF **, GBTIF *, uInt);
