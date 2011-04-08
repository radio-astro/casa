//# SDDHeader.cc: defines SDDHeader which contains the SDDFile header information
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

#include <nrao/SDD/SDDHeader.h>
#include <nrao/SDD/SDDBlock.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicMath/Math.h>
#include <casa/iostream.h>

// Vector<uInt> SDDHeader::nchars_p;

unsigned long linfinity[2] = {0x7ff00000, 0x00000000};
#define DINFINITY (*(double *)linfinity)

SDDHeader::SDDHeader() 
    : preamble_p(0), hdu_p(0), isOtf_p(False), nvectors_p(0)
{
    // set up the nchars vector used to determine how many characters each
    // string field has
    init_nchars();

    // create an empty thing capable of holding the header but no data
    if ((preamble_p = new Vector<Int>(16)) == 0) {
	throw(AllocError("SDDHeader::SDDHeader() unable to allocated preamble space",16));
    }

    (*preamble_p)(0) = 13;
    (*preamble_p)(1) = 4;
    (*preamble_p)(2) = (*preamble_p)(1) + LAST_BASIC;
    (*preamble_p)(3) = (*preamble_p)(2) + LAST_POINTING;
    (*preamble_p)(4) = (*preamble_p)(3) + LAST_OBSERVING;
    (*preamble_p)(5) = (*preamble_p)(4) + LAST_POSITIONS;
    (*preamble_p)(6) = (*preamble_p)(5) + LAST_ENVIRONMENT;
    (*preamble_p)(7) = (*preamble_p)(6) + LAST_MAP;
    (*preamble_p)(8) = (*preamble_p)(7) + LAST_DATA;
    (*preamble_p)(9) = (*preamble_p)(8) + LAST_ENGINEERING;
    (*preamble_p)(10)= (*preamble_p)(9) + max(LAST_GREENBANK, LAST_TUCSON);
    (*preamble_p)(11)= (*preamble_p)(10)+ nchars_p(OPENPAR)/sizeof(double);
    // as in current default unipops, make it large enough to hold 10 new class 11s
    (*preamble_p)(12)= (*preamble_p)(11)+ LAST_NEW_PHASE_BASICS
	                                + LAST_NEW_PHASE * 10;
    (*preamble_p)(13)= (*preamble_p)(12)+ LAST_RECEIVER;
    (*preamble_p)(14)= (*preamble_p)(13)+ LAST_REDUCTION;
    (*preamble_p)(15)= (*preamble_p)(14);
    init_strOffset();

    uInt headerLength = (*preamble_p)(15) * sizeof(double);
    // and create an empty block of the appropriate size, with no room for data
    if ((hdu_p = new SDDBlock(1, headerLength)) == 0) {
	throw(AllocError("SDDHeader::SDDHeader() unable to create empty block of "
			 "required number of bytes", headerLength));
    }
    // put the preamble into it
    putPreamble();
    // set the header and data size
    put(HEADLEN, headerLength);
    put(DATALEN, 0);
    // and empty it (set to default values)
    empty();
}


SDDHeader::SDDHeader(istream& in)
    : preamble_p(0), hdu_p(0), isOtf_p(False), nvectors_p(0)
{
    // set up the nchars vector used to determine how many characters each
    // string field has
    init_nchars();

    // create the preamble
    if ((preamble_p = new Vector<Int>(16)) == 0) {
	throw(AllocError("SDDHeader::SDDHeader() unable to allocated preamble space",16));
    }

    // create an SDDBlock of 1 standard record to start with
    if ((hdu_p = new SDDBlock(1)) == 0) {
	throw(AllocError("SDDHeader::SDDHeader(istream& in) unable to allocate a single "
			 "standard record SDDBlock",1));
    }
    // and fill it
    uInt nbytes = fill(in);
    if (nbytes != uInt(get(HEADLEN))) {
	throw(AipsError("SDDHeader::SDDHeader(istream& in) error reading data"));
    }
}

SDDHeader::SDDHeader(const SDDHeader& other)
    : preamble_p(0), hdu_p(0), isOtf_p(other.isOtf_p), 
      nvectors_p(other.nvectors_p), strOffset_p(other.strOffset_p)
{
    if ((preamble_p = new Vector<Int>(*other.preamble_p)) == 0) {
	throw(AllocError("SDDHeader::SDDHeader(const SDDHeader& other) unable to allocate "
			"space for preamble vector", 16));
    }
    if ((hdu_p = new SDDBlock(*other.hdu_p)) == 0) {
	throw(AllocError("SDDHeader::SDDHeader(const SDDHeader& other) unable to allocate "
			 "space for hdu block", 1));
    }
}

