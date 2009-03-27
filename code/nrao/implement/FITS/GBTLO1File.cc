//# GBTLO1File.h: Digests the LO1 FITS file.
//# Copyright (C) 2001,2002
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

#include <nrao/FITS/GBTLO1File.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <fits/FITS/fits.h>
#include <fits/FITS/fitsio.h>
#include <fits/FITS/hdu.h>
#include <fits/FITS/FITSTable.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicMath/Math.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MCFrequency.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MRadialVelocity.h>
#include <casa/OS/Path.h>
#include <tables/Tables/Table.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>

GBTLO1File::GBTLO1File()
    : itsSouvel(0), itsLO1Table(0)
{init();}

GBTLO1File::~GBTLO1File()
{
  delete itsSouvel;
  itsSouvel = 0;
  delete itsLO1Table;
  itsLO1Table = 0;
}

Bool GBTLO1File::attach(const String &fitsFile)
{
    LogIO os(LogOrigin("GBTLO1File", 
		       "attach(const String &fitsFile)"));

    init();

    // is the requested fitsFile non-empty, try and open it
    if (!fitsFile.empty()) {
	// doing it this way makes it easier to catch any problems opening the file
	// there may be as many as 3 tables here
	Bool more = True;
	uInt count = 0;
	while (count<3 && more) {
	    FITSTable fitsTab(fitsFile, count+1);
	    if (fitsTab.isValid() && fitsTab.keywords().fieldNumber("EXTNAME")>=0) {
		count++;
		String extname = fitsTab.keywords().asString("EXTNAME");
		if (extname == "STATE") {
		    setLO1State(fitsTab);
		} else if (extname == "SOUVEL") {
		    setSouvel(fitsTab);
		} else if (extname == "LO1TBL") {
		    setLO1Table(fitsTab);
		} else {
		    os << LogIO::WARN << WHERE
		       << "Unexpected binary table found in LO1 file : " << fitsFile
		       << LogIO::POST;
		    os << LogIO::WARN << WHERE
		       << "The unrecognized EXTNAME is " << extname
		       << LogIO::POST;
		    os << "This may cause unexpected failures in the filler." 
		       << LogIO::POST;
		}
	    } else {
		more = False;
	    }
	}
	// there should at the least be one table
	if (count <1) {
	    os << LogIO::WARN << WHERE
	       << "Unable to open LO1 file : " << fitsFile
	       << "\nThis file will be ignored." << LogIO::POST;
	    init();
	    return False;
	}
	// there should at least be an LO1 table
	if (!itsLO1Table) {
	    os << LogIO::WARN << WHERE
	       << "No valid LO1 table found in an apparent LO1 file : " << fitsFile
	       << "\nThis file will be ignored." << LogIO::POST;
	    init();
	    return False;
	}

	// the LO1 table should have at least one row
	if (itsLO1Table->nrow() <= 0) {
	    os << LogIO::WARN << WHERE
	       << "The LO1TBL in " << fitsFile << " has no rows." << endl;
	    os << "The frequency axis associated with this scan can not be constructed." << endl;
	    os << "Channel numbers will be used instead." << endl;
	    init();
	    return False;
	}
	// okay, it seems to be valid, start setting the internals
	itsAttached = True;

	// setLO1Table has already set the primary keywords
	
	// extract fields from the primary keywords

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
	Int restfrqField = primaryKeys().fieldNumber("RESTFRQ");
	if (restfrqField >= 0) {
	    itsRestfrq = primaryKeys().asDouble(restfrqField);
	}
	Int reqdptolField = primaryKeys().fieldNumber("REQDPTOL");
	if (reqdptolField >= 0) {
	    // use at least a 10Hz tolerance due to the discrepancy
	    // between aips++ and M&C doppler calculations of +- 5Hz
	    itsReqdptol = max(10.0, primaryKeys().asDouble(reqdptolField));
	}	    
    }
    return itsAttached;
}

const MDirection &GBTLO1File::dir(const MVEpoch &time) const
{
    if (itsLO1Table) {
	lo1Row(time);
	Quantity ra(itsRACol(itslo1Row),Unit("deg"));
	Quantity dec(itsDECCol(itslo1Row),Unit("deg"));
	itsCurrentDir.set(MVDirection(ra, dec));
    }
    return itsCurrentDir;
}

const MVFrequency &GBTLO1File::lo1freq(const MVEpoch &time, uInt state) const
{
    if (itsLO1Table) {
	lo1Row(time);
	Double lo1freq = itsLO1FREQCol(itslo1Row);
	Double offset = 0.0;
	if (state < itsFreqoff.nelements()) offset = itsFreqoff[state];
	itsLo1freq = MVFrequency(lo1freq + offset);
    }
    return itsLo1freq;
}

