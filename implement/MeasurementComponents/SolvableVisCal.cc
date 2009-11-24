//# SolvableVisCal.cc: Implementation of SolvableVisCal classes
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
//# $Id: VisCal.cc,v 1.15 2006/02/06 19:23:11 gmoellen Exp $

#include <synthesis/MeasurementComponents/CalCorruptor.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>

#include <msvis/MSVis/VisBuffer.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Memory.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <casa/sstream.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Containers/RecordField.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/System/Aipsrc.h>
#include <casa/System/ProgressMeter.h>

#include <fstream>

namespace casa { //# NAMESPACE CASA - BEGIN

// **********************************************************
//  SolvableVisCal Implementations
//

SolvableVisCal::SolvableVisCal(VisSet& vs) :
  VisCal(vs),
  cs_(NULL),
  cint_(NULL),
  maxTimePerSolution_p(0), 
  minTimePerSolution_p(10000000), 
  avgTimePerSolution_p(0),
  timer_p(),
  calTableName_(""),
  calTableSelect_(""),
  append_(False),
  tInterpType_(""),
  fInterpType_(""),
  spwMap_(vs.numberSpw(),-1),
  refant_(-1),
  minblperant_(4),
  solved_(False),
  apmode_(""),
  solint_("inf"),
  simint_("integration"),
  simulated_(False),
  solnorm_(False),
  minSNR_(0.0f),
  combine_(""),
  focusChan_(0),
  dataInterval_(0.0),
  fitWt_(0.0),
  fit_(0.0),
  solveCPar_(vs.numberSpw(),NULL),
  solveRPar_(vs.numberSpw(),NULL),
  solveParOK_(vs.numberSpw(),NULL),
  solveParErr_(vs.numberSpw(),NULL),
  solveParSNR_(vs.numberSpw(),NULL),
  srcPolPar_(),
  chanmask_(NULL),
  corruptor_p(NULL)
{

  if (prtlev()>2) cout << "SVC::SVC(vs)" << endl;

  caiRC_p = Aipsrc::registerRC("calibrater.activity.interval", "3600.0");
  cafRC_p = Aipsrc::registerRC("calibrater.activity.fraction", "0.00001");
  String ca_str = Aipsrc::get(caiRC_p);
  std::sscanf(std::string(ca_str).c_str(), "%f", &userPrintActivityInterval_p);
  userPrintActivityInterval_p = abs(userPrintActivityInterval_p);

  ca_str = Aipsrc::get(cafRC_p);
  std::sscanf(std::string(ca_str).c_str(), "%f", &userPrintActivityFraction_p);
  userPrintActivityFraction_p = abs(userPrintActivityFraction_p);

  initSVC();

};

SolvableVisCal::SolvableVisCal(const Int& nAnt) :
  VisCal(nAnt),
  cs_(NULL),
  cint_(NULL),
  maxTimePerSolution_p(0), 
  minTimePerSolution_p(10000000), 
  avgTimePerSolution_p(0),
  timer_p(),
  calTableName_(""),
  calTableSelect_(""),
  append_(False),
  tInterpType_(""),
  fInterpType_(""),
  spwMap_(1,-1),
  refant_(-1),
  minblperant_(4),
  solved_(False),
  apmode_(""),
  solint_("inf"),
  simint_("inf"),
  simulated_(False),
  solnorm_(False),
  minSNR_(0.0),
  combine_(""),
  focusChan_(0),
  dataInterval_(0.0),
  fitWt_(0.0),
  fit_(0.0),
  solveCPar_(1,NULL),
  solveRPar_(1,NULL),
  solveParOK_(1,NULL),
  solveParErr_(1,NULL),
  solveParSNR_(1,NULL),
  srcPolPar_(),
  chanmask_(NULL),
  corruptor_p(NULL)
{  

  if (prtlev()>2) cout << "SVC::SVC(i,j,k)" << endl;

  caiRC_p = Aipsrc::registerRC("calibrater.activity.interval", "3600.0");
  cafRC_p = Aipsrc::registerRC("calibrater.activity.fraction", "0.00001");
  String ca_str = Aipsrc::get(caiRC_p);
  std::sscanf(std::string(ca_str).c_str(), "%f", &userPrintActivityInterval_p);
  userPrintActivityInterval_p = abs(userPrintActivityInterval_p);

  ca_str = Aipsrc::get(cafRC_p);
  std::sscanf(std::string(ca_str).c_str(), "%f", &userPrintActivityFraction_p);
  userPrintActivityFraction_p = abs(userPrintActivityFraction_p);

  initSVC();

}

SolvableVisCal::~SolvableVisCal() {

  if (prtlev()>2) cout << "SVC::~SVC()" << endl;

  deleteSVC();

  if (cs_)   delete cs_;   cs_=NULL;
  if (cint_) delete cint_; cint_=NULL;

}

void SolvableVisCal::makeCalSet()  { makeCalSet(False); }

void SolvableVisCal::makeCalSet(Bool newtable)
{
  switch(parType())
    {
    case VisCalEnum::COMPLEX:
      {
	if (newtable) 
	  cs_ = new CalSet<Complex>(nSpw(),nPar(),Vector<Int>(1,1),nElem(),Vector<Int>(1,1));
	//cs_ = new CalSet<Complex>(nSpw(),nPar(),nChan(),nElem(),nTime());
	else
	  cs_ = new CalSet<Complex>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());
	cs().initCalTableDesc(typeName(),parType_);
	nChanParList() = cs().nChan();
	startChanList() = cs().startChan();

	// Create CalInterp
	cint_ = new CalInterp(cs(),tInterpType(),"nearest");
	ci().setSpwMap(spwMap());
	break;
      }
    case VisCalEnum::REAL:
      {
	if (newtable) 
	  rcs_ = new CalSet<Float>(nSpw(),nPar(),Vector<Int>(1,1),nElem(),Vector<Int>(1,1));
	//rcs_ = new CalSet<Float>(nSpw(),nPar(),nChan(),nElem(),nTime());
	else
	  rcs_ = new CalSet<Float>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());
	rcs().initCalTableDesc(typeName(),parType_);
	// Create CalInterp
// 	cint_ = new CalInterp(rcs(),tInterpType(),"nearest");
// 	ci().setSpwMap(spwMap());
	break;
      }
    default:
         throw(AipsError("Internal error(Calibrater Module): Unsupported parameter type "
			 "COMPLEXREAL found in SolvableVisCal::makeCalSet()"));
    }    
}

// Generic setapply
void SolvableVisCal::setApply() {

  if (prtlev()>2) cout << "SVC::setApply()" << endl;

  // Generic settings
  calTableName()="<none>";
  calTableSelect()="<none>";
  tInterpType()="nearest";
  indgen(spwMap());
  interval()=DBL_MAX;

  // This is apply context  
  setApplied(True);
  setSolved(False);

  // Assemble inflated (but empty) CalSet from current shapes
  Vector<Int> nTimes(nSpw(),1);
  switch(parType())
    {
    case VisCalEnum::COMPLEX:
      {
	cs_ = new CalSet<Complex>(nSpw(),nPar(),nChanParList(),nElem(),nTimes);
	cs().initCalTableDesc(typeName(),parType_);
	cint_ = new CalInterp(cs(),tInterpType(),"nearest");
	break;
      }
    case VisCalEnum::REAL:
      {
	rcs_ = new CalSet<Float>(nSpw(),nPar(),nChanParList(),nElem(),nTimes);
	rcs().initCalTableDesc(typeName(),parType_);
// 	cint_ = new CalInterp(rcs(),tInterpType(),"nearest");
	break;
      }
    default: 
      throw(AipsError("Internal SVC error: Got invalid VisCalEnum"));
    }
  //  cs_ = new CalSet<Complex>(nSpw(),nPar(),nChanParList(),nElem(),nTimes);

  // Assemble CalInterp
  ci().setSpwMap(spwMap());
}



// Setapply from a Cal Table, etc.
void SolvableVisCal::setApply(const Record& apply) {
  //  Inputs:
  //    apply           Record&       Contains application params
  //    

  if (prtlev()>2) cout << "SVC::setApply(apply)" << endl;

  // Call VisCal version for generic stuff
  VisCal::setApply(apply);

  // Collect Cal table parameters
  if (apply.isDefined("table")) {
    calTableName()=apply.asString("table");
    verifyCalTable(calTableName());
  }

  if (apply.isDefined("select"))
    calTableSelect()=apply.asString("select");

  else {
    
    calTableSelect()="";
    //    String spwsel("");
    //    if (apply.isDefined("spw")) {
    //      ostringstream os;
    //      os << Vector<Int>(apply.asArrayInt("spw"));
    //      spwsel = os.str();
    //    }
    //    cout << "spwsel = " << spwsel << endl;

    String fldsel("");
    if (apply.isDefined("field")) {
      ostringstream os;
      os << Vector<Int>(apply.asArrayInt("field"));
      if (os.str()!="[]")
	fldsel = os.str();
    }
    //    cout << "fldsel = " << fldsel << endl;

    if (fldsel.length()>0) {
      ostringstream os;
      os << "(FIELD_ID IN " << fldsel << ")";
      calTableSelect() = os.str();
    }
    //   cout << "calTableSelect() = " << calTableSelect() << endl;
  }
  
 
  // Does this belong here?
  if (apply.isDefined("append"))
    append()=apply.asBool("append");

  // Collect interpolation parameters
  if (apply.isDefined("interp"))
    tInterpType()=apply.asString("interp");

  // Protect against non-specific interp
  if (tInterpType()=="")
    tInterpType()="linear";

  // TBD: move spwmap to VisCal version?

  indgen(spwMap());
  Bool autoFanOut(False);
  if (apply.isDefined("spwmap")) {
    Vector<Int> spwmap(apply.asArrayInt("spwmap"));
    if (allGE(spwmap,0)) {
      // User has specified a valid spwmap
      if (spwmap.nelements()==1)
	spwMap()=spwmap(0);
      else
	spwMap()(IPosition(1,0),IPosition(1,spwmap.nelements()-1))=spwmap;
      // TBD: Report non-trivial spwmap to logger.
      //      cout << "Note: spwMap() = " << spwMap() << endl;
    }
    else if (spwmap(0)==-999)
      autoFanOut=True;
  }

  AlwaysAssert(allGE(spwMap(),0),AipsError);

  // TBD: move interval to VisCal version?
  if (apply.isDefined("t"))
    interval()=apply.asFloat("t");

  // This is apply context  
  setApplied(True);
  setSolved(False);

  //  TBD:  "Arranging to apply...."


  // Create CalSet, from table
  //  cs_ = new CalSet<Complex>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());
  makeCalSet();

  // Handle possible global spw fan-out
  if (autoFanOut) {
    // Use first valid spw for all spws
    Int ispw=0;
    while (!cs().spwOK()(ispw)) ++ispw;
    spwMap()=ispw;
    logSink() << "Using automatic calibration fan-out of spw = " << ispw
	      << " for " << typeName()
	      << LogIO::POST;
  }

  // These come from CalSet now, but will eventually come from CalInterp

  //  cout << "nChanParList().shape() = " << nChanParList().shape() << endl;
  //  cout << "nChanParList()         = " << nChanParList() << endl;
  //  cout << "cs().nChan().shape()   = " << cs().nChan().shape() << endl;
  //  cout << "cs().nChan()           = " << cs().nChan() << endl;

  switch (parType())
    {
    case VisCalEnum::COMPLEX:
      {
	for (Int ispw=0;ispw<nSpw();++ispw) 
	  {
	    nChanParList()(ispw) = cs().nChan()(spwMap()(ispw));
	    startChanList()(ispw) = cs().startChan()(spwMap()(ispw));
	  }
	// Create CalInterp
	cint_ = new CalInterp(cs(),tInterpType(),"nearest");
	break;
      }
    case VisCalEnum::REAL:
      {
	for (Int ispw=0;ispw<nSpw();++ispw) 
	  {
	    nChanParList()(ispw) = rcs().nChan()(spwMap()(ispw));
	    startChanList()(ispw) = rcs().startChan()(spwMap()(ispw));
	  }
	// Create CalInterp
	//	cint_ = new CalInterp(rcs(),tInterpType(),"nearest");
	break;
      }
    case VisCalEnum::COMPLEXREAL:
      throw(AipsError("Internal error(Calibrater Module): Unsupported parameter type "
		      "COMPLEXREAL found in SolvableVisCal::setApply()"));
        break;
    }
  //  cout << "nChanParList().shape() = " << nChanParList().shape() << endl;
  //  cout << "nChanParList()         = " << nChanParList() << endl;
  //  cout << "cs().nChan().shape()   = " << cs().nChan().shape() << endl;
  //  cout << "cs().nChan()           = " << cs().nChan() << endl;

  // Sanity check on parameter channel shape
  //  AlwaysAssert((freqDepPar()||allEQ(nChanParList(),1)),AipsError);

  ci().setSpwMap(spwMap());

}




// ===================================================

void SolvableVisCal::createCorruptor(const VisIter& vi,const Record& simpar, const Int nSim) {
  LogIO os(LogOrigin("SVC", "createCorruptor", WHERE));

  // this is the generic create and init 
  // ** specialists should call this after createing their corruptor
  // and before initializing

  if (!corruptor_p) {
    corruptor_p = new CalCorruptor(nSim);  
    os << LogIO::WARN << "creating generic CalCorruptor" << LogIO::POST;
  }

  // would be nice to reduce the amount of stuff passed down to the corruptor; 
  // fnChan is used in AtmosCorruptor, currAnt and curr Spw are used generically

  corruptor_p->prtlev()=prtlev();
  corruptor_p->freqDepPar()=freqDepPar();
  corruptor_p->simpar()=simpar;

//  corruptor_p->nCorr()=vi.nCorr();
//  if (prtlev()>3) 
//    os << LogIO::POST << "nCorr= " << vi.nCorr() << LogIO::POST;      
  // what matters is nPar not nCorr - then apply uses coridx
  corruptor_p->nPar()=nPar();

  const ROMSSpWindowColumns& spwcols = vi.msColumns().spectralWindow();  
  if (prtlev()>3) cout << " SpwCols accessed:" << endl;
  if (prtlev()>3) cout << "   nSpw()= " << nSpw() << endl;
  if (prtlev()>3) cout << "   spwcols.nrow()= " << spwcols.nrow() << endl;  
  AlwaysAssert(nSpw()==spwcols.nrow(),AipsError);
  // there's a member variable in Simulator nSpw, should we verify that 
  // this is the same? probably.

  // things will break if spw mapping, ie not in same order as in vs
  corruptor_p->nSpw()=nSpw();
  corruptor_p->nAnt()=nAnt();
  corruptor_p->currAnt()=0;
  corruptor_p->currSpw()=0;
  corruptor_p->fRefFreq().resize(nSpw());
  corruptor_p->fnChan().resize(nSpw());
  corruptor_p->fWidth().resize(nSpw());
  corruptor_p->currChans().resize(nSpw());
  
  for (Int irow=0;irow<nSpw();++irow) { 
    corruptor_p->currChans()[irow]=0;
    corruptor_p->fRefFreq()[irow]=spwcols.refFrequency()(irow);
    // don't need to change fnChan to 1 if not freqDepPar()
    // because fnChan is only used in AtmosCorruptor if 
    // freqDepPar() is set i.e. in initAtm().
    corruptor_p->fnChan()[irow]=spwcols.numChan()(irow);    
    corruptor_p->fWidth()[irow]=spwcols.totalBandwidth()(irow); 
    // totalBandwidthQuant ?  in other places its assumed to be in Hz
  }
  // see MSsummary.cc for more info/examples

}



void SolvableVisCal::setSimulate(VisSet& vs, Record& simpar, Vector<Double>& solTimes) {

  LogIO os(LogOrigin("SVC", "setSimulate()", WHERE));
  if (prtlev()>2) cout << "SVC::setSimulate(simpar)" << endl;

  // Extract calWt
  if (simpar.isDefined("calwt"))
    calWt()=simpar.asBool("calwt");
  
  // (copied from SolvableVisCal::setSolve)
  if (simpar.isDefined("simint"))
    simint()=simpar.asString("simint");
  
  if (upcase(simint()).contains("INF") || simint()=="") {
    simint()="infinite";
    interval()=-1.0;
  } else if (upcase(simint()).contains("INT")) {
    simint()="integration";
    interval()=0.0;
  } else {
    QuantumHolder qhsimint;
    String error;
    Quantity qsimint;
    qhsimint.fromString(error,simint());
    if (error.length()!=0)
      throw(AipsError("Unrecognized units for simint."));
    qsimint=qhsimint.asQuantumDouble();
    
    if (qsimint.isConform("s"))
      interval()=qsimint.get("s").getValue();
    else {
      // assume seconds
      interval()=qsimint.getValue();
      simint()=simint()+"s";
    }
  }

  // check if want to write a table:
  if (simpar.isDefined("caltable")) {
    calTableName()=simpar.asString("caltable");
    // RI todo SVC::setSimulate deal with over-writing existing caltables
    // verifyCalTable(calTableName());
    append()=False;
  } else {
    calTableName()="<none>";
  }
  if (calTableName().length()==0)
    calTableName()="<none>";
   
  setSolved(False);
  // this has to be true for some of George's VE stuff 
  // but be careful about VC structure and inflation!
  setApplied(True); 
  setSimulated(True);

  // without this, CI gets created without a sensible time
  // interpolation, and ends up bombing
  tInterpType()="nearest";


  // ----------------
  // assess size of ms:

  // how to combine data in sizeUp e.g. SPW,SCAN
  if (simpar.isDefined("combine"))
    combine()=simpar.asString("combine");
  else
    throw(AipsError("MS combination information not set"));

  // GM: organize calibration correction/corruption according to 
  // multi-spw consistency; e.g. move time ahead of data_desc_id so that 
  // data_desc_id (spw) changes faster than time, even within scans.

  // RI todo double-check logic in case of spwmap and multi-spw
  
  // Arrange for iteration over data
  Block<Int> columns;
  // include scan iteration
  columns.resize(5);
  columns[0]=MS::ARRAY_ID;
  columns[1]=MS::SCAN_NUMBER;
  columns[2]=MS::FIELD_ID;
  // GM's order:
  //columns[3]=MS::DATA_DESC_ID;
  //columns[4]=MS::TIME;
  // put spw after time:
  columns[4]=MS::DATA_DESC_ID;
  columns[3]=MS::TIME;
  

  // drop chunking time interval down to the simulation interval, else will 
  // chunk by entire scans.
  Double iterInterval(max(interval(),DBL_MIN));
  if (interval() < 0.0) {   // means no interval (infinite solint)
    iterInterval=0.0;
    interval()=DBL_MAX;   
  }
  vs.resetVisIter(columns,iterInterval);

  // assume only one ms attached to the VI. need vi for mscolumns.
  VisIter& vi(vs.iter());
  
  Vector<Int> nChunkPerSol;
  // independent of simpar details
  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);
  if (prtlev()>1 and prtlev()<3) cout << " VisCal sized for Simulation with " << nSim << " slots." << endl;
  if (prtlev()>4) cout << " solTimes = " << solTimes-solTimes[0] << endl;

  if (!(simpar.isDefined("startTime"))) {    
    throw(AipsError("can't add startTime field to Record"));
    // Record seems to have been designed strangely, so this doesn't work:
//    RecordDesc simParDesc = simpar.description();
//    simParDesc.addField("startTime",TpDouble);
//    simpar.restructure(simParDesc);
  }
  simpar.define("startTime",min(solTimes));

  if (!(simpar.isDefined("stopTime"))) {    
    throw(AipsError("can't add stopTime field to Record"));
//    RecordDesc simParDesc = simpar.description();
//    simParDesc.addField("stopTime",TpDouble);
//    simpar.restructure(simParDesc);
  }
  simpar.define("stopTime",max(solTimes));


  // -------------------
  // create (and initialize) a corruptor in a VC-specific way - 
  // specializations need to call the generic SVC::createCorruptor to get 
  // spw etc information passed down.
  createCorruptor(vi,simpar,nSim);
  
  // set times in the corruptor if createCorruptor didn't:
  if (!corruptor_p->times_initialized()) {
    corruptor_p->curr_slot()=0;
    corruptor_p->slot_times().resize(nSim);
    corruptor_p->slot_times()=solTimes;
    corruptor_p->startTime()=min(solTimes);
    corruptor_p->stopTime()=max(solTimes);
    corruptor_p->times_initialized()=True;
  }

  if (prtlev()>3) 
    cout << " slot_times= " 
	 << corruptor_p->slot_time(1)-corruptor_p->slot_time(0) << " " 
	 << corruptor_p->slot_time(2)-corruptor_p->slot_time(0) << " " 
	 << corruptor_p->slot_time(3)-corruptor_p->slot_time(0) << " " 
	 << corruptor_p->slot_time(4)-corruptor_p->slot_time(0) << " " 
	 << corruptor_p->slot_time(5)-corruptor_p->slot_time(0) << " " 
	 << corruptor_p->slot_time(6)-corruptor_p->slot_time(0) << " " 
	 << corruptor_p->slot_time(7)-corruptor_p->slot_time(0) << " " 
	 << endl;  


  os << LogIO::NORMAL << "Calculating corruption terms for " << siminfo() << LogIO::POST;
  //-------------------
  // actually calculate the calset
  // which was inflated by sizeupSim to the right size

  Vector<Int> slotidx(nSpw(),-1);

  vi.originChunks();
  Double t0(0.);
  
  Vector<Int> a1;
  Vector<Int> a2;
  Matrix<Bool> flags;
  
  ProgressMeter meter(0.,1. , "Simulating "+nameOfType(type())+" ", "", "", "", True, .1);

  for (Int isim=0;isim<nSim && vi.moreChunks();++isim) {      

    Int thisSpw=spwMap()(vi.spectralWindow());
    currSpw()=thisSpw;
    corruptor_p->currSpw()=thisSpw;
    slotidx(thisSpw)++;

    IPosition cparshape=solveCPar().shape();
    // this will get changed to True by keep() depending on solveParOK
    cs().solutionOK(currSpw())(slotidx(thisSpw))=False;  // all Pars, all Chans, all Ants

    Vector<Double> timevec;
    Double starttime,stoptime;
    starttime=vi.time(timevec)[0];

    IPosition blc(3,0,       0,0); // par,chan=focuschan,elem=ant
    IPosition trc(3,nPar()-1,0,0);
    
    Bool useBase(False);
    if (nElem()==nBln()) useBase=True;

    for (Int ichunk=0;ichunk<nChunkPerSol[isim];++ichunk) {
      // RI todo: SVC:setSim deal with spwmap and spwcomb() here

      for (vi.origin(); vi.more(); vi++) {

	if (prtlev()>5) cout << "vi++"<<endl;
	vi.antenna1(a1);
        vi.antenna2(a2);
        vi.flag(flags);
        vi.time(timevec);
	// assume that the corruptor slot i.e. time is the same for all rows.
	// (to the accuracy of simint())
	
	// set things for SVC::keep:
	Int tvsize;
	timevec.shape(tvsize);
	stoptime=timevec[tvsize-1];
	refTime() = 0.5*(starttime+stoptime);
	interval() = (stoptime-starttime);
	currField() = vi.fieldId();

	// make sure we have the right slot in the corruptor 
	// RI todo SVC::setSim can the corruptor slot be the same for all chunks?
	// we were setting curr_time() to timevec[0], but I think refTime is more 
	// accurate
	if (corruptor_p->curr_time()!=refTime()) {
	  corruptor_p->curr_time()=refTime();
	  // find new slot if required
	  Double dt(1e10),dt0(-1);
	  dt0 = abs(corruptor_p->slot_time() - refTime());
	  
	  for (Int newslot=0;newslot<corruptor_p->nSim();newslot++) {
	    dt=abs(corruptor_p->slot_time(newslot) - refTime());
	    // is this newslot closer to the current time?
	    if (dt<dt0) {
	      corruptor_p->curr_slot()=newslot;
	      dt0 = dt;
	    }
	  }
	}
	if (prtlev()>5) cout << "slot = "<< corruptor_p->curr_slot()<<endl;
	
	solveCPar()=Complex(0.0);
	solveParOK()=False;
	
	for (Int irow=0;irow<vi.nRow();++irow) {
	  
	  if (nfalse(flags.column(irow))> 0 ) {
	    
	    corruptor_p->currAnt()=a1(irow);
	    // only used for baseline-based SVCs
	    corruptor_p->currAnt2()=a2(irow);
	    
	    // baseline or antenna-based?
	    if (useBase) {
	      blc(2)=blnidx(a1(irow),a2(irow));
	      trc(2)=blc(2);
	    } else {
	      blc(2)=a1(irow);
	      trc(2)=a1(irow);
	    }
	    
	    // RI TODO make some freqDepPar VCs return all ch at once
	    //if not freqDepPar, then nChanPar=1 right?
	    for (Int ich=nChanPar()-1;ich>-1;--ich) {		
	      focusChan()=ich;
	      corruptor_p->setFocusChan(ich);
	      blc(1)=ich;
	      trc(1)=ich;

	      if ( a1(irow)==a2(irow) ) {
		// autocorrels should get 1. for multiplicative VC
		if (type()==VisCal::ANoise or type()==VisCal::A)
		  solveCPar()(blc,trc)=0.0;
		else
		  solveCPar()(blc,trc)=1.0;
	      } else {
		// specialized simPar for each VC - may depend on mode etc
		for (Int ipar=0;ipar<nPar();ipar++) 
		  solveCPar()(blc,trc)[ipar,0,0] = corruptor_p->simPar(vi,type(),ipar);		
	      }		      
	    }      
	    if (prtlev()>4) cout << "row "<<irow<< " set; cparshape="<<solveCPar().shape()<<endl;
	    blc(1)=0;
	    trc(1)=nChanPar()-1;
	    solveParOK()(blc,trc)=True;	      
	  }// if not flagged
	}// row
	if (prtlev()>4) cout << "about to keep, cs.parshape="<<cs().par(currSpw()).shape()<<endl;
	// sigh - need own keep too, to have all chans at once...
	{
	  if (slotidx(thisSpw)<cs().nTime(currSpw())) {
	    // An available valid slot
	    
	    cs().fieldId(currSpw())(slotidx(thisSpw))=currField();
	    cs().time(currSpw())(slotidx(thisSpw))=refTime();
	    
	    // Only stop-start diff matters
	    //  TBD: change CalSet to use only the interval
	    //  TBD: change VisBuffAcc to calculate exposure properly
	    cs().startTime(currSpw())(slotidx(thisSpw))=0.0;
	    cs().stopTime(currSpw())(slotidx(thisSpw))=interval();
	    
	    // For now, just make these non-zero:
	    cs().iFit(currSpw()).column(slotidx(thisSpw))=1.0;
	    cs().iFitwt(currSpw()).column(slotidx(thisSpw))=1.0;
	    cs().fit(currSpw())(slotidx(thisSpw))=1.0;
	    cs().fitwt(currSpw())(slotidx(thisSpw))=1.0;
	    
	    IPosition blc4(4,0,       0           ,0,        slotidx(thisSpw));
	    IPosition trc4(4,nPar()-1,nChanPar()-1,nElem()-1,slotidx(thisSpw));
	    cs().par(currSpw())(blc4,trc4).nonDegenerate(3) = solveCPar();
	    
	    // TBD:  Handle solveRPar here!
	    
	    cs().parOK(currSpw())(blc4,trc4).nonDegenerate(3)= solveParOK();
	    cs().parErr(currSpw())(blc4,trc4).nonDegenerate(3)= solveParErr();
	    cs().parSNR(currSpw())(blc4,trc4).nonDegenerate(3)= solveParSNR();
	    cs().solutionOK(currSpw())(slotidx(thisSpw)) = anyEQ(solveParOK(),True);
	  } else {    
	    throw(AipsError("SVC::keep: Attempt to store solution in non-existent CalSet slot"));
	  }
	}
	//keep(slotidx(thisSpw));
      }// vi
      if (vi.moreChunks()) vi.nextChunk();
    } // chunk loop

    // is this required?
    setSpwOK();

    // progress indicator
    meter.update(Double(isim)/nSim);

  }// nsim loop

  if (calTableName()!="<none>") {      
    // RI todo SVC::setSimulate check if user wants to overwrite calTable
    os << LogIO::NORMAL 
       << "Writing calTable = "+calTableName()+" ("+typeName()+")" 
       << endl << LogIO::POST;      
    // write the table
    append()=False; 
    store();
  } else {
    os << LogIO::NORMAL 
       << "calTable name not set - not writing to disk." 
       << endl << LogIO::POST;
  }  
}







