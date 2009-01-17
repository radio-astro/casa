//# SimDataDesc.cc: a container for Data Description records
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
#include <simulators/Simulators/SimDataDesc.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimDataDesc::SimDataDesc(uInt polId, uInt spw, Bool upto) 
    : row_p(-1), pol_p(polId), spwids_p(upto ? spw : 1)
{ 
    if (upto) {
	for(uInt i=0; i < spw; i++) spwids_p(i) = i;
    }
    else {
	spwids_p(0) = 1;
    }
}

SimDataDesc::SimDataDesc(uInt polId, const Vector<uInt>& spwids) 
    : row_p(-1), pol_p(polId), spwids_p(spwids)
{ }

SimDataDesc::SimDataDesc(const SimDataDesc& s) 
    : row_p(s.row_p), pol_p(s.pol_p), spwids_p(s.spwids_p)
{ }

SimDataDesc& SimDataDesc::operator=(const SimDataDesc& s) {
    row_p = s.row_p;
    pol_p = s.pol_p; 
    spwids_p.resize(s.spwids_p.nelements());
    spwids_p = s.spwids_p;
    return *this;
}

//###########################################

SimDataDescList::SimDataDescList(const SimDataDescList& t) : n_p(t.n_p), 
    chnk_p(t.chnk_p), nid_p(t.nid_p), rec_p(t.rec_p.nelements()), 
    bydd_p(t.bydd_p.nelements())
{
    for(uInt i=0; i < n_p; i++) {
	rec_p[i] = new SimDataDesc(*(t.rec_p[i]));
	for(uInt j=0; j < rec_p[i]->numWindows(); j++) bydd_p[j] = rec_p[i];
    }
}

SimDataDescList& SimDataDescList::operator=(const SimDataDescList& t) {
    deleteRecs();
    n_p = t.n_p; 
    chnk_p = t.chnk_p;  
    nid_p = t.nid_p; 

    rec_p.resize(t.rec_p.nelements());
    bydd_p.resize(t.bydd_p.nelements());

    for(uInt i=0; i < n_p; i++) {
	rec_p[i] = new SimDataDesc(*(t.rec_p[i]));
	for(uInt j=0; j < rec_p[i]->numWindows(); j++) bydd_p[j] = rec_p[i];
    }
    return *this;
}


SimDataDescList::SimDataDescList(const MSDataDescription& ddt, uInt stepz)
    : n_p(0), chnk_p(stepz), nid_p(0), rec_p(ddt.nrow()), 
      bydd_p()
{
    initFrom(ddt);
}

void SimDataDescList::initFrom(const MSDataDescription& ddt) {
    uInt nrow = ddt.nrow();
    if (nrow > 0) {
	Int polid;
	SimpleOrderedMap<Int, Vector<uInt> > pol2spw(Vector<uInt>(nrow,0),2);
	SimpleOrderedMap<Int, uInt> pol2row(0,2);
	SimpleOrderedMap<Int, uInt> pol2nrow(0,2);
	ROMSDataDescColumns ddc(ddt);

	for(uInt row=0; row < nrow; row++) {
	    polid = (ddc.polarizationId())(row);
	    (pol2spw(polid))(pol2nrow(polid)) = 
		static_cast<uInt>((ddc.spectralWindowId())(row));
	    if (! pol2row.isDefined(polid)) pol2row(polid) = row;
	    (pol2nrow(polid))++;
	}

	for(uInt i=0; i < pol2nrow.ndefined(); i++) {
	    polid = pol2nrow.getKey(i);
	    pol2spw(polid).resize(pol2nrow(polid));
	    SimDataDesc& dd = addDataDesc(static_cast<uInt>(polid), 
					  pol2spw(polid));
	    dd.setRow(pol2row(polid));
	}
    }
}

void SimDataDescList::flush(MSDataDescription& ddt, uInt npol, uInt nspw) {
    if (rec_p.nelements() == 0) return;

    LogIO log(LogOrigin("SimDataDesc", "flush()", WHERE));
    MSDataDescColumns ddc(ddt);
    SimDataDesc *dd;
    uInt row = ddt.nrow();

    for(uInt i=0; i < n_p; i++) {
	if (rec_p[i]->getRow() < 0) {
	    dd = rec_p[i];
	    const Vector<uInt> wins = dd->spectralWindowIds();

	    if (dd->polId() < npol) {
		for(uInt j=0; j < wins.nelements(); j++) {
		    if (wins(j) < nspw) {

			if (dd->getRow() < 0) dd->setRow(row);
			ddt.addRow(1);
			ddc.polarizationId().put(row, dd->polId());
			ddc.spectralWindowId().put(row, wins(j));
			ddc.flagRow().put(row, False);
			row++;
		    }
		    else {
			log << LogIO::WARN << "Dropping Data Description for "
			    << "non-existent SPECTRAL_WINDOW_ID=" 
			    << wins(j) << LogIO::POST;
		    }
		}
	    }
	    else {
		log << LogIO::WARN << "Dropping Data Description(s) for "
		    << "non-existent POLARIZATION_ID=" << dd->polId() 
		    << LogIO::POST;
	    }
	}
    }
}

void SimDataDescList::add(SimDataDesc *dop) {
    if (n_p == rec_p.nelements()) {
	rec_p.resize(n_p+chnk_p);
	for(uInt i=n_p; i < rec_p.nelements(); i++) rec_p[i] = 0;
    }
    rec_p[n_p++] = dop;

    bydd_p.resize(nid_p+dop->numWindows());
    for(uInt i=0; i < dop->numWindows(); i++, nid_p++) 
	bydd_p[nid_p] = dop;
}

uInt SimDataDescList::loadSpWinMap(Vector<uInt>& out, Int npol, Int nspw) {
    uInt i, j, k, nw;
    SimDataDesc *dd;

    out.resize(numIds());
    for(i=0, j=0; i < n_p; i++) {
	dd = rec_p[i];
	if (npol < 0 || npol > static_cast<Int>(dd->polId())) {
	    nw = rec_p[i]->numWindows();
	    const Vector<uInt>& spwids = rec_p[i]->spectralWindowIds();
	    for(k=0; k < nw; k++) {
		if (nspw < 0 || nspw > static_cast<Int>(spwids(k))) 
		    out(j++) = spwids(k);
	    }
	}
    }
    if (numIds() != j) out.resize(j);
    return j;
}

void SimDataDescList::throwOutOfRange(uInt legallength, Int index) const {
    ostringstream msg;
    msg << "index out of range of [0," << legallength 
	<< "]: " << index;
    throw AipsError(String(msg));
}

} //# NAMESPACE CASA - END

