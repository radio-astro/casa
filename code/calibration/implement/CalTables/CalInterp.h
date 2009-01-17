//# CalInterp.h: Definition for Calibration parameter Interpolation
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

#ifndef CALTABLES_CALINTERP_H
#define CALTABLES_CALINTERP_H
#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <calibration/CalTables/CalSet.h>
#include <casa/OS/File.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


class CalInterp
{
public:

  // Null ctor does nothing
  CalInterp() {};

  // From CalSet 
  CalInterp(CalSet<Complex>& cs,
	    const String& timetype,
	    const String& freqtype);

  // Destructor
  virtual ~CalInterp();


  // Set non-trivial spw mapping
  void setSpwMap(const Vector<Int>& spwmap) {spwMap_ = spwmap; setSpwOK();};

  // Interpolate, given timestamp, spw, freq list; returns T if new result
  Bool interpolate(const Double& time,
		   const Int& spw,
		   const Vector<Double>& freq=Vector<Double>());

  // Do time-dep interpolation (called by interpolate)
  Bool interpTime(const Double& time);

  // Do freq-dep interpolation (called by interpolate)
  void interpFreq(const Vector<Double>& freq);

  // Timestamp of current Slot
  Double slotTime() { return csTimes()(currSlot()); };

  // Access to result (by reference)
  inline Cube<Complex>& result() { return r; };
  inline Cube<Bool>& resultOK() { return ok; };

  // Spwmap-sensitive spwOK()
  inline Vector<Bool> spwOK() { return spwOK_; };

protected:

  // Find the reference timeslot for requested time
  Bool findSlot(const Double& time);

  void updTimeCoeff();

  void interpTimeCalc(const Double& time);

  void initFreqInterp(const Vector<Double> freq);

  void calcAPC();

  void updFreqCoeff();

  void interpFreqCalc();

  void finalize();

  void inflTimeC();
  void inflFreqC();
  void deflTimeC();
  void deflFreqC();
  void deflFreqA();

  // Access to private shape data (reflecting CalSet)
  inline Int& nSpw()  { return cs_->nSpw();  };
  inline Int& nPar()  { return cs_->nPar();  };
  inline Int& nElem() { return cs_->nElem(); };

  // Access to imutable private data
  inline String& timeType()  { return timeType_; };
  inline Bool   nearestT() { return (timeType()=="nearest"); };
  inline Bool   linearT()  { return (timeType()=="linear"); };
  inline Bool   aipslinT() { return (timeType()=="aipslin"); };
  inline String& freqType()  { return freqType_; };
  inline Bool   nearestF() { return (freqType()=="nearest"); };
  inline Bool   linearF()  { return (freqType()=="linear"); };
  inline Bool   aipslinF() { return (freqType()=="aipslin"); };

  inline Vector<Int>& spwMap()  { return spwMap_; };
  inline Int spwMap(const Int& spw) { return ((spwMap_(spw)>-1) ? spwMap_(spw) : spw); };

  // Access to info relevant to currSpw
  inline Int& currSpw()     { return currSpw_; };
  inline Int currSpwMap()  { return spwMap(currSpw()); };
  inline Int& currSlot()    { return currSlot_(currSpw()); };
  inline Int& nFreq()       { return nFreq_(currSpw()); };
  inline Vector<Double>& datFreq() { return (*datFreq_[currSpw()]); };
  inline Bool& finit()      { return finit_(currSpw()); };
  inline Double& lastTime() { return lastTime_(currSpw_); };
  inline Bool& exactTime()  { return exactTime_; };

  // Access to in-focus portion of CalSet (spwmap'd)
  inline Int& nTime()              { return cs_->nTime(currSpwMap()); };
  inline Int& nChan()              { return cs_->nChan(currSpwMap()); }
  inline Vector<Double>& csTimes() { return cs_->time(currSpwMap()); };
  inline Vector<Double>& csFreq()  { return cs_->frequencies(currSpwMap()); };
  inline Array<Complex>& csPar()   { return cs_->par(currSpwMap()); };
  inline Array<Bool>&    csParOK() { return cs_->parOK(currSpwMap()); };

  // Access to IPositions
  inline IPosition&  ip4d() { return (*ip4d_[currSpw_]); };
  inline IPosition&  ip3d() { return (*ip3d_[currSpw_]); };
  inline IPosition&  ip2d() { return (*ip2d_[currSpw_]); };

  // Access to in-focus interpolation abscissa data
  inline Double&         t0()  {return t0_(currSpw_);};
  inline Double&         tS()  {return tS_(currSpw_);};
  inline Int&        lastlo()  {return lastlo_(currSpw_); };

