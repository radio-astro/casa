//# GBTMSSpecWinFiller.cc: fills the MSSpectralWindow table for GBT fillers
//# Copyright (C) 2000,2001,2002,2003
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

//# Includes

#include <nrao/GBTFillers/GBTMSSpecWinFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicMath/Math.h>
#include <measures/Measures/Stokes.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/DataType.h>

GBTMSSpecWinFiller::GBTMSSpecWinFiller()
    : msSpecWin_p(0), msSpecWinCols_p(0), nfreq_p(0),
      theCache_p(0), cacheIndx_p(0), nextCacheRow_p(0), cacheSize_p(100)
{;}


GBTMSSpecWinFiller::GBTMSSpecWinFiller(MSSpectralWindow &msSpecWin)
    : msSpecWin_p(0), msSpecWinCols_p(0), nfreq_p(0),
      theCache_p(0), cacheIndx_p(0), nextCacheRow_p(0), cacheSize_p(100)
{init(msSpecWin);}

GBTMSSpecWinFiller::~GBTMSSpecWinFiller()
{
    delete msSpecWin_p;
    msSpecWin_p = 0;

    delete msSpecWinCols_p;
    msSpecWinCols_p = 0;

    delete theCache_p;
    theCache_p = 0;

    delete cacheIndx_p;
    cacheIndx_p = 0;
}

void GBTMSSpecWinFiller::attach(MSSpectralWindow &msSpecWin)
{init(msSpecWin);}

Int GBTMSSpecWinFiller::fill(uInt nfreq, Double centerFreq, Double centerChan,
			     Double chanWidth, MFrequency::Types restFrame,
			     Double tolerance, const Vector<Int> &exclude)
{
    // have we seen this one yet
    *nfreqKey_p = nfreq;
    *restFrameKey_p = Int(restFrame);
    *refchanKey_p = centerChan;
    specWinIds_p.resize(1);
    Double bw=abs(chanWidth)*nfreq;
    // there may be several matches
    Vector<uInt> rows(cacheIndx_p->getRowNumbers());
    Bool found = False;
    uInt rownr = 0;
    Int specWin = -1;
    // look for first obsfreqCacheCol_p value near centerFreq in found rows
    for (uInt i=0;i<rows.nelements();i++) {
	rownr = rows(i);
	if (!anyEQ(exclude, idCacheCol_p.asInt(rownr)) &&
	    nearAbs(obsfreqCacheCol_p.asdouble(rownr),centerFreq,tolerance) &&
	    nearAbs(freqresCacheCol_p.asdouble(rownr),chanWidth,tolerance) &&
	    nearAbs(bwCacheCol_p.asdouble(rownr),bw,tolerance))

	    {
		found = True;
		break;
	    }
    }
    if (found) {
	// get the ID for this row
	specWin = idCacheCol_p.asInt(rownr);
	// double check that its not in exclude
    } else {
	// add a new ID
	specWin = newSpecWin(*nfreqKey_p, centerFreq, chanWidth, 
			     *refchanKey_p, bw, *restFrameKey_p);
    }
    nfreq_p = nfreq;
    specWinIds_p = specWin;
    return specWin;
}

void GBTMSSpecWinFiller::fill(uInt nreceivers, uInt nfreq)
{
    // set the keys in the index record
    *nfreqKey_p = nfreq;
    *restFrameKey_p = Int(MFrequency::TOPO);
    *refchanKey_p = 0.0;
    specWinIds_p.resize(nreceivers);
    // this kind can only match once
    Bool found;
    uInt rownr = cacheIndx_p->getRowNumber(found);
    Int specWin = -1;
    if (found) {
	// get the ID for this row
	specWin = idCacheCol_p.asInt(rownr);
    } else {
	// add a new ID
	specWin = newSpecWin(nfreq, 0.0, 0.0, 
			     *refchanKey_p, 0.0, *restFrameKey_p);
    }
    nfreq_p = nfreq;
    specWinIds_p = specWin;
}

