//# SimFeed.h: container for containing feed descriptions in memory
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
#include <simulators/Simulators/SimFeed.h>

#include <ms/MeasurementSets/MSFeed.h>
#include <ms/MeasurementSets/MSFeedColumns.h>
#include <casa/Logging/LogIO.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimFeed& SimFeed::operator=(const SimFeed& f) {
    antId_p = f.antId_p; 
    spwId_p = f.spwId_p; 
    beamId_p = f.beamId_p; 
    row_p = -1;
    pols_p = f.pols_p; 

    deletePointers();
    if (f.position_p) position_p = f.position_p; 
    if (f.beamOff_p) beamOff_p = f.beamOff_p; 
    if (f.polResp_p) polResp_p = f.polResp_p; 
    if (f.angle_p) angle_p = f.angle_p;
    return *this;
}

uInt SimFeed::write(MSFeed& msf, MSFeedColumns &msfc, uInt nant) {

    Int a = (antId_p < 0) ? 0 : antId_p;
    Int na = (antId_p < 0) ? nant : a+1;
    uInt row = msf.nrow();

    Matrix<Double> defboff(2, nrecp_p, 0);
    Matrix<Complex> defpolresp(nrecp_p, nrecp_p, Complex(0.0, 0.0));
    defpolresp.diagonal() = Complex(1.0, 0.0);
    MPosition defpos;
    Vector<Double> defang(nrecp_p, 0.0);

    Vector<String> ptype(pols_p.length());
    uInt p;
    String::const_iterator c;
    for(p=0, c=pols_p.begin(); 
	p < ptype.nelements(); 
	p++, ++c) 
    {
	ptype(p) = *c;
    }

    msf.addRow(na-a);

    setRow(row);
    for (; a < na; a++, row++) {
	msfc.antennaId().put(row, a);
	msfc.feedId().put(row, feedId_p);
	msfc.spectralWindowId().put(row, spwId_p);
	msfc.time().put(row, time_p);
	msfc.interval().put(row, intv_p);
	msfc.numReceptors().put(row, nrecp_p);
	msfc.polarizationType().put(row, ptype);
	msfc.beamId().put(row, beamId_p);
	msfc.beamOffset().put(row, (beamOff_p) ? *beamOff_p : defboff);
	msfc.polResponse().put(row, (polResp_p) ? *polResp_p : defpolresp);
	msfc.positionMeas().put(row, (position_p) ? *position_p : defpos);
	msfc.receptorAngle().put(row, (angle_p) ? *angle_p : defang);
    }

    return na-a;
}

//#########################################

SimFeedList::SimFeedList(const MSFeed& fdt, uInt stepsz) 
    : n_p(0), chnk_p(stepsz), recs_p(fdt.nrow(), 0)
{
    initFrom(fdt);
}

SimFeedList& SimFeedList::operator=(const SimFeedList& f) {
    deleteRecs();
    n_p = f.n_p;
    chnk_p = f.chnk_p;
    recs_p.resize(f.recs_p.nelements());
    for(uInt i=0; i < recs_p.nelements(); i++)
	recs_p[i] = new SimFeed(*(f.recs_p[i]));
    return *this;
}

void SimFeedList::initFrom(const MSFeed& fdt) {
    Int fid, antid, spwid, nrecp;
    uInt nrow = fdt.nrow();
    ROMSFeedColumns fdc(fdt);
    String recps;
    String rlxy = "RLXY";
    MPosition pos;
    LogIO log;

    for(uInt row=0; row < nrow; row++) {
	fdc.feedId().get(row, fid);
	fdc.antennaId().get(row, antid);
	fdc.spectralWindowId().get(row, spwid);
	fdc.numReceptors().get(row, nrecp);

	Vector<String> ptypes;
	fdc.polarizationType().get(row, ptypes);
	recps = "";
	for(Int i=0; i < nrecp; i++) {
	    if (ptypes(i).length() > 1 || ! rlxy.contains(ptypes(i))) {
		log << LogIO::WARN << "Unrecognized polarization type: "
		    << ptypes(i) << ": skipping";
		continue;
	    }
	    recps += ptypes(i);
	}

	SimFeed& feed = addFeed(nrecp, recps);
	feed.setKey(fid, antid, spwid, (fdc.time())(row), 
		    (fdc.interval())(row));
	feed.setBeamID((fdc.beamId())(row));
	fdc.receptorAngle().get(row, feed.receptorAngle());
	fdc.polResponse().get(row, feed.polResponse());
	fdc.beamOffset().get(row, feed.beamOffset());
	fdc.positionMeas().get(row, pos);
	feed.setPosition(pos);
    }
}

uInt SimFeedList::write(MSFeed &msf, uInt nants) const {
    MSFeedColumns msfc(msf);

    uInt newrows = 0;    // countRows(nants);
    for(uInt i=0; i < n_p; i++) 
	newrows += recs_p[i]->write(msf, msfc, nants);
    return newrows;
}

uInt SimFeedList::flush(MSFeed &msf, uInt nants) const {
    MSFeedColumns msfc(msf);

    uInt newrows = 0;  // countRows(nants, True);
    for(uInt i=0; i < n_p; i++) {
	if (recs_p[i]->getRow() < 0)
	    newrows += recs_p[i]->write(msf, msfc, nants);
    }
    return newrows;
}

void SimFeedList::throwOutOfRange(uInt legallength, Int index) const {
    ostringstream msg;
    msg << "index out of range of [0," << legallength 
	<< "]: " << index;
    throw AipsError(String(msg));
}

} //# NAMESPACE CASA - END

