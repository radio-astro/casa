//# SDDBootStrap.cc: this defines SDDBootStrap, which encapsulates the SDD bootstrap block.
//# Copyright (C) 1995,2001
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

#include <nrao/SDD/SDDBootStrap.h>
#include <nrao/SDD/SDDBlock.h>
#include <casa/Exceptions/Error.h>

#include <casa/iostream.h>

SDDBootStrap::SDDBootStrap()
    : nIndexRec_p(0), nDataRec_p(0), 
      bytesPerRec_p(0), bytesPerEntry_p(0), maxEntryUsed_p(0), counter_p(0),
      type_p(SDDBootStrap::DATA), version_p(SDDBootStrap::CURRENT), bs_p(0)
{
    if ((bs_p = new SDDBlock()) == 0) {
	throw(AllocError("SDDBootStrap::SDDBootStrap() - "
			 "new of SDDBlock failed", 1));
    }
    bytesPerRec_p = bs_p->nbytes();
}

SDDBootStrap::SDDBootStrap(CountedPtr<fstream> streamPtr)
    : nIndexRec_p(0), nDataRec_p(0), 
      bytesPerRec_p(0), bytesPerEntry_p(0), maxEntryUsed_p(0), counter_p(0),
      type_p(SDDBootStrap::DATA), version_p(SDDBootStrap::CURRENT),
      bs_p(0), theFile_p(streamPtr)
{
    if ((bs_p = new SDDBlock()) == 0) {
	throw(AllocError("SDDBootStrap::SDDBootStrap(istream&) - "
			 "New of SDDBlock failed", 1));
    }
    sync();
}

SDDBootStrap::SDDBootStrap(const SDDBootStrap& other) :
    nIndexRec_p(other.nIndexRec()), nDataRec_p(other.nDataRec()),
    bytesPerRec_p(other.bytesPerRecord()),
    bytesPerEntry_p(other.bytesPerEntry()),
    maxEntryUsed_p(other.maxEntryUsed()), counter_p(other.counter()),
    type_p(other.type()), version_p(other.version()),
    bs_p(0), theFile_p(other.theFile_p) 
{ 
    if ((bs_p = new SDDBlock()) == 0) {
	throw(AllocError("SDDBootStrap::SDDBootStrap(SDDBootStrap&) - "
			 "New of SDDBlock failed", 1)); 
    } 
}

SDDBootStrap::~SDDBootStrap()
{
    if (bs_p) {
	delete bs_p;
	bs_p = 0;
    }
}

SDDBootStrap& SDDBootStrap::operator=(const SDDBootStrap& other)
{
    if (this == &other)
	return *this;

    nIndexRec_p = other.nIndexRec_p;
    nDataRec_p = other.nDataRec_p;
    bytesPerRec_p = other.bytesPerRec_p;
    bytesPerEntry_p = other.bytesPerEntry_p;
    maxEntryUsed_p = other.maxEntryUsed_p;
    counter_p = other.counter_p;
    type_p = other.type_p;
    version_p = other.version_p;
    theFile_p = other.theFile_p;

    return *this;
}

void SDDBootStrap::attach(CountedPtr<fstream> newStreamPtr)
{
    theFile_p = newStreamPtr;
    sync();
}

void SDDBootStrap::sync()
{
    // If not attached, do nothing
    if (theFile_p.null()) return;

    // First, actually do a sync on the stream
    //theFile_p->rdbuf()->sync();
    theFile_p->flush();

    // seek to start of file
    theFile_p->seekg(0);
    // a return of 0 indicates that there's nothing there yet
    // if so, first write what we have and then re-read it
    if (bs_p->read(*theFile_p) == 0) {
	write();
	// theFile_p->rdbuf()->sync();
        theFile_p->flush();
	theFile_p->seekg(0);
	// if we still do not get 1, throw exception
	if (bs_p->read(*theFile_p) != 1) {
	    throw (AipsError(
		"SDDBootStrap::sync() problem reading the bootstrap"));
	}
    }

    // Determine the version
    if (bs_p->asInt(SDDBootStrap::VERSION) == SDDBootStrap::CURRENT) {
	version_p = SDDBootStrap::CURRENT;
    } else {
	if (bs_p->asShort(SDDBootStrap::VERSION) != SDDBootStrap::ORIGINAL) {
	    throw (AipsError("SDDBootStrap::fill - invalid version found in bootstrap"));
	}
	version_p = SDDBootStrap::ORIGINAL;
    }

    // extract the values
    if (version_p == SDDBootStrap::CURRENT) {
	nIndexRec_p = bs_p->asInt(SDDBootStrap::N_INDEX_REC);
	nDataRec_p = bs_p->asInt(SDDBootStrap::N_DATA_REC);
	bytesPerRec_p = bs_p->asInt(SDDBootStrap::BYTES_PER_REC);
	bytesPerEntry_p = bs_p->asInt(SDDBootStrap::BYTES_PER_ENTRY);
	maxEntryUsed_p = bs_p->asInt(SDDBootStrap::MAX_ENTRY_USED);
	counter_p = bs_p->asInt(SDDBootStrap::COUNTER);
	type_p = bs_p->asInt(SDDBootStrap::TYPE);
    } else {
	nIndexRec_p = bs_p->asShort(SDDBootStrap::N_INDEX_REC);
	nDataRec_p = bs_p->asShort(SDDBootStrap::N_DATA_REC);
	bytesPerRec_p = bs_p->asShort(SDDBootStrap::BYTES_PER_REC);
	bytesPerEntry_p = bs_p->asShort(SDDBootStrap::BYTES_PER_ENTRY);
	maxEntryUsed_p = bs_p->asShort(SDDBootStrap::MAX_ENTRY_USED);
	counter_p = bs_p->asShort(SDDBootStrap::COUNTER);
	type_p = bs_p->asShort(SDDBootStrap::TYPE);
    }

    // sanity checks
    // we require everything be 1 record == size of bs_p
    // and we insist that the record size be the standard record size
    if (bytesPerRec_p != bs_p->nbytes()) {
	throw(AipsError("SDDBootStrap::fill - invalid bytes per record in bootstrap block"));
    }
    // it must be either DATA or RECORDS
    if (type_p != DATA && type_p != RECORDS) {
	throw(AipsError("SDDBootStrap::fill - invalid file type indiated in bootstrap"));
    }
    // maxEntryUsed_p must be less than the maximum allowed
    if (maxEntryUsed_p > maxEntries()) {
	throw(AipsError("SDDBootStrap::fill - bootstrap indicates that the highest "
			"index entry in use is more than can fit in the file"));
    }
}

