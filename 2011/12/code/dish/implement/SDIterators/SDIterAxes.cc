//# SDIterAxes.cc:  this defines SDIterAxes
//# Copyright (C) 1998,1999,2000,2001,2002,2003
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


#include <dish/SDIterators/SDIterAxes.h>

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <fits/FITS/FITSDateUtil.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/UnitMap.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>
#include <casa/iostream.h>

#include <casa/stdlib.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SDIterAxes::SDIterAxes(const Table& fitsTable)
    : tab_p(0), isWritable_(False), outColsTypeSet_(False), dataColumn_p(0),
      freqAxisRegex_p("((FREQ)|([VF]ELO)).*"), 
      longAxisRegex_p("((RA)|([GE]LON)).*"),
      latAxisRegex_p("((DEC)|([GE]LAT)).*"),
      timeAxisRegex_p("TIME"), stokesAxisRegex_p("STOKES"),
      beamAxisRegex_p("BEAM"), rcvrAxisRegex_p("RECEIVER"),
      cachedRow_p(-1), freqAxis_p(-1), longAxis_p(-1), latAxis_p(-1),
      timeAxis_p(-1), stokesAxis_p(-1), beamAxis_p(-1), rcvrAxis_p(-1)
{
    // initialize the cached Regex values
    reset(fitsTable);
}

SDIterAxes::~SDIterAxes()
{
    cleanup();
}

const Vector<String> &SDIterAxes::ctype(Int rownr)
{
    stringVector(rownr, ctypes_p, ctypeCols_p, "");
    return ctypes_p;
}

const Vector<String> &SDIterAxes::cunit(Int rownr)
{
    stringVector(rownr, cunits_p, cunitCols_p, crvalColUnits_p);
    return cunits_p;
}

const Vector<Double> &SDIterAxes::crval(Int rownr)
{
    doubleVector(rownr, crvals_p, crvalCols_p, 0.0);
    return crvals_p;
}

const Vector<Double> &SDIterAxes::crpix(Int rownr)
{
    doubleVector(rownr, crpixs_p, crpixCols_p, 1.0);
    return crpixs_p;
}

const Vector<Double> &SDIterAxes::crota(Int rownr)
{
    doubleVector(rownr, crotas_p, crotaCols_p, 0.0);
    return crotas_p;
}

const Vector<Double> &SDIterAxes::cdelt(Int rownr)
{
    doubleVector(rownr, cdelts_p, cdeltCols_p, 0.0);
    return cdelts_p;
}

Bool SDIterAxes::setctype(Int rownr, const Vector<String> &ctypes,
			  Bool &added)
{
    return setStringCols(rownr, ctypes, "CTYPE", 
			 ctypeCols_p, rwctypeCols_p, ctypeColNames_p, added, "");
}

Bool SDIterAxes::setcunit(Int rownr, const Vector<String> &cunits,
			  Bool &added)
{
    return setStringCols(rownr, cunits, "CUNIT",
			 cunitCols_p, rwcunitCols_p, cunitColNames_p, added, "");
}

Bool SDIterAxes::setcrval(Int rownr, const Vector<Double> &crvals,
			  Bool &added)
{
    return setDoubleCols(rownr, crvals, "CRVAL", crvalTypes_p,
			 crvalCols_p, rwcrvalCols_p, crvalColNames_p, added, 0.0);
}

Bool SDIterAxes::setcrpix(Int rownr, const Vector<Double> &crpixs,
			  Bool &added)
{
    return setDoubleCols(rownr, crpixs, "CRPIX", crpixTypes_p,
			 crpixCols_p, rwcrpixCols_p, crpixColNames_p, added, 1.0);
}

Bool SDIterAxes::setcrota(Int rownr, const Vector<Double> &crotas,
			  Bool &added)
{
    return setDoubleCols(rownr, crotas, "CROTA", crotaTypes_p,
			 crotaCols_p, rwcrotaCols_p, crotaColNames_p, added, 0.0);
}

