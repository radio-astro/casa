//# Flagger.cc: this defines Flagger
//# Copyright (C) 2000,2001,2002
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
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <casa/BasicSL/Complex.h>
#include <measures/Measures/Stokes.h>
#include <casa/Utilities/Regex.h>
#include <casa/OS/HostInfo.h>
#include <flagging/Flagging/Flagger.h>
#include <flagging/Flagging/RFAFlagExaminer.h>
#include <flagging/Flagging/RFAMedianClip.h>
#include <flagging/Flagging/RFASpectralRej.h>
#include <flagging/Flagging/RFASelector.h>
#include <flagging/Flagging/RFAUVBinner.h>
#include <flagging/Flagging/RFATimeFreqCrop.h>
#include <flagging/Flagging/RFAApplyFlags.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/System/PGPlotter.h>
#include <casa/System/ProgressMeter.h>
#include <casa/stdio.h>
#include <casa/math.h>
#include <stdarg.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/SetupNewTab.h>

#include <tables/Tables/ExprNode.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <measures/Measures/Stokes.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/Quanta/MVAngle.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/MVEpoch.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>


#include <flagging/Flagging/RFANewMedianClip.h>

namespace casa {
  
  LogIO Flagger::os( LogOrigin("Flagger") );
  static char str[1024];
  // uInt debug_ifr=9999,debug_itime=9999;
  
  
  // -----------------------------------------------------------------------
  // Default Constructor
  // -----------------------------------------------------------------------
  Flagger::Flagger ():mssel_p(0), vs_p(0)
  {
    dbg=False;
    
    msselection_p = new MSSelection();
    agents_p = NULL;
    agentCount_p=0;
    opts_p = NULL;
    
    // For HISTORY table logging
    logSink_p=LogSink(LogMessage::NORMAL, False);
    hist_p=0;
    histLockCounter_p = 0;
    
    nant=0;
    setdata_p = False;
    selectdata_p = False;
    // setupAgentDefaults();
    pgprep_nx=pgprep_ny=1;
  }
  
  // -----------------------------------------------------------------------
  // Constructor
  // constructs and attaches to MS
  // -----------------------------------------------------------------------
  Flagger::Flagger ( MeasurementSet &mset ) : mssel_p(0), vs_p(0)
  {
    dbg=False;
    
    msselection_p = new MSSelection();
    agents_p = NULL;
    agentCount_p=0;
    opts_p = NULL;
    
    // For HISTORY table logging
    logSink_p=LogSink(LogMessage::NORMAL, False);
    hist_p=0;
    histLockCounter_p = 0;
    
    nant=0;
    setdata_p = False;
    selectdata_p = False;
    attach(mset);
    pgprep_nx=pgprep_ny=1;
  }
  
  Flagger::~Flagger ()
  {
    if ( !ms.tableName().length() ){
      os << "Flagger closing out "<<ms.tableName()<<LogIO::POST;
      ms.flush();
      ms.relinquishAutoLocks(True);
      ms.unlock();
    }
    if (vs_p)  delete vs_p;
    vs_p = 0;
    if (dbg)cout << "Flagger destructor :: about to clean mssel_p" << endl;
    if (mssel_p) delete mssel_p;
    mssel_p = 0;
    if (dbg)cout << "Flagger destructor :: cleaned mssel_p" << endl;
    if (msselection_p) delete msselection_p;
    msselection_p=0;
    if (agents_p) delete agents_p;
    agents_p=NULL;
    if (opts_p) delete opts_p;
    opts_p = NULL;
  }
  
  // -----------------------------------------------------------------------
  // queryOptions
  // Returns record of available options and their default values
  // -----------------------------------------------------------------------
  const RecordInterface & Flagger::defaultOptions ()
  {
    static Record rec;
    // create record description on first entry
    if ( !rec.nfields() )
      {
	Vector<Int> plotscr(2, 3); 
	rec.define(RF_PLOTSCR, plotscr);
	rec.define(RF_PLOTDEV, plotscr);
	rec.define(RF_DEVFILE, "flagreport.ps/ps");
	rec.defineRecord(RF_GLOBAL, Record());
	rec.define(RF_TRIAL, False);
	rec.define(RF_RESET, False);
	
	rec.setComment(RF_PLOTSCR, "Format of screen plots: [NX, NY] or False to disable");
	rec.setComment(RF_PLOTDEV, "Format of hardcopy plots: [NX, NY], or False to disable");
	rec.setComment(RF_DEVFILE, "Filename for hardcopy (a PGPlot 'filename/device')");
	rec.setComment(RF_GLOBAL, "Record of global parameters applied to all methods");
	rec.setComment(RF_TRIAL, "T for trial run (no flags written out)");
	rec.setComment(RF_RESET, "T to reset existing flags before running");
      }
    return rec;
  }
  
  // -----------------------------------------------------------------------
  // Flagger::attach
  // attaches to MS
  // -----------------------------------------------------------------------
  bool Flagger::attach( MeasurementSet &mset, Bool setAgentDefaults )
  {
    
    nant=0;
    setdata_p = False;
    selectdata_p = False;
    if (vs_p) 
      delete vs_p;
    vs_p = 0;
    if (mssel_p)
      delete mssel_p;
    mssel_p = 0;
    if (setAgentDefaults)
      setupAgentDefaults();
    ms = mset;
    
    originalms = mset;
    originalms_p = &mset;
    
    //  	Matrix<Int> noselection;
    // Use default sort order - and no scratch cols....
    //        vs_p = new VisSet(*originalms_p,noselection,0.0);
    //	vs_p->resetVisIter(sort, 0.0);
    
    Matrix<Int> noselection;
    Block<Int> bsort(3);
    bsort[0] = MS::FIELD_ID;
    bsort[1] = MS::DATA_DESC_ID;
    bsort[2] = MS::TIME;
    
    vs_p = new VisSet(mset,bsort,noselection,0.0);
    vs_p->resetVisIter(bsort, 0.0);
    
    // extract various interesting info from the MS
    // obtain number of distinct time slots
    ROMSColumns msc(ms);
    Vector<Double> time( msc.time().getColumn() );
    uInt nrows = time.nelements();
    Bool dum;
    Sort sort( time.getStorage(dum),sizeof(Double) );
    sort.sortKey((uInt)0,TpDouble);
    Vector<uInt> index,uniq;
    sort.sort(index,time.nelements());
    ntime = sort.unique(uniq,index);
    // obtain central frequencies of spws.
    const MSSpectralWindow spwin( ms.spectralWindow() );
    ROScalarColumn<Double> sfreqs(spwin, "REF_FREQUENCY");
    spwfreqs.resize();
    spwfreqs = sfreqs.getColumn();
    spwfreqs *= 1e+6;
    
    // obtain number of antennas and interferometers
    const MSAntenna msant( ms.antenna() );
    nant = msant.nrow();
    nifr = nant*(nant+1)/2; // cheap & dirty
    ROScalarColumn<String> names(msant,"NAME");
    antnames.resize();
    antnames = names.getColumn();
    antnames.apply(stringUpper);
    //  cerr<<"Antenna names: "<<antnames<<endl;
    // map ifrs to antennas
    ifr2ant1.resize(nifr);
    ifr2ant1.set(-1);
    ifr2ant2.resize(nifr);
    ifr2ant2.set(-1);
    for( uInt i1=0; i1<nant; i1++ )
      for( uInt i2=0; i2<=i1; i2++ )
	{
	  uInt ifr = ifrNumber(i1,i2);
	  ifr2ant1(ifr) = i1;
	  ifr2ant2(ifr) = i2;
	}
    // get feed info
    const MSFeed msfeed( ms.feed() );
    nfeed = msfeed.nrow();
    nfeedcorr = nfeed*(nfeed+1)/2;
    
    sprintf(str,"attached MS %s: %d rows, %d times, %d baselines\n",ms.tableName().chars(),nrows,ntime,nifr);
    //os << "--------------------------------------------------" << LogIO::POST;
    os<<str<<LogIO::POST;
    
    //// Write LogIO to HISTORY Table in MS
    if (!(Table::isReadable(ms.historyTableName()))){
      // create a new HISTORY table if its not there
      TableRecord &kws = ms.rwKeywordSet();
      SetupNewTable historySetup(ms.historyTableName(),
				 MSHistory::requiredTableDesc(),Table::New);
      kws.defineTable(MS::keywordName(MS::HISTORY), Table(historySetup));
    }
    historytab_p=Table(ms.historyTableName(),
		       TableLock(TableLock::UserNoReadLocking), Table::Update);
    hist_p= new MSHistoryHandler( ms, "Flagger");
    ////
    return True;
  }    
  
  // -----------------------------------------------------------------------
  // Flagger::detach
  // detaches from MS
  // -----------------------------------------------------------------------
  void Flagger::detach()
  {
    if ( !ms.tableName().length() ){
      os<<"no measurement set was attached"<<LogIO::POST;
    }else{
      os<<"detaching from MS "<<ms.tableName()<<LogIO::POST;
      //cout <<"detaching from MS "<<ms.tableName()<<endl;
      hist_p=0;
      histLockCounter_p = 0;
      
      nant=0;
      setdata_p = False;
      selectdata_p = False;
      pgprep_nx=pgprep_ny=1;
      ms.flush();
      ms.relinquishAutoLocks(True);
      ms.unlock();
      ms = MeasurementSet();
    }
  }
  
