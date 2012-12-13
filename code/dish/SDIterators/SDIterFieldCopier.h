//# SDIterFieldCopier.h : this defines SDIterFieldCopier, which does stuff
//# Copyright (C) 1996,1999,2000,2001
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

#ifndef DISH_SDITERFIELDCOPIER_H
#define DISH_SDITERFIELDCOPIER_H


#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordField.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// </summary>

// <use visibility=local> 

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//

class SDIterFieldCopier
{
public:
    // set isOtherRecord when copying the "other" record
    SDIterFieldCopier(Bool isOtherRecord = False);

    ~SDIterFieldCopier();

    // Add the indicated fieldName to the copier.  It must exist in both in and out.
    // parentName must be supplied if this is copying the "other" record.  In
    // that case, no caching can be done be the contents of that record aren't fixed.
    void addField(const Record& in, Record& out, const String& fieldName,
		  const String &parentName = "");

    // Copy all of the fields from the input record to the output record.
    // The actual current other record must be supplied if this is copying the other record.
    void copy(uInt rec, const Record &otherRecord = Record());

    // Actually store the copied arrays back to the output record.  This should be
    // invoked after all calls to copy have been done.
    void store();

private:
    // undefined and inaccessible
    SDIterFieldCopier(const SDIterFieldCopier &);
    SDIterFieldCopier &operator=(const SDIterFieldCopier &);

    Bool itsIsOtherRecord;

    // input fields
    Block<RORecordFieldPtr<Bool> > in_bool;
    Block<RORecordFieldPtr<uChar> > in_uchar;
    Block<RORecordFieldPtr<Short> > in_short;
    Block<RORecordFieldPtr<Int> > in_int;
    Block<RORecordFieldPtr<uInt> > in_uint;
    Block<RORecordFieldPtr<Float> > in_float;
    Block<RORecordFieldPtr<Double> > in_double;
    Block<RORecordFieldPtr<Complex> > in_complex;
    Block<RORecordFieldPtr<DComplex> > in_dcomplex;
    Block<RORecordFieldPtr<String> > in_string;
    Block<RORecordFieldPtr<Array<Bool> > > in_array_bool;
    Block<RORecordFieldPtr<Array<uChar> > > in_array_uchar;
    Block<RORecordFieldPtr<Array<Short> > > in_array_short;
    Block<RORecordFieldPtr<Array<Int> > > in_array_int;
    Block<RORecordFieldPtr<Array<uInt> > > in_array_uint;
    Block<RORecordFieldPtr<Array<Float> > > in_array_float;
    Block<RORecordFieldPtr<Array<Double> > > in_array_double;
    Block<RORecordFieldPtr<Array<Complex> > > in_array_complex;
    Block<RORecordFieldPtr<Array<DComplex> > > in_array_dcomplex;
    Block<RORecordFieldPtr<Array<String> > > in_array_string;
    Block<RORecordFieldPtr<Record> > in_quantity;
    Block<RORecordFieldPtr<Record> > in_mdirection;
    Block<RORecordFieldPtr<Record> > in_mepoch;
    Block<RORecordFieldPtr<Record> > in_mposition;

    SimpleOrderedMap<Int, String> in_bool_map, in_uchar_map, in_short_map, in_int_map, in_uint_map, 
	in_float_map, in_double_map, in_complex_map, in_dcomplex_map, in_string_map,
	in_array_bool_map, in_array_uchar_map, in_array_short_map, in_array_int_map, in_array_uint_map,
	in_array_float_map, in_array_double_map, in_array_complex_map, in_array_dcomplex_map, 
	in_array_string_map;

    SimpleOrderedMap<Int, String> in_bool_field, in_uchar_field, in_short_field, in_int_field, 
	in_uint_field, in_float_field, in_double_field, in_complex_field, in_dcomplex_field, 
	in_string_field,
	in_array_bool_field, in_array_uchar_field, in_array_short_field, in_array_int_field, 
	in_array_uint_field, in_array_float_field, in_array_double_field, in_array_complex_field, 
	in_array_dcomplex_field, in_array_string_field;

    // output fields, all are array fields, but some are from arrays and some from scalars
    // and some have to do with measures and quantities
    Block<RecordFieldPtr<Array<Bool> > > field_out_bool;
    Block<RecordFieldPtr<Array<uChar> > > field_out_uchar;
    Block<RecordFieldPtr<Array<Short> > > field_out_short;
    Block<RecordFieldPtr<Array<Int> > > field_out_int;
    Block<RecordFieldPtr<Array<uInt> > > field_out_uint;
    Block<RecordFieldPtr<Array<Float> > > field_out_float;
    Block<RecordFieldPtr<Array<Double> > > field_out_double;
    Block<RecordFieldPtr<Array<Complex> > > field_out_complex;
    Block<RecordFieldPtr<Array<DComplex> > > field_out_dcomplex;
    Block<RecordFieldPtr<Array<String> > > field_out_string;
    Block<RecordFieldPtr<Array<Bool> > > field_out_array_bool;
    Block<RecordFieldPtr<Array<uChar> > > field_out_array_uchar;
    Block<RecordFieldPtr<Array<Short> > > field_out_array_short;
    Block<RecordFieldPtr<Array<Int> > > field_out_array_int;
    Block<RecordFieldPtr<Array<uInt> > > field_out_array_uint;
    Block<RecordFieldPtr<Array<Float> > > field_out_array_float;
    Block<RecordFieldPtr<Array<Double> > > field_out_array_double;
    Block<RecordFieldPtr<Array<Complex> > > field_out_array_complex;
    Block<RecordFieldPtr<Array<DComplex> > > field_out_array_dcomplex;
    Block<RecordFieldPtr<Array<String> > > field_out_array_string;
    Block<RecordFieldPtr<Array<Double> > > field_out_quantity;
    Block<RecordFieldPtr<Array<Double> > > field_out_mdirection;
    Block<RecordFieldPtr<Array<Double> > > field_out_mepoch;
    Block<RecordFieldPtr<Array<Double> > > field_out_mposition;

    //  output arrays that get put in place by the store function
    PtrBlock<Array<Bool> *> out_bool;
    PtrBlock<Array<uChar> *> out_uchar;
    PtrBlock<Array<Short> *> out_short;
    PtrBlock<Array<Int> *> out_int;
    PtrBlock<Array<uInt> *> out_uint;
    PtrBlock<Array<Float> *> out_float;
    PtrBlock<Array<Double> *> out_double;
    PtrBlock<Array<Complex> *> out_complex;
    PtrBlock<Array<DComplex> *> out_dcomplex;
    PtrBlock<Array<String> *> out_string;
    PtrBlock<Array<Bool> *> out_array_bool;
    PtrBlock<Array<uChar> *> out_array_uchar;
    PtrBlock<Array<Short> *> out_array_short;
    PtrBlock<Array<Int> *> out_array_int;
    PtrBlock<Array<uInt> *> out_array_uint;
    PtrBlock<Array<Float> *> out_array_float;
    PtrBlock<Array<Double> *> out_array_double;
    PtrBlock<Array<Complex> *> out_array_complex;
    PtrBlock<Array<DComplex> *> out_array_dcomplex;
    PtrBlock<Array<String> *> out_array_string;
    PtrBlock<Array<Double> *> out_quantity;
    PtrBlock<Array<Double> *> out_mdirection;
    PtrBlock<Array<Double> *> out_mepoch;
    PtrBlock<Array<Double> *> out_mposition;
};


} //# NAMESPACE CASA - END

#endif
