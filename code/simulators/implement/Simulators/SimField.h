//# SimField.h: a source for simulated observations
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

#ifndef SIMULATORS_SIMFIELD_H
#define SIMULATORS_SIMFIELD_H

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MDirection.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MeasurementSet;
class MSSource;
class MSField;
template <class K, class V> class SimpleOrderedMap;

// <summary> a container for data destined for an MS FIELD table
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimObservations">SimObservations</linkto>
//   <li> <linkto class="SimSource">SimSource</linkto>
// </prerequisite>
//
// <etymology>
// This is a container for describing fields to be observed in simulated 
// observations.
// </etymology>
//
// <synopsis> 
// This container describes a field to be observed in a set
// of simulated observations.  It provides a simplified data model for a field
// that assumes that the field position is not moving in time and that its
// delay, phase, and reference directions are all the same.  Furthermore, it
// provides an observer-oriented interface that allows one to specify the 
// field direction in terms of offsets from a source or reference direction.  
// This container is used to define simulated observations.  Simulators would
// not normally create these objects directly but rather implicitly via the
// <linkto class="SimObservations">SimObservations</linkto> class.  <p>
//
// The field direction and the associated source ID are set at construction 
// time and cannot be changed.  No check is done to ensure that the source 
// ID exists or is otherwise valid; this must be done at a higher level.
// The only writable data in this class are a row marker (via setRow(), used by 
// <linkto class="SimFieldList">SimFieldList</linkto>) and an optional 
// repetition factor.  The repetition factor (set with setRepetition()) 
// indicates how many times this field should be observed consecutively before
// observing slews to the next field.  This value is not recorded in an MS
// FIELD table.  (The <linkto class="SimpleSimulator">SimpleSimulator</linkto>
// uses this as a default repetition value if one is not set at a higher 
// level.)  <p>
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a FIELD table.  
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS FIELD table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimField {
public:

    // Create a record.  
    //  srcId    is the source ID that this field is associated with.
    //  dir      is normally the position of the source referenced by srcId 
    //               so that longOffset and latOffset gives the relative
    //               position of the field center; however, dir will be the
    //               absolute position of the field when 
    //               longOffset=latOffset=0.
    //  longOffset  is the true angular offset in radians in the direction
    //               perpendicular to the celestial pole (i.e. along a great
    //               circle) of the field center from the source position
    //               given by dir.
    //  latOffset   is the angular offset in radians in the direction of the 
    //               celestial (north) pole of the field center from the 
    //               source position.
    SimField(uInt srcId, const MDirection& dir, Double longOffset=0.0, 
	     Double latOffset=0.0, const String& name="",
	     const String& code="") 
	: row_p(-1), src_p(srcId), rep_p(0), off_p(2,0.0), name_p(name), 
	  code_p(code), dir_p(dir) 
    {
	if (longOffset != 0.0 || latOffset != 0.0) {
	    dir_p.shift(longOffset, latOffset, True);
	    off_p(0) = longOffset;
	    off_p(1) = latOffset;
	}
    }

    // create a copy
    // <group>
    SimField(const SimField& s);
    SimField& operator=(const SimField& s);
    // </group>

    ~SimField() { }

    // get and set the row ID, the row in the FIELD table containing this 
    // record.  An ID less than zero means that this record has not yet 
    // been recorded.
    // <group>
    Int getRow() const { return row_p; }
    void setRow(Int id) { row_p = id; }
    // </group>

    // return the ID of the source observed in this field
    uInt sourceId() const { return src_p; }

    // return the name of the field
    const String& name() const { return name_p; } 

    // return the code of the field
    const String& code() const { return code_p; } 

    // return the offset from the source position for this field.  The 
    // first element is the longitude offset and the second is the 
    // latitude offset.  Both are in radians.
    const Vector<Double>& offset() const { return off_p; }

    // return the direction of the field center.  (This will recorded 
    // simulataneously as DELAY_DIR, PHASE_DIR, and REFERENCE_DIR.)  
    const MDirection& direction() const { return dir_p; }

    // compute the shift between to positions.  The returned vector is 
    // the same as returned from offset().  The values are only approximate.
    static Vector<Double> offset(const MDirection srcdir, 
				 const MDirection flddir);

    // get and set the number of consecutive integrations to perform on 
    // this field.  A value of zero means a suitable default value should 
    // be used.  
    // <group>
    uInt repetition() const { return rep_p; } 
    void setRepetition(uInt n) { rep_p = n; } 
    // </group>
    
private:
    Int row_p;
    uInt src_p, rep_p;
    Vector<Double> off_p;
    String name_p, code_p;
    MDirection dir_p;
};

