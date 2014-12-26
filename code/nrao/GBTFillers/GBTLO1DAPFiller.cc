//# GBTLO1DAPFiller.cc: A GBTDAPFiller for LO1 DAPs
//# Copyright (C) 2000,2001
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

#include <nrao/GBTFillers/GBTLO1DAPFiller.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicMath/Math.h>
#include <tables/TaQL/ExprNode.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

GBTLO1DAPFiller::GBTLO1DAPFiller(const String &device, Table &parent)
    : GBTDAPFiller(device, parent), itsLO1Table(0), itsCachePtr(-1),
      itsCacheStart(-1), itsCacheEnd(-1), itsCacheSize(2048), 
      itsHasPhaseState(False)
{
    AlwaysAssert(GBTDAPFiller::device() == "LO1", AipsError);
}

GBTLO1DAPFiller::~GBTLO1DAPFiller() 
{
    cleanup();
}

void GBTLO1DAPFiller::cleanup()
{
    delete itsLO1Table;
    itsLO1Table = 0;
    // ensure that the columns are de-referenced
    itsTimeColumn.reference(ROScalarColumn<Double>());
    itsFrequencyColumn.reference(ROScalarColumn<Double>());
    itsPhaseColumn.reference(ROScalarColumn<Int>());

    itsCachePtr = itsCacheStart = itsCacheEnd = -1;
    itsTimeCache.resize(0);
    itsFreqCache.resize(0);
    itsPhaseCache.resize(0);
}

void GBTLO1DAPFiller::nextCache()
{
    if (!atCacheEnd()) {
	itsCacheStart = itsCacheEnd + 1;
	itsCacheEnd = min(Int(itsFrequencyColumn.nrow()), (itsCacheEnd+itsCacheSize));
	IPosition start(1,itsCacheStart);
	IPosition end(1,itsCacheEnd);
	Slicer slicer(start,end);
	itsFrequencyColumn.getColumnRange(slicer, itsFreqCache, True);
	itsTimeColumn.getColumnRange(slicer, itsTimeCache, True);
	itsPhaseColumn.getColumnRange(slicer, itsPhaseCache, True);
    }
    itsCachePtr = 0;
}

void GBTLO1DAPFiller::prevCache()
{
    if (!atCacheStart()) {
	// this can't happen unless there has already been a cache read
	itsCacheStart = max(0, Int(itsCacheStart-itsFreqCache.nelements()));
	itsCacheEnd = max(0, Int(itsCacheEnd-itsFreqCache.nelements()));
	IPosition start(1,itsCacheStart);
	IPosition end(1,itsCacheEnd);
	Slicer slicer(start,end);
	itsFrequencyColumn.getColumnRange(slicer, itsFreqCache, True);
	itsTimeColumn.getColumnRange(slicer, itsTimeCache, True);
	itsPhaseColumn.getColumnRange(slicer, itsPhaseCache, True);
    }
    itsCachePtr = 0;
}

Int GBTLO1DAPFiller::prepare(const String &fileName, 
			     const String &manager, const String &sampler,
			     const MVTime &startTime)
{
    Int result = -1;
    if (fileName == "") {
	// this is a signal that there is no LO1 info this round and so
	// we need to clear it
	cleanup();
    } else {
	result = GBTDAPFiller::prepare(fileName, manager, sampler, startTime);
	if (result >= 0 && sampler == "phaseState") {
	    itsHasPhaseState = True;
	}
    }
    return result;
}

void GBTLO1DAPFiller::fill()
{
    // remember where we start from
    uInt startRow = tab().nrow();
    GBTDAPFiller::fill();
    // and where we end at
    uInt endRow = tab().nrow()-1;
    // after filling, do a cleanup
    cleanup();
    // if we have a phaseState, create that selection now
    if (itsHasPhaseState && tab().nrow() > 0 && endRow >= startRow) {
	// first, row number
	Vector<uInt> rows((endRow-startRow)+1);
	indgen(rows,startRow);
	Table rowtab = tab()(rows);
	// and then use the SAMPLER column to get just the phaseState values
	itsLO1Table = new Table(rowtab(rowtab.col("SAMPLER") == "phaseState"));
	AlwaysAssert(itsLO1Table, AipsError);
	// and attach the columns
	itsFrequencyColumn.attach(*itsLO1Table, "frequency");
	// the GBTDAPFiller class behind here will have morphed the DMJD column into
	// a TIME column, where the TIME is in seconds now.
	itsTimeColumn.attach(*itsLO1Table, "TIME");
	itsPhaseColumn.attach(*itsLO1Table, "phase_number");

	// populate the caches
	nextCache();
    }
    // at this point, reset itsHasPhaseState for the next prepare
    itsHasPhaseState = False;
}
	
