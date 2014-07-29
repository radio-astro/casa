//# SDIterFieldCopier.cc:  this defines SDIterFieldCopier, which does stuff
//# Copyright (C) 1996,1997,1999,2000,2001
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

#include <dish/SDIterators/SDIterFieldCopier.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slice.h>
#include <measures/Measures/MeasureHolder.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/Quantum.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SDIterFieldCopier::SDIterFieldCopier(Bool otherRecord)
    : itsIsOtherRecord(otherRecord),
      in_bool_map(""), in_uchar_map(""), in_short_map(""),
      in_int_map(""), in_uint_map(""), in_float_map(""),
      in_double_map(""), in_complex_map(""), in_dcomplex_map(""),
      in_string_map(""),
      in_array_bool_map(""), in_array_uchar_map(""), in_array_short_map(""),
      in_array_int_map(""), in_array_uint_map(""), in_array_float_map(""),
      in_array_double_map(""), in_array_complex_map(""), in_array_dcomplex_map(""),
      in_array_string_map(""),
      in_bool_field(""), in_uchar_field(""), in_short_field(""),
      in_int_field(""), in_uint_field(""), in_float_field(""),
      in_double_field(""), in_complex_field(""), in_dcomplex_field(""),
      in_string_field(""),
      in_array_bool_field(""), in_array_uchar_field(""), in_array_short_field(""),
      in_array_int_field(""), in_array_uint_field(""), in_array_float_field(""),
      in_array_double_field(""), in_array_complex_field(""), in_array_dcomplex_field(""),
      in_array_string_field("")
{ ; }

SDIterFieldCopier::~SDIterFieldCopier()
{
    // delete all of the ptrs
    uInt i;
    for (i=0; i<out_bool.nelements();i++) {
	delete out_bool[i];
	out_bool[i] = 0;
    }
    for (i=0; i<out_uchar.nelements();i++) {
	delete out_uchar[i];
	out_uchar[i] = 0;
    }
    for (i=0; i<out_short.nelements();i++) {
	delete out_short[i];
	out_short[i] = 0;
    }
    for (i=0; i<out_int.nelements();i++) {
	delete out_int[i];
	out_int[i] = 0;
    }
    for (i=0; i<out_uint.nelements();i++) {
	delete out_uint[i];
	out_uint[i] = 0;
    }
    for (i=0; i<out_float.nelements();i++) {
	delete out_float[i];
	out_float[i] = 0;
    }
    for (i=0; i<out_double.nelements();i++) {
	delete out_double[i];
	out_double[i] = 0;
    }
    for (i=0; i<out_complex.nelements();i++) {
	delete out_complex[i];
	out_complex[i] = 0;
    }
    for (i=0; i<out_dcomplex.nelements();i++) {
	delete out_dcomplex[i];
	out_dcomplex[i] = 0;
    }
    for (i=0; i<out_string.nelements();i++) {
	delete out_string[i];
	out_string[i] = 0;
    }
    for (i=0; i<out_array_bool.nelements();i++) {
	delete out_array_bool[i];
	out_array_bool[i] = 0;
    }
    for (i=0; i<out_array_uchar.nelements();i++) {
	delete out_array_uchar[i];
	out_array_uchar[i] = 0;
    }
    for (i=0; i<out_array_short.nelements();i++) {
	delete out_array_short[i];
	out_array_short[i] = 0;
    }
    for (i=0; i<out_array_int.nelements();i++) {
	delete out_array_int[i];
	out_array_int[i] = 0;
    }
    for (i=0; i<out_array_uint.nelements();i++) {
	delete out_array_uint[i];
	out_array_uint[i] = 0;
    }
    for (i=0; i<out_array_float.nelements();i++) {
	delete out_array_float[i];
	out_array_float[i] = 0;
    }
    for (i=0; i<out_array_double.nelements();i++) {
	delete out_array_double[i];
	out_array_double[i] = 0;
    }
    for (i=0; i<out_array_complex.nelements();i++) {
	delete out_array_complex[i];
	out_array_complex[i] = 0;
    }
    for (i=0; i<out_array_dcomplex.nelements();i++) {
	delete out_array_dcomplex[i];
	out_array_dcomplex[i] = 0;
    }
    for (i=0; i<out_array_string.nelements();i++) {
	delete out_array_string[i];
	out_array_string[i] = 0;
    }
    for (i=0; i<out_quantity.nelements();i++) {
	delete out_quantity[i];
	out_quantity[i] = 0;
    }
    for (i=0; i<out_mdirection.nelements();i++) {
	delete out_mdirection[i];
	out_mdirection[i] = 0;
    }
    for (i=0; i<out_mepoch.nelements();i++) {
	delete out_mepoch[i];
	out_mepoch[i] = 0;
    }
    for (i=0; i<out_mposition.nelements();i++) {
	delete out_mposition[i];
	out_mposition[i] = 0;
    }
}

