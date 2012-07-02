//# SimpleSimulator.cc: writes out a simulated MS with blank data
//# Copyright (C) 2002,2003
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

#include <simulators/Simulators/SimpleSimulator.h>
#include <simulators/Simulators/SimScan.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayMath.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVAngle.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSDerivedValues.h>
#include <casa/Logging/LogIO.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/StandardStMan.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/Time.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// not publicly available
SimpleSimulator::SimpleSimulator() : delms_p(False), scannum_p(0), 
    obsnum_p(0), autocorrwt_p(0.0), blockFrac_p(0.0), obs_p(), ms_p(0), 
    start_p(Quantity(0, "s")), end_p(Quantity(0, "s"))
{ }

SimpleSimulator::SimpleSimulator(SimObservations *obs) 
    : delms_p(False), scannum_p(0), obsnum_p(0), autocorrwt_p(0.0), 
      blockFrac_p(0.0), obs_p(obs), ms_p(obs->ms()), 
      start_p(Quantity(0, "s")), end_p(Quantity(0, "s")) 
{ }

SimpleSimulator::SimpleSimulator(SimObservations *obs, const String& msname) 
    : delms_p(True), scannum_p(0), obsnum_p(0), autocorrwt_p(0.0), 
      blockFrac_p(0.0), obs_p(obs), ms_p(0), start_p(Quantity(0, "s")), 
      end_p(Quantity(0, "s"))
{ 
    TableDesc td = MS::requiredTableDesc();
    MS::addColumnToDesc(td, MS::DATA, 2);

    td.defineHypercolumn("TiledData",3,
			 stringToVector(MS::columnName(MS::DATA)));
    td.defineHypercolumn("TiledFlag",3,
			 stringToVector(MS::columnName(MS::FLAG)));
    td.defineHypercolumn("TiledFlagCategory",4,
			 stringToVector(MS::columnName(MS::FLAG_CATEGORY)));
    td.defineHypercolumn("TiledUVW",2,
			 stringToVector(MS::columnName(MS::UVW)));
    td.defineHypercolumn("TiledWgt",2,
			 stringToVector(MS::columnName(MS::WEIGHT)));
    td.defineHypercolumn("TiledSigma",2,
			 stringToVector(MS::columnName(MS::SIGMA)));

    SetupNewTable newtab(msname, td, Table::NewNoReplace);

    // Set the default Storage Manager to be the Incr one
    IncrementalStMan incrStMan ("ISMData");;
    newtab.bindAll(incrStMan, True);
    StandardStMan aipsStMan;
    newtab.bindColumn(MS::columnName(MS::ANTENNA1), aipsStMan);
    newtab.bindColumn(MS::columnName(MS::ANTENNA2), aipsStMan);

    Int tileSize = 32;
    TiledShapeStMan tiledStMan1("TiledData",
				IPosition(3,1,tileSize,16384/tileSize));
    TiledShapeStMan tiledStMan1f("TiledFlag",
				 IPosition(3,1,tileSize,16384/tileSize));
    TiledShapeStMan tiledStMan1fc("TiledFlagCategory",
				  IPosition(4,1,tileSize,1,16384/tileSize));
    TiledColumnStMan tiledStMan3("TiledUVW", IPosition(2,3,1024));
    TiledShapeStMan tiledStMan4("TiledWgt", IPosition(2,1,1024));
    TiledShapeStMan tiledStMan5("TiledSigma", IPosition(2,1,1024));

    // Bind the DATA, FLAG columns to the tiled stman
    newtab.bindColumn(MS::columnName(MS::DATA),tiledStMan1);
    newtab.bindColumn(MS::columnName(MS::FLAG),tiledStMan1f);
    newtab.bindColumn(MS::columnName(MS::FLAG_CATEGORY),tiledStMan1fc);
    newtab.bindColumn(MS::columnName(MS::UVW),tiledStMan3);
    newtab.bindColumn(MS::columnName(MS::WEIGHT),tiledStMan4);
    newtab.bindColumn(MS::columnName(MS::SIGMA),tiledStMan5);

    ms_p = new MeasurementSet(newtab);

    try {
	// create all required subtables (each will have 0 rows)
	ms_p->createDefaultSubtables(Table::New);
 
	// add the SOURCE table
        TableDesc tdesc = MSSource::requiredTableDesc();
        MSSource::addColumnToDesc(tdesc, MSSourceEnums::REST_FREQUENCY, 1);
	SetupNewTable sourceSetup(ms_p->sourceTableName(),tdesc,Table::New);
	ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
				       Table(sourceSetup));

	// add the DOPPLER table
	SetupNewTable dopplerSetup(ms_p->dopplerTableName(),
				   MSDoppler::requiredTableDesc(),Table::New);
	ms_p->rwKeywordSet().defineTable(MS::keywordName(MS::DOPPLER),
				       Table(dopplerSetup));

	// add optional columns to SPECTRAL_WINDOW table
        ms_p->spectralWindow().addColumn(
            ScalarColumnDesc<Int>(
                MSSpectralWindow::columnName(MSSpectralWindow::DOPPLER_ID),
                MSSpectralWindow::columnStandardComment(
                    MSSpectralWindow::DOPPLER_ID)));

	TableInfo& info(ms_p->tableInfo());
	info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
	info.setSubType(String("Simulated"));
	info.readmeAddLine(String("This is a measurement set Table holding ") +
			   "simulated observations");
    }
    catch (...) { delete ms_p; throw; }

    MSColumns msc(*ms_p);

    // set the flag catagories
    Vector<String> flgCat(3);
    flgCat(0)="FLAG_CMD";
    flgCat(1)="SHADOWING";
    flgCat(2)="ELEVATION_LIMIT";
    msc.flagCategory().rwKeywordSet().define("CATEGORY",flgCat);

    attach(ms_p);
}

