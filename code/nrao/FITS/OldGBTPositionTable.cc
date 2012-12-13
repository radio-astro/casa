//# OldGBTPositionTable.h: Simplified interface to FITS tables with AIPS++ Look and Feel.
//# Copyright (C) 1995,1997,1998,1999,2001
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

#include <nrao/FITS/OldGBTPositionTable.h>

#include <casa/Containers/RecordField.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <fits/FITS/fits.h>
#include <fits/FITS/fitsio.h>
#include <fits/FITS/hdu.h>

#include <casa/Arrays/Array.h>

static DataType arrayType2scalarType(DataType arrayType)
{
    if (arrayType <= TpTable || arrayType == TpOther)
	return arrayType;

    switch(arrayType) {
    case TpArrayBool: return TpBool;
    case TpArrayChar: return TpChar;
    case TpArrayUChar: return TpUChar;
    case TpArrayShort: return TpShort;
    case TpArrayUShort: return TpUShort;
    case TpArrayInt: return TpInt;
    case TpArrayUInt: return TpUInt;
    case TpArrayFloat: return TpFloat;
    case TpArrayDouble: return TpDouble;
    case TpArrayComplex: return TpComplex;
    case TpArrayDComplex: return TpDComplex;
    case TpArrayString: return TpString;
    default:
       return TpOther;
    }
}


OldGBTPositionTable::OldGBTPositionTable(const String &fileName)
    : row_nr_p(-1), raw_table_p(0), io_p(0), 
      row_p(RecordInterface::Variable), row_fields_p(0)
{
    isValid_p = reopen(fileName);
}

OldGBTPositionTable::~OldGBTPositionTable()
{
    clear_self();
}

Bool OldGBTPositionTable::reopen(const String &fileName)
{
    row_nr_p = -1;
    clear_self();

    name_p = fileName;

    io_p = new FitsInput(fileName.chars(), FITS::Disk);
    if (io_p->err()) {
	return False;
    }

    io_p->skip_hdu();

    if (io_p->err()) {
	return False;
    }
    
    // OK; we have a valid HDU
    if (io_p->hdutype() == FITS::BinaryTableHDU) {
	raw_table_p = new BinaryTableExtension(*io_p);
    } else if (io_p->hdutype() == FITS::AsciiTableHDU) {
	raw_table_p = new AsciiTableExtension(*io_p);
    } else {
	return False;
    }
    AlwaysAssert(raw_table_p, AipsError);
    keywords_p = FITSTabular::keywordsFromHDU(*raw_table_p);
    RecordDesc tmpDesc(FITSTabular::descriptionFromHDU(*raw_table_p));
    units_p = FITSTabular::unitsFromHDU(*raw_table_p);
    disps_p = FITSTabular::displayFormatsFromHDU(*raw_table_p);
    nulls_p = FITSTabular::nullsFromHDU(*raw_table_p);

    // for PositionTables, the non-scalar columns should be
    // treated as scalars with the time increment as determined
    // below
    maxCount_p = 0;
    // loop over the description, looking for array columns
    for (uInt i=0;i<tmpDesc.nfields();i++) {
	if (tmpDesc.isArray(i)) {
	    uInt length = tmpDesc.shape(i).product();
	    // if maxCount_p is not set, use this one to set it
	    if (!maxCount_p) maxCount_p = length;
	    if (maxCount_p != length) {
		throw(AipsError("OldGBTPositionTable::reopen() : "
				"inconsistent number of elements in " +
				tmpDesc.name(i)));
	    }
	    description_p.addField(tmpDesc.name(i),
				   arrayType2scalarType(tmpDesc.type(i)));
	} else {
	    description_p.addField(tmpDesc.name(i), tmpDesc.type(i));
	}
    }
    // there are maxCount_p samples per second
    delta_time_p = 1.0 / maxCount_p;
    // and now convert it to days
    delta_time_p /= (24.0 * 60.0 * 60.0);
    count_p = maxCount_p;

    row_p.restructure(description_p);

    Int dmjdFieldNR = description_p.fieldNumber("DMJD");
    if (dmjdFieldNR < 0) {
	throw(AipsError("OldGBTPositionTable::reopen() : "
			"no DMJD column found!"));
    }
    dmjdField_p.attachToRecord(row_p, dmjdFieldNR);

    // Setup the record fields (one time only)
    uInt n = description_p.nfields();
    row_fields_p.resize(n);
    for (uInt i=0; i < n; i++) {
	switch( description_p.type(i)) {
	case TpBool: 
	    row_fields_p[i] = new RecordFieldPtr<Bool>(row_p, i);
	    break;
	case TpArrayBool:
	    row_fields_p[i] = new RecordFieldPtr<Array<Bool> >(row_p, i);
	    break;
	case TpUChar:
	    row_fields_p[i] = new RecordFieldPtr<uChar>(row_p, i);
	    break;
	case TpArrayUChar:
	    row_fields_p[i] = new RecordFieldPtr<Array<uChar> >(row_p, i);
	    break;
	case TpShort:
	    row_fields_p[i] = new RecordFieldPtr<Short>(row_p, i);
	    break;
	case TpArrayShort:
	    row_fields_p[i] = new RecordFieldPtr<Array<Short> >(row_p, i);
	    break;
	case TpInt:
	    row_fields_p[i] = new RecordFieldPtr<Int>(row_p, i);
 	    break;
	case TpArrayInt:
	    row_fields_p[i] = new RecordFieldPtr<Array<Int> >(row_p, i);
	    break;
	case TpFloat:
	    row_fields_p[i] = new RecordFieldPtr<Float>(row_p, i);
	    break;
	case TpArrayFloat:
	    row_fields_p[i] = new RecordFieldPtr<Array<Float> >(row_p, i);
	    break;
	case TpDouble:
	    row_fields_p[i] = new RecordFieldPtr<Double>(row_p, i);
	    break;
	case TpArrayDouble:
	    row_fields_p[i] = new RecordFieldPtr<Array<Double> >(row_p, i);
	    break;
	case TpComplex:
	    row_fields_p[i] = new RecordFieldPtr<Complex>(row_p, i);
            break;
	case TpArrayComplex:
	    row_fields_p[i] = new RecordFieldPtr<Array<Complex> >(row_p, i);
	    break;
	case TpDComplex:
	    row_fields_p[i] = new RecordFieldPtr<DComplex>(row_p, i);
	    break;
	case TpArrayDComplex:
	    row_fields_p[i] = new RecordFieldPtr<Array<DComplex> >(row_p, i);
	    break;
	case TpString:
	    row_fields_p[i] = new RecordFieldPtr<String>(row_p, i);
	    break;
	default:
	    throw(AipsError("OldGBTPositionTable::reopen() - unknown field type"));
	}
	AlwaysAssert(row_fields_p[i] != 0, AipsError);
    }

    if (description_p.nfields() > 0) {
	fill_row();
    }

    return True;
}

