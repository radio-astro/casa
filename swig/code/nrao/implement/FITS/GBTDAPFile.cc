//# GBTDAPFile.cc: defines GBTDAPFile which is a FITSTable holding GBT DAP info
//# Copyright (C) 1999,2000
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

#include <nrao/FITS/GBTDAPFile.h>

#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <fits/FITS/CopyRecord.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicMath/Math.h>
#include <casa/OS/RegularFile.h>
#include <casa/Utilities/Assert.h>

GBTDAPFile::GBTDAPFile(const String &dapfile, const MVTime &startTime)
    : timeWarningIssued_p(False), currentRow_p(0), interval_p(0.0), row_nr_p(-1),
      unhandledFields_p(0), fieldCopier_p(0)
{
    // this isn't a prediction, really
    endOfTime_p = Quantity(5000.0,"a");
    reopen(dapfile, startTime, False);
}

Bool GBTDAPFile::reopen(const String &dapfile, const MVTime &startTime, 
			Bool resync)
{
    Int currRow = -1;
    if (resync && dapfile == name()) {
	currRow = rownr();
    } else {
	interval_p = 0.0;
	resync = False;
    }

    cleanup();

    // don't open dapfile if its empty, although this should return False
    Bool result = False;
    // if we get here, we trust that this is a RegularFile and that it exists
    RegularFile newfile(dapfile);
    if (newfile.size() != 0) result = FITSTable::reopen(dapfile);

    if (result) init();
    if (resync) {
	// if its a resync and there aren't at least currRow+1 rows
	// then this can't be the same file and the resync has failed
	if (currRow >= Int(nrow())) {
	    // just do a reopen with no resync
	    result = reopen(dapfile, startTime, False);
	} else {
	    move(currRow);
	}
    }
    if (result && isValid()) {
	fieldMap_p.resize(currentRow().nfields());
	fieldMap_p = 0;

	// "DJMD" is always handled
	handleField("DMJD");

	// advance until end or currentTime >= startTime
	while (currentTime()<startTime && !pastEnd()) next();
    }
    return (result && isValid());
}

void GBTDAPFile::next()
{
    if (isValid()) {
	// watch for time travel to the past - we don't believe in it
	if (nextTime_p < currentTime_p && !near(nextTime_p, currentTime_p)) {
	    if (!timeWarningIssued_p) {
		// issue a warning 
		LogIO os(LogOrigin("GBTDAPFile","GBTDAPFile::next()"));
		os << LogIO::WARN << WHERE
		   << "The DMJD column decreases with increasing row in " << name() << LogIO::POST;
		os << "Data from that sampler should be viewed with suspicion and this should be reported as error" << LogIO::POST;
		timeWarningIssued_p = True;
	    }
	} else {
	    currentTime_p = nextTime_p;
	}
	if (!FITSTable::pastEnd()) {
	    *currentRow_p = FITSTable::currentRow();
	    FITSTable::next();
	    row_nr_p++;
	} 
	if (FITSTable::pastEnd()) {
	    nextTime_p = endOfTime_p;
	    // if this is also pastEnd, i.e. currentTime == endOfTime_p,
	    // then there are no more rows available from this file
	    if (pastEnd()) row_nr_p = nrow()-1;
	} else {
	    nextTime_p = *dmjd_p;
	    interval_p = nextTime_p.second() - currentTime_p.second();
	}
	if (fieldCopier_p) fieldCopier_p->copy();
    }
}

void GBTDAPFile::move(Int torow) 
{
    // we can only move within the table and only ahead
    // if we have to change the rownr, set result to False to
    // indicate that the result might not be where the user
    if (torow < rownr()) torow = rownr();
    if (torow >= Int(nrow())) torow = Int(nrow()) - 1;
    // if we are already there, just return
    if (torow == rownr()) return;

    // move the underlying FITSTable to just before we want to be
    // and then use next to move the last row
    FITSTable::move(torow-1);
    next();
}

void GBTDAPFile::init()
{
    if (FITSTable::isValid()) {
	currentRow_p = new Record(FITSTable::currentRow().description());
	AlwaysAssert(currentRow_p, AipsError);
	*currentRow_p = FITSTable::currentRow();
	// must have a MANAGER and SAMPLER keyword or return -> invalid DAP file
	if (primaryKeywords().fieldNumber("MANAGER") < 0 ||
	    primaryKeywords().fieldNumber("SAMPLER") < 0) return;
	// get the value for those keywords
	sampler_p = primaryKeywords().asString("SAMPLER");
	manager_p = primaryKeywords().asString("MANAGER");
	if (FITSTable::currentRow().fieldNumber("DMJD") >= 0) {
	    dmjd_p.attachToRecord(FITSTable::currentRow(), "DMJD");
	    currentTime_p = *dmjd_p;
	    FITSTable::next();
	    if (!FITSTable::pastEnd()) {
		nextTime_p = *dmjd_p;
		interval_p = nextTime_p.second() - currentTime_p.second();
	    } else {
		nextTime_p = endOfTime_p;
	    }
	}
	row_nr_p = 0;
    }
}

void GBTDAPFile::cleanup()
{
    currentTime_p = 0;
    nextTime_p = endOfTime_p;
    dmjd_p.detach();
    delete currentRow_p;
    currentRow_p = 0;
    row_nr_p = -1;
    delete unhandledFields_p;
    unhandledFields_p = 0;
    delete fieldCopier_p;
    fieldCopier_p = 0;
}

void GBTDAPFile::handleField(const String& fieldname)
{
    Int whichField = currentRow().fieldNumber(fieldname);
    if (whichField >= 0) {
	fieldMap_p(whichField) = -1;
    }
}

void GBTDAPFile::initUnhandledFields()
{
    delete unhandledFields_p;
    unhandledFields_p = 0;
    delete fieldCopier_p;
    fieldCopier_p = 0;

    RecordDesc rd(currentRow().description());
    for (Int i=0;i<Int(fieldMap_p.nelements());i++) {
	if (fieldMap_p(i) < 0) {
	    rd.removeField(rd.fieldNumber(currentRow().name(i)));
	}
    }

    unhandledFields_p = new Record(rd);
    AlwaysAssert(unhandledFields_p, AipsError);

    for (Int i=0;i<Int(fieldMap_p.nelements());i++) {
	if (fieldMap_p(i) >= 0) {
	    fieldMap_p(i) = rd.fieldNumber(currentRow().name(i));
	}
    }

    fieldCopier_p = new CopyRecordToRecord(*unhandledFields_p,
					   currentRow(),
					   fieldMap_p);
    AlwaysAssert(fieldCopier_p, AipsError);
    fieldCopier_p->copy();
}
