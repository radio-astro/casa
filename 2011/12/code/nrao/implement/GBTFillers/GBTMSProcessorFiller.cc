//# GBTMSProcessorFiller.cc: GBTMSProcessorFiller fills the MSProcessor table for GBT fillers
//# Copyright (C) 2000,2001
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

#include <nrao/GBTFillers/GBTMSProcessorFiller.h>

#include <casa/Arrays/ArrayUtil.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSProcessor.h>
#include <ms/MeasurementSets/MSProcessorColumns.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Utilities/Assert.h>

GBTMSProcessorFiller::GBTMSProcessorFiller()
    : msProcessor_p(0), msProcCols_p(0), index_p(0)
{;}


GBTMSProcessorFiller::GBTMSProcessorFiller(MSProcessor &msProcessor)
    : msProcessor_p(0), msProcCols_p(0), index_p(0)
{init(msProcessor);}

GBTMSProcessorFiller::~GBTMSProcessorFiller()
{
    delete index_p;
    index_p = 0;

    delete msProcessor_p;
    msProcessor_p = 0;

    delete msProcCols_p;
    msProcCols_p = 0;
}

void GBTMSProcessorFiller::attach(MSProcessor &msProcessor)
{init(msProcessor);}

void GBTMSProcessorFiller::fill(const String &type, Int typeId)
{
    // Set the keys
    *typeKey_p = type;
    *typeIdKey_p = typeId;

    Bool found;
    uInt rownr = index_p->getRowNumber(found);
    if (found) {
	procId_p = rownr;
    } else {
	// add it in
	procId_p = msProcessor_p->nrow();
	msProcessor_p->addRow();
	msProcCols_p->type().put(procId_p, *typeKey_p);
	msProcCols_p->subType().put(procId_p, "GBT");
	msProcCols_p->typeId().put(procId_p, *typeIdKey_p);
	msProcCols_p->modeId().put(procId_p, -1);
	msProcCols_p->flagRow().put(procId_p, False);
    }
}

void GBTMSProcessorFiller::init(MSProcessor &msProcessor)
{
    msProcessor_p = new MSProcessor(msProcessor);
    AlwaysAssert(msProcessor_p, AipsError);

    msProcCols_p = new MSProcessorColumns(msProcessor);
    AlwaysAssert(msProcCols_p, AipsError);

    procId_p = -1;

    index_p = new ColumnsIndex(*msProcessor_p, stringToVector("TYPE,TYPE_ID"));
    AlwaysAssert(index_p, AipsError);

    typeKey_p.attachToRecord(index_p->accessKey(), "TYPE");
    typeIdKey_p.attachToRecord(index_p->accessKey(), "TYPE_ID");
}