SDDHeader::~SDDHeader()
{
    delete preamble_p;
    delete hdu_p;
}

SDDHeader& SDDHeader::operator=(const SDDHeader& other)
{
    if (this == &other) return *this;

    preamble_p->resize(other.preamble_p->shape());
    *preamble_p = *(other.preamble_p);

    strOffset_p = other.strOffset_p;

    *hdu_p = *other.hdu_p;

    isOtf_p = other.isOtf_p;
    nvectors_p = other.nvectors_p;

    return *this;
}

uInt SDDHeader::fill(istream& in)
{
    uInt nout = 0;
    if (hdu_p->readBytes(in,SDDBlock::STANDARD_RECORD_SIZE) != 
	SDDBlock::STANDARD_RECORD_SIZE) {
	throw(AipsError("SDDHeader::fill(istream& in) error reading first record"));
    }
    nout += SDDBlock::STANDARD_RECORD_SIZE;
    // extract the preamble, getPreamble does sanity checks
    getPreamble();
    init_strOffset();

    
    // reshape to the number of bytes required by the preamble
    uInt headlen = uInt(get(HEADLEN));
    hdu_p->reshape(1, headlen, False, True);

    // and read the rest
    nout += hdu_p->readBytes(in,(headlen - SDDBlock::STANDARD_RECORD_SIZE),
			     SDDBlock::STANDARD_RECORD_SIZE);

    // and set the cached info
    isOtf_p = (get(OBSMODE) == "LINEOTF ");
    uInt nelements = uInt(get(DATALEN)/sizeof(float));
    uInt nchan = uInt(get(NOINT));
    // the nvectors calculation is different for OTF data
    if (nchan == 0) {
	nvectors_p = 0;
    } else {
	if (isOTF()) {
	    nvectors_p = nelements / (nchan + 5);
	} else {
	    nvectors_p = nelements / nchan;
	}
    }
    return nout;
}

uInt SDDHeader::write(ostream& out) const
{
    return (hdu_p->writeBytes(out,uInt(get(HEADLEN))));
}

void SDDHeader::empty()
{
    // number of doubles in header
    uInt headlen = uInt(get(HEADLEN));
    // reshape hdu_p
    hdu_p->reshape(1, headlen);
    // how many doubles to set to default values
    uInt ndoubles = headlen / sizeof(double);
    for (uInt i = 0;i<ndoubles;i++) hdu_p->asdouble(i) = DINFINITY;

    // do the strings separately
    for (uInt j = 0;j<LAST_STRING;j++) {
	Int offset = strOffset_p(j);
	if (offset > 0) {
	    for (uInt k=0;k<nchars_p(j);k++) {
		(*hdu_p)[offset + k] = ' ';
	    }
	}
    }

    // Put the preamble back in
    putPreamble();

    // and reset HEADLEN to the saved value above and DATALEN to zero
    put(HEADLEN, headlen);
    put(DATALEN, 0);

    // finally, reset the cached info
    isOtf_p = False;
    nvectors_p = 0;
}


Bool SDDHeader::is12m() const
{
    return  (get(TELESCOP) == "NRAO 12M");
}

Bool SDDHeader::is140ft() const
{
    return  (get(TELESCOP) == "NRAO 43M");
}

Bool SDDHeader::isNRAO() const
{
    return  (is12m() || is140ft());
}

Bool SDDHeader::isOTF() const
{
    return isOtf_p;
}

uInt SDDHeader::nvectors() const
{
    return nvectors_p;
}

String SDDHeader::get(StringHeader field) const
{
    uInt nchar = nchars_p(field);
    uInt offset = strOffset_p(field);
    String tmp(" ",nchar);
    if (offset > 0) {
	for (uInt i=0;i<nchar;i++) tmp[i] = (*hdu_p)[offset+i];
    }
    return tmp;
}

Bool SDDHeader::put(StringHeader field, const String& value)
{
    uInt nchar = nchars_p(field);
    uInt offset = strOffset_p(field);
    if (offset < 0 || nchar < value.length()) return False;

    for (uInt i=0;i<nchar && i<value.length();i++) {
	if (value[i] != '\0') (*hdu_p)[offset+i] = value[i];
	else (*hdu_p)[offset+i] = ' ';
    }
    if (nchar > value.length()) {
	for (uInt i=value.length();i<nchar;i++) (*hdu_p)[offset+i]=' ';
    }

    if (get(OBSMODE) == "LINEOTF ") isOtf_p = True;

    return True;
}

