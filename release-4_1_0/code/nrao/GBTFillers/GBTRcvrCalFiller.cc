//# GBTRcvrCalFiller.cc:  A filler for GBT receiver TCAL tables.
//# Copyright (C) 2001,2002,2003
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

//# Include

#include <nrao/GBTFillers/GBTRcvrCalFiller.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>
#include <fits/FITS/FITSTable.h>
#include <casa/OS/File.h>
#include <casa/OS/RegularFile.h>
#include <scimath/Functionals/Interpolate1D.h>
#include <scimath/Functionals/ScalarSampledFunctional.h>
#include <casa/Quanta/Unit.h>
#include <casa/Utilities/GenSort.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>

GBTRcvrCalFiller::GBTRcvrCalFiller(Table &parent)
{
    // does one exist or should one be created
    String kwName = "NRAO_GBT_RCVRCAL";
    Int whichField = parent.keywordSet().fieldNumber(kwName);
    if (whichField >= 0 && parent.keywordSet().type(whichField) == TpTable) {
	// one exists, attach to that
	itsTable = Table(parent.rwKeywordSet().asTable(kwName));
    } else {
	// otherwise, create one and attach it to this table
	if (whichField >= 0) parent.rwKeywordSet().removeField(whichField);
	String tableName = parent.tableName() + "/" + kwName;

	TableDesc td;
	td.addColumn(ScalarColumnDesc<String>("FILENAME"));
	td.addColumn(ScalarColumnDesc<String>("RECEIVER"));
	td.addColumn(ScalarColumnDesc<String>("DATE-OBS"));
	td.addColumn(ScalarColumnDesc<Float>("MIN_FREQ"));
	// MIN_FREQ has units of Hz
	TableQuantumDesc lofreqd(td,"MIN_FREQ",Unit("Hz"));
	lofreqd.write(td);
	td.addColumn(ScalarColumnDesc<Float>("MAX_FREQ"));
	// MAX_FREQ has units of Hz
	TableQuantumDesc hifreqd(td,"MAX_FREQ",Unit("Hz"));
	hifreqd.write(td);
	td.addColumn(ScalarColumnDesc<Int>("NBEAM"));
	td.addColumn(ScalarColumnDesc<Float>("TAUZENIT"));
	td.addColumn(ScalarColumnDesc<Float>("ETAL"));
	td.addColumn(ScalarColumnDesc<Float>("APEREFF"));
	td.addColumn(ScalarColumnDesc<Float>("BEAMEFF"));

	td.addColumn(ScalarColumnDesc<String>("TESTDATE"));
	td.addColumn(ScalarColumnDesc<String>("RECEPTOR"));
	td.addColumn(ScalarColumnDesc<Int>("FEED"));
	td.addColumn(ScalarColumnDesc<String>("POLARIZE"));
	td.addColumn(ScalarColumnDesc<Float>("BANDWDTH"));
	// BANDWDTH has units of Hz
	TableQuantumDesc bwd(td,"BANDWDTH",Unit("Hz"));
	bwd.write(td);
	td.addColumn(ScalarColumnDesc<String>("ENGINEER"));
	td.addColumn(ScalarColumnDesc<String>("TECH"));
	td.addColumn(ArrayColumnDesc<Float>("FREQUENCY",1));
	// FREQUENCY has units of Hz	
	TableQuantumDesc freqd(td,"FREQUENCY",Unit("Hz"));
	freqd.write(td);
	td.addColumn(ArrayColumnDesc<Float>("RX_TEMP",1));
	// RX_TEMP has units of K
	TableQuantumDesc trxd(td,"RX_TEMP",Unit("K"));
	trxd.write(td);
	td.addColumn(ArrayColumnDesc<Float>("LOW_CAL_TEMP",1));
	// LOW_CAL_TEMP has units of K
	TableQuantumDesc tcallod(td,"LOW_CAL_TEMP",Unit("K"));
	tcallod.write(td);
	td.addColumn(ArrayColumnDesc<Float>("HIGH_CAL_TEMP",1));
	// HIGH_CAL_TEMP has units of K
	TableQuantumDesc tcalhid(td,"HIGH_CAL_TEMP",Unit("K"));
	tcalhid.write(td);

	SetupNewTable newtab(tableName, td, Table::New);
	IncrementalStMan incrStMan;
	StandardStMan stdStMan;
	newtab.bindAll(stdStMan);
	newtab.bindColumn("FILENAME", incrStMan);
	newtab.bindColumn("DATE-OBS", incrStMan);
	newtab.bindColumn("RECEIVER", incrStMan);
	newtab.bindColumn("MIN_FREQ", incrStMan);
	newtab.bindColumn("MAX_FREQ", incrStMan);
	newtab.bindColumn("NBEAM", incrStMan);
	newtab.bindColumn("TAUZENIT", incrStMan);
	newtab.bindColumn("ETAL", incrStMan);
	newtab.bindColumn("APEREFF", incrStMan);
	newtab.bindColumn("BEAMEFF", incrStMan);

	itsTable = Table(newtab, GBTBackendFiller::tableLock());
	parent.rwKeywordSet().defineTable(kwName, itsTable);
    }
    // and attach the columns
    fileNameCol.attach(itsTable, "FILENAME");
    receiver.attach(itsTable, "RECEIVER");
    dateObs.attach(itsTable, "DATE-OBS");
    minFreq.attach(itsTable, "MIN_FREQ");
    maxFreq.attach(itsTable, "MAX_FREQ");
    nBeam.attach(itsTable, "NBEAM");
    tauzenit.attach(itsTable, "TAUZENIT");
    etal.attach(itsTable, "ETAL");
    apereff.attach(itsTable, "APEREFF");
    beameff.attach(itsTable, "BEAMEFF");

    testDate.attach(itsTable, "TESTDATE");
    receptor.attach(itsTable, "RECEPTOR");
    feed.attach(itsTable, "FEED");
    polarize.attach(itsTable, "POLARIZE");
    bandwdth.attach(itsTable, "BANDWDTH");
    engineer.attach(itsTable, "ENGINEER");
    tech.attach(itsTable, "TECH");

    frequency.attach(itsTable, "FREQUENCY");
    trx.attach(itsTable, "RX_TEMP");
    tcalLo.attach(itsTable, "LOW_CAL_TEMP");
    tcalHi.attach(itsTable, "HIGH_CAL_TEMP");
}

