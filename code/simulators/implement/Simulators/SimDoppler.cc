//# SimDoppler.h: a source for simulated observations
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

#include <simulators/Simulators/SimDoppler.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSDoppler.h>
#include <ms/MeasurementSets/MSDopplerColumns.h>
#include <ms/MeasurementSets/MSSource.h>
#include <ms/MeasurementSets/MSSourceColumns.h>
#include <ms/MeasurementSets/MSSourceIndex.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <casa/Logging/LogIO.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <casa/Arrays/ArrayLogical.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Containers/RecordField.h>
#include <casa/Exceptions/Error.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimDoppler::SimDoppler(const SimDoppler& s) : row_p(s.row_p), src_p(s.src_p), 
    rfreq_p(s.rfreq_p), trans_p(s.trans_p), spwids_p(s.spwids_p)
{ }

SimDoppler& SimDoppler::operator=(const SimDoppler& s) {
    row_p = s.row_p;
    src_p = s.src_p; 
    rfreq_p = s.rfreq_p;
    trans_p = s.trans_p; 
    spwids_p.resize(s.spwids_p.nelements());
    spwids_p = s.spwids_p;
    return *this;
}

SimDopplerList::SimDopplerList(const SimDopplerList& t) : n_p(t.n_p), 
    chnk_p(t.chnk_p), rec_p(t.rec_p.nelements())
{
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimDoppler(*(t.rec_p[i]));
}

SimDopplerList& SimDopplerList::operator=(const SimDopplerList& t) {
    deleteRecs();
    n_p = t.n_p;
    chnk_p = t.chnk_p; 
    rec_p.resize(t.rec_p.nelements());

    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimDoppler(*(t.rec_p[i]));
    return *this;
}

SimDopplerList::SimDopplerList(const MSDoppler& dopt, const MSSource& srct, 
			       const MSSpectralWindow& spwt, uInt stepz)
    : n_p(0), chnk_p(stepz), rec_p(dopt.nrow())
{
    initFrom(dopt, srct, spwt);
}

void SimDopplerList::initFrom(const MSDoppler& dopt, const MSSource& srct, 
			      const MSSpectralWindow& spwt)
{
    uInt nrow = dopt.nrow();
    if (nrow > 0) {
	LogIO log(LogOrigin("SimDoppler", "ctor()", WHERE));
	Int transid, dopid, srcid;
	ROMSDopplerColumns dopc(dopt);
	ROMSSourceColumns srcc(srct);
	MSSourceIndex srcidx(srct);

	for(uInt row=0; row < nrow; row++) {
	    transid = (dopc.transitionId())(row);
	    dopid = (dopc.dopplerId())(row);
	    srcid = (dopc.sourceId())(row);

	    // find rest frequency and transition for this spectral line
	    srcidx.sourceId() = srcid;
	    Vector<uInt> srcrows = srcidx.getRowNumbers();
	    if (srcrows.nelements() == 0) {
		log << LogIO::WARN << "Skipping over DOPPLER record referring "
		    << "to non-existant source id=" << srcid
		    << LogIO::POST;
		continue;
	    }
	    if ((srcc.numLines())(srcrows(0)) <= transid) {
		log << LogIO::WARN << "Skipping over DOPPLER record referring "
		    << "to non-existant transition (id=" << transid 
		    << ") for source id=" << srcid
		    << LogIO::POST;
		continue;
	    }
	    Vector<MFrequency> rfreqs;
	    Vector<String> trans;
	    srcc.restFrequencyMeas().get(static_cast<uInt>(srcrows(0)), rfreqs);
	    srcc.transition().get(static_cast<uInt>(srcrows(0)), trans);

	    // find all the spectral line windows that point to this DOPPLER
	    // record
	    ColumnsIndex index(spwt, "DOPPLER_ID");
	    RecordFieldPtr<Int> dopFld(index.accessKey(), "DOPPLER_ID");
	    *dopFld = dopid;

	    SimDoppler &dop = addDoppler(static_cast<uInt>(srcid), 
					 rfreqs(transid), trans(transid),
					 index.getRowNumbers());
	    dop.setRow(row);
	}	
    }
}

