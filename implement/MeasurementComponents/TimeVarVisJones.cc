//# TimeVarVisJones.cc: Implementation of Jones classes
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <synthesis/MeasurementComponents/TimeVarVisJones.h>
#include <calibration/CalTables/CalTable.h>
#include <calibration/CalTables/TimeVarVJDesc.h>
#include <calibration/CalTables/TimeVarVJMRec.h>
#include <msvis/MSVis/VisBuffer.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/BinarySearch.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Exceptions/Error.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#include <casa/Quanta/MVTime.h>
namespace casa { //# NAMESPACE CASA - BEGIN

// ------------------------------------------------------------------
// Start of methods for a non-solveable Jones matrix for any solely
// visibility indexed gain. This caches the interferometer gains and
// the inverses according to time. It could be generalized to cache on
// something else. The antenna gain is calculated as needed per slot.

// From VisSet. 

TimeVarVisJones::TimeVarVisJones(VisSet& vs) :
  vs_(&vs), 
  localVS_(False), 
  solved_(False), 
  applied_(False),
  interval_(0.0),
  deltat_(1.0),
  numberAnt_(vs.numberAnt()),
  numberSpw_(vs.numberSpw()),
  currentSpw_(0),
  currentCalSpw_(0),
  currentStart_(0.0),
  currentStop_(0.0),
  lastTimeStamp_(0.0),
  jMatType_(2),
  interpType_("nearest")
{
// Generic TVVJ constructor, from VisSet alone
//   Makes a virtually brain-dead VisJones; the real action is in
//     setInterpolation (and setSolver in SVJ)
// Input:
//    vs                    VisSet&       VisSet (w/ implicit selection)
// Output to protected/private data:
//    vs_                   VisSet*       Pointer to parent VisSet
//    localVS_=F            Bool          Indicates VisSet is not local
//    solved_=F             Bool          solved for?
//    applied_=F            Bool          applied?
//    interval_=0.0         Double        Iteration interval
//    deltat__=0.0          Double        Time comparison precision
//    
//    numberAnt_            Int           Number of antennas in VisSet
//    numberSpw_            Int           Number of spws in VisSet
//    numberSlot_           Vector<Int>   Number of slots per spw (0)
//    currentSlot_          Vector<Int>   Current slot per spw (0)
//    nSolnChan_            Vector<Int>   Number of solution chans per spw (1)


  // Remember how many ants, spws
  numberAnt_=vs.numberAnt();
  numberSpw_=vs.numberSpw();

  // Number of slots per spw 
  numberSlot_.resize(numberSpw_);  numberSlot_=0;
  currentSlot_.resize(numberSpw_); currentSlot_=-1;

  // Default spwMap_ is no mapping
  spwMap_.resize(numberSpw_); spwMap_=-1;

  // Number of parameters, this type (none, this is abstract class)
  nPar_=0;

  // Assume 1 channel per spw, starting at 0
  nSolnChan_.resize(numberSpw_); nSolnChan_=1;
  startChan_.resize(numberSpw_); startChan_=0;

  // Remember which PBs were newed:
  newPB_.resize(numberSpw_); newPB_=False;

}

TimeVarVisJones::TimeVarVisJones(const TimeVarVisJones& other) {
  //  operator=(other);
}

TimeVarVisJones::~TimeVarVisJones() {

  deleteThisGain();
  deleteMetaCache();
  deleteAntGain();
  deleteInterp();

  if (localVS_ && vs_) {delete vs_; vs_=NULL;};
}

void TimeVarVisJones::setInterpolation(const Record& interpolation) 
{
//  Set up interpolation of applied types (non-solvable)
//  Inputs:
//    interpolation           Record&       Contains application params
//  Inputs from private/protected data
//    

  // Collect parameters needed at this level
  if (interpolation.isDefined("t")) interval_=interpolation.asDouble("t");
  if (interpolation.isDefined("interp")) interpType_=interpolation.asString("interp");

  // Make local VisSet with correct chunking
  //  makeLocalVisSet();

  // Collect meta info
  //  initMetaCache();
  //  fillMetaCache();

  // Set up Gain cache (one slot per spw)
  //  initGainCache();
  initThisGain();

  // This is apply context
  setSolved(False);
  setApplied(True);

}

void TimeVarVisJones::initInterp(const String& interptype,
				 const Vector<Int>& spwmap) {

  LogMessage message(LogOrigin("TimeVarVisJones", "initInterp"));

  interpType_=interptype;

  if (max(numberSlot_)==1) {
    interpType_="nearest";
    ostringstream o; o<<"Only one timeslot found for "
		      << typeName()
		      <<" calibration.";
    message.message(o);
    logSink().post(message);
  }
  
  if (interpType_== "linear") {
    
    timeRef_.resize(numberSpw_);
    timeStep_.resize(numberSpw_);
    
    ampRef_.resize(numberSpw_);   ampRef_=NULL;
    ampSlope_.resize(numberSpw_); ampSlope_=NULL;
    phaRef_.resize(numberSpw_);   phaRef_=NULL;
    phaSlope_.resize(numberSpw_); phaSlope_=NULL;
    
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      ampRef_[ispw]   = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
      ampSlope_[ispw] = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
      phaRef_[ispw]   = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
      phaSlope_[ispw] = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
    }
  } else if (interpType_=="aipslin") { 
    
    timeRef_.resize(numberSpw_);
    timeStep_.resize(numberSpw_);
    
    ampRef_.resize(numberSpw_);    ampRef_=NULL;
    ampSlope_.resize(numberSpw_);  ampSlope_=NULL;
    gainRef_.resize(numberSpw_);   gainRef_=NULL;
    gainSlope_.resize(numberSpw_); gainSlope_=NULL;
    
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      ampRef_[ispw]   = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
      ampSlope_[ispw] = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
      gainRef_[ispw]   = new Cube<Complex>(nPar_,nSolnChan_[ispw],numberAnt_);
      gainSlope_[ispw] = new Cube<Complex>(nPar_,nSolnChan_[ispw],numberAnt_);
    }
  } else {
    interpType_="nearest";
  }

  ostringstream o; o<<"Applying "
		    << typeName()
		    <<" calibration with "
		    << interpType_
		    <<" interpolation in time.";
  message.message(o);
  logSink().post(message);
  
  // setup spwmap
  Int nmap;
  spwmap.shape(nmap);
  
  // Make sure user-specified spwmap isn't too long!
  if (nmap > numberSpw_) {
    ostringstream o; o<<"Truncating spwmap list to "
		      << numberSpw_
		      <<" values, to match data.";
    message.message(o);
    message.priority(LogMessage::WARN);
    logSink().post(message);
    message.priority(LogMessage::NORMAL);
    
    //    spwmap.resize(numberSpw_,True);
    nmap=numberSpw_;
    
  }
  
  for (Int ispw=0;ispw<nmap;ispw++) {
    if (spwmap(ispw) > -1 && spwmap(ispw)!=ispw) {
      spwMap_(ispw)=spwmap(ispw);
      ostringstream o; o<<"Spwmap: Applying "
			<< typeName()
			<<" calibration from spw="
			<<spwMap_(ispw)+1
			<<" to spw="
			<<ispw+1;
      message.message(o);
      logSink().post(message);
    } else {
      spwMap_(ispw) = -1;
    }
  }
}


