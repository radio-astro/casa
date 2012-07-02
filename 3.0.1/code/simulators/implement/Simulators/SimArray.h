//# SimArray.h: container for containing array configuration description
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

#ifndef SIMULATORS_SIMARRAY_H
#define SIMULATORS_SIMARRAY_H

#include <casa/Utilities/PtrHolder.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MPosition.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MSAntenna;
class MSAntennaColumns;
class Unit;

// <summary> a container for data destined for an MS ANTENNA table
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimTelescope">SimTelescope</linkto>
// </prerequisite>
//
// <etymology>
// This is a container for describing a simulated interferometer array.
// </etymology>
//
// <synopsis> 
// This class describes a configuration of one or more antennas that make 
// up part or all of an interferometer.  It assumes that all of the antennas
// are identical except for their positions in the array and their names.    
// This container is used to define simulated observations.  Simulators would
// not normally create these objects directly but rather implicitly via the
// <linkto class="SimTelescope">SimTelescope</linkto> class. 
// <p>
//
// A heterogenous array (e.g. a VLBI array) can be described with multiple
// SimArray objects, usually held by a 
// <linkto class="SimArrayList">SimArrayList</linkto>.  This class handles 
// the recording of information to an MS ANTENNA table.  Obviously, when
// a SimArray is flushed to a Measurement Set, its data is written out in 
// multiple rows, one for each antenna.  <p>
//
// Positions can be given in one of several coordinate systems; see the 
// description LocalCoord below for the definitions.  Alternatively, this 
// class can hold a single antenna position, given by a position measure;
// this is most convenient for defining a VLBI array of hetergenous antennas.
// <p>
// 
// The positions, along with the rest of the antenna description data, are 
// set at construction and cannot be changed later.  The only writable data 
// of this class are a row marker (via setRow(), used by 
// <linkto class="SimArrayList">SimArrayList</linkto>) and an optional 
// sub-array index.  The sub-array index is assigned to the set of antennas
// as a whole; however, this is not recorded to the ANTENNA table.  
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a ANTENNA table.  
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS ANTENNA 
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimArray {
public:

    // Coordinate systems for specifying the antenna positions.  Values from
    // this enumeration are passed to the constructor as a flag indicating
    // how the x,y,z vector components of the position should be interpreted.
    // The types are defined as follows:
    // <pre>
    // CIRCUM    the positions are offsets from a geocentric spherical surface
    //           containing the reference position.  x is a longitudinal offset
    //           (i.e. not along a great circle) increasing to the east, y is
    //           latitudinal offset increasing to the north, and z is the 
    //           altitude relative to the reference location.  
    // 
    // TAN       the positions are offsets from a tangent plane to a 
    //           geocentric spherical surface containing the reference 
    //           position.  x is the offset increasing to the east, y is
    //           the offset increasing to the north, and z is the 
    //           altitude (relative to the reference location).  
    // 
    // EQUITORIAL  the positions represent are offsets from the reference 
    //           position in the rectangular system defined by the ITRS 
    //           coordinate frame in which y is parallel to the north pole.
    //           Thus, a simple vector addition of the offsets to the ITRS 
    //           vector of the reference position gives you the ITRS position 
    //           of the antenna.  
    // </pre>
    enum LocalCoord { 
	// position represent offsets from a geocentric spherical surface
	// containing the reference position.
	CIRCUM, 

	// position represents offsets from a tangent plane at the reference
	// position
	TAN, 

	// position represents linear offsets in the ITRF frame
	EQUITORIAL, 

	// number of types
	NTypes 
    };

    // Define a set of antennas.  The size of all input arrays must be equal
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
    //  diam        is the diameter of the dishes.
    //  mount       is name of the mount employed by the antennas.
    //  antNames    is a list of antenna names (e.g. "Ant 1").
    //  stationNames is a list of station names (e.g. "N8").
    //  offset      is the offset from the "feed reference position" from the
    //                 antenna positions.
    //  subarrayID  is default subarray to associate these antennas with.
    // <group>
    SimArray(const Vector<Double>& x,
	     const Vector<Double>& y,
	     const Vector<Double>& z, const String& unit,
	     const LocalCoord coord, const MPosition& refLoc,
	     const Quantity& diam, const String& mount,
	     const MPosition *offset=0,
	     uInt subarrayId=0);
    SimArray(const Vector<Double>& x,
	     const Vector<Double>& y,
	     const Vector<Double>& z, const String& unit,
	     const LocalCoord coord, const MPosition& refLoc,
	     const Quantity& diam, const String& mount,
	     const Vector<String>& antNames,
	     const Vector<String>& stationNames,
	     const MPosition *offset=0,
	     uInt subarrayId=0);
    // </group>

    // Define a single antenna.  This is useful for adding elements of a VLBI 
    // network of heterogenous dishes one at a time.
    //  name        the name of the antenna or station
    //  pos         defines the absolute position on the Earth of an
    //                 antenna (the reference Location is not used).
    //  diam        is the diameter of the dishes
    //  mount       is the mount used on the antenna
    //  subarrayId  is default subarray to associate these antennas with.
    SimArray(const String& name, const MPosition& pos, 
	     const Quantity& diam, const String& mount, 
	     const MPosition *offset=0, uInt subarrayId=0);

    SimArray(const SimArray& sa);

    SimArray& operator=(const SimArray& s);

    // get and set the row ID, the row of the first antenna making up this 
    // array.  An ID less than zero means that set has not yet been recorded.
    // This is mainly used by 
    // <linkto class="SimArrayList">SimArrayList</linkto>  to flush the 
    // SimArray data to a MeasurementSet's ANTENNA table.
    // <group>
    Int getRow() const { return row_p; }
    void setRow(Int id) { row_p = id; }
    // </group>

    // get and set the default subarray ID
    // <group>
    uInt getSubarrayID() const { return subary_p; }
    void getSubarrayID(uInt id) { subary_p = id; }
    // </group>

    uInt numAnts() const { return xyz_p.shape()[1]; }

    const Quantity& diameter() const { return diam_p; }
    const MPosition& offset() const { return offset_p; }
    const String& mount() const { return mount_p; }

    // fill the antennas positions associated with this record into a 
    // given vector.  start is the position in the vector to place the 
    // first antenna described by this record.  The vector should already 
    // be of sufficient size (no size checking on pos nor bounds checking 
    // on start is done).  The number of antennas loaded is returned.
    uInt antennaPositions(Vector<MPosition>& pos, uInt start) const;

    // fill the antenna positions as an array of ITRF position 
    // vectors.  The first axis of the given Matrix represents
    // the 3 components of the position vector in units of meters in the 
    // ITRF frame; the second axis is equal to the number of antennas in 
    // this subarray.  start is the position along the second axis to place 
    // the first antenna described by this record.  The matrix should already 
    // be of sufficient size (no size checking on pos nor bounds checking 
    // on start is done).  The number of antennas loaded is returned.
    uInt antennaPositions(Matrix<Double>& pos, uInt start) const;

    // fill the antenna names.  If the names have not been set, the names
    // will be returned as empty strings.  start is the position in the 
    // vector to place the first name for this set of antennas.  The vector 
    // should already be of sufficient size (no size checking on pos nor 
    // bounds checking on start is done).  The number of antennas loaded is 
    // returned.
    uInt antennaNames(Vector<String>& names, uInt start) const {
	return loadNames(names, names_p.ptr(), start);
    }

    // fill the station names.  If the names have not been set, the names
    // will be returned as empty strings.  start is the position in the 
    // vector to place the first name for this set of antennas.  The vector 
    // should already be of sufficient size (no size checking on pos nor 
    // bounds checking on start is done).  The number of antennas loaded is 
    // returned.
    uInt stationNames(Vector<String>& names, uInt start) const {
	return loadNames(names, stations_p.ptr(), start);
    }

    uInt write(MSAntenna &antt, MSAntennaColumns &antc, 
	       MPosition::Types refType=MPosition::ITRF, 
	       Vector<Unit> *units=0); 

protected:
    // create an empty record
    SimArray() : subary_p(0), row_p(-1), xyz_p(), diam_p(), offset_p(), 
		   mount_p(), basename_p(defbase_p), stations_p(), names_p() { }

private:
    void loadarray(const Vector<Double>& x, const Vector<Double>& y, 
		   const Vector<Double>& z, const Unit& unit, 
		   LocalCoord coord, const MPosition& refLoc);
    void loadcircum(const MPosition& refLoc);
    void loadtan(const MPosition& refLoc);
    void loadequitorial(const MPosition& refLoc);
    uInt loadNames(Vector<String>& names, const Vector<String> *from,
		   uInt start) const;

    uInt subary_p;
    Int row_p;
    Matrix<Double> xyz_p;
    Quantity diam_p;
    MPosition offset_p;
    String mount_p;
    String basename_p;
    PtrHolder<Vector<String> > stations_p;
    PtrHolder<Vector<String> > names_p;

    static const String defbase_p;
};

