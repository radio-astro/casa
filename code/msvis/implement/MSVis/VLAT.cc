//# VLAT.cc: Implemenation of visibility lookahead thread related functionality.
//# Copyright (C) 2011
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
//# Correspondence concerning CASA should be addressed as follows:
//#        Internet email: CASA-request@nrao.edu.
//#        Postal address: CASA Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#include <assert.h>
#include <time.h>
#include <sys/time.h>

#include "VLAT.h"
#include "VisBufferAsync.h"
#include "VisibilityIteratorAsync.h"
#include <casa/Logging/LogIO.h>
#include <casa/System/AipsrcValue.h>

#include "AsynchronousTools.h"
using namespace casa::async;

#include <algorithm>
#include <cstdarg>
#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
//#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace boost;
using namespace casa::utilj;
using namespace std;

#include "UtilJ.h"

#define Log(level, ...) \
    {if (VlaData::loggingInitialized_p && level <= VlaData::logLevel_p) \
         Logger::get()->log (__VA_ARGS__);};

using namespace casa::utilj;
using namespace casa::asyncio;

namespace casa {

//  **********************************************
//	*                                            *
//	* ROVisibilityIteratorAsyncImpl Implementation *
//	*                                            *
//  **********************************************

ROVisibilityIteratorAsyncImpl::ROVisibilityIteratorAsyncImpl ()
 : vlaData_p (NULL),
   vlat_p (NULL)
{
}

//  **************************
//	*                        *
//	* VlaData Implementation *
//	*                        *
//  **************************

//Semaphore VlaData::debugBlockSemaphore_p (0); // used to block a thread for debugging
Bool VlaData::loggingInitialized_p = False;
Int VlaData::logLevel_p = -1;

VlaData::VlaData (Int maxNBuffers)
 : fillCycle_p (True),
   fillOperate_p (True),
   fillWait_p (True),
   MaxNBuffers_p (maxNBuffers),
   readCycle_p (True),
   readOperate_p (True),
   readWait_p (True)
{
    timeStart_p = ThreadTimes();
    lookaheadTerminationRequested_p = False;
    sweepTerminationRequested_p = False;
    viResetComplete_p = False;
    viResetRequested_p = False;

    if (logLevel_p < 0){
        loggingInitialized_p = initializeLogging();
    }
}

VlaData::~VlaData ()
{
    timeStop_p = ThreadTimes();

    if (statsEnabled()){
        Log (1, "VlaData stats:\n%s", makeReport ().c_str());
    }

    resetBufferData ();
}

void
VlaData::addModifier (RoviaModifier * modifier)
{
    Log (1, "VlaData::addModifier: {%s}\n", lexical_cast<string> (* modifier).c_str());

    MutexLocker ml (vlaDataMutex_p);

    roviaModifiers_p.add (modifier);
}

Int
VlaData::clock (Int arg, Int base)
{
	Int r = arg % base;

	if (r < 0){
		r += base;
	}

	return r;
}

void
VlaData::debugBlock ()
{
//    Log (1, "VlaData::debugBlock(): Blocked\n");
//
//    debugBlockSemaphore_p.wait ();
//
//    Log (1, "VlaData::debugBlock(): Unblocked\n");
}

void
VlaData::debugUnblock ()
{
//    int v = debugBlockSemaphore_p.getValue();
//
//    if (v == 0){
//        Log (1, "VlaData::debugUnblock()\n");
//        debugBlockSemaphore_p.post ();
//    }
//    else
//        Log (1, "VlaData::debugUnblock(): already unblocked; v=%d\n", v);
}


void
VlaData::fillComplete (VlaDatum * datum)
{
    MutexLocker ml (vlaDataMutex_p);

    if (statsEnabled()){
        fill3_p = ThreadTimes();
        fillWait_p += fill2_p - fill1_p;
        fillOperate_p += fill3_p - fill2_p;
        fillCycle_p += fill3_p - fill1_p;
    }

    data_p.push (datum);

	Log (2, "VlaData::fillComplete on %s\n", datum->getSubChunkPair ().toString().c_str());

	assert ((Int)data_p.size() <= MaxNBuffers_p);

	vlaDataChanged_p.broadcast ();
}

VlaDatum *
VlaData::fillStart (Int chunkNumber, Int subChunkNumber)
{
    MutexLocker ml (vlaDataMutex_p);

    statsEnabled () && (fill1_p = ThreadTimes(), True);

	while ((int) data_p.size() >= MaxNBuffers_p && ! sweepTerminationRequested_p){
	    vlaDataChanged_p.wait (vlaDataMutex_p);
	}

	VlaDatum * datum = new VlaDatum (chunkNumber, subChunkNumber);

	Log (2, "VlaData::fillStart on %s\n", datum->getSubChunkPair().toString().c_str());

	if (validChunks_p.empty() || validChunks_p.back() != chunkNumber)
	    insertValidChunk (chunkNumber);

	insertValidSubChunk (chunkNumber, subChunkNumber);

	statsEnabled () && (fill2_p = ThreadTimes(), True);

	if (sweepTerminationRequested_p){
	    delete datum;
	    datum = NULL; // datum may not be ready to fill and shouldn't be anyway
	}

	return datum;
}

asyncio::ChannelSelection
VlaData::getChannelSelection () const
{
    MutexLocker ml (vlaDataMutex_p);

    return channelSelection_p;
}

void
VlaData::initialize ()
{
	resetBufferData ();

}

Bool
VlaData::initializeLogging()
{
	// If the log file variable is defined then start
	// up the logger

	const String logFileVariable = "Casa_VIA_LogFile";
	const String logLevelVariable = "Casa_VIA_LogLevel";

	String logFilename;
	Bool logFileFound = AipsrcValue<String>::find (logFilename,
	                                               ROVisibilityIteratorAsync::getAipsRcBase () + ".debug.logFile",
	                                               "");

	if (logFileFound && ! logFilename.empty() && downcase (logFilename) != "null" &&
	    downcase (logFilename) != "none"){

		Logger::get()->start (logFilename.c_str());
		AipsrcValue<Int>::find (logLevel_p, ROVisibilityIteratorAsync::getAipsRcBase () + ".debug.logLevel", 1);
		Logger::get()->log ("VlaData log-level is %d; async I/O: %s; nBuffers=%d\n", logLevel_p,
		                    ROVisibilityIteratorAsync::isAsynchronousIoEnabled() ? "enabled" : "disabled",
		                    ROVisibilityIteratorAsync::getDefaultNBuffers() );

		return True;

	}

	return False;
}

void
VlaData::insertValidChunk (Int chunkNumber)
{
    // Caller locks mutex.

    validChunks_p.push (chunkNumber);

	vlaDataChanged_p.broadcast ();
}

void
VlaData::insertValidSubChunk (Int chunkNumber, Int subChunkNumber)
{
    // Caller locks mutex.

	validSubChunks_p.push (SubChunkPair (chunkNumber, subChunkNumber));

	vlaDataChanged_p.broadcast ();
}

Bool
VlaData::isSweepTerminationRequested () const
{
    return sweepTerminationRequested_p;
}

Bool
VlaData::isValidChunk (Int chunkNumber) const
{
    bool validChunk = False;

    // Check to see if this is a valid chunk.  If the data structure is empty
    // then sleep for a tiny bit to allow the VLAT thread to either make more
    // chunks available for insert the sentinel value INT_MAX into the data
    // structure.

    MutexLocker ml (vlaDataMutex_p);

    do {

        while (validChunks_p.empty()){
            vlaDataChanged_p.wait (vlaDataMutex_p);
        }

        while (! validChunks_p.empty() && validChunks_p.front() < chunkNumber){
            validChunks_p.pop();
        }

        if (! validChunks_p.empty())
            validChunk = validChunks_p.front() == chunkNumber;

    } while (validChunks_p.empty());

    Log (3, "isValidChunk (%d) --> %s\n", chunkNumber, validChunk ? "true" : "false");

	return validChunk;
}

Bool
VlaData::isValidSubChunk (Int chunkNumber, Int subChunkNumber) const
{
	SubChunkPair subChunk (chunkNumber, subChunkNumber);
	SubChunkPair s;

    bool validSubChunk = False;

    // Check to see if this is a valid subchunk.  If the data structure is empty
    // then sleep for a tiny bit to allow the VLAT thread to either make more
    // subchunks available for insert the sentinel value (INT_MAX, INT_MAX) into the data
    // structure.

    MutexLocker ml (vlaDataMutex_p);

    do {

        while (validSubChunks_p.empty()){
            vlaDataChanged_p.wait (vlaDataMutex_p);
        }

        while (! validSubChunks_p.empty() && validSubChunks_p.front() < subChunk){
            validSubChunks_p.pop();
        }

        if (! validSubChunks_p.empty())
            validSubChunk = validSubChunks_p.front() == subChunk;

    } while (validSubChunks_p.empty());

    Log (3, "isValidSubChunk (%d, %d) --> %s\n", chunkNumber, subChunkNumber, validSubChunk ? "true" : "false");

	return validSubChunk;
}

String
VlaData::makeReport ()
{
    String report;

    DeltaThreadTimes duration = (timeStop_p - timeStart_p); // seconds
    report += format ("\nLookahead Stats: nCycles=%d, duration=%.3f sec\n...\n",
                      readWait_p.n(), duration.elapsed());
    report += "...ReadWait:    " + readWait_p.formatAverage () + "\n";
    report += "...ReadOperate: " + readOperate_p.formatAverage() + "\n";
    report += "...ReadCycle:   " + readCycle_p.formatAverage() + "\n";

    report += "...FillWait:    " + fillWait_p.formatAverage() + "\n";
    report += "...FillOperate: " + fillOperate_p.formatAverage () + "\n";
    report += "...FillCycle:   " + fillCycle_p.formatAverage () + "\n";

    Double syncCycle = fillOperate_p.elapsedAvg() + readOperate_p.elapsedAvg();
    Double asyncCycle = max (fillCycle_p.elapsedAvg(), readCycle_p.elapsedAvg());
    report += format ("...Sync cycle would be %6.1f ms\n", syncCycle * 1000);
    report += format ("...Speedup is %5.1f%%\n", (syncCycle / asyncCycle  - 1) * 100);
    report += format ("...Total time savings estimate is %7.3f seconds\n", fillOperate_p.elapsed ());

    return report;

}


void
VlaData::readComplete (Int chunkNumber, Int subChunkNumber)
{
    MutexLocker ml (vlaDataMutex_p);

    if (statsEnabled()){
        read3_p = ThreadTimes();
        readWait_p += read2_p - read1_p;
        readOperate_p += read3_p - read2_p;
        readCycle_p += read3_p - read1_p;
    }

	Log (2, "VlaData::readComplete on (%d,%d)\n", chunkNumber, subChunkNumber);
}

VisBufferAsync *
VlaData::readStart (Int chunkNumber, Int subChunkNumber)
{
    MutexLocker ml (vlaDataMutex_p);

    statsEnabled () && (read1_p = ThreadTimes(), True);

    // Wait for a subchunk's worth of data to be available.

	while (data_p.empty()){
	    vlaDataChanged_p.wait(vlaDataMutex_p);
	}

	// Get the data off the queue and notify world of change in VlaData.

	VlaDatum * datum = data_p.front();
	data_p.pop ();
	vlaDataChanged_p.broadcast ();

	ThrowIf (! datum->isSubChunk (chunkNumber, subChunkNumber),
	         utilj::format ("Reader wanted subchunk (%d,%d) while next subchunk is %s",
	                        chunkNumber, subChunkNumber, datum->getSubChunkPair().toString().c_str()));

	Log (2, "VlaData::readStart on (%d, %d)\n", chunkNumber, subChunkNumber);

	statsEnabled () && (read2_p = ThreadTimes(), True);

    // Extract the VisBufferAsync enclosed in the datum for return to caller,
    // then destroy the rest of the datum object

    VisBufferAsync * vba = datum->releaseVisBufferAsync ();
    delete datum;
	return vba;
}

void
VlaData::requestViReset ()
{
    MutexLocker ml (vlaDataMutex_p);  // enter critical section

    Log (1, "Requesting VI reset\n");

    viResetRequested_p = True; // officially request the reset
    viResetComplete_p = False; // clear any previous completions

    terminateSweep ();

    // Wait for the request to be completed.

    Log (1, "Waiting for requesting VI reset\n");

    while (! viResetComplete_p){
        vlaDataChanged_p.wait (vlaDataMutex_p);
    }

    Log (1, "Notified that VI reset has completed\n");

    // The VI was reset
}

void
VlaData::resetBufferData ()
{
    // Caller already has mutex locked.

    // Flush any accumulated buffers

	while (! data_p.empty()){
		VlaDatum * datum = data_p.front();
		data_p.pop ();
		delete datum;
	}

	// Flush the chunk and subchunk indices

	while (! validChunks_p.empty())
	    validChunks_p.pop();

	while (! validSubChunks_p.empty())
	    validSubChunks_p.pop();
}

void
VlaData::setNoMoreData ()
{
    MutexLocker ml (vlaDataMutex_p);

    insertValidChunk (INT_MAX);
    insertValidSubChunk (INT_MAX, INT_MAX);
}

Bool
VlaData::statsEnabled () const
{
    // Determines whether asynchronous I/O is enabled by looking for the
    // expected AipsRc value.  If not found then async i/o is disabled.

    Bool doStats;
    AipsrcValue<Bool>::find (doStats, ROVisibilityIteratorAsync::getAipsRcBase () + ".doStats", False);

    return doStats;
}

void
VlaData::storeChannelSelection (const asyncio::ChannelSelection & channelSelection)
{
    MutexLocker ml (vlaDataMutex_p);

    channelSelection_p = channelSelection;
}

void
VlaData::terminateLookahead ()
{
    // Called by main thread to stop the VLAT, etc.

    MutexLocker ml (vlaDataMutex_p);

    lookaheadTerminationRequested_p = True;
    terminateSweep();
}

void
VlaData::terminateSweep ()
{
    sweepTerminationRequested_p = True;   // stop filling
    vlaDataChanged_p.broadcast ();
}

pair<Bool, RoviaModifiers>
VlaData::waitForViReset()
{
    // Called by VLAT

    MutexLocker ml (vlaDataMutex_p);

    Log (1, "Waiting for VI reset request\n");

    while (! viResetRequested_p && ! lookaheadTerminationRequested_p){
        vlaDataChanged_p.wait (vlaDataMutex_p);
    }

    if (lookaheadTerminationRequested_p){
        Log (1, "Terminating VLAT during wait for VI reset.\n");
        return make_pair (False, RoviaModifiers ());
    }
    else {
        Log (1, "Carrying out VI reset\n")
        viResetRequested_p = False;

        resetBufferData ();

        asyncio::RoviaModifiers roviaModifiersCopy;
        roviaModifiers_p.transfer (roviaModifiersCopy);

        sweepTerminationRequested_p = False;
        viResetComplete_p = True;

        vlaDataChanged_p.broadcast();
        return make_pair (True, roviaModifiersCopy);
    }
}

//  ***************************
//	*                         *
//	* VlaDatum Implementation *
//	*                         *
//  ***************************

VlaDatum::VlaDatum (Int chunkNumber, Int subChunkNumber)
: chunkNumber_p (chunkNumber),
  subChunkNumber_p (subChunkNumber),
  visBuffer_p (new VisBufferAsync)
{}

VlaDatum::~VlaDatum()
{
    delete visBuffer_p;
}

SubChunkPair
VlaDatum::getSubChunkPair () const
{
	return SubChunkPair (chunkNumber_p, subChunkNumber_p);
}

VisBufferAsync *
VlaDatum::getVisBuffer ()
{
	return visBuffer_p;
}

//const VisBufferAsync *
//VlaDatum::getVisBuffer () const
//{
//    assert (state_p == Filling || state_p == Reading);
//
//    return visBuffer_p;
//}

Bool
VlaDatum::isSubChunk (Int chunkNumber, Int subChunkNumber) const
{
	return chunkNumber == chunkNumber_p &&
		   subChunkNumber == subChunkNumber_p;
}

VisBufferAsync *
VlaDatum::releaseVisBufferAsync ()
{
    VisBufferAsync * vba = visBuffer_p;
    visBuffer_p = NULL;

    return vba;
}

//  ***********************
//	*                     *
//	* VLAT Implementation *
//	*                     *
//  ***********************

VLAT::VLAT (VlaData * vd)
{
	vlaData_p = vd;
	visibilityIterator_p = NULL;
	threadTerminated_p = False;
}

VLAT::~VLAT ()
{

	// Free up storage

	for (FillerDictionary::iterator f = fillerDictionary_p.begin();
		 f != fillerDictionary_p.end();
		 f++){
		delete (f->second);
	}

	for (Fillers::iterator f = fillers_p.begin();
		 f != fillers_p.end();
		 f++){
		delete (* f);
	}

	delete visibilityIterator_p;
}

void
VLAT::applyModifiers (ROVisibilityIterator * rovi)
{
    roviaModifiers_p.apply (rovi);
    roviaModifiers_p.clear ();

    Block< Vector<Int> > blockNGroup;
    Block< Vector<Int> > blockStart;
    Block< Vector<Int> > blockWidth;
    Block< Vector<Int> > blockIncr;
    Block< Vector<Int> > blockSpw;

    rovi->getChannelSelection (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);

    vlaData_p -> storeChannelSelection (asyncio::ChannelSelection (blockNGroup, blockStart,
                                                                   blockWidth, blockIncr, blockSpw));
}

void
VLAT::createFillerDictionary ()
{
	// Create a dictionary of all the possible fillers using the
	// ROVisibilityIteratorAsync::PrefetchColumnIds as the keys

	fillerDictionary_p.clear();

	fillerDictionary_p.add(Ant1,
	                       vlatFunctor0 (& VisBuffer::fillAnt1));
	fillerDictionary_p.add(Ant2,
	                       vlatFunctor0 (& VisBuffer::fillAnt2));
	fillerDictionary_p.add(ArrayId,
	                       vlatFunctor0 (& VisBuffer::fillArrayId));
	fillerDictionary_p.add(Channel,
	                       vlatFunctor0 (& VisBuffer::fillChannel));
	fillerDictionary_p.add(Cjones,
	                       vlatFunctor0 (& VisBuffer::fillCjones));
	fillerDictionary_p.add(CorrType,
	                       vlatFunctor0 (& VisBuffer::fillCorrType));
	fillerDictionary_p.add(Corrected,
	                       vlatFunctor1(& VisBuffer::fillVis,
	                                    VisibilityIterator::Corrected));
	fillerDictionary_p.add(CorrectedCube,
	                       vlatFunctor1(& VisBuffer::fillVisCube,
	                                    VisibilityIterator::Corrected));
	fillerDictionary_p.add(Direction1,
	                       vlatFunctor0 (& VisBuffer::fillDirection1));
	fillerDictionary_p.add(Direction2,
	                       vlatFunctor0 (& VisBuffer::fillDirection2));
	fillerDictionary_p.add(Exposure,
	                       vlatFunctor0 (& VisBuffer::fillExposure));
	fillerDictionary_p.add(Feed1,
	                       vlatFunctor0 (& VisBuffer::fillFeed1));
	fillerDictionary_p.add(Feed1_pa,
	                       vlatFunctor0 (& VisBuffer::fillFeed1_pa));
	fillerDictionary_p.add(Feed2,
	                       vlatFunctor0 (& VisBuffer::fillFeed2));
	fillerDictionary_p.add(Feed2_pa,
	                       vlatFunctor0 (& VisBuffer::fillFeed2_pa));
	fillerDictionary_p.add(FieldId,
	                       vlatFunctor0 (& VisBuffer::fillFieldId));
	fillerDictionary_p.add(Flag,
	                       vlatFunctor0 (& VisBuffer::fillFlag));
	fillerDictionary_p.add(FlagCategory,
	                       vlatFunctor0 (& VisBuffer::fillFlagCategory));
	fillerDictionary_p.add(FlagCube,
	                       vlatFunctor0 (& VisBuffer::fillFlagCube));
	fillerDictionary_p.add(FlagRow,
	                       vlatFunctor0 (& VisBuffer::fillFlagRow));
	fillerDictionary_p.add(Freq,
	                       vlatFunctor0 (& VisBuffer::fillFreq));
	fillerDictionary_p.add(ImagingWeight,
	                       vlatFunctor0 (& VisBuffer::fillImagingWeight));
	fillerDictionary_p.add(LSRFreq,
	                       vlatFunctor0 (& VisBuffer::fillLSRFreq));
	fillerDictionary_p.add(Model,
	                       vlatFunctor1(& VisBuffer::fillVis,
	                                    VisibilityIterator::Model));
	fillerDictionary_p.add(ModelCube,
	                       vlatFunctor1(& VisBuffer::fillVisCube,
	                                    VisibilityIterator::Model));
	fillerDictionary_p.add(NChannel,
	                       vlatFunctor0 (& VisBuffer::fillnChannel));
	fillerDictionary_p.add(NCorr,
	                       vlatFunctor0 (& VisBuffer::fillnCorr));
	fillerDictionary_p.add(NRow,
	                       vlatFunctor0 (& VisBuffer::fillnRow));
	fillerDictionary_p.add(ObservationId,
	                       vlatFunctor0 (& VisBuffer::fillObservationId));
	fillerDictionary_p.add(Observed,
	                       vlatFunctor1(& VisBuffer::fillVis,
	                                    VisibilityIterator::Observed));
	fillerDictionary_p.add(ObservedCube,
	                       vlatFunctor1(& VisBuffer::fillVisCube,
	                                    VisibilityIterator::Observed));
	fillerDictionary_p.add(PhaseCenter,
	                       vlatFunctor0 (& VisBuffer::fillPhaseCenter));
	fillerDictionary_p.add(PolFrame,
	                       vlatFunctor0 (& VisBuffer::fillPolFrame));
	fillerDictionary_p.add(ProcessorId,
	                       vlatFunctor0 (& VisBuffer::fillProcessorId));
	fillerDictionary_p.add(Scan,
	                       vlatFunctor0 (& VisBuffer::fillScan));
	fillerDictionary_p.add(Sigma,
	                       vlatFunctor0 (& VisBuffer::fillSigma));
	fillerDictionary_p.add(SigmaMat,
	                       vlatFunctor0 (& VisBuffer::fillSigmaMat));
	fillerDictionary_p.add(SpW,
	                       vlatFunctor0 (& VisBuffer::fillSpW));
	fillerDictionary_p.add(StateId,
	                       vlatFunctor0 (& VisBuffer::fillStateId));
	fillerDictionary_p.add(casa::asyncio::Time,
	                       vlatFunctor0 (& VisBuffer::fillTime));
	fillerDictionary_p.add(TimeCentroid,
	                       vlatFunctor0 (& VisBuffer::fillTimeCentroid));
	fillerDictionary_p.add(TimeInterval,
	                       vlatFunctor0 (& VisBuffer::fillTimeInterval));
	fillerDictionary_p.add(Uvw,
	                       vlatFunctor0 (& VisBuffer::filluvw));
	fillerDictionary_p.add(UvwMat,
	                       vlatFunctor0 (& VisBuffer::filluvwMat));
	fillerDictionary_p.add(Weight,
	                       vlatFunctor0 (& VisBuffer::fillWeight));
	fillerDictionary_p.add(WeightMat,
	                       vlatFunctor0 (& VisBuffer::fillWeightMat));
	fillerDictionary_p.add(WeightSpectrum,
	                       vlatFunctor0 (& VisBuffer::fillWeightSpectrum));

    assert (fillerDictionary_p.size() == N_PrefetchColumnIds);
                // Every supported prefetch column needs a filler

    //fillerDependencies_p.add ();

    //fillerDependencies_p.performTransitiveClosure (fillerDictionary_p);
}

void
VLAT::fillDatum (VlaDatum * datum)
{

    Int fillerId = -1;
    try{
        VisBufferAsync * vb = datum->getVisBuffer();

        vb->clear();
        vb->setFilling (true);
        vb->attachToVisIter (* visibilityIterator_p); // invalidates vb's cache as well

        fillDatumMiscellanyBefore (datum);

        for (Fillers::iterator filler = fillers_p.begin(); filler != fillers_p.end(); filler ++){

            //Log (2, "Filler id=%d name=%s starting\n", (* filler)->getId(), ROVisibilityIteratorAsync::prefetchColumnName((* filler)->getId()).c_str());

            fillerId = (* filler)->getId();
            (** filler) (vb);
        }

        fillerId = -1; //

        fillDatumMiscellanyAfter (datum);

        vb->detachFromVisIter ();
        vb->setFilling (false);
    }
    catch (...){

        if (fillerId == -1){
            Log (1, "VLAT: Error while filling datum at 0x%08x, vb at 0x%08x; rethrowing\n",
                 datum, datum->getVisBuffer());
        }
        else{
            Log (1, "VLAT: Error while filling datum at 0x%08x, vb at 0x%08x; "
                 "fillingColumn='%s'; rethrowing\n",
                 datum, datum->getVisBuffer(),
                 ROVisibilityIteratorAsync::prefetchColumnName (fillerId).c_str());
        }

        throw;
    }
}

void
VLAT::fillDatumMiscellanyAfter (VlaDatum * datum)
{
	datum->getVisBuffer()->setVisibilityShape (visibilityIterator_p->visibilityShape ());

	datum->getVisBuffer()->setDataDescriptionId (visibilityIterator_p->getDataDescriptionId());

	datum->getVisBuffer()->setNCoh(visibilityIterator_p->numberCoh ());

    Vector<Int> nvischan;
    Vector<Int> spw;

    visibilityIterator_p->allSelectedSpectralWindows(spw, nvischan);

    datum->getVisBuffer()->setSelectedNVisibilityChannels (nvischan);
    datum->getVisBuffer()->setSelectedSpectralWindows (spw);

    Vector<Double> lsrFreq, selFreq;
    visibilityIterator_p->getTopoFreqs (lsrFreq, selFreq);
    datum->getVisBuffer()->setTopoFreqs (lsrFreq, selFreq);

    fillLsrInfo (datum);

	datum->getVisBuffer()->setMSD (visibilityIterator_p->getMSD ()); // ought to be last
}

void
VLAT::fillDatumMiscellanyBefore (VlaDatum * datum)
{
	datum->getVisBuffer()->setMeasurementSet (visibilityIterator_p->getMeasurementSet());
	datum->getVisBuffer()->setMeasurementSetId (visibilityIterator_p->getMeasurementSetId(),
			                                    datum->getSubChunkPair().second == 0);

	datum->getVisBuffer()->setNAntennas (visibilityIterator_p->getNAntennas ());
	datum->getVisBuffer()->setMEpoch (visibilityIterator_p->getMEpoch ());
	datum->getVisBuffer()->setReceptor0Angle (visibilityIterator_p->getReceptor0Angle());
}


void
VLAT::fillLsrInfo (VlaDatum * datum)
{
    Block<Int> channelStart;
    Block<Int> channelWidth;
    Block<Int> channelIncrement;
    Block<Int> channelGroupNumber;
    const ROArrayColumn <Double> * chanFreqs;
    const ROScalarColumn<Int> * obsMFreqTypes;
    MPosition observatoryPositon;
    MDirection phaseCenter;
    Bool velocitySelection;

    visibilityIterator_p->getLsrInfo (channelStart,
                                      channelWidth,
                                      channelIncrement,
                                      channelGroupNumber,
                                      chanFreqs,
                                      obsMFreqTypes,
                                      observatoryPositon,
                                      phaseCenter,
                                      velocitySelection);

    datum->getVisBuffer()->setLsrInfo (channelStart,
                                       channelWidth,
                                       channelIncrement,
                                       channelGroupNumber,
                                       chanFreqs,
                                       obsMFreqTypes,
                                       observatoryPositon,
                                       phaseCenter,
                                       velocitySelection);
}

void
VLAT::initialize (const ROVisibilityIterator & rovi)
{
	ThrowIf (isStarted(), "VLAT::initialize: thread already started");

	visibilityIterator_p = new ROVisibilityIterator (rovi);

	//Bool newMS = visibilityIterator_p->msIter_p.newMS();

	visibilityIterator_p->originChunks (True);
	    // force the MSIter, etc., to be rewound, reinitialized, etc.

	//newMS = visibilityIterator_p->msIter_p.newMS();
	//visibilityIterator_p->originChunks (True);
	    // do it twice??

}

void
VLAT::initialize (const MeasurementSet & ms,
				  const Block<Int> & sortColumns,
		          Double timeInterval)
{
	ThrowIf (isStarted(), "VLAT::initialize: thread already started");

	visibilityIterator_p = new ROVisibilityIterator (ms, sortColumns, timeInterval);
}

void
VLAT::initialize (const MeasurementSet & ms,
			      const Block<Int> & sortColumns,
	              const Bool addDefaultSortCols,
	              Double timeInterval)
{
	ThrowIf (isStarted(), "VLAT::initialize: thread already started");

	visibilityIterator_p = new ROVisibilityIterator (ms, sortColumns, addDefaultSortCols, timeInterval);
}

void
VLAT::initialize (const Block<MeasurementSet> & mss,
	              const Block<Int> & sortColumns,
	              Double timeInterval)
{
	ThrowIf (isStarted(), "VLAT::initialize: thread already started");

	visibilityIterator_p = new ROVisibilityIterator (mss, sortColumns, timeInterval);
}

void
VLAT::initialize (const Block<MeasurementSet> & mss,
	              const Block<Int> & sortColumns,
	              const Bool addDefaultSortCols,
	              Double timeInterval)
{
	ThrowIf (isStarted(), "VLAT::initialize: thread already started");

	visibilityIterator_p = new ROVisibilityIterator (mss, sortColumns, addDefaultSortCols, timeInterval);
}

Bool
VLAT::isTerminated () const
{
    return threadTerminated_p;
}

void *
VLAT::run ()
{
    LogIO logIo (LogOrigin ("VLAT"));
	Log (1, "VLAT starting execution; tid=%d\n", gettid());
	logIo << "starting execution; tid=" << gettid() << endl << LogIO::POST;

    if (VlaData::loggingInitialized_p){
        Logger::get()->registerName ("VLAT");
    }

	try {

	    do{

	        Log (1, "VLAT starting VI sweep\n");

	        // Start sweeping the real VI over its entire range
	        // (subject to the number of free buffers).  The sweep
	        // can be ended abruptly if

            sweepVi ();

            pair<Bool,asyncio::RoviaModifiers> resetVi = vlaData_p->waitForViReset ();

            if (! resetVi.first){
                break; // Not resetting so it's time to quit
            }
            else{
                setModifiers (resetVi.second);
            }

	    } while (True);

		threadTerminated_p = true;
     	Log (1, "VLAT stopping execution.\n");
     	logIo << "stopping execution normally; tid=" << gettid() << endl << LogIO::POST;

		return NULL;

	}
	catch (std::exception & e){

		cerr << "VLAT thread caught exception: " << e.what() << endl;
		cerr.flush();

     	Log (1, "VLAT caught exception: %s.\n", e.what());
     	logIo << "caught exception; tid=" << gettid() << "-->" << e.what() << endl << LogIO::POST;

		threadTerminated_p = true;
		throw;
	}
	catch (...){

		cerr << "VLAT thread caught unknown exception: " << endl;
		cerr.flush();

     	Log (1, "VLAT caught unknown exception:\n");
     	logIo << "caught unknown exception; tid=" << gettid() << endl << LogIO::POST;

		threadTerminated_p = true;
		throw;
	}
}

void
VLAT::setModifiers (asyncio::RoviaModifiers & modifiers)
{
    roviaModifiers_p.transfer (modifiers);
}


void
VLAT::setPrefetchColumns (const ROVisibilityIteratorAsync::PrefetchColumns & columns)
{
	ThrowIf (isStarted(), "VLAT::setColumns: cannot do this after thread started");
	ThrowIf (! fillers_p.empty(), "VLAT::setColumns:: has already been done");

	createFillerDictionary ();

	for (ROVisibilityIteratorAsync::PrefetchColumns::const_iterator c = columns.begin();
		 c != columns.end();
		 c ++){

	    ThrowIf (! containsKey (*c, fillerDictionary_p), format ("Unknown prefetch column id (%d)", *c));

		fillers_p.push_back (fillerDictionary_p [*c]->clone());
	}

//	sort (fillers_p.begin(),
//	      fillers_p.end(),
//	      VlatFunctor::byDecreasingPrecedence);
}

void
VLAT::sweepVi ()
{
    Int chunkNumber = -1;
    Int subChunkNumber = -1;

    applyModifiers (visibilityIterator_p);

    for (visibilityIterator_p->originChunks();
          visibilityIterator_p->moreChunks();
          visibilityIterator_p->nextChunk()){

        chunkNumber ++;
        subChunkNumber = -1;

        for (visibilityIterator_p->origin();
             visibilityIterator_p->more();
             ++ (* visibilityIterator_p)){

            subChunkNumber ++;

            if (vlaData_p->isSweepTerminationRequested())
                goto done;

            VlaDatum * vlaDatum = vlaData_p -> fillStart (chunkNumber, subChunkNumber);

            if (vlaData_p->isSweepTerminationRequested())
                goto done;

            fillDatum (vlaDatum);

            if (vlaData_p->isSweepTerminationRequested())
                goto done;

            vlaData_p -> fillComplete (vlaDatum);

            if (vlaData_p->isSweepTerminationRequested())
                goto done;
        }
    }

    Log (1, "VLAT: no more data\n");

    vlaData_p -> setNoMoreData ();

done:

    Log (1, "VLAT stopping execution.\n");

}

void
VLAT::terminate ()
{
    // Called by another thread to terminate the VLAT.

    Log (2, "Terminating VLAT\n");
    //printBacktrace (cerr, "VLAT termination");

    Thread::terminate(); // ask thread to terminate

    vlaData_p->terminateLookahead (); // stop lookahead
}

} // end namespace casa
