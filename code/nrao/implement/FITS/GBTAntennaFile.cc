//# GBTAntennaFile.cc:  GBTAntennaFile digests the FITS file holding GBT Antenna info
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

//# Includes

#include <nrao/FITS/GBTAntennaFile.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <fits/FITS/FITSTable.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/Quantum.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRow.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <casa/Utilities/DataType.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>

GBTAntennaFile::GBTAntennaFile()
    : itsTable(0), itsBeamOffTable(0), itsSecondsPerDay(C::day/C::second),
      itsRowFromFeed(-1)
{ setDefaults();}

GBTAntennaFile::GBTAntennaFile(const String &antennaFile)
    : itsTable(0), itsBeamOffTable(0), itsSecondsPerDay(C::day/C::second),
      itsRowFromFeed(-1)
{
    attach(antennaFile);
}

GBTAntennaFile::GBTAntennaFile(const GBTAntennaFile &other)
    : itsTable(0), itsBeamOffTable(0), itsSecondsPerDay(C::day/C::second),
      itsRowFromFeed(-1)
{
    attach(other.file());
}

GBTAntennaFile::~GBTAntennaFile()
{
    delete itsTable;
    itsTable = 0;

    delete itsBeamOffTable;
    itsBeamOffTable = 0;
}

GBTAntennaFile &GBTAntennaFile::operator=(const GBTAntennaFile &other)
{
    if (this != &other) {
	attach(other.file());
    }
    return *this;
}

Bool GBTAntennaFile::attach(const String &antennaFile)
{
    LogIO os(LogOrigin("GBTAntennaFile", 
			"attach(const String &antennaFile)"));
    // initialize things with default values, just to be sure
    setDefaults();
	     
    // is the requested antennaFile non-empty, try and open it
    if (!antennaFile.empty()) {
	Int whichhdu=0;
	Bool more=True;
	while (more) {
	    whichhdu++;
	    try {
		FITSTable fitsTab(antennaFile, whichhdu);
		if (!fitsTab.isValid()) {
		    os << LogIO::WARN << WHERE
		       << "No valid FITS table found at the first HDU in apparent Antenna file : " << antennaFile
		       << "\nThis file will be ignored." << LogIO::POST;
		    return False;
		}
		// this seems to be a valid FITS table

		itsFileName = fitsTab.name();

		// extract things from the primaryKeywords
	
		setPrimaryKeys(fitsTab.primaryKeywords());

		// trckbeam if present
		if (primaryKeys().fieldNumber("TRCKBEAM") >= 0) {
		    itsTrckBeam = primaryKeys().asString("TRCKBEAM");
		    markAsUsed("TRCKBEAM");
		} else {
		    itsTrckBeam = "";
		}

		// telescope position
		Int latField, longField, elevField, sysField, typeField;
		latField = primaryKeys().fieldNumber("SITELAT");
		longField = primaryKeys().fieldNumber("SITELONG");
		elevField = primaryKeys().fieldNumber("SITEELEV");
		sysField = primaryKeys().fieldNumber("SITESYS");
		typeField = primaryKeys().fieldNumber("SITETYPE");
		// SITETYPE is currently ignored - I don't think our code cares
		if (typeField >= 0) markAsUsed("SITETYPE");
		// if things are GEODETIC or GEOCENTRIC
		if (latField >= 0 && longField >= 0 && elevField >= 0 ) {
		    markAsUsed("SITELAT");
		    markAsUsed("SITELONG");
		    markAsUsed("SITEELEV");
		    Vector<Double> siteLatLong(2);
		    siteLatLong[0] = primaryKeys().asDouble(longField);
		    siteLatLong[1] = primaryKeys().asDouble(latField);
		    // MS::ANTENNA::POSITION keeps things as IRTF by default.
		    // SITESYS holds the system information - NAD83, ITRF, WGS84
		    // If not set or unrecognized, default to NAD83 and assume that is
		    // the same as WGS84.
		    MPosition::Types posType = MPosition::WGS84;
		    if (sysField >= 0) {
			String posTypeString = primaryKeys().asString(sysField);
			if (posTypeString == "ITRF") posType = MPosition::ITRF;
			// everything else is WGS84
			if (posTypeString == "NAD83") {
			    // reverse sign of longitude 
			    siteLatLong[0] = -siteLatLong[0];
			}
			markAsUsed("SITESYS");
		    } else {
			// this must be NAD83, reverse sign of longitude
			siteLatLong[0] = -siteLatLong[0];
		    }
		    
		    Quantum<Vector<Double> > qSiteLatLong(siteLatLong, "deg");
		    Quantity siteelev(primaryKeys().asDouble(elevField), "m");
		    // convert WGS84 to IRTF here
		    if (posType == MPosition::WGS84) {
			itsPosition = MPosition::Convert(MPosition::WGS84, MPosition::ITRF)
			    (MPosition(siteelev, qSiteLatLong, MPosition::WGS84));
		    } else {
			itsPosition = MPosition(siteelev, qSiteLatLong, MPosition::WGS84);
		    }
		}
		Int field = primaryKeys().fieldNumber("EQUINOX");
		if (field >= 0 ) {
		    markAsUsed("EQUINOX");
		    itsEquinox = primaryKeys().asDouble(field); 
		}
		
		if (deviceVersion() < 1) {
		    // original behavior - RADECSYS, overused
		    field = primaryKeys().fieldNumber("RADECSYS");
		    if (field >= 0) {
			itsRadesys = primaryKeys().asString(field);
			markAsUsed("RADECSYS");
			if (itsRadesys != "FK5" && itsRadesys != "FK4") {
			    itsIndicsys = itsRadesys;
			    if (itsEquinox < 1984.0) itsRadesys = "FK4";
			    else itsRadesys = "FK5";
			} else {
			    itsIndicsys = "RADEC";
			}
			if (itsIndicsys == "APPARENT") itsIndicsys = "GAPPT";
		    }
		} else {
		    field = primaryKeys().fieldNumber("INDICSYS");
		    if (field >= 0) {
			itsIndicsys = primaryKeys().asString(field);
			markAsUsed("INDICSYS");
		    }
		    field = primaryKeys().fieldNumber("RADESYS");
		    if (field >= 0 ) {
			itsRadesys = primaryKeys().asString(field);
			markAsUsed("RADESYS");
		    }
		}
		if (itsIndicsys == "RADEC") {
		    if (itsRadesys == "FK5") {
			if (near(itsEquinox, Double(2000.0))) {
			    itsCoordType = MDirection::J2000;
			} else {
			    itsCoordType = MDirection::JMEAN;
			}
		    } else if (itsRadesys == "FK4") {
			itsCoordType = MDirection::B1950;
		    } else if (itsRadesys == "GAPPT") {
			itsCoordType = MDirection::APP;
		    } else {
			// unrecognized coordinate type
			os << LogIO::WARN << WHERE
			   << "Unrecognized RADESYS value : " << itsRadesys 
			   << " - marking this as USER type" << LogIO::POST;
			itsIsUser = True;
		    }
		} else if (itsIndicsys == "HADEC") {
		    itsCoordType = MDirection::HADEC;
		} else if (itsIndicsys == "AZEL") {
		    itsCoordType = MDirection::AZEL;
		} else if (itsIndicsys == "GALACTIC") {
		    itsCoordType = MDirection::GALACTIC;
		} else if (itsIndicsys == "USER") {
		    itsIsUser = True;
		} else {
		    // unrecognized coordinate type
		    os << LogIO::WARN << WHERE
		       << "Unrecognized INDICSYS value : " << itsIndicsys 
		       << " - marking this as USER type" << LogIO::POST;
		    itsIsUser = True;
		}
		// mark the ignorable primaryKeywords as used
		// the return value of markAsUsed doesn't matter, if its there it is marked
		// these all appeared in older versions
		if (deviceVersion() < 1) {
		    markAsUsed("STRTTIME");
		    markAsUsed("SOURCE");
		    markAsUsed("UTCSTART");
		    markAsUsed("UTDATE");
		}
		markAsUsed("LSTSTART");
		markAsUsed("DELTAUTC");
		
		// copy any unused primary keywords to itsPointingKeywords
		for (uInt i=0;i<usedPrimaryKeys().nelements();i++) {
		    if (!usedPrimaryKeys()[i]) {
			switch(primaryKeys().dataType(i)) {
			    // only 3 known types of keywords should be here
			case TpFloat:
			    itsPointingKeywords.define(primaryKeys().name(i),
						       primaryKeys().asFloat(i));
			    break;
			case TpDouble:
			    itsPointingKeywords.define(primaryKeys().name(i),
						       primaryKeys().asDouble(i));
			    break;
			case TpString:
			    itsPointingKeywords.define(primaryKeys().name(i),
						       primaryKeys().asString(i));
			    break;
			default:
			    // just ignore it
			    break;
			}		
		    }
		}
		
		// handle HDU based on given EXTNAME
		if (fitsTab.keywords().fieldNumber("EXTNAME")<0 ||
		    fitsTab.keywords().dataType("EXTNAME") != TpString) {
		    os << LogIO::WARN << WHERE
		       << "No EXTNAME keyword found in Antenna file : " << antennaFile
		       << "\nThis file will be ignored." << LogIO::POST;
		    return False;
		}
		String extname = fitsTab.keywords().asString("EXTNAME");
		if (extname == "BEAM_OFFSETS") {
		    digestBeamOffTab(fitsTab);
		} else if (extname == "ANTPOSPF" || 
			   extname == "ANTPOSGR" ||
			   extname == "ANTPOSST") {
		    digestAntPosTab(fitsTab, extname);
		    // that comes at the end of the file
		    more = False;
		} else {
		    os << LogIO::WARN << WHERE
		       << "Unrecognized EXTNAME = " << extname
		       << "\nFound in Antenna file : " << antennaFile
		       << "\nThis file will be ignored." << LogIO::POST;
		    return False;
		}
	    } catch (AipsError x) {
		// nothing to clean up here, just report the problem.
		os << LogIO::WARN << WHERE
		   << "Unable to open antenna file : " << antennaFile
		   << "\nThis file will be ignored." << LogIO::POST;
		return False;
	    }
	}
    }
    return True;
}