void TimeVarVisJones:: deleteInterp() {

  if (interpType_!="nearest") {
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      if (ampRef_.nelements() > 0 && ampRef_[ispw]){   delete ampRef_[ispw]; ampRef_[ispw]=NULL;}
      if (ampSlope_.nelements() > 0 && ampSlope_[ispw]){ delete ampSlope_[ispw]; ampSlope_[ispw]=NULL;}
      if (interpType_=="linear") {
        if (phaRef_.nelements() > 0 && phaRef_[ispw]){   delete phaRef_[ispw];  phaRef_[ispw]=NULL;}
        if (phaSlope_.nelements() > 0 && phaSlope_[ispw]){ delete phaSlope_[ispw]; phaSlope_[ispw];}
      } else if (interpType_=="aipslin") {
        if (gainRef_.nelements() > 0 && gainRef_[ispw]){   delete gainRef_[ispw]; gainRef_[ispw]=NULL;}
        if (gainSlope_.nelements() > 0 && gainSlope_[ispw]){ delete gainSlope_[ispw]; gainSlope_[ispw]=NULL;}
      }
    }
  }

}

void TimeVarVisJones::makeLocalVisSet() {

  Block<Int> columns;
  if (interval_==0.0) {
    // include scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
  } else {
    // avoid scan iteration
    columns.resize(4);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::FIELD_ID;
    columns[2]=MS::DATA_DESC_ID;
    columns[3]=MS::TIME;
  }

  vs_= new VisSet(*vs_,columns,interval_);
  localVS_=True;

  // find out how many intervals we have
  VisIter& ioc(vs_->iter());
  ioc.originChunks();
  VisBuffer vb(ioc);
  for (ioc.originChunks(); ioc.moreChunks();ioc.nextChunk()) {
    // count number of slots per spw
    numberSlot_[ioc.spectralWindow()]++;
  }
  

  LogMessage message(LogOrigin("TimeVarVisJones", "makeLocalVisSet"));
  {
    ostringstream o; o<<"For interval of "<<interval_<<" seconds, found "<<
		       sum(numberSlot_)<<" slots";message.message(o);
    logSink().post(message);
  }
}



// Initialize the time-dependent cache
void TimeVarVisJones::initAntGain() {
  
  // TODO:
  //  Consider initialization value (per type)

  // Delete the cache, in case it already exists
  deleteAntGain();

  // Resize PBs according to number of Spw
  jonesPar_.resize(numberSpw_);      jonesPar_=NULL;
  jonesParOK_.resize(numberSpw_);    jonesParOK_=NULL;

  // Construct and resize pointed-to objects for each available spw
  for (Int ispw=0; ispw<numberSpw_; ispw++) {
    uInt nslot=numberSlot_(ispw);
    if (nslot > 0) {
      IPosition parshape(4,nPar_,nSolnChan_(ispw),numberAnt_,nslot);
      
      jonesPar_[ispw]   = new Array<Complex>(parshape,Complex(1.0,0.0));
      jonesParOK_[ispw] = new Cube<Bool>(nSolnChan_(ispw),numberAnt_,nslot,False);
    }
  }

}

// Initialize the "this" cache
void TimeVarVisJones::initThisGain() {

  // TODO: Add use of parType_

  currentSlot_.resize(numberSpw_);
  currentSlot_=-1;
  currentStart_=0;
  currentStop_=0;

  // Matrices start out invalid
  thisJMValid_.resize(numberSpw_);
  thisJMValid_=False;
  thisMMValid_.resize(numberSpw_);
  thisMMValid_=False;

  thisTimeStamp_.resize(numberSpw_);
  thisTimeStamp_=0.0;

  // Delete the cache, in case it already exists
  deleteThisGain();

  // Resize all of the PBs according to number of Spws
  thisJonesPar_.resize(numberSpw_);
  thisJonesMat_.resize(numberSpw_);
  thisJonesOK_.resize(numberSpw_);
  thisMuellerMat_.resize(numberSpw_);

  // Create pointed-to objects and initialize them
  for (Int ispw=0; ispw<numberSpw_; ispw++) {

    thisJonesPar_[ispw]    = new Cube<Complex>(nPar_,nSolnChan_[ispw],numberAnt_);
    *(thisJonesPar_[ispw]) = Complex(1.0,0.0);

    thisJonesMat_[ispw]    = new Matrix<mjJones2>(nSolnChan_[ispw],numberAnt_);
    *(thisJonesMat_[ispw]) = mjJones2(Complex(1.0,0.0));

    thisJonesOK_[ispw]     = new Vector<Bool>(numberAnt_,True);

    thisMuellerMat_[ispw]    = new Cube<mjJones4>(nSolnChan_[ispw],numberAnt_, numberAnt_);
    *(thisMuellerMat_[ispw]) = mjJones4(Complex(1.0,0.0));
    thisMuellerMat_[ispw]->unique();
  }
};

