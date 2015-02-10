//# GBTFeed: this holds information about a specific feed with IFs
//# Copyright (C) 2003
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
//# $Id$

#ifndef NRAO_GBTFEED_H
#define NRAO_GBTFEED_H

#include <casa/aips.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

//# Forward Declarations
class GBTIF;

// <summary>
// This holds information about a specific feed with IFs
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tGBTFeed.cc" demos="">
// </reviewed>

// <prerequisite>
//   <li> ANTENNA BEAM_OFFSETS table
//   <li> MeasurementSet
// </prerequisite>
//
// <motivation>
// It is necessary to collect information about the feeds in use in a
// given scan and associate each feed with a set of IFs.
// </motivation>
//

class GBTFeed
{
public:
    // The default feed.  Zero offsets and no associated srfeeds.
    // Equivalent to GBTFeed(0.0, 0.0, -1, -1, -1, "");
    GBTFeed();

    // A specific feed with the values taken directly from one row
    // of the BEAM_OFFSETS table.  The offsets should be in
    // deg, which is how they are expressed there.  Since
    // other parts of this class use the integer equivalences of
    // the feedName and since the IF table only contains an integer
    // FEED column, only the physical feeds present in the BEAM_OFFSETS
    // table should be used here.
    GBTFeed(Double xeloffset, Double eloffset, const String &feedName,
	    Int srfeed1, Int srfeed2, const String &trckbeam);

    // Copy constructor.  Copy semantics.
    GBTFeed(const GBTFeed &other);

    ~GBTFeed();

    // Assignment operator, copy semantics.  This is the only way
    // to "unfreeze" a GBTFeed.  If other is not yet frozen than
    // this will become unfrozen as a result if it being made to
    // be a copy of other.
    GBTFeed &operator=(const GBTFeed &other);

    // This is the filled FEED_ID from the MS.  Returns -1 if not
    // yet known.
    Int feedId() const {return itsFeedId;}

    // Set the filled FEED_ID.
    void setFeedId(Int feedid) {itsFeedId = feedid;}

    // This is the filled FEED_ID from the MS corresponding to the
    // switched beam (i.e. corresponding to sdFitsFeedId).  Returns
    // -1 if not yet known.  This should be set equal to 
    // feedId() if there is no beam switching going on.
    // This will be filled to the GBT_SRFEED_ID column of the FEED
    // subtable.
    Int srFeedId() const {return itsSRFeedId;}

    // Set the filled GBT_SRFEED_ID.
    void setSRFeedId(Int srfeedid) {itsSRFeedId = srfeedid;}

    // The number of associated GBTIFs.
    uInt nif() const {return itsNif;}

    // Get a specific GBTIF
    const GBTIF &getIF(uInt whichIF) const {return *(itsIFPtr[whichIF]);}

    // Get one to possibly modify
    GBTIF &getIF(uInt whichIF) {return *(itsIFPtr[whichIF]);}

    // Add a new GBTIF - copies the values over, only adds if the IF
    // is unique.  Returns the index of the just newIF - which will
    // be < nif() before this call if this IF has already been added
    // (i.e. if it isn't unique).  The returned value can be used
    // as the whichIF argument of getIF.  Also combines the ifrows
    // and the correlations as necessary when mergeCorr is True.
    // If bankCheck is true, then the correlations are only
    // merged if the banks they came from are the same.  That
    // check should only be done on ACS data.
    Int addUniqueIF(const GBTIF &newIF, Bool mergeCorr,
		    Bool bankCheck);

    // Set the receiver name.
    void setReceiver(const String &receiver) {itsRxName = receiver;}

    // Get the receiver name.
    const String &receiver() const {return itsRxName;}

    // Add a receptor of a given polarization.  Has no effect after
    // freeze() has been called.  Should be one of just X,Y,L, or R.
    // The name comes from the IF manager.  Returns False if
    // that pol with a different receptorName has already been added.
    Bool addReceptor(const String &pol, const String &receptorName);

    // Freezes the number of receptors. This is necessary so that
    // receptors can be added in any order and sorted to a specific
    // order.  Sort order is x,y,r,l (although presumably in most
    // case there will be just x,y and r,l - or perhaps just one of
    // x,y,r, or l).  If no receptors have been added by the
    // time of the freeze, the default case of a single "x" receptor
    // will be made.
    void freeze();

    // The number of receptors.  Returns 0 before freeze().
    uInt numReceptors() const {return itsFrozen ? itsPolType.nelements():0;}

    // Returns the set of known receptors for this feed once
    // the feed has been frozen.  Prior to freeze() being called
    // the contents of this vector are not reliable.
    const Vector<String> &polType() const {return itsPolType;}

    // The names of the receptors as know in the IF FITS file once
    // the feed has been frozen.  Prior to freeze() being called
    // the contents of this vector are not reliable.
    const Vector<String> &receptorNames() const {return itsRecptNames;}

    // Return the offsets - to be used when filling the MS::FEED table.
    // The shape is (2,NUM_RECEPTORS). There is no actual dependence
    // here on NUM_RECEPTORS (i.e. all NUM_RECEPTORS values of each
    // row are identical).  The units are radians.  Prior to freeze
    // being called, the contents of this Matrix are not reliable.
    const Matrix<Double> &offsets() const {return itsOffsets;}

    // Return the feed number as it is known in the original FITS
    // file,  This is a direct translation of the String NAME column
    // in the FITS file to an integer.  Only those feeds which have
    // simple integer names will ever be used here..
    Int fitsFeedId() const {return itsFitsFeedId;}

    // The original feed name
    const String name() const {return itsName;}

    // The tracking beam string
    const String trckbeam() const {return itsTrckbeam;}

    // Return the associated switching feed number as it appears in
    // the FITS table as dediced 
    // from the SRFEED1 and SRFEED2 columns.  Then the value here
    // is the one that is NOT the fitsFeedId() for this feed.
    // It therefore represents the fitsFeedId() for the other feed.
    // If both SRFEED1 and SRFEED2 are <= 0 then that implies that
    // no switching is going on.  In that case srFitsFeedId() == fitsFeedId().
    Int srFitsFeedId() const {return itsSRFitsFeedId;}

    // Return a pointer to the associated switching feed.  This 
    // is the value set by setSRFeedPtr.  It should not be deleted.
    const GBTFeed *srFeedPtr() const {return itsSRFeedPtr;}

    // Set a pointer to the associated switching feed.
    void setSRFeedPtr(const GBTFeed *srFeedPtr) {itsSRFeedPtr = srFeedPtr;}
private:
    Int itsFeedId, itsSRFeedId;
    String itsName, itsTrckbeam, itsRxName;

    const GBTFeed *itsSRFeedPtr;
    
    //# the number of GBTIF * actually in use and available in
    //# itsIFPtr.
    Int itsNif;
    Block<GBTIF *> itsIFPtr;

    Bool itsFrozen;

    //# initially this has length 4 and it is used to record which
    //# receptors of X,Y,R,L, have been added, in that order.  These
    //# will be non-empty strings only when they have been set.
    //# After freeze(), this is reduced to only the non-empty values.
    //# Presumably just (X,Y) or (R,L) or individual receptors.
    Vector<String> itsPolType;
    
    Vector<String> itsRecptNames;

    //# ths offsets as given in the ctor, units are still deg
    Double itsXeloffset, itsEloffset;

    Matrix<Double> itsOffsets;

    Int itsFitsFeedId, itsSRFitsFeedId;
};

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <nrao/GBTFillers/GBTFeed.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif


