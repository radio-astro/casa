//# SimSpWin.h: container for containing Spectral Window description
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

#ifndef SIMULATORS_SIMSPWIN_H
#define SIMULATORS_SIMSPWIN_H

#include <casa/Arrays/Vector.h>
#include <measures/Measures/MFrequency.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MSSpectralWindow;

// <summary> a container for data destined for an MS SPECTRAL_WINDOW 
// record </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
// </prerequisite>
//
// <etymology>
// This is a container for describing a spectral window setup for a simulated
// telescope.
// </etymology>
//
// <synopsis> 
// This class describes a single spectral window from a spectrometer of 
// a telescope.  It assumes a simple model in which a window is made up of 
// one or more contiguous, uniform channels, typical of spectral 
// correlator found on modern interferometers.
// This container is used to define simulated observations.  Simulators would
// not normally create these objects directly but rather implicitly via the
// <linkto class="SimTelescope">SimTelescope</linkto> class.  <p>
//
// The data describing the window is set at construction time and cannot be
// changed later.  The data model assumes that the 
// spectral channels that can be described by a starting frequency, a 
// frequency increment, and a total number of channels.  Furthermore, all
// channels are assumed to have the same channel bandwidth.  The window 
// can be given a name and a sideband designation.  The only writable data 
// of this class are a row marker (via setRow()), used by 
// <linkto class="SimSpWinList">SimSpWinList</linkto> to keep track of 
// windows that have been written out to a SPECTRAL_WINDOW table in a 
// Measurement Set, and a uniform flag (via setUniform()) which is used by 
// <linkto class="SimSpWinList">SimSpWinList</linkto> to indicate that a
// window read in from an MS does not really fit into the simple uniform
// data model.  Neither of these writable items affect what is written out
// to a Measurement Set.
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// spectral windows that will ultimately be written to a SPECTRAL_WINDOW table.
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS SPECTRAL_WINDOW
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimSpWin {
public:
    // initialize the container
    // <group>
    SimSpWin(uInt nchan, const MFrequency& start, 
	     const MVFrequency& width, const MVFrequency& step, 
	     const String& name="", Bool usb=True);
    SimSpWin(uInt nchan, const MFrequency& start, 
	     const MVFrequency& width, const MVFrequency& step, 
	     const MFrequency refFreq, const String& name="", 
	     Bool usb=True);
    // </group>

    // initialize the container to a single-channel continuum window
    SimSpWin(const MFrequency& start, const MVFrequency& width,
	     const String& name="", Bool usb=True);

    // make a copy of the container
    // <group>
    SimSpWin(const SimSpWin& other);
    SimSpWin& operator=(const SimSpWin& other);
    // </group>

    ~SimSpWin() { }

    uInt numChannels() const { return nch_p; }
    Bool isUpperSideBand() const { return usb_p; }
    const MFrequency& startFreq() const { return sfreq_p; }
    const MFrequency& refFreq() const { return ref_p; }
    const MVFrequency& chanWidth() const { return width_p; }
    const MVFrequency& chanStep() const { return step_p; }

    // return the net sideband code 
    Int netSideband() const { return ((usb_p) ? 1 : -1); }

    // load a Vector with the channel frequencies in Hertz
    Vector<Double>& chanFreq(Vector<Double> &freqs) const {
	Double start = sfreq_p.get("Hz").getValue();
	Double step = step_p.get("Hz").getValue();
	freqs.resize(nch_p);
	for(uInt chan=0; chan < nch_p; chan++) 
	    freqs(chan) = start + chan*step;
	return freqs;
    }

    // load a Vector with the channel widths in Hertz
    Vector<Double>& chanWidth(Vector<Double>& widths) const {
	widths.resize(nch_p);
	widths = width_p.get("Hz").getValue();
	return widths;
    }

    // return the total bandwidth of the window in Hz.  This is equal to 
    // the step size times the number of channels.
    Double totalBandwidth() const {
	return step_p.get("Hz").getValue() * nch_p;
    }

    // get and set the name of the window
    // <group>
    const String& getName() const { return name_p; }
    void setName(const String& name) { name_p = name; }
    // </group>

    // get and set the ID.  An ID less than zero means that this record
    // has not yet been recorded.
    // <group>
    Int getId() const { return row_p; }
    void setId(Int id) { row_p = id; }
    // </group>

    // get and set the uniformity flag.  This flag is  
    // False if the set up is not actually uniform enough to be 
    // described by this container.  This will be the case if the 
    // set up originated from an atypical MS.
    // <group>
    Bool isUniform() const { return unif_p; }
    void setUniform(Bool flag) { unif_p = flag; }
    // </group>

protected:
    // create an empty container
    SimSpWin();

private:
    Int row_p;
    uInt nch_p;
    Bool usb_p, unif_p;
    String name_p;
    MFrequency sfreq_p, ref_p;
    MVFrequency width_p, step_p;
};

// <summary> a container for data destined for an MS SPECTRAL_WINDOW 
// record </summary>

// <use visibility=export>

 // <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimSpWin">SimSpWin</linkto>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