// Initialize the slot-dependent meta-data caches
void TimeVarVisJones::initMetaCache() {

  // Delete the cache, in case it already exists
  deleteMetaCache();

  // Resize PBs according to number of Spws
  MJDStart_.resize(numberSpw_);     MJDStart_=NULL;
  MJDStop_.resize(numberSpw_);      MJDStop_=NULL;
  MJDTimeStamp_.resize(numberSpw_); MJDTimeStamp_=NULL;
  fieldName_.resize(numberSpw_);    fieldName_=NULL;
  sourceName_.resize(numberSpw_);   sourceName_=NULL;
  fieldId_.resize(numberSpw_);      fieldId_=NULL;

  // Construct and resize pointed-to objects for each available spw
  for (Int ispw=0; ispw<numberSpw_; ispw++) {
    uInt nslot=numberSlot_(ispw);
    if (nslot > 0) {
      newPB_(ispw) = True;
      MJDStart_[ispw]     = new Vector<Double>(nslot,0.0);
      MJDStop_[ispw]      = new Vector<Double>(nslot,0.0);
      MJDTimeStamp_[ispw] = new Vector<Double>(nslot,0.0);
      fieldName_[ispw]    = new Vector<String>(nslot,"");
      sourceName_[ispw]   = new Vector<String>(nslot,"");
      fieldId_[ispw]      = new Vector<Int>(nslot,-1);
    }
  }

}


// Fill the slot-dependent meta-data caches with info
void TimeVarVisJones::fillMetaCache() {
  
  // TODO: 
  // 1. Can we use setRowBlocking() here to avoid in-chunk iterations?


  // Fill the cache within info
  VisIter& ioc(vs_->iter());
  ioc.originChunks();
  VisBuffer vb(ioc);
  for (ioc.originChunks(); ioc.moreChunks(); ioc.nextChunk()) {

    Int thisSpw=ioc.spectralWindow();

    // Count slots per spw
    uInt islot= (++currentSlot_(thisSpw));

    // Set field source info
    (*fieldId_[thisSpw])(islot)   =ioc.fieldId();
    (*fieldName_[thisSpw])(islot) =ioc.fieldName();
    (*sourceName_[thisSpw])(islot)=ioc.sourceName();

    // Set time info
    ioc.origin();
    (*MJDStart_[thisSpw])(islot)  =vb.time()(0)-vb.timeInterval()(0)/2.0;
    Double timeStamp(0.0);
    Int ntime(0);
    for (ioc.origin(); ioc.more(); ioc++,ntime++) {
      timeStamp+=vb.time()(0);
    }
    (*MJDStop_[thisSpw])(islot) =vb.time()(0)+vb.timeInterval()(0)/2.0;
    (*MJDTimeStamp_[thisSpw])(islot)=timeStamp/ntime;

  }

}


void TimeVarVisJones::deleteThisGain() {

  uInt nCache=thisJonesMat_.nelements();
  if (nCache > 0) {
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      if (thisJonesPar_[ispw]) {delete thisJonesPar_[ispw]; thisJonesPar_[ispw]=NULL;};
      if (thisJonesMat_[ispw]) {delete thisJonesMat_[ispw]; thisJonesMat_[ispw]=NULL;};
      if (thisJonesOK_[ispw])  {delete thisJonesOK_[ispw];  thisJonesOK_[ispw]=NULL;};
      if (thisMuellerMat_[ispw]) {delete thisMuellerMat_[ispw]; thisMuellerMat_[ispw]=NULL;};
    }
  }
}

void TimeVarVisJones::deleteAntGain() {
  
  uInt nCache=jonesPar_.nelements();
  if (nCache > 0) {
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      if (newPB_(ispw)) {
	if (jonesPar_[ispw])    delete jonesPar_[ispw];
	if (jonesParOK_[ispw])  delete jonesParOK_[ispw];
      }
      jonesPar_[ispw]=NULL;
      jonesParOK_[ispw]=NULL;
    }
  }
}

void TimeVarVisJones::deleteMetaCache() {

  uInt nCache=MJDStart_.nelements();
  if (nCache > 0) {
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      if (newPB_(ispw)) {
	if (MJDStart_[ispw])     delete MJDStart_[ispw];
	if (MJDStop_[ispw])      delete MJDStop_[ispw];
	if (MJDTimeStamp_[ispw]) delete MJDTimeStamp_[ispw];
	if (fieldName_[ispw])    delete fieldName_[ispw];
	if (sourceName_[ispw])   delete sourceName_[ispw];
	if (fieldId_[ispw])      delete fieldId_[ispw];
      }
      MJDStart_[ispw]=NULL;
      MJDStop_[ispw]=NULL;
      MJDTimeStamp_[ispw]=NULL;
      fieldName_[ispw]=NULL;
      sourceName_[ispw]=NULL;
      fieldId_[ispw]=NULL;
    }
  }
}

#undef ASSIGN
#ifdef ASSIGN
// Assignment
TimeVarVisJones& TimeVarVisJones::operator=(const TimeVarVisJones& other) {
  if(this!=&other) {
    solved_=other.solved_;
    applied_=other.applied_;
    interval_=other.interval_;
    deltat_=other.deltat_;
    numberAnt_=other.numberAnt_;
    numberSlot_=other.numberSlot_;
    currentSlot_=other.currentSlot_;
    numberSpw_=other.numberSpw_;  // new numberSpw_

    // Delete existing PBs, then Resize according to number of Spws
    MJDStart_.resize(numberSpw_);
    MJDStop_.resize(numberSpw_);
    MJDTimeStamp_.resize(numberSpw_);
    fieldName_.resize(numberSpw_);
    sourceName_.resize(numberSpw_);
    fieldId_.resize(numberSpw_);
    antGain_.resize(numberSpw_);
    antGainOK_.resize(numberSpw_);  
    initGainCache();

    // Allocate and fill pointed-to objects
    for (Int ispw=0; ispw<numberSpw_; ispw++) {
      MJDStart_[ispw]        = new Vector<Double>();
      (*MJDStart_[ispw])     = (*other.MJDStart_[ispw]); 
      MJDStop_[ispw]         = new Vector<Double>;
      (*MJDStop_[ispw])      = (*other.MJDStop_[ispw]);
      MJDTimeStamp_[ispw]    = new Vector<Double>;
      (*MJDTimeStamp_[ispw]) = (*other.MJDTimeStamp_[ispw]);
      fieldName_[ispw]       = new Vector<String>;
      (*fieldName_[ispw])    = (*other.fieldName_[ispw]);
      sourceName_[ispw]      = new Vector<String>;
      (*sourceName_[ispw])   = (*other.sourceName_[ispw]);
      fieldId_[ispw]         = new Vector<Int>;
      (*fieldId_[ispw])      = (*other.fieldId_[ispw]);
      antGain_[ispw]         = new Cube<mjJones2>;
      (*antGain_[ispw])      = (*other.antGain_[ispw]);
      antGainOK_[ispw]       = new Cube<Bool>;
      (*antGainOK_[ispw])    = (*other.antGainOK_[ispw]);
    }

  }
  return *this;
}
#endif