Bool SDIterAxes::setcdelt(Int rownr, const Vector<Double> &cdelts,
			  Bool &added)
{
    return setDoubleCols(rownr, cdelts, "CDELT", cdeltTypes_p,
			 cdeltCols_p, rwcdeltCols_p, cdeltColNames_p, added, 0.0);
}

MEpoch SDIterAxes::time(Int rownr)
{
    Int taxis = timeAxis(rownr);
    Double offset = 0.0;
    Unit defaultUnit = "s";
    if (taxis >= 0) {
	// this assumes there is only a single pixel on the time axis
	// and that we want the value at pixel 1
	offset = crval(rownr)(taxis) + cdelt(rownr)(taxis)*(crpix(rownr)(taxis)-1);
	defaultUnit = cunit(rownr)(taxis);
    } else if (!timeCol_p.isNull()) {
	// otherwise use a TIME column, if present
	offset = timeCol_p.asdouble(rownr);
	defaultUnit = timeUnit_p;
    }
    Quantity qtime(offset, defaultUnit);
    // convert the DATE-OBS value to an MVTime and add that to qtime, if available
    if (!dateObsCol_p.isNull()) {
	if (dateObs_p != dateObsCol_p.asString(rownr)) {
	    MVTime mvdateObs;
	    MEpoch::Types system;
	    // does not check for TIMESYS keyword/column, assumes UTC
	    FITSDateUtil::fromFITS(mvdateObs, system, dateObsCol_p.asString(rownr),"");
	    dateObsQuant_p = mvdateObs.get();
	}
	qtime += dateObsQuant_p;
    }
    return MEpoch(qtime, MEpoch::UTC);
}

Bool SDIterAxes::ok(Int rownr) const
{
    return (freqAxis(rownr) >= 0 &&
		  longAxis(rownr) >= 0 &&
		  latAxis(rownr) >= 0);
}