// <summary> a container for data destined for an MS ANTENNA table
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="SimArray">SimArray</linkto>
// </prerequisite>
//
// <etymology>
// This class holds a list of <linkto class="SimArray">SimArray</linkto> 
// objects.
// </etymology>
//
// <synopsis> 
// This class holds a list of <linkto class="SimArray">SimArray</linkto> 
// instances which, as a group, describes a potentially heterogenous array.  
// When using this class, one does not create SimArray objects directly,
// but rather implicitly via the addAntenna() and addAntennas() functions.
// The ultimate purpose of this container is to support simulated 
// observations.  Simulators would not normally create these objects 
// directly but rather implicitly via the 
// <linkto class="SimTelescope">SimTelescope</linkto> class.  <p>
//
// An important function handled by this class is the recording of the 
// antenna descriptions to a Measurement Set.  This is done with the flush()
// function.  When it writes the data from a SimArray in its list to the MS,
// it sets the row number (for the first antenna in the array) via setRow();
// this is used as a flag indicating that the SimArray as been flushed.  This
// allows one to later add additional SimArrays to the list; when flush is 
// called again, only the new SimArray data are written out.  This, of course, 
// assumes that the same output MSAntenna is passed to the flush() function 
// each time.  If you want to write all the data to a new ANTENNA table, you
// can call clearIds() to clear all the row markers.  <p>
// 
// A set of antennas can be read in from an ANTENNA table as well using the
// initFrom() method or the SimArrayList(const MSAntenna&, ...) constructor.  
// These will implicitly set the row markers from the input table.  This allows
// one to add new antennas to the already recorded set.
// </synopsis> 
//
// <motivation>
// This container provides an simplified in-memory representation of 
// feeds that will ultimately be written to a ANTENNA table.  
// The interface is oriented toward what is typical with current instruments
// and thus is simpler than the interface provided by the MS ANTENNA 
// table.
// </motivation>
//
// <todo asof="03/03/25">
//   <li> loading from an existing MS needs more testing.
//   <li> It would probably be helpful to have a common base classes for 
//        all Sim* and Sim*List classes that are managed by SimTelescope
//        and SimObservations
// </todo>
class SimArrayList {
public:

