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

#include <casa/System/AipsrcValue.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>
//#include <casa/Arrays/ArrayMath.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSFieldIndex.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <synthesis/MeasurementComponents/Calibrater.h>
#include <synthesis/CalTables/CLPatchPanel.h>
#include <synthesis/MeasurementComponents/VisCalSolver.h>
#include <synthesis/MeasurementComponents/UVMod.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/Quanta/MVTime.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

#include <tables/Tables/SetupNewTab.h>
#include <vector>
using std::vector;
#include <msvis/MSVis/UtilJ.h>

using namespace casa::utilj;

using namespace casa::vpf;

namespace casa { //# NAMESPACE CASA - BEGIN

Calibrater::Calibrater(): 
  ms_p(0), 
  mssel_p(0), 
  vs_p(0), 
  ve_p(0),
  vc_p(),
  svc_p(0),
  histLockCounter_p(), 
  hist_p(0),
  actRec_()
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

Bool Calibrater::initialize(MeasurementSet& inputMS, 
			    Bool compress,
			    Bool addScratch, Bool addModel)  {
  
  logSink() << LogOrigin("Calibrater","") << LogIO::NORMAL3;
  
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
    // jagonzal (CAS-4110): When the selectvis method throws an exception the initialize method
    // is called again to leave the calibrater in a proper state, and since there was a previous
    // initialization the history handler was already created, and has to be destroyed before
    // creating a new one to avoid leaveing the HISTORY table opened.
    if (hist_p) delete hist_p;
    hist_p= new MSHistoryHandler(*ms_p, "calibrater");


    // Remember the ms's name
    msname_p=ms_p->tableName();


    // Add/init scr cols, if requested (init is hard-wired)
    if (addScratch || addModel) {
      Bool alsoinit=True;
      VisSetUtil::addScrCols(*ms_p,addModel,addScratch,alsoinit,compress);
    }

    // Set the selected MeasurementSet to be the same initially
    // as the input MeasurementSet
    if (mssel_p) delete mssel_p;
    mssel_p=new MeasurementSet(*ms_p);
    
    logSink() << LogIO::NORMAL
	      << "Initializing nominal selection to the whole MS."
	      << LogIO::POST;


    // Create a VisSet with no selection
    // (gmoellen 2012/02/06: this merely makes a VisIter now)
    if (vs_p) {
      delete vs_p;
      vs_p=0;
    };
    Block<Int> nosort(0);
    Matrix<Int> noselection;
    Double timeInterval=0;
    // gmoellen 2012/02/06    vs_p=new VisSet(*ms_p,nosort,noselection,addScratch,timeInterval,compress, addModel);
    vs_p=new VisSet(*ms_p,nosort,noselection,False,timeInterval,False,False);

    // Size-up the chanmask PB
    initChanMask();

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

  //  logSink() << LogOrigin("Calibrater","initCalSet") << LogIO::NORMAL3;

  if (vs_p) {

    Block<Int> columns;
    // include scan iteration, for more optimal iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
    vs_p->resetVisIter(columns,0.0);

    vs_p->initCalSet(calSet);
    return True;
  }
  else {
    throw(AipsError("Calibrater cannot initCalSet"));
    return False;
  }
}


// Select data (using MSSelection syntax)
void Calibrater::selectvis(const String& time,
			   const String& spw,
			   const String& scan,
			   const String& field,
			   const String& intent,
			   const String& obsIDs,
			   const String& baseline,
			   const String& uvrange,
			   const String& chanmode,
			   const Int& nchan,
			   const Int& start, 
			   const Int& step,
			   const MRadialVelocity& mStart,
			   const MRadialVelocity& mStep,
			   const String& msSelect)
{
// Define primary measurement set selection criteria
// Inputs:
//    time
//    spw
//    scan
//    field
//    intent
//    obsIDs
//    baseline
//    uvrange
//    chanmode     const String&            Frequency/velocity selection mode
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
  logSink() << LogOrigin("Calibrater","selectvis") << LogIO::NORMAL3;
  
  try {

 /*   
    cout << "time     = " << time << " " << time.length() <<endl;
    cout << "spw      = " << spw << " " << spw.length() <<endl;
    cout << "scan     = " << scan << " " << scan.length() <<endl;
    cout << "field    = " << field << " " << field.length() <<endl;
    cout << "baseline = " << baseline << " " << baseline.length() << endl;
    cout << "uvrange  = " << uvrange << " " << uvrange.length() << endl;
 */

    logSink() << "Selecting data" << LogIO::POST;
    
    // Apply selection to the original MeasurementSet
    logSink() << "Performing selection on MeasurementSet" << endl;
    
    if (mssel_p) {
      delete mssel_p;
      mssel_p=0;
    };

    // Report non-trivial user selections
    if (time!="")
      logSink() << " Selecting on time: '" << time << "'" << endl;
    if (spw!="")
      logSink() << " Selecting on spw: '" << spw << "'" << endl;
    if (scan!="")
      logSink() << " Selecting on scan: '" << scan << "'" << endl;
    if (field!="")
      logSink() << " Selecting on field: '" << field << "'" << endl;
    if (intent!="")
      logSink() << " Selecting on intent: '" << intent << "'" << endl;
    if(obsIDs != "")
      logSink() << " Selecting by observation IDs: '" << obsIDs << "'" << endl;
    if (baseline!="")
      logSink() << " Selecting on antenna/baseline: '" << baseline << "'" << endl;
    if (uvrange!="")
      logSink() << " Selecting on uvrange: '" << uvrange << "'" << endl;
    if (msSelect!="")
      logSink() << " Selecting with TaQL: '" << msSelect << "'" << endl;
    logSink() << LogIO::POST;


    // Assume no selection, for starters
    // gmoellen 2012/01/30    mssel_p = new MeasurementSet(sorted, ms_p);
    mssel_p = new MeasurementSet(*ms_p);

    // Apply user-supplied selection
    Bool nontrivsel=False;
    // gmoellen 2012/01/30    nontrivsel= mssSetData(MeasurementSet(sorted, ms_p),
    nontrivsel= mssSetData(*ms_p,
			   *mssel_p,"",
			   time,baseline,
			   field,spw,
			   uvrange,msSelect,
			   "",scan,"",intent, obsIDs);

    // Keep any MR status for the MS
    mssel_p->setMemoryResidentSubtables(ms_p->getMrsEligibility());

    // If non-trivial MSSelection invoked and nrow reduced:
    if(nontrivsel && mssel_p->nrow()<ms_p->nrow()) {

      // Escape if no rows selected
      if (mssel_p->nrow()==0) 
	throw(AipsError("Specified selection selects zero rows!"));

      // ...otherwise report how many rows are selected
      logSink() << "By selection " << ms_p->nrow() 
		<< " rows are reduced to " << mssel_p->nrow() 
		<< LogIO::POST;
    }
    else {
      // Selection did nothing:
      logSink() << "Selection did not drop any rows" << LogIO::POST;
    }

    // Now, re-create the associated VisSet
    if(vs_p) delete vs_p; vs_p=0;
    Block<int> sort(0);
    Matrix<Int> noselection;
    // gmoellen 2012/01/30    vs_p = new VisSet(*mssel_p,sort,noselection);
    vs_p = new VisSet(*mssel_p,sort,noselection,False,0.0,False,False);
    AlwaysAssert(vs_p, AipsError);

    // Attempt to use MSSelection for channel selection
    //  if user not using the old way
    if (chanmode=="none") {
      selectChannel(spw);
    }
    else {
      // Reluctantly use the old-fashioned way
      logSink() << LogIO::WARN 
		<< "You have used the old-fashioned mode parameter" << endl
		<< "for channel selection.  It still works, for now," << endl
		<< "but this will be eliminated in the near future." << endl
		<< "Please begin using the new channel selection" << endl
		<< "syntax in the spw parameter." << LogIO::POST;
      selectChannel(chanmode,nchan,start,step,mStart,mStep);
    }

  }
  catch (MSSelectionError& x) {
    // Re-initialize with the existing MS
    logSink() << LogOrigin("Calibrater","selectvis",WHERE) 
	      << LogIO::SEVERE << "Caught exception: " << x.getMesg()
	      << LogIO::POST;
    // jagonzal (CAS-4110): I guess it is not necessary to create these columns when the selection is empty
    initialize(*ms_p,False,False,False);
    throw(AipsError("Error in data selection specification: " + x.getMesg()));
  } 
  catch (AipsError x) {
    // Re-initialize with the existing MS
    logSink() << LogOrigin("Calibrater","selectvis",WHERE) 
	      << LogIO::SEVERE << "Caught exception: " << x.getMesg()
	      << LogIO::POST;
    // jagonzal (CAS-4110): I guess it is not necessary to create these columns when the selection is empty.
    initialize(*ms_p,False,False,False);
    throw(AipsError("Error in Calibrater::selectvis(): " + x.getMesg()));
  } 
};


Bool Calibrater::setapply(const String& type, 
			  const Double& t,
			  const String& table,
			  const String& spw,
			  const String& field,
			  const String& interp,
			  const Bool& calwt,
			  const Vector<Int>& spwmap,
			  const Vector<Double>& opacity) 
{

  logSink() << LogOrigin("Calibrater",
                         "setapply(type, t, table, spw, field, interp, calwt, spwmap, opacity)")
            << LogIO::NORMAL;


  //  cout << "Calibrater::setapply: field="<< field << endl;
   
  // Set record format for calibration table application information
  RecordDesc applyparDesc;
  applyparDesc.addField ("t", TpDouble);
  applyparDesc.addField ("table", TpString);
  applyparDesc.addField ("interp", TpString);
  applyparDesc.addField ("spw", TpArrayInt);
  //  applyparDesc.addField ("field", TpArrayInt);
  applyparDesc.addField ("fieldstr", TpString);
  applyparDesc.addField ("calwt",TpBool);
  applyparDesc.addField ("spwmap",TpArrayInt);
  applyparDesc.addField ("opacity",TpArrayDouble);
  
  // Create record with the requisite field values
  Record applypar(applyparDesc);
  applypar.define ("t", t);
  applypar.define ("table", table);

  /*
  String cinterp=interp;
  //  cinterp.erase(remove_if(cinterp.begin(), cinterp.end(), isspace), cinterp.end());
  cinterp.erase( remove( cinterp.begin(), cinterp.end(), ' ' ), cinterp.end() );
  */
 
  applypar.define ("interp", interp);
  applypar.define ("spw",getSpwIdx(spw));
  //  applypar.define ("field",getFieldIdx(field));
  applypar.define ("fieldstr",field);
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
			   const Record& applypar)
{
  logSink() << LogOrigin("Calibrater", "setapply(type, applypar)");

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


// Validate a Cal Library record
Bool Calibrater::validatecallib(Record callib) {

  uInt ntab=callib.nfields();
  for (uInt itab=0;itab<ntab;++itab) {

    String tabname=callib.name(itab);
    Record thistabrec=callib.asRecord(itab);
    uInt ncl=thistabrec.nfields();

    for (uInt icl=0;icl<ncl;++icl) {

      Record thisicl=thistabrec.asRecord(icl);
      try {
	CalLibSlice::validateCLS(thisicl);
      }
      catch ( AipsError x) {
	logSink() << LogIO::SEVERE
		  << "Caltable " << tabname 
		  << " is missing the following fields: " 
		  << x.getMesg() 
		  << LogIO::POST;
      }
    }
  }
  return True;
}
// Set up apply-able calibration via a Cal Library
Bool Calibrater::setcallib(Record callib) {

  logSink() << LogOrigin("Calibrater", "setcallib(callib)");

  //  cout << "Calibrater::setcallib: callib.isFixed() = " << boolalpha << callib.isFixed() << endl;

  uInt ntab=callib.nfields();

  //  cout << "callib.nfields() = " << ntab << endl;

  // Do some preliminary per-table verification
  for (uInt itab=0;itab<ntab;++itab) {

    String tabname=callib.name(itab);

    // Insist that the table exists on disk
    if (!Table::isReadable(tabname))
      throw(AipsError("Caltable "+tabname+" does not exist."));

  }

  // Tables exist, so deploy them...

  for (uInt itab=0;itab<ntab;++itab) {

    String tabname=callib.name(itab);

    // Get the type from the table
    String upType=calTableType(tabname);
    upType.upcase();

    // Add table name to the record
    Record thistabrec=callib.asrwRecord(itab);
    thistabrec.define("tablename",tabname);

    // First try to create the requested VisCal object
    VisCal *vc(NULL);

    try {

      if(!ok()) 
	throw(AipsError("Calibrater not prepared for setapply."));
      
      logSink() << LogIO::NORMAL 
		<< "Arranging to APPLY:"
		<< LogIO::POST;
      
      // Add a new VisCal to the apply list
      vc = createVisCal(upType,*vs_p);  


      // ingest this table according to its callib
      vc->setCallib(thistabrec,*mssel_p);

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
      
    } catch (AipsError x) {
      logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
		<< LogIO::POST;
      if (vc) delete vc;
      throw(AipsError("Error in Calibrater::setapply."));
      return False;
    } 
  }

  // All ok, if we get this far!
  return True;

}

Bool Calibrater::setmodel(const String& modelImage)
{
  if (!svc_p)
    throw(AipsError("Calibrater::setmodel() called before Calibrater::setsolve()"));
  svc_p->setModel(modelImage);
  return True;
}

Bool Calibrater::setModel(const Vector<Double>& stokes) {

  if (ve_p) {
    Vector<Float> fstokes(stokes.shape());
    convertArray(fstokes,stokes);
    ve_p->setModel(fstokes);
  }
  else
    throw(AipsError("Error in Calibrater::setModel: no VisEquation."));

  return True;

}

Bool Calibrater::setsolve (const String& type, 
			   const String& solint,
			   const String& table,
                           const Bool append,
                           const Double preavg, 
			   const String& apmode,
			   const Int minblperant,
                           const String& refant,
			   const Bool solnorm,
			   const Float minsnr,
			   const String& combine,
			   const Int fillgaps,
			   const String& cfcache,
			   const Double painc,
                           const Int fitorder)
{
  
  logSink() << LogOrigin("Calibrater","setsolve") << LogIO::NORMAL3;
  
  // Create a record description containing the solver parameters
  RecordDesc solveparDesc;
  solveparDesc.addField ("solint", TpString);
  solveparDesc.addField ("preavg", TpDouble);
  solveparDesc.addField ("apmode", TpString);
  solveparDesc.addField ("refant", TpArrayInt);
  solveparDesc.addField ("minblperant", TpInt);
  solveparDesc.addField ("table", TpString);
  solveparDesc.addField ("append", TpBool);
  solveparDesc.addField ("solnorm", TpBool);
  solveparDesc.addField ("minsnr", TpFloat);
  solveparDesc.addField ("type", TpString);
  solveparDesc.addField ("combine", TpString);
  solveparDesc.addField ("maxgap", TpInt);
  solveparDesc.addField ("cfcache", TpString);
  solveparDesc.addField ("painc", TpDouble);
  solveparDesc.addField ("fitorder", TpInt);
  
  // Create a solver record with the requisite field values
  Record solvepar(solveparDesc);
  solvepar.define ("solint", solint);
  solvepar.define ("preavg", preavg);
  String upmode=apmode;
  upmode.upcase();
  solvepar.define ("apmode", upmode);
  solvepar.define ("refant", getRefantIdxList(refant));
  solvepar.define ("minblperant", minblperant);
  solvepar.define ("table", table);
  solvepar.define ("append", append);
  solvepar.define ("solnorm", solnorm);
  solvepar.define ("minsnr", minsnr);
  String uptype=type;
  uptype.upcase();
  solvepar.define ("type", uptype);
  
  String upcomb=combine;
  upcomb.upcase();
  solvepar.define("combine",upcomb);
  solvepar.define("maxgap",fillgaps);
  solvepar.define ("cfcache", cfcache);
  solvepar.define ("painc", painc);
  solvepar.define("fitorder", fitorder);

  return setsolve(type,solvepar);
}

Bool Calibrater::setsolvebandpoly(const String& table,
				  const Bool& append,
				  const String& solint,
				  const String& combine,
				  const Vector<Int>& degree,
				  const Bool& visnorm,
				  const Bool& solnorm,
				  const Int& maskcenter,
				  const Float& maskedge,
				  const String& refant) {

  logSink() << LogOrigin("Calibrater","setsolvebandpoly") << LogIO::NORMAL3;

  // TBD: support solution interval!

    // Create a record description containing the solver parameters
    RecordDesc solveparDesc;
    solveparDesc.addField ("table", TpString);
    solveparDesc.addField ("append", TpBool);
    solveparDesc.addField ("solint", TpString);
    solveparDesc.addField ("combine", TpString);
    solveparDesc.addField ("degree", TpArrayInt);
    solveparDesc.addField ("visnorm", TpBool);
    solveparDesc.addField ("solnorm", TpBool);
    solveparDesc.addField ("maskcenter", TpInt);
    solveparDesc.addField ("maskedge", TpFloat);
    solveparDesc.addField ("refant", TpArrayInt);

    //    solveparDesc.addField ("preavg", TpDouble);
    //    solveparDesc.addField ("phaseonly", TpBool);
    
    // Create a solver record with the requisite field values
    Record solvepar(solveparDesc);
    solvepar.define ("table", table);
    solvepar.define ("append", append);
    solvepar.define ("solint",solint);
    String upcomb=combine;
    upcomb.upcase();
    solvepar.define ("combine",combine);
    solvepar.define ("degree", degree);
    solvepar.define ("visnorm", visnorm);
    solvepar.define ("solnorm", solnorm);
    solvepar.define ("maskcenter", maskcenter);
    solvepar.define ("maskedge", maskedge);
    solvepar.define ("refant", getRefantIdxList(refant));


    //    solvepar.define ("t", t);
    //    solvepar.define ("preavg", preavg);
    //    solvepar.define ("phaseonly", phaseonly);


    return setsolve("BPOLY",solvepar);

}

Bool Calibrater::setsolvegainspline(const String& table,
				    const Bool&   append,
				    const String& apmode,
				    const Double& splinetime,
				    const Double& preavg,
				    const Int&    numpoint,
				    const Double& phasewrap,
				    const String& refant) {
  
  logSink() << LogOrigin("Calibrater","setsolvegainspline") << LogIO::NORMAL3;

  // Create a record description containing the solver parameters
  RecordDesc solveparDesc;
  solveparDesc.addField ("table", TpString);
  solveparDesc.addField ("append", TpBool);
  solveparDesc.addField ("apmode", TpString);
  solveparDesc.addField ("splinetime", TpDouble);
  solveparDesc.addField ("preavg", TpDouble);
  solveparDesc.addField ("refant", TpArrayInt);
  solveparDesc.addField ("numpoint", TpInt);
  solveparDesc.addField ("phasewrap", TpDouble);
  
  // Create a solver record with the requisite field values
  Record solvepar(solveparDesc);
  solvepar.define ("table", table);
  solvepar.define ("append", append);
  String upMode=apmode;
  upMode.upcase();
  solvepar.define ("apmode", upMode);
  solvepar.define ("splinetime",splinetime);
  solvepar.define ("preavg", preavg);
  solvepar.define ("refant", getRefantIdxList(refant));
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

  logSink() << LogOrigin("Calibrater","state") << LogIO::NORMAL3;

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

  // Delete chanmask
  initChanMask();

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
  if (apply && vs_p) 
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


Bool
Calibrater::correct(String mode)
{
    logSink() << LogOrigin("Calibrater","correct") << LogIO::NORMAL;

    Bool retval = true;

    try {

        // make mode all-caps
        String upmode=mode;
	upmode.upcase();

	// If trialmode=T, only the flags will be set
	//   (and only written if not TRIAL)
	Bool trialmode=(upmode.contains("TRIAL") || 
			upmode.contains("FLAGONLY"));

        // Set up VisSet and its VisibilityIterator.

        VisibilityIterator::DataColumn whichOutCol = configureForCorrection ();

        // See if this is to be handled using the Visibility Processing Framework

        Bool useVpf;
        AipsrcValue<Bool>::find (useVpf, "Calibrater.useVpf", False);

        if (useVpf){
            logSink() << "Using VPF for correction" << LogIO::POST;
            return correctUsingVpf ();
        }

        VisIter& vi(vs_p->iter());
        VisBufferAutoPtr vb (vi);
        vi.origin();

        // Pass each timestamp (VisBuffer) to VisEquation for correction

        Vector<Bool> uncalspw(vi.numberSpw());	// Used to accumulate error messages
        uncalspw.set(False);		        // instead of bombing the user
                                        // in a loop.

        for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

            for (vi.origin(); vi.more(); vi++) {

                uInt spw = vb->spectralWindow();

		// Re-initialize weights from sigma column
		vb->resetWeightMat();

		// If we can calibrate this vb, do it...
                if (ve_p->spwOK(spw)){
		  
		  // throws exception if nothing to apply
		  ve_p->correct(*vb,trialmode);
		    
                }
		// ...else don't, prepare warning, and possibly set flags
                else{

		  // set uncalspw for warning message
		  uncalspw[spw] = true;
		  // set the flags, if we are being strict
		  if (upmode.contains("STRICT"))
		    // set the flags
		    // (don't touch the data/weights, which are initialized)
		    vb->flag().set(True);
                }

		// Only if not a trial run, trigger write to disk
		if (!upmode.contains("TRIAL")) {
		      
		  if (upmode.contains("CAL")) {
		    vi.setVis (vb->visCube(), whichOutCol);
		    vi.setWeightMat(vb->weightMat()); 
		  }
		  
		  if (upmode.contains("FLAG"))
		    vi.setFlag (vb->flag());
		  
		}
		
            }
        }

        vs_p->flush (); // Flush to disk

        // Now that we're out of the loop, summarize any errors.

        retval = summarize_uncalspws(uncalspw, "correct", 
				     upmode.contains("STRICT"));

	actRec_=Record();
	actRec_.define("origin","Calibrater::correct");
	actRec_.defineRecord("VisEquation",ve_p->actionRec());

    }
    catch (AipsError x) {
        logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg()
	              << LogIO::POST;

        logSink() << "Resetting all calibration application settings." << LogIO::POST;
        unsetapply();

        throw(AipsError("Error in Calibrater::correct."));
        retval = False;         // Not that it ever gets here...
    }
    return retval;
 }


VisibilityIterator::DataColumn
Calibrater::configureForCorrection ()
{
    if (!ok())
      throw(AipsError("Calibrater not prepared for correct!"));

    // Nominally, we write out to the CORRECTED_DATA, unless absent
    VisibilityIterator::DataColumn whichOutCol(VisibilityIterator::Corrected);

    if (!ms_p->tableDesc().isColumn("CORRECTED_DATA"))
      throw(AipsError("CORRECTED_DATA column unexpectedly absent. Cannot correct."));

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

    // Reset the VisibilityIterator in the VisSet.  If asyncio is to be
    // used then configure the prefetch columns.  Use the casarc setting
    // Calibrater.async to decide if applying calibration should
    // utilize async i/o.  This is in addition to the global setting which
    // must also be enabled to use async i/o (see VisibilityIterator.{cc,h}).

    Bool isEnabled=False; // Initializing to False to get rid of Warning.  Jim, please check if this is what you intended. It was earlier " Bool isEnabled; ". 
    //UNUSED: Bool foundSetting = AipsrcValue<Bool>::find (isEnabled, "Calibrater.asyncio", False);

    // isEnabled = ! foundSetting || isEnabled; // let global flag call shots if setting not present
    // For now (3/19/12) make asyncio for apply cal be explicitly enabled.

    asyncio::PrefetchColumns * prefetchColumns = NULL;

    if (isEnabled){
        prefetchColumns = new asyncio::PrefetchColumns ();
        * prefetchColumns =
            asyncio::PrefetchColumns::prefetchColumns (VisBufferComponents::Ant1,
                                                       VisBufferComponents::Ant2,
                                                       VisBufferComponents::Flag,
                                                       VisBufferComponents::FlagRow,
                                                       VisBufferComponents::FieldId,
                                                       VisBufferComponents::Freq,
                                                       VisBufferComponents::NChannel,
                                                       VisBufferComponents::NCorr,
                                                       VisBufferComponents::NRow,
                                                       VisBufferComponents::ObservedCube,
                                                       VisBufferComponents::SigmaMat,
                                                       VisBufferComponents::SpW,
                                                       VisBufferComponents::Time,
                                                       VisBufferComponents::WeightMat,
                                                       -1);
    }

    vs_p->resetVisIter (columns, 0.0, prefetchColumns);

    delete prefetchColumns;

    return whichOutCol;
}

Bool
Calibrater::correctUsingVpf ()
{
    Bool result = False;

    auto_ptr<CorrectorVp> correctorVp (getCorrectorVp ());
    auto_ptr<WriterVp> writerVp ( new WriterVp ("CorrectionWriter"));
    auto_ptr<VpContainer> vpContainer (new VpContainer ("CalibraterContainer"));

    vpContainer->add (correctorVp.get());
    vpContainer->add (writerVp.get());

    vpContainer->connect (correctorVp.get(), "Out", writerVp.get(), "In");
    vpContainer->connect ("In", correctorVp.get(), "In");

    ROVisibilityIterator * vi = correctorVp->getVisibilityIterator ();

    VpEngine vpEngine;

    try{

        vpEngine.process (* vpContainer, * vi);
        result = True;
    }
    catch (AipsError & e){

        logSink () << LogIO::SEVERE << "Calibrated::correctUsingVpf: " << e.what() << LogIO::POST
                   << LogIO::NORMAL;

    }

    return result;

}


Bool Calibrater::corrupt() {
  
  logSink() << LogOrigin("Calibrater","corrupt") << LogIO::NORMAL;
  Bool retval = true;

  try {

    if (!ok())
      throw(AipsError("Calibrater not prepared for corrupt!"));

    // Nominally, we write out to the MODEL_DATA, unless absent
    VisibilityIterator::DataColumn whichOutCol(VisibilityIterator::Model);

    if (!ms_p->tableDesc().isColumn("MODEL_DATA"))
      throw(AipsError("MODEL_DATA column unexpectedly absent. Cannot corrupt."));

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
    
    // Pass each timestamp (VisBuffer) to VisEquation for corruption.
    Vector<Bool> uncalspw(vi.numberSpw());	// Used to accumulate error messages
    uncalspw.set(False);		        // instead of bombing the user
						// in a loop.
    for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
      Int spw = vi.spectralWindow();

      // Only proceed if spw can be calibrated
      if (ve_p->spwOK(spw)) {

	for (vi.origin(); vi.more(); vi++) {
	  
	  // Corrupt the MODEL_DATA
	  //  (note we are not treating weights and flags)
	  ve_p->corrupt(vb);    // throws exception if nothing to apply
	  vi.setVis(vb.modelVisCube(),whichOutCol);

	}
      }
      else 
	uncalspw[spw] = true;
    }
    // Flush to disk
    vs_p->flush();

    // Now that we're out of the loop, summarize any errors.
    retval = summarize_uncalspws(uncalspw, "corrupt");
  }
  catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;

    logSink() << "Resetting all calibration application settings." << LogIO::POST;
    unsetapply();

    throw(AipsError("Error in Calibrater::corrupt."));
    retval = False;  // Not that it ever gets here...
  } 
  return retval;
}