void GBTAntennaFile::digestBeamOffTab(FITSTable &beamofftab)
{
    // just convert it to a table for now
    if (itsBeamOffTable) {
	delete itsBeamOffTable;
	itsBeamOffTable = 0;
	itsRowFromFeed.clear();
    }

    // fill this table to a scratch table
    TableDesc td(FITSTable::tableDesc(beamofftab));
    SetupNewTable newtab("", td, Table::Scratch);
    uInt nrow = beamofftab.nrow();
    itsBeamOffTable = new Table(newtab, TableLock::PermanentLocking);
    AlwaysAssert(itsBeamOffTable, AipsError);
    itsBeamOffTable->addRow(nrow);
    uInt rownr = 0;
    TableRow row(*itsBeamOffTable);
    
    while (rownr < beamofftab.nrow()) {
	row.put(rownr, TableRecord(beamofftab.currentRow()));
	beamofftab.next();
	rownr++;
    }

    itsFeedNameCol.attach(*itsBeamOffTable, "NAME");
    itsXELOffCol.attach(*itsBeamOffTable, "BEAMXELOFFSET");
    itsELOffCol.attach(*itsBeamOffTable, "BEAMELOFFSET");
    itsSR1Col.attach(*itsBeamOffTable, "SRFEED1");
    itsSR2Col.attach(*itsBeamOffTable, "SRFEED2");

    // convert the simple numeric feed names to integers - these are the
    // physical feeds - remember where they are.
    for (uInt i=0;i<itsBeamOffTable->nrow();i++) {
	String thisName = itsFeedNameCol(i);
	if (thisName.matches(Regex("^ *[0-9]+ *$"))) {
	    // convert it to an integer
	    Int thisFeed = atoi(thisName.chars());
	    // and rememeber where it is
	    itsRowFromFeed(thisFeed) = i;
	}
    }  
}

