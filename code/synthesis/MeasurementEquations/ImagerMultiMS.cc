//# ImagerMultiMS.cc: Implementation of ImagerMultiMS.h
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

#include <synthesis/MeasurementEquations/ImagerMultiMS.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>

#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSDataDescIndex.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MSVis/SimpleSubMS.h>
#include <synthesis/MSVis/SubMS.h>
#include <synthesis/MSVis/VisSetUtil.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/SetupNewTab.h>

#include <lattices/Lattices/LatticeExpr.h> 

#include <casa/OS/File.h>
#include <casa/OS/HostInfo.h>
#include <casa/Containers/Record.h>

#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  ImagerMultiMS::ImagerMultiMS() 
    : Imager(), blockNChan_p(0), blockStart_p(0), blockStep_p(0), blockSpw_p(0),
      blockMSSel_p(0), dataSet_p(False)
  {
    
    lockCounter_p=0;
    ms_p=0;
    mssel_p=0;
    se_p=0;
    vs_p=0;
    ft_p=0;
    cft_p=0;
    rvi_p=wvi_p=0;
    numMS_p=0;
    
  }

  Bool ImagerMultiMS::setDataToMemory(const String& msname, 
                                      const String& /*mode*/, 
				   const Vector<Int>& nchan, 
				   const Vector<Int>& start,
				   const Vector<Int>& step,
                                      const Vector<Int>& /*spectralwindowids*/,
				   const Vector<Int>& /*fieldids*/,
				   const String& msSelect,
				   const String& timerng,
				   const String& fieldnames,
                                      const Vector<Int>& /*antIndex*/,
				   const String& antnames,
				   const String& spwstring,
				   const String& uvdist,
				      const String& scan,
                                      const String& intent,
                                      const String& obs){
    useModelCol_p=False;
    LogIO os(LogOrigin("imager", "setDataToMemory()"), logSink_p);
    if(!Table::isReadable(msname)){
      os << LogIO::SEVERE << "MeasurementSet " 
	 << msname << " does not exist  " 
	 << LogIO::POST;
      return False;
    }
    MeasurementSet thisms(msname, TableLock(TableLock::AutoNoReadLocking), 
			      Table::Old);
    SimpleSubMS splitter(thisms);
    //SubMS splitter(thisms);
    splitter.setmsselect(spwstring, fieldnames, antnames, scan, obs, uvdist,
			 msSelect, nchan, start, step, "");
    splitter.selectCorrelations("");
    splitter.selectTime(-1.0, timerng);
    MS::PredefinedColumns whichCol=MS::DATA;
    if(thisms.tableDesc().isColumn("CORRECTED_DATA"))
      whichCol=MS::CORRECTED_DATA;
    CountedPtr<MeasurementSet> subMS(splitter.makeMemSubMS(whichCol), True);
    //CountedPtr<MeasurementSet> subMS(splitter.makeScratchSubMS(Vector<MS::PredefinedColumns>(1,whichCol), True), True);
    return setDataOnThisMS(*subMS);

  }


  Bool ImagerMultiMS::setDataPerMS(const String& msname, const String& mode, 
				   const Vector<Int>& nchan, 
				   const Vector<Int>& start,
				   const Vector<Int>& step,
				   const Vector<Int>& spectralwindowids,
				   const Vector<Int>& fieldids,
				   const String& msSelect,
				   const String& timerng,
				   const String& fieldnames,
				   const Vector<Int>& antIndex,
				   const String& antnames,
				   const String& spwstring,
				   const String& uvdist,
                                   const String& scan,
                                   const String& intent,
                                   const String& obs,
                                   const Bool useModel, 
				   const Bool readonly){
    useModelCol_p=useModel;
    Bool rd=readonly;
    LogIO os(LogOrigin("imager", "setDataPerMS()"), logSink_p);
    if(useModel) 
      rd=True;
    if(!Table::isReadable(msname)){
      os << LogIO::SEVERE << "MeasurementSet " 
	 << msname << " does not exist  " 
	 << LogIO::POST;
      return False;
    }
    else{
      MeasurementSet thisms;
      if(!readonly)
	thisms=MeasurementSet(msname, TableLock(TableLock::AutoNoReadLocking), 
			      Table::Update);
      else
       thisms=MeasurementSet(msname, TableLock(TableLock::AutoNoReadLocking), 
			      Table::Old);
      //thisms=MeasurementSet(msname, TableLock(), 
      //			      Table::Old);
      
      thisms.setMemoryResidentSubtables (MrsEligibility::defaultEligible());

      return setDataOnThisMS(thisms, mode, nchan, start, step, spectralwindowids, fieldids, msSelect, timerng, 
			     fieldnames, antIndex, antnames, spwstring, uvdist, scan, intent, obs);  
      
    }
    
  }



  Bool ImagerMultiMS::setDataOnThisMS(MeasurementSet& thisms, const String& mode, 
				   const Vector<Int>& nchan, 
				   const Vector<Int>& start,
				   const Vector<Int>& step,
				   const Vector<Int>& spectralwindowids,
				   const Vector<Int>& fieldids,
				   const String& msSelect,
				   const String& timerng,
				   const String& fieldnames,
				   const Vector<Int>& antIndex,
				   const String& antnames,
				   const String& spwstring,
				   const String& uvdist,
                                      const String& scan,
                                      const String& intent,
                                      const String& obs){
    LogIO os(LogOrigin("imager", "setDataOnThisMS()"), logSink_p);  


    dataMode_p=mode;
    dataNchan_p.resize();
    dataStart_p.resize();
    dataStep_p.resize();
    dataNchan_p=nchan;
    dataStart_p=start;
    dataStep_p=step;
    dataspectralwindowids_p.resize(spectralwindowids.nelements());
    dataspectralwindowids_p=spectralwindowids;
    datafieldids_p.resize(fieldids.nelements());
    datafieldids_p=fieldids;
    


    //make ms
    // auto lock for now
    // make ms and visset
    ++numMS_p;
    blockMSSel_p.resize(numMS_p);
    blockNChan_p.resize(numMS_p);
    blockStart_p.resize(numMS_p);
    blockStep_p.resize(numMS_p);
    blockSpw_p.resize(numMS_p);
    //Using autolocking here 
    //Will need to rethink this when in parallel mode with multi-cpu access
    blockMSSel_p[numMS_p-1]=thisms;
    //breaking reference
    if(ms_p.null())
      ms_p=new MeasurementSet();
    else
      (*ms_p)=MeasurementSet();
    (*ms_p)=thisms;
    
    
    try{
      openSubTables();
      
      //if spectralwindowids=-1 take all
      if(mode=="none" && 
	 (spectralwindowids.nelements()==1 && spectralwindowids[0]<0)){
	Int nspw=thisms.spectralWindow().nrow();
	dataspectralwindowids_p.resize(nspw);
	indgen(dataspectralwindowids_p);
	
      }
      
      // If a selection has been made then close the current MS
      // and attach to a new selected MS. We do this on the original
      // MS.
      //I don't think i need this if statement
      //   if(datafieldids_p.nelements()>0||datadescids_p.nelements()>0) {
      os << "Performing selection on MeasurementSet : " << thisms.tableName() << LogIO::POST;
      //if(vs_p) delete vs_p; vs_p=0;
      //if(mssel_p) delete mssel_p; 
      mssel_p=0;
      
      // check that sorted table exists (it should), if not, make it now.
      //this->makeVisSet(thisms);
      
      //if you want to use scratch col...make sure they are there
      if(useModelCol_p){
	VisSetUtil::addScrCols(thisms, True, False, True, False);
	VisModelData::clearModel(thisms);
      }
      //MeasurementSet sorted=thisms.keywordSet().asTable("SORTED_TABLE");
      
      
      //Some MSSelection 
      MSSelection thisSelection;
      if(datafieldids_p.nelements() > 0){
	thisSelection.setFieldExpr(MSSelection::indexExprStr(datafieldids_p));
	os << "Selecting on field ids : " << datafieldids_p << LogIO::POST;
      }
      if(fieldnames != ""){
	thisSelection.setFieldExpr(fieldnames);
	os << "Selecting on fields : " << fieldnames << LogIO::POST;
      }
      if(dataspectralwindowids_p.nelements() > 0){
	thisSelection.setSpwExpr(MSSelection::indexExprStr(dataspectralwindowids_p));
	os << "Selecting on spectral windows" << LogIO::POST;
      }
      else if(spwstring != ""){
	thisSelection.setSpwExpr(spwstring);
	os << "Selecting on spectral windows expression :"<< spwstring  << LogIO::POST;
      }
      if(antIndex.nelements() >0){
	thisSelection.setAntennaExpr( MSSelection::indexExprStr(antIndex) );
	os << "Selecting on antenna ids : " << antIndex << LogIO::POST;	
      }
      if(antnames != ""){
	Vector<String> antNames(1, antnames);
	// thisSelection.setAntennaExpr(MSSelection::nameExprStr( antNames));
	thisSelection.setAntennaExpr(antnames);
	os << "Selecting on antenna names : " << antnames << LogIO::POST;
	
      }            
      if(timerng != ""){
	thisSelection.setTimeExpr(timerng);
	os << "Selecting on time range : " << timerng << LogIO::POST;	
      }
      if(uvdist != ""){
	thisSelection.setUvDistExpr(uvdist);
	os << "Selecting on uvdist : " << uvdist << LogIO::POST;	
      }
      if(scan != ""){
	thisSelection.setScanExpr(scan);
	os << "Selecting on scan : " << scan << LogIO::POST;	
      }
      if(intent != ""){
	thisSelection.setStateExpr(intent);
	os << "Selecting on State(scan intent) Expr " << intent << LogIO::POST;	
      }
      if(obs != ""){
	thisSelection.setObservationExpr(obs);
	os << "Selecting on Observation Expr : " << obs << LogIO::POST;	
      }
      if(msSelect != ""){
	thisSelection.setTaQLExpr(msSelect);
	os << "Selecting via TaQL : " << msSelect << LogIO::POST;	
      }
      //***************
      
      TableExprNode exprNode=thisSelection.toTableExprNode(&thisms);
      //if(exprNode.isNull())
      //	throw(AipsError("Selection failed...review ms and selection parameters"));
      datafieldids_p.resize();
      datafieldids_p=thisSelection.getFieldList();
      if(datafieldids_p.nelements()==0){
	Int nf=ms_p->field().nrow();
	datafieldids_p.resize(nf);
	indgen(datafieldids_p);
      }
      if((numMS_p > 1) || datafieldids_p.nelements() > 1)
	multiFields_p= True;
      //Now lets see what was selected as spw and match it with datadesc
      //dataspectralwindowids_p.resize();
      //dataspectralwindowids_p=thisSelection.getSpwList();
      Matrix<Int> chansels=thisSelection.getChanList(NULL, 1, True);
      mssFreqSel_p.resize();
      mssFreqSel_p=thisSelection.getChanFreqList(NULL, True);
     
      uInt nms = numMS_p;
      uInt nrow = chansels.nrow(); 
      dataspectralwindowids_p.resize();
      const ROMSSpWindowColumns spwc(thisms.spectralWindow());
      uInt nspw = spwc.nrow();
      const ROScalarColumn<Int> spwNchans(spwc.numChan());
      Vector<Int> nchanvec = spwNchans.getColumn();
      //cerr<<"SetDataOnThisMS::numMS_p="<<numMS_p<<" nchanvec="<<nchanvec<<endl;
      Int maxnchan = 0;
      
      for (uInt i=0;i<nchanvec.nelements();i++) {
	maxnchan=max(nchanvec[i],maxnchan);
      }
      //cout<<"spwchansels_p.shape()="<<spwchansels_p.shape()<<endl;
      uInt maxnspw = 0;
      if (numMS_p ==1) {
        maxnspw=nspw;
      }
      else {
        for (Int i=0;i<numMS_p-1;i++) {
          maxnspw=max(maxnspw,blockSpw_p[i].nelements());
        }
        maxnspw=max(nspw,maxnspw);
      }
      spwchansels_p.resize(nms,maxnspw,maxnchan,True);
      //cout<<"After resize: spwchansels_p.shape()="<<spwchansels_p.shape()<<endl;
      uInt nselspw=0;
      if (nrow==0) {
        //no channel selection, select all channels
        spwchansels_p.yzPlane(numMS_p-1)=1; 
        dataspectralwindowids_p=thisSelection.getSpwList();
      }
      else {
        spwchansels_p.yzPlane(numMS_p-1)=0; 
        Int prvspwid=-1;
        Vector<Int> selspw;
	for (uInt i=0;i<nrow;i++) {
	  Vector<Int> sel = chansels.row(i);
	  Int spwid = sel[0];
	  if((sel[1] >= nchanvec[spwid]) || (sel[2] >=nchanvec[spwid]))
	    throw(AipsError("Unexpected selection  in spw selection of spwid "+String::toString(spwid)));
	  if (spwid != prvspwid){
	    nselspw++;
	    selspw.resize(nselspw,True);
	    selspw[nselspw-1]=spwid;
	  }
	  uInt minc= sel[1];
	  uInt maxc = sel[2];
	  uInt step = sel[3];
	  // step as the same context as in im.selectvis
	  // select channels
	  for (uInt k=minc;k<(maxc+1);k+=step) {
	    spwchansels_p(numMS_p-1,spwid,k)=1;
	  }
	  prvspwid=spwid;
	}
	dataspectralwindowids_p=selspw;
      }
      
      //cout<<"spwchansels_p(before shifting)="<<spwchansels_p<<endl;
      if(dataspectralwindowids_p.nelements()==0){
        Int nspwinms=thisms.spectralWindow().nrow();
        dataspectralwindowids_p.resize(nspwinms);
        indgen(dataspectralwindowids_p);
      }
      
      // old code
      /***
	  if(dataspectralwindowids_p.nelements()==0){
	  Int nspwinms=thisms.spectralWindow().nrow();
	  dataspectralwindowids_p.resize(nspwinms);
	  indgen(dataspectralwindowids_p);
	  }
      ***/
      
      // Map the selected spectral window ids to data description ids
      MSDataDescIndex msDatIndex(thisms.dataDescription());
      datadescids_p.resize(0);
      datadescids_p=msDatIndex.matchSpwId(dataspectralwindowids_p);
      
      freqrange_p.resize(nms,2,True);
      if(mode=="none"){
	//check if we can find channel selection in the spw string
	//if(nselspw==dataspectralwindowids_p.nelements()){
	dataMode_p="channel";
	dataStep_p.resize(dataspectralwindowids_p.nelements());
	dataStart_p.resize(dataspectralwindowids_p.nelements());
	dataNchan_p.resize(dataspectralwindowids_p.nelements());
	Double fmin=C::dbl_max;
	Double fmax=-(C::dbl_max);
	
	Cube<Int> spwchansels_tmp=spwchansels_p;
	for (uInt k =0 ; k < dataspectralwindowids_p.nelements(); ++k){
	  uInt curspwid=dataspectralwindowids_p[k];
	  Vector<Double> chanFreq=spwc.chanFreq()(curspwid);
	  Vector<Double> freqResolution = spwc.chanWidth()(curspwid);
	  
          
	  //dataStep_p[k]=chanselmat.row(k)(3);
	  if (nrow==0) {
	    //dataStep_p=step[0];
	    dataStep_p[k]=step[0];
	  }
	  else {
	    dataStep_p[k]=chansels.row(k)(3);
	  }
	  //if(dataStep_p[k] < 1)
	  //  dataStep_p[k]=1;
	  dataStart_p[k]=0;
	  dataNchan_p[k]=nchanvec(curspwid);
	  //cout<<"SetDataOnThisMS: initial setting dataNchan_p["<<k<<"]="<<dataNchan_p[k]<<endl;
	  //find start
	  Bool first = True;
	  uInt nchn = 0;
	  uInt lastchan = 0;
	  for (Int j=0 ; j < nchanvec(curspwid); j++) {
	    if (spwchansels_p(numMS_p-1,curspwid,j)==1) {
	      if(first) {
		dataStart_p[k]=j;
		first = False;
	      }
	      lastchan=j;
	      nchn++;
	    }
	  }
	  //dataStart_p[k]=chanselmat.row(k)(1);
	  //dataNchan_p[k]=Int(ceil(Double(chanselmat.row(k)(2)-dataStart_p[k]))/Double(dataStep_p[k]))+1;
	  dataNchan_p[k]=Int(ceil(Double(lastchan-dataStart_p[k])/Double(dataStep_p[k])))+1;
	  //cout<<"SetDataOnThisMS: after recalc. of nchan dataNchan_p["<<k<<"]="<<dataNchan_p[k]<<endl;
	  //if(dataNchan_p[k]<1)
	  //  dataNchan_p[k]=1;
	  // 
	  //Since msselet will be applied to the data before flags from spwchansels_p
	  //are applied to the data in FTMachine, shift spwchansels_p by dataStart_p
	  for (Int j=0  ; j < nchanvec(curspwid); j++){
	    if (j<nchanvec(curspwid)-dataStart_p[k]) {
	      spwchansels_tmp(numMS_p-1,curspwid,j) = spwchansels_p(numMS_p-1,curspwid,j+dataStart_p[k]);
	    }
	    else {
	      spwchansels_tmp(numMS_p-1,curspwid,j) = 0;
	    }
	  }
	  //for mfs mode need to keep fmin,max info for later image setup
	  //Int lastchan=dataStart_p[k]+ dataNchan_p[k]*dataStep_p[k];
	  Int endchanused;
	  if (nrow==0) {
	    // default spw case
	    endchanused=nchanvec(curspwid);
	  }
	  else {
	    endchanused=lastchan;
	  }
	  for(Int j=dataStart_p[k] ; j < endchanused ;  j+=dataStep_p[k]){
	    fmin=min(fmin,chanFreq[j]-abs(freqResolution[j]*(dataStep_p[k]-0.5)));
	    fmax=max(fmax,chanFreq[j]+abs(freqResolution[j]*(dataStep_p[k]-0.5)));
	  }
	}
	//cerr<<"numMS_p="<<numMS_p<<" fmin="<<fmin<<" fmax="<<fmax<<endl;
	freqrange_p(numMS_p-1,0)=fmin;
	freqrange_p(numMS_p-1,1)=fmax;
	
        spwchansels_p=spwchansels_tmp;
	//}
      }
      
      
      
      // Now remake the selected ms
      if(!(exprNode.isNull())){
	mssel_p = new MeasurementSet(thisms(exprNode));
      }
      else{
	// Null take all the ms ...setdata() blank means that
	mssel_p = new MeasurementSet(thisms);
      } 
      AlwaysAssert(!mssel_p.null(), AipsError);
      if(mssel_p->nrow()==0) {
	//delete mssel_p; 
	mssel_p=0;
	
	os << "Selection is empty: you may want to review this MSs selection"
	   << LogIO::EXCEPTION;
      }
      else {
	mssel_p->flush();
      }
      
      if(mssel_p->nrow()!=thisms.nrow()) {
	os << "Selected " << mssel_p->nrow() << " out of  "
	   << thisms.nrow() << " rows." << LogIO::POST;
      }
      else {
	os << "Selected all " << mssel_p->nrow() << " rows" << LogIO::POST;
      }
      //  }
      
      // Tell the user how many channels have been selected.
      // NOTE : This code is replicated in Imager.cc.
      Vector<Int> chancounts(dataspectralwindowids_p.nelements());
      chancounts=0;
      //    if( spwstring == "" ) os << "Selected all spws and channels" << LogIO::POST;
      //   else os << "Channel selection : " << spwstring << LogIO::POST;
      os << "Selected :";
      for(uInt k=0;k<dataspectralwindowids_p.nelements();k++)
	{
	  for(uInt ch=0;ch<uInt(nchanvec(dataspectralwindowids_p[k]));ch++) 
	    {if(spwchansels_p(numMS_p-1,dataspectralwindowids_p[k],ch)) chancounts[k]++; }
	  //if(chancounts[k]<1)
	  //  throw(AipsError("bad selection in spw "+ String::toString( dataspectralwindowids_p[k])));
	  os << " [" << chancounts[k] << " chans in spw " << dataspectralwindowids_p[k] << "]";
	  //	os << "Selected " << chancounts[k] << " chans in spw " 
	  //	   << dataspectralwindowids_p[k] << LogIO::POST;
	}
      os << LogIO::POST;
      
      
      blockMSSel_p[numMS_p-1]=*mssel_p;
      //lets make the visSet now
      Block<Matrix<Int> > noChanSel;
      noChanSel.resize(numMS_p);
      Block<Int> sort(0);
      //if(vs_p) delete vs_p; vs_p=0;
      if(rvi_p) delete rvi_p;
      rvi_p=0;
      wvi_p=0;
      
      //vs_p= new VisSet(blockMSSel_p, sort, noChanSel, useModelCol_p);
      if(!(mssel_p->isWritable())){
	rvi_p=new ROVisibilityIterator(blockMSSel_p, sort);
	
      }
      else{
	wvi_p=new VisibilityIterator(blockMSSel_p, sort);
	rvi_p=wvi_p;    
      }
      
      if(imwgt_p.getType()=="none")
	imwgt_p=VisImagingWeight("natural");
      rvi_p->useImagingWeight(imwgt_p);
      //rvi_p->slurp();
      
      selectDataChannel();
      dataSet_p=True;
      
      return dataSet_p;
    }
    catch(AipsError x){
      //Ayeee...lets back out of this one
      --numMS_p;
      blockMSSel_p.resize(numMS_p, True);
      blockNChan_p.resize(numMS_p, True);
      blockStart_p.resize(numMS_p, True);
      blockStep_p.resize(numMS_p, True);
      blockSpw_p.resize(numMS_p, True);
      //point it back to the previous ms
      if(numMS_p >0){
	mssel_p=new MeasurementSet(blockMSSel_p[numMS_p-1]);
      }
      throw(AipsError(x));
    }

    } //End of setDataPerMS
    

