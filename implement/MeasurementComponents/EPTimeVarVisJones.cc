//# EPTimeVarVisJones<T>.cc: Implementation of Jones classes
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

#include <synthesis/MeasurementComponents/EPTimeVarVisJones.h>
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
#include <synthesis/MeasurementComponents/Utils.h>
namespace casa {

// ------------------------------------------------------------------
// Start of methods for a non-solveable Jones matrix for any solely
// visibility indexed gain. This caches the interferometer gains and
// the inverses according to time. It could be generalized to cache on
// something else. The antenna gain is calculated as needed per slot.
//
// From VisSet. 
//
//---------------------------------------------------------------------
//
EPTimeVarVisJones::EPTimeVarVisJones(VisSet& vs) :
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
  interpType_("nearest"),
  hasData(False)
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
  //
  // Remember how many ants, spws
  numberAnt_=vs.numberAnt();
  numberSpw_=vs.numberSpw();
  //
  // Number of slots per spw 
  //
  numberSlot_.resize(numberSpw_);  numberSlot_=0;
  currentSlot_.resize(numberSpw_); currentSlot_=-1;
  //
  // Default spwMap_ is no mapping
  //
  spwMap_.resize(numberSpw_); spwMap_=-1;
  //
  // Number of parameters, this type (none, this is abstract class)
  //
  nPar_=0;
  //
  // Assume 1 channel per spw, starting at 0
  //
  nSolnChan_.resize(numberSpw_); nSolnChan_=1;
  startChan_.resize(numberSpw_); startChan_=0;
  //
  // Remember which PBs were newed:
  //
  newPB_.resize(numberSpw_); newPB_=False;
}
//
//---------------------------------------------------------------------
//
EPTimeVarVisJones::EPTimeVarVisJones(const EPTimeVarVisJones& other) 
{
  //  operator=(other);
}
//
//---------------------------------------------------------------------
//
EPTimeVarVisJones::~EPTimeVarVisJones() 
{
  deleteThisGain();
  deleteMetaCache();
  deleteAntGain();
  deleteInterp();

  if (localVS_ && vs_) {delete vs_; vs_=NULL;};
}
//
//---------------------------------------------------------------------
//
void EPTimeVarVisJones::setInterpolation(const Record& interpolation) 
{
  //  Set up interpolation of applied types (non-solvable)
  //  Inputs:
  //    interpolation           Record&       Contains application params
  //  Inputs from private/protected data
  //    

  //
  // Collect parameters needed at this level
  //
  if (interpolation.isDefined("t")) interval_=interpolation.asDouble("t");
  if (interpolation.isDefined("interp")) interpType_=interpolation.asString("interp");
  //
  // Make local VisSet with correct chunking
  makeLocalVisSet();

  // Collect meta info
  //  initMetaCache();
  //  fillMetaCache();

  //
  // Set up Gain cache (one slot per spw)
  //  initGainCache();
  //
  initThisGain();

  //
  // This is apply context
  //
  setSolved(False);
  setApplied(True);
}
//
//---------------------------------------------------------------------
//
void EPTimeVarVisJones::initInterp(const Record& interpolation) 
{
  LogMessage message(LogOrigin("EPTimeVarVisJones","initInterp"));
  //
  // Initialize interpolation parameters
  //
  interpType_="nearest";
  if (interpolation.isDefined("interp")) 
    {
      interpType_= interpolation.asString("interp");
      
      if (max(numberSlot_)==1) 
	{
	  interpType_="nearest";
	  ostringstream o; o<<"Only one timeslot found for "
			    << typeName()
			    <<" calibration.";
	  message.message(o);
	  logSink().post(message);
	}

      if (interpType_== "linear") 
	{
	  timeRef_.resize(numberSpw_);
	  timeStep_.resize(numberSpw_);
	
	  ampRef_.resize(numberSpw_);   ampRef_=NULL;
	  ampSlope_.resize(numberSpw_); ampSlope_=NULL;
	  phaRef_.resize(numberSpw_);   phaRef_=NULL;
	  phaSlope_.resize(numberSpw_); phaSlope_=NULL;
	
	  for (Int ispw=0; ispw<numberSpw_; ispw++) 
	    {
	      ampRef_[ispw]   = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
	      ampSlope_[ispw] = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
	      phaRef_[ispw]   = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
	      phaSlope_[ispw] = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
	    }
	} 
      else if (interpType_=="aipslin") 
	{ 
	  timeRef_.resize(numberSpw_);
	  timeStep_.resize(numberSpw_);
	
	  ampRef_.resize(numberSpw_);    ampRef_=NULL;
	  ampSlope_.resize(numberSpw_);  ampSlope_=NULL;
	  gainRef_.resize(numberSpw_);   gainRef_=NULL;
	  gainSlope_.resize(numberSpw_); gainSlope_=NULL;
	
	  for (Int ispw=0; ispw<numberSpw_; ispw++) 
	    {
	      ampRef_[ispw]   = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
	      ampSlope_[ispw] = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
	      gainRef_[ispw]   = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
	      gainSlope_[ispw] = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
	    }
	} 
      else  interpType_="nearest";
    }
  ostringstream o; o<<"Applying "
		    << typeName()
		    <<" calibration with "
		    << interpType_
		    <<" interpolation in time.";
  message.message(o);
  logSink().post(message);
  
  // setup spwmap
  if (interpolation.isDefined("spwmap")) 
    {
      Vector<Int> spwmap;
      spwmap=interpolation.asArrayInt("spwmap");
      Int nmap;
      spwmap.shape(nmap);
      
      // Make sure user-specified spwmap isn't too long!
      if (nmap > numberSpw_) 
	{
	  ostringstream o; o<<"Truncating spwmap list to "
			    << numberSpw_
			    <<" values, to match data.";
	  message.message(o);
	  message.priority(LogMessage::WARN);
	  logSink().post(message);
	  message.priority(LogMessage::NORMAL);
	  
	  spwmap.resize(numberSpw_,True);
	  nmap=numberSpw_;
	  
	}
      
      for (Int ispw=0;ispw<nmap;ispw++) 
	{
	if (spwmap(ispw) > -1 && spwmap(ispw)!=ispw) 
	  {
	    spwMap_(ispw)=spwmap(ispw);
	    ostringstream o; o<<"Spwmap: Applying "
			      << typeName()
			      <<" calibration from spw="
			      <<spwMap_(ispw)+1
			      <<" to spw="
			      <<ispw+1;
	    message.message(o);
	    logSink().post(message);
	  } 
	else 
	  {
	    spwMap_(ispw) = -1;
	  }
	}
    }
}
//
//---------------------------------------------------------------------
//
void EPTimeVarVisJones:: deleteInterp() 
{
  if (interpType_!="nearest") 
    {
      for (Int ispw=0; ispw<numberSpw_; ispw++) 
	{
	  if (ampRef_[ispw])   delete ampRef_[ispw];
	  if (ampSlope_[ispw]) delete ampSlope_[ispw];
	  if (interpType_=="linear") 
	    {
	      if (phaRef_[ispw])   delete phaRef_[ispw]; 
	      if (phaSlope_[ispw]) delete phaSlope_[ispw];
	    } 
	  else if (interpType_=="aipslin") 
	    {
	      if (gainRef_[ispw])   delete gainRef_[ispw];
	      if (gainSlope_[ispw]) delete gainSlope_[ispw];
	    }
	}
      ampRef_=NULL;
      ampSlope_=NULL;
      phaRef_=NULL;
      phaSlope_=NULL;
      gainRef_=NULL;
      gainSlope_=NULL;
    }
}
//
//---------------------------------------------------------------------
//
void EPTimeVarVisJones::makeLocalVisSet() 
{
  Block<Int> columns;
  if (interval_==0.0) 
    {
      //
      // include scan iteration
      //
      columns.resize(5);
      columns[0]=MS::ARRAY_ID;
      columns[1]=MS::SCAN_NUMBER;
      columns[2]=MS::FIELD_ID;
      columns[3]=MS::DATA_DESC_ID;
      columns[4]=MS::TIME;
    } 
  else 
    {
      //
      // avoid scan iteration
      //
      columns.resize(4);
      columns[0]=MS::ARRAY_ID;
      columns[1]=MS::FIELD_ID;
      columns[2]=MS::DATA_DESC_ID;
      columns[3]=MS::TIME;
    }

  vs_= new VisSet(*vs_,columns,interval_);
  localVS_=True;
  //
  // find out how many intervals we have
  //
  VisIter& ioc(vs_->iter());
  ioc.originChunks();
  VisBuffer vb(ioc);
  //
  // count number of slots per spw
  //
  for (ioc.originChunks(); ioc.moreChunks();ioc.nextChunk()) 
    numberSlot_[ioc.spectralWindow()]++;
  
  LogMessage message(LogOrigin("EPTimeVarVisJones", "makeLocalVisSet"));
  {
    ostringstream o; o<<"For interval of "<<interval_<<" seconds, found "<<
		       sum(numberSlot_)<<" slots";message.message(o);
    logSink().post(message);
  }
}
//
//---------------------------------------------------------------------
//
// Initialize the time-dependent cache
void EPTimeVarVisJones::initAntGain() 
{
  // TODO:
  //  Consider initialization value (per type)
  //
  // Delete the cache, in case it already exists
  //
  deleteAntGain();
  //
  // Resize PBs according to number of Spw
  //
  antGainPar_.resize(numberSpw_);      antGainPar_=NULL;
  antGainParOK_.resize(numberSpw_);    antGainParOK_=NULL;
  //
  // Construct and resize pointed-to objects for each available spw
  //
  for (Int ispw=0; ispw<numberSpw_; ispw++) 
    {
      uInt nslot=numberSlot_(ispw);
      if (nslot > 0) 
	{
	  IPosition parshape(4,nPar_,nSolnChan_(ispw),numberAnt_,nslot);
	  //      antGainPar_[ispw]   = new Array<Float>(parshape,Float(1.0));
	  antGainPar_[ispw]   = new Array<Float>(parshape,Float(0.0));
	  antGainParOK_[ispw] = new Cube<Bool>(nSolnChan_(ispw),numberAnt_,nslot,False);
	}
    }
}
//
//---------------------------------------------------------------------
//
// Initialize the "this" cache
//
void EPTimeVarVisJones::initThisGain() 
{
  // TODO: Add use of parType_

  currentSlot_.resize(numberSpw_);
  currentSlot_=-1;
  currentStart_=0;
  currentStop_=0;
  //
  // Matrices start out invalid
  //
  thisAGMValid_.resize(numberSpw_);
  thisAGMValid_=False;
  thisIGMValid_.resize(numberSpw_);
  thisIGMValid_=False;

  thisTimeStamp_.resize(numberSpw_);
  thisTimeStamp_=0.0;
  //
  // Delete the cache, in case it already exists
  //
  deleteThisGain();
  //
  // Resize all of the PBs according to number of Spws
  //
  thisAntGainPar_.resize(numberSpw_);
  thisAntGainMat_.resize(numberSpw_);
  thisAntGainOK_.resize(numberSpw_);
  thisIntGainMat_.resize(numberSpw_);
  //
  // Create pointed-to objects and initialize them
  //
  for (Int ispw=0; ispw<numberSpw_; ispw++) 
    {
      thisAntGainPar_[ispw]   = new Cube<Float>(nPar_,nSolnChan_[ispw],numberAnt_);
      //    *(thisAntGainPar_[ispw]) = Complex(1.0,0.0);
      *(thisAntGainPar_[ispw])= Float(1.0);

      thisAntGainMat_[ispw]   = new Matrix<EPmjJones2>(nSolnChan_[ispw],numberAnt_);
      *(thisAntGainMat_[ispw])= EPmjJones2(Float(1.0));

      thisAntGainOK_[ispw]    = new Vector<Bool>(numberAnt_,True);

      thisIntGainMat_[ispw]   = new Cube<EPmjJones4>(nSolnChan_[ispw],numberAnt_, numberAnt_);
      *(thisIntGainMat_[ispw]) = EPmjJones4(Float(1.0));
      thisIntGainMat_[ispw]->unique();
    }
};
//
//---------------------------------------------------------------------
//
// Initialize the slot-dependent meta-data caches
//
void EPTimeVarVisJones::initMetaCache()
{
  //
  // Delete the cache, in case it already exists
  //
  deleteMetaCache();
  //
  // Resize PBs according to number of Spws
  //
  MJDStart_.resize(numberSpw_);     MJDStart_=NULL;
  MJDStop_.resize(numberSpw_);      MJDStop_=NULL;
  MJDTimeStamp_.resize(numberSpw_); MJDTimeStamp_=NULL;
  fieldName_.resize(numberSpw_);    fieldName_=NULL;
  sourceName_.resize(numberSpw_);   sourceName_=NULL;
  fieldId_.resize(numberSpw_);      fieldId_=NULL;
  //
  // Construct and resize pointed-to objects for each available spw
  //
  for (Int ispw=0; ispw<numberSpw_; ispw++) 
    {
      uInt nslot=numberSlot_(ispw);
      if (nslot > 0) 
	{
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
//
//---------------------------------------------------------------------
//
// Fill the slot-dependent meta-data caches with info
void EPTimeVarVisJones::fillMetaCache() 
{
  // TODO: 
  // 1. Can we use setRowBlocking() here to avoid in-chunk iterations?

  //
  // Fill the cache within info
  //
  VisIter& ioc(vs_->iter());
  ioc.originChunks();
  VisBuffer vb(ioc);
  for (ioc.originChunks(); ioc.moreChunks(); ioc.nextChunk()) 
    {
      Int thisSpw=ioc.spectralWindow();
      //
      // Count slots per spw
      //
      uInt islot= (++currentSlot_(thisSpw));
      //
      // Set field source info
      //
      (*fieldId_[thisSpw])(islot)   =ioc.fieldId();
      (*fieldName_[thisSpw])(islot) =ioc.fieldName();
      (*sourceName_[thisSpw])(islot)=ioc.sourceName();
      //
      // Set time info
      //
      ioc.origin();
      (*MJDStart_[thisSpw])(islot)  =vb.time()(0)-vb.timeInterval()(0)/2.0;
      Double timeStamp(0.0);
      Int ntime(0);
      for (ioc.origin(); ioc.more(); ioc++,ntime++) timeStamp+=vb.time()(0);

      (*MJDStop_[thisSpw])(islot) =vb.time()(0)+vb.timeInterval()(0)/2.0;
      (*MJDTimeStamp_[thisSpw])(islot)=timeStamp/ntime;
    }
}
//
//---------------------------------------------------------------------
//
void EPTimeVarVisJones::deleteThisGain() 
{
  uInt nCache=thisAntGainMat_.nelements();
  if (nCache > 0) 
    for (Int ispw=0; ispw<numberSpw_; ispw++) 
      {
	if (thisAntGainPar_[ispw]) {delete thisAntGainPar_[ispw];thisAntGainPar_[ispw]=NULL;};
	if (thisAntGainMat_[ispw]) {delete thisAntGainMat_[ispw];thisAntGainMat_[ispw]=NULL;};
	if (thisAntGainOK_[ispw])  {delete thisAntGainOK_[ispw]; thisAntGainOK_[ispw]=NULL;};
	if (thisIntGainMat_[ispw]) {delete thisIntGainMat_[ispw];thisIntGainMat_[ispw]=NULL;};
      }
}
//
//---------------------------------------------------------------------
//
void EPTimeVarVisJones::deleteAntGain() 
{
  uInt nCache=antGainPar_.nelements();
  if (nCache > 0) 
    for (Int ispw=0; ispw<numberSpw_; ispw++) 
      {
	//	cout << newPB_(ispw) << " " << numberSpw_ << " " << antGainPar_[ispw] << endl;
	if (newPB_(ispw)) 
	  {
	    if (antGainPar_[ispw])    delete antGainPar_[ispw];
	    if (antGainParOK_[ispw])  delete antGainParOK_[ispw];
	  }
	antGainPar_[ispw]=NULL;
	antGainParOK_[ispw]=NULL;
      }
}
//
//---------------------------------------------------------------------
//
void EPTimeVarVisJones::deleteMetaCache() 
{
  uInt nCache=MJDStart_.nelements();
  if (nCache > 0) 
    for (Int ispw=0; ispw<numberSpw_; ispw++) 
      {
	if (newPB_(ispw)) 
	  {
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
//
//---------------------------------------------------------------------
//
#undef ASSIGN
#ifdef ASSIGN
// Assignment
EPTimeVarVisJones& EPTimeVarVisJones::operator=(const EPTimeVarVisJones& other) 
{
  if(this!=&other) 
    {
      solved_=other.solved_;
      applied_=other.applied_;
      interval_=other.interval_;
      deltat_=other.deltat_;
      numberAnt_=other.numberAnt_;
      numberSlot_=other.numberSlot_;
      currentSlot_=other.currentSlot_;
      numberSpw_=other.numberSpw_;  // new numberSpw_
      //
      // Delete existing PBs, then Resize according to number of Spws
      //
      MJDStart_.resize(numberSpw_);
      MJDStop_.resize(numberSpw_);
      MJDTimeStamp_.resize(numberSpw_);
      fieldName_.resize(numberSpw_);
      sourceName_.resize(numberSpw_);
      fieldId_.resize(numberSpw_);
      antGain_.resize(numberSpw_);
      antGainOK_.resize(numberSpw_);  
      initGainCache();
      //
      // Allocate and fill pointed-to objects
      //
      for (Int ispw=0; ispw<numberSpw_; ispw++) 
	{
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
//
//---------------------------------------------------------------------
//
// the following reduces the indexing time by 90%, but makes the code
// pretty unredable. It's safe, because visbuffer arrays are guaranteed simple:
// zero origin, no stride (same for intGainCache)
/*
VisBuffer& EPTimeVarVisJones::apply(VisBuffer& vb) {

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
  Bool* flagRowp      = &vb.flagRow()(0);
  Int* ant1p          = &vb.antenna1()(0);
  Int* ant2p          = &vb.antenna2()(0);
  Bool* flagp         = &vb.flag()(0,0);
  Bool polSwitch      = polznSwitch(vb);
  CStokesVector* visp = &vb.visibility()(0,0);
  
  // Solution info
  Bool* antOKp        = &((*thisAntGainOK_[spw])(0));
  
  // iterate rows

  for (Int row=0; row<nRow; row++,flagRowp++,ant1p++,ant2p++,timep++)
    {
      if (!*flagRowp)  // if this row unflagged
	{ 
	  if(antOKp[*ant1p]&&antOKp[*ant2p]) // if baseline solution ok
	    {
	      // A pointer to this baseline's gain (spectrum) matrix
	      EPmjJones4* gainp=&((*thisIntGainMat_[spw])
				  (0,*ant1p,*ant2p));   // this is apply()
	      dataChan=&vb.channel()(0);
	
	      for (Int chn=0; chn<nDataChan; chn++,flagp++,visp++,dataChan++) 
		{// inc soln ch axis if freq-dependent (and dataChan within soln range)
		  if (freqDep() &&      
		      ((*dataChan)>startChan && 
		       (*dataChan)<(startChan+nSolnChan)))
		    gainp++; 
		  // if this data channel unflagged
		  if (!*flagp) 
		    {  
		      if (polSwitch) polznMap(*visp);
		      (*visp) *= (*gainp);
		      if (polSwitch) polznUnmap(*visp);
		    };
		}
	    } // antOKp
	  else 
	    {
	      for (Int chn=0; chn<nDataChan; chn++,flagp++,visp++) *flagp=True;
	      vb.flagRow()(row)=True;
	    }
	} // !*flagRowp
      else 
	flagp+=nDataChan; visp+=nDataChan;
    }

  return vb;
}
//
//---------------------------------------------------------------------
//
VisBuffer& EPTimeVarVisJones::applyInv(VisBuffer& vb) {

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
  Bool* antOKp= &((*thisAntGainOK_[spw])(0));

  // iterate rows
  for (Int row=0; row<nRow; row++,flagRowp++,ant1p++,ant2p++,timep++) {

    if (!*flagRowp) {  // if this row unflagged

      if(antOKp[*ant1p]&&antOKp[*ant2p]) {  // if baseline solution ok

	// A pointer to this baseline's gain (spectrum) matrix
	EPmjJones4* gainp=&((*thisIntGainMat_[spw])(0,*ant1p,*ant2p));  // This is applyInv()
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
*/
//
//---------------------------------------------------------------------
//
Double EPTimeVarVisJones::interval() {return interval_;}
//
//---------------------------------------------------------------------
//
void EPTimeVarVisJones::query() 
{
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
//
//---------------------------------------------------------------------
//
// Get the current set of antenna & baseline gains 
//  into the this* cache
void EPTimeVarVisJones::getThisGain(const VisBuffer& vb, 
				  const Bool& forceAntMat,
				  const Bool& doInverse,
				  const Bool& forceIntMat) 
{
  //  if (!hasData) 
  //    throw(AipsError("Data not loaded"));
  // TODO:
  //  Re-evalutate need for force* flags
  //  
  currentSpw_=vb.spectralWindow();
  //
  // realize spw mapping:
  //
  currentCalSpw_= (spwMap_(currentSpw_)>-1) ? spwMap_(currentSpw_) : currentSpw_;
  //
  //  cout << " gTG: ";
  //
  // Calculate antenna gain parameters for current timestamp
  //
  //  if (!isSolved())                                
    calcAntGainPar(vb);
  //
  // Form antenna gain matrices
  //
  if (forceAntMat && !AGMValid(currentCalSpw_))   fillAntGainMat();
  //
  // Invert antenna matrices, if requested and required
  //
  if (doInverse && !IGMValid(currentCalSpw_))     invAntGainMat();
  //
  // Form baseline matrices
  //
  if (forceIntMat && !IGMValid(currentCalSpw_))   calcIntGainMat();
}
//
//---------------------------------------------------------------------
//
// Calculate the current antenna gain parameters by 
//  some means (e.g., interpolation, analytic calculation)
void EPTimeVarVisJones::calcAntGainPar(const VisBuffer& vb) 
{
  // TODO:
  //  - WHAT ABOUT thisAntGainOK_ = antGainOK_ setting??? ****
  //      -> see old getAntGain methods (esp. SVJ)
  //  - Add more interpolation types *****
  
  //
  // Current time in data to match to solutions
  //
  //  Double currTimeStamp(vb.time()(0));
  Double currTimeStamp(getCurrentTimeStamp(vb));
  //
  // References for semantic convenience
  //
  Vector<Double> timelist;  timelist. reference( *(MJDTimeStamp_[currentCalSpw_]) );
  Array<Float>   aGP;       aGP.      reference( *(antGainPar_[currentCalSpw_]) );
  Cube<Bool>     aGPOK;     aGPOK.    reference( *(antGainParOK_[currentCalSpw_]) );
  Cube<Float>    thisAGP;   thisAGP.  reference( *(thisAntGainPar_[currentCalSpw_]) );
  Vector<Bool>   thisAGPOK; thisAGPOK.reference( *(thisAntGainOK_[currentCalSpw_]) );

  Double time0; time0=86400.0*floor(timelist(0)/86400.0);

  //  cout << MJDTimeStamp_.nelements() << endl;
  //  cout << *(MJDTimeStamp_[0])/1.0e9 << endl;
  Int slot(0);
  Bool found(False);
  //
  // Find index in timelist where currTimeStamp would be
  //
  if (numberSlot_(currentCalSpw_)>1) 
    slot=binarySearch(found,timelist,currTimeStamp,numberSlot_(currentCalSpw_),0);

  //  cout << "Timelist: " << timelist-4.60203e+9 << " " << currTimeStamp-4.60203e9 << endl;

  if (found || interpType_=="nearest") 
    {
      //
      // fine-tune slot selection
      //
      if (!found && slot!=0) 
	if ( slot>numberSlot_(currentCalSpw_)-1 ||
	     (timelist(slot)-currTimeStamp) > (currTimeStamp-timelist(slot-1)) )
	  slot--;
      //
      // If we have found a new slot, get new gain values
      //
      if (currentSlot_(currentCalSpw_)!=slot) 
	{
	  currentSlot_(currentCalSpw_)=slot;
      
	  invalidateAGM(currentCalSpw_);
	  invalidateIGM(currentCalSpw_);
	  //
	  // copy gain par values to thisAntGainPar_
	  //  this should be a referencing operation for NEAREST
	  {
	    IPosition blc(4,0,0,0,slot);
	    IPosition trc(4,nPar_-1,nSolnChan_(currentCalSpw_)-1,numberAnt_-1,slot);
	    IPosition thisshape(3,nPar_,nSolnChan_(currentCalSpw_),numberAnt_);
	    thisAGP=aGP(blc,trc).reform(thisshape);
	    /*
	    cout << "Pointing offsets: " 
		 << currTimeStamp << " "
		 << slot << " " << currTimeStamp-time0 
		 << " " << timelist(slot)-time0 << " "
		 << currTimeStamp-timelist(slot) << " "
		 << slot << thisAGP << endl;
	    */
	  }
	  {
	    IPosition blc(3,0,0,slot);
	    IPosition trc(3,0,numberAnt_-1,slot);
	    IPosition thisshape(1,numberAnt_);
	    thisAGPOK = aGPOK(blc,trc).reform(thisshape);
	  }
	}
      else if (interpType_=="linear") 
	{ // linear
	  //
	  // Identify this slot with low end of interp range
	  //
	  if (slot > 0) slot--;
	  if (slot==(numberSlot_(currentCalSpw_)-1)) slot--;
	  //
	  // If new slot, calc new linear interp params
	  //
	  if (currentSlot_(currentCalSpw_)!=slot) 
	    {
	      currentSlot_(currentCalSpw_)=slot;
	      timeRef_(currentCalSpw_) = timelist(slot);
	      timeStep_(currentCalSpw_) = Float(timelist(slot+1)-timeRef_(currentCalSpw_));
      
	      IPosition lo(4,0,0,0,slot), hi(4,0,0,0,slot+1);
	      for (Int iant=0;iant<numberAnt_;iant++)
		{
		  lo(2)=hi(2)=iant;
	
		  thisAGPOK(iant) = aGPOK(0,iant,slot) && aGPOK(0,iant,slot+1);

		  for (Int ichan=0;ichan<nSolnChan_(currentCalSpw_);ichan++) 
		    {
		      lo(1)=hi(1)=ichan;
		      for (Int ipar=0;ipar<nPar_;ipar++) 
			{
			  lo(0)=hi(0)=ipar;
			  if (thisAGPOK(iant)) 
			    {
			      (*ampRef_[currentCalSpw_])(ipar,ichan,iant) = (aGP(lo));
			      (*ampSlope_[currentCalSpw_])(ipar,ichan,iant) = 
				(aGP(hi))-(*ampRef_[currentCalSpw_])(ipar,ichan,iant);
			    } 
			  else 
			    {
			      (*ampRef_[currentCalSpw_])(ipar,ichan,iant) = 1.0;
			      (*ampSlope_[currentCalSpw_])(ipar,ichan,iant) = 0.0;
			    }
			}
		    }
		}
	    }
    
	  //
	  // Calculate pars at current timestamp:
	  //
	  Float fTime(0.0), amp(0.0), pha(0.0);
	  if (currTimeStamp!=lastTimeStamp_) 
	    {
	      lastTimeStamp_=currTimeStamp;
	      invalidateAGM(currentCalSpw_);
	      invalidateIGM(currentCalSpw_);
	      //
	      // Fractional time across interval
	      //
	      fTime = Float(currTimeStamp-timeRef_(currentCalSpw_))/timeStep_(currentCalSpw_);
	      //
	      // Calculate pars at fTime
	      //
	      for (Int iant=0;iant<numberAnt_;iant++) 
		{
		  for (Int ichan=0;ichan<nSolnChan_(currentCalSpw_);ichan++) 
		    {
		      for (Int ipar=0;ipar<nPar_;ipar++) 
			{
			  amp=(*ampRef_[currentCalSpw_])(ipar,ichan,iant) + 
			    (*ampSlope_[currentCalSpw_])(ipar,ichan,iant)*fTime;
			  thisAGP(ipar,ichan,iant) = amp;
			}
		    }
		}
	    }
	}
      //Linear
    }
}
//
//---------------------------------------------------------------------
//
// Fill 2x2 antenna gain matrices (all chans, ants), using
//  methods provided in concrete types
//
void EPTimeVarVisJones::fillAntGainMat() 
{
  //
  // temporaries to reference this spw's parameters & matrices
  //
  Cube<Float> tAGP; tAGP.reference( *(thisAntGainPar_[currentCalSpw_]));
  Matrix<EPmjJones2> tAGM; tAGM.reference( *(thisAntGainMat_[currentCalSpw_]));
  //
  // Fill matrices for each antenna and channel
  //  ArrayIterator<Complex> pariter(tAGP,1);
  //  pariter.origin();
  //
  IPosition blc(3,0,0,0);
  IPosition trc(3,nPar_-1,0,0);
  IPosition vec(1,nPar_);

  for (Int iant=0; iant<numberAnt_; iant++) {
    blc(2)=trc(2)=iant;
    for (Int ichan=0; ichan<nSolnChan_(currentCalSpw_); ichan++) {
      blc(1)=trc(1)=ichan;

      Vector<Float> par(tAGP(blc,trc).reform(vec));
      //
      // Use specialized method to fill matrix
      //
      calcAntGainMat(tAGM(ichan,iant),par);

      // advance iterator
      //      pariter.next();
    }
  }
  //
  // AGM now ok, IGM not yet
  //
  validateAGM(currentCalSpw_);
  invalidateIGM(currentCalSpw_);
}
//
//---------------------------------------------------------------------
//
// Calculate 2x2 Jones matrices from parameters
void EPTimeVarVisJones::calcAntGainMat(EPmjJones2& mat, 
				       Vector<Float>& par ) 
{
  // Default version, creates either scalar (nPar_=1) or diagonal (nPar_=2)

  switch (nPar_) {
  case 1: { mat=par[0]; break; };
  case 2: { mat=par; break; };
  }    
}
//
//---------------------------------------------------------------------
//
// Invert antenna gain matrices
void EPTimeVarVisJones::invAntGainMat() 
{
  //  cout << " iAGM ";

  Matrix<EPmjJones2> tAGM;   tAGM.reference( *(thisAntGainMat_[currentCalSpw_]) );
  Vector<Bool>     tAGOK; tAGOK.reference( *(thisAntGainOK_[currentCalSpw_]) );
  EPmjJones2 gainInv;
  for (Int iant=0;iant<numberAnt_;iant++) 
    if (tAGOK(iant)) 
      for (Int ichan=0; ichan<nSolnChan_[currentCalSpw_]; ichan++) 
	{
	  tAGM(ichan,iant).inverse(gainInv);
	  tAGM(ichan,iant)=gainInv;
	}
}
//
//---------------------------------------------------------------------
//
// Determine if polarization re-sequencing is required
Bool EPTimeVarVisJones::polznSwitch(const VisBuffer& vb)
{
  Vector<Int> corr=vb.corrType();
  Bool needToSwitch=False;
  if (corr.nelements() > 2) 
    {
      needToSwitch=(corr(0)==Stokes::XX && corr(1)==Stokes::YY) ||
	(corr(0)==Stokes::RR && corr(1)==Stokes::LL);
    };
  return needToSwitch;
};
//
//---------------------------------------------------------------------
//
// Re-sequence to (XX,XY,YX,YY) or (RR,RL,LR,LL)
void EPTimeVarVisJones::polznMap(CStokesVector& vis)
{
  Complex vswap=vis(1);
  vis(1)=vis(2);
  vis(2)=vis(3);
  vis(3)=vswap;
};
//
//---------------------------------------------------------------------
//
// Re-sequence to (XX,YY,XY,YX) or (RR,LL,RL,LR)
void EPTimeVarVisJones::polznUnmap(CStokesVector& vis)
{
  Complex vswap=vis(3);
  vis(3)=vis(2);
  vis(2)=vis(1);
  vis(1)=vswap;
};
//
//---------------------------------------------------------------------
//
// Calculate 4x4 baseline gain matrices
/*
void EPTimeVarVisJones::calcIntGainMat() 
{
  //  cout << " cIGM ";

  // References for syntax
  Matrix<EPmjJones2> tAGM1; tAGM1.reference(   *(thisAntGainMat_[currentCalSpw_]) );
  Vector<Bool>     tAGOK; tAGOK.reference(   *(thisAntGainOK_[currentCalSpw_]) );
  Cube<EPmjJones4>   tIGM;   tIGM.reference(   *(thisIntGainMat_[currentCalSpw_]) );
  
  // Now refresh the intGain actual caches
  //  Clean up indexing a la apply/applyInv?
  
  // Form conjugated Jones matrices for use in direct product
  Matrix<EPmjJones2> tAGM2(nSolnChan_(currentCalSpw_),numberAnt_);
  for (Int iant=0;iant<numberAnt_;iant++) {
    for (Int ichan=0; ichan<nSolnChan_[currentCalSpw_]; ichan++) {
      tAGM2(ichan,iant)=tAGM1(ichan,iant);
      tAGM2(ichan,iant).conj();
    }
  }
  
  // Is this initialization expensive? necessary? *****
  tIGM  = EPmjJones4(Float(1.0));
  
  // Form basline matrices
  for (Int iant1=0;iant1<numberAnt_;iant1++) {
    for (Int iant2=iant1;iant2<numberAnt_;iant2++) {
      if( tAGOK(iant2) && tAGOK(iant1) ) {
	for (Int ichan=0; ichan<nSolnChan_[currentCalSpw_]; ichan++) {
	  directProduct(tIGM(ichan,iant1,iant2), tAGM1(ichan,iant1),tAGM2(ichan,iant2));
	}
      }
    }
  }
  
  validateIGM(currentCalSpw_);
}
*/
}