Bool GBTRcvrCalFiller::fill(const String &fileName)
{
    // do we already know about this fileName
    if (fileName != itsLastFileName) {
	// have we ever seen this fileName
	if (itsTable.nrow() > 0) {
	    itsLastFilled = itsTable(itsTable.col("FILENAME") == fileName);
	    if (itsLastFilled.nrow() > 0) {
		itsLastFileName = fileName;
		return True;
	    }
	}
	// never seen this before
	// verify that the fileName exists and is readable
	File thefile(fileName);
	if (thefile.exists() && thefile.isReadable()) {
	    // don't bother if the file is empty
	    RegularFile newfile(fileName);
	    if (newfile.size() == 0) return False;
	    
	    // start at HDU == 1 and advance until the thing isn't valid
	    Bool ok = True;
	    Int whichHDU = 1;
	    while (ok) {
		FITSTable fitsTab(fileName, whichHDU);
		ok = fitsTab.isValid() && fitsTab.keywords().asString("EXTNAME") == "RX_CAL_INFO";
		if (ok) {
		    // add a row for this table
		    Int rownr = itsTable.nrow();
		    itsTable.addRow();
		    // if this is the first HDU, fill in the keywords
		    // from the primary HDU to this row
		    if (whichHDU == 1) {
			fileNameCol.putScalar(rownr, fileName);
			// be careful about this so that if things
			// change, as much as possible won't break
			putScalarString("RECEIVER",fitsTab.primaryKeywords(),
					rownr, "", receiver);
			// DATE-OBS may be spelled wrong
			putScalarString("DATE-OBS", fitsTab.primaryKeywords(),
					rownr, "", dateObs, "DATE_OBS");
			// MIN_FREQ was once LO_FREQ
			putScalarFloat("MIN_FREQ",fitsTab.primaryKeywords(),
				       rownr, 1.0, 0.0, minFreq, "LO_FREQ", 1.0e9);
			// MAX_FREQ was once HI_FREQ
			putScalarFloat("MAX_FREQ",fitsTab.primaryKeywords(),
				       rownr, 1.0, 0.0, maxFreq, "HI_FREQ", 1.0e9);
			putScalarInt("NBEAM",fitsTab.primaryKeywords(),
				     rownr, 0, nBeam);
			putScalarFloat("TAUZENIT", fitsTab.primaryKeywords(),
				       rownr, 1.0, 1.0, tauzenit);
			putScalarFloat("ETAL", fitsTab.primaryKeywords(),
				       rownr, 1.0, 1.0, etal);
			putScalarFloat("APEREFF", fitsTab.primaryKeywords(),
				       rownr, 1.0, 1.0, apereff);
			putScalarFloat("BEAMEFF", fitsTab.primaryKeywords(),
				       rownr, 1.0, 1.0, beameff);
		    }
		    // scalars from this HDU
		    putScalarString("TESTDATE",fitsTab.keywords(),
				    rownr, "", testDate);
		    // RECEPTOR was once called CHANNEL
		    putScalarString("RECEPTOR",fitsTab.keywords(),
				    rownr, "unknown", receptor, "CHANNEL");
		    // FEED was once BEAM
		    putScalarInt("FEED",fitsTab.keywords(),
				 rownr, -1, feed, "BEAM");
		    putScalarString("POLARIZE",fitsTab.keywords(),
				    rownr, "unknown", polarize);
		    // BANDWDTH in Hz was once FREQ_WID in MHz
		    // MHz -> Hz
		    putScalarFloat("BANDWDTH",fitsTab.keywords(),
				   rownr, 1.0, 0.0, bandwdth, "FREQ_WID", 1.0e6);
		    putScalarString("ENGINEER",fitsTab.keywords(),
				    rownr, "unknown", engineer);
		    putScalarString("TECH",fitsTab.keywords(),
				    rownr, "unknown", tech);
		    
		    // finally loop through the nrows, filling the vectors
		    // as we go, putting them in at the end
		    Int nrows = fitsTab.nrow();
		    Vector<Float> freqVec(nrows), trxVec(nrows), 
			tcalLoVec(nrows), tcalHiVec(nrows);
		    Int freqField, trxField, tcalLoField, tcalHiField;
		    freqField = fitsTab.currentRow().
			fieldNumber("FREQUENCY");
		    trxField = fitsTab.currentRow().fieldNumber("RX_TEMP");
		    tcalLoField = fitsTab.currentRow().
			fieldNumber("LOW_CAL_TEMP");
		    // LOW_CAL_TEMP was once LO_CAL_TEMP
		    if (tcalLoField < 0) {
			tcalLoField = fitsTab.currentRow().
			    fieldNumber("LO_CAL_TEMP");
		    }
		    tcalHiField = fitsTab.currentRow().
			fieldNumber("HIGH_CAL_TEMP");
		    // HIGH_CAL_TEMP was once HI_CAL_TEMP
		    if (tcalHiField < 0) {
			tcalHiField = fitsTab.currentRow().
			    fieldNumber("HI_CAL_TEMP");
		    }
		    if (freqField < 0) freqVec = 0;
		    if (trxField < 0) trxVec = 0;
		    if (tcalLoField < 0) tcalLoVec = 0;
		    if (tcalHiField < 0) tcalHiVec = 0;
		    for (Int i=0;i<nrows;i++) {
			if (freqField >= 0) freqVec(i) = fitsTab.currentRow().
						asFloat(freqField);
			if (trxField >= 0) trxVec(i) = fitsTab.currentRow().
					       asFloat(trxField);
			if (tcalLoField >= 0) 
			    tcalLoVec(i) = fitsTab.currentRow().
				asFloat(tcalLoField);
			if (tcalHiField >= 0) 
			    tcalHiVec(i) = fitsTab.currentRow().
				asFloat(tcalHiField);
			fitsTab.next();
		    }
		    // grab the units, see if conversion is necessary
		    if (freqField >= 0) {
			if (fitsTab.units().fieldNumber("FREQUENCY") < 0 ||
			    fitsTab.units().asString("FREQUENCY") != "Hz") {
			    // assumes GHz
			    freqVec *= Float(1.0e9);
			}
		    }
		    
		    // finally, sort them and eliminate any duplicates
		    Vector<uInt> indx;
		    // sort these, and remove duplicates in the process
		    uInt nfreq = freqVec.nelements();
		    if (GenSortIndirect<Float>::sort(indx, freqVec, Sort::Ascending, Sort::QuickSort | Sort::NoDuplicates) != nfreq) {
			// there were some duplicates
			LogIO os(LogOrigin("GBTRcvrCalFiller","interpolate"));
			os << LogIO::WARN << WHERE
			   << "There are some duplicate frequencies in the table of TCAL values for "
			   << fileName << " in HDU # " << whichHDU			   
			   << LogIO::POST;
			os << LogIO::WARN << WHERE
			   << "The duplicate values will be ignored."
			   << LogIO::POST;
		    }
		    Vector<Float> trxVecSort(indx.nelements()), 
			tcalLoVecSort(indx.nelements()), tcalHiVecSort(indx.nelements()),
			freqVecSort(indx.nelements());
		    for (uInt i=0;i<indx.nelements();i++) {
			uInt thisIndx = indx[i];
			freqVecSort[i] = freqVec[thisIndx];
			trxVecSort[i] = trxVec[thisIndx];
			tcalHiVecSort[i] = tcalHiVec[thisIndx];
			tcalLoVecSort[i] = tcalLoVec[thisIndx];
		    }

		    frequency.put(rownr, freqVecSort);
		    trx.put(rownr, trxVecSort);
		    tcalLo.put(rownr, tcalLoVecSort);
		    tcalHi.put(rownr, tcalHiVecSort);
		    
		    whichHDU++;

		    ok = !fitsTab.eof();
		}
	    }
	    // and do the selection on this file name
	    if (whichHDU != 1) {
		itsLastFileName = fileName;
		itsLastFilled = itsTable(itsTable.col("FILENAME") == fileName);
	    } // otherwise, nothing was ok and nothing was filled, leave as is
	}
    }
    return True;
}

