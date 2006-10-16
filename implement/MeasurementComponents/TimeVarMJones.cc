//# TimeVarMJones.cc: Implementation of Jones classes
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2003
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

#include <synthesis/MeasurementComponents/TimeVarMJones.h>
#include <calibration/CalTables/CalTable.h>
#include <calibration/CalTables/TimeVarMJDesc.h>
#include <calibration/CalTables/TimeVarMJMRec.h>
#include <msvis/MSVis/VisBuffer.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
namespace casa { //# NAMESPACE CASA - BEGIN

// ------------------------------------------------------------------
// Start of methods for a non-solveable Jones matrix for any solely
// visibility indexed gain. This caches the interferometer gains and
// the inverses according to time. It could be generalized to cache on
// something else. 

// From VisSet. This constructor makes an array of mjJones2 of
// type ScalarIdentity. This works for all derived classes
// since the first reassigment of form is done in the solve
// method where for example, DJones.solve inserts true matrices
TimeVarMJones::TimeVarMJones(VisSet& vs, Double interval, Double deltat) : 
  numberOfSlots_(0), interval_(interval), deltat_(deltat)
{
  initialize(vs);
}

TimeVarMJones::TimeVarMJones(const TimeVarMJones& other) {
  operator=(other);
}

TimeVarMJones::~TimeVarMJones() {}

// Assignment
TimeVarMJones& TimeVarMJones::operator=(const TimeVarMJones& other) {
  if(this!=&other) {
    numberOfSlots_=other.numberOfSlots_;
    currentSlot_=other.currentSlot_;
    interval_=other.interval_;
    deltat_=other.deltat_;
    MJDStart_=other.MJDStart_;
    MJDStop_=other.MJDStop_;
    fieldName_=other.fieldName_;
    sourceName_=other.sourceName_;
    initializeCache();
    intGain_.resize(other.intGain_.shape());
    intGain_=other.intGain_;
  }
  return *this;
}

VisBuffer& TimeVarMJones::apply(VisBuffer& vb) {
  Int spw=vb.spectralWindow();
  for (Int row=0; row<vb.nRow(); row++) {
    checkCache(vb,row);
    const mjJones4& j=intGainCache_(vb.antenna1()(row),
				    vb.antenna2()(row),spw);
    for (Int chn=0; chn<vb.nChannel(); chn++) {
      if (!vb.flag()(chn,row)) vb.visibility()(chn,row)*=j;
    }
  }
  return vb;
}

VisBuffer& TimeVarMJones::applyInv(VisBuffer& vb) {
  Int spw=vb.spectralWindow();
  for (Int row=0; row<vb.nRow(); row++) {
    checkCache(vb,row);
    const mjJones4& j=intGainInvCache_(vb.antenna1()(row),
				       vb.antenna2()(row),spw);
    for (Int chn=0; chn<vb.nChannel(); chn++) {
      if (!vb.flag()(chn,row)) vb.visibility()(chn,row)*=j;
    }
  }
  return vb;
}

// Write the contents to a table
void TimeVarMJones::store (const String& file, const Bool& append)
{
// Write the solutions to an output calibration table
// Input:
//    file           String        Cal table name
//    append         Bool          Append if true, else overwrite
//
  // Initialization:
  // No. of rows in cal_main, cal_desc and cal_history
  Int nMain = 0; 
  Int nDesc = 0;
  Int nHist = 0;
  
  // Calibration table
  CalTable* tab;

  // Open the output file if it already exists and is being appended to.
  if (append && Table::isWritable (file)) {
    tab  = new CalTable (file, Table::Update);
    nMain = tab->nRowMain();
    nDesc = tab->nRowDesc();
    nHist = tab->nRowHistory();
  } else {
    // Create a new calibration table
    TimeVarMJonesDesc tvmjDesc;
    tab = new CalTable (file, tvmjDesc);
  };

  // Declarations
  Int islot, iant, jant, ispw, i, j;
  
  // Sub-table records
  CalDescRecord* descRec;
  TimeVarMJonesMRec* mainRec;

  // Cal_desc fields
  Vector <Int> spwId;
  Double dzero = 0;
  IPosition ip(1,1);
  Array <Double> chanFreq(ip, dzero); 
  Array <Double> chanWidth(ip, dzero);
  Array <String> polznType(ip, "");
  Array <Int> chanRange(ip, 0);
  Array <Int> numChan(ip,1);

  // Fill the cal_desc record
  descRec = new CalDescRecord;
  descRec->defineNumSpw (numberSpw_);
  descRec->defineNumChan (numChan);
  descRec->defineNumReceptors (2);
  descRec->defineNJones (4);
  spwId.resize (numberSpw_);
  for (i = 0; i < numberSpw_; i++) {
    spwId(i) = i;
  };
  descRec->defineSpwId (spwId);
  descRec->defineChanFreq (chanFreq);
  descRec->defineChanWidth (chanWidth);
  descRec->defineChanRange (chanRange);
  descRec->definePolznType (polznType);
  descRec->defineJonesType ("full");
  descRec->defineMSName ("");
  
  // Write the cal_desc record
  tab->putRowDesc (nDesc++, *descRec);
  delete descRec;

  // Cal_main fields
  Array <Double> refDir(IPosition(1,2), dzero);
  Complex czero = 0;
  Array <Complex> gain(IPosition(3,4,4,numberSpw_), czero);

  // Loop over the number of time slots
  for (islot = 0; islot < numberOfSlots_; islot++) {

    // Loop over all interferometer baselines
    for (iant = 0; iant < numberAnt_; iant++) {
      for (jant = iant; jant < numberAnt_; jant++) {

	// Fill the cal_main record
	mainRec = new TimeVarMJonesMRec;

	// Fill MS labels
	mainRec->defineTime ((MJDStart_(islot) + MJDStop_(islot)) / 2.0);
	mainRec->defineTimeEP (0);
	mainRec->defineInterval (MJDStop_(islot) - MJDStart_(islot));
	mainRec->defineAntenna1 (iant);
	mainRec->defineFeed1 (0);
	mainRec->defineAntenna2 (jant);
	mainRec->defineFeed2 (0);
	mainRec->defineFieldId (0);
	mainRec->defineArrayId (0);
	mainRec->defineObsId (0);
	mainRec->defineScanNo (0);
	mainRec->defineProcessorId (0);
	mainRec->definePulsarBin (0);
	mainRec->definePulsarGateId (0);
	mainRec->defineFreqGrp (0);
	mainRec->defineFieldName (fieldName_(islot));
	mainRec->defineSourceName (sourceName_(islot));
	
	// Fill gain information
	for (ispw = 0; ispw < numberSpw_; ispw++) {
	  for (i = 0; i < 4; i++) {
	    for (j = 0; j < 4; j++) {
	      gain(IPosition(3,i,j,ispw)) = 
		((const mjJones4&)
		 intGain_(IPosition(4,iant,jant,ispw,islot)))(i,j);
	    };
	  };
	};
	mainRec->defineGain (gain);
	
	//	mainRec->defineRefAnt (0);
	//	mainRec->defineRefFeed (0);
	//	mainRec->defineRefReceptor (0);
	//	mainRec->defineRefFreq (0);
	
	// Fill indices for cal_desc and cal_history
	mainRec->defineCalDescId (nDesc - 1);
	mainRec->defineCalHistoryId (0);
	
	// Write the cal_main record
	tab->putRowMain (nMain++, *mainRec);
	
	delete mainRec;
      };
    };
  };
    
  // Clean up pointers
  delete tab;
};

// Load from a table
void TimeVarMJones::load (const String& file, const String& select, 
			  const String& type)
{
// Load data from a calibration table
// Input:
//    file         const String&          Cal table name
//    select       const String&          Selection string
//    type         const String&          Jones matrix type
//                                        (scalar, diagonal or general)
//
  // Decode the Jones matrix type
  Int jonesType = 0;
  if (type == "scalar") jonesType = 1;
  if (type == "diagonal") jonesType = 2;
  if (type == "general") jonesType = 3;

  // Open the calibration table
  CalTable ctab (file);

  // Select on the TAQL selection string
  CalTable selTab = ctab.select (select);

  // Sort on TIME in ascending order
  Block <String> sortCol(1,"TIME");
  CalTable tab = selTab.sort (sortCol);

  // Get calibration dimensions and parameters. Simple assumptions
  // are made at this point.

  // Get no. of antennas and time slots
  numberOfSlots_ = tab.numberTimeSlots (0.01);
  numberAnt_ = tab.maxAntenna() + 1;

  // Get no of spectral windows (assumed constant across table)
  CalDescRecord* calDescRec = new CalDescRecord (tab.getRowDesc(0));
  calDescRec->getNumSpw (numberSpw_);
  delete calDescRec;

  // Get the solution interval (assumed constant across the table)
  Double interval, deltat;
  TimeVarMJonesMRec* timeVarMJMRec = new TimeVarMJonesMRec (tab.getRowMain(0));
  timeVarMJMRec->getInterval (interval);
  deltat = 0.01 * interval;
  delete timeVarMJMRec;

  // Re-size the cache variables
  MJDStart_.resize (numberOfSlots_);
  MJDStop_.resize (numberOfSlots_);
  fieldName_.resize (numberOfSlots_);
  sourceName_.resize (numberOfSlots_);
  intGain_.resize 
    (IPosition(4,numberAnt_, numberAnt_, numberSpw_, numberOfSlots_));
  
  // Cache initialization
  MJDStart_ = 0;
  MJDStop_ = 0;
  fieldName_ = "";
  sourceName_ = "";

  // Initialize intGain_ depending on matrix type
  Matrix <Complex> lmat(4,4);
  Vector <Complex> lvec(4);
  switch (jonesType) {
  case 1: {
    intGain_ = mjJones4 (Complex (1.0, 0.0));
    break;
  };
  case 2: {
    lvec = Complex (1.0, 0.0);
    intGain_ = mjJones4 (lvec);
    break;
  };
  case 3: {
    lmat = Complex (0.0, 0.0);
    lmat(0,0) = lmat(1,1) = lmat(2,2) = lmat(3,3) = Complex (1.0, 0.0);
    intGain_ = mjJones4 (lmat);
  };
  }; // switch (jonesType)...

  // Declarations
  Complex cmplx;
  Double time, lastTime;
  Int islot, iant, jant, ispw, i, j, irow, nrow;
  Array <Complex> gains (IPosition(3,4,4,numberSpw_));

  // Read the calibration information
  nrow = tab.nRowMain();
  lastTime = 0;
  islot = 0;

  for (irow = 0; irow < nrow; irow++) {

    // Read a record from cal_main
    timeVarMJMRec = new TimeVarMJonesMRec (tab.getRowMain (irow));

    // Decode the relevant fields and copy to cache
    timeVarMJMRec->getAntenna1 (iant);
    timeVarMJMRec->getAntenna2 (jant);
    timeVarMJMRec->getTime (time);
    timeVarMJMRec->getInterval (interval);

    // Compute the time slot index in the cache
    if (irow == 0) {
      islot = 0;
    } else {
      if (abs (time - lastTime) > deltat) {
	islot = islot + 1;
      };
    };
    lastTime = time;

    MJDStart_(islot) = time - interval / 2.0;
    MJDStop_(islot) = time + interval / 2.0;

    timeVarMJMRec->getFieldName (fieldName_(islot));
    timeVarMJMRec->getSourceName (sourceName_(islot));

    timeVarMJMRec->getGain (gains);
    for (ispw = 0; ispw < numberSpw_; ispw++) {
      switch (jonesType) {
      case 1: {
	cmplx = gains (IPosition(3,0,0,ispw));
	intGain_(IPosition(4,iant,jant,ispw,islot)) = mjJones4 (cmplx);
      };
      case 2: {
	for (i = 0; i < 4; i++) {
	  lvec(i) = gains (IPosition(3,i,i,ispw));
	};
	intGain_(IPosition(4,iant,jant,ispw,islot)) = lvec;
      };
      case 3: {
	for (i = 0; i < 4; i++) {
	  for (j = 0; j < 4; j++) {
	    lmat(i,j) = gains (IPosition(3,i,j,ispw));
	  };
	};
	intGain_(IPosition(4,iant,jant,ispw,islot)) = lmat;
      };
      }; // switch (jonesType)...
    }; // for (ispw...)

    delete timeVarMJMRec;

  }; // for (islot...)

};



void TimeVarMJones::initialize(VisSet& vs)
{

  LogMessage message(LogOrigin("TimeVarMJones", "initialize"));

  {
    ostringstream o; o<<"initializing";message.message(o);
    logSink().post(message);
  }

  // Find maximum number of antennas
  numberAnt_=vs.numberAnt();
  numberSpw_=vs.numberSpw();
  AlwaysAssert(numberAnt_>0, AipsError);
  AlwaysAssert(interval_>0, AipsError);
  

  // Initialize for iteration in time intervals
  Block<Int> columns(0);
  // we need to keep track of which solutions go where better
  //columns[0]=MS::FIELD_ID;
  //columns[1]=MS::SPECTRAL_WINDOW;
  //  columns[0]=MS::TIME;
  VisSet intvs(vs,columns,interval_);

  // find out how many intervals we have
  VisIter& ioc=intvs.iter();
  ioc.originChunks();
  for (currentSlot_=0,ioc.originChunks(); ioc.moreChunks();
       currentSlot_++,ioc.nextChunk()) {
  }
  numberOfSlots_=currentSlot_;
  {
    ostringstream o; o<<"For interval of "<<interval_<<" seconds, found "<<
		    numberOfSlots_<<" slots";message.message(o);
		    logSink().post(message);
  }

  // Fill in start and end time of each slot
  MJDStart_.resize(numberOfSlots_);
  MJDStop_.resize(numberOfSlots_);
  fieldName_.resize(numberOfSlots_);
  sourceName_.resize(numberOfSlots_);
  VisBuffer vb(ioc);
  for (currentSlot_=0,ioc.originChunks(); ioc.moreChunks();
       currentSlot_++,ioc.nextChunk()) {
    MJDStart_(currentSlot_)=vb.time()(0);
    MJDStop_(currentSlot_) = vb.time()(vb.nRow()-1);
    fieldName_(currentSlot_) = ioc.fieldName();
    sourceName_(currentSlot_) = ioc.sourceName();
  }

  initializeCache();
}
void TimeVarMJones::initializeCache()
{
  // Initialize the antenna gains and the corresponding Jones matrix
  intGainCache_.resize(numberAnt_, numberAnt_, numberSpw_);
  intGainCache_= mjJones4(Complex(1.0,0.0));
  intGainInvCache_.resize(numberAnt_, numberAnt_, numberSpw_);
  intGainInvCache_= mjJones4(Complex(1.0,0.0));
  cacheValid_.resize(numberSpw_);
  cacheValid_=False;
  currentSlot_=0;
}

// Is this time in the slot? 
inline Bool TimeVarMJones::inSlot(Double start, Double end, Double time) {
  return ((time>=(start-deltat_))&&(time<(end+deltat_)));
}

