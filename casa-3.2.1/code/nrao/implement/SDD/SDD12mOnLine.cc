//# SDD12mOnLine.cc: defines SDD12mOnLine which encapsulates on-line SDD data files at the 12-m
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

#include <nrao/SDD/SDD12mOnLine.h>
#include <nrao/SDD/SDDFile.h>
#include <nrao/SDD/SDDHeader.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicMath/Math.h>

SDD12mOnLine::SDD12mOnLine()
    : offMap_p(-1), gainMap_p(-1),
      dataFile_p(0),  gainFile_p(0), nrow_p(0)
      
{
    init();
}    

SDD12mOnLine::SDD12mOnLine(const String& dataFileName, 
			   const String& gainFileName)
    : offMap_p(-1), gainMap_p(-1),
      dataFileName_p(dataFileName), gainFileName_p(gainFileName),
      dataFile_p(0), gainFile_p(0), nrow_p(0)
{
    init();
}

SDD12mOnLine::SDD12mOnLine(const SDD12mOnLine& other)
    : timeStamp_p(other.timeStamp_p), offMap_p(other.offMap_p),
      gainMap_p(other.gainMap_p), dataFileName_p(other.dataFileName_p),
      gainFileName_p(other.gainFileName_p), nrow_p(other.nrow_p),
      fileMap_p(other.fileMap_p), rowMap_p(other.rowMap_p)
{
    if ((dataFile_p = new SDDFile(*other.dataFile_p)) == 0) {
	throw(AllocError("Allocating the dataFile",1));
    }
    if ((gainFile_p = new SDDFile(*other.gainFile_p)) == 0) {
	throw(AllocError("Allocating the gainFile",1));
    }
}

SDD12mOnLine::~SDD12mOnLine() 
{
    delete dataFile_p;
    delete gainFile_p;
}

void SDD12mOnLine::attach(const String& dataFileName, 
			  const String& gainFileName)
{
    // if both files are the same, assume we are correctly attached
    // if either are not the same, effectively start over
    if (dataFileName != dataFileName_p ||
	gainFileName != gainFileName_p) {
	dataFileName_p = dataFileName;
	gainFileName_p = gainFileName;
	init();
    } else {
	// otherwise, just do a sync
	sync();
    }
}
    

const SDDIndexRep& SDD12mOnLine::index(uInt rownr) const
{
    if (rownr > nrow_p) {
	throw(AipsError("SDD12mOnLine::index(uInt rownr) - rownr is out of range"));
    }
    return (*fileMap_p[rownr]).index(rowMap_p[rownr].x());
}

const SDDHeader& SDD12mOnLine::header(uInt rownr) const
{
    if (rownr > nrow_p){
	throw(AipsError("SDD12mOnLine::header(uInt rownr) - rownr is out of range"));
    }
    return (*fileMap_p[rownr]).header(rowMap_p[rownr].x());
}

Bool SDD12mOnLine::getData(Array<Float>& data, uInt rownr) const
{
    if (rownr > nrow_p) {
	throw(AipsError("SDD12mOnLine::getData(uInt rownr) - rownr is out of range"));
    }
    return (*fileMap_p[rownr]).getData(data, rowMap_p[rownr].x(), 
				       rowMap_p[rownr].y());
}

Int SDD12mOnLine::timeStamp(uInt rownr) const
{
    if (rownr > nrow_p) {
	throw(AipsError("SDD12mOnLine::timeStamp(uInt rownr) - rownr is out of range"));
    }
    return timeStamp_p[rownr];
}

Int SDD12mOnLine::gainStamp(uInt rownr) const
{
    uInt scan = uInt(header(rownr).get(SDDHeader::GAINS));
    if (gainMap_p.isDefined(scan)) {
      return gainMap_p(scan);
    } else {
      return gainMap_p.defaultVal();
    }
}