void GBTAntennaFile::digestAntPosTab(FITSTable &antpostab, const String &extname)
{
    itsAttached = True;
    itsExtname = extname;

    // fill the binary table to the scratch table
    TableDesc td(FITSTable::tableDesc(antpostab));
    SetupNewTable newtab("", td, Table::Scratch);
    uInt nrow = antpostab.nrow();
    itsTable = new Table(newtab, TableLock::PermanentLocking);
    AlwaysAssert(itsTable, AipsError);
    itsTable->addRow(nrow);
    uInt rownr = 0;
    TableRow row(*itsTable);
    
    while (rownr < antpostab.nrow()) {
	row.put(rownr, TableRecord(antpostab.currentRow()));
	antpostab.next();
	rownr++;
    }
    
    // set the pointers to the fields coresponding to the pointing columns
    // common to all optics
    itsDMJD.attach(*itsTable, "DMJD");
    itsRAJ2000.attach(*itsTable, "RAJ2000");
    itsDECJ2000.attach(*itsTable, "DECJ2000");
    itsMAJOR.attach(*itsTable, "MAJOR");
    itsMINOR.attach(*itsTable, "MINOR");
    
    // units for those columns
    itsDMJDunit = Unit(itsDMJD.keywordSet().asString("UNIT"));
    itsRAunit = Unit(itsRAJ2000.keywordSet().asString("UNIT"));
    itsDECunit = Unit(itsDECJ2000.keywordSet().asString("UNIT"));
    itsMAJORunit = Unit(itsMAJOR.keywordSet().asString("UNIT"));
    itsMINORunit = Unit(itsMINOR.keywordSet().asString("UNIT"));
    
    itsMNT_AZ.attach(*itsTable, "MNT_AZ");
    itsMNT_EL.attach(*itsTable, "MNT_EL");
    itsREFRACT.attach(*itsTable, "REFRACT");
    
    itsMNT_AZunit = Unit(itsMNT_AZ.keywordSet().asString("UNIT"));
    itsMNT_ELunit = Unit(itsMNT_EL.keywordSet().asString("UNIT"));
    itsREFRACTunit = Unit(itsREFRACT.keywordSet().asString("UNIT"));
    
    // set the focus fields appropriate for this optics type
    if (itsExtname == "ANTPOSPF") {
	itsFOCUS.attach(*itsTable, "PF_FOCUS");
	itsROTATION.attach(*itsTable, "PF_ROTATION");
	itsX.attach(*itsTable, "PF_X");
	itsFOCUSunit = Unit(itsFOCUS.keywordSet().asString("UNIT"));
	itsROTATIONunit = Unit(itsROTATION.keywordSet().asString("UNIT"));
	itsXunit = Unit(itsX.keywordSet().asString("UNIT"));
    } else if (itsExtname == "ANTPOSGR") {
	itsXP.attach(*itsTable, "SR_XP");
	itsYP.attach(*itsTable, "SR_YP");
	itsZP.attach(*itsTable, "SR_ZP");
	itsXT.attach(*itsTable, "SR_XT");
	itsYT.attach(*itsTable, "SR_YT");
	itsZT.attach(*itsTable, "SR_ZT");
	itsXPunit = Unit(itsXP.keywordSet().asString("UNIT"));
	itsYPunit = Unit(itsYP.keywordSet().asString("UNIT"));
	itsXPunit = Unit(itsXP.keywordSet().asString("UNIT"));
	itsXTunit = Unit(itsXP.keywordSet().asString("UNIT"));
	itsYTunit = Unit(itsYT.keywordSet().asString("UNIT"));
	itsZTunit = Unit(itsZT.keywordSet().asString("UNIT"));
    }
}

Bool GBTAntennaFile::equalPointingModel(const RecordInterface &other) const
{
    // other must equal itsPointingKeywords
    // Same number of fields
    Bool result = other.nfields() == itsPointingKeywords.nfields();

    if (result) {
	// same name, type, and value for each field
	uInt thisField = 0;
	while (result && thisField < itsPointingKeywords.nfields()) {
	    uInt otherField = other.fieldNumber(itsPointingKeywords.name(thisField));
	    result = otherField >= 0 && 
		itsPointingKeywords.dataType(thisField) == itsPointingKeywords.dataType(otherField);
	    if (result) {
		// there can only be string, float, and double values
		switch (itsPointingKeywords.dataType(thisField)) {
		case TpFloat:
		    result = itsPointingKeywords.asFloat(thisField) == other.asFloat(thisField);
		    break;
		case TpDouble:
		    result = itsPointingKeywords.asDouble(thisField) == other.asDouble(thisField);
		    break;
		case TpString:
		    result = itsPointingKeywords.asString(thisField) == other.asString(thisField);
		    break;
		default:
		    result = False;
		    break;
		}
	    }
	    thisField++;
	}
    }

    return result;
}

