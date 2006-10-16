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
//# $Id$

#include <tables/Tables/Table.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableParse.h>

#include <casa/Arrays/ArrayUtil.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <synthesis/MeasurementComponents/Calibrater.h>
#include <synthesis/MeasurementComponents/UVMod.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

#include <tables/Tables/SetupNewTab.h>


namespace casa { //# NAMESPACE CASA - BEGIN

Calibrater::Calibrater(): ms_p(0), mssel_p(0), vs_p(0), ve_p(0), bj_p(0), 
			  gj_p(0), dj_p(0), cj_p(0), ej_p(0), pj_p(0),
			  tj_p(0), mj_p(0), km_p(NULL), mm_p(NULL), mfm_p(NULL),
			  svj_p(0), delsvj_(False),
			  svc_p(0), delsvc_(False),
			  histLockCounter_p(), hist_p(0)
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
  bj_p=other.bj_p;
  gj_p=other.gj_p;
  dj_p=other.dj_p;
  pj_p=other.pj_p;
  ej_p=other.ej_p;
  cj_p=other.cj_p;
  tj_p=other.tj_p;
  mj_p=other.mj_p;
  km_p=other.km_p;
  mm_p=other.mm_p;
  mfm_p=other.mfm_p;
  histLockCounter_p=other.histLockCounter_p;
  hist_p=other.hist_p;
  historytab_p=other.historytab_p;
  
  return *this;
}

Calibrater::~Calibrater()
{
  reset();
  unsetSolve();
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
  
  logSink() << LogOrigin("Calibrater","initialize") << LogIO::NORMAL;
  
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

    Bool needWeights=(!ms_p->tableDesc().isColumn("CORRECTED_DATA"));

    msname_p=ms_p->tableName();

    // Set the selected MeasurementSet to be the same initially
    // as the input MeasurementSet
    mssel_p=new MeasurementSet(*ms_p);

    // Delete the VisSet and VisEquation if they already exist
    if (vs_p) {
      delete vs_p;
      vs_p=0;
    };
    if (ve_p) {
      delete ve_p;
      ve_p=0;
    };

    // Create a VisSet with no selection 
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
    ve_p=new VisEquation(*vs_p);

    return True;

  } catch (AipsError x) {
    reset();
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() << 
      LogIO::POST;
    return False;
  } 
  return False;
}

void Calibrater::setdata(const String& mode, const Int& nchan,
			 const Int& start, const Int& step,
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
    dataStart_p=start;
    dataStep_p=step;
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
    };

    if (nullSelect) {
      if (mssel_p) {
	delete mssel_p; 
	mssel_p=0;
      };
      logSink() << LogIO::WARN
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

    // Re-create the associated VisEquation
    if (ve_p) {
      delete ve_p;
      ve_p=0;
    };
    ve_p=new VisEquation(*vs_p);
    AlwaysAssert(ve_p,AipsError);
    
  } catch (AipsError x) {
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg()
       << LogIO::POST;

  } 
};