const Double GBTLO1File::vframe(const MVEpoch &time) const
{
    Double result = 0.0;
    if (itsLO1Table) {
	lo1Row(time);
	result = itsVFRAMECol(itslo1Row);
    }
    return result;
}

const Double GBTLO1File::rvsys(const MVEpoch &time) const
{
    Double result = 0.0;
    if (itsLO1Table) {
	lo1Row(time);
	result = itsRVSYSCol(itslo1Row);
    }
    return result;
}

Bool GBTLO1File::isFreqSwitched() const {
    return anyNE(itsFreqoff, 0.0);
}

void GBTLO1File::init()
{
    itsAttached = False;
    itsName = "";

    itsPosition = MVPosition();
    itsRestfrq = MVFrequency();
    itsReqdptol = 10.0;

    itsFreqoff.resize(0);

    itslo1Row = -1;

    itsEquinox = 2000.0;
    itsDirType = MDirection::J2000;

    delete itsSouvel;
    itsSouvel = 0;
    delete itsLO1Table;
    itsLO1Table = 0;

    // itsSouvel points at a null table when not attached
    itsSouvel = new Table;
    AlwaysAssert(itsSouvel, AipsError);
}

void GBTLO1File::setLO1Table(FITSTable &lo1Tab) {
    delete itsLO1Table;
    itsLO1Table = 0;

    itsName = lo1Tab.name();

    // set the primary keywords 
    setPrimaryKeys(lo1Tab.primaryKeywords());

    // convert the table to a Table
    itsLO1Table = GBTFITSBase::tableFromFITS(lo1Tab);
    AlwaysAssert(itsLO1Table, AipsError);

    // attach columns
    itsDMJDCol.attach(*itsLO1Table, "DMJD");
    itsRACol.attach(*itsLO1Table, "RA");
    itsDECCol.attach(*itsLO1Table, "DEC");
    itsLO1FREQCol.attach(*itsLO1Table, "LO1FREQ");
    itsVFRAMECol.attach(*itsLO1Table, "VFRAME");
    itsRVSYSCol.attach(*itsLO1Table, "RVSYS");

    itslo1Row = -1;

    // set reference for itsCurrentDir
    Int radesysField = primaryKeys().fieldNumber("RADESYS");
    String radesys = "FK5";
    if (radesysField >= 0) {
	String radesys = primaryKeys().asString("RADESYS");
	markAsUsed("RADESYS");
    }
    Int equinoxField = primaryKeys().fieldNumber("EQUINOX");
    itsEquinox = 2000.0;
    if (equinoxField >= 0) {
	itsEquinox = primaryKeys().asDouble("EQUINOX");
	markAsUsed("EQUINOX");
    }
    // I believe the only possiblities here are J2000, B1950,
    // and possibly JMEAN (FK5 at equinox) and BMEAN (FK4 at
    // equinox)
    if (radesys == "FK5") {
	if (near(itsEquinox, 2000.0)) {
	    itsDirType = MDirection::J2000;
	} else {
	    itsDirType = MDirection::JMEAN;
	}
    } else if (radesys == "FK4") {
	if (near(itsEquinox, 1950.0)) {
	    itsDirType = MDirection::B1950;
	} else {
	    itsDirType = MDirection::BMEAN;
	}
    } else {
	// default to J2000
	itsDirType = MDirection::J2000;
    }
    itsCurrentDir.set(MDirection::Ref(itsDirType));
}

void GBTLO1File::setLO1State(FITSTable &stateTab) {
    itsState.reattach(stateTab);

    // extract the offset values
    ROScalarColumn<Double> freqoffCol(itsState.table(), "FREQOFF");
    itsFreqoff = freqoffCol.getColumn();
}

void GBTLO1File::setSouvel(FITSTable &souvelTab) {
    delete itsSouvel;
    itsSouvel = 0;

    // convert the table to a Table
    itsSouvel = GBTFITSBase::tableFromFITS(souvelTab);
    AlwaysAssert(itsSouvel, AipsError);
}