Bool Calibrater::initWeights() {

  logSink() << LogOrigin("Calibrater","initWeights2") << LogIO::NORMAL;
  Bool retval = true;

  try {

    if (!ok())
      throw(AipsError("Calibrater not prepared for initWeights!"));

    // Log that we are beginning...

    // Arrange for iteration over data
    Block<Int> columns;
    // include scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;

    vi::SortColumns sc(columns);
    vi::VisibilityIterator2 vi2(*ms_p,sc,True);
    vi::VisBuffer2 *vb = vi2.getVisBuffer();

    ROMSColumns mscol(*ms_p);
    const ROMSSpWindowColumns& msspw(mscol.spectralWindow());
    uInt nSpw=msspw.nrow();
    Vector<Double> effChBw(nSpw,0.0);
    for (uInt ispw=0;ispw<nSpw;++ispw) {
      effChBw[ispw]=mean(msspw.effectiveBW()(ispw));
    }

    Int ivb(0);
    for (vi2.originChunks(); vi2.moreChunks(); vi2.nextChunk()) {

      for (vi2.origin(); vi2.more(); vi2.next()) {

       Int spw = vb->spectralWindows()(0);

       Int nrow=vb->nRows();
       Int ncor=vb->nCorrelations();

       // Detect ACs
       const Vector<Int> a1(vb->antenna1());
       const Vector<Int> a2(vb->antenna2());
       Vector<Bool> ac(a1==a2);

       // XCs need an extra factor of 2
       Vector<Float> xcfactor(nrow,2.0);
       xcfactor(ac)=1.0;   // (but not ACs)

       // The row-wise integration time
       Vector<Float> expo(nrow);
       convertArray(expo,vb->exposure());

       Matrix<Float> newwt(ncor,nrow),newsig(ncor,nrow);
       newsig.set(1.0);

       // Set weights to channel bandwidth first.
       newwt.set(Float(effChBw(spw)));

       // For each correlation, apply exposure and xcfactor
       for (Int icor=0;icor<ncor;++icor) {
         Vector<Float> wt(newwt.row(icor));
         wt*=expo;
	 wt*=xcfactor;
       }

       // sig from wt is inverse sqrt
       newsig=newsig/sqrt(newwt);

       /*
       cout << ivb << " "
            << ncor << " " << nchan << " " << nrow << " "
            << expo(0) << " "
            << newwt(0,0) << " "
            << newsig(0,0) << " "
            << endl;
       */
       ++ivb;

       vb->setWeight(newwt);
       vb->setSigma(newsig);
       vb->writeChangesBack();

      }
    }
  }
  catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg()
             << LogIO::POST;

    logSink() << "Resetting all calibration application settings." << LogIO::POST;
    unsetapply();

    throw(AipsError("Error in Calibrater::initWeights."));
    retval = False;  // Not that it ever gets here...
  }
  return retval;
}


