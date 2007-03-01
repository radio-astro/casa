//# Calibrater.cc: Implementation of Calibrater.h
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: Calibrater.cc,v 19.37 2006/03/16 01:28:09 gmoellen Exp $

#include <tables/Tables/Table.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableParse.h>

#include <casa/Arrays/ArrayUtil.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSFieldIndex.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <synthesis/MeasurementComponents/Calibrater.h>
#include <synthesis/MeasurementComponents/VisCalSolver.h>
#include <synthesis/MeasurementComponents/UVMod.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
#include <casa/Quanta/MVTime.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

#include <tables/Tables/SetupNewTab.h>

namespace casa { //# NAMESPACE CASA - BEGIN

Calibrater::Calibrater(): 
  ms_p(0), 
  mssel_p(0), 
  vs_p(0), 
  ve_p(0),
  vc_p(),
  svc_p(0),
  histLockCounter_p(), 
  hist_p(0)
{
}

Calibrater::Calibrater(const Calibrater & other)
{
  operator=(other);
}

Calibrater &Calibrater::operator=(const Calibrater & other)
{
  ms_p=other.ms_p;
  mssel_p=other.mssel_p;
  vs_p=other.vs_p;
  ve_p=other.ve_p;
  histLockCounter_p=other.histLockCounter_p;
  hist_p=other.hist_p;
  historytab_p=other.historytab_p;
  
  return *this;
}

Calibrater::~Calibrater()
{
  cleanup();
  if (ms_p)   delete ms_p;   ms_p=0;
  if (hist_p) delete hist_p; hist_p=0;
}

LogIO& Calibrater::logSink() {return sink_p;};

String Calibrater::timerString() {
  ostringstream o;
  o <<" [user:   " << timer_p.user () << 
    "  system: " << timer_p.system () <<
    "  real:   " << timer_p.real () << "]"; 
  timer_p.mark();
  return o;
};

Bool Calibrater::initialize(MeasurementSet& inputMS, Bool compress)  {
  
  logSink() << LogOrigin("Calibrater","") << LogIO::NORMAL;
  
  try {
    timer_p.mark();

    // Set pointer ms_p from input MeasurementSet
    if (ms_p) {
      *ms_p=inputMS;
    } else {
      ms_p = new MeasurementSet(inputMS);
      AlwaysAssert(ms_p,AipsError);
    };

    // Setup to write LogIO to HISTORY Table in MS
    if(!(Table::isReadable(ms_p->historyTableName()))){
      // create a new HISTORY table if its not there
      TableRecord &kws = ms_p->rwKeywordSet();
      SetupNewTable historySetup(ms_p->historyTableName(),
				 MSHistory::requiredTableDesc(),Table::New);
      kws.defineTable(MS::keywordName(MS::HISTORY), Table(historySetup));
      MSHistoryHandler::addMessage(*ms_p, "HISTORY Table added by Calibrater",
				   "Calibrater","","Calibrater::initialize()");
    }
    historytab_p=Table(ms_p->historyTableName(),
		       TableLock(TableLock::UserNoReadLocking), Table::Update);
    hist_p= new MSHistoryHandler(*ms_p, "calibrater");


    // Recognize if we'll need to initialize the imaging weights
    //  TBD: should Calibrater care?  (Imager doesn't know how to verify)
    Bool needWeights=(!ms_p->tableDesc().isColumn("CORRECTED_DATA"));

    msname_p=ms_p->tableName();

    // Set the selected MeasurementSet to be the same initially
    // as the input MeasurementSet
    if (mssel_p) delete mssel_p;
    mssel_p=new MeasurementSet(*ms_p);
    
    logSink() << LogIO::NORMAL
	      << "Initializing nominal selection to the whole MS."
	      << LogIO::POST;

    // Create a VisSet with no selection 
    if (vs_p) {
      delete vs_p;
      vs_p=0;
    };
    Block<Int> nosort(0);
    Matrix<Int> noselection;
    Double timeInterval=0;
    vs_p=new VisSet(*ms_p,nosort,noselection,timeInterval,compress);


    // Initialize the weights if the scratch columns
    // were just created
    if(needWeights) {
      Double sumwt=0.0;
      VisSetUtil::WeightNatural(*vs_p, sumwt);
    }

    // Create the associated VisEquation
    //  TBD: move to ctor and make it non-pointer
    if (ve_p) {
      delete ve_p;
      ve_p=0;
    };
    ve_p=new VisEquation();

    // Reset the apply/solve VisCals
    reset(True,True);

    return True;

  } catch (AipsError x) {
    logSink() << LogOrigin("Calibrater","initialize",WHERE) 
	      << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    cleanup();
    if (ms_p) delete ms_p; ms_p=NULL;
    if (hist_p) delete hist_p; hist_p=NULL;

    throw(AipsError("Error in Calibrater::initialize()"));
    return False;
  } 
  return False;
}


Bool Calibrater::initCalSet(const Int& calSet) 
{

  //  logSink() << LogOrigin("Calibrater","initCalSet") << LogIO::NORMAL;

  if (vs_p) {
    vs_p->initCalSet(calSet);
    return True;
  }
  else {
    throw(AipsError("Calibrater cannot initCalSet"));
    return False;
  }
}

void Calibrater::setdata(const String& mode, 
			 const Int& nchan, const Int& start, const Int& step,
			 const MRadialVelocity& mStart,
			 const MRadialVelocity& mStep,
			 const String& msSelect)
{
// Define primary measurement set selection criteria
// Inputs:
//    mode         const String&            Frequency/velocity selection mode
//                                          ("channel", "velocity" or 
//                                           "opticalvelocity")
//    nchan        const Int&               No of channels to select
//    start        const Int&               Start channel to select
//    step         const Int&               Channel increment
//    mStart       const MRadialVelocity&   Start radial vel. to select
//    mStep        const MRadialVelocity&   Radial velocity increment
//    msSelect     const String&            MS selection string (TAQL)
// Output to private data:
//
  logSink() << LogOrigin("Calibrater","setdata") << LogIO::NORMAL;

  try {
    
    logSink() << "Selecting data" << LogIO::POST;
    
    // Set data selection variables
    dataMode_p=mode;
    dataNchan_p=nchan;
    if (dataNchan_p<0) dataNchan_p=0; 
    dataStart_p=start;
    if (dataStart_p<0) dataNchan_p=0; 
    dataStep_p=step;
    if (dataStep_p<1) dataNchan_p=1; 

    mDataStart_p=mStart;
    mDataStep_p=mStep;

    // Apply selection to the original MeasurementSet
    logSink() << "Performing selection on MeasurementSet" << LogIO::POST;

    // Delete VisSet and selected MS
    if (vs_p) {
      delete vs_p;
      vs_p=0;
    };
    if (mssel_p) {
      delete mssel_p;
      mssel_p=0;
    };

    // Force a re-sort of the MS
    if (ms_p->keywordSet().isDefined("SORTED_TABLE")) {
      ms_p->rwKeywordSet().removeField("SORTED_TABLE");
    };
    if (ms_p->keywordSet().isDefined("SORT_COLUMNS")) {
      ms_p->rwKeywordSet().removeField("SORT_COLUMNS");
    };

    // Re-make the sorted table as necessary
    if (!ms_p->keywordSet().isDefined("SORTED_TABLE")) {
      Block<int> sort(0);
      Matrix<Int> noselection;
      VisSet vs(*ms_p,sort,noselection);
    }
    Table sorted=ms_p->keywordSet().asTable("SORTED_TABLE");
      
    Int len = msSelect.length();
    Int nspace = msSelect.freq (' ');
    Bool nullSelect=(msSelect.empty() || nspace==len);
    if (!nullSelect) {
      // Apply the TAQL selection string, to remake the selected MS
      String parseString="select from $1 where " + msSelect;
      mssel_p=new MeasurementSet(tableCommand(parseString,sorted));
      AlwaysAssert(mssel_p, AipsError);
      // Rename the selected MS as */SELECTED_TABLE
      mssel_p->rename(msname_p+"/SELECTED_TABLE", Table::Scratch);
      nullSelect=(mssel_p->nrow()==0);

      // Null selection wasn't intended!
      if (nullSelect)
	throw(AipsError("Specified msselect failed to select any data."));

    };

    if (nullSelect) {
      // Selection of whole MS intended
      if (mssel_p) {
	delete mssel_p; 
	mssel_p=0;
      };
      logSink() << LogIO::NORMAL
		<< "Selection is empty: reverting to sorted MeasurementSet"
		<< LogIO::POST;
      mssel_p=new MeasurementSet(sorted);
    } else {
      mssel_p->flush();
    }

    if(mssel_p->nrow()!=ms_p->nrow()) {
      logSink() << "By selection " << ms_p->nrow() << 
	" rows are reduced to " << mssel_p->nrow() << LogIO::POST;
    }
    else {
      logSink() << "Selection did not drop any rows" << LogIO::POST;
    }
    
    // Now, re-create the associated VisSet
    if(vs_p) delete vs_p; vs_p=0;
    Block<int> sort(0);
    Matrix<Int> noselection;
    vs_p = new VisSet(*mssel_p,sort,noselection);
    AlwaysAssert(vs_p, AipsError);
    

    // Select on frequency channel
    if(dataMode_p=="channel") {
      // *** this bit here is temporary till we unifomize data selection
      //Getting the selected SPWs
      ROMSMainColumns msc(*mssel_p);
      Vector<Int> dataDescID = msc.dataDescId().getColumn();
      Bool dum;
      Sort sort( dataDescID.getStorage(dum),sizeof(Int) );
      sort.sortKey((uInt)0,TpInt);
      Vector<uInt> index,uniq;
      sort.sort(index,dataDescID.nelements());
      uInt nSpw = sort.unique(uniq,index);

      Vector<Int> selectedSpw(nSpw);
      Vector<Int> nChan(nSpw);
      for (uInt k=0; k < nSpw; ++k){
	selectedSpw[k]=dataDescID[index[uniq[k]]];
	nChan[k]=vs_p->numberChan()(selectedSpw[k]);

      }
      //********
      if(dataNchan_p==0) dataNchan_p=vs_p->numberChan()(selectedSpw[0]);
      if(dataStart_p<0) {
	logSink() << LogIO::SEVERE << "Illegal start pixel = " 
		  << dataStart_p + 1 << LogIO::POST;
      }
      Int end = Int(dataStart_p) + Int(dataNchan_p) * Int(dataStep_p);
      for (uInt k=0; k < selectedSpw.nelements() ; ++k){
	if(end < 1 || end > nChan[k]) {
	  logSink() << LogIO::SEVERE << "Illegal step pixel = " << dataStep_p
		    << " in Spw " << selectedSpw[k]+1 
		    << LogIO::POST;
	}
	   logSink() << "Selecting "<< dataNchan_p
		     << " channels, starting at visibility channel "
		     << dataStart_p + 1 << " stepped by "
		     << dataStep_p << " in Spw " << selectedSpw[k]+1 << LogIO::POST;

	// Set frequency channel selection for all spectral window id's
	Int nch;
	//Vector<Int> nChan=vs_p->numberChan();
	//Int nSpw=vs_p->numberSpw();
	if (dataNchan_p==0) {
	  nch=nChan(k);
	}else {
	  nch=dataNchan_p;
	};
	vs_p->selectChannel(1,dataStart_p,nch,dataStep_p,selectedSpw[k]);
	   	   
      }
    }
    // Select on velocity
    else if (dataMode_p=="velocity") {
      MVRadialVelocity mvStart(mDataStart_p.get("m/s"));
      MVRadialVelocity mvStep(mDataStep_p.get("m/s"));
      MRadialVelocity::Types
	vType((MRadialVelocity::Types)mDataStart_p.getRefPtr()->getType());
      logSink() << "Selecting "<< dataNchan_p
	 << " channels, starting at radio velocity " << mvStart
	 << " stepped by " << mvStep << ", reference frame is "
	 << MRadialVelocity::showType(vType) << LogIO::POST;
      vs_p->iter().selectVelocity(Int(dataNchan_p), mvStart, mvStep,
				  vType, MDoppler::RADIO);
    }

    // Select on optical velocity
    else if (dataMode_p=="opticalvelocity") {
      MVRadialVelocity mvStart(mDataStart_p.get("m/s"));
      MVRadialVelocity mvStep(mDataStep_p.get("m/s"));
      MRadialVelocity::Types
	vType((MRadialVelocity::Types)mDataStart_p.getRefPtr()->getType());
      logSink() << "Selecting "<< dataNchan_p
	 << " channels, starting at optical velocity " << mvStart
	 << " stepped by " << mvStep << ", reference frame is "
	 << MRadialVelocity::showType(vType) << LogIO::POST;
      vs_p->iter().selectVelocity(Int(dataNchan_p), mvStart, mvStep,
				  vType, MDoppler::OPTICAL);
    }

  } catch (AipsError x) {
    // Re-initialize with the existing MS
    logSink() << LogOrigin("Calibrater","setdata",WHERE) 
	      << LogIO::SEVERE << "Caught exception: " << x.getMesg()
	      << LogIO::POST;
    initialize(*ms_p,False);
    throw(AipsError("Error in Calibrater::setdata()"));
  } 
};



Bool Calibrater::setapply(const String& type, 
			  const Double& t,
			  const String& table,
			  const String& interp,
			  const String& select,
			  const Bool& calwt,
			  const Vector<Int>& spwmap,
			  const Float& opacity) 
{
  //                           const Vector<Int>& rawspw)

  logSink() << LogOrigin("Calibrater", "setapply") << LogIO::NORMAL;
 
  // Set record format for calibration table application information
  RecordDesc applyparDesc;
  applyparDesc.addField ("t", TpDouble);
  applyparDesc.addField ("table", TpString);
  applyparDesc.addField ("interp", TpString);
  applyparDesc.addField ("select", TpString);
  applyparDesc.addField ("calwt",TpBool);
  applyparDesc.addField ("spwmap",TpArrayInt);
  applyparDesc.addField ("opacity",TpFloat);
  
  // Create record with the requisite field values
  Record applypar(applyparDesc);
  applypar.define ("t", t);
  applypar.define ("table", table);
  applypar.define ("interp", interp);
  applypar.define ("select", select);
  applypar.define ("calwt",calwt);
  applypar.define ("spwmap",spwmap);
  applypar.define ("opacity", opacity);
  
  String upType=type;
  upType.upcase();
  if (upType=="")
    // Get type from table
    upType = calTableType(table);

  return setapply(upType,applypar);

}

Bool Calibrater::setapply (const String& type, 
			   const Record& applypar) {

  // First try to create the requested VisCal object
  VisCal *vc(NULL);
  try {

    if(!ok()) 
      throw(AipsError("Calibrater not prepared for setapply."));

    String upType=type;
    upType.upcase();

    logSink() << LogIO::NORMAL 
	      << "Arranging to APPLY:"
	      << LogIO::POST;

    // Add a new VisCal to the apply list
    vc = createVisCal(upType,*vs_p);  

    vc->setApply(applypar);       

      logSink() << LogIO::NORMAL << ".   "
		<< vc->applyinfo()
		<< LogIO::POST;

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << x.getMesg() 
	      << " Check inputs and try again."
	      << LogIO::POST;
    if (vc) delete vc;
    throw(AipsError("Error in Calibrater::setapply."));
    return False;
  }

  // Creation apparently successful, so add to the apply list
  // TBD: consolidate with above?
  try {
  
    uInt napp=vc_p.nelements();
    vc_p.resize(napp+1,False,True);      
    vc_p[napp] = vc;
    vc=NULL;
   
    // Maintain sort of apply list
    ve_p->setapply(vc_p);
    
    return True;

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    if (vc) delete vc;
    throw(AipsError("Error in Calibrater::setapply."));
    return False;
  } 
  return False;
}



Bool Calibrater::setsolve (const String& type, 
			   const Double& t,
                           const Double& preavg, 
			   const Bool& phaseonly,
                           const Int& refant, 
			   const String& table,
                           const Bool& append)
{
  
  logSink() << LogOrigin("Calibrater","setsolve") << LogIO::NORMAL;
  
  // Create a record description containing the solver parameters
  RecordDesc solveparDesc;
  solveparDesc.addField ("t", TpDouble);
  solveparDesc.addField ("preavg", TpDouble);
  solveparDesc.addField ("phaseonly", TpBool);
  solveparDesc.addField ("refant", TpInt);
  solveparDesc.addField ("table", TpString);
  solveparDesc.addField ("append", TpBool);
  
  // Create a solver record with the requisite field values
  Record solvepar(solveparDesc);
  solvepar.define ("t", t);
  solvepar.define ("preavg", preavg);
  solvepar.define ("phaseonly", phaseonly);
  solvepar.define ("refant", refant);
  solvepar.define ("table", table);
  solvepar.define ("append", append);
  
  return setsolve(type,solvepar);

}

Bool Calibrater::setsolvebandpoly(const String& table,
				  const Bool& append,
				  const Vector<Int>& degree,
				  const Bool& visnorm,
				  const Bool& bpnorm,
				  const Int& maskcenter,
				  const Float& maskedge,
				  const Int& refant) {

  logSink() << LogOrigin("Calibrater","setsolvebandpoly") << LogIO::NORMAL;

  // TBD: support solution interval!

    // Create a record description containing the solver parameters
    RecordDesc solveparDesc;
    solveparDesc.addField ("table", TpString);
    solveparDesc.addField ("append", TpBool);
    solveparDesc.addField ("t", TpDouble);
    solveparDesc.addField ("degree", TpArrayInt);
    solveparDesc.addField ("visnorm", TpBool);
    solveparDesc.addField ("bpnorm", TpBool);
    solveparDesc.addField ("maskcenter", TpInt);
    solveparDesc.addField ("maskedge", TpFloat);
    solveparDesc.addField ("refant", TpInt);

    //    solveparDesc.addField ("preavg", TpDouble);
    //    solveparDesc.addField ("phaseonly", TpBool);
    
    // Create a solver record with the requisite field values
    Record solvepar(solveparDesc);
    solvepar.define ("table", table);
    solvepar.define ("append", append);
    solvepar.define ("t",DBL_MAX);        // no time-dep, for the moment
    solvepar.define ("degree", degree);
    solvepar.define ("visnorm", visnorm);
    solvepar.define ("bpnorm", bpnorm);
    solvepar.define ("maskcenter", maskcenter);
    solvepar.define ("maskedge", maskedge);
    solvepar.define ("refant", refant);


    //    solvepar.define ("t", t);
    //    solvepar.define ("preavg", preavg);
    //    solvepar.define ("phaseonly", phaseonly);


    return setsolve("BPOLY",solvepar);

}

Bool Calibrater::setsolvegainspline(const String& table,
				    const Bool& append,
				    const String& mode,
				    const Double& splinetime,
				    const Double& preavg,
				    const Int& refant,
				    const Int& numpoint,
				    const Double& phasewrap) {
  
  logSink() << LogOrigin("Calibrater","setsolvegainspline") << LogIO::NORMAL;

  // Create a record description containing the solver parameters
  RecordDesc solveparDesc;
  solveparDesc.addField ("table", TpString);
  solveparDesc.addField ("append", TpBool);
  solveparDesc.addField ("mode", TpString);
  solveparDesc.addField ("splinetime", TpDouble);
  solveparDesc.addField ("preavg", TpDouble);
  solveparDesc.addField ("refant", TpInt);
  solveparDesc.addField ("numpoint", TpInt);
  solveparDesc.addField ("phasewrap", TpDouble);
  
  // Create a solver record with the requisite field values
  Record solvepar(solveparDesc);
  solvepar.define ("table", table);
  solvepar.define ("append", append);
  String upMode=mode;
  upMode.upcase();
  solvepar.define ("mode", upMode);
  solvepar.define ("splinetime",splinetime);
  solvepar.define ("preavg", preavg);
  solvepar.define ("refant", refant);
  solvepar.define ("numpoint",numpoint);
  solvepar.define ("phasewrap",phasewrap);
  
  return setsolve("GSPLINE",solvepar);
  
}

Bool Calibrater::setsolve (const String& type, 
			   const Record& solvepar) {

  // Attempt to create the solvable object
  SolvableVisCal *svc(NULL);
  try {

    if(!ok()) 
      throw(AipsError("Calibrater not prepared for setsolve."));

    String upType = type;
    upType.upcase();

    // Clean out any old solve that was lying around
    unsetsolve();

    logSink() << LogIO::NORMAL 
	      << "Arranging to SOLVE:"
	      << LogIO::POST;

    // Create the new SolvableVisCal
    svc = createSolvableVisCal(upType,*vs_p);
    svc->setSolve(solvepar);
    
    logSink() << LogIO::NORMAL << ".   "
	      << svc->solveinfo()
	      << LogIO::POST;

    // Creation apparently successful, keep it
    svc_p=svc;
    svc=NULL;

    return True;

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    unsetsolve();
    if (svc) delete svc;
    throw(AipsError("Error in Calibrater::setsolve."));
    return False;
  } 
  return False;
}

Bool Calibrater::state() {

  logSink() << LogOrigin("Calibrater","state") << LogIO::NORMAL;

  applystate();
  solvestate();

  return True;

}

Bool Calibrater::applystate() {


  //  logSink() << LogOrigin("Calibrater","applystate") << LogIO::NORMAL;

  logSink() << LogIO::NORMAL 
	    << "The following calibration terms are arranged for apply:"
	    << LogIO::POST;

  Int napp(vc_p.nelements());
  if (napp>0)
    for (Int iapp=0;iapp<napp;++iapp)
      logSink() << LogIO::NORMAL << ".   "
		<< vc_p[iapp]->applyinfo()
		<< LogIO::POST;
  else
    logSink() << LogIO::NORMAL << ".   "
	      << "(None)"
	      << LogIO::POST;

  return True;

}


Bool Calibrater::solvestate() {

  //  logSink() << LogOrigin("Calibrater","solvestate") << LogIO::NORMAL;

  logSink() << LogIO::NORMAL 
	    << "The following calibration term is arranged for solve:"
	    << LogIO::POST;

  if (svc_p)
    logSink() << LogIO::NORMAL << ".   "
	      << svc_p->solveinfo()
	      << LogIO::POST;
  else
    logSink()  << LogIO::NORMAL << ".   "
	      << "(None)"
	      << LogIO::POST;

  return True;
}


Bool Calibrater::cleanup() {

  //  logSink() << LogOrigin("Calibrater","cleanup") << LogIO::NORMAL;

  // Delete the VisCals
  reset();

  // Delete derived dataset stuff
  if(vs_p) delete vs_p; vs_p=0;
  if(mssel_p) delete mssel_p; mssel_p=0;

  // Delete the current VisEquation
  if(ve_p) delete ve_p; ve_p=0;

  return True;

}

Bool Calibrater::reset(const Bool& apply, const Bool& solve) {

  //  logSink() << LogOrigin("Calibrater","reset") << LogIO::NORMAL;

  // Delete the VisCal apply list
  if (apply) 
    unsetapply();

  // Delete the VisCal solve object
  if (solve)
    unsetsolve();
        
  return True;
}

// Delete all (default) or one VisCal in apply list
Bool Calibrater::unsetapply(const Int& which) {

  //  logSink() << LogOrigin("Calibrater","unsetapply") << LogIO::NORMAL;
  
  try {
    if (which<0) {
      for (uInt i=0;i<vc_p.nelements();i++)
	if (vc_p[i]) delete vc_p[i];
      vc_p.resize(0,True);
    } else {
      if (vc_p[which]) delete vc_p[which];
      vc_p.remove(which);
    }
    
    // Maintain size/sort of apply list
    if(ve_p) ve_p->setapply(vc_p);

    return True;
  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    throw(AipsError("Error in Calibrater::unsetapply."));

    return False;
  }
  return False;
}