  /************************************ DATA SELECTION **************************************/
  /* return: True iff succesful 
   */
  Bool Flagger::selectdata(Bool useoriginalms,
			   String field, String spw, String array,
			   String feed, String scan,
			   String baseline, String uvrange, String time,
			   String correlation)
  {
    if (dbg) cout << "selectdata: "
        << "useoriginalms=" << useoriginalms
        << " field=" << field << " spw=" << spw
        << " array=" << array << " feed=" << feed
        << " scan=" << scan << " baseline=" << baseline
        << " uvrange=" << uvrange << " time=" << time
        << " correlation=" << correlation << endl;

    LogIO os(LogOrigin("Flagger", "selectdata()", WHERE));
    if (ms.isNull()) {
      os << LogIO::SEVERE << "NO MeasurementSet attached"
	 << LogIO::POST;
      return False;
    }
    
    /* If a diff sort-order is required, put it in here, and 
       create the MSs with the new ordering */
    
    MeasurementSet *tms = NULL;
    if ( useoriginalms ) tms = &originalms;
    else
      {
	if (!mssel_p)
	  {
	    cout << "Flagger::selectdata -> mssel_p is NULL !!!" << endl;
	    return False;
	  }
	tms = mssel_p;
      }
    
    if (dbg) cout << "Setting selection strings" << endl;
    
    
    /* Row-Selection */
    
    if (!spw.length() && uvrange.length()) {
	spw = String("*");
    }
    
    const String dummyExpr = String("");
    if (msselection_p) {
	delete msselection_p;
	msselection_p = NULL;
    }
    msselection_p = new MSSelection(*tms,
				    MSSelection::PARSE_NOW, 
				    (const String)time,
				    (const String)baseline, 
				    (const String)field,
				    (const String)spw,
				    (const String)uvrange,
				    dummyExpr, // taqlExpr
				    dummyExpr, // corrExpr
				    (const String)scan,
				    (const String)array);
    
    selectdata_p = True;
    /* Print out selection info - before selecting ! */
    if (dbg)
      {
	cout.precision(16);
	cout << "Antenna 1 : " << msselection_p->getAntenna1List() << endl;
	cout << "Antenna 2 : " << msselection_p->getAntenna2List() << endl;
	Matrix<Int> baselinelist(msselection_p->getBaselineList());
	IPosition shp = baselinelist.shape();
	if (shp.product() < 20)
	  {
	    IPosition transposed = shp;
	    transposed[0]=shp[1]; transposed[1]=shp[0];
	    Matrix<Int> blist(transposed);
	    for(Int i=0;i<shp[0];i++)
	      for(Int j=0;j<shp[1];j++)
		blist(j,i) = baselinelist(i,j);
	    cout << "Baselines : " << blist << endl;
	  }
	cout << "Fields : " << msselection_p->getFieldList() << endl;
	cout << "Spw : " << msselection_p->getSpwList() << endl;
	cout << "SpwChans : " << msselection_p->getChanList() << endl;
	Matrix<Double> tlist(msselection_p->getTimeList());
	cout << "Time : " << tlist << endl;
	cout << "Time : " << endl;
	for(Int i=0;i<(tlist.shape())[1];i++)
	  cout << "[" << MVTime(tlist(0,i)/C::day).string(MVTime::DMY,7) << " ~ " << MVTime(tlist(1,i)/C::day).string(MVTime::DMY,7) << "]" << endl;
	cout << "UVrange : " << msselection_p->getUVList() << endl;
	cout << "UV Units : " << msselection_p->getUVUnitsList() << endl;
	cout << "Scans : " << msselection_p->getScanList() << endl;
	cout << "Arrays : " << msselection_p->getSubArrayList() << endl;
	// cout << "Feeds : " << msselection_p->getFeedList() << endl;
	
      }
    
    /* Correlations */
    correlations_p.resize(0);
    string tcorr[50];
    Regex delim("(,| )+");
    Int ncorr = split(correlation, tcorr, 50, delim);
    correlations_p.resize(ncorr);
    for(Int i=0;i<ncorr;i++) correlations_p[i] = upcase(String(tcorr[i]));
    
    if (dbg) cout << "Correlations : " << correlations_p << endl;
    
    return True;
  }
  
  Bool Flagger::setdata(
      String field, String spw, String array, 
      String feed, String scan,
      String baseline,  String uvrange,  String time,
      String correlation) 
  {
    if (dbg) cout << "setdata: " 
        << " field=" << field << " spw=" << spw
        << " array=" << array << " feed=" << feed
        << " scan=" << scan << " baseline=" << baseline
        << " uvrange=" << uvrange << " time=" << time
        << " correlation=" << correlation << endl;

    setdata_p = True;
    LogIO os(LogOrigin("Flagger", "setdata()", WHERE));
    
    /* check the MS */
    if (ms.isNull()) {
      os << LogIO::SEVERE << "NO MeasurementSet attached"
	 << LogIO::POST;
      return False;
    }
    
    nullSelect_p=False;
    
    /* Parse selection parameters */
    if (!spw.length()) spw = String("*");

    if (!selectdata(True,field,spw,array,feed,scan,baseline,uvrange,time,correlation))
	{
	    os << LogIO::SEVERE << "Selection failed !!"
	       << LogIO::POST;
	    return False;
	}
    
    /* Create selected reference MS */
    MeasurementSet mssel_p2(*originalms_p);
    msselection_p->getSelectedMS(mssel_p2, String(""));
    
    //os << "Original ms has nrows : " << originalms.nrow() << LogIO::POST;
    //os << "Selected ms has " << mssel_p2.nrow() << " rows." << LogIO::POST;
    
    if ( mssel_p2.nrow() ) {
	if (mssel_p) {
	    delete mssel_p; 
	    mssel_p=NULL;
	}
	mssel_p = new MeasurementSet(mssel_p2);
	if (dbg)cout << "assigned new MS to mssel_p" << endl;
	ROScalarColumn<String> fname( mssel_p->field(),"NAME" );
	if (dbg)cout << "fields : " << fname.getColumn() << endl;
	
	//mssel_p->rename("selectedms",Table::New);
	//mssel_p->flush();
    }
    else {
	os << LogIO::WARN << "Selected MS has zero rows" << LogIO::POST;
	mssel_p = &originalms;
    }
    
    /* Print out selection info - before selecting ! */
    if (mssel_p->nrow()!=ms.nrow()) {
	os << "By selection " << originalms.nrow() << " rows are reduced to "
	   << mssel_p->nrow() << LogIO::POST;
    }
    else {
	os << "Selection did not drop any rows" << LogIO::NORMAL3;
    }
    /* Channel selection */ // Always select all chans
    /* Create a vis iter */
    /*
      Matrix<Int> noselection;
      Block<Int> sort(3);
      sort[0] = MS::FIELD_ID;
      sort[1] = MS::DATA_DESC_ID;
      sort[2] = MS::SCAN_NUMBER;
      //sort[2] = MS::TIME;
      */
    
    Matrix<Int> noselection;
    Block<int> sort2(4);
    //sort2[0] = MS::SCAN_NUMBER;
    // Do scan priority only if quacking
    sort2[0]= MS::ARRAY_ID;
    sort2[1]= MS::FIELD_ID;
    sort2[2]= MS::DATA_DESC_ID;
    sort2[3] = MS::TIME;
    Double timeInterval = 7.0e9; //a few thousand years
    
    if (vs_p) {
	delete vs_p; vs_p = NULL;
    }
    if (!vs_p) {
	if (!mssel_p)
	    throw AipsError("No measurement set selection available");
	//vs_p = new VisSet(*mssel_p,sort,noselection,0.0);
	
	vs_p = new VisSet(*mssel_p,sort2,noselection,timeInterval);
	// Use default sort order - and no scratch cols....
	//vs_p = new VisSet(*mssel_p,noselection,0.0);
      }
    vs_p->resetVisIter(sort2, timeInterval);
    
    selectDataChannel();
    ms = *mssel_p;
    
    return True;
  }

  Bool Flagger::selectDataChannel(){
    if (!vs_p || !msselection_p) return False;
    /* Set channel selection in visiter */
    /* Set channel selection per spectral window - from msselection_p->getChanList(); */
    // this is needed when "setdata" is used to select data for autoflag algorithms 
    // This should not be done for manual flagging, because the channel indices for the
    //  selected subset start from zero - and throw everything into confusion. 
    Vector<Int> spwlist = msselection_p->getSpwList();
    
    if ( spwlist.nelements() ){
      Matrix<Int> spwchan = msselection_p->getChanList();
      IPosition cshp = spwchan.shape();
      if ( (Int)spwlist.nelements() > (spwchan.shape())[0] )
	cout << "WARN : Using only the first channel range per spw" << endl;
      for( uInt i=0;i<spwlist.nelements();i++ )
	{
	  Int j=0;
	  for ( j=0;j<(spwchan.shape())[0];j++ )
	    if ( spwchan(j,0) == spwlist[i] ) break;
	  vs_p->iter().selectChannel(1, Int(spwchan(j,1)), 
				     Int(spwchan(j,2)-spwchan(j,1)+1),
				     Int(spwchan(j,3)), spwlist[i]);
	}
    }
    else{
      return False;
    }
    
    
    return True;
  }


  // Help function for setdata use
#if 0
  Bool Flagger::selectDataChannel(Vector<Int> &spwidnchans, Vector<Int>& spectralwindowids, 
				  Vector<Int>& dataStart, 
				  Vector<Int>& dataEnd, Vector<Int>& dataStep)
  {
    LogIO os(LogOrigin("Flagger", "selectDataChannel()", WHERE));
    
    if (dataEnd.nelements() != spectralwindowids.nelements()){
      if (dataEnd.nelements()==1){
	dataEnd.resize(spectralwindowids.nelements(), True);
	for(uInt k=1; k < spectralwindowids.nelements(); ++k){
	  dataEnd[k]=dataEnd[0];
	}
      }
      else{
	os << LogIO::SEVERE 
	   << "Vector of endchan has to be of size 1 or be of the same shape as spw " 
	   << LogIO::POST;
	return False; 
      }
    }
    if (dataStart.nelements() != spectralwindowids.nelements()){
      if (dataStart.nelements()==1){
	dataStart.resize(spectralwindowids.nelements(), True);
	for(uInt k=1; k < spectralwindowids.nelements(); ++k){
	  dataStart[k]=dataStart[0];
	}
      }
      else{
	os << LogIO::SEVERE 
	   << "Vector of startchan has to be of size 1 or be of the same shape as spw " 
	   << LogIO::POST;
	return False; 
      }
    }
    if (dataStep.nelements() != spectralwindowids.nelements()){
      if (dataStep.nelements()==1){
	dataStep.resize(spectralwindowids.nelements(), True);
	for(uInt k=1; k < spectralwindowids.nelements(); ++k){
	  dataStep[k]=dataStep[0];
	}
      }
      else{
	os << LogIO::SEVERE 
	   << "Vector of stepchan has to be of size 1 or be of the same shape as spw " 
	   << LogIO::POST;
	return False; 
      }
    }
    
    for(uInt i=0;i<spectralwindowids.nelements();i++) {
      if (dataStart[i]==-1 || dataStart[i]>=spwidnchans[i])
	dataStart[i]=0;
      if (dataEnd[i]==-1 || dataEnd[i]>=spwidnchans[i])
	dataEnd[i] = spwidnchans[i]-1;
      if (dataStep[i]==-1) dataStep[i]=1;
      
    }
    
    return True;
  }
#endif
  
  /************************** Set Manual Flags ***************************/
  