  // Is the time in the specified slot. slot must be in range
inline Bool TimeVarMJones::inSlot(Int slot, Double time) {
  return inSlot(MJDStart_(slot), MJDStop_(slot), time);
}

Int TimeVarMJones::findSlot(Double time) {
  // First try just the next slot
  if(currentSlot_+1<numberOfSlots_) {
    if(inSlot(currentSlot_+1,time)) return currentSlot_+1;
  }
  // Next search the whole list
  for (Int slot=0;slot<numberOfSlots_;slot++)
    if(inSlot(slot, time)) return slot;
  // This should never happen
  throw(AipsError("TimeVarMJones::findSlot: Cannot find slot number"));
}

void TimeVarMJones::checkCache(const VisBuffer& vb, Int row)
{
  Int spw=vb.spectralWindow();
  if(!cacheValid(spw)||!inSlot(currentSlot_,vb.time()(row))) {

    currentSlot_=findSlot(vb.time()(row));
    
    // Calculate interferometer Gain by some means
    getIntGain(intGainCache_,vb,row);

    // Now refresh the actual cache
    for (Int i=0;i<numberAnt_;i++) {
      for (Int j=0;j<numberAnt_;j++) {
	intGainCache_(i,j,spw).inverse(intGainInvCache_(i,j,spw));
      }
    }
    validateCache(spw);
  }
}

} //# NAMESPACE CASA - END