void GBTRcvrCalFiller::putScalarString(const String &fieldName,
				       const TableRecord &rec,
				       Int rownr, const String &defval,
				       TableColumn &col,
				       const String &altName)
{
    
    Int fieldNr = rec.fieldNumber(fieldName);
    if (fieldNr < 0) {
	// try the alternate name
	fieldNr = rec.fieldNumber(altName);
    }
    if (fieldNr >= 0) {
	col.putScalar(rownr,rec.asString(fieldNr));
    } else {
	col.putScalar(rownr, defval);
    }
}

void GBTRcvrCalFiller::putScalarFloat(const String &fieldName,
				      const TableRecord &rec,
				      Int rownr,
				      Float scale,
				      Float defval,
				      TableColumn &col,
				      const String &altName,
				      Float altScale)
{
    Double thisScale = scale;
    Int fieldNr = rec.fieldNumber(fieldName);
    if (fieldNr < 0) {
	// try the alternate name
	fieldNr = rec.fieldNumber(altName);
	thisScale = altScale;
    }
    if (fieldNr >= 0) {
	col.putScalar(rownr,rec.asFloat(fieldNr)*thisScale);
    } else {
	col.putScalar(rownr, defval);
    }
}

void GBTRcvrCalFiller::putScalarInt(const String &fieldName,
				    const TableRecord &rec,
				    Int rownr,
				    Int defval,
				    TableColumn &col,
				    const String &altName)
{
    Int fieldNr = rec.fieldNumber(fieldName);
    if (fieldNr < 0) {
	// try the alternate name
	fieldNr = rec.fieldNumber(altName);
    }
    if (fieldNr >= 0) {
	col.putScalar(rownr,rec.asInt(fieldNr));
    } else {
	col.putScalar(rownr, defval);
    }
}