Int SDD12mOnLine::offStamp(uInt rownr) const
{
    uInt scan = uInt(header(rownr).get(SDDHeader::OFFSCAN));
    if (offMap_p.isDefined(scan)) {
      return offMap_p(scan);
    } else {
      return offMap_p.defaultVal();
    }
}

Bool SDD12mOnLine::needToSync() const {
    return (dataFile_p->bootStrap().hasChanged() ||
	    gainFile_p->bootStrap().hasChanged());
}

Int SDD12mOnLine::sync()
{
    // only do this if we need to
    if (needToSync()) {
	// remember how things stand now
	uInt maxData = dataFile_p->bootStrap().maxEntryUsed();
	uInt maxGain = gainFile_p->bootStrap().maxEntryUsed();

	if (dataFile_p->incrementalUpdate() && 
	    gainFile_p->incrementalUpdate()) {
	    // successful incrementalUpdate - just append new stuff to the maps
	    appendMaps(maxData, maxGain);
	} else {
	    // a full update is required
	    init();
	}
    }
    return nrow_p;
}

void SDD12mOnLine::init()
{
    if (dataFile_p) delete dataFile_p;
    if (dataFileName_p.length() > 0) {
	if ((dataFile_p = new SDDFile(dataFileName_p)) == 0) {
	    throw(AllocError("SDD12mOnLine::init() - allocating data file", 1));
	}
    } else {
	if ((dataFile_p = new SDDFile) == 0) {
	    throw(AllocError("SDD12mOnLine::init() - allocating empty data file", 1));
	}
    }
    if (gainFile_p) delete gainFile_p;
    if (gainFileName_p.length() > 0) {
	if ((gainFile_p = new SDDFile(gainFileName_p)) == 0) {
	    throw(AllocError("SDD12mOnLine::init() - allocating gain file", 1));
	}
    } else {
	if ((gainFile_p = new SDDFile) == 0) {
	    throw(AllocError("SDD12mOnLine::init() - allocating empty gain file", 1));
	}
    }

    nrow_p = 0;
    appendMaps(0,0);
}