    // create an empty container
    SimArrayList(uInt initsz=2, uInt stepsz=4)
	: n_p(0), chnk_p(stepsz), nants_p(0), rec_p(initsz, 0), byant_p() {}

    // load the antennas described in the given ANTENNA table 
    SimArrayList(const MSAntenna& antt, Bool uniform=True, uInt stepz=4);

    // create a copy
    SimArrayList(const SimArrayList& other);

    SimArrayList& operator=(const SimArrayList& s);

    ~SimArrayList() { deleteRecs(); }

    // reset all the row markers used to flag the SimArray members that 
    // have been recorded to a Measurement Set already.  Thus, the next call 
    // to flush() will record all SimArrays to the ANTENNA table.  This 
    // should be used when writing to a new ANTENNA table, different from 
    // one previously read from or written to.  
    void clearIds() {
	for(uInt i=0; i < n_p; i++) 
	    rec_p[i]->setRow(-1);
    }

    // remove all SimArray objects from this list.
    void clearRecs() { 
	byant_p.resize(0);
	deleteRecs(); 
	n_p = nants_p = 0; 
    }

    // access the i-th SimArray object in this list.  Note that a SimArray
    // object can describe several identical antennas.
    // <group>
    SimArray& operator[](Int i) { return get(i); }
    const SimArray& operator[](Int i) const { return get(i); }
    // </group>

    // access the SimArray object that describes the i-th antenna stored 
    // in this list.  
    // <group>
    SimArray& antDesc(uInt i) { return *byant_p[i]; }
    const SimArray& antDesc(uInt i) const { return *byant_p[i]; }
    // </group>

    // return the number of antennas described in this list.
    Int numAnts() const { return nants_p; }

    // return the number of SimArray objects (each which may describe multiple
    // identical antennas) in this list.
    Int numRecs() const { return n_p; }

