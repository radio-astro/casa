//# SimDataDesc.h: a container for Data Description records
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

#ifndef SIMULATORS_SIMDATADESC_H
#define SIMULATORS_SIMDATADESC_H

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MeasurementSet;
class MSDataDescription;
class MSPolarization;
class MSSpectralWindow;

// <summary> a container for data destined for an MS DATA_DESCRIPTION table
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimObservations">SimObservations</linkto>
//   <li> <linkto class="SimPol">SimPol</linkto>
//   <li> <linkto class="SimSpWin">SimSpWin</linkto>
// </prerequisite>
//
// <etymology>
// This is a container for describing a data description record for simulated 
// observations.
// </etymology>
//
// <synopsis> 
// This container describes a desired combination of polarization setup 
// and set of spectral windows, known in MS parlance, as data descriptions
// (as it defines the shape of the visibility data arrays).  
// This container is used to define simulated observations.  Simulators would
// not normally create these objects directly but rather implicitly via the
// <linkto class="SimObservations">SimObservations</linkto> class.  <p>
//
// The combination is defined in terms of IDs into an MS's POLARIZATION and
// SPECTRAL_WINDOW tables (which are usually managed via a 
// <linkto class="SimObservations">SimObservations</linkto> object), 
// similar to structure of a MeasurmentSet's DATA_DESCRIPTION record.  The
// difference being that in a SimDataDesc, a single polarization setup can be 
// with a set of windows as an aid to users simulating typical uses of 
// correlation spectrometers.  The IDs are set at construction time and 
// cannot be changed later.  <p>
// 
// Since this class has no connections either a spectral window table nor
// a polarization setup table, no checks are done to ensure that the IDs
// exist anywhere or are otherwise valid.  This must be done at a higher 
// level.  <p>
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a DATA_DESCRIPTION table.  
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS DATA_DESCRIPTION 
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimDataDesc {
public:

    // Create a record associating a polarization setup with a 
    // spectral window.  
    // polid  is the POLARIZATION_ID, and
    // spw    is the SPECTRALWINDOW_ID when upto=False.  
    // If upto is True, the polarization set up will be 
    // associated with all spectral windows with IDs less than spw.  
    SimDataDesc(uInt polId, uInt spw, Bool upto=False);

    // Create a record associating a polarization setup with a set of 
    // spectral windows
    SimDataDesc(uInt polId, const Vector<uInt>& spwids);

    // create a copy 
    // <group>
    SimDataDesc(const SimDataDesc& s);
    SimDataDesc& operator=(const SimDataDesc& s);
    // </group>

    ~SimDataDesc() { }

    // get and set the row ID, the first row in the DATA_DESCRIPTION  table 
    // containing this record.  An ID less than zero means that this record 
    // has not yet been recorded.
    // <group>
    Int getRow() const { return row_p; }
    void setRow(Int id) { row_p = id; }
    // </group>

    // return the ID of the polarization setup
    uInt polId() const { return pol_p; }

    // return the list of window IDs polarization setup is associated with.
    // An id < 0 indicates that the line is associated with all windows
    const Vector<uInt>& spectralWindowIds() const { return spwids_p; }

    // return the number of spectral windows refered to in this description
    uInt numWindows() const { return spwids_p.nelements(); }

private:
    Int row_p;
    uInt pol_p;
    Vector<uInt> spwids_p;
};

// <summary> a container for data destined for an MS DATA_DESCRIPTION table
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimDataDesc">SimDataDesc</linkto>
//   <li> <linkto class="SimObservations">SimObservations</linkto>
// </prerequisite>
//
// <etymology>
// This class holds a list of <linkto class="SimPol">SimDataDesc</linkto> 
// objects.
// </etymology>
//
// <synopsis> 
// This class holds a list of <linkto class="SimDataDesc">SimDataDesc</linkto> 
// instances which, as a group, describes a spectrometer set-up (typical
// of correlation spectrometers).  
// When using this class, one does not create SimDataDesc objects directly,
// but rather implicitly via the addDataDesc() function.
// The ultimate purpose of this container is to support simulated 
// observations.  Simulators would not normally create these objects 
// directly but rather implicitly via the 
// <linkto class="SimObservations">SimObservations</linkto> class.  <p>
//
// An important function handled by this class is the recording of the 
// data descriptiosn to a Measurement Set.  This is done with the flush()
// function.  When it writes the data from a SimDataDesc in its list to the 
// MS, it sets the row number of the window via setRow();
// this is used as a flag indicating that the SimDataDesc has been flushed.  
// This allows one to later add additional setups to the list; when flush() 
// is called again, only the new SimDataDesc data are written out.  This, of 
// course, assumes that the same output MSDataDescription is passed to the 
// flush() function each time.  If you want to write all the data to a new 
// DATA_DESCRIPTION table, you can call clearIds() to clear all the row 
// markers.  <p>
// 
// A list of data descriptions can be read in from a DATA_DESCRIPTION table 
// as well using the initFrom() method or the 
// SimDataDescList(const MSDataDescription&, ...) 
// constructor.  These will implicitly set the row markers from the input 
// table.  This allows one to add new setups to the already recorded set.
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// data descriptions that will ultimately be written to a DATA_DESCRIPTION 
// table.
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS DATA_DESCRIPTION 
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>

