//# VisibilityIterator.cc: Step through MeasurementEquation by visibility
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
//# $Id: VisibilityIterator.cc,v 19.15 2006/02/01 01:25:14 kgolap Exp $

#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSpwIndex.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColDescSet.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TiledStManAccessor.h>
#include <tables/Tables/StandardStManAccessor.h>
#include <tables/Tables/IncrStManAccessor.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Sort.h>
#include <cassert>

namespace casa { //# NAMESPACE CASA - BEGIN

ROVisibilityIterator::ROVisibilityIterator() 
 : selRows_p(0, 0) 
{}

// const of MS is cast away, but we don't actually change it.
// (just to share code between RO version and RW version of iterator)
ROVisibilityIterator::ROVisibilityIterator(const MeasurementSet &ms,
					   const Block<Int>& sortColumns,
					   Double timeInterval)
    : msIter_p(ms,sortColumns,timeInterval),selRows_p(0, 0),
curChanGroup_p(0),nChan_p(0),nRowBlocking_p(0),initialized_p(False),
msIterAtOrigin_p(False),stateOk_p(False),freqCacheOK_p(False),
floatDataFound_p(False),
      msHasWtSp_p(False),
      lastfeedpaUT_p(0),lastazelUT_p(0),velSelection_p(False)
{
  initsinglems(ms);
}


void ROVisibilityIterator::initsinglems(const MeasurementSet &ms){

  //  cout << "addDefaultSortColumns = False!" << endl;
  asyncEnabled_p = False;
  This = (ROVisibilityIterator*)this;
  isMultiMS_p=False;
  msCounter_p=0;
  Block<Vector<Int> > blockNGroup(1);
  Block<Vector<Int> > blockStart(1);
  Block<Vector<Int> > blockWidth(1);
  Block<Vector<Int> > blockIncr(1);
  Block<Vector<Int> > blockSpw(1);
  Int nspw=msIter_p.msColumns().spectralWindow().nrow();
  blockNGroup[0].resize(nspw);
  blockNGroup[0].set(1);
  blockStart[0].resize(nspw);
  blockStart[0].set(0);  
  blockWidth[0].resize(nspw);
  blockWidth[0]=msIter_p.msColumns().spectralWindow().numChan().getColumn(); 
  blockIncr[0].resize(nspw);
  blockIncr[0].set(1);
  blockSpw[0].resize(nspw); 
  indgen(blockSpw[0]);
  selectChannel(blockNGroup, blockStart, blockWidth, blockIncr,
		blockSpw);
  measurementSets_p.clear();
  measurementSets_p.push_back (& ms);
    

}
ROVisibilityIterator::ROVisibilityIterator(const MeasurementSet &ms,
					   const Block<Int>& sortColumns,
                                           const Bool addDefaultSort,
					   Double timeInterval)
 :msIter_p(ms,sortColumns,timeInterval, addDefaultSort),selRows_p(0, 0),
curChanGroup_p(0),nChan_p(0),nRowBlocking_p(0),initialized_p(False),
msIterAtOrigin_p(False),stateOk_p(False),freqCacheOK_p(False),
  floatDataFound_p(False),
  msHasWtSp_p(False),
  lastfeedpaUT_p(0),lastazelUT_p(0),velSelection_p(False)
{
  initsinglems(ms);
}


ROVisibilityIterator::ROVisibilityIterator(const Block<MeasurementSet> &mss,
					   const Block<Int>& sortColumns,
					   Double timeInterval)
: msIter_p(mss,sortColumns,timeInterval),selRows_p(0, 0),
curChanGroup_p(0),nChan_p(0),nRowBlocking_p(0),initialized_p(False),
msIterAtOrigin_p(False),stateOk_p(False),freqCacheOK_p(False),
floatDataFound_p(False),
  msHasWtSp_p(False),
  lastfeedpaUT_p(0),lastazelUT_p(0),velSelection_p(False)
{
  initmultims(mss);
}

ROVisibilityIterator::ROVisibilityIterator(const Block<MeasurementSet> &mss,
					   const Block<Int>& sortColumns, const Bool addDefaultSort,
					   Double timeInterval)
  : msIter_p(mss,sortColumns,timeInterval,addDefaultSort),selRows_p(0, 0),
curChanGroup_p(0),nChan_p(0),nRowBlocking_p(0),initialized_p(False),
msIterAtOrigin_p(False),stateOk_p(False),freqCacheOK_p(False),
    floatDataFound_p(False),
    msHasWtSp_p(False),
    lastfeedpaUT_p(0),lastazelUT_p(0),velSelection_p(False)
{
  initmultims(mss);
}

void ROVisibilityIterator::initmultims(const Block<MeasurementSet> &mss){

  asyncEnabled_p = False;
  This = (ROVisibilityIterator*)this; 
  msCounter_p=0;
  isMultiMS_p=True;
  Int numMS=mss.nelements();
  Block<Vector<Int> > blockNGroup(numMS);
  Block<Vector<Int> > blockStart(numMS);
  Block<Vector<Int> > blockWidth(numMS);
  Block<Vector<Int> > blockIncr(numMS);
  Block<Vector<Int> > blockSpw(numMS);
  measurementSets_p.clear();

  for (Int k=0; k < numMS; ++k){
    ROMSSpWindowColumns msSpW(mss[k].spectralWindow());
    Int nspw=msSpW.nrow();
    blockNGroup[k].resize(nspw);
    blockNGroup[k].set(1);
    blockStart[k].resize(nspw);
    blockStart[k].set(0);  
    blockWidth[k].resize(nspw);
    blockWidth[k]=msSpW.numChan().getColumn(); 
    blockIncr[k].resize(nspw);
    blockIncr[k].set(1);
    blockSpw[k].resize(nspw); 
    indgen(blockSpw[k]);
    measurementSets_p.push_back (& mss [k]);
  }
  selectChannel(blockNGroup, blockStart, blockWidth, blockIncr,
		blockSpw);

}


ROVisibilityIterator::ROVisibilityIterator(const ROVisibilityIterator& other)
    : selRows_p(other.selRows_p)
{
    operator=(other);
}

ROVisibilityIterator::~ROVisibilityIterator() 
{
}

ROVisibilityIterator& 
ROVisibilityIterator::operator=(const ROVisibilityIterator& other) 
{
  if (this==&other) return *this;
  This=(ROVisibilityIterator*)this;

  azel0_p=other.azel0_p;
  azel_p.assign(other.azel_p);
  blockChanInc_p = other.blockChanInc_p;
  blockChanStart_p = other.blockChanStart_p;
  blockChanWidth_p = other.blockChanWidth_p;
  blockNumChanGroup_p = other.blockNumChanGroup_p;
  blockSpw_p = other.blockSpw_p;
  cFromBETA_p=other.cFromBETA_p;
  chanInc_p=other.chanInc_p;
  chanStart_p=other.chanStart_p;
  chanWidth_p=other.chanWidth_p;
  channelGroupSize_p=other.channelGroupSize_p;
  curChanGroup_p=other.curChanGroup_p;
  curEndRow_p=other.curEndRow_p;
  curNumChanGroup_p=other.curNumChanGroup_p;
  curNumRow_p=other.curNumRow_p;
  curStartRow_p=other.curStartRow_p;
  curTableNumRow_p=other.curTableNumRow_p;
  feedpa_p.assign(other.feedpa_p);
  flagCube_p.assign(other.flagCube_p);
  flagOK_p = other.flagOK_p;
  floatDataFound_p=other.floatDataFound_p;
  floatDataCubeOK_p = other.floatDataCubeOK_p;
  floatDataCube_p.assign (other.floatDataCube_p);
  freqCacheOK_p=other.freqCacheOK_p;
  frequency_p.assign (frequency_p);
  hourang_p=other.hourang_p;
  initialized_p=other.initialized_p;
  isMultiMS_p=other.isMultiMS_p;
  lastazelUT_p=other.lastazelUT_p;
  lastfeedpaUT_p=other.lastfeedpaUT_p;
  lsrFreq_p.assign (other.lsrFreq_p);
  measurementSets_p = other.measurementSets_p;
  more_p=other.more_p;
  msCounter_p=other.msCounter_p;
  msIterAtOrigin_p=other.msIterAtOrigin_p;
  msIter_p=other.msIter_p;
  msd_p=other.msd_p;
  nAnt_p=other.nAnt_p;
  nChan_p=other.nChan_p;
  nPol_p=other.nPol_p;
  nRowBlocking_p=other.nRowBlocking_p;
  nVelChan_p=other.nVelChan_p;
  newChanGroup_p=other.newChanGroup_p;
  numChanGroup_p=other.numChanGroup_p;
  parang0_p=other.parang0_p;
  parang_p.assign (other.parang_p);
  preselectedChanStart_p=other.preselectedChanStart_p;
  preselectednChan_p=other.preselectednChan_p;
  selFreq_p.assign(other.selFreq_p);
  selRows_p=other.selRows_p;
  slicer_p=other.slicer_p;
  stateOk_p=other.stateOk_p;
  timeInterval_p.assign (other.timeInterval_p);
  time_p.assign(other.time_p);
  useSlicer_p=other.useSlicer_p;
  uvwMat_p.assign(other.uvwMat_p);
  vDef_p=other.vDef_p;
  vInc_p=other.vInc_p;
  vPrecise_p=other.vPrecise_p;
  vStart_p=other.vStart_p;
  velSelection_p = other.velSelection_p;
  visCube_p.assign(other.visCube_p);
  visOK_p = other.visOK_p;
  weightSlicer_p=other.weightSlicer_p;
  msHasWtSp_p = other.msHasWtSp_p;
  weightSpOK_p = other.weightSpOK_p;

  // Column access functions

  colAntenna1.reference(other.colAntenna1);
  colAntenna2.reference(other.colAntenna2);
  colCorrVis.reference(other.colCorrVis);
  colExposure.reference(other.colExposure);
  colFeed1.reference(other.colFeed1);
  colFeed2.reference(other.colFeed2);
  colFlag.reference(other.colFlag);
  colFlagCategory.reference(other.colFlagCategory);
  colFlagRow.reference(other.colFlagRow);
  colFloatVis.reference(other.colFloatVis);
  colModelVis.reference(other.colModelVis);
  colObservation.reference(other.colObservation);
  colProcessor.reference(other.colProcessor);
  colScan.reference(other.colScan);
  colSigma.reference(other.colSigma);
  colState.reference(other.colState);
  colTime.reference(other.colTime);
  colTimeCentroid.reference(other.colTimeCentroid);
  colTimeInterval.reference(other.colTimeInterval);
  colUVW.reference(other.colUVW);
  colVis.reference(other.colVis);
  colWeight.reference(other.colWeight);
  colWeightSpectrum.reference(other.colWeightSpectrum);

  imwgt_p=other.imwgt_p;

  return *this;
}

void
ROVisibilityIterator::setAsyncEnabled (Bool enabled)
{
    asyncEnabled_p = enabled;
}

Bool
ROVisibilityIterator::isAsyncEnabled () const
{
    return asyncEnabled_p;
}


void ROVisibilityIterator::setRowBlocking(Int nRow)
{
  nRowBlocking_p=nRow;
}

void ROVisibilityIterator::useImagingWeight(const VisImagingWeight& imWgt){
    imwgt_p=imWgt;
}
void ROVisibilityIterator::origin()
{
  if (!initialized_p) {
    originChunks();
  } else {
    curChanGroup_p=0;
    newChanGroup_p=True;
    curStartRow_p=0;
    freqCacheOK_p=False;
    flagOK_p = weightSpOK_p = False;
    visOK_p.resize(3); visOK_p[0]=visOK_p[1]=visOK_p[2]=False;
    floatDataCubeOK_p = False;
    setSelTable();
    attachColumns(attachTable());
    getTopoFreqs();
    updateSlicer();
    more_p=curChanGroup_p<curNumChanGroup_p;
    // invalidate any attached VisBuffer
    if (!vbStack_p.empty()) vbStack_p.top()->invalidate();
  }
}

void
ROVisibilityIterator::originChunks()
{
    originChunks (False);
}


void
ROVisibilityIterator::originChunks(Bool forceRewind)
{
    initialized_p=True;
    
    if (forceRewind){
        msIterAtOrigin_p = False;
    }
    
    if (!msIterAtOrigin_p) {

        msIter_p.origin();
        msIterAtOrigin_p=True;

        while ((! isInSelectedSPW (msIter_p.spectralWindowId())) &&
                msIter_p.more()){

            msIter_p++;
        }

        stateOk_p=False;
        msCounter_p=msId();

    }
    
    setState();
    origin();
    setTileCache();
}

Bool ROVisibilityIterator::isInSelectedSPW(const Int& spw){
  
  for (uInt k=0; k < blockSpw_p[msId()].nelements() ; ++k){
    if(spw==blockSpw_p[msId()][k])
      return True;
  }
  return False;
}

void ROVisibilityIterator::advance()
{
  newChanGroup_p=False;
  flagOK_p = False;
  visOK_p[0] = False;
  visOK_p[1] = False;
  visOK_p[2] = False;
  floatDataCubeOK_p = False;
  weightSpOK_p = False;
  curStartRow_p=curEndRow_p+1;
  if (curStartRow_p>=curTableNumRow_p) {
    if (++curChanGroup_p >= curNumChanGroup_p) {
      curChanGroup_p--;
      more_p=False;
    } else {
      curStartRow_p=0;
      newChanGroup_p=True;
      freqCacheOK_p=False;
      updateSlicer();
    }
  }
  if (more_p) {
    setSelTable();
    getTopoFreqs();
    // invalidate any attached VisBuffer
    if (!vbStack_p.empty()) vbStack_p.top()->invalidate();
  }
}

ROVisibilityIterator& ROVisibilityIterator::nextChunk()
{

  if (msIter_p.more()) {
    msIter_p++;
    if((!isInSelectedSPW(msIter_p.spectralWindowId()))){
      while( (!isInSelectedSPW(msIter_p.spectralWindowId()))
	     && (msIter_p.more()))
	msIter_p++;
      stateOk_p=False;
    }
      
    if(msIter_p.newMS()){
      msCounter_p=msId();
      doChannelSelection();
    }
    msIterAtOrigin_p=False;
    stateOk_p=False;
  }
  if (msIter_p.more()) {
    setState();
    getTopoFreqs();
    if (!vbStack_p.empty()) vbStack_p.top()->invalidate();
  }
  more_p=msIter_p.more();
  return *this;
}

void ROVisibilityIterator::setSelTable()
{
  // work out how many rows to return 
  // for the moment we return all rows with the same value for time
  // unless row blocking is set, in which case we return more rows at once.
  if (nRowBlocking_p>0) {
    curEndRow_p = curStartRow_p + nRowBlocking_p;
    if (curEndRow_p >= curTableNumRow_p) curEndRow_p=curTableNumRow_p-1;
  } else {
    for (curEndRow_p=curStartRow_p+1; curEndRow_p<curTableNumRow_p && 
    	   time_p(curEndRow_p)==time_p(curEndRow_p-1); 
	 curEndRow_p++);
    curEndRow_p--;
  }

  curNumRow_p=curEndRow_p-curStartRow_p+1;
  selRows_p = RefRows(curStartRow_p, curEndRow_p);
  rowIds_p.resize(0);
}

void
ROVisibilityIterator::getTopoFreqs()
{
    if (velSelection_p) {

        // Convert selected velocities to TOPO frequencies.
        // First convert observatory vel to correct frame (for this time).

        msd_p.setEpoch(msIter_p.msColumns().timeMeas()(curStartRow_p));
        if (msIter_p.newMS())
            msd_p.setObservatoryPosition(msIter_p.telescopePosition());

        MRadialVelocity obsRV = msd_p.obsVel(); // get obs velocity in required frame

        Double obsVel=cFromBETA_p(obsRV.toDoppler()).getValue().get().getValue();
            // convert to doppler in required definition and get out in m/s

        // Now compute corresponding TOPO freqs

        selFreq_p.resize(nVelChan_p);
        lsrFreq_p.resize(nVelChan_p);
        Double v0 = vStart_p.getValue(), dv=vInc_p.getValue();

        if (aips_debug) cout << "obsVel="<<obsVel<<endl;

        for (Int i=0; i<nVelChan_p; i++) {

            Double vTopo = v0 + i*dv - obsVel;
            MDoppler dTopo(Quantity(vTopo,"m/s"), vDef_p);
            selFreq_p(i) = MFrequency::fromDoppler
                    (dTopo,msIter_p.restFrequency().getValue()).getValue().getValue();

            // also calculate the frequencies in the requested frame for matching
            // up with the image planes
            // (they are called lsr here, but don't need to be in that frame)

            MDoppler dLSR (Quantity(v0+i*dv,"m/s"), vDef_p);
            const MFrequency& restFrequency = msIter_p.restFrequency();
            lsrFreq_p(i) = MFrequency::fromDoppler (dLSR,restFrequency.getValue()).getValue().getValue();
        }
    }
}

void
ROVisibilityIterator::getTopoFreqs(Vector<Double> & lsrFreq, Vector<Double> & selFreq)
{
    getTopoFreqs ();
    lsrFreq.assign (lsrFreq_p);
    selFreq.assign (selFreq_p);
}

void ROVisibilityIterator::setState()
{
  if (stateOk_p) return;

  curTableNumRow_p = msIter_p.table().nrow();
  // get the times for this (major) iteration, so we can do (minor) 
  // iteration by constant time (needed for VisBuffer averaging).
  ROScalarColumn<Double> lcolTime(msIter_p.table(),MS::columnName(MS::TIME));
  time_p.resize(curTableNumRow_p); 
  lcolTime.getColumn(time_p);
  ROScalarColumn<Double> lcolTimeInterval(msIter_p.table(),
					  MS::columnName(MS::INTERVAL));
  timeInterval_p.resize(curTableNumRow_p); 
  lcolTimeInterval.getColumn(timeInterval_p);
  curStartRow_p=0;
  setSelTable();
  attachColumns(attachTable());
  // If this is a new MeasurementSet then set up the antenna locations
  if (msIter_p.newMS()) {
    This->nAnt_p = msd_p.setAntennas(msIter_p.msColumns().antenna());
    This->feedpa_p.resize(nAnt_p);
    This->feedpa_p.set(0);
    This->parang_p.resize(nAnt_p);
    This->parang_p.set(0);
    This->azel_p.resize(nAnt_p);

  }	
  if (msIter_p.newField() || msIterAtOrigin_p) { 
    msd_p.setFieldCenter(msIter_p.phaseCenter());
  }
  if ( msIter_p.newDataDescriptionId() || msIterAtOrigin_p) {
    Int spw=msIter_p.spectralWindowId();
    nChan_p=msColumns().spectralWindow().numChan()(spw);
    nPol_p=msColumns().polarization().numCorr()(msIter_p.polarizationId());

    if (Int(numChanGroup_p.nelements())<= spw || 
	numChanGroup_p[spw] == 0) {
      // no selection set yet, set default = all
      // for a reference MS this will normally be set appropriately in VisSet
      selectChannel(1,msIter_p.startChan(),nChan_p);
    }
    channelGroupSize_p=chanWidth_p[spw];
    curNumChanGroup_p=numChanGroup_p[spw];
    freqCacheOK_p=False;
  }

  stateOk_p=True;
}

void ROVisibilityIterator::updateSlicer()
{
  
  if(msIter_p.newMS()){
    numChanGroup_p.resize(0, True, False);
    doChannelSelection();
  }
  
  // set the Slicer to get the selected part of spectrum out of the table
  Int spw=msIter_p.spectralWindowId();
  //Fixed what i think was a confusion between chanWidth and chanInc
  // 2007/11/12
  Int start=chanStart_p[spw]+curChanGroup_p*chanWidth_p[spw];
  start-=msIter_p.startChan();
  AlwaysAssert(start>=0 && start+channelGroupSize_p<=nChan_p,AipsError);
  //  slicer_p=Slicer(Slice(),Slice(start,channelGroupSize_p));
  // above is slow, use IPositions instead.
  slicer_p=Slicer(IPosition(2,0,start),
		  IPosition(2,nPol_p,channelGroupSize_p), 
		  IPosition(2,1, (chanInc_p[spw]<=0)? 1 : chanInc_p[spw] ));
  weightSlicer_p=Slicer(IPosition(1,start),IPosition(1,channelGroupSize_p), 
			IPosition(1,(chanInc_p[spw]<=0)? 1 : chanInc_p[spw]));
  useSlicer_p=channelGroupSize_p<nChan_p;

  //if(msIter_p.newDataDescriptionId()){
  setTileCache();
    //}
}



void ROVisibilityIterator::setTileCache(){
  // This function sets the tile cache because of a feature in 
  // sliced data access that grows memory dramatically in some cases
  //  if(useSlicer_p){
  {
    if(! (msIter_p.newDataDescriptionId() || msIter_p.newMS()) )
      return;

    const MeasurementSet& thems=msIter_p.ms();
    if(thems.tableType() == Table::Memory)
      return;
    const ColumnDescSet& cds=thems.tableDesc().columnDescSet();

    // Get the first row number for this DDID.
    Vector<uInt> rownums;
    rowIds(rownums);
    uInt startrow = rownums[0];

    Vector<String> columns(8);
    // complex
    columns(0)=MS::columnName(MS::DATA);
    columns(1)=MS::columnName(MS::CORRECTED_DATA);
    columns(2)=MS::columnName(MS::MODEL_DATA);
    // boolean
    columns(3)=MS::columnName(MS::FLAG);
    // float
    columns(4)=MS::columnName(MS::WEIGHT_SPECTRUM);
    columns(5)=MS::columnName(MS::WEIGHT);
    columns(6)=MS::columnName(MS::SIGMA);
    // double
    columns(7)=MS::columnName(MS::UVW);
    //
for (uInt k=0; k< columns.nelements(); ++k){
      if (cds.isDefined(columns(k))) {
	const ColumnDesc& cdesc=cds[columns(k)];
	String dataManType="";
	dataManType=cdesc.dataManagerType();
	//have to do something special about weight_spectrum as it tend to exist but 
	//has no valid data
	if(columns[k]==MS::columnName(MS::WEIGHT_SPECTRUM))
	  if(!existsWeightSpectrum())
	    dataManType="";
     
	// Sometimes  columns may not contain anything yet
	
	if((columns[k]==MS::columnName(MS::DATA) && (colVis.isNull() ||
						     !colVis.isDefined(0))) || 
	   (columns[k]==MS::columnName(MS::MODEL_DATA) && (colModelVis.isNull() ||
							   !colModelVis.isDefined(0))) ||
	   (columns[k]==MS::columnName(MS::CORRECTED_DATA) && (colCorrVis.isNull() ||
							       !colCorrVis.isDefined(0))) ||
	   (columns[k]==MS::columnName(MS::FLAG) && (colFlag.isNull() ||
						     !colFlag.isDefined(0))) ||
	   (columns[k]==MS::columnName(MS::WEIGHT) && (colWeight.isNull() ||
						       !colWeight.isDefined(0))) ||
	   (columns[k]==MS::columnName(MS::SIGMA) && (colSigma.isNull() ||
						      !colSigma.isDefined(0))) ||
	   (columns[k]==MS::columnName(MS::UVW) && (colUVW.isNull() ||
						    !colUVW.isDefined(0))) ){
	    dataManType="";
	}
	
   
	
	if(dataManType.contains("Tiled")  && (!String(cdesc.dataManagerGroup()).empty())){
	  try {
	    
	    ROTiledStManAccessor tacc=ROTiledStManAccessor(thems, 
							   cdesc.dataManagerGroup());
	    

	    /*
	    cerr << "Data man type " << cdesc.dataManagerType() << "  nhyper " <<
	      tacc.nhypercubes() << " colname " << columns[k] << endl;

	    for (uInt jj=0 ; jj <  tacc.nhypercubes(); ++jj){
	      cerr << " bucket sizes " << tacc.getBucketSize(jj) << " cacheSize  " << tacc.getCacheSize(jj) << " hypercubeShape "<<
		tacc.getHypercubeShape(jj) << endl;
	    }
	    */
	    //One tile only for now ...seems to work faster
	    tacc.clearCaches();
	    Bool setCache=True;
	    for (uInt jj=0 ; jj <  tacc.nhypercubes(); ++jj){
	      if (tacc.getBucketSize(jj)==0){
	    	setCache=False;
	      }
	    }
	    if(useSlicer_p)
	      setCache=True;
	    ///If some bucketSize is 0...there is trouble in setting cache
	    ///but if slicer is used it gushes anyways if one does not set cache
	    ///need to fix the 0 bucket size in the filler anyways...then this is not needed
	    if(setCache){
	      if(tacc.nhypercubes() ==1){
		tacc.setCacheSize (0, 1);
	      }
	      else{
		tacc.setCacheSize (startrow, 1);
	      }
	    }
	    
	  } catch (AipsError x) {
	    //  cerr << "Data man type " << dataManType << "  " << dataManType.contains("Tiled") << "  && " << (!String(cdesc.dataManagerGroup()).empty()) << endl;
	    //  cerr << "Failed to set settilecache due to " << x.getMesg() << " column " << columns[k]  <<endl;
	    //It failed so leave the caching as is
	    continue;
	  }
	  
	}
      }
    }
  }
  
}


/*
void ROVisibilityIterator::setTileCache(){
  // This function sets the tile cache because of a feature in 
  // sliced data access that grows memory dramatically in some cases
  //if(useSlicer_p){
  
  {   
    const MeasurementSet& thems=msIter_p.ms();
    const ColumnDescSet& cds=thems.tableDesc().columnDescSet();
    ROArrayColumn<Complex> colVis;
    ROArrayColumn<Float> colwgt;
    Vector<String> columns(3);
    columns(0)=MS::columnName(MS::DATA);
    columns(1)=MS::columnName(MS::CORRECTED_DATA);
    columns(2)=MS::columnName(MS::MODEL_DATA);
    //cout << "COL " << columns << endl;
    for (uInt k=0; k< 3; ++k){
      //cout << "IN loop k " << k << endl;
      if (thems.tableDesc().isColumn(columns(k)) ) {
	
	colVis.attach(thems,columns(k));
	String dataManType;
	dataManType = colVis.columnDesc().dataManagerType();
	//cout << "dataManType " << dataManType << endl;
	if(dataManType.contains("Tiled")){
       
	  ROTiledStManAccessor tacc(thems, 
				    colVis.columnDesc().dataManagerGroup());
	  uInt nHyper = tacc.nhypercubes();
	  // Find smallest tile shape
	  Int lowestProduct = 0;
	  Int lowestId = 0;
	  Bool firstFound = False;
	  for (uInt id=0; id < nHyper; id++) {
	    Int product = tacc.getTileShape(id).product();
	    if (product > 0 && (!firstFound || product < lowestProduct)) {
	      lowestProduct = product;
	      lowestId = id;
	      if (!firstFound) firstFound = True;
	    }
	  }
	  Int nchantile;
	  IPosition tileshape=tacc.getTileShape(lowestId);
	  IPosition axisPath(3,2,0,1);
	  //nchantile=tileshape(1);
	  tileshape(1)=channelGroupSize_p;
	  tileshape(2)=curNumRow_p;
	  //cout << "cursorshape " << tileshape << endl;
	  nchantile=tacc.calcCacheSize(0, tileshape, axisPath);
	
	//  if(nchantile > 0)
	 //   nchantile=channelGroupSize_p/nchantile*10;
	 // if(nchantile<3)
	  //  nchantile=10;
	 
	  ///////////////
	  //nchantile *=8;
	  nchantile=1;
	  //tileshape(2)=tileshape(2)*8;
	  //////////////
	  //cout << tacc.cacheSize(0) << " nchantile "<< nchantile << " max cache size " << tacc.maximumCacheSize() << endl; 
	  tacc.clearCaches();
	  tacc.setCacheSize (0, 1);
	  //tacc.setCacheSize (0, tileshape, axisPath);
	  //cout << k << "  " << columns(k) << " cache size  " <<  tacc.cacheSize(0) << endl;
	  
	}
      }
    }
  }
  
}
*/

void 
ROVisibilityIterator::attachColumns(const Table &t)
{
  const ColumnDescSet& cds=t.tableDesc().columnDescSet();

  colAntenna1.attach(t, MS::columnName(MS::ANTENNA1));
  colAntenna2.attach(t, MS::columnName(MS::ANTENNA2));

  if (cds.isDefined("CORRECTED_DATA"))
    colCorrVis.attach(t, "CORRECTED_DATA");

  colExposure.attach(t, MS::columnName(MS::EXPOSURE));
  colFeed1.attach(t, MS::columnName(MS::FEED1));
  colFeed2.attach(t, MS::columnName(MS::FEED2));
  colFlag.attach(t, MS::columnName(MS::FLAG));
  colFlagCategory.attach(t, MS::columnName(MS::FLAG_CATEGORY));
  colFlagRow.attach(t, MS::columnName(MS::FLAG_ROW));

  if (cds.isDefined(MS::columnName(MS::FLOAT_DATA))) {
    colFloatVis.attach(t, MS::columnName(MS::FLOAT_DATA));
    floatDataFound_p=True;
  } else {
    floatDataFound_p=False;
  }

  if (cds.isDefined("MODEL_DATA")) 
    colModelVis.attach(t, "MODEL_DATA");

  colObservation.attach(t, MS::columnName(MS::OBSERVATION_ID));
  colProcessor.attach(t, MS::columnName(MS::PROCESSOR_ID));
  colScan.attach(t, MS::columnName(MS::SCAN_NUMBER));
  colSigma.attach(t, MS::columnName(MS::SIGMA));
  colState.attach(t, MS::columnName(MS::STATE_ID));
  colTime.attach(t, MS::columnName(MS::TIME));
  colTimeCentroid.attach(t, MS::columnName(MS::TIME_CENTROID));
  colTimeInterval.attach(t, MS::columnName(MS::INTERVAL));
  colUVW.attach(t, MS::columnName(MS::UVW));

  if (cds.isDefined(MS::columnName(MS::DATA))) {
    colVis.attach(t, MS::columnName(MS::DATA));
  }

  colWeight.attach(t, MS::columnName(MS::WEIGHT));

  if (cds.isDefined("WEIGHT_SPECTRUM")) 
    colWeightSpectrum.attach(t, "WEIGHT_SPECTRUM");
}

ROVisibilityIterator & 
ROVisibilityIterator::operator++(int)
{
  if (!more_p) return *this;
  advance();
  return *this;
}

ROVisibilityIterator & ROVisibilityIterator::operator++()
{
  if (!more_p) return *this;
  advance();
  return *this;
}

void ROVisibilityIterator::update_rowIds() const
{
  if (rowIds_p.nelements() == 0) {
      rowIds_p = selRows_p.convert();
      
      Vector<uInt> msIter_rowIds(msIter_p.table().rowNumbers(msIter_p.ms()));

      for (uInt i = 0; i < rowIds_p.nelements(); i++) {
          rowIds_p(i) = msIter_rowIds(rowIds_p(i));
      }
  }
  return;
}


Int
ROVisibilityIterator::getDataDescriptionId () const
{
	return msIter_p.dataDescriptionId();
}


const MeasurementSet &
ROVisibilityIterator::getMeasurementSet() const
{
	return msIter_p.ms();
}

Int
ROVisibilityIterator::getMeasurementSetId() const
{
	return msIter_p.msId();
}


Int
ROVisibilityIterator::getNAntennas () const
{
	Int nAntennas = msIter_p.receptorAngle().shape()(1);

	return nAntennas;
}

MEpoch
ROVisibilityIterator::getMEpoch () const
{
    MEpoch mEpoch = msIter_p.msColumns().timeMeas()(0);

    return mEpoch;
}

Vector<Float>
ROVisibilityIterator::getReceptor0Angle ()
{
	Int nAntennas = getNAntennas();

	Vector<Float> receptor0Angle (nAntennas);

	for (int i = 0; i < nAntennas; i++)
	  receptor0Angle [i] = msIter_p.receptorAngle()(0,i);

	return receptor0Angle;
}

Vector<uInt>
ROVisibilityIterator::getRowIds () const
{
	update_rowIds();

	return rowIds_p;
}


Vector<uInt>& ROVisibilityIterator::rowIds(Vector<uInt>& rowids) const
{
  /* Calculate the row numbers in the original MS only when needed,
     i.e. when this function is called */
  update_rowIds();
  rowids.resize(rowIds_p.nelements());
  rowids = rowIds_p;
  return rowids;
}


Vector<Int>& ROVisibilityIterator::antenna1(Vector<Int>& ant1) const
{
  ant1.resize(curNumRow_p);
  getCol(colAntenna1, ant1);
  return ant1;
}

Vector<Int>& ROVisibilityIterator::antenna2(Vector<Int>& ant2) const
{
  ant2.resize(curNumRow_p);
  getCol(colAntenna2, ant2);
  return ant2;
}

Vector<Int>& ROVisibilityIterator::feed1(Vector<Int>& fd1) const
{
  fd1.resize(curNumRow_p);
  getCol(colFeed1, fd1);
  return fd1;
}

Vector<Int>& ROVisibilityIterator::feed2(Vector<Int>& fd2) const
{
  fd2.resize(curNumRow_p);
  getCol(colFeed2, fd2);
  return fd2;
}

Vector<Int>& ROVisibilityIterator::channel(Vector<Int>& chan) const
{
  Int spw = msIter_p.spectralWindowId();
  chan.resize(channelGroupSize_p);
  Int inc=chanInc_p[spw] <= 0 ? 1 : chanInc_p[spw];
  for (Int i=0; i<channelGroupSize_p; i++) {
    chan(i)=chanStart_p[spw]+curChanGroup_p*chanWidth_p[spw]+i*inc;
  }
  return chan;
}

Vector<Int>& ROVisibilityIterator::corrType(Vector<Int>& corrTypes) const
{
  Int polId = msIter_p.polarizationId();
  msIter_p.msColumns().polarization().corrType().get(polId,corrTypes,True);
  return corrTypes;
}

Cube<Bool>& ROVisibilityIterator::flag(Cube<Bool>& flags) const
{
  if (velSelection_p) {
    if (!flagOK_p) {
      // need to do the interpolation
      getInterpolatedVisFlagWeight(Corrected);
      This->flagOK_p = True;
      This->visOK_p[Corrected] = True;
      This->weightSpOK_p = True;
      if(floatDataFound_p){
        getInterpolatedFloatDataFlagWeight();
        This->floatDataCubeOK_p = True;
      }
    }
    flags.resize(flagCube_p.shape());  flags=flagCube_p; 
  } else {
    if (useSlicer_p) getCol(colFlag, slicer_p,flags,True);
    else getCol(colFlag, flags,True);
  }
  return flags;
}

Matrix<Bool>& ROVisibilityIterator::flag(Matrix<Bool>& flags) const
{
  if (useSlicer_p) getCol(colFlag, slicer_p,This->flagCube_p,True);
  else getCol(colFlag, This->flagCube_p,True);
  flags.resize(channelGroupSize_p,curNumRow_p);
  // need to optimize this...
  //for (Int row=0; row<curNumRow_p; row++) {
  //  for (Int chn=0; chn<channelGroupSize_p; chn++) {
  //    flags(chn,row)=flagCube(0,chn,row);
  //    for (Int pol=1; pol<nPol_p; pol++) {
  //	  flags(chn,row)|=flagCube(pol,chn,row);
  //    }
  //  }
  //}
  Bool deleteIt1;
  Bool deleteIt2;
  const Bool* pcube=This->flagCube_p.getStorage(deleteIt1);
  Bool* pflags=flags.getStorage(deleteIt2);
  for (uInt row=0; row<curNumRow_p; row++) {
    for (Int chn=0; chn<channelGroupSize_p; chn++) {
      *pflags=*pcube++;
      for (Int pol=1; pol<nPol_p; pol++, pcube++) {
	*pflags = *pcube ? *pcube : *pflags;
      }
      pflags++;
    }
  }
  flagCube_p.freeStorage(pcube, deleteIt1);
  flags.putStorage(pflags, deleteIt2);
  return flags;
}

Array<Bool>& ROVisibilityIterator::flagCategory(Array<Bool>& flagCategories) const
{
  if(colFlagCategory.isNull() || !colFlagCategory.isDefined(0)){ // It often is.
    flagCategories.resize();    // Zap it.
  }
  else{
    if(velSelection_p){
      throw(AipsError("velocity selection not allowed in flagCategory()."));
    }
    else{
      if(useSlicer_p)
        getCol(colFlagCategory, slicer_p, flagCategories, True);
      else
        getCol(colFlagCategory, flagCategories, True);
    }
  }
  return flagCategories;
}

Vector<Bool>& ROVisibilityIterator::flagRow(Vector<Bool>& rowflags) const
{
  rowflags.resize(curNumRow_p);
  getCol(colFlagRow, rowflags);
  return rowflags;
}

Vector<Int>& ROVisibilityIterator::observationId(Vector<Int>& obsIDs) const
{
    obsIDs.resize(curNumRow_p);
    getCol(colObservation, obsIDs);
    return obsIDs;
}

Vector<Int>& ROVisibilityIterator::processorId(Vector<Int>& procIDs) const
{
    procIDs.resize(curNumRow_p);
    getCol(colProcessor, procIDs);
    return procIDs;
}

Vector<Int>& ROVisibilityIterator::scan(Vector<Int>& scans) const
{
    scans.resize(curNumRow_p);
    getCol(colScan, scans);
    return scans;
}

Vector<Int>& ROVisibilityIterator::stateId(Vector<Int>& stateIds) const
{
    stateIds.resize(curNumRow_p);
    getCol(colState, stateIds);
    return stateIds;
}

Vector<Double>&
ROVisibilityIterator::frequency(Vector<Double>& freq) const
{
    if (velSelection_p) {
        freq.resize(nVelChan_p);
        freq=selFreq_p;
    } else {
        if (!freqCacheOK_p) {
            This->freqCacheOK_p=True;
            Int spw = msIter_p.spectralWindowId();
            This->frequency_p.resize(channelGroupSize_p);
            const Vector<Double>& chanFreq=msIter_p.frequency();
            Int start=chanStart_p[spw]-msIter_p.startChan();
            Int inc=chanInc_p[spw] <= 0 ? 1 : chanInc_p[spw];
            for (Int i=0; i<channelGroupSize_p; i++) {
                This->frequency_p(i)=chanFreq(start+curChanGroup_p*chanWidth_p[spw]+i*inc);
            }
        }
        freq.resize(channelGroupSize_p);
        freq=frequency_p;
    }
    return freq;
}

Vector<Double>& ROVisibilityIterator::lsrFrequency(Vector<Double>& freq) const
{
  if (velSelection_p) {
    freq.resize(nVelChan_p);
    freq=lsrFreq_p;
  } else {
    // if there is no vel selection, we just return the observing freqs
    frequency(freq);
  }
  return freq;
}

Vector<Double>& ROVisibilityIterator::time(Vector<Double>& t) const
{
  t.resize(curNumRow_p);

  getCol(colTime, t);

  return t;
}

Vector<Double>& ROVisibilityIterator::timeCentroid(Vector<Double>& t) const
{
  t.resize(curNumRow_p);
  getCol(colTimeCentroid, t);
  return t;
}

Vector<Double>& ROVisibilityIterator::timeInterval(Vector<Double>& t) const
{
  t.resize(curNumRow_p);
  getCol(colTimeInterval, t);
  return t;
}

Vector<Double>& ROVisibilityIterator::exposure(Vector<Double>& expo) const
{
  expo.resize(curNumRow_p);
  getCol(colExposure, expo);
  return expo;
}

Cube<Complex>& 
ROVisibilityIterator::visibility(Cube<Complex>& vis, DataColumn whichOne) const
{
  if (velSelection_p) {
    if (!visOK_p[whichOne]) {
      getInterpolatedVisFlagWeight(whichOne);
      This->visOK_p[whichOne] = True;
      This->flagOK_p = True;
      This->weightSpOK_p = True;
    }
    vis.resize(visCube_p.shape()); vis=visCube_p;
  } else { 
    if (useSlicer_p) getDataColumn(whichOne,slicer_p,vis);
    else getDataColumn(whichOne,vis);
  }
  return vis;
}


// helper function to swap the y and z axes of a Cube
void swapyz(Cube<Complex>& out, const Cube<Complex>& in)
{
  IPosition inShape=in.shape();
  uInt nx=inShape(0),ny=inShape(2),nz=inShape(1);
  out.resize(nx,ny,nz);
  Bool deleteIn,deleteOut;
  const Complex* pin = in.getStorage(deleteIn);
  Complex* pout = out.getStorage(deleteOut);
  uInt i=0, zOffset=0;
  for (uInt iz=0; iz<nz; iz++, zOffset+=nx) {
    Int yOffset=zOffset;
    for (uInt iy=0; iy<ny; iy++, yOffset+=nx*nz) {
      for (uInt ix=0; ix<nx; ix++) pout[i++] = pin[ix+yOffset];
    }
  }
  out.putStorage(pout,deleteOut);
  in.freeStorage(pin,deleteIn);
}

// helper function to swap the y and z axes of a Cube
void swapyz(Cube<Bool>& out, const Cube<Bool>& in)
{
  IPosition inShape=in.shape();
  uInt nx=inShape(0),ny=inShape(2),nz=inShape(1);
  out.resize(nx,ny,nz);
  Bool deleteIn,deleteOut;
  const Bool* pin = in.getStorage(deleteIn);
  Bool* pout = out.getStorage(deleteOut);
  uInt i=0, zOffset=0;
  for (uInt iz=0; iz<nz; iz++, zOffset+=nx) {
    Int yOffset=zOffset;
    for (uInt iy=0; iy<ny; iy++, yOffset+=nx*nz) {
      for (uInt ix=0; ix<nx; ix++) pout[i++] = pin[ix+yOffset];
    }
  }
}

Cube<Float>&
ROVisibilityIterator::floatData(Cube<Float>& fcube) const
{
  if(velSelection_p){
    if(!floatDataCubeOK_p){
      getInterpolatedFloatDataFlagWeight();
      This->floatDataCubeOK_p = True;
      This->flagOK_p = True;
      This->weightSpOK_p = True;
    }
    fcube.resize(floatDataCube_p.shape());
    fcube = floatDataCube_p;
  }
  else{
    if(useSlicer_p)
      getFloatDataColumn(slicer_p, fcube);
    else
      getFloatDataColumn(fcube);
  }
  return fcube;
}

// transpose a matrix
void transpose(Matrix<Float>& out, const Matrix<Float>& in)
{
  uInt ny=in.nrow(), nx=in.ncolumn();
  out.resize(nx,ny);
  Bool deleteIn,deleteOut;
  const Float* pin = in.getStorage(deleteIn);
  Float* pout = out.getStorage(deleteOut);
  uInt i=0;
  for (uInt iy=0; iy<ny; iy++) {
    uInt yOffset=0;
    for (uInt ix=0; ix<nx; ix++, yOffset+=ny) pout[i++] = pin[iy+yOffset];
  }
  out.putStorage(pout,deleteOut);
  in.freeStorage(pin,deleteIn);
}
void ROVisibilityIterator::getInterpolatedVisFlagWeight(DataColumn whichOne)
     const
{
  // get vis, flags & weights
  // tricky.. to avoid recursion we need to set velSelection_p to False
  // temporarily.
  This->velSelection_p = False; 
  visibility(This->visCube_p, whichOne);
  flag(This->flagCube_p); 
  imagingWeight(This->imagingWeight_p);
  Vector<Double> freq; frequency(freq);
  This->velSelection_p = True;

  // now interpolate visibilities, using selFreq as the sample points
  // we should have two options: flagging output points that have
  // any flagged inputs or interpolating across flagged data.
  // Convert frequencies to float (removing offset to keep accuracy) 
  // so we can multiply them with Complex numbers to do the interpolation.
  Block<Float> xfreq(channelGroupSize_p),sfreq(nVelChan_p); 
  Int i;
  for (i=0; i<channelGroupSize_p; i++) xfreq[i]=freq(i)-freq(0);
  for (i=0; i<nVelChan_p; i++) sfreq[i]=selFreq_p(i)-freq(0);
  // we should probably be using the flags for weight interpolation as well
  // but it's not clear how to combine the 4 pol flags into one.
  // (AND the flags-> weight flagged if all flagged?)
  Cube<Complex> vis,intVis;
  swapyz(vis,visCube_p);
  Cube<Bool> flag,intFlag;
  swapyz(flag,flagCube_p);
  Matrix<Float> wt,intWt;
  transpose(wt,imagingWeight_p);
  InterpolateArray1D<Float,Complex>::InterpolationMethod method1=
    InterpolateArray1D<Float,Complex>::linear;
  InterpolateArray1D<Float,Float>::InterpolationMethod method2=
    InterpolateArray1D<Float,Float>::linear;
  if (vInterpolation_p=="nearest") {
    method1=InterpolateArray1D<Float,Complex>::nearestNeighbour;
    method2= InterpolateArray1D<Float,Float>::nearestNeighbour;
  }
  InterpolateArray1D<Float,Complex>::
    interpolate(intVis,intFlag,sfreq,xfreq,vis,flag,method1);
  InterpolateArray1D<Float,Float>::interpolate(intWt,sfreq,xfreq,wt,method2);
  swapyz(This->visCube_p,intVis);
  swapyz(This->flagCube_p,intFlag);
  transpose(This->imagingWeight_p,intWt);
}

void ROVisibilityIterator::getInterpolatedFloatDataFlagWeight() const
{
  // get floatData, flags & weights
  // tricky.. to avoid recursion we need to set velSelection_p to False
  // temporarily.
  This->velSelection_p = False; 
  floatData(This->floatDataCube_p);
  flag(This->flagCube_p); 
  //imagingWeight(This->imagingWeight_p);
  Vector<Double> freq; frequency(freq);
  This->velSelection_p = True;

  // now interpolate floatData, using selFreq as the sample points
  // we should have two options: flagging output points that have
  // any flagged inputs or interpolating across flagged data.
  // Convert frequencies to float (removing offset to keep accuracy) 
  // so we can multiply them with Complex numbers to do the interpolation.
  Block<Float> xfreq(channelGroupSize_p),sfreq(nVelChan_p); 
  Int i;
  for (i=0; i<channelGroupSize_p; i++) xfreq[i]=freq(i)-freq(0);
  for (i=0; i<nVelChan_p; i++) sfreq[i]=selFreq_p(i)-freq(0);
  // we should probably be using the flags for weight interpolation as well
  // but it's not clear how to combine the 4 pol flags into one.
  // (AND the flags-> weight flagged if all flagged?)
  Cube<Float> floatData, intFloatData;
  swapyz(floatData, floatDataCube_p);
  Cube<Bool> flag, intFlag;
  swapyz(flag, flagCube_p);
  //Matrix<Float> wt, intWt;
  //transpose(wt,imagingWeight_p);
  InterpolateArray1D<Float, Float>::InterpolationMethod method =
    InterpolateArray1D<Float,Float>::linear;
  if(vInterpolation_p == "nearest")
    method = InterpolateArray1D<Float, Float>::nearestNeighbour;
  InterpolateArray1D<Float, Float>::
    interpolate(intFloatData, intFlag, sfreq, xfreq, floatData, flag, method);
  //InterpolateArray1D<Float,Float>::interpolate(intWt,sfreq,xfreq,wt,method);
  swapyz(This->floatDataCube_p,intFloatData);
  swapyz(This->flagCube_p,intFlag);
  //transpose(This->imagingWeight_p,intWt);
}

void ROVisibilityIterator::getDataColumn(DataColumn whichOne, 
					 const Slicer& slicer,
					 Cube<Complex>& data) const
{

 
  // Return the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat;
      getCol(colFloatVis, slicer,dataFloat,True);
      data.resize(dataFloat.shape());
      convertArray(data,dataFloat);
    } else {
      getCol(colVis, slicer,data,True);
    }
    break;
  case Corrected:
    getCol(colCorrVis, slicer,data,True);
    break;
  case Model:
    getCol(colModelVis, slicer,data,True);
    break;
  }
 
}