void SDIterAxes::reset(const Table &fitsTable)
{
    LogIO os(LogOrigin("SDIterAxes","reset(const Table &fitsTable)"));
    cleanup();
    UnitMap::addFITS();
    tab_p = new Table(fitsTable);
    AlwaysAssert(tab_p, AipsError);
    isWritable_ = tab_p->isWritable();
    Vector<String> colNames(tab_p->tableDesc().columnNames());
    // max axis is the maximum value of the n in CTYPEn
    Int maxAxis = 0;
    Int dataAxis = -1;
    Regex ctype("CTYPE[0-9]+");
    Regex crval("CRVAL[0-9]+");
    Regex crpix("CRPIX[0-9]+");
    Regex cdelt("CDELT[0-9]+");
    Regex crota("CROTA[0-9]+");
    Regex cunit("CUNIT[0-9]+");
    for (uInt i=0;i<colNames.nelements();i++) {
	if (colNames(i).matches(ctype)) {
	    maxAxis = max(maxAxis, atol(colNames(i).after("CTYPE").chars()));
	}
	if (colNames(i).matches("DATA")) {
	    dataAxis = i;
	}
    }
    if (dataAxis >= 0) {
	dataColumn_p = new ROTableColumn(*tab_p, colNames(dataAxis));
	AlwaysAssert(dataColumn_p, AipsError);
    } else {
	os << LogIO::SEVERE
	   << WHERE
	   << "A DATA column is required in an SDFITS table"
	   << "\n" << fitsTable.tableName() << " does not have that column"
	   << LogIO::POST;
    }

    if (maxAxis != 0) {
	if (isWritable_) {
	    rwctypeCols_p.resize(maxAxis, False, False);
	    rwcrvalCols_p.resize(maxAxis, False, False);
	    rwcrpixCols_p.resize(maxAxis, False, False);
	    rwcdeltCols_p.resize(maxAxis, False, False);
	    rwcrotaCols_p.resize(maxAxis, False, False);
	    rwcunitCols_p.resize(maxAxis, False, False);
	    rwctypeCols_p.set(static_cast<TableColumn *>(0));
	    rwcrvalCols_p.set(static_cast<TableColumn *>(0));
	    rwcrpixCols_p.set(static_cast<TableColumn *>(0));
	    rwcdeltCols_p.set(static_cast<TableColumn *>(0));
	    rwcrotaCols_p.set(static_cast<TableColumn *>(0));
	    rwcunitCols_p.set(static_cast<TableColumn *>(0));
	}
	ctypeCols_p.resize(maxAxis, False, False);
	crvalCols_p.resize(maxAxis, False, False);
	crpixCols_p.resize(maxAxis, False, False);
	cdeltCols_p.resize(maxAxis, False, False);
	crotaCols_p.resize(maxAxis, False, False);
	cunitCols_p.resize(maxAxis, False, False);
	ctypeCols_p.set(static_cast<ROTableColumn *>(0));
	crvalCols_p.set(static_cast<ROTableColumn *>(0));
	crpixCols_p.set(static_cast<ROTableColumn *>(0));
	cdeltCols_p.set(static_cast<ROTableColumn *>(0));
	crotaCols_p.set(static_cast<ROTableColumn *>(0));
	cunitCols_p.set(static_cast<ROTableColumn *>(0));
	crvalTypes_p.resize(maxAxis);
	crpixTypes_p.resize(maxAxis);
	cdeltTypes_p.resize(maxAxis);
	crotaTypes_p.resize(maxAxis);
	crvalColUnits_p.resize(maxAxis);
	crvalTypes_p = TpDouble;
	crpixTypes_p = TpDouble;
	crotaTypes_p = TpDouble;
	cdeltTypes_p = TpDouble;
	ctypeColNames_p.resize(maxAxis);
	crvalColNames_p.resize(maxAxis);
	crpixColNames_p.resize(maxAxis);
	cdeltColNames_p.resize(maxAxis);
	cunitColNames_p.resize(maxAxis);
	crotaColNames_p.resize(maxAxis);
	ctypeColNames_p = crvalColNames_p = crpixColNames_p = 
	  cdeltColNames_p = cunitColNames_p = crotaColNames_p = "";
    }
    for (uInt i=0;i<colNames.nelements();i++) {
	if (colNames(i).matches(ctype)) {
	    // FITS numbers are 1 relative
	    Int axisno = atol(colNames(i).after("CTYPE").chars()) - 1;
	    // no need to check axisno here as elsewhere, checked above
	    if (isWritable_) {
		rwctypeCols_p[axisno] = new TableColumn(*tab_p, colNames(i));
		AlwaysAssert(rwctypeCols_p[axisno], AipsError);
	    }
	    ctypeCols_p[axisno] = new ROTableColumn(*tab_p, colNames(i));
	    AlwaysAssert(ctypeCols_p[axisno], AipsError);
	    ctypeColNames_p(axisno) = colNames(i);
	} else if (colNames(i).matches(crval)) {
	    Int axisno = atol(colNames(i).after("CRVAL").chars()) - 1;
	    AlwaysAssert(axisno < maxAxis, AipsError);
	    if (isWritable_) {
		rwcrvalCols_p[axisno] = new TableColumn(*tab_p, colNames(i));
		AlwaysAssert(rwcrvalCols_p[axisno], AipsError);
	    }
	    crvalCols_p[axisno] = new ROTableColumn(*tab_p, colNames(i));
	    AlwaysAssert(crvalCols_p[axisno], AipsError);
	    crvalColNames_p(axisno) = colNames(i);
	    crvalTypes_p(axisno) = crvalCols_p[axisno]->columnDesc().dataType();
	    if (crvalCols_p[axisno]->keywordSet().fieldNumber("UNIT") >= 0) {
		crvalColUnits_p(axisno) = 
		    crvalCols_p[axisno]->keywordSet().asString("UNIT");
	    } else if (crvalCols_p[axisno]->keywordSet().fieldNumber("TUNIT") >= 0) {
		crvalColUnits_p(axisno) = 
		    crvalCols_p[axisno]->keywordSet().asString("TUNIT");
	    } else {
		crvalColUnits_p(axisno) = "";
	    }
	} else if (colNames(i).matches(crpix)) {
	    Int axisno = atol(colNames(i).after("CRPIX").chars()) - 1;
	    AlwaysAssert(axisno < maxAxis, AipsError);
	    if (isWritable_) {
		rwcrpixCols_p[axisno] = new TableColumn(*tab_p, colNames(i));
		AlwaysAssert(rwcrpixCols_p[axisno], AipsError);
	    } 
	    crpixCols_p[axisno] = new ROTableColumn(*tab_p, colNames(i));
	    AlwaysAssert(crpixCols_p[axisno], AipsError);
	    crpixColNames_p(axisno) = colNames(i);
	    crpixTypes_p(axisno) = crpixCols_p[axisno]->columnDesc().dataType();
	} else if (colNames(i).matches(cdelt)) {
	    Int axisno = atol(colNames(i).after("CDELT").chars()) - 1;
	    AlwaysAssert(axisno < maxAxis, AipsError);
	    if (isWritable_) {
		rwcdeltCols_p[axisno] = new TableColumn(*tab_p, colNames(i));
		AlwaysAssert(rwcdeltCols_p[axisno], AipsError);
	    }
	    cdeltCols_p[axisno] = new ROTableColumn(*tab_p, colNames(i));
	    AlwaysAssert(cdeltCols_p[axisno], AipsError);
	    cdeltColNames_p(axisno) = colNames(i);
	    cdeltTypes_p(axisno) = cdeltCols_p[axisno]->columnDesc().dataType();
	} else if (colNames(i).matches(crota)) {
	    Int axisno = atol(colNames(i).after("CROTA").chars()) - 1;
	    AlwaysAssert(axisno < maxAxis, AipsError);
	    if (isWritable_) {
		rwcrotaCols_p[axisno] = new TableColumn(*tab_p, colNames(i));
		AlwaysAssert(rwcrotaCols_p[axisno], AipsError);
	    }
	    crotaCols_p[axisno] = new ROTableColumn(*tab_p, colNames(i));
	    AlwaysAssert(crotaCols_p[axisno], AipsError);
	    crotaColNames_p(axisno) = colNames(i);
	    crotaTypes_p(axisno) = crotaCols_p[axisno]->columnDesc().dataType();
	} else if (colNames(i).matches(cunit)) {
	    Int axisno = atol(colNames(i).after("CUNIT").chars()) - 1;
	    AlwaysAssert(axisno < maxAxis, AipsError);
	    if (isWritable_) {
		rwcunitCols_p[axisno] = new TableColumn(*tab_p, colNames(i));
		AlwaysAssert(rwcunitCols_p[axisno], AipsError);
	    }
	    cunitCols_p[axisno] = new ROTableColumn(*tab_p, colNames(i));
	    AlwaysAssert(cunitCols_p[axisno], AipsError);
	    cunitColNames_p(axisno) = colNames(i);
	}
    }
    // time-related columns
    timeUnit_p = "s";
    if (tab_p->tableDesc().isColumn("TIME")) {
	timeCol_p.attach(*tab_p, "TIME");
	if (timeCol_p.keywordSet().fieldNumber("UNIT")) {
	    timeUnit_p = timeCol_p.keywordSet().asString("UNIT");
	}
    }
    if (tab_p->tableDesc().isColumn("DATE-OBS")) dateObsCol_p.attach(*tab_p, "DATE-OBS");
}

