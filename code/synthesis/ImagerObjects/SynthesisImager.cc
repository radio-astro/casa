//# SynthesisImager.cc: Implementation of Imager.h
//# Copyright (C) 1997-2008
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

#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>
#include <casa/System/ProgressMeter.h>

#include <casa/OS/DirectoryIterator.h>
#include <casa/OS/File.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/Path.h>

#include <lattices/LRegions/LCBox.h>

#include <measures/Measures/MeasTable.h>

#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MSSel/MSSelection.h>


#include <synthesis/ImagerObjects/SIIterBot.h>
#include <synthesis/ImagerObjects/SynthesisImager.h>

#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>
#include <synthesis/ImagerObjects/SIImageStore.h>
#include <synthesis/ImagerObjects/SIImageStoreMultiTerm.h>

#include <synthesis/MeasurementEquations/ImagerMultiMS.h>
#include <synthesis/MeasurementEquations/VPManager.h>
#include <msvis/MSVis/MSUtil.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <msvis/MSVis/VisImagingWeight.h>

#include <synthesis/TransformMachines/GridFT.h>
#include <synthesis/TransformMachines/WPConvFunc.h>
#include <synthesis/TransformMachines/WProjectFT.h>
#include <synthesis/TransformMachines/VisModelData.h>
#include <synthesis/TransformMachines/AWProjectFT.h>
#include <synthesis/TransformMachines/HetArrayConvFunc.h>
#include <synthesis/TransformMachines/MosaicFTNew.h>
#include <synthesis/TransformMachines/MultiTermFTNew.h>
#include <synthesis/TransformMachines/AWProjectWBFTNew.h>
#include <synthesis/TransformMachines/AWConvFunc.h>
#include <synthesis/TransformMachines/AWConvFuncEPJones.h>
#include <synthesis/TransformMachines/NoOpATerm.h>

#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casacore/casa/Utilities/Regex.h>
#include <casacore/casa/OS/Directory.h>

