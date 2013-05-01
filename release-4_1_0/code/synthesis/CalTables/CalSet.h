//# CalSet.h: Definition for Calibration parameter cache
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#ifndef CALTABLES_CALSET_H
#define CALTABLES_CALSET_H
#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays.h>
#include <casa/OS/File.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <synthesis/CalTables/CalTableDesc2.h>
#include <synthesis/CalTables/CalTable2.h>
#include <synthesis/CalTables/SolvableCalSetMCol.h>
#include <synthesis/CalTables/VisCalEnum.h>
//#include <synthesis/CalTables/BaseCalSet.h>

// #include <synthesis/MeasurementComponents/VisJones.h>
// #include <synthesis/MeasurementEquations/VisEquation.h>

namespace casa { //# NAMESPACE CASA - BEGIN


template<class T>
class CalSet 
{
public:

  // Null ctor does nothing
  CalSet() {};

  // Minimal, knows only how many potential spws
  //   (solve context)
  CalSet(const Int& nSpw);

  // From full shape
  CalSet(const Int& nSpw,
	 const Int& nPar,
	 const Vector<Int>& nChan,
	 const Int& nElem,
	 const Vector<Int>& nTime);


  // From existing CalTable
  //   (apply context)
  //   (for now, we will verify that filled solutions match nPar,nElem,nSpw)
  CalSet(const String& calTableName,
	 const String& select,
	 const Int& nSpw,
	 const Int& nPar,
	 const Int& nElem);

  // Copy
  CalSet(const CalSet& other);

  // Assignment
  //  virtual CalSet& operator=(const CalSet& other);

  // Destructor
  virtual ~CalSet();

  // Resize caches
  void resize(const Int& nPar,
	      const Vector<Int>& nChan,
	      const Int& nElem,
	      const Vector<Int>& nTime);

  // Access to shape info:
  Int& nSpw() { return nSpw_; };
  Int& nPar() { return nPar_; };
  Vector<Int>& nChan()  { return nChan_; };
  Int& nChan(const Int& spw)  { return nChan_(spw); };
  Int& nElem()  { return nElem_; };
  Vector<Int>& nTime()  { return nTime_; };
  Int& nTime(const Int& spw) { return nTime_(spw); };

  //  IPosition& shape(const Int& spw) { return IPosition(4,nPar(),nChan(spw),nElem(),nTime(spw)); };
  IPosition shape(const Int& spw) { return IPosition(4,nPar_,nChan_(spw),nElem_,nTime_(spw)); };

  // Set up spwOK according to solution availability
  inline void setSpwOK() { spwOK_ = (nTime()!=0); };

  // Report if solutions available for specified spw
  Vector<Bool> spwOK() { return spwOK_; };

  // Freq list per spw
  Vector<Double>& frequencies(const Int& spw) { return *freq_[spw]; };

  // Temporary method to support current conventions in SVJ
  void setStartChan(const Vector<Int>& startChan) {startChan_ = startChan;};
  Vector<Int>& startChan() {return startChan_;};

  // Access to whole solution caches 
  Vector<Double>& startTime(const Int& spw)    { return *MJDStart_[spw]; };
  Vector<Double>& stopTime(const Int& spw)     { return *MJDStop_[spw]; };
  Vector<Double>& time(const Int& spw)         { return *MJDTimeStamp_[spw]; };
  Vector<Int>&    fieldId(const Int& spw)      { return *fieldId_[spw]; };
  Vector<String>& fieldName(const Int& spw)    { return *fieldName_[spw]; };
  Vector<String>& sourceName(const Int& spw)   { return *sourceName_[spw]; };
  Array<T>&       par(const Int& spw)          { return *par_[spw]; };
  Array<Bool>&    parOK(const Int& spw)        { return *parOK_[spw]; };
  Array<Float>&   parErr(const Int& spw)       { return *parErr_[spw]; };
  Array<Float>&   parSNR(const Int& spw)       { return *parSNR_[spw]; };