  Bool Flagger::fillSelections(Record &rec)
  {
    
    //TableExprNode ten = msselection_p->toTableExprNode(originalms_p);
    
    /* Fill in record for selected values. */
    /* Field ID */
    Vector<Int> fieldlist = msselection_p->getFieldList();
    if (fieldlist.nelements())
      {
	RecordDesc flagDesc;       
	flagDesc.addField(RF_FIELD, TpArrayInt);
	Record flagRec(flagDesc);  
	flagRec.define(RF_FIELD, fieldlist);
	rec.mergeField(flagRec, RF_FIELD, RecordInterface::OverwriteDuplicates);
      }
    /* BASELINE */
    Matrix<Int> baselinelist = msselection_p->getBaselineList();
    if (baselinelist.nelements())
      {
	IPosition shp = baselinelist.shape();
	if (dbg)cout << "Original shape of baselinelist : " << shp << endl;
	IPosition transposed = shp;
	transposed[0] = shp[1]; transposed[1] = shp[0];
	Matrix<Int> blist(transposed);

	for(Int i=0; i < shp[0]; i++)
	  for(Int j=0; j < shp[1]; j++)
	    blist(j, i) = baselinelist(i, j);
	// need to add 1 because RFASelector expects 1-based indices.
	
	RecordDesc flagDesc;       
	flagDesc.addField(RF_BASELINE, TpArrayInt);
	Record flagRec(flagDesc);  
	flagRec.define(RF_BASELINE, blist);
	rec.mergeField(flagRec, RF_BASELINE, RecordInterface::OverwriteDuplicates);
      }
    /* FEED */
    /*
      Matrix<Int> feedlist = msselection_p->getFeedList();
      if (feedlist.nelements())
      {
      IPosition shp = feedlist.shape();
      if (dbg)cout << "Original shape of feedlist : " << shp << endl;
      IPosition transposed = shp;
      transposed[0]=shp[1]; transposed[1]=shp[0];
      Matrix<Int> blist(transposed);
      for(Int i=0;i<shp[0];i++)
      for(Int j=0;j<shp[1];j++)
      blist(j,i) = feedlist(i,j);
      // need to add 1 because RFASelector expects 1-based indices.
      
      RecordDesc flagDesc;       
      flagDesc.addField(RF_FEED, TpArrayInt);
      Record flagRec(flagDesc);  
      flagRec.define(RF_FEED, blist);
      rec.mergeField(flagRec, RF_FEED, RecordInterface::OverwriteDuplicates);
      }
    */
    /* TIME */
    Matrix<Double> timelist = msselection_p->getTimeList();
    if (timelist.nelements())
      {
	/* Times need to be in MJD */
	for( Int i=0;i<(timelist.shape())[0];i++ )
	  for( Int j=0;j<(timelist.shape())[1];j++ )
	    timelist(i,j) /= (Double)(24*3600);
	//for( Int i=0;i<(timelist.shape())[0];i++ )
	//  for( Int j=0;j<(timelist.shape())[1];j++ )
	//    cout << "timelist(" << i << ", " << j << ")=" 
        //           << timelist(i,j) << endl;
	RecordDesc flagDesc;       
	flagDesc.addField(RF_TIMERANGE, TpArrayDouble);
	Record flagRec(flagDesc);  
	flagRec.define(RF_TIMERANGE, timelist);
	rec.mergeField(flagRec, RF_TIMERANGE, RecordInterface::OverwriteDuplicates);
      }
    /* RF_CORR */
    if (correlations_p.nelements())
      {
	RecordDesc flagDesc;       
	flagDesc.addField(RF_CORR, TpArrayString);
	Record flagRec(flagDesc);  
	flagRec.define(RF_CORR, correlations_p);
	rec.mergeField(flagRec, RF_CORR, RecordInterface::OverwriteDuplicates);
      }
    /* Array ID */
    Vector<Int> arraylist = msselection_p->getSubArrayList();
    if (arraylist.nelements())
      {
	RecordDesc flagDesc;       
	flagDesc.addField(RF_ARRAY, TpArrayInt);
	Record flagRec(flagDesc);  
	flagRec.define(RF_ARRAY, arraylist);
	rec.mergeField(flagRec, RF_ARRAY, RecordInterface::OverwriteDuplicates);
      }
    /* Scan ID */
    Vector<Int> scanlist = msselection_p->getScanList();
    if (scanlist.nelements())
      {
	RecordDesc flagDesc;       
	flagDesc.addField(RF_SCAN, TpArrayInt);
	Record flagRec(flagDesc);  
	flagRec.define(RF_SCAN, scanlist);
	rec.mergeField(flagRec, RF_SCAN, RecordInterface::OverwriteDuplicates);
      }
    
    return True;
  }
  


    /*
      Sets up agents for mode = 'manualflag' and mode = 'summary' 
    */
    
    Bool Flagger::setmanualflags(Bool autocorr,
				 Bool unflag, 
				 String clipexpr, 
				 Vector<Double> cliprange, 
				 String clipcolumn, 
				 Bool outside, 
				 Double quackinterval, 
				 String opmode)
    {
     if (dbg)   cout << "setmanualflags: " 
             << "autocorr=" << autocorr
             << " unflag=" << unflag
             << " clipexpr=" << clipexpr << " cliprange=" << cliprange
             << " clipcolumn=" << clipcolumn << " outside=" << outside
             << " quackinterval=" << quackinterval << " opmode=" << opmode
             << endl;

    LogIO os(LogOrigin("Flagger", "setmanualflags()", WHERE));
    if (ms.isNull()) {
      os << LogIO::SEVERE << "NO MeasurementSet attached"
	 << LogIO::POST;
      return False;
    }
    if (!selectdata_p) {
      os << LogIO::SEVERE << "Please run selectdata with/without arguments before setmanualflags"
	 << LogIO::POST;
      return False;
    }
    
    /* Fill in an agent record */
    /* This assumes that selectdata has already been called */
    
    /* Loop over SPW and chan ranges. */
    
    Vector<Int> spwlist = msselection_p->getSpwList();
    Matrix<Int> spwchan = msselection_p->getChanList();
    Matrix<Double> uvrangelist = msselection_p->getUVList();
    Vector<Bool> uvrangeunits = msselection_p->getUVUnitsList();
    IPosition cshp = spwchan.shape();
    
    /* If no selection depends on spw, ( i.e. no spw, chan, uvrange )
       then no need to make separate records for each spw. */
    bool separatespw = False;
    Int nrec;
    if (spwlist.nelements()) {
	separatespw = True; 
	nrec = spwlist.nelements();
    }
    else { 
	separatespw = False; nrec = 1; 
    }
    
    for ( Int i=0; i < nrec; i++ ) {
	Record selrec;
	if (upcase(opmode).matches("FLAG") ||
	    upcase(opmode).matches("SHADOW")) {
	    selrec.define("id", String("select"));
	}
	else if (upcase(opmode).matches("SUMMARY")) {
	    selrec.define("id", String("flagexaminer"));
	}
	else {
	    throw AipsError("Unknown mode " + upcase(opmode));
	}
	
	/* Fill selections for all but spw, chan, corr */
	fillSelections(selrec);
	
	if (separatespw) {
	    
	    /* SPW ID */
	    {
	      RecordDesc flagDesc;       
	      flagDesc.addField(RF_SPWID, TpArrayInt);
	      Record flagRec(flagDesc);                        
	      Vector<Int> t_spw(1); t_spw[0] = spwlist[i];
	      flagRec.define(RF_SPWID, t_spw);
	      selrec.mergeField(flagRec, RF_SPWID, RecordInterface::OverwriteDuplicates);
	    }
	    
	    /* reform chan ranges */
	    Int ccount=0;
	    for( Int j=0;j<cshp[0];j++ ) if ( spwlist[i] == spwchan(j,0) ) ccount++;
	    Matrix<Int> chanlist(2,ccount); chanlist.set(0);
	    
	    ccount=0;
	    for( Int j=0;j<cshp[0];j++ ) 
	      {
		if ( spwlist[i] == spwchan(j,0) ) 
		  {
		    chanlist(0,ccount) = spwchan(j,1);
		    chanlist(1,ccount) = spwchan(j,2);
		    if ( spwchan(j,3) > 1 )
		      os << LogIO::WARN << ".... ignoring chan 'step' for manual flags" << LogIO::POST;
		    ccount++;
		  }
	      }
	    /* RF_CHANS */
	    {
	      RecordDesc flagDesc;       
	      flagDesc.addField(RF_CHANS, TpArrayInt);
	      Record flagRec(flagDesc);  
	      flagRec.define(RF_CHANS, chanlist);
	      selrec.mergeField(flagRec, RF_CHANS, RecordInterface::OverwriteDuplicates);
	    }
	    
	    /* UV-RANGE */
	    if (uvrangelist.nelements())
	      {
		Matrix<Double> templist(uvrangelist.shape());
		/* Convert to Metres... */
		/* or complain if units are not metres... */
		// current spw : spwlist[i];
		
		if ( (templist.shape())[1] != (Int)uvrangeunits.nelements() )
		  cout << "UVRANGE units are wrong length ! " << endl;
		for( Int j=0;j<(templist.shape())[1];j++ )
		  {
		    Double unit=1.0;
		    if ( ! uvrangeunits[j] ) unit = C::c/(spwfreqs[spwlist[i]]/1e+6);
		    for( Int k=0;k<(templist.shape())[0];k++ )
		      templist(k,j) = uvrangelist(k,j) * unit ;
		  }
		
		RecordDesc flagDesc;       
		flagDesc.addField(RF_UVRANGE, TpArrayDouble);
		Record flagRec(flagDesc);  
		flagRec.define(RF_UVRANGE, templist);
		selrec.mergeField(flagRec, RF_UVRANGE, RecordInterface::OverwriteDuplicates);
		if (dbg) cout << "uv list (m) : " << templist << endl;
	      }
	    
	  }
	
	// Operation related parameters.
#if 0
	if (upcase(opmode).matches("SUMMARY") ) {
	    /*
	      RecordDesc flagDesc;       
	      flagDesc.addField(RF_OPMODE, TpString);
	      Record flagRec(flagDesc);  
	      flagRec.define(RF_OPMODE, opmode);
	      selrec.mergeField(flagRec, RF_OPMODE, RecordInterface::OverwriteDuplicates);
	    */
	  }
#endif
	else {

	    if (upcase(opmode).matches("SHADOW") ) {
		RecordDesc flagDesc;       
		flagDesc.addField(RF_SHADOW, TpBool);
		Record flagRec(flagDesc);
		flagRec.define(RF_SHADOW, True);
		selrec.mergeField(flagRec, RF_SHADOW, RecordInterface::OverwriteDuplicates);
	    }
	    
	    /* Flag Autocorrelations too? */
	    if (autocorr) {
		RecordDesc flagDesc;       
		flagDesc.addField(RF_AUTOCORR, TpBool);
		Record flagRec(flagDesc);  
		flagRec.define(RF_AUTOCORR, autocorr);
		selrec.mergeField(flagRec, RF_AUTOCORR, RecordInterface::OverwriteDuplicates);
	      }
	    
	    /* Unflag! */
	    if (unflag) {
		RecordDesc flagDesc;       
		flagDesc.addField(RF_UNFLAG, TpBool);
		Record flagRec(flagDesc);  
		flagRec.define(RF_UNFLAG, unflag);
		selrec.mergeField(flagRec, RF_UNFLAG, RecordInterface::OverwriteDuplicates);
	      }
	    
	    /* Reset flags before applying new ones */
	    // ( I think... )
	    /*
	      {
	      RecordDesc flagDesc;       
	      flagDesc.addField(RF_RESET, TpBool);
	      Record flagRec(flagDesc);  
	      flagRec.define(RF_RESET, True);
	      selrec.mergeField(flagRec, RF_RESET, RecordInterface::OverwriteDuplicates);
	      }
	    */
	    
	    /* Clip/FlagRange */
	    /*Jira Casa 212 : Check if "clipexpr" has multiple 
	      comma-separated expressions
	      and loop here, creating multiple clipRecs. 
	      The RFASelector will handle it. */
	    if (clipexpr.length() && cliprange.nelements()==2 &&
		cliprange[0]<cliprange[1])
	      {
		RecordDesc flagDesc;       
		if ( outside )
		  flagDesc.addField(RF_CLIP, TpRecord);
		else
		  flagDesc.addField(RF_FLAGRANGE, TpRecord);
		
		Record flagRec(flagDesc);  
		
		RecordDesc clipDesc;
		clipDesc.addField(RF_EXPR, TpString);
		clipDesc.addField(RF_MIN, TpDouble);
		clipDesc.addField(RF_MAX, TpDouble);
		Record clipRec(clipDesc);
		clipRec.define(RF_EXPR, clipexpr);
		clipRec.define(RF_MIN, cliprange[0]);
		clipRec.define(RF_MAX, cliprange[1]);
		
		if ( outside )
		  {
		    flagRec.defineRecord(RF_CLIP, clipRec);
		    selrec.mergeField(flagRec, RF_CLIP, RecordInterface::OverwriteDuplicates);
		  }
		else
		  {
		    flagRec.defineRecord(RF_FLAGRANGE, clipRec);
		    selrec.mergeField(flagRec, RF_FLAGRANGE, RecordInterface::OverwriteDuplicates);
		  }
		
		/* clip column */
		if (!clipcolumn.length()) clipcolumn=String("DATA");
		RecordDesc flagDesc2;       
		flagDesc2.addField(RF_COLUMN, TpString);
		Record flagRec2(flagDesc2);  
		flagRec2.define(RF_COLUMN, clipcolumn);
		selrec.mergeField(flagRec2, RF_COLUMN, RecordInterface::OverwriteDuplicates);
		
	      }
	    
	    /* Quack ! */
	    if (quackinterval>0.0)
	      {
		
		//Reset the Visiter to have SCAN on top of sort
		Block<int> sort2(5);
		sort2[0] = MS::SCAN_NUMBER;
		sort2[1]= MS::ARRAY_ID;
		sort2[2]= MS::FIELD_ID;
		sort2[3]= MS::DATA_DESC_ID;
		sort2[4] = MS::TIME;
		Double timeInterval = 7.0e9; //a few thousand years
		
		vs_p->resetVisIter(sort2, timeInterval);
		//lets make sure the data channel selection is done
		selectDataChannel();
		
		RecordDesc flagDesc;       
		flagDesc.addField(RF_QUACK, TpArrayDouble);
		Record flagRec(flagDesc);  
		Vector<Double> quackparams(2);
		quackparams[0] = quackinterval;
		quackparams[1] = 0.0;
		flagRec.define(RF_QUACK, quackparams);
		selrec.mergeField(flagRec, RF_QUACK, RecordInterface::OverwriteDuplicates);
	      }
	} /* end if opmode = ... */
	
	/* Add this agent to the list */
	addAgent(selrec);
      }
    
    return True;
  }
  
