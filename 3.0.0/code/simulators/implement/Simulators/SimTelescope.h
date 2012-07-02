//# SimTelescope.h: container for containing key telescope setup info in memory
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

#ifndef SIMULATORS_SIMTELESCOPE_H
#define SIMULATORS_SIMTELESCOPE_H

#include <simulators/Simulators/SimFeed.h>
#include <simulators/Simulators/SimArray.h>
#include <simulators/Simulators/SimSpWin.h>
#include <simulators/Simulators/SimPol.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MeasurementSet;

// <summary> A container for data describing a simulated interferometer 
// </summary>
// <reviewed reviewer="" date="" tests="">
//
// <etymology>
// This class describes a simulated telescope.
// </etymology>
//
// <synopsis> 
// This class serves as a container for describing a simulated interferometer
// which, when used in tandem with 
// <linkto class="SimObservations">SimObservations</linkto>, can be used to 
// describe simulated observations.  The difference between SimTelescope and 
// <linkto class="SimObservations">SimObservations</linkto> is somewhat 
// arbitrary: this class focuses on the hardware configuration--namely, the 
// array configuration, the spectrometer set-up, the definition of feeds, 
// and desired polarization correlations.  These classes provide methods for
// defining the telescope and observations; a simulator class, such as 
// <linkto class="SimpleSimulator">SimpleSimulator</linkto> uses them to 
// actually carry out the simulated observations and recording the data
// held in this class to a Measurement Set.  <p>
// 
// This class uses some underlying container classes for storing the 
// specific parts of the configuration which correspond roughly to records 
// in the relevent sub-tables of a Measurement Set:  
// <ul>
//   <li> ANTENNA:  <linkto class="SimArray">SimArray</linkto>
//   <li> SPECTRAL_WINDOW:  <linkto class="SimArray">SimSpwin</linkto>
//   <li> FEED: <linkto class="SimFeed">SimFeed</linkto>
//   <li> POLARIZATION:  <linkto class="SimPol">SimPol</linkto>
// </ul>
// Normally, these classes are not handled directly by users of this class; 
// however, it is possible gain direct access when finer read or write control 
// over the data they hold is necessary.  <p>
//
// The data contained in this object will ultimately be written out to a 
// Measurement Set.  To accomplish this, one must "attach" an output 
// MeasurementSet to this object.  This can be done either at construction
// time or later via the attach() or initAndAttach() function.  If 
// initAndAttach() is used or attachment is done at construction, any data
// already recorded in the MS, such as antenna descriptions, are read-in and 
// loaded into this object.  New data is written out via the flush() function.
// The "attachment" model allows the class to keep track of what's been 
// written to disk already.  Thus, one can update the telescope configuration,
// e.g. add additional antennas, and write out only the new data via the 
// flush() command.  <p>
//
// It is intended that a SimTelescope object be managed by a 
// <linkto class="SimObservations">SimObservations</linkto> object.  
// Normally, a SimTelescope is constructed first and passed to the 
// constructor of the SimObservations class.  One would access the 
// SimTelescope directly via the SimObservations::telescope() function.  
// Attaching an MS to a SimObservations object will in turn attach it to its 
// underlying SimTelescope.  Accordingly, calling SimObservations::flush() 
// will implicitly call SimTelescope::flush().  <p>
// </synopsis> 
//
// <motivation>
// By breaking out telescope description from MSSimulator, the simulator can 
// take on more complex sequences of observations from multiple telescopes,
// subarrays, configurations, etc.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> since this models an interferometer and not a single dish telescope,
//        it probably should have its name changed.  
//   <li> loading from an existing MS needs more testing.
// </todo>
class SimTelescope {
public:
    enum LocalCoord { CIRCUM=SimArray::CIRCUM, TAN=SimArray::TAN, 
		      EQUITORIAL=SimArray::EQUITORIAL, 
		      NTypes=SimArray::NTypes };

    static const String defaultName;
    static const MPosition defaultLocation;

    // Create an undescribed telescope
    SimTelescope(const String& name=defaultName);