String SolvableVisCal::siminfo() {

  ostringstream o;
  o << "simulated " << typeName() 
    << ": output table="      << calTableName()
    << " simint="     << simint()
    << " t="          << interval();
  return String(o);
}

// ===================================================






String SolvableVisCal::applyinfo() {

  ostringstream o;
  o << typeName()
    << ": table="  << calTableName()
    << " select=" << calTableSelect()
    << " interp=" << tInterpType()
    << " spwmap=" << spwMap()
    << boolalpha
    << " calWt=" << calWt();
    //    << " t="      << interval();

  return String(o);

}


void SolvableVisCal::setSolve() {

  if (prtlev()>2) cout << "SVC::setSolve()" << endl;

  interval()=10.0;
  refant()=-1;
  apmode()="AP";
  calTableName()="<none>";
  solnorm()=False;
  minSNR()=0.0f;

  // This is the solve context
  setSolved(True);
  setApplied(False);
  setSimulated(False);

  // Create a pristine CalSet
  //  TBD: move this to inflate()?
  switch (parType())
    {
    case VisCalEnum::COMPLEX:
      {
	cs_ = new CalSet<Complex>(nSpw());
	cs().initCalTableDesc(typeName(),parType_);
	break;
      }
    case VisCalEnum::REAL:
      {
	rcs_ = new CalSet<Float>(nSpw());
	rcs().initCalTableDesc(typeName(),parType_);
	break;
      }
    default:
      throw(AipsError("Internal SVC::setSolve() error: Got invalide VisCalEnum"));
    }
}

void SolvableVisCal::setSolve(const Record& solve) 
{

  if (prtlev()>2) cout << "SVC::setSolve(solve)" << endl;

  // Collect parameters
  if (solve.isDefined("table"))
    calTableName()=solve.asString("table");

  if (calTableName().length()==0)
    throw(AipsError("Please specify a name for the output calibration table!"));

  if (solve.isDefined("solint")) 
    solint()=solve.asString("solint");

  if (upcase(solint()).contains("INF") || solint()=="") {
    solint()="inf";
    interval()=-1.0;
  }
  else if (upcase(solint()).contains("INT"))
    interval()=0.0;
  else {
    QuantumHolder qhsolint;
    String error;
    Quantity qsolint;
    qhsolint.fromString(error,solint());
    if (error.length()!=0)
      throw(AipsError("Unrecognized units for solint."));
    qsolint=qhsolint.asQuantumDouble();
    
    if (qsolint.isConform("s"))
      interval()=qsolint.get("s").getValue();
    else {
      // assume seconds
      interval()=qsolint.getValue();
      solint()=solint()+"s";
    }
  }
  
  if (solve.isDefined("preavg"))
    preavg()=solve.asFloat("preavg");

  if (solve.isDefined("refant"))
    refant()=solve.asInt("refant");

  if (solve.isDefined("minblperant"))
    minblperant()=solve.asInt("minblperant");

  if (solve.isDefined("apmode"))
    apmode()=solve.asString("apmode");
  apmode().upcase();

  if (solve.isDefined("append"))
    append()=solve.asBool("append");

  if (solve.isDefined("solnorm"))
    solnorm()=solve.asBool("solnorm");

  if (solve.isDefined("minsnr"))
    minSNR()=solve.asFloat("minsnr");

  if (solve.isDefined("combine"))
    combine()=solve.asString("combine");

  //  cout << "SVC::setsolve: minSNR() = " << minSNR() << endl;

  // TBD: Warn if table exists (and append=F)!

  // If normalizable & preavg<0, use full pre-averaging
  //  (or handle this per type, e.g. D)
  // TBD: make a nice log message concerning preavg
  if (normalizable() && preavg()<0.0)
    preavg()=DBL_MAX;

  // This is the solve context
  setSolved(True);
  setApplied(False);

  // Create a pristine CalSet
  //  TBD: move this to inflate()?
  switch (parType())
    {
    case VisCalEnum::COMPLEX:
      {
	cs_ = new CalSet<Complex>(nSpw());
	cs().initCalTableDesc(typeName(),parType_);
	break;
      }
    case VisCalEnum::REAL:
      {
	rcs_ = new CalSet<Float>(nSpw());
	rcs().initCalTableDesc(typeName(),parType_);
	break;
      }
    default:
      throw(AipsError("Internal SVC::setSolve(record) error: Got invalid VisCalEnum"));
    }
  //  state();

}

String SolvableVisCal::solveinfo() {

  // Get the refant name from the MS
  String refantName("none");
  /*
  if (refant()>-1) {
    MeasurementSet ms(msName());
    MSAntennaColumns mscol(ms.antenna());
    refantName=mscol.name()(refant());
  }
  */
  if (refant()>-1)
      refantName=csmi.getAntName(refant());

  ostringstream o;
  o << boolalpha
    << typeName()
    << ": table="      << calTableName()
    << " append="     << append()
    << " solint="     << solint()
    //    << " t="          << interval()
    //    << " preavg="     << preavg()
    << " refant="     << "'" << refantName << "'" // (id=" << refant() << ")"
    << " minsnr=" << minSNR()
    << " apmode="  << apmode()
    << " solnorm=" << solnorm();
  return String(o);

}


void SolvableVisCal::setAccumulate(VisSet& vs,
				   const String& table,
				   const String& select,
				   const Double& t,
				   const Int& refAnt) {

  LogMessage message(LogOrigin("SolvableVisCal","setAccumulate"));

  // meta-info
  calTableName()=table;
  calTableSelect()=select;
  interval()=t;

  // Not actually applying or solving
  setSolved(False);
  setApplied(False);

  // If interval<0, this signals an existing input cumulative table
  if (interval()<0.0) {

    logSink() << "Loading existing " << typeName()
	      << " table: " << table
	      << " for accumulation."
	      << LogIO::POST;

    // Create CalSet, from table
    switch (parType())
      {
      case VisCalEnum::COMPLEX:
	{
	  cs_ = new CalSet<Complex>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());
	  cs().initCalTableDesc(typeName(), parType_);
	  nChanParList() = cs().nChan();
	  startChanList() = cs().startChan();
	  break;
	}
      case VisCalEnum::REAL:
	{
	  rcs_ = new CalSet<Float>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());
	  rcs().initCalTableDesc(typeName(), parType_);
	  nChanParList() = rcs().nChan();
	  startChanList() = rcs().startChan();
	  break;
	}
      default:
	throw(AipsError("Internal SVC::setAccumulate(...) error: Got invalid VisCalEnum"));
      }


    // The following should be for trivial types only!    
    nChanMatList()=nChanParList();

  }

  // else, we are creating a cumulative table from scratch (the VisSet)
  else {

    logSink() << "Creating " << typeName()
	      << " table for accumulation."
	      << LogIO::POST;

    // Create a pristine CalSet
    //  TBD: move this to inflate()?
    switch (parType())
      {
      case VisCalEnum::COMPLEX:
	{
	  cs_ = new CalSet<Complex>(nSpw());
	  cs().initCalTableDesc(typeName(),parType_);

	  // Size, inflate CalSet (incl. filling meta data)
	  setSolveChannelization(vs);
	  // Override single-channel per solve! (trivial types only)
	  nChanMatList()=nChanParList();

	  inflate(vs,True);

	  // Set parOK,etc. to true
	  for (Int ispw=0;ispw<nSpw();ispw++) {
	    cs().parOK(ispw)=True;
	    cs().solutionOK(ispw)=True;
	  }
	  break;
	}
      case VisCalEnum::REAL:
	{
	  rcs_ = new CalSet<Float>(nSpw());
	  rcs().initCalTableDesc(typeName(),parType_);

	  // Size, inflate CalSet (incl. filling meta data)
	  setSolveChannelization(vs);
	  // Override single-channel per solve! (trivial types only)
	  nChanMatList()=nChanParList();

	  inflate(vs,True);

	  // Set parOK,etc. to true
	  for (Int ispw=0;ispw<nSpw();ispw++) {
	    rcs().parOK(ispw)=True;
	    rcs().solutionOK(ispw)=True;
	  }
	  break;
	}
      default:
	throw(AipsError("Internal SVC::setAccumulate(...) error: Got invalid VisCalEnum"));
      }
  }

}


void SolvableVisCal::setSpecify(const Record& specify) {

  LogMessage message(LogOrigin("SolvableVisCal","setSpecify"));

  // Not actually applying or solving
  setSolved(False);
  setApplied(False);

  // Collect Cal table parameters
  Bool tableExists(False);
  if (specify.isDefined("caltable")) {
    calTableName()=specify.asString("caltable");

    // Detect existence of the table
    tableExists=Table::isReadable(calTableName());

  }

  if (tableExists) {

    // Verify table has correct type
    verifyCalTable(calTableName());

    logSink() << "Loading existing " << typeName()
	      << " table: " << calTableName()
	      << "."
	      << LogIO::POST;

    // Create CalSet, from table
    switch (parType())
      {
      case VisCalEnum::COMPLEX:
	{
	  cs_ = new CalSet<Complex>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());
	  cs().initCalTableDesc(typeName(), parType_);
	  nChanParList() = cs().nChan();
	  startChanList() = cs().startChan();
	  // For now, check that table has only one slot...
	  for (Int ispw=0;ispw<nSpw();++ispw)
	    if (cs().nTime(ispw) != 1)
	      throw(AipsError("Cannot yet handle multi-timestamp calibration specification."));

	  break;
	}
      case VisCalEnum::REAL:
	{
	  rcs_ = new CalSet<Float>(calTableName(),calTableSelect(),nSpw(),nPar(),nElem());
	  rcs().initCalTableDesc(typeName(), parType_);
	  nChanParList() = rcs().nChan();
	  startChanList() = rcs().startChan();
	  // For now, check that table has only one slot...
	  for (Int ispw=0;ispw<nSpw();++ispw)
	    if (rcs().nTime(ispw) != 1)
	      throw(AipsError("Cannot yet handle multi-timestamp calibration specification."));
	  break;
	}
      default:
	throw(AipsError("Internal SVC::setSpecify(...) error: Got invalid VisCalEnum"));
      }
    
  }
  else {

    Vector<Int> nSlot(nSpw(),1);
    nChanParList()=Vector<Int>(nSpw(),1);
    startChanList()=Vector<Int>(nSpw(),0);
    
    // we are creating a table from scratch
    logSink() << "Creating " << typeName()
	      << " table from specified parameters."
	      << LogIO::POST;
    
    // Create a pristine CalSet
    switch (parType()) {
    case VisCalEnum::COMPLEX:
      {
	cs_ = new CalSet<Complex>(nSpw());
	cs().initCalTableDesc(typeName(),parType_);
	
	inflate(nChanParList(),startChanList(),nSlot);
	
	// Set parOK,etc. to true
	for (Int ispw=0;ispw<nSpw();ispw++) {
	  cs().par(ispw)=defaultPar();
	  cs().parOK(ispw)=True;
	  cs().solutionOK(ispw)=True;
	}
	break;
      }
    case VisCalEnum::REAL:
      {
	rcs_ = new CalSet<Float>(nSpw());
	rcs().initCalTableDesc(typeName(),parType_);
	
	inflate(nChanParList(),startChanList(),nSlot);
	
	// Set parOK,etc. to true
	for (Int ispw=0;ispw<nSpw();ispw++) {
	  rcs().par(ispw)=1.0;
	  rcs().parOK(ispw)=True;
	  rcs().solutionOK(ispw)=True;
	}
	break;
      }
    default:
      throw(AipsError("Internal SVC::setAccumulate(...) error: Got invalid VisCalEnum"));
    }
  }
}


void SolvableVisCal::specify(const Record& specify) {

  LogMessage message(LogOrigin("SolvableVisCal","specify"));

  Vector<Int> spws;
  Vector<Int> antennas;
  Vector<Int> pols;
  Vector<Double> parameters;

  Int Nspw(1);
  Int Ntime(1);
  Int Nant(0);
  Int Npol(1);
  
  Bool repspw(False);
  
  IPosition ip0(4,0,0,0,0);
  IPosition ip1(4,0,0,0,0);

  if (specify.isDefined("caltype")) {
    String caltype=specify.asString("caltype");
    if (upcase(caltype).contains("PH"))
      apmode()="P";
    else
      apmode()="A";
  }

 /*
  if (specify.isDefined("time")) {
    // TBD: the time label
    cout << "time = " << specify.asString("time") << endl;
    cout << "refTime() = " << refTime() << endl;
  }
 */

  if (specify.isDefined("spw")) {
    // TBD: the spws (in order) identifying the solutions
    spws=specify.asArrayInt("spw");
    cout << "spws = " << spws << endl;
    Nspw=spws.nelements();
    if (Nspw<1) {
      // None specified, so loop over all, repetitively
      //  (We need to optimize this...)
      cout << "Specified parameters repeated on all spws." << endl;
      repspw=True;
      Nspw=nSpw();
      spws.resize(Nspw);
      indgen(spws);
    }
  }


  if (specify.isDefined("antenna")) {
    // TBD: the antennas (in order) identifying the solutions
    antennas=specify.asArrayInt("antenna");
    //    cout << "antenna indices = " << antennas << endl;
    Nant=antennas.nelements();
    if (Nant<1) {
      // Use specified values for _all_ antennas implicitly
      Nant=1;   // For the antenna loop below
      ip0(2)=0;
      ip1(2)=nAnt()-1;
    }
    else {
      // Point to first antenna
      ip0(2)=antennas(0);
      ip1(2)=ip0(2);
    }
  }
  if (specify.isDefined("pol")) {
    // TBD: the pols (in order) identifying the solutions
    String polstr=specify.asString("pol");
    //    cout << "pol = " << polstr << endl;
    if (polstr=="R" || polstr=="X") 
      // Fill in only first pol
      pols=Vector<Int>(1,0);
    else if (polstr=="L" || polstr=="Y") 
      // Fill in only second pol
      pols=Vector<Int>(1,1);
    else if (polstr=="R,L" || polstr=="X,Y") {
      // Fill in both pols explicity
      pols=Vector<Int>(2,0);
      pols(1)=1;
    }
    else if (polstr=="L,R" || polstr=="Y,X") {
      // Fill in both pols explicity
      pols=Vector<Int>(2,0);
      pols(0)=1;
    }
    else if (polstr=="")
      // Fill in both pols implicitly
      pols=Vector<Int>();
    else
      throw(AipsError("Invalid pol specification"));
    
    Npol=pols.nelements();
    if (Npol<1) {
      // No pol axis specified
      Npol=1;
      ip0(0)=0;
      ip1(0)=nPar()-1;
    }
    else {
      // Point to the first polarization
      ip0(0)=pols(0);
      ip1(0)=ip0(0);
    }
  }
  if (specify.isDefined("parameter")) {
    // TBD: the actual cal values

    parameters=specify.asArrayDouble("parameter");

  }

  Int nparam=parameters.nelements();

  // Test for correct number of specified parameters
  //  Either all params are enumerated, or one is specified
  //  for all, [TBD:or a polarization pair is specified for all]
  //  else throw
  if (nparam!=(repspw ? (Ntime*Nant*Npol) : (Nspw*Ntime*Nant*Npol)) && 
      nparam!=1 )                // one for all
    //      (Npol==2 && nparam%2!=0) )  // poln pair for all
    throw(AipsError("Inconsistent number of parameters specified."));
  
  Int ipar(0);
  for (Int ispw=0;ispw<Nspw;++ispw) {
    // reset par index if we are repeating for all spws
    if (repspw) ipar=0;
    
    // Loop over specified timestamps
    for (Int itime=0;itime<Ntime;++itime) {
      ip1(3)=ip0(3)=itime;
      
      // Loop over specified antennas
      for (Int iant=0;iant<Nant;++iant) {
	if (Nant>1)
	  ip1(2)=ip0(2)=antennas(iant);
	
	// Loop over specified polarizations
	for (Int ipol=0;ipol<Npol;++ipol) {
	  if (Npol>1)
	    ip1(0)=ip0(0)=pols(ipol);
	  
	  Array<Complex> slice(cs().par(spws(ispw))(ip0,ip1));
	  // Default accumultion is multiplication
	  if (apmode()=="P") {
	    // Phases have been specified
	    Double phase=parameters(ipar)*C::pi/180.0;
	    slice*=Complex(cos(phase),sin(phase));
	  }
	  else
	    // Assume amplitude
	    slice*=Complex(parameters(ipar));


	  // increment ipar, but be sure not to run off the end
	  ++ipar;
	  ipar = ipar%nparam;

	}
      }
    }
  }
}



