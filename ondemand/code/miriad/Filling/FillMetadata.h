//# FillMetadata.h: this defines container classes used while actually filling data
//# Copyright (C) 2000,2001,2002
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
//#
//# $Id: FillMetadata.h,v 1.3 2011/08/11 18:03:53 pteuben Exp $

#ifndef BIMA_FILLMETADATA_H
#define BIMA_FILLMETADATA_H

#include <casa/Containers/List.h>
#include <casa/Containers/OrderedPair.h>
#include <casa/Containers/OrderedMap.h>
#include <casa/Containers/HashMap.h>
#include <casa/Containers/HashMapIter.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures/Stokes.h>
#include <miriad/Filling/MirTypeAssert.h>
#include <miriad/Filling/IDIndex.h>
#include <miriad/Filling/DataLoadingBuf.h>
#include <measures/Measures.h>

#include <casa/namespace.h>
//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
  class MeasurementSet;
  class MSColumns;
} //# NAMESPACE CASA - END


// <summary>
// an abstract base for classes that contain information about some 
// part of a Miriad Dataset
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// subclasses contain information about a Miriad dataset
// </etymology>
//
// <synopsis>
// This class provides a common type and interface for classes that organize 
// some collection of related information in a Miriad dataset.  
// </synopsis>
//
// <motivation>
// 
// </motivation>
//
class MirInfo : MirTypeAssert {
 public:
  MirInfo();
  virtual ~MirInfo();
 protected:
};

// <summary>
// a container for field information
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// a Miriad observing field is expressed as an offset from a reference position
// </etymology>
//
// <synopsis>
// MirField is a helper class used by MirFiller and FillMetadata to hold
// the data describing an observing field in a single container.  Most of the 
// interaction with this class is through public data members (for programming 
// and runtime efficiency); thus, this class is not appropriate for use outside 
// of this module.  A few helper funtions are provided to aid in utilizing the
// information (mainly for comparison with other MirField objects).  
// </synopsis>
//
// <motivation>
// A Miriad dataset can contain a number of observing fields, described as offsets
// from a reference position.  When scanning a multi-field dataset, one usually 
// will encounter data each field as the observations cycles through the set of 
// pointings.  Thus, the filler must keep a list of unique fields that can be 
// matched against the current field.
// </motivation>
//
class MirField : public MirInfo
{
 public:
    // the position offsets
    Float dra, ddec;

    // the ID assigned to this field within the MeasurementSet being filled.
    // A value of -1 means that the ID has not yet been assigned.
    Int id;

    // create a new field description
    MirField(Float delra, Float deldec, Int fid=-1);

    // delete this field
    virtual ~MirField();

    // return true if this field's offsets match those of another 
    Bool operator==(const MirField &that) {
	return (dra==that.dra && ddec==that.ddec);
    }

    // return false if this field's offsets match those of another 
    Bool operator!=(const MirField &that) {
	return ! (*this == that);
    }

    // return a pointer to a field in a given list of fields that is equal 
    // to this field.
    MirField *findIn(List<MirField*> &fldlist) {
	ListIter<MirField*> li(fldlist);
        while (! li.atEnd() && *(li.getRight()) != *this) li++;
        return ((li.atEnd()) ? NULL : li.getRight());
    }

    // clear the ID values for all the fields in a field list by setting them
    // to -1.  
    static void clearIDs(List<MirField*> &fldlist) {
	for(ListIter<MirField*> li(fldlist); ! li.atEnd(); ++li) 
            li.getRight()->id = -1;
    }
	
};

// <summary>
// A description of a Miriad spectroscopy (correlator) setup
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// a description of the frequency setup used by a Miriad dataset
// </etymology>
//
// <synopsis>
// This class contains the data describing the frequency domain of a 
// portion of the dataset.  <p>
// 
// Most of the interaction with this class is through public data members (for 
// programming and runtime efficiency); thus, this class is not appropriate for 
// use outside of this module.  A few helper funtions are provided to aid in 
// utilizing the information (mainly for comparison with other MirFreqSetup objects).
// </synopsis>
//
// <motivation>
// a Miriad dataset may switch back and forth between different correlator 
// configurations.  The filler must keep track of them, and recognize them
// when the are reused.
// </motivation>
// 
class MirFreqSetup : public MirInfo {
 public: 

    // the correlator mode
    Int mode;

    // the correlator filter bandwidths
    Float *corbw;

    // the correlator LO frequencies
    Float *corf;

    // the number of spectral windows
    Int nspect;

