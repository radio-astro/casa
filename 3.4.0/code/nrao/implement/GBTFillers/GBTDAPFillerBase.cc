//# GBTDAPFillerBase.h: the base class for DAP fillers
//# Copyright (C) 2000
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

#include <nrao/GBTFillers/GBTDAPFillerBase.h>

#include <casa/Exceptions/Error.h>
#include <casa/OS/Path.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/Assert.h>

GBTDAPFillerBase::GBTDAPFillerBase()
    : rowsToFill_p(0), dapmap_p(-1), dapblock_p(1), readyToFill_p(1), ndap_p(0),
      currentPtr_p(-1)
{
    // this shouldn't be necessary, but it also couldn't hurt
    dapblock_p[0] = 0;
}

GBTDAPFillerBase::GBTDAPFillerBase(const GBTDAPFillerBase &other)
    : rowsToFill_p(0), dapmap_p(-1), dapblock_p(1), readyToFill_p(1), ndap_p(0),
      currentPtr_p(-1)
{
    *this = other;
}

GBTDAPFillerBase &GBTDAPFillerBase::operator=(const GBTDAPFillerBase &other) {
    if (this != &other) {
	rowsToFill_p = other.rowsToFill_p;
	dapmap_p = other.dapmap_p;
	for (uInt i=0;i<ndap_p;i++) {
	    delete dapblock_p[i];
	    dapblock_p[i] = 0;
	}
	ndap_p = other.ndap_p;
	dapblock_p.resize(other.dapblock_p.nelements());
	readyToFill_p.resize(other.readyToFill_p.nelements());
	MVTime zeroTime(0.0);
	for (uInt i=0;i<ndap_p;i++) {
	    dapblock_p[i] = new GBTDAPFile(other.dapblock_p[i]->name(), zeroTime);
	    AlwaysAssert(dapblock_p[i], AipsError);
	    // move this DAP file to the same row as in other
	    dapblock_p[i]->move(other.dapblock_p[i]->rownr());
	    readyToFill_p[i] = other.readyToFill_p[i];
	}
	currentPtr_p = other.currentPtr_p;
    }
    return *this;
}

GBTDAPFillerBase::~GBTDAPFillerBase() 
{
    for (uInt i=0; i<ndap_p; i++) {
	delete dapblock_p[i];
	dapblock_p[i] = 0;
    }
    ndap_p = 0;
}

Int GBTDAPFillerBase::prepare(const String &fileName, 
			       const String &manager, const String &sampler,
			       const MVTime &startTime)
{
    Int id = -1;
    String mansamp = manager + sampler;
    if (!dapmap_p.isDefined(mansamp)) {
	// try and make a new one
	GBTDAPFile *newDAP = new GBTDAPFile(fileName, startTime);
	AlwaysAssert(newDAP, AipsError);
	// if result is false, we give up at this point
	if (newDAP->isValid()) {
	    if (dapblock_p.nelements() <= ndap_p) {
		dapblock_p.resize(dapblock_p.nelements()*2);
		readyToFill_p.resize(readyToFill_p.nelements()*2);
	    }
	    id = ndap_p++;
	    dapblock_p[id] = newDAP;
	    dapmap_p.define(mansamp, id);
	} else {
	    delete newDAP;
	}
    } else {
	// we already have one here, re-open it
	id = dapmap_p(mansamp);
	dapblock_p[id]->reopen(fileName, startTime, False);
    }
    if (id >= 0) {
	// a false return here indicates a problem in a re-opened DAP file
	readyToFill_p[id] = dapblock_p[id]->isValid();
	// and count the number of rows this adds to the number to be filled
	if (readyToFill_p[id] && !dapblock_p[id]->pastEnd()) {
	    rowsToFill_p += (dapblock_p[id]->nrow() - dapblock_p[id]->rownr());
	}
	if (!readyToFill_p[id]) id = -1;
    }
    return id;
}

MVTime GBTDAPFillerBase::minTime() const
{
    MVTime result(Quantity(5000.0,"a"));
    for (uInt i=0;i<ndap_p;i++) {
	if (dapblock_p[i]->isValid() && !dapblock_p[i]->pastEnd() &&
	    dapblock_p[i]->nrow() > 0 && 
	    Double(dapblock_p[i]->currentTime()) < Double(result)) {
	    result = dapblock_p[i]->currentTime();
	}
    }
    return result;
}

Int GBTDAPFillerBase::currentId() {
    Int result = -1;
    if (rowsToFill_p > 0) {
	// if the currentPtr_p doesn't point at anything yet, then calling next will
	// find the first scan we should fill
	if (currentPtr_p < 0) next();
	// there must be one, since rowsToFill_p indicates more rows left to fill
	result = currentPtr_p;
    }
    return result;
}

const Record &GBTDAPFillerBase::currentRow() {
    Int id = currentId();
    if (id < 0) {
	return emptyRecord_p;
    } 
    // we only get here if we should have something to return
    return dapRecord(id);
}

const String &GBTDAPFillerBase::currentSampler() {
    Int id = currentId();
    if (id < 0) return emptyString_p;
    return dapblock_p[id]->sampler();
}

const String &GBTDAPFillerBase::currentManager() {
    Int id = currentId();
    if (id < 0) return emptyString_p;
    return dapblock_p[id]->manager();
}

Double GBTDAPFillerBase::currentInterval() {
    Int id = currentId();
    if (id < 0) return 0.0;
    return dapblock_p[id]->interval();
}

void GBTDAPFillerBase::next()
{
    // if the currentPtr_p points at something, advance that since its what was used last
    if (currentPtr_p >= 0) {
	dapblock_p[currentPtr_p]->next();
	rowsToFill_p--;
	// turn this one off when its past the end
	if (dapblock_p[currentPtr_p]->pastEnd()) readyToFill_p[currentPtr_p] = False;
    }   
    if (rowsToFill_p > 0) {
       ctime_p = minTime();
       for (uInt i=0;i<ndap_p;i++) {
	   if (readyToFill_p[i] && dapblock_p[i]->isValid() && !dapblock_p[i]->pastEnd() &&
	       near(dapblock_p[i]->currentTime().second(),ctime_p.second())) {
	       // this is what we wanted
	       currentPtr_p = i;
	       break;
	    }
	}
    } else {
	currentPtr_p = -1;
    }
}

Bool GBTDAPFillerBase::parseName(const String &fileName,
			     String &device, String &manager,
			     String &sampler)
{
    String res[4];
    Path filePath(fileName);
    Path dirPath(filePath.dirName());
    String thisDir = dirPath.baseName();
    Int num = split(thisDir, res, 3, String("-"));
    if (num != 3) return False;
    device = res[0];
    manager =  res[1];
    sampler = res[2];
    return True;
}