Int GBTMSSpecWinFiller::spectralWindowId(uInt whichReceiver) const {
    Int result = -1;
    if (Int(whichReceiver) < nrec()) {
	result = specWinIds_p(whichReceiver);
    }
    return result;
}

void GBTMSSpecWinFiller::setCacheSize(uInt newCacheSize) {
    // just start over - possibly wasteful, but this shouldn't be called very
    // often, if ever, except by init()
    delete cacheIndx_p;
    cacheIndx_p = 0;

    delete theCache_p;
    theCache_p = 0;

    cacheSize_p = newCacheSize;
    nextCacheRow_p = 0;

    // create the temporary cache table
    TableDesc td;
    td.addColumn(ScalarColumnDesc<Int> ("ID"));
    td.addColumn(ScalarColumnDesc<Int> ("NFREQ"));
    td.addColumn(ScalarColumnDesc<Int> ("RESTFRAME"));
    td.addColumn(ScalarColumnDesc<Double> ("OBSFREQ"));
    td.addColumn(ScalarColumnDesc<Double> ("FREQRES"));
    td.addColumn(ScalarColumnDesc<Double> ("REFCHAN"));
    td.addColumn(ScalarColumnDesc<Double> ("BW"));
    SetupNewTable newTab("", td, Table::Scratch);
    theCache_p = new Table(newTab, GBTBackendFiller::tableLock());
    AlwaysAssert(theCache_p, AipsError);

    // and attach the columns
    idCacheCol_p.attach(*theCache_p,"ID");
    nfreqCacheCol_p.attach(*theCache_p,"NFREQ");
    restFrameCacheCol_p.attach(*theCache_p,"RESTFRAME");
    obsfreqCacheCol_p.attach(*theCache_p,"OBSFREQ");
    freqresCacheCol_p.attach(*theCache_p,"FREQRES");
    refchanCacheCol_p.attach(*theCache_p,"REFCHAN");
    bwCacheCol_p.attach(*theCache_p,"BW");

    // suck any existing SPECTRAL_WINDOW table into the cache
    uInt rowsToAdd = min(msSpecWin_p->nrow(), cacheSize_p);
    // this may be too innefficient - one cell at a time
    uInt rowStart = 0;
    if (rowsToAdd == cacheSize_p) {
	rowStart = msSpecWin_p->nrow()-cacheSize_p;
    } 
    // add the rows
    theCache_p->addRow(rowsToAdd);
    // insert the values
    for (uInt i=rowStart; i<(rowStart+rowsToAdd); i++) {
	idCacheCol_p.putScalar(nextCacheRow_p, Int(i));
	nfreqCacheCol_p.putScalar(nextCacheRow_p, msSpecWinCols_p->numChan()(i));
	Double obsfreq = msSpecWinCols_p->refFrequency()(i);
	obsfreqCacheCol_p.putScalar(nextCacheRow_p, obsfreq);
	bwCacheCol_p.putScalar(nextCacheRow_p, msSpecWinCols_p->totalBandwidth()(i));
	Vector<Double> chanFreq(msSpecWinCols_p->chanFreq()(i));
	Vector<Double> resolution(msSpecWinCols_p->resolution()(i));
	if (resolution.nelements() > 0 && chanFreq.nelements() > 0) {
	    Double freqRes = resolution(0);
	    // freqres here also includes the sign
	    if (chanFreq.nelements() > 1 && chanFreq[1] < chanFreq[0]) freqRes *= -1.0;
	    freqresCacheCol_p.putScalar(nextCacheRow_p, freqRes);
	    // this may be losy
	    refchanCacheCol_p.putScalar(nextCacheRow_p, (obsfreq - chanFreq(0))/freqRes);
	} else {
	    freqresCacheCol_p.putScalar(nextCacheRow_p, 0.0);
	    refchanCacheCol_p.putScalar(nextCacheRow_p, 0.0);
	}
	restFrameCacheCol_p.putScalar(nextCacheRow_p, msSpecWinCols_p->measFreqRef()(i));
	nextCacheRow_p++;
    }

    // and create the cache index
    cacheIndx_p = new ColumnsIndex(*theCache_p, 
				   stringToVector("NFREQ,RESTFRAME,REFCHAN"));
    AlwaysAssert(cacheIndx_p, AipsError);
    // and attach the key field pointers
    nfreqKey_p.attachToRecord(cacheIndx_p->accessKey(),"NFREQ");
    restFrameKey_p.attachToRecord(cacheIndx_p->accessKey(), "RESTFRAME");
    refchanKey_p.attachToRecord(cacheIndx_p->accessKey(),"REFCHAN");
}