    // the number of wideband channels
    Int nwide;

    // the total number of spectral line channels
    Int nchan;

    // the number of channels in each window
    Int *nschan;

    // the number of the first channel in each window
    Int *ischan;

    // the frequency of the first channel in each window
    Double *sfreq;

    // the frequency separation between channels in each window
    Double *sdf;

    // the rest frequency for each window
    Double *rfreq;

    // the center frequency of each wideband channel
    Float *wfreq;

    // the bandwidth of each wideband channel
    Float *wwidth;

    // the reference frequency for this setup.  Usually the rest frequency
    // of the primary line of the observations.
    Double freq;

    // the ID assigned to this setup within the MeasurementSet being filled.
    // A value of -1 means that the ID has not yet been assigned.
    Int id;

    // the narrow window mapping index.  This maps
    // Miriad window numbers to MS spectral window IDs.
    IDIndex nfidx;

    // the wideband mapping index.  This maps
    // Miriad wideband channel numbers to MS spectral window IDs.
    IDIndex wfidx;

    // create a new setup by reading the relevent variable data from 
    // the given miriad dataset handle
    MirFreqSetup(Int mir_handle);

    virtual ~MirFreqSetup();

    // return true if this setup is the same as another.
    // Two setups will be considered equal if the mode, nwide, nspect,
    // corfs and corbws are identical.  (The window starting frequencies can
    // change as they are topocentric; this may cause problems for miriad 
    // data that did not originate from BIMA.)
    Bool operator==(MirFreqSetup &that);

    // return false if this setup is the same as another.
    Bool operator!=(MirFreqSetup &that) {
	return ! (*this == that);
    }

    // clear the id and index maps
    void clear() { 
	id = -1;
	nfidx.clear();
	wfidx.clear();
    }

    // return a pointer to a setup in a given list of setups that is equal 
    // to this setup.
    MirFreqSetup *findIn(List<MirFreqSetup*> &setuplist) {
        ListIter<MirFreqSetup*> li(setuplist);
        while (! li.atEnd() && *(li.getRight()) != *this) li++;
        return ((li.atEnd()) ? NULL : li.getRight());
    }

    // clear the ID values for all the setups in a setup list by setting them
    // to -1.  
    static void clearIDs(List<MirFreqSetup*> &setuplist) {
	for(ListIter<MirFreqSetup*> li(setuplist); ! li.atEnd(); ++li) 
            li.getRight()->clear();
    }
};

// <summary>
// a container for a single Miriad polarization correlation 
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// the name is short for Miriad Polarization Correlation.  A polarization 
// correlation type refers to the combination (via cross-correlation) of two 
// polarizations.
// </etymology>
//
// <synopsis>
// This class contains a polarization cross-correlation combination.  It 
// provides methods for determining the constituent polarization types.  An
// integer Id can be associated with it as well; this is used by the MirFiller
// class to remember what table entry it is associated with.
// </synopsis>
//
// <motivation>
// Miriad datasets record different polarization measurements in seperate 
// records; thus, there is a notion of a current polarization among several 
// being read.   Furthermore, you usually don't know which polarizations 
// are in the dataset until you've read at least one record with each kind 
// of polarization.  You do usually know the total number of different 
// polarizations after reading the first record.  
// </motivation>
//
// 
class MirPolCorr : public MirInfo {
 private:
    Int corr;
    mutable Int id;

    static const Stokes::StokesTypes corratopol[];
    static const Stokes::StokesTypes corrbtopol[];

 public:
    // create a container.  Poltype is the polarization type in the Miriad 
    // convention (i.e. as read from the "pol" variable in a Miriad dataset).
    MirPolCorr(Int poltype);

    // create a copy of a container.  
    MirPolCorr(const MirPolCorr& that) : id(-1) { corr = that.corr; }

    // delete the container
    virtual ~MirPolCorr();

    Bool operator==(const MirPolCorr& that) const { return corr == that.corr; }
    Bool operator==(Int poltype) const { return corr == poltype; }

    // set the independent ID.  A value less than zero should be interpreted
    // as meaning unset.
    void setID(Int val) const { id = val; }

    // return the independent ID.  A value less than zero should be interpreted
    // as meaning unset.
    Int getID() const { return id; }

    // clear the independent ID value
    void clearID() const { id = -1; }

    // return the polarization correlation type in the Miriad 
    // convention (i.e. as read from the "pol" variable in a Miriad dataset).
    Int getType() const { return corr; }

