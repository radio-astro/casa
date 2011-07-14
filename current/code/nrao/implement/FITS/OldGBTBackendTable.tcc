//# OldGBTBackendTable.cc: interface to GBT backend FITS tables with AIPS++ Look and Feel.
//# Copyright (C) 1995,1996,1997,1998,1999,2000,2001
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

#include <nrao/FITS/OldGBTBackendTable.h>
#include <fits/FITS/BinTable.h>

#include <casa/Containers/RecordField.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <fits/FITS/fits.h>
#include <fits/FITS/fitsio.h>
#include <fits/FITS/hdu.h>
#include <tables/Tables.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/Time.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>

#include <casa/stdio.h>

// a useful constant, number of seconds in a day
Double secPerDay = 24.0 * 60.0 * 60.0;

Int arrayFieldToScalar(RecordDesc &desc, Int whichField)
{
    // note, this changes the field number of an affected field
    // return the new field number

    AlwaysAssert(whichField >= 0, AipsError);

    if (!desc.isArray(whichField)) return whichField;

    DataType dtype = desc.type(whichField);
    String fname(desc.name(whichField));
    desc.removeField(whichField);

    // determine equivalent non-array type
    switch (dtype) {
    case TpArrayBool: dtype = TpBool; break;
    case TpArrayChar: dtype = TpChar; break;
    case TpArrayUChar: dtype = TpUChar; break;
    case TpArrayShort: dtype = TpShort; break;
    case TpArrayUShort: dtype = TpUShort; break;
    case TpArrayInt: dtype = TpInt; break;
    case TpArrayUInt: dtype = TpUInt; break;
    case TpArrayFloat: dtype = TpFloat; break;
    case TpArrayDouble: dtype = TpDouble; break;
    case TpArrayComplex: dtype = TpComplex; break;
    case TpArrayDComplex: dtype = TpDComplex; break;
    case TpArrayString: dtype = TpString; break;
	// default, do nothing
    default:
	;
    }
    desc.addField(fname, dtype);
    return desc.fieldNumber(fname);
}

RecordDesc OldGBTBackendTable::descriptionFromTableDesc(const TableDesc& tabDesc) 
{
    RecordDesc description;
    uInt ncol = tabDesc.ncolumn();
    for (uInt i=0; i<ncol; i++) {
	if (tabDesc[i].isArray()) {
	    description.addField(tabDesc[i].name(), tabDesc[i].dataType(), 
				 tabDesc[i].shape());
	} else if (tabDesc[i].isScalar()) {
	    description.addField(tabDesc[i].name(), tabDesc[i].dataType());
	} else {
	    // the only other possibility is that it is a table
	    description.addField(tabDesc[i].name(),
			   descriptionFromTableDesc(*(tabDesc[i].tableDesc())));
	}
	// set the comment
	description.setComment(description.fieldNumber(tabDesc[i].name()), 
			       tabDesc[i].comment());
    }
    return description;
}

RecordDesc OldGBTBackendTable::descriptionFromOKKeys(const TableRecord& kwset) {
    RecordDesc description;
    for (uInt i=0;i<kwset.nfields();i++) {
	// everything is a scalar
	if (!anyEQ(ignoredFITS,kwset.description().name(i))) {
	    // promot any floats to double
	    DataType dtype = kwset.description().type(i);
	    if (dtype == TpFloat) dtype = TpDouble;
	    description.addField(kwset.description().name(i), dtype);
	    // set comment
	    description.setComment(description.fieldNumber(kwset.description().name(i)), 
				   kwset.description().comment(i));
	}
    }
    return description;
}

OldGBTBackendTable::OldGBTBackendTable(const String &fileName)
    : isValid_p(True), row_nr_p(-1), phase_table_p(0), rec_table_p(0), 
      phase_map_p(0), rec_map_p(0), phase_id_p(0), rec_id_p(0), 
      rcvrIndexed_p(0), time_p(0), data_table_p(0), fitsIO_p(0), 
      row_p(RecordInterface::Variable), row_fields_p(0), oldStart_p(0)
{
    init_ignoredFITS();
    isValid_p = reopen(fileName);
}

OldGBTBackendTable::~OldGBTBackendTable()
{
    clear_self();
}

void OldGBTBackendTable::init_ignoredFITS()
{
    // This is most of the FITS Reserved Names
    // some of these, such as TUNIT, convey information that
    // ultimately needs to propogate to the table, but this
    // must wait until we have a units class (hopefully a better
    // FITS class set will handle things like TZERO, TSCAL, etc, 
    // in a transparant fasion
    ignoredFITS.resize(30);
    ignoredFITS(0) = "BITPIX";
    ignoredFITS(1) = "BLOCKED";
    ignoredFITS(2) = "CDELT";
    ignoredFITS(3) = "CROTA";
    ignoredFITS(4) = "CRPIX";
    ignoredFITS(5) = "CRVAL";
    ignoredFITS(6) = "CTYPE";
    ignoredFITS(7) = "END";
    ignoredFITS(8) = "EXTEND";
    ignoredFITS(9) = "EXTLEVEL";
    ignoredFITS(10) = "EXTNAME";
    ignoredFITS(11) = "EXTVER";
    ignoredFITS(12) = "GCOUNT";
    ignoredFITS(13) = "GROUPS";
    ignoredFITS(14) = "NAXIS";
    ignoredFITS(15) = "PCOUNT";
    ignoredFITS(16) = "SIMPLE";
    ignoredFITS(17) = "TBCOL";
    ignoredFITS(18) = "TDIM";
    ignoredFITS(19) = "TDISP";
    ignoredFITS(20) = "TFIELDS";
    ignoredFITS(21) = "TFORM";
    ignoredFITS(22) = "TTYPE";
    ignoredFITS(23) = "THEAP";
    ignoredFITS(24) = "TNULL";
    ignoredFITS(25) = "TSCAL";
    ignoredFITS(26) = "TTYPE";
    ignoredFITS(27) = "TUNIT";
    ignoredFITS(28) = "TZERO";
    ignoredFITS(29) = "XTENSION";
}