void ROVisibilityIterator::getDataColumn(DataColumn whichOne,
					 Cube<Complex>& data) const
{
  // Return the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat;
      getCol(colFloatVis, dataFloat,True);
      data.resize(dataFloat.shape());
      convertArray(data,dataFloat);
    } else {
      getCol(colVis, data,True);
    }
    break;
  case Corrected:
    getCol(colCorrVis, data,True);
    break;
  case Model:
    getCol(colModelVis, data,True);
    break;
  }
}  

void ROVisibilityIterator::getFloatDataColumn(const Slicer& slicer,
                                              Cube<Float>& data) const
{
  // Return FLOAT_DATA as real Floats.
  if(floatDataFound_p)
    getCol(colFloatVis, slicer, data, True);
}

void ROVisibilityIterator::getFloatDataColumn(Cube<Float>& data) const
{
  // Return FLOAT_DATA as real Floats.
  if(floatDataFound_p)
    getCol(colFloatVis, data, True);
}  

Matrix<CStokesVector>& 
ROVisibilityIterator::visibility(Matrix<CStokesVector>& vis,
				 DataColumn whichOne) const
{
  if (useSlicer_p) getDataColumn(whichOne,slicer_p,This->visCube_p);
  else getDataColumn(whichOne,This->visCube_p);
  vis.resize(channelGroupSize_p,curNumRow_p);
  Bool deleteIt;
  Complex* pcube=This->visCube_p.getStorage(deleteIt);
  if (deleteIt) cerr<<"Problem in ROVisIter::visibility - deleteIt True"<<endl;
  // Here we cope in a limited way with cases where not all 4 
  // polarizations are present: if only 2, assume XX,YY or RR,LL
  // if only 1, assume it's an estimate of Stokes I (one of RR,LL,XX,YY)
  // The cross terms are zero filled in these cases.
  switch (nPol_p) {
  case 4: {
    for (uInt row=0; row<curNumRow_p; row++) {
      for (Int chn=0; chn<channelGroupSize_p; chn++,pcube+=4) {
	vis(chn,row)=pcube;
      }
    }
    break;
  }
  case 2: {
    vis.set(Complex(0.,0.));
    for (uInt row=0; row<curNumRow_p; row++) {
      for (Int chn=0; chn<channelGroupSize_p; chn++,pcube+=2) {
	CStokesVector& v=vis(chn,row);
	v(0)=*pcube; 
	v(3)=*(pcube+1); 
      }
    }
    break;
  }
  case 1: {
    vis.set(Complex(0.,0.));
    for (uInt row=0; row<curNumRow_p; row++) {
      for (Int chn=0; chn<channelGroupSize_p; chn++,pcube++) {
	CStokesVector& v=vis(chn,row);
	v(0)=v(3)=*pcube; 
      }
    }
  } //# case 1
  } //# switch 
  return vis;
}

