//# Qimager.cc: Implementation of Qimager.h
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
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <synthesis/MeasurementEquations/Qimager.h>
#include <msvis/MSVis/VisSet.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

Qimager::Qimager(MeasurementSet& theMS)
{

  ms_p= &theMS;
 lockCounter_p=0;
}

Qimager::Qimager(const Qimager & other)
{
  operator=(other);
}

Qimager &Qimager::operator=(const Qimager & other)
{

  //Equating the table and ms parameters
  ms_p=other.ms_p;
  antab_p=other.antab_p;
  datadesctab_p=other.datadesctab_p;
  feedtab_p=other.feedtab_p;
  fieldtab_p=other.fieldtab_p;
  obstab_p=other.obstab_p;
  pointingtab_p=other.pointingtab_p;
  poltab_p=other.poltab_p;
  proctab_p=other.proctab_p;
  spwtab_p=other.spwtab_p;
  statetab_p=other.statetab_p;

  return *this;
}

Qimager::~Qimager()
{
}



Bool Qimager::openSubTables(){

 antab_p=Table(ms_p->antennaTableName(),
	       TableLock(TableLock::UserNoReadLocking));
 datadesctab_p=Table(ms_p->dataDescriptionTableName(),
	       TableLock(TableLock::UserNoReadLocking));
 feedtab_p=Table(ms_p->feedTableName(),
		 TableLock(TableLock::UserNoReadLocking));
 fieldtab_p=Table(ms_p->fieldTableName(),
		  TableLock(TableLock::UserNoReadLocking));
 obstab_p=Table(ms_p->observationTableName(),
		TableLock(TableLock::UserNoReadLocking));
 poltab_p=Table(ms_p->polarizationTableName(),
		TableLock(TableLock::UserNoReadLocking));
 proctab_p=Table(ms_p->processorTableName(),
		TableLock(TableLock::UserNoReadLocking));
 spwtab_p=Table(ms_p->spectralWindowTableName(),
		TableLock(TableLock::UserNoReadLocking));
 statetab_p=Table(ms_p->stateTableName(),
		TableLock(TableLock::UserNoReadLocking));

 if(Table::isReadable(ms_p->dopplerTableName()))
   dopplertab_p=Table(ms_p->dopplerTableName(),
		      TableLock(TableLock::UserNoReadLocking));

 if(Table::isReadable(ms_p->flagCmdTableName()))
   flagcmdtab_p=Table(ms_p->flagCmdTableName(),
		      TableLock(TableLock::UserNoReadLocking));
 if(Table::isReadable(ms_p->freqOffsetTableName()))
   freqoffsettab_p=Table(ms_p->freqOffsetTableName(),
			 TableLock(TableLock::UserNoReadLocking));

 if(Table::isReadable(ms_p->historyTableName()))
   historytab_p=Table(ms_p->historyTableName(),
		      TableLock(TableLock::UserNoReadLocking));
 if(Table::isReadable(ms_p->pointingTableName()))
   pointingtab_p=Table(ms_p->pointingTableName(), 
		       TableLock(TableLock::UserNoReadLocking));

 if(Table::isReadable(ms_p->sourceTableName()))
   sourcetab_p=Table(ms_p->sourceTableName(),
		     TableLock(TableLock::UserNoReadLocking));

 if(Table::isReadable(ms_p->sysCalTableName()))
 syscaltab_p=Table(ms_p->sysCalTableName(),
		   TableLock(TableLock::UserNoReadLocking));
 if(Table::isReadable(ms_p->weatherTableName()))
   weathertab_p=Table(ms_p->weatherTableName(),
		      TableLock(TableLock::UserNoReadLocking));
return True;

}

Bool Qimager::lock(){

  Bool ok; 
  ok=True;
  if(lockCounter_p == 0){

    ok= ok && (ms_p->lock());
    ok= ok && antab_p.lock(False);
    ok= ok && datadesctab_p.lock(False);
    ok= ok && feedtab_p.lock(False);
    ok= ok && fieldtab_p.lock(False);
    ok= ok && obstab_p.lock(False);
    ok= ok && poltab_p.lock(False);
    ok= ok && proctab_p.lock(False);
    ok= ok && spwtab_p.lock(False);
    ok= ok && statetab_p.lock(False);
    if(!dopplertab_p.isNull())
      ok= ok && dopplertab_p.lock(False);
    if(!flagcmdtab_p.isNull())
      ok= ok && flagcmdtab_p.lock(False);
    if(!freqoffsettab_p.isNull())
      ok= ok && freqoffsettab_p.lock(False);
    if(!historytab_p.isNull())
      ok= ok && historytab_p.lock(False);
    if(!pointingtab_p.isNull())
      ok= ok && pointingtab_p.lock(False);
    if(!sourcetab_p.isNull())
      ok= ok && sourcetab_p.lock(False);
    if(!syscaltab_p.isNull())
      ok= ok && syscaltab_p.lock(False);
    if(!weathertab_p.isNull())
      ok= ok && weathertab_p.lock(False);
 
  }
  ++lockCounter_p;

  return ok ; 
}

Bool Qimager::unlock(){

  if(lockCounter_p==1){
    ms_p->unlock();
    antab_p.unlock();
    datadesctab_p.unlock();
    feedtab_p.unlock();
    fieldtab_p.unlock();
    obstab_p.unlock();
    poltab_p.unlock();
    proctab_p.unlock();
    spwtab_p.unlock();
    statetab_p.unlock();
    if(!dopplertab_p.isNull())
      dopplertab_p.unlock();
    if(!flagcmdtab_p.isNull())
      flagcmdtab_p.unlock();
    if(!freqoffsettab_p.isNull())
    freqoffsettab_p.unlock();
    if(!historytab_p.isNull())
      historytab_p.unlock();
    if(!pointingtab_p.isNull())
      pointingtab_p.unlock();
    if(!sourcetab_p.isNull())
      sourcetab_p.unlock();
    if(!syscaltab_p.isNull())
      syscaltab_p.unlock();
    if(!weathertab_p.isNull())
      weathertab_p.unlock();
  }

  if(lockCounter_p > 0 )
    --lockCounter_p;
  return True ; 
}