void SDIterFieldCopier::addField(const Record& in, Record& out, const String& fieldName,
				 const String &parentRecordName)
{
    // just do nothing if there are any problems
    // parentRecordName must be supplied if this is an otherRecord
    if (itsIsOtherRecord && parentRecordName == "") return;

    // fieldName must be found in both records
    Int inField, outField;
    inField = in.fieldNumber(fieldName);
    outField = out.fieldNumber(fieldName);

    if (inField < 0 || outField < 0) return;

    uInt which;
    switch (in.type(inField)) {
    case TpRecord:
	{
	    // this isn't allowed for other records
	    if (itsIsOtherRecord) return;

	    // this must be some kind of Measure or Quanta
	    // get the comment
	    String comment = out.comment(outField);
	    if (comment == "Quantity") {
		which = in_quantity.nelements();
		in_quantity.resize(which+1);
		out_quantity.resize(which+1);
		field_out_quantity.resize(which+1);
		in_quantity[which].attachToRecord(in, inField);
		field_out_quantity[which].attachToRecord(out, outField);
		out_quantity[which] = new Array<Double>(field_out_quantity[which].get().shape());
		AlwaysAssert(out_quantity[which], AipsError);
	    } else if (comment == "MDirection") {
		which = in_mdirection.nelements();
		in_mdirection.resize(which+1);
		out_mdirection.resize(which+1);
		field_out_mdirection.resize(which+1);
		in_mdirection[which].attachToRecord(in, inField);
		field_out_mdirection[which].attachToRecord(out, outField);
		out_mdirection[which] = new Array<Double>(field_out_mdirection[which].get().shape());
		AlwaysAssert(out_mdirection[which], AipsError);
	    } else if (comment == "MEpoch") {
		which = in_mepoch.nelements();
		in_mepoch.resize(which+1);
		out_mepoch.resize(which+1);
		field_out_mepoch.resize(which+1);
		in_mepoch[which].attachToRecord(in, inField);
		field_out_mepoch[which].attachToRecord(out, outField);
		out_mepoch[which] = new Array<Double>(field_out_mepoch[which].get().shape());
		AlwaysAssert(out_mepoch[which], AipsError);
	    } else if (comment == "MPosition") {
		which = in_mposition.nelements();
		in_mposition.resize(which+1);
		out_mposition.resize(which+1);
		field_out_mposition.resize(which+1);
		in_mposition[which].attachToRecord(in, inField);
		field_out_mposition[which].attachToRecord(out, outField);
		out_mposition[which] = new Array<Double>(field_out_mposition[which].get().shape());
		AlwaysAssert(out_mposition[which], AipsError);
	    } else {
		return;
	    }
	}
	break;
    case TpBool:
	which = out_bool.nelements();
	if (itsIsOtherRecord) {
	    in_bool_map.define(which, parentRecordName);
	    in_bool_field.define(which, fieldName);
	} else {
	    in_bool.resize(which+1);
	    in_bool[which].attachToRecord(in, inField);
	}
	out_bool.resize(which+1);
	field_out_bool.resize(which+1);
	field_out_bool[which].attachToRecord(out, outField);
	out_bool[which] = new Array<Bool>(field_out_bool[which].get().shape());
	AlwaysAssert(out_bool[which], AipsError);
	break;
    case TpUChar:
	which = out_uchar.nelements();
	if (itsIsOtherRecord) {
	    in_uchar_map.define(which, parentRecordName);
	    in_uchar_field.define(which, fieldName);
	} else {
	    in_uchar.resize(which+1);
	    in_uchar[which].attachToRecord(in, inField);
	}
	out_uchar.resize(which+1);
	field_out_uchar.resize(which+1);
	field_out_uchar[which].attachToRecord(out, outField);
	out_uchar[which] = new Array<uChar>(field_out_uchar[which].get().shape());
	AlwaysAssert(out_uchar[which], AipsError);
	break;
    case TpShort:
	which = out_short.nelements();
	if (itsIsOtherRecord) {
	    in_short_map.define(which, parentRecordName);
	    in_short_field.define(which, fieldName);
	} else {
	    in_short.resize(which+1);
	    in_short[which].attachToRecord(in, inField);
	}
	out_short.resize(which+1);
	field_out_short.resize(which+1);
	field_out_short[which].attachToRecord(out, outField);
	out_short[which] = new Array<Short>(field_out_short[which].get().shape());
	AlwaysAssert(out_short[which], AipsError);
	break;
    case TpInt:
	which = out_int.nelements();
	if (itsIsOtherRecord) {
	    in_int_map.define(which, parentRecordName);
	    in_int_field.define(which, fieldName);
	} else {
	    in_int.resize(which+1);
	    in_int[which].attachToRecord(in, inField);
	}
	out_int.resize(which+1);
	field_out_int.resize(which+1);
	field_out_int[which].attachToRecord(out, outField);
	out_int[which] = new Array<Int>(field_out_int[which].get().shape());
	AlwaysAssert(out_int[which], AipsError);
	break;
    case TpUInt:
	which = out_uint.nelements();
	if (itsIsOtherRecord) {
	    in_uint_map.define(which, parentRecordName);
	    in_uint_field.define(which, fieldName);
	} else {
	    in_uint.resize(which+1);
	    in_uint[which].attachToRecord(in, inField);
	}
	out_uint.resize(which+1);
	field_out_uint.resize(which+1);
	field_out_uint[which].attachToRecord(out, outField);
	out_uint[which] = new Array<uInt>(field_out_uint[which].get().shape());
	AlwaysAssert(out_uint[which], AipsError);
	break;
    case TpFloat:
	which = out_float.nelements();
	if (itsIsOtherRecord) {
	    in_float_map.define(which, parentRecordName);
	    in_float_field.define(which, fieldName);
	} else {
	    in_float.resize(which+1);
	    in_float[which].attachToRecord(in, inField);
	}
	out_float.resize(which+1);
	field_out_float.resize(which+1);
	field_out_float[which].attachToRecord(out, outField);
	out_float[which] = new Array<Float>(field_out_float[which].get().shape());
	AlwaysAssert(out_float[which], AipsError);
	break;
    case TpDouble:
	which = out_double.nelements();
	if (itsIsOtherRecord) {
	    in_double_map.define(which, parentRecordName);
	    in_double_field.define(which, fieldName);
	} else {
	    in_double.resize(which+1);
	    in_double[which].attachToRecord(in, inField);
	}
	out_double.resize(which+1);
	field_out_double.resize(which+1);
	field_out_double[which].attachToRecord(out, outField);
	out_double[which] = new Array<Double>(field_out_double[which].get().shape());
	AlwaysAssert(out_double[which], AipsError);
	break;
    case TpComplex:
	which = out_complex.nelements();
	if (itsIsOtherRecord) {
	    in_complex_map.define(which, parentRecordName);
	    in_complex_field.define(which, fieldName);
	} else {
	    in_complex.resize(which+1);
	    in_complex[which].attachToRecord(in, inField);
	}
	out_complex.resize(which+1);
	field_out_complex.resize(which+1);
	field_out_complex[which].attachToRecord(out, outField);
	out_complex[which] = new Array<Complex>(field_out_complex[which].get().shape());
	AlwaysAssert(out_complex[which], AipsError);
	break;
    case TpDComplex:
	which = out_dcomplex.nelements();
	if (itsIsOtherRecord) {
	    in_dcomplex_map.define(which, parentRecordName);
	    in_dcomplex_field.define(which, fieldName);
	} else {
	    in_dcomplex.resize(which+1);
	    in_dcomplex[which].attachToRecord(in, inField);
	}
	out_dcomplex.resize(which+1);
	field_out_dcomplex.resize(which+1);
	field_out_dcomplex[which].attachToRecord(out, outField);
	out_dcomplex[which] = new Array<DComplex>(field_out_dcomplex[which].get().shape());
	AlwaysAssert(out_dcomplex[which], AipsError);
	break;
    case TpString:
	which = out_string.nelements();
	if (itsIsOtherRecord) {
	    in_string_map.define(which, parentRecordName);
	    in_string_field.define(which, fieldName);
	} else {
	    in_string.resize(which+1);
	    in_string[which].attachToRecord(in, inField);
	}
	out_string.resize(which+1);
	field_out_string.resize(which+1);
	field_out_string[which].attachToRecord(out, outField);
	out_string[which] = new Array<String>(field_out_string[which].get().shape());
	AlwaysAssert(out_string[which], AipsError);
	break;
    case TpArrayBool:
	which = out_array_bool.nelements();
	if (itsIsOtherRecord) {
	    in_array_bool_map.define(which, parentRecordName);
	    in_array_bool_field.define(which, fieldName);
	} else {
	    in_array_bool.resize(which+1);
	    in_array_bool[which].attachToRecord(in, inField);
	}
	out_array_bool.resize(which+1);
	field_out_array_bool.resize(which+1);
	field_out_array_bool[which].attachToRecord(out, outField);
	out_array_bool[which] = 
	    new Array<Bool>(field_out_array_bool[which].get().shape());
	AlwaysAssert(out_array_bool[which], AipsError);
	break;
    case TpArrayUChar:
	which = out_array_uchar.nelements();
	if (itsIsOtherRecord) {
	    in_array_uchar_map.define(which, parentRecordName);
	    in_array_uchar_field.define(which, fieldName);
	} else {
	    in_array_uchar.resize(which+1);
	    in_array_uchar[which].attachToRecord(in, inField);
	}
	out_array_uchar.resize(which+1);
	field_out_array_uchar.resize(which+1);
	field_out_array_uchar[which].attachToRecord(out, outField);
	out_array_uchar[which] = 
	    new Array<uChar>(field_out_array_uchar[which].get().shape());
	AlwaysAssert(out_array_uchar[which], AipsError);
	break;
    case TpArrayShort:
	which = out_array_short.nelements();
	if (itsIsOtherRecord) {
	    in_array_short_map.define(which, parentRecordName);
	    in_array_short_field.define(which, fieldName);
	} else {
	    in_array_short.resize(which+1);
	    in_array_short[which].attachToRecord(in, inField);
	}
	out_array_short.resize(which+1);
	field_out_array_short.resize(which+1);
	field_out_array_short[which].attachToRecord(out, outField);
	out_array_short[which] = 
	    new Array<Short>(field_out_array_short[which].get().shape());
	AlwaysAssert(out_array_short[which], AipsError);
	break;
    case TpArrayInt:
	which = out_array_int.nelements();
	if (itsIsOtherRecord) {
	    in_array_int_map.define(which, parentRecordName);
	    in_array_int_field.define(which, fieldName);
	} else {
	    in_array_int.resize(which+1);
	    in_array_int[which].attachToRecord(in, inField);
	}
	out_array_int.resize(which+1);
	field_out_array_int.resize(which+1);
	field_out_array_int[which].attachToRecord(out, outField);
	out_array_int[which] = new Array<Int>(field_out_array_int[which].get().shape());
	AlwaysAssert(out_array_int[which], AipsError);
	break;
    case TpArrayUInt:
	which = out_array_uint.nelements();
	if (itsIsOtherRecord) {
	    in_array_uint_map.define(which, parentRecordName);
	    in_array_uint_field.define(which, fieldName);
	} else {
	    in_array_uint.resize(which+1);
	    in_array_uint[which].attachToRecord(in, inField);
	}
	out_array_uint.resize(which+1);
	field_out_array_uint.resize(which+1);
	field_out_array_uint[which].attachToRecord(out, outField);
	out_array_uint[which] = new Array<uInt>(field_out_array_uint[which].get().shape());
	AlwaysAssert(out_array_uint[which], AipsError);
	break;
    case TpArrayFloat:
	which = out_array_float.nelements();
	if (itsIsOtherRecord) {
	    in_array_float_map.define(which, parentRecordName);
	    in_array_float_field.define(which, fieldName);
	} else {
	    in_array_float.resize(which+1);
	    in_array_float[which].attachToRecord(in, inField);
	}
	out_array_float.resize(which+1);
	field_out_array_float.resize(which+1);
	field_out_array_float[which].attachToRecord(out, outField);
	out_array_float[which] = 
	    new Array<Float>(field_out_array_float[which].get().shape());
	AlwaysAssert(out_array_float[which], AipsError);
	break;
    case TpArrayDouble:
	which = out_array_double.nelements();
	if (itsIsOtherRecord) {
	    in_array_double_map.define(which, parentRecordName);
	    in_array_double_field.define(which, fieldName);
	} else {
	    in_array_double.resize(which+1);
	    in_array_double[which].attachToRecord(in, inField);
	}
	out_array_double.resize(which+1);
	field_out_array_double.resize(which+1);
	field_out_array_double[which].attachToRecord(out, outField);
	out_array_double[which] = 
	    new Array<Double>(field_out_array_double[which].get().shape());
	AlwaysAssert(out_array_double[which], AipsError);
	break;
    case TpArrayComplex:
	which = out_array_complex.nelements();
	if (itsIsOtherRecord) {
	    in_array_complex_map.define(which, parentRecordName);
	    in_array_complex_field.define(which, fieldName);
	} else {
	    in_array_complex.resize(which+1);
	    in_array_complex[which].attachToRecord(in, inField);
	}
	out_array_complex.resize(which+1);
	field_out_array_complex.resize(which+1);
	field_out_array_complex[which].attachToRecord(out, outField);
	out_array_complex[which] = 
	    new Array<Complex>(field_out_array_complex[which].get().shape());
	AlwaysAssert(out_array_complex[which], AipsError);
	break;
    case TpArrayDComplex:
	which = out_array_dcomplex.nelements();
	if (itsIsOtherRecord) {
	    in_array_dcomplex_map.define(which, parentRecordName);
	    in_array_dcomplex_field.define(which, fieldName);
	} else {
	    in_array_dcomplex.resize(which+1);
	    in_array_dcomplex[which].attachToRecord(in, inField);
	}
	out_array_dcomplex.resize(which+1);
	field_out_array_dcomplex.resize(which+1);
	field_out_array_dcomplex[which].attachToRecord(out, outField);
	out_array_dcomplex[which] = 
	    new Array<DComplex>(field_out_array_dcomplex[which].get().shape());
	AlwaysAssert(out_array_dcomplex[which], AipsError);
	break;
    case TpArrayString:
	which = out_array_string.nelements();
	if (itsIsOtherRecord) {
	    in_array_string_map.define(which, parentRecordName);
	    in_array_string_field.define(which, fieldName);
	} else {
	    in_array_string.resize(which+1);
	    in_array_string[which].attachToRecord(in, inField);
	}
	out_array_string.resize(which+1);
	field_out_array_string.resize(which+1);
	field_out_array_string[which].attachToRecord(out, outField);
	out_array_string[which] = 
	    new Array<String>(field_out_array_string[which].get().shape());
	AlwaysAssert(out_array_string[which], AipsError);
	break;
    default:
	// do nothing
	break;
    }
}

