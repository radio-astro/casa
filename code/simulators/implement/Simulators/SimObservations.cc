//# SimObservations.h: containers for containing key telescope setup info in memory
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
#include <simulators/Simulators/SimObservations.h>

#include <ms/MeasurementSets/MeasurementSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimObservations::SimObservations(SimTelescope *tel, Bool dodel) 
    : deltel_p(dodel), tel_p(tel), srcs_p(), flds_p(), dops_p(), dds_p()
{ }

SimObservations::SimObservations(const SimObservations& obs) 
    : deltel_p(True), tel_p(0), srcs_p(obs.srcs_p), flds_p(obs.flds_p), 
      dops_p(obs.dops_p), dds_p(obs.dds_p)
{ 
    tel_p = new SimTelescope(*(obs.tel_p));
}

SimObservations& SimObservations::operator=(const SimObservations& obs) {
    if (deltel_p && tel_p) delete tel_p;

    deltel_p = True; 
    tel_p = 0; 
    srcs_p = obs.srcs_p; 
    flds_p = obs.flds_p; 
    dops_p = obs.dops_p; 
    dds_p = obs.dds_p;

    tel_p = (obs.tel_p) ? new SimTelescope(*(obs.tel_p)) : 0;
    return *this;
}

SimObservations::SimObservations(const MeasurementSet &ms)
    : deltel_p(True), tel_p(0), srcs_p(), flds_p(), dops_p(), dds_p()
{ 
    tel_p = new SimTelescope(ms);
    init(ms);
}

SimObservations::SimObservations(MeasurementSet &ms) 
    : deltel_p(True), tel_p(0), srcs_p(), flds_p(), dops_p(), dds_p()
{ 
    tel_p = new SimTelescope(ms);
    initAndAttach(ms);
}
    
SimObservations::~SimObservations() {
    if (deltel_p && tel_p) delete tel_p;
}

void SimObservations::init(const MeasurementSet &ms) {
    tel_p->init(ms);
    srcs_p.initFrom(ms.source());
    flds_p.initFrom(ms.field());
    dops_p.initFrom(ms.doppler(), ms.source(), ms.spectralWindow());
    dds_p.initFrom(ms.dataDescription());
}

void SimObservations::initAndAttach(MeasurementSet &ms) {
    init(ms);
    attach(ms, False);
}

void SimObservations::flush() {
    MeasurementSet *ms = tel_p->ms();
    if (! ms) return;

    tel_p->flush();
    srcs_p.flush(ms->source());
    flds_p.flush(ms->field(), ms->source());
    dops_p.flush(ms->doppler(), ms->source(), ms->spectralWindow());
    dds_p.flush(ms->dataDescription(), ms->polarization().nrow(),
		ms->spectralWindow().nrow());
}

void SimObservations::attach(MeasurementSet &ms, Bool clear) {
    tel_p->attach(ms, clear);
    if (clear) {
	srcs_p.clearIds();
	flds_p.clearIds();
	dops_p.clearIds();
	dds_p.clearIds();
    }
}


} //# NAMESPACE CASA - END