// the following reduces the indexing time by 90%, but makes the code
// pretty unredable. It's safe, because visbuffer arrays are guaranteed simple:
// zero origin, no stride (same for intGainCache)
VisBuffer& TimeVarVisJones::apply(VisBuffer& vb) {

  //  cout << " apply " << typeName() << ": ";

  Double* timep=&vb.time()(0);
  Int nRow=vb.nRow();
  Int nDataChan=vb.nChannel();
  
  // Obtain gain matrices appropriate to this VisBuffer
  getThisGain(vb,True,False,True);

  // currentCalSpw_ has been set by getThisGain
  Int spw=currentCalSpw_;
  Int nSolnChan=nSolnChan_(spw);
  Int startChan=startChan_(spw);

  // Data info/indices
  Int* dataChan;
  Bool* flagRowp=&vb.flagRow()(0);
  Int* ant1p=&vb.antenna1()(0);
  Int* ant2p=&vb.antenna2()(0);
  Bool* flagp=&vb.flag()(0,0);
  Bool polSwitch=polznSwitch(vb);
  CStokesVector* visp=&vb.visibility()(0,0);
  
  // Solution info
  Bool* antOKp=   &((*thisJonesOK_[spw])(0));
  
  // iterate rows
  for (Int row=0; row<nRow; row++,flagRowp++,ant1p++,ant2p++,timep++) {
    
    if (!*flagRowp) {  // if this row unflagged
      
      if(antOKp[*ant1p]&&antOKp[*ant2p]) {  // if baseline solution ok
	
	// A pointer to this baseline's gain (spectrum) matrix
	mjJones4* gainp=&((*thisMuellerMat_[spw])(0,*ant1p,*ant2p));   // this is apply()
	dataChan=&vb.channel()(0);
	
	for (Int chn=0; chn<nDataChan; chn++,flagp++,visp++,dataChan++) {
	  // inc soln ch axis if freq-dependent (and dataChan within soln range)
	  if (freqDep() &&      
	      ((*dataChan)>startChan && (*dataChan)<(startChan+nSolnChan)) ) gainp++;  
	  // if this data channel unflagged
	  if (!*flagp) {  
	    if (polSwitch) polznMap(*visp);
	    (*visp) *= (*gainp);
	    if (polSwitch) polznUnmap(*visp);
	  };
	}
      } // antOKp
      else {
	for (Int chn=0; chn<nDataChan; chn++,flagp++,visp++) {
	  *flagp=True;
	}
	vb.flagRow()(row)=True;
      }
    } // !*flagRowp
    else {
      flagp+=nDataChan; visp+=nDataChan;
    }
  }
  return vb;
}

VisBuffer& TimeVarVisJones::applyInv(VisBuffer& vb) {

  Double* timep=&vb.time()(0);

  //  query();

  //  cout << " applyInv " << typeName() << ": ";

  // Obtain gain matrices appropriate to this VisBuffer (doInverse=True)
  getThisGain(vb,True,True,True);

  // currentCalSpw_ has been set by getThisGain
  Int spw=currentCalSpw_;
  Int nSolnChan=nSolnChan_(spw);
  Int startChan=startChan_(spw);


  // Data info/indices
  Int nRow=vb.nRow();
  Int nDataChan=vb.nChannel();
  Int* dataChan;
  Bool* flagRowp=&vb.flagRow()(0);
  Int* ant1p=&vb.antenna1()(0);
  Int* ant2p=&vb.antenna2()(0);
  Bool* flagp=&vb.flag()(0,0);
  Bool polSwitch=polznSwitch(vb);
  CStokesVector* visp=&vb.visibility()(0,0);

  // Solution info
  Bool* antOKp= &((*thisJonesOK_[spw])(0));

  // iterate rows
  for (Int row=0; row<nRow; row++,flagRowp++,ant1p++,ant2p++,timep++) {

    if (!*flagRowp) {  // if this row unflagged

      if(antOKp[*ant1p]&&antOKp[*ant2p]) {  // if baseline solution ok

	// A pointer to this baseline's gain (spectrum) matrix
	mjJones4* gainp=&((*thisMuellerMat_[spw])(0,*ant1p,*ant2p));  // This is applyInv()
	dataChan=&vb.channel()(0);

	for (Int chn=0; chn<nDataChan; chn++,flagp++,visp++,dataChan++) {
	  // inc soln ch axis if freq-dependent (and dataChan within soln range)
	  if (freqDep() &&      
	      ((*dataChan)>startChan && (*dataChan)<(startChan+nSolnChan)) ) gainp++;  

	  // if this channel unflagged
	  if (!*flagp) {  
	    if (polSwitch) polznMap(*visp);
	    (*visp) *= (*gainp);
	    if (polSwitch) polznUnmap(*visp);
	  };
	}
      } // antOKp
      else {
	for (Int chn=0; chn<nDataChan; chn++,flagp++,visp++) {
          *flagp=True;
	}
	vb.flagRow()(row)=True;
      }
    } // !*flagRowp
    else {
      flagp+=nDataChan; visp+=nDataChan;
    }
  }
  return vb;
}


Double TimeVarVisJones::interval() {return interval_;}

void TimeVarVisJones::query() {

  cout << "This is a report of the state of type = " << type() << endl;
  cout << "------------------------------------------------" << endl;
  cout << " vs_                 = " << vs_ << endl;
  cout << " localVS_            = " << localVS_ << endl;

  cout << " numberAnt_          = " << numberAnt_ << endl;
  cout << " numberSpw_          = " << numberSpw_ << endl;
  if (numberSlot_.nelements() > 0) cout << " numberSlot_         = " << numberSlot_ << endl;

  cout << " freqDep()           = " << freqDep() << endl;
  if (nSolnChan_.nelements() > 0) cout << " nSolnChan_          = " << nSolnChan_ << endl;
  if (startChan_.nelements() > 0) cout << " startChan_          = " << startChan_ << endl;

  cout << " interval_           = " << interval_ << endl;
  cout << " deltat_             = " << deltat_ << endl;

  cout << " solved_             = " << solved_ << endl;
  cout << " applied_            = " << applied_  << endl;

  cout << " currentSpw_         = " << currentSpw_ << endl;
  cout << " currentCalSpw_      = " << currentCalSpw_ << endl;
  if (currentSlot_.nelements() > 0) cout << " currentSlot_        = " << currentSlot_ << endl;

  
  //  Int day; day=Int(floor(currentStart_/86400.0));
  cout << " currentStart_       = " << currentStart_ << endl;
  cout << " currentStop_        = " << currentStop_ << endl;

}


