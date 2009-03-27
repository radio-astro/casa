//# SDDBlock.cc:  This implements SDDBlock, a Block<uChar> with some helpful functions for SDD files.
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

#include <nrao/SDD/SDDBlock.h>
#include <casa/iostream.h>

SDDBlock::SDDBlock(uInt n, uInt bytesPerRecord)
    : cblk_p(0), sarray(0), iarray(0), farray(0), darray(0), 
      bytesPerRecord_p(bytesPerRecord)
{
    // verify that bytesPerRecord_p is a multiple of 8
    if (bytesPerRecord_p % 8) {
	throw(AipsError("SDDBlock::SDDBlock(uInt n, uInt bytesPerRecord) - "
			"bytesPerRecord not an integer multiple of 8"));
    }
    cblk_p = new Block<Char>(n*bytesPerRecord_p, '\0');
    if (cblk_p == 0) {
	throw(AllocError("SDDBlock::SDDBlock(uInt n) - "
			 "new of Block<char> fails", n*bytesPerRecord_p));
    }
    // and set the other pointers
    set_pointers();
}

SDDBlock::SDDBlock(const SDDBlock& other)
    : cblk_p(0), sarray(0), iarray(0), farray(0), darray(0), 
      bytesPerRecord_p(other.bytesPerRecord_p)
{
    cblk_p = new Block<char>(*(other.cblk_p));
    if (cblk_p == 0) {
	throw(AllocError("SDDBlock::SDDBlock(const SDDBlock& other) - "
			 "new of Block<char> fails",other.cblk_p->nelements()));
    }
    // and set the other pointers
    set_pointers();
}


SDDBlock::~SDDBlock()
{
    delete cblk_p;
    cblk_p = 0;
    sarray = 0;
    iarray = 0;
    farray = 0;
    darray = 0;
}

void SDDBlock::set_pointers()
{
    Char* cptr = (Char *)cblk_p->storage();
    sarray = (Short *)cptr;
    iarray = (Int *)cptr;
    farray = (float *)cptr;
    darray = (double *)cptr;
}

SDDBlock& SDDBlock::operator=(const SDDBlock& other)
{
    if (this != &other) {
	*cblk_p = *(other.cblk_p);
	bytesPerRecord_p = other.bytesPerRecord_p;
	set_pointers();
    }
    return *this;
}

void SDDBlock::resize(uInt nRecords, Bool forceSmaller, Bool copyElements)
{
    cblk_p->resize(nRecords*bytesPerRecord_p, 
		   forceSmaller, copyElements);
    set_pointers();
}

void SDDBlock::reshape(uInt nRecords, uInt bytesPerRecord, Bool forceSmaller,
		       Bool copyElements)
{
    // verify that bytesPerRecord is a multiple of 8
    if (bytesPerRecord % 8) {
	throw(AipsError("SDDBlock::reshape - "
			"bytesPerRecord not an integer multiple of 8"));
    }
    bytesPerRecord_p = bytesPerRecord;
    resize(nRecords, forceSmaller, copyElements);
}



