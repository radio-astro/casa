//# SimScan.cc: a choice of telescope configuration used to produce a scan
//               of observations.
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
#include <simulators/Simulators/SimScan.h>
#include <simulators/Simulators/SimObservations.h>
#include <casa/Arrays/ArrayLogical.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimScan::SimScan() : pid_p(0), subary_p(0), fieldrep_p(1), srcid_p(0), 
    ants_p(), flds_p(), descs_p(), inttime_p(60.0, "s"), gaptime_p(0.0, "s"),
    slewtime_p(0.0, "s")
{ }

SimScan::SimScan(const Vector<uInt> dds, const Vector<uInt> ants, 
		 const Vector<uInt> flds, const Quantity& inttime, 
		 uInt fieldrep, uInt srcid, const Quantity& slewtime, 
		 const Quantity& gaptime, uInt subary, uInt procid)
    : pid_p(procid), subary_p(subary), fieldrep_p(fieldrep), srcid_p(srcid), 
      ants_p(ants), flds_p(flds), descs_p(dds), inttime_p(inttime), 
      gaptime_p(gaptime), slewtime_p(slewtime)
{ }

SimScan::SimScan(const Vector<uInt> dds, const Vector<uInt> ants, 
		 const Vector<uInt> flds, uInt fieldrep, uInt srcid, 
		 uInt subary, uInt procid)
    : pid_p(procid), subary_p(subary), fieldrep_p(fieldrep), srcid_p(srcid), 
      ants_p(ants), flds_p(flds), descs_p(dds), inttime_p(60.0, "s"), 
      gaptime_p(0.0, "s"), slewtime_p(0.0, "s")
{ }

SimScan::SimScan(const Vector<uInt> dds, const Vector<uInt> ants, 
		 const Quantity& inttime, uInt srcid, const Quantity& gaptime, 
		 uInt subary, uInt procid)
    : pid_p(procid), subary_p(subary), fieldrep_p(1), srcid_p(srcid), 
      ants_p(ants), flds_p(1), descs_p(dds), inttime_p(inttime), 
      gaptime_p(gaptime), slewtime_p(0.0, "s")
{ 
    flds_p(0) = 0;
}

SimScan::SimScan(const SimObservations& obs, const Quantity& inttime, 
		 uInt srcid, uInt fieldrep, Bool sanityCheck, 
		 const Quantity& gaptime, const Quantity& slewtime, 
		 uInt procid) 
    : pid_p(procid), subary_p(0), fieldrep_p(fieldrep), srcid_p(srcid), 
      ants_p(), flds_p(), descs_p(), inttime_p(inttime), gaptime_p(gaptime), 
      slewtime_p(slewtime)
{
    const SimTelescope *tel = obs.telescope();
    if (tel) {
	ants_p.resize(tel->numAnts());
	if (sanityCheck && ants_p.nelements() == 0)
	    throw AipsError("No antennas currently defined");
	for(uInt i=0; i < ants_p.nelements(); i++) ants_p(i) = i;
    }
    else if (sanityCheck) {
	throw AipsError("Telescope not defined for observations");
    }

    if (srcid < obs.numSources()) {
	flds_p.resize(obs.numFields(srcid));
	if (sanityCheck && flds_p.nelements() == 0)
	    throw AipsError("No fields currently defined");
	uInt i, f;
	for(i=0, f=0; i < obs.numFields(); i++) {
	    if (obs.field(i).sourceId() == srcid) {
		flds_p(f) = f;
		f++;
	    }
	}
    }
    else {
	throw AipsError(String("No source defined with id=")+srcid);
    }

    descs_p.resize(obs.numDataDesc());
    if (sanityCheck && descs_p.nelements() == 0)
	throw AipsError("No antennas currently defined");
    for(uInt i=0; i < descs_p.nelements(); i++) descs_p(i) = i;
}

SimScan::SimScan(const SimScan& s) : pid_p(s.pid_p), subary_p(s.subary_p), 
    fieldrep_p(s.fieldrep_p), srcid_p(s.srcid_p), ants_p(s.ants_p), 
    flds_p(s.flds_p), descs_p(s.descs_p), inttime_p(s.inttime_p), 
    gaptime_p(s.gaptime_p), slewtime_p(s.slewtime_p)
{ }

SimScan& SimScan::operator=(const SimScan& s) {
    pid_p = s.pid_p; 
    subary_p = s.subary_p; 
    srcid_p = s.srcid_p; 
    fieldrep_p = s.fieldrep_p; 

    ants_p.resize(s.ants_p.nelements());
    ants_p = s.ants_p; 
    flds_p.resize(s.flds_p.nelements());
    flds_p = s.flds_p; 
    descs_p.resize(s.descs_p.nelements());
    descs_p = s.descs_p; 

    inttime_p = s.inttime_p; 
    gaptime_p = s.gaptime_p; 
    slewtime_p = s.slewtime_p;
    return *this;
}

SimScan::~SimScan() { }

Bool SimScan::checkDataDesc(const Vector<uInt>& descs, 
			    const SimObservations& obs) 
{
    return allLT(descs, obs.numDataDescIds());
}

Bool SimScan::checkAntennas(const Vector<uInt>& ants, 
			    const SimObservations& obs) 
{
    return allLT(ants, obs.telescope()->numAnts());
}

Bool SimScan::checkSourceId(Int srcid, const SimObservations& obs) {
    return (srcid >= 0 && srcid < static_cast<Int>(obs.numSources()));
}

Bool SimScan::checkFields(Int srcid, const Vector<uInt>& flds, 
			  const SimObservations& obs) 
{
    if (! checkSourceId(srcid, obs)) return False;
    if (anyGE(flds, obs.numFields())) return False;

    // make sure that all the fields are truely associated with 
    // the given source id.
    for(uInt i=0; i < flds.nelements(); i++) {
	if (static_cast<Int>(obs.field(flds(i)).sourceId()) != srcid) 
	    return False;
    }

    return True;
}


} //# NAMESPACE CASA - END