void SDDBootStrap::write()
{
    // increment counter on all writes
    ++counter_p;

    // Do nothing else if not attached
    if (theFile_p.null()) return;

    // switch on version and fill the block with the values
    // there should be no need to do any sanity checks here
    if (version_p == SDDBootStrap::CURRENT) {
	bs_p->asInt(SDDBootStrap::N_INDEX_REC)     = nIndexRec_p;
	bs_p->asInt(SDDBootStrap::N_DATA_REC)      = nDataRec_p;
	bs_p->asInt(SDDBootStrap::BYTES_PER_REC)   = bytesPerRec_p;
	bs_p->asInt(SDDBootStrap::BYTES_PER_ENTRY) = bytesPerEntry_p;
	bs_p->asInt(SDDBootStrap::MAX_ENTRY_USED)  = maxEntryUsed_p;
	bs_p->asInt(SDDBootStrap::COUNTER)         = counter_p;
	bs_p->asInt(SDDBootStrap::TYPE)            = type_p;
	bs_p->asInt(SDDBootStrap::VERSION)         = version_p;
    } else if (version_p == SDDBootStrap::ORIGINAL) {
	bs_p->asShort(SDDBootStrap::N_INDEX_REC)     = nIndexRec_p;
	bs_p->asShort(SDDBootStrap::N_DATA_REC)      = nDataRec_p;
	bs_p->asShort(SDDBootStrap::BYTES_PER_REC)   = bytesPerRec_p;
	bs_p->asShort(SDDBootStrap::BYTES_PER_ENTRY) = bytesPerEntry_p;
	bs_p->asShort(SDDBootStrap::MAX_ENTRY_USED)  = maxEntryUsed_p;
	bs_p->asShort(SDDBootStrap::COUNTER)         = counter_p;
	bs_p->asShort(SDDBootStrap::TYPE)            = type_p;
	bs_p->asShort(SDDBootStrap::VERSION)         = version_p;
    } else {
	// this should never happen
	throw(AipsError("SDDBootStrap::write - internal error"));
    }

    // First, actually do a sync on the stream
    // theFile_p->rdbuf()->sync();
    theFile_p->flush();

    // seek to start of file
    theFile_p->seekg(0);

    // and write it out
    if (bs_p->write(*theFile_p) != 1) {
	throw(AipsError("SDDBootStrap::write - incorrect number of records written"));
    }
}

Bool SDDBootStrap::hasChanged()
{
    // if not attached, return False
    if (theFile_p.null()) return False;

    // First, actually do a sync on the stream
    // theFile_p->rdbuf()->sync();
    theFile_p->flush();

    // seek to start of fill
    theFile_p->seekg(0);

    // read into the block
    if (bs_p->read(*theFile_p) != 1) {
	throw(AipsError("SDDBootStrap::hasChanged- incorrect number of records read"));
    }

    // Assume the version is the same for now, if not, this will find out shortly
    Bool result;
    if (version_p == SDDBootStrap::CURRENT) {
	result = (
	    (Int(nIndexRec_p)     != bs_p->asInt(SDDBootStrap::N_INDEX_REC))     ||
	    (Int(nDataRec_p)      != bs_p->asInt(SDDBootStrap::N_DATA_REC))      ||
	    (Int(bytesPerRec_p)   != bs_p->asInt(SDDBootStrap::BYTES_PER_REC))   ||
	    (Int(bytesPerEntry_p) != bs_p->asInt(SDDBootStrap::BYTES_PER_ENTRY)) ||
	    (Int(maxEntryUsed_p)  != bs_p->asInt(SDDBootStrap::MAX_ENTRY_USED))  ||
	    (Int(counter_p)       != bs_p->asInt(SDDBootStrap::COUNTER))         ||
	    (Int(type_p)          != bs_p->asInt(SDDBootStrap::TYPE)));
    } else {
	result = (
	    (Int(nIndexRec_p)     != bs_p->asShort(SDDBootStrap::N_INDEX_REC))     ||
	    (Int(nDataRec_p)      != bs_p->asShort(SDDBootStrap::N_DATA_REC))      ||
	    (Int(bytesPerRec_p)   != bs_p->asShort(SDDBootStrap::BYTES_PER_REC))   ||
	    (Int(bytesPerEntry_p) != bs_p->asShort(SDDBootStrap::BYTES_PER_ENTRY)) ||
	    (Int(maxEntryUsed_p)  != bs_p->asShort(SDDBootStrap::MAX_ENTRY_USED))  ||
	    (Int(counter_p)       != bs_p->asShort(SDDBootStrap::COUNTER))         ||
	    (Int(type_p)          != bs_p->asShort(SDDBootStrap::TYPE)));
    }

    return result;
}
   
