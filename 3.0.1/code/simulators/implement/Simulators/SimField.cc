//# SimField.h: a pointing field for simulated observations
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

#include <simulators/Simulators/SimField.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSource.h>
#include <ms/MeasurementSets/MSSourceColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/Exceptions/Error.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimField::SimField(const SimField& s) 
    : row_p(s.row_p), src_p(s.src_p), rep_p(s.rep_p), off_p(s.off_p), 
      name_p(s.name_p), code_p(s.code_p), dir_p(s.dir_p)
{ }

SimField& SimField::operator=(const SimField& s) {
    row_p = s.row_p; 
    src_p = s.src_p; 
    rep_p = s.rep_p; 
    name_p = s.name_p; 
    code_p = s.code_p;
    dir_p = s.dir_p;
    off_p = s.off_p;
    return *this;
}

Vector<Double> SimField::offset(const MDirection srcdir, 
				const MDirection flddir) 
{
    Vector<Double> off(2, 0.0);
    MDirection::Convert tofk5(srcdir, MDirection::J2000);
    MDirection src(tofk5());
    MDirection fld(tofk5(flddir));
    MVDirection fv(fld.getValue());
    MVDirection sv(src.getValue());

    off(1) = fv.getLat() - sv.getLat();
    off(0) = fv.getLong() - sv.getLong();
    if (sv.getLat() != 0.0) 
	off(0) /= cos(sv.getLat());

    return off;
}

SimFieldList::SimFieldList(const SimFieldList& t) : n_p(t.n_p), 
    chnk_p(t.chnk_p), rec_p(t.rec_p.nelements())
{
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimField(*(t.rec_p[i]));
}

SimFieldList& SimFieldList::operator=(const SimFieldList& t) { 
    deleteRecs();
    n_p = t.n_p;
    chnk_p = t.chnk_p;
    rec_p.resize(t.rec_p.nelements());
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimField(*(t.rec_p[i]));
    return *this;
}

SimFieldList::SimFieldList(const MSField& fldt, uInt stepz) 
    : n_p(0), chnk_p(stepz), rec_p(fldt.nrow())
{
    initFrom(fldt);
}

void SimFieldList::initFrom(const MSField& fldt) {
    uInt nrow = fldt.nrow();
    if (nrow > 0) {
	String name, code;
	ROMSFieldColumns fldc(fldt);

	for(uInt row=0; row < nrow; row++) {
	    fldc.name().get(row, name);
	    fldc.code().get(row, code);
	    SimField &fld = addField((fldc.sourceId())(row), 
				     fldc.phaseDirMeas(row, (fldc.time())(row)),
				     0.0, 0.0, name, code);
	    fld.setRow(row);
	}
    }
}


void SimFieldList::flush(MSField& fldt, MSSource& srct) {
    MSFieldColumns fldc(fldt);
    uInt row = fldt.nrow();
    SimField *fld;
    SimpleOrderedMap<Int, String> snames("");
    SimpleOrderedMap<Int, Double> times(0.0);
    Vector<MDirection> dir(1);
    Bool srcloaded = False;

    String name;
    Double time;
    for(uInt i=0; i < n_p; i++) {
	if (rec_p[i]->getRow() < 0) {
	    if (! srcloaded) {
		// cache the names of sources to use as default field names
		loadSourceInfo(srct, snames, times);
		srcloaded = True;
	    }

	    fldt.addRow(1);
	    fld = rec_p[i];

	    if (! snames.isDefined(fld->sourceId())) {
		// FIX: warn about missing source reference
		ostringstream ns;
		ns << "FIELD_" << row;
		name = ns.str();
		time = 0;
	    } else {
		ostringstream ns;
		ns << snames(fld->sourceId()) << "_" << row;
		name = ns.str();
		time = times(fld->sourceId());
	    }
	    if ((fld->name()).length() == 0) {
		fldc.name().put(row, name);
	    }
	    else {
		fldc.name().put(row, fld->name());
	    }
	    fldc.time().put(row, time);

	    fldc.code().put(row, fld->code());
	    fldc.numPoly().put(row, 0);
	    fldc.sourceId().put(row, fld->sourceId());
	    fldc.flagRow().put(row, False);

	    dir(0) = fld->direction();
	    fldc.delayDirMeasCol().put(row, dir);
	    fldc.phaseDirMeasCol().put(row, dir);
	    fldc.referenceDirMeasCol().put(row, dir);

	    fld->setRow(row++);
	}
    }
}

uInt SimFieldList::loadSourceInfo(MSSource& srct,
				  SimpleOrderedMap<Int, String> &snames,
				  SimpleOrderedMap<Int, Double> &times)
{
    MSSourceColumns srcc(srct);
    Int id;
    uInt nsrc=0;
    String name;
    Double time;
    for(uInt i=0; i < srct.nrow(); i++) {
	srcc.sourceId().get(i, id);
	if (! snames.isDefined(id)) {
	    srcc.name().get(i, name);
	    snames.define(id, name);

	    srcc.time().get(i, time);
	    times.define(id, time);
	    nsrc++;
	}
    }

    return nsrc;
}

void SimFieldList::throwOutOfRange(uInt legallength, Int index) const {
    ostringstream msg;
    msg << "index out of range of [0," << legallength 
	<< "]: " << index;
    throw AipsError(String(msg));
}

} //# NAMESPACE CASA - END