  // Delete solve VisCal
Bool Calibrater::unsetsolve() {

  //  logSink() << LogOrigin("Calibrater","unsetsolve") << LogIO::NORMAL;

  try {
    if (svc_p) delete svc_p;
    svc_p=NULL;
    
    if(ve_p) ve_p->setsolve(*svc_p);

    return True;

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    throw(AipsError("Error in Calibrater::unsetsolve."));
    return False;
  }
  return False;
}


Bool Calibrater::correct() {
  
  logSink() << LogOrigin("Calibrater","correct") << LogIO::NORMAL;
  
  try {
    
    if (!ok())
      throw(AipsError("Calibrater not prepared for correct!"));

    // Ensure apply list non-zero and properly sorted
    ve_p->setapply(vc_p);

    // Report the types that will be applied
    applystate();

    // Arrange for iteration over data
    Block<Int> columns;
    // include scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
    vs_p->resetVisIter(columns,0.0);
    VisIter& vi(vs_p->iter());
    VisBuffer vb(vi);
    
    // Pass each timestamp (VisBuffer) to VisEquation for correction
    Bool calwt(calWt());
    for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {

	// If we are going to update the weights, reset them first
	// TBD: move this to VisEquation::correct?
	if (calwt) vb.resetWeightMat();

	ve_p->correct(vb);    // throws exception if nothing to apply
	vi.setVis(vb.visCube(),VisibilityIterator::Corrected);
	vi.setFlag(vb.flag());

	// Write out weight col, if it has changed
	if (calwt) vi.setWeightMat(vb.weightMat()); 
      }
    }

    // Flush to disk
    vs_p->flush();

    return True;  

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;

    logSink() << "Reseting all calibration application settings." << LogIO::POST;
    unsetapply();

    throw(AipsError("Error in Calibrater::correct."));
    return False;
  } 
  return False;
}

