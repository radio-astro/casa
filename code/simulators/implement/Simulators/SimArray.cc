//# SimArray.h: container for containing array configuration description
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
#include <simulators/Simulators/SimArray.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Slice.h>
#include <casa/Quanta/Unit.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

const String SimArray::defbase_p = "SimTelescope";

SimArray::SimArray(const Vector<Double>& x,
			     const Vector<Double>& y,
			     const Vector<Double>& z, const String& unit,
			     const LocalCoord coord, const MPosition& refLoc,
			     const Quantity& diam, const String& mount,
			     const Vector<String>& antNames,
			     const Vector<String>& stationNames,
			     const MPosition *offset,
			     uInt subarrayId)
    : subary_p(0), row_p(-1), xyz_p(), diam_p(diam), offset_p(), mount_p(),
      basename_p(defbase_p), stations_p(), names_p()
{
    if (offset) offset_p = *offset;

    loadarray(x, y, z, unit, coord, refLoc);

    uInt nants = numAnts();
    if (antNames.nelements() != nants) 
	throw AipsError(String("Number of antennas names provided (") + 
			antNames.nelements() + ") does not equal number of " +
			"antennas (" + nants + ")");
    if (stationNames.nelements() != nants) 
	throw AipsError(String("Number of antennas names provided (") + 
			stationNames.nelements() + ") does not equal number " +
			"of antennas (" + nants + ")");

    names_p.set(new Vector<String>(antNames));
    stations_p.set(new Vector<String>(stationNames));
}

SimArray::SimArray(const Vector<Double>& x,
			     const Vector<Double>& y,
			     const Vector<Double>& z, const String& unit,
			     const LocalCoord coord, const MPosition& refLoc,
			     const Quantity& diam, const String& mount,
			     const MPosition *offset,
			     uInt subarrayId)
    : subary_p(0), row_p(-1), xyz_p(), diam_p(diam), offset_p(), mount_p(),
      basename_p(defbase_p), stations_p(), names_p()
{
    if (offset) offset_p = *offset;

    loadarray(x, y, z, unit, coord, refLoc);
}

SimArray::SimArray(const String& name, const MPosition& pos, 
			     const Quantity& diam, const String& mount, 
			     const MPosition *offset, uInt subarrayId)
    : subary_p(0), row_p(-1), xyz_p(3,1), diam_p(diam), offset_p(), mount_p(),
      basename_p(defbase_p), stations_p(new Vector<String>(1)), 
      names_p(new Vector<String>(1))
{
    if (offset) offset_p = *offset;

    MPosition::Convert toitrf(pos, MPosition::ITRF);
    MPosition loc(toitrf());
    xyz_p = loc.get("m").getValue();

    (*stations_p.ptr())(0) = name;
    (*names_p.ptr())(0) = name;
}

SimArray::SimArray(const SimArray& sa) 
    : subary_p(sa.subary_p), row_p(sa.row_p), xyz_p(sa.xyz_p), 
    diam_p(sa.diam_p), offset_p(sa.offset_p), mount_p(sa.mount_p), 
    basename_p(sa.basename_p), stations_p(), names_p()
{
    if (sa.stations_p.ptr()) {
	stations_p.set(new Vector<String>());
	*(stations_p.ptr()) = *(sa.stations_p.ptr());
    }
    if (sa.names_p.ptr()) {
	names_p.set(new Vector<String>());
	*(names_p.ptr()) = *(sa.names_p.ptr());
    }
}

SimArray& SimArray::operator=(const SimArray& s) {
    subary_p = s.subary_p; 
    row_p = s.row_p; 
    xyz_p.resize(s.xyz_p.nrow(), s.xyz_p.ncolumn());
    xyz_p = s.xyz_p; 
    diam_p = s.diam_p; 
    offset_p = s.offset_p; 
    mount_p = s.mount_p; 
    basename_p = s.basename_p; 
    if (s.stations_p.ptr()) 
	stations_p.set(new Vector<String>(*(s.stations_p.ptr()))); 
    else 
	stations_p.clear();
    if (s.names_p.ptr()) 
	names_p.set(new Vector<String>(*(s.names_p.ptr())));
    else 
	names_p.clear();
    return *this;
}