Int SolvableVisCal::sizeUpSolve(VisSet& vs, Vector<Int>& nChunkPerSol) {

  // New version that counts solutions (which may overlap in 
  //   field and/or ddid) rather than chunks

  Bool verby(False);

  // Set Nominal per-spw channelization
  setSolveChannelization(vs);

  // Interpret solution interval for the VisIter
  Double iterInterval(max(interval(),DBL_MIN));
  if (interval() < 0.0) {   // means no interval (infinite solint)
    iterInterval=0.0;
    interval()=DBL_MAX;
  }

  if (verby) {
    cout << "solint = " << interval() << endl;
    cout << boolalpha << "combscan() = " << combscan() << endl;
    cout << boolalpha << "combfld()  = " << combfld() << endl;
    cout << boolalpha << "combspw()  = " << combspw() << endl;
  }

  Int nsortcol(4+Int(!combscan()));  // include room for scan
  Block<Int> columns(nsortcol);
  Int i(0);
  columns[i++]=MS::ARRAY_ID;
  if (!combscan()) columns[i++]=MS::SCAN_NUMBER;  // force scan boundaries
  if (!combfld()) columns[i++]=MS::FIELD_ID;      // force field boundaries
  if (!combspw()) columns[i++]=MS::DATA_DESC_ID;  // force spw boundaries
  columns[i++]=MS::TIME;
  if (combspw() || combfld()) iterInterval=DBL_MIN;  // force per-timestamp chunks
  if (combfld()) columns[i++]=MS::FIELD_ID;      // effectively ignore field boundaries
  if (combspw()) columns[i++]=MS::DATA_DESC_ID;  // effectively ignore spw boundaries
  
  if (verby) {
    cout << "Sort columns: ";
    for (Int i=0;i<nsortcol;++i) 
      cout << columns[i] << " ";
    cout << endl;
  }

  vs.resetVisIter(columns,iterInterval);
  
  // Number of VisIter chunks per spw
  Vector<Int> nChunkPerSpw(vs.numberSpw(),0);

  // Number of VisIter chunks per solution
  nChunkPerSol.resize(100);
  nChunkPerSol=0;

  VisIter& vi(vs.iter());
  VisBuffer vb(vi);
  vi.originChunks();
  vi.origin();
    
  Double time0(86400.0*floor(vb.time()(0)/86400.0));
  Double time1(0.0),time(0.0);

  Int thisscan(-1),lastscan(-1);
  Int thisfld(-1),lastfld(-1);
  Int thisspw(-1),lastspw(-1);
  Int chunk(0);
  Int sol(-1);
  Double soltime1(-1.0);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk(),chunk++) {
    vi.origin();
    time1=vb.time()(0);  // first time in this chunk
    thisscan=vb.scan()(0);
    thisfld=vb.fieldId();
    thisspw=vb.spectralWindow();
    
    nChunkPerSpw(thisspw)++;

    // New chunk means new sol interval, IF....
    if ( (!combfld() && !combspw()) ||              // not combing fld nor spw, OR
	 ((time1-soltime1)>interval()) ||           // (combing fld and/or spw) and solint exceeded, OR
	 ((time1-soltime1)<0.0) ||                  // a negative time step occurs, OR
	 (!combscan() && (thisscan!=lastscan)) ||   // not combing scans, and new scan encountered OR
	 (!combspw() && (thisspw!=lastspw)) ||      // not combing spws, and new spw encountered  OR
	 (!combfld() && (thisfld!=lastfld)) ||      // not combing fields, and new field encountered OR 
	 (sol==-1))  {                              // this is the first interval
      soltime1=time1;
      sol++;
      if (verby) {
	cout << "--------------------------------" << endl;
	cout << "sol = " << sol << endl;
      }
      // increase size of nChunkPerSol array, if needed
      if (nChunkPerSol.nelements()<uInt(sol+1))
	nChunkPerSol.resize(nChunkPerSol.nelements()+100,True);
      nChunkPerSol(sol)=0;
    }

    // Increment chunk-per-sol count for current solution
    nChunkPerSol(sol)++;

    if (verby) {
      cout << "          ck=" << chunk << " " << soltime1-time0 << endl;
      
      Int iter(0);
      for (vi.origin(); vi.more();vi++,iter++) {
	time=vb.time()(0);
	cout  << "                 " << "vb=" << iter << " ";
	cout << "ar=" << vb.arrayId() << " ";
	cout << "sc=" << vb.scan()(0) << " ";
	if (!combfld()) cout << "fl=" << vb.fieldId() << " ";
	if (!combspw()) cout << "sp=" << vb.spectralWindow() << " ";
	cout << "t=" << floor(time-time0)  << " (" << floor(time-soltime1) << ") ";
	if (combfld()) cout << "fl=" << vb.fieldId() << " ";
	if (combspw()) cout << "sp=" << vb.spectralWindow() << " ";
	cout << endl;
      }
    }
    
    lastscan=thisscan;
    lastfld=thisfld;
    lastspw=thisspw;
    
  }
  
  if (verby) {
    cout << "nChunkPerSpw = " << nChunkPerSpw << " " << sum(nChunkPerSpw) << endl;
    cout << "nchunk = " << chunk << endl;
  }

  Int nSol(sol+1);
  
  nChunkPerSol.resize(nSol,True);
  
  spwMap().resize(vs.numberSpw());
  indgen(spwMap());
  Vector<Int> spwlist=spwMap()(nChunkPerSpw>0).getCompressedArray();
  Int spwlab=0;
  if (combspw()) {
    while (nChunkPerSpw(spwlab)<1) spwlab++;
    if (verby) cout << "Obtaining " << nSol << " solutions, labelled as spw=" << spwlab  << endl;
    spwMap()=-1;  // TBD: needed?
    spwMap()(nChunkPerSpw>0)=spwlab;

    if (verby)
      cout << "nChanParList = " << nChanParList()(nChunkPerSpw>0).getCompressedArray() 
	   << "==" << nChanParList()(spwlab) <<  endl;

    // Verify that all spws have same number of channels (so they can be combined!)
    if (!allEQ(nChanParList()(spwMap()>-1).getCompressedArray(),nChanParList()(spwlab)))
      throw(AipsError("Spws with different selected channelizations cannot be combined."));

    nChunkPerSpw = 0;
    nChunkPerSpw(spwlab)=nSol;
  }

  if (verby) {
    cout << " spwMap()  = " << spwMap() << endl;
    cout << " spwlist = " << spwlist << endl;
    cout << "nChunkPerSpw = " << nChunkPerSpw << " " << sum(nChunkPerSpw) << endl;
    cout << "Total solutions = " << nSol << endl;
    cout << "nChunkPerSol = " << nChunkPerSol << endl;
  }

  if (combscan())
    logSink() << "Combining scans." << LogIO::POST;
  if (combspw()) 
    logSink() << "Combining spws: " << spwlist << " -> " << spwlab << LogIO::POST;
  if (combfld()) 
    logSink() << "Combining fields." << LogIO::POST;
  

  //if (!isSimulated()) {
    logSink() << "For solint = " << solint() << ", found "
	      <<  nSol << " solution intervals."
	      << LogIO::POST;
  //}

  // Inflate the CalSet
  inflate(nChanParList(),startChanList(),nChunkPerSpw);

  // Size the solvePar arrays
  initSolvePar();

  // Return the total number of solution intervals
  return nSol;

}







Int SolvableVisCal::sizeUpSim(VisSet& vs, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes) {

  // New version that counts solutions (which may overlap in 
  //   field and/or ddid) rather than chunks
  LogIO os(LogOrigin("SVC", "sizeUpSim()", WHERE));

  // Interpret solution interval for the VisIter
  Double iterInterval(max(interval(),DBL_MIN));
  if (interval() < 0.0) {   // means no interval (infinite solint)
    iterInterval=0.0;
    interval()=DBL_MAX;
  }

  if (prtlev()>2) {
    cout << " simint = " << interval() ;
    cout << " combscan() = " << combscan();
    cout << " combfld()  = " << combfld() ;
    cout << " combspw()  = " << combspw() ;
  }

  Int nsortcol(4+Int(!combscan()));  // include room for scan
  Block<Int> columns(nsortcol);
  Int i(0);
  columns[i++]=MS::ARRAY_ID;
  if (!combscan()) columns[i++]=MS::SCAN_NUMBER;  // force scan boundaries
  if (!combfld()) columns[i++]=MS::FIELD_ID;      // force field boundaries
  if (!combspw()) columns[i++]=MS::DATA_DESC_ID;  // force spw boundaries
  columns[i++]=MS::TIME;
  if (combspw() || combfld()) iterInterval=DBL_MIN;  // force per-timestamp chunks
  if (combfld()) columns[i++]=MS::FIELD_ID;      // effectively ignore field boundaries
  if (combspw()) columns[i++]=MS::DATA_DESC_ID;  // effectively ignore spw boundaries
  
  if (prtlev()>2) {
    cout << " Sort columns: ";
    for (Int i=0;i<nsortcol;++i) 
      cout << columns[i] << " ";
    cout << endl;
  }

  // this sets the vi to send chunks by iterInterval (e.g. integration time)
  // instead of default which would go until the scan changed
  vs.resetVisIter(columns,iterInterval);
  
  // Number of VisIter chunks per spw
  Vector<Int> nChunkPerSpw(vs.numberSpw(),0);

  // Number of VisIter chunks per solution
  nChunkPerSol.resize(100);
  nChunkPerSol=0;

  VisIter& vi(vs.iter());
  VisBuffer vb(vi);
  vi.originChunks();
  vi.origin();
    
  Double time0(86400.0*floor(vb.time()(0)/86400.0));
  Double time1(0.0),time(0.0);

  Int thisscan(-1),lastscan(-1);
  Int thisfld(-1),lastfld(-1);
  Int thisspw(-1),lastspw(-1);
  Int chunk(0);
  Int sol(-1);
  Double soltime1(-1.0);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk(),chunk++) {
    vi.origin();
    time1=vb.time()(0);  // first time in this chunk
    thisscan=vb.scan()(0);
    thisfld=vb.fieldId();
    thisspw=vb.spectralWindow();
    
    nChunkPerSpw(thisspw)++;

    // New chunk means new sol interval, IF....
    if ( (!combfld() && !combspw()) ||              // not combing fld nor spw, OR
	 ((time1-soltime1)>interval()) ||           // (combing fld and/or spw) and solint exceeded, OR
	 ((time1-soltime1)<0.0) ||                  // a negative time step occurs, OR
	 (!combscan() && (thisscan!=lastscan)) ||   // not combing scans, and new scan encountered OR
	 (!combspw() && (thisspw!=lastspw)) ||      // not combing spws, and new spw encountered  OR
	 (!combfld() && (thisfld!=lastfld)) ||      // not combing fields, and new field encountered OR 
	 (sol==-1))  {                              // this is the first interval
      soltime1=time1;
      sol++;
      if (prtlev()>4) {
	cout << "--------------------------------" << endl;
	cout << "sol = " << sol << endl;
      }
      // increase size of nChunkPerSol array, if needed
      if (nChunkPerSol.nelements()<uInt(sol+1))
	nChunkPerSol.resize(nChunkPerSol.nelements()+100,True);
      nChunkPerSol(sol)=0;
      // keep the times!
      if (solTimes.nelements()<uInt(sol+1))
	solTimes.resize(solTimes.nelements()+100,True);
      solTimes(sol)=soltime1;
    }

    // Increment chunk-per-sol count for current solution
    nChunkPerSol(sol)++;

    if (prtlev()>4) {
      cout << "          ck=" << chunk << " " << soltime1-time0 << endl;
      
      Int iter(0);
      for (vi.origin(); vi.more();vi++,iter++) {
	time=vb.time()(0);
	cout  << "                 " << "vb=" << iter << " ";
	cout << "ar=" << vb.arrayId() << " ";
	cout << "sc=" << vb.scan()(0) << " ";
	if (!combfld()) cout << "fl=" << vb.fieldId() << " ";
	if (!combspw()) cout << "sp=" << vb.spectralWindow() << " ";
	cout << "t=" << floor(time-time0)  << " (" << floor(time-soltime1) << ") ";
	if (combfld()) cout << "fl=" << vb.fieldId() << " ";
	if (combspw()) cout << "sp=" << vb.spectralWindow() << " ";
	cout << endl;
      }
    }
    
    lastscan=thisscan;
    lastfld=thisfld;
    lastspw=thisspw;
    
  }
  
  Int nSol(sol+1);

  nChunkPerSol.resize(nSol,True);
  solTimes.resize(nSol,True);

  if (prtlev()>4) {
    cout << "    solTimes = " << solTimes-solTimes[0] << endl;
    cout << "nChunkPerSol = " << nChunkPerSol << " " << sum(nChunkPerSol) << endl;
  }

  // Set Nominal per-spw channelization - this does set chanParList to full
  // # chans
  setSolveChannelization(vs);
  if (prtlev()>3) cout<<" freqDepPar="<<freqDepPar()<<" nChanParList="<<nChanParList()<<endl;
  
  // makeCalSet(True);
  // do it ourselves in order to use nchanparlist just set in setsolvechan

  switch(parType())
    {
    case VisCalEnum::COMPLEX:
      {
	cs_ = new CalSet<Complex>(nSpw(),nPar(),nChanParList(),nElem(),Vector<Int>(nSpw(),nSol));
	cs().initCalTableDesc(typeName(),parType_);
	
	// Create CalInterp
	cint_ = new CalInterp(cs(),tInterpType(),"nearest");
	ci().setSpwMap(spwMap());
	break;
      }
    case VisCalEnum::REAL:
      {
	rcs_ = new CalSet<Float>(nSpw(),nPar(),nChanParList(),nElem(),Vector<Int>(nSpw(),nSol));
	rcs().initCalTableDesc(typeName(),parType_);
	// Create CalInterp
	// 	cint_ = new CalInterp(rcs(),tInterpType(),"nearest");
	// 	ci().setSpwMap(spwMap());
	break;
      }
    default:
      throw(AipsError("Internal error(Calibrater Module): Unsupported parameter type "
		      "COMPLEXREAL found in SolvableVisCal::makeCalSet()"));
    }
  
  if (prtlev()>3) cout<<" calset of size "<<cs().par(currSpw()).shape()<<" created"<<endl;

  spwMap().resize(vs.numberSpw());
  indgen(spwMap());
  Vector<Int> spwlist=spwMap()(nChunkPerSpw>0).getCompressedArray();
  Int spwlab=0;
  if (combspw()) {
    while (nChunkPerSpw(spwlab)<1) spwlab++;
    if (prtlev()>2) cout << "Obtaining " << nSol << " solutions, labelled as spw=" << spwlab  << endl;
    spwMap()=-1;  // TBD: needed?
    spwMap()(nChunkPerSpw>0)=spwlab;

    if (prtlev()>2)
      cout << "nChanParList = " << nChanParList()(nChunkPerSpw>0).getCompressedArray() 
	   << "==" << nChanParList()(spwlab) <<  endl;

    // Verify that all spws have same number of channels (so they can be combined!)
    if (!allEQ(nChanParList()(spwMap()>-1).getCompressedArray(),nChanParList()(spwlab)))
      throw(AipsError("Spws with different selected channelizations cannot be combined."));

    nChunkPerSpw = 0;
    nChunkPerSpw(spwlab)=nSol;
  }

  if (prtlev()>2) {
    cout << " spwMap()  = " << spwMap() ;
    cout << " spwlist = " << spwlist ;
    cout << " nChunkPerSpw = " << nChunkPerSpw << " " << sum(nChunkPerSpw) << " = " << nSol << endl;
    //cout << "Total solutions = " << nSol << endl;
  }
  if (prtlev()>4) 
    cout << "nChunkPerSim = " << nChunkPerSol << endl;
  
  
  if (combscan())
    os << "Combining scans." << LogIO::POST;
  if (combspw()) 
    os << "Combining spws: " << spwlist << " -> " << spwlab << LogIO::POST;
  if (combfld()) 
    os << "Combining fields." << LogIO::POST;
  
  os << "For simint = " << simint() << ", found "
     <<  nSol << " solution intervals."
     << LogIO::POST;
  
  // Inflate the CalSet - RI TODO redundant with create above?
  inflate(nChanParList(),startChanList(),nChunkPerSpw);

  if (prtlev()>3) cout<<" calset inflated to "<<cs().par(currSpw()).shape()<<" created"<<endl;
  
  // Size the solvePar arrays
  // fuckin' eh - Jones' insists on having 1 channel, but mullers have lots.  
  //initSolvePar();
  // so i have to copy this from initsolvepar and make it all chans

  for (Int ispw=0;ispw<nSpw();++ispw) {
    
    currSpw()=ispw;

    switch(parType())
      {
      case VisCalEnum::COMPLEX:
	{
	    solveCPar().resize(nPar(),nChanPar(),nElem());
	   solveParOK().resize(nPar(),nChanPar(),nElem());
	  solveParErr().resize(nPar(),nChanPar(),nElem());
	  solveParSNR().resize(nPar(),nChanPar(),nElem());
	  	  
	  solveCPar()=Complex(1.0);
	  solveParOK()=True;
	  solveParErr()=0.0;
	  solveParSNR()=0.0;
	  break;
	}
      case VisCalEnum::REAL:
	{
	    solveRPar().resize(nPar(),nChanPar(),nElem());
	   solveParOK().resize(nPar(),nChanPar(),nElem());
	  solveParErr().resize(nPar(),nChanPar(),nElem());
	  solveParSNR().resize(nPar(),nChanPar(),nElem());
	  
	  solveRPar()=0.0;
	  solveParOK()=True;
	  solveParErr()=0.0;
	  solveParSNR()=0.0;
	  break;
	}
      case VisCalEnum::COMPLEXREAL:
         throw(AipsError("Internal error(Calibrater Module): Unsupported parameter type COMPLEXREAL found"));
         break;
      }
  }

  // cout << "calset shape = " << cs().shape(0) << " solveCPar shape = " << solveCPar().shape() << endl;

  // Return the total number of solution intervals
  return nSol;

}














void SolvableVisCal::initSolve(VisSet& vs) {

  if (prtlev()>2) cout << "SVC::initSolve(vs)" << endl;

  // Determine solving channelization according to VisSet & type
  setSolveChannelization(vs);

  // Nominal spwMap in solve is identity
  spwMap().resize(vs.numberSpw());
  indgen(spwMap());

  // Inflate the CalSet according to VisSet
  inflate(vs);

  switch (parType())
    {
    case VisCalEnum::COMPLEX:
      {
	cs().initCalTableDesc(typeName(), parType_);
	break;
      }
    case VisCalEnum::REAL:
      {
	rcs().initCalTableDesc(typeName(), parType_);
	break;
      }
    default:
      throw(AipsError("Internal SVC::initSolve(vs) error: Got invalid VisCalEnum"));
    }

  // Size the solvePar arrays
  initSolvePar();

}


// Inflate the internal CalSet according to VisSet info
void SolvableVisCal::inflate(VisSet& vs, const Bool& fillMeta) {

  if (prtlev()>3) cout << " SVC::inflate(vs)" << endl;

  // This method sets up various shape parameters
  //  according to the current VisSet.  It is necessary
  //  to run this after Calibrater::setdata and before
  //  the main part of the solve.  (In case the setdata
  //  was run after the setsolve.)  This method calls
  //  a generic version to interpret the data shapes
  //  in the proper context-dependent way and size
  //  the CalSet.

  //  TBD: Move this slot counting exercise out to Calibrater?
  //   --> Not clear we should do this...

  //  TBD: How do we generalize this to create bracketing
  //   slots at scan start/stop (for accumulation context)?

  // Count slots in the VisIter
  Vector<Int> nSlot(nSpw(),0);
  {
    VisIter& vi(vs.iter());
    for (vi.originChunks(); vi.moreChunks(); vi.nextChunk())
      nSlot(vi.spectralWindow())++;
    vi.originChunks();

    logSink() << "For interval of "<<interval()<<" seconds, found "
	      <<  sum(nSlot)<<" slots"
	      << LogIO::POST;
  }

  // Call generic version to actually inflate the CalSet
  //  (assumes nChanParList()/startChanList() already valid!)
  inflate(nChanParList(),startChanList(),nSlot);

  // Fill the meta data in the CalSet (according to VisSet)
  //  (NB: currently, only used in accumulate context)
  if (fillMeta) fillMetaData(vs);

}



// Inflate the internal CalSet generically
void SolvableVisCal::inflate(const Vector<Int>& nChan,
			     const Vector<Int>& startChan,
			     const Vector<Int>& nSlot) {

  if (prtlev()>3) cout << "  SVC::inflate(,,)" << endl;

  // Size up the CalSet
  
  switch (parType())
    {
    case VisCalEnum::COMPLEX:
      {
	cs().resize(nPar(),nChan,nElem(),nSlot);
	cs().setStartChan(startChan);
	break;
      }
    case VisCalEnum::REAL:
      {
	rcs().resize(nPar(),nChan,nElem(),nSlot);
	rcs().setStartChan(startChan);
	break;
      }
    default:
      throw(AipsError("Internal SVC::inflate(...) error: Got invalide VisCalEnum"));
    }
}


void SolvableVisCal::setSolveChannelization(VisSet& vs) {

  //  TBD: include anticipated decimation when partial freq ave supported?
  //      (NB: note difference between chan-ave on selection [VisSet] and
  //       chan-ave on-the-fly with vb.freqAve())


  Vector<Int> nDatChan(vs.numberChan());
  Vector<Int> startDatChan(vs.startChan());

  // Figure out channel axis shapes (solve context!):

  // If multi-channel pars, this is a frequency-sampled calibration (e.g., B)
  if (freqDepPar()) {
    // Overall par shape follows data shape
    nChanParList() = nDatChan;
    startChanList() = startDatChan;

    // However, for solving, we will only consider one channel at a time:
    nChanMatList() = 1;

  }
  else {
    // Pars are not themselves channel-dependent
    nChanParList() = 1;

    // Check if matrices may still be freq-dep:
    if (freqDepMat()) {
      // cal is an explicit f(freq) (e.g., like delay)
      nChanMatList()  = nDatChan;
      startChanList() = startDatChan;
    } else {
      // cal has no freq dep at all
      nChanMatList()  = Vector<Int>(nSpw(),1);
      startChanList() = Vector<Int>(nSpw(),0);
    }

  }

  // At this point:
  //  1. nChanParList() represents the (per-Spw) overall length of the
  //     output parameter channel axis, appropriate for shaping the
  //     CalSet.  This value is irrelevant during the solve, since
  //     we will only solve for one parameter channel at a time (or 
  //     there is only one channel to solver for).
  //  2. nChanMatList() represents the per-Spw  matrix channel axis length to
  //     be used during the solve, independent of the parameter channel
  //     axis length.  In the solve context, nChanMat()>1 when there is
  //     more than one channel of data upon which the (single channel)
  //     solve parameters depend (e.g., delay, polynomial bandpass, etc.)

}


// Fill the Calset with meta data
void SolvableVisCal::fillMetaData(VisSet& vs) {

  if (prtlev()>3) cout << "  SVC::fillMetaData(vs)" << endl;

  // NB: Currently, this is only used for the accumulate
  //     context; in solve, meta-data is filled on-the-fly
  //     (this ensures more accurate timestamps, etc.)

  // NB: Assumes CalSet already has correct shape!

  // TODO:
  // 1. Can we use setRowBlocking() here to avoid in-chunk iterations?

  // Fill the Calset with meta info
  VisIter& vi(vs.iter());
  vi.originChunks();
  VisBuffer vb(vi);
  Vector<Int> islot(nSpw(),0);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

    Int thisSpw=vi.spectralWindow();

    if (islot(thisSpw)>=cs().nTime(thisSpw))
      throw(AipsError("Error filling CalSet with Meta data."));
	      
    // Set field source info
    cs().fieldId(thisSpw)(islot(thisSpw))  =vi.fieldId();
    cs().fieldName(thisSpw)(islot(thisSpw)) =vi.fieldName();
    cs().sourceName(thisSpw)(islot(thisSpw))=vi.sourceName();

    // Set time info
    vi.origin();
    cs().startTime(thisSpw)(islot(thisSpw))  =vb.time()(0)-vb.timeInterval()(0)/2.0;
    Double timeStamp(0.0);
    Int ntime(0);
    for (vi.origin(); vi.more(); vi++,ntime++) {
      timeStamp+=vb.time()(0);
    }
    cs().stopTime(thisSpw)(islot(thisSpw)) =vb.time()(0)+vb.timeInterval()(0)/2.0;
    cs().time(thisSpw)(islot(thisSpw))=timeStamp/ntime;

    // Advance slot counter (per spw)
    islot(thisSpw)++;

  }

}


Bool SolvableVisCal::syncSolveMeta(VisBuffGroupAcc& vbga) {

  // Adopt meta data from FIRST CalVisBuffer in the VBGA, for now
  currSpw()=spwMap()(vbga(0).spectralWindow());
  currField()=vbga(0).fieldId();
  
  // The timestamp really is global, in any case
  Double& rTime(vbga.globalTimeStamp());
  if (rTime > 0.0) {
    refTime()=rTime;
    return True;
  }
  else
    return False;

}

Bool SolvableVisCal::syncSolveMeta(VisBuffer& vb, 
				   const Int& fieldId) {

  if (prtlev()>2) cout << "SVC::syncSolveMeta(,,)" << endl;

  // Returns True, only if sum of weights is positive,
  //  i.e., there is data to solve with

  // TBD: freq info, etc.

  currSpw()=spwMap()(vb.spectralWindow());
  currField()=vb.fieldId();

  // Row weights as a Doubles
  Vector<Double> dWts;
  dWts.resize(vb.weight().shape());
  convertArray(dWts,vb.weight());
  Vector<Double> times;
  times = vb.time();
  
  // The following assumes flagRow is accurate
  LogicalArray gRows(!vb.flagRow());
  Double sumWts(0.0);
  MaskedArray<Double> gTimes(times,gRows);
  MaskedArray<Double> gWts(dWts,gRows);

  if (sum(gRows)>0) {
    sumWts=sum(gWts);
  }

  if (sumWts>0.0) {
    gTimes*=gWts;
    refTime()=sum(gTimes);
    refTime()/=sumWts;
    return True;
  }
  else
    return False;

}

