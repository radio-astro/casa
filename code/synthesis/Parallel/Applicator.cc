//# Applicator.cc: Implementation of Applicator.h
//# Copyright (C) 1999,2000,2002
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

#include <casa/Utilities/Assert.h>
#include <synthesis/Parallel/Applicator.h>
#include <synthesis/Parallel/Algorithm.h>
#include <synthesis/MeasurementComponents/ClarkCleanAlgorithm.h>
#include <synthesis/MeasurementComponents/ReadMSAlgorithm.h>
#include <synthesis/MeasurementComponents/MakeApproxPSFAlgorithm.h>
#include <synthesis/MeasurementComponents/PredictAlgorithm.h>
#include <synthesis/MeasurementComponents/ResidualAlgorithm.h>
#include <casa/BasicMath/Math.h>
#include <synthesis/Parallel/MPIError.h>
#ifdef PABLO_IO
#include <synthesis/Parallel/PabloIO.h>
#endif

namespace casa { //# NAMESPACE CASA - BEGIN

Applicator::Applicator() : comm(0), algorithmIds(0),
  knownAlgorithms((Algorithm*)0), LastID(101), usedAllThreads(False),
  serial(True), nProcs(0), procStatus(0)
{
// Default constructor; requires later init().
}

Applicator::~Applicator()
{
// Default destructor
//  
  if (comm) {
    // If controller, then stop all worker processes
    if (isController()) {
      comm->setTag(STOP);
      for (Int i=0; i<nProcs; i++) {
	if (i != comm->controllerRank()) {
	  comm->connect(i);
	  put(STOP);
	}
      }
    }
    delete comm;
  }
}

void Applicator::initThreads(Int /*argc*/, Char */*argv*/[]){

   // A no-op if not using MPI
#ifdef HasMPI

  // Initialize the MPI transport layer
  try {
     comm = new MPITransport(argc, argv);

     // Initialize the process status list
#ifdef PABLO_IO
     PabloIO::init(argc, argv, comm->cpu());
#endif
     setupProcStatus();

     // If controller then exit, else loop, waiting for an assigned task
     if (isWorker()) {
       loop();
     }
  } catch (MPIError x) {
    cerr << x.getMesg() << endl;
    initThreads();
  } 

#endif

  return;
}

