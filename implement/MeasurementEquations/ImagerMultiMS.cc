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

#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisibilityIterator.h>
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
      blockMSSel_p(0), numMS_p(0), dataSet_p(False)
  {
    
    lockCounter_p=0;
    ms_p=0;
    mssel_p=0;
    se_p=0;
    vs_p=0;
    ft_p=0;
    cft_p=0;
    
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
				   const String& antnames) {
    LogIO os(LogOrigin("imager", "setDataPerMS()"), logSink_p);  


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
    MeasurementSet thisms;
    if(!Table::isWritable(msname)){
      os << LogIO::SEVERE << "MeasurementSet " 
	 << msname << " does not exist or is not writable " 
	 << LogIO::POST;
      return False;
    }
    else{
      ++numMS_p;
      blockMSSel_p.resize(numMS_p);
      blockNChan_p.resize(numMS_p);
      blockStart_p.resize(numMS_p);
      blockStep_p.resize(numMS_p);
      blockSpw_p.resize(numMS_p);
      //Using autolocking here 
      //Will need to rethink this when in parallel mode with multi-cpu access
      thisms=MeasurementSet(msname, TableLock(TableLock::AutoLocking), 
			    Table::Update);
      blockMSSel_p[numMS_p-1]=thisms;
      //breaking reference
      ms_p=new MeasurementSet();
      (*ms_p)=thisms;
    }
    

    openSubTables();

    //if spectralwindowids=-1 take all
    if(mode=="none" && 
       (spectralwindowids.nelements()==1 && spectralwindowids[0]<0)){
      Int nspw=thisms.spectralWindow().nrow();
      dataspectralwindowids_p.resize(nspw);
      indgen(dataspectralwindowids_p);

    }

    // Map the selected spectral window ids to data description ids
    MSDataDescIndex msDatIndex(thisms.dataDescription());
    datadescids_p.resize(0);
    datadescids_p=msDatIndex.matchSpwId(dataspectralwindowids_p);

 // If a selection has been made then close the current MS
    // and attach to a new selected MS. We do this on the original
    // MS.
    //I don't think i need this if statement
    //   if(datafieldids_p.nelements()>0||datadescids_p.nelements()>0) {
      os << "Performing selection on MeasurementSet" << LogIO::POST;
      if(vs_p) delete vs_p; vs_p=0;
      if(mssel_p) delete mssel_p; mssel_p=0;
      
      // check that sorted table exists (it should), if not, make it now.
      this->makeVisSet(thisms);
      
      MeasurementSet sorted=thisms.keywordSet().asTable("SORTED_TABLE");
      
      
      // Now we make a condition to do the old FIELD_ID, SPECTRAL_WINDOW_ID
      // selection
      /*      TableExprNode condition;
      String colf=MS::columnName(MS::FIELD_ID);
      String cols=MS::columnName(MS::DATA_DESC_ID);
      if(datafieldids_p.nelements()>0&&datadescids_p.nelements()>0){
	condition=sorted.col(colf).in(datafieldids_p)&&
	  sorted.col(cols).in(datadescids_p);
        os << "Selecting on field and spectral window ids" << LogIO::POST;
      }
      else if(datadescids_p.nelements()>0) {
	condition=sorted.col(cols).in(datadescids_p);
        os << "Selecting on spectral window id" << LogIO::POST;
      }
      else if(datafieldids_p.nelements()>0) {
	condition=sorted.col(colf).in(datafieldids_p);
        os << "Selecting on field id" << LogIO::POST;
      }
      
      */
      //Some MSSelection 
      MSSelection thisSelection;
      if(datafieldids_p.nelements() > 0){
	thisSelection.setFieldExpr(MSSelection::indexExprStr(datafieldids_p));
	os << "Selecting on field ids" << LogIO::POST;
      }
      if(fieldnames != ""){
	thisSelection.setFieldExpr(fieldnames);
      }
      if(datadescids_p.nelements() > 0){
	thisSelection.setSpwExpr(MSSelection::indexExprStr(dataspectralwindowids_p));
	os << "Selecting on spectral windows" << LogIO::POST;
      }
      if(antIndex.nelements() >0){
	thisSelection.setAntennaExpr( MSSelection::indexExprStr(antIndex) );
	os << "Selecting on antenna ids" << LogIO::POST;	
      }
      if(antnames != ""){
	Vector<String> antNames(1, antnames);
	// thisSelection.setAntennaExpr(MSSelection::nameExprStr( antNames));
	thisSelection.setAntennaExpr(antnames);
	os << "Selecting on antenna names" << LogIO::POST;
	
      }            
      if(timerng != ""){
	Vector<String>timerange(1, timerng);
	thisSelection.setTimeExpr(MSSelection::nameExprStr(timerange));
	os << "Selecting on time range" << LogIO::POST;	
      }
      //***************

      TableExprNode exprNode=thisSelection.toTableExprNode(&sorted);
      if(exprNode.isNull())
	throw(AipsError("Selection led to a null exprnode...review ms and selection parameters"));
      datafieldids_p.resize();
      datafieldids_p=thisSelection.getFieldList();
      if((numMS_p > 1) || datafieldids_p.nelements() > 1)
	multiFields_p= True;
      // Now remake the selected ms
      mssel_p = new MeasurementSet(sorted(exprNode));
      AlwaysAssert(mssel_p, AipsError);
      mssel_p->rename(msname+"/SELECTED_IMAGER_TABLE", Table::Scratch);
      if(mssel_p->nrow()==0) {
	delete mssel_p; mssel_p=0;
	//Ayeee...lets back out of this one
	--numMS_p;
	blockMSSel_p.resize(numMS_p, True);
	blockNChan_p.resize(numMS_p, True);
	blockStart_p.resize(numMS_p, True);
	blockStep_p.resize(numMS_p, True);
	blockSpw_p.resize(numMS_p, True);
	mssel_p=new MeasurementSet(sorted);	
	os << "Selection is empty: you may want to review this MSs selection"
	   << LogIO::EXCEPTION;
      }
      else {
	mssel_p->flush();
      }

      Int len = msSelect.length();
      Int nspace = msSelect.freq (' ');
      Bool nullSelect=(msSelect.empty() || nspace==len);
      if (!nullSelect) {
	MeasurementSet* mssel_p2;
	// Apply the TAQL selection string, to remake the selected MS
	String parseString="select from $1 where " + msSelect;
	mssel_p2=new MeasurementSet(tableCommand(parseString,*mssel_p));
	AlwaysAssert(mssel_p2, AipsError);
	// Rename the selected MS as */SELECTED_TABLE2
	mssel_p2->rename(msname+"/SELECTED_TABLE2", Table::Scratch); 
	if (mssel_p2->nrow()==0) {
	  os << LogIO::WARN
	     << "Selection string results in empty MS: "
	     << "reverting to sorted MeasurementSet"
	     << LogIO::POST;
	  delete mssel_p2;
	} else {
	  if (mssel_p) {
	    delete mssel_p; 
	    mssel_p=mssel_p2;
	    mssel_p->flush();
	  }
	}
      } else {
	os << "No selection string given" << LogIO::POST;
      }

      if(mssel_p->nrow()!=thisms.nrow()) {
	os << "By selection " << thisms.nrow() << " rows are reduced to "
	   << mssel_p->nrow() << LogIO::POST;
      }
      else {
	os << "Selection did not drop any rows" << LogIO::POST;
      }
      //  }

    blockMSSel_p[numMS_p-1]=*mssel_p;
    //lets make the visSet now
    Block<Matrix<Int> > noChanSel;
    noChanSel.resize(numMS_p);
    Block<Int> sort(0);
    if(vs_p) delete vs_p; vs_p=0;
    vs_p= new VisSet(blockMSSel_p, sort, noChanSel);
    selectDataChannel();
    dataSet_p=True;

    return dataSet_p;

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
    LogIO os(LogOrigin("imager", "setimage()"), logSink_p);  
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
      
      MSSpWindowColumns msSpW(ms_p->spectralWindow());
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
	       << dataStart_p[i] + 1 << " for spw " << spwid+1
	       << LogIO::POST;
	    return False;
	  }
	 
	  if(dataNchan_p[i]<=0) nch=blockNChan_p[numMS_p-1](i);
	  else nch = dataNchan_p[i];
	  Int end = Int(dataStart_p[i]) + Int(nch) * Int(dataStep_p[i]);
	  if(end < 1 || end > blockNChan_p[numMS_p-1](i)) {
	    os << LogIO::SEVERE << "Illegal step pixel = " << dataStep_p[i]
	       << " for spw " << spwid+1
	       << LogIO::POST;
	    return False;
	  }
	  os << "Selecting "<< nch
	     << " channels, starting at visibility channel "
	     << dataStart_p[i] + 1 << " stepped by "
	     << dataStep_p[i] << " for spw " << spwid+1 << LogIO::POST;
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

     vs_p->iter().selectChannel(blockGroup, blockStart_p, blockNChan_p, 
				blockStep_p, blockSpw_p);
    return True;
  }

  Bool ImagerMultiMS::openSubTables(){

    antab_p=Table(ms_p->antennaTableName(),
		  TableLock(TableLock::AutoNoReadLocking));
 datadesctab_p=Table(ms_p->dataDescriptionTableName(),
	       TableLock(TableLock::AutoNoReadLocking));
 feedtab_p=Table(ms_p->feedTableName(),
		 TableLock(TableLock::AutoNoReadLocking));
 fieldtab_p=Table(ms_p->fieldTableName(),
		  TableLock(TableLock::AutoNoReadLocking));
 obstab_p=Table(ms_p->observationTableName(),
		TableLock(TableLock::AutoNoReadLocking));
 poltab_p=Table(ms_p->polarizationTableName(),
		TableLock(TableLock::AutoNoReadLocking));
 proctab_p=Table(ms_p->processorTableName(),
		TableLock(TableLock::AutoNoReadLocking));
 spwtab_p=Table(ms_p->spectralWindowTableName(),
		TableLock(TableLock::AutoNoReadLocking));
 statetab_p=Table(ms_p->stateTableName(),
		TableLock(TableLock::AutoNoReadLocking));

 if(Table::isReadable(ms_p->dopplerTableName()))
   dopplertab_p=Table(ms_p->dopplerTableName(),
		      TableLock(TableLock::AutoNoReadLocking));

 if(Table::isReadable(ms_p->flagCmdTableName()))
   flagcmdtab_p=Table(ms_p->flagCmdTableName(),
		      TableLock(TableLock::AutoNoReadLocking));
 if(Table::isReadable(ms_p->freqOffsetTableName()))
   freqoffsettab_p=Table(ms_p->freqOffsetTableName(),
			 TableLock(TableLock::AutoNoReadLocking));

 if(!(Table::isReadable(ms_p->historyTableName()))){
   // setup a new table in case its not there
   TableRecord &kws = ms_p->rwKeywordSet();
   SetupNewTable historySetup(ms_p->historyTableName(),
			      MSHistory::requiredTableDesc(),Table::New);
   kws.defineTable(MS::keywordName(MS::HISTORY), Table(historySetup));
   
 }
 historytab_p=Table(ms_p->historyTableName(),
		    TableLock(TableLock::AutoNoReadLocking), Table::Update);
 if(Table::isReadable(ms_p->pointingTableName()))
   pointingtab_p=Table(ms_p->pointingTableName(), 
		       TableLock(TableLock::AutoNoReadLocking));

 if(Table::isReadable(ms_p->sourceTableName()))
   sourcetab_p=Table(ms_p->sourceTableName(),
		     TableLock(TableLock::AutoNoReadLocking));

 if(Table::isReadable(ms_p->sysCalTableName()))
 syscaltab_p=Table(ms_p->sysCalTableName(),
		   TableLock(TableLock::AutoNoReadLocking));
 if(Table::isReadable(ms_p->weatherTableName()))
   weathertab_p=Table(ms_p->weatherTableName(),
		      TableLock(TableLock::AutoNoReadLocking));

 hist_p= new MSHistoryHandler(*ms_p, "imager");

return True;

  
  }
} //# NAMESPACE CASA - END