uInt SDIterAxes::nsdrecords(Int rownr) const
{
    IPosition thisShape(shape(rownr));
    Int faxis = freqAxis(rownr);
    Int saxis = stokesAxis(rownr);
    if (faxis >= 0) thisShape(faxis) = 1;
    if (saxis >= 0) thisShape(saxis) = 1;
    return thisShape.product();
}

uInt SDIterAxes::nsdrecords() const
{
    uInt sum = 0;
    for (uInt i=0;i<dataColumn_p->nrow();i++) {
	sum += nsdrecords(i);
    }
    return sum;
}


void SDIterAxes::stringVector(Int rownr, Vector<String> &result,
			      const PtrBlock<ROTableColumn *> &colsPtr,
			      String defaultValue)
{
    if (result.nelements() != shape(rownr).nelements()) {
	result.resize(shape(rownr).nelements());
    }
    result = defaultValue;
    AlwaysAssert(colsPtr.nelements() >= result.nelements(), AipsError);
    for (uInt i=0; i<result.nelements();i++) {
	if (colsPtr[i] && colsPtr[i]->isDefined(rownr)) 
	    result(i) = colsPtr[i]->asString(rownr);
    }
}

void SDIterAxes::stringVector(Int rownr, Vector<String> &result,
			      const PtrBlock<ROTableColumn *> &colsPtr,
			      Vector<String> &defaultValues)
{
    stringVector(rownr, result, colsPtr, "");
    // would this be more efficient with a MaskedArray?
    if (defaultValues.nelements() < result.nelements()) {
	uInt oldSize = defaultValues.nelements();
	defaultValues.resize(result.nelements(), True);
	for (uInt i=oldSize;i<defaultValues.nelements();i++)
	    defaultValues(i) = "";
    }
    for (uInt i=0;i<result.nelements();i++) {
	if (result(i) == "") result(i) = defaultValues(i);
    }
}