// Get the current set of antenna & baseline gains 
//  into the this* cache
void TimeVarVisJones::getThisGain(const VisBuffer& vb, 
				  const Bool& forceAntMat,
				  const Bool& doInverse,
				  const Bool& forceIntMat) {

  
  // TODO:
  //  Re-evalutate need for force* flags
  
  currentSpw_=vb.spectralWindow();

  // realize spw mapping:
  currentCalSpw_= (spwMap_(currentSpw_)>-1) ? spwMap_(currentSpw_) : currentSpw_;

  //  cout << " gTG: ";

  // Calculate antenna gain parameters for current timestamp
  if (!isSolved()) calcAntGainPar(vb);

  // Form antenna gain matrices
  if (forceAntMat && !JMValid(currentCalSpw_)) {
    fillAntGainMat();
  }

  // Invert antenna matrices, if requested and required
  if (doInverse && !MMValid(currentCalSpw_)) {
    invAntGainMat();
  }
  // Form baseline matrices
  if (forceIntMat && !MMValid(currentCalSpw_)) {
    calcIntGainMat();
  }

  //  cout << endl;

}


void TimeVarVisJones::syncGain(const Int& spw,
			       const Double& time,
			       const Int& field,
			       const Bool& doInv) {

  // Realize spw mapping (should move this up) :
  currentCalSpw_= (spwMap_(currentSpw_)>-1) ? spwMap_(currentSpw_) : currentSpw_;

  syncMuellerMat(currentCalSpw_,time,field,doInv);
}

void TimeVarVisJones::syncGain(const Int& spw,
			       const Int& slot,
			       const Bool& doInv) {

  // Realize spw mapping (should move this up) :
  currentCalSpw_= (spwMap_(currentSpw_)>-1) ? spwMap_(currentSpw_) : currentSpw_;

  syncMuellerMat(currentCalSpw_,slot,doInv);
}

Cube<mjJones4>& TimeVarVisJones::syncMuellerMat(const Int& spw,
						const Double& time,
						const Int& field,
						const Bool& doInv) {
  // Update Jones matrices first
  syncJonesMat(spw,time,field,doInv);

  // Now make the Mueller matrices
  if (!MMValid(spw)) calcMuellerMat(spw);

  return (*thisMuellerMat_[spw]);
}

Cube<mjJones4>& TimeVarVisJones::syncMuellerMat(const Int& spw,
						const Int& slot,
						const Bool& doInv) {
  // Update Jones matrices first
  syncJonesMat(spw,slot,doInv);

  // make the Mueller matrices
  if (!MMValid(spw)) calcMuellerMat(spw);

  return (*thisMuellerMat_[spw]);
}

Matrix<mjJones2>& TimeVarVisJones::syncJonesMat(const Int& spw,
						const Double& time,
						const Int& field,
						const Bool& doInv) {

  // Update Jones parameters first
  syncJonesPar(spw,time,field);

  // Now make the Jones matrices
  if (!JMValid(spw)) {
    fillJonesMat(spw);
    // Invert, if requested
    if (doInv) invJonesMat(spw);
  }

  return (*thisJonesMat_[spw]);
}
Matrix<mjJones2>& TimeVarVisJones::syncJonesMat(const Int& spw,
						const Int& slot,
						const Bool& doInv) {

  // Update Jones parameters first
  syncJonesPar(spw,slot);

  // Now make the Jones matrices
  if (!JMValid(spw)) {
    fillJonesMat(spw);
    if (doInv) invJonesMat(spw);
  }

  return (*thisJonesMat_[spw]);
}

Cube<Complex>& TimeVarVisJones::syncJonesPar(const Int& spw,
					     const Double& time,
					     const Int& field) {

  calcJonesPar(spw,time,field);

  return (*thisJonesPar_[spw]);

}
Cube<Complex>& TimeVarVisJones::syncJonesPar(const Int& spw,
					     const Int& slot) {

  if (slot!=currentSlot_(spw)) {
    currentSlot_(spw)=slot;
    calcJonesPar(spw,slot);
  }

  return (*thisJonesPar_[spw]);

}


// Calculate the current antenna gain parameters by 
//  some means (e.g., interpolation, analytic calculation)
void TimeVarVisJones::calcAntGainPar(const VisBuffer& vb) {

  //  cout << " cAGP(gen) ";

  // Current time in data to match to solutions
  Double currTimeStamp(vb.time()(0));

  calcJonesPar(currentCalSpw_,currTimeStamp);

}

void TimeVarVisJones::calcJonesPar(const Int& spw,
				   const Double& time,
				   const Int& field) {

  //  cout << " cJP(gen) ";

  // TODO:
  //  - Add more interpolation types *****

  // References for semantic convenience
   if(  MJDTimeStamp_[spw] == 0 // no slot, so return
	  || jonesPar_[spw] == 0
	  || jonesParOK_[spw] == 0
	  || thisJonesPar_[spw] == 0
	  || thisJonesOK_[spw] == 0 ){
	  return;
	}
  Vector<Double> timelist; timelist.reference( *(MJDTimeStamp_[spw]) );
  Array<Complex> jP;       jP.reference( *(jonesPar_[spw]) );
  Cube<Bool>     jPOK;     jPOK.reference( *(jonesParOK_[spw]) );
  Cube<Complex>  thisJP;   thisJP.reference( *(thisJonesPar_[spw]) );
  Vector<Bool>   thisJPOK; thisJPOK.reference( *(thisJonesOK_[spw]) );

  Double time0; time0=86400.0*floor(timelist(0)/86400.0);

  Int slot(0);
  Bool found(False);

  // Force nearest, in case of only one solution to choose from
  if (numberSlot_(spw)==1) interpType_=="nearest";

  // Discern slot if more than one to choose from
  if (numberSlot_(spw)> 1){

    // Find index in timelist where time would be
    slot=binarySearch(found,timelist,time,numberSlot_(spw),0);

    // Identify this timestamp with the slot just before it
    if (!found && slot > 0) slot--;

  }

  // If nearest, or found an exact timestamp
  if (found || interpType_=="nearest") {
    
    // fine-tune slot selection to nearest
    if (!found && slot!=(numberSlot_(spw)-1) )
      if ( (timelist(slot+1)-time) < (time-timelist(slot)) )
	slot++;
    
    if (slot!=currentSlot_(spw)) {
      currentSlot_(spw)=slot;
      // Copy a specific slot
      calcJonesPar(spw,slot);
    }

  } else if (interpType_=="linear") {

    // Step back by one if at last slot
    if (slot == numberSlot_(spw)-1) slot--;

    // If new slot, calc new linear interp params
    if (currentSlot_(spw)!=slot) {
      currentSlot_(spw)=slot;
      timeRef_(spw) = timelist(slot);
      timeStep_(spw) = timelist(slot+1)-timeRef_(spw);
      
      IPosition lo(4,0,0,0,slot), hi(4,0,0,0,slot+1);
      for (Int iant=0;iant<numberAnt_;iant++) {
	lo(2)=hi(2)=iant;
	
	thisJPOK(iant) = jPOK(0,iant,slot) && jPOK(0,iant,slot+1);

	for (Int ichan=0;ichan<nSolnChan_(spw);ichan++) {
	  lo(1)=hi(1)=ichan;
	  for (Int ipar=0;ipar<nPar_;ipar++) {
	    lo(0)=hi(0)=ipar;
	    if (thisJPOK(iant)) {
	      (*ampRef_[spw])(ipar,ichan,iant) = abs(jP(lo));
	      (*ampSlope_[spw])(ipar,ichan,iant) = 
		abs(jP(hi))-(*ampRef_[spw])(ipar,ichan,iant);
	      
	      (*phaRef_[spw])(ipar,ichan,iant) = arg(jP(lo));
	      (*phaSlope_[spw])(ipar,ichan,iant) = 
		arg(jP(hi))-(*phaRef_[spw])(ipar,ichan,iant);
	      
	      // Catch phase wraps
	      if ((*phaSlope_[spw])(ipar,ichan,iant)>C::pi) 
		(*phaSlope_[spw])(ipar,ichan,iant)-=(2*C::pi);
	      if ((*phaSlope_[spw])(ipar,ichan,iant)<-C::pi) 
		(*phaSlope_[spw])(ipar,ichan,iant)+=(2*C::pi);

	    } else {
	      (*ampRef_[spw])(ipar,ichan,iant) = 1.0;
	      (*ampSlope_[spw])(ipar,ichan,iant) = 0.0;
	      (*phaRef_[spw])(ipar,ichan,iant) = 0.0;
	      (*phaSlope_[spw])(ipar,ichan,iant) = 0.0;
	    }
	  }
	}

      }
    }
    
    
    // Calculate pars at current timestamp:
    Float fTime(0.0), amp(0.0), pha(0.0);
    if (time!=lastTimeStamp_) {
      lastTimeStamp_=time;
      invalidateJM(spw);
      invalidateMM(spw);
      
      // Fractional time across interval
      fTime = Float( (time-timeRef_(spw))/timeStep_(spw) );
      
      // Calculate pars at fTime
      for (Int iant=0;iant<numberAnt_;iant++) {
	for (Int ichan=0;ichan<nSolnChan_(spw);ichan++) {
	  for (Int ipar=0;ipar<nPar_;ipar++) {
	    amp=(*ampRef_[spw])(ipar,ichan,iant) + 
	      (*ampSlope_[spw])(ipar,ichan,iant)*fTime;
	    pha=(*phaRef_[spw])(ipar,ichan,iant) + 
	      (*phaSlope_[spw])(ipar,ichan,iant)*fTime;
	    thisJP(ipar,ichan,iant) = amp*Complex(cos(pha),sin(pha));
	  }
	}
      }
    }
  } else if (interpType_=="aipslin") {

    // Identify this slot with low end of interp range
    if (slot==(numberSlot_(spw)-1)) slot--;
    
    // If new slot, calc new linear interp params
    if (currentSlot_(spw)!=slot) {
      currentSlot_(spw)=slot;
      timeRef_(spw) = timelist(slot);
      timeStep_(spw) = Float(timelist(slot+1)-timeRef_(spw));
      
      IPosition lo(4,0,0,0,slot), hi(4,0,0,0,slot+1);
      for (Int iant=0;iant<numberAnt_;iant++) {
	lo(2)=hi(2)=iant;

	thisJPOK(iant) = jPOK(0,iant,slot) && jPOK(0,iant,slot+1);

	for (Int ichan=0;ichan<nSolnChan_(spw);ichan++) {
	  lo(1)=hi(1)=ichan;
	  for (Int ipar=0;ipar<nPar_;ipar++) {
	    lo(0)=hi(0)=ipar;
	    if (thisJPOK(iant)) {
	      (*ampRef_[spw])(ipar,ichan,iant) = abs(jP(lo));
	      (*ampSlope_[spw])(ipar,ichan,iant) = 
		abs(jP(hi))-(*ampRef_[spw])(ipar,ichan,iant);
	      (*gainRef_[spw])(ipar,ichan,iant) = jP(lo);
	      (*gainSlope_[spw])(ipar,ichan,iant) = jP(hi)-jP(lo);
	    } else {
	      (*ampRef_[spw])(ipar,ichan,iant) = 1.0;
	      (*ampSlope_[spw])(ipar,ichan,iant) = 0.0;
	      (*gainRef_[spw])(ipar,ichan,iant) = Complex(1.0,0.0);
	      (*gainSlope_[spw])(ipar,ichan,iant) = Complex(0.0,0.0);
	    }
	  }
	}
      }
    }
    
    // Calculate pars at current timestamp:
    Float fTime(0.0), amp(0.0);
    Complex gain(Complex(0.0,0.0));
    if (time!=lastTimeStamp_) {
      lastTimeStamp_=time;
      invalidateJM(spw);
      invalidateMM(spw);
      
      // Fractional time across interval
      fTime = Float(time-timeRef_(spw))/timeStep_(spw);
      
      // Calculate pars at fTime
      for (Int iant=0;iant<numberAnt_;iant++) {
	for (Int ichan=0;ichan<nSolnChan_(spw);ichan++) {
	  for (Int ipar=0;ipar<nPar_;ipar++) {
	    amp=(*ampRef_[spw])(ipar,ichan,iant) + 
	      (*ampSlope_[spw])(ipar,ichan,iant)*fTime;
	    gain=(*gainRef_[spw])(ipar,ichan,iant) + 
	      (*gainSlope_[spw])(ipar,ichan,iant)*fTime;
	    thisJP(ipar,ichan,iant) = amp*gain/abs(gain);
	  }
	}
      }
    }
  }

}

