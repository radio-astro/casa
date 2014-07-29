//# SDDBlock.h: A Block<uChar> with some helpful functions for SDD files.
//# Copyright (C) 1995,1999,2001
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
//#
//# $Id$

#ifndef NRAO_SDDBLOCK_H
#define NRAO_SDDBLOCK_H

#include <casa/aips.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>

#include <casa/namespace.h>
//# Forward Declarations
#include <casa/iosfwd.h>

#include <casa/namespace.h>
// <summary> 
// SDDBlock handles the IO and value extraction from SDD records (blocks).
// </summary>

// <use visibility=local> 

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> SDDFile
// <li> Block
// <li> iostreams
// <li> The SDD file format.
// </prerequisite>
//
// <etymology>
// SDD stands for Single Dish Data.  It is the data format that UniPOPS
// understands.  It is also the on-line data format at the NRAO 12m.
// SDD files are organized in records or blocks.  This class handles the
// io to and from those blocks.  it is used by the other SDD classes.
// </etymology>
//
// <synopsis> 
// SDDBlock is used primarily to handle the IO to and from an SDD file
// in units of an SDD record size.  The standard rccord size if 512
// bytes although this class can handle arbitrarily sized records so
// long as the size is an integer multiple of 8 bytes (the size of
// and IEEE double precision floating point number).  The total size
// of an SDDBlock is an integer multiple of the record size (i.e. no
// fractional records are allowed).  This class also aids in extracting
// values of different types from the block.  This is not likely to
// be immediately portable.  However, by isolating that conversion here,
// it should make any future ports easier.  Since SDD files will only
// ever be written on Suns, its unlikely that this issue will be a
// problem.
//
// The IO only involves iostreams and not fstream.  It is intended
// that the public SDD classes handle all of the fstream specific
// operations and that this class does not need to do any seeks.
//
// An SDDBlock is always at least 1 SDD record in size.
// </synopsis> 
//
// <example>
// This is a somewhat contrived example.
// <srcblock>
// // This istream would be initialized appropriately
// istream in;
// // construct an SDDBlock of 1 SDD record using the standard record size
// SDDBlock block;
// block.read(in, 1);
// // Extract the 5th double from the record, which indicates the size
// // of the full record to read in bytes, convert that to SDD records
// // read it in.   This would need some additional checks if partial
// // records might be indicated.
// double fullSize = block.asdouble(4);
// uInt nrecs = uInt(fullSize/block.bytesPerRecord());
// // read the remaining records
// block.read(in, (nrecs-1));
// </srcblock>
// </example>
//
// <motivation>
// SDD files are organized into units that are integer number of
// records (always of 512 bytes at this point in history, but they
// could theoretically be of a different size).  It makes sense to
// have a class that the other SDD classes could use to deal with
// the data in those chunks.
// </motivation>
//
// <thrown>
// <li> indexError - by the asT functions but only when AIPS_ARRAY_INDEX_CHECK is on.
// <li> AllocError - during construction
// <li> AipsError - by read/write indicates an IO error
// <li> AipsError - by copy and write if the indicated size is invalid
// </thrown>
//
// <todo asof="">
// </todo>

class SDDBlock
{
public: 
    // SDD files have traditionally been composed of 512 byte records, but they are not
    // not really required to be.
    enum {STANDARD_RECORD_SIZE = 512};

    // An SDDBlock with the indicated number of SDD records, each record having
    // the indicated size in bytes, the whole thing filled with zeros.
    SDDBlock(uInt n = 1, uInt bytesPerRecord = STANDARD_RECORD_SIZE);
    // copy constructor, this makes a true copy.
    SDDBlock(const SDDBlock& other);

    ~SDDBlock();
        
    // assignment operator, this makes a true copy
    SDDBlock& operator=(const SDDBlock& other);

    // return the size in SDD records
    uInt nRecords() const {return cblk_p->nelements()/bytesPerRecord_p;}
    // return the number of bytes
    uInt nbytes() const { return cblk_p->nelements();}
    // return the number of bytes per record
    uInt bytesPerRecord() const { return bytesPerRecord_p;}

    // resize this block to the indicated number of SDD records
    // forceSmaller and copyElements work as with the Block class
    void resize(uInt nRecords, Bool forceSmaller = False,
		     Bool copyElements = False);

