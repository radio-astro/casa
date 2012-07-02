//# SimpleSimulator.h: writes out a simulated MS with blank data
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

#ifndef SIMULATORS_SIMPLESIMULATOR_H
#define SIMULATORS_SIMPLESIMULATOR_H

#include <simulators/Simulators/SimObservations.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MEpoch;
class MeasurementSet;
class SimScan;

// <summary> 
// a simple simulated MS writer
// </summary>
// <reviewed reviewer="" date="" tests="">
//
// <prerequisite>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto> class
//   <li> <linkto class="SimObservations">SimObservations</linkto> class
//   <li> <linkto class="SimScan">SimScan</linkto> class
// </prerequisite>
//
// <etymology>
// This class is simulator that uses a simple observational model that is 
// typical of modern "guest observer" interferometers. 
// </etymology>
//
// <synopsis> 
// Despite the "Simple" in its name, this class is capable of driving 
// simulated interometric observations using a variety of typical observing 
// patterns.  At its simplest, it can integrate on a single source for a 
// set amount of time; at its most complex it can observe a pattern of fields,
// switch between sources (e.g. target and calibrator), switch between 
// polarization set ups, and fill multiple spectral windows.  It does assume
// that it will write all its data to a single MeasurementSet that it creates.
// <p>
//
// This class uses a general pattern intended for a whole class of simulators
// of which this simulator is one.  In this pattern, the application first 
// defines the telescope's hardware--i.e. the antennas, the feeds and 
// polarizations, and the spectral windows,  via a SimTelescope object.  
// Next, the SimTelescope is passed to a SimObservation object, and the 
// observational configurations are added; this includes the sources, fields,
// spectral lines, and window-polarization combinations (i.e. data 
// descriptions).  The SimObservation object is then passed to the constructor
// of a simulator.  One can then define some global properties of the 
// observations, include blockage and elevation limits.  <p>
//
// The simulations are carried out by a series of calls to the write() function.
// This causes the definition of the telescope and the observational parameters
// to be written to the output MS along with integration records in the MS's 
// main table for each baseline, data description, and timestamp over some 
// interval of time.  Note that the MS's DATA column will be filled with zeros;
// it is intended that visibilities based on a source model will be filled 
// into this column later using the Measurment Equation.  <p>
//
// Each call to the write() function represents a new scan.  The parameters 
// that define the scan are a start time, a duration, and the various 
// properties held in a <linkto class="SimScan">SimScan</linkto> object.  In 
// non-trivial simulation, an application may manage several SimScan objects,
// each containing the configuration choices for the different phases of the 
// observations.  For example, if the observational program observes multiple
// sources, the application would have one SimScan for each source.  <p>
//
// The writeObsRecord() function is meant to be called after a series of 
// calls to write(), marking a major transition point in the observations 
// (e.g. switch to a new day or track).  It should be called at least once at 
// the end of the observations.  This will write out a record to the 
// OBSERVATION subtable and increment the OBSERVATION_ID that is written to
// the main table.  <p>
//
// Data is flushed after every call to write() and writeObsRecord(); thus,
// it is possible for other classes to open the output measurement set for 
// read-only access.  Destructing the class closes out the measurment set 
// permanently.  <p>
// 
// It is possible after calls to write() to add to the telescopes 
// configuration--e.g. add antennas, windows, sources, etc.  When write() 
// is called again, the new configuration will get flushed to the MS 
// automatically. <p>
// </synopsis> 
//
// <motivation>
// This replaces the functionality of the MSSimulator class and is based on the 
// more flexible SimTelescope & SimObservations classes.  Together, these 
// classes are capable of simulated the most typical types of observations
// carried out by interferometers like the VLA, BIMA, ATCA, ....
// </motivation>
//
// <todo asof="02/09/03">
// </todo>
class SimpleSimulator {
public:

    // Create a simulator.  This will operate on the MS that is currently 
    // attached to obs; an exception is thrown if none is yet attached.
    SimpleSimulator(SimObservations *obs);

    // Create a simulator that will create a new measurement set with a given
    // name.  
    SimpleSimulator(SimObservations *obs, const String& msname);

    // delete this simulator
    virtual ~SimpleSimulator();

    // get and set the autocorrelation weight.  If the weight is <= 0.0 
    // (the default), autocorrelation is not written out.  For 
    // cross-correlation records, the value in the WEIGHT column is 
    // set to 1/sigma.  For autocorrelation records, the 1/sigma value is
    // multiplied by this autocorrelation weight value.
    // <group>
    Float getAutoCorrWeight() const { return autocorrwt_p; }
    void setAutoCorrWeight(Float wt) { autocorrwt_p = wt; }
    // </group>

    // get and set the elevation limit.
    // <group>
    Quantity getElevationLimit() const { return elevLimit_p; }
    void setElevationLimit(const Quantity& limit) { elevLimit_p = limit; }
    // </group>

    // get and set the fractional shadowing limit.
    // <group>
    Double getFractionalBlockageLimit() const { return blockFrac_p; }
    void setFractionalBlockageLimit(Double limit) { blockFrac_p = limit; }
    // </group>

    // write out the next scan.  Each call to write increments the scan
    // number written to the MS's main table.  
    void write(const MEpoch& start, const Quantity& duration,
	       const SimScan& scan);

    // write out an OBSERVATION record for the scans written so far.  If
    // write() has not yet been called, the observation internally stored 
    // start and end times will be undefined, and no record will be written 
    // out.  In this case, False will be returned.   Otherwise, True is 
    // returned, and the times will be reset to undefined.
    Bool writeObsRecord();

protected:
    // calculate the amount of shadowing between 2 antennas
    static void blockage(Double &fraction1, Double &fraction2,
			 const Vector<Double>& uvw, 
			 const Double diam1, const Double diam2);

    SimpleSimulator();

    // Create a simulator that will extend an existing MS.
    // This is not currently supported; thus, this constructor always 
    // throws an exception.
    SimpleSimulator(SimObservations *obs, MeasurementSet& ms);

    // create an MS with the necessary optional tables
    void attach(MeasurementSet *ms);

private:
    Bool delms_p;
    uInt scannum_p, obsnum_p;
    Float autocorrwt_p;
    Double blockFrac_p;
    SimObservations *obs_p;
    MeasurementSet *ms_p;
    Quantity elevLimit_p;
    MEpoch start_p;        // the start of the first scan written with write()
    MEpoch end_p;          // the end of the last scan written with write()
};

} //# NAMESPACE CASA - END

#endif