    // Define and add a set of antennas.  The size of all input arrays must 
    // be equal to the number antennas being set.  
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
    //  diam        is the diameter of the dishes.
    //  mount       is name of the mount employed by the antennas.
    //  antNames    is a list of antenna names (e.g. "Ant 1").
    //  stationNames is a list of station names (e.g. "N8").
    //  offset      is the offset from the "feed reference position" from the
    //                 antenna positions, given as a pointer.  (The offset
    //                 position will be copied; thus, the caller is in 
    //                 charge of the memory.)
    //  subarrayID  is default subarray to associate these antennas with.
    // <group>
    SimArray& addAntennas(const Vector<Double>& x,
			       const Vector<Double>& y,
			       const Vector<Double>& z, const String& unit,
			       const SimArray::LocalCoord coord, 
			       const MPosition& refLoc,
			       const Quantity& diam, const String& mount,
			       const MPosition *offset=0,
			       uInt subarrayId=0) 
    {
	SimArray *out = new SimArray(x,y,z, unit, coord, refLoc, diam,
					       mount, offset, subarrayId);
	add(out);
	return *out;
    }
    SimArray& addAntennas(const Vector<Double>& x,
			       const Vector<Double>& y,
			       const Vector<Double>& z, const String& unit,
			       const SimArray::LocalCoord coord, 
			       const MPosition& refLoc,
			       const Quantity& diam, const String& mount,
			       const Vector<String>& antNames,
			       const Vector<String>& stationNames,
			       const MPosition *offset=0,
			       uInt subarrayId=0)
    {
	SimArray *out = new SimArray(x,y,z, unit, coord, refLoc, diam,
					       mount, antNames, stationNames,
					       offset, subarrayId);
	add(out);
	return *out;
    }
    // </group>

    // Define and add a single antenna.  This is useful for adding elements 
    // of a VLBI network of heterogenous dishes one at a time.
    //  name        the name of the antenna or station
    //  pos         defines the absolute position on the Earth of an
    //                 antenna (the reference Location is not used).
    //  diam        is the diameter of the dishes
    //  mount       is the mount used on the antenna
    //  subarrayId  is default subarray to associate these antennas with.
    SimArray& addAntenna(const String& name, const MPosition& pos, 
			      const Quantity& diam, const String& mount, 
			      const MPosition *offset=0, uInt subarrayId=0)
    {
	SimArray *out = new SimArray(name, pos, diam, mount, 
					       offset, subarrayId);
	add(out);
	return *out;
    }

    // fill the currently configured antennas positions into a given vector.
    // The vector will be resized appropriately.  The number of antennas is
    // returned.
    uInt antennaPositions(Vector<MPosition>& pos) const;

    // fill the currently configured antenna positions as an array of 
    // ITRF position vectors.  The first axis of the given Matrix represents
    // the 3 components of the position vector in units of meters in the 
    // ITRF frame; the second axis is equal to the number of antennas in 
    // this subarray.  start is the position along the second axis to place 
    // The matrix will be resized appropriately.  The number of antennas is
    // returned.
    uInt antennaPositions(Matrix<Double>& pos) const;

    // fill the antenna names into a given Vector of Strings.  If the names 
    // have not been set, the names will be returned as empty strings.  In 
    // any event, the given vector will be resized.  The number of antennas 
    // is returned.
    uInt antennaNames(Vector<String>& names) const;

    // fill the station names into a given Vector of Strings.  If the names 
    // have not been set, the names will be returned as empty strings.  In 
    // any event, the given vector will be resized.  The number of antennas 
    // is returned.
    uInt stationNames(Vector<String>& names) const;

    // add array descriptions from an MS ANTENNA table
    void initFrom(const MSAntenna& antt, Bool uniform) {
	initEclectic(antt);
    }

    // write out all antennas that have yet to be written to an MS ANTENNA 
    // table.  
    uInt flush(MSAntenna& antt);

private:
    SimArray& get(Int i) const {
	if (i < 0 || i >= static_cast<Int>(n_p)) throwOutOfRange(n_p-1, i);
	return *(rec_p[i]);
    }

    void throwOutOfRange(uInt legallength, Int index) const;

    void add(SimArray *ary) {
	if (n_p == rec_p.nelements()) {
	    rec_p.resize(n_p+chnk_p);
	    for(uInt i=n_p; i < rec_p.nelements(); i++) rec_p[i] = 0;
	}
	rec_p[n_p++] = ary;

	byant_p.resize(nants_p+ary->numAnts());
	for(uInt i=0; i < ary->numAnts(); i++, nants_p++) 
	    byant_p[nants_p] = ary;
    }
    void deleteRecs() {
	for(uInt i=0; i < n_p; i++) {
	    if (rec_p[i] != 0) delete rec_p[i];
	    rec_p[i] = 0;
	}
    }
    void initEclectic(const MSAntenna& antt);

    uInt n_p, chnk_p, nants_p;
    Block<SimArray*> rec_p;
    Block<SimArray*> byant_p;
};


} //# NAMESPACE CASA - END

#endif