  Bool Flagger::applyFlags(const std::vector<FlagIndex> &fi) {
  
    Record agent(defaultAgents().asRecord("applyflags") );
    
    //cerr << __FILE__ << __LINE__ << "the hello agent = " << hello << endl;
    
    RFAApplyFlags::setIndices(&fi); // static memory!

    agent.define("id", String("applyflags"));
    addAgent(agent);

    return True;
  }
  
  
  Bool Flagger::setautoflagparams(String algorithm,Record &parameters)
  {
    LogIO os(LogOrigin("Flagger", "setautoflagparams()", WHERE));
    if (ms.isNull()) {
      os << LogIO::SEVERE << "NO MeasurementSet attached"
	 << LogIO::POST;
      return False;
    }
    if (!selectdata_p) {
      os << LogIO::SEVERE << "Please run setdata with/without arguments before setautoflagparams"
	 << LogIO::POST;
      return False;
    }
    
    /* Create an agent record */
    Record selrec;
    selrec = getautoflagparams(algorithm);
    selrec.merge(parameters,Record::OverwriteDuplicates);
    
    /* special case for "sprej".
       need to parse a param.*/
    if (algorithm.matches("sprej") && selrec.isDefined("fitspwchan"))
      {
	/* Get the "fitspwchan" string" */
	/* Pass this through the msselection parser and getChanList */
	/* Construct a list of regions records from this list */
	String fitspwchan;
	selrec.get(RecordFieldId("fitspwchan"),fitspwchan);
	
	if (fitspwchan.length())
	  {
	    /* Parse it */
	    const String dummy("");
	    MSSelection tmpmss(*mssel_p,MSSelection::PARSE_NOW, 
			       dummy,dummy, dummy, fitspwchan, dummy, 
			       dummy, dummy, dummy, dummy);
	    Matrix<Int> spwchanlist = tmpmss.getChanList();
	    Vector<Int> spwlist = tmpmss.getSpwList();
	    
	    /* Create region record template */
	    RecordDesc regdesc;
	    for(uInt i=0;i<spwlist.nelements();i++)
	      regdesc.addField(String(i),TpRecord);
	    Record regions(regdesc);
	    
	    /* reform chan ranges */
	    Int ccount=0;
	    IPosition cshp = spwchanlist.shape();
	    for(uInt i=0;i<spwlist.nelements();i++)
	      {
		ccount=0;
		for( Int j=0;j<cshp[0];j++ ) 
		  if ( spwlist[i] == spwchanlist(j,0) ) ccount++;
		Matrix<Int> chanlist(2,ccount); chanlist.set(0);
		
		ccount=0;
		for( Int j=0;j<cshp[0];j++ ) 
		  {
		    if ( spwlist[i] == spwchanlist(j,0) ) 
		      {
			chanlist(0,ccount) = spwchanlist(j,1);
			chanlist(1,ccount) = spwchanlist(j,2);
			if ( spwchanlist(j,3) > 1 )
			  os << LogIO::WARN << ".... ignoring chan 'step' for 'sprej' fitting" << LogIO::POST;
			ccount++;
		      }
		  }
		
		RecordDesc spwDesc;
		spwDesc.addField(RF_SPWID, TpInt);
		spwDesc.addField(RF_CHANS, TpArrayInt);
		Record spwRec(spwDesc);
		spwRec.define(RF_SPWID, spwlist[i]);
		spwRec.define(RF_CHANS, chanlist);
		
		/* create a single region record */
		/* add this to the list of region records */
		regions.defineRecord(String(i),spwRec);
	      }
	    
	    /* Attach the list of region records */
	    selrec.defineRecord(RF_REGION, regions);
	  }
      }
    
    /* Add this agent to the list */
    addAgent(selrec);
    
    return True;
  }
  