Vector<RigidVector<Double,3> >& 
ROVisibilityIterator::uvw(Vector<RigidVector<Double,3> >& uvwvec) const
{
    uvwvec.resize(curNumRow_p);
    getColArray<Double>(colUVW, This->uvwMat_p, True);
    // get a pointer to the raw storage for quick access
    Bool deleteIt;
    Double* pmat=This->uvwMat_p.getStorage(deleteIt);
    for (uInt row=0; row<curNumRow_p; row++, pmat+=3) uvwvec(row)=pmat;
    return uvwvec;
}

Matrix<Double>& ROVisibilityIterator::uvwMat(Matrix<Double>& uvwmat) const
{
    getCol(colUVW, uvwmat,True);
    return uvwmat;
}

// Fill in parallactic angle.
Vector<Float>
ROVisibilityIterator::feed_pa(Double time) const
{
	//  LogMessage message(LogOrigin("ROVisibilityIterator","feed_pa"));

	// Absolute UT
	Double ut=time;

	if (ut!=lastfeedpaUT_p) {

		This->lastfeedpaUT_p=ut;

		// Set up the Epoch using the absolute MJD in seconds
		// get the Epoch reference from the column

		MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);

		Int nAnt = msIter_p.receptorAngle().shape()(1);

		Vector<Float> receptor0Angle (nAnt, 0);

		for (int i = 0; i < nAnt; i++)
			receptor0Angle [i] = msIter_p.receptorAngle()(0,i);

		This->feedpa_p.assign (feed_paCalculate (time, This->msd_p, nAnt, mEpoch, receptor0Angle));
	}
	return feedpa_p;
}