Bool OldGBTBackendTable::reopen(const String &fileName)
{
    row_nr_p = -1;
    clear_self();

    name_p = fileName;

    // open the file and read the tables

    fitsIO_p = new FitsInput(fileName.chars(), FITS::Disk);
    AlwaysAssert(fitsIO_p, AipsError);
    if (fitsIO_p->err()) {
	cerr << "OldGBTBackendTable::reopen() - error in initial FitsInput"
	    << " construction " << endl;
	clear_self();
	return False;
    }

    // Digest the primary hdu, which must be first
    // it is annoying that this is templated and I can't get just the 
    // keywords, which is all I want
    TableRecord primaryKeys;
    switch (fitsIO_p->datatype()) {
    case FITS::BYTE:
    {
	PrimaryArray<unsigned char> primaryHDU(*fitsIO_p);
	primaryKeys = FITSTabular::keywordsFromHDU(primaryHDU);
    }
    break;
    case FITS::SHORT:
    {
	PrimaryArray<short> primaryHDU(*fitsIO_p);
	primaryKeys = FITSTabular::keywordsFromHDU(primaryHDU);
    }
    break;
    case FITS::LONG:
    {
	PrimaryArray<FitsLong> primaryHDU(*fitsIO_p);
	primaryKeys = FITSTabular::keywordsFromHDU(primaryHDU);
    }
    break;
    case FITS::FLOAT:
    {
	PrimaryArray<Float> primaryHDU(*fitsIO_p);
	primaryKeys = FITSTabular::keywordsFromHDU(primaryHDU);
    }
    break;
    case FITS::DOUBLE:
    {
	PrimaryArray<Double> primaryHDU(*fitsIO_p);
	primaryKeys = FITSTabular::keywordsFromHDU(primaryHDU);
    }
    break;
    default:
	// well, this should never happen, but ignore it for now
	;
    }

    // look for BinaryTableHDUs
    // The DATA table MUST be last
    Regex trailing(" *$"); // trailing blanks
    while (fitsIO_p->err() == FitsIO::OK && ! fitsIO_p->eof()) {
	if (fitsIO_p->hdutype() == FITS::BinaryTableHDU) {
	    BinaryTable *binTab = new BinaryTable(*fitsIO_p);
	    AlwaysAssert(binTab, AipsError);
	    // which table is this
	    String extname(binTab->extname());
	    extname = extname.before(trailing);
	    if (extname == "DATA") {
		data_table_p = binTab;
		// this must be the end
		break;
	    } else if (extname == "PHASE" || extname == "STATE") {
		phase_table_p = 
		    new Table(binTab->fullTable("",Table::Scratch));
		AlwaysAssert(phase_table_p, AipsError);
		delete binTab;
	    } else if (extname == "RECEIVER") {
		rec_table_p = 
		    new Table(binTab->fullTable("",Table::Scratch));
		AlwaysAssert(rec_table_p, AipsError);
		delete binTab;
	    } else {
		// we should NEVER get to here
		cerr << "OldGBTBackendTable::reopen() - unrecognized extention name: "
		    << extname << endl;
		delete binTab;
		clear_self();
		return False;
	    }
	} else {
	    // A skip is only required if the HDU is not created, i.e. if
	    // a non-binary table was encounted, should this throw an
	    // exception here since it is not expected?
	    fitsIO_p->skip_hdu();
	}
    }

    // All of the tables should be set by now, unless this is
    // holography backend data, so first find out what the
    // backend name is from the primary HDU keywords
    Bool isHolo = False;
    if (primaryKeys.isDefined("BACKEND") &&
	primaryKeys.asString("BACKEND") == "Holo") 
	isHolo = True;

    if (!(isHolo && data_table_p) && 
	(! data_table_p || ! phase_table_p || ! rec_table_p)) {
	cerr << "OldGBTBackedTable::reopen() - unable to construct one or "
	    << "more of the DATA, PHASE, or RECEIVER tables" << endl;
	clear_self();
	return False;
    }

    if (fitsIO_p->err()) {
	cerr << "OldGBTBackendTable::reopen() - " << 
	    "Error during read of FITS file" << endl;
	clear_self();
	return False;
    }

    // ok, time to construct center_phase_time_p;
    // cycle offset, zero by default
    Double cycle_offset = 0.0;
    // if these keywords exist, use them
    if (primaryKeys.isDefined("CYCLETIM") &&
	primaryKeys.isDefined("CYCLES")) {
	cycle_offset = (primaryKeys.asdouble("CYCLES") - 1) *
	               primaryKeys.asdouble("CYCLETIM") / 2.0;
    }
    duration_p = 0.0;
    duration_offset_p = 0.0;
    if (primaryKeys.isDefined("DURATION"))
	duration_p = primaryKeys.asdouble("DURATION");
    duration_p /= secPerDay;
    if (isHolo) {
	center_phase_time_p.resize(1,1);
	center_phase_time_p = cycle_offset;
    } else {
	uInt nphases = phase_table_p->nrow();
	uInt nrcvrs = rec_table_p->nrow();
	center_phase_time_p.resize(nphases, nrcvrs);
	// do things differently if phase columns are arrays vs scalars
	// just check PHASETIM
	if (phase_table_p->tableDesc().columnDesc("PHASETIM").isArray()) {
	    // Double needs to be delt with separatly from Float, unfortunately,
	    // SP data uses Float and DCR uses Double.  This also assumes that
	    // the types are the same for PHASETIM and BLANKTIM
	    if (phase_table_p->tableDesc().columnDesc("PHASETIM").dataType() ==
		TpArrayDouble ||
		phase_table_p->tableDesc().columnDesc("PHASETIM").dataType() ==
		TpDouble) {
		ArrayColumn<Double> phasetim(*phase_table_p, "PHASETIM");
		ArrayColumn<Double> blanktim(*phase_table_p, "BLANKTIM");
		Vector<Double> phase_offset(nrcvrs);
		Vector<Double> this_phase, this_offset;
		phase_offset = 0;
		for (uInt i=0;i<nphases;i++) {
		    this_phase = phasetim(i);
		    this_offset = blanktim(i) + this_phase / 2.0;
		    center_phase_time_p.row(i) = 
			phase_offset + cycle_offset + this_offset;
		    phase_offset += this_phase;
		}
	    } else if (phase_table_p->tableDesc().columnDesc("PHASETIM").dataType() ==
		       TpArrayFloat||
		       phase_table_p->tableDesc().columnDesc("PHASETIM").dataType() ==
		       TpFloat) {
		ArrayColumn<Float> phasetim(*phase_table_p, "PHASETIM");
		ArrayColumn<Float> blanktim(*phase_table_p, "BLANKTIM");
		Vector<Float> fphase, fblank;
		Vector<Double> phase_offset(nrcvrs);
		Vector<Double> this_phase, this_offset, dblank;
		phase_offset = 0;
		for (uInt i=0;i<nphases;i++) {
		    fphase = phasetim(i);
		    fblank = blanktim(i);
		    this_phase.resize(fphase.nelements());
		    dblank.resize(fblank.nelements());
		    convertArray(this_phase, fphase);
		    convertArray(dblank, fblank);
		    this_offset = dblank + this_phase / 2.0;
		    center_phase_time_p.row(i) = 
			phase_offset + cycle_offset + this_offset;
		    phase_offset += this_phase;
		}
	    } else {
		cerr << "OldGBTBackendTable::reopen() - unrecognized type for "
		    "PHASETIM column" << endl;
		clear_self();
		return False;
	    }
	} else {
	    TableColumn phasetim(*phase_table_p, "PHASETIM");
	    TableColumn blanktim(*phase_table_p, "BLANKTIM");
	    Double phase_offset;
	    Double this_phase, this_offset;
	    phase_offset = 0;
	    for (uInt i=0;i<nphases;i++) {
		this_phase = phasetim.asdouble(i);
		this_offset = blanktim.asdouble(i) + this_phase / 2.0;
		center_phase_time_p.row(i) = 
		    phase_offset + cycle_offset + this_offset;
		phase_offset += this_phase;
	    }
	}
    }
    // and convert the whole damn thing to days
    center_phase_time_p /= secPerDay;
	      
    // find out the shape of the DATA column
    // holography backend has no data column
    if (isHolo) {
	dataCol_p = -1;
	datalength_p = 0;
	max_row_nr_p = data_table_p->nrows();

    } else {
	for (dataCol_p = 0;dataCol_p<data_table_p->ncols();dataCol_p++) {
	    String colname(data_table_p->ttype(dataCol_p));
	    colname = colname.before(trailing);
	    if (colname == String("DATA")) break;
	}
	if (dataCol_p > data_table_p->ncols()) {
	    cerr << "OldGBTBackendTable::reopen() no DATA column found." << endl;
	    clear_self();
	    return False;
	}
	// it would be nice if I didn't need to do this
	// decode tdim
	String tdim(data_table_p->tdim(dataCol_p));
	// sanity check, must have 1 left paren and 1 right paren
	AlwaysAssert((tdim.freq('(') == 1 && tdim.freq(')') == 1), AipsError);
	// number of commas implies number of axes
	IPosition data_shape_p(tdim.freq(',')+1);
	// skip to after the first parenthesis
	Int axis = 0;
	tdim = tdim.after('(');
	int value;
	for (uInt i=0;i<data_shape_p.nelements()-1;i++) {
	    // put it into value
	    sscanf(tdim.chars(),"%i", &value);
	    data_shape_p(axis) = value;
	    axis++;
	    // skip after the next comma
	    tdim = tdim.after(',');
	}
	// and finally the last axis
	sscanf(tdim.chars(),"%i", &value);
	data_shape_p(axis) = value;
    
	// 2D or 3D data
	Bool is2D = (data_shape_p.nelements() == 2);
	Int phaseAxis, recAxis;
	phaseAxis = recAxis = -1;

	for (uInt i=0;i<data_shape_p.nelements();i++) {
	    FITS::ReservedName tmp = FITS::CTYPE;
	    const FitsKeyword* ctype_p = data_table_p->kw(tmp,i+1);
	    if (ctype_p) {
		String extname = ctype_p->asString();
		extname = extname.before(trailing);
		// PHASE or RECEIVER 
		if (extname == "PHASE" || extname == "STATE") phaseAxis = i;
		if (extname == "RECEIVER") recAxis = i;
	    }
	}
	// there's the little problem of spectral processor FITS files which
	// currently don't use the CTYPE convention above, try and guess by matching
	// the row number to the size of an axis in data_shape_p
	// only do this if phaseAxis or recAxis are not set
	Bool isSpecProc = False;
	if (phaseAxis < 0 || recAxis < 0) {
	    isSpecProc = True;
	    // only need to look at last 2 axes
	    for (uInt i=data_shape_p.nelements()-2;i<data_shape_p.nelements();i++) {
		if (Int(phase_table_p->nrow()) == data_shape_p(i) && phaseAxis < 0)
		    phaseAxis = i;
		if (Int(rec_table_p->nrow()) == data_shape_p(i) && recAxis < 0)
		    recAxis = i;
	    }
	}
	if (!is2D) {
	    // verify that neither rec or phase axis is 0
	    if (recAxis == 0 || phaseAxis == 0) {
		cerr << "OldGBTBackendTable::reopen() - "
		    "first axis is not a frequency-like axis" << endl;
		clear_self();
		return False;
	    }
	}
	if (recAxis > phaseAxis) {
	    rec_offset_p = 1;
	    phase_offset_p = phase_table_p->nrow();
	} else if (phaseAxis > recAxis) {
	    phase_offset_p = 1;
	    rec_offset_p = rec_table_p->nrow();
	} else if (isSpecProc) {
	    // use "standard" order - i.e. a blind guess
	    // phase axis before receiver axis
	    phase_offset_p = 1;
	    rec_offset_p = rec_table_p->nrow();
	} else {
	    // this must never happen
	    cerr << "OldGBTBackendTable::reopen() - "
		"RECEIVER and PHASE axes appear to be the same" << endl;
	    clear_self();
	    return False;
	}
	
	// the default data length is 1
	datalength_p = 1;
	if (!is2D) datalength_p = data_shape_p(0);

	max_row_nr_p = data_table_p->nrows() * 
	    (phase_table_p->nrow() * rec_table_p->nrow());
    }

    // the keywords_p is just a merged keyword set from everything, skipping duplicates
    // description_p starts out as a merged description from the tables
    keywords_p = primaryKeys;
    keywords_p.merge(keywordsFromHDU(*data_table_p), 
				  TableRecord::SkipDuplicates);
    // first, add the Time column
    uInt timeField = (description_p.addField("Time", TpDouble)-1);
    // add Time units 
    units_p.define("Time","s");
    // then add the phase and receceiver ID fields
    uInt recIDField = 0;
    uInt phaseIDField = 0;
    if (!isHolo) {
	recIDField = (description_p.addField("RECEIVER_ID", TpInt)-1);
	phaseIDField = (description_p.addField("PHASE_ID", TpInt)-1);
    }

    // and begin to merge in the rest
    description_p.merge(descriptionFromHDU(*data_table_p));
    units_p.merge(unitsFromHDU(*data_table_p), TableRecord::SkipDuplicates);
    // get any display formats and nulls from the data table
    disps_p = displayFormatsFromHDU(*data_table_p);
    nulls_p = nullsFromHDU(*data_table_p);
    if (!isHolo) {
	// remove the DATA field and add it in by hand with the appropriate shape
	dataField_p = description_p.fieldNumber("DATA");
	if (dataField_p < 0) {
	    cerr << "OldGBTBackendTable::reopen() - no DATA field found in "
		 << "description" << endl;
	    clear_self();
	    return False;
	}
	if (datalength_p <= 1) {
	    // need to convert to equivalent non-array type (if already
	    // scalar, this function does nothing
	    dataField_p = arrayFieldToScalar(description_p,dataField_p);
	} else {
	    // simply need to ensure that the current field 
	    // indicates an indirect array
	    // currently, indirect arrays are not really supported
	    // but you can fake it out with a zero length array
	    IPosition dshape(1, 0);
	    DataType dtype = description_p.type(dataField_p);
	    description_p.removeField(dataField_p);
	    description_p.addField("DATA", dtype, dshape);
	    dataField_p = description_p.fieldNumber("DATA");
	}
	keywords_p.merge(phase_table_p->tableDesc().keywordSet(),
			 TableRecord::SkipDuplicates);
	RecordDesc phaseDesc(descriptionFromTableDesc(phase_table_p->tableDesc()));
	RecordDesc newPhaseDesc;
	// deal with columns in phase_table_p which might be indexed by recevier number
	// look for array columns where the number of elements equals the number
	// of receivers
	rcvrIndexed_p.resize(phaseDesc.nfields());
	rcvrIndexed_p.set(False);
	// iterate throught the RecordDesc looking for vector fields with the number
	// of elements equal to nrcvrs
	uInt nrcvrs = rec_table_p->nrow();
	for (uInt i=0;i<phaseDesc.nfields();i++) {
	    // first, merge it in 
	    uInt newField = (newPhaseDesc.mergeField(phaseDesc, i) - 1);
	    if (phaseDesc.isArray(i)) {
		IPosition shape(phaseDesc.shape(i));
		if (shape.nelements() == 1) {
		    if (shape(0) == Int(nrcvrs)) {
			// found one
			rcvrIndexed_p[i] = True;
			// convert it into a simple scalar
			arrayFieldToScalar(newPhaseDesc, newField);
		    }
		}
	    }
	}
	// finally merge it into the main description
	description_p.merge(newPhaseDesc, TableRecord::SkipDuplicates);
		    
	keywords_p.merge(rec_table_p->tableDesc().keywordSet(),
			 TableRecord::SkipDuplicates);
	description_p.merge(descriptionFromTableDesc(rec_table_p->tableDesc()),
			    TableRecord::SkipDuplicates);
    }
    
    // this is a convenient point to see if UTCSTART is a column
    // if it exists in description_p, then it must be an actual column
    // otherwise it must be only a keyword
    utcIsColumn_p = (description_p.fieldNumber("UTCSTART") >= 0);

    // now, add the keywords that should not be ignored
    description_p.merge(descriptionFromOKKeys(keywords_p), 
			TableRecord::SkipDuplicates);

    // Finally, remove fields that end in -comment - these are the
    // comments for keywords as implemented in BinTable using Record,
    // which does not allow comments per field
    Vector<String> comFields(description_p.nfields());
    uInt maxComFields = 0;
    for (uInt i=0;i<description_p.nfields();i++) {
        if (description_p.name(i).matches("-comment",
					  description_p.name(i).length()-8))
	    comFields(maxComFields++) = description_p.name(i);
    }
    for (uInt i=0;i<maxComFields;i++) 
      description_p.removeField(description_p.fieldNumber(comFields(i)));
	    
    // TIME related stuff
    dmjdField_p = -1;
    if (isHolo) {
	// its all just in the DMJD field
	dmjdField_p = description_p.fieldNumber("DMJD");
	if (dmjdField_p < 0) {
	    cerr << "OldGBTBackendTable::reopen() - DMJD not found" << endl;
	    clear_self();
	    return False;
	}
    } else {
	// find utcstart and utdate in description
	utdateField_p = description_p.fieldNumber("UTDATE");
	if (utdateField_p < 0) {
	    cerr << "OldGBTBackendTable::reopen() - UTDATE not found" << endl;
	    clear_self();
	    return False;
	}
	utcstartField_p = description_p.fieldNumber("UTCSTART");
	if (utcstartField_p< 0) {
	    cerr << "OldGBTBackendTable::reopen() - UTCSTART not found" << endl;
	    clear_self();
	    return False;
	}
	// assume seconds, check for radians
	ut_factor_p = 1.0;
	// get the UTCSTART keyword from the data table
	const FitsKeyword *utckw = data_table_p->kw("UTCSTART");
	if (utckw) {
	    String comm(utckw->comm(), utckw->commlen());
	    if (comm.contains("radians")) {
		// assume that there are 2pi radians per day
		ut_factor_p = secPerDay / (2.0 * C::pi);
	    }
	} else {
	    cerr << "OldGBTBackendTable::reopen() - no UTCSTART keyword found" << endl;
	    clear_self();
	    return False;
	}
    }
    
    // Now, the mappings to field number
    // First, set up a mask so we can know when something has been mapped
    Block<Bool> mapped(description_p.nfields());
    mapped.set(False);

    // data, time, receiver_id, and phase_id are already mapped
    mapped[description_p.fieldNumber("Time")] = True;
    if (!isHolo) {
	mapped[description_p.fieldNumber("DATA")] = True;
	mapped[description_p.fieldNumber("RECEIVER_ID")] = True;
	mapped[description_p.fieldNumber("PHASE_ID")] = True;
    }

    // first, the data table
    data_map_p.resize(data_table_p->ncols());
    data_map_p.set(-1);
    for (uInt i=0; Int(i)<data_table_p->ncols(); i++) {
	String fieldName(data_table_p->ttype(i));
	fieldName = fieldName.before(trailing);
	Int fieldnr = description_p.fieldNumber(fieldName);
	if (fieldnr >= 0 && !mapped[fieldnr]) {
	    data_map_p[i] = fieldnr;
	    mapped[fieldnr] = True;
	} 
    }

    // then the phase table
    if (!isHolo) {
	Vector<String> colNames = phase_table_p->tableDesc().columnNames();
	phase_map_p.resize(colNames.nelements());
	phase_map_p.set(-1);
	for (uInt j=0; j< colNames.nelements();j++) {
	    Int fieldnr = description_p.fieldNumber(colNames(j));
	    if (fieldnr >= 0 && !mapped[fieldnr]) {
		phase_map_p[j] = fieldnr;
		mapped[fieldnr] = True;
	    } 
	}

	// and finally the receiver table
	colNames.resize(0);
	colNames = rec_table_p->tableDesc().columnNames();
	rec_map_p.resize(colNames.nelements());
	rec_map_p.set(-1);
	for (uInt j=0; j< colNames.nelements();j++) {
	    Int fieldnr = description_p.fieldNumber(colNames(j));
	    if (fieldnr >= 0 && !mapped[fieldnr]) {
		rec_map_p[j] = fieldnr;
		mapped[fieldnr] = True;
	    } 
	}
    }

    // finally, set up the row
    row_p.restructure(description_p);

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
	    cerr << "OldGBTBackendTable::reopen() - unknown field type : " 
		<< description_p.type(i) << " on field " << i << endl;
	    clear_self();
	    return False;
	}
	AlwaysAssert(row_fields_p[i] != 0, AipsError);
    }

    // copy over the keywords if the name matches and it is unmapped and scalar
    for (uInt fieldnr=0;fieldnr<description_p.nfields();fieldnr++) {
	String keyname(description_p.name(fieldnr));
	if (! mapped[fieldnr] && 
	    description_p.isScalar(fieldnr)) {
	    // switch on type, ugh!
	    // we don't need to worry about it being an array type
	    // or uChar
	    switch (description_p.type(fieldnr)) {
	    case TpBool:
	    {
		RecordFieldPtr<Bool> &rowRef = 
		    *((RecordFieldPtr<Bool> *)row_fields_p[fieldnr]);
		*rowRef = keywords_p.asBool(keyname);
	    }
	    break;
	    case TpShort:
	    {
		RecordFieldPtr<Short> &rowRef = 
		    *((RecordFieldPtr<Short> *)row_fields_p[fieldnr]);
		*rowRef = keywords_p.asInt(keyname);
	    }
	    break;
	    case TpInt:
	    {
		RecordFieldPtr<Int> &rowRef = 
		    *((RecordFieldPtr<Int> *)row_fields_p[fieldnr]);
		*rowRef = keywords_p.asInt(keyname);
	    }
	    break;
	    case TpFloat:
	    {
		RecordFieldPtr<Float> &rowRef = 
		    *((RecordFieldPtr<Float> *)row_fields_p[fieldnr]);
		*rowRef = keywords_p.asfloat(keyname);
	    }
	    break;
	    case TpDouble:
	    {
		RecordFieldPtr<Double> &rowRef = 
		    *((RecordFieldPtr<Double> *)row_fields_p[fieldnr]);
		*rowRef = keywords_p.asdouble(keyname);
	    }
	    break;
	    case TpComplex:
	    {
		RecordFieldPtr<Complex> &rowRef = 
		    *((RecordFieldPtr<Complex> *)row_fields_p[fieldnr]);
		*rowRef = keywords_p.asComplex(keyname);
	    }
	    break;
	    case TpDComplex:
	    {
		RecordFieldPtr<DComplex> &rowRef = 
		    *((RecordFieldPtr<DComplex> *)row_fields_p[fieldnr]);
		*rowRef = keywords_p.asDComplex(keyname);
	    }
	    break;
	    case TpString:
	    {
		RecordFieldPtr<String> &rowRef = 
		    *((RecordFieldPtr<String> *)row_fields_p[fieldnr]);
		*rowRef = keywords_p.asString(keyname);
	    }
	    break;
	    default:
		cerr << "OldGBTBackendTable::reopen() - unknown data type" << endl;
		clear_self();
		return False;
	    }
	}
    }

    // and Time and ID fields
    time_p = new RecordFieldPtr<Double>(row_p, timeField);
    if (!isHolo) {
	phase_id_p = new RecordFieldPtr<Int>(row_p, phaseIDField);
	rec_id_p = new RecordFieldPtr<Int>(row_p, recIDField);
	if (! phase_id_p || ! rec_id_p) {
	    throw(AllocError("OldGBTBackendTable::reopen() - could not allocate " 
			     "a RecordFieldPtr", 1));
	}
	// initialize the id rows for appropriate use
	*(*rec_id_p) = -1;
	*(*phase_id_p) = 0;
    } else {
	if (!time_p) {
	    throw(AllocError("OldGBTBackendTable::reopen() - could not allocate " 
			     "a RecordFieldPtr", 1));
	}
    }
	
    fill_row();

    return True;
}

