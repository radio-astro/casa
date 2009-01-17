//# SimPol.h: container for containing Polarization setup description
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

#include <simulators/Simulators/SimPol.h>

#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

const String SimPol::pnames_p = "RLXY";
const Int SimPol::ptbits_p[] = { 1, 2, 4, 8 };

SimPol::SimPol() : row_p(-1), f1_p(0), f2_p(0), p1_p(0), p2_p(0), 
    types_p(0), prods_p(), stokes_p(0) { }

SimPol::SimPol(const Vector<Stokes::StokesTypes>& corrTypes, 
			 const SimFeed& feed1, const SimFeed& feed2) 
    : row_p(-1), f1_p(0), f2_p(0), p1_p(0), p2_p(0), 
      types_p(), prods_p(), stokes_p(0) 
{ 
    setCorrProducts(corrTypes, feed1, feed2);
}

SimPol::SimPol(const SimPol& that) 
    : row_p(that.row_p), f1_p(that.f1_p), f2_p(that.f2_p), 
      p1_p(that.p1_p), p2_p(that.p2_p), types_p(that.types_p), 
      prods_p(that.prods_p), stokes_p(that.stokes_p)
{ }

SimPol& SimPol::operator=(const SimPol& that) {
    row_p = that.row_p; 
    f1_p = that.f1_p; 
    f2_p = that.f2_p; 
    p1_p = that.p1_p; 
    p2_p = that.p2_p; 

    types_p.resize(that.types_p.nelements());
    types_p = that.types_p; 
    prods_p.resize(that.prods_p.nrow(), that.prods_p.ncolumn());
    prods_p = that.prods_p; 
    stokes_p.resize(that.stokes_p.nelements());
    stokes_p = that.stokes_p;
    return *this;
}

void SimPol::requiredReceptors(uInt& t1, uInt& t2, 
			       Vector<String>& stokes) const 
{
    // determine what polarization receptors we need from each feed.
    String::size_type j, k;
    stokes.resize(types_p.nelements());
    t1 = t2 = 0;
    for(uInt i=0; i < types_p.nelements(); i++) {
	stokes(i) = 
	    Stokes::name(static_cast<Stokes::StokesTypes>(types_p(i)));
	if ((j = pnames_p.find(stokes(i)[0])) == String::npos)
	    throw AipsError(String("Unsupported polarization: ") + stokes[0]);
	if ((k = pnames_p.find(stokes(i)[1])) == String::npos)
	    throw AipsError(String("Unsupported polarization: ") + stokes[0]);
	t1 |= ptbits_p[j];
	t2 |= ptbits_p[k];
    }
}

Bool SimPol::setCorrProducts(const Vector<Stokes::StokesTypes>& corrTypes, 
				  const SimFeed& feed1, 
				  const SimFeed& feed2)
{
    String found1, found2;
    Int i, ncorrs;
    uInt p1, p2;
    Vector<String> stokes;

    ncorrs = corrTypes.nelements();
    if (ncorrs == 0 || ncorrs == 3 || ncorrs > 4)
	throw AipsError(String("Illegal number of correlations: ") + 
			corrTypes.nelements());

    types_p.resize(ncorrs);
    for(i=0; i < ncorrs; i++) types_p(i) = corrTypes(i);
    requiredReceptors(p1, p2, stokes);

    if (! canUseFeed(feed1, True) || ! canUseFeed(feed2, False))
	return False;
    f1_p = feed1.feedId();
    f2_p = feed2.feedId();
    p1_p = p1;
    p2_p = p2;
    stokes_p = stokes;

    // figure out the polarization types
    found1 = feed1.getPolTypes();
    found2 = feed2.getPolTypes();
    prods_p.resize(2,ncorrs);
    for(i=0; i < ncorrs; i++) {
	prods_p(0,i) = found1.find(stokes_p(i)[0]);
	prods_p(1,i) = found2.find(stokes_p(i)[1]);
    }

    return True;
}

Bool SimPol::canUseFeed(const SimFeed& feed, Bool asFeed1) {
    uInt j, need, got;
    String::size_type k;

    need = (asFeed1) ? p1_p : p2_p;

    const String &rec = feed.getPolTypes();
    for(got=0, j=0; j < rec.length(); j++) {
	if ((k = pnames_p.find(rec[j])) != String::npos) 
	    got |= ptbits_p[k];
    }
    return ((got&need) == need);
}