void SDIterFieldCopier::copy(uInt rec, const Record &otherRecord)
{
    uInt i;
    // ensure otherRecord is populated as required
    if (itsIsOtherRecord && otherRecord.nfields() == 0) return;

    for (i=0;i<out_bool.nelements();i++) {
	Vector<Bool> tmp(*out_bool[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_bool_map(i)).asBool(in_bool_field(i));
	} else {
	    tmp(rec) = *(in_bool[i]);
	}
    }
    for (i=0;i<out_uchar.nelements();i++) {
	Vector<uChar> tmp(*out_uchar[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_uchar_map(i)).asuChar(in_uchar_field(i));
	} else {
	    tmp(rec) = *(in_uchar[i]);
	}
    }
    for (i=0;i<out_short.nelements();i++) {
	Vector<Short> tmp(*out_short[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_short_map(i)).asShort(in_short_field(i));
	} else {
	    tmp(rec) = *(in_short[i]);
	}
    }
    for (i=0;i<out_int.nelements();i++) {
	Vector<Int> tmp(*out_int[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_int_map(i)).asInt(in_int_field(i));
	} else {
	    tmp(rec) = *(in_int[i]);
	}
    }
    for (i=0;i<out_uint.nelements();i++) {
	Vector<uInt> tmp(*out_uint[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_uint_map(i)).asuInt(in_int_field(i));
	} else {
	    tmp(rec) = *(in_uint[i]);
	}
    }
    for (i=0;i<out_float.nelements();i++) {
	Vector<Float> tmp(*out_float[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_float_map(i)).asFloat(in_float_field(i));
	} else {
	    tmp(rec) = *(in_float[i]);
	}
    }
    for (i=0;i<out_double.nelements();i++) {
	Vector<Double> tmp(*out_double[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_double_map(i)).asDouble(in_double_field(i));
	} else {
	    tmp(rec) = *(in_double[i]);
	}
    }
    for (i=0;i<out_complex.nelements();i++) {
	Vector<Complex> tmp(*out_complex[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_complex_map(i)).asComplex(in_complex_field(i));
	} else {
	    tmp(rec) = *(in_complex[i]);
	}
    }
    for (i=0;i<out_dcomplex.nelements();i++) {
	Vector<DComplex> tmp(*out_dcomplex[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_dcomplex_map(i)).asDComplex(in_dcomplex_field(i));
	} else {
	    tmp(rec) = *(in_dcomplex[i]);
	}
    }
    for (i=0;i<out_string.nelements();i++) {
	Vector<String> tmp(*out_string[i]);
	if (itsIsOtherRecord) {
	    tmp(rec) = otherRecord.subRecord(in_string_map(i)).asString(in_string_field(i));
	} else {
	    tmp(rec) = *(in_string[i]);
	}
    }
    for (i=0;i<out_array_bool.nelements();i++) {
	if (!out_array_bool[i]) continue;
	IPosition start((*out_array_bool[i]).shape());
	IPosition end((*out_array_bool[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_bool_map(i)).
		asArrayBool(in_array_bool_field(i)).shape();
	} else {
	    inShape = (*(in_array_bool[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_bool[i];
	    out_array_bool[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_bool[i])(start,end) = 
		    otherRecord.subRecord(in_array_bool_map(i)).
		    asArrayBool(in_array_bool_field(i)).addDegenerate(1);
	    } else {
		(*out_array_bool[i])(start,end) = 
		    (*(in_array_bool[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_uchar.nelements();i++) {
	if (!out_array_uchar[i]) continue;
	IPosition start((*out_array_uchar[i]).shape());
	IPosition end((*out_array_uchar[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_uchar_map(i)).
		asArrayuChar(in_array_uchar_field(i)).shape();
	} else {
	    inShape = (*(in_array_uchar[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_uchar[i];
	    out_array_uchar[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_uchar[i])(start,end) = 
		    otherRecord.subRecord(in_array_uchar_map(i)).
		    asArrayuChar(in_array_uchar_field(i)).addDegenerate(1);
	    } else {
		(*out_array_uchar[i])(start,end) = 
		    (*(in_array_uchar[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_short.nelements();i++) {
	if (!out_array_short[i]) continue;
	IPosition start((*out_array_short[i]).shape());
	IPosition end((*out_array_short[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_short_map(i)).
		asArrayShort(in_array_short_field(i)).shape();
	} else {
	    inShape = (*(in_array_short[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_short[i];
	    out_array_short[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_short[i])(start,end) = 
		    otherRecord.subRecord(in_array_short_map(i)).
		    asArrayShort(in_array_short_field(i)).addDegenerate(1);
	    } else {
		(*out_array_short[i])(start,end) = 
		    (*(in_array_short[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_int.nelements();i++) {
	if (!out_array_int[i]) continue;
	IPosition start((*out_array_int[i]).shape());
	IPosition end((*out_array_int[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_int_map(i)).
		asArrayInt(in_array_int_field(i)).shape();
	} else {
	    inShape = (*(in_array_int[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_int[i];
	    out_array_int[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_int[i])(start,end) = 
		    otherRecord.subRecord(in_array_int_map(i)).
		    asArrayInt(in_array_int_field(i)).addDegenerate(1);
	    } else {
		(*out_array_int[i])(start,end) = 
		    (*(in_array_int[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_uint.nelements();i++) {
	if (!out_array_uint[i]) continue;
	IPosition start((*out_array_uint[i]).shape());
	IPosition end((*out_array_uint[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_uint_map(i)).
		asArrayuInt(in_array_uint_field(i)).shape();
	} else {
	    inShape = (*(in_array_uint[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_uint[i];
	    out_array_uint[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_uint[i])(start,end) = 
		    otherRecord.subRecord(in_array_uint_map(i)).
		    asArrayuInt(in_array_uint_field(i)).addDegenerate(1);
	    } else {
		(*out_array_uint[i])(start,end) = 
		    (*(in_array_uint[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_float.nelements();i++) {
	if (!out_array_float[i]) continue;
	IPosition start((*out_array_float[i]).shape());
	IPosition end((*out_array_float[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_float_map(i)).
		asArrayFloat(in_array_float_field(i)).shape();
	} else {
	    inShape = (*(in_array_float[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	IPosition diff = end - start + 1;
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_float[i];
	    out_array_float[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_float[i])(start,end) = 
		    otherRecord.subRecord(in_array_float_map(i)).
		    asArrayFloat(in_array_float_field(i)).addDegenerate(1);
	    } else {
		(*out_array_float[i])(start,end) = 
		    (*(in_array_float[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_double.nelements();i++) {
	if (!out_array_double[i]) continue;
	IPosition start((*out_array_double[i]).shape());
	IPosition end((*out_array_double[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_double_map(i)).
		asArrayDouble(in_array_double_field(i)).shape();
	} else {
	    inShape = (*(in_array_double[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_double[i];
	    out_array_double[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_double[i])(start,end) = 
		    otherRecord.subRecord(in_array_double_map(i)).
		    asArrayDouble(in_array_double_field(i)).addDegenerate(1);
	    } else {
		(*out_array_double[i])(start,end) = 
		    (*(in_array_double[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_complex.nelements();i++) {
	if (!out_array_complex[i]) continue;
	IPosition start((*out_array_complex[i]).shape());
	IPosition end((*out_array_complex[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_complex_map(i)).
		asArrayComplex(in_array_complex_field(i)).shape();
	} else {
	    inShape = (*(in_array_complex[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_complex[i];
	    out_array_complex[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_complex[i])(start,end) = 
		    otherRecord.subRecord(in_array_complex_map(i)).
		    asArrayComplex(in_array_complex_field(i)).addDegenerate(1);
	    } else {
		(*out_array_complex[i])(start,end) = 
		    (*(in_array_complex[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_dcomplex.nelements();i++) {
	if (!out_array_dcomplex[i]) continue;
	IPosition start((*out_array_dcomplex[i]).shape());
	IPosition end((*out_array_dcomplex[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_dcomplex_map(i)).
		asArrayDComplex(in_array_dcomplex_field(i)).shape();
	} else {
	    inShape = (*(in_array_dcomplex[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_dcomplex[i];
	    out_array_dcomplex[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_dcomplex[i])(start,end) = 
		    otherRecord.subRecord(in_array_dcomplex_map(i)).
		    asArrayDComplex(in_array_dcomplex_field(i)).addDegenerate(1);
	    } else {
		(*out_array_dcomplex[i])(start,end) = 
		    (*(in_array_dcomplex[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_array_string.nelements();i++) {
	if (!out_array_string[i]) continue;
	IPosition start((*out_array_string[i]).shape());
	IPosition end((*out_array_string[i]).shape());
	IPosition inShape;
	if (itsIsOtherRecord) {
	    inShape = otherRecord.subRecord(in_array_string_map(i)).
		asArrayString(in_array_string_field(i)).shape();
	} else {
	    inShape = (*(in_array_string[i])).shape();
	}
	IPosition check(start.asVector()(Slice(0,inShape.nelements())));
	// verify that the shapes are consistent
	if (!check.isEqual(inShape)) {
	    // nope, give up on this field
	    delete out_array_string[i];
	    out_array_string[i] = 0;
	} else {
	    start = 0;
	    start(start.nelements()-1) = rec;
	    end -= 1;
	    end(start.nelements()-1) = rec;
	    if (itsIsOtherRecord) {
		(*out_array_string[i])(start,end) = 
		    otherRecord.subRecord(in_array_string_map(i)).
		    asArrayString(in_array_string_field(i)).addDegenerate(1);
	    } else {
		(*out_array_string[i])(start,end) = 
		    (*(in_array_string[i])).addDegenerate(1);
	    }
	}
    }
    for (i=0;i<out_quantity.nelements();i++) {
	// get the Quantity from the record
	QuantumHolder qHolder;
	String errMsg;
	if (qHolder.fromRecord(errMsg, *(in_quantity[i]))) {
	    if (qHolder.isQuantity()) {
		// store the value in the out array at rec
		Vector<Double> tmp(*out_quantity[i]);
		// we return them all in whatever units they have for that row
		tmp(rec) = qHolder.asQuantity().getValue();
	    }
	}
	// we might throw things here on errors, but I think it is not worth the effort
    }
    for (i=0;i<out_mdirection.nelements();i++) {
	// get the Measure from the record
	MeasureHolder mHolder;
	String errMsg;
	if (mHolder.fromRecord(errMsg, *(in_mdirection[i]))) {
	    if (mHolder.isMDirection()) {
		// store the value in the out matrix at column=rec
		Matrix<Double> tmp(*out_mdirection[i]);
		// we return them as radians in whatever reference frame they are in
		tmp.column(rec) = mHolder.asMDirection().getAngle().getValue();
	    }
	}
	// we might throw things here on errors, but I think it is not worth the effort
    }
    for (i=0;i<out_mepoch.nelements();i++) {
	// get the Measure from the record
	MeasureHolder mHolder;
	String errMsg;
	if (mHolder.fromRecord(errMsg, *(in_mepoch[i]))) {
	    if (mHolder.isMEpoch()) {
		// store the value in the out vector at rec
		Vector<Double> tmp(*out_mepoch[i]);
		// we return times in seconds in whatever time system they are in
		tmp(rec) = mHolder.asMEpoch().get("s").getValue();
	    }
	}
	// we might throw things here on errors, but I think it is not worth the effort
    }
    for (i=0;i<out_mposition.nelements();i++) {
	// get the Measure from the record
	MeasureHolder mHolder;
	String errMsg;
	if (mHolder.fromRecord(errMsg, *(in_mposition[i]))) {
	    if (mHolder.isMPosition()) {
		// store the value in the out matrix at column=rec
		Matrix<Double> tmp(*out_mposition[i]);
		// we return them all as xyz in "m" in whatever system the are in
		MPosition pos(mHolder.asMPosition());
		tmp.column(rec) = mHolder.asMPosition().get("m").getValue();
	    }
	}
	// we might throw things here on errors, but I think it is not worth the effort
    }
}

void SDIterFieldCopier::store()
{
    uInt i;
    for (i=0;i<out_bool.nelements();i++) {
	field_out_bool[i].define(*(out_bool[i]));
    }
    for (i=0;i<out_uchar.nelements();i++) {
	field_out_uchar[i].define(*(out_uchar[i]));
    }
    for (i=0;i<out_short.nelements();i++) {
	field_out_short[i].define(*(out_short[i]));
    }
    for (i=0;i<out_int.nelements();i++) {
	field_out_int[i].define(*(out_int[i]));
    }
    for (i=0;i<out_uint.nelements();i++) {
	field_out_uint[i].define(*(out_uint[i]));
    }
    for (i=0;i<out_float.nelements();i++) {
	field_out_float[i].define(*(out_float[i]));
    }
    for (i=0;i<out_double.nelements();i++) {
	field_out_double[i].define(*(out_double[i]));
    }
    for (i=0;i<out_complex.nelements();i++) {
	field_out_complex[i].define(*(out_complex[i]));
    }
    for (i=0;i<out_dcomplex.nelements();i++) {
	field_out_dcomplex[i].define(*(out_dcomplex[i]));
    }
    for (i=0;i<out_string.nelements();i++) {
	field_out_string[i].define(*(out_string[i]));
    }
    for (i=0;i<out_array_bool.nelements();i++) {
	if (out_array_bool[i]) 
	    field_out_array_bool[i].define(*(out_array_bool[i]));
    }
    for (i=0;i<out_array_uchar.nelements();i++) {
	if (out_array_uchar[i]) 
	    field_out_array_uchar[i].define(*(out_array_uchar[i]));
    }
    for (i=0;i<out_array_short.nelements();i++) {
	if (out_array_short[i]) 
	    field_out_array_short[i].define(*(out_array_short[i]));
    }
    for (i=0;i<out_array_int.nelements();i++) {
	if (out_array_int[i]) 
	    field_out_array_int[i].define(*(out_array_int[i]));
    }
    for (i=0;i<out_array_uint.nelements();i++) {
	if (out_array_uint[i]) 
	    field_out_array_uint[i].define(*(out_array_uint[i]));
    }
    for (i=0;i<out_array_float.nelements();i++) {
	if (out_array_float[i]) 
	    field_out_array_float[i].define(*(out_array_float[i]));
    }
    for (i=0;i<out_array_double.nelements();i++) {
	if (out_array_double[i]) 
	    field_out_array_double[i].define(*(out_array_double[i]));
    }
    for (i=0;i<out_array_complex.nelements();i++) {
	if (out_array_complex[i]) 
	    field_out_array_complex[i].define(*(out_array_complex[i]));
    }
    for (i=0;i<out_array_dcomplex.nelements();i++) {
	if (out_array_dcomplex[i]) 
	    field_out_array_dcomplex[i].define(*(out_array_dcomplex[i]));
    }
    for (i=0;i<out_array_string.nelements();i++) {
	if (out_array_string[i]) 
	    field_out_array_string[i].define(*(out_array_string[i]));
    }
    for (i=0;i<out_quantity.nelements();i++) {
	if (out_quantity[i]) 
	    field_out_quantity[i].define(*(out_quantity[i]));
    }
    for (i=0;i<out_mdirection.nelements();i++) {
	if (out_mdirection[i]) 
	    field_out_mdirection[i].define(*(out_mdirection[i]));
    }
    for (i=0;i<out_mepoch.nelements();i++) {
	if (out_mepoch[i]) 
	    field_out_mepoch[i].define(*(out_mepoch[i]));
    }
    for (i=0;i<out_mposition.nelements();i++) {
	if (out_mposition[i]) 
	    field_out_mposition[i].define(*(out_mposition[i]));
    }
}

} //# NAMESPACE CASA - END

