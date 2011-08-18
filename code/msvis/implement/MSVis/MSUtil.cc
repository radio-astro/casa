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
#include <ms/MeasurementSets/MSSpwIndex.h>
#include <msvis/MSVis/MSUtil.h>

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
    Vector<Int> ddId;
    Vector<Int> fldId;
    ROScalarColumn<Int> (ms,MS::columnName(MS::DATA_DESC_ID)).getColumn(ddId);
    ROScalarColumn<Int> (ms,MS::columnName(MS::FIELD_ID)).getColumn(fldId);
    ROMSFieldColumns fieldCol(ms.field());
    ROMSDataDescColumns ddCol(ms.dataDescription());
    ROMSSpWindowColumns spwCol(ms.spectralWindow());
    ROScalarMeasColumn<MEpoch> timeCol(ms, MS::columnName(MS::TIME));
    Vector<uInt>  uniqIndx;
    uInt nTimes=GenSortIndirect<Double>::sort (uniqIndx, t, Sort::Ascending, Sort::QuickSort|Sort::NoDuplicates);
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
    Int oldDD=ddId[0];
    //For now we will assume that the field is not moving very far from polynome 0
    MDirection dir =fieldCol.phaseDirMeas(fieldId);
    MFrequency::Types obsMFreqType= (MFrequency::Types) (spwCol.measFreqRef()(ddCol.spectralWindowId()(ddId[0])));
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
      if(fldId[uniqIndx[j]] ==fieldId){
	timeCol.get(uniqIndx[j], ep);
	if(oldDD != ddId[uniqIndx[j]]){
	  oldDD=ddId[uniqIndx[j]];
	  if(spwCol.measFreqRef()(ddCol.spectralWindowId()(ddId[uniqIndx[j]])) != obsMFreqType){
	    obsMFreqType= (MFrequency::Types) (spwCol.measFreqRef()(ddCol.spectralWindowId()(ddId[uniqIndx[j]])));
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


} //# NAMESPACE CASA - END