void SDIterAxes::doubleVector(Int rownr, Vector<Double> &result,
			      const PtrBlock<ROTableColumn *> &colsPtr,
			      Double defaultValue)
{
    if (result.nelements() != shape(rownr).nelements()) {
	result.resize(shape(rownr).nelements());
    }
    result = defaultValue;
    AlwaysAssert(colsPtr.nelements() >= result.nelements(), AipsError);
    for (uInt i=0; i<result.nelements();i++) {
	if (colsPtr[i] && colsPtr[i]->isDefined(rownr)) 
	    result(i) = colsPtr[i]->asdouble(rownr);
    }
}

Bool SDIterAxes::setStringCols(Int rownr, const Vector<String> &vals,
			       const String &baseName,
			       PtrBlock<ROTableColumn *> &rocolsPtr,
			       PtrBlock<TableColumn *> &rwcolsPtr,
			       Vector<String> &colNames,
			       Bool &added,
			       String defaultValue)
{
    LogIO os(LogOrigin("SDIterAxes","setStringCols"));
    Bool result = True;
    if (isWritable_) {
	if (rwcolsPtr.nelements() < vals.nelements()) resize(vals.nelements());
	for (uInt i=0; i<vals.nelements();i++) {
	    if (!rwcolsPtr[i] || rwcolsPtr[i]->isNull()) {
		// make one
		ostringstream ostr;
		ostr << baseName << (i+1);
		String newColName(ostr);
		if (!tab_p->tableDesc().isColumn(newColName)) {
		    ScalarColumnDesc<String> newColDesc(newColName);
		    newColDesc.setDefault(defaultValue);
		    tab_p->addColumn(newColDesc);
		    added = (added || True);
		} 
		delete rocolsPtr[i];
		rocolsPtr[i] = new ROTableColumn(*tab_p, newColName);
		AlwaysAssert(rocolsPtr[i], AipsError);
		colNames(i) = newColName;
		delete rwcolsPtr[i];
		rwcolsPtr[i] = new TableColumn(*tab_p, newColName);
		AlwaysAssert(rwcolsPtr[i], AipsError);
	    }
	    rwcolsPtr[i]->putScalar(rownr, vals(i));
	}
	for (uInt i=vals.nelements();i<rwcolsPtr.nelements();i++) {
	    if (rwcolsPtr[i] && !rwcolsPtr[i]->isNull()) {
		rwcolsPtr[i]->putScalar(rownr, defaultValue);
	    }
	}
    } else {
	os << LogIO::SEVERE
	   << WHERE
	   << "The output table is not writable."
	   << "\nUnable to save information to " << tab_p->tableName()
	   << LogIO::POST;
	result = False;
    }
    return result;
}