Bool Calibrater::summarize_uncalspws(const Vector<Bool>& uncalspw,
				     const String& origin,
				     Bool strictflag)
{
  Bool hadprob = false;
  uInt totNspw = uncalspw.nelements();

  for(uInt i = 0; i < totNspw; ++i){
    if(uncalspw[i]){
      hadprob = true;
      break;
    }
  }
  if(hadprob){
    logSink() << LogIO::WARN
	      << "Spectral window(s) ";
    for(uInt i = 0; i < totNspw; ++i){
      if(uncalspw[i]){
	logSink() << i << ", ";
      }
    }
    logSink() << "\n  could not be " << origin << "ed due to missing (pre-)calibration\n"
	      << "    in one or more of the specified tables.\n";
    if (strictflag)
      logSink() << "    These spws have been flagged!";
    else
      logSink() << "    Please check your results carefully!";

    logSink() << LogIO::POST;
  }
  return !hadprob;
}

Bool Calibrater::solve() {

  logSink() << LogOrigin("Calibrater","solve") << LogIO::NORMAL3;

  try {

    if (!ok()) 
      throw(AipsError("Calibrater not prepared for solve."));

    // Handle nothing-to-solve-for case
    if (!svc_p)
      throw(AipsError("Please run setsolve before attempting to solve."));

    // Handle specified caltable
    if (True && svc_p) {

      /*      
      cout << "name: " << svc_p->calTableName() << endl;
      cout << boolalpha;
      cout << "append?   " << svc_p->append() << endl;
      cout << "opened?   " << Table::isOpened(svc_p->calTableName()) << endl;
      cout << "readable? " << Table::isReadable(svc_p->calTableName()) << endl;
      //      cout << "writable? " << Table::isWritable(svc_p->calTableName()) << endl;
      cout << "canDelete? " << Table::canDeleteTable(svc_p->calTableName(),True) << endl;
      */

      // If table exists (readable) and not deletable
      //   we have to abort (append=T requires deletable)
      if ( Table::isReadable(svc_p->calTableName()) &&
	   !Table::canDeleteTable(svc_p->calTableName()) ) {
	throw(AipsError("Specified caltable ("+svc_p->calTableName()+") exists and\n cannot be replaced (or appended to) because it appears to be open somewhere (Quit plotcal?)."));
      }
    }

    // Arrange VisEquation for solve
    ve_p->setsolve(*svc_p);

    // Ensure apply list properly sorted w.r.t. solvable term
    ve_p->setapply(vc_p);

    // Report what is being applied and solved-for
    applystate();
    solvestate();


    // Report correct/corrupt apply order
    //    ve_p->state();

    // Set the channel mask
    svc_p->setChanMask(chanmask_);

    // Generally use standard solver
    if (svc_p->useGenericGatherForSolve())
      genericGatherAndSolve();   // using VisBuffGroupAcc
    else {
      //cout << "Fully self-directed data gather and solve" << endl;
      // Fully self-directed data gather and solve
      svc_p->selfGatherAndSolve(*vs_p,*ve_p);
    }

    svc_p->clearChanMask();

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;

    logSink() << "Reseting entire solve/apply state." << LogIO::POST;
    reset();

    throw(AipsError("Error in Calibrater::solve."));
    return False;
  } 

  return True;

}