Double SDDHeader::get(BasicInformation field) const
{
    // class 1
    return getFromClass(1, field);
}

Bool SDDHeader::put(BasicInformation field, const Double& value)
{
    // class 1
    Bool tmp = putToClass(1, field, value);
    if (tmp && field == DATALEN) {
	uInt nmax = uInt(get(DATALEN)/sizeof(float));
	if (get(NOINT) > nmax) tmp = put(NOINT, nmax);
	if (uInt(get(NOINT)) == 0) {
	    nvectors_p = 0;
	} else {
	    if (isOTF() && tmp)
		nvectors_p = nmax/(uInt(get(NOINT))+5);
	    else if (tmp)
		nvectors_p = nmax/uInt(get(NOINT));
	}
    }
    return tmp;
}

Double SDDHeader::get(PointingParameters field) const
{
    // class 2
    return getFromClass(2, field);
}

Bool SDDHeader::put(PointingParameters field, const Double& value)
{
    // class 2
    return putToClass(2, field, value);
}

Double SDDHeader::get(ObservingParameters field) const
{
    // class 3
    return getFromClass(3, field);
}

Bool SDDHeader::put(ObservingParameters field, const Double& value)
{
    // class 3
    return putToClass(3, field, value);
}

Double SDDHeader::get(Positions field) const
{
    // class 4
    return getFromClass(4, field);
}

Bool SDDHeader::put(Positions field, const Double& value)
{
    // class 4
    return putToClass(4, field, value);
}

Double SDDHeader::get(Environment field) const
{
    // class 5
    return getFromClass(5, field);
}

Bool SDDHeader::put(Environment field, const Double& value)
{
    // class 5
    return putToClass(5, field, value);
}

Double SDDHeader::get(MapParameters field) const
{
    // class 6
    return getFromClass(6, field);
}

Bool SDDHeader::put(MapParameters field, const Double& value)
{
    // class 6
    return putToClass(5, field, value);
}

Double SDDHeader::get(DataParameters field) const
{
    // class 7
    return getFromClass(7, field);
}

Bool SDDHeader::put(DataParameters field, const Double& value)
{
    // class 7
    return putToClass(7, field, value);
}

Double SDDHeader::get(EngineeringParameters field) const
{
    // class 8
    return getFromClass(8, field);
}

Bool SDDHeader::put(EngineeringParameters field, const Double& value)
{
    // class 8
    return putToClass(8, field, value);
}

Double SDDHeader::get(GreenBank field) const
{
    // class 9, NRAO, non-12m
    Double tmp = DINFINITY;
    if (isNRAO() && !is12m()) tmp = getFromClass(9, field);
    return tmp;
}

Bool SDDHeader::put(GreenBank field, const Double& value)
{
    // class 9, NRAO, non-12m
    Bool tmp = (isNRAO() && !is12m());
    if (tmp) tmp = putToClass(9, field, value);
    return tmp;
}

Double SDDHeader::get(Tucson field) const
{
    // class 9, 12m
    Double tmp = DINFINITY;
    if (is12m()) tmp = getFromClass(9, field);
    return tmp;
}

Bool SDDHeader::put(Tucson field, const Double& value)
{
    // class 9, 12m
    Bool tmp = is12m();
    if (tmp) tmp = putToClass(9, field, value);
    return tmp;
}

Double SDDHeader::get(ReceiverBlock field) const
{
    // class 12
    return getFromClass(12, field);
}

Bool SDDHeader::put(ReceiverBlock field, const Double& value)
{
    // class 12
    Bool tmp = putToClass(12, field, value);
    if (tmp && field == NOINT) {
	uInt nmax = uInt(get(DATALEN)/sizeof(float));
	if (get(NOINT) > nmax) tmp = put(DATALEN,nmax*sizeof(float));
	if (uInt(get(NOINT)) == 0) {
	    nvectors_p = 0;
	} else {
	    if (isOTF() && tmp)
		nvectors_p = nmax/(uInt(get(NOINT))+5);
	    else if (tmp)
		nvectors_p = nmax/uInt(get(NOINT));
	}
    }
    return tmp;
}

Double SDDHeader::get(ReductionParameters field) const
{
    // class 13
    return getFromClass(13, field);
}

Bool SDDHeader::put(ReductionParameters field, const Double& value)
{
    // class 13
    return putToClass(13, field, value);
}

Double SDDHeader::get(OriginalPhaseBlock field, uInt whichBlock) const
{
    // original class 11, first, does this really exist
    Double tmp = DINFINITY;
    if (whichBlock < get(NOSWVAR)) {
	// there are 3 Doubles per each class 11 block
	uInt fieldOffset = 3*whichBlock;
	tmp = getFromClass(11, fieldOffset+field);
    }
    return tmp;
}

