//# GBTMSDataDescFiller.cc: GBTMSDataDescFiller fills the MSDataDescription table for GBT fillers
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

#include <nrao/GBTFillers/GBTMSDataDescFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Arrays/ArrayUtil.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/Assert.h>

GBTMSDataDescFiller::GBTMSDataDescFiller()
    : msDataDesc_p(0), msDataDescCols_p(0), theCache_p(0), cacheIndx_p(0),
      nextCacheRow_p(0), cacheSize_p(1000)
{;}


GBTMSDataDescFiller::GBTMSDataDescFiller(MSDataDescription &msDataDesc)
    : msDataDesc_p(0), msDataDescCols_p(0), theCache_p(0), cacheIndx_p(0),
      nextCacheRow_p(0), cacheSize_p(1000)
{init(msDataDesc);}

GBTMSDataDescFiller::~GBTMSDataDescFiller()
{
    delete msDataDesc_p;
    msDataDesc_p = 0;

    delete msDataDescCols_p;
    msDataDescCols_p = 0;

    delete theCache_p;
    theCache_p = 0;

    delete cacheIndx_p;
    cacheIndx_p = 0;
}

void GBTMSDataDescFiller::attach(MSDataDescription &msDataDesc)
{init(msDataDesc);}

void GBTMSDataDescFiller::fill(Int polarizationId, const Vector<Int> &specWinIds, 
			       const Vector<Int> &receiverIds)
{
    Bool useDefaultRecId = receiverIds.nelements() == 0;

    // receiverIds should be the same length as specWinIds - only check when debugging
    DebugAssert(useDefaultRecId || receiverIds.nelements() == specWinIds.nelements(), AipsError);
    
    // now find one for each spectral window id, receiverId combination
    uInt idCount = 0;
    if (specWinIds.nelements() > 0) {
	// we will need at most specWinIds.nelements() ids
	receiverMap_p.resize(specWinIds.nelements(), True, False);
	dataDescIds_p.resize(specWinIds.nelements());
	dataDescIds_p = -1;
	for (uInt i=0;i<specWinIds.nelements();i++) {
	    Int recId = -1;
	    if (!useDefaultRecId) recId = receiverIds(i);
	    Int thisId = checkAndFill(polarizationId, specWinIds(i), recId);
	    // have we seen this ID yet
	    uInt j;
	    for (j=0;j<idCount;j++) {
		if (thisId == dataDescIds_p(j)) break;
	    }
	    if (j < idCount) {
		// yes, reuse it
		receiverMap_p[i] = j;
	    } else {
		// no, use the next one
		dataDescIds_p(idCount) = thisId;
		receiverMap_p[i] = idCount;
		idCount++;
	    }
	}
    } else {
	idCount = 1;
	// we will need at one DATA_DESC_ID
	receiverMap_p.resize(1, True, False);
	dataDescIds_p.resize(1);
	dataDescIds_p(0) = checkAndFill(polarizationId, -1, -1);
	receiverMap_p[0] = 0;
    }
    dataDescIds_p.resize(idCount,True);
}

Int GBTMSDataDescFiller::fill(Int polarizationId, Int specWinId) 
{
    receiverMap_p.resize(1, True, False);
    dataDescIds_p.resize(1);
    Int itsId = checkAndFill(polarizationId, specWinId, 0);
    receiverMap_p[0] = 0;
    dataDescIds_p(0) = itsId;
    dataDescIds_p.resize(1, True);
    return itsId;
}

Int GBTMSDataDescFiller::dataDescriptionId(uInt whichReceiver) const {
    Int result = -1;
    if (Int(whichReceiver) < nrec()) {
	result = dataDescIds_p(receiverMap_p[whichReceiver]);
    }
    return result;
}

