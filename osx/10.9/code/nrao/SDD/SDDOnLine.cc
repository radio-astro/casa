//# SDDOnLine.cc: defines SDDOnLine, which encapsulates on-line SDD data files at the 140ft
//# Copyright (C) 1999,2001
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

#include <nrao/SDD/SDDOnLine.h>
#include <nrao/SDD/SDDFile.h>
#include <nrao/SDD/SDDHeader.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicMath/Math.h>

SDDOnLine::SDDOnLine()
    :  offMap_p(-1), sddFile_p(0), nrow_p(0)
{
    init();
}    

SDDOnLine::SDDOnLine(const String& sddFileName)
    : offMap_p(-1), sddFileName_p(sddFileName), sddFile_p(0), nrow_p(0)
{
    init();
}

SDDOnLine::SDDOnLine(const SDDOnLine& other)
    : timeStamp_p(other.timeStamp_p), offMap_p(other.offMap_p),
      sddFileName_p(other.sddFileName_p), nrow_p(other.nrow_p),
      rowMap_p(other.rowMap_p)
{
    if ((sddFile_p = new SDDFile(*other.sddFile_p)) == 0) {
	throw(AllocError("Allocating the sddFile",1));
    }
}

SDDOnLine::~SDDOnLine() 
{
    delete sddFile_p;
}

void SDDOnLine::attach(const String& sddFileName)
{
    // if the file is the same, assume we are correctly attached
    // if not the same, effectively start over
    if (sddFileName != sddFileName_p ) {
	sddFileName_p = sddFileName;
	init();
    } else {
	// otherwise, just do a sync
	sync();
    }
}
    

const SDDIndexRep& SDDOnLine::index(uInt rownr) const
{
    if (rownr > nrow_p) {
	throw(AipsError("SDDOnLine::index(uInt rownr) - rownr is out of range"));
    }
    return (sddFile_p->index(rowMap_p[rownr]));
}

const SDDHeader& SDDOnLine::header(uInt rownr) const
{
    if (rownr > nrow_p){
	throw(AipsError("SDDOnLine::header(uInt rownr) - rownr is out of range"));
    }
    return (sddFile_p->header(rowMap_p[rownr]));
}

Bool SDDOnLine::getData(Array<Float>& data, uInt rownr) const
{
    if (rownr > nrow_p) {
	throw(AipsError("SDDOnLine::getData(uInt rownr) - rownr is out of range"));
    }
    return (sddFile_p->getData(data, rowMap_p[rownr]));
}

Int SDDOnLine::timeStamp(uInt rownr) const
{
    if (rownr > nrow_p) {
	throw(AipsError("SDDOnLine::timeStamp(uInt rownr) - rownr is out of range"));
    }
    return timeStamp_p[rownr];
}

Int SDDOnLine::offStamp(uInt rownr) const
{
    uInt scan = uInt(header(rownr).get(SDDHeader::OFFSCAN));
    if (offMap_p.isDefined(scan)) {
      return offMap_p(scan);
    } else {
      return offMap_p.defaultVal();
    }
}

Bool SDDOnLine::needToSync() const {
    return (sddFile_p->bootStrap().hasChanged());
}

Int SDDOnLine::sync()
{
    // only do this if we need to
    if (needToSync()) {
	// remember how things stand now
	uInt maxUsed = sddFile_p->bootStrap().maxEntryUsed();

	if (sddFile_p->incrementalUpdate()) {
	    // successful incrementalUpdate - just append new stuff to the maps
	    appendMaps(maxUsed);
	} else {
	    // a full update is required
	    init();
	}
    }
    return nrow_p;
}

void SDDOnLine::init()
{
    if (sddFile_p) delete sddFile_p;
    if (sddFileName_p.length() > 0) {
	if ((sddFile_p = new SDDFile(sddFileName_p)) == 0) {
	    throw(AllocError("SDDOnLine::init() - allocating SDD file", 1));
	}
    } else {
	if ((sddFile_p = new SDDFile) == 0) {
	    throw(AllocError("SDDOnLine::init() - allocating empty SDD file", 1));
	}
    }

    nrow_p = 0;
    appendMaps(0);
}

void SDDOnLine::appendMaps(uInt startSDDEntry)
{
    uInt maxUsed = sddFile_p->bootStrap().maxEntryUsed();

    uInt newRows = 0;

    for (uInt i = startSDDEntry;i < maxUsed; i++) {
	if (sddFile_p->inUse(i)) {
	    newRows++;
	}
    }

    if (newRows+nrow_p > timeStamp_p.nelements()) {
	uInt newSize = max(newRows+nrow_p, nrow_p * 2);
	timeStamp_p.resize(newSize);
	rowMap_p.resize(newSize);
    }

    // Get most recent timestamp and lst set, or set them to zero if this is
    // the first row
    // TBD the index needs a function to return the full time using LST and utDate
    Int timestamp;
    Float lst;
    if (nrow_p > 0) {
	timestamp = timeStamp_p[nrow_p-1];
	lst = sddFile_p->index(rowMap_p[nrow_p-1]).lst();
    } else {
	timestamp = 0;
	lst = 0;
    }

    uInt sddEntry = startSDDEntry;

    // advance until in use

    uInt thisEntry;
    while (sddEntry < sddFile_p->bootStrap().maxEntryUsed()) {
	if (sddFile_p->inUse(sddEntry)) {
	    thisEntry = sddEntry;
	    // do we need a new timestamp for this entry
	    if (sddFile_p->index(thisEntry).lst() != lst) {
		timestamp++;
	    }

	    lst = sddFile_p->index(thisEntry).lst();
	    // switch on filePtr
	    offMap_p.define(sddFile_p->index(sddEntry).scan(), 
			    timestamp);
	    rowMap_p[nrow_p] = sddEntry;
	    timeStamp_p[nrow_p] = timestamp;
	    nrow_p++;
	}
	// and advance to the next scan in use
	++sddEntry;
    }
}
