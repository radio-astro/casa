//# SimScan.h: a choice of telescope configuration used to produce a scan
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

#ifndef SIMULATORS_SIMSCAN_H
#define SIMULATORS_SIMSCAN_H

#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SimObservations;

// <summary> 
// a choice of telescope configuration used to produce a scan of observations
// </summary>
// <reviewed reviewer="" date="" tests="">
//
// <prerequisite>
//   <li> <linkto class="SimObservations">SimObservations</linkto>
// </prerequisite>
//
// <etymology>
// This class holds the parameters for a particular scan in a set of 
// simulated observations.  A scan is defined here as a set of consecutive 
// integrations on the same source with unchanging observational parameters.
// </etymology>
//
// <synopsis> 
// This class is a container for parameters to be used in a single scan 
// of observations.  (See etymology above for a definition of a scan.)  A 
// simulator (e.g. <linkto class="SimpleSimulator">SimpleSimulator</linkto>) 
// would combine these parameters along with those access via a
// <linkto class="SimObservations">SimObservations</linkto> object to drive 
// the simulated observations.  When observations are more complicated than
// just observing a single source, the simulator would typically manage one
// SimObservation but switch between a set of SimScans--say, one for each 
// source.  <p>
//
// Thus, a SimScan holds the part of the observational configuration that 
// is expected to change the most through the run--primarily, the source, 
// the set of fields, the set of data descriptions to fill (which selects 
// the desired polarizations and spectral windows), the set of antennas to 
// use (assigning a sub-array ID), the integration time, the scan start time 
// and duration.  One can also control gap times between integrations and 
// a slew time between fields.  This class also provides check functions 
// that can check the validity of any of the IDs referenced against what 
// is currently defined in a SimObservation object.  <p>
// 
// In general, the data held in a SimScan represents information that 
// eventually winds up or otherwise affects the data that gets written to
// an MeasurementSet's main table.  All subtable information is encapsulated
// in the SimObservations class.
// </synopsis> 
//
// <motivation>
// To make typical observing patterns (e.g. in which one switches between 
// target and calibrator sources) easier to manage in a simulator, it is 
// helpful to separate out the parameters that change between scans from 
// the more static parameters held in the 
// <linkto class="SimObservations">SimObservations</linkto> class.
// </motivation>
//
// <todo asof="03/03/25">
// </todo>
class SimScan {
public:

    // construct an undefined SimScan
    SimScan();

    // construct a defined SimScan
    // <group>
    SimScan(const Vector<uInt> dds, const Vector<uInt> ants, 
	    const Vector<uInt> flds, uInt fieldrep=1, uInt srcid=0, 
	    uInt subary=0, uInt procid=0);
    SimScan(const Vector<uInt> dds, const Vector<uInt> ants, 
	    const Quantity& inttime, uInt srcid=0, 
	    const Quantity& gaptime=Quantity(0.0, "s"),
	    uInt subary=0, uInt procid=0);
    SimScan(const Vector<uInt> dds, const Vector<uInt> ants, 
	    const Vector<uInt> flds, const Quantity& inttime, uInt fieldrep=1, 
	    uInt srcid=0, const Quantity& slewtime=Quantity(0.0, "s"),
	    const Quantity& gaptime=Quantity(0.0, "s"),
	    uInt subary=0, uInt procid=0);
    // </group>

    // construct a scan that will observe all fields for the first source 
    // with all antennas and data descriptions currently defined for the 
    // given SimObservations.
    SimScan(const SimObservations& obs, const Quantity& inttime, 
	    uInt srcid=0, uInt fieldrep=1, Bool sanityCheck=True, 
	    const Quantity& gaptime=Quantity(0.0, "s"),
	    const Quantity& slewtime=Quantity(0.0, "s"),
	    uInt procid=0);

    // construct a copy of another scan
    SimScan(const SimScan& scan);
    SimScan& operator=(const SimScan& scan);

    // delete this scan
    virtual ~SimScan();

    // return the list of data description ids that will be observed
    // simultaneously in this scan.
    // <group>
    const Vector<uInt>& dataDescriptions() const { return descs_p; }
    Vector<uInt>& dataDescriptions() { return descs_p; }
    // </group>

    // return true if data description ids can all be applied to the 
    // given SimObservations object.  False is returned if any of the ids 
    // are out of range.  The static version allows one to check a potential 
    // list of IDs before setting them.  
    // <group>
    Bool checkDataDesc(const SimObservations& obs) const {
	return checkDataDesc(descs_p, obs);
    }
    static Bool checkDataDesc(const Vector<uInt>& descs, 
			      const SimObservations& obs);
    // </group>