void GBTMSDataDescFiller::setCacheSize(uInt newCacheSize) {
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
    td.addColumn(ScalarColumnDesc<Int> ("SPWINID"));
    td.addColumn(ScalarColumnDesc<Int> ("POLID"));
    td.addColumn(ScalarColumnDesc<Int> ("RECID"));
    SetupNewTable newTab("", td, Table::Scratch);
    theCache_p = new Table(newTab, GBTBackendFiller::tableLock());
    AlwaysAssert(theCache_p, AipsError);

    // and attach the columns
    idCacheCol_p.attach(*theCache_p,"ID");
    spWinIdCacheCol_p.attach(*theCache_p,"SPWINID");
    polIdCacheCol_p.attach(*theCache_p,"POLID");
    receiverIdCacheCol_p.attach(*theCache_p,"RECID");

    // suck any existing DATA_DESCRIPTION table into the cache
    uInt rowsToAdd = min(msDataDesc_p->nrow(), cacheSize_p);
    // this may be too innefficient - one cell at a time
    uInt rowStart = 0;
    if (rowsToAdd == cacheSize_p) {
	rowStart = msDataDesc_p->nrow()-cacheSize_p;
    } 
    // add the rows
    theCache_p->addRow(rowsToAdd);
    // insert the values
    for (uInt i=rowStart; i<(rowStart+rowsToAdd); i++) {
	idCacheCol_p.putScalar(nextCacheRow_p, Int(i));
	spWinIdCacheCol_p.putScalar(nextCacheRow_p, msDataDescCols_p->spectralWindowId()(i));
	polIdCacheCol_p.putScalar(nextCacheRow_p, msDataDescCols_p->polarizationId()(i));
	receiverIdCacheCol_p.putScalar(nextCacheRow_p, receiverIdCol_p.asInt(i));
	nextCacheRow_p++;
    }

    // and create the cache index
    cacheIndx_p = new ColumnsIndex(*theCache_p, 
				   stringToVector("SPWINID,POLID,RECID"));
    AlwaysAssert(cacheIndx_p, AipsError);
    // and attach the key field pointers
    spWinIdKey_p.attachToRecord(cacheIndx_p->accessKey(),"SPWINID");
    polIdKey_p.attachToRecord(cacheIndx_p->accessKey(),"POLID");
    receiverIdKey_p.attachToRecord(cacheIndx_p->accessKey(),"RECID");
}

void GBTMSDataDescFiller::init(MSDataDescription &msDataDesc)
{
    msDataDesc_p = new MSDataDescription(msDataDesc);
    AlwaysAssert(msDataDesc_p, AipsError);

    msDataDescCols_p = new MSDataDescColumns(msDataDesc);
    AlwaysAssert(msDataDescCols_p, AipsError);

    // add the NRAO_GBT_RECEIVER_ID column if necessary
    if (!msDataDesc_p->tableDesc().isColumn("NRAO_GBT_RECEIVER_ID")) {
	msDataDesc_p->addColumn(ScalarColumnDesc<Int>("NRAO_GBT_RECEIVER_ID",
						      "Ad-hoc GBT receiver ID until freq and pol info is available"));
	// are there already some rows in the table, if so, fill them with -1 for this column
	if (msDataDesc_p->nrow() > 0) {
	    ScalarColumn<Int> recCol(*msDataDesc_p, "NRAO_GBT_RECEIVER_ID");
	    recCol.fillColumn(-1);
	}
    }
    // and attach it
    receiverIdCol_p.attach(*msDataDesc_p, "NRAO_GBT_RECEIVER_ID");

    setCacheSize(cacheSize_p);

    dataDescIds_p.resize(0);
    receiverMap_p.resize(0, True, False);
}

Int GBTMSDataDescFiller::checkAndFill(Int polarizationId, Int specWinId, Int receiverId)
{
    *spWinIdKey_p = specWinId;
    *receiverIdKey_p = receiverId;
    *polIdKey_p = polarizationId;

    Bool found;
    uInt rownr = cacheIndx_p->getRowNumber(found);
    Int thisId = -1;
    if (found) {
	thisId = idCacheCol_p.asInt(rownr);
    } else {
	// a new row is necessary
	thisId = msDataDesc_p->nrow();
	msDataDesc_p->addRow();
	msDataDescCols_p->spectralWindowId().put(thisId, *spWinIdKey_p);
	msDataDescCols_p->polarizationId().put(thisId, *polIdKey_p);
	receiverIdCol_p.putScalar(thisId, *receiverIdKey_p);
	msDataDescCols_p->flagRow().put(thisId, False);
	
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
	idCacheCol_p.putScalar(nextCacheRow_p, Int(thisId));
	spWinIdCacheCol_p.putScalar(nextCacheRow_p, *spWinIdKey_p);
	polIdCacheCol_p.putScalar(nextCacheRow_p, *polIdKey_p);
	receiverIdCacheCol_p.putScalar(nextCacheRow_p, *receiverIdKey_p);
	nextCacheRow_p++;
	
	if (updateIndex) cacheIndx_p->setChanged();
    }
    return thisId;
}