Vector<Float>
ROVisibilityIterator::feed_paCalculate(Double time, MSDerivedValues & msd,
  									   Int nAntennas, const MEpoch & mEpoch0,
									   const Vector<Float> & receptor0Angle)
{
	MEpoch mEpoch = mEpoch0;

	mEpoch.set(MVEpoch(Quantity(time, "s")));

	msd.setEpoch(mEpoch);

	// Calculate pa for all antennas.

	Vector<Float> feedpa (nAntennas);

	for (Int iant=0; iant<nAntennas; iant++) {

		msd.setAntenna(iant);
		feedpa (iant) = msd.parAngle();

		// add angle for receptor 0

		feedpa (iant) += receptor0Angle (iant);

		if (aips_debug && iant == 0) {

			cout<<"Antenna "<<iant<<" at time: "<<MVTime(mEpoch.getValue())<<
					" has PA = "<<feedpa(iant)*57.28<<endl;
		}
	}

	return feedpa;
}

// Fill in parallactic angle.
const Float&
ROVisibilityIterator::parang0(Double time) const
{
	//  LogMessage message(LogOrigin("ROVisibilityIterator","parang0"));

	// Absolute UT
	Double ut=time;

	if (ut!=lastfeedpaUT_p) {

		This->lastfeedpaUT_p=ut;

		// Set up the Epoch using the absolute MJD in seconds
		// get the Epoch reference from the column
		MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);
		This->parang0_p = parang0Calculate (time, This->msd_p, mEpoch);
	}
	return parang0_p;
}

Float
ROVisibilityIterator::parang0Calculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0)
{
	MEpoch mEpoch = mEpoch0;

    mEpoch.set(MVEpoch(Quantity(time, "s")));
    msd.setEpoch(mEpoch);

    // Calculate pa for all antennas.
    msd.setAntenna(-1);
    Float parang0 = msd.parAngle();

    if (aips_debug) 
      cout<<"At time: "<<MVTime(mEpoch.getValue())<<
	" PA = "<<parang0*57.28<<" deg"<<endl;

    return parang0;
}


// Fill in parallactic angle (NO FEED PA offset!).
Vector<Float>
ROVisibilityIterator::parang(Double time) const
{
	//  LogMessage message(LogOrigin("ROVisibilityIterator","parang"));

	// Absolute UT
	Double ut=time;

	if (ut!=lastfeedpaUT_p) {

		This->lastfeedpaUT_p=ut;

		// Set up the Epoch using the absolute MJD in seconds
		// get the Epoch reference from the column

		MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);
		Int nAnt = msIter_p.receptorAngle().shape()(1);

		This->parang_p = parangCalculate (time, This->msd_p, nAnt, mEpoch);

	}
	return parang_p;
}

Vector<Float>
ROVisibilityIterator::parangCalculate (Double time, MSDerivedValues & msd, int nAntennas, const MEpoch mEpoch0)
{
	MEpoch mEpoch= mEpoch0;
	mEpoch.set(MVEpoch(Quantity(time, "s")));

	msd.setEpoch(mEpoch);

	// Calculate pa for all antennas.

	Vector<Float> parang (nAntennas);

	for (Int iant=0; iant<nAntennas; iant++){

		msd.setAntenna(iant);
		parang (iant) = msd.parAngle();

		if (aips_debug && iant == 0) {
			cout<<"Antenna "<<iant<<" at time: "<<MVTime(mEpoch.getValue())<<
					" has PA = "<<parang(iant)*57.28<<endl;
		}
	}

	return parang;
}

// Fill in azimuth/elevation of the antennas.
// Cloned from feed_pa, we need to check that this is all correct!
Vector<MDirection>
ROVisibilityIterator::azel(Double time) const
{
	//  LogMessage message(LogOrigin("ROVisibilityIterator","azel"));

	// Absolute UT
	Double ut=time;

	if (ut!=lastazelUT_p) {

		This->lastazelUT_p=ut;
		Int nAnt = msIter_p.receptorAngle().shape()(1);
		MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);

		azelCalculate (ut, This->msd_p, This->azel_p, nAnt, mEpoch);

	}
	return azel_p;
}

void
ROVisibilityIterator::azelCalculate (Double time, MSDerivedValues & msd, Vector<MDirection> & azel,
		                             Int nAnt, const MEpoch & mEpoch0)
{
	// Refactored into a static method to allow VisBufferAsync to use

	MEpoch mEpoch=mEpoch0;

	mEpoch.set(MVEpoch(Quantity(time, "s")));

	msd.setEpoch(mEpoch);

	// Calculate az/el for all antennas.

	azel.resize (nAnt);

	for (Int iant=0;iant<nAnt;iant++) {
		msd.setAntenna(iant);
		azel(iant) = msd.azel();
		if (aips_debug) {
			if (iant==0)
				cout<<"Antenna "<<iant<<" at time: "<<MVTime(mEpoch.getValue())<<
				" has AzEl = "<<azel(iant).getAngle("deg")<<endl;
		}
	}
}

// Fill in azimuth/elevation of the antennas.
// Cloned from feed_pa, we need to check that this is all correct!
MDirection
ROVisibilityIterator::azel0(Double time) const
{
  //  LogMessage message(LogOrigin("ROVisibilityIterator","azel0"));

  // Absolute UT
  Double ut=time;

  if (ut!=lastazelUT_p) {

    This->lastazelUT_p=ut;

    MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);

    azel0Calculate (time, This->msd_p, This->azel0_p, mEpoch);

  }
  return azel0_p;
}

void
ROVisibilityIterator::azel0Calculate (Double time, MSDerivedValues & msd,
		                              MDirection & azel0, const MEpoch & mEpoch0)
{
	// Refactored into a static method to allow VisBufferAsync to use

	MEpoch mEpoch=mEpoch0;

	mEpoch.set(MVEpoch(Quantity(time, "s")));

	msd.setEpoch(mEpoch);

	msd.setAntenna(-1);

	azel0 = msd.azel();

	if (aips_debug){
		cout<<"At time: "<<MVTime(mEpoch.getValue())<<
				" AzEl = "<<azel0.getAngle("deg")<<endl;
	}

}

// Hour angle at specified time.
Double
ROVisibilityIterator::hourang(Double time) const
{
  //  LogMessage message(LogOrigin("ROVisibilityIterator","azel"));

  // Absolute UT
  Double ut=time;

  if (ut!=lastazelUT_p) {

    This->lastazelUT_p=ut;

    // Set up the Epoch using the absolute MJD in seconds
    // get the Epoch reference from the column keyword

    MEpoch mEpoch=msIter_p.msColumns().timeMeas()(0);

    This->hourang_p = hourangCalculate (time, This->msd_p, mEpoch);

  }
  return hourang_p;
}

Double
ROVisibilityIterator::hourangCalculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0)
{
    MEpoch mEpoch = mEpoch0;

    mEpoch.set(MVEpoch(Quantity(time, "s")));

    msd.setEpoch(mEpoch);

    msd.setAntenna(-1);

    Double hourang = msd.hourAngle();

    return hourang;
}

Vector<Float>& ROVisibilityIterator::sigma(Vector<Float>& sig) const
{
  Matrix<Float> sigmat;
  getCol(colSigma, sigmat);
  // Do a rough average of the parallel hand polarizations to get a single 
  // sigma. Should do this properly someday, or return all values
  sig.resize(sigmat.ncolumn());
  sig=sigmat.row(0);
  sig+=sigmat.row(nPol_p-1);
  sig/=2.0f;
  return sig;
}

Matrix<Float>& ROVisibilityIterator::sigmaMat(Matrix<Float>& sigmat) const
{
  sigmat.resize(nPol_p,curNumRow_p);
  getCol(colSigma, sigmat);
  return sigmat;
}

Vector<Float>& ROVisibilityIterator::weight(Vector<Float>& wt) const
{
  // Take average of parallel hand polarizations for now.
  // Later convert weight() to return full polarization dependence
  Matrix<Float> polWeight;
  getCol(colWeight, polWeight);
  wt.resize(polWeight.ncolumn());
  wt=polWeight.row(0);
  wt+=polWeight.row(nPol_p-1);
  wt/=2.0f;
  return wt;
}

Matrix<Float>& ROVisibilityIterator::weightMat(Matrix<Float>& wtmat) const
{
  wtmat.resize(nPol_p,curNumRow_p);
  getCol(colWeight, wtmat);
  return wtmat;
}


Bool ROVisibilityIterator::existsWeightSpectrum()
{
  if(msIter_p.newMS()){ // Cache to avoid testing unnecessarily.
    try{
      msHasWtSp_p = (!colWeightSpectrum.isNull() &&
                     colWeightSpectrum.isDefined(0) &&
                     colWeightSpectrum.shape(0)[0] > 0 &&
                     colWeightSpectrum.shape(0)[1] > 0);
      // Comparing colWeightSpectrum.shape(0) to
      // IPosition(2, nPol_p, channelGroupSize()) is too strict
      // when channel averaging might have changed
      // channelGroupSize() or weightSpectrum() out of sync.  Unfortunately the
      // right answer might not get cached soon enough.
      //
      //       colWeightSpectrum.shape(0).isEqual(IPosition(2, nPol_p,
      //                                                    channelGroupSize())));
      // if(!msHasWtSp_p){
      //   cerr << "colWeightSpectrum.shape(0): " << colWeightSpectrum.shape(0) << endl;
      //   cerr << "(nPol_p, channelGroupSize()): " << nPol_p 
      //        << ", " << channelGroupSize() << endl;
      // }
    }
    catch (AipsError x) {
      msHasWtSp_p = False;
    } 
  }
  return msHasWtSp_p;
}


Cube<Float>& ROVisibilityIterator::weightSpectrum(Cube<Float>& wtsp) const
{
  if (!colWeightSpectrum.isNull()) {
    wtsp.resize(nPol_p,nChan_p,curNumRow_p);
    getCol(colWeightSpectrum, wtsp);
  } else {
    wtsp.resize(0,0,0);
  }
  return wtsp;
}