//#########################################

//  SimPolList::SimPolList(const MSPolarization& polt, const SimFeedBuf& feeds, 
//  			 uInt stepsz) 
//      : n_p(0), chnk_p(stepsz), rec_p(polt.nrow()) 
//  {
//      ROMSPolarizationColumns polc(polt);
//  }

SimPolList::SimPolList(const SimPolList& w) : n_p(w.n_p), 
    chnk_p(w.chnk_p), rec_p(w.rec_p.nelements())
{ 
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimPol(*(w.rec_p[i]));
}

SimPolList& SimPolList::operator=(const SimPolList& w) { 
    deleteRecs();
    n_p = w.n_p;
    chnk_p = w.chnk_p;
    rec_p.resize(w.rec_p.nelements());
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimPol(*(w.rec_p[i]));
    return *this;
}

SimPolList::SimPolList(const MSPolarization& polt, const SimFeedList& fdl, 
		       uInt stepsz)
    : n_p(0), chnk_p(stepsz), rec_p(polt.nrow())
{
    initFrom(polt, fdl);
}

void SimPolList::initFrom(const MSPolarization& polt, const SimFeedList& fdl) {
    uInt nrow = polt.nrow();
    if (nrow > 0) {
	ROMSPolarizationColumns polc(polt);
	SimpleOrderedMap<uInt, uInt> fdid(0, 2);
	SimPol *pol;

	// assume that all antennas have identical sets of feed receptors
	for(uInt i=0; i < fdl.numFeeds(); i++) {
	    if (! fdid.isDefined(fdl[i].feedId()))
		fdid.define(fdl[i].feedId(), i);
	}

	for(uInt row=0; row < nrow; row++) {
	    Vector<Int> ctype;
	    polc.corrType().get(row, ctype);
	    Vector<Stokes::StokesTypes> stype(ctype.nelements());
	    for(uInt i=0; i < stype.nelements(); i++) 
		stype(i) = static_cast<Stokes::StokesTypes>(ctype(i));
	    Matrix<Int> cprod;
	    polc.corrProduct().get(row, cprod);

	    // this is really inefficient for more than a few different
	    // feeds.
	    for(uInt i=0; i < fdid.ndefined(); i++) {
		for(uInt j=0; j < fdid.ndefined(); j++) {
		    if (feedsGiveProducts(fdl[fdid.getVal(i)], 
					  fdl[fdid.getVal(j)], stype, cprod))
		    {
			pol = addPolSetup(stype, 
					  fdl[fdid.getVal(i)], 
					  fdl[fdid.getVal(j)]);
			if (pol) pol->setId(row);
		    }
		}
	    }
	}
    }
}

Bool SimPolList::feedsGiveProducts(const SimFeed& fd1, const SimFeed& fd2, 
				   const Vector<Stokes::StokesTypes> ctype,
				   const Matrix<Int>& cprod) 
{
    if (cprod.ncolumn() != ctype.nelements()) 
	throw AipsError(String("Stokes list and Correlation matrix have ") + 
			"inconsistant sizes!");
    for(uInt i=0; i < ctype.nelements(); i++) {
	if (String((fd1.getPolTypes())[cprod(0,i)]) + 
	    (fd2.getPolTypes())[cprod(1,i)] != Stokes::name(ctype[i]))
	  return False;
    }
    return True;
}

void SimPolList::flush(MSPolarization& polt) {
    MSPolarizationColumns polc(polt);
    uInt nrow = polt.nrow();
    SimPol *pol;

    for(uInt i=0; i < n_p; i++) {
	if (rec_p[i]->getId() < 0) {
	    polt.addRow(1);
	    pol = rec_p[i];

	    polc.numCorr().put(nrow, pol->numTypes());
	    polc.corrType().put(nrow, pol->corrTypes());
	    polc.corrProduct().put(nrow, pol->corrProducts());
	    polc.flagRow().put(nrow, False);

	    pol->setId(nrow++);
	}
    }
}

void SimPolList::throwOutOfRange(uInt legallength, Int index) const {
    ostringstream msg;
    msg << "index out of range of [0," << legallength 
	<< "]: " << index;
    throw AipsError(String(msg));
}

} //# NAMESPACE CASA - END

