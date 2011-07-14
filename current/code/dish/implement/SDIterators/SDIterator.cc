//# SDIterator.cc:  this defined SDIterator
//# Copyright (C) 2000,2001,2002,2003,2004
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

#include <dish/SDIterators/SDIterator.h>
#include <dish/SDIterators/SDRecord.h>
#include <dish/SDIterators/SDIterFieldCopier.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Constants.h>
#include <measures/Measures/MeasureHolder.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Utilities/Assert.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// used recursively by get_vectors
void SDIterator::makeDescFromTemplate(const Record& recTemplate, const Record& sdrec,
				      RecordDesc& target, uInt nrows)
{
    // loop through the fields
    RecordDesc sdrecDesc(sdrec.description());
    RecordDesc tempDesc(recTemplate.description());
    for (uInt i=0;i<recTemplate.nfields();i++) {
	// if this field does not appear in sdrec, ignore it
	Int sdField = sdrec.fieldNumber(tempDesc.name(i));
	if (sdField < 0) continue;
	if (sdrecDesc.isTable(sdField)) {
	    // if a table, ignore it.  This should never happen
	    continue;
	} else if (sdrecDesc.isSubRecord(sdField)) {
	    // if the template is NOT a subRecord - it might be a Quanta or a Measure
	    if (!tempDesc.isSubRecord(i)) {
		// there's no other way than to try and extract it, first as a Measure, then as a Quanta
		String errMsg;
		MeasureHolder mHolder;
		if (mHolder.fromRecord(errMsg, sdrec.subRecord(sdField))) {
		    // its a Measure
		    // we currently use 3 types - MEpoch, MDirection, and MPosition
		    if (mHolder.isMDirection()) {
			// vector of two values, make output array of shape (2,nrows)
			target.addField(sdrecDesc.name(sdField), TpArrayDouble, IPosition(2,2,nrows));
			// store the type in the comment for this field
			target.setComment(target.fieldNumber(sdrecDesc.name(sdField)), "MDirection");
		    } else if (mHolder.isMEpoch()) {
			// single value, make output vector of length nrows
			target.addField(sdrecDesc.name(sdField), TpArrayDouble, IPosition(1,nrows));
			// store the type in the comment for this field
			target.setComment(target.fieldNumber(sdrecDesc.name(sdField)), "MEpoch");
		    } else if (mHolder.isMPosition()) {
			// vector of three values, make output array of shape (3,nrows)
			target.addField(sdrecDesc.name(sdField), TpArrayDouble, IPosition(2,3,nrows));
			// store the type in the comment for this field
			target.setComment(target.fieldNumber(sdrecDesc.name(sdField)), "MPosition");
		    }
		    // otherwise, just ignore it
		} else {
		    QuantumHolder qHolder;
		    if (qHolder.fromRecord(errMsg, sdrec.subRecord(sdField))) {
			if (qHolder.isQuantity()) {
			    // its a Quantity - single values, all double, make this a vector of length nrows
			    target.addField(sdrecDesc.name(sdField), TpArrayDouble, IPosition(1,nrows));
			    // indicate its a quantity
			    target.setComment(target.fieldNumber(sdrecDesc.name(sdField)), "Quantity");
			} 
			// otherwise, just ignore it
		    } 
		    // otherwise, just ignore it
		}
	    } else {
		// if a sub-record, copy as is and descend
		target.addField(sdrecDesc.name(sdField), sdrecDesc.type(sdField));
		Int subnr = target.fieldNumber(sdrecDesc.name(sdField));
		makeDescFromTemplate(recTemplate.subRecord(i), 
				     sdrec.subRecord(sdField),
				     target.rwSubRecord(subnr), nrows);
	    }
	} else if (sdrecDesc.isScalar(sdField)) {
	    // make this a vector with length (nrows)
	    target.addField(sdrecDesc.name(sdField), sdrecDesc.type(sdField),
			    IPosition(1,nrows));
	} else {
	    // it must be an array, add 1 to the shape and make that length nrows
	    IPosition shape(sdrec.shape(sdField));
	    // watch for variable length arrays - hope that they are really fixed
	    // at any rate, we'll deal with that later
	    if (shape.nelements() == 1 && shape(0) < 0) {
		// use shape of this field in the sdrec, which we decend as the other recs
		shape.resize(0);
		shape = sdrec.shape(sdField);
	    }
	    // watch for empty arrays - ignore them
	    if (shape.product() > 0) {
		shape.resize(shape.nelements()+1, True);
		shape(shape.nelements()-1) = nrows;
		target.addField(sdrecDesc.name(sdField), sdrecDesc.type(sdField), shape);
	    }
	}
    }
}