void GBTRcvrCalFiller::interpolate(const String &receiverName, 
				   const Vector<String> &feedName,
				   const Vector<String> &polarizations,
				   const Vector<Double> &frequencies,
				   String &testDate,
				   Matrix<Float> &tcal, 
				   Matrix<Float> &trx,
				   Bool useHighCal)
{
    // lookup into most recently filled rcvr cal info table
    // using receiver name
    Table tmp = lastFilled()(lastFilled().col("RECEIVER") == receiverName);
    testDate = "";
    if (tmp.nrow() > 0) {// find row with feed (rcptr) and polarization as indicated
	TableColumn polCol(tmp, "POLARIZE");
	TableColumn rcptrCol(tmp, "RECEPTOR");
	Bool testDateSet = False;
	for (uInt whichPol=0;whichPol<polarizations.nelements();whichPol++) {
	    for (uInt whichRow=0;whichRow<tmp.nrow();whichRow++) {
		if (rcptrCol.asString(whichRow) == feedName(whichPol) &&
		    polCol.asString(whichRow) == polarizations(whichPol)) {
		    String calTempColName = "LOW_CAL_TEMP";
		    if (useHighCal) calTempColName = "HIGH_CAL_TEMP";
		    // found a match, get the measured freq,
		    // *_CAL_TEMP and RX_TEMP values
		    ArrayColumn<Float> trxCol(tmp, "RX_TEMP");
		    ArrayColumn<Float> tcalCol(tmp, calTempColName);
		    ArrayColumn<Float> freqCol(tmp, "FREQUENCY");
		    Vector<Float> trxMeas, tcalMeas, freqMeas;
		    trxMeas = trxCol(whichRow);
		    tcalMeas = tcalCol(whichRow);
		    freqMeas = freqCol(whichRow);
		    // convert freq to Doubles
		    Vector<Double> dfreq(freqMeas.nelements());
		    convertArray(dfreq, freqMeas);
		    ScalarSampledFunctional<Double> dfreqFunc(dfreq);
		    ScalarSampledFunctional<Float> tcalFunc(tcalMeas);
		    ScalarSampledFunctional<Float> trxFunc(trxMeas);
		    // we know these are sorted
		    Interpolate1D<Double, Float> tcalInterp(dfreqFunc, tcalFunc, True);
		    Interpolate1D<Double, Float> trxInterp(dfreqFunc, trxFunc, True);
		    tcalInterp.setMethod(Interpolate1D<Float,Double>::linear);
		    Vector<Float> tcalVec(frequencies.nelements());
		    Vector<Float> trxVec(frequencies.nelements());
		    for (uInt i=0;i<frequencies.nelements();i++) {
			tcalVec[i] = tcalInterp(frequencies[i]);
			trxVec[i] = trxInterp(frequencies[i]);
		    }
		    tcal.row(whichPol) = tcalVec;
		    trx.row(whichPol) = trxVec;
		    if (!testDateSet) {
			testDateSet = True;
			TableColumn tdateCol(tmp,"TESTDATE");
			testDate= tdateCol.asString(whichRow);
		    }
		    break;
		}
	    }
	}
    } else {
	tcal = trx = 1.0;
    }
}