Matrix<Float>& ROVisibilityIterator::imagingWeight(Matrix<Float>& wt) const
{
    if(imwgt_p.getType() == "none")
        throw(AipsError("Programmer Error... imaging weights not set"));
    Vector<Float> weightvec;
    weight(weightvec);
    Matrix<Bool> flagmat;
    flag(flagmat);
    wt.resize(flagmat.shape());
    if(imwgt_p.getType()=="uniform"){
        Vector<Double> fvec;
        frequency(fvec);
        Matrix<Double> uvwmat;
        uvwMat(uvwmat);
        imwgt_p.weightUniform(wt, flagmat, uvwmat, fvec, weightvec, msId(), fieldId());
        if(imwgt_p.doFilter())
	    imwgt_p.filter(wt, flagmat, uvwmat, fvec, weightvec);
    }
    else if(imwgt_p.getType()=="radial"){
        Vector<Double> fvec;
        frequency(fvec);
        Matrix<Double> uvwmat;
        uvwMat(uvwmat);
        imwgt_p.weightRadial(wt, flagmat, uvwmat, fvec, weightvec);
        if(imwgt_p.doFilter())
	    imwgt_p.filter(wt, flagmat, uvwmat, fvec, weightvec);
    }
    else{
	imwgt_p.weightNatural(wt, flagmat, weightvec);
	if(imwgt_p.doFilter()){
            Matrix<Double> uvwmat;
            uvwMat(uvwmat);
            Vector<Double> fvec;
            frequency(fvec);
            imwgt_p.filter(wt, flagmat, uvwmat, fvec, weightvec);

	}
    }

    return wt;
}

Int ROVisibilityIterator::nSubInterval() const
{
// Return the number of sub-intervals in the current chunk,
// i.e. the number of unique time stamps
//
  // Find all unique times in time_p
  Int retval=0;
  uInt nTimes=time_p.nelements();
  if (nTimes > 0) {
  
    Vector<Double> times(time_p); /* Do not change time_p, make a copy */
    Bool deleteIt;
    Double *tp = times.getStorage(deleteIt);

    std::sort(tp, tp + nTimes);
    
    /* Count unique times */
    retval = 1;
    for (unsigned i = 0; i < nTimes - 1; i++) {
      if (tp[i] < tp[i+1]) retval += 1;
    }
  }
  return retval;
}

ROVisibilityIterator& 
ROVisibilityIterator::selectVelocity (Int nChan,
                                      const MVRadialVelocity& vStart,
                                      const MVRadialVelocity& vInc,
                                      MRadialVelocity::Types rvType,
                                      MDoppler::Types dType,
                                      Bool precise)
{
    if (!initialized_p) {
        // initialize the base iterator only (avoid recursive call to originChunks)
        if (!msIterAtOrigin_p) {
            msIter_p.origin();
            msIterAtOrigin_p=True;
            stateOk_p=False;
        }
    }
    velSelection_p=True;
    nVelChan_p=nChan;
    vStart_p=vStart;
    vInc_p=vInc;
    msd_p.setVelocityFrame(rvType);
    vDef_p=dType;
    cFromBETA_p.set(MDoppler(MVDoppler(Quantity(0.,"m/s")),
                             MDoppler::BETA),vDef_p);
    vPrecise_p=precise;
    if (precise) {
        // set up conversion engine for full conversion
    }
    // have to reset the iterator so all caches get filled
    originChunks();
    return *this;
}


ROVisibilityIterator&  
ROVisibilityIterator::selectChannel(Int nGroup, Int start, Int width, 
                                    Int increment, Int spectralWindow)
{

    if (!initialized_p) {
        // initialize the base iterator only (avoid recursive call to originChunks)
        if (!msIterAtOrigin_p) {
            msIter_p.origin();
            msIterAtOrigin_p=True;
            stateOk_p=False;
        }
    }
    Int spw=spectralWindow;
    if (spw<0) spw = msIter_p.spectralWindowId();
    Int n = numChanGroup_p.nelements();
    if(n==0){
        blockSpw_p.resize(1, True, False);
        blockSpw_p[0].resize(1);
        blockSpw_p[0][0]=spw;
        blockNumChanGroup_p.resize(1,True,False);
        blockNumChanGroup_p[0].resize(1);
        blockNumChanGroup_p[0][0]=nGroup;
        blockChanStart_p.resize(1, True, False);
        blockChanStart_p[0].resize(1);
        blockChanStart_p[0][0]=start;
        blockChanWidth_p.resize(1, True, False);
        blockChanWidth_p[0].resize(1);
        blockChanWidth_p[0][0]=width;
        blockChanInc_p.resize(1, True, False);
        blockChanInc_p[0].resize(1);
        blockChanInc_p[0][0]=increment;
        msCounter_p=0;



    }
    else{
        Bool hasSpw=False;
        Int spwIndex=-1;
        for (uInt k = 0; k < blockSpw_p[0].nelements(); ++k){
            if(spw==blockSpw_p[0][k]){
                hasSpw=True;
                spwIndex=k;
                break;
            }
        }
        if(!hasSpw){
            Int nspw=blockSpw_p[0].nelements()+1;
            blockSpw_p[0].resize(nspw, True);
            blockSpw_p[0][nspw-1]=spw;
            blockNumChanGroup_p[0].resize(nspw,True);
            blockNumChanGroup_p[0][nspw-1]=nGroup;
            blockChanStart_p[0].resize(nspw, True);
            blockChanStart_p[0][nspw-1]=start;
            blockChanWidth_p[0].resize(nspw, True);
            blockChanWidth_p[0][nspw-1]=width;
            blockChanInc_p[0].resize(nspw, True);
            blockChanInc_p[0][nspw-1]=increment;
        }
        else{
            blockSpw_p[0][spwIndex]=spw;
            blockNumChanGroup_p[0][spwIndex]=nGroup;
            blockChanStart_p[0][spwIndex]=start;
            blockChanWidth_p[0][spwIndex]=width;
            blockChanInc_p[0][spwIndex]=increment;
        }


    }
    if (spw >= n) {
        // we need to resize the blocks
        Int newn = max(2,max(2*n,spw+1));
        numChanGroup_p.resize(newn);
        chanStart_p.resize(newn);
        chanWidth_p.resize(newn);
        chanInc_p.resize(newn);
        for (Int i = n; i<newn; i++) numChanGroup_p[i] = 0;
    }
    chanStart_p[spw] = start;
    chanWidth_p[spw] = width;

    chanInc_p[spw] = increment;
    numChanGroup_p[spw] = nGroup;
    // have to reset the iterator so all caches get filled & slicer sizes
    // get updated
    //  originChunks();
//
//    if(msIterAtOrigin_p){
//        if(!isInSelectedSPW(msIter_p.spectralWindowId())){
//            while((!isInSelectedSPW(msIter_p.spectralWindowId()))
//                    && (msIter_p.more()))
//                msIter_p++;
//            stateOk_p=False;
//            setState();
//        }
//    }

    //leave the state where msiter is pointing
    channelGroupSize_p = chanWidth_p[msIter_p.spectralWindowId()];
    curNumChanGroup_p = numChanGroup_p[msIter_p.spectralWindowId()];

    return *this;
}

ROVisibilityIterator&  
ROVisibilityIterator::selectChannel(Block<Vector<Int> >& blockNGroup, 
				    Block<Vector<Int> >& blockStart, 
				    Block<Vector<Int> >& blockWidth, 
				    Block<Vector<Int> >& blockIncr,
				    Block<Vector<Int> >& blockSpw)
{
  /*
  No longer needed
  if(!isMultiMS_p){
    //Programmer error ...so should not reach here
    cout << "Cannot use this function if Visiter was not constructed with multi-ms" 
	 << endl;
  }
  */

  blockNumChanGroup_p.resize(0, True, False);
  blockNumChanGroup_p=blockNGroup;
  blockChanStart_p.resize(0, True, False);
  blockChanStart_p=blockStart;
  blockChanWidth_p.resize(0, True, False);
  blockChanWidth_p=blockWidth;
  blockChanInc_p.resize(0, True, False);
  blockChanInc_p=blockIncr;
  blockSpw_p.resize(0, True, False);
  blockSpw_p=blockSpw;

  if (!initialized_p) {
    // initialize the base iterator only (avoid recursive call to originChunks)
    if (!msIterAtOrigin_p) {
      msIter_p.origin();
      msIterAtOrigin_p=True;
      stateOk_p=False;
    }
  }    

  numChanGroup_p.resize(0);
  msCounter_p=0;

  doChannelSelection();
  // have to reset the iterator so all caches get filled & slicer sizes
  // get updated
  
  if(msIterAtOrigin_p){
    if(!isInSelectedSPW(msIter_p.spectralWindowId())){
      while((!isInSelectedSPW(msIter_p.spectralWindowId())) 
	    && (msIter_p.more()))
	msIter_p++;
      stateOk_p=False;
    }
    
  }
  
  originChunks();
  return *this;
}


void
ROVisibilityIterator::getChannelSelection(Block< Vector<Int> >& blockNGroup,
                                          Block< Vector<Int> >& blockStart,
                                          Block< Vector<Int> >& blockWidth,
                                          Block< Vector<Int> >& blockIncr,
                                          Block< Vector<Int> >& blockSpw){


    blockNGroup.resize(0, True, False);
    blockNGroup=blockNumChanGroup_p;
    blockStart.resize(0, True, False);
    blockStart=blockChanStart_p;
    blockWidth.resize(0, True, False);
    blockWidth=blockChanWidth_p;
    blockIncr.resize(0, True, False);
    blockIncr=blockChanInc_p;
    blockSpw.resize(0, True, False);
    blockSpw=blockSpw_p;



}
void  ROVisibilityIterator::doChannelSelection()
{


  
  for (uInt k=0; k < blockSpw_p[msCounter_p].nelements(); ++k){
    Int spw=blockSpw_p[msCounter_p][k];
    if (spw<0) spw = msIter_p.spectralWindowId();
    Int n = numChanGroup_p.nelements();
    if (spw >= n) {
      // we need to resize the blocks
      Int newn = max(2,max(2*n,spw+1));
      numChanGroup_p.resize(newn, True, True);
      chanStart_p.resize(newn, True, True);
      chanWidth_p.resize(newn, True, True);
      chanInc_p.resize(newn, True, True);
      for (Int i = n; i<newn; i++) numChanGroup_p[i] = 0;
    }
   
    chanStart_p[spw] = blockChanStart_p[msCounter_p][k];
    chanWidth_p[spw] = blockChanWidth_p[msCounter_p][k];
    channelGroupSize_p = blockChanWidth_p[msCounter_p][k];
    chanInc_p[spw] = blockChanInc_p[msCounter_p][k];
    numChanGroup_p[spw] = blockNumChanGroup_p[msCounter_p][k];
    curNumChanGroup_p = blockNumChanGroup_p[msCounter_p][k];
    
  }
  Int spw=msIter_p.spectralWindowId();
  Int spIndex=-1;
  for (uInt k=0; k < blockSpw_p[msCounter_p].nelements(); ++k){
    if(spw==blockSpw_p[msCounter_p][k]){
      spIndex=k;
      break;
    }
  }

 
  if(spIndex < 0)
    spIndex=0;
  //leave this at the stage where msiter is pointing
  channelGroupSize_p = blockChanWidth_p[msCounter_p][spIndex];
  curNumChanGroup_p = blockNumChanGroup_p[msCounter_p][spIndex];



}

void
ROVisibilityIterator::getSpwInFreqRange(Block<Vector<Int> >& spw,
                                        Block<Vector<Int> >& start,
                                        Block<Vector<Int> >& nchan,
                                        Double freqStart,
                                        Double freqEnd,
                                        Double freqStep)
{
    // This functionality was relocated from MSIter in order to support this operation
    // within the VI to make the ROVisibilityIteratorAsync implementation feasible.

    int nMS = measurementSets_p.size();

    spw.resize(nMS, True, False);
    start.resize(nMS, True, False);
    nchan.resize(nMS, True, False);

    for (Int k=0; k < nMS; ++k){
        MSSpwIndex spwIn(measurementSets_p[k]->spectralWindow());

        spwIn.matchFrequencyRange(freqStart-0.5*freqStep, freqEnd+0.5*freqStep, spw[k], start[k], nchan[k]);
    }
}

vector<const MeasurementSet *>
ROVisibilityIterator::getMeasurementSets () const
{
    return measurementSets_p;
}


void ROVisibilityIterator::allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan){

  spws.resize();
  spws=blockSpw_p[msId()];
  nvischan.resize();
  nvischan.resize(max(spws)+1);
  nvischan.set(-1);
  for (uInt k=0; k < spws.nelements(); ++k){
      nvischan[spws[k]]=chanWidth_p[spws[k]];
  }
}