const Vector<Double> &GBTLO1DAPFiller::frequency(Double time, Double interval, 
						 Int nphases)
{
    // this is insane, there must be an easier way to do this
    itsFreqs.resize(nphases);
    itsFreqs = 0.0;
    
    if (!itsFrequencyColumn.isNull() && itsFrequencyColumn.nrow() > 0) {
	if (itsFrequencyColumn.nrow() == 1) {
	    // single row, frequency has no phase dependence
	    itsFreqs = itsFrequencyColumn.asdouble(0);
	} else {
	    const Double *timePtr, *freqPtr;
	    const Int *phasePtr;
	    Bool dtimePtr, dfreqPtr, dphasePtr;
	    timePtr = itsTimeCache.getStorage(dtimePtr);
	    freqPtr = itsFreqCache.getStorage(dfreqPtr);
	    phasePtr = itsPhaseCache.getStorage(dphasePtr);
	    Double startTime = time - interval/2.0;
	    
	    // find the first time > or near time-interval/2
	    // back up as necessary
	    while (*(timePtr+itsCachePtr) > startTime && !atStart()) {
		itsCachePtr--;
		if (itsCachePtr < 0) prevCache();
	    }
	    
	    while (!(*(timePtr+itsCachePtr) >= startTime || 
		     near(startTime, *(timePtr+itsCachePtr))) && !atEnd()) {
		itsCachePtr++;
		if (itsCachePtr >= Int(itsFreqCache.nelements())) nextCache();
	    }
	    
	    if (*(timePtr+itsCachePtr) < startTime && !near(startTime, *(timePtr+itsCachePtr))) {
		// nothing found, bail out
		return itsFreqs;
	    }
	    
	    Vector<Double> phaseCount(nphases);
	    phaseCount = 0.0;
	    // first one - phases are 1-relative
	    Int phase = *(phasePtr + itsCachePtr)-1;
	    // sanity check
	    if (phase >= Int(itsFreqs.nelements())) {
		itsFreqs = 0.0;
		// need to emit a SEVERE error without throwing an exception
		cout << "Would issue a SEVERE error here" << endl;
		// clean up
		itsTimeCache.freeStorage(timePtr, dtimePtr);
		itsFreqCache.freeStorage(freqPtr, dfreqPtr);
		itsPhaseCache.freeStorage(phasePtr, dphasePtr);
		return itsFreqs;
	    }
	    Int lastPhase = phase;
	    Int firstPhase = phase;
	    Double lastFreq = *(freqPtr+itsCachePtr);
	    itsFreqs(phase) = lastFreq;
	    phaseCount(phase) = 1;
	    itsCachePtr++;
	    if (itsCachePtr >= Int(itsFreqCache.nelements())) nextCache();
	    
	    // continue while the time < time + interval/2 - counting phases and summing all the time
	    Double endTime = time + interval/2.0;
	    while (!atEnd() && (*(timePtr+itsCachePtr) <= endTime ||
				near(endTime, *(timePtr+itsCachePtr)))) {
		// phases are 1-relative
		Int phase = *(phasePtr + itsCachePtr)-1;
		// sanity check
		if (phase >= Int(itsFreqs.nelements())) {
		    itsFreqs = 0.0;
		    // need to emit a SEVERE error without throwing an exception
		    // clean up
		    itsTimeCache.freeStorage(timePtr, dtimePtr);
		    itsFreqCache.freeStorage(freqPtr, dfreqPtr);
		    itsPhaseCache.freeStorage(phasePtr, dphasePtr);
		    return itsFreqs;
		}
		// cycle through the phases from lastPhase to this phase - 
		// freq was constant throughout
		lastPhase += 1;
		if (lastPhase >= nphases) lastPhase = 0;
		while (phase != lastPhase) {
		    itsFreqs(lastPhase) += lastFreq;
		    phaseCount(lastPhase)++;
		    lastPhase += 1;
		    if (lastPhase >= nphases) lastPhase = 0;
		}
		lastFreq = *(freqPtr+itsCachePtr);
		itsFreqs(phase) += lastFreq;
		phaseCount(phase) += 1;
		lastPhase = phase;
		itsCachePtr++;
		if (itsCachePtr >= Int(itsFreqCache.nelements())) nextCache();
	    }
	    // final pathological case, cycle from lastPhase+1 back to firstPhase
	    lastPhase += 1;
	    if (lastPhase >= nphases) lastPhase = 0;
	    while (lastPhase != firstPhase) {
		itsFreqs(lastPhase) += lastFreq;
		phaseCount(lastPhase)++;
		lastPhase += 1;
		if (lastPhase >= nphases) lastPhase = 0;
	    }	
	    itsTimeCache.freeStorage(timePtr, dtimePtr);
	    itsFreqCache.freeStorage(freqPtr, dfreqPtr);
	    itsPhaseCache.freeStorage(phasePtr, dphasePtr);
	    itsFreqs /= phaseCount;
	}
    }
    return itsFreqs;
}