void SolvableVisCal::enforceAPonData(VisBuffer& vb) {

  // TBD: migrate this to VisEquation?

  if (apmode()!="AP") {
    Int nCorr(vb.corrType().nelements());
    Float amp(1.0);
    Complex cor(1.0);
    Bool *flR=vb.flagRow().data();
    Bool *fl =vb.flag().data();
    Vector<Float> ampCorr(nCorr);
    Vector<Int> n(nCorr,0);
    for (Int irow=0;irow<vb.nRow();++irow,++flR) {
      if (!vb.flagRow()(irow)) {
	ampCorr=0.0f;
	n=0;
	for (Int ich=0;ich<vb.nChannel();++ich,++fl) {
	  if (!vb.flag()(ich,irow)) {
	    for (Int icorr=0;icorr<nCorr;icorr++) {
	      
	      amp=abs(vb.visCube()(icorr,ich,irow));
	      if (amp>0.0f) {
		
		if (apmode()=="P")
		  // we will scale by amp to make data phase-only
		  cor=Complex(amp,0.0);
		else if (apmode()=="A")
		  // we will scale by "phase" to make data amp-only
		  cor=vb.visCube()(icorr,ich,irow)/amp;
		
		// Apply the complex scaling and count
		vb.visCube()(icorr,ich,irow)/=cor;
		ampCorr(icorr)+=amp;
		n(icorr)++;
	      }
	    } // icorr
	  } // !*fl
	} // ich
	// Make appropriate weight adjustment
	for (Int icorr=0;icorr<nCorr;icorr++)
	  if (n(icorr)>0)
	    // weights adjusted by square of the mean(amp)
	    vb.weightMat()(icorr,irow)*=square(ampCorr(icorr)/Float(n(icorr)));
	  else
	    // weights now zero
	    vb.weightMat()(icorr,irow)=0.0f;
      } // !*flR
    } // irow

  } // phase- or amp-only

  //  cout << "amp(vb.visCube())=" << amplitude(vb.visCube().reform(IPosition(1,vb.visCube().nelements()))) << endl;


}

void SolvableVisCal::setUpForPolSolve(VisBuffer& vb) {

  // TBD: migrate this to VisEquation?
  
  // Divide model and data by (scalar) stokes I (which may be resolved!), 
  //  and set model cross-hands to (1,0) so we can solve for fractional
  //  pol factors.

  // Only if solving for Q an U
  //  (leave cross-hands alone if just solving for X)
  if (solvePol()>1) {

    Int nCorr(vb.corrType().nelements());
    Bool *flR=vb.flagRow().data();
    Bool *fl =vb.flag().data();
    Vector<Float> ampCorr(nCorr);
    Vector<Int> n(nCorr,0);
    Complex sI(0.0);
    for (Int irow=0;irow<vb.nRow();++irow,++flR) {
      if (!vb.flagRow()(irow)) {
	ampCorr=0.0f;
	n=0;
	for (Int ich=0;ich<vb.nChannel();++ich,++fl) {
	  if (!vb.flag()(ich,irow)) {
	    
	    sI=(vb.modelVisCube()(0,ich,irow)+vb.modelVisCube()(3,ich,irow))/Complex(2.0);
	    if (abs(sI)>0.0) {
	      for (Int icorr=0;icorr<nCorr;icorr++) {
		vb.visCube()(icorr,ich,irow)/=sI;
		ampCorr(icorr)+=abs(sI);
		n(icorr)++;
	      } // icorr
	    }
	    else
	      vb.flag()(ich,irow)=True;
	    
	  } // !*fl
	} // ich
	// Make appropriate weight adjustment
	for (Int icorr=0;icorr<nCorr;icorr++)
	  if (n(icorr)>0)
	    // weights adjusted by square of the mean(amp)
	    vb.weightMat()(icorr,irow)*=square(ampCorr(icorr)/Float(n(icorr)));
	  else
	    // weights now zero
	    vb.weightMat()(icorr,irow)=0.0f;
      } // !*flR
    } // irow
    
    // Model is now all unity  (Is this ok for flagged data? Probably.)
    vb.modelVisCube()=Complex(1.0);

  }

}

Bool SolvableVisCal::verifyConstraints(VisBuffGroupAcc& vbag) {

  // TBD: handle multi-channel infocusFlag properly
  // TBD: optimize array access
  
  // Assemble nominal baseline weights distribution
  Matrix<Double> blwtsum(nAnt(),nAnt(),0.0);
  for (Int ivb=0;ivb<vbag.nBuf();++ivb) {
    CalVisBuffer& cvb(vbag(ivb));

    cvb.setFocusChan(focusChan());

    for (Int irow=0;irow<cvb.nRow();++irow) {
      Int& a1(cvb.antenna1()(irow));
      Int& a2(cvb.antenna2()(irow));
      if (!cvb.flagRow()(irow) && a1!=a2) {
	if (!cvb.infocusFlag()(0,irow)) {
	  Double wt=Double(sum(cvb.weightMat().column(irow)));
	  blwtsum(a2,a1)+=wt;
	  blwtsum(a1,a2)+=wt;
	} // flag
      } // flagRow 
    } // irow
  } // ivb

  // Recursively apply threshold on baselines per antenna
  //  Currently, we insist on at least 3 baselines per antenna
  //  (This will eventually be a user-specified parameter: blperant)
  Vector<Bool> antOK(nAnt(),True);  // nominally OK
  Vector<Int> blperant(nAnt(),0);
  Int iant=0;
  while (iant<nAnt()) {
    if (antOK(iant)) {   // avoid reconsidering already bad ones
      Int nbl=ntrue(blwtsum.column(iant)>0.0);
      blperant(iant)=nbl;
      if (nbl<minblperant()) {
	// some baselines available, but not enough
	//  so eliminate this antenna 
	antOK(iant)=False;
	blwtsum.row(iant)=0.0;
	blwtsum.column(iant)=0.0;
	blperant(iant)=0;
	// ensure we begin recount at first antenna again
	iant=-1;
      }
    }      
    ++iant;
  }

  //  cout << "  blperant = " << blperant << " (minblperant = " << minblperant() << endl;
  //  cout << "  antOK    = " << antOK << endl;
  //  cout << "  ntrue(antOK) = " << ntrue(antOK) << endl;

  // Apply constraints results to solutions and data
  solveParOK()=False;   // Solutions nominally bad
  for (Int iant=0;iant<nAnt();++iant) {
    if (antOK(iant)) {
      // set solution good
      solveParOK().xyPlane(iant) = True;
    }
    else {
      // This ant not ok, set soln to zero
      if (parType()==VisCalEnum::COMPLEX)
      	solveCPar().xyPlane(iant)=1.0;
      else if (parType()==VisCalEnum::REAL)
      	solveRPar().xyPlane(iant)=0.0;

      // Flag corresponding data
      for (Int ivb=0;ivb<vbag.nBuf();++ivb) {
	CalVisBuffer& cvb(vbag(ivb));
	Vector<Int>& a1(cvb.antenna1());
	Vector<Int>& a2(cvb.antenna2());
	for (Int irow=0;irow<cvb.nRow();++irow) {
	  if (a1(irow)==iant || a2(irow)==iant)
	    cvb.infocusFlag()(0,irow)=True;
	}
	// the following didn't work because row(0) behaved
	//  as contiguous and set the wrong flags for multi-chan data!
	//	cvb.infocusFlag().row(0)(a1==iant)=True;
	//	cvb.infocusFlag().row(0)(a2==iant)=True;
      } // ivb

    } // antOK
  } // iant
  
  // We return sum(antOK)>0 here because it is not how many 
  //  good ants there are, but rather how many good baselines 
  //  per ant there are.  The above counting exercise will 
  //  reduce sum(antOK) to zero when the baseline counts 
  //  constraint is violated over enough of the whole array.  
  //  so as to make the solution impossible.  Otherwise
  //  there will be at least blperant+1 (>0) good antennas.

  return (ntrue(antOK)>0);

}

// Verify VisBuffer data sufficient for solving (wts, etc.)
Bool SolvableVisCal::verifyForSolve(VisBuffer& vb) {

  //  cout << "verifyForSolve..." << endl;

  Int nAntForSolveFinal(-1);
  Int nAntForSolve(0);

  // We will count baselines and weights per ant
  //   and set solveParOK accordingly
  Vector<Int> blperant(nAnt(),0);
  Vector<Double> wtperant(nAnt(),0.0);
  Vector<Bool> antOK(nAnt(),False);
    
  while (nAntForSolve!=nAntForSolveFinal) {

    nAntForSolveFinal=nAntForSolve;
    nAntForSolve=0;


    // TBD: optimize indexing with pointers in the following
    blperant=0;
    wtperant=0.0;

    for (Int irow=0;irow<vb.nRow();++irow) {
      Int a1=vb.antenna1()(irow);
      Int a2=vb.antenna2()(irow);
      if (!vb.flagRow()(irow) && a1!=a2) {
	
	if (!vb.flag()(focusChan(),irow)) {
	  
	  blperant(a1)+=1;
	  blperant(a2)+=1;
	  
	  wtperant(a1)+=Double(sum(vb.weightMat().column(irow)));
	  wtperant(a2)+=Double(sum(vb.weightMat().column(irow)));
	  
	}
      }
    }
    
    antOK=False;
    for (Int iant=0;iant<nAnt();++iant) {
      if (blperant(iant)>3 &&
	  wtperant(iant)>0.0) {
	// This antenna is good, keep it
	nAntForSolve+=1;
	antOK(iant)=True;
      }
      else {
	// This antenna under-represented; flag it
	vb.flag().row(focusChan())(vb.antenna1()==iant)=True;
	vb.flag().row(focusChan())(vb.antenna2()==iant)=True;
	//	vb.flagRow()(vb.antenna1()==iant)=True;
	//	vb.flagRow()(vb.antenna2()==iant)=True;
      }
    }

    //    cout << "blperant     = " << blperant << endl;
    //    cout << "wtperant = " << wtperant << endl;
    //    cout << "nAntForSolve = " << nAntForSolve << " " << antOK << endl;

  }

  // We've converged on the correct good antenna count
  nAntForSolveFinal=nAntForSolve;

  // Set a priori solution flags  
  solveParOK() = False;
  for (Int iant=0;iant<nAnt();++iant)
    if (antOK(iant))
      // This ant ok
      solveParOK().xyPlane(iant) = True;
    else
      // This ant not ok, set soln to zero
      if (parType()==VisCalEnum::COMPLEX)
      	solveCPar().xyPlane(iant)=1.0;
      else if (parType()==VisCalEnum::REAL)
      	solveRPar().xyPlane(iant)=0.0;
  //  cout << "antOK = " << antOK << endl;
  //  cout << "solveParOK() = " << solveParOK() << endl;
  //  cout << "amp(solveCPar()) = " << amplitude(solveCPar()) << endl;

  if (nAntForSolve<4) cout << "Only " << nAntForSolve 
			   << "/" << nAnt() 
			   << " antennas (" 
			   << floor(100*Float(nAntForSolve/nAnt()))
			   << "%) have sufficient baselines at " 
			   << MVTime(refTime()/C::day).string(MVTime::YMD,7)
			   << endl;
  return (nAntForSolve>3);
    
}

void SolvableVisCal::selfSolve(VisSet& vs, VisEquation& ve) {
    
  if (standardSolve())
    throw(AipsError("Spurious call to selfSolve()."));
  else
    throw(AipsError("Attempt to call un-implemented selfSolve()"));

}
void SolvableVisCal::selfSolve2(VisBuffGroupAcc& vbga) {
    
  if (standardSolve())
    throw(AipsError("Spurious call to selfSolve()."));
  else
    throw(AipsError("Attempt to call un-implemented selfSolve()"));

}

     
void SolvableVisCal::updatePar(const Vector<Complex> dCalPar,const Vector<Complex> dSrcPar) {

  AlwaysAssert((solveCPar().nelements()==dCalPar.nelements()),AipsError);

  Cube<Complex> dparcube(dCalPar.reform(solveCPar().shape()));

  // zero flagged increments
  dparcube(!solveParOK())=Complex(0.0);
  
  // Add the increment
  solveCPar()+=dparcube;

  // Update source params
  if (solvePol()) {
    srcPolPar()+=dSrcPar;
    //    cout << "Updated Q,U = " << real(srcPolPar()) << endl;
  }

  // The matrices are nominally out-of-sync now
  invalidateCalMat();

  // Ensure phaseonly-ness, if necessary
  //  if (apmode()=='P') {   
  //  NB: Disable this, for the moment (07May24); testing a fix for
  //      a problem Kumar noticed.  See VC::makeSolnPhaseOnly(), etc.
  if (False) {
    Float amp(0.0);
    for (Int iant=0;iant<nAnt();++iant) {
      for (Int ipar=0;ipar<nPar();++ipar) {
	if (solveParOK()(ipar,0,iant)) {
	  amp=abs(solveCPar()(ipar,0,iant));
	  if (amp>0.0)
	    solveCPar()(ipar,0,iant)/=amp;
	}
      }
    }
  }
}

void SolvableVisCal::formSolveSNR() {

  // Nominally zero
  solveParSNR()=0.0;

  for (Int iant=0;iant<nAnt();++iant)
    for (Int ipar=0;ipar<nPar();++ipar) {
      if (solveParOK()(ipar,0,iant))
	if (solveParErr()(ipar,0,iant)>0.0f) 
	  solveParSNR()(ipar,0,iant)=abs(solveCPar()(ipar,0,iant))/solveParErr()(ipar,0,iant);
	else 
	  // if error is zero, SNR is officially infinite; use a (large) special value here
	  solveParSNR()(ipar,0,iant)=9999999.0;
    }
}

void SolvableVisCal::applySNRThreshold() {

  Int nOk1(ntrue(solveParOK()));
  
  for (Int iant=0;iant<nAnt();++iant)
    for (Int ipar=0;ipar<nPar();++ipar)
      if (solveParOK()(ipar,0,iant))
	solveParOK()(ipar,0,iant)=(solveParSNR()(ipar,0,iant)>minSNR());
  
  Int nOk2(ntrue(solveParOK()));
  Int nFail=nOk1-nOk2;    
  
  if (False) {
    // Report some stuff re SNR
    cout << endl 
	 << "Time = " << MVTime(refTime()/C::day).string(MVTime::YMD,7) << endl;
    cout << "SNR      = " << solveParSNR() << endl;
    cout << nOk1 << " " << nOk2 << " " << nFail << endl;
    Float meansnr(0.0f);
    if (ntrue(solveParOK())>0) {
      meansnr=mean(solveParSNR()(solveParOK()));
      cout << "mean solution SNR = " << meansnr
	   << " (passing threshold)."
	   << endl;
    }
  }
  
  if (nFail>0)
    cout << nFail << " of " << nOk1
	 << " solutions rejected due to SNR < " << minSNR() 
	 << " in spw=" << currSpw()
	 << " at " << MVTime(refTime()/C::day).string(MVTime::YMD,7)
	 << endl;
  
}

void SolvableVisCal::smooth(Vector<Int>& fields,
			    const String& smtype,
			    const Double& smtime) {

  if (smoothable()) 
    // Call CalSet's global smooth method
    casa::smooth(cs(),smtype,smtime,fields);
  else
    throw(AipsError("This type does not support smoothing!"));

}


void SolvableVisCal::syncPar(const Int& spw, const Int& slot) {

  if (prtlev()>4) cout << "    SVC::syncPar(spw,slot)" << endl;

  IPosition blc(4,0,0,0,slot);
  IPosition trc(4,cs().nPar()-1,cs().nChan(spw)-1,cs().nElem()-1,slot);

  currCPar().reference(cs().par(spw)(blc,trc).nonDegenerate(3));;
  currParOK().reference(cs().parOK(spw)(blc,trc).nonDegenerate(3));

  validateP();
  invalidateCalMat();

}

void SolvableVisCal::syncSolveCal() {

  if (prtlev()>4) cout << "    SVC::syncSolveCal()" << endl;
  
  // Ensure parameters are ready
  syncSolvePar();

  if (!PValid())
    throw(AipsError("No valid parameters in syncSolveCal"));

  // Sync up matrices using current params
  syncCalMat(False);    // NEVER invert in solve context
  syncDiffMat();

}
void SolvableVisCal::syncSolvePar() {

  if (prtlev()>5) cout << "      SVC::syncSolvePar()" << endl;

  // In solve context, reference solveCPar(), etc.
  AlwaysAssert((solveCPar().nelements()>0 || solveRPar().nelements()>0),AipsError);
  currCPar().reference(solveCPar());
  currRPar().reference(solveRPar());
  currParOK().reference(solveParOK());
  validateP();

}

void SolvableVisCal::calcPar() {

  if (prtlev()>6) cout << "      SVC::calcPar()" << endl;

  // This method is relevant only to the apply context

  Bool newcal(False);

  // Interpolate solution
  newcal=ci().interpolate(currTime(),currSpw());

  // TBD: signal failure to find calibration??  (e.g., for a spw?)

  // Parameters now valid (independent of newcal!!)
  //  (TBD: should have ci() to tell us this?)
  validateP();

  // If new cal parameters, use them 
  if (newcal) {

    //    cout << "Found new cal!" << endl;
    
    // Reference result
    currCPar().reference(ci().result());
    currParOK().reference(ci().resultOK());

    // For now, assume all OK
    //    currParOK().resize(nPar(),nChanPar(),nElem()); currParOK()=True;

    // In case we need solution timestamp
    refTime() = ci().slotTime();

    // If new parameters, matrices (generically) are necessarily invalid now
    invalidateCalMat();
  }

}

// Report solved-for QU
void SolvableVisCal::reportSolvedQU() {

  /*
  MeasurementSet ms(msName());
  MSFieldColumns msfldcol(ms.field());
  String fldname(msfldcol.name()(currField()));
  */
  String fldname(csmi.getFieldName(currField()));

  if (solvePol()==2) {
    Float Q=real(srcPolPar()(0));
    Float U=real(srcPolPar()(1));
    Float P=sqrt(Q*Q + U*U);
    Float X=atan2(U,Q)/2.0*180.0/C::pi;

    logSink() << "Fractional polarization solution for " << fldname
	      << " (spw = " << currSpw() << "): "
	      << ": Q = " << Q
	      << ", U = " << U
	      << "  (P = " << P
	      << ", X = " << X << " deg)"
	      << LogIO::POST;
  }
  else if (solvePol()==1) {
    logSink() << "Position angle offset solution for " << fldname
	      << " (spw = " << currSpw() << ") = "
	      << real(srcPolPar()(0))*180.0/C::pi/2.0
	      << " deg."
	      << LogIO::POST;
  }
}

// File a solved solution (and meta-data) into a slot in the CalSet
void SolvableVisCal::keep(const Int& slot) {

  if (prtlev()>4) cout << " SVC::keep(i)" << endl;

  if (slot<cs().nTime(currSpw())) {
    // An available valid slot

    cs().fieldId(currSpw())(slot)=currField();
    cs().time(currSpw())(slot)=refTime();

    // Only stop-start diff matters
    //  TBD: change CalSet to use only the interval
    //  TBD: change VisBuffAcc to calculate exposure properly
    cs().startTime(currSpw())(slot)=0.0;
    cs().stopTime(currSpw())(slot)=interval();

    // For now, just make these non-zero:
    cs().iFit(currSpw()).column(slot)=1.0;
    cs().iFitwt(currSpw()).column(slot)=1.0;
    cs().fit(currSpw())(slot)=1.0;
    cs().fitwt(currSpw())(slot)=1.0;

    IPosition blc4(4,0,       focusChan(),0,        slot);
    IPosition trc4(4,nPar()-1,focusChan(),nElem()-1,slot);
    if (prtlev()>3) cout<<"keep cspar.shape="<< cs().par(currSpw())(blc4,trc4).nonDegenerate(3).shape()<<"  cparshape="<<solveCPar().shape()<<endl;
    cs().par(currSpw())(blc4,trc4).nonDegenerate(3) = solveCPar();

    // TBD:  Handle solveRPar here!

    cs().parOK(currSpw())(blc4,trc4).nonDegenerate(3)= solveParOK();
    cs().parErr(currSpw())(blc4,trc4).nonDegenerate(3)= solveParErr();
    cs().parSNR(currSpw())(blc4,trc4).nonDegenerate(3)= solveParSNR();
    cs().solutionOK(currSpw())(slot) = anyEQ(solveParOK(),True);

//    if ( cs().solutionOK(currSpw())(slot) != True) 
//      cout << "soln !OK " << solveCPar().shape() << " : " << solveCPar() << " -> " << solveParOK() << endl;
  }
  else {    
    throw(AipsError("SVJ::keep: Attempt to store solution in non-existent CalSet slot"));
  }

}

void SolvableVisCal::globalPostSolveTinker() {

  // Make solutions phase- or amp-only, if required
  if (apmode()!="AP") enforceAPonSoln();
  
  // Apply normalization
  if (solnorm()) normalize();

}

// Divide all solutions by their amplitudes to make them "phase-only"
void SolvableVisCal::enforceAPonSoln() {

  if (cs_) {    

    logSink() << "Enforcing apmode on solutions." 
	      << LogIO::POST;

    for (Int ispw=0;ispw<nSpw();++ispw) {
      if (cs().nTime(ispw)>0) {
	Array<Float> amps(amplitude(cs().par(ispw)));
	// No zeroes:
	cs().par(ispw)(amps==0.0f)=Complex(1.0);
	cs().par(ispw)(operator!(LogicalArray(cs().parOK(ispw))))=Complex(1.0);
	amps(amps==0.0f)=1.0f;
	amps(!cs().parOK(ispw))=1.0f;
	//	amps(operator!(LogicalArray(cs().parOK(ispw))))=1.0f;

	Array<Complex> cor(amps.shape());
	if (apmode()=='P')
	  // we will scale solns by amp to make them phase-only
	  convertArray(cor,amps);
	else if (apmode()=='A') {
	  // we will scale solns by "phase" to make them amp-only
	  cor=cs().par(ispw);
	  cor/=amps;
	}

	if (ntrue(amplitude(cor)==0.0f)==0)
	  cs().par(ispw)/=cor;
	else
	  throw(AipsError("enforceAPonSoln divide-by-zero error."));
      }
    }
  }
  else 
    throw(AipsError("Solution apmode enforcement not supported."));
}

void SolvableVisCal::normalize() {

  // Only if we have a CalSet...
  if (cs_) {

    logSink() << "Normalizing solution amplitudes per spw." 
	      << LogIO::POST;

    // Normalize each non-trivial spw in the CalSet
    for (Int ispw=0;ispw<nSpw();++ispw)
      if (cs().nTime(ispw)>0) 
	normSolnArray(cs().par(ispw),cs().parOK(ispw),False);

  }
  else 
    throw(AipsError("Solution normalization not supported."));
	
}

void SolvableVisCal::store() {

  if (prtlev()>3) cout << " SVC::store()" << endl;

  if (append())
    logSink() << "Appending solutions to table: " << calTableName()
	      << LogIO::POST;
  else
    logSink() << "Writing solutions to table: " << calTableName()
	      << LogIO::POST;

  cs().store(calTableName(),typeName(),append(),msName());

}

void SolvableVisCal::store(const String& table,const Bool& append) {

  if (prtlev()>3) cout << " SVC::store(table,append)" << endl;

  // Override tablename
  calTableName()=table;
  SolvableVisCal::append()=append;

  // Call conventional store
  store();

}