    // reshape is like resize, except that the number of bytes per record can also
    // change.  The number of bytesPerRecord must be a multiple of 8.
    void reshape(uInt nRecords, uInt bytesPerRecord,
		 Bool forceSmaller = False, Bool copyElements = False);

    // read from istream and fill until this SDDBlock is full or istream is exhausted
    // return the number of records actually read 
    uInt read(istream& in);
    // read nrecord records from istream, filling starting at the indicated record
    // stop if istream is exhausted, return the number of records actually read
    uInt read(istream& in, uInt nrecord, uInt startRecord = 0);

    // read the indicated number of bytes from in, starting at the optionally indicated
    // starting bytes offset, returns the number of bytes actually read
    uInt readBytes(istream& in, uInt nbytes, uInt startOffset = 0);

    // write the entire SDDBlock to ostream, return the number of records written
    uInt write(ostream& out) const;
    // write nrecord records to ostream, starting at the indicated record
    // return the number of records written
    uInt write(ostream& out, uInt nrecord, uInt startRecord = 0) const;

    // write the indicated number of bytes to out, starting at the optionally indicated
    // starting byte offset, returns the number of bytes actually written
    uInt writeBytes(ostream& out, uInt nbytes, uInt startOffset = 0) const;

    // copy ncopy character to another SDDBlock starting at outStart (characters) 
    // offset into and starting at inStart (characters) offset into this blcck
    void copy(SDDBlock& outBlock, uInt ncopy,
	      uInt outStart = 0, uInt inStart = 0) const;

    // index into the block 
    Char& operator[](uInt index) {return (*cblk_p)[index];}
    const Char& operator[](uInt index) const {return (*cblk_p)[index];}

    // Index into the block as Shorts (0-based)
    Short& asShort(uInt index);
    const Short& asShort(uInt index) const;
    // Index into the block as Ints (0-based)
    Int& asInt(uInt location);
    const Int& asInt(uInt location) const;
    // Index into the block as floats (0-based)
    float& asfloat(uInt location);
    const float& asfloat(uInt location) const;
    // Index into the block as doubles (0-based)
    double& asdouble(uInt location);
    const double& asdouble(uInt location) const;
    
private:
    Block<Char> *cblk_p;
    Short *sarray;
    Int *iarray;
    float *farray;
    double *darray;

    uInt bytesPerRecord_p;

    void set_pointers();
};

inline Short& SDDBlock::asShort(uInt index)
{
#if defined(AIPS_ARRAY_INDEX_CHECK)
    if ((cblk_p->nelements() == 0) || 
	 (index > (cblk_p->nelements()/sizeof(Short)) - 1)) {
	throw(indexError<Short>(index, "SDDBlock::asShort(uInt index) - "
				"index out of range"));
    }
#endif
    return sarray[index];
}

inline const Short& SDDBlock::asShort(uInt index) const
{
#if defined(AIPS_ARRAY_INDEX_CHECK)
    if ((cblk_p->nelements() == 0) || 
	 (index > (cblk_p->nelements()/sizeof(Short)) - 1)) {
	throw(indexError<Short>(index, "SDDBlock::asShort(uInt index) - "
				"index out of range"));
    }
#endif
    return sarray[index];
}

inline Int& SDDBlock::asInt(uInt index)
{
#if defined(AIPS_ARRAY_INDEX_CHECK)
    if ((cblk_p->nelements() == 0) || 
	 (index > (cblk_p->nelements()/sizeof(Int)) - 1)) {
	throw(indexError<Int>(index, "SDDBlock::asInt(uInt index) - "
			      "index out of range"));
    }
#endif
    return iarray[index];
}

inline const Int& SDDBlock::asInt(uInt index) const
{
#if defined(AIPS_ARRAY_INDEX_CHECK)
    if ((cblk_p->nelements() == 0) || 
	 (index > (cblk_p->nelements()/sizeof(Int)) - 1)) {
	throw(indexError<Int>(index, "SDDBlock::asInt(uInt index) - "
			      "index out of range"));
    }
#endif
    return iarray[index];
}


