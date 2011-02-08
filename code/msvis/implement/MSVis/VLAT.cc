/*
 * VLAT.cc
 *
 *  Created on: Nov 2, 2010
 *      Author: jjacobs
 */

#include <assert.h>

#include "VLAT.h"
#include "VisBufferAsync.h"
#include "VisibilityIteratorAsync.h"
#include <casa/System/AipsrcValue.h>


#include "AsynchronousTools.h"
using namespace casa::async;

#include <algorithm>
#include <functional>
#include <cstdarg>

using namespace std;

#include "UtilJ.h"

#define Log(level, ...) \
    {if (VlaData::loggingInitialized_p && level <= VlaData::logLevel_p) \
         Logger::log (__VA_ARGS__);};

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

Semaphore VlaData::debugBlockSemaphore_p (0); // used to block a thread for debugging
const Bool VlaData::loggingInitialized_p = initializeLogging();
Int VlaData::logLevel_p = 1;

VlaData::VlaData (Int nBuffers)
 : data_p (nBuffers, NULL)
{
    viResetRequested_p = False;
    viResetComplete_p = False;
}

VlaData::~VlaData ()
{
    Log (1, "VlaData dtor: nReadNoWaits=%d, nReadWaits=%d, nWriteNoWaits=%d, nWriteWaits=%d\n",
         stats_p.nReadNoWaits_p, stats_p.nReadWaits_p, stats_p.nWriteNoWaits_p, stats_p.nWriteWaits_p);

    for (Data::iterator d = data_p.begin(); d != data_p.end(); d++){
        delete (* d);
    }
}

void
VlaData::addModifier (RoviaModifier * modifier)
{
    MutexLocker ml (mutex_p);

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
    Log (1, "VlaData::debugBlock(): Blocked\n");

    debugBlockSemaphore_p.wait ();

    Log (1, "VlaData::debugBlock(): Unblocked\n");

}

void
VlaData::debugUnblock ()
{
    int v = debugBlockSemaphore_p.getValue();

    if (v == 0){
        Log (1, "VlaData::debugUnblock()\n");
        debugBlockSemaphore_p.post ();
    }
    else
        Log (1, "VlaData::debugUnblock(): already unblocked; v=%d\n", v);
}


void
VlaData::fillComplete ()
{
	Log (1, "VlaData::fillComplete on %d\n", fillIndex_p);

	assert (fillIndex_p >= 0 && fillIndex_p < (int) data_p.size());

	data_p [fillIndex_p]->fillComplete();

}

VlaDatum *
VlaData::fillStart (Int chunkNumber, Int subChunkNumber)
{

	VlaDatum * datum = getNextDatum (fillIndex_p);

	Log (1, "VlaData::fillStart on %d;\tsubchunk=(%d,%d)\n", fillIndex_p, chunkNumber, subChunkNumber);

	if (validChunks_p.empty() || validChunks_p.back() != chunkNumber)
	    insertValidChunk (chunkNumber);

	insertValidSubChunk (chunkNumber, subChunkNumber);

	Bool hadToWait = datum->fillStart (chunkNumber, subChunkNumber);

	if (hadToWait)
		++ stats_p.nWriteWaits_p;
	else
		++ stats_p.nWriteNoWaits_p;

	return datum;
}

asyncio::ChannelSelection
VlaData::getChannelSelection () const
{
    MutexLocker ml (mutex_p);

    return channelSelection_p;
}


VlaDatum *
VlaData::getNextDatum (Int & index)
{
	assert (index >= 0 && index < (int) data_p.size());

	MutexLocker m (mutex_p);

	index = clock (index + 1, data_p.size());

	VlaDatum * datum = data_p [index];

	return datum;
}