Bool SDDHeader::put(OriginalPhaseBlock field, const Double& value, uInt whichBlock)
{
    // original class 11, first, does this really exist
    Bool tmp = (whichBlock < get(NOSWVAR));
    if (tmp) {
	uInt fieldOffset = 3*whichBlock;
	tmp = putToClass(11,fieldOffset+field, value);
    }
    return tmp;
}

String SDDHeader::get(OriginalPhaseBlockStrings field, uInt whichBlock) const
{
    String tmp(" ",8);
    if (whichBlock < get(NOSWVAR)) {
	uInt fieldOffset = 3*whichBlock;
	uInt offset = setOffset(11,fieldOffset + field);
	if (offset != 0) {
	    for (uInt i=0;i<8;i++) tmp[i] = (*hdu_p)[offset+i];
	}
    }
    return tmp;
}

Bool SDDHeader::put(OriginalPhaseBlockStrings field, const String& value, 
	   uInt whichBlock)
{
    Bool tmp = (whichBlock < get(NOSWVAR));
    if (tmp) {
	uInt fieldOffset = 3*whichBlock;
	uInt nchar = 8;
	uInt offset = setOffset(11,fieldOffset + field);
	if (tmp = (offset != 0 && nchar > value.length())) {
	    for (uInt i=0;i<nchar && i<value.length();i++) {
		if (value[i] != '\0') (*hdu_p)[offset+i] = value[i];
		else (*hdu_p)[offset+i] = ' ';
	    }
	    if (nchar > value.length()) {
		for (uInt i=value.length();i<nchar;i++) (*hdu_p)[offset+i]=' ';
	    }
	}
    }
    return tmp;
}

Double SDDHeader::get(NewPhaseBlockBasics field) const
{
    // class 11
    return getFromClass(11, field);
}

Bool SDDHeader::put(NewPhaseBlockBasics field, const Double& value)
{
    // class 11
    return putToClass(11, field, value);
}

Double SDDHeader::get(NewPhaseBlock field, uInt whichBlock) const
{
    // new class 11, first, does this really exist
    Double tmp = DINFINITY;
    if (whichBlock < (get(NOSWVAR) + get(NOSWVARF))) {
	// there are 6 Doubles per each new class 11 block
	uInt fieldOffset = 6*whichBlock;
	tmp = getFromClass(11, fieldOffset+field);
    }
    return tmp;
}

Bool SDDHeader::put(NewPhaseBlock field, const Double& value, uInt whichBlock)
{
    // new class 11, first, does this really exist
    Bool tmp = (whichBlock < (get(NOSWVAR) + get(NOSWVARF)));
    if (tmp) {
	uInt fieldOffset = 6*whichBlock;
	tmp = putToClass(11, fieldOffset+field, value);
    }
    return tmp;
}

String SDDHeader::get(NewPhaseBlockStrings field, uInt whichBlock) const
{
    uInt nchar;
    switch (field) {
    case PHASTB:
	nchar = 32;
	break;
    default:
	nchar = 8;
    }

    String tmp(" ",nchar);
    if (whichBlock < (get(NOSWVAR) + get(NOSWVARF))) {
	uInt fieldOffset = 6*whichBlock;
	uInt offset = setOffset(11,fieldOffset + field);
	if (offset != 0) {
	    for (uInt i=0;i<nchar;i++) tmp[i] = (*hdu_p)[offset+i];
	}
    }
    return tmp;
}

Bool SDDHeader::put(NewPhaseBlockStrings field, const String& value, 
	   uInt whichBlock)
{
    uInt nchar;
    switch (field) {
    case PHASTB:
	nchar = 32;
	break;
    default:
	nchar = 8;
    }
    Bool tmp = (whichBlock < get(NOSWVAR));
    if (tmp) {
	uInt fieldOffset = 6*whichBlock;
	uInt offset = setOffset(11,fieldOffset + field);
	if (tmp = (offset != 0 && nchar > value.length())) {
	    for (uInt i=0;i<nchar && i<value.length();i++) {
		if (value[i] != '\0') (*hdu_p)[offset+i] = value[i];
		else (*hdu_p)[offset+i] = ' ';
	    }
	    if (nchar > value.length()) {
		for (uInt i=value.length();i<nchar;i++) (*hdu_p)[offset+i]=' ';
	    }
	}
    }
    return tmp;
}