// </prerequisite>
//
// <etymology>
// This class holds a list of <linkto class="SimSpWin">SimSpWin</linkto> 
// objects.
// </etymology>
//
// <synopsis> 
// This class holds a list of <linkto class="SimSpWin">SimSpWin</linkto> 
// instances which, as a group, describes a spectrometer set-up (typical
// of correlation spectrometers).  
// When using this class, one does not create SimSpWin objects directly,
// but rather implicitly via the addWindow() function.  (A special version
// is available for adding single-channel windows.)
// The ultimate purpose of this container is to support simulated 
// observations.  Simulators would not normally create these objects 
// directly but rather implicitly via the 
// <linkto class="SimTelescope">SimTelescope</linkto> class.  <p>
//
// An important function handled by this class is the recording of the 
// window descriptions to a Measurement Set.  This is done with the flush()
// function.  When it writes the data from a SimSpWin in its list to the MS,
// it sets the row number of the window via setRow();
// this is used as a flag indicating that the SimSpWin as been flushed.  This
// allows one to later add additional windows to the list; when flush() is 
// called again, only the new SimSpWin data are written out.  This, of course, 
// assumes that the same output MSSpectralWindow is passed to the flush() 
// function each time.  If you want to write all the data to a new 
// SPECTRAL_WINDOW table, you can call clearIds() to clear all the row 
// markers.  <p>
// 
// A set of windows can be read in from a SPECTRAL_WINDOW table as well using 
// the initFrom() method or the SimSpWinList(const MSSpectralWindow&, ...) 
// constructor.  These will implicitly set the row markers from the input 
// table.  This allows one to add new windows to the already recorded set.
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// spectral windows that will ultimately be written to a SPECTRAL_WINDOW table.
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS SPECTRAL_WINDOW
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimSpWinList {
public:
    // create an empty buffer
    SimSpWinList(uInt initsz=2, uInt stepsz=4)
	: n_p(0), chnk_p(stepsz), rec_p(initsz, 0) {}

    // create a buffer and fill it with the contents of a SPECTRAL_WINDOW table
    SimSpWinList(const MSSpectralWindow& spwt, uInt stepsz=4);

    // create a copy 
    // <group>
    SimSpWinList(const SimSpWinList& other);
    SimSpWinList& operator=(const SimSpWinList& other);
    // </group>

    ~SimSpWinList() { deleteRecs(); }

    // reset all the row markers used to flag the SimSpWin members that 
    // have been recorded to a Measurement Set already.  Thus, the next call 
    // to flush() will record all SimSpWins to the SPECTRAL_WINDOW table.  
    // This should be used when writing to a new SPECTRAL_WINDOW table, 
    // different from one previously read from or written to.  
    void clearIds() {
	for(uInt i=0; i < n_p; i++) 
	    rec_p[i]->setId(-1);
    }

    // access the i-th window in this list.  
    // <group>
    SimSpWin& operator[](Int i) { return get(i); }
    const SimSpWin& operator[](Int i) const { return get(i); }
    // </group>

    // return the number of windows described in this list
    Int numWindows() const { return n_p; }

    // add a multi-channel spectral window
    // <group>
    SimSpWin& addWindow(uInt nchan, const MFrequency& start, 
			const MVFrequency& width, const MVFrequency& step, 
			const MFrequency &refFreq, const String& name="", 
			Bool usb=True) 
    {
	SimSpWin *out = new SimSpWin(nchan, start, width, step, 
				     refFreq, name, usb);
	add(out);
	return *out;
    }
    SimSpWin& addWindow(uInt nchan, const MFrequency& start, 
			const MVFrequency& width, const MVFrequency& step, 
			const String& name="", Bool usb=True) 
    {
	SimSpWin *out = new SimSpWin(nchan, start, width, step, name, usb);
	add(out);
	return *out;
    }
    // </group>

    // add a single channel continuum window
    SimSpWin& addWindow(const MFrequency& start, const MVFrequency& width,
			const String& name="", Bool usb=True)
    {
	SimSpWin *out = new SimSpWin(start, width, name, usb);
	add(out);
	return *out;
    }

    // load the number of channels in each window into given Vector.
    // The input Vector, nchan, will be resized to the value returned by 
    // numWindows().
    void numChannels(Vector<Int> nchan) const;

    // add window descriptions from an MS SPECTRAL_WINDOW table
    void initFrom(const MSSpectralWindow& spwt);

    // write out all windows that have yet to be written
    void flush(MSSpectralWindow& spwt);

private:
    void deleteRecs() {
	for(uInt i=0; i < n_p; i++) {
	    if (rec_p[i] != 0) delete rec_p[i];
	    rec_p[i] = 0;
	}
    }
    SimSpWin& get(Int i) const {
	if (i < 0 || i >= static_cast<Int>(n_p)) throwOutOfRange(n_p-1, i);
	return *(rec_p[i]);
    }

    void throwOutOfRange(uInt legallength, Int index) const;

    void add(SimSpWin *win) {
	if (n_p == rec_p.nelements()) {
	    rec_p.resize(n_p+chnk_p);
	    for(uInt i=n_p; i < rec_p.nelements(); i++) rec_p[i] = 0;
	}
	rec_p[n_p++] = win;
    }

    uInt n_p, chnk_p;
    Block<SimSpWin*> rec_p;
};



} //# NAMESPACE CASA - END

#endif
