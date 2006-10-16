//# SolvableMJones.cc: Implementation of Jones classes
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

#include <synthesis/MeasurementComponents/SolvableMJones.h>
#include <calibration/CalTables/CalTable.h>
#include <calibration/CalTables/SolvableMJDesc.h>
#include <calibration/CalTables/SolvableMJMRec.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <msvis/MSVis/VisBuffer.h>

#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Quanta/MVTime.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// ------------------------------------------------------------------
// Start of methods for the solveable version i.e. SolvableMJones

SolvableMJones::SolvableMJones(VisSet& vs, Double interval, Double deltat)
:TimeVarMJones(vs,interval,deltat)
{
  // Initialize for iteration in time intervals. 
  Block<Int> columns(0);
  //  columns[0]=MS::TIME;
  vs_=new VisSet(vs,columns,interval);
  initialize();
}

SolvableMJones::SolvableMJones(const SolvableMJones& other)
{
  operator=(other);
}

SolvableMJones::~SolvableMJones() {
  if(vs_) delete vs_; vs_=0;
}

// Assignment
SolvableMJones& SolvableMJones::operator=(const SolvableMJones& other) {
  if(this!=&other) {
    TimeVarMJones::operator=(other);
    vs_=other.vs_;
    sumwt_=other.sumwt_;
    chisq_=other.chisq_;
    gS_.resize(other.gS_.shape());
    gS_=other.gS_;
    ggS_.resize(other.ggS_.shape());
    ggS_=other.ggS_;
  }
  return *this;
}

void SolvableMJones::initializeGradients() {
  sumwt_=0.0;
  chisq_=0.0;
  Matrix<Complex> gS(4,4); gS=Complex(0.,0.);
  gS_=gS;
  Matrix<Float> ggS(4,4); ggS=0.;
  ggS_=ggS;
}

void SolvableMJones::finalizeGradients() {}

// Add to Gradient Chisq
void SolvableMJones::addGradients(const VisBuffer& vb, Int row,
				  const Antenna& a1, const Antenna& a2,
				  const Vector<Float>& sumwt, 
				  const Vector<Float>& chisq,
				  const Cube<Complex>& c, 
				  const Cube<Float>& f) {
  Int spw=vb.spectralWindow();
  for (Int chn=0; chn<vb.nChannel(); chn++) {
    if (!vb.flag()(chn,row)) {
      sumwt_+=sumwt(chn);
      chisq_+=chisq(chn);
      gS_(a1,a2,spw)+=c.xyPlane(chn);
      ggS_(a1,a2,spw)+=f.xyPlane(chn);
    }
  }
}

// Solve for the  Jones matrix. Updates the MJones thus found.
// Also inserts it into the VisEquation thus it is not const.
Bool SolvableMJones::solve (VisEquation& me)
{

  LogMessage message(LogOrigin("SolvableMJones", "solve"));

  // Make a local copy of the Measurement Equation so we can change
  // some of the entries
  VisEquation lme(me);

  // Initialize for iteration in time intervals. I don't understand
  // what keeps overwriting the iterators and makes this necessary
//  Block<Int> columns(1);
//  columns[0]=MS::TIME;
//  VisSet intvs(vs_,columns,interval_);

  // Aggregate Chisqs
  Float aInitialChisq=0.0;
  
  AlwaysAssert(gain()>0.0,AipsError);
  AlwaysAssert(numberIterations()>0,AipsError);
  AlwaysAssert(tolerance()>0.0,AipsError);
  
  // Set the Jones matrix in the local copy of the Measurement Equation 
  lme.setMJones(*this);
  
  // Count number of failed solutions
  Int failed=0;
  
  VisIter& vi(vs_->iter());
  
  // Iterate chunks
  Int slot;
  for(slot=0, vi.originChunks(); slot<numberOfSlots_&&vi.moreChunks();
      vi.nextChunk(),slot++)    {
    {
      ostringstream o; o<<typeName()<<" Slot "<<slot<<": "
		     << MVTime(MJDStart_(slot)/86400.0)<<" to "
		     << MVTime(MJDStop_(slot)/86400.0);message.message(o);
      logSink().post(message);
    }
    
    // Find gradient and Hessian
    lme.gradientsChiSquared(*this);
    
    // Stop if the current fit is good enough
    Bool fail;
    if((chisq_<=0)||(sumwt_<=0.0)) {
      {
	ostringstream o; o<<typeName()<<"No valid data found for this slot";
	message.message(o);
	logSink().post(message);
      }
    }
    else {
      
      Float currentChisq=0.0;
      currentChisq=chisq_;
      AlwaysAssert(currentChisq>=0.0, AipsError);
      AlwaysAssert(sumwt_>0.0, AipsError);
      Float rms;
      rms = sqrt(currentChisq/sumwt_);
      {
	ostringstream o; o<<typeName()<<"    Initial fit per unit weight = "<<rms<<
			" Jy, sum of weights = "<<sumwt_;message.message(o);
			logSink().post(message);
      }
      aInitialChisq+=currentChisq;
      Float originalChisq=currentChisq;
      Float previousChisq=currentChisq;
      
      // Iterate    
      fail=False;
      solutionOK_(slot)=False;
      fit_(slot)=0.0;
      fitwt_(slot)=0.0;
      for (Int iter=0;(!fail)&&(iter<numberIterations());iter++) {
	
	// Update int gains from gradients. This can be different for
	// derived classes
	updateIntGain(slot);
	
	// Force refresh of cache
	invalidateCache();
	
	// Find gradient and Hessian
	lme.gradientsChiSquared(*this);
	
	// Assess fit
	previousChisq=currentChisq;
	currentChisq=chisq_;
	AlwaysAssert(sumwt_>0.0, AipsError);
	AlwaysAssert(chisq_>=0.0, AipsError);
	rms = sqrt(chisq_/sumwt_);
	
	// Converged?
	if(abs(currentChisq-previousChisq)<tolerance()*originalChisq) {
          {
	    ostringstream o; o<<typeName()<<"    Final   fit per unit weight = "
			   <<rms<<" Jy, after " <<iter<<" iterations";
	    message.message(o);
	    logSink().post(message);
	  }
	  solutionOK_(slot)=True;
	  fit_(slot)=rms;
	  fitwt_(slot)=sumwt_;
	  break;
	};
	if(iter==0) {
	  previousChisq=currentChisq;
	  originalChisq=currentChisq;
	};
	
	// Diverging? We need a better way of identifying divergence.
	if((iter>(numberIterations()/2))&&
	   (currentChisq>originalChisq)&&
	   (currentChisq>previousChisq)) {
	  {
	    ostringstream o; o<<typeName()<<" Diverging: discarding these solutions";message.message(o);
	    logSink().post(message);
	  }
	  invalidateCache();
	  fail=True;
	}
      }
      fail=(fail||
		  abs(currentChisq-previousChisq)>tolerance()*originalChisq);
      if(fail) failed++;
    }
  }      
  
  if(failed>0) {
    {
      ostringstream o; o<<typeName()<<" Did not converge for "<<
		      failed<<" time intervals";message.message(o);
		      logSink().post(message);
    }
    return(False);
  }
  else {
    return(True);
  }
  
}