Bool Calibrater::solve() {

  logSink() << LogOrigin("Calibrater","solve") << LogIO::NORMAL;

  try {

    if (!ok()) 
      throw(AipsError("Calibrater not prepared for solve."));

    // Handle nothing-to-solve-for case
    if (!svc_p)
      throw(AipsError("Please run setsolve before attempting to solve."));

    // Arrange VisEquation for solve
    ve_p->setsolve(*svc_p);

    // Ensure apply list properly sorted w.r.t. solvable term
    ve_p->setapply(vc_p);

    // Report what is being applied and solved-for
    applystate();
    solvestate();

    // Generally use standard solver
    if (svc_p->standardSolve())
      standardSolve();
    else
      svc_p->selfSolve(*vs_p,*ve_p);

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;

    logSink() << "Reseting entire solve/apply state." << LogIO::POST;
    reset();

    throw(AipsError("Error in Calibrater::solve."));
    return False;
  } 

  return True;

}

Bool Calibrater::standardSolve() {
  
  // Create the solver
  VisCalSolver vcs;
  
  // Inform logger/history
  logSink() << "Solving for " << svc_p->typeName()
	    << LogIO::POST;
  
  // Arrange for iteration over data
  Block<Int> columns;
  if (svc_p->interval()==0.0) {
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
  vs_p->resetVisIter(columns,svc_p->interval());
  VisIter& vi(vs_p->iter());
  VisBuffer vb(vi);

  // Initialize the svc according to current VisSet
  //  (this counts intervals, sizes CalSet)
  svc_p->initSolve(*vs_p);
  
  // Solve each solution interval (chunk)
  Vector<Int> islot(vs_p->numberSpw(),0);
  Int nGood(0);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
    
    Int spw(vi.spectralWindow());
    
    // Arrange to accumulate 
    VisBuffAccumulator vba(vs_p->numberAnt(),svc_p->preavg(),False); 
    
    // Collapse each timestamp in this chunk according to VisEq
    //  with calibration and averaging
    for (vi.origin(); vi.more(); vi++) {

      // This forces the data/model/wt I/O, and applies
      //   any prior calibrations
      ve_p->collapse(vb);
      
      // If permitted/required by solvable component, normalize
      if (svc_p->normalizable()) 
	vb.normalize();

      // Accumulate collapsed vb in a time average
      vba.accumulate(vb);
    }
    vba.finalizeAverage();
    
    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff()); 

    svc_p->makePhaseOnly(svb);

    // Establish meta-data for this interval
    //  (some of this may be used _during_ solve)
    //  (this sets currSpw() in the SVC)
    //  (TBD: handle fieldId better)
    Bool vbOk=svc_p->syncSolveMeta(svb,vi.fieldId());
    
    if (vbOk) {

      svc_p->guessPar(svb);
      //svc_p->solveCPar()=Complex(0.3);
      //svc_p->solveParOK()=True;
    
      // Solve for each parameter channel (in curr Spw)
      
      // (NB: force const version of nChanPar()  [why?])
      //	for (Int ich=0;ich<((const SolvableVisCal*)svc_p)->nChanPar();++ich) {
      Bool totalGoodSol(False);
      for (Int ich=((const SolvableVisCal*)svc_p)->nChanPar()-1;ich>-1;--ich) {

	// If pars chan-dep, SVC mechanisms for only one channel at a time
	svc_p->focusChan()=ich;

	// Pass VE, SVC, VB to solver
	Bool goodSoln=vcs.solve(*ve_p,*svc_p,svb);

	// If good... 
	if (goodSoln) {
	  totalGoodSol=True;
	  // ...consider referencing to refant...
	  if (svc_p->refant()>-1)
	    svc_p->reReference();

	  // ..and file this solution in the correct slot
	  svc_p->keep(islot(spw));
	}
	
      } // parameter channels

      // Cound good solutions.
      if (totalGoodSol)	nGood++;
      
    } // vbOK
    
    islot(spw)++;
    
  } // chunks


  logSink() << "  Found " << nGood << " good " 
	    << svc_p->typeName() << " solutions." 
	    << LogIO::POST;
  
  // Store whole of result in a caltable
  if (nGood==0)
    logSink() << "No output calibration table written."
	      << LogIO::POST;
  else
    // write the table
    svc_p->store();

  return True;

}




