//# MSUtil.cc: Some MS specific Utilities
//# Copyright (C) 2011
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$
#include <casa/Utilities/Sort.h>
#include <measures/Measures/MeasTable.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MSSel/MSSpwIndex.h>
#include <ms/MSSel/MSDataDescIndex.h>
#include <msvis/MSVis/MSUtil.h>
#include <casa/Arrays/ArrayMath.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  MSUtil::MSUtil(){};
  void MSUtil::getSpwInFreqRange(Vector<Int>& spw, Vector<Int>& start,
				  Vector<Int>& nchan,
				  const MeasurementSet& ms, 
				  const Double freqStart,
				  const Double freqEnd,
				  const Double freqStep,
			    const MFrequency::Types freqframe,
			    const Int fieldId)
  {
    spw.resize();
    start.resize();
    nchan.resize();
    Vector<Double> t;
    ROScalarColumn<Double> (ms,MS::columnName(MS::TIME)).getColumn(t);
    //Vector<Int> ddId;
    //Vector<Int> fldId;
    
    ROMSFieldColumns fieldCol(ms.field());
    ROMSDataDescColumns ddCol(ms.dataDescription());
    ROMSSpWindowColumns spwCol(ms.spectralWindow());
    ROScalarMeasColumn<MEpoch> timeCol(ms, MS::columnName(MS::TIME));
    Vector<uInt>  uniqIndx;
    uInt nTimes=GenSortIndirect<Double>::sort (uniqIndx, t, Sort::Ascending, Sort::QuickSort|Sort::NoDuplicates);

    t.resize(0);
    //ROScalarColumn<Int> (ms,MS::columnName(MS::DATA_DESC_ID)).getColumn(ddId);
    //ROScalarColumn<Int> (ms,MS::columnName(MS::FIELD_ID)).getColumn(fldId);
    ROScalarColumn<Int> ddId(ms,MS::columnName(MS::DATA_DESC_ID));
    ROScalarColumn<Int> fldId(ms,MS::columnName(MS::FIELD_ID));
    //now need to do the conversion to data frame from requested frame
    //Get the epoch mesasures of the first row
    MEpoch ep;
    timeCol.get(0, ep);
    String observatory;
    MPosition obsPos;
    /////observatory position
    ROMSColumns msc(ms);
    if (ms.observation().nrow() > 0) {
      observatory = msc.observation().telescopeName()(msc.observationId()(0));
    }
    if (observatory.length() == 0 || 
	!MeasTable::Observatory(obsPos,observatory)) {
      // unknown observatory, use first antenna
      obsPos=msc.antenna().positionMeas()(0);
    }
    //////
    Int oldDD=ddId(0);
    Int newDD=oldDD;
    //For now we will assume that the field is not moving very far from polynome 0
    MDirection dir =fieldCol.phaseDirMeas(fieldId);
    MFrequency::Types obsMFreqType= (MFrequency::Types) (spwCol.measFreqRef()(ddCol.spectralWindowId()(ddId(0))));
    //cout << "nTimes " << nTimes << endl;
    //cout << " obsframe " << obsMFreqType << " reqFrame " << freqframe << endl; 
    MeasFrame frame(ep, obsPos, dir);
    MFrequency::Convert toObs(freqframe,
                              MFrequency::Ref(obsMFreqType, frame));
    Double freqEndMax=freqEnd;
    Double freqStartMin=freqStart;
    if(freqframe != obsMFreqType){
      freqEndMax=0.0;
      freqStartMin=C::dbl_max;
    }
    for (uInt j=0; j< nTimes; ++j){
      if(fldId(uniqIndx[j]) ==fieldId){
	timeCol.get(uniqIndx[j], ep);
	newDD=ddId(uniqIndx[j]);
	if(oldDD != newDD){
	  oldDD=newDD;
	  if(spwCol.measFreqRef()(ddCol.spectralWindowId()(newDD)) != obsMFreqType){
	    obsMFreqType= (MFrequency::Types) (spwCol.measFreqRef()(ddCol.spectralWindowId()(newDD)));
	    toObs.setOut(MFrequency::Ref(obsMFreqType, frame));
	  }
	}
	if(obsMFreqType != freqframe){
	  frame.resetEpoch(ep);
	  Double freqTmp=toObs(Quantity(freqStart, "Hz")).get("Hz").getValue();
	  freqStartMin=(freqStartMin > freqTmp) ? freqTmp : freqStartMin;
	  freqTmp=toObs(Quantity(freqEnd, "Hz")).get("Hz").getValue();
	  freqEndMax=(freqEndMax < freqTmp) ? freqTmp : freqEndMax; 
	}
      }
    }

    //cout << "freqStartMin " << freqStartMin << " freqEndMax " << freqEndMax << endl;
    MSSpwIndex spwIn(ms.spectralWindow());
    spwIn.matchFrequencyRange(freqStartMin-0.5*freqStep, freqEndMax+0.5*freqStep, spw, start, nchan);


 
  }
  void MSUtil::getSpwInFreqRangeAllFields(Vector<Int>& outspw, Vector<Int>& outstart,
  			  Vector<Int>& outnchan,
  			  const MeasurementSet& ms,
  			  const Double freqStart,
  			  const Double freqEnd,
  			  const Double freqStep,
  			  const MFrequency::Types freqframe){
	  Vector<Int> fldId;
	  ROScalarColumn<Int> (ms, MS::columnName (MS::FIELD_ID)).getColumn (fldId);
	  const Int option = Sort::HeapSort | Sort::NoDuplicates;
	  const Sort::Order order = Sort::Ascending;

	  Int nfields = GenSort<Int>::sort (fldId, order, option);

	  fldId.resize(nfields, True);
	  outspw.resize();
	  outstart.resize();
	  outnchan.resize();
	  for (Int k=0; k < nfields; ++k){
		  Vector<Int> locspw, locstart, locnchan;
		  MSUtil::getSpwInFreqRange(locspw, locstart, locnchan, ms, freqStart, freqEnd, freqStep, freqframe, fldId[k]);
		  for (Int j=0; j< locspw.shape()(0); ++j ){
			  Bool hasthisspw=False;
			  for (Int i=0; i< outspw.shape()(0); ++i){
				  if(outspw[i]==locspw[j]){
					  hasthisspw=True;
					  if(locstart[j] < outstart[i]){
						  Int endchan=outstart[i]+outnchan[i]-1;
						  outstart[i]=locstart[j];
						  if(endchan < (locstart[j]+ locnchan[j]-1))
							  endchan=locstart[j]+ locnchan[j]-1;
						  outnchan[i]=endchan+outstart[i]+1;
					  }
				  }
			  }
			  if(!hasthisspw){
			    uInt nout=outspw.nelements();
				  outspw.resize(nout+1, True);
				  outnchan.resize(nout+1, True);
				  outstart.resize(nout+1, True);
				  outspw[nout]=locspw[j];
				  outstart[nout]=locstart[j];
				  outnchan[nout]=locnchan[j];
				 


			  }
		  }

	  }


  }
  void MSUtil::getFreqRangeInSpw( Double& freqStart,
				  Double& freqEnd, const Vector<Int>& spw, const Vector<Int>& start,
				  const Vector<Int>& nchan,
				  const MeasurementSet& ms, 
				  const MFrequency::Types freqframe,
				  const Int fieldId){
    

    freqStart=C::dbl_max;
    freqEnd=0.0;
    Vector<Double> t;
    ROScalarColumn<Double> (ms,MS::columnName(MS::TIME)).getColumn(t);
    Vector<Int> ddId;
    Vector<Int> fldId;
    ROScalarColumn<Int> (ms,MS::columnName(MS::DATA_DESC_ID)).getColumn(ddId);
    ROScalarColumn<Int> (ms,MS::columnName(MS::FIELD_ID)).getColumn(fldId);
    ROMSFieldColumns fieldCol(ms.field());
    ROMSDataDescColumns ddCol(ms.dataDescription());
    ROMSSpWindowColumns spwCol(ms.spectralWindow());
    ROScalarMeasColumn<MEpoch> timeCol(ms, MS::columnName(MS::TIME));
    Vector<Double> ddIdD(ddId.shape());
    convertArray(ddIdD, ddId);
    ddIdD+= 1.0; //no zero id
    //we have to do this as one can have multiple dd for the same time. 
    t*=ddIdD;
    //t(fldId != fieldId)=-1.0;
    Vector<Double> elt;
    Vector<Int> elindx;
    //rejecting the large blocks of same time for all baselines
    //this speeds up by a lot GenSort::sort
    rejectConsecutive(t, elt, elindx);
    Vector<uInt>  uniqIndx;
    
    uInt nTimes=GenSortIndirect<Double>::sort (uniqIndx, elt, Sort::Ascending, Sort::QuickSort|Sort::NoDuplicates);
    MDirection dir =fieldCol.phaseDirMeas(fieldId);
    MSDataDescIndex mddin(ms.dataDescription());
    MFrequency::Types obsMFreqType= (MFrequency::Types) (spwCol.measFreqRef()(0));
    MEpoch ep;
    timeCol.get(0, ep);
    String observatory;
    MPosition obsPos;
    /////observatory position
    ROMSColumns msc(ms);
    if (ms.observation().nrow() > 0) {
      observatory = msc.observation().telescopeName()(msc.observationId()(0));
    }
    if (observatory.length() == 0 || 
	!MeasTable::Observatory(obsPos,observatory)) {
      // unknown observatory, use first antenna
      obsPos=msc.antenna().positionMeas()(0);
    }
    //////
    MeasFrame frame(ep, obsPos, dir);
    
						
    for (uInt ispw =0 ; ispw < spw.nelements() ; ++ispw){
      Double freqStartObs=C::dbl_max;
      Double freqEndObs=0.0;
      Vector<Double> chanfreq=spwCol.chanFreq()(spw[ispw]);
      Vector<Double> chanwid=spwCol.chanWidth()(spw[ispw]);
      Vector<Int> ddOfSpw=mddin.matchSpwId(spw[ispw]);
      for (Int ichan=start[ispw]; ichan<start[ispw]+nchan[ispw]; ++ichan){ 
	if(freqStartObs > (chanfreq[ichan]-fabs(chanwid[ichan])/2.0)) freqStartObs=chanfreq[ichan]-fabs(chanwid[ichan])/2.0;
	if(freqEndObs < (chanfreq[ichan]+fabs(chanwid[ichan])/2.0)) freqEndObs=chanfreq[ichan]+fabs(chanwid[ichan])/2.0;    
      }
      obsMFreqType= (MFrequency::Types) (spwCol.measFreqRef()(spw[ispw]));
      if((obsMFreqType==MFrequency::REST) || (obsMFreqType==freqframe && obsMFreqType != MFrequency::TOPO)){
	if(freqStart > freqStartObs)  freqStart=freqStartObs;
	if(freqEnd < freqStartObs)  freqEnd=freqStartObs;
	if(freqStart > freqEndObs)  freqStart=freqEndObs;
	if(freqEnd < freqEndObs)  freqEnd=freqEndObs;
      }
      else{
	MFrequency::Convert toframe(obsMFreqType,
				    MFrequency::Ref(freqframe, frame));
	for (uInt j=0; j< nTimes; ++j){
	  if((fldId[elindx[uniqIndx[j]]] ==fieldId) && anyEQ(ddOfSpw, ddId[elindx[uniqIndx[j]]])){
	    timeCol.get(elindx[uniqIndx[j]], ep);
	    frame.resetEpoch(ep);
	    Double freqTmp=toframe(Quantity(freqStartObs, "Hz")).get("Hz").getValue();
	    if(freqStart > freqTmp)  freqStart=freqTmp;
	    if(freqEnd < freqTmp)  freqEnd=freqTmp;
	    freqTmp=toframe(Quantity(freqEndObs, "Hz")).get("Hz").getValue();
	    if(freqStart > freqTmp)  freqStart=freqTmp;
	    if(freqEnd < freqTmp)  freqEnd=freqTmp;
	  }
	}
      }
    }
  }
  void MSUtil::rejectConsecutive(const Vector<Double>& t, Vector<Double>& retval, Vector<Int>& indx){
    uInt n=t.nelements();
    if(n >0){
      retval.resize(n);
      indx.resize(n);
      retval[0]=t[0];
      indx[0]=0;
    }
    else
      return;
    Int prev=0;
    for (uInt k=1; k < n; ++k){ 
      if(t[k] != retval(prev)){
	++prev;
	//retval.resize(prev+1, True);
	retval[prev]=t[k];
	//indx.resize(prev+1, True);
	indx[prev]=k;
      }
    }
    retval.resize(prev+1, True);
    indx.resize(prev+1, True);
    
  }

  Vector<String> MSUtil::getSpectralFrames(Vector<MFrequency::Types>& types, const MeasurementSet& ms)
  {
	  Vector<String> retval;
	  Vector<Int> typesAsInt=ROMSSpWindowColumns(ms.spectralWindow()).measFreqRef().getColumn();
	  if(ms.nrow()==Table(ms.getPartNames()).nrow()){
		  types.resize(typesAsInt.nelements());
		  for (uInt k=0; k < types.nelements(); ++k)
			  types[k]=MFrequency::castType(typesAsInt[k]);
  	  }
	  else{
		  Vector<Int> ddId;
		  ROScalarColumn<Int> (ms,MS::columnName(MS::DATA_DESC_ID)).getColumn(ddId);
		  Vector<uInt>  uniqIndx;
		  uInt nTimes=GenSort<Int>::sort (ddId, Sort::Ascending, Sort::QuickSort|Sort::NoDuplicates);
		  ddId.resize(nTimes, True);
		  Vector<Int> spwids(nTimes);
		  Vector<Int> spwInDD=ROMSDataDescColumns(ms.dataDescription()).spectralWindowId().getColumn();
		  for (uInt k=0; k < nTimes; ++k)
			  spwids[k]=spwInDD[ddId[k]];

		  nTimes=GenSort<Int>::sort (spwids, Sort::Ascending, Sort::QuickSort|Sort::NoDuplicates);
		  spwids.resize(nTimes, True);
		  types.resize(nTimes);
		  for(uInt k=0; k <nTimes; ++k)
			  types[k]=MFrequency::castType(typesAsInt[spwids[k]]);
	  }

	  retval.resize(types.nelements());
	  for (uInt k=0; k < types.nelements(); ++k){
		retval[k]=MFrequency::showType(types[k]);
	  }


	  return retval;

  }


} //# NAMESPACE CASA - END