Bool SDIterAxes::setDoubleCols(Int rownr, const Vector<Double> &vals,
			       const String &baseName,
			       Vector<Int> &types,
			       PtrBlock<ROTableColumn *> &rocolsPtr,
			       PtrBlock<TableColumn *> &rwcolsPtr,
			       Vector<String> &colNames,
			       Bool &added,
			       Double defaultValue)
{
    LogIO os(LogOrigin("SDIterAxes","setDoubleCols"));
    Bool result = True;
    if (isWritable_) {
	if (rwcolsPtr.nelements() < vals.nelements()) resize(vals.nelements());
	for (uInt i=0; i<vals.nelements();i++) {
	    if (!rwcolsPtr[i] || rwcolsPtr[i]->isNull()) {
		// make one
		ostringstream ostr;
		ostr << baseName << (i+1);
		String newColName(ostr);
		if (!tab_p->tableDesc().isColumn(newColName)) {
		    ScalarColumnDesc<Double> newColDesc(newColName);
		    newColDesc.setDefault(defaultValue);
		    tab_p->addColumn(newColDesc);
		    added = (added || True);
		} 
		types(i) = TpDouble;
		delete rocolsPtr[i];
		rocolsPtr[i] = new ROTableColumn(*tab_p, newColName);
		AlwaysAssert(rocolsPtr[i], AipsError);
		colNames(i) = newColName;
		delete rwcolsPtr[i];
		rwcolsPtr[i] = new TableColumn(*tab_p, newColName);
		AlwaysAssert(rwcolsPtr[i], AipsError);
	    }
	    switch (types(i)) {
	    case TpUShort:
		{
		    if (vals(i) <= USHRT_MAX && vals(i) >= 0.0) {
			uShort val = uShort(vals(i));
			if (near(Double(val),vals(i))) {
			    rwcolsPtr[i]->putScalar(rownr, val);
			} else {
			    os << LogIO::SEVERE
			       << WHERE
			       << "Unable to demote the value "
			       << vals(i) << " to the uShort type of the output column "
			       << rwcolsPtr[i]->columnDesc().name()
			       << LogIO::POST;
			    result = False;
			}
		    } else {
			os << LogIO::SEVERE
			   << WHERE
			   << "Unable to demote the value "
			   << vals(i) << " to the uShort type of the output column "
			   << rwcolsPtr[i]->columnDesc().name()
			   << LogIO::POST;
			result = False;
		    }
		}
		break;
	    case TpShort:
		{
		    if (vals(i) <= SHRT_MAX && vals(i) >= SHRT_MIN) {
			Short val = Short(vals(i));
			if (near(Double(val),vals(i))) {
			    rwcolsPtr[i]->putScalar(rownr, val);
			} else {
			    os << LogIO::SEVERE
			       << WHERE
			       << "Unable to demote the value "
			       << vals(i) << " to the Short type of the output column "
			       << rwcolsPtr[i]->columnDesc().name()
			       << LogIO::POST;
			    result = False;
			}
		    } else {
			os << LogIO::SEVERE
			   << WHERE
			   << "Unable to demote the value "
			   << vals(i) << " to the Short type of the output column "
			   << rwcolsPtr[i]->columnDesc().name()
			   << LogIO::POST;
			result = False;
		    }
		    break;
		}
	    case TpUInt:
		{
		    if (vals(i) <= ULONG_MAX && vals(i) >= 0.0) {
			uInt val = uInt(vals(i));
			if (near(Double(val),vals(i))) {
			    rwcolsPtr[i]->putScalar(rownr, val);
			} else {
			    os << LogIO::SEVERE
			       << WHERE
			       << "Unable to demote the value "
			       << vals(i) << " to the uInt type of the output column "
			       << rwcolsPtr[i]->columnDesc().name()
			       << LogIO::POST;
			    result = False;
			}
		    } else {
			os << LogIO::SEVERE
			   << WHERE
			   << "Unable to demote the value "
			   << vals(i) << " to the uInt type of the output column "
			   << rwcolsPtr[i]->columnDesc().name()
			   << LogIO::POST;
			result = False;
		    }
		}
		break;
	    case TpInt:
		{
		    if (vals(i) <= LONG_MAX && vals(i) >= LONG_MIN) {
			Int val = Int(vals(i));
			if (near(Double(val),vals(i))) {
			    rwcolsPtr[i]->putScalar(rownr, val);
			} else {
			    os << LogIO::SEVERE
			       << WHERE
			       << "Unable to demote the value "
			       << vals(i) << " to the Int type of the output column "
			       << rwcolsPtr[i]->columnDesc().name()
			       << LogIO::POST;
			    result = False;
			}
		    } else {
			os << LogIO::SEVERE
			   << WHERE
			   << "Unable to demote the value "
			   << vals(i) << " to the Int type of the output column "
			   << rwcolsPtr[i]->columnDesc().name()
			   << LogIO::POST;
			result = False;
		    }
		}
		break;
	    case TpFloat:
		{
		    if (vals(i) <= C::flt_max && vals(i) >= -C::flt_max) {
			rwcolsPtr[i]->putScalar(rownr, Float(vals(i)));
		    } else {
			os << LogIO::SEVERE
			   << WHERE
			   << "Unable to demote the value "
			   << vals(i) << " to the Float type of the output column "
			   << rwcolsPtr[i]->columnDesc().name()
			   << LogIO::POST;
			result = False;
		    }
		}
		break;
	    case TpDouble:
		rwcolsPtr[i]->putScalar(rownr, vals(i));
		break;
	    default:
		os << LogIO::SEVERE
		   << WHERE
		   << "Unable to store the value "
		   << vals(i) << " to the "
		   << Int(rwcolsPtr[i]->columnDesc().dataType())
		   << " type of the output column "
		   << rwcolsPtr[i]->columnDesc().name()
		   << LogIO::POST;
		result = False;
		break;
	    }
	}
	for (uInt i=vals.nelements();i<rwcolsPtr.nelements();i++) {
	    if (rwcolsPtr[i] && !rwcolsPtr[i]->isNull()) {
		rwcolsPtr[i]->putScalar(rownr, defaultValue);
	    }
	}
    } else {
	os << LogIO::SEVERE
	   << WHERE
	   << "The output table is not writable."
	   << "\nUnable to save information to " << tab_p->tableName()
	   << LogIO::POST;
	result = False;
    }
    return result;
}