Vector<Double> Calibrater::modelfit(const Int& niter,
				    const String& stype,
				    const Vector<Double>& par,
				    const Vector<Bool>& vary,
			            const String& file) {

 /*
  cout << "Calibrater::modelfit" << endl;
  cout << " niter = " << niter << endl;
  cout << " stype = " << stype << endl;
  cout << " par   = " << par << endl;
  cout << " vary  = " << vary << endl;
  cout << " file  = " << file << endl;
 */
  //  logSink() << LogOrigin("Calibrater","modelfit") << LogIO::NORMAL;
  
  try {
    if(!ok()) throw(AipsError("Calibrater not ok()"));

    // Construct UVMod with the VisSet
    UVMod uvmod(*vs_p);

    if (stype=="P")
      uvmod.setModel(ComponentType::POINT, par, vary);
    else if (stype=="G")
      uvmod.setModel(ComponentType::GAUSSIAN, par, vary);
    else if (stype=="D")
      uvmod.setModel(ComponentType::DISK, par, vary);
    else
      throw(AipsError("Unrecognized component type in Calibrater::modelfit."));
    
    // Run the fit
    uvmod.modelfit(niter,file);

    // Return the parameter vector
    return uvmod.par();

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    throw(AipsError("Error in Calibrater::modelfit."));
    
    return Vector<Double>();
  } 

}

