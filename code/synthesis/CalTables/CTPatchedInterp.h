//# CTPatchedInterp.h: Definition for Calibration patch panel
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

#ifndef CALTABLES_CTPATCHEDINTERP_H
#define CALTABLES_CTPATCHEDINTERP_H

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTTimeInterp1.h>
#include <synthesis/CalTables/RIorAParray.h>
#include <synthesis/CalTables/VisCalEnum.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/aips.h>

//#include <casa/BasicSL/Constants.h>
//#include <casa/OS/File.h>
//#include <casa/Logging/LogMessage.h>
//#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


class CTPatchedInterp
{
public:

  // From NewCalTable only 
  CTPatchedInterp(NewCalTable& ct,
		  VisCalEnum::MatrixType mtype,
		  Int nPar,
		  const String& timetype,
		  const String& freqtype,
		  const String& fieldtype,
		  Vector<Int> spwmap=Vector<Int>());

  // From NewCalTable and MS 
  CTPatchedInterp(NewCalTable& ct,
		  VisCalEnum::MatrixType mtype,
		  Int nPar,
		  const String& timetype,
		  const String& freqtype,
		  const String& fieldtype,
		  const MeasurementSet& ms,
		  Vector<Int> spwmap=Vector<Int>());

  // From NewCalTable and MSColumns 
  CTPatchedInterp(NewCalTable& ct,
		  VisCalEnum::MatrixType mtype,
		  Int nPar,
		  const String& timetype,
		  const String& freqtype,
		  const String& fieldtype,
		  const ROMSColumns& mscol,
		  Vector<Int> spwmap=Vector<Int>());


  // Destructor
  virtual ~CTPatchedInterp();

  // Interpolate, given input field, spw, timestamp, & (optionally) freq list
  //    returns T if new result (anywhere)
  Bool interpolate(Int obs, Int fld, Int spw, Double time, Double freq=-1.0);
  Bool interpolate(Int obs, Int fld, Int spw, Double time, const Vector<Double>& freq);

  // Access to the result
  Array<Float>& resultF(Int obs, Int fld, Int spw) { return result_(spw,fld,thisobs(obs)); };
  Array<Complex> resultC(Int obs, Int fld, Int spw) { return RIorAPArray(result_(spw,fld,thisobs(obs))).c(); };
  Array<Bool>& rflag(Int obs, Int fld, Int spw) { return resFlag_(spw,fld,thisobs(obs)); };

  // Temporary public function for testing
  Array<Float>& tresultF(Int obs, Int fld, Int spw) { return timeResult_(spw,fld,thisobs(obs)); };
  Array<Bool>& tresultFlag(Int obs, Int fld, Int spw) { return timeResFlag_(spw,fld,thisobs(obs)); };

  // spwOK info for users
  Bool spwOK(Int spw) const;
  Bool spwInOK(Int spw) const;

  // Const access to various state
  // TBD

  // Report state
  void state();

private:

  // Null ctor does nothing
  CTPatchedInterp() :mtype_(VisCalEnum::GLOBAL) {};
  
  // Setup methods
  void sliceTable();
  void makeInterpolators();

  Int thisobs(Int obs) { return (byObs_?obs:0); };

  // Methods to set up 1:1 patch-panel maps
  //  Private for now as not yet ready to control from outside
  // Field
  // default: all 0 (no field-dep yet)
  void setDefFldMap() {fldMap_.resize(nMSFld_); fldMap_.set(0);};
  void setFldMap(const MSField& msfld);           // via nearest on-sky
  void setFldMap(const ROMSFieldColumns& fcol);  // via nearest on-sky
  //void setFldMap(Vector<Int>& field);        // via ordered index list
  //void setFldMap(Vector<String>& field);     // via name matching
  //void setFldMap(uInt to, uInt from);        // via single to/from 

  // Spw
  // default: indgen (index identity)
  void setDefSpwMap() {spwMap_.resize(nMSSpw_); indgen(spwMap_);};
  void setSpwMap(Vector<Int>& spwmap);
  //void setSpwMap(Vector<Double>& refFreqs);  // via refFreq matching
  //void setSpwMap(uInt to, uInt from);        // via single to/from

