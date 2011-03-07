//# Applicator.h: interface to parallelization infrastructure
//# Copyright (C) 1999,2000
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
//#
//# $Id$

#ifndef SYNTHESIS_APPLICATOR_H
#define SYNTHESIS_APPLICATOR_H

//# Includes

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <lattices/Lattices/Lattice.h>
#include <casa/Containers/OrderedMap.h>
#include <casa/Containers/Record.h>
#include <synthesis/Parallel/PTransport.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class Algorithm;

// <summary>
// Class which provides an interface to the parallelization infrastructure
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> Algorithm
//   <li> PTransport
// </prerequisite>
//
// <etymology>
// Applies or controls the execution of parallelized algorithms.
// </etymology>
//
// <synopsis>
// The Applicator class provides the interface to parallel communication.
// It holds the parallel transport layer, and controls the execution of
// parallelized algorithms.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// To provide a simple programming interface to parallelization, and
// to encapsulate the transport layer and parallel process control.
// </motivation>
//
//# <thrown>
//#    <li>
//#    <li>
//# </thrown>
//
//# <todo asof="1999/12/21">
//#   <li> Document
//# </todo>


class Applicator {
 public:
  // Enum to define the process status table
  enum Status {FREE, ASSIGNED};

  // Recognized signals
  enum Signals {STOP = 0, DONE = -1};

  // Default constructor, and destructor
  Applicator();
  ~Applicator();

  // Initialization (includes parallel transport initialization)
  void init(Int argc, Char *argv[]);
  void initThreads(Int argc, Char *argv[]);
  void initThreads();

  // define an Algorithm if we need too;
  void defineAlgorithm(Algorithm *);

  // Status functions to indicate whether this Applicator is
  // executing as a controller or worker process
  Bool isController();
  Bool isWorker();

  // True if executing serially
  Bool isSerial() {return serial;};

  // Return the number of processes
  Int numProcs() {return nProcs;};

  // Assign the next free worker process to a specified Algorithm
  Bool nextAvailProcess(Algorithm &a, Int &rank);

  // Return the rank of the next process to complete the specified Algorithm
  Int nextProcessDone(Algorithm &a, Bool &allDone);

  // Signal that a worker process is done
  void done();

  // Execute an algorithm directly
  void apply(Algorithm &a);

  // Put and get methods to be executed on the parallel transport layer
  Int put(const Array<Float> &an) {return comm->put(an);};
  Int put(const Array<Double> &an) {return comm->put(an);};
  Int put(const Array<Int> &an) {return comm->put(an);};
  Int put(const Array<Complex> &an) {return comm->put(an);};
  Int put(const Array<DComplex> &an) {return comm->put(an);};
  Int put(const Float &n) {return comm->put(n);};
  Int put(const Complex &n) {return comm->put(n);};
  Int put(const DComplex &n) {return comm->put(n);};
  Int put(const Double &n) {return comm->put(n);};
  Int put(const Int &n) {return comm->put(n);};
  Int put(const Bool &b) {return comm->put(b);};
  Int put(const String &s) {return comm->put(s);};
  Int put(const Record &r) {return comm->put(r);};

  Int get(Array<Float> &an) {return comm->get(an);};
  Int get(Array<Double> &an) {return comm->get(an);};
  Int get(Array<Complex> &an) {return comm->get(an);};
  Int get(Array<DComplex> &an) {return comm->get(an);};
  Int get(Array<Int> &an) {return comm->get(an);};
  Int get(Float &n) {return comm->get(n);};
  Int get(Double &n) {return comm->get(n);};
  Int get(Complex &n) {return comm->get(n);};
  Int get(DComplex &n) {return comm->get(n);};
  Int get(Int &n) {return comm->get(n);};
  Int get(Bool &b) {return comm->get(b);};
  Int get(String &s) {return comm->get(s);};
  Int get(Record &r) {return comm->get(r);};

 private:
  // Pointer to the parallel transport
  PTransport *comm;

  // Map of known algorithm names and id.'s
  OrderedMap<String, Int> algorithmIds;
  OrderedMap<Int, Algorithm*> knownAlgorithms;

  // ID for the last Algorithm defined.
  Int LastID;

  // True if no more processes are free
  Bool usedAllThreads;

  // True if executing in serial
  Bool serial;

  // Number of processes
  Int nProcs;

  // Process status list
  Vector<Int> procStatus;

  // Executed by worker process waiting for an assigned task
  void loop();

  // Fill algorithm map
  void defineAlgorithms();

  // Utility functions for the current list of processes, and their status
  void setupProcStatus();
  Int findFreeProc(Bool &lastOne);
};


} //# NAMESPACE CASA - END

#endif