VlaData::Stats
VlaData::getStats () const
{
    return stats_p;
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
	                                               ROVisibilityIteratorAsync::getRcBase () + ".debug.logFile",
	                                               "");

	if (logFileFound && ! logFilename.empty() && downcase (logFilename) != "null" &&
	    downcase (logFilename) != "none"){

		Logger::start (logFilename.c_str());
		AipsrcValue<Int>::find (logLevel_p, ROVisibilityIteratorAsync::getRcBase () + ".debug.logLevel", 1);
		Logger::log ("VlaData log-level is %d; async I/O: %s; nBuffers=%d\n", logLevel_p,
		             ROVisibilityIteratorAsync::isAsynchronousIoEnabled() ? "enabled" : "disabled",
		             ROVisibilityIteratorAsync::getDefaultNBuffers() );

		return True;

	}

	return False;
}

void
VlaData::insertValidChunk (Int chunkNumber)
{
	MutexLocker m (mutex_p);

	validChunks_p.push (chunkNumber);

	validChunksCondition_p.broadcast ();
}

void
VlaData::insertValidSubChunk (Int chunkNumber, Int subChunkNumber)
{
	MutexLocker m (mutex_p);

	validSubChunks_p.push (SubChunkPair (chunkNumber, subChunkNumber));

	validChunksCondition_p.broadcast ();
}