SimpleSimulator::SimpleSimulator(SimObservations *obs, MeasurementSet& ms)
    : delms_p(True), scannum_p(0), obsnum_p(0), autocorrwt_p(0.0), 
      blockFrac_p(0.0), obs_p(obs), ms_p(0), start_p(Quantity(0)), 
      end_p(Quantity(0))
{ 
    throw AipsError("Extending an MS currently not supported");
    ms_p = new MeasurementSet(ms);
    attach(ms_p);
}

SimpleSimulator::~SimpleSimulator() { 
    if (delms_p && ms_p) delete ms_p;
}

void SimpleSimulator::attach(MeasurementSet *ms) {

    // make sure we have SOURCE tables
    if (! ms->keywordSet().isDefined("SOURCE")) {
        TableDesc tdesc = MSSource::requiredTableDesc();
        MSSource::addColumnToDesc(tdesc, MSSourceEnums::REST_FREQUENCY, 1);
        SetupNewTable sourceSetup(ms->sourceTableName(),tdesc,Table::New);
        ms->rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
                                       Table(sourceSetup));
    }

    // make sure we have DOPPLER tables
    if (! ms->keywordSet().isDefined("DOPPLER")) {
        SetupNewTable dopplerSetup(ms->dopplerTableName(),
                                   MSDoppler::requiredTableDesc(),Table::New);
        ms->rwKeywordSet().defineTable(MS::keywordName(MS::DOPPLER),
                                       Table(dopplerSetup));
    }

    // attach the ms
    obs_p->attach(*ms, True);
}