   // Serial transport all around.
void Applicator::initThreads(){
     // Initialize a serial transport layer
  comm = new SerialTransport();
     // Initialize the process status list
  setupProcStatus();
  return;
}

void Applicator::init(Int /*argc*/, Char */*argv*/[])
{
// Initialize the process and parallel transport layer
//
  // Fill the map of known algorithms
  defineAlgorithms();

#ifdef HasMPI
  initThreads(argc, argv);
#else
#ifdef PABLO_IO
     PabloIO::init(argc, argv, 0);
#endif
  initThreads();
#endif
  return;
}

Bool Applicator::isController()
{
// Return T if the current process is the controller
//
  Bool result;
  if (comm) {
    result = comm->isController();
  } else {
    throw(AipsError("Parallel transport layer not initialized"));
  }
  return result;
}

Bool Applicator::isWorker()
{
// Return T if the current process is a worker process
//
  Bool result;
  if (comm) {
    result = comm->isWorker();
  } else {
    throw(AipsError("Parallel transport layer not initialized"));
  }
  return result;
}

void Applicator::loop()
{
// Loop, if a worker process, waiting for an assigned task
//
  Bool die(False);
  Int what;
  // Wait for a message from the controller with any Algorithm tag
  while(!die){
    comm->connectToController();
    comm->setAnyTag();
    comm->get(what);
    switch(what){
    case STOP :
      die = True;
      break;
    default :
      // In this case, an Algorithm tag is expected.
      // First check that it is known.
      if (knownAlgorithms.isDefined(what)) {
	// Identified algorithm tag; set for subsequent communication
	comm->setTag(what);
	// Execute (apply) the algorithm
	knownAlgorithms(what)->apply();
      } else {
	throw(AipsError("Unidentified parallel algorithm code"));
      }
      break;
    }
  }
  return;
}

Bool Applicator::nextAvailProcess(Algorithm &a, Int &rank)
{
// Assign the next available process for the specified Algorithm
//  
  // Must be the controller to request a worker process
  Bool assigned;
  if (isWorker()) {
    throw(AipsError("Must be the controller to assign a worker process"));
  } else {
    if (!usedAllThreads) {
      // Connect to the next available process in the list
      Bool lastOne;
      rank = findFreeProc(lastOne);
      AlwaysAssert(rank >= 0, AipsError);
      if (lastOne) usedAllThreads = True;
      Int tag = algorithmIds(a.name());
      
      // Send wake-up message (containing the Algorithm tag) to
      // the assigned worker process to activate it (see loop()).
      comm->connect(rank);
      comm->setTag(tag);
      put(tag);
      assigned = True;
      procStatus(rank) = ASSIGNED;
    } else {
      assigned = False;
    }
  }
  return assigned;
}

Int Applicator::nextProcessDone(Algorithm &a, Bool &allDone)
{
// Return the rank of the next process to complete the specified algorithm
//
  Int rank = -1;
  allDone = True;
  for (uInt i=0; i<procStatus.nelements(); i++) {
    if (procStatus(i) == ASSIGNED) {
      if (isSerial()) {
	// In the serial case, the controller can be assigned
	allDone = False;
      } else {
	// In the parallel case, the controller is not assigned
	if (i != static_cast<uInt>(comm->controllerRank())) {
	  allDone = False;
	}
      }
    }
  }
  if (!allDone) {
    // Wait for a process to finish with the correct algorithm tag
    comm->connectAnySource();
    Int tag = algorithmIds(a.name());
    comm->setTag(tag);
    Int doneSignal;
    rank = get(doneSignal);
    // Consistency check; should return a DONE signal to contoller
    // on completion.
    if (doneSignal != DONE) {
      throw(AipsError("Worker process terminated unexpectedly"));
    } else {
      // Set source in parallel transport layer
      comm->connect(rank);
      // Mark process as free
      procStatus(rank) = FREE;
      usedAllThreads = False;
    }
  }
  return rank;
}

void Applicator::done()
{
// Signal that a worker process is done
//
  put(DONE);
  return;
}

void Applicator::apply(Algorithm &a)
{
// Execute an algorithm directly
//
  // Null operation unless serial, in which case the 
  // controller needs to execute the algorithm directly.
  // In the parallel case, the algorithm applies are
  // performed in workers processes' applicator.init().
  if (isSerial() && isController()) {
    a.apply();
  }
  return;
}

void Applicator::defineAlgorithm(Algorithm *a)
{
   knownAlgorithms.define(LastID, a);
   algorithmIds.define(a->name(), LastID);
   LastID++;
   return;
}

void Applicator::defineAlgorithms()
{
// Fill the algorithm map
//
  // Clark CLEAN parallel deconvolution
  Algorithm *a1 = new ClarkCleanAlgorithm;
  knownAlgorithms.define(LastID, a1);
  algorithmIds.define(a1->name(), LastID);
  LastID++;
  Algorithm *a2 = new ReadMSAlgorithm;
  knownAlgorithms.define(LastID, a2);
  algorithmIds.define(a2->name(), LastID);
  LastID++;
  Algorithm *a3 = new MakeApproxPSFAlgorithm;
  knownAlgorithms.define(LastID, a3);
  algorithmIds.define(a3->name(), LastID);
  LastID++;
  Algorithm *a4 = new PredictAlgorithm;
  knownAlgorithms.define(LastID, a4);
  algorithmIds.define(a4->name(), LastID);
  LastID++;
  Algorithm *a5 = new ResidualAlgorithm;
  knownAlgorithms.define(LastID, a5);
  algorithmIds.define(a5->name(), LastID);
  LastID++;
  return;
}

void Applicator::setupProcStatus()
{
// Set up the process status list
//
  nProcs = comm->numThreads();
  if (nProcs <= 1) {
    serial = True;
  } else {
    serial = False;
  }
  // Resize the process list, and mark as unassigned (except for controller)
  usedAllThreads = False;
  procStatus.resize(max(nProcs,1));
  procStatus = FREE;
  // In the parallel case, the controller is never assigned
  if (!isSerial()) procStatus(comm->controllerRank()) = ASSIGNED;
  return;
}

Int Applicator::findFreeProc(Bool &lastOne)
{
// Search the process status list for the next free process
// 
  Int freeProc = -1;
  Int nfree = 0;
  for (uInt i=0; i<procStatus.nelements(); i++) {
    if (procStatus(i) == FREE) {
      nfree++;
      if (freeProc < 0) freeProc = i;
    }
  }
  lastOne = (nfree==1);
  return freeProc;
}

// The applicator is ominpresent.
// Moved here for shared libraries.
Applicator applicator;


} //# NAMESPACE CASA - END