void GBTMSSpecWinFiller::init(MSSpectralWindow &msSpecWin)
{
    msSpecWin_p = new MSSpectralWindow(msSpecWin);
    AlwaysAssert(msSpecWin_p, AipsError);

    msSpecWinCols_p = new MSSpWindowColumns(msSpecWin);
    AlwaysAssert(msSpecWinCols_p, AipsError);

    setCacheSize(cacheSize_p);

    nfreq_p = 0;
    specWinIds_p.resize(0);
}

Int GBTMSSpecWinFiller::newSpecWin(Int nfreq, Double obsfreq, Double freqres, 
				   Double refchan, Double bw, Int restframe)
{
    Int rownr = msSpecWin_p->nrow();
    msSpecWin_p->addRow();
    msSpecWinCols_p->numChan().put(rownr, nfreq);
    msSpecWinCols_p->name().put(rownr,"");
    // we need to be able to set the frequency reference code here, eventually
    msSpecWinCols_p->refFrequency().putScalar(rownr, obsfreq);
    Vector<Double> chanFreq(nfreq);
    Vector<Double> resolution(nfreq);
    resolution = abs(freqres);
    indgen(chanFreq);
    chanFreq = (chanFreq - refchan)*freqres + obsfreq;
    msSpecWinCols_p->chanFreq().put(rownr, chanFreq);
    msSpecWinCols_p->measFreqRef().put(rownr, restframe);
    // assume that the width, effectiveBW and resolution are the same
    msSpecWinCols_p->chanWidth().put(rownr, resolution);
    msSpecWinCols_p->effectiveBW().put(rownr, resolution);
    msSpecWinCols_p->resolution().put(rownr, resolution);
    msSpecWinCols_p->totalBandwidth().putScalar(rownr, bw);
    // I have no idea where we'll get this from
    msSpecWinCols_p->netSideband().put(rownr, 0);
    msSpecWinCols_p->ifConvChain().putScalar(rownr, -1);
    // no group's yet
    msSpecWinCols_p->freqGroup().put(rownr,-1);
    msSpecWinCols_p->freqGroupName().put(rownr,"");
    msSpecWinCols_p->flagRow().put(rownr,False);

    // and add this to the cache
    Bool updateIndex = False;
    if (nextCacheRow_p >= cacheSize_p) nextCacheRow_p = 0;
    if (nextCacheRow_p >= theCache_p->nrow()) {
	// we're still adding rows - the index can keep up with this just fine
	theCache_p->addRow();
    } else {
	// we're reusing rows - tell the index when things have changed
	updateIndex = True;
    }
    idCacheCol_p.putScalar(nextCacheRow_p, Int(rownr));
    nfreqCacheCol_p.putScalar(nextCacheRow_p, nfreq);
    obsfreqCacheCol_p.putScalar(nextCacheRow_p, obsfreq);
    bwCacheCol_p.putScalar(nextCacheRow_p, bw);
    freqresCacheCol_p.putScalar(nextCacheRow_p, freqres);
    refchanCacheCol_p.putScalar(nextCacheRow_p, refchan);
    restFrameCacheCol_p.putScalar(nextCacheRow_p, restframe);

    nextCacheRow_p++;

    if (updateIndex) cacheIndx_p->setChanged();
    
    return rownr;
}