void Calibrater::fluxscale(const String& infile, 
			   const String& outfile,
			   const Vector<String>& refFields, 
			   const Vector<Int>& refSpwMap, 
			   const Vector<String>& tranFields,
			   const Bool& append,
			   Matrix<Double>& fluxScaleFactor) {

  // TBD:  Permit more flexible matching on specified field names
  //  (Currently, exact matches are required.)

  logSink() << LogOrigin("Calibrater","fluxscale") << LogIO::NORMAL;

  // Convert refFields/transFields to index lists
  Vector<Int> refidx(0);
  Vector<Int> tranidx(0);

  // Field name->index matcher
  MSFieldIndex msfldidx(ms_p->field());

  Int nf;
  { 
    nf=refFields.nelements();
    if (nf>0) {
      refidx.resize(nf);
      for (Int i=0;i<nf;++i) {
	Vector<Int> idx=msfldidx.matchFieldName(refFields(i));
	if (idx.nelements()>0)
	  refidx(i)=idx(0);
	else
	  throw(AipsError("Reference field name matching error"));
      }
    }
  }

  { 
    nf=tranFields.nelements();
    if (nf>0) {
      tranidx.resize(nf);
      for (Int i=0;i<nf;++i) {
	Vector<Int> idx=msfldidx.matchFieldName(tranFields(i));
	if (idx.nelements()>0)
	  tranidx(i)=idx(0);
	else
	  throw(AipsError("Transfer field name matching error"));
      }
      
    }
  }

  // Call Vector<Int> version:
  fluxscale(infile,outfile,refidx,refSpwMap,tranidx,append,fluxScaleFactor);

}