Bool Calibrater::genericGatherAndSolve() {

  //cout << "Generic gather and solve." << endl;

  // Create the solver
  VisCalSolver vcs;
  
   // Inform logger/history
  logSink() << "Solving for " << svc_p->typeName()
	    << LogIO::POST;
  
  // Initialize the svc according to current VisSet
  //  (this counts intervals, sizes CalSet)
  Vector<Int> nChunkPerSol;
  Int nSol = svc_p->sizeUpSolve(*vs_p,nChunkPerSol);

  // Create the in-memory (New)CalTable
  svc_p->createMemCalTable();

  // The iterator, VisBuffer
  VisIter& vi(vs_p->iter());
  VisBuffer vb(vi);
  
  Vector<Int> slotidx(vs_p->numberSpw(),-1);

  // We will remember which spws couldn't be processed
  Vector<Bool> unsolspw(vi.numberSpw());	
  unsolspw.set(False);		       

  // Manage verbosity of partial channel averaging
  Vector<Bool> verb(vi.numberSpw(),True);

  Vector<Int64> nexp(vi.numberSpw(),0), natt(vi.numberSpw(),0),nsuc(vi.numberSpw(),0);

  Int nGood(0);
  vi.originChunks();
  for (Int isol=0;isol<nSol && vi.moreChunks();++isol) {

    nexp(vi.spectralWindow())+=1;

    // capture obs, scan info so we can set it later 
    //   (and not rely on what the VB averaging code can't properly do)
    Vector<Int> scv,obsv;
    Int solscan=vi.scan(scv)(0),solobs=vi.observationId(obsv)(0);

    // Arrange to accumulate 
    //    VisBuffAccumulator vba(vs_p->numberAnt(),svc_p->preavg(),False); 
    VisBuffGroupAcc vbga(vs_p->numberAnt(),vs_p->numberSpw(),vs_p->numberFld(),svc_p->preavg()); 
    
    for (Int ichunk=0;ichunk<nChunkPerSol(isol);++ichunk) {
    
      // Current _chunk_'s spw
      Int spw(vi.spectralWindow());
    
      // Only accumulate for solve if we can pre-calibrate
      if (ve_p->spwOK(spw)) {

	// Collapse each timestamp in this chunk according to VisEq
	//  with calibration and averaging
	for (vi.origin(); vi.more(); vi++) {
	  
	  // Force read of the field Id
	  vb.fieldId();

	  // Apply the channel mask (~no-op, if unnecessary)
	  svc_p->applyChanMask(vb);
	  
	  // This forces the data/model/wt I/O, and applies
	  //   any prior calibrations
	  ve_p->collapse(vb);
	  
	  // If permitted/required by solvable component, normalize
	  if (svc_p->normalizable()) 
	    vb.normalize();
	  
	  // If this solve not freqdep, and channels not averaged yet, do so
	  if (!svc_p->freqDepMat() && vb.nChannel()>1)
	    vb.freqAveCubes();

	  if (svc_p->freqDepPar() && 
	      svc_p->fsolint()!="none" &&
	      svc_p->fintervalCh()>0.0) {
	    //	    cout << "svc_p->currSpw() = " << svc_p->currSpw() << endl;
	    if (verb(spw)) 
	      logSink() << " Reducing nchan in spw " 
			<< spw
			<< " from " << vb.nChannel();
	    vb.channelAve(svc_p->chanAveBounds(spw));

	    // Kludge for 3.4 to reset corr-indep flag to correct channel axis shape
	    // (because we use vb.flag() below, rather than vb.flagCube())
	    vb.flag().assign(operator>(partialNTrue(vb.flagCube(),IPosition(1,0)),uInt(0)));

	    if (verb(spw)) {
	      logSink() << " to " 
			<< vb.nChannel() << LogIO::POST;
	      verb(spw)=False;  // suppress future verbosity in this spw
	    }
	  }
	  
	  // Accumulate collapsed vb in a time average
	  //  (only if the vb contains any unflagged data)
	  if (nfalse(vb.flag())>0)
	    vbga.accumulate(vb);
	  
	}
      }
      else
	// This spw not accumulated for solve
	unsolspw(spw)=True;

      // Advance the VisIter, if possible
      if (vi.moreChunks()) vi.nextChunk();

    }
    
    // Finalize the averged VisBuffer
    vbga.finalizeAverage();

    // Establish meta-data for this interval
    //  (some of this may be used _during_ solve)
    //  (this sets currSpw() in the SVC)
    Bool vbOk=(vbga.nBuf()>0 && svc_p->syncSolveMeta(vbga));

    svc_p->overrideObsScan(solobs,solscan);

    if (vbOk) {

      // Use spw of first VB in vbga
      // TBD: (currSpw==thisSpw) here??  (I.e., use svc_p->currSpw()?  currSpw is prot!)
      Int thisSpw=svc_p->spwMap()(vbga(0).spectralWindow());
    
      natt(thisSpw)+=1;

      slotidx(thisSpw)++;
      
      // Make data amp- or phase-only, if needed
      vbga.enforceAPonData(svc_p->apmode());
      
      // Select on correlation via weights, according to the svc
      vbga.enforceSolveCorrWeights(svc_p->phandonly());

      if (svc_p->useGenericSolveOne()) {
	// generic individual solve

	//cout << "Generic individual solve: isol=" << isol << endl;

	// First guess
	svc_p->guessPar(vbga(0));
	
	// Solve for each parameter channel (in curr Spw)
	
	// (NB: force const version of nChanPar()  [why?])
	//	for (Int ich=0;ich<((const SolvableVisCal*)svc_p)->nChanPar();++ich) {
	Bool totalGoodSol(False);
	for (Int ich=((const SolvableVisCal*)svc_p)->nChanPar()-1;ich>-1;--ich) {
	  // for (Int ich=0;ich<((const SolvableVisCal*)svc_p)->nChanPar();++ich) {
	  
	  // If pars chan-dep, SVC mechanisms for only one channel at a time
	  svc_p->markTimer();
	  svc_p->focusChan()=ich;
	  
	  //	  svc_p->state();
	  
	  //	  cout << "Starting solution..." << endl;

	  // Pass VE, SVC, VB to solver
	  Bool goodSoln=vcs.solve(*ve_p,*svc_p,vbga);
	  
	  //	  cout << "goodSoln= " << boolalpha << goodSoln << endl;

	  // If good... 
	  if (goodSoln) {
	    totalGoodSol=True;
	    
	    svc_p->formSolveSNR();
	    svc_p->applySNRThreshold();
	    
	    // ..and file this solution in the correct slot
	    if (svc_p->freqDepPar())
	      svc_p->keep1(ich);
	    //	    svc_p->keep(slotidx(thisSpw));
	    //      Int n=svc_p->nSlots(thisSpw);
	    //	    svc_p->printActivity(n,slotidx(thisSpw),vi.fieldId(),thisSpw,nGood);	      
	    
	  }
	  else {
	    // report where this failure occured
	    svc_p->currMetaNote();
	    if (svc_p->freqDepPar())
	      // We must record a flagged solution for this channel
	      svc_p->keep1(ich);
	  }
	  
	} // parameter channels

	if (totalGoodSol) {
	  svc_p->keepNCT();
	  nsuc(thisSpw)+=1;
	}
	
	// Count good solutions.
	if (totalGoodSol)	nGood++;
	
      }
      else {
	//cout << "Self-directed individual solve: isol=" << isol << endl;
	// self-directed individual solve
	// TBD: selfSolveOne should return T/F for "good"
	svc_p->selfSolveOne(vbga);

	// File this solution in the correct slot of the CalSet
	svc_p->keepNCT();

	nGood++;
	nsuc(thisSpw)+=1;
      } 
	
    } // vbOK

  } // isol

  logSink() << "  Found good " 
	    << svc_p->typeName() << " solutions in "
	    << nGood << " slots."
	    << LogIO::POST;

  summarize_uncalspws(unsolspw, "solv");
  
  // Fill activity record
  actRec_=Record();
  actRec_.define("origin","Calibrater::genericGatherAndSolve");
  actRec_.define("nExpected",nexp);
  actRec_.define("nAttempt",natt);
  actRec_.define("nSucceed",nsuc);

  // Store whole of result in a caltable
  if (nGood==0) {
    logSink() << "No output calibration table written."
	      << LogIO::POST;
  }
  else {

    // TBD: Remove BPOLY specificity here
    if (svc_p->typeName()!="BPOLY") {
      // Do global post-solve tinkering (e.g., phase-only, normalization, etc.)
      svc_p->globalPostSolveTinker();

      // write the table
      svc_p->storeNCT();

    }
  }

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
			   const String& refFields, 
			   const Vector<Int>& refSpwMap, 
			   const String& tranFields,
			   const Bool& append,
                           const Float& inGainThres,
                           const String& antSel,
                           const String& timerangeSel,
                           const String& scanSel,
			   SolvableVisCal::fluxScaleStruct& oFluxScaleFactor,
			   Vector<Int>& tranidx,
			   const String& oListFile,
                           const Bool& incremental,
                           const Int& fitorder,
                           const Bool& display) {

  // TBD:  Permit more flexible matching on specified field names
  //  (Currently, exact matches are required.)

  logSink() << LogOrigin("Calibrater","fluxscale") << LogIO::NORMAL3;

  // Convert refFields/transFields to index lists
  Vector<Int> refidx(0);

  if (refFields.length()>0)
    refidx=getFieldIdx(refFields);
  else
    throw(AipsError("A reference field must be specified!"));

  if (tranFields.length()>0)
    tranidx=getFieldIdx(tranFields);

  // Call Vector<Int> version:
  fluxscale(infile,outfile,refidx,refSpwMap,tranidx,append,inGainThres,antSel,timerangeSel,
      scanSel,oFluxScaleFactor,oListFile,incremental,fitorder,display);

}