Bool GBTAntennaFile::getPointing(Double time, Double interval,
				 MDirection &j2000, MDirection &user)
{
    Bool result = False;
    uInt start, stop;
    start = stop = 0;
    if (itsTable && itsTable->nrow()>0) {
	uInt startRow, endRow;
	result = findTimeRange(time, interval, startRow, endRow);
	start = startRow;
	stop = endRow;
	if (result) {
	    if (endRow > startRow && (endRow-startRow)>1) {
		// everything went as expected, find the mean values and their statistics
		Double meanRA = getMean(itsRAJ2000, startRow, endRow, True);
		Double meanDEC = getMean(itsDECJ2000, startRow, endRow);
		// make an MPosition from the mean values
		j2000 = MDirection(Quantity(meanRA, itsRAunit),
				   Quantity(meanDEC, itsDECunit),
				   MDirection::J2000);

		// and similary for the user coordinates
		meanRA = getMean(itsMAJOR, startRow, endRow, True);
		meanDEC = getMean(itsMINOR, startRow, endRow);
		// make an MPosition from the mean values, in degrees
		user = MDirection(Quantity(meanRA, itsMAJORunit),
				  Quantity(meanDEC, itsMINORunit),
				  type());
	    } else {
		if (startRow != endRow) {
		    // simple linear interpolation
		    Double timeRange = itsDMJD.asdouble(endRow) - itsDMJD.asdouble(startRow);
		    Double timeFrac = (time/itsSecondsPerDay - itsDMJD.asdouble(startRow))/timeRange;
		    Double ra = getInterpolated(itsRAJ2000, startRow, endRow, timeFrac,
						True);
		    Double dec = getInterpolated(itsDECJ2000, startRow, endRow, timeFrac);
		    j2000 = MDirection(Quantity(ra, itsRAunit),
				       Quantity(dec, itsDECunit),
				       MDirection::J2000);
		    
		    // and again for the user coordinates
		    ra = getInterpolated(itsMAJOR, startRow, endRow, timeFrac, True);
		    dec = getInterpolated(itsMINOR, startRow, endRow, timeFrac);
		    user = MDirection(Quantity(ra, itsMAJORunit),
				      Quantity(dec, itsMINORunit),
				      type());
		} else {
		    // just a single point, no interpolation
		    j2000 = MDirection(Quantity(itsRAJ2000(startRow), itsRAunit),
				       Quantity(itsDECJ2000(startRow), itsDECunit),
				       MDirection::J2000);
		    
		    // and again for the user coordinates
		    user = MDirection(Quantity(itsMAJOR(startRow), itsMAJORunit),
				      Quantity(itsMINOR(startRow), itsMINORunit),
				      type());
		}
	    }
	} else {
	    // the interval is off the end, which end implies which
	    // values to use, returns False
	    if (startRow >= itsTable->nrow()) startRow = (itsTable->nrow()-1);
	    j2000 = MDirection(Quantity(itsRAJ2000(startRow), itsRAunit),
			       Quantity(itsDECJ2000(startRow), itsDECunit),
			       MDirection::J2000);
	    
	    // and again for the user coordinates
	    user = MDirection(Quantity(itsMAJOR(startRow), itsMAJORunit),
			      Quantity(itsMINOR(startRow), itsMINORunit),
			      type());
	}
    }
    return result;
}