Bool ImagerMultiMS::setimage(const Int nx, const Int ny,
			     const Quantity& cellx, const Quantity& celly,
			     const String& stokes,
			     Bool doShift,
			     const MDirection& phaseCenter, 
			     const Quantity& shiftx, const Quantity& shifty,
			     const String& mode, const Int nchan,
			     const Int start, const Int step,
			     const MRadialVelocity& mStart, 
			     const MRadialVelocity& mStep,
			     const Vector<Int>& spectralwindowids,
			     const Int fieldid,
			     const Int facets,
			     const Quantity& distance)
{

  if(!dataSet_p){
    LogIO os(LogOrigin("imagerMultiMS", "setimage()"), logSink_p);  
    os << LogIO::SEVERE 
       << "Please use setdata before setimage as imager need one ms at least " 
       << LogIO::POST;
    return False;

  }

  Bool returnval=Imager::setimage(nx, ny, cellx, celly, stokes,doShift,
				  phaseCenter, shiftx, shifty, mode, nchan,
				  start, step, mStart, mStep, 
				  spectralwindowids, 
				  fieldid, facets, distance);

  return returnval;
}

  Bool ImagerMultiMS::lock(){
    //Do nothing for now as its Autolocked..but a better scheme is necessary
    //for parallel access to same data for modification

    return True;
  }

  Bool ImagerMultiMS::unlock(){

    for (uInt k=0; k < blockMSSel_p.nelements(); ++k){
      blockMSSel_p[k].relinquishAutoLocks(True);
    }
    return True;

  }


  Bool ImagerMultiMS::selectDataChannel(){

    LogIO os(LogOrigin("imager", "setDataPerMS()"), logSink_p);

    (blockNChan_p[numMS_p-1]).resize(dataspectralwindowids_p.nelements());
    (blockStart_p[numMS_p-1]).resize(dataspectralwindowids_p.nelements());
    (blockStart_p[numMS_p-1]).set(0);
    (blockStep_p[numMS_p-1]).resize(dataspectralwindowids_p.nelements());
    (blockStep_p[numMS_p-1]).set(1);
    (blockSpw_p[numMS_p-1]).resize(dataspectralwindowids_p.nelements());
    (blockSpw_p[numMS_p-1]).resize();
    (blockSpw_p[numMS_p-1])=dataspectralwindowids_p;
    {
      //Fill default numChan for now
      
      ROMSSpWindowColumns msSpW(ms_p->spectralWindow());
      Vector<Int> numChan=msSpW.numChan().getColumn(); 
      for (uInt k=0; k < dataspectralwindowids_p.nelements(); ++k){
	blockNChan_p[numMS_p-1][k]=numChan[dataspectralwindowids_p[k]];

      }
    }

     if(dataMode_p=="channel") {

      if (dataNchan_p.nelements() != dataspectralwindowids_p.nelements()){
	if(dataNchan_p.nelements()==1){
	  dataNchan_p.resize(dataspectralwindowids_p.nelements(), True);
	  for(uInt k=1; k < dataspectralwindowids_p.nelements(); ++k){
	    dataNchan_p[k]=dataNchan_p[0];
	  }
	}
	else{
	  os << LogIO::SEVERE 
	     << "Vector of nchan has to be of size 1 or be of the same shape as spw " 
	     << LogIO::POST;
	  return False; 
	}
      }
      if (dataStart_p.nelements() != dataspectralwindowids_p.nelements()){
	if(dataStart_p.nelements()==1){
	  dataStart_p.resize(dataspectralwindowids_p.nelements(), True);
	  for(uInt k=1; k < dataspectralwindowids_p.nelements(); ++k){
	    dataStart_p[k]=dataStart_p[0];
	  }
	}
	else{
	  os << LogIO::SEVERE 
	     << "Vector of start has to be of size 1 or be of the same shape as spw " 
	     << LogIO::POST;
	  return False; 
	}
      }
      if (dataStep_p.nelements() != dataspectralwindowids_p.nelements()){
	if(dataStep_p.nelements()==1){
	  dataStep_p.resize(dataspectralwindowids_p.nelements(), True);
	  for(uInt k=1; k < dataspectralwindowids_p.nelements(); ++k){
	    dataStep_p[k]=dataStep_p[0];
	  }
	}
	else{
	  os << LogIO::SEVERE 
	     << "Vector of step has to be of size 1 or be of the same shape as spw " 
	     << LogIO::POST;
	  return False; 
	}
      }

       {
	Int nch=0;
	for(uInt i=0;i<dataspectralwindowids_p.nelements();i++) {
	  Int spwid=dataspectralwindowids_p(i);
	  if(dataStart_p[i]<0) {
	    os << LogIO::SEVERE << "Illegal start pixel = " 
	       << dataStart_p[i]  << " for spw " << spwid
	       << LogIO::POST;
	    return False;
	  }
	 
	  if(dataNchan_p[i]<=0){
	    if(dataStep_p[i] <= 0)
	      dataStep_p[i]=1;
	    nch=(blockNChan_p[numMS_p-1](i)-dataStart_p[i])/Int(dataStep_p[i])+1;
	    
	  }
	  else nch = dataNchan_p[i];
	  while((nch*dataStep_p[i]+dataStart_p[i]) >
		blockNChan_p[numMS_p-1](i)){
	    --nch;
	  }
	  Int end = Int(dataStart_p[i]) + Int(nch-1) * Int(dataStep_p[i]);
	  if(end < 0 || end > (blockNChan_p[numMS_p-1](i)-1)) {
	    os << LogIO::SEVERE << "Illegal step pixel = " << dataStep_p[i]
	       << " for spw " << spwid
	       << "\n end channel " << end << " out of range " 
	       << dataStart_p[i] << " to "   
	       << (blockNChan_p[numMS_p-1](i)-1)

	       << LogIO::POST;
	    return False;
	  }
	  os << LogIO::DEBUG1 << "Selecting "<< nch
	     << " channels, starting at visibility channel "
	     << dataStart_p[i]  << " stepped by "
	     << dataStep_p[i] << " for spw " << spwid << LogIO::POST;
	  dataNchan_p[i]=nch;
	  blockNChan_p[numMS_p-1][i]=nch;
	  blockStep_p[numMS_p-1][i]=dataStep_p[i];
	  blockStart_p[numMS_p-1][i]=dataStart_p[i];
	}
       }
     
     }
 
     Block<Vector<Int> > blockGroup(numMS_p);
     for (Int k=0; k < numMS_p; ++k){
       blockGroup[k].resize(blockSpw_p[k].nelements());
       blockGroup[k].set(1);
     }

     rvi_p->selectChannel(blockGroup, blockStart_p, blockNChan_p, 
			  blockStep_p, blockSpw_p);
     return True;
  }

  Bool ImagerMultiMS::openSubTables(){

      // Reopen disk-resident tables with the default table lock

      if((ms_p->tableType()) != Table::Memory){

          TableLock tableLock; // default table lock

          openSubTable (ms_p->antenna(), antab_p, tableLock);
          openSubTable (ms_p->dataDescription (), datadesctab_p, tableLock);
          openSubTable (ms_p->doppler(), dopplertab_p, tableLock);
          openSubTable (ms_p->feed(), feedtab_p, tableLock);
          openSubTable (ms_p->field(), fieldtab_p, tableLock);
          openSubTable (ms_p->flagCmd(), flagcmdtab_p, tableLock);
          openSubTable (ms_p->freqOffset(), freqoffsettab_p, tableLock);
          openSubTable (ms_p->observation(), obstab_p, tableLock);
          openSubTable (ms_p->pointing(), pointingtab_p, tableLock);
          openSubTable (ms_p->polarization(), poltab_p, tableLock);
          openSubTable (ms_p->processor(), proctab_p, tableLock);
          openSubTable (ms_p->source(), sourcetab_p, tableLock);
          openSubTable (ms_p->spectralWindow(), spwtab_p, tableLock);
          openSubTable (ms_p->state(), statetab_p, tableLock);
          openSubTable (ms_p->sysCal(), syscaltab_p, tableLock);
          openSubTable (ms_p->weather(), weathertab_p, tableLock);

          // Handle the history table specially

          if(ms_p->isWritable()){

              if(!(Table::isReadable(ms_p->historyTableName()))){

                  // setup a new table in case its not there
                  TableRecord &kws = ms_p->rwKeywordSet();
                  SetupNewTable historySetup(ms_p->historyTableName(),
                                             MSHistory::requiredTableDesc(),Table::New);
                  kws.defineTable(MS::keywordName(MS::HISTORY), Table(historySetup));

              }

              historytab_p=Table(ms_p->historyTableName(), TableLock(), Table::Update);

              hist_p= new MSHistoryHandler(*ms_p, "imager");
          }
      }

      return True;
  }

} //# NAMESPACE CASA - END