    // Create an telescope at the given reference location
    SimTelescope(const MPosition& aryLoc, const String& name=defaultName);

    // Create a telescope that is a copy of another telescope
    SimTelescope(const SimTelescope &tel);
    SimTelescope& operator=(const SimTelescope &tel);

    // Create a telescope described by the data in a MeasurementSet.  The
    // first version (taking a const MeasurmentSet) simply calls init(ms), 
    // while the second one calls initAndAttach().
    // <group>
    SimTelescope(const MeasurementSet &ms);
    SimTelescope(MeasurementSet &ms);
    // </group>

    virtual ~SimTelescope();

    // Load the description given by the data in a MeasurementSet
    // The ANTENNA, FEED, SPECTRAL_WINDOW, POLARIZATION, and DATA_DESCRIPTION
    // subtables will be read to determine the basic telscope description,
    // then the main table is scanned to determine the currently defined 
    // subarrays.  (This constructor will read the MeasurementSet as if it 
    // describes a single telescope; thus, some complexity may be lost.)
    // The second version will also call attach(ms, False);
    // <group>
    void init(const MeasurementSet &ms);
    void initAndAttach(MeasurementSet &ms) {
	init(ms);
	attach(ms, False);
    }
    // </group>

    // Attach this description to a MeasurementSet so that when flush() is 
    // called, the description will get written to it.  
    //  clear, if set to True, will cause all internal flags that mark 
    //         which descriptions have been written so far to be cleared.
    //         Thus, the next time flush is called, all configuration records
    //         currently set will be written to the attached MS.
    void attach(MeasurementSet &ms, Bool clear=False);

    // return a pointer to the currently attached Measurement Set or null
    // if none is currently attached.
    MeasurementSet *ms() { return ms_p; }

    // Write out the current description to a given MeasurementSet.  Only 
    // those "new" records that have not already been written out to the MS 
    // will be flushed.  
    void flush();

    // Set and get a name for this simulated telescope
    // <group>
    void setName(const String& name) { name_p = name; }
    const String& getName() const { return name_p; }
    // </group>

    // Set and get the reference location for the telescope
    // <group>
    void setRefLocation(const MPosition& loc) { refLoc_p = loc; }
    const MPosition& getRefLocation() const { return refLoc_p; }
    // </group>

    // Define a set of antennas assuming the current reference location for 
    // the array.  The size of all input arrays must be equal
    // to the number antennas being set.  
    //  x,y,z       define the position of the antennas relative to the 
    //                 reference location.
    //  unit        is the unit of time or length used to give positions (if 
    //                 unit is time, values will be multiplied by the speed 
    //                 of light).  This value should be a string recognized 
    //                 as a time or length by the MVTime and MVBaseline (via
    //                 Quantity).
    //  coord       is the coordinate system for the position vector x,y,z; 
    //                 the value is taken from the 
    //                 <linkto class="SimArray">SimArray::LocalCoord</linkto>
    //                 enumeration.  
    //  refloc      is the reference location for the array.
    //  diam        is the diameter of the dishes
    //  mount       is name of the mount employed by the antennas
    //  antNames    is a list of antenna names (e.g. "Ant 1").
    //  stationNames is a list of station names (e.g. "N8").
    //  offset      is the offset from the "feed reference position" from the
    //                 antenna positions, given as a pointer.
    //  subarrayID  is default subarray to associate these antennas with.
    // <group>
    uInt addAntennas(const Vector<Double>& x,
		     const Vector<Double>& y,
		     const Vector<Double>& z, const String& unit,
		     const LocalCoord coord, const Quantity& diam, 
		     const String& mount, const MPosition *offset=0,
		     uInt subarrayId=0) 
    {
	return ary_p.addAntennas(x, y, z, unit, 
				 SimArray::LocalCoord(Int(coord)), 
				 refLoc_p, diam, mount, offset, 
				 subarrayId).numAnts();
				 
    }
    uInt addAntennas(const Vector<Double>& x,
		     const Vector<Double>& y,
		     const Vector<Double>& z, const String& unit,
		     const LocalCoord coord, const Quantity& diam, 
		     const String& mount, const Vector<String>& antNames,
		     const Vector<String>& stationNames,
		     const MPosition *offset=0,
		     uInt subarrayId=0)
    {
	return ary_p.addAntennas(x, y, z, unit, 
				 SimArray::LocalCoord(Int(coord)), 
				 refLoc_p, diam, mount, 
				 antNames, stationNames, offset, 
				 subarrayId).numAnts();
    }
    // </group>

