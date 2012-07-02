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

#ifndef SIMULATORS_SIMPOL_H
#define SIMULATORS_SIMPOL_H

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/Stokes.h>
#include <casa/Exceptions/Error.h>

#include <simulators/Simulators/SimFeed.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MSPolarization;

// <summary> a container for data destined for an MS POLARIZATION
// record </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
//   <li> <linkto class="SimFeed">SimFeed</linkto>
// </prerequisite>
//
// <etymology>
// This is a container for describing a polarization setup for a simulated 
// telescope.
// </etymology>
//
// <synopsis> 
// This describes a set of polarization correlations that are allowed from 
// a set of feeds.  The data model for this class relates closely to that 
// of the POLARIZATION table, indicating which feeds and which receptors 
// are cross-correlated to produce the set of correlations.  
// This container is used to define simulated observations.  Simulators would
// not normally create these objects directly but rather implicitly via the
// <linkto class="SimTelescope">SimTelescope</linkto> class.  <p>
//
// The correlation types, set either at construction or vis setCorrProducts(), 
// are represented as values from the 
// <linkto class="Stokes">Stokes::StokesTypes</linkto>
// enumeration.  To help ensure consistancy with the FEED table, one must 
// also provide the SimFeed objects for the feeds being correlated; these 
// feeds will be checked to ensure that the combinations are possible with 
// the receptors associated with the given feeds.  <p>
//
// A row marker, set with setId(), is used by 
// <linkto class="SimPolList">SimPolList</linkto> to indicate that this 
// SimPol has been written out to a POLARIZATION table.  <p>
// 
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a POLARIZATION table.  
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS POLARIZATION
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimPol {
public:
    // create an empty container
    SimPol();

    // initialize the container
    SimPol(const Vector<Stokes::StokesTypes>& corrTypes, 
		const SimFeed& feed1, const SimFeed& feed2);

    // make a copy of the container
    SimPol(const SimPol& other);
    SimPol& operator=(const SimPol& other);

    virtual ~SimPol() { }

    uInt feed1() const { return f1_p; }
    uInt feed2() const { return f2_p; }
    const Matrix<Int>& corrProducts() const { return prods_p; }
    const Vector<Int>& corrTypes() const { return types_p; }

    uInt numTypes() const { return types_p.nelements(); }
    Stokes::StokesTypes getCorrType(Int i) const { 
	return static_cast<Stokes::StokesTypes>(types_p[i]);
    }

    // set the correlation products using the given feeds.  If it is not 
    // possilble to form the correlations with the given feeds, the 
    // correlations are not set and False is returned.
    Bool setCorrProducts(const Vector<Stokes::StokesTypes>& corrTypes, 
			 const SimFeed& feed1, const SimFeed& feed2);

    Bool canUseFeed(const SimFeed& feed, Bool asFeed1);

    // get and set the ID.  An ID less than zero means that this record
    // has not yet been recorded.
    // <group>
    Int getId() const { return row_p; }
    void setId(Int id) { row_p = id; }
    // </group>

protected:

private:
    // determine what polarization receptors we need from each feed.
    void requiredReceptors(uInt& need1, uInt& need2, 
			   Vector<String>& stokes) const;

    Int row_p;
    uInt f1_p, f2_p;
    uInt p1_p, p2_p;
    Vector<Int> types_p;
    Matrix<Int> prods_p;
    Vector<String> stokes_p;

    static const String pnames_p;
    static const Int ptbits_p[];
};