Bool Calibrater::write() {
  
  logSink() << LogOrigin("Calibrater","write") << LogIO::NORMAL;
  
  try {
  vs_p->flush();
  return True;
  } catch (AipsError x) {
    reset();
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return False;
}

Bool Calibrater::setApply (const String& typeComp, const Record& interpolation,
			   Vector<Int> rawspw)
{
  logSink() << LogOrigin("Calibrater", "setApply") << LogIO::NORMAL;
  
  try {
    if(!ok()) return False;
    String upcaseType = typeComp;
    upcaseType.upcase();

    // Extract default parameters from the interpolation record
    String tableName = "";
    if (interpolation.isDefined("table")) {
      tableName = interpolation.asString("table");
    };
    String select = "";
    if (interpolation.isDefined("select")) {
      select = interpolation.asString("select");
    };
    Float interval = 0;
    if (interpolation.isDefined("t")) {
      interval = interpolation.asFloat("t");
    };

    // Default iteration parameters
    Int nit=100;
    Float tol=0.00001;
    Float g=0.5;

    // Handle non-solvable, pre-computed components [C,P]
    if(upcaseType=="C") {
      logSink() << "Initializing non-solvable configuration (C-matrix)" 
		<< LogIO::POST;
      if(cj_p) delete cj_p; cj_p=0;
      cj_p = new CJones(*vs_p);
      cj_p->setInterpolation(interpolation);
      ve_p->setVisJones(*cj_p);
      return True;
    };
  
    if(upcaseType=="P") {
      logSink() << "Initializing non-solvable parallactic angle (P-matrix)" 
		<< LogIO::POST;
      if(pj_p) delete pj_p; pj_p=0;
      pj_p = new PJones(*vs_p);
      pj_p->setInterpolation(interpolation);
      ve_p->setVisJones(*pj_p);
      return True;
    };

    if(upcaseType=="TOPAC") {
      logSink() << "Initializing Opacity corrections (TOpac-matrix)"
		<< LogIO::POST;
      if(tj_p) delete tj_p; tj_p=0;
      tj_p = new TOpac(*vs_p);
      tj_p->setInterpolation(interpolation);
      ve_p->setVisJones(*tj_p);
      return True;
    };
    
    if(upcaseType=="GAINCURVE") {
      logSink() << "Initializing Gain Curve corrections (EVisJones-matrix)"
		<< LogIO::POST;
      if(ej_p) delete ej_p; ej_p=0;
      ej_p = new EVisJones(*vs_p);
      ej_p->setInterpolation(interpolation);
      ve_p->setVisJones(*ej_p);
      return True;
    };

    // Handle components to be interpolated from a calibration table.

    if(Table::isReadable(tableName)) {
      logSink() << "Initializing " << typeComp << " table from file: " << tableName
		<< LogIO::POST;

      if(upcaseType=="T") {
	if(tj_p) delete tj_p; tj_p=0;
	tj_p=new TJones(*vs_p);
	tj_p->setInterpolation(interpolation);
	ve_p->setVisJones(*tj_p);
	tj_p->setGain(g);
	tj_p->setNumberIterations(nit);
	tj_p->setTolerance(tol);   
	return True;
      }

      if(upcaseType=="B") {
	if(bj_p) delete bj_p; bj_p=0;
	bj_p=new BJones(*vs_p);
	bj_p->setInterpolation(interpolation);
	ve_p->setVisJones(*bj_p);
	bj_p->setGain(g);
	bj_p->setNumberIterations(nit);
	bj_p->setTolerance(tol);   
	return True;
      }

      if(upcaseType=="BPOLY") {
	if(bj_p) delete bj_p; bj_p=0;
	bj_p=new BJonesPoly(*vs_p);
	bj_p->setInterpolation(interpolation);
	ve_p->setVisJones(*bj_p);
	bj_p->setGain(g);
	bj_p->setNumberIterations(nit);
	bj_p->setTolerance(tol);   
	return True;
      }

      if(upcaseType=="G") {
	if(gj_p) delete gj_p; gj_p=0;
	gj_p=new GJones(*vs_p);
	gj_p->setInterpolation(interpolation);
	ve_p->setVisJones(*gj_p);
	gj_p->setGain(g);
	gj_p->setNumberIterations(nit);
	gj_p->setTolerance(tol);   
	return True;
      }

  /*
      if(upcaseType=="GPOLY") {
	if(gj_p) delete gj_p; gj_p=0;
	gj_p=new GJones(*vs_p, tableName, select);
	ve_p->setVisJones(*gj_p);
	gj_p->setGain(g);
	gj_p->setNumberIterations(nit);
	gj_p->setTolerance(tol);   
	return True;
      }
  */
      if(upcaseType=="GSPLINE") {
	// Don't delete gj_p, because maybe it has already been 
	//  instatiated in setapply (i.e., phase part)
	if (!gj_p) {
	  gj_p=new GJonesSpline(*vs_p);
	};  
	if(rawspw.nelements() > 0) {
	  if(rawspw[0] >= 0){
	    getRawPhaseVisSet(rawspw);
	    ((GJonesSpline *)gj_p)->setRawPhaseVisSet(*rawvs_p);
	  }
	}
	gj_p->setInterpolation(interpolation);
	ve_p->setVisJones(*gj_p);
	gj_p->setGain(g);
	gj_p->setNumberIterations(nit);
	gj_p->setTolerance(tol);   
	return True;
      }

      if(upcaseType=="D") {
	if(dj_p) delete dj_p; dj_p=0;
	dj_p=new DJones(*vs_p);
	dj_p->setInterpolation(interpolation);
	ve_p->setVisJones(*dj_p);
	dj_p->setGain(g);
	dj_p->setNumberIterations(nit);
	dj_p->setTolerance(tol);   
	return True;
      }

      if(upcaseType=="K") {
	if(km_p) delete km_p; km_p=NULL;
	km_p=new KMueller(*vs_p);  
	km_p->setApply(interpolation);
	ve_p->setVisCal(*km_p);
	return True;
      }

      if(upcaseType=="M") {
	if(mm_p) delete mm_p; mm_p=NULL;
	mm_p=new MMueller(*vs_p);  
	mm_p->setApply(interpolation);
	ve_p->setVisCal(*mm_p);
	return True;
      }

      if(upcaseType=="MF") {
	if(mfm_p) delete mfm_p; mfm_p=NULL;
	mfm_p=new MfMueller(*vs_p);  
	mfm_p->setApply(interpolation);
	ve_p->setVisCal(*mfm_p);
	return True;
      }

      {
      logSink() << "Error: unknown type " <<typeComp << LogIO::POST;
      return False;
      }
    }
    else {
      logSink() << LogIO::SEVERE << "Error: Cannot read file "
		<< tableName << LogIO::POST;
      return False;
    };
    return True;

  } catch (AipsError x) {
    reset();
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    return False;
  } 
  return False;
}



Bool Calibrater::setSolve (const String& typeComp, const Record& solver)
{
  logSink() << LogOrigin("Calibrater","setSolve") << LogIO::NORMAL;
  
  try {

    // Default iteration parameters
    Int nit=100;
    Float tol=0.00001;
    Float g=0.5;

    if(!ok()) return False;

    String upcaseType = typeComp;
    upcaseType.upcase();

    // Clean out old solve
    unsetSolve();

    logSink() << "Arranging to solve for ";

    if(upcaseType=="B") {
      logSink() << "bandpass (B-matrix)" << LogIO::POST;
      svj_p = new BJones(*vs_p);
      delsvj_=True;
    };
    if(upcaseType=="G") {
      logSink() << "electronic gain terms (G-matrix)" << LogIO::POST;
      svj_p = new GJones(*vs_p);
      delsvj_=True;
    };
    if(upcaseType=="D") {
      logSink() << "instrumental polarization leakage (D-matrix)" << LogIO::POST;
      svj_p = new DJones(*vs_p);
      delsvj_=True;
    };
    if(upcaseType=="T") {
      logSink() << "atmospheric gain/transmission (T-matrix)" << LogIO::POST;
      svj_p = new TJones(*vs_p);
      delsvj_=True;
    };
    
    if(upcaseType=="BPOLY") {
      logSink() << "bandpass polynomial (B-matrix)" << LogIO::POST;
      svj_p = new BJonesPoly(*vs_p);
      delsvj_=True;
    };
    
    if(upcaseType=="GSPLINE") {
      logSink() << "electronic gain spline (G-matrix)" << LogIO::POST;
      
      // The following assumes that an existing gj_p is an apply-able GSPLINE
      //   (this is a bad assumption in general!)
      if (gj_p) {
	svj_p = gj_p;
	delsvj_=False;
      }
      else {
	svj_p = new GJonesSpline(*vs_p);
	delsvj_=True;
      }
      // Should move this into GJonesSpline::setSolver
      ((GJonesSpline *)svj_p)->setPhaseWrapHelp(splinepoint_p, phasewrap_p);
    } 
    
    if(upcaseType=="K") {
      logSink() << "Prototype Fringe-fitting (K-matrix)" << LogIO::POST;
      if(svc_p) delete svc_p; svc_p=0;
      svc_p = new KMueller(*vs_p);
      delsvc_=True;
    };

    if(upcaseType=="M") {
      logSink() << "closure errors (M-matrix)" << LogIO::POST;
      if(svc_p) delete svc_p; svc_p=0;
      svc_p = new MMueller(*vs_p);
      delsvc_=True;
    };

    if(upcaseType=="MF") {
      logSink() << "frequency-dependent closure errors (MF-matrix)" << LogIO::POST;
      if(svc_p) delete svc_p; svc_p=0;
      svc_p = new MfMueller(*vs_p);
      delsvc_=True;
    };

    if(upcaseType=="UVMOD") {
      logSink() << "UV Modelfit (single point source)" << LogIO::POST;
      if(svc_p) delete svc_p; svc_p=0;
      svc_p = new UVpMod(*vs_p);
      delsvc_=True;
    };

    // Some setup common to all types
    //   VisJones:
    if (svj_p) {
      svj_p->setSolver(solver);
      svj_p->setGain(g);
      svj_p->setNumberIterations(nit);
      svj_p->setTolerance(tol);   
    }
    //   VisCal:
    if (svc_p) {
      svc_p->setSolve(solver);
    }
    
    return True;

  } catch (AipsError x) {
    reset();
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    return False;
  } 
  return False;
}

Bool Calibrater::unset(String typeComp) {

  typeComp.upcase();
  if ((typeComp=="T" || typeComp=="TOPAC") && tj_p) {
    ve_p->unSetVisJones(*tj_p);
    delete tj_p; tj_p=0;
  }
  if (typeComp=="P" && pj_p) {
    ve_p->unSetVisJones(*pj_p);
    delete pj_p; pj_p=0;
  }
  if (typeComp=="GAINCURVE" && ej_p) {
    ve_p->unSetVisJones(*ej_p);
    delete ej_p; ej_p=0;
  }
  if (typeComp=="C" && cj_p) {
    ve_p->unSetVisJones(*cj_p);
    delete cj_p; cj_p=0;
  }
  if (typeComp=="D" && dj_p) {
    ve_p->unSetVisJones(*dj_p);
    delete dj_p; dj_p=0;
  }
  //  if ((typeComp=="G" || typeComp=="GPOLY" || typeComp=="GSPLINE" ||
  //       typeComp=="GDELAYRATESB") && gj_p) {
  if ((typeComp=="G" || typeComp=="GPOLY" || typeComp=="GSPLINE") && gj_p) {
    ve_p->unSetVisJones(*gj_p);
    delete gj_p; gj_p=0;
  }
  if ((typeComp=="B" || typeComp=="BPOLY") && bj_p) {
    ve_p->unSetVisJones(*bj_p);
    delete bj_p; bj_p=0;
  }

  if (typeComp=="K" && km_p) {
    ve_p->unSetVisCal(*km_p);
    delete km_p; km_p=0;
  }

  if (typeComp=="M" && mm_p) {
    ve_p->unSetVisCal(*mm_p);
    delete mm_p; mm_p=0;
  }
  if (typeComp=="MF" && mfm_p) {
    ve_p->unSetVisCal(*mfm_p);
    delete mfm_p; mfm_p=0;
  }

  return False;
}

Bool Calibrater::unsetSolve() {

  if (svj_p && delsvj_) delete svj_p;
  svj_p=NULL;
  delsvj_=False;

  if (svc_p && delsvc_) delete svc_p;
  svc_p=NULL;
  delsvc_=False;

  return True;

}

Bool Calibrater::correct() {
  
  logSink() << LogOrigin("Calibrater","correct") << LogIO::NORMAL;
  
  try {
  if(!ok()) return False;
  
  timer_p.mark();
  ve_p->correct();
  return True;  
  } catch (AipsError x) {
    reset();
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() 
	      << LogIO::POST;
    return False;
  } 
  return False;
}

Bool Calibrater::solve(String typeComp) {

  logSink() << LogOrigin("Calibrater","solve") << LogIO::NORMAL;
  
  try {
  if(!ok()) return False;
  
  typeComp.upcase();
  if(typeComp=="T") {
    logSink() << "Solving for T" << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Solving for T", "calibrater", typeComp, "calibrater::solve()");
    return ve_p->solve(*svj_p);

  }
 
  if((typeComp=="B" || typeComp=="BPOLY")) {
    logSink() << "Solving for B" << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Solving for B", "calibrater", typeComp, "calibrater::solve()");
    return ve_p->solve(*svj_p);
  }
 
  if((typeComp=="G" || typeComp=="GPOLY" || typeComp=="GSPLINE")) {
    logSink() << "Solving for G" << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Solving for G", "calibrater", typeComp, "calibrater::solve()");
    return ve_p->solve(*svj_p);
  }
 
  if(typeComp=="D") {
    logSink() << "Solving for D" << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Solving for D", "calibrater", typeComp, "calibrater::solve()");
    return ve_p->solve(*svj_p);
  };


  if(typeComp=="K") {
    logSink() << "Solving for K" << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Solving for K", "calibrater", typeComp, "calibrater::solve()");
    return ve_p->solve(*svc_p);
  };


  if(typeComp=="M") {
    logSink() << "Solving for M" << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Solving for M", "calibrater", typeComp, "calibrater::solve()");
    return ve_p->solve(*svc_p);
  };


  if(typeComp=="MF") {
    logSink() << "Solving for M (freq-dep)" << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Solving for M (freq-dep)", "calibrater", typeComp, "calibrater::solve()");
    return ve_p->solve(*svc_p);
  };

  if(typeComp=="UVMOD") {
    logSink() << "Solving for UV point-model" << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Solving for UV point-model", "calibrater", typeComp, "calibrater::solve()");
    return ve_p->solve(*svc_p);
  };


  logSink() << LogIO::SEVERE << "unknown type " << typeComp << " for solution"
	    << LogIO::POST;
  MSHistoryHandler::addMessage(*ms_p, "unknown type for solution", "calibrater", typeComp, "calibrater::solve()");

  return False;
  } catch (AipsError x) {
    reset();
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    MSHistoryHandler::addMessage(*ms_p, "Caught exception", "calibrater", typeComp, "calibrater::solve()");
    return False;
  } 

  return False;
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
  logSink() << LogOrigin("Calibrater","modelfit") << LogIO::NORMAL;
  
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
    
    return Vector<Double>();
  } 

}