  // Antenna
  // default: indgen (index identity) 
  void setDefAntMap() {antMap_.resize(nMSAnt_); indgen(antMap_);};
  //void setAntMap(Vector<Int>& ant);          // via ordered index list
  //void setAntMap(Vector<String>& ant);       // via name/station matching
  //void setAntMap(uInt to, uInt from);        // via single to/from

  // Set generic antenna/baseline map
  void setElemMap();

  // Resample in frequency
  void resampleInFreq(Matrix<Float>& fres,Matrix<Bool>& fflg,const Vector<Double>& fout,
		      Matrix<Float>& tres,Matrix<Bool>& tflg,const Vector<Double>& fin);

  // Baseline index from antenna indices: (assumes a1<=a2 !!)
  inline Int blnidx(const Int& a1, const Int& a2, const Int& nAnt) { return  a1*nAnt-a1*(a1+1)/2+a2; };

  // Translate freq axis interpolation string
  InterpolateArray1D<Double,Float>::InterpolationMethod ftype(String& strtype);


  // PRIVATE DATA:
  
  // The Caltable
  NewCalTable ct_;

  // Matrix type
  VisCalEnum::MatrixType mtype_;

  // Are parameters fundamentally complex?
  Bool isCmplx_;

  // The number of (Float) parameters (per-chan, per-element)
  Int nPar_, nFPar_;

  // Interpolation modes
  String timeType_, freqType_;

  InterpolateArray1D<Double,Float>::InterpolationMethod ia1dmethod_;

  // Are we slicing caltable by field?
  Bool byObs_,byField_;

  // CalTable freq axis info
  Vector<Int> nChanIn_;
  Vector<Vector<Double> > freqIn_;


  // Obs, Field, Spw, Ant _output_ (MS) sizes 
  //   calibration required for up to this many
  Int nMSObs_, nMSFld_, nMSSpw_, nMSAnt_, nMSElem_;

  // Alternate field indices
  Vector<Int> altFld_;

  // Obs, Field, Spw, Ant _input_ (CalTable) sizes
  //  patch panels should not violate these (point to larger indices)
  Int nCTObs_, nCTFld_, nCTSpw_, nCTAnt_, nCTElem_;

  // OK flag
  Vector<Bool> spwInOK_;

  // The patch panels
  //   Each has length from MS, values refer to CT
  Vector<Int> fldMap_, spwMap_, antMap_, elemMap_;

  // Control conjugation of baseline-based solutions when mapping requires
  Vector<Bool> conjTab_;

  // Internal result Arrays
  Cube<Cube<Float> > timeResult_,freqResult_;   // [nMSSpw_,nMSFld_,nMSObs_][nFpar,nChan,nAnt]
  Cube<Cube<Bool> >  timeResFlag_,freqResFlag_; // [nMSSpw_,nMSFld_,nMSObs_][nFpar,nChan,nAnt]

  // Current interpolation result Arrays
  //  These will reference time or freq result, depending on context,
  //  and may be referenced by external code
  Cube<Cube<Float> > result_;     // [nMSSpw_,nMSFld_,nMSObs_][nFpar,nChan,nAnt]
  Cube<Cube<Bool> >  resFlag_;    // [nMSSpw_,nMSFld_,nMSObs_][nFpar,nChan,nAnt]

  // The CalTable slices
  Array<NewCalTable*> ctSlices_;  // [nCTElem_,nCTSpw_,nCTFld_,nCTObs_]

  // The pre-patched Time interpolation engines
  //   These are populated by the available caltables slices
  Array<CTTimeInterp1*> tI_;  // [nMSElem_,nMSSpw_,nMSFld_,nMSObs_]
  Array<Bool> tIdel_;         // [nMSElem_,nMSSpw_,nMSFld_,mMSObs_]

};


} //# NAMESPACE CASA - END

#endif