  inline Vector<Int>&    ch0() {return (*ch0_[currSpw_]); };
  inline Vector<Bool>&   ef()  {return (*ef_[currSpw_]); };
  inline Vector<Double>& df()  {return (*df_[currSpw_]); };
  inline Vector<Double>& fdf() {return (*df_[currSpw_]); };

  // In-focus Time Interpolation coefficients 
  inline Array<Float>&   tAC() {return (*tAC_[currSpw()]); };
  inline Array<Float>&   tPC() {return (*tPC_[currSpw()]); };
  inline Array<Complex>& tCC() {return (*tCC_[currSpw()]); };
  inline Cube<Bool>&     tOk() {return (*tOk_[currSpw()]); };

  // In-focus Freq Interpolation coefficients
  inline Array<Float>&   fAC() {return (*fAC_[currSpw()]); };
  inline Array<Float>&   fPC() {return (*fPC_[currSpw()]); };
  inline Array<Complex>& fCC() {return (*fCC_[currSpw()]); };
  inline Cube<Bool>&     fOk() {return (*fOk_[currSpw()]); };

  inline void rPart(Array<Complex>& c,Array<Float>& rp) { return part(c,0,rp); };
  inline void iPart(Array<Complex>& c,Array<Float>& ip) { return part(c,1,ip); };

  void asFloatArr(const Array<Complex>& in, Array<Float>& out);

  void part(const Array<Complex>& c, const Int& which, Array<Float>& f);


private:

  // Set spwOK_ according to spwmap
  void setSpwOK();

  // The CalSet from which we interpolate
  CalSet<Complex>* cs_;

  // Interpolation modes
  String timeType_, freqType_;

  // Spw map
  Vector<Int> spwMap_;

  // SpwOK
  Vector<Bool> spwOK_;

  // Prior interp time, per spw
  Vector<Double> lastTime_;

  // Frequency initialization flag (True if init complete)
  Vector<Bool> finit_;

  // Number of channels
  Vector<Int> nFreq_;          // (nSpw_) number of _data_ channels per spw

  // Freq lists
  PtrBlock<Vector<Double>*> solFreq_;   // [nSpw](nChan)
  PtrBlock<Vector<Double>*> datFreq_;   // [nSpw](nFreq)

  Int currSpw_;
  Vector<Int> currSlot_;

  // Exact time match flag
  Bool exactTime_;

  Cube<Float> a,p;    // For referencing interp results
  Cube<Complex> c;
  Cube<Bool> ok;

  Cube<Complex> r, r_;

  // Array shapes
  PtrBlock<IPosition*> ip4d_;     // [nSpw]
  PtrBlock<IPosition*> ip3d_;     // [nSpw]
  PtrBlock<IPosition*> ip2d_;     // [nSpw]

  // Time Interpolation ordinate info (per spw)
  Vector<Double>            t0_;         // (nSpw)
  Vector<Double>            tS_;         // (nSpw)
  Vector<Int>               lastlo_;     // (nSpw)

  // Time Interpolation coefficients (per spw)
  PtrBlock<Array<Float>*>   tAC_, tPC_;  // [nSpw](2,nPar,nChan,nElem)
  PtrBlock<Array<Complex>*> tCC_;        // [nSpw](2,nPar,nChan,nElem)
  PtrBlock<Cube<Bool>*>     tOk_;        // [nSpw](nPar,nChan,nElem)

  // Time Interpolation results (currSpw)
  Cube<Float>    tA_, tP_;    // (nPar,nChan,nElem)
  Cube<Complex>  tC_;         // (nPar,nChan,nElem)

  // Freq Interpolation ordinate info (per spw)
  PtrBlock<Vector<Int>*>    ch0_;        // [nSpw](nFreq)
  PtrBlock<Vector<Bool>*>   ef_;         // [nSpw](nFreq)
  PtrBlock<Vector<Double>*> df_;         // [nSpw](nFreq)
  PtrBlock<Vector<Double>*> fdf_;        // [nSpw](nFreq)

  // Freq Interpolation coefficients (per spw)
  PtrBlock<Array<Float>*>   fAC_, fPC_;  // [nSpw](2,nPar,nFreq,nElem)
  PtrBlock<Array<Complex>*> fCC_;        // [nSpw](2,nPar,nFreq,nElem)
  PtrBlock<Cube<Bool>*>     fOk_;        // [nSpw](nPar,nFreq,nElem)

  // Freq Interpolation results (currSspw)
  Cube<Float>    fA_, fP_;               // (nPar,nFreq,nElem)
  Cube<Complex>  fC_;                    // (nPar,nFreq,nElem)

  LogSink logSink_p;
  LogSink& logSink() {return logSink_p;};

  Bool verbose_;

};


} //# NAMESPACE CASA - END

#endif