Bool OldGBTPositionTable::isValid() const
{
    return isValid_p;
}


const TableRecord &OldGBTPositionTable::keywords() const
{
    return keywords_p;
}

const RecordDesc &OldGBTPositionTable::description() const
{
    return description_p;
}

const Record &OldGBTPositionTable::units() const
{
    return units_p;
}

const Record &OldGBTPositionTable::displayFormats() const
{
    return disps_p;
}

const Record &OldGBTPositionTable::nulls() const
{
    return nulls_p;
}

void OldGBTPositionTable::next()
{
    if (isValid()) fill_row();
}

// What an ugly function! Simplify somehow!
void OldGBTPositionTable::fill_row()
{
    count_p++;
    if (count_p >= maxCount_p) {
	row_nr_p++;
	if (row_nr_p >= raw_table_p->nrows()) {
	    return; // Don't read past the end
	}
	count_p = 0;

	// Use the native FITS classes
	raw_table_p->read(1);
    }

    // And now fill it into the Row object.

    // it would be more efficient to separate this into
    // scalar and array parts and only do the scalar ones
    // when the FITS row changes

    uInt n = description_p.nfields();
    for (uInt i=0; i < n; i++) {

	Int offset = 0;
	// true scalar versus true array in FITS
	if (raw_table_p->field(i).nelements() > 1) offset = count_p;

	switch (raw_table_p->field(i).fieldtype()) {
	case FITS::LOGICAL:
	{
	    DebugAssert(description_p.type(i) == TpBool, AipsError);
	    FitsField<FitsLogical> &fitsRef = 
		(FitsField<FitsLogical> &)(raw_table_p->field(i));
	    RecordFieldPtr<Bool> &rowRef =
		*((RecordFieldPtr<Bool> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
	case FITS::BIT:
	{
	    DebugAssert(description_p.type(i) == TpBool, AipsError);
	    FitsField<FitsBit> &fitsRef = 
		(FitsField<FitsBit> &)(raw_table_p->field(i));
	    RecordFieldPtr<Bool> &rowRef =
		*((RecordFieldPtr<Bool> *)row_fields_p[i]);
	    *rowRef = (int(fitsRef(offset)));
	}
	break;
	case FITS::CHAR:
	{
	    DebugAssert(description_p.type(i) == TpUChar, AipsError);
	    FitsField<char> &fitsRef = 
		(FitsField<char> &)(raw_table_p->field(i));
	    RecordFieldPtr<uChar> &rowRef =
		*((RecordFieldPtr<uChar> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
	case FITS::BYTE:
	{
	    DebugAssert(description_p.type(i) == TpUChar, AipsError);
	    FitsField<unsigned char> &fitsRef = 
		(FitsField<unsigned char> &)(raw_table_p->field(i));
	    RecordFieldPtr<uChar> &rowRef =
		*((RecordFieldPtr<uChar> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
	case FITS::SHORT:
	{
	    DebugAssert(description_p.type(i) == TpShort, AipsError);
	    FitsField<short> &fitsRef = 
		(FitsField<short> &)(raw_table_p->field(i));
	    RecordFieldPtr<Short> &rowRef =
		*((RecordFieldPtr<Short> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
	case FITS::LONG:
	{
	    DebugAssert(description_p.type(i) == TpInt, AipsError);
	    FitsField<FitsLong> &fitsRef = 
		(FitsField<FitsLong> &)(raw_table_p->field(i));
	    RecordFieldPtr<Int> &rowRef =
		*((RecordFieldPtr<Int> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
	case FITS::FLOAT:
	{
	    DebugAssert(description_p.type(i) == TpFloat, AipsError);
	    FitsField<float> &fitsRef = 
		(FitsField<float> &)(raw_table_p->field(i));
	    RecordFieldPtr<Float> &rowRef =
		*((RecordFieldPtr<Float> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
	case FITS::DOUBLE:
	{
	    DebugAssert(description_p.type(i) == TpDouble, AipsError);
	    FitsField<double> &fitsRef = 
		(FitsField<double> &)(raw_table_p->field(i));
	    RecordFieldPtr<Double> &rowRef =
		*((RecordFieldPtr<Double> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
	case FITS::COMPLEX:
	{
	    DebugAssert(description_p.type(i) == TpComplex, AipsError);
	    FitsField<Complex> &fitsRef = 
		(FitsField<Complex> &)(raw_table_p->field(i));
	    RecordFieldPtr<Complex> &rowRef =
		*((RecordFieldPtr<Complex> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
	case FITS::DCOMPLEX:
	{
	    DebugAssert(description_p.type(i) == TpDComplex, AipsError);
	    FitsField<DComplex> &fitsRef = 
		(FitsField<DComplex> &)(raw_table_p->field(i));
	    RecordFieldPtr<DComplex> &rowRef =
		*((RecordFieldPtr<DComplex> *)row_fields_p[i]);
	    *rowRef = fitsRef(offset);
	}
	break;
        default:
	    throw(AipsError("OldGBTPositionTable::fill_row() - "
			    "unknown data type"));
	}
    }
    // finally, twiddle the timeField appropriately
    *dmjdField_p += delta_time_p * count_p;
}

void OldGBTPositionTable::clear_self()
{
    row_nr_p = -1;

    delete raw_table_p;
    raw_table_p = 0;

    delete io_p;
    io_p = 0;

    maxCount_p = 0;
    count_p = 0;
    delta_time_p = 0;

    uInt n = description_p.nfields();
    for (uInt i=0; i < n; i++) {
	switch( description_p.type(i)) {
	case TpBool: 
	    delete (RecordFieldPtr<Bool> *)row_fields_p[i];
	    break;
	case TpArrayBool:
	    delete (RecordFieldPtr<Array<Bool> > *)row_fields_p[i]; 
	    break;
	case TpUChar:
	    delete (RecordFieldPtr<uChar> *)row_fields_p[i];
	    break;
	case TpArrayUChar:
	    delete (RecordFieldPtr<Array<uChar> > *)row_fields_p[i];
	    break;
	case TpShort:
	    delete (RecordFieldPtr<Short> *)row_fields_p[i];
	    break;
	case TpArrayShort:
	    delete (RecordFieldPtr<Array<Short> > *)row_fields_p[i];
	    break;
	case TpInt:
	    delete (RecordFieldPtr<Int> *)row_fields_p[i];
 	    break;
	case TpArrayInt:
	    delete (RecordFieldPtr<Array<Int> > *)row_fields_p[i];
	    break;
	case TpFloat:
	    delete (RecordFieldPtr<Float> *)row_fields_p[i];
	    break;
	case TpArrayFloat:
	    delete (RecordFieldPtr<Array<Float> > *)row_fields_p[i];
	    break;
	case TpDouble:
	    delete (RecordFieldPtr<Double> *)row_fields_p[i];
	    break;
	case TpArrayDouble:
	    delete (RecordFieldPtr<Array<Double> > *)row_fields_p[i];
	    break;
	case TpComplex:
	    delete (RecordFieldPtr<Complex> *)row_fields_p[i];
            break;
	case TpArrayComplex:
	    delete (RecordFieldPtr<Array<Complex> > *)row_fields_p[i];
	    break;
	case TpDComplex:
	    delete (RecordFieldPtr<DComplex> *)row_fields_p[i];
	    break;
	case TpArrayDComplex:
	    delete (RecordFieldPtr<Array<DComplex> > *)row_fields_p[i];
	    break;
	case TpString:
	    delete (RecordFieldPtr<String> *)row_fields_p[i];
	    break;
	default:
	    throw(AipsError("OldGBTPositionTable::clear_self() - unknown field type"));
	}
	row_fields_p[i] = 0;
    }
    row_fields_p.resize(0);
    RecordDesc tmp;
    description_p = tmp;
    row_p.restructure(tmp);
}

const Record &OldGBTPositionTable::currentRow() const
{
    return row_p;
}

Bool OldGBTPositionTable::pastEnd() const
{
    return (row_nr_p >= raw_table_p->nrows() || ! isValid() );
}