    // Add a single antenna.  This is useful for adding elements of a VLBI 
    // network of heterogenous dishes one at a time.
    //  name        the name of the antenna or station
    //  pos         defines the absolute position on the Earth of an
    //                 antenna (the reference Location is not used).
    //  diam        is the diameter of the dishes
    //  mount       is the mount used on the antenna
    //  subarrayId  is default subarray to associate these antennas with.
    uInt addAntenna(const String& name, const MPosition& pos, 
		    const Quantity& diam, const String& mount, 
		    const MPosition *offset=0, uInt subarrayId=0) 
    {
	return ary_p.addAntenna(name, pos, diam, mount, offset, 
				subarrayId).numAnts();
    }

    // fill the currently configured antennas positions into a given vector.
    // The vector will be resized appropriately.  The number of antennas is
    // returned.
    uInt antennaPositions(Vector<MPosition>& pos) const {
	return ary_p.antennaPositions(pos);
    }

    // fill the currently configured antenna positions as an array of 
    // ITRF position vectors.  The first axis of the given Matrix represents
    // the 3 components of the position vector in units of meters in the 
    // ITRF frame; the second axis is equal to the number of antennas in 
    // this subarray.  start is the position along the second axis to place 
    // The matrix will be resized appropriately.  The number of antennas is
    // returned.
    uInt antennaPositions(Matrix<Double>& pos) const {
	return ary_p.antennaPositions(pos);
    }

    // fill the antenna names into a given Vector of Strings.  If the names 
    // have not been set, the names will be returned as empty strings.  In 
    // any event, the given vector will be resized.  The number of antennas 
    // is returned.
    uInt antennaNames(Vector<String>& names) const {
	return ary_p.antennaNames(names);
    }

    // fill the station names into a given Vector of Strings.  If the names 
    // have not been set, the names will be returned as empty strings.  In 
    // any event, the given vector will be resized.  The number of antennas 
    // is returned.
    uInt stationNames(Vector<String>& names) const {
	return ary_p.stationNames(names);
    }

    // return number of currently added antennas
    uInt numAnts() const { return ary_p.numAnts(); }

    // return a description of an antenna
    const SimArray& getAntennaDesc(uInt i) const { return ary_p.antDesc(i); }

    // remove all array descriptions
    void removeAllAntennas() { ary_p.clearRecs(); }

    // Add a multi-channel spectral window.  Channels are restricted to be 
    // regularly spaced and having resolutions equal to the channel widths.
    //  nchan    the number of channels in the window
    //  start    the starting frequency
    //  width    the channel width/resolution
    //  step     the difference in frequency between each channel
    //  refFreq  the reference frequency; if not specified, the central channel
    //              will be used.  
    //  usb      if True, mark as upper sideband; otherwise, lower sideband.
    // The ID of the new window will be returned.
    // <group>
    uInt addSpWindow(uInt nchan, const MFrequency& start, 
		     const MVFrequency& width, const MVFrequency& step, 
		     const MFrequency& refFreq, const String& name="", 
		     Bool usb=True)
    {
	wins_p.addWindow(nchan, start, width, step, refFreq, name, usb);
	return wins_p.numWindows()-1;
    }
    uInt addSpWindow(uInt nchan, const MFrequency& start, 
		     const MVFrequency& width, const MVFrequency& step, 
		     const String& name="", Bool usb=True)
    {
	wins_p.addWindow(nchan, start, width, step, name, usb);
	return wins_p.numWindows()-1;
    }
    // </group>