//#include <casadbus/utilities/BusAccess.h>
//#include <casadbus/session/DBusSession.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  SynthesisImager::SynthesisImager() : itsMappers(SIMapperCollection()), writeAccess_p(True)
  {

     imwgt_p=VisImagingWeight("natural");
     imageDefined_p=False;
     useScratch_p=False;
     readOnly_p=True;

     mss4vi_p.resize(0);
     wvi_p=0;
     rvi_p=0;

     facetsStore_p=-1;
     unFacettedImStore_p=NULL;
     dataSel_p.resize();

     nMajorCycles=0;

  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  SynthesisImager::~SynthesisImager() 
  {
    LogIO os( LogOrigin("SynthesisImager","destructor",WHERE) );
    os << LogIO::DEBUG1 << "SynthesisImager destroyed" << LogIO::POST;

    if(rvi_p) delete rvi_p;
    rvi_p=NULL;
    //    cerr << "IN DESTR"<< endl;
    //    VisModelData::listModel(mss4vi_p[0]);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  Bool SynthesisImager::selectData(const String& msname, 
				   const String& spw, 
				   const String& freqBeg, 
				   const String& freqEnd, 
				   const MFrequency::Types freqframe, 
				   const String& field, 
				   const String& antenna, 
				   const String& timestr,
				   const String& scan, 
				   const String& obs, 
				   const String& state,
				   const String& uvdist, 
				   const String& taql,
				   const Bool usescratch, 
				   const Bool readonly, 
				   const Bool incrModel)
  {
    SynthesisParamsSelect pars;
    pars.msname=msname;
    pars.spw=spw;
    pars.freqbeg=freqBeg;
    pars.freqend=freqEnd;
    pars.freqframe=freqframe;
    pars.field=field;
    pars.antenna=antenna;
    pars.timestr=timestr;
    pars.scan=scan;
    pars.obs=obs;
    pars.state=state;
    pars.uvdist=uvdist;
    pars.taql=taql;
    pars.usescratch=usescratch;
    pars.readonly=readonly;
    pars.incrmodel=incrModel;


    String err = pars.verify();

    if( err.length()>0 ) throw(AipsError("Invalid Selection parameters : " + err));

    selectData( pars );

    return True;
  }
  
  Bool SynthesisImager::selectData(const SynthesisParamsSelect& selpars)
  {
    LogIO os( LogOrigin("SynthesisImager","selectData",WHERE) );

    try
      {

    //Respect the readonly flag...necessary for multi-process access
    MeasurementSet thisms(selpars.msname, TableLock(TableLock::AutoNoReadLocking),
				selpars.readonly ? Table::Old : Table::Update);
    thisms.setMemoryResidentSubtables (MrsEligibility::defaultEligible());

    useScratch_p=selpars.usescratch;
    readOnly_p = selpars.readonly;
    //    cout << "**************** usescr : " << useScratch_p << "     readonly : " << readOnly_p << endl;
    //if you want to use scratch col...make sure they are there
    if(selpars.usescratch && !selpars.readonly){
      VisSetUtil::addScrCols(thisms, True, False, True, False);
      VisModelData::clearModel(thisms);
    }
    if(!selpars.incrmodel && !selpars.usescratch && !selpars.readonly)
    	VisModelData::clearModel(thisms, selpars.field, selpars.spw);

    os << "MS : " << selpars.msname << " | ";

    //Some MSSelection 
    //If everything is empty (which is valid) it will throw an exception..below
    //So make sure the main defaults are not empy i.e field and spw
    MSSelection thisSelection;
    if(selpars.field != ""){
      thisSelection.setFieldExpr(selpars.field);
      os << "Selecting on fields : " << selpars.field << " | " ;//LogIO::POST;
    }else
      thisSelection.setFieldExpr("*");
    if(selpars.spw != ""){
	thisSelection.setSpwExpr(selpars.spw);
	os << "Selecting on spw :"<< selpars.spw  << " | " ;//LogIO::POST;
    }else
      thisSelection.setSpwExpr("*");
    
    if(selpars.antenna != ""){
      Vector<String> antNames(1, selpars.antenna);
      // thisSelection.setAntennaExpr(MSSelection::nameExprStr( antNames));
      thisSelection.setAntennaExpr(selpars.antenna);
      os << "Selecting on antenna names : " << selpars.antenna << " | " ;//LogIO::POST;
	
    }            
    if(selpars.timestr != ""){
	thisSelection.setTimeExpr(selpars.timestr);
	os << "Selecting on time range : " << selpars.timestr << " | " ;//LogIO::POST;	
      }
    if(selpars.uvdist != ""){
      thisSelection.setUvDistExpr(selpars.uvdist);
      os << "Selecting on uvdist : " << selpars.uvdist << " | " ;//LogIO::POST;	
    }
    if(selpars.scan != ""){
      thisSelection.setScanExpr(selpars.scan);
      os << "Selecting on scan : " << selpars.scan << " | " ;//LogIO::POST;	
    }
    if(selpars.obs != ""){
      thisSelection.setObservationExpr(selpars.obs);
      os << "Selecting on Observation Expr : " << selpars.obs << " | " ;//LogIO::POST;	
    }
    if(selpars.state != ""){
      thisSelection.setStateExpr(selpars.state);
      os << "Selecting on Scan Intent/State : " << selpars.state << " | " ;//LogIO::POST;	
    }
    // if(selpars.taql != ""){
    // 	thisSelection.setTaQLExpr(selpars.taql);
    // 	os << "Selecting via TaQL : " << selpars.taql << " | " ;//LogIO::POST;	
    // }
    os << "[Opened " << (readOnly_p?"in readonly mode":(useScratch_p?"with scratch model column":"with virtual model column"))  << "]" << LogIO::POST;
    TableExprNode exprNode=thisSelection.toTableExprNode(&thisms);
    if(!(exprNode.isNull()))
      {
	mss4vi_p.resize(mss4vi_p.nelements()+1, False, True);
	MeasurementSet thisMSSelected0 = MeasurementSet(thisms(exprNode));

	if(selpars.taql != "")
	  {
	    MSSelection mss0;
	    mss0.setTaQLExpr(selpars.taql);
	    os << "Selecting via TaQL : " << selpars.taql << " | " ;//LogIO::POST;	

	    TableExprNode tenWithTaQL=mss0.toTableExprNode(&thisMSSelected0);
	    MeasurementSet thisMSSelected1 = MeasurementSet(thisMSSelected0(tenWithTaQL));
	    //mss4vi_p[mss4vi_p.nelements()-1]=MeasurementSet(thisms(exprNode));
	    mss4vi_p[mss4vi_p.nelements()-1]=thisMSSelected1;
	  }
	else
	    mss4vi_p[mss4vi_p.nelements()-1]=thisMSSelected0;
	  
	os << "  NRows selected : " << (mss4vi_p[mss4vi_p.nelements()-1]).nrow() << LogIO::POST;
      }
    else{
      throw(AipsError("Selection for given MS "+selpars.msname+" is invalid"));
    }
    //We should do the select channel here for  the VI construction later
    //Need a cross check between channel selection and ms
    // replace below if/when viFrquencySelectionUsingChannels takes in a MSSelection
    // rather than the following gymnastics
    {
      Matrix<Int> chanlist = thisSelection.getChanList(  & mss4vi_p[mss4vi_p.nelements()-1] );
      
      IPosition shape = chanlist.shape();
      uInt nSelections = shape[0];
      Int spw,chanStart,chanEnd,chanStep,nchan;

      ///////////////Temporary revert to using Vi/vb
      Int msin=mss4vi_p.nelements();
      blockNChan_p.resize(msin, False, True);
      blockStart_p.resize(msin, False, True);
      blockStep_p.resize(msin, False, True);
      blockSpw_p.resize(msin, False, True);
      msin-=1;
      blockNChan_p[msin].resize(nSelections);
      blockStart_p[msin].resize(nSelections);
      blockStep_p[msin].resize(nSelections);
      blockSpw_p[msin].resize(nSelections);
      ///////////////////////
      
      ////Channel selection 'flags' need for when using old VI/VB
      //set up Cube for storing the 'flags' for all MSes
      //find max no. channels from the current ms 
      const ROMSSpWindowColumns spwc(thisms.spectralWindow());
      uInt nspw = spwc.nrow();
      const ROScalarColumn<Int> spwNchans(spwc.numChan());
      Vector<Int> nchanvec = spwNchans.getColumn();
      Int maxnchan = 0;
      for (uInt i=0;i<nchanvec.nelements();i++) {
        maxnchan=max(nchanvec[i],maxnchan);
      }
      uInt maxnspw = 0;
      // msin is now zero based index
      for (Int i=0;i<msin+1;i++) {
        maxnspw=max(maxnspw,nspw);
      }
      //maxnspw=max(nspw,maxnspw);
      // update the channel selections and initialize to 1's (all selected)
      chanSel_p.resize(msin+1,nspw,maxnchan,True);
      chanSel_p.yzPlane(msin)=1;

      if(selpars.freqbeg==""){
	  //re-initialize to false 
          chanSel_p.yzPlane(msin)=0;
          Vector<Int> loChans(nspw, -1);
    	  /////So this gymnastic is needed
          Int nUsedSpw = 0;
          Vector<Int> chanStepPerSpw(nspw,-1);
          Vector<Int> maxChanEnd(nspw,0); 
          Vector<Int> usedSpw(nspw,-1);
    	  for(uInt k=0; k < nSelections; ++k)
    	  {

    		  spw = chanlist(k,0);

    		  // channel selection
    		  chanStart = chanlist(k,1);
    		  chanEnd = chanlist(k,2);
    		  chanStep = chanlist(k,3);
    		  //nchan = chanEnd-chanStart+1;
    		  nchan = Int(ceil(Double(chanEnd-chanStart+1)/Double(chanStep)));
                  maxChanEnd(spw) = max(maxChanEnd(spw), chanEnd);
                  chanStepPerSpw(spw) = chanStep;
                  // find lowest selected channel for each spw
                  if(loChans(spw) == -1) { 
                    loChans(spw) = chanStart;
                    nUsedSpw++;
                    usedSpw.resize(nUsedSpw,True);
                    usedSpw(nUsedSpw-1) = spw;
                  }
                  else {
                    if ( loChans(spw) > chanStart) loChans(spw) = chanStart;
                  }

    		  //channelSelector.add (spw, chanStart, nchan,chanStep);
    		  ///////////////Temporary revert to using Vi/vb
    		  // will not work with multi-selections within a spw
    		  //blockNChan_p[msin][k]=nchan;
    		  //blockStart_p[msin][k]=chanStart;
    		  //blockStep_p[msin][k]=chanStep;
    		  //blockSpw_p[msin][k]=spw;
           }

           // vi.selectChannel() does not handle multiple chan sels within a spw???
           // So store a single channel sel(range) per spw... + chanflags...
           blockNChan_p[msin].resize(nUsedSpw);
           blockStart_p[msin].resize(nUsedSpw);
           blockStep_p[msin].resize(nUsedSpw);
           blockSpw_p[msin].resize(nUsedSpw);
           for(uInt j=0; j < (uInt) nUsedSpw; ++j)
           {
                  Int ispw = usedSpw(j);
                  if(loChans(ispw)!=-1) 
                  {
                    blockNChan_p[msin][j] = Int(ceil(Double(maxChanEnd(ispw)-loChans(ispw)+1)/Double(chanStepPerSpw(ispw))));
                    blockStart_p[msin][j] = loChans(ispw);
                    blockStep_p[msin][j] = chanStepPerSpw(ispw);
                    blockSpw_p[msin][j] = ispw; 
                  }
           }
 
           Int relStart=0;
    	   for(uInt k=0; k < nSelections; ++k)
           {
    		  spw = chanlist(k,0);
    		  chanStart = chanlist(k,1);
    		  chanEnd = chanlist(k,2);
                  
                  // for 'channel flags' (for old VI/VB)
                  // MSSelect will be applied before the channel flags in FTMachine so
                  // chanSel_p will be relative to the start chan
                  //for (uInt k=chanStart;k<(chanEnd+1);k+=chanStep) {
                  if (loChans(spw) == chanStart ) {
                    relStart = 0;
                  }
                  else {
                    relStart = chanStart - loChans(spw);
                  }
                  for (uInt k=relStart;k<(uInt)(chanEnd-loChans(spw)+1);k+=chanStep) {
                    chanSel_p(msin,spw,k)=1;
                    //cerr<<"chanselflag spw="<<spw<<" chan="<<k<<endl;
                  }
    		  /////////////////////////////////////////

    	  }

      }
      else{
    	  Quantity freq;
    	  Quantity::read(freq, selpars.freqbeg);
    	  Double lowfreq=freq.getValue("Hz");
    	  Quantity::read(freq, selpars.freqend);
    	  Double topfreq=freq.getValue("Hz");
    	  //////////OLD VI/VB
    	  //ImagerMultiMS im;
    	  Vector<Int> elspw, elstart, elnchan;
          // Intent can be used to select a field id so check
          // field ids actually present in the selection-applied MS
    	  Vector<Int>fields=thisSelection.getFieldList( & mss4vi_p[msin]);
          ROMSColumns tmpmsc(mss4vi_p[msin]);
          Vector<Int> fldidv=tmpmsc.fieldId().getColumn();
          std::set<Int> ufldids(fldidv.begin(),fldidv.end());
          std::vector<Int> tmpv(ufldids.begin(), ufldids.end());
          fields.resize(tmpv.size());
          uInt count=0;
          for (std::vector<int>::const_iterator it=tmpv.begin();it != tmpv.end(); it++)
            {
               fields(count) = *it;
               count++;
            }

    	  Int fieldid=fields.nelements() ==0 ? 0: fields[0];
          Vector<Int> chanSteps = chanlist.column(3); 
          Vector<Int> spws = chanlist.column(0);
          Int nspw = genSort( spws, Sort::Ascending, (Sort::QuickSort | Sort::NoDuplicates ));
          Vector<Int> chanStepPerSpw(nspw);
          uInt icount = 0; 
          Int prevspw = -1;
          for (uInt j=0; j < spws.nelements(); j++ ) {
            if (spws[j] != prevspw) {
              chanStepPerSpw[icount] =  chanSteps[j];
              prevspw = spws[j];
              icount++;
            }
          }
    	  //getSpwInFreqRange seems to assume the freqs to be the center of a channel while freqbeg and freqend appear to be
    	  //frequencies at the channel range edges. So set freqStep = 0.0 .
    	  //MSUtil::getSpwInFreqRange(elspw, elstart,elnchan,mss4vi_p[msin],lowfreq, topfreq,1.0, selpars.freqframe, fieldid);
    	  MSUtil::getSpwInFreqRange(elspw, elstart,elnchan,mss4vi_p[msin],lowfreq, topfreq,0.0, selpars.freqframe, fieldid);
    	  //im.adviseChanSelex(lowfreq, topfreq, 1.0, selpars.freqframe, elspw, elstart, elnchan, selpars.msname, fieldid, False);
    	  // Refine elspw if there is spw selection
    	  if ( elspw.nelements()==0 || elstart.nelements()==0 || elnchan.nelements()==0 )
            throw(AipsError("Failed to select vis. data by frequency range"));
          Vector<Int> tempspwlist, tempnchanlist, tempstartlist;
          uInt nselspw=0;
    	  if (selpars.spw != "") {
            Vector<Int> spwids = thisSelection.getSpwList( & mss4vi_p[msin]);
            for (uInt k=0; k < elspw.nelements(); k++ ) {
              if (std::find(spwids.begin(), spwids.end(), elspw[k]) != spwids.end()) {
                tempspwlist.resize(tempspwlist.nelements()+1,True);
                tempnchanlist.resize(tempnchanlist.nelements()+1,True);
                tempstartlist.resize(tempstartlist.nelements()+1,True);
                tempspwlist[nselspw] = elspw[k];
                tempnchanlist[nselspw] = elnchan[k];
                tempstartlist[nselspw] = elstart[k];
                nselspw++;
              }
            }
            elspw.resize(tempspwlist.nelements());
            elnchan.resize(tempnchanlist.nelements());
            elstart.resize(tempstartlist.nelements());
            elspw = tempspwlist;
            elnchan = tempnchanlist;
            elstart = tempstartlist;
          }
    	  blockNChan_p[msin].resize(elspw.nelements());
    	  blockStart_p[msin].resize(elspw.nelements());
    	  blockStep_p[msin].resize(elspw.nelements());
    	  blockSpw_p[msin].resize(elspw.nelements());
    	  blockNChan_p[msin]=elnchan;
    	  blockStart_p[msin]=elstart;
    	  blockStep_p[msin].set(1);
    	  blockSpw_p[msin]=elspw;
    	  //////////////////////
      }
      os << "selected spw " << blockSpw_p[msin] << " start channels " << blockStart_p[msin] << " nchannels "<< blockNChan_p[msin] << LogIO::POST;

    }
    writeAccess_p=writeAccess_p && !selpars.readonly;
    createVisSet(writeAccess_p);

    /////// Remove this when the new vi/vb is able to get the full freq range.
    mssFreqSel_p.resize();
    mssFreqSel_p  = thisSelection.getChanFreqList(NULL,True);

    //// Set the data column on which to operate
    // TT: added checks for the requested data column existace 
    //    cout << "Using col : " << selpars.datacolumn << endl;
    if( selpars.datacolumn.contains("data") || selpars.datacolumn.contains("obs") ) 
      {    if( thisms.tableDesc().isColumn("DATA") ) { datacol_p = FTMachine::OBSERVED; }
           else { os << LogIO::SEVERE <<"DATA column does not exist" << LogIO::EXCEPTION;}
      }
    else if( selpars.datacolumn.contains("corr") ) {    
      if( thisms.tableDesc().isColumn("CORRECTED_DATA") ) { datacol_p = FTMachine::CORRECTED; } 
      else 
	{
	  if( thisms.tableDesc().isColumn("DATA") ) { 
	    datacol_p = FTMachine::OBSERVED;
	    os << "CORRECTED_DATA column does not exist. Using DATA column instead" << LogIO::POST; 
	  }
	  else { 
	    os << LogIO::SEVERE <<"Neither CORRECTED_DATA nor DATA columns exist" << LogIO::EXCEPTION;
	  }
	}
	
      }
    else { os << LogIO::WARN << "Invalid data column : " << datacol_p << ". Using corrected (or observed if corrected doesn't exist)" << LogIO::POST;  datacol_p = thisms.tableDesc().isColumn("CORRECTED_DATA") ? FTMachine::CORRECTED : FTMachine::OBSERVED; }

    dataSel_p.resize(dataSel_p.nelements()+1, True);

    dataSel_p[dataSel_p.nelements()-1]=selpars;


    unlockMSs();
      }
    catch(AipsError &x)
      {
	unlockMSs();
	throw( AipsError("Error in selectData() : "+x.getMesg()) );
      }

    return True;

  }


  void SynthesisImager::unlockMSs()
  {
    LogIO os( LogOrigin("SynthesisImager","unlockMSs",WHERE) );
    for(uInt i=0;i<mss4vi_p.nelements();i++)
      { 
	os << LogIO::NORMAL2 << "Unlocking : " << mss4vi_p[i].tableName() << LogIO::POST;
	mss4vi_p[i].unlock(); 
      }
  }
 

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Bool SynthesisImager::defineImage(const String& imagename, const Int nx, const Int ny,
				    const Quantity& cellx, const Quantity& celly,
				    const String& stokes,
				    const MDirection& phaseCenter, 
				    const Int nchan,
				    const Quantity&freqStart,
				    const Quantity& freqStep, 
				    const Vector<Quantity>& restFreq,
				    const Int facets,
				    const String ftmachine, 
				    const Int nTaylorTerms,
				    const Quantity& refFreq,
				    const Projection& projection,
				    const Quantity& distance,
				    const MFrequency::Types& freqFrame,
				    const Bool trackSource, 
				    const MDirection& trackDir, 
				    const Bool overwrite,
				    const Float padding, 
				    const Bool useAutocorr, 
				    const Bool useDoublePrec, 
				    const Int wprojplanes, 
				    const String convFunc, 
				    const String startmodel,
				    // The extra params for WB-AWP
				    const Bool aTermOn,//    = True,
				    const Bool psTermOn,//   = True,
				    const Bool mTermOn,//    = False,
				    const Bool wbAWP,//      = True,
				    const String cfCache,//  = "",
				    const Bool doPointing,// = False,
				    const Bool doPBCorr,//   = True,
				    const Bool conjBeams,//  = True,
				    const Float computePAStep,         //=360.0
				    const Float rotatePAStep          //=5.0
				    )
{
  String err("");

  SynthesisParamsImage impars;
  impars.imageName=imagename;
  Vector<Int> ims(2);ims[0]=nx; ims[1]=ny;
  impars.imsize=ims;
  Vector<Quantity> cells(2); cells[0]=cellx, cells[1]=celly;
  impars.cellsize=cells;
  impars.stokes=stokes;
  impars.phaseCenter=phaseCenter;
  impars.nchan=nchan;
  impars.mode= nchan < 0 ? "mfs" : "cube";
  impars.freqStart=freqStart;
  impars.freqStep=freqStep;
  impars.restFreq=restFreq;
  impars.nTaylorTerms=nTaylorTerms;
  impars.refFreq=refFreq;
  impars.projection=projection;
  impars.freqFrame=freqFrame;
  impars.overwrite=overwrite;
  impars.startModel=startmodel;

  err += impars.verify();

  SynthesisParamsGrid gridpars;
  gridpars.ftmachine=ftmachine;
  gridpars.distance=distance;
  gridpars.trackSource=trackSource;
  gridpars.trackDir=trackDir;
  gridpars.padding=padding;
  gridpars.facets=facets;
  gridpars.useAutoCorr=useAutocorr;
  gridpars.useDoublePrec=useDoublePrec;
  gridpars.wprojplanes=wprojplanes;
  gridpars.convFunc=convFunc;
  gridpars.aTermOn=aTermOn;
  gridpars.psTermOn=psTermOn;
  gridpars.mTermOn=mTermOn;
  gridpars.wbAWP=wbAWP;
  gridpars.cfCache=cfCache;
  gridpars.doPointing=doPointing;
  gridpars.doPBCorr=doPBCorr;
  gridpars.conjBeams=conjBeams;
  gridpars.computePAStep=computePAStep;
  gridpars.rotatePAStep=rotatePAStep;

  err += gridpars.verify();

  if( err.length()>0 ) throw(AipsError("Invalid Image/Gridding parameters : " + err));

  defineImage( impars, gridpars );

  return True;
}

  Bool SynthesisImager::defineImage(SynthesisParamsImage& impars, 
			   const SynthesisParamsGrid& gridpars)
  {

    LogIO os( LogOrigin("SynthesisImager","defineImage",WHERE) );
    if(mss4vi_p.nelements() ==0)
      os << "SelectData has to be run before defineImage" << LogIO::EXCEPTION;

    CoordinateSystem csys;
    CountedPtr<FTMachine> ftm, iftm;


    try
      {

	os << "Define image coordinates for [" << impars.imageName << "] : " << LogIO::POST;

	csys = impars.buildCoordinateSystem( rvi_p );
	IPosition imshape = impars.shp();

	if( (itsMappers.nMappers()==0) || 
	    (impars.imsize[0]*impars.imsize[1] > itsMaxShape[0]*itsMaxShape[1]))
	  {
	    itsMaxShape=imshape;
	    itsMaxCoordSys=csys;
	  }

        itsCsysRec = impars.getcsys();
	/*
	os << "Define image  [" << impars.imageName << "] : nchan : " << impars.nchan 
	   //<< ", freqstart:" << impars.freqStart.getValue() << impars.freqStart.getUnit() 
	   << ", start:" << impars.start
	   <<  ", imsize:" << impars.imsize 
	   << ", cellsize: [" << impars.cellsize[0].getValue() << impars.cellsize[0].getUnit() 
	   << " , " << impars.cellsize[1].getValue() << impars.cellsize[1].getUnit() 
	   << LogIO::POST;
	*/
      }
    catch(AipsError &x)
      {
	os << "Error in building Coordinate System and Image Shape : " << x.getMesg() << LogIO::EXCEPTION;
      }

	
    try
      {
	os << "Set Gridding options for [" << impars.imageName << "] with ftmachine : " << gridpars.ftmachine << LogIO::POST;

	createFTMachine(ftm, iftm, gridpars.ftmachine, impars.nTaylorTerms, gridpars.mType, 
			gridpars.facets, gridpars.wprojplanes,
			gridpars.padding,gridpars.useAutoCorr,gridpars.useDoublePrec,
			gridpars.convFunc,
			gridpars.aTermOn,gridpars.psTermOn, gridpars.mTermOn,
			gridpars.wbAWP,gridpars.cfCache,gridpars.doPointing,
			gridpars.doPBCorr,gridpars.conjBeams,
			gridpars.computePAStep,gridpars.rotatePAStep,
			gridpars.interpolation, impars.freqFrameValid, 1000000000,  16, impars.stokes,
			impars.imageName);

      }
    catch(AipsError &x)
      {
	os << "Error in setting up FTMachine() : " << x.getMesg() << LogIO::EXCEPTION;
      }

    try
      {
	appendToMapperList(impars.imageName,  csys,  impars.shp(),
			   ftm, iftm,
			   gridpars.distance, gridpars.facets, impars.overwrite,
			   gridpars.mType, impars.nTaylorTerms, impars.startModel);
	imageDefined_p=True;
      }
    catch(AipsError &x)
      {
	os << "Error in adding Mapper : "+x.getMesg() << LogIO::EXCEPTION;
      }

    // Set the model image for prediction -- Call an SIImageStore function that does the REGRIDDING.
    /*
      if( startmodel.length()>0 && !itsCurrentImages.null() )
      {
      os << "Setting " << startmodel << " as starting model for prediction " << LogIO::POST;
      itsCurrentImages->setModelImage( startmodel );
      }
    */
    return True;
  }

  Bool SynthesisImager::defineImage(CountedPtr<SIImageStore> imstor, 
				    const String& ftmachine)
  {
    CountedPtr<FTMachine> ftm, iftm;

    // The following call to createFTMachine() uses the
    // following defaults
    //
    // facets=1, wprojplane=1, padding=1.0, useAutocorr=False, 
    // useDoublePrec=True, gridFunction=String("SF")
    //
    createFTMachine(ftm, iftm, ftmachine);
    
    Int id=itsMappers.nMappers();
    CoordinateSystem csys =imstor->residual()->coordinates();
    IPosition imshape=imstor->residual()->shape();
    Int nx=imshape[0], ny=imshape[1];
    if( (id==0) || (nx*ny > itsMaxShape[0]*itsMaxShape[1]))
      {
	itsMaxShape=imshape;
	itsMaxCoordSys=csys;
      }

    itsMappers.addMapper(  createSIMapper( "default", imstor, ftm, iftm, id ) );
    
    return True;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  
  Vector<SynthesisParamsSelect> SynthesisImager::tuneSelectData(){
	   if(itsMappers.nMappers() < 1)
		   ThrowCc("defineimage has to be run before tuneSelectData");
	   Vector<SynthesisParamsSelect> origDatSel(dataSel_p.nelements());
	   origDatSel=dataSel_p;
	   /*Record selpars;
	   for(uInt k=0; k < origDatSel.nelements(); ++k){
		   Record inSelRec=origDatSel[k].toRecord();
		   selpars.defineRecord("ms"+String::toString(k), inSelRec);
	   }
	   */
	   Int nchannel=itsMaxShape[3];
	   CoordinateSystem cs=itsMaxCoordSys;
	   cs.setSpectralConversion("LSRK");
	   Vector<Double> pix(4);
	   pix[0]=0; pix[1]=0; pix[2]=0; pix[3]=-0.5;
	   Double freq1=cs.toWorld(pix)[3];
	   pix[3]=Double(nchannel)-0.5;
	   Double freq2=cs.toWorld(pix)[3];
	   String units=cs.worldAxisUnits()[3];
	   if(freq2 < freq1){
		   Double tmp=freq1;
		   freq1=freq2;
		   freq2=tmp;
	   }
	   //String freqbeg=String::toString(freq1)+units;
	   //String freqend=String::toString(freq2)+units;
	   // String::toString drops the digits for double precision
	   String freqbeg=doubleToString(freq1)+units;
	   String freqend=doubleToString(freq2)+units;
	   //Record outRec=SynthesisUtilMethods::cubeDataPartition(selpars, 1, freq1, freq2);
	   //Record partRec=outRec.asRecord("0");
	   ///resetting the block ms
	   mss4vi_p.resize(0,True, False);
	   //resetting data selection stored

	   dataSel_p.resize();

	   for(uInt k=0; k< origDatSel.nelements(); ++k){
		   SynthesisParamsSelect outsel=origDatSel[k];
		   outsel.freqbeg=freqbeg;
		   outsel.freqend=freqend;
		   outsel.freqframe=MFrequency::LSRK;
		   selectData(outsel);
	   }
	   return dataSel_p;

   }


   ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  void SynthesisImager::setComponentList(const ComponentList& cl, Bool sdgrid){
	  String cft="SimpleComponentFTMachine";
	  if(sdgrid)
		  cft="SimpCompGridFTMachine";
	  CountedPtr<SIMapper> sm=new SIMapper(cl, cft);
	  itsMappers.addMapper(sm);
	  ////itsMappers.addMapper(  createSIMapper( mappertype, imstor, ftm, iftm, id) );

  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //////////////////////Reset the Mapper
  ////////////////////
  void SynthesisImager::resetMappers(){
    ////reset code
	itsMappers=SIMapperCollection();
	unFacettedImStore_p=NULL;
  }
//////////////////////////////////////////////////////////////////
/////////////////////////////////////////////
  CountedPtr<SIImageStore> SynthesisImager::imageStore(const Int id)
  {
    if(facetsStore_p >1)
      {
	if(id==0)
	  {
	    return unFacettedImStore_p;
	  }
	else
	  {
	    return itsMappers.imageStore(facetsStore_p*facetsStore_p+id-1);
	  }
      }
    return itsMappers.imageStore(id);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  void SynthesisImager::executeMajorCycle(Record& controlRecord)
  {
    LogIO os( LogOrigin("SynthesisImager","executeMajorCycle",WHERE) );

    nMajorCycles++;

    Bool lastcycle=False;
    if( controlRecord.isDefined("lastcycle") )
      {
	controlRecord.get( "lastcycle" , lastcycle );
	//cout << "lastcycle : " << lastcycle << endl;
      }
    //else {cout << "No lastcycle" << endl;}

    os << "----------------------------------------------------------- Run ";
    if (lastcycle) os << "(Last) " ;
    os << "Major Cycle " << nMajorCycles << " -------------------------------------" << LogIO::POST;

    try
      {    
	runMajorCycle(False, lastcycle);

	itsMappers.releaseImageLocks();

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in running Major Cycle : "+x.getMesg()) );
      }    

  }// end of executeMajorCycle
  //////////////////////////////////////////////
  /////////////////////////////////////////////

  void SynthesisImager::makePSF()
    {
      LogIO os( LogOrigin("SynthesisImager","makePSF",WHERE) );

      os << "----------------------------------------------------------- Make PSF ---------------------------------------------" << LogIO::POST;
    
      try
      {
	runMajorCycle(True, False);

	//  	  if(facetsStore_p >1)
	//   {
	//     setPsfFromOneFacet();
	//     }

    	  itsMappers.releaseImageLocks();

      }
      catch(AipsError &x)
      {
    	  throw( AipsError("Error in making PSF : "+x.getMesg()) );
      }

    }


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Bool SynthesisImager::weight(const String& type, const String& rmode,
			       const Quantity& noise, const Double robust,
			       const Quantity& fieldofview,
			       const Int npixels, const Bool multiField,
			       const String& filtertype, const Quantity& filterbmaj,
			       const Quantity& filterbmin, const Quantity& filterbpa   )
  {
    LogIO os(LogOrigin("SynthesisImager", "weight()", WHERE));

       try {
    	//Int nx=itsMaxShape[0];
    	//Int ny=itsMaxShape[1];
	 Quantity cellx=Quantity(itsMaxCoordSys.increment()[0], itsMaxCoordSys.worldAxisUnits()[0]);
	 Quantity celly=Quantity(itsMaxCoordSys.increment()[1], itsMaxCoordSys.worldAxisUnits()[1]);
	 os << LogIO::NORMAL // Loglevel INFO
	    << "Set imaging weights : " ; //<< LogIO::POST;
	 
	 if (type=="natural") {
	   os << LogIO::NORMAL // Loglevel INFO
	      << "Natural weighting" << LogIO::POST;
	   imwgt_p=VisImagingWeight("natural");
	 }
      else if (type=="radial") {
	os << "Radial weighting" << LogIO::POST;
    	  imwgt_p=VisImagingWeight("radial");
      }
      else{
    	  if(!imageDefined_p)
    		  throw(AipsError("Need to define image"));
    	  Int nx=itsMaxShape[0];
    	  Int ny=itsMaxShape[1];
    	  Quantity cellx=Quantity(itsMaxCoordSys.increment()[0], itsMaxCoordSys.worldAxisUnits()[0]);
    	  Quantity celly=Quantity(itsMaxCoordSys.increment()[1], itsMaxCoordSys.worldAxisUnits()[1]);
    	  if(type=="superuniform"){
    		  if(!imageDefined_p) throw(AipsError("Please define image first"));
    		  Int actualNpix=npixels;
    		  if(actualNpix <=0)
    			  actualNpix=3;
    		  os << LogIO::NORMAL // Loglevel INFO
    				  << "SuperUniform weighting over a square cell spanning ["
    				  << -actualNpix
    				  << ", " << actualNpix << "] in the uv plane" << LogIO::POST;
    		  imwgt_p=VisImagingWeight(*rvi_p, rmode, noise, robust, nx,
    				  ny, cellx, celly, actualNpix,
    				  actualNpix, multiField);
    	  }
    	  else if ((type=="robust")||(type=="uniform")||(type=="briggs")) {
    		  if(!imageDefined_p) throw(AipsError("Please define image first"));
    		  Quantity actualFieldOfView(fieldofview);
    		  Int actualNPixels(npixels);
    		  String wtype;
    		  if(type=="briggs") {
    			  wtype = "Briggs";
    		  }
    		  else {
    			  wtype = "Uniform";
    		  }
    		  if(actualFieldOfView.get().getValue()==0.0&&actualNPixels==0) {
    			  actualNPixels=nx;
    			  actualFieldOfView=Quantity(actualNPixels*cellx.get("rad").getValue(),
    					  "rad");
    			  os << LogIO::NORMAL // Loglevel INFO
    					  << wtype
    					  << " weighting: sidelobes will be suppressed over full image"
    					  << LogIO::POST;
    		  }
    		  else if(actualFieldOfView.get().getValue()>0.0&&actualNPixels==0) {
    			  actualNPixels=nx;
    			  os << LogIO::NORMAL // Loglevel INFO
    					  << wtype
    					  << " weighting: sidelobes will be suppressed over specified field of view: "
    					  << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
    		  }
    		  else if(actualFieldOfView.get().getValue()==0.0&&actualNPixels>0) {
    			  actualFieldOfView=Quantity(actualNPixels*cellx.get("rad").getValue(),
    					  "rad");
    			  os << LogIO::NORMAL // Loglevel INFO
    					  << wtype
    					  << " weighting: sidelobes will be suppressed over full image field of view: "
    					  << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
    		  }
    		  else {
    			  os << LogIO::NORMAL // Loglevel INFO
    					  << wtype
    					  << " weighting: sidelobes will be suppressed over specified field of view: "
    					  << actualFieldOfView.get("arcsec").getValue() << " arcsec" << LogIO::POST;
    		  }
    		  os << LogIO::DEBUG1
    				  << "Weighting used " << actualNPixels << " uv pixels."
    				  << LogIO::POST;
    		  Quantity actualCellSize(actualFieldOfView.get("rad").getValue()/actualNPixels, "rad");

		  //		  cerr << "rmode " << rmode << " noise " << noise << " robust " << robust << " npixels " << actualNPixels << " cellsize " << actualCellSize << " multifield " << multiField << endl;
		  //		  Timer timer;
		  //timer.mark();
		  //Construct imwgt_p with old vi for now if old vi is in use as constructing with vi2 is slower 


		  imwgt_p=VisImagingWeight(*rvi_p, wtype=="Uniform" ? "none" : rmode, noise, robust,
                                 actualNPixels, actualNPixels, actualCellSize,
                                 actualCellSize, 0, 0, multiField);

		  /*
		  if(rvi_p !=NULL){
		    imwgt_p=VisImagingWeight(*rvi_p, rmode, noise, robust,
                                 actualNPixels, actualNPixels, actualCellSize,
                                 actualCellSize, 0, 0, multiField);
		  }
		  else{
		    ////This is slower by orders of magnitude as of 2014/06/25
		    imwgt_p=VisImagingWeight(*vi_p, rmode, noise, robust,
                                 actualNPixels, actualNPixels, actualCellSize,
                                 actualCellSize, 0, 0, multiField);
		  }
		  */
		    //timer.show("After making visweight ");

    	  }
    	  else {
    		  //this->unlock();
    		  os << LogIO::SEVERE << "Unknown weighting " << type
    				  << LogIO::EXCEPTION;
    		  return False;
    	  }
      }
	 
	 //// UV-Tapering
	 //cout << "Taper type : " << filtertype << " : " << (filtertype=="gaussian") <<  endl;
	 if( filtertype == "gaussian" ) {
	   //	   os << "Setting uv-taper" << LogIO::POST;
	   imwgt_p.setFilter( filtertype,  filterbmaj, filterbmin, filterbpa );
	 }
	 
	 rvi_p->useImagingWeight(imwgt_p);
      ///////////////////////////////
	 
	 
	 ///	 return True;
	 
       }
       catch(AipsError &x)
	 {
	   throw( AipsError("Error in Weighting : "+x.getMesg()) );
	 }
       
       return True;
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //// Get/Set Weight Grid.... write to disk and read

  /// todo : do for full mapper list, and taylor terms.
  Bool SynthesisImager::getWeightDensity( )
  {
    LogIO os(LogOrigin("SynthesisImager", "getWeightDensity()", WHERE));
    try
      {
	Block<Matrix<Float> > densitymatrices;
	imwgt_p.getWeightDensity( densitymatrices );
	if ( densitymatrices.nelements()>0 )
	  {
	    for (uInt fid=0;fid<densitymatrices.nelements();fid++)
	      {
		//cout << "Density shape (get) for f " << fid << ": " << densitymatrices[fid].shape() << endl;
		itsMappers.imageStore(fid)->gridwt(0)->put(densitymatrices[fid]);
	      }
	  }
	itsMappers.releaseImageLocks();

      }
    catch (AipsError &x)
      {
	throw(AipsError("In getWeightDensity : "+x.getMesg()));
      }
    return True;
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// todo : do for full mapper list, and taylor terms.
  
  Bool SynthesisImager::setWeightDensity( )
  {
    LogIO os(LogOrigin("SynthesisImager", "setWeightDensity()", WHERE));
    try
      {
	Block<Matrix<Float> > densitymatrices(itsMappers.nMappers());
	for (uInt fid=0;fid<densitymatrices.nelements();fid++)
	  {
	    Array<Float> arr;
	    itsMappers.imageStore(fid)->gridwt(0)->get(arr,True);
	    densitymatrices[fid].reference( arr );
	    //cout << "Density shape (set) for f " << fid << " : " << arr.shape() << " : " << densitymatrices[fid].shape() << endl;
	  }


	imwgt_p.setWeightDensity( densitymatrices );
	rvi_p->useImagingWeight(imwgt_p);
	itsMappers.releaseImageLocks();

      }
    catch (AipsError &x)
      {
	throw(AipsError("In setWeightDensity : "+x.getMesg()));
      }
    return True;
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////
  ////////////This should be called  at each defineimage
  CountedPtr<SIImageStore> SynthesisImager::createIMStore(String imageName, 
							  CoordinateSystem& cSys,
							  IPosition imShape, 
							  const Bool overwrite,
							  String mappertype,
							  uInt ntaylorterms,
							  Quantity distance,
							  uInt facets,
							  Bool useweightimage,
							  String startmodel)
  {
    LogIO os( LogOrigin("SynthesisImager","createIMStore",WHERE) );

    CountedPtr<SIImageStore> imstor;

    try
      {
	
	if( mappertype=="default" || mappertype=="imagemosaic" )
	  {
	    imstor=new SIImageStore(imageName, cSys, imShape, overwrite, (useweightimage || (mappertype=="imagemosaic") ));
	    //	    imstor=new SIImageStore(imageName, cSys, imShape, facets, overwrite, (useweightimage || (mappertype=="imagemosaic") ));
	  }
	else if (mappertype == "multiterm" )  // Currently does not support imagemosaic.
	  {
	    //cout << "Making multiterm IS with nterms : " << ntaylorterms << endl;
	    imstor=new SIImageStoreMultiTerm(imageName, cSys, imShape, facets, overwrite, ntaylorterms, useweightimage);
	  }
	else
	  {
	    throw(AipsError("Internal Error : Invalid mapper type in SynthesisImager::createIMStore"));
	  }
	
	// Fill in miscellaneous information needed by FITS
	ROMSColumns msc(mss4vi_p[0]);
	Record info;
	String objectName=msc.field().name()(msc.fieldId()(0));
	String telescop=msc.observation().telescopeName()(0);
	info.define("OBJECT", objectName);
	info.define("TELESCOP", telescop);
	info.define("INSTRUME", telescop);
	info.define("distance", distance.get("m").getValue());
	////////////// Send misc info into ImageStore. 
	imstor->setImageInfo( info );

	// Get polRep from 'msc' here, and send to imstore. 
	StokesImageUtil::PolRep polRep(StokesImageUtil::CIRCULAR);
	Vector<String> polType=msc.feed().polarizationType()(0);
	if (polType(0)!="X" && polType(0)!="Y" &&  polType(0)!="R" && polType(0)!="L") {
	  os << LogIO::WARN << "Unknown stokes types in feed table: ["
	     << polType(0) << ", " << polType(1) << "]" << endl
	     << "Results open to question!" << LogIO::POST;
	}
	
	if (polType(0)=="X" || polType(0)=="Y") {
	  polRep=StokesImageUtil::LINEAR;
	  os << LogIO::DEBUG1 << "Preferred polarization representation is linear" << LogIO::POST;
	}
	else {
	  polRep=StokesImageUtil::CIRCULAR;
	  os << LogIO::DEBUG1 << "Preferred polarization representation is circular" << LogIO::POST;
	}
	/// end of reading polRep info
	
	///////// Send this info into ImageStore.
	imstor->setDataPolFrame(polRep);

	///////// Set Starting model if it exists.
	//cout << "In SI, set starting model to : " << startmodel << endl;
	if( startmodel.length()>0 ) 
	  {
	    imstor->setModelImage( startmodel );
	  }

      }
    catch(AipsError &x)
      {
	throw(AipsError("Error in createImStore : " + x.getMesg() ) );
      }
    
    
    return imstor;
  }
  
  CountedPtr<SIMapper> SynthesisImager::createSIMapper(String mappertype,  
							   CountedPtr<SIImageStore> imagestore,
							   CountedPtr<FTMachine> ftmachine,
							   CountedPtr<FTMachine> iftmachine,
						       uInt /*ntaylorterms*/)
  {
    LogIO os( LogOrigin("SynthesisImager","createSIMapper",WHERE) );
    
    CountedPtr<SIMapper> localMapper;

    try
      {
	
	if( mappertype == "default" || mappertype == "multiterm" )
	  {
	    localMapper = new SIMapper( imagestore, ftmachine, iftmachine );
	  }
	else if( mappertype == "imagemosaic") // || mappertype == "mtimagemosaic" )
	  {
	    localMapper = new SIMapperImageMosaic( imagestore, ftmachine, iftmachine );
	  }
	else
	  {
	    throw(AipsError("Unknown mapper type : " + mappertype));
	  }

      }
    catch(AipsError &x) {
	throw(AipsError("Error in createSIMapper : " + x.getMesg() ) );
      }
    return localMapper;
  }
  

  Block<CountedPtr<SIImageStore> > SynthesisImager::createFacetImageStoreList(
									      CountedPtr<SIImageStore> imagestore,
									      Int facets)
  {
    Block<CountedPtr<SIImageStore> > facetList( facets*facets );

    if( facets==1 ) { facetList[0] = imagestore;  return facetList; }

    // Remember, only the FIRST field in each run can have facets. So, check for this.
    if( ! unFacettedImStore_p.null() ) {
	throw( AipsError("A facetted image has already been set. Facets are supported only for the main (first) field. Please submit a feature-request if you need multiple facets for outlier fields as well. ") );
      }
    
    unFacettedImStore_p = imagestore;
    facetsStore_p = facets;
    
    // Note : facets : Number of facets on a side.
    // Note : facet : index from range(0, facets*facets)
    for (Int facet=0; facet< facets*facets; ++facet){
	facetList[facet] = unFacettedImStore_p->getSubImageStore(facet, facets);
      }
    
    return facetList;
  }

  void SynthesisImager::setPsfFromOneFacet()
  {

    if( unFacettedImStore_p.null() ){
	throw(AipsError("Internal Error in SynthesisImager : Setting PSF on Null unfacettedimage"));
      }

    // Copy the PSF from one facet to the center of the full image, to use for the minor cycle
    //
    // This code segment will work for single and multi-term 
    // - for single term, the index will always be 0, and SIImageStore's access functions know this.
    // - for multi-term, the index will be the taylor index and SIImageStoreMultiTerm knows this.
      {
	IPosition shape=(unFacettedImStore_p->psf(0))->shape();
	IPosition blc(4, 0, 0, 0, 0);
	IPosition trc=shape-1;
	for(uInt tix=0; tix<2 * unFacettedImStore_p->getNTaylorTerms() - 1; tix++)
	  {
	    TempImage<Float> onepsf((itsMappers.imageStore(0)->psf(tix))->shape(), 
				    (itsMappers.imageStore(0)->psf(tix))->coordinates());
	    onepsf.copyData(*(itsMappers.imageStore(0)->psf(tix)));
	    //now set the original to 0 as we have a copy of one facet psf
	    (unFacettedImStore_p->psf(tix))->set(0.0);
	    blc[0]=(shape[0]-(onepsf.shape()[0]))/2;
	    trc[0]=onepsf.shape()[0]+blc[0]-1;
	    blc[1]=(shape[1]-(onepsf.shape()[1]))/2;
	    trc[1]=onepsf.shape()[1]+blc[1]-1;
	    Slicer sl(blc, trc, Slicer::endIsLast);
	    SubImage<Float> sub(*(unFacettedImStore_p->psf(tix)), sl, True);
	    sub.copyData(onepsf);
	  }
      }

      //cout << "In setPsfFromOneFacet : sumwt : " << unFacettedImStore_p->sumwt()->get() << endl;

  }


  
  
  
  void SynthesisImager::appendToMapperList(String imagename,  
					   CoordinateSystem& csys, 
					   IPosition imshape,
					   CountedPtr<FTMachine>& ftm,
					   CountedPtr<FTMachine>& iftm,
					   Quantity distance, 
					   Int facets, 
					   const Bool overwrite,
					   String mappertype,
					   uInt ntaylorterms,
					   String startmodel)
    {
      LogIO log_l(LogOrigin("SynthesisImager", "appendToMapperList(ftm)"));
      //---------------------------------------------
      // Some checks..
      if(facets > 1 && itsMappers.nMappers() > 0)
	log_l << "Facetted image has to be first of multifields" << LogIO::EXCEPTION;
      
      AlwaysAssert( ( ( ! (ftm->name()=="MosaicFTNew" && mappertype=="imagemosaic") )  && 
      		      ( ! (ftm->name()=="AWProjectWBFTNew" && mappertype=="imagemosaic") )) ,
		    AipsError );
      //---------------------------------------------

      // Create the ImageStore object
      CountedPtr<SIImageStore> imstor;
      imstor = createIMStore(imagename, csys, imshape, overwrite,mappertype, ntaylorterms, distance,facets, iftm->useWeightImage(), startmodel );

      // Create the Mappers
      if( facets<2 ) // One facet. Just add the above imagestore to the mapper list.
	{
	  itsMappers.addMapper(  createSIMapper( mappertype, imstor, ftm, iftm, ntaylorterms) );
	}
      else // This field is facetted. Make a list of reference imstores, and add all to the mapper list.
	{
	  // First, make sure that full images have been allocated before trying to make references.....
	  //	  if( ! imstor->checkValidity(True/*psf*/, True/*res*/,True/*wgt*/,True/*model*/,False/*image*/,False/*mask*/,True/*sumwt*/ ) ) 
	  //	    { throw(AipsError("Internal Error : Invalid ImageStore for " + imstor->getName())); }

	  // Make and connect the list.
	  Block<CountedPtr<SIImageStore> > imstorList = createFacetImageStoreList( imstor, facets );
	  for( uInt facet=0; facet<imstorList.nelements(); facet++)
	    {
	      CountedPtr<FTMachine> new_ftm, new_iftm;
	      if(facet==0){ new_ftm = ftm;  new_iftm = iftm; }
	      else{ new_ftm=ftm->cloneFTM();  new_iftm=iftm->cloneFTM(); }
	      itsMappers.addMapper(createSIMapper( mappertype, imstorList[facet], new_ftm, new_iftm, ntaylorterms));
	    }
	}

    }

  /////////////////////////
  /*
  Bool SynthesisImager::toUseWeightImage(CountedPtr<FTMachine>& ftm, String mappertype)
  {
    if( (ftm->name() == "GridFT" || ftm->name() == "WProjectFT")&&(mappertype!="imagemosaic") )  
      { return False; }
    else
      { return True; }
  }
  */

  // Make the FT-Machine and related objects (cfcache, etc.)
  void SynthesisImager::createFTMachine(CountedPtr<FTMachine>& theFT, 
					CountedPtr<FTMachine>& theIFT, 
					const String& ftname,
					const uInt nTaylorTerms,
					const String mType,
					const Int facets,            //=1
					//------------------------------
					const Int wprojplane,        //=1,
					const Float padding,         //=1.0,
					const Bool useAutocorr,      //=False,
					const Bool useDoublePrec,    //=True,
					const String gridFunction,   //=String("SF"),
					//------------------------------
					const Bool aTermOn,          //= True,
					const Bool psTermOn,         //= True,
					const Bool mTermOn,          //= False,
					const Bool wbAWP,            //= True,
					const String cfCache,        //= "",
					const Bool doPointing,       //= False,
					const Bool doPBCorr,         //= True,
					const Bool conjBeams,        //= True,
					const Float computePAStep,         //=360.0
					const Float rotatePAStep,          //=5.0
					const String interpolation,  //="linear"
					const Bool freqFrameValid, //=True
					const Int cache,             //=1000000000,
					const Int tile,               //=16
					const String stokes, //=I
					const String imageNamePrefix
					)

  {
    LogIO os( LogOrigin("SynthesisImager","createFTMachine",WHERE));

    if(ftname=="gridft"){
      if(facets >1){
    	  theFT=new GridFT(cache, tile, gridFunction, mLocation_p, phaseCenter_p, padding, useAutocorr, useDoublePrec);
    	  theIFT=new GridFT(cache, tile, gridFunction, mLocation_p, phaseCenter_p, padding, useAutocorr, useDoublePrec);

      }
      else{
    	  theFT=new GridFT(cache, tile, gridFunction, mLocation_p, padding, useAutocorr, useDoublePrec);
    	  theIFT=new GridFT(cache, tile, gridFunction, mLocation_p, padding, useAutocorr, useDoublePrec);
      }
    }
    else if(ftname== "wprojectft"){
      theFT=new WProjectFT(wprojplane,  mLocation_p,
			   cache/2, tile, useAutocorr, padding, useDoublePrec);
      theIFT=new WProjectFT(wprojplane,  mLocation_p,
			   cache/2, tile, useAutocorr, padding, useDoublePrec);
      CountedPtr<WPConvFunc> sharedconvFunc= new WPConvFunc();
      static_cast<WProjectFT &>(*theFT).setConvFunc(sharedconvFunc);
      static_cast<WProjectFT &>(*theFT).setConvFunc(sharedconvFunc);
    }
    else if ((ftname == "awprojectft") || (ftname== "mawprojectft") || (ftname == "protoft")) {
      createAWPFTMachine(theFT, theIFT, ftname, facets, wprojplane, 
			 padding, useAutocorr, useDoublePrec, gridFunction,
			 aTermOn, psTermOn, mTermOn, wbAWP, cfCache, 
			 doPointing, doPBCorr, conjBeams, computePAStep,
			 rotatePAStep, cache,tile,imageNamePrefix);
    }
    else if ( ftname == "mosaic" || ftname== "mosft" || ftname == "mosaicft" || ftname== "MosaicFT"){

      createMosFTMachine(theFT, theIFT, padding, useAutocorr, useDoublePrec, rotatePAStep, stokes);
    }
    else
      {
	throw( AipsError( "Invalid FTMachine name : " + ftname ) );
      }
    /* else if(ftname== "MosaicFT"){

       }*/



    ///////// Now, clone and pack the chosen FT into a MultiTermFT if needed.
    if( mType=="multiterm" )
      {
	AlwaysAssert( nTaylorTerms>=1 , AipsError );

	CountedPtr<FTMachine> theMTFT = new MultiTermFTNew( theFT , nTaylorTerms, True/*forward*/ );
	CountedPtr<FTMachine> theMTIFT = new MultiTermFTNew( theIFT , nTaylorTerms, False/*forward*/ );

	theFT = theMTFT;
	theIFT = theMTIFT;
      }




    ////// Now, set the SkyJones if needed, and if not internally generated.
    if( mType=="imagemosaic" && 
	(ftname != "awprojectft" && ftname != "mawprojectft" && ftname != "proroft") )
      {
	CountedPtr<SkyJones> vp;
	ROMSColumns msc(mss4vi_p[0]);
	Quantity parang(0.0,"deg");
	Quantity skyposthreshold(0.0,"deg");
	vp = new VPSkyJones(msc, True,  parang, BeamSquint::NONE,skyposthreshold);

	Vector<CountedPtr<SkyJones> > skyJonesList(1);
	skyJonesList(0) = vp;
	theFT->setSkyJones(  skyJonesList );
	theIFT->setSkyJones(  skyJonesList );

      }

    //// For mode=cubedata, set the freq frame to invalid..
    // get this info from buildCoordSystem
    Vector<Int> tspws(0);
    //theFT->setSpw( tspws, False );
    //theIFT->setSpw( tspws, False );
    theFT->setSpw( tspws, freqFrameValid );
    theIFT->setSpw( tspws, freqFrameValid );

    //// Set interpolation mode
    theFT->setFreqInterpolation( interpolation );
    theIFT->setFreqInterpolation( interpolation );

    //channel selections from spw param
    theFT->setSpwChanSelection(chanSel_p);
    theIFT->setSpwChanSelection(chanSel_p);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void SynthesisImager::createAWPFTMachine(CountedPtr<FTMachine>& theFT, CountedPtr<FTMachine>& theIFT, 
					   const String&,// ftmName,
					   const Int,// facets,            //=1
					   //------------------------------
					   const Int wprojPlane,        //=1,
					   const Float,// padding,         //=1.0,
					   const Bool,// useAutocorr,      //=False,
					   const Bool useDoublePrec,    //=True,
					   const String,// gridFunction,   //=String("SF"),
					   //------------------------------
					   const Bool aTermOn,          //= True,
					   const Bool psTermOn,         //= True,
					   const Bool mTermOn,          //= False,
					   const Bool wbAWP,            //= True,
					   const String cfCache,        //= "",
					   const Bool doPointing,       //= False,
					   const Bool doPBCorr,         //= True,
					   const Bool conjBeams,        //= True,
					   const Float computePAStep,   //=360.0
					   const Float rotatePAStep,    //=5.0
					   const Int cache,             //=1000000000,
					   const Int tile,               //=16
					   const String imageNamePrefix
					)

  {
    LogIO os( LogOrigin("SynthesisImager","createAWPFTMachine",WHERE));

    if (wprojPlane<=1)
      {
	os << LogIO::NORMAL
	   << "You are using wprojplanes=1. Doing co-planar imaging (no w-projection needed)" 
	   << LogIO::POST;
	os << LogIO::NORMAL << "Performing WBA-Projection" << LogIO::POST; // Loglevel PROGRESS
      }
    if((wprojPlane>1)&&(wprojPlane<64)) 
      {
	os << LogIO::WARN
	   << "No. of w-planes set too low for W projection - recommend at least 128"
	   << LogIO::POST;
	os << LogIO::NORMAL << "Performing WBAW-Projection" << LogIO::POST; // Loglevel PROGRESS
      }

    // CountedPtr<ATerm> apertureFunction = createTelescopeATerm(mss4vi_p[0], aTermOn);
    // CountedPtr<PSTerm> psTerm = new PSTerm();
    // CountedPtr<WTerm> wTerm = new WTerm();
    
    // //
    // // Selectively switch off CFTerms.
    // //
    // if (aTermOn == False) {apertureFunction->setOpCode(CFTerms::NOOP);}
    // if (psTermOn == False) psTerm->setOpCode(CFTerms::NOOP);

    // //
    // // Construct the CF object with appropriate CFTerms.
    // //
    // CountedPtr<ConvolutionFunction> tt;
    // tt = AWProjectFT::makeCFObject(aTermOn, psTermOn, True, mTermOn, wbAWP);
    // CountedPtr<ConvolutionFunction> awConvFunc;
    // //    awConvFunc = new AWConvFunc(apertureFunction,psTerm,wTerm, !wbAWP);
    // if ((ftmName=="mawprojectft") || (mTermOn))
    //   awConvFunc = new AWConvFuncEPJones(apertureFunction,psTerm,wTerm,wbAWP);
    // else
    //   awConvFunc = new AWConvFunc(apertureFunction,psTerm,wTerm,wbAWP);

    ROMSObservationColumns msoc(mss4vi_p[0].observation());
    String telescopeName=msoc.telescopeName()(0);
    CountedPtr<ConvolutionFunction> awConvFunc = AWProjectFT::makeCFObject(telescopeName, 
									   aTermOn,
									   psTermOn, True, mTermOn, wbAWP);
    //
    // Construct the appropriate re-sampler.
    //
    CountedPtr<VisibilityResamplerBase> visResampler;
    //    if (ftmName=="protoft") visResampler = new ProtoVR();
    //elsef
      visResampler = new AWVisResampler();
    //    CountedPtr<VisibilityResamplerBase> visResampler = new VisibilityResampler();

    //
    // Construct and initialize the CF cache object.
    //


    // CountedPtr<CFCache> cfCacheObj = new CFCache();
    // cfCacheObj->setCacheDir(cfCache.data());
    // //    cerr << "Setting wtImagePrefix to " << imageNamePrefix.c_str() << endl;
    // cfCacheObj->setWtImagePrefix(imageNamePrefix.c_str());
    // cfCacheObj->initCache2();

      CountedPtr<CFCache> cfCacheObj;
      

    //
    // Finally construct the FTMachine with the CFCache, ConvFunc and
    // Re-sampler objects.  
    //
    Float pbLimit_l=1e-3;
    theFT = new AWProjectWBFTNew(wprojPlane, cache/2, 
			      cfCacheObj, awConvFunc, 
			      visResampler,
			      /*True */doPointing, doPBCorr, 
			      tile, computePAStep, pbLimit_l, True,conjBeams,
			      useDoublePrec);

    cfCacheObj = new CFCache();
    cfCacheObj->setCacheDir(cfCache.data());
    //    cerr << "Setting wtImagePrefix to " << imageNamePrefix.c_str() << endl;
    cfCacheObj->setWtImagePrefix(imageNamePrefix.c_str());
    cfCacheObj->initCache2();

    theFT->setCFCache(cfCacheObj);
    

    Quantity rotateOTF(rotatePAStep,"deg");
    static_cast<AWProjectWBFTNew &>(*theFT).setObservatoryLocation(mLocation_p);
    static_cast<AWProjectWBFTNew &>(*theFT).setPAIncrement(Quantity(computePAStep,"deg"),rotateOTF);

    // theIFT = new AWProjectWBFT(wprojPlane, cache/2, 
    // 			       cfCacheObj, awConvFunc, 
    // 			       visResampler,
    // 			       /*True */doPointing, doPBCorr, 
    // 			       tile, computePAStep, pbLimit_l, True,conjBeams,
    // 			       useDoublePrec);

    // static_cast<AWProjectWBFT &>(*theIFT).setObservatoryLocation(mLocation_p);
    // static_cast<AWProjectWBFT &>(*theIFT).setPAIncrement(Quantity(computePAStep,"deg"),rotateOTF);

    theIFT = new AWProjectWBFTNew(static_cast<AWProjectWBFTNew &>(*theFT));

    //// Send in Freq info.
    os << "Sending frequency selection information " <<  mssFreqSel_p  <<  " to AWP FTM." << LogIO::POST;
    theFT->setSpwFreqSelection( mssFreqSel_p );
    theIFT->setSpwFreqSelection( mssFreqSel_p );

  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  ATerm* SynthesisImager::createTelescopeATerm(const MeasurementSet& ms, const Bool& isATermOn)
  {
    LogIO os(LogOrigin("SynthesisImager", "createTelescopeATerm",WHERE));
    
    if (!isATermOn) return new NoOpATerm();
    
    ROMSObservationColumns msoc(ms.observation());
    String ObsName=msoc.telescopeName()(0);
    if ((ObsName == "EVLA") || (ObsName == "VLA"))
      return new EVLAAperture();
    else
      {
	os << "Telescope name ('"+
	  ObsName+"') in the MS not recognized to create the telescope specific ATerm" 
	   << LogIO::WARN;
      }
    
    return NULL;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void SynthesisImager:: createMosFTMachine(CountedPtr<FTMachine>& theFT,CountedPtr<FTMachine>&  theIFT, const Float /*padding*/, const Bool useAutoCorr, const Bool useDoublePrec, const Float rotatePAStep, const String stokes){
    
    LogIO os(LogOrigin("SynthesisImager", "createMosFTMachine",WHERE));
   
    ROMSColumns msc(rvi_p->ms());
    String telescop=msc.observation().telescopeName()(0);
    // Hack...start
    if(telescop=="EVLA"){os << LogIO::WARN << "vpmanager does not list EVLA. Using VLA beam parameters" << LogIO::POST; telescop="VLA";}
    // Hack...stop
    VPManager *vpman=VPManager::Instance();
    PBMath::CommonPB kpb;
    PBMath::enumerateCommonPB(telescop, kpb);
    Record rec;
    vpman->getvp(rec, telescop);
    VPSkyJones* vps=NULL;
    if(rec.asString("name")=="COMMONPB" && kpb !=PBMath::UNKNOWN ){
      vps= new VPSkyJones(msc, True, Quantity(rotatePAStep, "deg"), BeamSquint::GOFIGURE, Quantity(360.0, "deg"));
      /////Don't know which parameter has pb threshold cutoff that the user want 
      ////leaving at default
      ////vps.setThreshold(minPB);
      
    }
    
    theFT = new MosaicFTNew(vps, mLocation_p, stokes, 1000000000, 16, useAutoCorr, 
		      useDoublePrec);
    PBMathInterface::PBClass pbtype=PBMathInterface::AIRY;
    if(rec.asString("name")=="IMAGE")
       pbtype=PBMathInterface::IMAGE;
    ///Use Heterogenous array mode for the following
    if((kpb == PBMath::UNKNOWN) || (kpb==PBMath::OVRO) || (kpb==PBMath::ACA)
       || (kpb==PBMath::ALMA)){
      CountedPtr<SimplePBConvFunc> mospb=new HetArrayConvFunc(pbtype, "");
      static_cast<MosaicFTNew &>(*theFT).setConvFunc(mospb);
    }
    ///////////////////make sure both FTMachine share the same conv functions.
    theIFT= new MosaicFTNew(static_cast<MosaicFTNew &>(*theFT));

    
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Do MS-Selection and set up vi/vb. 
  // Only this functions needs to know anything about the MS 
  void SynthesisImager::createVisSet(const Bool writeAccess)
  {
    LogIO os( LogOrigin("SynthesisImager","createVisSet",WHERE) );

    ////////////Temporary revert to vi/vb
    Block<Int> sort(0);
    Block<MeasurementSet> msblock(mss4vi_p.nelements());
    //for (uInt k=0; k< msblock.nelements(); ++k){
    //	msblock[k]=*mss_p[k];
    //}

    //vs_p= new VisSet(blockMSSel_p, sort, noChanSel, useModelCol_p);
    if(!writeAccess){

    	rvi_p=new ROVisibilityIterator(mss4vi_p, sort);

    }
    else{
    	wvi_p=new VisibilityIterator(mss4vi_p, sort);
    	rvi_p=wvi_p;
    }
    Block<Vector<Int> > blockGroup(msblock.nelements());
    for (uInt k=0; k < msblock.nelements(); ++k){
    	blockGroup[k].resize(blockSpw_p[k].nelements());
    	blockGroup[k].set(1);
        //    	cerr << "start " << blockStart_p[k] << " nchan " << blockNChan_p[k] << " step " << blockStep_p[k] << " spw "<< blockSpw_p[k] <<endl;
    }

    rvi_p->selectChannel(blockGroup, blockStart_p, blockNChan_p,
    			  blockStep_p, blockSpw_p);
    rvi_p->useImagingWeight(VisImagingWeight("natural"));
    ////////////////////end of revert vi/vb
  }// end of createVisSet

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  void SynthesisImager::runMajorCycle(const Bool dopsf, 
				      const Bool savemodel)
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );

    //    cout << "Savemodel : " << savemodel << "   readonly : " << readOnly_p << "   usescratch : " << useScratch_p << endl;

    Bool savemodelcolumn = savemodel && !readOnly_p && useScratch_p;
    Bool savevirtualmodel = savemodel && !readOnly_p && !useScratch_p;

    if( savemodelcolumn ) os << "Saving model column" << LogIO::POST;
    if( savevirtualmodel ) os << "Saving virtual model" << LogIO::POST;

    itsMappers.checkOverlappingModels("blank");

    {
    	VisBufferAutoPtr vb(rvi_p);
    	rvi_p->originChunks();
    	rvi_p->origin();

	ProgressMeter pm(1.0, Double(vb->numberCoh()), 
			 dopsf?"Gridding Weights and PSF":"Major Cycle", "","","",True);
	Int cohDone=0;


    	if(!dopsf)itsMappers.initializeDegrid(*vb);
    	itsMappers.initializeGrid(*vb,dopsf);
    	for (rvi_p->originChunks(); rvi_p->moreChunks();rvi_p->nextChunk())
    	{

    		for (rvi_p->origin(); rvi_p->more(); (*rvi_p)++)
    		{
		  //if (SynthesisUtilMethods::validate(*vb)==SynthesisUtilMethods::NOVALIDROWS) break; // No valid rows in this VB
		  //		  cerr << "nRows "<< vb->nRow() << "   " << max(vb->visCube()) <<  endl;
    			if(!dopsf) {
    				vb->setModelVisCube(Complex(0.0, 0.0));
    				itsMappers.degrid(*vb, savevirtualmodel );
    				if(savemodelcolumn && writeAccess_p )
    					wvi_p->setVis(vb->modelVisCube(),VisibilityIterator::Model);
    			}
    			itsMappers.grid(*vb, dopsf, datacol_p);
			cohDone += vb->nRow();
			pm.update(Double(cohDone));
    		}
    	}
    	//cerr << "IN SYNTHE_IMA" << endl;
    	//VisModelData::listModel(rvi_p->getMeasurementSet());
    	if(!dopsf) itsMappers.finalizeDegrid(*vb);
    	itsMappers.finalizeGrid(*vb, dopsf);

    }

    itsMappers.checkOverlappingModels("restore");

    unlockMSs();

  }// end runMajorCycle

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Method to run the AWProjectFT machine to seperate the CFCache
  // construction from imaging.  This is done by splitting the
  // operation in two steps: (1) run the FTM in "dry" mode to create a
  // "blank" CFCache, and (2) re-load the "blank" CFCache and "fill"
  // it.
  //
  // If someone can get me (SB) out of the horrible statc_casts in the
  // code below, I will be most grateful (we are out of it! :-)).
  //
  void SynthesisImager::dryGridding(const Vector<String>& cfList)
  {
    LogIO os( LogOrigin("SynthesisImager","dryGridding",WHERE) );
    Int cohDone=0, whichFTM=0;
    (void)cfList;
    // If not an AWProject-class FTM, make this call a NoOp.  Might be
    // useful to extend it to other projection FTMs -- but later.
    String ftmName = ((*(itsMappers.getFTM(whichFTM)))).name();

    if (!((itsMappers.getFTM(whichFTM,True))->isUsingCFCache())) return;

    os << "---------------------------------------------------- Dry Gridding ---------------------------------------------" << LogIO::POST;

    //
    // Go through the entire MS in "dry" mode to set up a "blank"
    // CFCache.  This is done by setting the AWPWBFT in dryrun mode
    // and gridding.  The process of gridding emits CFCache, which
    // will be "blank" in a dry run.
    {
      VisBufferAutoPtr vb(rvi_p);
      rvi_p->originChunks();
      rvi_p->origin();

      ProgressMeter pm(1.0, Double(vb->numberCoh()), "dryGridding", "","","",True);

      itsMappers.initializeGrid(*vb);
    
      // Set the gridder (iFTM) to run in dry-gridding mode
      (itsMappers.getFTM(whichFTM,True))->setDryRun(True);

      os << "Making a \"blank\" CFCache" << LogIO::WARN << LogIO::POST;

      // Step through the MS.  This triggers the logic in the Gridder
      // to determine all the CFs that will be required.  These empty
      // CFs are written to the CFCache which can then be filled via
      // a call to fillCFCache().
      for (rvi_p->originChunks(); rvi_p->moreChunks();rvi_p->nextChunk())
	{
	  
	  for (rvi_p->origin(); rvi_p->more(); (*rvi_p)++)
	    {
	      if (SynthesisUtilMethods::validate(*vb)==SynthesisUtilMethods::NOVALIDROWS) break; //No valid rows in this MS
	      itsMappers.grid(*vb, True, FTMachine::OBSERVED, whichFTM);
	      cohDone += vb->nRow();
	      pm.update(Double(cohDone));
	    }
	}
    }
    // Unset the dry-gridding mode.
    (itsMappers.getFTM(whichFTM,True))->setDryRun(False);

    //itsMappers.checkOverlappingModels("restore");
    unlockMSs();
    //fillCFCache(cfList);
  }
  //
  // Re-load the CFCache from the disk using the supplied list of CFs
  // (as cfList).  Then extract the ConvFunc object (which was setup
  // in the FTM) and call it's makeConvFunction2() to fill the CF.
  // Finally, unset the dry-run mode of the FTM.
  //
  void SynthesisImager::fillCFCache(const Vector<String>& cfList,
				    const String& ftmName,
				    const String& cfcPath,
				    const Bool& psTermOn,
				    const Bool& aTermOn)
    {
      LogIO os( LogOrigin("SynthesisImager","fillCFCache",WHERE) );
      // If not an AWProject-class FTM, make this call a NoOp.  Might be
      // useful to extend it to other projection FTMs -- but later.
      // String ftmName = ((*(itsMappers.getFTM(whichFTM)))).name();

      if (!ftmName.contains("awproject") and
	  !ftmName.contains("multitermftnew")) return;
      //if (!ftmName.contains("awproject")) return;
      
      os << "---------------------------------------------------- fillCFCache ---------------------------------------------" << LogIO::POST;

      //String cfcPath = itsMappers.getFTM(whichFTM)->getCacheDir();
      //String imageNamePrefix=itsMappers.getFTM(whichFTM)->getCFCache()->getWtImagePrefix();

      //cerr << "Path = " << path << endl;

      // CountedPtr<AWProjectWBFTNew> tmpFT = new AWProjectWBFTNew(static_cast<AWProjectWBFTNew &> (*(itsMappers.getFTM(whichFTM))));


      Float dPA=360.0,selectedPA=2*360.0;
      if (cfList.nelements() > 0)
      {
	  CountedPtr<CFCache> cfCacheObj = new CFCache();
	  //Vector<String> wtCFList; wtCFList.resize(cfList.nelements());
	  //for (Int i=0; i<wtCFList.nelements(); i++) wtCFList[i] = "WT"+cfList[i];
	  //Directory dir(path);
	  Vector<String> cfList_p=cfList;//dir.find(Regex(Regex::fromPattern("CFS*")));
	  Vector<String> wtCFList_p;
	  wtCFList_p.resize(cfList_p.nelements());
	  for (Int i=0; i<(Int)wtCFList_p.nelements(); i++) wtCFList_p[i]="WT"+cfList_p[i];

	  cerr << cfList_p << endl;
      	  cfCacheObj->setCacheDir(cfcPath.data());

	  os << "Re-loading the \"blank\" CFCache for filling" << LogIO::WARN << LogIO::POST;

      	  cfCacheObj->initCacheFromList2(cfcPath, cfList_p, wtCFList_p,
      					 selectedPA, dPA,1);
	  // tmpFT->setCFCache(cfCacheObj);
	  Vector<Double> uvScale, uvOffset;
	  Matrix<Double> vbFreqSelection;
	  CountedPtr<CFStore2> cfs2 = CountedPtr<CFStore2>(&cfCacheObj->memCache2_p[0],False);//new CFStore2;
	  CountedPtr<CFStore2> cfwts2 =  CountedPtr<CFStore2>(&cfCacheObj->memCacheWt2_p[0],False);//new CFStore2;

	  //
	  // Get whichFTM from itsMappers (SIMapperCollection) and
	  // cast it as AWProjectWBFTNew.  Then get the ConvFunc from
	  // the FTM and cast it as AWConvFunc.  Finally call
	  // AWConvFunc::makeConvFunction2().
	  //
	  // (static_cast<AWConvFunc &> 
	  //  (*(static_cast<AWProjectWBFTNew &> (*(itsMappers.getFTM(whichFTM)))).getAWConvFunc())
	  //  ).makeConvFunction2(String(path), uvScale, uvOffset, vbFreqSelection,
	  // 		       *cfs2, *cfwts2);

	  // This is a global methond in AWConvFunc.  Does not require
	  // FTM to be constructed (which can be expensive in terms of
	  // memory footprint).
	  AWConvFunc::makeConvFunction2(String(cfcPath), uvScale, uvOffset, vbFreqSelection,
					*cfs2, *cfwts2, psTermOn, aTermOn);
      	}
      //cerr << "Mem used = " << itsMappers.getFTM(whichFTM)->getCFCache()->memCache2_p[0].memUsage() << endl;
      //(static_cast<AWProjectWBFTNew &> (*(itsMappers.getFTM(whichFTM)))).getCFCache()->initCache2();
    }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void SynthesisImager::reloadCFCache()
  {
      LogIO os( LogOrigin("SynthesisImager","reloadCFCache",WHERE) );
      Int whichFTM=0;
      String ftmName = ((*(itsMappers.getFTM(whichFTM)))).name();
      if (!ftmName.contains("AWProject")) return;

      os << "-------------------------------------------- reloadCFCache ---------------------------------------------" << LogIO::POST;
      String path = itsMappers.getFTM(whichFTM)->getCacheDir();
      String imageNamePrefix=itsMappers.getFTM(whichFTM)->getCFCache()->getWtImagePrefix();

      CountedPtr<CFCache> cfCacheObj = new CFCache();
      cfCacheObj->setCacheDir(path.data());
      cfCacheObj->setWtImagePrefix(imageNamePrefix.c_str());
      cfCacheObj->initCache2();
      
      // This assumes the itsMappers is always SIMapperCollection.
      for (whichFTM = 0; whichFTM < itsMappers.nMappers(); whichFTM++)
	{
	  (static_cast<AWProjectWBFTNew &> (*(itsMappers.getFTM(whichFTM)))).setCFCache(cfCacheObj,True); // Setup iFTM
	  (static_cast<AWProjectWBFTNew &> (*(itsMappers.getFTM(whichFTM,False)))).setCFCache(cfCacheObj,True); // Set FTM
	}
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void SynthesisImager::predictModel(){
    LogIO os( LogOrigin("SynthesisImager","predictModel ",WHERE) );

    os << "---------------------------------------------------- Predict Model ---------------------------------------------" << LogIO::POST;
    
    Bool savemodelcolumn = !readOnly_p && useScratch_p;
    Bool savevirtualmodel = !readOnly_p && !useScratch_p;

    if( savemodelcolumn ) os << "Saving model column" << LogIO::POST;
    if( savevirtualmodel ) os << "Saving virtual model" << LogIO::POST;

    itsMappers.checkOverlappingModels("blank");


    {
      VisBufferAutoPtr vb(rvi_p);
      rvi_p->originChunks();
      rvi_p->origin();

      ProgressMeter pm(1.0, Double(vb->numberCoh()), 
		       "Predict Model", "","","",True);
      Int cohDone=0;

      itsMappers.initializeDegrid(*vb);
      for (rvi_p->originChunks(); rvi_p->moreChunks();rvi_p->nextChunk())
	{
	  
	  for (rvi_p->origin(); rvi_p->more(); (*rvi_p)++)
	    {
	      //if (SynthesisUtilMethods::validate(*vb)==SynthesisUtilMethods::NOVALIDROWS) break; //No valid rows in this MS
	      //if !usescratch ...just save
	      vb->setModelVisCube(Complex(0.0, 0.0));
	      itsMappers.degrid(*vb, savevirtualmodel);
	      if(savemodelcolumn && writeAccess_p )
		wvi_p->setVis(vb->modelVisCube(),VisibilityIterator::Model);

	      //	      cout << "nRows "<< vb->nRow() << "   " << max(vb->modelVisCube()) <<  endl;
	      cohDone += vb->nRow();
	      pm.update(Double(cohDone));

	    }
	}
      itsMappers.finalizeDegrid(*vb);
    }

    itsMappers.checkOverlappingModels("restore");
    unlockMSs();
   
  }// end of predictModel

  //Utility function to properly convert Double to String.
  //With C++11 we can probably use STL to_string() function instead...
  String SynthesisImager::doubleToString(const Double& df) {
    std::ostringstream ss;
    ss.precision(std::numeric_limits<double>::digits10+2);
    ss << df;
    return ss.str();
  }

 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


} //# NAMESPACE CASA - END