void TimeVarVisJones::calcJonesPar(const Int& spw,
				   const Int& slot) {

  Array<Complex> jP;       jP.reference( *(jonesPar_[spw]) );
  Cube<Bool>     jPOK;     jPOK.reference( *(jonesParOK_[spw]) );
  Cube<Complex>  thisJP;   thisJP.reference( *(thisJonesPar_[spw]) );
  Vector<Bool>   thisJPOK; thisJPOK.reference( *(thisJonesOK_[spw]) );

  // copy gain par values to thisJonesPar_
  //  this should be a referencing operation
  {
    IPosition blc(4,0,0,0,slot);
    IPosition trc(4,nPar_-1,nSolnChan_(spw)-1,numberAnt_-1,slot);
    IPosition thisshape(3,nPar_,nSolnChan_(spw),numberAnt_);
    thisJP=jP(blc,trc).reform(thisshape);
  }
  
  {
    IPosition blc(3,0,0,slot);
    IPosition trc(3,0,numberAnt_-1,slot);
    IPosition thisshape(1,numberAnt_);
    thisJPOK = jPOK(blc,trc).reform(thisshape);
  }

  // New params, so invalidate matrices
  invalidateJM(spw);
  invalidateMM(spw);

}

// Fill 2x2 antenna gain matrices (all chans, ants), using
//  methods provided in concrete types

void TimeVarVisJones::fillAntGainMat() {

  //  cout << " fAGM(gen) ";

  fillJonesMat(currentCalSpw_);
}

void TimeVarVisJones::fillJonesMat(const Int& spw) {

  //  cout << " fJM(gen) ";

  // temporaries to reference this spw's parameters & matrices
  Cube<Complex> tJP; tJP.reference( *(thisJonesPar_[spw]));
  Matrix<mjJones2> tJM; tJM.reference( *(thisJonesMat_[spw]));
  
  // Fill matrices for each antenna and channel
  //  ArrayIterator<Complex> pariter(tJP,1);
  //  pariter.origin();

  IPosition blc(3,0,0,0);
  IPosition trc(3,nPar_-1,0,0);
  IPosition vec(1,nPar_);

  for (Int iant=0; iant<numberAnt_; iant++) {
    blc(2)=trc(2)=iant;
    for (Int ichan=0; ichan<nSolnChan_(spw); ichan++) {
      blc(1)=trc(1)=ichan;

      Vector<Complex> par(tJP(blc,trc).reform(vec));

      // Use specialized method to fill matrix
      calcAntGainMat(tJM(ichan,iant),par);

      // advance iterator
      //      pariter.next();
    }
  }
  // JM now ok, MM not yet
  validateJM(spw);
  invalidateMM(spw);
  
}

// Calculate 2x2 Jones matrices from parameters
void TimeVarVisJones::calcAntGainMat(mjJones2& mat, Vector<Complex>& par ) {

  calcJonesMat(mat,par);

}

void TimeVarVisJones::calcJonesMat(mjJones2& mat, Vector<Complex>& par ) {

  // Default version, creates either scalar (nPar_=1) or diagonal (nPar_=2)

  switch (nPar_) {
  case 1: { mat=par[0]; break; };
  case 2: { mat=par; break; };
  }    
  
  
}

// Invert antenna gain matrices
void TimeVarVisJones::invAntGainMat() {

  //  cout << " iAGM ";

  invJonesMat(currentCalSpw_);
}

void TimeVarVisJones::invJonesMat(const Int& spw) {

  //  cout << " iJM ";

  Matrix<mjJones2> tJM;   tJM.reference( *(thisJonesMat_[spw]) );
  Vector<Bool>     tJOK; tJOK.reference( *(thisJonesOK_[spw]) );
  mjJones2 gainInv;
  for (Int iant=0;iant<numberAnt_;iant++) {
    if (tJOK(iant)) {
      for (Int ichan=0; ichan<nSolnChan_[spw]; ichan++) {
	tJM(ichan,iant).inverse(gainInv);
	tJM(ichan,iant)=gainInv;
      }
    }
  }
}


  // Calculate 4x4 baseline gain matrices
void TimeVarVisJones::calcIntGainMat() {

  //  cout << " cIGM ";

  calcMuellerMat(currentCalSpw_);
}
 
void TimeVarVisJones::calcMuellerMat(const Int& spw) { 

  //  cout << " cMM ";

  // References for syntax
  Matrix<mjJones2> tJM1; tJM1.reference(   *(thisJonesMat_[spw]) );
  Vector<Bool>     tJOK; tJOK.reference(   *(thisJonesOK_[spw]) );
  Cube<mjJones4>   tMM;   tMM.reference(   *(thisMuellerMat_[spw]) );
  
  // Now refresh the intGain actual caches
  //  Clean up indexing a la apply/applyInv?
  
  // Form conjugated Jones matrices for use in direct product
  Matrix<mjJones2> tJM2(nSolnChan_(spw),numberAnt_);
  for (Int iant=0;iant<numberAnt_;iant++) {
    for (Int ichan=0; ichan<nSolnChan_[spw]; ichan++) {
      tJM2(ichan,iant)=tJM1(ichan,iant);
      tJM2(ichan,iant).conj();
    }
  }
  
  // Is this initialization expensive? necessary? *****
  tMM  = mjJones4(Complex(1.0,0.0));
  
  // Form basline matrices
  for (Int iant1=0;iant1<numberAnt_;iant1++) {
    for (Int iant2=iant1;iant2<numberAnt_;iant2++) {
      if( tJOK(iant2) && tJOK(iant1) ) {
	for (Int ichan=0; ichan<nSolnChan_[spw]; ichan++) {
	  directProduct(tMM(ichan,iant1,iant2), tJM1(ichan,iant1),tJM2(ichan,iant2));
	}
      }
    }
  }
  
  validateMM(spw);
  
}