  Record Flagger::getautoflagparams(String algorithm)
  {
    LogIO os(LogOrigin("Flagger", "getautoflagparams()", WHERE));
    
    // Use "RFATimeMedian::getDefaults()" !!!!!!!
    
    Record defrecord;
    if ( agent_defaults.isDefined(algorithm) )
      {
	RecordFieldId rid(algorithm);
	defrecord = agent_defaults.asRecord(rid);
	defrecord.define("id",algorithm);
	
	if (defrecord.isDefined("expr")) defrecord.define("expr","ABS I");
      }
    return defrecord;
    
    /*
      
    RecordDesc flagDesc;
    
    if (algorithm.matches("timemed") || algorithm.matches("timemedian"))
    {
    flagDesc.addField("id", TpString);
    flagDesc.addField("thr", TpFloat);
    flagDesc.addField("hw", TpInt);
    flagDesc.addField("rowthr", TpFloat);
    flagDesc.addField("rowhw", TpInt);
    flagDesc.addField("norow", TpBool);
    flagDesc.addField("column", TpString);
    flagDesc.addField("expr", TpString);
    flagDesc.addField("fignore", TpBool);
    }
    if (algorithm.matches("freqmed") || algorithm.matches("freqmedian"))
    {
    flagDesc.addField("id", TpString);
    flagDesc.addField("thr", TpFloat);
    flagDesc.addField("hw", TpInt);
    flagDesc.addField("rowthr", TpFloat);
    flagDesc.addField("rowhw", TpInt);
    flagDesc.addField("norow", TpBool);
    flagDesc.addField("column", TpString);
    flagDesc.addField("expr", TpString);
    flagDesc.addField("fignore", TpBool);
    }
    if (algorithm.matches("uvbin"))
    {
    flagDesc.addField("id", TpString);
    flagDesc.addField("thr", TpFloat);
    flagDesc.addField("minpop", TpInt);
    flagDesc.addField("nbins", TpArrayInt);
    //flagDesc.addField("plotchan", TpBool);
    //flagDesc.addField("econoplot", TpBool);
    flagDesc.addField("column", TpString);
    flagDesc.addField("expr", TpString);
    //flagDesc.addField("debug", TpBool);
    flagDesc.addField("fignore", TpBool);
    }
    if (algorithm.matches("sprej"))
    {
    flagDesc.addField("id", TpString);
    flagDesc.addField(RF_NDEG, TpInt);
    flagDesc.addField("rowthr", TpFloat);
    flagDesc.addField("rowhw", TpInt);
    flagDesc.addField("column", TpString);
    flagDesc.addField("expr", TpString);
    flagDesc.addField("norow", TpBool);
    //flagDesc.addField("debug", TpBool);
    flagDesc.addField("fignore", TpBool);
    flagDesc.addField("fitspwchan",TpString);
    flagDesc.addField(RF_REGION, TpRecord);
    }
    
    
    Record flagRec(flagDesc);
    
    if (algorithm.matches("timemed") || algorithm.matches("timemedian"))
    {
    flagRec.define("id", String("timemed"));
    flagRec.define("thr", Float(5.0));
    flagRec.define("hw", 10);
    flagRec.define("rowthr", Float(10.0));
    flagRec.define("rowhw", 10);
    flagRec.define("norow", False);
    flagRec.define("column", String("DATA"));
    flagRec.define("expr", String("ABS I"));
    flagRec.define("fignore", False);
    }
    if (algorithm.matches("freqmed") || algorithm.matches("freqmedian"))
    {
    flagRec.define("id", String("freqmed"));
    flagRec.define("thr", Float(5.0));
    flagRec.define("hw", 10);
    flagRec.define("rowthr", Float(10.0));
    flagRec.define("rowhw", 10);
    flagRec.define("column", String("DATA"));
    flagRec.define("expr", String("ABS I"));
    flagRec.define("fignore", False);
    }
    if (algorithm.matches("uvbin"))
    {
    flagRec.define("id", String("uvbin"));
    flagRec.define("thr", Float(0.0));
    flagRec.define("minpop", 0);
    Vector<Int> dnbins(2);
    dnbins(0) = 50;
    dnbins(1) = 50;
    flagRec.define("nbins", dnbins);
    //flagRec.define("plotchan", plotchan);
    //flagRec.define("econoplot", econoplot);
    flagRec.define("column", "DATA");
    flagRec.define("expr", "ABS I");
    //flagRec.define("debug", debug);
    flagRec.define("fignore", False);
    }
    if (algorithm.matches("sprej"))
    {
    flagRec.define("id",String("sprej"));
    flagRec.define("ndeg", 2);
    flagRec.define("rowthr", Float(5.0));
    flagRec.define("rowhw", 6);
    flagRec.define("column", "DATA");
    flagRec.define("expr", "ABS I");
    flagRec.define("norow", False);
    //flagRec.define("debug", debug);
    flagRec.define("fignore", False);
    flagRec.define("fitspwchan", "");
    //flagRec.defineRecord(RF_REGION, *allRegions); // will get set later.
    }
    
    //
    
    return flagRec;
    
    */
  }
  
  /* Clean up all selections */
  // TODO Later add in the ability to clear specified
  // agent types.
  //
  // subRecord = agents_p->getField(RecordFieldId(x));
  // if ( subRecord.isDefined('id') && 'id' is "select" )
  //         agents_p->removeField(RecordFieldId(x));
  //
  Bool Flagger::clearflagselections(Int recordindex)
  {
    LogIO os(LogOrigin("Flagger", "clearflagselections()", WHERE));
    
    if ( agents_p && agents_p->nfields() )
      {
	if ( recordindex >= 0 )
	  {
	    if (dbg) cout << "Deleting only agent : " << recordindex << endl;
	    agents_p->removeField(RecordFieldId(recordindex));
	  }
	else
	  {
	    if (dbg) cout << "Deleting all agents" << endl;
	    delete agents_p;
	    agents_p =0;
	    agentCount_p = 0;
	  }
      }
    
    //      printflagselections();
    
    return True;
  }
  
  Bool Flagger::printflagselections()
  {
    LogIO os(LogOrigin("Flagger", "printflagselections()", WHERE));
    if ( agents_p )
      {
	os << "Current list of agents : " << agents_p << LogIO::POST;
	ostringstream out; 
	agents_p->print(out);
	os << out.str() << LogIO::POST;
      }
    else os << " No current agents " << LogIO::POST;
    
    return True;
  }
  
  Bool Flagger::addAgent(RecordInterface &newAgent)
  {
    if (!agents_p)
      {
	agentCount_p = 0;
	agents_p = new Record;
	if (dbg) cout << "creating new agent" << endl;
      }
    
    ostringstream fieldName;
    fieldName << agentCount_p++;
    agents_p->defineRecord(*(new String(fieldName)), newAgent);
    
    //      printflagselections();
    
    return True;
  }
  
  
  // computes IFR index, given two antennas
  uInt Flagger::ifrNumber ( Int ant1,Int ant2 ) const
  {
    if ( ant1<ant2 )
      return ifrNumber(ant2,ant1);
    return ant1*(ant1+1)/2 + ant2;
  }
  //TODO
  // Here, fill in correct indices for the baseline ordering in the MS
  // All agents will see this and will be fine.
  
  // computes vector of IFR indeces, given two antennas
  Vector<Int> Flagger::ifrNumbers ( Vector<Int> ant1,Vector<Int> ant2 ) const
  {
    Vector<Int> a1( ::casa::max(static_cast<Array<Int> >(ant1),static_cast<Array<Int> >(ant2)) ),
      a2( ::casa::min(static_cast<Array<Int> >(ant1),static_cast<Array<Int> >(ant2)) );
    return a1*(a1+1)/2 + a2;
  }
  
  void Flagger::ifrToAnt ( uInt &ant1,uInt &ant2,uInt ifr ) const
  {
    ant1 = ifr2ant1(ifr);
    ant2 = ifr2ant2(ifr);
  }
  
  // -----------------------------------------------------------------------
  // Flagger::setupAgentDefaults
  // Sets up record of available agents and their default parameters
  // -----------------------------------------------------------------------
  const RecordInterface & Flagger::setupAgentDefaults ()
  {
    agent_defaults = Record();
    agent_defaults.defineRecord("timemed", RFATimeMedian::getDefaults());
    agent_defaults.defineRecord("newtimemed", RFANewMedianClip::getDefaults());
    agent_defaults.defineRecord("freqmed", RFAFreqMedian::getDefaults());
    agent_defaults.defineRecord("sprej", RFASpectralRej::getDefaults());
    agent_defaults.defineRecord("select", RFASelector::getDefaults());
    agent_defaults.defineRecord("flagexaminer", RFAFlagExaminer::getDefaults());
    agent_defaults.defineRecord("uvbin", RFAUVBinner::getDefaults());
    agent_defaults.defineRecord("tfcrop", RFATimeFreqCrop::getDefaults());
    agent_defaults.defineRecord("applyflags", RFAApplyFlags::getDefaults());
    return agent_defaults;
  }
  
  // -----------------------------------------------------------------------
  // Flagger::createAgent
  // Creates flagging agent based on name
  // -----------------------------------------------------------------------
  RFABase * Flagger::createAgent (const String &id,
				  RFChunkStats &chunk,
				  const RecordInterface &parms )
  {
    // cerr << "Agent id: " << id << endl;
    if ( id == "timemed" )
      return new RFATimeMedian(chunk, parms);
    else if ( id == "newtimemed" )
      return new RFANewMedianClip(chunk, parms);
    else if ( id == "freqmed" )
      return new RFAFreqMedian(chunk, parms);
    else if ( id == "sprej" )
      return new RFASpectralRej(chunk, parms);
    else if ( id == "select" )
      return new RFASelector(chunk, parms);
    else if ( id == "flagexaminer" )
      return new RFAFlagExaminer(chunk, parms);
    else if ( id == "uvbin" )
      return new RFAUVBinner(chunk, parms);
    else if ( id == "tfcrop" )
      return new RFATimeFreqCrop(chunk, parms);
    else if ( id == "applyflags" )
      return new RFAApplyFlags(chunk, parms);
    else
      return NULL;
  }
  
  
  // -----------------------------------------------------------------------
  // setReportPanels
  // Calls SUBP on the pgp_report plotter
  // -----------------------------------------------------------------------
  void Flagger::setReportPanels ( Int nx,Int ny )
  {
    if ( !nx && !ny ) // reset
      pgprep_nx=pgprep_ny=0;
    if ( pgp_report.isAttached() && (pgprep_nx!=nx || pgprep_ny!=ny) )
      {  
	//    dprintf(os,"pgp_report.subp(%d,%d)\n",nx,ny);
	pgp_report.subp(pgprep_nx=nx,pgprep_ny=ny);
      }
  }
  void Flagger::summary( const RecordInterface &agents,const RecordInterface &opt ) 
  {
    //os << "Autoflag summary will report results here" << LogIO::POST;
    for(uInt i=0;i<agents.nfields(); i++){
      
      if (agents.dataType(i) != TpRecord){
	os << "Unrecognized field: " << agents.name(i) << LogIO::EXCEPTION;
      }
      String agent_id(downcase(agents.name(i)));
      // cerr << i << " " << agent_id << endl;
      printAgentRecord(agent_id, i, agents.asRecord(i));
    }
  }
  void Flagger::printAgentRecord(String &agent_id, uInt agentCount,
				 const RecordInterface &agent_rec){
    // but if an id field is set in the sub-record, use that instead
    if ( agent_rec.isDefined("id") && agent_rec.dataType("id") == TpString ){
      agent_id = agent_rec.asString("id");
    }
    for(uInt i=0; i<agent_rec.nfields(); i++){
      os << agent_id << "[" << agentCount+1 << "] : ";
      String myName(agent_rec.name(i));
      os << myName << ": ";
      switch(agent_rec.type(i)){
      case TpRecord :
	printAgentRecord(myName, i, agent_rec.asRecord(i));
	break;
      case TpArrayBool :
	os << agent_rec.asArrayBool(i);
	break;
      case TpArrayUChar :
	os << agent_rec.asArrayuChar(i);
	break;
      case TpArrayShort:
	os << agent_rec.asArrayShort(i);
	break;
      case TpArrayInt:
	os << agent_rec.asArrayInt(i);
	break;
      case TpArrayUInt:
	os << agent_rec.asArrayuInt(i);
	break;
      case TpArrayFloat:
	os << agent_rec.asArrayFloat(i);
	break;
      case TpArrayDouble:
	os << agent_rec.asArrayDouble(i);
	break;
      case TpArrayComplex:
	os << agent_rec.asArrayComplex(i);
	break;
      case TpArrayDComplex:
	os << agent_rec.asArrayDComplex(i);
	break;
      case TpArrayString:
	os << agent_rec.asArrayString(i);
	break;
      case TpBool:
	os << agent_rec.asBool(i);
	break;
      case TpUChar:
	os << agent_rec.asuChar(i);
	break;
      case TpShort:
	os << agent_rec.asShort(i);
	break;
      case TpInt:
	os << agent_rec.asInt(i);
	break;
      case TpUInt:
	os << agent_rec.asuInt(i);
	break;
      case TpFloat:
	os << agent_rec.asFloat(i);
	break;
      case TpDouble:
	os << agent_rec.asDouble(i);
	break;
      case TpComplex:
	os << agent_rec.asComplex(i);
	break;
      case TpDComplex:
	os << agent_rec.asDComplex(i);
	break;
      case TpString:
	os << agent_rec.asString(i);
	break;
      default :
	break;
      }
      os << endl << LogIO::POST;
    }
    //
  }
  