Bool SimpleSimulator::writeObsRecord() {
    if (start_p.getValue().get() == 0) return False;
    if (! ms_p) throw new AipsError("null MeasurementSet object");

    MSObservation& obst = ms_p->observation();
    MSObservationColumns obsc(obst);
    uInt row = obst.nrow();
    obst.addRow(1);

    obsc.telescopeName().put(row, obs_p->telescope()->getName());

    Vector<Double> range(2);
    range(0) = start_p.get("s").getValue("s");
    range(1) = end_p.get("s").getValue("s");
    obsc.timeRange().put(row, range);

    obsc.observer().put(row, "");

    Vector<String> log(1, String("Simulated Scans"));
    obsc.log().put(row, log);

    obsc.scheduleType().put(row, String("SimpleSimulator"));

    log(0) = "";
    obsc.schedule().put(row, log);

    Time date;
    MEpoch now(MVEpoch(date.modifiedJulianDay()), MEpoch::Ref(MEpoch::UTC));
    obsc.releaseDateMeas().put(row, now);

    obsc.flagRow().put(row, False);
    obsnum_p++;
    return True;
}

void SimpleSimulator::write(const MEpoch& start, const Quantity& duration,
			    const SimScan& scan) 
{
    LogIO os(LogOrigin("SimpleSimulator", "write()", WHERE));

    if (! obs_p) throw new AipsError("null SimObservations object");
    if (! ms_p) throw new AipsError("null MeasurementSet object");
    if (! scan.check(*obs_p)) 
	throw AipsError(String("incomplete SimScan definition; ") + 
			"no observations possible");

    // make sure subtables are up to date
    obs_p->flush();

    MSColumns msc(*ms_p);
    msc.setEpochRef(MEpoch::UTC, False);

    start_p = start;
    end_p = MEpoch(start_p.get("s") + duration, start_p.getRef());
    Double Tstart = start.get("s").getValue("s");
    Double Tend = end_p.get("s").getValue("s");
    Double inttime = scan.intTimeSec();
    Double gaptime = scan.gapTimeSec();
    Double time = Tstart;
    Double gmst, ra, dec;

    Vector<uInt> dd2spw;
    obs_p->loadDataDesc2SpwId(dd2spw);

    Vector<Double> uvw(3);
    SimTelescope *tel = obs_p->telescope();
    Matrix<Double> xyz;
    Vector<MPosition> mpos;
    tel->antennaPositions(xyz);
    tel->antennaPositions(mpos);
    MSDerivedValues msd;
    msd.setAntennaPositions(mpos);
    msd.setObservatoryPosition(tel->getRefLocation());

    Double diamMax2 = max( msc.antenna().dishDiameter().getColumn() );
    diamMax2 *= (msc.antenna().dishDiameter())(0) /
                         (msc.antenna().dishDiameterQuant())(0).getValue("m");
    diamMax2 = diamMax2*diamMax2;

    const Vector<uInt> &fid = scan.fields();
    const Vector<uInt> &aid = scan.antennas();
    const Vector<uInt> &did = scan.dataDescriptions();
    uInt f = fid.nelements();
    uInt nant = aid.nelements();
    uInt nbase = nant*(nant-1)/2;
    if (autocorrwt_p > 0.0) nbase += nant;
    uInt feed1, feed2, ant1, ant2;
    Bool firstbase = True;
    uInt row = ms_p->nrow();
    uInt rowBlock = row;
    Float sigma, wt;
    Double diam1, diam2, elev, blockage1, blockage2;
    uInt rep = 0, repLimit = 1;
    while (time < Tend) {

	// adjust time
	if (time+inttime > Tend) inttime = Tend-time;
	time += inttime/2;

        MEpoch epUT1 (Quantity(time/C::day, "d"), MEpoch::UT1);
	MEpoch::Convert epGMST1(epUT1, MEpoch::Ref(MEpoch::GMST1));
	gmst = epGMST1().get("d").getValue("d");
	gmst = (gmst - Int(gmst)) * C::_2pi;  // Into Radians
	msd.setEpoch(epUT1);

	// increment to the next field
//	if (++rep >= scan.fieldRepetition()) {
	if (++rep >= repLimit) {
	    if (++f >= fid.nelements()) f = 0;
	    const SimField& field = obs_p->field(fid(f));

	    rep = 0;
	    repLimit = field.repetition();
	    if (repLimit == 0) {
		repLimit = scan.fieldRepetition();
		if (repLimit == 0) repLimit = 1;
	    }

	    const MDirection& dir = field.direction();
	    msd.setFieldCenter(dir);
	    ra = dir.getAngle().getValue()(0);
	    dec = dir.getAngle().getValue()(1);
	}

        Matrix<Double> firstRot(Rot3D(2,(gmst-ra +0.25*C::_2pi)));
	Matrix<Double> secondRot(Rot3D(0,C::pi_2-dec));

	ms_p->addRow(did.nelements()*nbase);

	Vector<Bool> isTooLow(nant, False);
	Vector<Bool> isShadowed(nant, False);

	// iterate through the data descriptions (spectral window/polarization
	// combinations)
	for(uInt d=0; d < did.nelements(); d++) {
	    rowBlock = row;

	    const SimDataDesc& ddesc = obs_p->dataDesc(did(d));
	    uInt polid = ddesc.polId();
	    uInt spwid = dd2spw(did(d));

	    Matrix<Complex> data(tel->getPolSetup(polid).numTypes(), 
				 tel->getWindowDesc(spwid).numChannels(), 
				 Complex(0.0, 0.0));
	    Matrix<Bool> flags(tel->getPolSetup(polid).numTypes(), 
			       tel->getWindowDesc(spwid).numChannels(), False);
	    Cube<Bool> flagcat(tel->getPolSetup(polid).numTypes(), 
			       tel->getWindowDesc(spwid).numChannels(),3,False);
	    Vector<Float> noise(tel->getPolSetup(polid).numTypes());

	    feed1 = tel->getPolSetup(polid).feed1();
	    feed2 = tel->getPolSetup(polid).feed2();

	    // iterate through the antennas
	    firstbase = True;
	    for(uInt a1=0; a1 < nant; a1++) {

		// check for elevation limit
		if (d == 0) {
		    msd.setAntenna(aid(a1));
		    elev = msd.azel().getAngle("rad").getValue("rad")(1);
		    if (row==0)
			os << LogIO::NORMAL << "elev: " 
			   << MVAngle(Quantity(elev,"rad"))
			                 .string(MVAngle::ANGLE,6)
			   << LogIO::POST;
		    if (elev < elevLimit_p.getValue("rad")) 
			isTooLow(a1) = True;
		}

		for(uInt a2=a1; a2< nant; a2++) {
		    ant1 = aid(a1);
		    ant2 = aid(a2);
		    diam1 = tel->getAntennaDesc(ant1).diameter().getValue("s");
		    diam2 = tel->getAntennaDesc(ant2).diameter().getValue("s");
		    if (ant1 != ant2 || autocorrwt_p > 0.0) {
			if (ant1 == ant2) {
			    uvw = 0.0;
			}
			else {
			    uvw(0) = xyz(0,ant2)-xyz(0,ant1);
			    uvw(1) = xyz(1,ant2)-xyz(1,ant1);
			    uvw(2) = xyz(2,ant2)-xyz(2,ant1);
			    uvw = product(firstRot, uvw);
			    uvw = product(secondRot, uvw);

			    // deal with shadowing
			    if (d == 0 && a1 == 0 && ant1 != ant2) {
				blockage(blockage1, blockage2,
					 uvw, diam1, diam2);
				if (blockage1 > blockFrac_p) 
				    isShadowed(a1) = True;
				if (blockage2 > blockFrac_p) 
				    isShadowed(a2) = True;
			    }
			}

			// load up the table
			if (firstbase) {
			    msc.scanNumber().put(row, scannum_p);
			    msc.observationId().put(row, obsnum_p);
			    msc.fieldId().put(row, fid(f));
			    msc.processorId().put(row, scan.processorId());
			    msc.dataDescId().put(row, did(d));
			    msc.time().put(row, time);
			    msc.timeCentroid().put(row, time);
			    msc.arrayId().put(row, scan.arrayId());
			    msc.interval().put(row, inttime);
			    msc.exposure().put(row, inttime);
			    firstbase = False;
			}
			msc.uvw().put(row, uvw);
			msc.antenna1().put(row, ant1);
			msc.antenna2().put(row, ant2);
			msc.feed1().put(row, feed1);
			msc.feed2().put(row, feed2);
			msc.data().put(row, data);
			msc.flag().put(row, flags);
			msc.flagCategory().put(row, flagcat);
			msc.flagRow().put(row, False);

			// set the weights.  The sigma is calculated as a 
			// fraction of the max product of the diameters
			sigma = diamMax2 / (diam1 * diam2);
			noise = sigma;
			msc.sigma().put(row, noise);
			wt = 1/square(sigma);
			if (ant1 == ant2) wt *= autocorrwt_p;
			noise = wt;
			msc.weight().put(row, noise);

			row++;
		    }
		}
	    }

	    // go back and flag based on shadowing and elevation limit
	    row = rowBlock;
	    Bool shadowed, toolow;
	    flags = True;
	    for(uInt a1=0; a1 < nant; a1++) {
		for(uInt a2=a1; a2< nant; a2++) {
		    if (a1 != a2 || autocorrwt_p > 0.0) {
			shadowed = toolow = False;
			if (isTooLow(a1) || isTooLow(a2)) 
			    flagcat.xyPlane(2) = toolow = True;
			if (isShadowed(a1) || isShadowed(a2))
			    flagcat.xyPlane(1) = shadowed = True;

			if (shadowed || toolow) {
			    msc.flagCategory().put(row, flagcat);
			    msc.flag().put(row, flags);
			    msc.flagRow().put(row, True);
			}
			row++;
		    }
		}
	    }
	}

	time += inttime/2 + gaptime;
    }

    scannum_p++;
}