Bool Qimager::selectDataChannel(VisSet& vs, Vector<Int>& spectralwindowids, 
			       String& dataMode, 
			       Vector<Int>& dataNchan, 
			       Vector<Int>& dataStart, Vector<Int>& dataStep,
			       MRadialVelocity& mDataStart, 
			       MRadialVelocity& mDataStep){



  LogIO os(LogOrigin("Qimager", "selectDataChannel()", WHERE));

  if(dataMode=="channel") {
      if (dataNchan.nelements() != spectralwindowids.nelements()){
	if(dataNchan.nelements()==1){
	  dataNchan.resize(spectralwindowids.nelements(), True);
	  for(uInt k=1; k < spectralwindowids.nelements(); ++k){
	    dataNchan[k]=dataNchan[0];
	  }
	}
	else{
	  os << LogIO::SEVERE 
	     << "Vector of nchan has to be of size 1 or be of the same shape as spw " 
	     << LogIO::POST;
	  return False; 
	}
      }
      if (dataStart.nelements() != spectralwindowids.nelements()){
	if(dataStart.nelements()==1){
	  dataStart.resize(spectralwindowids.nelements(), True);
	  for(uInt k=1; k < spectralwindowids.nelements(); ++k){
	    dataStart[k]=dataStart[0];
	  }
	}
	else{
	  os << LogIO::SEVERE 
	     << "Vector of start has to be of size 1 or be of the same shape as spw " 
	     << LogIO::POST;
	  return False; 
	}
      }
      if (dataStep.nelements() != spectralwindowids.nelements()){
	if(dataStep.nelements()==1){
	  dataStep.resize(spectralwindowids.nelements(), True);
	  for(uInt k=1; k < spectralwindowids.nelements(); ++k){
	    dataStep[k]=dataStep[0];
	  }
	}
	else{
	  os << LogIO::SEVERE 
	     << "Vector of step has to be of size 1 or be of the same shape as spw " 
	     << LogIO::POST;
	  return False; 
	}
      }

      if(spectralwindowids.nelements()>0) {
	Int nch=0;
	for(uInt i=0;i<spectralwindowids.nelements();i++) {
	  Int spwid=spectralwindowids(i);
	  if(dataStart[i]<0) {
	    os << LogIO::SEVERE << "Illegal start pixel = " 
	       << dataStart[i] + 1 << " for spw " << spwid+1
	       << LogIO::POST;
	    return False;
	  }
	 
	  if(dataNchan[i]==0) nch=vs.numberChan()(spwid);
	  else nch = dataNchan[i];
	  Int end = Int(dataStart[i]) + Int(nch) * Int(dataStep[i]);
	  if(end < 1 || end > vs.numberChan()(spwid)) {
	    os << LogIO::SEVERE << "Illegal step pixel = " << dataStep[i]
	       << " for spw " << spwid+1
	       << LogIO::POST;
	    return False;
	  }
	  os << "Selecting "<< nch
	     << " channels, starting at visibility channel "
	     << dataStart[i] + 1 << " stepped by "
	     << dataStep[i] << " for spw " << spwid+1 << LogIO::POST;
	  vs.iter().selectChannel(1, Int(dataStart[i]), Int(nch),
				     Int(dataStep[i]), spwid);
	  dataNchan[i]=nch;
	}
      }	else {
	if(dataNchan[0]==0) dataNchan[0]=vs.numberChan()(0);
	Int end = Int(dataStart[0]) + Int(dataNchan[0]) 
	  * Int(dataStep[0]);
	if(end < 1 || end > vs.numberChan()(0)) {
	  os << LogIO::SEVERE << "Illegal step pixel = " << dataStep[0]
	     << LogIO::POST;
	  return False;
	}
	os << "Selecting "<< dataNchan[0]
	   << " channels, starting at visibility channel "
	 << dataStart[0] + 1 << " stepped by "
	   << dataStep[0] << LogIO::POST;
      }
    }
    else if (dataMode=="velocity") {
      MVRadialVelocity mvStart(mDataStart.get("m/s"));
      MVRadialVelocity mvStep(mDataStep.get("m/s"));
      MRadialVelocity::Types
	vType((MRadialVelocity::Types)mDataStart.getRefPtr()->getType());
      os << "Selecting "<< dataNchan[0]
	 << " channels, starting at radio velocity " << mvStart
	 << " stepped by " << mvStep << ", reference frame is "
	 << MRadialVelocity::showType(vType) << LogIO::POST;
      vs.iter().selectVelocity(Int(dataNchan[0]), mvStart, mvStep,
				  vType, MDoppler::RADIO);
    }
    else if (dataMode=="opticalvelocity") {
      MVRadialVelocity mvStart(mDataStart.get("m/s"));
      MVRadialVelocity mvStep(mDataStep.get("m/s"));
      MRadialVelocity::Types
	vType((MRadialVelocity::Types)mDataStart.getRefPtr()->getType());
      os << "Selecting "<< dataNchan[0]
	 << " channels, starting at optical velocity " << mvStart
	 << " stepped by " << mvStep << ", reference frame is "
	 << MRadialVelocity::showType(vType) << LogIO::POST;
      vs.iter().selectVelocity(Int(dataNchan[0]), mvStart, mvStep,
				  vType, MDoppler::OPTICAL);
    }

  return True;

}

} //# NAMESPACE CASA - END