void Calibrater::fluxscale(const String& infile, 
			   const String& outfile,
			   const Vector<Int>& refField, 
			   const Vector<Int>& refSpwMap, 
			   const Vector<Int>& tranField,
			   const Bool& append,
			   Matrix<Double>& fluxScaleFactor) {

  logSink() << "Beginning fluxscale." << LogIO::POST;

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
      if (tranField(0)>=0) {
	
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
      
      // Construct proper SVVJ object
      SolvableVisJones *fsvj_;
      if (caltype == "G Jones") {
	fsvj_ = new GJones(*vs_p);
      } else if (caltype == "T Jones") {
	fsvj_ = new TJones(*vs_p);
      } else {
	// Can't process other than G and T (B?)
	ostringstream typeErr;
	typeErr << "Type " << caltype 
	       << " not supported in fluxscale.";

	throw(AipsError(typeErr.str()));
      }

      // fill table with selection
      fsvj_->setInterpolationParam(infile,select,0.0);

      // Make fluxscale calculation
      fsvj_->fluxscale(refField,tranField,refSpwMap,fluxScaleFactor);
     
      // Store result
      if (append) {
	logSink() << "Appending result to " << outfile << LogIO::POST;
	String message="Appending result to "+outfile;
	MSHistoryHandler::addMessage(*ms_p, message, "calibrater", "", "calibrater::fluxscale()");
      } else {
	logSink() << "Storing result in " << outfile << LogIO::POST;
	String message="Storing result in "+outfile;
	MSHistoryHandler::addMessage(*ms_p, message, "calibrater", "", "calibrater::fluxscale()");
      }
      fsvj_->store(outfile,append);
      
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
    String message="Caught Exception: "+x.getMesg();
    MSHistoryHandler::addMessage(*ms_p, message, "calibrater", "", "calibrater::fluxscale()");
    return;

  }
  return;
}