Double SDDHeader::getFromClass(uInt whichClass, uInt offset) const
{
    Double tmp = DINFINITY;
    if (Int(whichClass) < (*preamble_p)(0) && whichClass < 15) {
	offset = offset + (*preamble_p)(whichClass);
	if (Int(offset) < (*preamble_p)(whichClass+1)) 
	    tmp = (*hdu_p).asdouble(offset);
    }
    return tmp;
}

Bool SDDHeader::putToClass(uInt whichClass, uInt offset, const Double& value)
{
    Bool tmp = (Int(whichClass) < (*preamble_p)(0) && whichClass < 15);
    if (tmp) {
	offset = offset + (*preamble_p)(whichClass);
	tmp = (Int(offset) < (*preamble_p)(whichClass+1));
	if (tmp) (*hdu_p).asdouble(offset) = value;
    }
    return tmp;
}

void SDDHeader::init_nchars()
{
    // only do this once, everything is non-zero
    if (nchars_p.nelements() == LAST_STRING) return;
    nchars_p.resize(LAST_STRING);
    // everything is 8 unless it isn't
    for (uInt i=0;i<LAST_STRING;i++) nchars_p(i) = 8;

    nchars_p(OBSERVER) = 16;
    nchars_p(OBJECT) = 16;
    nchars_p(CFFORM) = 24;
    nchars_p(OPENPAR) = 80;
    nchars_p(RX_INFO) = 16;
}

void SDDHeader::init_strOffset()
{
    // this requires that preamble_p exist and be filled
    if (!preamble_p ||
	(*preamble_p)(0) > 15) throw(AipsError("SDDHeader::init_strOffset() "
					       "preamble does not yet exist"));

    if (strOffset_p.nelements() != LAST_STRING) strOffset_p.resize(LAST_STRING);

    strOffset_p(OBSID) = setOffset(1, 3);
    strOffset_p(OBSERVER) = setOffset(1,4);
    strOffset_p(TELESCOP) = setOffset(1,6);
    strOffset_p(PROJID) = setOffset(1,7);
    strOffset_p(OBJECT) = setOffset(1,8);
    strOffset_p(OBSMODE) = setOffset(1,10);
    strOffset_p(FRONTEND) = setOffset(1,11);
    strOffset_p(BACKEND) = setOffset(1,12);
    strOffset_p(PRECIS) = setOffset(1,13);
    strOffset_p(PT_MODEL) = setOffset(2,12);
    strOffset_p(CL11TYPE) = setOffset(3,8);
    strOffset_p(COORDCD) = setOffset(4,16);
    strOffset_p(FRAME) = setOffset(6,10);
    strOffset_p(VELDEF) = setOffset(7,5);
    strOffset_p(TYPECAL) = setOffset(7,6);
    strOffset_p(CFFORM) = setOffset(9,15);
    strOffset_p(OPENPAR) = setOffset(10,0);
    strOffset_p(POLARIZ) = setOffset(12,20);
    strOffset_p(RX_INFO) = setOffset(12,22);
}

uInt SDDHeader::setOffset(uInt whichClass, uInt number) const
{
    uInt offset = 0;
    if (Int(whichClass) < (*preamble_p)(0) && whichClass < 15) {
	offset = (*preamble_p)(whichClass) + number;
	if (Int(offset) >= (*preamble_p)(whichClass+1)) offset = 0;
    }
    return offset*8;
}

void SDDHeader::getPreamble()
{
    for (uInt i=0;i<16;i++)
	(*preamble_p)(i) = uInt(hdu_p->asShort(i));


    // sanity check
    if ((*preamble_p)(0) > 14) {
	throw(AipsError("SDDHeader::getPreamble() too many data classes indicated "
			"in preamble."));
    }
    // adjust if necessary
    Bool adjusted = True;
    while (adjusted) {
	adjusted = False;
	for (uInt i=2;Int(i)<=((*preamble_p)(0));i++) {
	    if ((*preamble_p)(i) < (*preamble_p)(i-1)) {
		adjusted = True;
		(*preamble_p)(i) = (*preamble_p)(i+1);
	    }
	}
    }
    // this simply isn't very general!
    (*preamble_p)(14) = (*preamble_p)(13) + LAST_REDUCTION;
    // and correct from 1-relative to 0-relative
    for (uInt i=1;i<15;i++) (*preamble_p)(i)--;
}

void SDDHeader::putPreamble()
{
    // correct back to 1-relative while putting back
    hdu_p->asShort(0) = (*preamble_p)(0);
    for (uInt i=1;i<16;i++)
	hdu_p->asShort(i) = (*preamble_p)(i)+1;
}