void Calibrater::fluxscale(const String& infile, 
			   const String& outfile,
			   const Vector<Int>& refField, 
			   const Vector<Int>& refSpwMap, 
			   const Vector<Int>& tranField,
			   const Bool& append,
			   Matrix<Double>& fluxScaleFactor) {

  //  throw(AipsError("Method 'fluxscale' is temporarily disabled."));

  // TBD: write inputs to MSHistory
  logSink() << LogOrigin("Calibrater","fluxscale") << LogIO::NORMAL;

  try {
    // If infile is Calibration table
    if (Table::isReadable(infile) && 
	Table::tableInfo(infile).type()=="Calibration") {

      // get calibration type
      String caltype;
      caltype = Table::tableInfo(infile).subType();
      logSink() << "Table " << infile 
		<< " is of type: "<< caltype 
		<< LogIO::POST;
      String message="Table "+infile+" is of type: "+caltype;
      MSHistoryHandler::addMessage(*ms_p, message, "calibrater", "", "calibrater::fluxscale()");
      
      // form selection
      String select="";
      // Selection is empty for case of no tran specification
      if (tranField.nelements()>0) {
	
	// All selected fields
	Vector<Int> allflds = concatenateArray(refField,tranField);
	
	// Assemble TaQL
	ostringstream selectstr;
	selectstr << "FIELD_ID IN [";
	for (Int iFld=0; iFld<allflds.shape(); iFld++) {
	  if (iFld>0) selectstr << ", ";
	  selectstr << allflds(iFld);
	}
	selectstr << "]";
	select=selectstr.str();
      }

      // Construct proper SVC object
      SolvableVisCal *fsvj_;
      if (caltype == "G Jones") {
	fsvj_ = createSolvableVisCal("G",*vs_p);
      } else if (caltype == "T Jones") {
	fsvj_ = createSolvableVisCal("T",*vs_p);
      } else {
	// Can't process other than G and T (add B?)
	ostringstream typeErr;
	typeErr << "Type " << caltype 
	       << " not supported in fluxscale.";

	throw(AipsError(typeErr.str()));
      }

      // fill table with selection
      RecordDesc applyparDesc;
      applyparDesc.addField ("table", TpString);
      applyparDesc.addField ("select", TpString);
      Record applypar(applyparDesc);
      applypar.define ("table", infile);
      applypar.define ("select", select);
      fsvj_->setApply(applypar);

      // Make fluxscale calculation
      Vector<String> fldnames(ROMSFieldColumns(ms_p->field()).name().getColumn());
      fsvj_->fluxscale(refField,tranField,refSpwMap,fldnames,fluxScaleFactor);
     
      // If no outfile specified, use infile (overwrite!)
      String out(outfile);
      if (out.length()==0)
	out = infile;

      // Store result
      if (append) {
	logSink() << "Appending result to " << out << LogIO::POST;
	String message="Appending result to "+out;
	MSHistoryHandler::addMessage(*ms_p, message, "calibrater", "", "calibrater::fluxscale()");
      } else {
	logSink() << "Storing result in " << out << LogIO::POST;
	String message="Storing result in "+out;
	MSHistoryHandler::addMessage(*ms_p, message, "calibrater", "", "calibrater::fluxscale()");
      }
      fsvj_->store(out,append);
      
      // Clean up
      delete fsvj_;

    } else {
      // Table not found/unreadable, or not Calibration
      ostringstream tabErr;
      tabErr << "File " << infile
	     << " does not exist or is not a Calibration Table.";

      throw(AipsError(tabErr.str()));

    }
  } catch (AipsError x) {

    logSink() << LogIO::SEVERE
	      << "Caught Exception: "
	      << x.getMesg()
	      << LogIO::POST;
    
    // Write to MS History table
    //    String message="Caught Exception: "+x.getMesg();
    //    MSHistoryHandler::addMessage(*ms_p, message, "calibrater", "", "calibrater::fluxscale()");

    throw(AipsError("Error in Calibrater::fluxscale."));

    return;

  }
  return;


}