void Calibrater::accumulate(const String& intab,
			    const String& incrtab,
			    const String& outtab,
			    const Vector<Int>& fields,
			    const Vector<Int>& calFields,
			    const String& interp,
			    const Double& t) {

  logSink() << "Beginning accumulate." << LogIO::POST;

  // SVJ objects:
  SolvableVisJones *invj_(0), *incrvj_(0);

  try {

    // Abort if incremental table isn't a cal table
    if ( !(Table::isReadable(incrtab) && 
	   Table::tableInfo(incrtab).type()=="Calibration") ) {

      ostringstream typeErr;
      typeErr << "Table " << incrtab
	      << " does not exist or is not a readable cal table.";      
      throw(AipsError(typeErr.str()));

    }
    
    // Incre table sets type we are accumulating
    String caltype(Table::tableInfo(incrtab).subType());

    // If no input cumulative timescale specified, then
    //   a valid input cumulative table must be specified
    if (t < 0.0) {
      if (!(Table::isReadable(intab) && 
	    Table::tableInfo(intab).type()=="Calibration" &&
	    Table::tableInfo(intab).subType()==caltype) ) {
      
	ostringstream typeErr;
	typeErr << "Table " << intab
		<< " does not exist or is not a readable cal table of type "
		<< caltype;
	throw(AipsError(typeErr.str()));
      }
    }

    // At this point all is ok; we will:
    //  o fill from intab and accumulate to it (t<0), OR
    //  o create a new cumulative table from scratch (t>0)

    
    //	logSink() << "Table " << infile 
    //		  << " is of type: "<< caltype 
    //		  << LogIO::POST;
    
    if (caltype == "G Jones") {
      invj_ = new GJones(*vs_p);
      incrvj_ = new GJones(*vs_p);
    } else if (caltype == "T Jones") {
      invj_ = new TJones(*vs_p);
      incrvj_ = new TJones(*vs_p);
    } else if (caltype == "B Jones") {
      invj_ = new BJones(*vs_p);
      incrvj_ = new BJones(*vs_p);
    } else {
      // Can't yet process types other than G, T, B
      ostringstream typeErr;
      typeErr << "Type " << caltype 
	      << " not yet supported in accumulate.";
      
      throw(AipsError(typeErr.str()));
    }
    
    // initialize the cumulative solutions
    invj_->setAccumulate(intab,"",t,-1);
    
    // form selection on incr table
    String incrSel="";
    if (calFields(0)>=0) {
      
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
    incrvj_->setInterpolationParam(incrtab,incrSel,0.0);
    incrvj_->initInterp(interp,Vector<Int>(1,-1));

    // All ready, now do the accumulation
    Vector<Int> tfields(fields);
    if (tfields(0)<0) tfields.resize();
    invj_->accumulate(*incrvj_,tfields);
    
    // ...and store the result
    logSink() << "Storing accumulated calibration in table: " 
	      << outtab
	      << LogIO::POST;
    invj_->store(outtab,False);
    
    delete invj_;
    delete incrvj_;

    logSink() << "Finished accumulation." 
	      << LogIO::POST;

  } catch (AipsError x) {

    if (invj_) delete invj_;
    if (incrvj_) delete incrvj_;
    
    logSink() << LogIO::SEVERE
	      << "Caught Exception: "
	      << x.getMesg()
	      << LogIO::POST;
    return;
  }
  return;
}


Bool Calibrater::smooth(const String& infile, 
                        const String& outfile,const Bool& append,
                        const String& select,
                        const String& smoothtype,const Double& smoothtime,
                        const String& interptype,const Double& interptime)
{
  
  logSink() << "Beginning smoothing/interpolating method." << LogIO::POST;

  // Make some pointers for SVJs at each step
  SolvableVisJones * currvj;

  // Open input table
  currvj = new GJones(*vs_p);
  currvj->setInterpolationParam(infile,select,0.0);

  if (smoothtype!="none") {
    // Process smoothing parameters
    SolvableVisJones::FilterType smtype(SolvableVisJones::meanFilter);
    String smtypetxt;
    if (smoothtype.contains("smedian")) {
      smtype=SolvableVisJones::slidingMedian;
      smtypetxt=" sliding median";
    }
    else if (smoothtype.contains("median")) {
      smtype=SolvableVisJones::medianFilter;
      smtypetxt=" decimating median";
    }
    else if (smoothtype.contains("smean")) {
      smtype=SolvableVisJones::slidingMean;
      smtypetxt=" sliding mean";
    }
    else if (smoothtype.contains("mean")) {
      smtype=SolvableVisJones::meanFilter;
      smtypetxt=" decimating mean";
    }

    Bool doAmpPhase(False);
    String doAPtxt="";
    if (smoothtype.contains("AP")) {
      doAmpPhase=True;
      doAPtxt=" amp/phase";
    }

    logSink() << "Smoothing" << doAPtxt << " solutions with a" 
	      << smtypetxt <<" on a " << smoothtime << "-sec timescale."
	      << LogIO::POST;

    // Make a SVJ to hold smoothed table 
    SolvableVisJones *smvj;
    smvj = new GJones(*vs_p);
    smvj->setInterpolationParam(infile,select,0.0);

    // ...and smooth it
    smvj->filterFrom(*currvj,smtype,Float(smoothtime),doAmpPhase);

    // Delete input vj and point to smoothed one
    delete currvj;
    currvj=smvj;
  };

  if (interptype!="none") {

    SolvableVisJones::InterpolaterType inttype(SolvableVisJones::spline);
    String intypetxt;
    if (interptype.contains("nearest")) {
      inttype=SolvableVisJones::nearestNeighbour;
      intypetxt=" nearest neighbor";
    }
    else if (interptype.contains("linear")) {
      inttype=SolvableVisJones::linear;
      intypetxt=" linear";
    }
    else if (interptype.contains("cubic")) {
      inttype=SolvableVisJones::cubic;
      intypetxt=" cubic";
    }
    else if (interptype.contains("spline")) {
      inttype=SolvableVisJones::spline;
      intypetxt=" spline";
    }

    Bool doAmpPhase(False);
    String doAPtxt=" to";
    if (interptype.contains("AP")) {
      doAmpPhase=True;
      doAPtxt=" to amp/phase";
    }

    logSink() << "Applying" << intypetxt << " interpolation" << doAPtxt 
	      << " solutions on a " << interptime << "-sec timescale."
	      << LogIO::POST;

    // Make a SVJ to hold interpolated solutions....
    SolvableVisJones *intvj;
    intvj = new GJones(*vs_p);
    intvj->setSolverParam(outfile,interptime);

    // ...and interpolate
    intvj->interpolateFrom(*currvj,inttype,doAmpPhase);

    // Delete input vj and point to interpolated one
    delete currvj;
    currvj=intvj;
  };

  // Store the result on disk
  if (append) logSink() << "Appending result to " << outfile << LogIO::POST;
  else logSink() << "Storing result in " << outfile << LogIO::POST;
  currvj->store(outfile,append);

  // Clean up
  if (currvj) delete currvj; currvj=0;

  return True;
}


Bool Calibrater::put(String typeComp, String tableName, Bool append) {
  
  logSink() << LogOrigin("Calibrater","put") << LogIO::NORMAL;
  
  try {
  if(!ok()) return False;
  
  typeComp.upcase();
  logSink() <<   "Storing "<<typeComp<<" matrix in table "
	    << tableName << LogIO::POST;
  if(typeComp=="P") {
    if(svj_p) {
      svj_p->store(tableName, append);
      return True;
    }
    else {
      logSink() <<   "Error: " << typeComp << " not yet initialized"
		<< LogIO::POST;
      return False;
    }
  }
  if(typeComp=="C") {
    if(svj_p) {
      svj_p->store(tableName, append);
      return True;
    }
    else {
      logSink() <<   "Error: " << typeComp << " not yet initialized"
		<< LogIO::POST;
      return False;
    }
  }
  if(typeComp=="T") {
    if(svj_p) {
      svj_p->store(tableName, append);
      return True;
    }
    else {
      logSink() <<   "Error: " << typeComp << " not yet initialized"
		<< LogIO::POST;
      return False;
    }
  }
  if(typeComp=="B" || typeComp=="BPOLY") {
    if(svj_p) {
      svj_p->store(tableName, append);
      return True;
    }
    else {
      logSink() <<   "Error: " << typeComp << " not yet initialized"
		<< LogIO::POST;
      return False;
    }
  }
  if(typeComp=="G" || typeComp=="GPOLY" || typeComp=="GSPLINE") {
    if(svj_p) {
      svj_p->store(tableName, append);
      return True;
    }
    else {
      logSink() <<   "Error: " << typeComp << " not yet initialized"
		<< LogIO::POST;
      return False;
    }
  }
  if(typeComp=="D") {
    if(svj_p) {
      svj_p->store(tableName, append);
      return True;
    }
    else {
      logSink() <<   "Error: " << typeComp << " not yet initialized"
		<< LogIO::POST;
      return False;
    }
  }

  {
    logSink() << LogIO::SEVERE << "Error: unknown type " <<typeComp << LogIO::POST;
  }
  return False;
  } catch (AipsError x) {
    reset();
    logSink() << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return False;
}

Bool Calibrater::reset() 
{
  //  if(ms_p) delete ms_p; ms_p=0;
  if(mssel_p) delete mssel_p; mssel_p=0;
  if(bj_p) delete bj_p; bj_p=0;
  if(gj_p) delete gj_p; gj_p=0;
  if(dj_p) delete dj_p; dj_p=0;
  if(pj_p) delete pj_p; pj_p=0;
  if(ej_p) delete ej_p; ej_p=0;
  if(cj_p) delete cj_p; cj_p=0;
  if(tj_p) delete tj_p; tj_p=0;
  if(mj_p) delete mj_p; mj_p=0;
  if(km_p) delete km_p; km_p=0;
  if(mm_p) delete mm_p; mm_p=0;
  if(mfm_p) delete mfm_p; mfm_p=0;
  return True;
}

Bool Calibrater::initCalSet(const Int& calSet) 
{
  if (vs_p) {
    vs_p->initCalSet(calSet);
    return True;
  }
  else {
    return False;
  }
}

Bool Calibrater::ok() {

  logSink() << LogOrigin("Calibrater","ok") << LogIO::NORMAL;
  if(vs_p && ms_p && mssel_p && ve_p) {
    return True;
  }
  else {
    logSink() << "calibrater is not yet initialized" << LogIO::POST;
    return False;
  }
}


void Calibrater::getRawPhaseVisSet(Vector<Int>& spwid){



  // Get the fieldids of the calibraters
  Vector<Int> fieldid(1);
  VisIter& vi(vs_p->iter());
  Int numfield=1;
  vi.originChunks();
  fieldid[0]=vi.fieldId();
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()){
    if (vi.newFieldId() && (vi.fieldId() != fieldid[numfield-1])){
      ++numfield;
      fieldid.resize(numfield, True);
      fieldid[numfield-1]=vi.fieldId();
    }
  }

  Table sorted=ms_p->keywordSet().asTable("SORTED_TABLE");
  TableExprNode condition;
  String colf=MS::columnName(MS::FIELD_ID);
  String cols=MS::columnName(MS::DATA_DESC_ID);
  condition=sorted.col(colf).in(fieldid)&&
    sorted.col(cols).in(spwid);
  msselRaw_p = new MeasurementSet(sorted(condition));
  
  AlwaysAssert(msselRaw_p, AipsError);
  msselRaw_p->rename(msname_p+"/RAWPHASE_TABLE", Table::Scratch);
  Block<int> sort(0);
  Matrix<Int> noselection;
  rawvs_p= new VisSet(*msselRaw_p, sort, noselection); 
 

}

void Calibrater::setPhaseSplineParam(const Int& npoint, const Double& phasewrap){


  phasewrap_p=phasewrap;
  splinepoint_p=npoint;

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