void SolvableVisCal::stateSVC(const Bool& doVC) {
  
  // If requested, report VisCal state
  if (doVC) VisCal::state();

  if (prtlev()>3) cout << "SVC::stateSVC():" << endl;
  cout << boolalpha;

  // Now SVC-specific stuff:
  cout << "  isSolved() = " << isSolved() << endl;
  cout << "  apmode() = " << apmode() << endl;
  cout << "  calTableName() = " << calTableName() << endl;
  cout << "  calTableSelect() = " << calTableSelect() << endl;
  cout << "  tInterpType() = " << tInterpType() << endl;
  cout << "  fInterpType() = " << fInterpType() << endl;
  cout << "  spwMap() = " << spwMap() << endl;
  cout << "  refant() = " << refant() << endl;
  
  cout << "  solveCPar().shape()   = " << solveCPar().shape() 
       << " (" << solveCPar().data() << ")" << endl;
  cout << "  solveRPar().shape()   = " << solveRPar().shape() 
       << " (" << solveRPar().data() << ")" << endl;
  cout << "  solveParOK().shape() = " << solveParOK().shape()
       << " (" << solveParOK().data() << ") "
       << " (ntrue=" << ntrue(solveParOK()) << ")" << endl;

  cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;

}

void SolvableVisCal::normSolnArray(Array<Complex>& sol, 
				   const Array<Bool>& solOK,
				   const Bool doPhase) {

  // Only do something if 2 or more good solutions
  if (ntrue(solOK)>1) {

    Complex factor(1.0);
    
    Array<Float> amp(amplitude(sol));

    // If desired, determine phase part of the normalization
    if (doPhase) {
      // Prepare to divide by amplitudes indiscriminately
      amp(!solOK)=1.0f;
      Array<Complex> sol1=sol/amp;
      sol1(!solOK)=Complex(0.0);
      factor=sum(sol1);
      factor/=abs(factor);
    }

    // Determine amplitude normalization
    amp(!solOK)=0.0f;
    factor*=Complex(sum(amp)/Float(ntrue(solOK)));
    
    // Apply the normalization factor, if non-zero
    if (abs(factor) > 0.0)
      sol/=factor;
    
  } // ntrue > 0

}



void SolvableVisCal::currMetaNote() {

  cout << "   ("
       << "time=" << MVTime(refTime()/C::day).string(MVTime::YMD,7)
       << " field=" << currField()
       << " spw=" << currSpw()
       << " chan=" << focusChan()
       << ")"
       << endl;

}


void SolvableVisCal::initSVC() {

  if (prtlev()>2) cout << " SVC::initSVC()" << endl;

  for (Int ispw=0;ispw<nSpw(); ispw++) {
    solveCPar_[ispw] = new Cube<Complex>();
    solveRPar_[ispw] = new Cube<Float>();
    solveParOK_[ispw] = new Cube<Bool>();
    solveParErr_[ispw] = new Cube<Float>();
    solveParSNR_[ispw] = new Cube<Float>();
  }
  
  parType_=VisCalEnum::COMPLEX;
  csmi.reset(msName());
}

void SolvableVisCal::deleteSVC() {

  if (prtlev()>2) cout << " SVC::deleteSVC()" << endl;

  for (Int ispw=0; ispw<nSpw(); ispw++) {
    if (solveCPar_[ispw])  delete solveCPar_[ispw];
    if (solveRPar_[ispw])  delete solveRPar_[ispw];
    if (solveParOK_[ispw]) delete solveParOK_[ispw];
    if (solveParErr_[ispw]) delete solveParErr_[ispw];
    if (solveParSNR_[ispw]) delete solveParSNR_[ispw];
  }
  solveCPar_=NULL;
  solveRPar_=NULL;
  solveParOK_=NULL;
  solveParErr_=NULL;
  solveParSNR_=NULL;
}

void SolvableVisCal::verifyCalTable(const String& caltablename) {

  // Call external method to get type (will throw if bad table)
  String calType=calTableType(caltablename);

  // Check if proper Calibration type...
  if (calType!=typeName()) {
    ostringstream o;
    o << "Table " << caltablename 
      << " has wrong Calibration type: " << calType;
    throw(AipsError(String(o)));
  }
}

void SolvableVisCal::applyChanMask(VisBuffer& vb) {

  if (chanmask_) {

    // A reference to de-referenced pointer
    PtrBlock<Vector<Bool>*>& chmask(*chanmask_);
  
    Int spw=vb.spectralWindow();
    Int chan0=vb.channel()(0);
    Int nchan=vb.nChannel();
    if (chmask.nelements()==uInt(nSpw()) &&
	chmask[spw] &&
	sum((*chmask[spw])(Slice(chan0,nchan))) > 0 ) {
      // There are some channels to mask...
      Vector<Bool> fr(vb.flagRow());
      Matrix<Bool> f(vb.flag());
      Vector<Bool> fc;
      Vector<Bool> chm((*(*chanmask_)[spw])(Slice(chan0,nchan)));
      for (Int irow=0;irow<vb.nRow();++irow)
	if (!fr(irow)) {
	  fc.reference(f.column(irow));
	  fc = fc||chm;
	  
	  //	cout << irow << ": ";
	  //	for (Int j=0;j<nchan;++j) cout << fc(j);
	  //	cout << endl;
	  
	}
    }
  }

}
  //
  //-----------------------------------------------------------------------
  //  
  void SolvableVisCal::printActivity(const Int nSlots, const Int slotNo, 
				     const Int fieldId, const Int spw, 
				     const Int nSolutions)
  {
    Int nMesg;

    //    nSlots = rcs().nTime(spw);
    
    Double timeTaken = timer_p.all();
    if (maxTimePerSolution_p < timeTaken) maxTimePerSolution_p = timeTaken;
    if (minTimePerSolution_p > timeTaken) minTimePerSolution_p = timeTaken;
    avgTimePerSolution_p += timeTaken;
    Double avgT =  avgTimePerSolution_p/(nSolutions>0?nSolutions:1);
    //
    // Boost the no. of messages printed if the next message, based on
    // the average time per solution, is going to appear after a time
    // longer than your patience would permit!  The limit of
    // patience defaults to 1h.
    //
    Float boost = userPrintActivityInterval_p/avgT;
    boost = userPrintActivityInterval_p/avgT;
    boost = (boost < 1.0)? 1.0 : nSlots*userPrintActivityFraction_p;
    nMesg = (Int)boost;
    
    Int tmp=abs(nSlots-slotNo); Bool print;
    print = False;
    if (nMesg <= 0) print=False;
    else if ((slotNo == 0) || (slotNo == nSlots-1))  print=True;
    else if ((tmp > 0 ) && ((slotNo+1)%nMesg ==0)) print=True;
    else print=False;

    if (print)
      {
	Int f = (Int)(100*(slotNo+1)/(nSlots>0?nSlots:1));
	logSink()<< LogIO::NORMAL 
		 << "Spw=" << spw << " slot=" << slotNo << "/" << nSlots
		 << " field=" << fieldId << ". Done " << f << "%"
		 << " Time taken per solution (max/min/avg): "
		 << maxTimePerSolution_p << "/" 
		 << (minTimePerSolution_p<0?1:minTimePerSolution_p) << "/"
		 << avgT << " sec" << LogIO::POST;
      }
  }
  
// **********************************************************
//  SolvableVisMueller Implementations
//


SolvableVisMueller::SolvableVisMueller(VisSet& vs) :
  VisCal(vs),
  VisMueller(vs),
  SolvableVisCal(vs),
  dM_(NULL),
  diffMElem_(),
  DMValid_(False)
{
  if (prtlev()>2) cout << "SVM::SVM(vs)" << endl;
}

SolvableVisMueller::SolvableVisMueller(const Int& nAnt) :
  VisCal(nAnt),
  VisMueller(nAnt),
  SolvableVisCal(nAnt),
  dM_(NULL),
  diffMElem_(),
  DMValid_(False)
{
  if (prtlev()>2) cout << "SVM::SVM(i,j,k)" << endl;
}

SolvableVisMueller::~SolvableVisMueller() {

  if (prtlev()>2) cout << "SVM::~SVM()" << endl;

}

// Setup solvePar shape (Mueller version)
void SolvableVisMueller::initSolvePar() {

  if (prtlev()>3) cout << " SVM::initSolvePar()" << endl;
  
  for (Int ispw=0;ispw<nSpw();++ispw) {
    
    currSpw()=ispw;

    switch(parType())
      {
      case VisCalEnum::COMPLEX:
	{
	  // TBD: Consider per-baseline solving (3rd=1)
	  solveCPar().resize(nPar(),nChanPar(),nBln());
	  solveParOK().resize(nPar(),nChanPar(),nBln());
	  solveParErr().resize(nPar(),nChanPar(),nBln());
	  solveParSNR().resize(nPar(),nChanPar(),nBln());
	  
	  // TBD: solveRPar()?
	  
	  solveCPar()=Complex(1.0);
	  solveParOK()=True;
	  solveParErr()=0.0;
	  solveParSNR()=0.0;
	  break;
	}
      case VisCalEnum::REAL:
	{
	  // TBD: Consider per-baseline solving (3rd=1)
	  solveRPar().resize(nPar(),nChanPar(),nBln());
	  solveParOK().resize(nPar(),nChanPar(),nBln());
	  solveParErr().resize(nPar(),nChanPar(),nBln());
	  solveParSNR().resize(nPar(),nChanPar(),nBln());
	  
	  // TBD: solveRPar()?
	  
	  solveRPar()=0.0;
	  solveParOK()=True;
	  solveParErr()=0.0;
	  solveParSNR()=0.0;
	  break;
	}
      case VisCalEnum::COMPLEXREAL:
         throw(AipsError("Internal error(Calibrater Module): Unsupported parameter type "
			 "COMPLEXREAL found in SolvableVisMueller::initSolvePar()"));
         break;
      }//endcase
  }
  currSpw()=0;

}

void SolvableVisMueller::syncDiffMat() {

  if (prtlev()>5) cout << "     SVM::syncDiffMat()" 
		       << " (DMValid()=" << DMValid() << ")" << endl;

  // Sync the diff'd Mueller matrices
  if (!DMValid()) syncDiffMueller();

}

void SolvableVisMueller::syncDiffMueller() {

  if (prtlev()>6) cout << "      SVM::syncDiffMueller()" << endl;

  // TBD:  validateDM() for trivialMuellerElem()=True??
  //    (cf. where does invalidateDM() occur?)

  if (trivialDM())
    // Ensure trivial matrices ready
    initTrivDM();
  else {
    diffMElem().resize(IPosition(4,muellerNPar(muellerType()),nPar(),nChanMat(),nCalMat()));
    diffMElem().unique();
    invalidateDM();

    // Calculate for all blns/chans
    calcAllDiffMueller();

  }

  // Ensure diff'd Mueller matrix renders are OK
  createDiffMueller();

  // diff'd Mueller matrices now valid
  validateDM();

}

void SolvableVisMueller::calcAllDiffMueller() {

  if (prtlev()>6) cout << "       SVM::calcAllDiffMueller" << endl;

  // Should handle OK flags in this method, and only
  //  do  calc if OK

  // Sanity check on parameter channel axis
  AlwaysAssert((solveCPar().shape()(1)==1),AipsError);

  // Referencing arrays, per baseline
  Matrix<Complex> oneDM;       //  (nElem,nPar)
  Vector<Complex> onePar;      //  (nPar)

  ArrayIterator<Complex> dMiter(diffMElem(),2);
  ArrayIterator<Complex> Piter(solveCPar(),1);
  
  for (Int ibln=0; ibln<nCalMat(); ++ibln) {

    // Solving parameters are NEVER channel-dependent
    //  (even if data & matrices are)
    onePar.reference(Piter.array());
      
    for (Int ich=0; ich<nChanMat(); ich++) {
      
      oneDM.reference(dMiter.array());

      // Calculate the DM matrices for each par on this bln/chan
      calcOneDiffMueller(oneDM,onePar);

      // Advance iterators
      dMiter.next();

    }
    Piter.next();
  }

}

void SolvableVisMueller::calcOneDiffMueller(Matrix<Complex>& mat, 
					    const Vector<Complex>& par) {

  if (prtlev()>10) cout << "        SVM::calcOneDiffMueller()" << endl;

  // If Mueller matrix is trivial, shouldn't get here
  if (trivialMuellerElem()) 
    throw(AipsError("Trivial Mueller Matrix logic error."));

  // Otherwise, this method apparently hasn't been specialized, as required
  else
    throw(AipsError("Unknown non-trivial dMueller-from-parameter calculation requested."));

}

void SolvableVisMueller::createDiffMueller() {

  if (prtlev()>6) cout << "       SVM::createDiffMueller()" << endl;

  Mueller::MuellerType mtype(muellerType());

  // Delete if wrong type
  if (dM_ && dM().type() != mtype) delete dM_;
  
  // If needed, construct the correct diff Mueller
  if (!dM_) dM_ = casa::createMueller(mtype);
      
}

void SolvableVisMueller::initTrivDM() {

  if (prtlev()>7) cout << "        SVM::initTrivDM()" << endl;

  // If DM matrice not trivial, shouldn't get here
  if (!trivialDM()) 
    throw(AipsError("Trivial Mueller Matrix logic error."));

  // Otherwise, this method apparently hasn't been specialized, as required
  else
    throw(AipsError("Unknown trivial dM initialization requested."));

}

void SolvableVisMueller::stateSVM(const Bool& doVC) {
  
  // If requested, report VisCal state
  if (doVC) VisMueller::state();
  
  // Get parent's state (w/out VC):
  SolvableVisCal::stateSVC(False);

  if (applyByMueller()) {
    if (prtlev()>3) cout << "SVM::stateSVM()" << endl;
    cout << boolalpha;

  // Now SVM-specific stuff:
    cout << "DMValid() = " << DMValid() << endl;
    cout << "diffMElem().shape() = " << diffMElem().shape() 
	 << " (" << diffMElem().data() << ")" << endl;
    
    cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
  }
}


// **********************************************************
//  SolvableVisJones Implementations
//


SolvableVisJones::SolvableVisJones(VisSet& vs) :
  VisCal(vs),                           // virtual base
  VisMueller(vs),                       // virtual base
  SolvableVisMueller(vs),               // immediate parent
  VisJones(vs),                         // immediate parent
  dJ1_(NULL),                           // data...
  dJ2_(NULL),
  diffJElem_(),
  DJValid_(False)
{
  if (prtlev()>2) cout << "SVJ::SVJ(vs)" << endl;
}

SolvableVisJones::SolvableVisJones(const Int& nAnt) : 
  VisCal(nAnt),               // virtual base
  VisMueller(nAnt),           // virtual base
  SolvableVisMueller(nAnt),   // immediate parent
  VisJones(nAnt),             // immediate parent
  dJ1_(NULL),                 // data...
  dJ2_(NULL),
  diffJElem_(),
  DJValid_(False)
{
  if (prtlev()>2) cout << "SVJ::SVJ(i,j,k)" << endl;
}

SolvableVisJones::~SolvableVisJones() {

  if (prtlev()>2) cout << "SVJ::~SVJ()" << endl;

}

void SolvableVisJones::reReference() {

  // TBD: Handle single-poln referencing
  // TBD: Handle missing refant

  // Generic version for trivial types
  if (trivialJonesElem()) {

    // Determine multiplicative complex phase
    Matrix<Complex> refgain;
    refgain=solveCPar().xyPlane(refant());

    //    cout << "refgain add:  " << refgain.data() << " " << solveCPar().xyPlane(refant()).data() << endl;

    Float amp(1.0);
    Complex* rg=refgain.data();
    // TBD: clean this up: there is only 1 solvePar chan!
    //    for (Int ich=0;ich<nChanPar();++ich) {
      for (Int ip=0;ip<nPar();++ip,++rg) {
	amp=abs(*rg);
	if (amp>0.0) {
	  *rg/=amp;
	  *rg=conj(*rg);
	}
	else
	  *rg=Complex(1.0);
      }
      //    }

      //      cout << "amp(refgain) = " << amplitude(refgain) << endl;


    // Apply complex phase to each ant
    Matrix<Complex> antgain;
    for (Int ia=0;ia<nAnt();++ia) {
      antgain.reference(solveCPar().xyPlane(ia));
      antgain*=refgain;
    }
  }
  else 
    throw(AipsError("Attempt to reference non-trivial calibration type."));


}


void SolvableVisJones::differentiate(CalVisBuffer& cvb) {

  if (prtlev()>3) cout << "  SVJ::differentiate(CVB)" << endl;

  // NB: For freqDepPar()=True, the data and solutions are
  //     multi-channel, but nChanMat()=1 because we only 
  //     consider one channel at a time.  In this case,
  //     focusChan is the specific channel under consideration.
  //     Otherwise, we will use all channels in the vb 
  //     simultaneously

  // Some vb shape info
  Int& nRow(cvb.nRow());
  Int& nCorr(cvb.nCorr());

  // Size (diff)residuals workspace in the CVB
  cvb.setFocusChan(focusChan());
  cvb.sizeResiduals(nPar(),2);    // 2 sets of nPar() derivatives per baseline

  // Copy in-focus model to residual workspace
  cvb.initResidWithModel();


  // References to workspaces
  Cube<Complex>& Vout(cvb.residuals());
  Array<Complex>& dVout(cvb.diffResiduals());
  Matrix<Bool>& Vflg(cvb.residFlag());
  
  // "Apply" the current Q,U or X estimates to the crosshand model
  if (solvePol()>0) {
    Complex pol(1.0);

    if (solvePol()==2)  // pol = Q+iU
      pol=Complex(real(srcPolPar()(0)),real(srcPolPar()(1)));
    else if (solvePol()==1)   // pol = exp(iX)
      pol=exp(Complex(0.0,real(srcPolPar()(0))));
    
    IPosition blc(3,1,0,0), trc(3,1,nChanMat()-1,nRow-1);
    Array<Complex> RL(Vout(blc,trc));
    RL*=pol;
    blc(0)=trc(0)=2;
    Array<Complex> LR(Vout(blc,trc));
    LR*=conj(pol);
  }
  
  // Visibility vector renderers
  VisVector::VisType vt(visType(nCorr));
  VisVector cVm(vt);  // The model data corrupted by trial solution
  VisVector dV1(vt);  // The deriv of V wrt pars of 1st ant in bln 
  VisVector dV2(vt);  // The deriv of V wrt pars of 2nd ant in bln 

  // Temporary non-iterating VisVectors to hold partial applies
  VisVector J1V(vt,True);
  VisVector VJ2(vt,True);

  // Starting synchronization for output visibility data
  cVm.sync(Vout(0,0,0));
  dV1.sync(dVout(IPosition(5,0,0,0,0,0)));
  dV2.sync(dVout(IPosition(5,0,0,0,0,1)));

  // Synchronize current calibration pars/matrices
  syncSolveCal();

  // Nominal synchronization of dJs
  dJ1().sync(diffJElem()(IPosition(4,0,0,0,0)));
  dJ2().sync(diffJElem()(IPosition(4,0,0,0,0)));

  // VisBuffer indices
  Double* time=  cvb.time().data();
  Int*    a1=    cvb.antenna1().data();
  Int*    a2=    cvb.antenna2().data();
  Bool*   flagR= cvb.flagRow().data();
  Bool*   flag=  Vflg.data();            // via local reference
  
  // TBD: set weights according to flags??

  // iterate rows
  for (Int irow=0; irow<nRow; irow++,flagR++,a1++,a2++,time++) {
    
    // Avoid ACs
    if (*a1==*a2) *flagR=True;

    if (!*flagR) {  // if this row unflagged
	
      // Re-update matrices if time changes
      if (timeDepMat() && *time != lastTime()) {
	currTime()=*time;
	invalidateDiffCalMat();
	syncCalMat();
	syncDiffMat();
	lastTime()=currTime();
      }

      // Synchronize Jones renderers for the ants on this baseline
      J1().sync(currJElem()(0,0,*a1),currJElemOK()(0,0,*a1));
      J2().sync(currJElem()(0,0,*a2),currJElemOK()(0,0,*a2));

      // Synchronize differentiated Jones renderers for this baseline
      if (trivialDJ()) {
	dJ1().origin();
	dJ2().origin();
      } else {
	dJ1().sync(diffJElem()(IPosition(4,0,0,0,*a1)));
	dJ2().sync(diffJElem()(IPosition(4,0,0,0,*a2)));
      }

      // Assumes all iterating quantities have nChanMat() channelization
      for (Int ich=0; ich<nChanMat();ich++,flag++,
	     cVm++, J1()++, J2()++) {
	     
	// if channel unflagged an cal ok
	//	if (!*flag && (*J1Ok && *J2Ok) ) {  
	if (!*flag) { 
	  
	  // Partial applies for repeated use below
	  VJ2=cVm;                    
	  J2().applyLeft(VJ2,*flag);      // VJ2 = Vm*J2, used below

	  J1().applyRight(cVm,*flag);     
	  J1V=cVm;                        // J1V = J1*Vm, used below

	  // Finish trial corruption
	  J2().applyLeft(cVm,*flag);      // cVm = (J1*Vm)*J2

	}

	// Only continue with diff-ing, if we aren't flagged yet
	if (!*flag) {

	  // Differentiation per par
	  for (Int ip=0;ip<nPar();ip++,
		 dV1++,dJ1()++,
		 dV2++,dJ2()++) {
	    
	    dV1=VJ2;
	    dJ1().applyRight(dV1);  // dV1 = dJ1(ip)*(Vm*J2)
	    
	    dV2=J1V;
	    dJ2().applyLeft(dV2);   // dV2 = (J1*Vm)*dJ2(ip)
	  }
	  
	} // (!*flag)
	else {
	  // set trial corruption to zero
	  cVm.zero();
	  
	  // Advance all par-dep pointers over flagged channel
	  dV1.advance(nPar());
	  dV2.advance(nPar());
	  dJ1().advance(nPar());
	  dJ2().advance(nPar());
	}

      } // chn
		
    } // !*flagR
    else {
      // Must advance all chan-, par-dep pointers over flagged row
      flag+=nChanMat(); 
      cVm.advance(nChanMat());
      J1().advance(nChanMat());
      J2().advance(nChanMat());
      Int chpar(nChanMat()*nPar());
      dV1.advance(chpar);
      dV2.advance(chpar);
      dJ1().advance(chpar);
      dJ2().advance(chpar);
    }
  }

  // Subtract the obs'd data from the trial-corrupted model
  //  to form residuals
  cvb.finalizeResiduals();

}