void Calibrater::accumulate(const String& intab,
			    const String& incrtab,
			    const String& outtab,
			    const Vector<String>& fields,
			    const Vector<String>& calFields,
			    const String& interp,
			    const Double& t,
			    const Vector<Int>& spwmap) {

  logSink() << LogOrigin("Calibrater","accumulate") << LogIO::NORMAL;

  // Convert refFields/transFields to index lists
  Vector<Int> fldidx(0);
  Vector<Int> cfldidx(0);

  // Field name->index matcher
  MSFieldIndex msfldidx(ms_p->field());

  Int nfld;
  { 
    nfld=fields.nelements();
    if (nfld>0) {
      fldidx.resize(nfld);
      for (Int i=0;i<nfld;++i) {
	Vector<Int> idx=msfldidx.matchFieldName(fields(i));
	if (idx.nelements()>0)
	  fldidx(i)=idx(0);
	else
	  throw(AipsError("field name matching error"));
      }
    }
  }
  { 
    nfld=calFields.nelements();
    if (nfld>0) {
      cfldidx.resize(nfld);
      for (Int i=0;i<nfld;++i) {
	Vector<Int> idx=msfldidx.matchFieldName(calFields(i));
	if (idx.nelements()>0)
	  cfldidx(i)=idx(0);
	else
	  throw(AipsError("calfield name matching error"));
      }
      
    }
  }

  // Call Vector<Int> version
  accumulate(intab,incrtab,outtab,fldidx,cfldidx,interp,t,spwmap);

}

void Calibrater::accumulate(const String& intab,
			    const String& incrtab,
			    const String& outtab,
			    const Vector<Int>& fields,
			    const Vector<Int>& calFields,
			    const String& interp,
			    const Double& t,
			    const Vector<Int>& spwmap) {

  //  throw(AipsError("Method 'accumulate' is temporarily disabled."));
  
  //  logSink() << LogOrigin("Calibrater","accumulate") << LogIO::NORMAL;

  logSink() << "Beginning accumulate." << LogIO::POST;

  // SVJ objects:
  SolvableVisCal *incal_(NULL), *incrcal_(NULL);

  try {
    
  /*
    cout << "intab     = " << intab << endl;
    cout << "incrtab   = " << incrtab << endl;
    cout << "outtab    = " << outtab << endl;
    cout << "fields    = " << fields << endl;
    cout << "calFields = " << calFields << endl;
    cout << "interp    = " << interp << endl;
    cout << "t         = " << t << endl;
  */

    // Incremental table's type sets the type we are dealing with
    String caltype=calTableType(incrtab);

    // If no input cumulative timescale specified, then
    //   a valid input cumulative table must be specified
    if (t < 0.0) {

      String intype=calTableType(intab);

      if (intype!=caltype) {
      
	ostringstream typeErr;
	typeErr << "Table " << intab
		<< " is not the same type as "
		<< incrtab << " (" << caltype << ")";
	throw(AipsError(typeErr.str()));
      }
    }

    // At this point all is ok; we will:
    //  o fill from intab and accumulate to it (t<0), OR
    //  o create a new cumulative table from scratch (t>0)

    // If creating a new cumulative table, it must span the whole dataset,
    //   so reset data selection to whole MS, and setup iterator
    if (t>0.0) {
      setdata();
      Block<Int> columns;
      columns.resize(4);
      columns[0]=MS::ARRAY_ID;
      columns[1]=MS::TIME;
      columns[2]=MS::FIELD_ID;
      columns[3]=MS::DATA_DESC_ID;
      vs_p->resetVisIter(columns,t);
    }

    //	logSink() << "Table " << infile 
    //		  << " is of type: "<< caltype 
    //		  << LogIO::POST;

    incal_ = createSolvableVisCal(caltype,*vs_p);
    incrcal_ = createSolvableVisCal(caltype,*vs_p);
    
    // TBD: move to svj.setAccumulate?
    if ( !(incal_->accumulatable()) ) {
      ostringstream typeErr;
      typeErr << "Type " << caltype 
	      << " not yet supported in accumulate.";
      throw(AipsError(typeErr.str()));
    }

    // At this point, accumulation apparently supported,
    //   so continue...
    
    // initialize the cumulative solutions
    incal_->setAccumulate(*vs_p,intab,"",t,-1);
    
    // form selection on incr table
    String incrSel="";
    if (calFields.shape()>0) {
      
      // Assemble TaQL
      ostringstream selectstr;
      selectstr << "FIELD_ID IN [";
      for (Int iFld=0; iFld<calFields.shape(); iFld++) {
	if (iFld>0) selectstr << ", ";
	selectstr << calFields(iFld);
      }
      selectstr << "]";
      incrSel=selectstr.str();
    }
    
    // fill incr table with selection
    logSink() << "Preparing to accumulate calibration from table: "
	      << incrtab
	      << LogIO::POST;

    // Set record format for calibration table application information
    RecordDesc applyparDesc;
    applyparDesc.addField ("t", TpDouble);
    applyparDesc.addField ("table", TpString);
    applyparDesc.addField ("select", TpString);
    applyparDesc.addField ("interp", TpString);
    applyparDesc.addField ("spwmap",TpArrayInt);
    
    // Create record with the requisite field values
    Record applypar(applyparDesc);
    applypar.define ("t", t);
    applypar.define ("table", incrtab);
    applypar.define ("select", incrSel);
    applypar.define ("interp", interp);
    applypar.define ("spwmap",spwmap);

    incrcal_->setApply(applypar);

    // All ready, now do the accumulation
    incal_->accumulate(incrcal_,fields);
    
    // ...and store the result
    logSink() << "Storing accumulated calibration in table: " 
	      << outtab
	      << LogIO::POST;

    if (outtab != "") 
      incal_->calTableName()=outtab;

    incal_->store();
    
    delete incal_;
    delete incrcal_;

    logSink() << "Finished accumulation." 
	      << LogIO::POST;

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE
	      << "Caught Exception: "
	      << x.getMesg()
	      << LogIO::POST;

    if (incal_) delete incal_;
    if (incrcal_) delete incrcal_;
    
    throw(AipsError("Error in Calibrater::accumulate."));
    return;
  }
  return;

}