void OldGBTBackendTable::fill_row()
{
    row_nr_p++;
    // simply return if this row is past the end of the table
    if (pastEnd()) return;

    uInt phaseNr = 0;
    uInt recNr = 0;
    // Holo backend doesn't have rec_table_p, everything else currently does
    if (rec_table_p) {
	(*(*rec_id_p))++;
	if (*(*rec_id_p) >= Int(rec_table_p->nrow())) {
	    *(*rec_id_p) = 0;
	    (*(*phase_id_p))++;
	}
	if (*(*phase_id_p) >= Int(phase_table_p->nrow())) {
	    *(*phase_id_p) = 0;
	}

	phaseNr = *(*phase_id_p);
	recNr = *(*rec_id_p);
    }
    // if rec and phase id numbers are both zero, we need to read a new 
    // FITS data row except for the first row, which has already been read
    if (phaseNr == 0 && recNr == 0 && row_nr_p != 0) {
	// Use the native FITS classes
	data_table_p->read(1);
	duration_offset_p += duration_p;
    }

    // And now fill everything into the Row object.
    // First the data_table fields
    for (uInt i=0; i < data_map_p.nelements(); i++) {
	Int fieldnr = data_map_p[i];
	if (fieldnr >= 0) {
	    putField(row_fields_p[fieldnr], data_table_p->field(i),
		     description_p.type(fieldnr));
	}
    }

    if (rec_table_p) {
	// and the supporting tables, first PHASE
	for (uInt j=0; j < phase_map_p.nelements(); j++) {
	    ROTableColumn tabCol(*phase_table_p, j);
	    Int fieldnr = phase_map_p[j];
	    if (fieldnr >= 0) {
		// if indexed on receiver, do things slightly differently
		if (rcvrIndexed_p[j]) {
		    putColumn(row_fields_p[fieldnr], tabCol, phaseNr,
			      description_p.type(fieldnr), recNr);
		} else {
		    putColumn(row_fields_p[fieldnr], tabCol, phaseNr,
			      description_p.type(fieldnr));
		}
	    }
	}
	// Then RECEIVER
	for (uInt j=0; j < rec_map_p.nelements(); j++) {
	    ROTableColumn tabCol(*rec_table_p, j);
	    Int fieldnr = rec_map_p[j];
	    if (fieldnr >= 0) {
		putColumn(row_fields_p[fieldnr], tabCol, recNr,
			  description_p.type(fieldnr));
	    }
	}
	// The data array
	uInt offset = datalength_p * 
	    (phaseNr * rec_offset_p + recNr * phase_offset_p);
	putField(row_fields_p[dataField_p], data_table_p->field(dataCol_p), 
		 description_p.type(dataField_p), datalength_p, offset);
    }

    // finally, get the time column correct
    // Holo data uses the dmjdField_p explicitly
    if (dmjdField_p >= 0) {
	*(*time_p) = scalarFieldAsDouble(dmjdField_p);
    } else {
	// first, get the ut at the start of the scan correct
	// there is a bug in some FITS files where utcstart and utdate conspire
	// to produce a new time that is less than the previous one
	// this should check for that, I think.
	Double newStart = utdate() + utcstart()*ut_factor_p/secPerDay;
	if (newStart < oldStart_p) newStart = newStart + 1.0;
	oldStart_p = newStart;
	*(*time_p) = newStart + duration_offset_p;
	// now add the appropriate phase offset
	*(*time_p) += center_phase_time_p(phaseNr, recNr);
    }
}