    // return the polarization type associated with the first 
    // receptor for this correlation type.  The returned StokesType for 
    // non-true Stokes types will be that for a single type cross-correlated
    // with itself.
    Stokes::StokesTypes getFirstPolType() const {
	return corratopol[corr+8];
    }

    // return the polarization type associated with the second
    // receptor for the given correlation type.  The returned StokesType for 
    // non-true Stokes types will be that for a single type cross-correlated
    // with itself.
    Stokes::StokesTypes getSecondPolType() const {
	return corrbtopol[corr+8];
    }

    // return True if the given single polarization type is one of the 
    // components of this correlation type
    Bool hasPol(Stokes::StokesTypes pol) const {
	return (getFirstPolType() == pol || getSecondPolType() == pol);
    }

    // convert the Miriad polarization code to the AIPS
    static Stokes::StokesTypes toAips(Int pol);

    // convert the Aips Stokes polarization code to the Miriad convention
    static Int toMiriad(Stokes::StokesTypes pol);

    // return the AIPS++ Stokes type for the current polarization
    Stokes::StokesTypes getAipsType() const { return toAips(corr); }

    // return a string representation of this polarization 
    String toString() const { return Stokes::name(getAipsType()); }
};

// <summary>
// a static container for a set of Miriad polarization correlation types
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// the name is short for Constant Miriad Polarization Setup.  
// </etymology>
//
// <synopsis>
// This class contains a list of polarization cross-correlation combinations.
// New correlations cannot be added to this list.
// </synopsis>
//
// <motivation>
// Miriad datasets record different polarization measurements in seperate 
// records; thus, there is a notion of a current polarization among several 
// being read.   Furthermore, you usually don't know which polarizations 
// are in the dataset until you've read at least one record with each kind 
// of polarization.  You do usually know the total number of different 
// polarizations after reading the first record.  
// </motivation>
//
// 
class ConstMirPolSetup : public MirInfo {
 public:
    virtual ~ConstMirPolSetup();

 protected:
    List<MirPolCorr> corrs;
    ListIter<MirPolCorr> iter;

    ConstMirPolSetup();
    ConstMirPolSetup(ConstMirPolSetup& other);
    Bool find(Int poltype, ConstListIter<MirPolCorr>& li);
    
 public:

    // return the number of polarization correlation types in this setup
    uInt getCorrCount() { return corrs.len(); }

    // set the current polarization correlation to the given type.  True 
    // is returned if the type is found; if it is not, the current 
    // polarization is unchanged.
    Bool setCorr(Int poltype) {
	// Note: this has been made non-virtual on purpose
	return find(poltype, iter);
    }

    // return True if the given polarization correlation type is a member
    // of this set
    Bool hasCorr(Int poltype) {
	ConstListIter<MirPolCorr> li(corrs);
	return find(poltype, li);
    }

    // clear all the IDs associated with the polarization correlation types
    // in this list
    void clearIDs() {
	ConstListIter<MirPolCorr> li(corrs);
	for(li.toStart(); ! li.atEnd(); ++li) {
	    li.getRight().clearID();
	}
    }

    // return the list of polarization correlation types stored in this set
    const List<MirPolCorr>& getCorrs() { return corrs; }

    // return the currently selected polarization
    MirPolCorr& getCurrent() { return iter.getRight(); }

    // return the AIPS type for the current polarization.  If no
    // current polarization correlation type is set, YY is returned
    Stokes::StokesTypes getCurrentAipsType() {
	return ((iter.atEnd()) ? Stokes::YY : iter.getRight().getAipsType());
    }
};

// <summary>
// an editable container for a set of Miriad polarization correlation types
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// the name is short for Miriad Polarization Setup.  
// </etymology>
//
// <synopsis>
// This class contains a list of polarization cross-correlation combinations.
// This version allows new values to be added to the list.
// </synopsis>
//
// <motivation>
// Miriad datasets record different polarization measurements in seperate 
// records; thus, there is a notion of a current polarization among several 
// being read.   Furthermore, you usually don't know which polarizations 
// are in the dataset until you've read at least one record with each kind 
// of polarization.  You do usually know the total number of different 
// polarizations after reading the first record.  
// </motivation>
//
// 
class MirPolSetup : public ConstMirPolSetup {
 private:
    // add a new correlation type to this list.  This function assumes that
    // the type does not already exist in the list.
    void addCorr(Int poltype);

 public: 
    MirPolSetup();
    MirPolSetup(ConstMirPolSetup& other);
    virtual ~MirPolSetup();