Bool GBTAntennaFile::fillPointingTable(Table &pointingTable)
{
    Bool result = False;
    if (itsTable) {
	// is pointingTable locked, if not, lock it
	Bool lockAcquired = False;
	if (!pointingTable.hasLock()) {
	    pointingTable.lock();
	    lockAcquired = True;
	}
	// add any columns as necessary
	TableDesc pointingTd(pointingTable.tableDesc());
	TableDesc itsTd(itsTable->tableDesc());
	TableDesc newCols;
	Double lastTime = 0.0;
	if (!pointingTd.isColumn("TIME")) {
	    // DMJD is really TIME, which should be an MEpoch column
	    newCols.addColumn(ScalarColumnDesc<Double>("TIME", "Modified Julain Day"));
	    TableMeasDesc<MEpoch> measCol(TableMeasValueDesc(newCols,"TIME"),
					  TableMeasRefDesc(MEpoch::DEFAULT));
	    measCol.write(newCols);
	    // change the units to "s" from the default of "d"
	    TableQuantumDesc timeqd(newCols, "TIME", Unit("s"));
	    timeqd.write(newCols);
	} else if (pointingTable.nrow() > 0) {
	    // find the last time in the table - we start after that
	    TableColumn timeCol(pointingTable,"TIME");
	    lastTime = timeCol.asdouble(pointingTable.nrow()-1);
	}
	// all other columns, just make simply Quantity columns, using existing units
	// all of these are Double columns, just remove the UNIT keyword from the
	// column desc in itsTd
	if (!pointingTd.isColumn("RAJ2000")) {
	    newCols.addColumn(itsRAJ2000.columnDesc());
	    if (newCols["RAJ2000"].keywordSet().fieldNumber("UNIT") >= 0) {
		newCols.rwColumnDesc("RAJ2000").rwKeywordSet().removeField("UNIT");
	    }
	    // and now construct the TableQuantumDesc to make sure its correct
	    TableQuantumDesc tqd(newCols, "RAJ2000", itsRAunit);
	    tqd.write(newCols);
	}
	if (!pointingTd.isColumn("DECJ2000")) {
	    newCols.addColumn(itsDECJ2000.columnDesc());
	    if (newCols["DECJ2000"].keywordSet().fieldNumber("UNIT") >= 0) {
		newCols.rwColumnDesc("DECJ2000").rwKeywordSet().removeField("UNIT");
	    }
	    // and now construct the TableQuantumDesc to make sure its correct
	    TableQuantumDesc tqd(newCols, "DECJ2000", itsDECunit);
	    tqd.write(newCols);
	}
	if (!pointingTd.isColumn("MAJOR")) {
	    newCols.addColumn(itsMAJOR.columnDesc());
	    if (newCols["MAJOR"].keywordSet().fieldNumber("UNIT") >= 0) {
		newCols.rwColumnDesc("MAJOR").rwKeywordSet().removeField("UNIT");
	    }
	    // and now construct the TableQuantumDesc to make sure its correct
	    TableQuantumDesc tqd(newCols, "MAJOR", itsMAJORunit);
	    tqd.write(newCols);
	}
	if (!pointingTd.isColumn("MINOR")) {
	    newCols.addColumn(itsMINOR.columnDesc());
	    if (newCols["MINOR"].keywordSet().fieldNumber("UNIT") >= 0) {
		newCols.rwColumnDesc("MINOR").rwKeywordSet().removeField("UNIT");
	    }
	    // and now construct the TableQuantumDesc to make sure its correct
	    TableQuantumDesc tqd(newCols, "MINOR", itsMINORunit);
	    tqd.write(newCols);
	}
	if (!pointingTd.isColumn("MNT_AZ")) {
	    newCols.addColumn(itsMNT_AZ.columnDesc());
	    if (newCols["MNT_AZ"].keywordSet().fieldNumber("UNIT") >= 0) {
		newCols.rwColumnDesc("MNT_AZ").rwKeywordSet().removeField("UNIT");
	    }
	    // and now construct the TableQuantumDesc to make sure its correct
	    TableQuantumDesc tqd(newCols, "MNT_AZ", itsMNT_AZunit);
	    tqd.write(newCols);
	}
	if (!pointingTd.isColumn("MNT_EL")) {
	    newCols.addColumn(itsMNT_EL.columnDesc());
	    if (newCols["MNT_EL"].keywordSet().fieldNumber("UNIT") >= 0) {
		newCols.rwColumnDesc("MNT_EL").rwKeywordSet().removeField("UNIT");
	    }
	    // and now construct the TableQuantumDesc to make sure its correct
	    TableQuantumDesc tqd(newCols, "MNT_EL", itsMNT_ELunit);
	    tqd.write(newCols);
	}
	if (!pointingTd.isColumn("REFRACT")) {
	    newCols.addColumn(itsREFRACT.columnDesc());
	    if (newCols["REFRACT"].keywordSet().fieldNumber("UNIT") >= 0) {
		newCols.rwColumnDesc("REFRACT").rwKeywordSet().removeField("UNIT");
	    }
	    // and now construct the TableQuantumDesc to make sure its correct
	    TableQuantumDesc tqd(newCols, "REFRACT", itsREFRACTunit);
	    tqd.write(newCols);
	}
	if (!pointingTd.isColumn("EQUINOX")) {
	    newCols.addColumn(ScalarColumnDesc<Double>("EQUINOX","EQUINOX of MAJOR and MINOR columns"));
	}
	if (!pointingTd.isColumn("INDICSYS")) {
	    newCols.addColumn(ScalarColumnDesc<String>("INDICSYS","Coordinate system of MAJOR and MINOR columns"));
	}
	if (!pointingTd.isColumn("RADESYS")) {
	    newCols.addColumn(ScalarColumnDesc<String>("RADESYS","RADESYS of MAJOR and MINOR columns - if equitorial system."));
	}

	// are there any columns to add
	if (newCols.columnDescSet().ncolumn() > 0) {
	    for (uInt i=0;i<newCols.columnDescSet().ncolumn();i++) {
		pointingTable.addColumn(newCols.columnDescSet()[i]);
	    }
	}

	// remember where we start from
	uInt startOut = pointingTable.nrow();
	uInt length = itsTable->nrow();
	// where do we start from in itsTable
	uInt startIn = 0;
	while (startIn < length && itsDMJD(startIn)*itsSecondsPerDay < lastTime) startIn++;
	length -= startIn;
	if (length > 0) {
	    pointingTable.addRow(length, True);
	    Slice outSlice(startOut, length, 1);
	    Slice inSlice(startIn, length, 1);
	    // and copy it column by column
	    {
		ScalarColumn<Double> outCol(pointingTable, "TIME");
		// scale to convert to seconds
		outCol.putColumnRange(outSlice, itsDMJD.getColumnRange(inSlice)*itsSecondsPerDay);
	    }
	    
	    {
		ScalarColumn<Double> outCol(pointingTable, "RAJ2000");
		outCol.putColumnRange(outSlice, itsRAJ2000.getColumnRange(inSlice));
	    }
	    
	    {
		ScalarColumn<Double> outCol(pointingTable, "DECJ2000");
		outCol.putColumnRange(outSlice, itsDECJ2000.getColumnRange(inSlice));
	    }
	    
	    {
		ScalarColumn<Double> outCol(pointingTable, "MAJOR");
		outCol.putColumnRange(outSlice, itsMAJOR.getColumnRange(inSlice));
	    }
	    
	    {
		ScalarColumn<Double> outCol(pointingTable, "MINOR");
		outCol.putColumnRange(outSlice, itsMINOR.getColumnRange(inSlice));
	    }
	    
	    {
		ScalarColumn<Double> outCol(pointingTable, "MNT_AZ");
		outCol.putColumnRange(outSlice, itsMNT_AZ.getColumnRange(inSlice));
	    }
	    
	    {
		ScalarColumn<Double> outCol(pointingTable, "MNT_EL");
		outCol.putColumnRange(outSlice, itsMNT_EL.getColumnRange(inSlice));
	    }
	    
	    {
		ScalarColumn<Double> outCol(pointingTable, "REFRACT");
		outCol.putColumnRange(outSlice, itsREFRACT.getColumnRange(inSlice));
	    }
	    {
		ScalarColumn<Double> outCol(pointingTable, "EQUINOX");
		outCol.putColumnRange(outSlice, Vector<Double>(length, equinox()));
	    }
	    {
		ScalarColumn<String> outCol(pointingTable, "INDICSYS");
		outCol.putColumnRange(outSlice, Vector<String>(length, indicsys()));
	    }
	    {
		ScalarColumn<String> outCol(pointingTable, "RADESYS");
		outCol.putColumnRange(outSlice, Vector<String>(length, radesys()));
	    }
	}

	// unlock it if we acquired the lock here
	if (lockAcquired) pointingTable.unlock();
    }
    return result;
}

Bool GBTAntennaFile::getPrimeFocus(Double time, Double interval,
				   Quantity &focus,
				   Quantity &rotation,
				   Quantity &x)
{
    Bool result = False;
    uInt start, stop;
    start = stop = 0;
    if (itsTable && itsTable->nrow()>0 && opticsType() == "ANTPOSPF") {
	uInt startRow, endRow;
	result = findTimeRange(time, interval, startRow, endRow);
	start = startRow;
	stop = endRow;
	if (result) {
	    if (endRow > startRow) {
		// cache the units from the table
		focus = Quantity(getMean(itsFOCUS, startRow, endRow), itsFOCUSunit);
		rotation = Quantity(getMean(itsROTATION, startRow, endRow), itsROTATIONunit);
		x = Quantity(getMean(itsX, startRow, endRow), itsXunit);
	    } else {
		if (startRow != endRow) {
		    // simple linear interpolation
		    Double timeRange = itsDMJD.asdouble(endRow) - itsDMJD.asdouble(startRow);
		    Double timeFrac = (time/itsSecondsPerDay - itsDMJD.asdouble(startRow))/timeRange;
		    focus = Quantity(getInterpolated(itsFOCUS, startRow, endRow, timeFrac), itsFOCUSunit);
		    rotation = Quantity(getInterpolated(itsROTATION, startRow, endRow, timeFrac), 
					itsROTATIONunit);
		    x = Quantity(getInterpolated(itsX, startRow, endRow, timeFrac), itsXunit);
		} else {
		    // single point
		    focus = Quantity(itsFOCUS(startRow), itsFOCUSunit);
		    rotation = Quantity(itsROTATION(startRow), itsROTATIONunit);
		    x = Quantity(itsX(startRow), itsXunit);
		}
	    }
	} else {
	    // the interval is off the end, which end implies which
	    // values to use, returns False
	    if (startRow >= itsTable->nrow()) startRow = (itsTable->nrow()-1);
	    focus = Quantity(itsFOCUS(startRow), itsFOCUSunit);
	    rotation = Quantity(itsROTATION(startRow), itsROTATIONunit);
	    x = Quantity(itsX(startRow), itsXunit);
	}
    }
    return result;
}