Double OldGBTBackendTable::scalarFieldAsDouble(Int whichField) const
{
    if (whichField < 0) return 0;

    Double  value;
    switch (description_p.type(whichField)) {
    case TpShort:
	value = *(*((RecordFieldPtr<Short> *)row_fields_p[whichField]));
	break;
    case TpInt:
	value = *(*((RecordFieldPtr<Int> *)row_fields_p[whichField]));
	break;
    case TpFloat:
	value = *(*((RecordFieldPtr<Float> *)row_fields_p[whichField]));
	break;
    case TpDouble:
	value = *(*((RecordFieldPtr<Double> *)row_fields_p[whichField]));
	break;
    default:
	// should this throw an exception
	value = 0.0;
	break;
    }
    return value;
}

// What an ugly function! Simplify somehow!
void OldGBTBackendTable::putField(void *fieldPtr, FitsBase &base, DataType dtype,
			       Int nelements, uInt offset)
{
    // all these resizes here are probably expensive
    // they are only required for variable length arrays
    // also, this only does vectors, which is fine
    IPosition shape(1,nelements);

    if (nelements == -1) nelements = base.nelements();
    switch (base.fieldtype()) {
    case FITS::LOGICAL:
    {
	FitsField<FitsLogical> &fitsRef = 
	    (FitsField<FitsLogical> &)(base);
	if (dtype == TpBool) {
	    RecordFieldPtr<Bool> &rowRef =
		*((RecordFieldPtr<Bool> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayBool, AipsError);
	    RecordFieldPtr<Array<Bool> > &rowRef =
		*((RecordFieldPtr<Array<Bool> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    Bool *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::BIT:
    {
	FitsField<FitsBit> &fitsRef = 
	    (FitsField<FitsBit> &)(base);
	if (dtype == TpBool) {
	    RecordFieldPtr<Bool> &rowRef =
		*((RecordFieldPtr<Bool> *)fieldPtr);
	    *rowRef = (int(fitsRef(offset)));
	} else {
	    DebugAssert(dtype == TpArrayBool, AipsError);
	    RecordFieldPtr<Array<Bool> > &rowRef =
		*((RecordFieldPtr<Array<Bool> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    Bool *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = (int(fitsRef(n+offset)));
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::CHAR:
    {
	FitsField<char> &fitsRef = 
	    (FitsField<char> &)(base);
	if (dtype == TpUChar) {
	    RecordFieldPtr<uChar> &rowRef =
		*((RecordFieldPtr<uChar> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayUChar, AipsError);
	    RecordFieldPtr<Array<uChar> > &rowRef =
		*((RecordFieldPtr<Array<uChar> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    uChar *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::BYTE:
    {
	FitsField<unsigned char> &fitsRef = 
	    (FitsField<unsigned char> &)(base);
	if (dtype == TpUChar) {
	    RecordFieldPtr<uChar> &rowRef =
		*((RecordFieldPtr<uChar> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayUChar, AipsError);
	    RecordFieldPtr<Array<uChar> > &rowRef =
		*((RecordFieldPtr<Array<uChar> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    uChar *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::SHORT:
    {
	FitsField<short> &fitsRef = 
	    (FitsField<short> &)(base);
	if (dtype == TpShort) {
	    RecordFieldPtr<Short> &rowRef =
		*((RecordFieldPtr<Short> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayShort, AipsError);
	    RecordFieldPtr<Array<Short> > &rowRef =
		*((RecordFieldPtr<Array<Short> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    Short *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::LONG:
    {
	FitsField<FitsLong> &fitsRef = 
	    (FitsField<FitsLong> &)(base);
	if (dtype == TpInt) {
	    RecordFieldPtr<Int> &rowRef =
		*((RecordFieldPtr<Int> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayInt, AipsError);
	    RecordFieldPtr<Array<Int> > &rowRef =
		*((RecordFieldPtr<Array<Int> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    Int *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::FLOAT:
    {
	FitsField<float> &fitsRef = 
	    (FitsField<float> &)(base);
	if (dtype == TpFloat) {
	    RecordFieldPtr<Float> &rowRef =
		*((RecordFieldPtr<Float> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayFloat, AipsError);
	    RecordFieldPtr<Array<Float> > &rowRef =
		*((RecordFieldPtr<Array<Float> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    Float *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::DOUBLE:
    {
	FitsField<double> &fitsRef = 
	    (FitsField<double> &)(base);
	if (dtype == TpDouble) {
	    RecordFieldPtr<Double> &rowRef =
		*((RecordFieldPtr<Double> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayDouble, AipsError);
	    RecordFieldPtr<Array<Double> > &rowRef =
		*((RecordFieldPtr<Array<Double> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    Double *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::COMPLEX:
    {
	FitsField<Complex> &fitsRef = 
	    (FitsField<Complex> &)(base);
	if (dtype == TpComplex) {
	    RecordFieldPtr<Complex> &rowRef =
		*((RecordFieldPtr<Complex> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayComplex, AipsError);
	    RecordFieldPtr<Array<Complex> > &rowRef =
		*((RecordFieldPtr<Array<Complex> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    Complex *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    case FITS::DCOMPLEX:
    {
	FitsField<DComplex> &fitsRef = 
	    (FitsField<DComplex> &)(base);
	if (dtype == TpDComplex) {
	    RecordFieldPtr<DComplex> &rowRef =
		*((RecordFieldPtr<DComplex> *)fieldPtr);
	    *rowRef = fitsRef(offset);
	} else {
	    DebugAssert(dtype == TpArrayDComplex, AipsError);
	    RecordFieldPtr<Array<DComplex> > &rowRef =
		*((RecordFieldPtr<Array<DComplex> > *)fieldPtr);
	    (*rowRef).resize(shape);
	    Bool deleteIt;
	    DComplex *data = (*rowRef).getStorage(deleteIt);
	    Int n = nelements;
	    while (n) {
		n--;
		data[n] = fitsRef(n+offset);
	    }
	    (*rowRef).putStorage(data, deleteIt);
	}
    }
    break;
    default:
	throw(AipsError("OldGBTBackendTable::fill_row() - unknown data type"));
    }
}

// What an ugly function! Simplify somehow!
// index is only used if dtype is scalar by tabCol is an array
// A vector is assumed in that case
// this assumes that the table column is the same basic type as the field
void OldGBTBackendTable::putColumn(void *fieldPtr, const ROTableColumn& tabCol,
				uInt tabRow, DataType dtype, uInt index)
{
    Bool isArray = tabCol.columnDesc().isArray();
    switch (dtype) {
    case TpBool:
    {
	RecordFieldPtr<Bool> &rowRef = *((RecordFieldPtr<Bool> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<Bool> colRef(tabCol);
	    Vector<Bool> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    case TpArrayBool:
    {
	RecordFieldPtr<Array<Bool> > &rowRef = 
	    *((RecordFieldPtr<Array<Bool> > *)fieldPtr);
	ROArrayColumn<Bool> colRef(tabCol);
	*rowRef = colRef(tabRow);
    }
    break;
    case TpUChar:
    {
	RecordFieldPtr<uChar> &rowRef = *((RecordFieldPtr<uChar> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<uChar> colRef(tabCol);
	    Vector<uChar> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    case TpArrayUChar:
    {
	RecordFieldPtr<Array<uChar> > &rowRef = 
	    *((RecordFieldPtr<Array<uChar> > *)fieldPtr);
	ROArrayColumn<uChar> colRef(tabCol);
	*rowRef = colRef(tabRow);
    }
    break;
    case TpShort:
    {
	RecordFieldPtr<Short> &rowRef = *((RecordFieldPtr<Short> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<Short> colRef(tabCol);
	    Vector<Short> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    case TpArrayShort:
    {
	RecordFieldPtr<Array<Short> > &rowRef = 
	    *((RecordFieldPtr<Array<Short> > *)fieldPtr);
	ROArrayColumn<Short> colRef(tabCol);
	*rowRef = colRef(tabRow);
    }
    break;
    case TpInt:
    {
	RecordFieldPtr<Int> &rowRef = *((RecordFieldPtr<Int> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<Int> colRef(tabCol);
	    Vector<Int> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    case TpArrayInt:
    {
	RecordFieldPtr<Array<Int> > &rowRef = 
	    *((RecordFieldPtr<Array<Int> > *)fieldPtr);
	ROArrayColumn<Int> colRef(tabCol);
	*rowRef = colRef(tabRow);
    }
    break;
    case TpFloat:
    {
	RecordFieldPtr<Float> &rowRef = *((RecordFieldPtr<Float> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<Float> colRef(tabCol);
	    Vector<Float> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    case TpArrayFloat:
    {
	RecordFieldPtr<Array<Float> > &rowRef = 
	    *((RecordFieldPtr<Array<Float> > *)fieldPtr);
	ROArrayColumn<Float> colRef(tabCol);
	*rowRef = colRef(tabRow);
    }
    break;
    case TpDouble:
    {
	RecordFieldPtr<Double> &rowRef = *((RecordFieldPtr<Double> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<Double> colRef(tabCol);
	    Vector<Double> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    case TpArrayDouble:
    {
	RecordFieldPtr<Array<Double> > &rowRef = 
	    *((RecordFieldPtr<Array<Double> > *)fieldPtr);
	ROArrayColumn<Double> colRef(tabCol);
	*rowRef = colRef(tabRow);
    }
    break;
    case TpComplex:
    {
	RecordFieldPtr<Complex> &rowRef = *((RecordFieldPtr<Complex> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<Complex> colRef(tabCol);
	    Vector<Complex> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    case TpArrayComplex:
    {
	RecordFieldPtr<Array<Complex> > &rowRef = 
	    *((RecordFieldPtr<Array<Complex> > *)fieldPtr);
	ROArrayColumn<Complex> colRef(tabCol);
	*rowRef = colRef(tabRow);
    }
    break;
    case TpDComplex:
    {
	RecordFieldPtr<DComplex> &rowRef = *((RecordFieldPtr<DComplex> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<DComplex> colRef(tabCol);
	    Vector<DComplex> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    case TpArrayDComplex:
    {
	RecordFieldPtr<Array<DComplex> > &rowRef = 
	    *((RecordFieldPtr<Array<DComplex> > *)fieldPtr);
	ROArrayColumn<DComplex> colRef(tabCol);
	*rowRef = colRef(tabRow);
    }
    break;
    case TpString:
    {
	RecordFieldPtr<String> &rowRef = *((RecordFieldPtr<String> *)fieldPtr);
	if (isArray) {
	    ROArrayColumn<String> colRef(tabCol);
	    Vector<String> colVec;
	    colRef.get(tabRow, colVec, True);
	    *rowRef = colVec(index);
	} else {
	    tabCol.getScalar(tabRow, *rowRef);
	}
    }
    break;
    default:
	throw(AipsError("OldGBTBackendTable::fill_row() - unknown data type"));
    }
}

void OldGBTBackendTable::clear_self()
{
    row_nr_p = -1;

    delete data_table_p;
    data_table_p = 0;

    delete fitsIO_p;
    fitsIO_p = 0;

    delete phase_table_p;
    phase_table_p = 0;

    delete rec_table_p;
    rec_table_p = 0;

    delete time_p;
    time_p = 0;

    delete phase_id_p;
    phase_id_p = 0;

    delete rec_id_p;
    rec_id_p = 0;

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
	    throw(AipsError("OldGBTBackendTable::clear_self() - unknown field type"));
	}
	row_fields_p[i] = 0;
    }
    row_fields_p.resize(0);
    RecordDesc tmp;
    description_p = tmp;
    row_p.restructure(tmp);
}