class SimDataDescList {
public: 
    // create an empty list
    SimDataDescList(uInt initsz=2, uInt stepsz=4)
	: n_p(0), chnk_p(stepsz), nid_p(0), rec_p(initsz, 0), bydd_p() {}

    // create a list and fill it with the contents of a DATA_DESCRIPTION table
    SimDataDescList(const MSDataDescription& ddt, uInt stepz=4);

    // create a copy
    // <group>
    SimDataDescList(const SimDataDescList& t);
    SimDataDescList& operator=(const SimDataDescList& t);
    // </group>

    ~SimDataDescList() { deleteRecs(); }
    
    // reset all the row markers used to flag the SimDataDesc members that 
    // have been recorded to a Measurement Set already.  Thus, the next call 
    // to flush() will record all SimPols to the DATA_DESCRIPTION table.  
    // This should be used when writing to a new DATA_DESCRIPTION table, 
    // different from one previously read from or written to.  
    void clearIds() {
	for(uInt i=0; i < n_p; i++) 
	    rec_p[i]->setRow(-1);
    }

    // access the i-th SimDataDesc object in this list.  
    // <group>
    SimDataDesc& operator[](Int i) { return get(i); }
    const SimDataDesc& operator[](Int i) const { return get(i); }
    // </group>

    // return the SimDataDesc for the data description with a given ID.
    // <group>
    SimDataDesc& dataDesc(uInt i) { return *bydd_p[i]; }
    const SimDataDesc& dataDesc(uInt i) const { return *bydd_p[i]; }
    // </group>

    // return the number of SimDataDesc objects in this list
    uInt numDesc() const { return n_p; }

    // return the number of data description IDs in this list.  That is,
    // return the number of window-polarization setup cominations in this list.
    uInt numIds() const { return nid_p; }

    // Add a description associating a polarization setup with a 
    // spectral window.  
    // polid  is the POLARIZATION_ID, and
    // spw    is the SPECTRALWINDOW_ID when upto=False.  
    // If upto is True, the polarization set up will be 
    // associated with all spectral windows with IDs less than spw.  
    SimDataDesc& addDataDesc(uInt polId, uInt spw, Bool upto=False) {
	SimDataDesc *out = new SimDataDesc(polId, spw, upto);
	add(out);
	return *out;
    }

    // Create a record associating a polarization setup with a set of 
    // spectral windows
    SimDataDesc& addDataDesc(uInt polId, const Vector<uInt>& spwids) {
	SimDataDesc *out = new SimDataDesc(polId, spwids);
	add(out);
	return *out;
    }

    // load a vector that provides a map of DataDescription IDs to 
    // spectral window.  If npol is >= 0, then only those data description
    // IDs that point to polarization IDs < npol will be included in the 
    // map.  Similarly, if nspw >= 0, only those data description
    // IDs that point to spectral window IDs < nspw will be included.
    // To get a map that is consistent with what is actually written out 
    // to the DATA_DESCRIPTION table by flush(), npol and nspw must both be
    // > 0.  When npol and nspw are not provided, the map returned is one
    // based on the assumption that all data description records are valid
    // and will be written out.
    uInt loadSpWinMap(Vector<uInt>& out, Int npol=-1, Int nspw=-1);

    // add data descriptions lookups from an MS
    void initFrom(const MSDataDescription& ddt);

    // write out all data descriptions that have yet to be written
    void flush(MSDataDescription& ddt, uInt npol, uInt nspw); 

private:
    void deleteRecs() {
	for(uInt i=0; i < n_p; i++) {
	    if (rec_p[i] != 0) delete rec_p[i];
	    rec_p[i] = 0;
	}
    }
    SimDataDesc& get(Int i) const {
	if (i < 0 || i >= static_cast<Int>(n_p)) throwOutOfRange(n_p-1, i);
	return *(rec_p[i]);
    }

    void throwOutOfRange(uInt legallength, Int index) const;

    void add(SimDataDesc *dop);

    uInt n_p, chnk_p, nid_p;
    Block<SimDataDesc*> rec_p;
    Block<SimDataDesc*> bydd_p;
};


} //# NAMESPACE CASA - END

#endif
