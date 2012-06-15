//# GBTFeedInfo.h: GBTFeedInfo is a simple class used by GBTMSFeedFiller
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

#ifndef NRAO_GBTFEEDINFO_H
#define NRAO_GBTFEEDINFO_H

#include <casa/aips.h>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/BasicSL/String.h>
#include <casa/namespace.h>

// <summary>
// GBTFeedInfo is simple class used in GBTMSFeedFiller
// </summary>

// <use visibility=local> 

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// The FEED filler needs to keep track of related information to
// sort out what feeds need to be filled with unique FEED_IDs.
// This class keeps that all together for each feed to be filled.
// </etymology>
//
// <synopsis>
// Used internally in GBTMSFeedFiller.
// </synopsis>
//
// <motivation>
// Book-keeping in the code using Block, Matricies, et al was getting
// too cumbersom primarily because the number of receptors on a given
// feed can vary.  Putting it in here cleans up the code in the filler.
// </motivation>
//

class GBTFeedInfo
{
public:

    // default constructor - initializes internals for unknown feed
    GBTFeedInfo();

    // copy constructor - copy semantics
    GBTFeedInfo(const GBTFeedInfo &other);
    
    ~GBTFeedInfo() {;}
    
    // assignment operator - copy semantics
    GBTFeedInfo &operator=(const GBTFeedInfo &other);

    // get/set the feed number as used in the ifloHelper
    Int feedNumber() const {return itsFeedNumber;}
    void setFeedNumber(Int feedNumber) {itsFeedNumber = feedNumber;}

    // get/set the physical feed number
    Int physFeed() const {return itsPhysFeed;}
    void setPhysFeed(Int physFeed) {itsPhysFeed = physFeed;}

    // get the rows in the IF table that map to this feed
    const Block<Int> ifRows() const { return itsIFRows;}
    // add a row to the set of if rows that map to this feed
    void addRow(Int whichRow);

    // set the polarization information for the indicated
    // receptor and also set the number of receptors if not
    // otherwise set.  If nrec is not equal to the current
    // size of the polarization vector, it will be resized
    // and all of its contents will be lost.
    void setPol(const String &pol, Int whichRec, Int nrec);

    // get the polarization vector
    const Vector<String> &polarize() const { return itsPolz;}

    // set the xel and el offsets - converts from degrees to radians
    void setOffsets(Double xeloffset, Double eloffset);

    // get the vector of offsets
    const Vector<Double> &offsets() const { return itsOffsets;}

    // set the name this is known by
    void setName(const String &name) {itsName = name;}
    // get the name
    const String &name() const {return itsName;}

    // set the feed number of the associated feed, if any.
    // Set this to itself if there is no associated feed.
    // This number should be the feedNumber() of the
    // associated feed.
    void setSRFeedNumber(Int feedNumber) {itsSRFeedNumber = feedNumber;}
    // get the associated feed number
    Int srFeedNumber() const { return itsSRFeedNumber;}

private:
    Int itsFeedNumber, itsPhysFeed;
    Block<Int> itsIFRows;
    Vector<String> itsPolz;
    Vector<Double> itsOffsets;
    String itsName;
    Int itsSRFeedNumber;
};


#endif