// Determine if polarization re-sequencing is required
Bool TimeVarVisJones::polznSwitch(const VisBuffer& vb)
{
  Vector<Int> corr=vb.corrType();
  Bool needToSwitch=False;
  if (corr.nelements() > 2) {
    needToSwitch=(corr(0)==Stokes::XX && corr(1)==Stokes::YY) ||
      (corr(0)==Stokes::RR && corr(1)==Stokes::LL);
  };
  return needToSwitch;
};

// Re-sequence to (XX,XY,YX,YY) or (RR,RL,LR,LL)
void TimeVarVisJones::polznMap(CStokesVector& vis)
{
  Complex vswap=vis(1);
  vis(1)=vis(2);
  vis(2)=vis(3);
  vis(3)=vswap;
};

// Re-sequence to (XX,YY,XY,YX) or (RR,LL,RL,LR)
void TimeVarVisJones::polznUnmap(CStokesVector& vis)
{
  Complex vswap=vis(3);
  vis(3)=vis(2);
  vis(2)=vis(1);
  vis(1)=vswap;
};


// ********************************************************************
// ********************************************************************
// ********************************************************************
// 
//   Specialized non-solvable types
//

PJones::PJones(VisSet& vs) : 
  TimeVarVisJones(vs),
  polFrame_(VisibilityIterator::Circular) {

  // A single paramter type (parallactic angle)
  nPar_=1;

};

PJones::PJones(VisSet& vs,
	       Double interval,
	       Double deltat) : 
  TimeVarVisJones(vs),
  polFrame_(VisibilityIterator::Circular) {

  // This is a single parameter type (the parallactic angle)
  nPar_=1;

  deltat_=deltat;

  RecordDesc recdesc;
  recdesc.addField("t",TpDouble);
  Record rec(recdesc);
  rec.define("t",interval);
  setInterpolation(rec);

};

PJones::~PJones() {}

// Calculate feed angle: includes both
// intrinsic angle of X feed, and parallactic angle  (True?)
void PJones::calcAntGainPar(const VisBuffer& vb) {

  //  cout << " cAGP(P) ";

  // What time is it?
  Double currTimeStamp(vb.time()(0));

  // If new timestamp
  if (currTimeStamp!=thisTimeStamp_(currentCalSpw_)) {
    invalidateJM(currentCalSpw_);
    invalidateMM(currentCalSpw_);

    // Remember polFrame for Matrix calculation
    polFrame_=vb.polFrame();

    // Calculate parallactic angle and store as local parameter
    parang_=vb.feed_pa(currTimeStamp);
    (*thisJonesOK_[currentCalSpw_])=True;
    thisTimeStamp_(currentCalSpw_)=currTimeStamp;
  }

}

void PJones::calcJonesPar(const Int& spw,
			  const Double& time,
			  const Int& field) {

  //  cout << " cJP(P) ";

  // If new timestamp
  if (time!=thisTimeStamp_(spw)) {
    invalidateJM(spw);
    invalidateMM(spw);

    // Remember polFrame for Matrix calculation 
    polFrame_=vs_->iter().polFrame();

    // Calculate parallactic angle from VisIter method
    //  and store as local parameter 
    parang_=vs_->iter().feed_pa(time);

    cout << "parang_(0) = " << parang_(0)*180.0/C::pi << endl;

    (*thisJonesOK_[spw])=True;
    thisTimeStamp_(spw)=time;

  }

//     LogMessage message(LogOrigin("PJones", "calcAntGainPar"));
//     {
//       ostrstream o; o<<"At time "<<MVTime::Format(MVTime::YMD,6)<<
//   		    MVTime(time/86400.0)<<" Antenna 1 pa="<<antpa(0)<<endl;
//       message.message(o);
//       logSink().post(message);
//     }

}

// Form P matrix from paramter
void PJones::fillAntGainMat() {
    
  //  cout << " fJM(P) ";

  fillJonesMat(currentCalSpw_);

}

void PJones::fillJonesMat(const Int& spw) {

  //  cout << " fJM(P) ";

  if(polFrame_==VisibilityIterator::Linear) {
    // Rotation of dipoles
    Matrix<Complex> mat(2,2);
    for (Int iant=0; iant<numberAnt_; iant++) {
      Float pa=parang_(iant);
      mat(0,0)=cos(pa);
      mat(0,1)=sin(pa);
      mat(1,1)=mat(0,0);
      mat(1,0)=-mat(0,1);
      (*thisJonesMat_[spw])(0,iant)=mat;
    }
  }
  else {
    // Phase delay for circular
    Vector<Complex> vec(2);
    for (Int iant=0; iant<numberAnt_; iant++) {
      Float pa=parang_(iant);
      vec(0)=Complex(cos(pa),-sin(pa));
      vec(1)=Complex(cos(pa),sin(pa));
      (*thisJonesMat_[spw])(0,iant)=vec;
    }
  }
  validateJM(spw);

}

void PJones::store(const String& file, const Bool& append) 
{
    throw(AipsError("store is not (yet) implemented for PJones"));
}


// ********************************************************************
// ********************************************************************


CJones::CJones(VisSet& vs) : 
  TimeVarVisJones(vs) {

  // This has no parameters, we will get CJones directly from VisBuffer
  nPar_=0;

  cjones_.resize(numberAnt_);

};

CJones::~CJones() {}

// Get configuration matrix directly from iterator
void CJones::calcAntGainPar(const VisBuffer& vb) {

  // What time is it?
  Double time(vb.time()(0));
  CJones::calcJonesPar(currentCalSpw_,time);

}  
void CJones::calcJonesPar(const Int& spw, const Double& time) {

  // Get CJones from iterator
  vs_->iter().CJones(cjones_);
  invalidateJM(spw);
  invalidateMM(spw);
}

// Specialize fillJM since we aren't using ordinary jP.
void CJones::fillAntGainMat() {

  fillJonesMat(currentCalSpw_);

}

void CJones::fillJonesMat(const Int& spw) {

  for (Int iant=0;iant<numberAnt_;iant++) {
    (*thisJonesMat_[spw])(0,iant)=cjones_(iant);
  }
  validateJM(spw);
}

void CJones::store(const String& file, const Bool& append) 
{
    throw(AipsError("store is not (yet) implemented for CJones"));
}

} //# NAMESPACE CASA - END