void SolvableMJones::initialize()
{
  LogMessage message(LogOrigin("SolvableMJones", "initialize"));
  {
    ostringstream o; o<<"SolvableMJones initializing";message.message(o);
    logSink().post(message);
  }
  
  gS_.resize(numberAnt_,numberAnt_,numberSpw_);  
  ggS_.resize(numberAnt_,numberAnt_,numberSpw_);  
  // Resize intGain_ to hold a mjJones4 for every baseline
  // for every slot
  intGain_.resize(IPosition(4,numberAnt_,numberAnt_,numberSpw_,numberOfSlots_));
  fit_.resize(numberOfSlots_);
  fitwt_.resize(numberOfSlots_);
  solutionOK_.resize(numberOfSlots_);
}


// Update formula for general matrices
void SolvableMJones::updateIntGain(Int slot)
{
  DebugAssert(slot>=0&&slot<numberOfSlots_, AipsError);
  Matrix<Complex> lmat(4,4);
  for (Int spw=0;spw<numberSpw_;spw++) {
    for (Int jant=0;jant<numberAnt_;jant++) {
      for (Int iant=0;iant<numberAnt_;iant++) {
	lmat=intGain_(IPosition(4,iant,jant,spw,slot)).matrix();
	for(uInt i=0;i<4;i++) {
	  for(uInt j=0;j<4;j++) {
	    if(ggS_(iant,jant,spw)(i,j)>0.0) {
	      lmat(i,j)-=gain()*gS_(iant,jant,spw)(i,j)/
		ggS_(iant,jant,spw)(i,j);
	    }
	  }
	}
	intGain_(IPosition(4,iant,jant,spw,slot))=lmat;
      }
    }
  }
}

// Simply return the relevant value from the vector of values
// Here we just retrieve the diagonal values and make the SquareMatrix
void SolvableMJones::getIntGain(Cube<mjJones4>& ig, const VisBuffer& vb,
				Int row) {
  Int slot;
  slot=findSlot(vb.time()(row));
  DebugAssert(slot>=0&&slot<numberOfSlots_, AipsError);
  IPosition start(4,0,0,0,slot),end(intGain_.shape());
  end(4)=slot;
  ig=intGain_(start,end).nonDegenerate(3);
}