    // set the current polarization correlation to the given type.  If 
    // the type is not already part of this set, it will be added.  True 
    // is returned if the type is found without having to add it.
    Bool setCorr(Int poltype) {
	if (! find(poltype, iter)) {
	    addCorr(poltype);
	    return False;
	}
	return True;
    }

}; 

// <summary>
// a container for a set of Miriad (single-) polarization receptor types
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// the name is short for Miriad Polarization Receptors
// </etymology>
//
// <synopsis>
// This class contains a list of single-polarization types that correspond
// the types of polarizations a group of receptors are sensitive to.
//
// Single Polarization types are identified using the AIPS' StokesTypes 
// enumeration.  For non-true Stokes types, the value for the polarization
// cross-correlated with itself is used.
// </synopsis>
//
// <motivation>
// Miriad datasets record different polarization measurements in seperate 
// records; thus, there is a notion of a current polarization among several 
// being read.   Furthermore, you usually don't know which polarizations 
// are in the dataset until you've read at least one record with each kind 
// of polarization.  You do usually know the total number of different 
// polarizations after reading the first record.  
// </motivation>
//
// 
class MirPolRecps : public MirInfo {
 private:
    List<Stokes::StokesTypes> pols;
    mutable ListIter<Stokes::StokesTypes> iter;

 public:
    MirPolRecps() : pols(), iter(pols) { }
    virtual ~MirPolRecps();

    // return the number of distinct single polarization that make up 
    // this setup
    uInt getPolCount() const { return pols.len(); }

    // add a polarization type if it isn't already added.
    void addPol(Stokes::StokesTypes pol) {
	if (! hasPol(pol)) {
	    iter.toEnd();
	    iter.addRight(pol);
	}
    }

    // add the polarization types involved in the cross-correlation represented
    // by the given correlation type
    void addPolsFor(const MirPolCorr &pol) {
	addPol(pol.getFirstPolType());
	addPol(pol.getSecondPolType());
    }

    // add the polarization types involved in the cross-correlations
    // represented by the correlation types in the given list
    void addPolsFor(ConstMirPolSetup &pol) {
	ConstListIter<MirPolCorr> li(pol.getCorrs());
	for(li.toStart(); ! li.atEnd(); ++li) 
	    addPolsFor(li.getRight());
    }

    // return the i-th polarization 
    Stokes::StokesTypes getPol(Int i) const { 
	iter.pos(i); 
	return iter.getRight();
    }

    // find the position of the given polarization type.  -1 is returned
    // if the polarization is not found
    Int find(Stokes::StokesTypes pol) {
	for(iter.toStart(); ! iter.atEnd(); ++iter) {
	    if (iter.getRight() == pol) return iter.pos();
	}
	return -1;
    }

    // return True if the given polarization is present in this list
    Bool hasPol(Stokes::StokesTypes pol) { return (find(pol) >= 0); }

    // return the unique list of single polarization types that comprise
    // the correlation types.
    const List<Stokes::StokesTypes>& getPols() const { return pols; }

    // return a string representing the i-th polarization 
    String toString(Int i) const { return polToString(getPol(i)); }

    // return a string representing the given polarization 
    static String polToString(Stokes::StokesTypes pol) {
	return String( (Stokes::name(pol)).at(0,1) );
    }
};


// <summary>
// a container for source information
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// a description of a source and its position as stored in a Miriad dataset
// </etymology>
//
// <synopsis>
// This class contains the data describing a source from a Miriad dataset. <p>
// 
// Most of the interaction with this class is through public data members (for 
// programming and runtime efficiency); thus, this class is not appropriate for 
// use outside of this module.  A few helper funtions are provided to aid in 
// utilizing the information (mainly for comparison with other MirSource objects).
// </synopsis>
//
// <motivation>
// a Miriad dataset may switch back and forth between different sources.
// The filler must keep track of them, and recognize them when the are reused.
// </motivation>
// 
class MirSource : public MirInfo {
 public:
    enum SolSysObject { 
	MERCURY,
	VENUS,
	MARS,
	JUPITER,
	SATURN,
	URANUS,
	NEPTUNE,
	PLUTO,
	SUN,
	MOON,
	N_Objects
    };

    // the source name
    String name;

    // the initial source position
    Double ra, dec;

    // the position epoch
    Float epoch;

    // the time of first observation
    Double time;

    // the source id
    Int id;

    // the first row of this source record appears in the SOURCE table.  
    Int row;

    // a planet identifier
    SolSysObject solsys;

    // Planet names
    static const String solSysNames[];