void SDD12mOnLine::appendMaps(uInt startDataEntry, uInt startGainEntry)
{
    uInt maxData = dataFile_p->bootStrap().maxEntryUsed();
    uInt maxGain = gainFile_p->bootStrap().maxEntryUsed();

    uInt newRows = 0;

    for (uInt i = startDataEntry;i < maxData; i++) {
	if (dataFile_p->inUse(i)) {
	    newRows += dataFile_p->nvectors(i);
	}
    }

    for (uInt i = startGainEntry;i < maxGain; i++) {
	if (dataFile_p->inUse(i)) {
	    newRows += gainFile_p->nvectors(i);
	}
    }

    if (newRows+nrow_p > fileMap_p.nelements()) {
	uInt newSize = max(newRows+nrow_p, nrow_p * 2);
	timeStamp_p.resize(newSize);
	fileMap_p.resize(newSize);
	rowMap_p.resize(newSize);
    }

    // Get most recent timestamp and lst set, or set them to zero if this is
    // the first row
    // TBD the index needs a function to return the full time using LST and utDate
    Int timestamp;
    Float lst;
    if (nrow_p > 0) {
	timestamp = timeStamp_p[nrow_p-1];
	lst = (fileMap_p[nrow_p-1])->index(rowMap_p[nrow_p-1].x()).lst();
    } else {
	timestamp = 0;
	lst = 0;
    }

    uInt dataEntry = startDataEntry;
    uInt gainEntry = startGainEntry;
    SDDFile *filePtr = 0, *oldFilePtr = 0;

    // advance until both are in use

    uInt thisEntry;
    uInt tsOffset = 0;
    while (dataEntry < dataFile_p->bootStrap().maxEntryUsed() ||
	   gainEntry < gainFile_p->bootStrap().maxEntryUsed()) {
	if (dataFile_p->inUse(dataEntry) && 
	    gainFile_p->inUse(gainEntry)) {
	    // both are in use, which one comes first
	    if (dataFile_p->index(dataEntry).lst() <=
		gainFile_p->index(gainEntry).lst()) {
		filePtr = dataFile_p;
		thisEntry = dataEntry;
	    } else {
		filePtr = gainFile_p;
		thisEntry = gainEntry;
	    }
	} else if (dataFile_p->inUse(dataEntry)) {
	    filePtr = dataFile_p;
	    thisEntry = dataEntry;
	} else if (gainFile_p->inUse(gainEntry)) {
	    filePtr = gainFile_p;
	    thisEntry = gainEntry;
	} else {
	    // this should never happend
	    throw(AipsError("Internal SDD12mOnLine error, in appendMaps()"));
	}
	// do we need a new timestamp
	if (filePtr->index(thisEntry).lst() != lst ||
	    filePtr != oldFilePtr ) {
	    timestamp++;
	    timestamp += tsOffset;
	    tsOffset = 0;
	    oldFilePtr = filePtr;
	    lst = filePtr->index(thisEntry).lst();
	}
	// switch on filePtr
	if (filePtr == dataFile_p) {
	    // is this a single vector entry if so, it might be an OFF scan
	    if (dataFile_p->nvectors(dataEntry) == 1) {
		// currently we simply need to remember the mapping of scan to row
		offMap_p.define(dataFile_p->index(dataEntry).scan(), 
				timestamp);
	    }
	    for (uInt vecnr=0;vecnr<dataFile_p->nvectors(dataEntry);
			      vecnr++) {
		rowMap_p[nrow_p].x() = dataEntry;
		rowMap_p[nrow_p].y() = vecnr;
		timeStamp_p[nrow_p] = timestamp + vecnr;
		fileMap_p[nrow_p] = dataFile_p;
		nrow_p++;
	    }
	    tsOffset = dataFile_p->nvectors(dataEntry) - 1;
	    // and advance to the next scan in use
	    while (++dataEntry < dataFile_p->bootStrap().maxEntryUsed() &&
		   !(dataFile_p->index().inUse(dataEntry)));
	} else {
	    gainMap_p.define(gainFile_p->index(gainEntry).scan(),timestamp);
	    rowMap_p[nrow_p].x() = gainEntry;
	    rowMap_p[nrow_p].y() = 0;
	    timeStamp_p[nrow_p] = timestamp;
	    fileMap_p[nrow_p] = gainFile_p;
	    nrow_p++;
	    // and advance to the next scan in use
	    while (++gainEntry < gainFile_p->bootStrap().maxEntryUsed() &&
		   !(gainFile_p->index().inUse(gainEntry)));
	}
    }
}
	    
Float SDD12mOnLine::time(uInt rownr) const
{
    return (*fileMap_p[rownr]).time(rowMap_p[rownr].x(), rowMap_p[rownr].y());
}

Float SDD12mOnLine::raOffset(uInt rownr) const
{
    return (*fileMap_p[rownr]).raOffset(rowMap_p[rownr].x(), rowMap_p[rownr].y());
}

Float SDD12mOnLine::decOffset(uInt rownr) const
{
    return (*fileMap_p[rownr]).decOffset(rowMap_p[rownr].x(), rowMap_p[rownr].y());
}

Float SDD12mOnLine::az(uInt rownr) const
{
    return (*fileMap_p[rownr]).az(rowMap_p[rownr].x(), rowMap_p[rownr].y());
}

Float SDD12mOnLine::el(uInt rownr) const
{
    return (*fileMap_p[rownr]).el(rowMap_p[rownr].x(), rowMap_p[rownr].y());
}

Float SDD12mOnLine::raRange(uInt rownr) const
{
    SDDFile * thisFile = fileMap_p[rownr];
    Int entry = rowMap_p[rownr].x();
    Int nvecs = thisFile->nvectors(entry);
    return (abs(thisFile->raOffset(entry,0) - 
		thisFile->raOffset(entry,nvecs-1)));
}
