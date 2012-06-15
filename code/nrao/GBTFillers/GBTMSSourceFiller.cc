//# GBTMSSourceFiller.cc: GBTMSSourceFiller fills the MSSource table for GBT fillers
//# Copyright (C) 2000,2001,2003
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

#include <nrao/GBTFillers/GBTMSSourceFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSSource.h>
#include <ms/MeasurementSets/MSSourceColumns.h>
#include <tables/Tables/ColumnsIndex.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

GBTMSSourceFiller::GBTMSSourceFiller()
    : msSource_p(0), msSourceCols_p(0), sourceId_p(-1), theCache_p(0),
      cacheIndx_p(0), sourceIndx_p(0)
{;}


GBTMSSourceFiller::GBTMSSourceFiller(MSSource &msSource)
    : msSource_p(0), msSourceCols_p(0), sourceId_p(-1), theCache_p(0),
      cacheIndx_p(0), sourceIndx_p(0)
{init(msSource);}

GBTMSSourceFiller::~GBTMSSourceFiller()
{
    cleanup();
}

void GBTMSSourceFiller::attach(MSSource &msSource)
{init(msSource);}

void GBTMSSourceFiller::fill(const Vector<Int> &spectralWindowIds,
			     const String &name, Double restFrequency,
			     Double sysvel, const String &code,
			     Double time, Double interval,
			     Int calibrationGroup,
			     const Vector<Double> &direction,
			     const Vector<Double> &position)
{
    // set up the index
    *nameKey_p = name;
    *codeKey_p = code;
    *timeKey_p = time;
    *intervalKey_p = interval;
    *restFreqKey_p = restFrequency;
    *sysvelKey_p = sysvel;
    *calGroupKey_p = calibrationGroup;
    *dirKey0_p = direction(0);
    *dirKey1_p = direction(1);
    *posKey0_p = position(0);
    *posKey1_p = position(1);
    *posKey2_p = position(2);

    // find the unique spectral window Ids
    // there's probably a better way to do this
    Int count = 0;
    SimpleOrderedMap<Int, Int> uniqueMap(-1);
    for (uInt i=0;i<spectralWindowIds.nelements();i++) {
	Int val = spectralWindowIds(i);
	if (!uniqueMap.isDefined(val)) {
	    uniqueMap(val) = count;
	    count++;
	}
    }

    // is it indexed yet
    Bool found = False;

    Vector<Bool> newRow(uniqueMap.ndefined(), True);
    Int newRows = 0;
    uInt rownr = cacheIndx_p->getRowNumber(found);
    if (!found) {
	// add it in
	rownr = theCache_p->nrow();
	theCache_p->addRow(1);
	nameCol_p.put(rownr, *nameKey_p);
	codeCol_p.put(rownr, *codeKey_p);
	restFreqCol_p.put(rownr, *restFreqKey_p);
	sysvelCol_p.put(rownr, *sysvelKey_p);
	timeCol_p.put(rownr, *timeKey_p);
	intervalCol_p.put(rownr, *intervalKey_p);
	calGroupCol_p.put(rownr, *calGroupKey_p);
	dirCol0_p.put(rownr, *dirKey0_p);
	dirCol1_p.put(rownr, *dirKey1_p);
	posCol0_p.put(rownr, *posKey0_p);
	posCol1_p.put(rownr, *posKey1_p);
	posCol2_p.put(rownr, *posKey2_p);
	// the ID is the rownr
	sourceId_p = Int(rownr);
	// this always gets new rows in the output table, but only one for
	// each unique spectralWindowId
	newRow = True;
	newRows = newRow.nelements();
    } else {
	// the ID is the rownr
	sourceId_p = Int(rownr);
	// do we know about each of spectral window Id and sourceId combinations
	*sourceIdKey_p = sourceId_p;
	for (uInt i=0;i<uniqueMap.ndefined();i++) {
	    *spWinIdKey_p = uniqueMap.getKey(i);
	    sourceIndx_p->getRowNumber(found);
	    newRow(i) = !found;
	    if (!found) newRows++;
	}
    }
    // add anything to the source table that needs to be added
    if (newRows > 0) {
	uInt thisRow = msSource_p->nrow();
	msSource_p->addRow(newRows);
	for (uInt i=0;i<newRow.nelements();i++) {
	    if (newRow(i)) {
		msSourceCols_p->calibrationGroup().put(thisRow, calibrationGroup);
		msSourceCols_p->code().put(thisRow, code);
		msSourceCols_p->direction().put(thisRow, direction);
		msSourceCols_p->interval().put(thisRow, interval);
		msSourceCols_p->name().put(thisRow, name);
		msSourceCols_p->position().put(thisRow, position);
		msSourceCols_p->sourceId().put(thisRow, sourceId_p);
		msSourceCols_p->spectralWindowId().put(thisRow, uniqueMap.getKey(i));
		msSourceCols_p->time().put(thisRow, time);
		// check first to see that it has a REST_FREQUENCY column or
		// a SYSVEL column
		if (!(msSourceCols_p->restFrequency().isNull()) ||
		    !(msSourceCols_p->sysvel().isNull()))
		{
		    msSourceCols_p->numLines().put(thisRow, 1);
		    if (!(msSourceCols_p->restFrequency().isNull())) {
			msSourceCols_p->restFrequency().
			    put(thisRow, Vector<Double>(1,restFrequency));
		    }
		    if (!(msSourceCols_p->sysvel().isNull())) {
			msSourceCols_p->sysvel().
			    put(thisRow, Vector<Double>(1,sysvel));
		    }
		}
		thisRow++;
	    }
	}
    }
}