    // return the source ID of the source observed in this scan.  A negative 
    // value means that the requested fields refer to multiple sources.
    uInt sourceId() const { return srcid_p; }

    // set the source ID of the source to be observed in this scan.  A 
    // negative value means that the requested fields refer to multiple 
    // sources.
    void setSourceId(uInt srcid) { srcid_p = srcid; }

    // return true if the source id is defined in the given SimObservations. 
    // The static version allows one to check a source ID before it.
    // <group>
    Bool checkSourceId(const SimObservations& obs) const {
	return checkSourceId(srcid_p, obs);
    }
    static Bool checkSourceId(Int srcid, const SimObservations& obs);
    // </group>

    // return the IDs of the fields that will be observed in 
    // succession with current subarray.  ids will be resized if necessary.
    // <group>
    const Vector<uInt>& fields() const { return flds_p; }
    Vector<uInt>& fields() { return flds_p; }
    // </group>

    // return true if the field Ids can all be applied to the given 
    // SimObservations.  False is returned if any of the field Ids 
    // are out of range or don't match the current srcid.  The static 
    // version allows one to check a potential list of IDs before setting 
    // them.  
    // <group>
    Bool checkFields(const SimObservations& obs) const {
	return checkFields(srcid_p, flds_p, obs);
    }
    static Bool checkFields(Int srcid, const Vector<uInt>& flds, 
			    const SimObservations& obs);
    // </group>

    // load into ids the IDs of the antennas observing the source in this 
    // scan. 
    // <group>
    const Vector<uInt>& antennas() const { return ants_p; }
    Vector<uInt>& antennas() { return ants_p; }
    // </group>

    // return true if the antenna Ids can all be applied to the given 
    // SimObservations.  False is returned if any of the antenna Ids 
    // are out of range.  The static version allows one to check a potential 
    // list of IDs before setting them.  
    // <group>
    Bool checkAntennas(const SimObservations& obs) const {
	return checkAntennas(ants_p, obs);
    }
    static Bool checkAntennas(const Vector<uInt>& ants, 
			      const SimObservations& obs);
    // </group>

    // return the subarray ID to give to the chosen set of antennas
    uInt arrayId() const { return subary_p; }

    // set the subarray ID to assign to the current set of antennas
    void setArrayId(uInt arrayid) { subary_p = arrayid; }

    // return the integration time
    // <group>
    const Quantity& intTime() const { return inttime_p; }
    Double intTimeSec() const { return inttime_p.getValue("s"); }
    // </group>

    // set the integration time 
    void setIntTime(const Quantity& inttime) { inttime_p = inttime; }

    // return the time interval between integrations
    // <group>
    const Quantity& gapTime() const { return gaptime_p; }
    Double gapTimeSec() const { return gaptime_p.getValue("s"); }
    // </group>

    // set the gap time interval between integrations
    void setGapTime(const Quantity& gaptime) { gaptime_p = gaptime; }

    // return the time interval between observations of different fields
    // <group>
    const Quantity& slewTime() const { return slewtime_p; }
    Double slewTimeSec() const { return slewtime_p.getValue("s"); }
    // </group>

    // set the time interval between observations of different fields
    void setSlewTime(const Quantity& slewtime) { slewtime_p = slewtime; }

    // get and set the number of integrations to perform on each field before 
    // slewing to the next one.  This may be interpreted as a default value
    // that may be overridden by the specific Field description.
    // <group>
    uInt fieldRepetition() const { return fieldrep_p; } 
    void setFieldRepetition(uInt n) { fieldrep_p = n; } 
    // </group>

    // return the processor ID for this scan
    uInt processorId() const { return pid_p; }

    // set the processor ID
    void setProcessorID(uInt id) { pid_p = id; }

    // return true if this Scan is applicable to the given SimObservations
    // object.  This calls the check*() routines in succession.
    Bool check(const SimObservations& obs) const {
	return (checkDataDesc(obs) && checkFields(obs) && checkAntennas(obs));
    }

private:
    uInt pid_p, subary_p, fieldrep_p, srcid_p;
    Vector<uInt> ants_p, flds_p, descs_p;
    Quantity inttime_p, gaptime_p, slewtime_p;
};


} //# NAMESPACE CASA - END

#endif