void GBTLO1File::lo1Row(const MVEpoch &time) const
{
    if (itsLO1Table && (itslo1Row < 0) || !time.near(itsCurrentTime)) {
	Int inc = 1;
	Int limit = itsLO1Table->nrow()-1;
	if (time.get() < itsCurrentTime.get()) {
	    // need to go backwards
	    inc = -1;
	    limit = 0;
	}
	itslo1Row += inc;
	if (itslo1Row < 0) itslo1Row = 0;
	if (uInt(itslo1Row) >= itsLO1Table->nrow()) itslo1Row = itsLO1Table->nrow()-1;
	itsCurrentTime = time;
	Double curDMJD = time.get();
	Bool found = False;
	while (itslo1Row != limit  && !found) {
	    // if its near, we have a match
	    Double rowTime = itsDMJDCol(itslo1Row);
	    if (near(curDMJD, rowTime)) found = True;
	    else {
		if (inc > 0) {
		    if (curDMJD > rowTime) {
			// too far, back up
			itslo1Row -= 1;
			if (itslo1Row < 0) itslo1Row = 0;
			found = True;
		    }
		} else {
		    if (curDMJD < rowTime) {
			// exactly right, stop
			found = True;
		    }
		}
	    }
	}
	// final check against limits
	if (itslo1Row < 0) itslo1Row = 0;
	else if (uInt(itslo1Row) >= itsLO1Table->nrow()) itslo1Row = itsLO1Table->nrow()-1;
    }
}

Vector<Double> GBTLO1File::firstLO1(Double &factor, MFrequency::Types &refframe,
			    MDoppler::Types &doptype, Double &vsource) const
{
    Vector<Double> result(itsFreqoff.nelements(), 0.0);
    factor = 1.0;
    refframe = MFrequency::TOPO;
    doptype = MDoppler::RADIO;
    vsource = 0.0;
    if (itsLO1Table && itsLO1Table->nrow() > 0 && !itsLO1FREQCol.isNull()) {
	result = itsLO1FREQCol(0);
	// get the VELOCITY from the SOUVEL table's first row
	MRadialVelocity::Types radvType = MRadialVelocity::TOPO;
	if (souvel().nrow() > 0 && souvel().keywordSet().fieldNumber("VELDEF")>=0) {
	    ROScalarColumn<Double> velcol(souvel(),"VELOCITY");
	    vsource = velcol(0);
	    if (deviceVersion() < 3 && baseVersion() < 2) {
		// prior to FITSVERS 1.2 this was in km/s
		vsource *= 1000.0;
	    }
	    String veldef = souvel().keywordSet().asString("VELDEF");
	    if (veldef.length() == 8) {
		// its okay, split it
		String defn = veldef.before(4);
		String frame = veldef.after(4);
		// and translate it to aips++ codes
		if (defn == "VELO") {
		    doptype = MDoppler::BETA;
		} else if (defn == "VRAD") {
		    doptype = MDoppler::RADIO;
		} else if (defn == "VOPT") {
		    doptype = MDoppler::OPTICAL;
		}
		if (frame == "TOP") {
		    radvType = MRadialVelocity::TOPO;
		} else if (frame == "GEO") {
		    radvType = MRadialVelocity::GEO;
		} else if (frame == "HEL" || frame == "BAR") {
		    radvType = MRadialVelocity::BARY;
		} else if (frame == "LSR") {
		    radvType = MRadialVelocity::LSRK;
		} else if (frame == "LSD") {
		    radvType = MRadialVelocity::LSRD;
		} else if (frame == "GAL") {
		    radvType = MRadialVelocity::GALACTO;
		} else {
		    radvType = MRadialVelocity::TOPO;
		}
	    }
	}

	// no need to go any further if radvType is TOPO
	if (radvType != MRadialVelocity::TOPO) {
	    
	    MDoppler vdop(Quantity(vsource,"m/s"), doptype);
	    MRadialVelocity vsys = MRadialVelocity::fromDoppler(vdop, radvType);

	    // set frame information
	    MeasFrame frame(position());
	    MEpoch firstTime = MEpoch(Quantity(itsDMJDCol(0),"d"));
	    frame.set(firstTime);
	    frame.set(dir(firstTime.getValue()));

	    // get the MFrequency equivalent type
	    MFrequency::getType(refframe,
				MRadialVelocity::showType(vsys.getRef().getType()));
	    // convert that to a frequency, same frame
	    MFrequency fsys =
		MFrequency::fromDoppler(vsys.toDoppler(),
					MVFrequency(restfrq()),
					refframe);

	    // convert fsys to a TOPO frequency
	    MFrequency fsysTopo(MFrequency::Convert(fsys, MFrequency::Ref(MFrequency::TOPO, frame))());
	    // factor - when multiplied by fsysTopo gets fsys
	    factor = fsys.getValue().getValue()/fsysTopo.getValue().getValue();
	}	    
    }
    result += itsFreqoff;
    return result;
}