Bool
VlaData::isValidChunk (Int chunkNumber) const
{
    bool validChunk = False;

    // Check to see if this is a valid chunk.  If the data structure is empty
    // then sleep for a tiny bit to allow the VLAT thread to either make more
    // chunks available for insert the sentinel value INT_MAX into the data
    // structure.

    MutexLocker ml (mutex_p);

    do {

        while (validChunks_p.empty()){
            validChunksCondition_p.wait (mutex_p);
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

    MutexLocker ml (mutex_p);

    do {

        while (validSubChunks_p.empty()){
            validChunksCondition_p.wait (mutex_p);
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
	Log (1, "VlaData::readComplete  on %d;\tsubchunk=(%d,%d)\n",
	     readIndex_p, data_p [readIndex_p]->getChunkNumber(),
	     data_p [readIndex_p]->getSubChunkNumber());

	assert (readIndex_p >= 0 && readIndex_p < (int) data_p.size());

	data_p [readIndex_p]->readComplete();

}

const VlaDatum *
VlaData::readStart ()
{
	VlaDatum * datum = getNextDatum (readIndex_p);

	Log (2, "VlaData::readStart on %d\n", readIndex_p);

	bool hadToWait = datum->readStart ();

	if (hadToWait)
		++ stats_p.nReadWaits_p;
	else
		++ stats_p.nReadNoWaits_p;

	return datum;
}

void
VlaData::requestViReset (VLAT * vlat)
{
    MutexLocker ml (mutex_p);  // enter critical section

    Log (1, "Requesting VI reset\n");

    viResetRequested_p = True; // officially request the reset
    viResetComplete_p = False; // clear any previous completions

    vlat->requestFillTermination (); // ask VLAT to stop filling

    terminateFill (True); // unblock VLAT if waiting for a buffer to free up

    viResetRequestCondition_p.broadcast (); // wake filling thread if
                                            // already waiting for reset

    // Wait for the request to be completed.

    Log (1, "Waiting for requesting VI reset\n");

    while (! viResetComplete_p){
        viResetCompleteCondition_p.wait (mutex_p);
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
    insertValidChunk (INT_MAX);
    insertValidSubChunk (INT_MAX, INT_MAX);
}

void
VlaData::storeChannelSelection (const asyncio::ChannelSelection & channelSelection)
{
    MutexLocker ml (mutex_p);

    channelSelection_p = channelSelection;
}


void
VlaData::terminateFill (bool alreadyLocked)
{
    // Make sure that the fill operation is not
    // blocking on the semaphore.  This should only be
    // done when the filler is expected to quickly terminate.
    // If misused it will destroy the concurrency semantics.

    if (! alreadyLocked){
        mutex_p.lock();
    }

    // Terminate filling on all of the buffers so that if the
    // VLAT is waiting to fill one it will awaken and detect
    // the termination of fill operations.

    for (uint i = 0; i < data_p.size(); i++)
        data_p [i] -> terminateFill ();

    if (! alreadyLocked){
        mutex_p.unlock();
    }
}

void
VlaData::terminateReset ()
{
    viResetRequestCondition_p.broadcast ();
}



Bool
VlaData::waitForViReset(VLAT * vlat)
{
    MutexLocker ml (mutex_p);

    Log (1, "Waiting for VI reset request\n");

    while (! viResetRequested_p && ! vlat->terminationRequested()){
        viResetRequestCondition_p.wait (mutex_p);
    }

    if (vlat->terminationRequested()){
        Log (1, "Terminating VLAT during wait for VI reset.\n")
        return False;
    }
    else {
        Log (1, "Carrying out VI reset\n")
        viResetRequested_p = False;

        resetBufferRing ();

        vlat->setModifiers (roviaModifiers_p); // gives possession to vlat

        viResetComplete_p = True;
        viResetCompleteCondition_p.broadcast();
        vlat->clearFillTerminationRequest ();
        return True;
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
	readyToFillSemaphore_p = NULL;
	readyToReadSemaphore_p = NULL;
	state_p = Empty;
	terminating_p = False;
    visBuffer_p = NULL;
}

VlaDatum::~VlaDatum()
{
    delete readyToFillSemaphore_p;
    delete readyToReadSemaphore_p;
    delete visBuffer_p;
}

void
VlaDatum::fillComplete ()
{
	assert (state_p == Filling);
	assert (terminating_p || readyToFillSemaphore_p->getValue() == 0);
	assert (terminating_p || readyToReadSemaphore_p->getValue() == 0);

	state_p = Full;

	readyToReadSemaphore_p->post ();
}

Bool
VlaDatum::fillStart (Int chunkNumber, Int subChunkNumber)
{
	Bool hadToWait = readyToFillSemaphore_p->getValue() == 0;

	readyToFillSemaphore_p->wait();

	assert (terminating_p || state_p == Empty);
	assert (terminating_p || readyToReadSemaphore_p->getValue() == 0);

	state_p = Filling;

	chunkNumber_p = chunkNumber;
	subChunkNumber_p = subChunkNumber;

	return hadToWait;
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
    readyToFillSemaphore_p = new Semaphore (1);
    readyToReadSemaphore_p = new Semaphore ();

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
	assert (readyToFillSemaphore_p->getValue() == 0);
	assert (readyToReadSemaphore_p->getValue() == 0);

	state_p = Empty;
	readyToFillSemaphore_p->post ();
}

Bool
VlaDatum::readStart ()
{
	Bool hadToWait = readyToReadSemaphore_p->getValue() == 0;

	readyToReadSemaphore_p->wait ();

	assert (state_p == Full);
	assert (readyToFillSemaphore_p->getValue() == 0);

	state_p = Reading;

	return hadToWait;
}

void
VlaDatum::reset ()
{
    terminating_p = False;
	chunkNumber_p = -1;
	state_p = Empty;
	subChunkNumber_p = -1;
	visBuffer_p->clear();

	// Adjust semaphores so that they indicate a
	// readiness to be filled buy an unreadiness to be read
	// ====================================================

	if (readyToFillSemaphore_p->getValue() == 0)
	    readyToFillSemaphore_p->post(); // set it

	// During fill termination an extra post might occur so to keep the
	// semaphore binary adjust it back down to one.

	while (readyToFillSemaphore_p->getValue() > 1){
	    readyToFillSemaphore_p->wait(); // clear one
	}


	while (readyToReadSemaphore_p->getValue() >= 1){
	    readyToReadSemaphore_p->wait(); // clear it
	}

}

void
VlaDatum::terminateFill ()
{
    terminating_p = True;
    readyToFillSemaphore_p -> post ();
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
	fillTerminationRequested_p = False;
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
VLAT::clearFillTerminationRequest ()
{
    fillTerminationRequested_p = false;
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

    try{
        VisBufferAsync * vb = datum->getVisBuffer();
        vb->clear();
        vb->setFilling (true);
        vb->attachToVisIter (* visibilityIterator_p); // invalidates vb's cache as well

        fillDatumMiscellanyBefore (datum);

        for (Fillers::iterator filler = fillers_p.begin(); filler != fillers_p.end(); filler ++){

            if (terminationRequested())
                break;

            //Log (2, "Filler id=%d name=%s starting\n", (* filler)->getId(), ROVisibilityIteratorAsync::prefetchColumnName((* filler)->getId()).c_str());

            (** filler) (vb);
        }

        fillDatumMiscellanyAfter (datum);

        vb->setFilling (false);

        vb->detachFromVisIter ();
    }
    catch (AipsError & e){

        Log (1, "VLAT: AIPS error while filling datum [%d] at 0x%08x, vb at 0x%08x; rethrowing\n",
             datum->getId(), datum, datum->getVisBuffer());

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

Bool
VLAT::fillTerminationRequested () const
{
    return fillTerminationRequested_p || terminationRequested();
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

void
VLAT::requestFillTermination ()
{
    fillTerminationRequested_p = true;
}

void *
VLAT::run ()
{
	Log (1, "VLAT starting execution.\n");

    if (VlaData::loggingInitialized_p){
        Logger::registerName ("VLAT");
    }

	try {

	    do{

	        Log (1, "VLAT starting VI sweep\n");

	        // Start sweeping the real VI over its entire range
	        // (subject to the number of free buffers).  The sweep
	        // can be ended abruptly if

            sweepVi ();

            if (terminationRequested()){
                break;
            }

            Bool resetVi = vlaData_p->waitForViReset (this);

            if (! resetVi){
                break; // time to quit
            }

	    } while (True);

		threadTerminated_p = true;
     	Log (1, "VLAT stopping execution.\n");
		return NULL;

	} catch (AipsError & e){

		cerr << "VLAT thread caught exception: " << e.what() << endl;
		cerr.flush();

     	Log (1, "VLAT caught exception: %s.\n", e.what());

		threadTerminated_p = true;
		throw;
	}
}

void
VLAT::setModifiers (RoviaModifiers & modifiers)
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

            if (fillTerminationRequested ())
                goto done;

            VlaDatum * vlaDatum = vlaData_p -> fillStart (chunkNumber, subChunkNumber);

            if (fillTerminationRequested ())
                goto done;

            Log (2, "Filling datum with subchunk (%d,%d)\n", chunkNumber, subChunkNumber);

            fillDatum (vlaDatum);

            if (fillTerminationRequested ())
                goto done;

            vlaData_p -> fillComplete ();

            if (fillTerminationRequested ())
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
    Log (2, "Terminating VLAT\n");
    //printBacktrace (cerr, "VLAT termination");
    Thread::terminate(); // ask thread to terminate

    vlaData_p->terminateFill (false); // unblock the thread
    vlaData_p->terminateReset ();

}

//void
//VLAT::FillerDependencies::add (ROVisibilityIteratorAsync::PrefetchColumnIds column,
//                               ROVisibilityIteratorAsync::PrefetchColumnIds requiresColumn1, ...)
//{
//    va_list vaList;
//
//    va_start (vaList, requiresColumn1);
//
//    Int id = requiresColumn1;
//    set<ROVisibilityIteratorAsync::PrefetchColumnIds> requiredColumns;
//
//    while (id >= 0 && id < ROVisibilityIteratorAsync::N_PrefetchColumnIds){
//        requiredColumns.insert ((ROVisibilityIteratorAsync::PrefetchColumnIds) id);
//        id = va_arg (vaList, Int);
//    }
//
//    va_end (vaList);
//
//    insert (make_pair (column, requiredColumns));
//}

//void
//VLAT::FillerDependencies::checkForCircularDependence ()
//{
//    for (iterator i = begin(); i != end (); i++){
//        for (iterator j = i; j != end(); j++){
//
//            if (i != j &&
//                contains (i->first, j->second) &&
//                contains (j->first, i->second)){
//
//                Log (1, "VLAT:: Detected circular dependency between %s and %s!\n",
//                     ROVisibilityIteratorAsync::prefetchColumnName(i->first).c_str(),
//                     ROVisibilityIteratorAsync::prefetchColumnName(j->first).c_str());
//                Assert (False);
//            }
//        }
//    }
//}

//Bool
//VLAT::FillerDependencies::isRequiredBy (ROVisibilityIteratorAsync::PrefetchColumnIds requiree,
//                                        ROVisibilityIteratorAsync::PrefetchColumnIds requirer) const
//{
//    const_iterator bDependencies = find (requirer);
//    Bool result = False;
//
//    if (bDependencies != end()){
//        result = utilj::contains (requiree, bDependencies->second);
//    }
//
//    return result;
//}

//Bool
//VLAT::FillerDependencies::isRequiredByAny (ROVisibilityIteratorAsync::PrefetchColumnIds requiree,
//                                           const set<ROVisibilityIteratorAsync::PrefetchColumnIds> & requirers) const
//{
//    typedef const set<ROVisibilityIteratorAsync::PrefetchColumnIds> Set;
//
//    Bool result = false;
//
//    // See if any of the members of the requirers need the requiree.
//
//    for (Set::const_iterator requirer = requirers.begin(); requirer != requirers.end(); requirer ++){
//        result = result || isRequiredBy (requiree, * requirer);
//    }
//
//    return result;
//}

//void
//VLAT::FillerDependencies::performTransitiveClosure (FillerDictionary & dictionary)
//{
//    typedef set<ROVisibilityIteratorAsync::PrefetchColumnIds> DependencySet;
//    bool workToBeDone = true;
//
//    while (workToBeDone){
//
//        workToBeDone = false; // assume done until proven otherwise
//
//        // Loop through all items having dependencies
//
//        for (iterator i = begin(); i != end(); i++){
//
//            DependencySet & iDependencies = i->second;
//            DependencySet newDependencies = iDependencies; // working copy
//
//            for (DependencySet::iterator j = iDependencies.begin(); j != iDependencies.end(); j++){
//
//                // If an item in i's dependencies, j, has dependencies then add them to i's dependencies
//
//                iterator jDependencies = find (*j);
//
//                if (jDependencies != end()){
//                    newDependencies.insert (jDependencies->second.begin(), jDependencies->second.end());
//                }
//            }
//
//            // If i's dependencies has changed then there's potentially more work to be done
//
//            workToBeDone = workToBeDone || iDependencies != newDependencies;
//
//            iDependencies = newDependencies;
//        }
//        checkForCircularDependence (); // could happen
//    }
//
//    dictionary.setPrecedences (* this);
//}

//void
//VLAT::FillerDictionary::setPrecedences (const FillerDependencies & dependencies)
//{
//    // Using the dependencies between the different fillers, compute a precedence
//    // for each filler.  A numerically higher precedence level means a higher
//    // precedence filler.  Higher precedence filler will be applied first.
//
//    typedef set<ROVisibilityIteratorAsync::PrefetchColumnIds> PrecedenceLevel;
//    vector<PrecedenceLevel> levels (1);
//
//    // Put everything into level 0 to start with
//
//    for (iterator i = begin(); i != end(); i++){
//        levels[0].insert (i->first);
//    }
//
//    // Using dependencies, create the next higher precedence level
//    // based on the previous one.  Higher precedence fillers will
//    // be applied first.
//
//    Int level = 0;
//    Bool workToBeDone = True;
//
//    while (workToBeDone){
//
//        PrecedenceLevel plPlus; // next higher predence level is initially empty
//        PrecedenceLevel & pl = levels [level]; // current highest precedence level
//        PrecedenceLevel plNew = pl; // interim revised copy of the current level
//
//        for (PrecedenceLevel::iterator i = pl.begin(); i != pl.end(); i ++){
//            if (dependencies.isRequiredByAny (* i, pl)){
//                plPlus.insert (* i);  // into the next level
//                plNew.erase (* i);    // out of the current level
//            }
//        }
//
//        if (! plPlus.empty()){
//
//            // The new predence level is not empty: install it
//            // and the updated previous level and flag that more work
//            // is required.
//
//            workToBeDone = True;
//
//            levels.push_back (plPlus);
//            pl = plNew;
//            level ++;
//        }
//    }
//
//    // Now apply the precedences.
//
//    for (uint level = 0; level < levels.size(); level ++){
//        for (PrecedenceLevel::iterator i = levels [level].begin(); i != levels [level].end(); i++){
//            find (*i)->second->setPrecedence (level);
//        }
//    }
//}


} // end namespace casa