uInt SimArray::antennaPositions(Vector<MPosition>& pos, uInt start) const {
    uInt na = numAnts();

    for(uInt i=0; i < na; i++) {
	pos(start+i).set(MVPosition(xyz_p.column(i)), 
			 MPosition::Ref(MPosition::ITRF));
    }

    return na;
}

uInt SimArray::antennaPositions(Matrix<Double>& pos, uInt start) const {
    uInt na = numAnts();
    pos(Slice(0,3), Slice(start,na)) = xyz_p;
    return na;
}

uInt SimArray::loadNames(Vector<String>& names, const Vector<String> *from,
			 uInt start) const 
{
    uInt n = numAnts();
    if (from) {
	names(Slice(start, n)) = *from;
    }
    else {
	for(uInt i=0; i < n; i++) names(i) = "";
    }
    return n;
}

uInt SimArray::write(MSAntenna &antt, MSAntennaColumns &antc,
			  MPosition::Types ref, Vector<Unit> *units) 
{
    uInt row = antt.nrow();
    Vector<String> *names = names_p.ptr();
    if (!names) {
	names = new Vector<String>(numAnts());
	for(uInt i=0; i < names->nelements(); i++) {
	    ostringstream ns;
	    ns << basename_p << "-" << Int(i+row);
	    (*names)(i) = ns.str();
	}
    }

    Vector<String> *stations = stations_p.ptr();
    if (!stations) {
	stations = new Vector<String>(numAnts());
	for(uInt i=0; i < stations->nelements(); i++) 
	    (*stations)(i) = basename_p + "-" + Int(i+row);
    }

    Slicer range(IPosition(1,row), IPosition(1,numAnts()), IPosition(1,1),
		 Slicer::endIsLength);
    setRow(row);
    antt.addRow(numAnts());

    if (ref != MPosition::ITRF) {
	MPosition itrfpos;
	MPosition outpos;
	MPosition::Convert tooutref;
	if (units) {
	    MVPosition tmp(Quantity(0.0, (*units)(0)), 
			   Quantity(0.0, (*units)(1)), 
			   Quantity(0.0, (*units)(2)));
	    outpos = MPosition(tmp, ref);
	    tooutref = MPosition::Convert(outpos);
	}
	else {
	    outpos = MPosition(MVPosition(), ref);
	    tooutref = MPosition::Convert(MPosition::ITRF, ref);
	}

	uInt i, j;
	for(i=0, j=row; i < numAnts(); i++, j++) {
	    MVPosition xyz(xyz_p(0,i), xyz_p(1,i), xyz_p(2,i));
	    itrfpos = MPosition(xyz, MPosition::ITRF);
	    outpos = tooutref(itrfpos);
	    antc.positionMeas().put(j, outpos);
	}
    }
    else if (units) {
	Quantum<Vector<Double> > out;
	Matrix<Double> xyz(xyz_p.shape());

	out.setUnit("m");
	out.setValue(xyz_p.row(0));
	out.convert((*units)(1));
	xyz.row(0) = out.getValue();

	out.setUnit("m");
	out.setValue(xyz_p.row(1));
	out.convert((*units)(2));
	xyz.row(1) = out.getValue();
	
	out.setUnit("m");
	out.setValue(xyz_p.row(2));
	out.convert((*units)(0));
	xyz.row(2) = out.getValue();

	antc.position().putColumnRange(range, xyz);
    }
    else {
	// the easiest way
	antc.position().putColumnRange(range, xyz_p);
    }

    antc.name().putColumnRange(range, *names);
    antc.station().putColumnRange(range, *stations);

    for(uInt i=0; i < numAnts(); i++, row++) {
	antc.dishDiameterQuant().put(row, diam_p);
	antc.offsetMeas().put(row, offset_p);
	antc.mount().put(row, mount_p);
    }

    return numAnts();
}