void Calibrater::fluxscale(const String& infile, 
			   const String& outfile,
			   const Vector<Int>& refField, 
			   const Vector<Int>& refSpwMap, 
			   const Vector<Int>& tranField,
			   const Bool& append,
                           const Float& inGainThres,
                           const String& antSel,
                           const String& timerangeSel,
                           const String& scanSel,
			   SolvableVisCal::fluxScaleStruct& oFluxScaleFactor,
			   const String& oListFile,
                           const Bool& incremental,
                           const Int& fitorder,
                           const Bool& display) {

  //  throw(AipsError("Method 'fluxscale' is temporarily disabled."));

  // TBD: write inputs to MSHistory
  logSink() << LogOrigin("Calibrater","fluxscale") << LogIO::NORMAL3;

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

      //Bool incremental=False;
      // Make fluxscale calculation
      Vector<String> fldnames(ROMSFieldColumns(ms_p->field()).name().getColumn());
      //fsvj_->fluxscale(refField,tranField,refSpwMap,fldnames,oFluxScaleFactor,
      fsvj_->fluxscale(outfile,refField,tranField,refSpwMap,fldnames,inGainThres,antSel,
        timerangeSel,scanSel,oFluxScaleFactor, oListFile,incremental,fitorder,display);
//        oListFile);
     
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
      fsvj_->storeNCT(out,append);
      
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
			    const String& fields,
			    const String& calFields,
			    const String& interp,
			    const Double& t,
			    const Vector<Int>& spwmap) {
  
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
      selectvis();
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
    

    /*
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
    */
    
    // fill incr table with selection
    logSink() << "Preparing to accumulate calibration from table: "
	      << incrtab
	      << LogIO::POST;

    // Set record format for calibration table application information
    RecordDesc applyparDesc;
    applyparDesc.addField ("t", TpDouble);
    applyparDesc.addField ("table", TpString);
    //    applyparDesc.addField ("select", TpString);
    applyparDesc.addField ("fieldstr", TpString);
    applyparDesc.addField ("interp", TpString);
    applyparDesc.addField ("spwmap",TpArrayInt);
    
    // Create record with the requisite field values
    Record applypar(applyparDesc);
    applypar.define ("t", t);
    applypar.define ("table", incrtab);
    //    applypar.define ("select", incrSel);
    applypar.define ("fieldstr", calFields);
    applypar.define ("interp", interp);
    applypar.define ("spwmap",spwmap);

    incrcal_->setApply(applypar);

    Vector<Int> fldidx(0);
    if (fields.length()>0)
      fldidx=getFieldIdx(fields);

    // All ready, now do the accumulation
    incal_->accumulate(incrcal_,fldidx);
    
    // ...and store the result
    logSink() << "Storing accumulated calibration in table: " 
	      << outtab
	      << LogIO::POST;

    if (outtab != "") 
      incal_->calTableName()=outtab;

    incal_->storeNCT();
    
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

void Calibrater::specifycal(const String& type,
			    const String& caltable,
			    const String& time,
			    const String& spw,
			    const String& antenna,
			    const String& pol,
			    const Vector<Double>& parameter) {

  logSink() << LogOrigin("Calibrater","specifycal") << LogIO::NORMAL;

  // SVJ objects:
  SolvableVisCal *cal_(NULL);

  try {
 			    
    // Set record format for calibration table application information
    RecordDesc specifyDesc;
    specifyDesc.addField ("caltable", TpString);
    specifyDesc.addField ("time", TpString);
    specifyDesc.addField ("spw", TpArrayInt);
    specifyDesc.addField ("antenna", TpArrayInt);
    specifyDesc.addField ("pol", TpString);
    specifyDesc.addField ("parameter", TpArrayDouble);
    specifyDesc.addField ("caltype",TpString);

    // Create record with the requisite field values
    Record specify(specifyDesc);
    specify.define ("caltable", caltable);
    specify.define ("time", time);
    if (spw=="*")
      specify.define ("spw",Vector<Int>(1,-1));
    else
      specify.define ("spw",getSpwIdx(spw));
    if (antenna=="*")
      specify.define ("antenna",Vector<Int>(1,-1) );
    else
      specify.define ("antenna",getAntIdx(antenna));
    specify.define ("pol",pol);
    specify.define ("parameter",parameter);
    specify.define ("caltype",type);

    // Now do it
    String utype=upcase(type);
    if (utype=="G" || utype.contains("AMP") || utype.contains("PH"))
      cal_ = createSolvableVisCal("G",*vs_p);
    else if (utype=='K' || utype.contains("SBD") || utype.contains("DELAY"))
      cal_ = createSolvableVisCal("K",*vs_p);
    else if (utype.contains("MBD"))
      cal_ = createSolvableVisCal("KMBD",*vs_p);
    else if (utype.contains("ANTPOS"))
      cal_ = createSolvableVisCal("KANTPOS",*vs_p);
    else if (utype.contains("TSYS"))
      cal_ = createSolvableVisCal("TSYS",*vs_p);
    else if (utype.contains("EVLAGAIN") ||
	     utype.contains("SWP") ||
	     utype.contains("RQ"))
      cal_ = createSolvableVisCal("EVLASWP",*vs_p);
    else if (utype.contains("OPAC"))
      cal_ = createSolvableVisCal("TOPAC",*vs_p);
    else if (utype.contains("GC") || utype.contains("EFF"))
      cal_ = createSolvableVisCal("GAINCURVE",*vs_p);
    else if (utype.contains("TEC"))
      cal_ = createSolvableVisCal("TEC",*vs_p);
    else
      throw(AipsError("Unrecognized caltype."));

    // set up for specification (set up the CalSet)
    cal_->setSpecify(specify);

    // fill with specified values
    cal_->specify(specify);

    // Store result
    cal_->storeNCT();

    delete cal_;

  } catch (AipsError x) {
    logSink() << LogIO::SEVERE
	      << "Caught Exception: "
	      << x.getMesg()
	      << LogIO::POST;

    if (cal_) delete cal_;
    
    throw(AipsError("Error in Calibrater::specifycal."));
    return;
  }
  return;

}


Bool Calibrater::smooth(const String& infile, 
			String& outfile,  // const Bool& append,
                        const String& smoothtype,
			const Double& smoothtime,
                        const String& fields)
{

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
      if (fields.length()>0)
	fldidx=getFieldIdx(fields);

      // Delegate to SVC
      svc->smooth(fldidx,smoothtype,smoothtime);
      
      // Store the result on disk
      //    if (append) logSink() << "Appending result to " << outfile << LogIO::POST;
      //else 
      logSink() << "Storing result in " << outfile << LogIO::POST;
      
      
      if (outfile != "") 
	svc->calTableName()=outfile;
      svc->storeNCT();

      // Clean up
      if (svc) delete svc; svc=NULL;
      
      // Apparently, it worked
      return True;

    }
    else
      throw(AipsError("This type ("+svc->typeName()+") does not support smoothing."));

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


  // List a calibration table
Bool Calibrater::listCal(const String& infile,
			 const String& field,
			 const String& antenna,
			 const String& spw,
			 const String& listfile,
             const Int& pagerows) {
    
    SolvableVisCal *svc(NULL);
    logSink() << LogOrigin("Calibrater","listCal");
    
    try {
        
        // Trap (currently) unsupported types
        if (upcase(calTableType(infile))=="GSPLINE" ||
            upcase(calTableType(infile))=="BPOLY")
            throw(AipsError("GSPLINE and BPOLY tables cannot currently be listed."));
        
        // Get user's selected fields, ants
        Vector<Int> ufldids=getFieldIdx(field);
        Vector<Int> uantids=getAntIdx(antenna);
        
        String newSpw = spw;
        Bool defaultSelect = False;
        if (spw.empty()) { // list all channels (default)
            defaultSelect = True;
            newSpw = "*"; 
            logSink() << LogIO::NORMAL1 << "Spws selected: ALL" << endl
                      << "Channels selected: ALL" << LogIO::POST;
        }
        // Get user's selected spw and channels
        Vector<Int> uspwids=getSpwIdx(newSpw);
        Matrix<Int> uchanids=getChanIdx(newSpw);
        if (!defaultSelect) {
            logSink() << LogIO::NORMAL1 << "Spw and Channel selection matrix: "
                      << endl << "Each rows shows: [ Spw , Start Chan , Stop Chan , Chan Step ]"
                      << endl << uchanids << LogIO::POST;
        }
        logSink() << LogIO::DEBUG2 
                  << "uspwids = "  << uspwids  << endl
                  << "uchanids = " << uchanids << LogIO::POST;
        
        // By default, do first spw, first chan
        if (uspwids.nelements()==0) {
            uchanids.resize(1,4);
            uchanids=0;
        } 
        
        // Set record format for calibration table application information
        RecordDesc applyparDesc;
        applyparDesc.addField ("table", TpString);
        
        // Create record with the requisite field values
        Record applypar(applyparDesc);
        applypar.define ("table", infile);
        
        // Generate the VisCal to be listed
        svc = createSolvableVisCal(calTableType(infile),*vs_p);  
        svc->setApply(applypar);       
        
        // list it
        svc->listCal(ufldids,uantids,uchanids,  //uchanids(0,0),uchanids(0,1),
                     listfile,pagerows);
        
        if (svc) delete svc; svc=NULL;
        
        return True;
        
    } catch (AipsError x) {
        
        logSink() << LogIO::SEVERE
                  << "Caught Exception: "
                  << x.getMesg()
                  << LogIO::POST;
        // Clean up
        if (svc) delete svc; svc=NULL;
        
        throw(AipsError("Error in Calibrater::listCal."));
        
        return False;
    }
    return False;
    
}

Bool Calibrater::updateCalTable(const String& caltable) {

  // Call the SVC method that knows how
  return NewCalTable::CTBackCompat(caltable);

}



void Calibrater::selectChannel(const String& spw) {

  // Initialize the chanmask_
  initChanMask();

  Matrix<Int> chansel = getChanIdx(spw);
  uInt nselspw=chansel.nrow();

  if (nselspw==0)
    logSink() << "Frequency selection: Selecting all channels in all spws." 
	      << LogIO::POST;
  else {

    logSink() << "Frequency selection: " << LogIO::POST;

    // Trap non-unit step (for now)
    if (ntrue(chansel.column(3)==1)!=nselspw) {
      logSink() << LogIO::WARN
		<< "Calibration does not support non-unit channel stepping; "
		<< "using step=1."
		<< LogIO::POST;
      chansel.column(3)=1;
    }

    Int nspw=vs_p->numberSpw();
    Vector<Int> nChan0;
    nChan0 = vs_p->numberChan();

    Vector<Int> uspw(chansel.column(0));
    Vector<Int> ustart(chansel.column(1));
    Vector<Int> uend(chansel.column(2));

    Vector<Int> start(nspw,INT_MAX);
    Vector<Int> end(nspw,-INT_MAX);
    logSink() << LogIO::NORMAL;
    for (uInt i=0;i<nselspw;++i) {
      
      Int& spw=uspw(i);

      // Initialize this spw mask, if necessary (def = masked)
      if (!chanmask_[spw])
      	chanmask_[spw]=new Vector<Bool>(nChan0(spw),True);

      // revise net start/end/nchan
      start(spw)=min(start(spw),ustart(i));
      end(spw)=max(end(spw),uend(i));
      Int nchan=end(spw)-start(spw)+1;  // net inclusive nchan

      // User's 
      Int step=chansel(i,3);
      Int unchan=uend(i)-ustart(i)+1;
      
      // Update the mask (False = valid)
      (*chanmask_[spw])(Slice(ustart(i),unchan))=False;


      logSink() << ".  Spw " << spw << ":"
		<< ustart(i) << "~" << uend(i) 
		<< " (" << uend(i)-ustart(i)+1 << " channels,"
		<< " step by " << step << ")"
		<< endl;

  /*
      cout << i << " " << spw << " {" 
	   << start(spw) << " [" << ustart(i) << " " 
	   << uend(i) << "] " << end(spw) << "}" << endl;
      cout << "chanmask = ";
      for (Int j=0;j<nChan0(spw);++j) cout << (*chanmask_[spw])(j);
      cout << endl << endl;
  */
	
      // Call via VisSet (avoid call to VisIter::origin)
      vs_p->selectChannel(1,start(spw),nchan,step,spw,False);
	
    } // i
    logSink() << LogIO::POST;

  } // non-triv spw selection

  // For testing:
  if (False) {

    VisIter& vi(vs_p->iter());
    VisBuffer vb(vi);
    
    // Pass each timestamp (VisBuffer) to VisEquation for correction
    for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
      vi.origin();
      //      for (vi.origin(); vi.more(); vi++)
	cout << vb.spectralWindow() << " "
	     << vb.nChannel() << " "
	     << vb.channel() << " "
	     << vb.visCube().shape()
	     << endl;
    }
  }

}

