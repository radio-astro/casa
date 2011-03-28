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
const Bool VlaData::loggingInitialized_p = initializeLogging();
Int VlaData::logLevel_p = 1;

VlaData::VlaData (Int nBuffers)
 : data_p (nBuffers, static_cast<VlaDatum *> (0))
{
    lookaheadTerminationRequested_p = False;
    sweepTerminationRequested_p = False;
    viResetComplete_p = False;
    viResetRequested_p = False;
}

VlaData::~VlaData ()
{
    if (stats_p.isEnabled()){
        Log (1, "VlaData stats:\n%s", stats_p.makeReport ().c_str());
    }

    for (Data::iterator d = data_p.begin(); d != data_p.end(); d++){
        delete (* d);
    }
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
VlaData::fillComplete ()
{
    MutexLocker ml (vlaDataMutex_p);

    stats_p.addEvent (Stats::Fill|Stats::End);

    Int chunkNumber = data_p [fillIndex_p]->getChunkNumber();
    Int subChunkNumber = data_p [fillIndex_p]->getSubChunkNumber();

	Log (2, "VlaData::fillComplete on (%d,%d) at [%d]\n", chunkNumber, subChunkNumber, fillIndex_p);

	assert (fillIndex_p >= 0 && fillIndex_p < (int) data_p.size());

	data_p [fillIndex_p]->fillComplete();

	vlaDataChanged_p.broadcast ();
}

VlaDatum *
VlaData::fillStart (Int chunkNumber, Int subChunkNumber)
{
    MutexLocker ml (vlaDataMutex_p);

    stats_p.addEvent (Stats::Fill|Stats::Request);

	VlaDatum * datum = getNextDatum (fillIndex_p);

	Log (2, "VlaData::fillStart on (%d,%d) at [%d]\n", chunkNumber, subChunkNumber, fillIndex_p);

	if (validChunks_p.empty() || validChunks_p.back() != chunkNumber)
	    insertValidChunk (chunkNumber);

	insertValidSubChunk (chunkNumber, subChunkNumber);

	Bool hadToWait = False;

	while (! datum->fillStart (chunkNumber, subChunkNumber) &&
	       ! sweepTerminationRequested_p){
	    hadToWait = True;
	    vlaDataChanged_p.wait (vlaDataMutex_p);
	}

    stats_p.addEvent (Stats::Fill|Stats::Begin);

	if (sweepTerminationRequested_p)
	    datum = NULL; // datum may not be ready to fill and shouldn't be anyway

	return datum;
}

asyncio::ChannelSelection
VlaData::getChannelSelection () const
{
    MutexLocker ml (vlaDataMutex_p);

    return channelSelection_p;
}


VlaDatum *
VlaData::getNextDatum (Int & index)
{
    // Assumes caller has the mutex locked.

	assert (index >= 0 && index < (int) data_p.size());

	index = clock (index + 1, data_p.size());

	VlaDatum * datum = data_p [index];

	return datum;
}

void
VlaData::initialize ()
{
	Int id = 0;
	for (Data::iterator d = data_p.begin(); d != data_p.end(); d++){
		* d = new VlaDatum (id);
		(* d) -> initialize ();
		Log (2, "VlaDatum [%d]: vb @ 0x%08x\n", id, (*d)->getVisBuffer (True));
		id ++;
	}

	resetBufferRing ();

    if (statsEnabled())
        stats_p.reserve (10000);
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

void
VlaData::readComplete ()
{
    MutexLocker ml (vlaDataMutex_p);

    stats_p.addEvent (Stats::End);

	Log (2, "VlaData::readComplete on (%d,%d) at [%d]\n",
	     data_p [readIndex_p]->getChunkNumber(),
	     data_p [readIndex_p]->getSubChunkNumber(),
	     readIndex_p);

	assert (readIndex_p >= 0 && readIndex_p < (int) data_p.size());

	data_p [readIndex_p]->readComplete();

	vlaDataChanged_p.broadcast();
}

const VlaDatum *
VlaData::readStart (Int chunkNumber, Int subChunkNumber)
{
    MutexLocker ml (vlaDataMutex_p);

    stats_p.addEvent (Stats::Request);

	VlaDatum * datum = getNextDatum (readIndex_p);

	Log (2, "VlaData::readStart on (%d, %d) at [%d]\n", chunkNumber, subChunkNumber, readIndex_p);

	bool hadToWait = False;
	while (! datum->readStart ()){
	    hadToWait = True;
	    vlaDataChanged_p.wait(vlaDataMutex_p);
	}

    stats_p.addEvent (Stats::Begin);

	return datum;
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
VlaData::resetBufferRing ()
{
	fillIndex_p = data_p.size() - 1;
	readIndex_p = data_p.size() - 1;

	for (Data::iterator d = data_p.begin(); d != data_p.end(); d++){
		(* d) -> reset ();
	}

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

        resetBufferRing ();

        asyncio::RoviaModifiers roviaModifiersCopy;
        roviaModifiers_p.transfer (roviaModifiersCopy);

        sweepTerminationRequested_p = False;
        viResetComplete_p = True;

        vlaDataChanged_p.broadcast();
        return make_pair (True, roviaModifiersCopy);
    }
}

VlaData::Stats::Stats ()
{
    enabled_p = false;
}

void
VlaData::Stats::addEvent (Int type)
{
    if (enabled_p){
        struct timeval tVal;
        gettimeofday (& tVal, NULL);

        Double t = tVal.tv_sec + tVal.tv_usec * 1e-6;

        events_p.push_back (Event (type, t));
    }
}


String
VlaData::Stats::makeReport ()
{
    String report;

    if (enabled_p) {

        OpStats readStats, fillStats;

        for (Events::iterator event = events_p.begin(); event != events_p.end(); ++ event){

            Int type = event->get<0>();

            if ((type & Fill) != 0){
                fillStats.update (type, event->get<1>());
            }
            else {
                readStats.update (type, event->get<1>());
            }
        }

        report += format ("\nLookahead Stats: nCycles=%d\n...\n", readStats.getN());
        report += "...ReadWait:    " + readStats.format(Wait) + "\n";
        report += "...ReadOperate: " + readStats.format(Operate) + "\n";
        report += "...ReadCycle:   " + readStats.format(Cycle) + "\n";

        report += "...FillWait:    " + fillStats.format(Wait) + "\n";
        report += "...FillOperate: " + fillStats.format(Operate) + "\n";
        report += "...FillCycle:   " + fillStats.format(Cycle) + "\n";

        Double syncCycle = fillStats.getAvg (Operate) + readStats.getAvg (Operate);
        report += format ("...Sync cycle would be %6.1f ms\n", syncCycle * 1000);
        report += format ("...Speedup is %5.1f%%\n", (syncCycle / readStats.getAvg (Cycle) - 1) * 100);

    }

    return report;

}

void
VlaData::Stats::reserve (Int size)
{
    events_p.reserve (size);
    enabled_p = True;
}

VlaData::Stats::OpStats::OpStats ()
: events_p (End + 1, 0),
  max_p (3, -1E20),
  min_p (3, 1E20),
  n_p (0),
  ssq_p (3, 0),
  sum_p (3, 0)
{}

void
VlaData::Stats::OpStats::accumulate (Double wait, Double operate, Double cycle)
{
    ++ n_p;
    vector<Double> s (3);
    s [Wait] = wait;
    s [Operate] = operate;
    s [Cycle] = cycle;

    using namespace boost::lambda;
    //using boost::lambda::_1;
    //using boost::lambda::_2;

    // Compute the sum for all periods.

    transform (sum_p.begin(), sum_p.end(), s.begin(), sum_p.begin(), _1 + _2);

    // Compute the sum of squares for all periods.

    transform (ssq_p.begin(), ssq_p.end(), s.begin(), ssq_p.begin(), _1 + _2 * _2);

    // Compute the min for all periods using simple wrapper of min function.

    transform (min_p.begin(), min_p.end(), s.begin(), min_p.begin(), dmin);

    // Compute the max for all periods using simple wrapper of max function.

    transform (max_p.begin(), max_p.end(), s.begin(), max_p.begin(), dmax);
}

String
VlaData::Stats::OpStats::format (Int component)
{
    Double avg = (n_p != 0) ? sum_p [component] / n_p : 0;
    Double stdev = (n_p != 0) ? sqrt (ssq_p[component] / n_p - avg * avg) : 0;

    String s = utilj::format ("avg=%6.1f ms, stdev=%6.1f, range=[%6.1f, %6.1f]", avg*1000, stdev * 1000,
                              min_p[component] * 1000, max_p[component] * 1000);

    return s;
}


void
VlaData::Stats::OpStats::update (Int type, Double t)
{
    Int masked = type & (~Fill);

    events_p [masked] = t;

    if (masked == End){

        Double wait = events_p [Begin] - events_p [Request];
        Double operate = events_p [End] - events_p [Begin];
        Double cycle = events_p [End] - events_p [Request];

        accumulate (wait, operate, cycle);

        events_p [Begin] = 0;
        events_p [Request] = 0;
        events_p [End] = 0;
    }
}

//  ***************************
//	*                         *
//	* VlaDatum Implementation *
//	*                         *
//  ***************************

VlaDatum::VlaDatum (Int id)
{
	id_p = id;
	state_p = Empty;
    visBuffer_p = NULL;
}

VlaDatum::~VlaDatum()
{
    delete visBuffer_p;
}

void
VlaDatum::fillComplete ()
{
	assert (state_p == Filling);

	state_p = Full;
}

Bool
VlaDatum::fillStart (Int chunkNumber, Int subChunkNumber)
{
    // Caller has mutex locked.

    assert (state_p != Filling);

    bool fillStarted = False;

    if (state_p == Empty){

        // Change datum state to be Filling the expected
        // chunk and subchunk

        chunkNumber_p = chunkNumber;
        subChunkNumber_p = subChunkNumber;
        state_p = Filling;

        fillStarted = True;
    }

	return fillStarted;
}

Int
VlaDatum::getChunkNumber () const
{
	return chunkNumber_p;
}

Int
VlaDatum::getId () const
{
    return id_p;
}

Int
VlaDatum::getSubChunkNumber () const
{
	return subChunkNumber_p;
}

VisBufferAsync *
VlaDatum::getVisBuffer (Bool bypassAssert)
{
    assert (bypassAssert || state_p == Filling || state_p == Reading);

	return visBuffer_p;
}

const VisBufferAsync *
VlaDatum::getVisBuffer () const
{
    assert (state_p == Filling || state_p == Reading);

    return visBuffer_p;
}

void
VlaDatum::initialize ()
{
    visBuffer_p = new VisBufferAsync ();

    reset ();
}

Bool
VlaDatum::isChunk (Int chunkNumber, Int subChunkNumber) const
{
    assert (state_p == Reading);

	return chunkNumber == chunkNumber_p &&
		   subChunkNumber == subChunkNumber_p;
}

void
VlaDatum::readComplete ()
{
	assert (state_p == Reading);

	state_p = Empty;
}

Bool
VlaDatum::readStart ()
{
    Bool readStarted = False;

    assert (state_p != Reading);

    if (state_p == Full){

        readStarted = True;
        state_p = Reading;
    }

	return readStarted;
}

void
VlaDatum::reset ()
{
	chunkNumber_p = -1;
	state_p = Empty;
	subChunkNumber_p = -1;
	visBuffer_p->clear();
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

	fillerDictionary_p.add(ROVisibilityIteratorAsync::Ant1,
	                       vlatFunctor0 (& VisBuffer::fillAnt1));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Ant2,
	                       vlatFunctor0 (& VisBuffer::fillAnt2));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::ArrayId,
	                       vlatFunctor0 (& VisBuffer::fillArrayId));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Channel,
	                       vlatFunctor0 (& VisBuffer::fillChannel));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Cjones,
	                       vlatFunctor0 (& VisBuffer::fillCjones));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::CorrType,
	                       vlatFunctor0 (& VisBuffer::fillCorrType));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Corrected,
	                       vlatFunctor1(& VisBuffer::fillVis,
	                                    VisibilityIterator::Corrected));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::CorrectedCube,
	                       vlatFunctor1(& VisBuffer::fillVisCube,
	                                    VisibilityIterator::Corrected));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Direction1,
	                       vlatFunctor0 (& VisBuffer::fillDirection1));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Direction2,
	                       vlatFunctor0 (& VisBuffer::fillDirection2));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Exposure,
	                       vlatFunctor0 (& VisBuffer::fillExposure));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Feed1,
	                       vlatFunctor0 (& VisBuffer::fillFeed1));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Feed1_pa,
	                       vlatFunctor0 (& VisBuffer::fillFeed1_pa));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Feed2,
	                       vlatFunctor0 (& VisBuffer::fillFeed2));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Feed2_pa,
	                       vlatFunctor0 (& VisBuffer::fillFeed2_pa));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::FieldId,
	                       vlatFunctor0 (& VisBuffer::fillFieldId));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Flag,
	                       vlatFunctor0 (& VisBuffer::fillFlag));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::FlagCategory,
	                       vlatFunctor0 (& VisBuffer::fillFlagCategory));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::FlagCube,
	                       vlatFunctor0 (& VisBuffer::fillFlagCube));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::FlagRow,
	                       vlatFunctor0 (& VisBuffer::fillFlagRow));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Freq,
	                       vlatFunctor0 (& VisBuffer::fillFreq));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::ImagingWeight,
	                       vlatFunctor0 (& VisBuffer::fillImagingWeight));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::LSRFreq,
	                       vlatFunctor0 (& VisBuffer::fillLSRFreq));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Model,
	                       vlatFunctor1(& VisBuffer::fillVis,
	                                    VisibilityIterator::Model));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::ModelCube,
	                       vlatFunctor1(& VisBuffer::fillVisCube,
	                                    VisibilityIterator::Model));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::NChannel,
	                       vlatFunctor0 (& VisBuffer::fillnChannel));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::NCorr,
	                       vlatFunctor0 (& VisBuffer::fillnCorr));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::NRow,
	                       vlatFunctor0 (& VisBuffer::fillnRow));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::ObservationId,
	                       vlatFunctor0 (& VisBuffer::fillObservationId));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Observed,
	                       vlatFunctor1(& VisBuffer::fillVis,
	                                    VisibilityIterator::Observed));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::ObservedCube,
	                       vlatFunctor1(& VisBuffer::fillVisCube,
	                                    VisibilityIterator::Observed));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::PhaseCenter,
	                       vlatFunctor0 (& VisBuffer::fillPhaseCenter));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::PolFrame,
	                       vlatFunctor0 (& VisBuffer::fillPolFrame));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::ProcessorId,
	                       vlatFunctor0 (& VisBuffer::fillProcessorId));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Scan,
	                       vlatFunctor0 (& VisBuffer::fillScan));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Sigma,
	                       vlatFunctor0 (& VisBuffer::fillSigma));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::SigmaMat,
	                       vlatFunctor0 (& VisBuffer::fillSigmaMat));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::SpW,
	                       vlatFunctor0 (& VisBuffer::fillSpW));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::StateId,
	                       vlatFunctor0 (& VisBuffer::fillStateId));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Time,
	                       vlatFunctor0 (& VisBuffer::fillTime));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::TimeCentroid,
	                       vlatFunctor0 (& VisBuffer::fillTimeCentroid));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::TimeInterval,
	                       vlatFunctor0 (& VisBuffer::fillTimeInterval));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Uvw,
	                       vlatFunctor0 (& VisBuffer::filluvw));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::UvwMat,
	                       vlatFunctor0 (& VisBuffer::filluvwMat));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::Weight,
	                       vlatFunctor0 (& VisBuffer::fillWeight));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::WeightMat,
	                       vlatFunctor0 (& VisBuffer::fillWeightMat));
	fillerDictionary_p.add(ROVisibilityIteratorAsync::WeightSpectrum,
	                       vlatFunctor0 (& VisBuffer::fillWeightSpectrum));

    assert (fillerDictionary_p.size() == ROVisibilityIteratorAsync::N_PrefetchColumnIds);
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
            Log (1, "VLAT: Error while filling datum [%d] at 0x%08x, vb at 0x%08x; rethrowing\n",
                 datum->getId(), datum, datum->getVisBuffer());
        }
        else{
            Log (1, "VLAT: Error while filling datum [%d] at 0x%08x, vb at 0x%08x; "
                 "fillingColumn='%s'; rethrowing\n",
                 datum->getId(), datum, datum->getVisBuffer(),
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

	datum->getVisBuffer()->setRowIds (visibilityIterator_p->getRowIds());

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

}

void
VLAT::fillDatumMiscellanyBefore (VlaDatum * datum)
{
	datum->getVisBuffer()->setMeasurementSet (visibilityIterator_p->getMeasurementSet());
	datum->getVisBuffer()->setMeasurementSetId (visibilityIterator_p->getMeasurementSetId(),
			                                    datum->getSubChunkNumber() == 0);

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
	Log (1, "VLAT starting execution.\n");

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
		return NULL;

	}
	catch (std::exception & e){

		cerr << "VLAT thread caught exception: " << e.what() << endl;
		cerr.flush();

     	Log (1, "VLAT caught exception: %s.\n", e.what());

		threadTerminated_p = true;
		throw;
	}
	catch (...){

		cerr << "VLAT thread caught unknown exception: " << endl;
		cerr.flush();

     	Log (1, "VLAT caught unknown exception:\n");

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

	    ThrowIf (! contains (*c, fillerDictionary_p), format ("Unknown prefetch column id (%d)", *c));

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

            vlaData_p -> fillComplete ();

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