  // -----------------------------------------------------------------------
  // Flagger::run
  // Performs the actual flagging
  // -----------------------------------------------------------------------
  //void Flagger::run ( const RecordInterface &agents,const RecordInterface &opt,uInt ind_base ) 
  bool Flagger::run (Bool trial, Bool reset) 
  {
    if (!agents_p)
      {
	agentCount_p = 0;
	agents_p = new Record;
	if (dbg) cout << "creating new EMPTY agent and returning" << endl;
	return False;
      }
    Record agents = *agents_p;
    
    if (!opts_p)
      {
	opts_p = new Record();
      }
    *opts_p = defaultOptions();
    opts_p->define(RF_RESET,reset);
    opts_p->define(RF_TRIAL,trial);
    Record opt = *opts_p;
    
    if (!setdata_p) {
      os << LogIO::SEVERE << "Please run setdata with/without arguments before any setmethod"
	 << LogIO::POST;
      return False;
    }
    
    //printflagselections();
    
#if 1  
    if ( !nant )
      os<<"No Measurement Set has been attached\n"<<LogIO::EXCEPTION;
    
    RFABase::setIndexingBase(0);
    // set debug level
    Int debug_level=0;
    if ( opt.isDefined("debug") )
      debug_level = opt.asInt("debug");
    
    // reset existing flags?
    Bool reset_flags = isFieldSet(opt, RF_RESET);
    
    try { // all exceptions to be caught below
      
      uInt didSomething=0;
      // create iterator, visbuffer & chunk manager
      // Block<Int> sortCol(1);
      // sortCol[0] = MeasurementSet::SCAN_NUMBER;
      //sortCol[0] = MeasurementSet::TIME;
      // Setdata already made a data selection
      
      VisibilityIterator &vi(vs_p->iter()); 
      VisBuffer vb(vi);
      
      RFChunkStats chunk(vi, vb,
			 *this,
			 &pgp_screen, &pgp_report);
      
      // setup global options for flagging agents
      Record globopt(Record::Variable);
      if ( opt.isDefined(RF_GLOBAL) )
	globopt = opt.asRecord(RF_GLOBAL);
      
      // clean up any dead agents from previous run  
      for( uInt i=0; i<acc.nelements(); i++ )
	if ( acc[i] )
	  {
	    delete acc[i];
	    acc[i] = NULL;
	  }
      
      // generate new array of agents by iterating through agents record
      Record agcounts; // record of agent instance counts
      acc.resize(agents.nfields());
      
      //cerr << __FILE__ << " " << __LINE__ << agents << endl;

      acc.set(NULL);
      uInt nacc = 0;
      for( uInt i=0; i<agents.nfields(); i++ ) 
	{
	  if (  agents.dataType(i) != TpRecord )
	    os << "Unrecognized field '" << agents.name(i) << "' in agents\n" << LogIO::EXCEPTION;

	  const RecordInterface & agent_rec( agents.asRecord(i) );

	  // normally, the field name itself is the agent ID

	  String agent_id( downcase(agents.name(i)) );
	  	  
	  // but if an id field is set in the sub-record, use that instead
	  if ( agent_rec.isDefined("id") && agent_rec.dataType("id") == TpString )
	    {
	      agent_id = agent_rec.asString("id");
	    }
	  // check that this is agent really exists
	  if ( !agent_defaults.isDefined(agent_id) )
	    {
	      //cerr << agent_defaults;
	      os << "Unknown flagging method '" <<
		  agents.name(i) << "'\n" << LogIO::EXCEPTION;
	    }

	  // create parameter record by taking agent defaults, and merging in global
	  // and specified options
	  const RecordInterface & defparms(agent_defaults.asRecord(agent_id));
	  Record parms(defparms);
	  parms.merge(globopt,Record::OverwriteDuplicates); 
	  parms.merge(agent_rec,Record::OverwriteDuplicates);

	  // add the global reset argumnent
	  parms.define(RF_RESET,reset_flags);

	  // see if this is a different instance of an already activated agent
	  if (agcounts.isDefined(agent_id)) {
	      // increment the instance counter
	      Int count = agcounts.asInt(agent_id)+1;
	      agcounts.define(agent_id,count);

	      // modify the agent name to include an instance count
	      char s[1024];
	      sprintf(s,"%s#%d",defparms.asString(RF_NAME).chars(),count);
	      parms.define(RF_NAME,s);
	    }
	  else
	    agcounts.define(agent_id,1);
	  // create agent based on name
	  RFABase *agent = createAgent(agent_id,
				       chunk,
				       parms);
	  if ( !agent )
	    os<<"Unrecognized method name '"<<agents.name(i)<<"'\n"<<LogIO::EXCEPTION;
	  agent->init();
	  String inp,st;
	  //    agent->logSink()<<agent->getDesc()<<endl<<LogIO::POST;
	  acc[nacc++] = agent;
	}
      
      acc.resize(nacc, True);

      // begin iterating over chunks
      uInt nchunk=0;

      // process just the first chunk because something's screwy
      for (vi.originChunks(); 
	   vi.moreChunks(); 
	   vi.nextChunk(), nchunk++) {
	  //Start of loop over chunks
	  didSomething = 0;
	  for( uInt i = 0; i<acc.nelements(); i++ ) acc[i]->initialize();

	  chunk.newChunk();

	  // limit frequency of progmeter updates (duh!)
	  Int pm_update_freq = chunk.num(TIME)/200;

	  // How much memory do we have?
	  Int availmem = opt.isDefined("maxmem") ? 
	    opt.asInt("maxmem") : HostInfo::memoryTotal()/1024;
	  if ( debug_level>0 )
	    dprintf(os,"%d MB memory available\n",availmem);
	  // see if a flag cube is being used, and tell it to use/not use memory
	  if ( RFFlagCube::numInstances() )
	    {
	      Int flagmem = RFFlagCube::estimateMemoryUse(chunk);

	      // memory tight? use a disk-based flag cube
	      if ( flagmem>.75*availmem )
		{
		  if ( debug_level>0 )
		    dprintf(os,"%d MB flag cube: using disk\n",flagmem);
		  RFFlagCube::setMaxMem(0);
		  availmem -= 2; // reserve 2 MB for the iterator
		}
	      else // else use an in-memory cube
		{
		  if ( debug_level>0 )
		    dprintf(os,"%d MB flag cube: using memory\n",flagmem);
		  RFFlagCube::setMaxMem(availmem);
		  availmem -= flagmem;
		}
	    }
	  // call newChunk() for all accumulators; determine which ones are active
	  Vector<Int> iter_mode(acc.nelements(),RFA::DATA);
	  Vector<Bool> active(acc.nelements());
	  
	  for( uInt i = 0; i<acc.nelements(); i++ ) 
	    {
	      Int maxmem;
	      maxmem = availmem;
	      if ( ! (active(i) = acc[i]->newChunk(maxmem))  ) // refused this chunk?
		{
		  iter_mode(i) = RFA::STOP;  // skip over it
		}
	      else
		{ // active, so reserve its memory 
		  if ( debug_level>0 )
		    dprintf(os,"%s reserving %d MB of memory, %d left in pool\n",
			    acc[i]->name().chars(),availmem-maxmem,maxmem);
		  availmem = maxmem>0 ? maxmem : 0;
		}
	    }
	  if ( !sum(active) )
	    {
	       //os<<LogIO::WARN<<"Unable to process this chunk with any active method.\n"<<LogIO::POST;
	      continue;
	    }
	  // initially active agents
	  Vector<Bool> active_init = active;
	  // start executing passes    
	  char subtitle[1024];
	  sprintf(subtitle,"Flagging %s chunk %d: ",ms.tableName().chars(),nchunk+1);
	  String title(subtitle);
          //cout << "--------title=" << title << endl;

	  Int inRowFlags=0, outRowFlags=0, totalRows=0, inDataFlags=0, outDataFlags=0, totalData=0;
	  for( uInt npass=0; anyNE(iter_mode,(Int)RFA::STOP); npass++ ) // repeat passes while someone is active
	    {
	      uInt itime=0;
	      chunk.newPass(npass);
	      // count up who wants a data pass and who wants a dry pass
	      Int ndata = sum(iter_mode==(Int)RFA::DATA);
	      Int ndry  = sum(iter_mode==(Int)RFA::DRY);
	      Int nactive = ndata+ndry;
	      if ( !nactive ) // no-one? break out then
		break;
	      //	      didSomething++;
	      // Decide when to schedule a full data iteration, and when do dry runs only.
	      // There's probably room for optimizations here, but let's keep it simple 
	      // for now: since data iterations are more expensive, hold them off as long
	      // as someone is requesting a dry run.
	      Bool data_pass = !ndry;
	      // Doing a full data iteration    
	      if ( data_pass )
		{
            
		  sprintf(subtitle,"pass %d (data)",npass+1);
		  ProgressMeter progmeter(1.0,static_cast<Double>(chunk.num(TIME)+0.001),title+subtitle,"","","",True,pm_update_freq);
		  // start pass for all active agents
                  //cout << "-----------subtitle=" << subtitle << endl;
		  for( uInt ival = 0; ival<acc.nelements(); ival++ ) 
		    if ( active(ival) )
		      if ( iter_mode(ival) == RFA::DATA )
			acc[ival]->startData();
		      else if ( iter_mode(ival) == RFA::DRY )
			acc[ival]->startDry();
		  // iterate over visbuffers
		  for( vi.origin(); vi.more() && nactive; vi++,itime++ ) {
		    progmeter.update(itime);
		    chunk.newTime();
		    Bool anyActive = False;
		    anyActive=False;
		    for( uInt i = 0; i<acc.nelements(); i++ ) 
		      {
			  //if ((acc[i]->getID() != "FlagExaminer") && 
			  if (active_init(i))
			      anyActive=True;
		      }

		    for(uInt i=0;i<acc.nelements();i++) {
			if (anyActive) acc[i]->initializeIter(itime);
		    }

		    for(uInt ii=0;ii<vb.flagRow().nelements();ii++)
			if (vb.flagRow()(ii) == True) inRowFlags++;
		    totalRows += vb.flagRow().nelements();
		    totalData += vb.flagCube().shape().product();
		    for(Int ii = 0;
			ii < vb.flagCube().shape()(0);
			ii++) {
			
			for(Int jj = 0;
			    jj < vb.flagCube().shape()(1);
			    jj++) {
			    
			    for(Int kk = 0;
				kk < vb.flagCube().shape()(2);
				kk++) {
				
				if (vb.flagCube()(ii,jj,kk)) inDataFlags++;
			    }
			}
		    }
		    
		    // now, call individual VisBuffer iterators
		    for( uInt ival = 0; ival<acc.nelements(); ival++ ) 
		      if ( active(ival) ) {
			// call iterTime/iterDry as appropriate
			RFA::IterMode res = RFA::STOP;
			if ( iter_mode(ival) == RFA::DATA )
			  res = acc[ival]->iterTime(itime);
			else if ( iter_mode(ival) == RFA::DRY ) 
			  res = acc[ival]->iterDry(itime);
			// change requested? Deactivate agent
			if ( ! ( res == RFA::CONT || res == iter_mode(ival) ) )
			  {
			    active(ival) = False;
			    nactive--;
			    iter_mode(ival)==RFA::DATA ? ndata-- : ndry--;
			    iter_mode(ival) = res;
			    if ( nactive <= 0 )
			      break;
			  }
		      }
		    
		    // also iterate over rows for data passes
		    for( Int ir=0; ir<vb.nRow() && ndata; ir++ ) {
		      for( uInt ival = 0; ival<acc.nelements(); ival++ ) 
			if ( iter_mode(ival) == RFA::DATA )
			  {
			    RFA::IterMode res = acc[ival]->iterRow(ir);
			    if ( ! ( res == RFA::CONT || res == RFA::DATA ) )
			      {
				ndata--; nactive--;
				iter_mode(ival) = res;
				active(ival) = False;
				if ( ndata <= 0 )
				  break;
			      }
			  }
		    }
		  }
		  // end pass for all agents
		  for( uInt ival = 0; ival<acc.nelements(); ival++ ) 
		    {
		      if ( active(ival) )
			if ( iter_mode(ival) == RFA::DATA )
			  iter_mode(ival) = acc[ival]->endData();
			else if ( iter_mode(ival) == RFA::DRY )
			  iter_mode(ival) = acc[ival]->endDry();
		    }
		}
	      else  // dry pass only
		{
		  sprintf(subtitle,"pass %d (dry)",npass+1);
                  //cout << "-----------subtitle=" << subtitle << endl;

		  ProgressMeter progmeter(1.0,static_cast<Double>(chunk.num(TIME)+0.001),title+subtitle,"","","",True,pm_update_freq);
		  // start pass for all active agents
		  for( uInt ival = 0; ival<acc.nelements(); ival++ ) 
		    if ( iter_mode(ival) == RFA::DRY )
		      acc[ival]->startDry();
		  for( uInt itime=0; itime<chunk.num(TIME) && ndry; itime++ )
		    {
		      progmeter.update(itime);
		      // now, call individual VisBuffer iterators
		      for( uInt ival = 0; ival<acc.nelements(); ival++ ) 
			if ( iter_mode(ival) == RFA::DRY )
			  {
			    // call iterTime/iterDry as appropriate
			    RFA::IterMode res = acc[ival]->iterDry(itime);
			    // change requested? Deactivate agent
			    if ( ! ( res == RFA::CONT || res == RFA::DRY ) )
			      {
				iter_mode(ival) = res;
				active(ival) = False;
				if ( --ndry <= 0 )
				  break;
			      }
			  }
		    }
		  // end pass for all agents
		  for( uInt ival = 0; ival<acc.nelements(); ival++ ) 
		    if ( iter_mode(ival) == RFA::DRY )
		      iter_mode(ival) = acc[ival]->endDry();
		} // end of dry pass
	    } // end loop over passes
	  
	  //cout << opt << endl;
	  //cout << "any active = " << active_init << endl;

	  if ( !isFieldSet(opt, RF_TRIAL) && anyNE(active_init, False) )
	    {
		sprintf(subtitle,"pass (flag)");
		//cout << "-----------subtitle=" << subtitle << endl;

	      ProgressMeter progmeter(1.0,static_cast<Double>(chunk.num(TIME)+0.001),title+"storing flags","","","",True,pm_update_freq);
	      for( uInt i = 0; i<acc.nelements(); i++ ) 
		if ( active_init(i) )
		  acc[i]->startFlag();
	      uInt itime=0;
	      for( vi.origin(); vi.more(); vi++,itime++ ) {
		  progmeter.update(itime);

		  chunk.newTime();
		  //		  inRowFlags += sum(chunk.nrfIfr());

		  Bool anyActive = False;
		  anyActive=False;
		  for( uInt i = 0; i<acc.nelements(); i++ ) 
		    {
		      //		      cout << i << " " << acc[i]->getID() << " " << active_init(i) << endl;
			if ((acc[i]->getID() != "FlagExaminer") && 
			    active_init(i))
			    anyActive=True;
		    }

		  //cout << "anyActive" << anyActive << endl;

		  didSomething = (anyActive==True);
		  for( uInt i = 0; i<acc.nelements(); i++ ) {
		      if ( active_init(i) ) {
			  //if (acc[i]->getID() != "FlagExaminer" )
			  acc[i]->iterFlag(itime);
		      }
		      if (anyActive) acc[i]->finalizeIter(itime);
		  }
		  
		  //		  outRowFlags += sum(chunk.nrfIfr());
		  {
		      for(uInt ii=0; ii < vb.flagRow().nelements(); ii++)
			  if (vb.flagRow()(ii) == True) outRowFlags++;
		      for(Int ii = 0; ii < vb.flagCube().shape()(0); ii++)
		      for(Int jj = 0; jj < vb.flagCube().shape()(1); jj++)
		      for(Int kk = 0; kk < vb.flagCube().shape()(2); kk++)
			  if (vb.flagCube()(ii, jj, kk)) outDataFlags++;
		  }
	      }  // for (vi ... )
	      if (didSomething) {
		  for( uInt i = 0; i < acc.nelements(); i++ ) 
		      if (acc[i]) acc[i]->finalize();
		  LogIO osss(LogOrigin("Flagger", "run"),logSink_p);
		  
		  osss << "Field = " << chunk.visBuf().fieldId() << " , Spw Id : " 
		       << chunk.visBuf().spectralWindow() 
		       << "  Total rows = " << totalRows
		       << endl;
		  osss << "Input:    "
		       << "  Rows flagged = " << inRowFlags << " " //" / " << totalRows << " "
		       << "( " << 100.0*inRowFlags/totalRows << " %)."
		       << "  Data flagged = " << inDataFlags << " " //" / " << totalData << " "
		       << "( " << 100.0*inDataFlags/totalData << " %)."
		       << endl;
		  osss << "This run: "
		       << "  Rows flagged = " << outRowFlags - inRowFlags << " " //" / " << totalRows << " "
		       << "( " << 100.0*(outRowFlags-inRowFlags)/totalRows << " %)."
		       << "  Data flagged = "  << outDataFlags - inDataFlags << " " //" / " << totalData << " " 
		       << "( " << 100.0*(outDataFlags-inDataFlags)/totalData << " %)."
		       << endl;
		  osss << LogIO::POST;

// 		  osss << "InRowFlags = " << inRowFlags << " " 
// 		       << "outRowFlags = " << outRowFlags << " " 
// 		       << "DiffRowFlags = " << outRowFlags - inRowFlags << " "
// 		       << "Sum = " << sum(chunk.nrfIfr())  << " "
// 		       << "Total Rows = " << totalRows << " "
// 		       << "inDataFlags = " << inDataFlags << " "
// 		       << "outDataFlags = " << outDataFlags << " "
// 		       << endl;
		}
	      for( uInt i = 0; i<acc.nelements(); i++ ) 
		if ( active_init(i) )
		  acc[i]->endFlag();
	      
	      {
		//os << "Writing the following to MS HISTORY Table:" << LogIO::POST;
		logSink_p.clearLocally();
		LogIO oss(LogOrigin("Flagger", "run()"), logSink_p);
		os=oss;
	      }
	    }
	  // call endChunk on all agents
	  for( uInt i = 0; i<acc.nelements(); i++ ) 
	    acc[i]->endChunk();
	  
	} // end loop over chunks
      
      if (dbg)
	cout << "Total number of data chunks : " << nchunk << endl;

    } 
    catch( AipsError x )
      {
	// clean up agents
	for( uInt i=0; i<acc.nelements(); i++ )
	  {
	    if ( acc[i] )
	      {
		delete acc[i];
		acc[i] = NULL;
	      }
	  }
	acc.resize(0);
	// clean up PGPlotters
	//cleanupPlotters();
	// throw the exception on
	throw x;
      }  
    //cleanupPlotters();
    ms.flush();
    //os<<"Flagging complete\n"<<LogIO::POST;
    
#endif
    
    /* Clear the current flag selections */
    clearflagselections(0);
    
    return True;
  }
  