void SimArray::loadarray(const Vector<Double>& x, const Vector<Double>& y, 
			      const Vector<Double>& z, const Unit& unit, 
			      SimArray::LocalCoord coord, 
			      const MPosition& refLoc)
{
    uInt nants = x.nelements();
    if (y.nelements() != nants || z.nelements() != nants) 
	throw AipsError(String("Unequal number of antenna position ") +
			"components: [" + nants + ", " + y.nelements() + 
			", " + z.nelements() + "]");
    xyz_p.resize(3,nants);
    xyz_p.row(0) = x;
    xyz_p.row(1) = y;
    xyz_p.row(2) = z;
    if (unit != Unit("m")) 
	xyz_p = (Quantum<Matrix<Double> >(xyz_p, unit)).get("m").getValue();

    switch (coord) {
    case EQUITORIAL:
	loadequitorial(refLoc);
	break;
    case TAN:
	loadtan(refLoc);
	break;
    default:
	loadcircum(refLoc);
	break;
    }
}


void SimArray::loadcircum(const MPosition& refLoc) {
    uInt nants = numAnts();
    MPosition::Convert toitrf(refLoc, MPosition::ITRF);
    MPosition itrf = toitrf();

    Double re = norm(itrf.get("m").getValue());
    Vector<Double> ang = itrf.getAngle("rad").getValue();
    Double lon, lat;

    for(uInt i=0; i < nants; i++) {
	lon = ang(0) + xyz_p(0,i)/(re*cos(ang(1)));
	lat = ang(1) + xyz_p(1,i)/re;
	MPosition pos(Quantity(re+xyz_p(2,i), "m"), 
		      Quantity(lon, "rad"), Quantity(lat, "rad"), 
		      MPosition::ITRF);
	xyz_p.column(i) = pos.get("m").getValue();
    }
}

void SimArray::loadequitorial(const MPosition& refLoc) {
    MPosition::Convert toitrf(refLoc, MPosition::ITRF);
    MPosition itrf = toitrf(refLoc);

    xyz_p += itrf.get("m").getValue();
}

void SimArray::loadtan(const MPosition& refLoc) {
    uInt nants = numAnts();
    MPosition::Convert toitrf(refLoc, MPosition::ITRF);
    MPosition itrf = toitrf(refLoc);

    Vector<Double> xyz = itrf.get("m").getValue();

    Vector<Double> ang = itrf.getAngle("rad").getValue();
    Double d1, d2;
    d1 = ang(0);
    d2 = ang(1);
    Double cosLong = cos(d1);
    Double sinLong = sin(d1);
    Double cosLat = cos(d2);
    Double sinLat = sin(d2);

    for (uInt i=0; i< nants; i++) {

	Double xG1 = -sinLat * xyz_p(1,i) + cosLat * xyz_p(2,i);
	Double yG1 = xyz_p(1,i);

	xyz_p(2,i) = cosLat * xyz_p(1,i)  + sinLat * xyz_p(2,i)  + xyz(2);
	xyz_p(0,i) = cosLong * xG1 - sinLong * yG1  + xyz(0);
	xyz_p(1,i) = sinLong * xG1 + cosLong * yG1  + xyz(1);
    }
}
//#########################################

SimArrayList::SimArrayList(const SimArrayList& other) 
    : n_p(other.n_p), chnk_p(other.chnk_p), nants_p(other.nants_p), 
      rec_p(other.rec_p), byant_p(other.byant_p.nelements())
{
    for(uInt i=0; i < n_p; i++) {
	rec_p[i] = new SimArray(*(other.rec_p[i]));
	for(uInt j=0; j < rec_p[i]->numAnts(); j++) byant_p[j] = rec_p[i];
    }
}

SimArrayList& SimArrayList::operator=(const SimArrayList& other) {
    deleteRecs();
    n_p = other.n_p; 
    chnk_p = other.chnk_p; 
    nants_p = other.nants_p; 
    rec_p.resize(other.rec_p.nelements()); 
    byant_p.resize(other.byant_p.nelements());

    for(uInt i=0; i < n_p; i++) {
	rec_p[i] = new SimArray(*(other.rec_p[i]));
	for(uInt j=0; j < rec_p[i]->numAnts(); j++) byant_p[j] = rec_p[i];
    }
    return *this;
}