// Write the contents to a table
void SolvableMJones::store (const String& file, const Bool& append)
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
    SolvableMJonesDesc smjDesc;
    tab = new CalTable (file, smjDesc);
  };

  // Declarations
  Int islot, iant, jant, ispw, i, j;
  
  // Sub-table records
  CalDescRecord* descRec;
  SolvableMJonesMRec* mainRec;

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
  Vector <Bool> solnOk(numberSpw_, True);
  Vector <Float> fit(numberSpw_, 1.0);
  Vector <Float> fitWgt(numberSpw_, 1.0);

  // Loop over the number of time slots
  for (islot = 0; islot < numberOfSlots_; islot++) {

    // Loop over all interferometer baselines
    for (iant = 0; iant < numberAnt_; iant++) {
      for (jant = iant; jant < numberAnt_; jant++) {

	// Fill the cal_main record
	mainRec = new SolvableMJonesMRec;

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
	mainRec->defineFieldId (fieldid_(islot));
	
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
	
	// Fill fit statistics and weights
	mainRec->defineTotalSolnOk (solutionOK_(islot));
	mainRec->defineTotalFit (fit_(islot));
	mainRec->defineTotalFitWgt (fitwt_(islot));

	mainRec->defineSolnOk (solnOk);
	mainRec->defineFit (fit);
	mainRec->defineFitWgt (fitWgt);
	
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
void SolvableMJones::load (const String& file, const String& select, 
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

  // Get solution interval (assumed constant across table)
  SolvableMJonesMRec* solvableMJMRec = 
    new SolvableMJonesMRec (tab.getRowMain(0));
  solvableMJMRec->getInterval (interval_);
  deltat_ = 0.01 * interval_;
  delete solvableMJMRec;

  // Re-size the cache variables
  MJDStart_.resize (numberOfSlots_);
  MJDStop_.resize (numberOfSlots_);
  fieldName_.resize (numberOfSlots_);
  sourceName_.resize (numberOfSlots_);
  fieldid_.resize (numberOfSlots_);
  solutionOK_.resize (numberOfSlots_);
  fit_.resize (numberOfSlots_);
  fitwt_.resize (numberOfSlots_);
  intGain_.resize 
    (IPosition(4,numberAnt_, numberAnt_, numberSpw_, numberOfSlots_));
  
  // Cache initialization
  MJDStart_ = 0;
  MJDStop_ = 0;
  fieldName_ = "";
  sourceName_ = "";
  fieldid_ = 0;
  solutionOK_ = False;
  fit_ = 0;
  fitwt_ = 0;

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
  Double time, lastTime, interval;
  Int islot, iant, jant, ispw, i, j, irow, nrow;
  Array <Complex> gains (IPosition(3,4,4,numberSpw_));

  // Read the calibration information
  nrow = tab.nRowMain();
  lastTime = 0;
  islot = 0;

  for (irow = 0; irow < nrow; irow++) {

    // Read a record from cal_main
    solvableMJMRec = new SolvableMJonesMRec (tab.getRowMain (irow));

    // Decode the relevant fields and copy to cache
    solvableMJMRec->getAntenna1 (iant);
    solvableMJMRec->getAntenna2 (jant);
    solvableMJMRec->getTime (time);
    solvableMJMRec->getInterval (interval);

    // Compute the time slot index in the cache
    if (irow == 0) {
      islot = 0;
    } else {
      if (abs (time - lastTime) > deltat_) {
	islot = islot + 1;
      };
    };
    lastTime = time;

    MJDStart_(islot) = time - interval / 2.0;
    MJDStop_(islot) = time + interval / 2.0;

    solvableMJMRec->getFieldName (fieldName_(islot));
    solvableMJMRec->getSourceName (sourceName_(islot));
    solvableMJMRec->getFieldId (fieldid_(islot));

    solvableMJMRec->getTotalSolnOk (solutionOK_(islot));
    solvableMJMRec->getTotalFit (fit_(islot));
    solvableMJMRec->getTotalFitWgt (fitwt_(islot));

    solvableMJMRec->getGain (gains);
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

    delete solvableMJMRec;

  }; // for (islot...)

};


MIfr::MIfr(VisSet& vs, Double interval, Double deltat) :
  SolvableMJones(vs, interval, deltat) { init(); }

MIfr::MIfr(VisSet& vs, String file, const String& select) {
  load(file, select, "general");
  initializeCache();
  Block<Int> columns(0);
  //columns[0]=MS::TIME;
  vs_=new VisSet(vs,columns,interval_);
}

MIfr::~MIfr() {
  if(vs_) delete vs_; vs_=0;
}

// init mask and cache for diagonal matrices
void MIfr::init()
{
    // create a diagonal unity mjJones4
    Vector<Complex> gains(4); gains=Complex(1.0,0.0);
    mjJones4 jones(gains);
    // assign to all cache elements
    intGain_=jones;
}

// Update formula for diagonal matrices
void MIfr::updateIntGain(Int slot)
{
  DebugAssert(slot>=0&&slot<numberOfSlots_, AipsError);
  Vector<Complex> lvec;
  for (Int jant=0;jant<numberAnt_;jant++) {
    for (Int iant=0;iant<numberAnt_;iant++) {
      for (Int ispw=0; ispw<numberSpw_; ispw++) {
	lvec=intGain_(IPosition(4,iant,jant,ispw,slot)).matrix().diagonal();
	for(uInt i=0;i<4;i++) {
	  if(ggS_(iant,jant,ispw)(i,i)>0.0) {
	    lvec(i)-=gain()*gS_(iant,jant,ispw)(i,i)/ggS_(iant,jant,ispw)(i,i);
	  }
	}
	intGain_(IPosition(4,iant,jant,ispw,slot))=lvec;
      }
    }
  }
}

} //# NAMESPACE CASA - END