  // -----------------------------------------------------------------------
  // Flagger::setupPlotters
  // Sets up screen and hardcopy plotters according to options
  // -----------------------------------------------------------------------
  void Flagger::setupPlotters ( const RecordInterface &opt )
  {
    if ( !isFieldSet(opt,RF_PLOTSCR) )
      { 
	// skip the on-screen plot report
      }
    else  // else generate report
      {
	pgp_screen = PGPlotter("/xw",80);
	// setup colormap for PS
	uInt c1=16,nc=64;
	Float scale=1.0/(nc-1);
	pgp_screen.scir(c1,c1+nc-1);
	for( uInt c=0; c<nc; c++ )
	  pgp_screen.scr(c1+c,c*scale,c*scale,c*scale);
	if ( fieldType(opt,RF_PLOTSCR,TpArrayInt) )
	  {
	    Vector<Int> subp( opt.asArrayInt(RF_PLOTSCR) );
	    pgp_screen.subp(subp(0),subp(1)); 
	  }
	else
	  pgp_screen.subp(3,3);
      }
    // Device for hardcopy report 
    //   plotdev=F for no plot
    //   plotdev=T for plot (*default*)
    //   plotdev=[nx,ny] for NX x NY sub-panels
    if ( !isFieldSet(opt,RF_PLOTDEV) )
      {
	// skip the hardcopy report
      }
    else 
      {
	String filename( defaultOptions().asString(RF_DEVFILE) );
	if ( fieldType(opt,RF_DEVFILE,TpString) )
	  filename = opt.asString(RF_DEVFILE);
	if ( filename.length() )
	  {
	    // make sure default device is "/ps"
	    if ( !filename.contains(Regex("/[a-zA-Z0-9]+$")) ) 
	      filename += "/ps";
	    pgp_report = PGPlotter(filename,80);
	    // setup colormap for PS
	    uInt c1=16,nc=64;
	    Float scale=1.0/(nc-1);
	    pgp_report.scir(c1,c1+nc-1);
	    for( uInt c=0; c<nc; c++ )
	      pgp_report.scr(c1+c,c*scale,c*scale,c*scale);
	  }
      }
  }
  
  
  // -----------------------------------------------------------------------
  // cleanupPlotters
  // detaches any active PGPlotters
  // -----------------------------------------------------------------------
  void Flagger::cleanupPlotters ()
  {
    if ( pgp_screen.isAttached() )
      pgp_screen.detach();
    if ( pgp_report.isAttached() )
      pgp_report.detach();
    setReportPanels(0,0);
  }
  
