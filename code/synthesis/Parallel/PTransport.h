//# PTransport.h: Base class for parallel data transport models
//# Copyright (C) 1998,1999,2000
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

#ifndef SYNTHESIS_PTRANSPORT_H
#define SYNTHESIS_PTRANSPORT_H

//# Includes
#include <casa/aips.h>
#include <casa/Arrays/Array.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class Algorithm;
class Record;

// <summary>
// Base class for parallel data transport models
// </summary>

// <use visibility=local>   or   <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
//
//# <todo asof="yyyy/mm/dd">
//#   <li> add this feature
//#   <li> fix this bug
//#   <li> start discussion of this possible extension
//# </todo>


class PTransport {
 public:
  // Default constructor and destructor
  PTransport() : numprocs(0), myCpu(0), aWorker(0), aTag(0) {};
  virtual ~PTransport() {}

  // Return the number of processes
  Int numThreads() {return numprocs;};

  // Return the current process rank
  Int cpu() {return myCpu;}

  // Set the properties of the current connection including
  // source/destination and message tag.
  Int connect(Int i) {aWorker=i; return i;}
  void connectAnySource() {aWorker=anySource(); return;};
  void connectToController() {aWorker=controllerRank(); return;};
  void setTag(Int tag) {aTag=tag; return;};
  void setAnyTag() {aTag=anyTag(); return;};
  
  // Status functions for worker/controller designation
  Bool isController() {return (cpu()==controllerRank());};
  Bool isWorker() {return (cpu()!=controllerRank());};
  
  // Default source and message tag values
  virtual Int anyTag() = 0;
  virtual Int anySource() = 0;
  
  // Define the rank of the controller process
  virtual Int controllerRank() = 0;
  
  // Get and put functions on the parallel data transport layer
  virtual Int put(const Array<Float> &) = 0;
  virtual Int put(const Array<Double> &) = 0;
  virtual Int put(const Array<Complex> &) = 0;
  virtual Int put(const Array<DComplex> &) = 0;
  virtual Int put(const Array<Int> &) = 0;
  virtual Int put(const Float &) = 0;
  virtual Int put(const Double &) = 0;
  virtual Int put(const Complex &) = 0;
  virtual Int put(const DComplex &) = 0;
  virtual Int put(const Int &) = 0;
  virtual Int put(const String &) = 0;
  virtual Int put(const Bool &) = 0;
  virtual Int put(const Record &) = 0;

  virtual Int get(Array<Float> &) = 0;
  virtual Int get(Array<Double> &) = 0;
  virtual Int get(Array<Complex> &) = 0;
  virtual Int get(Array<DComplex> &) = 0;
  virtual Int get(Array<Int> &) = 0;
  virtual Int get(Float &) = 0;
  virtual Int get(Double &) = 0;
  virtual Int get(Complex &) = 0;
  virtual Int get(DComplex &) = 0;
  virtual Int get(Int &) = 0;
  virtual Int get(String &) = 0;
  virtual Int get(Bool &) = 0;
  virtual Int get(Record &) = 0;
  
 protected: 
  // Number of processes
  Int numprocs;
  
  // Rank of current process
  Int myCpu;
   
  // Current source or destination process
  Int aWorker;

  // Current message tag
  Int aTag;
};

// Putting in the MPI Transport stuff only makes sense if we have MPI

#ifdef HasMPI
// <summary>
// MPI data transport models
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
//
//# <todo asof="yyyy/mm/dd">
//#   <li> add this feature
//#   <li> fix this bug
//#   <li> start discussion of this possible extension
//# </todo>

class MPITransport : public PTransport {
 public:
  // Default constructor and destructor
  MPITransport();
  virtual ~MPITransport();

  // Construct from argv
  MPITransport(Int, Char *argv[]);

  // Default source and message tag values
  virtual Int anyTag();
  virtual Int anySource();
  
  // Define the rank of the controller process
  virtual Int controllerRank() {return 0;};
  
  // Get and put functions on the parallel data transport layer
  virtual Int put(const Array<Float> &);
  virtual Int put(const Array<Double> &);
  virtual Int put(const Array<Complex> &);
  virtual Int put(const Array<DComplex> &);
  virtual Int put(const Array<Int> &);
  virtual Int put(const Float &);
  virtual Int put(const Double &);
  virtual Int put(const Complex &);
  virtual Int put(const DComplex &);
  virtual Int put(const Int &);
  virtual Int put(const String &);
  virtual Int put(const Bool &);
  virtual Int put(const Record &);

  virtual Int get(Array<Float> &);
  virtual Int get(Array<Double> &);
  virtual Int get(Array<Complex> &);
  virtual Int get(Array<DComplex> &);
  virtual Int get(Array<Int> &);
  virtual Int get(Float &);
  virtual Int get(Double &);
  virtual Int get(Complex &);
  virtual Int get(DComplex &);
  virtual Int get(Int &);
  virtual Int get(String &);
  virtual Int get(Bool &);
  virtual Int get(Record &);

 private:
  // Local work variables
  Int sendTo, myOp, getFrom;

  // Utility functions to set default source/destination and tag values
  void setSourceAndTag (Int &source, Int &tag);
  void setDestAndTag (Int &dest, Int &tag);
};
#endif

// <summary>
// Serial Data Transport Model
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
//
//# <todo asof="yyyy/mm/dd">
//#   <li> add this feature
//#   <li> fix this bug
//#   <li> start discussion of this possible extension
//# </todo>

// SerialTransport is your basic no-op.  We're just passing the pointers
// to avoid unnecessary data copying.

class SerialTransport : public PTransport {
 public:
  // Default constructor and destructor
  SerialTransport() : PTransport(), inQue(0), outQue(0), lastInQue(0)
    {_data.resize(20);}
  virtual ~SerialTransport(){}

  // Default source and message tag values
  virtual Int anyTag() {return -1;};
  virtual Int anySource() {return -1;};
  
  // Define the rank of the controller process
  virtual Int controllerRank() {return 0;};

  // Get and put functions on the data transport layer
  virtual Int put(const Array<Float> &);
  virtual Int put(const Array<Double> &);
  virtual Int put(const Array<Complex> &);
  virtual Int put(const Array<DComplex> &);
  virtual Int put(const Array<Int> &);
  virtual Int put(const Float &);
  virtual Int put(const Double &);
  virtual Int put(const Complex &);
  virtual Int put(const DComplex &);
  virtual Int put(const Int &);
  virtual Int put(const String &);
  virtual Int put(const Bool &);
  virtual Int put(const Record &);

  virtual Int get(Array<Float> &);
  virtual Int get(Array<Double> &);
  virtual Int get(Array<Complex> &);
  virtual Int get(Array<DComplex> &);
  virtual Int get(Array<Int> &);
  virtual Int get(Float &);
  virtual Int get(Double &);
  virtual Int get(Complex &);
  virtual Int get(DComplex &);
  virtual Int get(Int &);
  virtual Int get(String &);
  virtual Int get(Bool &);
  virtual Int get(Record &);

 private:
  uInt inQue;
  uInt outQue;
  uInt lastInQue;
  PtrBlock<void *> _data;
  
  Int add2Queue(void *);
  void *getFromQueue();
};


} //# NAMESPACE CASA - END

#endif


