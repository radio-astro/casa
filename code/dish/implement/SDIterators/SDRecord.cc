//# SDRecord.cc:  this defines SDRecord, the single dish record
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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

#include <dish/SDIterators/SDRecord.h>

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasureHolder.h>
#include <measures/Measures/Stokes.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>

#include <casa/iostream.h>


namespace casa { //# NAMESPACE CASA - BEGIN

RecordDesc SDRecord::itsBasicDesc;
RecordDesc SDRecord::itsBasicComplexDesc;

SDRecord::SDRecord(DataType dataType)
    : Record(requiredDesc(dataType)), itsShape(2,0,0)
{
    initPointers();
    resize(IPosition(2,1,0));
    defaultValues();
}

SDRecord::SDRecord(const SDRecord& other)
    : Record(requiredDesc(other.arrType())), itsShape(2,0,0)
{
    initPointers();
    if (other.isValid()) {
	*this = other;
    } else {
	itsShape = IPosition(2,1,0);
	resize(IPosition(2,1,0));
	defaultValues();
    }
}

SDRecord::SDRecord(const Record& other)
    : Record(requiredDesc(other.subRecord("data").dataType("arr"))), itsShape(2,0,0)
{
    initPointers();
    if (isValid(other)) {
	Record::operator=(other);
	cacheShape();
    } else {
	resize(IPosition(2,1,0));
	defaultValues();
    }
}

SDRecord::~SDRecord()
{ 
// nothing
}

SDRecord& SDRecord::operator=(const SDRecord& other)
{
    if (this != &other && other.isValid()) {
	Record::operator=(other);
	cacheShape();
    }
    return *this;
}

void SDRecord::initPointers() 
{
    itsData.attachToRecord(*this, "data");
    itsDesc.attachToRecord(*itsData, "desc");
    itsHeader.attachToRecord(*this, "header");
    itsOther.attachToRecord(*this, "other");
    itsHist.attachToRecord(*this, "hist");

    itsCorrType.attachToRecord(*itsDesc,"corr_type");

    itsWeight.attachToRecord(*itsData, "weight");
    itsSigma.attachToRecord(*itsData, "sigma");
    itsFlag.attachToRecord(*itsData, "flag");

    itsROWeight.attachToRecord(*itsData, "weight");
    itsROSigma.attachToRecord(*itsData, "sigma");
    itsROFlag.attachToRecord(*itsData, "flag");

    switch (arrType()) {
    case TpFloat:
    case TpArrayFloat:
      itsFarr.attachToRecord(*itsData, "arr");
      itsROFarr.attachToRecord(*itsData, "arr");
      break;
    case TpComplex:
    case TpArrayComplex:
      itsCarr.attachToRecord(*itsData, "arr");
      itsROCarr.attachToRecord(*itsData, "arr");
      break;
    default:
      throw(AipsError("SDRecord::init() unrecognized DataType for arr field"));
      break;
    }

    itsTcal.attachToRecord(*itsHeader, "tcal");
    itsTrx.attachToRecord(*itsHeader, "trx");

    itsTsys.attachToRecord(*itsHeader, "tsys");
    itsTdur.attachToRecord(*itsHeader, "duration");
    itsTexp.attachToRecord(*itsHeader, "exposure");
    itsFrest.attachToRecord(*itsDesc, "restfrequency");
    itsFref.attachToRecord(*itsDesc, "reffrequency");
    itsVeldef.attachToRecord(*itsHeader, "veldef");
    itsChanFreq.attachToRecord(*itsDesc, "chan_freq");
    itsRefFrame.attachToRecord(*itsDesc, "refframe");

    itsROTsys.attachToRecord(*itsHeader, "tsys");
    itsROTdur.attachToRecord(*itsHeader, "duration");
    itsROTexp.attachToRecord(*itsHeader, "exposure");
    itsROFrest.attachToRecord(*itsDesc, "restfrequency");
    itsROFref.attachToRecord(*itsDesc, "reffrequency");
    itsROVeldef.attachToRecord(*itsHeader, "veldef");
    itsROChanFreq.attachToRecord(*itsDesc, "chan_freq");
    itsRORefFrame.attachToRecord(*itsDesc, "refframe");
}

void SDRecord::cacheShape()
{
    switch (arrType()) {
    case TpFloat:
    case TpArrayFloat:
	itsShape = (*itsFarr).shape();
	break;
    case TpComplex:
    case TpArrayComplex:
	itsShape = (*itsCarr).shape();
	break;
    default:
	throw(AipsError("SDRecord::cacheShape() unrecognized DataType for arr field"));
	break;
    }
}    

RecordDesc SDRecord::requiredDesc(DataType dataType) 
{
    initStatics();
    switch (dataType) {
    case TpArrayComplex:
    case TpComplex:
	return itsBasicComplexDesc;
	break;
    case TpArrayFloat:
    case TpFloat:
	return itsBasicDesc;
	break;
    default:
	throw(AipsError(String("SDRecord::requiredDesc(DataType dataType) - data type is not complex or float")));
	break;
    }
}

void SDRecord::initStatics()
{
    if (itsBasicDesc.nfields() == 0) {
	RecordDesc desc;
	desc.addField("chan_freq", TpRecord);
	desc.addField("refframe", TpString);
	desc.addField("reffrequency", TpDouble);
	desc.addField("chan_width", TpDouble);
	desc.addField("restfrequency", TpDouble);
	desc.addField("corr_type", TpArrayString);
	desc.addField("units", TpString);

	RecordDesc data;
	data.addField("desc",desc);
	data.addField("arr",TpArrayFloat);
	data.addField("flag",TpArrayBool);
	data.addField("weight",TpArrayFloat);
	data.addField("sigma",TpArrayFloat);

	RecordDesc complexData;
	complexData.addField("desc",desc);
	complexData.addField("arr",TpArrayComplex);
	complexData.addField("flag",TpArrayBool);
	complexData.addField("weight",TpArrayFloat);
	complexData.addField("sigma",TpArrayFloat);

	RecordDesc header;
	header.addField("time",TpRecord);
	header.addField("scan_number",TpInt);
	header.addField("source_name",TpString);
	header.addField("direction",TpRecord);
	header.addField("refdirection",TpRecord);
	header.addField("veldef", TpString);
	header.addField("transition",TpString);
	header.addField("exposure",TpDouble);
	header.addField("duration",TpDouble);
	header.addField("observer",TpString);
	header.addField("project",TpString);
	header.addField("resolution",TpDouble);
	header.addField("bandwidth",TpDouble);
	header.addField("tcal",TpArrayFloat);
	header.addField("trx",TpArrayFloat);
	header.addField("tsys",TpArrayFloat);
	header.addField("telescope",TpString);
	header.addField("telescope_position",TpRecord);
	header.addField("pressure", TpDouble);
	header.addField("tambient", TpDouble);
	header.addField("dewpoint", TpDouble);
	header.addField("wind_dir", TpDouble);
	header.addField("wind_speed", TpDouble);
	header.addField("azel",TpRecord);

	// and add them to the itsBasicDesc
	itsBasicDesc.addField("data",data);
	itsBasicDesc.addField("header",header);
	itsBasicDesc.addField("hist",TpArrayString);
	itsBasicDesc.addField("other",TpRecord);

	// and itsBasicComplexDesc
	itsBasicComplexDesc.addField("data",complexData);
	itsBasicComplexDesc.addField("header",header);
	itsBasicComplexDesc.addField("hist",TpArrayString);
	itsBasicComplexDesc.addField("other",TpRecord);
    }
}

void SDRecord::defaultValues()
{
    // scalar and fixed shape fields
    String holderErr;
    desc().define("refframe","LSRK");
    desc().define("reffrequency", Double(0.0));
    desc().define("chan_width", Double(0.0));
    desc().define("restfrequency", Double(0.0));
    desc().define("units","");

    // default time - O MJD
    MEpoch time;
    MeasureHolder timeHolder(time);
    if (!timeHolder.toRecord(holderErr, header().rwSubRecord("time"))) {
	String errMsg = "SDRecord::defaultValues() unexpected problem saving default time Measure :" 
	  + holderErr;
	throw(AipsError(errMsg));
    }
    header().define("scan_number", -1);
    header().define("source_name", "");
    MDirection dir;
    MeasureHolder dirHolder(dir);
    if (!dirHolder.toRecord(holderErr, header().rwSubRecord("direction"))) {
	String errMsg = "SDRecord::defaultValues() unexpected problem saving default direction Measure :" 
	  + holderErr;
	throw(AipsError(errMsg));
    }
    if (!dirHolder.toRecord(holderErr, header().rwSubRecord("refdirection"))) {
	String errMsg = "SDRecord::defaultValues() unexpected problem saving default refdirection Measure :"
	  + holderErr;
	throw(AipsError(errMsg));
    }
    header().define("veldef",MDoppler::showType(MDoppler::RADIO));
    header().define("transition", "");
    header().define("exposure", Double(0.0));
    header().define("duration", Double(0.0));
    header().define("observer", "");
    header().define("project", "");
    header().define("resolution", Double(0.0));
    header().define("bandwidth", Double(0.0));
    header().define("telescope", "");
    MPosition pos;
    MeasureHolder posHolder(pos);
    if (!posHolder.toRecord(holderErr, header().rwSubRecord("telescope_position"))) {
	String errMsg = 
	    "SDRecord::defaultValues() unexpected problem saving default telescope_position Measure :" + 
	    holderErr;
	throw(AipsError(errMsg));
    }
    header().define("pressure", Double(0.0));
    header().define("dewpoint", Double(0.0));
    header().define("tambient", Double(0.0));
    header().define("wind_dir", Double(0.0));
    header().define("wind_speed", Double(0.0));
    // we know that azel is a MDirection of type AZEL, default to 0,0
    MDirection azel((MVDirection(Quantity(0,"deg"), Quantity(0,"deg"))),
		     MDirection::Ref(MDirection::AZEL));
    MeasureHolder azelHolder(azel);
    if (!azelHolder.toRecord(holderErr, header().rwSubRecord("azel"))) {
	String errMsg = "SDRecord::defaultValues() unexpected problem saving default azel Measure :" 
	  + holderErr;
	throw(AipsError(errMsg));
    }

    // variable shaped fields
    defaultMatValues();
    defaultVecValues();

    // Make sure history is a vector with zero elements to start with
    hist().resize(IPosition(1,0));
}

void SDRecord::defaultMatValues()
{
    *itsWeight = 0.0;
    *itsSigma = 0.0;

    *itsFlag = True;
    switch (arrType()) {
    case TpFloat:
    case TpArrayFloat:
	*itsFarr = 0.0;
	break;
    case TpComplex:
    case TpArrayComplex:
	*itsCarr = 0.0;
	break;
    default:
      throw(AipsError("SDRecord::defaultMatValues() unrecognized DataType for arr field"));
      break;
    }

    // chan_freq is set here rather than in defaultVecValues because the things
    // in defaultVecValues are all vectors with nstokes elements and parts of this class
    // rely on that fact

    String holderErr;
    Vector<Double> zeroFreq(shape()(1), 0.0);
    Quantum<Vector<Double> > chanFreq(zeroFreq, "Hz");
    QuantumHolder chanFreqHolder(chanFreq);
    if (!chanFreqHolder.toRecord(holderErr, desc().rwSubRecord("chan_freq"))) {
	String errMsg = 
	    "SDRecord::defaultValues() unexpected problem saving default chan_freq Quanta :" + 
	    holderErr;
	throw(AipsError(errMsg));
    }
}

void SDRecord::defaultVecValues()
{
    *itsTcal = 0.0;
    *itsTrx = 0.0;
    *itsTsys = 0.0;

    Vector<String> corrTypes(shape()(0), Stokes::name(Stokes::Undefined));
    desc().define("corr_type", corrTypes);
}

Bool SDRecord::resize(const IPosition &newShape) 
{
    if (newShape.nelements() != 2) return False;
    if (itsShape == newShape) return False;

    // matrices
    (*itsWeight).resize(newShape);
    (*itsSigma).resize(newShape);
    (*itsFlag).resize(newShape);

    switch (arrType()) {
    case TpFloat:
    case TpArrayFloat:
	(*itsFarr).resize(newShape);
	break;
    case TpComplex:
    case TpArrayComplex:
	(*itsCarr).resize(newShape);
	break;
    default:
      throw(AipsError("SDRecord::resize() unrecognized DataType for arr field"));
      break;
    }

    // vectors, only if nstokes has changed
    Bool newVecs = (itsShape(0) != newShape(0));
    if (newVecs) {
        IPosition vecShape(1,newShape(0));

	(*itsTcal).resize(vecShape);
	(*itsTrx).resize(vecShape);
	(*itsTsys).resize(vecShape);
    }
    itsShape = newShape;

    // and set them to their default values
    defaultMatValues();
    if (newVecs) defaultVecValues();

    return True;
}
	
ostream& SDRecord::showRecord(ostream &os, const Record& rec)
{
    RecordDesc desc(rec.description());
    os <<  "Description: " << endl;
    os << desc << endl;
    // loop over each field
    for (uInt i=0;i<desc.nfields();i++) {
 	switch (desc.type(i)) {
 	case TpBool:
	    {
		RORecordFieldPtr<Bool> field(rec, i);
		os << *field;
	    }
	    break;
	case TpUChar:
	    {
		RORecordFieldPtr<uChar> field(rec, i);
		os << uInt(*field);
	    }
	    break;
	case TpShort:
	    {
		RORecordFieldPtr<Short> field(rec, i);
		os << *field;
	    }
	    break;
	case TpInt:
	    {
		RORecordFieldPtr<Int> field(rec, i);
		os << *field;
	    }
	    break;
	case TpFloat:
	    {
		RORecordFieldPtr<Float> field(rec, i);
		os << *field;
	    }
	    break;
	case TpDouble:
	    {
		RORecordFieldPtr<Double> field(rec, i);
		os << *field;
	    }
	    break;
	case TpComplex:
	    {
		RORecordFieldPtr<Complex> field(rec, i);
		os << *field;
	    }
	    break;
	case TpDComplex:
	    {
		RORecordFieldPtr<DComplex> field(rec, i);
		os << *field;
	    }
	    break;
	case TpString:
	    {
		RORecordFieldPtr<String> field(rec, i);
		os << *field;
	    }
	    break;
	case TpTable:
	    {
		os << "Table";
		break;
	    }
	case TpArrayBool:
	    {
		RORecordFieldPtr<Array<Bool> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpArrayUChar:
	    {
		RORecordFieldPtr<Array<uChar> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpArrayShort:
	    {
		RORecordFieldPtr<Array<Short> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpArrayInt:
	    {
		RORecordFieldPtr<Array<Int> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpArrayFloat:
	    {
		RORecordFieldPtr<Array<Float> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpArrayDouble:
	    {
		RORecordFieldPtr<Array<Double> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpArrayComplex:
	    {
		RORecordFieldPtr<Array<Complex> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpArrayDComplex:
	    {
		RORecordFieldPtr<Array<DComplex> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpArrayString:
	    {
		RORecordFieldPtr<Array<String> > field(rec, i);
		os << *field;
	    }
	    break;
	case TpRecord:
	    {
		RORecordFieldPtr<Record> field(rec, i);
		SDRecord::showRecord(os,*field);
	    }
	    break;
	case TpOther:
	    {
		os << "TpOther";
	    }
	    break;
	default:
	    {
		os << "Unrecognized type : " << desc.type(i);
	    }
	}
	os << " | ";
    }
    os << endl;
    return os;
}

Bool SDRecord::isConsistent(String &errmsg) const
{
    Bool result = True;
    errmsg = "";

    // shape of data.arr, data.flag, data.sigma, and data.weight must be the same
    if (result && !itsShape.isEqual((*itsWeight).shape())) {
	errmsg = "shape of weight does not match that of arr";
	result = False;
    }
    if (result && !itsShape.isEqual((*itsFlag).shape())) {
	errmsg = "shape of flag does not match that of arr";
	result = False;
    }
    if (result && !itsShape.isEqual((*itsSigma).shape())) {
	errmsg = "shape of sigma does not match that of arr";
	result = False;
    }
    uInt nstokes = itsShape(0);
    // There must be nstokes of corr_type, tcal, trx, and tsys
    if (result && nstokes != (*itsCorrType).nelements()) {
	errmsg = "the number of elements of corr_type does not match the length of the first axis of arr";
	result = False;
    }
    if (result && nstokes != (*itsTcal).nelements()) {
	errmsg = "the number of elements of tcal does not match the length of the first axis of arr";
	result = False;
    }
    if (result && nstokes != (*itsTrx).nelements()) {
	errmsg = "the number of elements of trx does not match the length of the first axis of arr";
	result = False;
    }
    if (result && nstokes != (*itsTsys).nelements()) {
	errmsg = "the number of elements of tsys does not match the length of the first axis of arr";
	result = False;
    }
    // its too much effort to check the number of values of chan_freq here since they are stored
    // as a Quantity vector.
    return result;
}

} //# NAMESPACE CASA - END