SimArrayList::SimArrayList(const MSAntenna& antt, Bool uniform, uInt stepz)
    : n_p(0), chnk_p(stepz), nants_p(0), rec_p(antt.nrow(), 0), 
      byant_p(antt.nrow(), 0)
{
    initFrom(antt, uniform);
}

void SimArrayList::initEclectic(const MSAntenna& antt) {
    uInt nrow = antt.nrow();
    if (nrow > 0) {
	String name, station, type, mount;
	MPosition pos;
	MPosition off;
	Quantity diam;
	ROMSAntennaColumns antc(antt);

	for(uInt row=0; row < nrow; row++) {
	    if ((antc.flagRow())(row)) continue;

	    antc.name().get(row, name);
	    antc.station().get(row, station);
	    antc.type().get(row, type);
	    antc.mount().get(row, mount);
	    antc.positionMeas().get(row, pos);
	    antc.offsetMeas().get(row, off);
	    antc.dishDiameterQuant().get(row, diam);

	    addAntenna(name, pos, diam, mount, &off).setRow(row);
	}
    }
}

uInt SimArrayList::flush(MSAntenna& antt) {
    MSAntennaColumns antc(antt);
    uInt nrow = antt.nrow();
    Vector<Unit> units;
    MPosition::Types ref = MPosition::ITRF;
    Bool unitsok;

    // figure out what position reference type we need to use
    if (nrow == 0) {

 	// empty table; set to ITRF
 	antc.positionMeas().setDescRefCode(MPosition::ITRF);
 	Vector<Unit> units(3, "m");
 	antc.positionMeas().setDescUnits(units);
 	unitsok = True;
    }
    else {
	Vector<String> u = antc.positionQuant().getUnits();
 	if (u.nelements()!=3 || u(0)!="rad" || u(1)!="rad" || u(2)!="m") {
	    units.resize(3);
	    for(uInt k=0; k<3; k++) units(k) = u(k);
	}
	MPosition tmp;
	antc.positionMeas().get(0, tmp);
	ref = MPosition::castType(tmp.type());
    }

    uInt row, i;
    Vector<Unit> *useunits = (units.nelements() == 3) ? &units : 0;
    for(i=0, row=0; i < n_p; i++, row++) {
	if (rec_p[i]->getRow() < 0) {
	    row += rec_p[i]->write(antt, antc, ref, useunits);
	}
    }

    return row;
}

uInt SimArrayList::antennaPositions(Vector<MPosition>& pos) const {
    pos.resize(nants_p);
    uInt i, a;
    for(i=0, a=0; i < n_p; i++) {
	a += rec_p[i]->antennaPositions(pos, a);
    }
    return a;
}

uInt SimArrayList::antennaPositions(Matrix<Double>& pos) const {
    pos.resize(3,nants_p);
    uInt i, a;
    for(i=0, a=0; i < n_p; i++) {
	a += rec_p[i]->antennaPositions(pos, a);
    }
    return a;
}

uInt SimArrayList::antennaNames(Vector<String>& names) const {
    names.resize(nants_p);
    uInt i, a;
    for(i=0, a=0; i < n_p; i++) {
	a += rec_p[i]->antennaNames(names, a);
    }
    return a;
}

uInt SimArrayList::stationNames(Vector<String>& names) const {
    names.resize(nants_p);
    uInt i, a;
    for(i=0, a=0; i < n_p; i++) {
	a += rec_p[i]->stationNames(names, a);
    }
    return a;
}

void SimArrayList::throwOutOfRange(uInt legallength, Int index) const {
    ostringstream msg;
    msg << "index out of range of [0," << legallength 
	<< "]: " << index;
    throw AipsError(String(msg));
}

} //# NAMESPACE CASA - END