  // -----------------------------------------------------------------------
  // printSummaryReport
  // Generates a summary flagging report for current chunk
  // -----------------------------------------------------------------------
  void Flagger::printSummaryReport (RFChunkStats &chunk,const RecordInterface &opt )
  {
    if (dbg) cout << "Flagger:: printSummaryReport" << endl;
    // generate a short text report in the first pane
    char s[1024];
    sprintf(s,"MS '%s'\nchunk %d (field %s, spw %d)",ms.tableName().chars(),
	    chunk.nchunk(),chunk.visIter().fieldName().chars(),chunk.visIter().spectralWindow());
    os << "---------------------------------------------------------------------" << LogIO::POST;
    os<<s<<LogIO::POST;
    
    // print overall flagging stats
    uInt n=0,n0;
    
    sprintf(s,"%s, %d channels, %d time slots, %d baselines, %d rows\n",
	    chunk.getCorrString().chars(),chunk.num(CHAN),chunk.num(TIME),
	    chunk.num(IFR),chunk.num(ROW));
    os<<s<<LogIO::POST;
    
    // % of rows flagged
    n  = sum(chunk.nrfIfr());
    n0 = chunk.num(ROW);
    sprintf(s,"%d (%0.2f%%) rows are flagged (all baselines/times/chans/corrs in this chunk).",n,n*100.0/n0);
    os<<s<<LogIO::POST;
    
    // % of data points flagged
    n  = sum(chunk.nfIfrTime());
    n0 = chunk.num(ROW)*chunk.num(CHAN)*chunk.num(CORR);
    sprintf(s,"%d of %d (%0.2f%%) data points are flagged (all baselines/times/chans/corrs in this chunk).",n,n0,n*100.0/n0);
    os<<s<<LogIO::POST;
    //os << "---------------------------------------------------------------------" << LogIO::POST;
    
    // % flagged per baseline (ifr)
    // % flagged per timestep (itime)
    // // there is info about (ifr,itime)
    // % flagged per antenna (ifr) -> decompose into a1,a2
    // % flagged per channel/corr -> (ich,ifr)
    // % flagged per ( field, spw, array, scan ) are chunks - i think !!!
    
    
    // print per-agent flagging summary
    /*
      for( uInt i=0; i<acc.nelements(); i++ )
      {
      String name(acc[i]->name() + "["+i+"]"+": ");
      String stats( acc[i]->isActive() ? acc[i]->getStats() : String("can't process this chunk") );
      os<<name+stats<<LogIO::POST;
      }
    */
    if (dbg) cout << "end of.... Flagger:: printSummaryReport" << endl;
  }
  
  // -----------------------------------------------------------------------
  // plotSummaryReport
  // Generates a summary flagging report for current chunk
  // -----------------------------------------------------------------------
  void Flagger::plotSummaryReport ( PGPlotterInterface &pgp,RFChunkStats &chunk,const RecordInterface &opt )
  {
    // generate a short text report in the first pane
    pgp.env(0,1,0,1,0,-2);
    char s[1024];
    sprintf(s,"Flagging MS '%s' chunk %d (field %s, spw %d)",ms.tableName().chars(),
	    chunk.nchunk(),chunk.visIter().fieldName().chars(),chunk.visIter().spectralWindow());
    pgp.lab("","",s);
    
    Float y0=1,dy=(pgp.qcs(4))(1)*1.5; // dy is text baseline height
    Vector<Float> vec01(2);
    vec01(0)=0; vec01(1)=1;
    
    // print chunk field, etc.
    
    // print overall flagging stats
    uInt n=0,n0;
    for( uInt i=0; i<chunk.num(IFR); i++ )
      if ( chunk.nrowPerIfr(i) )
	n++;
    sprintf(s,"%s, %d channels, %d time slots, %d baselines, %d rows\n",
	    chunk.getCorrString().chars(),chunk.num(CHAN),chunk.num(TIME),
	    chunk.num(IFR),chunk.num(ROW));
    pgp.text(0,y0-=dy,s);
    if ( isFieldSet(opt,RF_TRIAL) )
      {
	if ( isFieldSet(opt,RF_RESET) )
	  pgp.text(0,y0-=dy,"trial: no flags written out; reset: existing flags ignored");
	else 
	  pgp.text(0,y0-=dy,"trial: no flags written out");
      }
    else if ( isFieldSet(opt,RF_RESET) )
      pgp.text(0,y0-=dy,"reset: existing flags were reset");
    
    n  = sum(chunk.nrfIfr());
    n0 = chunk.num(ROW);
    sprintf(s,"%d (%0.2f%%) rows have been flagged.",n,n*100.0/n0);
    pgp.text(0,y0-=dy,s);
    os<<s<<LogIO::POST;
    n  = sum(chunk.nfIfrTime());
    n0 = chunk.num(ROW)*chunk.num(CHAN)*chunk.num(CORR);
    sprintf(s,"%d of %d (%0.2f%%) data points have been flagged.",n,n0,n*100.0/n0);
    os<<s<<LogIO::POST;
    pgp.text(0,y0-=dy,s);
    pgp.line(vec01,Vector<Float>(2,y0-dy/4));
    
    // print per-agent flagging summary
    for( uInt i=0; i<acc.nelements(); i++ )
      {
	String name(acc[i]->name() + ": ");
	pgp.text(0,y0-=dy,name+acc[i]->getDesc());
	String stats( acc[i]->isActive() ? acc[i]->getStats() : String("can't process this chunk") );
	pgp.text(0,y0-=dy,String("     ")+stats);
	os<<name+stats<<LogIO::POST;
      }
    pgp.line(vec01,Vector<Float>(2,y0-dy/4));
    pgp.iden();
  }
  
  // -----------------------------------------------------------------------
  // plotAgentReport
  // Generates per-agent reports for current chunk of data
  // Meant to be called before doing endChunk() on all the flagging 
  // agents.
  // -----------------------------------------------------------------------
  void Flagger::plotAgentReports( PGPlotterInterface &pgp )
  {
      if ( !pgp.isAttached() )
	  return;
      // call each agent to produce summary plots
      for( uInt i=0; i<acc.nelements(); i++ )
	  acc[i]->plotFlaggingReport(pgp);
  }
  // -----------------------------------------------------------------------
  // printAgentReport
  // Generates per-agent reports for current chunk of data
  // Meant to be called before doing endChunk() on all the flagging 
  // agents.
  // -----------------------------------------------------------------------
  void Flagger::printAgentReports( )
  {
    // call each agent to produce summary plots
    for( uInt i=0; i<acc.nelements(); i++ )
      acc[i]->printFlaggingReport();
  }
  
  
  // -----------------------------------------------------------------------
  // dprintf
  // Function for printfing stuff to a debug stream
  // -----------------------------------------------------------------------
  void Flagger::writeHistory(LogIO& os, Bool cliCommand){
    /*
      if (!historytab_p.isNull()) {
      if (histLockCounter_p == 0) {
      historytab_p.lock(True);
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
      os << LogIO::SEVERE << "must attach to MeasurementSet" << LogIO::POST;
      }
    */
  }
  
  /* FLAG VERSION SUPPORT*/
  
  Bool Flagger::saveFlagVersion(String versionname, String comment, String merge )
  {
    try
      {
	FlagVersion fv(originalms.tableName(),"FLAG","FLAG_ROW");
	fv.saveFlagVersion(versionname, comment, merge);
      }
    catch (AipsError x)
      {
	os << LogIO::SEVERE << "Could not save Flag Version : " << x.getMesg() << LogIO::POST;
	return False;
      }
    return True;
  }
  Bool Flagger::restoreFlagVersion(Vector<String> versionname, String merge )
  {
    try
      {
	FlagVersion fv(originalms.tableName(),"FLAG","FLAG_ROW");
	for(Int j=0;j<(Int)versionname.nelements();j++)
	  fv.restoreFlagVersion(versionname[j], merge);
      }
    catch (AipsError x)
      {
	os << LogIO::SEVERE << "Could not restore Flag Version : " << x.getMesg() << LogIO::POST;
	return False;
      }
    return True;
  }
  Bool Flagger::deleteFlagVersion(Vector<String> versionname)
  {
    try
      {
	FlagVersion fv(originalms.tableName(),"FLAG","FLAG_ROW");
	for(Int j=0;j<(Int)versionname.nelements();j++)
	  fv.deleteFlagVersion(versionname[j]);
      }
    catch (AipsError x)
      {
	os << LogIO::SEVERE << "Could not delete Flag Version : " << x.getMesg() << LogIO::POST;
	return False;
      }
    return True;
  }
  Bool Flagger::getFlagVersionList(Vector<String> &verlist)
  {
    try
      {
	verlist.resize(0);
	Int num;
	FlagVersion fv(originalms.tableName(),"FLAG","FLAG_ROW");
	Vector<String> vlist = fv.getVersionList();
	
	num = verlist.nelements();
	verlist.resize( num + vlist.nelements() + 1, True );
	verlist[num] = String("\nMS : ") + originalms.tableName() + String("\n");
	for(Int j=0;j<(Int)vlist.nelements();j++)
	  verlist[num+j+1] = vlist[j];
      }
    catch (AipsError x)
      {
	os << LogIO::SEVERE << "Could not get Flag Version List : " << x.getMesg() << LogIO::POST;
	return False;
      }
    return True;
  }
  
  
  
} //#end casa namespace