void Calibrater::initChanMask() {

  for (uInt i=0;i<chanmask_.nelements();++i) 
    if (chanmask_[i])
      delete chanmask_[i];
  if (vs_p) {
    chanmask_.resize(vs_p->numberSpw(),True);
    chanmask_=NULL;
  }
  else {
    //    throw(AipsError("Trouble sizing chanmask!"));
    // just don't support channel masking:
    chanmask_.resize(0,True);
  }

}

// Select on channel in the VisSet
void Calibrater::selectChannel(const String& mode, 
			       const Int& nchan, 
			       const Int& start, const Int& step,
			       const MRadialVelocity& mStart,
			       const MRadialVelocity& mStep) {
  
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
    for (uInt k=0; k < nSpw; ++k) {
      selectedSpw[k]=dataDescID[index[uniq[k]]];
      nChan[k]=vs_p->numberChan()(selectedSpw[k]);
      
    }
    if(dataNchan_p==0) dataNchan_p=vs_p->numberChan()(selectedSpw[0]);
    if(dataStart_p<0) {
      logSink() << LogIO::SEVERE << "Illegal start pixel = " 
		<< dataStart_p << LogIO::POST;
    }
    Int end = Int(dataStart_p) + Int(dataNchan_p) * Int(dataStep_p);
    for (uInt k=0; k < selectedSpw.nelements() ; ++k){
      if(end < 1 || end > nChan[k]) {
	logSink() << LogIO::SEVERE << "Illegal step pixel = " << dataStep_p
		  << " in Spw " << selectedSpw[k]
		  << LogIO::POST;
      }
      logSink() << "Selecting "<< dataNchan_p
		<< " channels, starting at visibility channel "
		<< dataStart_p  << " stepped by "
		<< dataStep_p << " in Spw " << selectedSpw[k] << LogIO::POST;
      
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
  
  
}

// Parse refant specification
Vector<Int> Calibrater::getRefantIdxList(const String& refant) {

  Vector<Int> irefant;
  if (refant.length()==0) {
    // Nothing specified, return -1 in single-element vector
    irefant.resize(1);
    irefant(0)=-1;
  }
  else {
    // parse the specification
    MSSelection msselect;
    msselect.setAntennaExpr(refant);
    Vector<Int> iref=msselect.getAntenna1List(mssel_p);
    if (anyLT(iref,0))
      cout << "Negated selection (via '!') not yet implemented for refant," << endl << " and will be ignored." << endl;
    irefant=iref(iref>-1).getCompressedArray();
    if (irefant.nelements()==0) {
      irefant.resize(1);
      irefant(0)=-1;
    }
  }

  return irefant;
}


// Interpret refant *index*
Vector<Int> Calibrater::getAntIdx(const String& antenna) {

    MSSelection msselect;
    msselect.setAntennaExpr(antenna);
    return msselect.getAntenna1List(mssel_p);

}

// Interpret field indices (MSSelection)
Vector<Int> Calibrater::getFieldIdx(const String& fields) {

  MSSelection mssel;
  mssel.setFieldExpr(fields);
  return mssel.getFieldList(mssel_p);

}

// Interpret spw indices (MSSelection)
Vector<Int> Calibrater::getSpwIdx(const String& spws) {

  MSSelection mssel;
  mssel.setSpwExpr(spws);

  /*
  cout << "mssel.getSpwList(mssel_p) = " << mssel.getSpwList(mssel_p) << endl;
  cout << "mssel.getChanList(mssel_p) = " << mssel.getChanList(mssel_p) << endl;  cout << "Vector<Int>() = " << Vector<Int>() << endl;
  */

  // Use getChanList (column 0 for spw) because it is 
  //  more reliable about the number and order of specified spws.
  //  return mssel.getSpwList(mssel_p);
  Matrix<Int> chanmat=mssel.getChanList(mssel_p);
  if (chanmat.nelements()>0) 
    return chanmat.column(0);
  else
    return Vector<Int>();

}

Matrix<Int> Calibrater::getChanIdx(const String& spw) {

  MSSelection mssel;
  mssel.setSpwExpr(spw);

  return mssel.getChanList(mssel_p);

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

CorrectorVp *
Calibrater::getCorrectorVp ()
{
    return new CorrectorVp (this);
}

const String CorrectorVp::In = "In";
const String CorrectorVp::Out = "Out";


CorrectorVp::CorrectorVp (Calibrater * calibrater, const String & name)
: VisibilityProcessor (name, vector<String> (1, In), vector<String> (1, Out)),
  calibrater_p (calibrater)
{}


void
CorrectorVp::chunkStartImpl (const vpf::SubchunkIndex &)
{
}

vpf::VisibilityProcessor::ProcessingResult
CorrectorVp::doProcessingImpl (ProcessingType processingType,
                               VpData & inputData,
                               const SubchunkIndex & /*subchunkIndex*/)
{
    VpPort inputPort = getInputs () [0];
    VbPtr inputVbPtr = inputData [inputPort];

    ProcessingResult result;

    if (processingType == EndOfData){
        calibrater_p->vs_p->flush(); // flush to disk
    }
    else if (processingType != Subchunk){
        // do nothing
    }
    else if (!calibrater_p->ve_p->spwOK (inputVbPtr->spectralWindow())){
        uncalibratedSpectralWindows_p [inputVbPtr->spectralWindow()] = True;
    }
    else {

        // If the VB's spectral window can be calibrated, then do so.

        inputVbPtr->resetWeightMat();

        calibrater_p->ve_p->correct(* inputVbPtr);    // throws exception if nothing to apply

        // Mark the modified columns in the VB as dirty.  The "correct" method makes its
        // modifications on the observed component of the data.

        if (whichOutputColumn_p == VisibilityIterator::Corrected){

            // Mark the corrected cube as modified and copy the results from the
            // observed cube to the corrected cube component of the VB.

            inputVbPtr->dirtyComponentsAdd (VisBufferComponents::CorrectedCube);
            inputVbPtr->correctedVisCube() = inputVbPtr->visCube();
//VisBuffer * vb = inputVbPtr.get();
//Cube<Complex> & cube = vb->correctedVisCube();
//Int d1, d2, d3;
//cube.shape (d1, d2, d3);
//cout << "CorrectedVisCube [" << d1 << "," << d2 << "," << d3 << "] {vpf}" << endl;
//for (int i = 0; i < min(d1, 10); i++){
//    for (int j = 0; j < min (d2, 10); j++){
//        for (int k = 0; k < min (d3, 10); k++){
//            cout << "[" << i << "," << j << "," << k << "] = " << cube(i,j,k) << endl;
//        }
//    }
//}
        }
        else{

            // The modifications can be left in place.  Mark the observed cube as
            // dirty.  N.B., this will overwrite the actual data!

            inputVbPtr->dirtyComponentsAdd (VisBufferComponents::ObservedCube);
        }

        inputVbPtr->dirtyComponentsAdd (VisBufferComponents::Flag);

        // Write out weight col, if it has changed

	inputVbPtr->dirtyComponentsAdd (VisBufferComponents::WeightMat);

        VpData output;
        VpPort outputPort = getOutputs () [0];

        output [outputPort] = inputVbPtr;

        result = ProcessingResult (Normal, output);
    }

    return result;
}

ROVisibilityIterator *
CorrectorVp::getVisibilityIterator ()
{
    if (!calibrater_p->ok())
      throw(AipsError("Calibrater not prepared for correct!"));

    // Ensure apply list non-zero and properly sorted

    calibrater_p->ve_p->setapply(calibrater_p->vc_p);

    // Report the types that will be applied

    calibrater_p->applystate();

    // Arrange for iteration over data

    Block<Int> columns;
    columns.resize(5);

    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER; // include scan iteration
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;

    calibrater_p->vs_p->resetVisIter(columns,0.0);

    return & (calibrater_p->vs_p->iter());
}

void
CorrectorVp::processingStartImpl ()
{
    calculateWeights_p = calibrater_p->calWt();
    uncalibratedSpectralWindows_p.resize ();
    uncalibratedSpectralWindows_p.set (False);
    whichOutputColumn_p = VisibilityIterator::Corrected;
}

void
CorrectorVp::validateImpl ()
{
    throwIfAnyPortsUnconnected ();
}


} //# NAMESPACE CASA - END
