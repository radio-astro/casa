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

namespace casacore{

class Record;
}

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class Algorithm;

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
  casacore::Int numThreads() {return numprocs;};

  // Return the current process rank
  casacore::Int cpu() {return myCpu;}

  // Set the properties of the current connection including
  // source/destination and message tag.
  casacore::Int connect(casacore::Int i) {aWorker=i; return i;}
  void connectAnySource() {aWorker=anySource(); return;};
  void connectToController() {aWorker=controllerRank(); return;};
  void setTag(casacore::Int tag) {aTag=tag; return;};
  void setAnyTag() {aTag=anyTag(); return;};
  
  // Status functions for worker/controller designation
  casacore::Bool isController() {return (cpu()==controllerRank());};
  casacore::Bool isWorker() {return (cpu()!=controllerRank());};
  
  // Default source and message tag values
  virtual casacore::Int anyTag() = 0;
  virtual casacore::Int anySource() = 0;
  
  // Define the rank of the controller process
  virtual casacore::Int controllerRank() = 0;
  
  // Get and put functions on the parallel data transport layer
  virtual casacore::Int put(const casacore::Array<casacore::Float> &) = 0;
  virtual casacore::Int put(const casacore::Array<casacore::Double> &) = 0;
  virtual casacore::Int put(const casacore::Array<casacore::Complex> &) = 0;
  virtual casacore::Int put(const casacore::Array<casacore::DComplex> &) = 0;
  virtual casacore::Int put(const casacore::Array<casacore::Int> &) = 0;
  virtual casacore::Int put(const casacore::Float &) = 0;
  virtual casacore::Int put(const casacore::Double &) = 0;
  virtual casacore::Int put(const casacore::Complex &) = 0;
  virtual casacore::Int put(const casacore::DComplex &) = 0;
  virtual casacore::Int put(const casacore::Int &) = 0;
  virtual casacore::Int put(const casacore::String &) = 0;
  virtual casacore::Int put(const casacore::Bool &) = 0;
  virtual casacore::Int put(const casacore::Record &) = 0;

  virtual casacore::Int get(casacore::Array<casacore::Float> &) = 0;
  virtual casacore::Int get(casacore::Array<casacore::Double> &) = 0;
  virtual casacore::Int get(casacore::Array<casacore::Complex> &) = 0;
  virtual casacore::Int get(casacore::Array<casacore::DComplex> &) = 0;
  virtual casacore::Int get(casacore::Array<casacore::Int> &) = 0;
  virtual casacore::Int get(casacore::Float &) = 0;
  virtual casacore::Int get(casacore::Double &) = 0;
  virtual casacore::Int get(casacore::Complex &) = 0;
  virtual casacore::Int get(casacore::DComplex &) = 0;
  virtual casacore::Int get(casacore::Int &) = 0;
  virtual casacore::Int get(casacore::String &) = 0;
  virtual casacore::Int get(casacore::Bool &) = 0;
  virtual casacore::Int get(casacore::Record &) = 0;
  
 protected: 
  // Number of processes
  casacore::Int numprocs;
  
  // Rank of current process
  casacore::Int myCpu;
   
  // Current source or destination process
  casacore::Int aWorker;

  // Current message tag
  casacore::Int aTag;
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
  MPITransport(casacore::Int, casacore::Char *argv[]);

  // Default source and message tag values
  virtual casacore::Int anyTag();
  virtual casacore::Int anySource();
  
  // Define the rank of the controller process
  virtual casacore::Int controllerRank() {return 0;};
  
  // Get and put functions on the parallel data transport layer
  virtual casacore::Int put(const casacore::Array<casacore::Float> &);
  virtual casacore::Int put(const casacore::Array<casacore::Double> &);
  virtual casacore::Int put(const casacore::Array<casacore::Complex> &);
  virtual casacore::Int put(const casacore::Array<casacore::DComplex> &);
  virtual casacore::Int put(const casacore::Array<casacore::Int> &);
  virtual casacore::Int put(const casacore::Float &);
  virtual casacore::Int put(const casacore::Double &);
  virtual casacore::Int put(const casacore::Complex &);
  virtual casacore::Int put(const casacore::DComplex &);
  virtual casacore::Int put(const casacore::Int &);
  virtual casacore::Int put(const casacore::String &);
  virtual casacore::Int put(const casacore::Bool &);
  virtual casacore::Int put(const casacore::Record &);

  virtual casacore::Int get(casacore::Array<casacore::Float> &);
  virtual casacore::Int get(casacore::Array<casacore::Double> &);
  virtual casacore::Int get(casacore::Array<casacore::Complex> &);
  virtual casacore::Int get(casacore::Array<casacore::DComplex> &);
  virtual casacore::Int get(casacore::Array<casacore::Int> &);
  virtual casacore::Int get(casacore::Float &);
  virtual casacore::Int get(casacore::Double &);
  virtual casacore::Int get(casacore::Complex &);
  virtual casacore::Int get(casacore::DComplex &);
  virtual casacore::Int get(casacore::Int &);
  virtual casacore::Int get(casacore::String &);
  virtual casacore::Int get(casacore::Bool &);
  virtual casacore::Int get(casacore::Record &);

 private:
  // Local work variables
  casacore::Int sendTo, myOp, getFrom;

  // Utility functions to set default source/destination and tag values
  void setSourceAndTag (casacore::Int &source, casacore::Int &tag);
  void setDestAndTag (casacore::Int &dest, casacore::Int &tag);
};
#endif

