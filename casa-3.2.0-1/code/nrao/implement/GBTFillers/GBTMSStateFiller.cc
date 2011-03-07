//# GBTMSStateFiller.cc: GBTMSStateFiller fills the MSState table for GBT fillers
//# Copyright (C) 2002,2003
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

#include <nrao/GBTFillers/GBTMSStateFiller.h>

#include <casa/Arrays/ArrayUtil.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSStateColumns.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Utilities/Assert.h>

GBTMSStateFiller::GBTMSStateFiller()
    : index_p(0), msState_p(0), msStateCols_p(0)
{;}


GBTMSStateFiller::GBTMSStateFiller(MSState &msState)
    : index_p(0), msState_p(0), msStateCols_p(0)
{init(msState);}

GBTMSStateFiller::~GBTMSStateFiller()
{
    delete index_p;
    index_p = 0;

    delete msState_p;
    msState_p = 0;

    delete msStateCols_p;
    msStateCols_p = 0;
}

void GBTMSStateFiller::attach(MSState &msState)
{init(msState);}

void GBTMSStateFiller::fill(const Vector<Bool> &sigref, const Vector<Bool> &cal,
			    const String &procname, Int procsize, Int procseqn,
			    const String &swstate, const String &swtchsig)
{
    // sanity check - this should never happen when used properly.
    AlwaysAssert(sigref.nelements() == cal.nelements(), AipsError);

    // resize the ids
    stateIds_p.resize(sigref.nelements());
    sigs_p.resize(sigref.nelements());

    if (stateIds_p.nelements() == 0) return;

    // set the index keys which don't change with state
    *subScanKey_p = procseqn;
    *procSizeKey_p = procsize;
    *obsModeKey_p = procname + ":" + swstate + ":" + swtchsig;

    // loop over state
    Vector<Bool> usedSigref = sigref;
    //  This is the way this code originally looked.  Unfortunately, there
    // were cases where swtchsig was not TPWCALSP and yet the SIGREF should
    // have been interpreted as all True.
//     if (swtchsig == "TPWCALSP") {
// 	// in this mode - the SIGREF values are all True, independent
// 	// from their actual values in the backend
// 	usedSigref = True;
//     }
    // And this is the workaround.  Whenever there are 2 states AND
    // the cal values change, then SIGREF should be interpreted as
    // all True
    if (usedSigref.nelements() == 2 && cal[0] != cal[1]) {
	usedSigref = True;
    }

    for (uInt i=0;i<stateIds_p.nelements();i++) {
	*sigKey_p = sigref(i);
	// MS definition has CAL as a DOUBLE - have to use what is there for now
	if (cal(i)) {
	    *calKey_p = 1.0;
	} else {
	    *calKey_p = 0.0;
	}
	Bool found;
	Int thisIndex = index_p->getRowNumber(found);
	if (!found) {
	    // need a new row
	    thisIndex = msState_p->nrow();
	    msState_p->addRow();
	    msStateCols_p->sig().put(thisIndex, *sigKey_p);
	    // REF is redundent - always !SIG
	    msStateCols_p->ref().put(thisIndex, !(*sigKey_p));
	    msStateCols_p->cal().put(thisIndex, *calKey_p);
	    // LOAD is not currently used
	    msStateCols_p->load().put(thisIndex, 0.0);
	    msStateCols_p->subScan().put(thisIndex, *subScanKey_p);
	    msStateCols_p->obsMode().put(thisIndex, *obsModeKey_p);
	    // FLAG_ROW is always False (this data is always valid - not flagged)
	    msStateCols_p->flagRow().put(thisIndex, False);
	    // the 3 NRAO_GBT rows - procname is in obsMode and procseq is in
	    // subScan so they don't need to be involved in index_p
	    procnameCol_p.put(thisIndex, procname);
	    procsizeCol_p.put(thisIndex, *procSizeKey_p);
	    procseqnCol_p.put(thisIndex, procseqn);
	} 
	stateIds_p[i] = thisIndex;
	sigs_p[i] = *sigKey_p;
    }
}

void GBTMSStateFiller::init(MSState &msState)
{
    msState_p = new MSState(msState);
    AlwaysAssert(msState_p, AipsError);

    msStateCols_p = new MSStateColumns(msState);
    AlwaysAssert(msStateCols_p, AipsError);

    procnameCol_p.attach(*msState_p, "NRAO_GBT_PROCNAME");
    procseqnCol_p.attach(*msState_p, "NRAO_GBT_PROCSEQN");
    procsizeCol_p.attach(*msState_p, "NRAO_GBT_PROCSIZE");

    String indexString = MSState::columnName(MSState::SIG);
    indexString += ",";
    indexString += MSState::columnName(MSState::CAL);
    indexString += ",";
    indexString += MSState::columnName(MSState::SUB_SCAN);
    indexString += ",";
    indexString += MSState::columnName(MSState::OBS_MODE);
    indexString += ",";
    indexString += "NRAO_GBT_PROCSIZE";
    index_p = new ColumnsIndex(*msState_p, stringToVector(indexString));
    AlwaysAssert(index_p, AipsError);

    sigKey_p.attachToRecord(index_p->accessKey(), 
			    MSState::columnName(MSState::SIG));
    calKey_p.attachToRecord(index_p->accessKey(), 
			    MSState::columnName(MSState::CAL));
    subScanKey_p.attachToRecord(index_p->accessKey(), 
				MSState::columnName(MSState::SUB_SCAN));
    obsModeKey_p.attachToRecord(index_p->accessKey(), 
				MSState::columnName(MSState::OBS_MODE));
    procSizeKey_p.attachToRecord(index_p->accessKey(), "NRAO_GBT_PROCSIZE");

    stateIds_p.resize(0);
    sigs_p.resize(0);
}