void SDIterAxes::setAxisCache(Int rownr) const
{
    freqAxis_p = longAxis_p = latAxis_p = timeAxis_p = stokesAxis_p =
	beamAxis_p = rcvrAxis_p = -1;
    cachedRow_p = rownr;
    uInt nels = shape(rownr).nelements();
    for (uInt i=0;i<nels;i++) {
	String thisType(ctypeCols_p[i]->asString(rownr));
	thisType.upcase();
	if (freqAxis_p == -1 && thisType.matches(freqAxisRegex_p))
	    freqAxis_p = i;
	else if (longAxis_p == -1 && thisType.matches(longAxisRegex_p))
	    longAxis_p = i;
	else if (latAxis_p == -1 && thisType.matches(latAxisRegex_p))
	    latAxis_p = i;
	else if (timeAxis_p == -1 && thisType.matches(timeAxisRegex_p))
	    timeAxis_p = i;
	else if (stokesAxis_p == -1 && thisType.matches(stokesAxisRegex_p))
	    stokesAxis_p = i;
	else if (beamAxis_p == -1 && thisType.matches(beamAxisRegex_p))
	    beamAxis_p = i;
	else if (rcvrAxis_p == -1 && thisType.matches(rcvrAxisRegex_p))
	    rcvrAxis_p = i;
    }
}