// <summary> a container for data destined for an MS FIELD table
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimField">SimField</linkto>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
// </prerequisite>
//
// <etymology>
// This class holds a list of <linkto class="SimField">SimField</linkto> 
// objects.
// </etymology>
//
// <synopsis> 
// This class holds a list of <linkto class="SimField">SimField</linkto> 
// instances which, as a group, describes a list of sources to observe.
// When using this class, one does not create SimField objects directly,
// but rather implicitly via the addSource() function.
// The ultimate purpose of this container is to support simulated 
// observations.  Simulators would not normally create these objects 
// directly but rather implicitly via the 
// <linkto class="SimObservations">SimObservations</linkto> class.  <p>
//
// An important function handled by this class is the recording of the 
// fields to a Measurement Set.  This is done with the flush()
// function.  When it writes the data from a SimField in its list to the MS,
// it sets the row number of the source record via setRow();
// this is used as a flag indicating that the SimField has been flushed.  This
// allows one to later add additional setups to the list; when flush() is 
// called again, only the new SimField data are written out.  This, of 
// course, assumes that the same output MSDataDescription is passed to the 
// flush() function each time.  If you want to write all the data to a new 
// FIELD table, you can call clearIds() to clear all the row 
// markers.  <p>
// 
// A list of fields can be read in from a FIELD table 
// as well using the initFrom() method or the SimFieldList(const MSField&, ...) 
// constructor.  These will implicitly set the row markers from the input 
// table.  This allows one to add new setups to the already recorded set.
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// sources that will ultimately be written to a FIELD table.
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS FIELD 
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimFieldList {
public: 
    // create an empty list
    SimFieldList(uInt initsz=2, uInt stepsz=4)
	: n_p(0), chnk_p(stepsz), rec_p(initsz, 0) {}

    // create a list and fill it with the contents of a FIELD table
    SimFieldList(const MSField& fldt, uInt stepz=4);

    // create a copy
    // <group>
    SimFieldList(const SimFieldList& t);
    SimFieldList& operator=(const SimFieldList& t);
    // </group>

    ~SimFieldList() { deleteRecs(); }
    
    // reset all the row markers used to flag the SimField members that 
    // have been recorded to a Measurement Set already.  Thus, the next call 
    // to flush() will record all SimFields to the FIELD table.  
    // This should be used when writing to a new FIELD table, 
    // different from one previously read from or written to.  
    void clearIds() {
	for(uInt i=0; i < n_p; i++) 
	    rec_p[i]->setRow(-1);
    }

    // access the i-th field in this list.  
    // <group>
    SimField& operator[](Int i) { return get(i); }
    const SimField& operator[](Int i) const { return get(i); }
    // </group>

    // return the number of fields in this list
    uInt numFields() const { return n_p; }

    // Add a field.
    //  srcId    is the source ID that this field is associated with.
    //  dir      is normally the position of the source referenced by srcId 
    //               so that longOffset and latOffset gives the relative
    //               position of the field center; however, dir will be the
    //               absolute position of the field when 
    //               longOffset=latOffset=0.
    //  longOffset  is the true angular offset in radians in the direction
    //               perpendicular to the celestial pole (i.e. along a great
    //               circle) of the field center from the source position
    //               given by dir.
    //  latOffset   is the angular offset in radians in the direction of the 
    //               celestial (north) pole of the field center from the 
    //               source position.
    SimField& addField(uInt srcId, const MDirection& dir, 
		       Double longOffset=0.0, Double latOffset=0.0, 
		       const String& name="", const String& code="") 
    {
	SimField *out = new SimField(srcId, dir, longOffset, latOffset,
				     name, code);
	add(out);
	return *out;
    }

    // add field descriptions from an MS FIELD table
    void initFrom(const MSField& fldt);

    // write out all fields that have yet to be written
    void flush(MSField& fldt, MSSource& srct);

private:
    void deleteRecs() {
	for(uInt i=0; i < n_p; i++) {
	    if (rec_p[i] != 0) delete rec_p[i];
	    rec_p[i] = 0;
	}
    }
    SimField& get(Int i) const {
	if (i < 0 || i >= static_cast<Int>(n_p)) throwOutOfRange(n_p-1, i);
	return *(rec_p[i]);
    }

    void throwOutOfRange(uInt legallength, Int index) const;

    void add(SimField *fld) {
	if (n_p == rec_p.nelements()) {
	    rec_p.resize(n_p+chnk_p);
	    for(uInt i=n_p; i < rec_p.nelements(); i++) rec_p[i] = 0;
	}
	rec_p[n_p++] = fld;
    }
    uInt loadSourceInfo(MSSource& srct,
			SimpleOrderedMap<Int, String> &snames,
			SimpleOrderedMap<Int, Double> &times);

    uInt n_p, chnk_p;
    Block<SimField*> rec_p;

};


} //# NAMESPACE CASA - END

#endif
