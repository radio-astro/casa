//# SimTelescope.h: containers for containing key telescope setup info in memory
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
#include <simulators/Simulators/SimTelescope.h>

#include <ms/MeasurementSets/MeasurementSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

const String SimTelescope::defaultName("Very Simulated Array");
const MPosition SimTelescope::defaultLocation(MVPosition(0.0,0.0,0.0), 
						MPosition::WGS84);

SimTelescope::SimTelescope(const String& name) 
    : name_p(name), refLoc_p(defaultLocation), 
      ary_p(), wins_p(), feeds_p(), pols_p(), ms_p(0) 
{ }

SimTelescope::SimTelescope(const MPosition& aryLoc, const String& name) 
    : name_p(name), refLoc_p(aryLoc), ary_p(), wins_p(), feeds_p(), 
      pols_p(), ms_p(0) 
{ }

SimTelescope::SimTelescope(const SimTelescope &tel) 
    : name_p(tel.name_p), refLoc_p(defaultLocation), ary_p(tel.ary_p), 
      wins_p(tel.wins_p), feeds_p(tel.feeds_p), pols_p(tel.pols_p), 
      ms_p(tel.ms_p ? new MeasurementSet(*tel.ms_p) : 0) 
{ }

SimTelescope& SimTelescope::operator=(const SimTelescope &tel) {
    name_p = tel.name_p; 
    refLoc_p = defaultLocation; 
    ary_p = tel.ary_p; 
    wins_p = tel.wins_p; 
    feeds_p = tel.feeds_p; 
    pols_p = tel.pols_p; 
    delete ms_p;
    ms_p = tel.ms_p ? new MeasurementSet(*tel.ms_p) : 0;
    return *this;
}

SimTelescope::SimTelescope(const MeasurementSet &ms) 
    : name_p(), refLoc_p(defaultLocation), ary_p(), wins_p(), feeds_p(), 
      pols_p(), ms_p(0) 
{ 
    init(ms);
    if (ary_p.numRecs() > 0) {
	Vector<MPosition> ants(ary_p[0].numAnts());
	ary_p[0].antennaPositions(ants, 0);
	if (ants.nelements() > 0) refLoc_p = ants(0);
    }
}

SimTelescope::SimTelescope(MeasurementSet &ms)
    : name_p(), refLoc_p(defaultLocation), ary_p(), wins_p(), feeds_p(), 
      pols_p(), ms_p(0) 
{ 
    initAndAttach(ms);
    if (ary_p.numRecs() > 0) {
	Vector<MPosition> ants(ary_p[0].numAnts());
	ary_p[0].antennaPositions(ants, 0);
	if (ants.nelements() > 0) refLoc_p = ants(0);
    }
}

SimTelescope::~SimTelescope() {
    delete ms_p;
}

void SimTelescope::init(const MeasurementSet &ms) {
    ary_p.initFrom(ms.antenna(), False);
    wins_p.initFrom(ms.spectralWindow());
    feeds_p.initFrom(ms.feed());
    pols_p.initFrom(ms.polarization(), feeds_p);
}

void SimTelescope::flush() {
    if (! ms_p) return;
    ary_p.flush(ms_p->antenna());
    wins_p.flush(ms_p->spectralWindow());
    feeds_p.flush(ms_p->feed(), ary_p.numAnts());
    pols_p.flush(ms_p->polarization());
}

void SimTelescope::attach(MeasurementSet &ms, Bool clear) {
    ms_p = new MeasurementSet(ms);
    if (clear) {
	wins_p.clearIds();
	feeds_p.clearIds();
	pols_p.clearIds();
    }
}

uInt SimTelescope::addPolCorrs(const Vector<Stokes::StokesTypes>& corrTypes, 
			       uInt feed1, uInt feed2)
{
    for(uInt i=0; i < feeds_p.numFeeds(); i++) {
	if (feeds_p[i].feedId() == feed1) {
	    for(uInt j=0; j < feeds_p.numFeeds(); j++) {
		if (feeds_p[j].feedId() == feed2 &&
		    pols_p.addPolSetup(corrTypes, feeds_p[i], feeds_p[j])) 
		{
		    return pols_p.numSetups()-1;
		}
	    }
	}
    }

    throw AipsError(String("Unable to form correlations with ") +
		    "requested feeds");
}

uInt SimTelescope::addPolCorrs(uInt ncorr, uInt feed1, uInt feed2, uInt which) {
    Vector<Stokes::StokesTypes> corrs(ncorr);
    String f1pols, f2pols;

    if (ncorr < 1 || ncorr > 4 || ncorr == 3) 
	throw AipsError(String("Illegal number of polarization correlations ") +
			       "requested: " + ncorr);

    // find the first pair of feed descriptions that have our requested feedids
    // and the proper number of receptors
    for(uInt i=0; i < feeds_p.numFeeds(); i++) {
	if (feeds_p[i].feedId() == feed1) {
	    f1pols = feeds_p[i].getPolTypes();
	    if ((ncorr == 1 && which < f1pols.length()) ||
		(f1pols.length() >= 2))
	    {
		for(uInt j=0; j < feeds_p.numFeeds(); j++) {
		    if (feeds_p[j].feedId() == feed2) {
			f2pols = feeds_p[i].getPolTypes();
			if (ncorr == 1 && which < f2pols.length()) {

			    // request 1 correlation
			    corrs(0) = Stokes::type(String(f1pols[which]) +
						    f2pols[which]);
			    if (pols_p.addPolSetup(corrs,feeds_p[i],feeds_p[j]))
				return pols_p.numSetups()-1;
			}
			else if (f2pols.length() >= 2) {
			  if (ncorr == 2) {

			   // request 2 parallel correlations
			   corrs(0) = Stokes::type(String(f1pols[0])+f2pols[0]);
			   corrs(1) = Stokes::type(String(f1pols[1])+f2pols[1]);

			  }
			  else {

			   // request 4 cross correlations
			   corrs(0) = Stokes::type(String(f1pols[0])+f2pols[0]);
			   corrs(1) = Stokes::type(String(f1pols[0])+f2pols[1]);
			   corrs(2) = Stokes::type(String(f1pols[1])+f2pols[1]);
			   corrs(3) = Stokes::type(String(f1pols[1])+f2pols[0]);
			  }
			  if (pols_p.addPolSetup(corrs,feeds_p[i],feeds_p[j]))
				return pols_p.numSetups()-1;
			}
		    }
		}
	    }
	}
    }

    throw AipsError(String("Unable to form correlations with ") +
		    "requested feeds");
}

uInt SimTelescope::addFeed(uInt id, const String& pols, 
			   const MPosition *position,
			   const Vector<Double> *receprAngle,
			   const Matrix<Double> *beamOff, 
			   const Matrix<Complex> *polResp,
			   Int antId, Double time, Double interval,
			   Int spwId) 
{
    SimFeed& rec = feeds_p.addFeed(pols.length(), pols);
    rec.setKey(id, antId, spwId, time, interval);
    if (position) rec.setPosition(*position);
    if (receprAngle) rec.receptorAngle() = *receprAngle;
    if (beamOff) rec.beamOffset() = *beamOff;
    if (polResp) rec.polResponse() = *polResp;
    return feeds_p.numFeeds()-1;
}


} //# NAMESPACE CASA - END