RecordDesc SDIterator::cloneDesc(const Record& rec)
{
  RecordDesc desc;
  RecordDesc recDesc(rec.description());
  for (uInt i=0;i<recDesc.nfields();i++) {
    if (recDesc.isSubRecord(i)) {
      desc.addField(rec.name(i),cloneDesc(rec.subRecord(i)));
    } else if (recDesc.isScalar(i)) {
      desc.addField(rec.name(i),rec.type(i));
    } else if (recDesc.isArray(i)) {
	// get the shape from the description
      desc.addField(rec.name(i),rec.type(i),rec.description().shape(i));
    }
  }
  return desc;
}

void SDIterator::addFieldsToCopier(SDIterFieldCopier& copier, const Record& sdrec,
				   Record& result, const String &parentName)
{
    
    String fieldName;
    for (uInt i=0;i<result.nfields();i++) {
	fieldName = result.name(i);
	if (result.type(i) == TpRecord) {
	    addFieldsToCopier(copier, sdrec.subRecord(fieldName), 
			      result.rwSubRecord(i), fieldName);
	} else {
	    copier.addField(sdrec, result, fieldName, parentName);
	}
    }
}

Record SDIterator::get_vectors(const Record &recTemplate)
{
    // split up recTemplate into things which are handled by getVectorShortCuts
    // and everything else
    Record bruteForce, shortCuts, result;
    bruteForce = recTemplate;
    // loosen up bruteForce
    makeVariableSubRecs(bruteForce);
    if (recTemplate.fieldNumber("header") >= 0 && 
	recTemplate.dataType("header") == TpRecord) {
	for (uInt i=0; i<recTemplate.subRecord("header").nfields();i++) {
	    String name = recTemplate.subRecord("header").name(i);
	    if (name == "time" || 
		name == "scan_number" ||
		name == "source_name" ||
		name == "telescope_position" ||
		name == "date" ||
		name == "ut") {
		bruteForce.rwSubRecord("header").
		    removeField(recTemplate.subRecord("header").name(i));
		if (shortCuts.fieldNumber("header") < 0) {
		    shortCuts.defineRecord("header", Record());
		}
		// the type doesn't matter here
		shortCuts.rwSubRecord("header").
		    define(recTemplate.subRecord("header").name(i), True);
	    }
	}
	if (bruteForce.rwSubRecord("header").nfields() == 0) {
	    bruteForce.removeField("header");
	}
    }
    if (recTemplate.fieldNumber("data") >= 0 && recTemplate.dataType("data") == TpRecord &&
	recTemplate.subRecord("data").fieldNumber("desc") >= 0 && 
	recTemplate.subRecord("data").dataType("desc") == TpRecord &&
	recTemplate.subRecord("data").subRecord("desc").fieldNumber("restfrequency") >= 0) {
	bruteForce.rwSubRecord("data").rwSubRecord("desc").removeField("restfrequency");
	shortCuts.defineRecord("data",Record());
	shortCuts.rwSubRecord("data").defineRecord("desc",Record());
	shortCuts.rwSubRecord("data").rwSubRecord("desc").define("restfrequency",True);
	if (bruteForce.rwSubRecord("data").rwSubRecord("desc").nfields() == 0) {
	    bruteForce.rwSubRecord("data").removeField("desc");
	}
	if (bruteForce.rwSubRecord("data").nfields() == 0) {
	    bruteForce.removeField("data");
	}
    }
    if (bruteForce.nfields() > 0) {
	result = getVectors(bruteForce);
    }
    if (shortCuts.nfields() > 0) {
	// carefully merge
	Record shortCutResult(getVectorShortCuts(shortCuts));
	if (result.fieldNumber("header")>=0) {
	    if (shortCutResult.fieldNumber("header") >= 0) {
		result.rwSubRecord("header").
		    merge(shortCutResult.subRecord("header"));
	    } // else nothing to merge in header
	} else if (shortCutResult.fieldNumber("header") >= 0) {
	    result.defineRecord("header", shortCutResult.subRecord("header"));
	}
	// the existance of a data field always implies a desc subrecord
	if (result.fieldNumber("data") >= 0) {
	    if (shortCutResult.fieldNumber("data") >= 0) {
		result.rwSubRecord("data").rwSubRecord("desc").
		    merge(shortCutResult.subRecord("data").subRecord("desc"));
	    } // else nothing to merge in data
	} else if (shortCutResult.fieldNumber("data") >= 0) {
	    result.defineRecord("data",Record());
	    result.rwSubRecord("data").
		defineRecord("desc",
		      shortCutResult.subRecord("data").subRecord("desc"));
	}
    }
    return result;
}