  // Statistics
  //  Matrix<Bool>&   iSolutionOK(const Int& spw)  { return *iSolutionOK_[spw]; };
  Matrix<Float>&  iFit(const Int& spw)         { return *iFit_[spw]; };
  Matrix<Float>&  iFitwt(const Int& spw)       { return *iFitwt_[spw]; };
  Vector<Bool>&   solutionOK(const Int& spw)   { return *solutionOK_[spw]; };
  Vector<Float>&  fit(const Int& spw)          { return *fit_[spw]; };
  Vector<Float>&  fitwt(const Int& spw)        { return *fitwt_[spw]; };

  // Store/Retrieve solutions to/from a table.
  //  (will evolve to use table iteration)
  virtual void initCalTableDesc(const String& type, const Int& parType);
  virtual void attach();
  virtual void store(const String& file, const String& type, const Bool& append,
		     const String& msname="");
//   virtual void store(const String& file, const String& type, 
// 		     const String& msname, const Bool& append);
  virtual void load(const String& file, const String& select);

protected:


private:

  // new/delete of cache
  void inflate();
  void deflate();


  // Table name
  String calTableName_;

  // Number of Spectral windows
  Int nSpw_;

  // Number of parameters describing this calibration component
  Int nPar_;

  // Number of channels
  Vector<Int> nChan_;          // (nSpw_) number of _soln_ channels per spw

  // Number of elements (ants or baselines)
  Int nElem_;

  // Number of time slots
  Vector<Int> nTime_;         // (nSpw_) number of slots per spw

  // Spw OK?
  Vector<Bool> spwOK_;

  // Channel frequencies, etc.
  Vector<Int> startChan_;            // (nSpw_) start data channel per spw
  PtrBlock<Vector<Double>*> freq_;   // [nSpw_](nChan_)

  // Per spw, per slot Meta information
  PtrBlock<Vector<Double>*> MJDStart_;       // (nSpw_)(numberSlots_)
  PtrBlock<Vector<Double>*> MJDStop_;        // (nSpw_)(numberSlots_)
  PtrBlock<Vector<Double>*> MJDTimeStamp_;   // (nSpw_)(numberSlots_)
  PtrBlock<Vector<Int>*>    fieldId_;        // (nSpw_)(numberSlots_)
  PtrBlock<Vector<String>*> fieldName_;      // (nSpw_)(numberSlots_)
  PtrBlock<Vector<String>*> sourceName_;     // (nSpw_)(numberSlots_)

  // Per spw, per channel, per element, per slot solution generic PARAMETER storage  
  PtrBlock<Array<T>*>      par_;    // (nSpw_)(nPar_,nSolnChan_,nElem_,numberSlots_)
  PtrBlock<Array<Bool>*>   parOK_;  // (nSpw_)(nPar_,nSolnChan_,nElem_,numberSlots_)
  PtrBlock<Array<Float>*>  parErr_; // (nSpw_)(nPar_,nSolnChan_,nElem_,numberSlots_)
  PtrBlock<Array<Float>*>  parSNR_; // (nSpw_)(nPar_,nSolnChan_,nElem_,numberSlots_)

  // Statistics
  //  PtrBlock<Matrix<Bool>*> iSolutionOK_;  // [nSpw_](nElem_,nSlots_)
  PtrBlock<Matrix<Float>*> iFit_;        // [nSpw_](nElem_,nSlots_)
  PtrBlock<Matrix<Float>*> iFitwt_;      // [nSpw_](nElem_,nSlots_)

  PtrBlock<Vector<Bool>*> solutionOK_;   // [nSpw_](numberSlots_)
  PtrBlock<Vector<Float>*> fit_;         // [nSpw_](numberSlots_)
  PtrBlock<Vector<Float>*> fitwt_;       // [nSpw_](numberSlots_)

  LogSink logSink_p;
  LogSink& logSink() {return logSink_p;};

  CalTableDesc2 *calTabDesc_;
  CalTable2 *calTab_;
  SolvableCalSetMCol<T> *svjmcol_;

};

// Globals

// Smooth the solutions in a CalSet
  void smooth(CalSet<Complex>& cs,
	      const String& smtype,
	      const Double& smtime,
	      Vector<Int> selfields);

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <synthesis/CalTables/CalSet.tcc>
#endif

#endif