void
ROVisibilityIterator::lsrFrequency(const Int& spw, Vector<Double>& freq,
                                   Bool& convert)
{
    // This method is not good for conversion between frames which are extremely
    // time dependent over the course of the observation e.g topo to lsr unless
    // the epoch is in the actual buffer

    if (velSelection_p) {
        getTopoFreqs();
        lsrFrequency(freq);
        return;
    }

    if (!freqCacheOK_p) {
        frequency(freq);
    }

    //MFrequency::Types obsMFreqType=(MFrequency::Types)(msIter_p.msColumns().spectralWindow().measFreqRef()(spw));

    //chanFreq=msIter_p.msColumns().spectralWindow().chanFreq()(spw);

    const ROArrayColumn <Double> & chanFreqs = msIter_p.msColumns().spectralWindow().chanFreq();
    const ROScalarColumn<Int> & obsMFreqTypes= msIter_p.msColumns().spectralWindow().measFreqRef();

    MEpoch ep = msIter_p.msColumns().timeMeas()(0); // Setting epoch to iteration's first one
    MPosition obsPos = msIter_p.telescopePosition();
    MDirection dir = msIter_p.phaseCenter();

    lsrFrequency (spw, freq, convert, chanStart_p, chanWidth_p, chanInc_p, numChanGroup_p,
                  chanFreqs, obsMFreqTypes, ep, obsPos, dir);

}

void
ROVisibilityIterator::lsrFrequency (const Int& spw,
                                    Vector<Double>& freq,
                                    Bool& convert,
                                    const Block<Int> & chanStart,
                                    const Block<Int> & chanWidth,
                                    const Block<Int> & chanInc,
                                    const Block<Int> & numChanGroup,
                                    const ROArrayColumn <Double> & chanFreqs,
                                    const ROScalarColumn<Int> & obsMFreqTypes,
                                    const MEpoch & ep,
                                    const MPosition & obsPos,
                                    const MDirection & dir)
{

    Vector<Double> chanFreq(0);
    chanFreq = chanFreqs (spw);

    //chanFreq=msIter_p.msColumns().spectralWindow().chanFreq()(spw);
    //      Int start=chanStart_p[spw]-msIter_p.startChan();
    //Assuming that the spectral windows selected is not a reference ms from
    //visset ...as this will have a start chan offseted may be.

    Int start=chanStart[spw];
    freq.resize(chanWidth[spw]);
    MFrequency::Types obsMFreqType= (MFrequency::Types) (obsMFreqTypes (spw));
    MeasFrame frame(ep, obsPos, dir);
    MFrequency::Convert tolsr(obsMFreqType,
                              MFrequency::Ref(MFrequency::LSRK, frame));

    if(obsMFreqType != MFrequency::LSRK){
        convert=True;
    }

    for (Int i=0; i<chanWidth[spw]; i++) {
        Int inc=chanInc[spw] <= 0 ? 1 : chanInc[spw] ;
        if(convert){
            freq[i]=tolsr(chanFreq(start+
                                   (numChanGroup[spw]-1)*chanWidth[spw]+i*inc)).
                                   getValue().getValue();
        }
        else{
            freq[i]=chanFreq(start+
                             (numChanGroup[spw]-1)*chanWidth[spw]+i*inc);
        }
    }
}

void
ROVisibilityIterator::getLsrInfo (Block<Int> & channelStart,
                                  Block<Int> & channelWidth,
                                  Block<Int> & channelIncrement,
                                  Block<Int> & channelGroupNumber,
                                  const ROArrayColumn <Double> * & chanFreqs,
                                  const ROScalarColumn<Int> * & obsMFreqTypes,
                                  MPosition & observatoryPositon,
                                  MDirection & phaseCenter,
                                  Bool & velocitySelection) const
{
    channelStart = chanStart_p;
    channelWidth = chanWidth_p;
    channelIncrement = chanInc_p;
    chanFreqs = & msIter_p.msColumns().spectralWindow().chanFreq();
    channelGroupNumber = numChanGroup_p;
    obsMFreqTypes = & msIter_p.msColumns().spectralWindow().measFreqRef();
    observatoryPositon = msIter_p.telescopePosition();
    phaseCenter = msIter_p.phaseCenter();
    velocitySelection = velSelection_p;
}


void ROVisibilityIterator::attachVisBuffer(VisBuffer& vb)
{
  vbStack_p.push(& vb);
  vb.invalidate();
}

VisBuffer *
ROVisibilityIterator::getVisBuffer ()
{
    VisBuffer * result = NULL;

    if (! vbStack_p.empty()){
        result = vbStack_p.top();
    }

    return result;
}

void ROVisibilityIterator::detachVisBuffer(VisBuffer& vb)
{
  if (!vbStack_p.empty()) {
    if (vbStack_p.top() == & vb) {
      vbStack_p.pop();
      if (!vbStack_p.empty()) vbStack_p.top()->invalidate();
    } else {
      throw(AipsError("ROVisIter::detachVisBuffer - attempt to detach "
		      "buffer that is not the last one attached"));
    }
  }
}

Int ROVisibilityIterator::numberAnt(){
  return msColumns().antenna().nrow(); // for single (sub)array only..
}

Int ROVisibilityIterator::numberSpw(){
  return msColumns().spectralWindow().nrow();
}

Int ROVisibilityIterator::numberDDId(){
  return msColumns().dataDescription().nrow(); 
}

Int ROVisibilityIterator::numberPol(){
  return msColumns().polarization().nrow(); 
}

Int ROVisibilityIterator::numberCoh(){
  Int numcoh=0;
  for (uInt k=0; k < uInt(msIter_p.numMS()) ; ++k){
    numcoh+=msIter_p.ms(k).nrow();
  }
  return numcoh;
  
}

template<class T>
void ROVisibilityIterator::getColScalar(const ROScalarColumn<T> &column, Vector<T> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
    return;
}

void ROVisibilityIterator::getCol(const ROScalarColumn<Bool> &column, Vector<Bool> &array, Bool resize) const
{
    getColScalar<Bool>(column, array, resize);
}

void ROVisibilityIterator::getCol(const ROScalarColumn<Int> &column, Vector<Int> &array, Bool resize) const
{
    getColScalar<Int>(column, array, resize);
}

void ROVisibilityIterator::getCol(const ROScalarColumn<Double> &column, Vector<Double> &array, Bool resize) const
{
    getColScalar<Double>(column, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Bool> &column, Array<Bool> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Float> &column, Array<Float> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
}

template<class T>
void ROVisibilityIterator::getColArray(const ROArrayColumn<T> &column, Array<T> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
    return;
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Double> &column, Array<Double> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Complex> &column, Array<Complex> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Bool> &column, const Slicer &slicer, Array<Bool> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, slicer, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Float> &column, const Slicer &slicer, Array<Float> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, slicer, array, resize);
}

void ROVisibilityIterator::getCol(const ROArrayColumn<Complex> &column, const Slicer &slicer, Array<Complex> &array, Bool resize) const
{
    column.getColumnCells(selRows_p, slicer, array, resize);
}

const Table
ROVisibilityIterator::attachTable() const
{
    return msIter_p.table();
}

void ROVisibilityIterator::slurp() const
{
    /* Set the table data manager (ISM and SSM) cache size to the full column size, for
       the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW
    */
  Record dmInfo(msIter_p.ms().dataManagerInfo());

  //  cout << "nfields = " << dmInfo.nfields() << endl;
  //  cout << "dminfo = " << dmInfo.description() << endl;
  RecordDesc desc = dmInfo.description();
  for (unsigned i = 0; i < dmInfo.nfields(); i++) {
      //      cout << "field " << i << " isSubRecord = " << desc.isSubRecord(i) << endl;
      //      cout << "field " << i << " isArray = " << desc.isArray(i) << endl;
      if (desc.isSubRecord(i)) {

          Record sub = dmInfo.subRecord(i);

          //          cout << "sub = " << sub << endl;
          if (sub.fieldNumber("NAME") >= 0 &&
              sub.fieldNumber("TYPE") >= 0 &&
              sub.fieldNumber("COLUMNS") >= 0 &&
              sub.type(sub.fieldNumber("NAME")) == TpString &&
              sub.type(sub.fieldNumber("TYPE")) == TpString &&
              sub.type(sub.fieldNumber("COLUMNS")) == TpArrayString) {
              
              Array<String> columns;
              dmInfo.subRecord(i).get("COLUMNS", columns);
              
              bool match = false;
              for (unsigned j = 0; j < columns.nelements(); j++) {
                  String column = columns(IPosition(1, j));
                  match |= (column == MS::columnName(MS::ANTENNA1) ||
                            column == MS::columnName(MS::ANTENNA2) ||
                            column == MS::columnName(MS::FEED1) ||
                            column == MS::columnName(MS::FEED2) ||
                            column == MS::columnName(MS::TIME) ||
                            column == MS::columnName(MS::INTERVAL) ||
                            column == MS::columnName(MS::FLAG_ROW) ||
                            column == MS::columnName(MS::SCAN_NUMBER) ||
                            column == MS::columnName(MS::UVW));
              }
              //              cout << "columns = " << columns << endl;
              
              if (match) {
                  
                  String dm_name;
                  dmInfo.subRecord(i).get("NAME", dm_name);
                  // cout << "dm_name = " << dm_name << endl;

                  String dm_type;
                  dmInfo.subRecord(i).get("TYPE", dm_type);
                  // cout << "dm_type = " << dm_type << endl;

                  Bool can_exceed_nr_buckets = False;
                  uInt num_buckets = msIter_p.ms().nrow(); 
                  // One bucket is at least one row, so this is enough

                  if (dm_type == "IncrementalStMan") {
                      ROIncrementalStManAccessor acc(msIter_p.ms(), dm_name);
                      acc.setCacheSize(num_buckets, can_exceed_nr_buckets);
                  }
                  else if (dm_type == "StandardStMan") {
                      ROStandardStManAccessor acc(msIter_p.ms(), dm_name);
                      acc.setCacheSize(num_buckets, can_exceed_nr_buckets);
                  }
                  /* These are the only storage managers which use the BucketCache
                     (and therefore are slow for random access and small cache sizes)
                   */
              }
              else {
                  String dm_name;
                  dmInfo.subRecord(i).get("NAME", dm_name);
                  //cout << "IGNORING...." << dm_name << endl;
              }
          }
          else {
              cerr << "Data manager info has unexpected shape! " << sub << endl;
          }
      }
  }
  return;
}

VisibilityIterator::VisibilityIterator() {}

VisibilityIterator::VisibilityIterator(MeasurementSet &MS, 
				       const Block<Int>& sortColumns, 
				       Double timeInterval)
 :ROVisibilityIterator(MS, sortColumns, timeInterval)
{
}

VisibilityIterator::VisibilityIterator(MeasurementSet &MS, 
				       const Block<Int>& sortColumns, const Bool addDefSort,
				       Double timeInterval)
 :ROVisibilityIterator(MS, sortColumns, addDefSort, timeInterval)
{
}
VisibilityIterator::VisibilityIterator(Block<MeasurementSet>& mss, 
				       const Block<Int>& sortColumns, 
				       Double timeInterval)
:ROVisibilityIterator(mss, sortColumns, timeInterval)
{
}
VisibilityIterator::VisibilityIterator(Block<MeasurementSet>& mss, 
				       const Block<Int>& sortColumns, const Bool addDefSort,
				       Double timeInterval)
 :ROVisibilityIterator(mss, sortColumns, addDefSort, timeInterval)
{
}

VisibilityIterator::VisibilityIterator(const VisibilityIterator & other)
    : ROVisibilityIterator()
{
    operator=(other);
}

VisibilityIterator::~VisibilityIterator() 
{
}

VisibilityIterator& 
VisibilityIterator::operator=(const VisibilityIterator& other)
{
    if (this!=&other) {
	ROVisibilityIterator::operator=(other);
	RWcolFlag.reference(other.RWcolFlag);
        RWcolFlagRow.reference(other.RWcolFlagRow);
	RWcolVis.reference(other.RWcolVis);
	RWcolFloatVis.reference(other.RWcolFloatVis);
	RWcolModelVis.reference(other.RWcolModelVis);
	RWcolCorrVis.reference(other.RWcolCorrVis);
	RWcolWeight.reference(other.RWcolWeight);
        RWcolWeightSpectrum.reference(other.RWcolWeightSpectrum);
	RWcolSigma.reference(other.RWcolSigma);
    }
    return *this;
}

VisibilityIterator & VisibilityIterator::operator++(int)
{
  if (!more_p) return *this;
  advance();
  return *this;
}

VisibilityIterator & VisibilityIterator::operator++()
{
  if (!more_p) return *this;
  advance();
  return *this;
}