void SolvableVisJones::differentiate(VisBuffer& vb,
				     Cube<Complex>& Vout, 
				     Array<Complex>& dVout,
				     Matrix<Bool>& Vflg) {
    
  if (prtlev()>3) cout << "  SVJ::differentiate()" << endl;

  // NB: For freqDepPar()=True, the data and solutions are
  //     multi-channel, but nChanMat()=1 because we only 
  //     consider one channel at a time.  In this case,
  //     focusChan is the specific channel under consideration.
  //     Otherwise, we will use all channels in the vb 
  //     simultaneously

  // Some vb shape info
  Int& nRow(vb.nRow());
  Int nCorr(vb.corrType().nelements());

  // Size up the output data arrays
  // Vout = [nCorr,nChan,nRow]
  // dVout = [nCorr,nPar,nChan,nRow,2]   (1 set of dV for both ants on baseline)
  Vout.resize(IPosition(3,nCorr,nChanMat(),nRow));
  Vout.unique();             // ensure unique storage

  dVout.resize(IPosition(5,nCorr,nPar(),nChanMat(),nRow,2));
  dVout.unique();
  dVout=Complex(0.0);
  
  // Copy the input model data from the VisBuffer to Vout
  //  for in-place application (do this according to focusChan)
  //  (also flags)
  Matrix<Bool> fl;
  if (freqDepPar()) {
    // Copy just the focusChan; all work below is single-channel
    AlwaysAssert((nChanMat()==1),AipsError);  // sanity
    AlwaysAssert((focusChan()>-1),AipsError);       // sanity

    Vout = vb.modelVisCube()(IPosition(3,0,      focusChan(),0     ),
			     IPosition(3,nCorr-1,focusChan(),nRow-1));

    Vflg.resize(IPosition(2,1,nRow));   // proper single channel size
    Vflg.unique();                      // unique storage
    Vflg = vb.flag()(IPosition(2,focusChan(),0     ),
		     IPosition(2,focusChan(),nRow-1));
  }
  else {
    // Copy all channels in the vb
    Vout = vb.modelVisCube();
    Vflg.reference(vb.flag());   // Just reference whole flag array
  }

  // "Apply" the current Q,U or X estimates to the crosshand model
  if (solvePol()>0) {
    Complex pol(1.0);

    if (solvePol()==2)  // pol = Q+iU
      pol=Complex(real(srcPolPar()(0)),real(srcPolPar()(1)));
    else if (solvePol()==1)   // pol = exp(iX)
      pol=exp(Complex(0.0,real(srcPolPar()(0))));
    
    IPosition blc(3,1,0,0), trc(3,1,nChanMat()-1,nRow-1);
    Array<Complex> RL(Vout(blc,trc));
    RL*=pol;
    blc(0)=trc(0)=2;
    Array<Complex> LR(Vout(blc,trc));
    LR*=conj(pol);
  }
  
  // Visibility vector renderers
  VisVector::VisType vt(visType(nCorr));
  VisVector cVm(vt);  // The model data corrupted by trial solution
  VisVector dV1(vt);  // The deriv of V wrt pars of 1st ant in bln 
  VisVector dV2(vt);  // The deriv of V wrt pars of 2nd ant in bln 

  // Temporary non-iterating VisVectors to hold partial applies
  VisVector J1V(vt,True);
  VisVector VJ2(vt,True);

  // Starting synchronization for output visibility data
  cVm.sync(Vout(0,0,0));
  dV1.sync(dVout(IPosition(5,0,0,0,0,0)));
  dV2.sync(dVout(IPosition(5,0,0,0,0,1)));

  // Synchronize current calibration pars/matrices
  syncSolveCal();

  // Nominal synchronization of dJs
  dJ1().sync(diffJElem()(IPosition(4,0,0,0,0)));
  dJ2().sync(diffJElem()(IPosition(4,0,0,0,0)));

  // VisBuffer indices
  Double* time=  vb.time().data();
  Int*    a1=    vb.antenna1().data();
  Int*    a2=    vb.antenna2().data();
  Bool*   flagR= vb.flagRow().data();
  Bool*   flag=  Vflg.data();            // via local reference
  
  // TBD: set weights according to flags??

  // iterate rows
  for (Int irow=0; irow<nRow; irow++,flagR++,a1++,a2++,time++) {
    
    // Avoid ACs
    if (*a1==*a2) *flagR=True;

    if (!*flagR) {  // if this row unflagged
	
      // Re-update matrices if time changes
      if (timeDepMat() && *time != lastTime()) {
	currTime()=*time;
	invalidateDiffCalMat();
	syncCalMat();
	syncDiffMat();
	lastTime()=currTime();
      }

      // Synchronize Jones renderers for the ants on this baseline
      J1().sync(currJElem()(0,0,*a1),currJElemOK()(0,0,*a1));
      J2().sync(currJElem()(0,0,*a2),currJElemOK()(0,0,*a2));

      // Synchronize differentiated Jones renderers for this baseline
      if (trivialDJ()) {
	dJ1().origin();
	dJ2().origin();
      } else {
	dJ1().sync(diffJElem()(IPosition(4,0,0,0,*a1)));
	dJ2().sync(diffJElem()(IPosition(4,0,0,0,*a2)));
      }

      // Assumes all iterating quantities have nChanMat() channelization
      for (Int ich=0; ich<nChanMat();ich++,flag++,
	     cVm++, J1()++, J2()++) {
	     
	// if channel unflagged an cal ok
	//	if (!*flag && (*J1Ok && *J2Ok) ) {  
	if (!*flag) { 
	  
	  // Partial applies for repeated use below
	  VJ2=cVm;                    
	  J2().applyLeft(VJ2,*flag);      // VJ2 = Vm*J2, used below

	  J1().applyRight(cVm,*flag);     
	  J1V=cVm;                        // J1V = J1*Vm, used below

	  // Finish trial corruption
	  J2().applyLeft(cVm,*flag);      // cVm = (J1*Vm)*J2

	}

	// Only continue with diff-ing, if we aren't flagged yet
	if (!*flag) {

	  // Differentiation per par
	  for (Int ip=0;ip<nPar();ip++,
		 dV1++,dJ1()++,
		 dV2++,dJ2()++) {
	    
	    dV1=VJ2;
	    dJ1().applyRight(dV1);  // dV1 = dJ1(ip)*(Vm*J2)
	    
	    dV2=J1V;
	    dJ2().applyLeft(dV2);   // dV2 = (J1*Vm)*dJ2(ip)
	  }
	  
	} // (!*flag)
	else {
	  // set trial corruption to zero
	  cVm.zero();
	  
	  // Advance all par-dep pointers over flagged channel
	  dV1.advance(nPar());
	  dV2.advance(nPar());
	  dJ1().advance(nPar());
	  dJ2().advance(nPar());
	}

      } // chn
		
    } // !*flagR
    else {
      // Must advance all chan-, par-dep pointers over flagged row
      flag+=nChanMat(); 
      cVm.advance(nChanMat());
      J1().advance(nChanMat());
      J2().advance(nChanMat());
      Int chpar(nChanMat()*nPar());
      dV1.advance(chpar);
      dV2.advance(chpar);
      dJ1().advance(chpar);
      dJ2().advance(chpar);
    }
  }

}

void SolvableVisJones::diffSrc(VisBuffer& vb,
			       Array<Complex>& dVout) {
    
  if (prtlev()>3) cout << "  SVJ::diffSrc()" << endl;

  // Some vb shape info
  Int& nRow(vb.nRow());
  Int nCorr(vb.corrType().nelements());

  // Size up the output data arrays
  dVout.resize(IPosition(4,nCorr,nChanMat(),nRow,solvePol()));
  dVout.unique();
  dVout=Complex(0.0);
  
  // For now, we don't actually need gradients w.r.t. the source
  return;

  IPosition blc(4,0,0,0,0), trc(4,0,nChanMat()-1,nRow-1,0);

  if (solvePol()==2) {
    blc(3)=trc(3)=0;
    blc(0)=1;trc(0)=2;
    dVout(blc,trc)=Complex(1.0);   // Q part (both RL & LR)
    blc(3)=trc(3)=1;
    blc(0)=trc(0)=1;
    dVout(blc,trc)=Complex(0.0,1.0);  // U part (in RL)
    blc(0)=trc(0)=2;
    dVout(blc,trc)=Complex(0.0,-1.0); // U part (in LR)
  }
  else if (solvePol()==1) {
    Complex dX=Complex(0.0,1.0)*exp(Complex(0.0,real(srcPolPar()(0))));
    blc(3)=trc(3)=0;
    blc(0)=trc(0)=1;
    dVout(blc,trc)=dX;  // multiplying RL
    blc(0)=trc(0)=2;
    dVout(blc,trc)=conj(dX); // multiplying LR
  }

  // Visibility vector renderers
  VisVector::VisType vt(visType(nCorr));
  VisVector dSm1(vt);  // The model data corrupted by trial solution
  VisVector dSm2(vt);  // The model data corrupted by trial solution

  // Starting synchronization for output visibility data
  dSm1.sync(dVout(IPosition(4,0,0,0,0)));
  if (solvePol()>1)
    dSm2.sync(dVout(IPosition(4,0,0,0,1)));

  // Synchronize current calibration pars/matrices
  syncSolveCal();

  // Nominal synchronization of dJs
  dJ1().sync(diffJElem()(IPosition(4,0,0,0,0)));
  dJ2().sync(diffJElem()(IPosition(4,0,0,0,0)));

  // VisBuffer indices
  Double* time=  vb.time().data();
  Int*    a1=    vb.antenna1().data();
  Int*    a2=    vb.antenna2().data();
  Bool*   flagR= vb.flagRow().data();
  Bool*   flag=  vb.flag().data();
  
  // TBD: set weights according to flags??

  // iterate rows
  for (Int irow=0; irow<nRow; irow++,flagR++,a1++,a2++,time++) {
    
    // Avoid ACs
    if (*a1==*a2) *flagR=True;

    if (!*flagR) {  // if this row unflagged
	
      // Re-update matrices if time changes
      if (timeDepMat() && *time != lastTime()) {
	currTime()=*time;
	invalidateDiffCalMat();
	syncCalMat();
	syncDiffMat();
	lastTime()=currTime();
      }

      // Synchronize Jones renderers for the ants on this baseline
      J1().sync(currJElem()(0,0,*a1),currJElemOK()(0,0,*a1));
      J2().sync(currJElem()(0,0,*a2),currJElemOK()(0,0,*a2));

      // Assumes all iterating quantities have nChanMat() channelization
      for (Int ich=0; ich<nChanMat();ich++,flag++,
	     dSm1++, dSm2++, J1()++, J2()++) {
	     
	// if channel unflagged an cal ok
	if (!*flag) { 
	  
	  J1().applyRight(dSm1);
	  J2().applyLeft(dSm1);
	  if (solvePol()>1) {
	    J1().applyRight(dSm2);
	    J2().applyLeft(dSm2);
	  }
	}

      } // chn
		
    } // !*flagR
    else {
      // Must advance all chan-, par-dep pointers over flagged row
      flag+=nChanMat(); 
      dSm1.advance(nChanMat());
      dSm2.advance(nChanMat());
      J1().advance(nChanMat());
      J2().advance(nChanMat());
    }
  }

}

void SolvableVisJones::accumulate(SolvableVisCal* incr,
                                  const Vector<Int>& fields) {

  // Use SVJ interface for the incremental component
  //  (this should always be safe at this point?)
  SolvableVisJones* svj = dynamic_cast<SolvableVisJones*>(incr);

  // Catch bad SVJ conversion or fundamental type mismatch
  if (svj==NULL || svj->type() != this->type())
    throw(AipsError("Incremental calibration is not of compatible type."));

  Int nfield(fields.nelements());

  Bool fldok(True);

  for (Int ispw=0; ispw<nSpw(); ispw++) {

    // Only update spws which are available in the incr table:
    if (incr->spwOK()(ispw)) {

      currSpw()=ispw;
      
      Int nSlot(cs().nTime(ispw));
      
      // For each slot in this spw
      for (Int islot=0; islot<nSlot; islot++) {
	
	Double thistime=cs().time(ispw)(islot);
	Int thisfield=cs().fieldId(ispw)(islot);
	
	// TBD: proper frequency?  (from CalSet?)
	Vector<Double> thisfreq(nSpw(),0.0);
	
	// Is current field among those we need to update?
	fldok = (nfield==0 || anyEQ(fields,thisfield));

	if (fldok) {

	  // TBD: group following into syncCal(cs)?
	  syncMeta(currSpw(),thistime,thisfield,thisfreq,nChanPar());
	  syncPar(currSpw(),islot);
	  syncCalMat(False);
	  
	  // Sync svj with this
	  svj->syncCal(*this);
	  
	  // Relevant channels to update are bounded by
	  //  channels available in incr table:
	  Int prestep(svj->startChan());
	  Int poststep(nChanMat()-prestep-svj->nChanMat());
	  
	  AlwaysAssert((prestep<nChanMat()),AipsError);
	  AlwaysAssert((poststep>-1),AipsError);

	  // Do the multiplication each ant, chan
	  for (Int iant=0; iant<nAnt(); iant++) {

	    J1().advance(prestep);
	    for (Int ichan=0; ichan<svj->nChanMat(); ichan++) {
	      J1()*=(svj->J1());
	      J1()++;
	      svj->J1()++;
	    } // ichan
	    J1().advance(poststep);
	  } // iant
	  IPosition blc(4,0,0,0,islot);
	  IPosition trc(cs().parOK(currSpw()).shape());
	  trc-=1;
	  trc(3)=islot;
	  cs().solutionOK(currSpw())(islot)=
	    anyEQ(cs().parOK(currSpw())(blc,trc),True);
	} // fldok
      } // islot
    } // spwOK
  } // ispw
}



// Setup solvePar shape (Jones version)
void SolvableVisJones::initSolvePar() {

  if (prtlev()>3) cout << " SVJ::initSolvePar()" << endl;

  for (Int ispw=0;ispw<nSpw();++ispw) {

    currSpw()=ispw;

    switch (parType()) {
    case VisCalEnum::COMPLEX: {
      solveCPar().resize(nPar(),1,nAnt());
      solveCPar()=Complex(1.0);
      break;
    }
    case VisCalEnum::REAL: {
      solveRPar().resize(nPar(),1,nAnt());
      solveRPar()=0.0;
      break;
    }
    default:
      throw(AipsError("Internal error(Calibrater Module): Unsupported parameter type "
		      "COMPLEXREAL found in SolvableVisJones::initSolvePar()"));
    }

    solveParOK().resize(nPar(),1,nAnt());
    solveParOK()=True;
    solveParErr().resize(nPar(),1,nAnt());
    solveParErr()=0.0;
    solveParSNR().resize(nPar(),1,nAnt());
    solveParSNR()=0.0;

  }
  currSpw()=0;

}

void SolvableVisJones::syncDiffMat() {

  if (prtlev()>5) cout << "     SVJ::syncDiffMat()" 
		       << " (DJValid()=" << DJValid() << ")" << endl;

  // Sync the diff'd Jones matrices
  if (!DJValid()) syncDiffJones();

  // Sync up Muellers, if necessary
  //  if (applyByMueller()) 
      // Do nothing for now!  --All dJ applied directly in differentiate
      // if (!DMValid()) syncDiffMueller();

}

void SolvableVisJones::syncDiffJones() {

  if (prtlev()>6) cout << "      SVJ::syncDiffJones()" << endl;

  // If differentiated Jones are trivial, we are
  // already referencing the type-dep trivial versions
  //  TBD: Review this for D, where trivialJonesElem()=False,
  //   but diffJ is "trivial-ish"!!!  (Probably need trivDiffJonesElem(),
  //   or override this method in D to make it no-op)

  if (trivialDJ())
    // Ensure trivial matrices ready
    initTrivDJ();
  else {
    diffJElem().resize(IPosition(4,jonesNPar(jonesType()),nPar(),nChanMat(),nCalMat()));
    diffJElem().unique();
    invalidateDJ();

    // Calculate for all ants/chans
    calcAllDiffJones();

  }

  // Ensure diff'd Jones matrix renders are OK
  createDiffJones();

  // diff'd Jones matrices now valid
  validateDJ();
  invalidateDM();   // dMs still invalid, probably forever

}

void SolvableVisJones::calcAllDiffJones() {

  if (prtlev()>6) cout << "       SVJ::calcAllDiffJones" << endl;

  // Should handle OK flags in this method, and only
  //  do  calc if OK

  Matrix<Complex> oneDJ;   //  (nElem,nPar)
  Vector<Complex> onePar;      //  (nPar)

  ArrayIterator<Complex> dJiter(diffJElem(),2);
  ArrayIterator<Complex> Piter(currCPar(),1);
  
  for (Int iant=0; iant<nCalMat(); iant++) {

    // Solving parameters are NEVER channel-dependent
    //  (even if data & matrices are)
    onePar.reference(Piter.array());
      
    for (Int ich=0; ich<nChanMat(); ich++) {
      
      oneDJ.reference(dJiter.array());

      // Calculate the DJ matrices w.r.t. each par on this ant/chan
      calcOneDiffJones(oneDJ,onePar);

      // Advance iterators
      dJiter.next();

    }
    Piter.next();
  }

}

void SolvableVisJones::calcOneDiffJones(Matrix<Complex>& mat, 
					const Vector<Complex>& par) {

  if (prtlev()>10) cout << "        SVJ::calcOneDiffJones()" << endl;

  // If Jones matrix is trivial, shouldn't get here
  if (trivialJonesElem()) 
    throw(AipsError("Trivial Jones Matrix logic error."));

  // Otherwise, this method apparently hasn't been specialized, as required
  else
    throw(AipsError("Unknown non-trivial dJones-from-parameter calculation requested."));

}

void SolvableVisJones::createDiffJones() {

  if (prtlev()>6) cout << "       SVJ::createDiffJones()" << endl;

  Jones::JonesType jtype(jonesType());

  // Delete if wrong type
  if (dJ1_ && dJ1().type() != jtype) delete dJ1_;
  if (dJ2_ && dJ2().type() != jtype) delete dJ2_;
  
  // If needed, construct the correct diff Jones
  if (!dJ1_) dJ1_ = casa::createJones(jtype);
  if (!dJ2_) dJ2_ = casa::createJones(jtype);
      
}

void SolvableVisJones::initTrivDJ() {

  if (prtlev()>7) cout << "        SVJ::initTrivDJ()" << endl;

  // If DJ matrice not trivial, shouldn't get here
  if (!trivialDJ()) 
    throw(AipsError("Trivial Jones Matrix logic error."));

  // Otherwise, this method apparently hasn't been specialized, as required
  else
    throw(AipsError("Unknown trivial dJ initialization requested."));

}

void SolvableVisJones::stateSVJ(const Bool& doVC) {
  
  // If requested, report VisCal state
  if (doVC) VisJones::state();

  // Get parent's state (w/out VC):
  SolvableVisMueller::stateSVM(False);

  if (applyByJones()) {
    if (prtlev()>3) cout << "SVJ::stateSVJ()" << endl;
    cout << boolalpha;
    
    // Now SVJ-specific stuff:
    cout << "  DJValid() = " << DJValid() << endl;
    
    cout << "  diffJElem().shape() = " << diffJElem().shape() 
	 << " (" << diffJElem().data() << ")" << endl;
    cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -" << endl;
  }
}

void SolvableVisJones::globalPostSolveTinker() {

  // Re-reference the phase, if requested
  if (refant()>-1) applyRefAnt();

  // Apply more general post-solve stuff
  SolvableVisCal::globalPostSolveTinker();

}