Record SDIterator::getVectorShortCuts(const Record &recTemplate)
{
    // default is brute force
    return getVectors(recTemplate);
}

Record SDIterator::getVectors(const Record& recTemplate)
{
    // reset iterator to top, remember where we are
    uInt posn = where();
    origin();

    Record currHeader(getHeader().description());
    Record currData(getData().description());
    Record currOther(getOther());
	
    RecordDesc headerDesc, dataDesc, otherDesc;
    Bool hasHeader, hasData, hasOther;
    hasHeader = hasData = hasOther = False;

    if (recTemplate.fieldNumber("data")>=0) {
	makeDescFromTemplate(recTemplate.subRecord("data"), 
			     currData, 
			     dataDesc, nrecords());
	if (countFields(dataDesc) > 0) {
	    hasData = True;
	}
    }
    if (recTemplate.fieldNumber("header")>=0) {
	makeDescFromTemplate(recTemplate.subRecord("header"), 
			     currHeader, 
			     headerDesc, nrecords());
	if (countFields(headerDesc)) {
	    hasHeader = True;
	}
    }

    if (recTemplate.fieldNumber("other")>=0) {
	makeDescFromTemplate(recTemplate.subRecord("other"), 
			     currOther, 
			     otherDesc, nrecords());
	if (countFields(otherDesc)) {
	    hasOther = True;
	}
    }

    Record headerResult(headerDesc);
    Record dataResult(dataDesc);
    Record otherResult(otherDesc);
    Record result;

    if (hasHeader || hasData || hasOther) {
	SDIterFieldCopier headerFieldCopier, dataFieldCopier;
	SDIterFieldCopier otherFieldCopier(True);
	// add each field to the copier as appropriate
	if (hasHeader) 
	    addFieldsToCopier(headerFieldCopier, currHeader, headerResult, "header");
	if (hasData) 
	    addFieldsToCopier(dataFieldCopier, currData, dataResult, "data");
	if (hasOther)
	    addFieldsToCopier(otherFieldCopier, currOther, otherResult, "other");
	
	// iterate to the end, copying as appropriate
	// do the first one
	Int nrecs = nrecords();
	for (Int i=0;i<nrecs;i++) {
	    if (hasHeader) headerFieldCopier.copy(i);
	    if (hasData) dataFieldCopier.copy(i);
	    if (hasOther) otherFieldCopier.copy(i, currOther);
	    (*this)++;
	    if (hasHeader) currHeader = getHeader();
	    if (hasData) currData = getData();
	    if (hasOther) currOther = getOther();
	}
	
	// return iterator to previous location
	// we're at the end so posn must be <= current location
	if (posn == 0) {
	    origin();
	} else {
	    while(posn < where()) (*this)--;
	}
	
	// tell the copiers to put the arrays into result
	headerFieldCopier.store();
	dataFieldCopier.store();
	otherFieldCopier.store();
    }
    if (recTemplate.fieldNumber("data") >= 0) {
	result.defineRecord("data",dataResult);
    }
    if (recTemplate.fieldNumber("header") >= 0) {
	result.defineRecord("header",headerResult);
    }
    if (recTemplate.fieldNumber("other") >= 0) {
	result.defineRecord("other",otherResult);
    }
    
    return result;
}