// Note: implementation taken from MSSimulator.cc
void SimpleSimulator::blockage(Double &fraction1, Double &fraction2,
			       const Vector<Double>& uvw, 
			       const Double diam1, const Double diam2) 
{
    Double separation = sqrt( square(uvw(0)) + square(uvw(1)) );
    Double rmin = 0.5 * min(abs(diam1),abs(diam2));
    Double rmax = 0.5 * max(abs(diam1),abs(diam2));

    if (separation >= (rmin+rmax)) {
	fraction1 = 0.0;
	fraction2 = 0.0;
    } 
    else if ((separation+rmin) <= rmax) {
	fraction1 = min(1.0, square(abs(diam2)/abs(diam1)));
	fraction2 = min(1.0, square(abs(diam1)/abs(diam2)));
    } 
    else {
	Double c = separation/(0.5 * abs(diam1));
	Double s = abs(diam2)/abs(diam1);        
	Double sinb=sqrt(2.0 * (square(c*s)+square(c)+square(s)) - 
			 pow(c,4.0)-pow(s,4.0)-1.0)
	            / (2.0 * c);
	Double sina=sinb/s;

	//  Due to roundoff, sina or sinb might be ever so slightly larger 
	//  than 1 in the case of unequal radii, with the center of one 
	//  antenna pattern inside the other:
	sinb=min(1.0, sinb);
	sina=min(1.0, sina);
    
	Double b=asin(sinb);
	Double a=asin(sina);
	Double area=(square(s)*a+b)-(square(s)*sina*cos(a)+sinb*cos(b));
	fraction1 = area/C::pi;
	fraction2 = fraction1/square(s);
    }

    // if antenna1 is in behind, w is > 0, 2 is NOT shadowed
    if (uvw(2) > 0.0) fraction2 = 0.0;

    // if antenna1 is in front, w is < 0, 1 is NOT shadowed
    if (uvw(2) < 0.0) fraction1 = 0.0;

    return;
}


} //# NAMESPACE CASA - END

