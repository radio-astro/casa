//# GBTAntennaDAPFiller.cc: A GBTDAPFiller for antenna DAPs
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

#include <nrao/GBTFillers/GBTAntennaDAPFiller.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/Table.h>
#include <tables/TaQL/ExprNode.h>
#include <casa/Utilities/Assert.h>
#include <casa/namespace.h>

GBTAntennaDAPFiller::GBTAntennaDAPFiller(const String &device, Table &parent)
    : GBTDAPFiller(device, parent), skyTable_p(0), hasSkyPosition_p(False)
{
    AlwaysAssert(GBTDAPFiller::device() == "ANTENNA", AipsError);
}

GBTAntennaDAPFiller::~GBTAntennaDAPFiller() 
{
    cleanup();
}

void GBTAntennaDAPFiller::cleanup()
{
    delete skyTable_p;
    skyTable_p = 0;
    // ensure that the columns are de-referenced
    j2000ra_p.reference(ROTableColumn());
    j2000dec_p.reference(ROTableColumn());
    time_p.reference(ROTableColumn());
}

Int GBTAntennaDAPFiller::prepare(const String &fileName, 
			  const String &manager, const String &sampler,
			  const MVTime &startTime)
{
    Int result = -1;
    if (fileName == "") {
	// this is a signal that there is no antenna info this round and so
	// we need to clear it
	cleanup();
    } else {
	result = GBTDAPFiller::prepare(fileName, manager, sampler, startTime);
	if (result >= 0 && sampler == "skyPosition") {
	    hasSkyPosition_p = True;
	}
    }
    return result;
}

void GBTAntennaDAPFiller::fill()
{
    // remember where we start from
    uInt startRow = tab().nrow();
    GBTDAPFiller::fill();
    // and where we end at
    uInt endRow = tab().nrow()-1;
    // after filling, do a cleanup
    cleanup();
    // if we have a skyPosition, create the selection now
    if (hasSkyPosition_p && endRow > startRow) {
	// first, row number
	Vector<uInt> rows((endRow-startRow)+1);
	indgen(rows,startRow);
	Table rowtab = tab()(rows);
	// and then using the SAMPLER column to get just the skyPosition values
	skyTable_p = new Table(rowtab(rowtab.col("SAMPLER") == "skyPosition"));
	AlwaysAssert(skyTable_p, AipsError);
	// and attach the table columns
	j2000ra_p.attach(*skyTable_p, "j2000_Ra");
	j2000dec_p.attach(*skyTable_p, "j2000_Dec");
	time_p.attach(*skyTable_p, "TIME");
    }
    // at this point, reset hasSkyPosition for the next prepare
    hasSkyPosition_p = False;
}
	