void VisibilityIterator::attachColumns(const Table &t)
{
  ROVisibilityIterator::attachColumns(t);

  const ColumnDescSet& cds=t.tableDesc().columnDescSet();
  if (cds.isDefined(MS::columnName(MS::DATA))) {
    RWcolVis.attach(t, MS::columnName(MS::DATA));
  }
  if (cds.isDefined(MS::columnName(MS::FLOAT_DATA))) {
    floatDataFound_p=True;
    RWcolFloatVis.attach(t, MS::columnName(MS::FLOAT_DATA));
  } else {
    floatDataFound_p=False;
  }
  if (cds.isDefined("MODEL_DATA")) 
    RWcolModelVis.attach(t, "MODEL_DATA");
  if (cds.isDefined("CORRECTED_DATA")) 
    RWcolCorrVis.attach(t, "CORRECTED_DATA");
  RWcolWeight.attach(t, MS::columnName(MS::WEIGHT));
  if (cds.isDefined("WEIGHT_SPECTRUM"))
    RWcolWeightSpectrum.attach(t, "WEIGHT_SPECTRUM");
  RWcolSigma.attach(t, MS::columnName(MS::SIGMA));
  RWcolFlag.attach(t, MS::columnName(MS::FLAG));
  RWcolFlagRow.attach(t, MS::columnName(MS::FLAG_ROW));
}

void VisibilityIterator::setFlag(const Matrix<Bool>& flag)
{
  // use same value for all polarizations
  flagCube_p.resize(nPol_p,channelGroupSize_p,curNumRow_p);
  Bool deleteIt;
  Bool* p=flagCube_p.getStorage(deleteIt);
  const Bool* pflag=flag.getStorage(deleteIt);
  if (Int(flag.nrow())!=channelGroupSize_p) {
    throw(AipsError("VisIter::setFlag(flag) - inconsistent number of channels"));
  }
  
  for (uInt row=0; row<curNumRow_p; row++) {
    for (Int chn=0; chn<channelGroupSize_p; chn++) {
      for (Int pol=0; pol<nPol_p; pol++) {
	*p++=*pflag;
      }
      pflag++;
    }
  }
  if (useSlicer_p) putCol(RWcolFlag, slicer_p,flagCube_p);
  else putCol(RWcolFlag, flagCube_p);
}

void VisibilityIterator::setFlag(const Cube<Bool>& flags)
{
  if (useSlicer_p) putCol(RWcolFlag, slicer_p,flags);
  else putCol(RWcolFlag, flags);
}

void VisibilityIterator::setFlagRow(const Vector<Bool>& rowflags)
{
    putCol(RWcolFlagRow, rowflags);
}

void VisibilityIterator::setVis(const Matrix<CStokesVector> & vis,
				DataColumn whichOne)
{
  // two problems: 1. channel selection -> we can only write to reference
  // MS with 'processed' channels
  //               2. polarization: there could be 1, 2 or 4 in the
  // original data, predict() always gives us 4. We save what was there
  // originally.

  //  if (!preselected_p) {
  //    throw(AipsError("VisIter::setVis(vis) - cannot change original data"));
  //  }
  if (Int(vis.nrow())!=channelGroupSize_p) {
    throw(AipsError("VisIter::setVis(vis) - inconsistent number of channels"));
  }
  // we need to reform the vis matrix to a cube before we can use
  // putColumn to a Matrix column
  visCube_p.resize(nPol_p,channelGroupSize_p,curNumRow_p);
  Bool deleteIt;
  Complex* p=visCube_p.getStorage(deleteIt);
  for (uInt row=0; row<curNumRow_p; row++) {
    for (Int chn=0; chn<channelGroupSize_p; chn++) {
      const CStokesVector& v=vis(chn,row);
      switch (nPol_p) {
      case 4: *p++=v(0); *p++=v(1); *p++=v(2); *p++=v(3); break;     
      case 2: *p++=v(0); *p++=v(3); break;
      case 1: *p++=(v(0)+v(3))/2; break;
      }
    }
  }
  if (useSlicer_p) putDataColumn(whichOne,slicer_p,visCube_p);
  else putDataColumn(whichOne,visCube_p);
}

void VisibilityIterator::setVisAndFlag(const Cube<Complex>& vis,
				       const Cube<Bool>& flag,
				       DataColumn whichOne)
{
  if (velSelection_p) {
    setInterpolatedVisFlag(vis,flag);
    if (useSlicer_p) putDataColumn(whichOne,slicer_p,visCube_p);
    else putDataColumn(whichOne,visCube_p);
    if (useSlicer_p) putCol(RWcolFlag, slicer_p,flagCube_p);
    else putCol(RWcolFlag, flagCube_p);
  } else {
    if (useSlicer_p) putDataColumn(whichOne,slicer_p,vis);
    else putDataColumn(whichOne,vis);
    if (useSlicer_p) putCol(RWcolFlag, slicer_p,flag);
    else putCol(RWcolFlag, flag);
  }
}

void VisibilityIterator::setVis(const Cube<Complex>& vis, DataColumn whichOne)
{
  if (velSelection_p) {
    setInterpolatedVisFlag(vis,flagCube_p);
    if (useSlicer_p) putDataColumn(whichOne,slicer_p,visCube_p);
    else putDataColumn(whichOne,visCube_p);
  } else {
    if (useSlicer_p) putDataColumn(whichOne,slicer_p,vis);
    else putDataColumn(whichOne,vis);
  }
}

void VisibilityIterator::setWeight(const Vector<Float>& weight)
{
  // No polarization dependence for now
  Matrix<Float> polWeight;
  getCol(colWeight, polWeight);
  for (Int i=0; i<nPol_p; i++) {
    Vector<Float> r=polWeight.row(i);
    r=weight;
  }
  putCol(RWcolWeight, polWeight);
}

void VisibilityIterator::setWeightMat(const Matrix<Float>& weightMat)
{
  putCol(RWcolWeight, weightMat);
}

void VisibilityIterator::setWeightSpectrum(const Cube<Float>& weightSpectrum)
{
  if (!colWeightSpectrum.isNull()) {
    putCol(RWcolWeightSpectrum, weightSpectrum);
  }
}

void VisibilityIterator::setSigma(const Vector<Float>& sigma)
{
  Matrix<Float> sigmat;
  getCol(colSigma, sigmat);
  for (Int i=0; i < nPol_p; i++) {
    Vector<Float> r = sigmat.row(i);
    r = sigma;
  }
  putCol(RWcolSigma, sigmat);
}

void VisibilityIterator::setSigmaMat(const Matrix<Float>& sigMat)
{
  putCol(RWcolSigma, sigMat);
}

void VisibilityIterator::setInterpolatedVisFlag(const Cube<Complex>& vis, 
						const Cube<Bool>& flag)
{
  // get the frequencies to interpolate to
  velSelection_p = False; 
  Vector<Double> freq; frequency(freq);
  velSelection_p = True;

  // now interpolate visibilities, using freq as the sample points
  // we should have two options: flagging output points that have
  // any flagged inputs or interpolating across flagged data.
  // Convert frequencies to float (removing offset to keep accuracy) 
  // so we can multiply them with Complex numbers to do the interpolation.
  Block<Float> xfreq(channelGroupSize_p),sfreq(nVelChan_p); 
  Int i;
  for (i=0; i<channelGroupSize_p; i++) xfreq[i]=freq(i)-freq(0);
  for (i=0; i<nVelChan_p; i++) sfreq[i]=selFreq_p(i)-freq(0);
  // set up the Functionals for the interpolation
  Cube<Complex> swapVis,intVis;
  swapyz(swapVis,vis);
  Cube<Bool> swapFlag,intFlag;
  swapyz(swapFlag,flag);
  InterpolateArray1D<Float,Complex>::InterpolationMethod method1=
    InterpolateArray1D<Float,Complex>::linear;
  if (vInterpolation_p=="nearest") {
    method1=InterpolateArray1D<Float,Complex>::nearestNeighbour;
  }
  InterpolateArray1D<Float,Complex>::
    interpolate(intVis,intFlag,xfreq,sfreq,swapVis,swapFlag,method1);
  swapyz(visCube_p,intVis);
  swapyz(flagCube_p,intFlag);
}



void VisibilityIterator::setInterpolatedWeight(const Matrix<Float>& wt)
{
  // get the frequencies to interpolate to
  velSelection_p = False; 
  Vector<Double> freq; frequency(freq);
  velSelection_p = True;

  // now interpolate weights, using freq as the sample points
  // we should have two options: flagging output points that have
  // any flagged inputs or interpolating across flagged data.
  // Convert frequencies to float (removing offset to keep accuracy) 
  // so we can multiply them with Complex numbers to do the interpolation.
  Block<Float> xfreq(channelGroupSize_p),sfreq(nVelChan_p); 
  Int i;
  for (i=0; i<channelGroupSize_p; i++) xfreq[i]=freq(i)-freq(0);
  for (i=0; i<nVelChan_p; i++) sfreq[i]=selFreq_p(i)-freq(0);
  // set up the Functionals for the interpolation
  Matrix<Float> twt,intWt;
  transpose(twt,wt);
  InterpolateArray1D<Float,Float>::InterpolationMethod method2=
    InterpolateArray1D<Float,Float>::linear;
  if (vInterpolation_p=="nearest") {
    method2= InterpolateArray1D<Float,Float>::nearestNeighbour;
  }
  InterpolateArray1D<Float,Float>::
    interpolate(intWt,xfreq,sfreq,twt,method2);
  transpose(imagingWeight_p,intWt);
}

void VisibilityIterator::putDataColumn(DataColumn whichOne,
				       const Slicer& slicer,
				       const Cube<Complex>& data)
{
  // Set the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.


  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat=real(data);
      putCol(RWcolFloatVis, slicer,dataFloat);
    } else {
      putCol(RWcolVis, slicer,data);
    }
    break;
  case Corrected:
    putCol(RWcolCorrVis, slicer,data);
    break;
  case Model:
    putCol(RWcolModelVis, slicer,data);
    break;
  }
}  

void VisibilityIterator::putDataColumn(DataColumn whichOne,
				       const Cube<Complex>& data)
{
  // Set the visibility (observed, model or corrected);
  // deal with DATA and FLOAT_DATA seamlessly for observed data.
  switch (whichOne) {
  case Observed:
    if (floatDataFound_p) {
      Cube<Float> dataFloat=real(data);
      putCol(RWcolFloatVis, dataFloat);
    } else {
      putCol(RWcolVis, data);
    }
    break;
  case Corrected:
    putCol(RWcolCorrVis, data);
    break;
  case Model:
    putCol(RWcolModelVis, data);
    break;
  }
}  

void VisibilityIterator::putColScalar(ScalarColumn<Bool> &column, const Vector<Bool> &array)
{
    column.putColumnCells(selRows_p, array);
    return;
}

void VisibilityIterator::putCol(ScalarColumn<Bool> &column, const Vector<Bool> &array)
{
    putColScalar(column, array);
}

void VisibilityIterator::putCol(ArrayColumn<Bool> &column, const Array<Bool> &array)
{
    column.putColumnCells(selRows_p, array);
}

void VisibilityIterator::putCol(ArrayColumn<Float> &column, const Array<Float> &array)
{
    column.putColumnCells(selRows_p, array);
}

void VisibilityIterator::putCol(ArrayColumn<Complex> &column, const Array<Complex> &array)
{
    column.putColumnCells(selRows_p, array);
}

void VisibilityIterator::putCol(ArrayColumn<Bool> &column, const Slicer &slicer, const Array<Bool> &array)
{
    column.putColumnCells(selRows_p, slicer, array);
}

void VisibilityIterator::putCol(ArrayColumn<Float> &column, const Slicer &slicer, const Array<Float> &array)
{
    column.putColumnCells(selRows_p, slicer, array);
}

void VisibilityIterator::putCol(ArrayColumn<Complex> &column, const Slicer &slicer, const Array<Complex> &array)
{
    column.putColumnCells(selRows_p, slicer, array);
}


ROVisibilityIterator::AsyncEnabler::AsyncEnabler (ROVisibilityIterator & rovi)
: roVisibilityIterator_p (& rovi)
{
    oldEnabledState_p = roVisibilityIterator_p->isAsyncEnabled();
    roVisibilityIterator_p->setAsyncEnabled (True);
}

ROVisibilityIterator::AsyncEnabler::AsyncEnabler (ROVisibilityIterator * rovi)
: roVisibilityIterator_p (rovi)
{
    oldEnabledState_p = roVisibilityIterator_p->isAsyncEnabled();
    roVisibilityIterator_p->setAsyncEnabled (True);
}

ROVisibilityIterator::AsyncEnabler::~AsyncEnabler ()
{
    release ();
}

void
ROVisibilityIterator::AsyncEnabler::release ()
{
    roVisibilityIterator_p->setAsyncEnabled (oldEnabledState_p);
}

} //# NAMESPACE CASA - END