// <summary> a container for data destined for an MS POLARIZATION
// table </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimPol">SimPol</linkto>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
// </prerequisite>
//
// <etymology>
// This class holds a list of <linkto class="SimPol">SimPol</linkto> 
// objects.
// </etymology>
//
// <synopsis> 
// This class holds a list of <linkto class="SimPol">SimPol</linkto> 
// instances which, as a group, describes a polarization set-up.
// When using this class, one does not create SimPol objects directly,
// but rather implicitly via the addPolSetup() function.
// The ultimate purpose of this container is to support simulated 
// observations.  Simulators would not normally create these objects 
// directly but rather implicitly via the 
// <linkto class="SimTelescope">SimTelescope</linkto> class.  <p>
//
// An important function handled by this class is the recording of the 
// polarization setups to a Measurement Set.  This is done with the flush()
// function.  When it writes the data from a SimPol in its list to the MS,
// it sets the row number of the window via setRow();
// this is used as a flag indicating that the SimPol as been flushed.  This
// allows one to later add additional setups to the list; when flush() is 
// called again, only the new SimPol data are written out.  This, of course, 
// assumes that the same output MSPolarization is passed to the flush() 
// function each time.  If you want to write all the data to a new 
// POLARIZATION table, you can call clearIds() to clear all the row 
// markers.  <p>
// 
// A list of polarization setups can be read in from a POLARIZATION table 
// as well using the initFrom() method or the 
// SimPolList(const MSPolarization&, ...) 
// constructor.  These will implicitly set the row markers from the input 
// table.  This allows one to add new setups to the already recorded set.
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// spectral windows that will ultimately be written to a POLARIZATION table.
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS POLARIZATION
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimPolList {
public:
    // create an empty buffer
    SimPolList(uInt initsz=2, uInt stepsz=4)
	: n_p(0), chnk_p(stepsz), rec_p(initsz, 0) 
    {
	for(uInt i=0; i < initsz; i++) rec_p[i] = 0;
    }

    // create a buffer and fill it with the contents of a POLARIZATION table
    SimPolList(const MSPolarization& polt, const SimFeedList& fdl, 
	       uInt stepsz=4);

    // make a copy of the container
    // <group>
    SimPolList(const SimPolList& other);
    SimPolList& operator=(const SimPolList& other);
    // </group>

    ~SimPolList() { deleteRecs(); }

    // return the number of setups currently defined.
    uInt numSetups() const { return n_p; }

    // reset all the row markers used to flag the SimPol members that 
    // have been recorded to a Measurement Set already.  Thus, the next call 
    // to flush() will record all SimPols to the POLARIZATION table.  
    // This should be used when writing to a new POLARIZATION table, 
    // different from one previously read from or written to.  
    void clearIds() {
	for(uInt i=0; i < n_p; i++) 
	    rec_p[i]->setId(-1);
    }

    // access the i-th polarization setup in this list.  
    // <group>
    SimPol& operator[](Int i) { return get(i); }
    const SimPol& operator[](Int i) const { return get(i); }
    // </group>

    // add a polarization setup for a set of correlations.  If the correlations
    // cannot be formed with the given feeds, the correlations will not be
    // added and null is returned.
    SimPol* addPolSetup(const Vector<Stokes::StokesTypes>& corrTypes, 
		     const SimFeed& feed1, const SimFeed& feed2)
    {
	SimPol *out = new SimPol();
	if (! out->setCorrProducts(corrTypes, feed1, feed2)) {
	    delete out;
	    return 0;
	}
	if (n_p == rec_p.nelements()) {
	    rec_p.resize(n_p+chnk_p);
	    for(uInt i=n_p; i < rec_p.nelements(); i++) rec_p[i] = 0;
	}
	rec_p[n_p++] = out;
	return out;
    }

    // fill list with the contents of a POLARIZATION table
    void initFrom(const MSPolarization& polt, const SimFeedList& fdl);

    // write out all windows that have yet to be written
    void flush(MSPolarization& polt);

private:
    void deleteRecs() {
	for(uInt i=0; i < n_p; i++) {
	    if (rec_p[i] != 0) delete rec_p[i];
	    rec_p[i] = 0;
	}
    }

    SimPol& get(Int i) const {
	if (i < 0 || i >= static_cast<Int>(n_p)) throwOutOfRange(n_p-1, i);
	return *(rec_p[i]);
    }

    void throwOutOfRange(uInt legallength, Int index) const;
    static Bool feedsGiveProducts(const SimFeed& fd1, const SimFeed& fd2, 
				  const Vector<Stokes::StokesTypes> ctype,
				  const Matrix<Int>& cprod);

    uInt n_p, chnk_p;
    Block<SimPol*> rec_p;
};


} //# NAMESPACE CASA - END

#endif