    // a list of fields
    List<MirField*> flds;

    MirSource(String source, Double sra, Double sdec, Float sepoch, 
              Double stime=0, Int sid=-1);
    MirSource(int mirh, Double stime=0);
    virtual ~MirSource();

    // return true if this source is the same as another.
    // Two sources are considered the same if they have the same name and 
    // position epoch.
    Bool operator==(const MirSource &that) {
	return (name==that.name && epoch==that.epoch);
    }

    // return false if this source is the same as another.
    Bool operator!=(const MirSource &that) {
	return ! (*this == that);
    }

    // return a pointer to a source in a given list of source that is equal 
    // to this source.
    MirSource *findIn(List<MirSource*> &srclist) {
	ListIter<MirSource*> li(srclist);
        while (! li.atEnd() && *(li.getRight()) != *this) li++;
        return ((li.atEnd()) ? NULL : li.getRight());
    }

    // return a pointer to a source in a given list of source that has the same
    // name as this source.
    MirSource *findNameIn(List<MirSource*> &srclist) {
	ListIter<MirSource*> li(srclist);
        while (! li.atEnd() && li.getRight()->name != this->name) li++;
        return ((li.atEnd()) ? NULL : li.getRight());
    }

    // clear the ID values for all the source in a source list by setting them
    // to -1.  
    static void clearIDs(List<MirSource*> &srclist) {
	for(ListIter<MirSource*> li(srclist); ! li.atEnd(); ++li) {
            li.getRight()->id = -1;
            if (li.getRight()->flds.len() > 0) {
                MirField::clearIDs(li.getRight()->flds);
            }
        }
    }

    // return True if this source is a moving object (by virtue of having 
    // multiple positions added to this container).
    Bool isMoving() { return (motion_p != 0); }

    // return True if this source is identified as a major solar system object
    Bool isSolSysObj() { return (solsys < N_Objects); }

    // return True if this source is identified as a planet
    Bool isPlanet() { return (solsys < SUN); }

    // return True if any of the fields is pointed off the main source 
    // position (as given by ra and dec)
    Bool offSource() {
	if (flds.len() > 1) return True;
	ListIter<MirField*> f(flds);
	return (f.getRight()->dra != 0 || f.getRight()->ddec != 0);
    }

    // add a new position for this source
    void addPosition(Double mtime, Double mra, Double mdec) {
	if (! motion_p) {
	    motion_p = new OrderedMap<Double, OrderedPair<Double, Double> >(
		OrderedPair<Double,Double>(0,0), 2);
	    motion_p->define(time, OrderedPair<Double, Double>(ra, dec));
	}
	if (! motion_p->isDefined(mtime))
	    motion_p->define(mtime, OrderedPair<Double, Double>(mra, mdec));
    }

    // load the motion data into the given arrays
    void getMotion(Vector<Double> &time, 
		   Vector<Double> &ra, Vector<Double> &dec) 
    {
	if (! motion_p) return;
	uInt n = motion_p->ndefined();
	time.resize(n);
	ra.resize(n);
	dec.resize(n);

	MapIter<Double, OrderedPair<Double, Double> > iter(*motion_p);
	for(uInt i=0; ! iter.atEnd(); ++iter, ++i) {
	    time(i) = iter.getKey();
	    ra(i) = iter.getVal().x();
	    dec(i) = iter.getVal().y();
	}
    }

    static SolSysObject matchSolSysObj(const String &name);

    static Int nextID() {  
	Int out = nxtid_p;
	nxtid_p++;
	return out;
    }

private:
    OrderedMap<Double, OrderedPair<Double, Double> > *motion_p;
    static Int nxtid_p;
};

// <summary>
// a pair of indicies identifying the spectral window and polarization ids
// that make up a data description ID
// </summary>
// 
// <use visibility=local>   
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// The Data Description subtable record has two main components: a spectral
// window ID and a polarization ID.
// </etymology>
//
// <synopsis>
// This class holds two IDs that make up the "data description" of a record
// in an MS MAIN table: the spectral window ID and the polarization ID.  
// </synopsis>
//
// <motivation>
// Records in the MAIN MS table are linked to rows in the SPECTRAL_WINDOW and 
// POLARIZATION subtables via a single DATA_DESC_ID; this class collects the
// indicies into those two subtables into a single class that can be linked to
// an ID in the DATA_DESCRIPTION subtable (via a Map).  
// </motivation>
//
class DataDescComponents {
 private:
    Int spw;
    Int pol;
 public:
    DataDescComponents(Int spwid=-1, Int polid=-1) : spw(spwid), pol(polid) { }
    ~DataDescComponents() { }