    // add a single channel continuum window
    //  freq     the starting frequency
    //  width    the channel width/resolution
    //  usb      if True, mark as upper sideband; otherwise, lower sideband.
    // The ID of the new window will be returned.
    uInt addSpWindow(const MFrequency& freq, const MVFrequency& width,
		     const String& name="", Bool usb=True)
    {
	wins_p.addWindow(freq, width, name, usb);
	return wins_p.numWindows()-1;
    }

    // return the number of windows currently configured
    uInt numSpWins() { return wins_p.numWindows(); }

    const SimSpWin& getWindowDesc(uInt i) { return wins_p[i]; }

    // Add a feed.  
    //  id            is the ID to associate with the feed.  
    //  pols          is a string containing the polarizations of the
    //                  receptors.  There should be one character for each 
    //                  receptor and should be equal to one of "XYRL".
    //  position      give the position of the feed relative to the "feed 
    //                  reference position"
    //  recepAngle    hold the receptor position angles (assuming linear 
    //                  polarization receptor)
    //  beamOff       holds the beam offset for each receptor  
    //  polResp       holds polarization response of each receptor
    //  antId         is the antenna to associated this feed description with; 
    //                  if negative, then associate with all antenna
    //  time,interval specify the time interval over which this description is
    //                  correct
    //  spwId         is the window to associate with this feed. 
    uInt addFeed(uInt id, const String& pols, 
		 const MPosition *position=0,
		 const Vector<Double> *receprAngle=0,
		 const Matrix<Double> *beamOff=0, 
		 const Matrix<Complex> *polResp=0,
		 Int antId=-1, Double time=0, Double interval=0,
		 Int spwId=-1);

    // return the number of feed entries
    uInt numFeeds() const { return feeds_p.numFeeds(); }

    const SimFeed& getFeedDesc(uInt i) { return feeds_p[i]; }

    // set polarization correlations.  An exception is thrown if the requested
    // correlations cannot be formed by the given feeds.  setFeed() must be
    // called at least once.
    //  corrTypes  lists the desired correlations.  This vector should contain 
    //                one, two or four types, each of which must be one of the 
    //                cross-correlation types (e.g. XX, XY, RR, LR, XR, etc.).
    //                An exception is thrown if the list has an invalid number
    //                of elements.  
    //  feed1      is the feed containing the first receptor in the correlation.
    //  feed2      is the feed containing the second receptor in the 
    //                correlation.
    uInt addPolCorrs(const Vector<Stokes::StokesTypes>& corrTypes, 
		     uInt feed1=0, uInt feed2=0);

    // set polarization correlations.  This is a convenience function for 
    // easily selecting correlations across feeds with the same receptors
    // If this is the case, you can get parallel hand correlations (e.g. RR, 
    // LL) by request ncorr=2.  For full cross-correlation, use ncorr=4.  In 
    // detail, 
    // 
    //  ncorr      is an integer equal to 1, 2, or 4 indicating the number 
    //                of correlations desired.  When ncorr=4, all combinations
    //                of the first two receptors in each feed will be set.
    //                If ncorr=2, only 2 combinations will be recorded: the 
    //                first receptor from each correlated and the second 
    //                receptor from each.  If ncorr=1, only a single correlation
    //                between the same receptor on each will be recorded; the
    //                chosen receptor is given by the value of whichSingle.
    //  feed1      is the feed containing the first receptor in the correlation.
    //  feed2      is the feed containing the second receptor in the 
    //                correlation.
    //  whichSingle  is the index of the receptor in each feed that should 
    //                be correlated when ncorr=1; it is ignored otherwise.
    uInt addPolCorrs(uInt ncorr, uInt feed1=0, uInt feed2=0, 
		     uInt whichsingle=0);

    uInt numPolSetups() { return pols_p.numSetups(); }

    const SimPol& getPolSetup(uInt i) { return pols_p[i]; }

private:
    String name_p;

    MPosition refLoc_p;
    SimArrayList ary_p;
    SimSpWinList wins_p;
    SimFeedList feeds_p;
    SimPolList pols_p;

    MeasurementSet *ms_p;
};


} //# NAMESPACE CASA - END

#endif