uInt SDIterator::countFields(const RecordDesc &rec) {
    uInt result = 0;
    for (uInt i=0;i<rec.nfields();i++) {
	if (rec.type(i) == TpRecord) {
	  result += countFields(rec.subRecord(i));
	} else { 
	  result++;
	}
    }
    return result;
}

String SDIterator::parseRanges(const Matrix<Double> &ranges, const String &name,
			       Bool colIsArray)
{
    String result;
    String prefix;
    String colname = name;
    if (colIsArray) {
      prefix = "NELEMENTS(" + name + ") >= 1 && ";
      colname = name + "[1]";
    }
    for (uInt i=0;i<ranges.ncolumn();i++) {
	String tag = "";
	Double rmin, rmax;
	rmin = ranges(0,i);
	rmax = ranges(1,i);
	if (rmin == -C::dbl_max && rmax == C::dbl_max) {
	    // entire range is specified, no selection necessary
	    continue;
	} else if (rmin == rmax) {
	    // single value, use NEAR selection
	    tag = prefix + "NEAR(" + colname + "," + doubleToString(rmin) + ")";
	} else if (rmin == -C::dbl_max) {
	    // upper limit
	    tag = prefix + colname + " IN [ := " + 
		doubleToString(rmax) + " ] ";
	} else if (rmax == C::dbl_max) {
	    // lower limit
	    tag = prefix + colname + " IN [ " + doubleToString(rmin) +
		" =: ] ";
	} else {
	    // a true range, assume order is ok
	    tag = prefix + colname + " IN [ " + doubleToString(rmin) +
		" =:= " + doubleToString(rmax) + " ] ";
	}
	if (tag.length() == 0) continue;
	if (result.length() > 0) result = result + " || ";
	result = result + " ( " + tag + " ) ";
    }
    return result;
}

String SDIterator::parseList(const Vector<String> &list, const String &name)
{
    String result;
    for (uInt i=0;i<list.nelements();i++) {
	if (list(i).length() != 0) {
	    if (result.length() > 0) result = result + " || ";
	    result = result + name + " == pattern(" + '"' + list(i) + '"' + ")";
	}
    }
    return result;
}

String SDIterator::doubleToString(Double dbl)
{
    ostringstream obuff;
    obuff.precision(15);
    obuff.setf(ios::showpoint);
    obuff << dbl;
    String result(obuff);
    return result;
}

void SDIterator::makeVariableSubRecs(Record &rec)
{
    // iterate from last to first so that the field number doesn't change for those we
    // have yet to examine
    // First pass, copy out the fixed records, remove them, replace them
    for (Int i=(rec.nfields()-1);i>=0;i--) {
	if (rec.dataType(i) == TpRecord && rec.rwSubRecord(i).isFixed()) {
	    Record thisRecord = rec.subRecord(i);
	    String thisName = rec.name(i);
	    rec.removeField(i);
	    rec.defineRecord(thisName, thisRecord);
	} 
    }
    // second pass, descend 
    for (uInt i=0;i<rec.nfields();i++) {
	if (rec.dataType(i) == TpRecord) makeVariableSubRecs(rec.rwSubRecord(i));
    }
}

} //# NAMESPACE CASA - END

