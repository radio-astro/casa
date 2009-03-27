//# SimSource.h: a source for simulated observations
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

#include <simulators/Simulators/SimSource.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSource.h>
#include <ms/MeasurementSets/MSSourceColumns.h>
#include <measures/Measures/MPosition.h>
#include <casa/Exceptions/Error.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimSource::SimSource(const String& name, const MDirection& dir, 
		     const String& code, Int spWinId) 
    : spwin_p(spWinId), row_p(-1), id_p(-1), name_p(name), code_p(code), 
      dir_p(dir), time_p(Quantity(0.0, "s")), intv_p(-1.0), dist_p(0.0)
{ }

SimSource::SimSource(const SimSource& t) 
    : spwin_p(t.spwin_p), row_p(t.row_p), id_p(t.id_p), name_p(t.name_p), 
      code_p(t.code_p), dir_p(t.dir_p), time_p(t.time_p), intv_p(t.intv_p),
      dist_p(0.0)
{ }

SimSource& SimSource::operator=(const SimSource& t) {
    spwin_p = t.spwin_p; 
    row_p = t.row_p; 
    id_p = t.id_p; 
    name_p = t.name_p; 
    code_p = t.code_p; 
    dir_p = t.dir_p; 
    time_p = t.time_p; 
    intv_p = t.intv_p;
    dist_p = 0.0; 
    return *this;
}

//#########################################

SimSourceList::SimSourceList(const SimSourceList& s) : n_p(s.n_p), 
    chnk_p(s.chnk_p), rec_p(s.rec_p.nelements())
{ 
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimSource(*(s.rec_p[i]));
}

SimSourceList& SimSourceList::operator=(const SimSourceList& s) {
    deleteRecs();
    n_p = s.n_p;
    chnk_p = s.chnk_p;
    rec_p.resize(s.rec_p.nelements());
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimSource(*(s.rec_p[i]));
    return *this;
}

SimSourceList::SimSourceList(const MSSource& srct, uInt stepz) 
    : n_p(0), chnk_p(stepz), rec_p(srct.nrow(), 0)
{
    initFrom(srct);
}

void SimSourceList::initFrom(const MSSource& srct) {
    uInt nrow = srct.nrow();
    if (nrow > 0) {
	String name, code;
	MDirection dir;
	MEpoch time;
	Quantity intv;
	Int spwid;
	ROMSSourceColumns srcc(srct);

	for(uInt row=0; row < nrow; row++) {
	    srcc.name().get(row, name);
	    srcc.code().get(row, code);
	    srcc.directionMeas().get(row, dir);
	    srcc.spectralWindowId().get(row, spwid);

	    SimSource& src = addSource(name, dir, code, spwid);
	    src.setId((srcc.sourceId())(row));
	    srcc.timeMeas().get(row, time);
	    src.setTime(time);
	    srcc.intervalQuant().get(row, intv);
	    src.setInterval(intv);

	    src.setRow(row);
	}
    }
}

void SimSourceList::flush(MSSource& srct) {
    MSSourceColumns srcc(srct);
    uInt row = srct.nrow();
    SimSource *src;
    Vector<Double> propmo(2, 0.0);

    for(uInt i=0; i < n_p; i++) {
	if (rec_p[i]->getRow() < 0) {
	    srct.addRow(1);
	    src = rec_p[i];

	    if (src->getId() < 0) src->setId(row);
	    srcc.sourceId().put(row, src->getId());
	    srcc.time().put(row, src->timeSec());
	    srcc.interval().put(row, src->interval());
	    srcc.spectralWindowId().put(row, src->spectralWindow());
	    srcc.name().put(row, src->name());
	    srcc.code().put(row, src->code());
	    srcc.directionMeas().put(row, src->direction());
	    srcc.properMotion().put(row, propmo);

	    src->setRow(row++);
	}
    }

}

void SimSourceList::throwOutOfRange(uInt legallength, Int index) const {
    ostringstream msg;
    msg << "index out of range of [0," << legallength 
	<< "]: " << index;
    throw AipsError(String(msg));
}

} //# NAMESPACE CASA - END