Bool GBTAntennaFile::getGregorianFocus(Double time, Double interval,
				       Quantity &xp, Quantity &yp, Quantity &zp,
				       Quantity &xt, Quantity &yt, Quantity &zt)
{
    Bool result = False;
    uInt start, stop;
    start = stop = 0;
    if (itsTable && itsTable->nrow()>0 && opticsType() == "ANTPOSGR") {
	uInt startRow, endRow;
	result = findTimeRange(time, interval, startRow, endRow);
	start = startRow;
	stop = endRow;
	if (result) {
	    if (endRow > startRow) {
		// cache the units from the table
		xp = Quantity(getMean(itsXP, startRow, endRow), itsXPunit);
		yp = Quantity(getMean(itsYP, startRow, endRow), itsYPunit);
		zp = Quantity(getMean(itsZP, startRow, endRow), itsZPunit);
		xt = Quantity(getMean(itsXT, startRow, endRow), itsXTunit);
		yt = Quantity(getMean(itsYT, startRow, endRow), itsYTunit);
		zt = Quantity(getMean(itsZT, startRow, endRow), itsZTunit);
	    } else {
		if (startRow != endRow) {
		    // simple linear interpolation
		    Double timeRange = itsDMJD.asdouble(endRow) - itsDMJD.asdouble(startRow);
		    Double timeFrac = (time/itsSecondsPerDay - itsDMJD.asdouble(startRow))/timeRange;
		    xp = Quantity(getInterpolated(itsXP, startRow, endRow, timeFrac), itsXPunit);
		    yp = Quantity(getInterpolated(itsYP, startRow, endRow, timeFrac), itsYPunit);
		    zp = Quantity(getInterpolated(itsZP, startRow, endRow, timeFrac), itsZPunit);
		    xt = Quantity(getInterpolated(itsXT, startRow, endRow, timeFrac), itsXTunit);
		    yt = Quantity(getInterpolated(itsYT, startRow, endRow, timeFrac), itsYTunit);
		    zt = Quantity(getInterpolated(itsZT, startRow, endRow, timeFrac), itsZTunit);
		} else {
		    // single point, no interpolation
		    xp = Quantity(itsXP(startRow), itsXPunit);
		    yp = Quantity(itsYP(startRow), itsYPunit);
		    zp = Quantity(itsZP(startRow), itsZPunit);
		    xt = Quantity(itsXT(startRow), itsXTunit);
		    yt = Quantity(itsYT(startRow), itsYTunit);
		    zt = Quantity(itsZT(startRow), itsZTunit);		}
	    }
	} else {
	    // the interval is off the end, which end implies which
	    // values to use, returns False
	    if (startRow >= itsTable->nrow()) startRow = (itsTable->nrow()-1);
	    xp = Quantity(itsXP(startRow), itsXPunit);
	    yp = Quantity(itsYP(startRow), itsYPunit);
	    zp = Quantity(itsZP(startRow), itsZPunit);
	    xt = Quantity(itsXT(startRow), itsXTunit);
	    yt = Quantity(itsYT(startRow), itsYTunit);
	    zt = Quantity(itsZT(startRow), itsZTunit);
	}
    }
    return result;
}