void SolvableVisJones::applyRefAnt() {

  // TBD:
  // 1. Synchronize refant changes on par axis
  // 2. Implement minimum mean deviation algorithm

  if (refant()<0) 
    throw(AipsError("No refant specified."));

  // Get the refant name from the nMS
  String refantName("none");
  /*
  MeasurementSet ms(msName());
  MSAntennaColumns msantcol(ms.antenna());
  refantName=msantcol.name()(refant());
  */
  refantName=csmi.getAntName(refant());

  logSink() << "Applying refant: " << refantName
	    << LogIO::POST;
  // , optimizing phase continuity even if refant sometimes missing." 

  Bool newway(True);

  // Use ANTENNA-table-ordered refant list if user's choice bad
  //  The first two entries in this list are the user's refant,
  //   and the one that was used on the previous interval.  This
  //   makes the priorities correct all the time.
  Vector<Int> refantlist(nAnt()+2);
  indgen(refantlist);
  refantlist-=2;
  refantlist(0)=refantlist(1)=refant();

  Bool usedaltrefant(False);

  for (Int ispw=0;ispw<nSpw();++ispw) {

    currSpw()=ispw;

    // TBD: use spwOK here when spwOK is activated in solve context
    if (cs().nTime(ispw)>0) {

    // References to ease access to solutions
    Array<Complex> sol(cs().par(ispw));
    Array<Bool> sok(cs().parOK(ispw));

    // Find time-ordered slot index
    Vector<uInt> ord;
    genSort(ord,cs().time(ispw));
    Int nslots(ord.nelements());

    // Reference each channel and par independently (?)
    for (Int ichan=0;ichan<nChanPar();++ichan) {
      for (Int ipar=0;ipar<nPar();++ipar) {
	
	Int iref=0;
	Int lastrefant(-1);
	
	// Apply refant to first slot
	if (cs().nTime(ispw)>0) {
	  
	  IPosition blc1(4,ipar,ichan,0,ord(0));
	  IPosition trc1(4,ipar,ichan,nAnt()-1,ord(0));
	  if (ntrue(sok(blc1,trc1))>0) {
	    
	    while ( iref<(nAnt()+2) &&
		    !sok(IPosition(4,ipar,ichan,refantlist(iref),ord(0))) ) {
	      ++iref;
	    }
	    
	    if (iref<nAnt()+2) {
	      // found a refant, use it
	      
	      if (iref>0) usedaltrefant=True;
	      
	      Int currrefant=refantlist(iref);

	      // Only report if using an alternate refant
	      if (currrefant!=lastrefant && iref>0) {
		logSink() 
			  << "At " 
			  << MVTime(cs().time(ispw)(ord(0))/C::day).string(MVTime::YMD,7) 
			  << " ("
			  << "Spw=" << ispw 
			  << ", Fld=" << cs().fieldId(ispw)(ord(0))
			  << ", pol=" << ipar 
			  << ", chan=" << ichan 
			  << ")"
		  //			  << ", using refant " << msantcol.name()(currrefant)
			  << ", using refant " << csmi.getAntName(currrefant)
			  << " (id=" << currrefant 
			  << ")" << " (alternate)"
			  << LogIO::POST;
	      }  

	      Complex rph=sol(IPosition(4,ipar,ichan,currrefant,ord(0)));

	      // make reference phasor phase-only
	      Float amp=abs(rph);
	      if (amp>0.0) {
		rph/=Complex(amp);

		// Adjust each good ant by this phasor
		for (Int iant=0;iant<nAnt();++iant) 
		  if (sok(IPosition(4,ipar,ichan,iant,ord(0)))) 
		    sol(IPosition(4,ipar,ichan,iant,ord(0)))/=rph;
	      }
	      else
		// This shouldn't happen
		cout << "Refant has undefined phase (zero amplitude) " << endl;

	      lastrefant=currrefant;

	    }
	    else {
	      // unlikely?
	      cout << "Couldn't find a refant for the first slot in spw = " << currSpw() << endl;
	    }
	  }
	}

	
	// Only if time-dep referencing required....
	if (cs().nTime(ispw) > 1) {
	  
	if (newway) {
	    
          Int islot0(0);
          Int islot1(0);

          IPosition blc0(4,ipar,ichan,0,ord(islot0));
          IPosition trc0(4,ipar,ichan,nAnt()-1,ord(islot0));
          IPosition blc1(4,ipar,ichan,0,ord(islot1));
          IPosition trc1(4,ipar,ichan,nAnt()-1,ord(islot1));

	  while (islot0<nslots && ntrue(sok(blc0,trc0))<1) {
	    ++islot0;
	    blc0(3)=trc0(3)=ord(islot0);
	  }
	  islot1=islot0+1;

	  // Only if we found more than one good slot
	  if (islot1<nslots) {

	    Complex rph(1.0);
	    Float refantph(0.0);
	    Int currrefant(-1);
	    while (islot1<nslots) {
	      
	      blc1(3)=trc1(3)=ord(islot1);
	      
	      // If current slot has good solutions in it,
	      //  attempt to reference their phases
	      if (ntrue(sok(blc1,trc1))>0) {
		
		iref=0;
		while ( iref<(nAnt()+2) &&
			(!sok(IPosition(4,ipar,ichan,refantlist(iref),ord(islot0))) ||
			 !sok(IPosition(4,ipar,ichan,refantlist(iref),ord(islot1))))   ) {
		  ++iref;
		}
		
		if (iref<nAnt()+2) {
		  // found a refant, use it
		  
		  if (iref>0) usedaltrefant=True;
		  
		  currrefant=refantlist(iref);
  
		  // Only report if using an alternate refant
		  if (currrefant!=lastrefant && iref>0) {
		    logSink() << LogIO::NORMAL
			      << "At " 
			      << MVTime(cs().time(ispw)(ord(islot1))/C::day).string(MVTime::YMD,7) 
			      << " ("
			      << "Spw=" << ispw 
			      << ", Fld=" << cs().fieldId(ispw)(ord(islot1))
			      << ", pol=" << ipar 
			      << ", chan=" << ichan 
			      << ")"
		      //			      << ", using refant " << msantcol.name()(currrefant)
			      << ", using refant " << csmi.getAntName(currrefant)
			      << " (id=" << currrefant 
			      << ")" << " (alternate)"
			      << LogIO::POST;
		  }
		  
		  lastrefant=currrefant;
		  
		  // 2nd priority refant on next iteration is the one used this iteration
		  refantlist(1)=currrefant;
		  
		  Complex &r0=sol(IPosition(4,ipar,ichan,currrefant,ord(islot0)));
		  Complex &r1=sol(IPosition(4,ipar,ichan,currrefant,ord(islot1)));
		  
		  // If we can calculate a meaningful ref phasor, do it
		  if (abs(r0)>0.0f && abs(r1)>0.0f) {
		    
		    rph=Complex(DComplex(r1)/DComplex(r0));
		    rph/=abs(rph);
		    
		    // Remember first encountered refant phase
		    if (refantph==0.0 && iref==0) refantph=arg(r0);
		    
		    // TBD: attempt to optimize precision...
		    //DComplex drph=(DComplex(r1)/DComplex(r0));
		    //drph/=abs(drph);
		    
		    // Adjust each good ant by this phasor
		    for (Int iant=0;iant<nAnt();++iant) 
		      if (sok(IPosition(4,ipar,ichan,iant,ord(islot1)))) {
			sol(IPosition(4,ipar,ichan,iant,ord(islot1)))/=rph;
			
			// TBD: attempt to optimize precision
			//DComplex stmp=sol(IPosition(4,ipar,ichan,iant,ord(islot1)));
			//stmp/=drph;
			//sol(IPosition(4,ipar,ichan,iant,ord(islot1)))=Complex(stmp);
		      }
		    
		    //		  cout << " --> " << arg(r1)*180.0/C::pi << endl;
		    
		  } // non-zero reference amps
		  else 
		    // Should be impossible to reach here!
		    cout << "Bad referencing phasors." << endl;
		  
		  
		} // refant ok
		else {
		  // Continuity broken...
		  logSink() << LogIO::WARN
			    << " Phase continuity broken at "
			    << MVTime(cs().time(ispw)(ord(islot1))/C::day).string(MVTime::YMD,7) 
			    << " ("
			    << "Spw=" << ispw 
			    << ", Fld=" << cs().fieldId(ispw)(ord(islot1))
			    << ", pol=" << ipar 
			    << ")" << endl
			    << "  due to lack of overlapping antennas with previous interval."
			    << LogIO::POST;
		}
		
		// Good slot is basis for referencing the next slot
		islot0=islot1;
		blc0(3)=trc0(3)=ord(islot0);
		
	      }
	      ++islot1;
	    }
	    
	    // Zero initial refant phase
	    if (refantph!=0.0) {
	      //	    cout << "refantph = " << refantph*180.0/C::pi << endl;
	      rph=Complex(cos(refantph),sin(refantph));
	      for (Int islot=0;islot<nslots;++islot)
		for (Int iant=0;iant<nAnt();++iant) 
		  if (sok(IPosition(4,ipar,ichan,iant,ord(islot))))
		    sol(IPosition(4,ipar,ichan,iant,ord(islot)))/=rph;
	    }
	    
	  } // islot0<nslots
	  }
	  else {

	  // This is the old way (out of time order)

	  IPosition blc(3,ipar,ichan,0);
	  IPosition trc(3,ipar,ichan,nAnt()-1);

	  ArrayIterator<Complex> sol(cs().par(ispw),3);
	  ArrayIterator<Bool> sOk(cs().parOK(ispw),3);

	  Int islot(0);
	  
	  // Advance to first slot that has some good solutions
	  while (ntrue(sOk.array()(blc,trc))<1 &&
		 !sol.pastEnd()) {
	    sol.next();
	    sOk.next();
	    islot++;
	  }

	  // Arrays for referencing slices
	  Cube<Complex> s0, s1;
	  Cube<Bool> ok0, ok1;

	  // We are at the initial "prior" solution
	  s0.reference(sol.array());
	  ok0.reference(sOk.array());
	  
	  // Nominally, first solution to adjust is the next one
	  sol.next();
	  sOk.next();
	  islot++;
	
	  Complex rph(1.0);
	  Int lastiref(-1);
	  while (!sol.pastEnd()) {
	
	    // Do referencing if current slot has any good solutions
	    if (ntrue(sOk.array()(blc,trc))>0) {
	      
	      // The current solution
	      s1.reference(sol.array());
	      ok1.reference(sOk.array());

	      // Find first refant this and prev slot have in common
	      Int iref=0;
	      while ( iref<(nAnt()+1) &&
		      (!ok0(ipar,ichan,refantlist(iref)) || 
		       !ok1(ipar,ichan,refantlist(iref)))  ) iref++;

	      if (iref>nAnt())
		cout << "No antenna overlap..." << endl;
	      else {
		// found a refant, use it

		if (iref!=lastiref)
		  cout << "Using refant id=" << refantlist(iref) << " at "
		       << MVTime(cs().time(ispw)(islot)/C::day).string(MVTime::YMD,7) << " "
		       << "(islot=" << islot 
		       << ", Spw=" << ispw 
		       << ", Fld=" << cs().fieldId(ispw)(islot)
		       << ", ipar=" << ipar 
		       << ")" << endl;

		lastiref=iref;
	    
	      		
		Complex &r0=s0(ipar,ichan,refantlist(iref));
		Complex &r1=s1(ipar,ichan,refantlist(iref));

		//		cout << arg(r0)*180.0/C::pi << " "
		//		     << arg(r1)*180.0/C::pi << " ";

		
		// If we can calculate a meaningful ref phasor, do it
		if (abs(r0)>0.0f && abs(r1)>0.0f) {
		  
		  rph=r1/r0;
		  rph/=abs(rph);
		  
		  //		  cout << "(" << arg(rph)*180.0/C::pi << ") ";


		  /*
		    cout << islot << " " << ichan << " " << ipar << " "
		    << " refant = " << refantlist(iref) << " ph="
		    << arg(rph)*180.0/C::pi 
		    << endl;
		  */
		  
		  // Adjust each good ant by this phasor
		  for (Int iant=0;iant<nAnt();++iant) 
		    if (ok1(ipar,ichan,iant))
		      s1(ipar,ichan,iant)/=rph;

		  //		  cout << " --> " << arg(r1)*180.0/C::pi << endl;

		} // non-zero reference amps
		else 
		  cout << "Bad referencing phasors." << endl;


	      } // refant ok

	      // This slot is now basis for referencing the next one 
	      s0.reference(s1);
	      ok0.reference(ok1);
	      
	    } // a good interval

	    // Advance to next solution
	    islot++;
	    sol.next();
	    sOk.next();

	  } // !pastEnd
	  
	  } // newway

	} // nTime>1
	
      } // ipar
    } // ichan
    }      
  } // ispw


  if (usedaltrefant)
    logSink() << LogIO::NORMAL
	      << " NB: An alternate refant was used at least once to maintain" << endl
	      << "  phase continuity where the user's refant drops out." << endl
	      << "  This may introduce apparent phase jumps in " << endl
	      << "  the reference antenna; these are generally harmless."
	      << LogIO::POST;

}

void SolvableVisJones::fluxscale(const Vector<Int>& refFieldIn,
                                 const Vector<Int>& tranFieldIn,
                                 const Vector<Int>& inRefSpwMap,
				 const Vector<String>& fldNames,
                                 Matrix<Double>& fluxScaleFactor) {

  // For updating the MS History Table
  //  LogSink logSink_p = LogSink(LogMessage::NORMAL, False);
  //  logSink_p.clearLocally();
  //  LogIO oss(LogOrigin("calibrater", "fluxscale()"), logSink_p);

  // PtrBlocks to hold mean gain moduli and related
  PtrBlock< Matrix<Bool>* >   ANTOK;
  PtrBlock< Matrix<Double>* > MGNORM;
  PtrBlock< Matrix<Double>* > MGNORM2;
  PtrBlock< Matrix<Double>* > MGNWT;
  PtrBlock< Matrix<Double>* > MGNVAR;
  PtrBlock< Matrix<Int>* >    MGNN;

  Int nMSFld; fldNames.shape(nMSFld);

  // assemble complete list of available fields
  Vector<Int> fldList;
  for (Int iSpw=0;iSpw<nSpw();iSpw++) {
    Int currlen;
    fldList.shape(currlen);

    if (cs().nTime(iSpw) > 0) {

      //      cout << "cs().fieldId(iSpw) = " << cs().fieldId(iSpw) << endl;

      Vector<Int> thisFldList; thisFldList=cs().fieldId(iSpw);
      Int nThisFldList=genSort(thisFldList,(Sort::QuickSort | Sort::NoDuplicates));
      thisFldList.resize(nThisFldList,True);
      fldList.resize(currlen+nThisFldList,True);
      for (Int ifld=0;ifld<nThisFldList;ifld++) {
        fldList(currlen+ifld) = thisFldList(ifld);
      }
    }
  }
  Int nFldList=genSort(fldList,(Sort::QuickSort | Sort::NoDuplicates));
  fldList.resize(nFldList,True);

  //  cout << "fldList = " << fldList << endl;

  Int nFld=max(fldList)+1;

  try {

    // Resize, NULL-initialize PtrBlocks
    ANTOK.resize(nFld);   ANTOK=NULL;
    MGNORM.resize(nFld);  MGNORM=NULL;
    MGNORM2.resize(nFld); MGNORM2=NULL;
    MGNWT.resize(nFld);   MGNWT=NULL;
    MGNVAR.resize(nFld);  MGNVAR=NULL;
    MGNN.resize(nFld);    MGNN=NULL;

    // sort user-specified fields
    Vector<Int> refField; refField = refFieldIn;
    Vector<Int> tranField; tranField = tranFieldIn;
    Int nRef,nTran;
    nRef=genSort(refField,(Sort::QuickSort | Sort::NoDuplicates));
    nTran=genSort(tranField,(Sort::QuickSort | Sort::NoDuplicates));

    // make masks for ref/tran among available fields
    Vector<Bool> tranmask(nFldList,True);
    Vector<Bool> refmask(nFldList,False);
    for (Int iFld=0; iFld<nFldList; iFld++) {
      if ( anyEQ(refField,fldList(iFld)) ) {
        // this is a ref field
        refmask(iFld)=True;
        tranmask(iFld)=False;
      }
    }

    // Check availability of all ref fields
    if (ntrue(refmask)==0) {
      throw(AipsError(" Cannot find specified reference field(s)"));
    }
    // Any fields present other than ref fields?
    if (ntrue(tranmask)==0) {
      throw(AipsError(" Cannot find solutions for transfer field(s)"));
    }

    // make implicit reference field list
    MaskedArray<Int> mRefFldList(fldList,LogicalArray(refmask));
    Vector<Int> implRefField(mRefFldList.getCompressedArray());

    //    cout << "implRefField = " << implRefField << endl;
    // Check for missing reference fields
    if (Int(ntrue(refmask)) < nRef) {
      ostringstream x;
      for (Int iRef=0; iRef<nRef; iRef++) {
        if ( !anyEQ(fldList,refField(iRef)) ) {
          if (refField(iRef)>-1 && refField(iRef) < nMSFld) x << fldNames(refField(iRef)) << " ";
          else x << "Index="<<refField(iRef)+1<<"=out-of-range ";
        }
      }
      String noRefSol=x.str();
      logSink() << LogIO::WARN
		<< " The following reference fields have no solutions available: "
		<< noRefSol
		<< LogIO::POST;
      refField.reference(implRefField);
    }
    refField.shape(nRef);

    // make implicit tranfer field list
    MaskedArray<Int> mTranFldList(fldList,LogicalArray(tranmask));
    Vector<Int> implTranField(mTranFldList.getCompressedArray());
    Int nImplTran; implTranField.shape(nImplTran);

    //    cout << "implTranField = " << implTranField << endl;

    // Check availability of transfer fields

    // If user specified no transfer fields, use implicit 
    //  transfer field list, ELSE check for missing tran fields
    //  among those they specified
    if (nTran==0) {
      tranField.reference(implTranField);
      logSink() << LogIO::NORMAL
		<< " Assuming all non-reference fields are transfer fields."
		<< LogIO::POST;
    } else {
      if ( !(nTran==nImplTran &&
             allEQ(tranField,implTranField)) ) {
        ostringstream x;
        for (Int iTran=0; iTran<nTran; iTran++) {
          if ( !anyEQ(implTranField,tranField(iTran)) ) {
            if (tranField(iTran)>-1 && tranField(iTran) < nMSFld) x << fldNames(tranField(iTran)) << " ";
            else x << "Index="<<tranField(iTran)+1<<"=out-of-range ";
          }
        }
        String noTranSol=x.str();
	logSink() << LogIO::WARN
		  << " The following transfer fields have no solutions available: "
		  << noTranSol
		  << LogIO::POST;
        tranField.reference(implTranField);
      }
    }
    tranField.shape(nTran);

    // Report ref, tran field info
    String refNames(fldNames(refField(0)));
    for (Int iRef=1; iRef<nRef; iRef++) {
      refNames+=" ";
      refNames+=fldNames(refField(iRef));
    }
    logSink() << " Found reference field(s): " << refNames 
	      << LogIO::POST;
    String tranNames(fldNames(tranField(0)));
    for (Int iTran=1; iTran<nTran; iTran++) {
      tranNames+=" ";
      tranNames+=fldNames(tranField(iTran));
    }
    logSink() << " Found transfer field(s):  " << tranNames 
	      << LogIO::POST;

    //    cout << "fldList = " << fldList << endl;

    //    cout << "nFld = " << nFld << endl;


    // Handle spw referencing
    Vector<Int> refSpwMap;
    refSpwMap.resize(nSpw());
    indgen(refSpwMap);

    if (inRefSpwMap(0)>-1) {
      if (inRefSpwMap.nelements()==1) {
        refSpwMap=inRefSpwMap(0);
        logSink() << " All spectral windows will be referenced to spw=" << inRefSpwMap(0) 
		  << LogIO::POST;
      } else {
        for (Int i=0; i<Int(inRefSpwMap.nelements()); i++) {
          if (inRefSpwMap(i)>-1 && inRefSpwMap(i)!=i) {
            refSpwMap(i)=inRefSpwMap(i);
	    logSink() << " Spw=" << i << " will be referenced to spw=" << inRefSpwMap(i) 
		      << LogIO::POST;
          }
        }
      }
    }

    // Scale factor info
    fluxScaleFactor.resize(nSpw(),nFld); fluxScaleFactor=-1.0;

    Matrix<Double> fluxScaleError(nSpw(),nFld,-1.0);
    Matrix<Double> gainScaleFactor(nSpw(),nFld,-1.0);
    Matrix<Bool> scaleOK(nSpw(),nFld,False);

    Matrix<Double> fluxScaleRatio(nSpw(),nFld,0.0);
    Matrix<Double> fluxScaleRerr(nSpw(),nFld,0.0);

    // Field names for log messages
    Vector<String> fldname(nFld,"");

    //    cout << "Filling mgnorms....";

    // fill per-ant -fld, -spw  mean gain moduli
    for (Int iSpw=0; iSpw<nSpw(); iSpw++) {

      if (cs().nTime(iSpw) > 0 ) {

        for (Int islot=0; islot<cs().nTime(iSpw); islot++) {
          Int iFld=cs().fieldId(iSpw)(islot);
          if (ANTOK[iFld]==NULL) {
            // First time this field, allocate ant/spw matrices
            ANTOK[iFld]   = new Matrix<Bool>(nElem(),nSpw(),False);
            MGNORM[iFld]  = new Matrix<Double>(nElem(),nSpw(),0.0);
            MGNORM2[iFld] = new Matrix<Double>(nElem(),nSpw(),0.0);
            MGNWT[iFld]   = new Matrix<Double>(nElem(),nSpw(),0.0);
            MGNVAR[iFld]  = new Matrix<Double>(nElem(),nSpw(),0.0);
            MGNN[iFld]    = new Matrix<Int>(nElem(),nSpw(),0);

            // record name
            fldname(iFld) = cs().fieldName(iSpw)(islot);
          }
          // References to PBs for syntactical convenience
          Matrix<Bool>   antOK;   antOK.reference(*(ANTOK[iFld]));
          Matrix<Double> mgnorm;  mgnorm.reference(*(MGNORM[iFld]));
          Matrix<Double> mgnorm2; mgnorm2.reference(*(MGNORM2[iFld]));
          Matrix<Double> mgnwt;   mgnwt.reference(*(MGNWT[iFld]));
          Matrix<Int>    mgnn;    mgnn.reference(*(MGNN[iFld]));

          for (Int iAnt=0; iAnt<nElem(); iAnt++) {
	    Double wt=cs().iFitwt(iSpw)(iAnt,islot);
	    
	    for (Int ipar=0; ipar<nPar(); ipar++) {
	      IPosition ip(4,ipar,0,iAnt,islot);
	      if (cs().parOK(iSpw)(ip)) {
		antOK(iAnt,iSpw)=True;
		Double gn=norm( cs().par(iSpw)(ip) );
		mgnorm(iAnt,iSpw) += (wt*gn);
		mgnorm2(iAnt,iSpw)+= (wt*gn*gn);
		mgnn(iAnt,iSpw)++;
		mgnwt(iAnt,iSpw)+=wt;
	      }
            }
          }
        }
      }
    }
    //    cout << "done." << endl;

    //    cout << "Normalizing mgnorms...";

    // normalize mgn
    for (Int iFld=0; iFld<nFld; iFld++) {

      //      cout << "iFld = " << iFld << " " << ANTOK[iFld]->column(0) << endl;

      // Have data for this field?
      if (ANTOK[iFld]!=NULL) {

        // References to PBs for syntactical convenience
        Matrix<Bool>   antOK;   antOK.reference(*(ANTOK[iFld]));
        Matrix<Double> mgnorm;  mgnorm.reference(*(MGNORM[iFld]));
        Matrix<Double> mgnorm2; mgnorm2.reference(*(MGNORM2[iFld]));
        Matrix<Double> mgnwt;   mgnwt.reference(*(MGNWT[iFld]));
        Matrix<Double> mgnvar;  mgnvar.reference(*(MGNVAR[iFld]));
        Matrix<Int>    mgnn;    mgnn.reference(*(MGNN[iFld]));

        for (Int iSpw=0; iSpw<nSpw(); iSpw++) {
          for (Int iAnt=0; iAnt<nElem(); iAnt++) {
            if ( antOK(iAnt,iSpw) && mgnwt(iAnt,iSpw)>0.0 ) {
              mgnorm(iAnt,iSpw)/=mgnwt(iAnt,iSpw);
              mgnorm2(iAnt,iSpw)/=mgnwt(iAnt,iSpw);
              // Per-ant, per-spw variance (non-zero only if sufficient data)
              if (mgnn(iAnt,iSpw) > 2) {
                mgnvar(iAnt,iSpw) = (mgnorm2(iAnt,iSpw) - pow(mgnorm(iAnt,iSpw),2.0))/(mgnn(iAnt,iSpw)-1);
              }
            } else {
              mgnorm(iAnt,iSpw)=0.0;
              mgnwt(iAnt,iSpw)=0.0;
              antOK(iAnt,iSpw)=False;
            }
       /*
            cout << endl;
            cout << "iFld = " << iFld;
            cout << " iAnt = " << iAnt;
            cout << " mgnorm = " << mgnorm(iAnt,iSpw);
            cout << " +/- " << sqrt(1.0/mgnwt(iAnt,iSpw));
            cout << " SNR = " << mgnorm(iAnt,iSpw)/sqrt(1.0/mgnwt(iAnt,iSpw));
            cout << "  " << mgnn(iAnt,iSpw);
            cout << endl;
       */
          }
        }


      }
    }

    //    cout << "done." << endl;
    //    cout << "nTran = " << nTran << endl;

    //    cout << "Calculating scale factors...";

    // Scale factor calculation, per spw, trans fld
    for (Int iTran=0; iTran<nTran; iTran++) {
      Int tranidx=tranField(iTran);

      // References to PBs for syntactical convenience
      Matrix<Bool>   antOKT;  antOKT.reference(*(ANTOK[tranidx]));
      Matrix<Double> mgnormT; mgnormT.reference(*(MGNORM[tranidx]));
      Matrix<Double> mgnvarT; mgnvarT.reference(*(MGNVAR[tranidx]));
      Matrix<Double> mgnwtT;  mgnwtT.reference(*(MGNWT[tranidx]));

      for (Int iSpw=0; iSpw<nSpw(); iSpw++) {

        // Reference spw may be different
        Int refSpw(refSpwMap(iSpw));

        // Only if anything good for this spw
        if (ntrue(antOKT.column(iSpw)) > 0) {

          // Numerator/Denominator for this spw, trans fld
          Double sfref=0.0;
          Double sfrefwt=0.0;
          Double sfrefvar=0.0;
          Int sfrefn=0;

          Double sftran=0.0;
          Double sftranwt=0.0;
          Double sftranvar=0.0;
          Int sftrann=0;

          Double sfrat=0.0;
          Double sfrat2=0.0;
          Int sfrn=0;

	  Int nAntUsed(0);
          for (Int iAnt=0; iAnt<nElem(); iAnt++) {

            // this ant OK for this tran field?
            if ( antOKT(iAnt,iSpw) ) {

              // Check ref fields and accumulate them
              Bool refOK=False;
              Double sfref1=0.0;
              Double sfrefwt1=0.0;
              Double sfrefvar1=0.0;
              Int sfrefn1=0;
              for (Int iRef=0; iRef<nRef; iRef++) {

                Int refidx=refField(iRef);
                Matrix<Bool> antOKR; antOKR.reference(*(ANTOK[refidx]));
                Bool thisRefOK(antOKR(iAnt,refSpw));
                refOK = refOK || thisRefOK;
                if (thisRefOK) {
                  Matrix<Double> mgnormR; mgnormR.reference(*(MGNORM[refidx]));
                  Matrix<Double> mgnwtR;  mgnwtR.reference(*(MGNWT[refidx]));
                  Matrix<Double> mgnvarR; mgnvarR.reference(*(MGNVAR[refidx]));
                  sfref1   += mgnwtR(iAnt,refSpw)*mgnormR(iAnt,refSpw);
                  sfrefwt1 += mgnwtR(iAnt,refSpw);
                  if (mgnvarR(iAnt,refSpw)>0.0) {
                    sfrefvar1+= (mgnwtR(iAnt,refSpw)/mgnvarR(iAnt,refSpw));
                    sfrefn1++;
                  }

                }
              }

              // If ref field accumulation ok for this ant, accumulate
              if (refOK) {
		nAntUsed+=1;
                sftran   += (mgnwtT(iAnt,iSpw)*mgnormT(iAnt,iSpw));
                sftranwt += mgnwtT(iAnt,iSpw);
                if (mgnvarT(iAnt,iSpw)>0.0) {
                  sftranvar+= (mgnwtT(iAnt,iSpw)/mgnvarT(iAnt,iSpw));
                  sftrann++;
                }

                sfref    += sfref1;
                sfrefwt  += sfrefwt1;
                sfrefvar += sfrefvar1;
                sfrefn   += sfrefn1;

                // Accumlate per-ant ratio
                Double thissfrat= mgnormT(iAnt,iSpw)/(sfref1/sfrefwt1);
                sfrat    += thissfrat;
                sfrat2   += (thissfrat*thissfrat);
                sfrn++;
                //cout << "Ratio: " << thissfrat << " " << sfrat << " " << sfrat2 << " " << sfrn << endl;
              }



            } // antOKT
          } // iAnt

          // normalize numerator and denominator, variances
          if (sftranwt > 0.0) {
            sftran/=sftranwt;
            sftranvar/=sftranwt;
            if (sftranvar > 0.0 && sftrann > 0) {
              sftranvar*=sftrann;
              sftranvar=1.0/sftranvar;
            }
          }
          if (sfrefwt  > 0.0) {
            sfref/=sfrefwt;
            sfrefvar/=sfrefwt;
            if (sfrefvar > 0.0 && sfrefn > 0) {
              sfrefvar*=sfrefn;
              sfrefvar=1.0/sfrefvar;
            }
          }

          //      cout << endl;
          //      cout << "Tran = " << sftran << " +/- " << sqrt(sftranvar) << endl;
          //      cout << "Ref  = " << sfref  << " +/- " << sqrt(sfrefvar) << endl;

          // form scale factor for this tran fld, spw
          if (sftran > 0.0 && sfref > 0.0) {
            fluxScaleFactor(iSpw,tranidx) = sftran/sfref;
            fluxScaleError(iSpw,tranidx)  = sqrt( sftranvar/(sftran*sftran) + sfrefvar/(sfref*sfref) );
            fluxScaleError(iSpw,tranidx) *= fluxScaleFactor(iSpw,tranidx);
            gainScaleFactor(iSpw,tranidx) = sqrt(1.0/fluxScaleFactor(iSpw,tranidx));
            scaleOK(iSpw,tranidx)=True;

          /*
            cout << "iTran = " << iTran;
            cout << "  fluxScaleFactor = " << fluxScaleFactor(iSpw,tranidx);
            cout << " +/- " << fluxScaleError(iSpw,tranidx);
            cout << "  gainScaleFactor = " << gainScaleFactor(iSpw,tranidx);
            cout << endl;
          */
            // Report flux densities:
	    logSink() << " Flux density for " << fldNames(tranidx)
		      << " in SpW=" << iSpw;
            if (refSpw!=iSpw) 
	      logSink() << " (ref SpW=" << refSpw << ")";

	    logSink() << " is: " << fluxScaleFactor(iSpw,tranidx)
		      << " +/- " << fluxScaleError(iSpw,tranidx)
		      << " (SNR = " << fluxScaleFactor(iSpw,tranidx)/fluxScaleError(iSpw,tranidx)
		      << ", nAnt= " << nAntUsed << ")"
		      << LogIO::POST;

            // form scale factor from mean ratio
            sfrat  /= (Double(sfrn));
            sfrat2 /= (Double(sfrn));
            fluxScaleRatio(iSpw,tranidx) = sfrat;
            fluxScaleRerr(iSpw,tranidx) = sqrt( (sfrat2 - sfrat*sfrat) / Double(sfrn-1) );

          /*
            cout << "iTran = " << iTran;
            cout << "  fluxScaleRatio = " << fluxScaleRatio(iSpw,tranidx);
            cout << " +/- " << fluxScaleRerr(iSpw,tranidx);
            cout << endl;
          */
          } else {
            logSink() << LogIO::WARN
		      << " Insufficient information to calculate scale factor for "
		      << fldname(tranidx)
		      << " in SpW="<< iSpw
		      << LogIO::POST;
          }

        } // ntrue(antOKT) > 0
      } // iSpw
    } // iTran

    // quit if no scale factors found
    if (ntrue(scaleOK) == 0) throw(AipsError("No scale factors determined!"));

    //    cout << "done." << endl;

    //    cout << "Adjusting gains...";

    // Adjust tran field's gains here
    for (Int iSpw=0; iSpw<nSpw(); iSpw++) {
      if (cs().nTime(iSpw)>0) {
        for (Int islot=0; islot<cs().nTime(iSpw); islot++) {
          Int iFld=cs().fieldId(iSpw)(islot);
          // If this is a tran fld and gainScaleFactor ok
          if (scaleOK(iSpw,iFld)) {
            for (Int iAnt=0; iAnt<nElem(); iAnt++) {
	      for (Int ipar=0; ipar<nPar(); ipar++) {
		IPosition ip(4,ipar,0,iAnt,islot);
		if (cs().parOK(iSpw)(ip))
                  cs().par(iSpw)(ip)*=gainScaleFactor(iSpw,iFld);
              }
            }
          }
        }
      }
    }

    //    cout << "done." << endl;

    //    cout << "Cleaning up...";

    // Clean up PtrBlocks
    for (Int iFld=0; iFld<nFld; iFld++) {
      if (ANTOK[iFld]!=NULL) {
        delete ANTOK[iFld];
        delete MGNORM[iFld];
        delete MGNORM2[iFld];
        delete MGNWT[iFld];
        delete MGNVAR[iFld];
        delete MGNN[iFld];
      }
    }

    //    cout << "done." << endl;

    // Avoid this since problem with <msname>/SELECTED_TABLE (06Feb02 gmoellen)
    /*
    {

      MeasurementSet ms(vs_->msName().before("/SELECTED"));
      Table historytab = Table(ms.historyTableName(),
                               TableLock(TableLock::UserNoReadLocking),
                               Table::Update);
      MSHistoryHandler hist = MSHistoryHandler(ms, "calibrater");
      historytab.lock(True);
      oss.postLocally();
      hist.addMessage(oss);
      historytab.unlock();
    }
    */
  }
  catch (AipsError x) {

    // Clean up PtrBlocks
    for (Int iFld=0; iFld<nFld; iFld++) {
      if (ANTOK[iFld]!=NULL) {
        delete ANTOK[iFld];
        delete MGNORM[iFld];
        delete MGNORM2[iFld];
        delete MGNWT[iFld];
        delete MGNVAR[iFld];
        delete MGNN[iFld];
      }
    }

    throw(x);

  }

}