void SimDopplerList::flush(MSDoppler& dopt, MSSource& srct, 
			     MSSpectralWindow& spwt) 
{
    if (rec_p.nelements() == 0) return;

    LogIO log(LogOrigin("SimDoppler", "flush()", WHERE));
    MSDopplerColumns dopc(dopt);
    SimDoppler *dop;
    uInt row = dopt.nrow();
    Vector<Int> wins;

    Vector<Int> allwins(spwt.nrow());
    for(uInt i=0; i < allwins.nelements(); i++) allwins(i) = i;

    // make sure the SPECTRAL_WINDOW table has a DOPPLER_ID column
    if (! spwt.isColumn(MSSpectralWindow::DOPPLER_ID)) {
	spwt.addColumn(
	    ScalarColumnDesc<Int>(
                MSSpectralWindow::columnName(MSSpectralWindow::DOPPLER_ID),
                MSSpectralWindow::columnStandardComment(
                    MSSpectralWindow::DOPPLER_ID)));
    }
    MSSpWindowColumns spwc(spwt);

    // make sure the SOURCE table has REST_FREQUENCY & TRANSITION columns
    if (! srct.isColumn(MSSource::REST_FREQUENCY)) {
	spwt.addColumn(
	    ArrayColumnDesc<Double>(
                MSSource::columnName(MSSource::REST_FREQUENCY),
                MSSource::columnStandardComment(MSSource::REST_FREQUENCY)));
    }
    if (! srct.isColumn(MSSource::TRANSITION)) {
	spwt.addColumn(
	    ScalarColumnDesc<String>(
                MSSource::columnName(MSSource::TRANSITION),
                MSSource::columnStandardComment(MSSource::TRANSITION)));
    }
    MSSourceColumns srcc(srct);

    Vector<Int> srcids(srct.nrow());
    srcc.sourceId().getColumn(srcids, False);

    for(uInt i=0; i < n_p; i++) {
	if (rec_p[i]->getRow() < 0) {
	    dop = rec_p[i];

	    // first check to see if a DOPPLER_ID has already been assigned 
	    // to the spectral windows
	    const Vector<Int> *selwins = (anyLT(dop->spectralWindowIds(), 0))
		? &allwins : &(dop->spectralWindowIds());

	    wins.resize(selwins->nelements());
	    uInt j,k;
	    for(j=k=0; j < selwins->nelements(); j++) {
		if ((*selwins)(j) < static_cast<Int>(spwt.nrow())) {
		    // is the DOPPLER_ID <= 0?
		    if ((spwc.dopplerId())((*selwins)(j)) <= 0) {
		      if (selwins != &allwins)
			log << LogIO::WARN << "DOPPLER_ID already set for "
			    << "window id=" << (*selwins)(j)
			    << "; will not re-assign" << LogIO::POST;
		    }
		    else {
			wins(k++) = (*selwins)(j);
		    }
		}
		else {
		    log << LogIO::WARN << "Dropping assignment of spectral "
			<< "line to non-existent window id=" 
			<< (*selwins)(j) << LogIO::POST;
		}
	    }

	    // check to see if the line needs to be added to the 
	    // SOURCE table
	    Vector<String> trans;
	    Vector<Double> rfs;
	    Int transid = -1;
	    for(j=0; j < srct.nrow(); j++) {
	      if (srcids(j) == static_cast<Int>(dop->sourceId())) {
		Int nlines = (srcc.numLines())(j);
		if (nlines > 0) {

		    // determine if this line has been added already 
		    // by searching for a matching transition string
		    srcc.transition().get(j, trans, True);
		    if (! anyEQ(trans, dop->transition())) {
			srcc.restFrequency().get(j, rfs);
			trans.resize(nlines+1, True);
			rfs.resize(nlines+1, True);
			trans(nlines) = dop->transition();
			rfs(nlines) = 
			    dop->restFrequency().get("Hz").getValue("Hz");
			srcc.numLines().put(j, nlines+1);
			srcc.transition().put(j, trans);
			srcc.restFrequency().put(j, rfs);

			// this assumes that the all records of the 
			// with our source id has the same number of 
			// transitions recorded.
			if (transid < 0) transid = nlines;
		    }
		}
		else {
		    // no lines have yet been added to this source record
		    trans.resize(1);
		    rfs.resize(1);
		    trans(0) = dop->transition();
		    rfs(0) = dop->restFrequency().get("Hz").getValue("Hz");
		    srcc.numLines().put(j, 1);
		    srcc.transition().put(j, trans);
		    srcc.restFrequency().put(j, rfs);
		    transid = 0;
		}
	      }
	    }
	    if (transid < 0) {
		// source id not found!  
		log << LogIO::WARN << "Source ID=" << dop->sourceId()
		    << " not found; dropping this spectral line assignment."
		    << LogIO::POST;
		continue;
	    }

	    // now that we know the TRANSITION_ID, add the record to the 
	    // DOPPLER table
	    dopt.addRow(1);
	    dopc.dopplerId().put(row, row);
	    dopc.sourceId().put(row, dop->sourceId());
	    dopc.transitionId().put(row, transid);
	    dopc.velDefMeas().put(row, MDoppler(Quantity(0),MDoppler::RADIO));
	    dop->setRow(row);

	    // add the DOPPLER_ID to the SPECTRAL_WINDOW table
	    for(j=0; j < wins.nelements(); j++) {
		if (wins(j) >= static_cast<Int>(spwt.nrow())) {
		    // should not happen due to check done above
		    log << LogIO::WARN << "Dropping assignment of spectral "
			<< "line to non-existent window id=" << wins(j) 
			<< " (Programmer Error)"
			<< LogIO::POST;
		}
		else {
		    spwc.dopplerId().put(wins(j), row);
		}
	    }
	    row++;
	}
    }    
}

void SimDopplerList::throwOutOfRange(uInt legallength, Int index) const {
    ostringstream msg;
    msg << "index out of range of [0," << legallength 
	<< "]: " << index;
    throw AipsError(String(msg));
}

} //# NAMESPACE CASA - END