// <summary>
// Serial casacore::Data Transport Model
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
  virtual casacore::Int anyTag() {return -1;};
  virtual casacore::Int anySource() {return -1;};
  
  // Define the rank of the controller process
  virtual casacore::Int controllerRank() {return 0;};

  // Get and put functions on the data transport layer
  virtual casacore::Int put(const casacore::Array<casacore::Float> &);
  virtual casacore::Int put(const casacore::Array<casacore::Double> &);
  virtual casacore::Int put(const casacore::Array<casacore::Complex> &);
  virtual casacore::Int put(const casacore::Array<casacore::DComplex> &);
  virtual casacore::Int put(const casacore::Array<casacore::Int> &);
  virtual casacore::Int put(const casacore::Float &);
  virtual casacore::Int put(const casacore::Double &);
  virtual casacore::Int put(const casacore::Complex &);
  virtual casacore::Int put(const casacore::DComplex &);
  virtual casacore::Int put(const casacore::Int &);
  virtual casacore::Int put(const casacore::String &);
  virtual casacore::Int put(const casacore::Bool &);
  virtual casacore::Int put(const casacore::Record &);

  virtual casacore::Int get(casacore::Array<casacore::Float> &);
  virtual casacore::Int get(casacore::Array<casacore::Double> &);
  virtual casacore::Int get(casacore::Array<casacore::Complex> &);
  virtual casacore::Int get(casacore::Array<casacore::DComplex> &);
  virtual casacore::Int get(casacore::Array<casacore::Int> &);
  virtual casacore::Int get(casacore::Float &);
  virtual casacore::Int get(casacore::Double &);
  virtual casacore::Int get(casacore::Complex &);
  virtual casacore::Int get(casacore::DComplex &);
  virtual casacore::Int get(casacore::Int &);
  virtual casacore::Int get(casacore::String &);
  virtual casacore::Int get(casacore::Bool &);
  virtual casacore::Int get(casacore::Record &);

 private:
  casacore::uInt inQue;
  casacore::uInt outQue;
  casacore::uInt lastInQue;
  casacore::PtrBlock<void *> _data;
  
  casacore::Int add2Queue(void *);
  void *getFromQueue();
};


} //# NAMESPACE CASA - END

#endif