Bool GBTAntennaFile::fillFocusTable(Table &focusTable)
{
    // prefix is ignored right now
    Bool result = False;
    if (itsTable) {
	// is focusTable locked, if not, lock it
	Bool lockAcquired = False;
	if (!focusTable.hasLock()) {
	    focusTable.lock();
	    lockAcquired = True;
	}
	// add any columns as necessary
	TableDesc focusTd(focusTable.tableDesc());
	TableDesc itsTd(itsTable->tableDesc());
	TableDesc newCols;
	Double lastTime = 0.0;
	if (!focusTd.isColumn("TIME")) {
	    // DMJD is really TIME, which should be an MEpoch column
	    newCols.addColumn(ScalarColumnDesc<Double>("TIME", "Modified Julain Day"));
	    TableMeasDesc<MEpoch> measCol(TableMeasValueDesc(newCols,"TIME"),
					  TableMeasRefDesc(MEpoch::DEFAULT));
	    measCol.write(newCols);
	    // change the units to "s" from the default of "d"
	    TableQuantumDesc timeqd(newCols, "TIME", Unit("s"));
	    timeqd.write(newCols);
	} else if (focusTable.nrow() > 0) {
	    // find the last time in the table - we start after that
	    TableColumn timeCol(focusTable,"TIME");
	    lastTime = timeCol.asdouble(focusTable.nrow()-1);
	}
	if (!focusTd.isColumn("EXTNAME")) {
	    newCols.addColumn(ScalarColumnDesc<String>("EXTNAME", "EXTNAME keyword value"));
	}

	if (opticsType() == "ANTPOSPF") {
	    if (!focusTd.isColumn("PF_FOCUS")) {
		newCols.addColumn(itsFOCUS.columnDesc());
		if (newCols["PF_FOCUS"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("PF_FOCUS").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "PF_FOCUS", itsRAunit);
		tqd.write(newCols);
	    }
	    if (!focusTd.isColumn("PF_ROTATION")) {
		newCols.addColumn(itsROTATION.columnDesc());
		if (newCols["PF_ROTATION"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("PF_ROTATION").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "PF_ROTATION", itsRAunit);
		tqd.write(newCols);
	    }
	    if (!focusTd.isColumn("PF_X")) {
		newCols.addColumn(itsX.columnDesc());
		if (newCols["PF_X"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("PF_X").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "PF_X", itsRAunit);
		tqd.write(newCols);
	    }
	    
	    // are there any columns to add
	    if (newCols.columnDescSet().ncolumn() > 0) {
		for (uInt i=0;i<newCols.columnDescSet().ncolumn();i++) {
		    focusTable.addColumn(newCols.columnDescSet()[i]);
		}
	    }

	    // remember where we start from
	    uInt startOut = focusTable.nrow();
	    uInt length = itsTable->nrow();
	    // where do we start from in itsTable
	    uInt startIn = 0;
	    while (startIn < length && itsDMJD(startIn)*itsSecondsPerDay < lastTime) startIn++;
	    length -= startIn;
	    if (length > 0) {
		Slice outSlice(startOut, length, 1);
		Slice inSlice(startIn, length, 1);
		focusTable.addRow(length, True);
		// and copy it column by column
		{
		    ScalarColumn<Double> outCol(focusTable, "TIME");
		    // scale to convert to seconds
		    outCol.putColumnRange(outSlice, itsDMJD.getColumnRange(inSlice)*itsSecondsPerDay);
		}
		
		{
		    ScalarColumn<Double> outCol(focusTable, "PF_FOCUS");
		    outCol.putColumnRange(outSlice, itsFOCUS.getColumnRange(inSlice));
		}
		
		{
		    ScalarColumn<Double> outCol(focusTable, "PF_ROTATION");
		    outCol.putColumnRange(outSlice, itsROTATION.getColumnRange(inSlice));
		}
		
		{
		    ScalarColumn<Double> outCol(focusTable, "PF_X");
		    outCol.putColumnRange(outSlice, itsX.getColumnRange(inSlice));
		}
		{
		    ScalarColumn<String> outCol(focusTable, "EXTNAME");
		    outCol.putColumnRange(outSlice, Vector<String>(length, opticsType()));
		}
	    }
	} else if (opticsType() == "ANTPOSGR") {
	    if (!focusTd.isColumn("SR_XP")) {
		newCols.addColumn(itsXP.columnDesc());
		if (newCols["SR_XP"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("SR_XP").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "SR_XP", itsRAunit);
		tqd.write(newCols);
	    }
	    if (!focusTd.isColumn("SR_YP")) {
		newCols.addColumn(itsYP.columnDesc());
		if (newCols["SR_YP"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("SR_YP").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "SR_YP", itsRAunit);
		tqd.write(newCols);
	    }
	    if (!focusTd.isColumn("SR_ZP")) {
		newCols.addColumn(itsZP.columnDesc());
		if (newCols["SR_ZP"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("SR_ZP").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "SR_ZP", itsRAunit);
		tqd.write(newCols);
	    }
	    if (!focusTd.isColumn("SR_XT")) {
		newCols.addColumn(itsXT.columnDesc());
		if (newCols["SR_XT"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("SR_XT").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "SR_XT", itsRAunit);
		tqd.write(newCols);
	    }
	    if (!focusTd.isColumn("SR_YT")) {
		newCols.addColumn(itsYT.columnDesc());
		if (newCols["SR_YT"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("SR_YT").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "SR_YT", itsRAunit);
		tqd.write(newCols);
	    }
	    if (!focusTd.isColumn("SR_ZT")) {
		newCols.addColumn(itsZT.columnDesc());
		if (newCols["SR_ZT"].keywordSet().fieldNumber("UNIT") >= 0) {
		    newCols.rwColumnDesc("SR_ZT").rwKeywordSet().removeField("UNIT");
		}
		// and now construct the TableQuantumDesc to make sure its correct
		TableQuantumDesc tqd(newCols, "SR_ZT", itsRAunit);
		tqd.write(newCols);
	    }
	    
	    // are there any columns to add
	    if (newCols.columnDescSet().ncolumn() > 0) {
		for (uInt i=0;i<newCols.columnDescSet().ncolumn();i++) {
		    focusTable.addColumn(newCols.columnDescSet()[i]);
		}
	    }
		
	    // remember where we start from
	    uInt startOut = focusTable.nrow();
	    uInt length = itsTable->nrow();
	    // where do we start from in itsTable
	    uInt startIn = 0;
	    while (startIn < length && itsDMJD(startIn)*itsSecondsPerDay < lastTime) startIn++;
	    length -= startIn;
	    if (length > 0) {
		Slice outSlice(startOut, length, 1);
		Slice inSlice(startIn, length, 1);
		focusTable.addRow(length, True);
		// and copy it column by column
		{
		    ScalarColumn<Double> outCol(focusTable, "TIME");
		    // scale to convert to seconds
		    outCol.putColumnRange(outSlice, itsDMJD.getColumnRange(inSlice)*itsSecondsPerDay);
		}
		
		{
		    ScalarColumn<Double> outCol(focusTable, "SR_XP");
		    outCol.putColumnRange(outSlice, itsXP.getColumnRange(inSlice));
		}
		{
		    ScalarColumn<Double> outCol(focusTable, "SR_YP");
		    outCol.putColumnRange(outSlice, itsYP.getColumnRange(inSlice));
		}
		{
		    ScalarColumn<Double> outCol(focusTable, "SR_ZP");
		    outCol.putColumnRange(outSlice, itsZP.getColumnRange(inSlice));
		}
		
		{
		    ScalarColumn<Double> outCol(focusTable, "SR_XT");
		    outCol.putColumnRange(outSlice, itsXT.getColumnRange(inSlice));
		}
		{
		    ScalarColumn<Double> outCol(focusTable, "SR_YT");
		    outCol.putColumnRange(outSlice, itsYT.getColumnRange(inSlice));
		}
		{
		    ScalarColumn<Double> outCol(focusTable, "SR_ZT");
		    outCol.putColumnRange(outSlice, itsZT.getColumnRange(inSlice));
		}
		{
		    ScalarColumn<String> outCol(focusTable, "EXTNAME");
		    outCol.putColumnRange(outSlice, Vector<String>(length, opticsType()));
		}
	    }
		
	} // there is no focus information for ANTPOSST
	    
	// unlock it if we acquired the lock here
	if (lockAcquired) focusTable.unlock();
    }
    return result;
}

void GBTAntennaFile::setDefaults()
{
    if (itsTable) delete itsTable;
    itsTable = 0;

    itsFileName = "";
    itsAttached = False;
    itsExtname = "";
    MeasTable::Observatory(itsPosition, "GBT");
    itsPointingKeywords = Record();
    itsEquinox = 2000.0;
    itsCoordType = MDirection::J2000;
    itsRadesys = "FK5";
    itsIndicsys = "RADEC";
    itsIsUser = False;

    itsLastTime = itsLastInterval = itsLastStartTime = itsLastEndTime = 0.0;
    itsLastStart = itsLastEnd = itsLastReturnedStart = itsLastReturnedEnd = 0;

    itsTrckBeam = "";

    itsRowFromFeed.clear();
}

Bool GBTAntennaFile::findTimeRange(Double time, Double interval,
				   uInt &startRow, uInt &endRow)
{
    Bool result = False;
    if (time == itsLastTime && interval == itsLastInterval) {
	// we've just seen this - use it again
	startRow = itsLastReturnedStart;
	endRow = itsLastReturnedEnd;
	result = True;
    } else {
	Int iend;
	startRow = 0;
	iend = 0;
	uInt npos = itsTable->nrow();
	// DMJD is in days, convert time and interval to days, both are now
	// in seconds
	Double dayTime = time / itsSecondsPerDay;
	Double dayInterval = interval / itsSecondsPerDay;
	Double startTime = dayTime - dayInterval/2.0;
	Double stopTime = dayTime + dayInterval/2.0;
	if (startTime >= itsLastEndTime) startRow = itsLastEnd;
	else if (startTime >= itsLastStartTime) startRow = itsLastStart;
	// otherwise we start at 0
	
	// find the first time >= startTime
	for (;startRow<npos;startRow++) {
	    if (itsDMJD(startRow) >= startTime) break;
	}
	
	// then, find the first time > stopTime
	for (iend=startRow; iend<Int(npos); iend++) {
	    if (itsDMJD(iend) > stopTime) break;
	}
	// we end with the previous point
	iend--;
	if (iend >= 0 && uInt(iend) >= startRow) {
	    // everything went as expected
	    endRow = uInt(iend);
	    result = True;
	} else {
	    // is this beyond the end
	    if (startRow != 0 && iend >= 0 && startRow < npos) {
		// no, its between adjacent values, leave as is
		endRow = iend;
		result = True;
	    } else {
		// otherwise it is beyond the end, will return False;
		endRow = startRow;
	    }
	}
	if (result) {
	    itsLastTime = time;
	    itsLastInterval = interval;
	    itsLastStart = startRow;
	    itsLastEnd = endRow;
	    itsLastStartTime = itsDMJD(startRow);
	    if (endRow < itsTable->nrow()) {
		itsLastEndTime = itsDMJD(endRow);
	    } else {
		itsLastEndTime = itsDMJD(itsTable->nrow());
	    }
	    // now rationalize startRow and endRow - the above settings are appropriate
	    // for restarts but the consumer doesn't need to know about the following
	    // interpretation details.
	    if (startRow == endRow) {
		// one point fell in the interval.  Set start and end row such that
		// they bracket the center of the interval.  Return start=end only if
		// the center is near that time or if this is at an end point.
		Double antTime = itsDMJD.asdouble(startRow);
		if (!nearAbs(antTime, dayTime)) {
		    if (itsDMJD.asdouble(startRow) > dayTime) {
			// skew towards previous pixel
			if (startRow != 0) startRow--;
			// otherwise leave as is and just return this point
		    } else {
			// skew towards next pixel
			if (endRow != (itsTable->nrow()-1)) endRow++;
			// otherwise leave as is and just return this point
		    }
		}
	    } else if (endRow < startRow) {
		// this means that the entire interval is between adjacent points
		// return them -> reverse sense of start and end
		uInt tmp = startRow;
		startRow = endRow;
		endRow = tmp;
	    }
	    itsLastReturnedStart = startRow;
	    itsLastReturnedEnd = endRow;
	}
    }
    return result;
}

Double GBTAntennaFile::getMean(ROScalarColumn<Double> &col, uInt startRow, 
			       uInt endRow, Bool canWrap)
{
    // need to watch for things which wrap around 0.0/360.0
    Double result;
    if (canWrap) {
	Vector<Double> colVec = 
	    col.getColumnRange(Slice(Int(startRow), (endRow-startRow+1), 1));
	if (anyLT(colVec, 10.0) && anyGT(colVec, 350.0)) {
	    MaskedArray<Double> maskedColVec(colVec, colVec > 350.0);
	    Vector<Double> subColVec(maskedColVec.getCompressedArray());
	    subColVec -= 360.0;
	    maskedColVec.setCompressedArray(subColVec);
	}
	result = mean(colVec);
    } else {
	result = 
	    mean(col.getColumnRange(Slice(Int(startRow), (endRow-startRow+1), 1)));
    }
    return result;
}

Double GBTAntennaFile::getInterpolated(ROScalarColumn<Double> &col, uInt low, uInt high, 
				       Double fraction, Bool canWrap)
{
    // need to watch for things which wrap around 0.0/360.0
    Double lowVal = col(low);
    Double highVal = col(high);
    if (canWrap && abs(lowVal-highVal) > 180.0) {
	if (lowVal > 180.0) lowVal -= 360.0;
	if (highVal > 180.0) highVal -= 360.0;
    }
    return (lowVal + fraction*(highVal-lowVal));
}

Bool GBTAntennaFile::getFeedInfo(Int whichFeed, Double &xeloffset, Double &eloffset,
				 Int &srfeed1, Int &srfeed2, String &feedName) const
{
    Bool result = False;
    if (itsRowFromFeed.isDefined(whichFeed)) {
	result = True;
	Int whichRow = itsRowFromFeed(whichFeed);
	xeloffset = itsXELOffCol(whichRow);
	eloffset = itsELOffCol(whichRow);
	srfeed1 = itsSR1Col(whichRow);
	srfeed2 = itsSR2Col(whichRow);
	feedName = itsFeedNameCol(whichRow);
    }
    return result;
}