inline float& SDDBlock::asfloat(uInt index)
{
#if defined(AIPS_ARRAY_INDEX_CHECK)
    if ((cblk_p->nelements() == 0) || 
	 (index > (cblk_p->nelements()/sizeof(float)) - 1)) {
	throw(indexError<float>(index, "SDDBlock::asfloat(uInt index) - "
				"index out of range"));
    }
#endif
    return farray[index];
}

inline const float& SDDBlock::asfloat(uInt index) const
{
#if defined(AIPS_ARRAY_INDEX_CHECK)
    if ((cblk_p->nelements() == 0) || 
	 (index > (cblk_p->nelements()/sizeof(float)) - 1)) {
	throw(indexError<float>(index, "SDDBlock::asfloat(uInt index) - "
				"index out of range"));
    }
#endif
    return farray[index];
}

inline double& SDDBlock::asdouble(uInt index)
{
#if defined(AIPS_ARRAY_INDEX_CHECK)
    if ((cblk_p->nelements() == 0) || 
	 (index > (cblk_p->nelements()/sizeof(double)) - 1)) {
	throw(indexError<double>(index, "SDDBlock::asdouble(uInt index) - "
				 "index out of range"));
    }
#endif
    return darray[index];
}

inline const double& SDDBlock::asdouble(uInt index) const
{
#if defined(AIPS_ARRAY_INDEX_CHECK)
    if ((cblk_p->nelements() == 0) || 
	 (index > (cblk_p->nelements()/sizeof(double)) - 1)) {
	throw(indexError<double>(index, "SDDBlock::asdouble(uInt index) - "
				 "index out of range"));
    }
#endif
    return darray[index];
}

inline uInt SDDBlock::readBytes(istream& in, uInt nbytes, uInt startOffset)
{
    // is a resize necessary, copy elements if it is necessary
    // this may be wastefull, if so, use SDDBlock::resize() first without copying
    if (nbytes + startOffset - 1 > cblk_p->nelements())
	resize(nbytes+startOffset-1, False, True);

    in.read((cblk_p->storage() + startOffset), nbytes);
    return in.gcount();
}

inline uInt SDDBlock::read(istream& in)
{
    // attempt to read from istream until Block is full
    uInt nchars = readBytes(in, cblk_p->nelements());
    if (nchars%bytesPerRecord_p) {
	throw(AipsError("SDDBlock::read(istream& in) - partial record read"));
    }
    return nchars/bytesPerRecord_p;
}

inline uInt SDDBlock::read(istream& in, uInt nrecord, uInt startRecord)
{
    uInt nchars = 
	readBytes(in, nrecord*bytesPerRecord_p, startRecord*bytesPerRecord_p);

    if (nchars%bytesPerRecord_p) {
	throw(AipsError("SDDBlock::read(istream& in, uInt nrecord, uInt startRecord) - "
			"partial record read"));
    }
    return nchars/bytesPerRecord_p;
}

inline uInt SDDBlock::writeBytes(ostream& out, uInt nbytes, uInt startOffset) const
{
    // make sure we have what has been requested
    if (nbytes+startOffset-1 > cblk_p->nelements()) {
	throw(AipsError("SDDBlock::write - request exceeds current size of SDDBlock"));
    }
    out.write(cblk_p->storage() + startOffset, nbytes);
    return nbytes;
}

inline uInt SDDBlock::write(ostream& out) const
{
    // write all of Block to ostream
    return (writeBytes(out, cblk_p->nelements()))/bytesPerRecord_p;
}

inline uInt SDDBlock::write(ostream& out, uInt nrecord, uInt startRecord) const
{
    writeBytes(out, nrecord*bytesPerRecord_p, startRecord*bytesPerRecord_p);
    return nrecord;
}

inline void SDDBlock::copy(SDDBlock& out, uInt ncopy, uInt outStart, uInt inStart) const
{
    if ((outStart+ncopy-1) > out.nbytes() ||
	(inStart+ncopy-1) > nbytes()) {
	throw (AipsError("SDDBlock::copy(SDDBlock& out, uInt, uInt, uInt) - "
			 "copy would extend beyond end of one or both blocks"));
    }
    const Char* incptr = cblk_p->storage() + inStart;
    Char* outcptr = out.cblk_p->storage() + outStart;
    for (uInt i=0;i<ncopy;i++, incptr++, outcptr++) *outcptr = *incptr;
}

#endif