    // return the spectral window ID component
    Int getSpectralWindowID() const { return spw; }
    
    // return the polarization ID component
    Int getPolarizationID() const { return pol; }

    Bool operator==(const DataDescComponents& other) const {
	return (spw == other.spw && pol == other.pol);
    }

    Bool exists() const { return (spw >= 0 && pol >= 0); }
};

extern uInt hashFunc(const DataDescComponents& key);

// <summary>
// a container for storing the Miriad metadata that must be tracked while 
// filling
// </summary>

// <use visibility=local>   

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MirFiller">MirFiller</linkto>
// </prerequisite>
//
// <etymology>
// This stores the metadata from an input Miriad dataset (e.g. Miriad variable 
// values) that are important for orchestrating its filling into a measurement 
// set.
// </etymology>
//
// <synopsis>
// This class is a container for various information gleaned from the Miriad 
// dataset that is important for organizing the data into a Measurement Set.  
// It essentially holds all state information needed by MirFiller's functions 
// during filling.  <p>
//
// Most of the interaction with this class is through public data members (for 
// programming and runtime efficiency); thus, this class is not appropriate for 
// use outside of this module.  A few helper funtions are provided to aid in 
// utilizing the information.
// </synopsis>
//
// <motivation>
// This allows MirFiller to pass the state of the filling process to its internal
// functions rather than storing it as a member object.  Thus, if one attempt 
// to fill fails half way, MirFiller does not have to worry about cleaning up 
// its internal state.  
// </motivation>
//
//
class FillMetadata {
 public:

    // info that doesn't change during filling
    MeasurementSet *ms;              // the output ms
    MSColumns *msc;                  // a pointer to its columns
    String outname;

    // output MS info that can vary during filling process
    Int obsid;                       // ids & offsets
    Float inttime;                   // the current integration time
    Float jyperk;
	Float plangle, plmaj, plmin, pltb;	 // miriad planet variables
	Double freq; // rest frequency of the primary line -- used to convert
	             // pltb to flux using Raleigh-Jeans appx to BB.
    String telescope, project;
    Bool obsupd;
    Int nants, narrays, arrayAnt;
    Vector<Double> antpos;
    Vector<Double> arrayXYZ;
    Int *mount;
    Double *diam;
    Double starttime, obstime, feedtime, modeltime, lasttime;

    MirFreqSetup *fsetup;
//    ListIter<MirFreqSetup *> fsiter;
//     IDIndex wfidx;   // moved to MirFreqSetup
//     IDIndex nfidx;

    MirSource *source;
    Bool movingsrc;
//    const List<MirSource*> *srclist;

//    ListIter<MirField *> flditer;
    MirField *field;

    ConstMirPolSetup *pol;
    MirPolRecps *polrecp;
    Bool polotf;           // set to true if loading polarizations on the fly
    // PJT
    //HashMap<DataDescComponents, Int> ddids;

    Matrix<Float> nsystemp;
    Matrix<Float> wsystemp;
    Int tsyscorrect;

    // data-loading buffers
    DataLoadingBuf buf;

    static const String HATCREEK;      // "HATCREEK"
    static const String BIMA;          // "BIMA"
    static const String CARMA;         // "CARMA"
    static const String ATCA;          // "ATCA"
    static const String VLA;           // "VLA"

    FillMetadata(const String &msfile="");
    ~FillMetadata();

    // return the antenna id for a given miriad antenna number.
    Int getAntennaID(Int antnum) { return arrayAnt+antnum-1; }

    // return the data description id for a given spectral window id.
    Int getDataDescID(Int sid=0, Int pid=0) { 
#if 0
        DataDescComponents ddid(sid, pid);
	return (ddids.isDefined(ddid) ? ddids(ddid) : -1);
#else
	return -1;
#endif
    }

    // set all fiducial times to the given time
    void inittime(Double time, Double updmodelint=0) { 
	lasttime = feedtime = obstime = starttime = time;
	modeltime = time - updmodelint;
    }

    // set the telescope.  This will also set the telescope location if 
    // it is known.
    void setTelescope(String tel);

    Int bimaAntCount() {
	Int i;
	for(i=nants-1; i >= 0; i--) {
	    if (antpos(i) != 999            ||
		antpos(i+nants) != 999   ||
		antpos(i+2*nants) != 999   )  break;
	}
	return ((i >= 0) ? i+1 : nants);
    }
};

#endif