Bool Calibrater::smooth(const String& infile, 
			String& outfile,  // const Bool& append,
                        const String& smoothtype,
			const Double& smoothtime,
                        const Vector<String>& fields)
{

  //  throw(AipsError("Method 'smooth' is temporarily disabled."));
  
  // TBD: support append?
  // TBD: spw selection?

  logSink() << LogOrigin("Calibrater","smooth") << LogIO::NORMAL;

  logSink() << "Beginning smoothing/interpolating method." << LogIO::POST;


  // A pointer to an SVC
  SolvableVisCal *svc(NULL);

  try {
    
    // Handle no in file 
    if (infile=="")
      throw(AipsError("Please specify an input calibration table."));

    // Handle bad smoothtype
    if (smoothtype!="mean" && smoothtype!="median")
      throw(AipsError("Unrecognized smooth type!"));

    // Handle bad smoothtime
    if (smoothtime<=0)
      throw(AipsError("Please specify a strictly positive smoothtime."));

    // Handle no outfile
    if (outfile=="") {
      outfile=infile;
      logSink() << "Will overwrite input file with smoothing result." 
		<< LogIO::POST;
    }


    svc = createSolvableVisCal(calTableType(infile),*vs_p);
    
    if (svc->smoothable()) {
      
      // Fill calibration table using setApply
      RecordDesc applyparDesc;
      applyparDesc.addField ("table", TpString);
      Record applypar(applyparDesc);
      applypar.define ("table", infile);
      svc->setApply(applypar);

      // Convert refFields/transFields to index lists
      Vector<Int> fldidx(0);

      // Field name->index matcher
      MSFieldIndex msfldidx(ms_p->field());
      
      Int nfld;
      { 
	nfld=fields.nelements();
	if (nfld>0) {
	  fldidx.resize(nfld);
	  for (Int i=0;i<nfld;++i) {
	    Vector<Int> idx=msfldidx.matchFieldName(fields(i));
	    if (idx.nelements()>0)
	      fldidx(i)=idx(0);
	    else
	      throw(AipsError("field name matching error"));
	  }
	}
      }

      // Delegate to SVC
      svc->smooth(fldidx,smoothtype,smoothtime);
      
      // Store the result on disk
      //    if (append) logSink() << "Appending result to " << outfile << LogIO::POST;
      //else 
      logSink() << "Storing result in " << outfile << LogIO::POST;
      
      
      if (outfile != "") 
	svc->calTableName()=outfile;
      svc->store();

      //TBD:      svc->store(outfile,append);
      
      // Clean up
      if (svc) delete svc; svc=NULL;
      
      // Apparently, it worked
      return True;

    }
    else
      throw(AipsError("This type does not support smoothing."));

  } catch (AipsError x) {
   
    logSink() << LogIO::SEVERE
	      << "Caught Exception: "
	      << x.getMesg()
	      << LogIO::POST;
    // Clean up
    if (svc) delete svc; svc=NULL;

    throw(AipsError("Error in Calibrater::smooth."));

    return False;
  }
  return False;
}

// Query apply types if we are calibrating the weights
Bool Calibrater::calWt() {

  Int napp(vc_p.nelements());
  // Return True as soon as we find a type which is cal'ing wts
  for (Int iapp=0;iapp<napp;++iapp)
    if (vc_p[iapp] && vc_p[iapp]->calWt())
      return True;

  // None cal'd weights, so return False
  return False;

}

Bool Calibrater::ok() {

  if(vs_p && ms_p && mssel_p && ve_p) {
    return True;
  }
  else {
    logSink() << "Calibrater is not yet initialized" << LogIO::POST;
    return False;
  }
}

void Calibrater::writeHistory(LogIO& os, Bool cliCommand)
{
  if (!historytab_p.isNull()) {
    if (histLockCounter_p == 0) {
      historytab_p.lock(False);
    }
    ++histLockCounter_p;

    os.postLocally();
    if (cliCommand) {
      hist_p->cliCommand(os);
    } else {
      hist_p->addMessage(os);
    }

    if (histLockCounter_p == 1) {
      historytab_p.unlock();
    }
    if (histLockCounter_p > 0) {
      --histLockCounter_p;
    }
  } else {
    os << LogIO::SEVERE << "calibrater is not yet initialized" << LogIO::POST;
  }
}

} //# NAMESPACE CASA - END