void GBTMSSourceFiller::init(MSSource &msSource)
{
    cleanup();

    msSource_p = new MSSource(msSource);
    AlwaysAssert(msSource_p, AipsError);

    msSourceCols_p = new MSSourceColumns(msSource);
    AlwaysAssert(msSourceCols_p, AipsError);

    // construct the temporary cache table
    TableDesc td;
    td.addColumn(ScalarColumnDesc<Int> ("calGroup"));
    td.addColumn(ScalarColumnDesc<String> ("code"));
    td.addColumn(ScalarColumnDesc<String> ("name"));
    td.addColumn(ScalarColumnDesc<Double> ("time"));
    td.addColumn(ScalarColumnDesc<Double> ("interval"));
    td.addColumn(ScalarColumnDesc<Double> ("restfreq"));
    td.addColumn(ScalarColumnDesc<Double> ("sysvel"));
    td.addColumn(ScalarColumnDesc<Double> ("dir0"));
    td.addColumn(ScalarColumnDesc<Double> ("dir1"));
    td.addColumn(ScalarColumnDesc<Double> ("pos0"));
    td.addColumn(ScalarColumnDesc<Double> ("pos1"));
    td.addColumn(ScalarColumnDesc<Double> ("pos2"));

    SetupNewTable newTab("", td, Table::Scratch);
    theCache_p = new Table(newTab, GBTBackendFiller::tableLock());
    AlwaysAssert(theCache_p, AipsError);

    // attach the columns
    calGroupCol_p.attach(*theCache_p, "calGroup");
    codeCol_p.attach(*theCache_p, "code");
    nameCol_p.attach(*theCache_p, "name");
    timeCol_p.attach(*theCache_p, "time");
    intervalCol_p.attach(*theCache_p, "interval");
    restFreqCol_p.attach(*theCache_p, "restfreq");
    sysvelCol_p.attach(*theCache_p, "sysvel");
    dirCol0_p.attach(*theCache_p, "dir0");
    dirCol1_p.attach(*theCache_p, "dir1");
    posCol0_p.attach(*theCache_p, "pos0");
    posCol1_p.attach(*theCache_p, "pos1");
    posCol2_p.attach(*theCache_p, "pos2");

    // fill the columns for anything already in the source table
    Vector<Double> twoElVect(2);
    Vector<Double> threeElVect(3);
    for (uInt i=0;i<msSource_p->nrow();i++) {
	Int thisId = msSourceCols_p->sourceId()(i);
	if (thisId >= Int(theCache_p->nrow())) {
	    // normally, this table will have been filled by this filler 
	    // and hence the source IDs come in order - when debugging make sure that is true
	    DebugAssert(thisId == Int(theCache_p->nrow()), AipsError);
	    theCache_p->addRow(1);
	    calGroupCol_p.put(thisId,msSourceCols_p->calibrationGroup()(i));
	    codeCol_p.put(thisId,msSourceCols_p->code()(i));
	    nameCol_p.put(thisId,msSourceCols_p->name()(i));
	    timeCol_p.put(thisId,msSourceCols_p->time()(i));
	    intervalCol_p.put(thisId,msSourceCols_p->interval()(i));
	    Vector<Double> rf;
	    if (!(msSourceCols_p->restFrequency().isNull())) {
		rf = msSourceCols_p->restFrequency()(i);
	    
	    }
	    if (rf.nelements()>0) {
		restFreqCol_p.put(thisId,rf(0));
	    } else {
		restFreqCol_p.put(thisId, 0.0);
	    }
	    Vector<Double> sv;
	    if (!(msSourceCols_p->sysvel().isNull())) {
		sv = msSourceCols_p->sysvel()(i);
	    }
	    if (sv.nelements()>0) {
		sysvelCol_p.put(thisId, sv(0));
	    } else {
		sysvelCol_p.put(thisId, 0.0);
	    }
	    twoElVect = msSourceCols_p->direction()(i);
	    dirCol0_p.put(thisId,twoElVect(0));
	    dirCol1_p.put(thisId,twoElVect(1));
	    threeElVect = msSourceCols_p->position()(i);
	    posCol0_p.put(thisId,threeElVect(0));
	    posCol1_p.put(thisId,threeElVect(1));
	    posCol2_p.put(thisId,threeElVect(2));
	}
    }
 
    // construct the cache index
    cacheIndx_p = new ColumnsIndex(*theCache_p,
				   stringToVector("calGroup,code,name,time,interval,restfreq,sysvel,dir0,dir1,pos0,pos1,pos2"));
    AlwaysAssert(cacheIndx_p, AipsError);
    // and attach the keys
    calGroupKey_p.attachToRecord(cacheIndx_p->accessKey(),"calGroup");
    codeKey_p.attachToRecord(cacheIndx_p->accessKey(),"code");
    nameKey_p.attachToRecord(cacheIndx_p->accessKey(),"name");
    timeKey_p.attachToRecord(cacheIndx_p->accessKey(),"time");
    intervalKey_p.attachToRecord(cacheIndx_p->accessKey(),"interval");
    restFreqKey_p.attachToRecord(cacheIndx_p->accessKey(),"restfreq");
    sysvelKey_p.attachToRecord(cacheIndx_p->accessKey(),"sysvel");
    dirKey0_p.attachToRecord(cacheIndx_p->accessKey(),"dir0");
    dirKey1_p.attachToRecord(cacheIndx_p->accessKey(),"dir1");
    posKey0_p.attachToRecord(cacheIndx_p->accessKey(),"pos0");
    posKey1_p.attachToRecord(cacheIndx_p->accessKey(),"pos1");
    posKey2_p.attachToRecord(cacheIndx_p->accessKey(),"pos2");

    // and the index to the MSSource table
    String sourceIndexCols = MSSource::columnName(MSSource::SOURCE_ID);
    sourceIndexCols += ",";
    sourceIndexCols += MSSource::columnName(MSSource::SPECTRAL_WINDOW_ID);
    sourceIndx_p = new ColumnsIndex(*msSource_p, stringToVector(sourceIndexCols));
    AlwaysAssert(sourceIndx_p, AipsError);
    // and attach its keys
    sourceIdKey_p.attachToRecord(sourceIndx_p->accessKey(), MSSource::columnName(MSSource::SOURCE_ID));
    spWinIdKey_p.attachToRecord(sourceIndx_p->accessKey(), MSSource::columnName(MSSource::SPECTRAL_WINDOW_ID));
}

void GBTMSSourceFiller::cleanup()
{
    delete msSource_p;
    msSource_p = 0;

    delete msSourceCols_p;
    msSourceCols_p = 0;

    delete theCache_p;
    theCache_p = 0;

    delete cacheIndx_p;
    cacheIndx_p = 0;

    delete sourceIndx_p;
    sourceIndx_p = 0;
}