void SDIterAxes::cleanup() 
{
    for (uInt i=0;i<ctypeCols_p.nelements();i++) {
	delete ctypeCols_p[i]; ctypeCols_p[i] = 0;
	delete crvalCols_p[i]; crvalCols_p[i] = 0;
	delete crpixCols_p[i]; crpixCols_p[i] = 0;
	delete cdeltCols_p[i]; cdeltCols_p[i] = 0;
	delete crotaCols_p[i]; crotaCols_p[i] = 0;
	delete cunitCols_p[i]; cunitCols_p[i] = 0;
    }
    for (uInt i=0;i<rwctypeCols_p.nelements();i++) {
	delete rwctypeCols_p[i]; rwctypeCols_p[i] = 0;
	delete rwcrvalCols_p[i]; rwcrvalCols_p[i] = 0;
	delete rwcrpixCols_p[i]; rwcrpixCols_p[i] = 0;
	delete rwcdeltCols_p[i]; rwcdeltCols_p[i] = 0;
	delete rwcrotaCols_p[i]; rwcrotaCols_p[i] = 0;
	delete rwcunitCols_p[i]; rwcunitCols_p[i] = 0;
    }
    delete dataColumn_p;
    dataColumn_p = 0;
    delete tab_p;
    tab_p = 0;
}

void SDIterAxes::resize(Int naxes) 
{
    // expand everything appropriately, but don't attach to anything
    // that will be done elsewhere as necessary
    // Blocks preserve contents on resize by default, vectors do not
    // by default.
    Int oldSize = rwctypeCols_p.nelements();
    rwctypeCols_p.resize(naxes);
    rwcrvalCols_p.resize(naxes);
    rwcrpixCols_p.resize(naxes);
    rwcdeltCols_p.resize(naxes);
    rwcrotaCols_p.resize(naxes);
    rwcunitCols_p.resize(naxes);
    ctypeCols_p.resize(naxes);
    crvalCols_p.resize(naxes);
    crpixCols_p.resize(naxes);
    cdeltCols_p.resize(naxes);
    crotaCols_p.resize(naxes);
    cunitCols_p.resize(naxes);
    crvalColUnits_p.resize(naxes, True);
    crvalTypes_p.resize(naxes, True);
    crpixTypes_p.resize(naxes, True);
    cdeltTypes_p.resize(naxes, True);
    crotaTypes_p.resize(naxes, True);
    ctypeColNames_p.resize(naxes, True);
    crvalColNames_p.resize(naxes, True);
    crpixColNames_p.resize(naxes, True);
    cdeltColNames_p.resize(naxes, True);
    cunitColNames_p.resize(naxes, True);
    crotaColNames_p.resize(naxes, True);
    for (Int i=oldSize;i<naxes;i++) {
	rwctypeCols_p[i] = static_cast<TableColumn *>(0);
	rwcrvalCols_p[i] = static_cast<TableColumn *>(0);
	rwcrpixCols_p[i] = static_cast<TableColumn *>(0);
	rwcdeltCols_p[i] = static_cast<TableColumn *>(0);
	rwcrotaCols_p[i] = static_cast<TableColumn *>(0);
	rwcunitCols_p[i] = static_cast<TableColumn *>(0);
	ctypeCols_p[i] = static_cast<ROTableColumn *>(0);
	crvalCols_p[i] = static_cast<ROTableColumn *>(0);
	crpixCols_p[i] = static_cast<ROTableColumn *>(0);
	cdeltCols_p[i] = static_cast<ROTableColumn *>(0);
	crotaCols_p[i] = static_cast<ROTableColumn *>(0);
	cunitCols_p[i] = static_cast<ROTableColumn *>(0);
	crvalColUnits_p(i) = "";
    }
}

} //# NAMESPACE CASA - END