void SolvableVisJones::listCal(const Vector<Int> ufldids,
                               const Vector<Int> uantids,
                               const Matrix<Int> uchanids, 
			                   const String& listfile,
                               const Int& maxScrRows) {

    char cfill = cout.fill(' '); // Set fill character for terminal output
    
    //Int uSpwID = uchanids(0,0);
    //Int chan = uchanids(0,1);
    
    // The number of spws; that is, the number of rows in matrix uchanids.
    // Actually, this is the number of spw/channel selections made in the
    // spw selection parameter.  The rows of uchanids may contain the same
    // spw.
    uInt nSpws = uchanids.nrow();
    
    // Prep for listing
    Bool endOutput = False; // if true, end output immediately
    Bool prompt = True; // if true, issue prompt
    Int isol = 0; // total solution counter
    Int scrRows=0; // screen row counter, reset after continue prompt
    
    // Redirect cout to listfile
    ofstream file;
    streambuf* sbuf = cout.rdbuf();
    if(listfile!="") { // non-interactive
        prompt = False;
        // Guard against trampling existing file
        File diskfile(listfile);
        if (diskfile.exists()) {
            String errmsg = "File: " + listfile + 
                " already exists; delete it or choose a different name.";
            throw(AipsError(errmsg));
        }
        else
            cout << "Writing output to file: " << listfile << endl;
        logSink() << LogIO::NORMAL2 << "Redirecting output to "
                  << diskfile.path().originalName().data() << LogIO::POST;
        file.open(diskfile.path().originalName().data());
        cout.rdbuf(file.rdbuf());
    } else { 
        logSink() << LogIO::DEBUG1 
                  << "Not redirecting output: cout remains untouched."
                  << LogIO::POST;
    }
    
    // Access to ms for meta info
    /*
    MeasurementSet ms(msName());
    MSAntennaColumns msant(ms.antenna());
    Vector<String> antname(msant.name().getColumn());
    MSFieldColumns msfld(ms.field());
    Vector<String> fldname(msfld.name().getColumn());
    */

    Vector<String> antname=csmi.getAntNames();
    Vector<String> fldname=csmi.getFieldNames();

    // Default header info.
    logSink() << LogIO::NORMAL1 << "MS name = " << msName() << LogIO::POST;        
        
    logSink() << LogIO::DEBUG1 << "Number of spectral window selections (may not be unique) = " 
              << nSpws << LogIO::POST;
    logSink() << LogIO::DEBUG1 << "Number of (unique) spws in cal table = "
              << cs().nSpw() << LogIO::POST;
    // Begin loop through SPWs
    for(uInt iUchanids = 0; iUchanids<nSpws; iUchanids++) {
        logSink() << LogIO::DEBUG2 << "At top of spwID loop" << LogIO::POST;
        Vector<Int> RowUchanids = uchanids.row(iUchanids);
        uInt spwID = RowUchanids(0); // Current spw ID
        
        // If Spw is out of range for cal table, go to next spw.
        if (spwID<0 || cs().nTime(spwID)==0) {
            String errMsg;
            errMsg = "Nothing to list for selected SpwID: " 
                            + errMsg.toString(spwID);
            logSink() << LogIO::NORMAL1 << errMsg << LogIO::POST;
        } else { // Spw is not out of range.  Proceed with listing...

            // Handle channel selection here.
            if(cs().par(spwID).shape()(1) < 0) 
                throw(AipsError("Number of spectral channels in calibration table < 0!"));
            const uInt nchan= (uInt)cs().par(spwID).shape()(1); // number of channels in Cal table
            logSink() << LogIO::DEBUG1 << "For Spw ID " << spwID 
                      << ": Number of spectral channels in calibration table = "
                      << nchan << LogIO::POST;

            // Extract channel selection info from uchanids
            Int stepChan   = RowUchanids(3);
            if (stepChan == 0) { stepChan = 1; } // one channel at a time (default)
            else if (stepChan < 0) {
                throw(AipsError("Stepping backwards through channels not supported."));
            }
            // Cal table channels are a subset of the MS channels.
            // MS indices of channels in cal table.
            const uInt msCalStartChan = cs().startChan()(spwID);
            const uInt msCalStopChan  = cs().startChan()(spwID) + nchan - 1;
            // MS indices of selected channels
            uInt msSelStartChan = RowUchanids(1);
            uInt msSelStopChan  = RowUchanids(2);
            // Cal table indices of selected channels
            Int startChan = msSelStartChan - msCalStartChan;
            Int stopChan  = msSelStopChan - msCalStartChan;
            if (startChan > stopChan) { 
                throw(AipsError("Start channel must be less than or equal to stop channel."));
            } else if ((stopChan < 0) || (startChan > (Int)nchan - 1)) { 
                // All selected channels out of range
                String errMsg = "None of the selected channels are present in cal table.";
                logSink() << LogIO::SEVERE << errMsg
                          << endl << "Selected channel range = [" 
                          << msSelStartChan << "," << msSelStopChan << "]" << endl
                          << "Cal table channel range = [" 
                          << msCalStartChan << "," << msCalStopChan << "]" << LogIO::POST;
                throw(AipsError(errMsg));
            } 
            if (startChan < 0) {
                logSink() << LogIO::NORMAL1 << "Start channel ( " << msSelStartChan
                          << " ) below allowed range." << endl
                          << "Increasing start channel to " << msCalStartChan
                          << LogIO::POST;
                startChan = 0;
            }
            if (stopChan > (Int)nchan - 1) {
                logSink() << LogIO::NORMAL1 << "Stop channel ( " << msSelStopChan
                          << " ) above allowed range." << endl
                          << "Decreasing stop channel to " << msCalStopChan
                          << LogIO::POST;
                stopChan = (Int)nchan - 1;
            }
            // Number of channels selected
            Int numChans = ((stopChan - startChan) / stepChan) + 1; 
            if (numChans > Int(nchan)) {
                logSink() << LogIO::NORMAL1 
                          << "More channels requested than are present in the cal table."
                          << endl << "Cal table channels for this spw: "
                          << msCalStartChan << " to " << msCalStopChan
                          << LogIO::POST;
            } 
            
            logSink() << LogIO::DEBUG1 << "For spwID " << spwID << endl
                      << "  startChan = " << startChan << endl
                      << "  stopChan = " << stopChan << endl
                      << "  stepChan = " << stepChan << endl
                      << "  Number of channels to list: numChans = " << numChans
                      << LogIO::POST;
            
            // Setup the column widths.  Check width of each string.        
            uInt precAmp, precPhase; // Column precision
            uInt oAmp, oPhase; // Column order
            
            wTime_p = 10; // set width of time column
            // set the field column width (looking at the whole cal table)
            wField_p = 0; 
            if(cs().nElem() < 0) throw(AipsError("Number of elements < 0!"));
            for (Int iElem=0;iElem<cs().nElem(); iElem++) {
                for (Int itime=0;itime<cs().nTime(spwID);++itime) {
                    Int fldid(cs().fieldId(spwID)(itime));
                    String fldstr=(fldname(fldid)); 
                    uInt fldstrLength = fldstr.length();
                    if (wField_p < fldstrLength) { wField_p = fldstrLength; }
                }
            }
            wChan_p = (uInt)max(3,(Int)rint(log10(nchan)+0.5));
            wPreAnt_p = wTime_p + 1 + wField_p + 1 + wChan_p; // do not count final pipe ("|")
            logSink() << LogIO::DEBUG1 << "Column widths:" << endl
                      << "  time = "    << wTime_p  << endl
                      << "  field = "   << wField_p << endl
                      << "  channel = " << wChan_p << LogIO::POST;
            
            // For multiple channels, I think I will need to write a method that 
            // looks through all spws to determine the necessary order for the 
            // Amplitude and Phase.
            
            // set width of amplitude column
            //oAmp = (uInt)max(1,(Int)rint(log10(max())+0.5));
            oAmp = 1;
            precAmp = 3; 
            wAmp_p = oAmp + precAmp + 1; // order + precision + decimal point
            
            // set width of phase column
            //oPhase = (uInt)max(1,(Int)rint(abs(log10(max())+0.5)));
            oPhase = 4;
            precPhase = 1; 
            wPhase_p = oPhase + precPhase + 1; // order + precision + decimal point
            
            wFlag_p=1;
            wPol_p = wAmp_p + 1 + wPhase_p + 1 + wFlag_p + 1; 
            wAntCol_p = wPol_p*2; 
            
            logSink() << LogIO::DEBUG1 << "oAmp = " << oAmp 
                << ", precAmp = " << precAmp 
                << ", wAmp_p = " << wAmp_p << endl
                << "oPhase = " << oPhase 
                << ", precPhase = " << precPhase 
                << ", wPhase_p = " << wPhase_p
                << LogIO::POST;
            
            uInt numAntCols = 4; // Number of antenna columns
            wTotal_p = wPreAnt_p + 1 + wAntCol_p*numAntCols;
            
            // Construct the horizontal separator
            String hSeparator=replicate('-',wTotal_p); 
            uInt colPos=0;
            colPos+=wPreAnt_p; hSeparator[colPos]='|'; colPos++;
            for(uInt iPol=0; iPol<numAntCols*2; iPol++) {
                colPos+=wPol_p-1;
                hSeparator[colPos]='|';
                colPos++;
            }
            
            logSink() << LogIO::DEBUG1
                << "Listing CalTable: " << calTableName()
                << "   (" << typeName() << ") "
                << LogIO::POST;
            
            String dateTimeStr0=MVTime(cs().time(spwID)(0)/C::day).string(MVTime::YMD,7);
            String dateStr0=dateTimeStr0.substr(0,10);
            
            String headLine = "SpwID = " + String::toString(spwID) + ", "
                              "Date = " + dateStr0 + ",  " +
                              "CalTable = " + calTableName() + " (" + typeName() + "), " +
                              "MS name = " + msName();
            cout.setf(ios::left,ios::adjustfield);
            cout << setw(wTotal_p) << headLine << endl
                 << replicate('-',wTotal_p) << endl;
            scrRows+=2;
            
            // labels for flagged solutions
            Vector<String> flagstr(2); 
            flagstr(0)="F";
            flagstr(1)=" ";
            
            //  Complex *g=cs().par(spw).data()+dochan;
            //  Bool *gok=cs().parOK(spw).data()+dochan;
            
            Array<Complex> calGains;
            calGains.reference(cs().par(spwID));
            Array<Bool> calGainOK;
            calGainOK.reference(cs().parOK(spwID));
            IPosition gidx(4,0,0,0,0); // 4-element IPosition, all zeros
            
            // Setup a Vector of antenna ID's to ease the process of printing 
            // multiple antenna columns per row.
            uInt numAnts; // Hold number of antennas to be output.
            Vector<Int> pAntids(cs().nElem()); // Vector of antenna ID's.
            if (uantids.nelements()==0) { // Print all antennas.
                numAnts = cs().nElem(); 
                for(uInt i=0; i< numAnts; i++) { // Fill pAntids with all antenna IDs.
                    pAntids[i] = i;
                }
            } else { // Use the user-specified antenna ID's.
                numAnts = uantids.nelements(); 
                pAntids.resize(numAnts);
                pAntids = uantids;
            }
            
            // loop over antenna elements
            for (uInt iElem=0;iElem<numAnts;iElem+=numAntCols) { 
                gidx(2)=pAntids(iElem);
                
                Bool header=True; // New antenna, require print header
                
                // If antenna element is among selected antennas, print it
                if (uantids.nelements()==0 || anyEQ(uantids,pAntids(iElem))) {
                    
                    // Begin loop over time
                    for (Int itime=0;itime<cs().nTime(spwID);++itime) { 
                        gidx(3)=itime;
                        
                        Int fldid(cs().fieldId(spwID)(itime));
                        
                        // Get date-time string
                        String dateTimeStr=MVTime(cs().time(spwID)(itime)/C::day).string(MVTime::YMD,7);
                        String dateStr=dateTimeStr.substr(0,10);
                        String timeStr=dateTimeStr.substr(11,10);
                        // Get field string
                        String fldStr=(fldname(fldid));
                        
                        String tmp_timestr = timeStr; // tmp_ variables get reset during the loop
                        String tmp_fldstr = fldStr; //
                        
                        // If no user-specified fields, or fldid is in user's list
                        if (ufldids.nelements()==0 || anyEQ(ufldids,fldid) ) {
                            
                            // Set i/o flags for writing data
                            cout << setiosflags(ios::fixed) << setiosflags(ios::right);
                            
                            // Loop over channels
                            for (uInt iChan=startChan; iChan<=uInt(stopChan); iChan+=stepChan) {
                                
                                // If beginning new screen, print the header
                                if (scrRows == 0 || header) {
                                    header=False;
                                    // Write Antenna line (put spaces over the time, field cols)
                                    for(uInt k=0; k<(wPreAnt_p); k++) { cout<<" "; }
                                    cout << "|";
                                    for(uInt k=0; (k<numAntCols) and (iElem+k<=numAnts-1); k++) {
                                        String tAntName = " Ant = " + String(antname(pAntids(iElem+k)));
                                        cout.setf(ios::left,ios::adjustfield);
                                        cout << setw(wAntCol_p-1) << tAntName << "|";
                                    }
                                    cout << endl;    scrRows++;
                                    
                                    // Write part of header with no data
                                    scrRows += writeHeader(numAntCols, numAnts, iElem);
                                }
                                
                                
                                gidx(1)=iChan;
                                // Write the Time, Field, and Channel for each row
                                cout << setw(wTime_p) << timeStr << " "
                                     << setw(wField_p) << fldStr << " "
                                     << setw(wChan_p) << msCalStartChan + iChan << "|";
                                
                                // Write data for each antenna column
                                for (uInt kelem=iElem; (kelem<iElem+numAntCols) and (kelem<=numAnts-1); 
                                     kelem++) {
                                    gidx(2)=pAntids(kelem);
                                    
                                    // Loop over polarization
                                    for (Int ipar=0;ipar<nPar();++ipar) {
                                        gidx(0)=ipar; 
                                        
                                        // WRITE THE DATA
                                             // amplitude
                                        cout << setprecision(precAmp) << setw(wAmp_p) << abs(calGains(gidx)) << " "
                                             // phase
                                             << setprecision(precPhase) << setw(wPhase_p) << arg(calGains(gidx))*180.0/C::pi << " "
                                             // flag
                                             << flagstr(Int(calGainOK(gidx))) << " ";
                                    } // end ipar loop
                                    
                                } // end kelem loop
                                
                                cout << resetiosflags(ios::right) << endl;
                                isol=isol+numAntCols; scrRows++;
                                
                                // If at end of screen prompt user or new header
                                if (maxScrRows>0 && (scrRows >= maxScrRows-1) ) { // scrRows counts from 0
                                    scrRows = 0; // signal a new page
                                    if (prompt) { // query the user, if we are interactive
                                        string contStr;
                                        cout << "Type Q to quit, A to list all, or RETURN to continue [continue]: ";
                                        getline(cin,contStr);
                                        if ( (contStr.compare(0,1,"q") == 0) or 
                                             (contStr.compare(0,1,"Q") == 0) ) { endOutput=True; }
                                        if ( (contStr.compare(0,1,"a") == 0) or 
                                             (contStr.compare(0,1,"A") == 0) ) { prompt = False; }
                                    }
                                }
                            } // end iChan loop
                            
                        } // end if (field)
                        
                        if (endOutput) {break;} // break out of itime loop
                        
                    } // itime
                    
                } // end if (antenna)
                
                if (endOutput) {break;} // break out of ielem loop
                
            } // end iElem loop
            
            if (endOutput) {break;} // break out of spw loop

        } // end spw if (verification) block
        
    } // end spw loop   
    
    cout << endl
         << "Listed " << isol << " antenna solutions." 
         << endl << endl;
         
    if (listfile!="") cout.rdbuf(sbuf); // restore cout
    cout.fill(cfill);

}// end function listCal

int SolvableVisJones::writeHeader(const uInt numAntCols, 
                                  const uInt numAnts,
                                  const uInt iElem) {
    uInt lineCount=0;
    // Write time and field headings (only once)
    cout << setiosflags(ios::left)
         << setw(wTime_p) << "Time"  << " "
         << setw(wField_p) << "Field" << " "
         << setw(wChan_p) << "Chn" << "|";
    
    // Write Amp and Phase headings for each antenna column
    cout.setf(ios::right, ios::adjustfield);
    for(uInt k=0; (k<numAntCols) and (iElem+k<=numAnts-1); k++) {
        cout << setw(wAmp_p)   << "Amp"   << " " 
             << setw(wPhase_p) << "Phs" << " "
             << "F" << " "
             << setw(wAmp_p)   << "Amp"   << " " 
             << setw(wPhase_p) << "Phs" << " "
             << "F" << "|";
    }
    cout << endl;    lineCount++;
    // Construct the horizontal separator
    String hSeparator=replicate('-',wTotal_p); 
    uInt colPos=0;
    colPos+=wTime_p; hSeparator[colPos]='|'; colPos++;
    colPos+=wField_p; hSeparator[colPos]='|'; colPos++;
    colPos+=wChan_p; hSeparator[colPos]='|'; colPos++;
    for(uInt iPol=0; iPol<numAntCols*2; iPol++) {
        colPos+=wPol_p-1;
        hSeparator[colPos]='|';
        colPos++;
    }
    // Write horizontal separator
    cout << hSeparator << endl;   lineCount++;
    return lineCount;
} // end writeHeader

// Globals

// Return a cal table's type, verifying its existence
String calTableType(const String& tablename) {

  // Check existence...
  if (!Table::isReadable(tablename)) {
    ostringstream o;
    o << "Table " << tablename
      << " does not exist.";
    throw(AipsError(String(o)));
  }

  // Table exists...
  
  TableInfo ti(Table::tableInfo(tablename));
  
  // ...Check if Calibration table....
  if (ti.type()!="Calibration") {
    ostringstream o;
    o << "Table " << tablename
      << " is not a valid Calibration table."
      << " (expected type = \"Calibration\"; type found = \""
      << ti.type() << "\")";
    throw(AipsError(String(o)));
    
  }
  
  // If we get here, we have a calibration table,
  //  so return its type
  return ti.subType();

}


} //# NAMESPACE CASA - END
