//# GBTMSFeedFiller.h: GBTMSFeedFiller fills the MSFeed table for GBT fillers
//# Copyright (C) 2000,2001,2003
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

#ifndef NRAO_GBTMSFEEDFILLER_H
#define NRAO_GBTMSFEEDFILLER_H

#include <casa/aips.h>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/SimOrdMap.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <ms/MeasurementSets/MSFeed.h>

namespace casa { //# NAMESPACE CASA - BEGIN
class MSFeedColumns;
class Table;

template <class T> class Matrix;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

class GBTIFFiller;
class GBTAntennaFile;
class GBTFeed;

// <summary>
// GBTMSFeedFiller fills the MSFeed table for GBT fillers
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSets
//   <li> GBT FITS files
// </prerequisite>
//
// <etymology>
// This class puts GBT data (fills) into the MSFeed table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSFeed
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSFeed are made.  
//
// The feedIds are available to be used by 
// other GBTMS*Fillers as other subtables are filled in turn.
//
// <example>
// </example>
//
// <motivation>
// Writing a filler is a daunting task.  There are various dependencies
// between the subtables.  It helps to encapsulate the knowlege and
// task of filling a specific subtable to separate classes.
// </motivation>
//

class GBTMSFeedFiller
{
public:

    // The default ctor.  No MSFeed table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSFeed has been created.
    GBTMSFeedFiller();

    // A filler attached to an MSFeed table
    GBTMSFeedFiller(MSFeed &msFeed);

    ~GBTMSFeedFiller();

    // attach it to an MSFeed
    void attach(MSFeed &msFeed);

    // fill - using the given keys from other tables
    // this implementation always fills just one feed ID (i.e. FEED_ID == 0 
    // here) TIME and INTERVAL are both 0.0 - there is no time dependence and
    // there is no spectral window dependence.  This is only used by the
    // holography filler since as yet there is no IF information filled for
    // that backend and hence no feed information is available.
    // Only one row will ever be filled by this and this should not be
    // used after the other fill method has been used.
    void fill();

    // Add the feeds found here, reusing the previous sequence number if
    // sequence is >= 0. sequence is not checked for validity. checkPrior should
    // be used to verify that things can be reused.  
    // It should be followed by a call to setTime when the mid-point 
    // and interval appropriate for the whole scan are known.
    // return the feed ID just filled
    Int fill(const GBTFeed &feed, Int sequence);

    // Get the srfeed ID at the indicated sequency in the current
    // set of filled feeds
    Int srFeedId(Int sequence);

    // Set the srfeed ID for the indicated FEED_ID
    void setSRFeedId(Int feedId, Int srFeedId);

    // Use this prior to calling fill on all feeds to be filled.  This
    // checks to see if the previous item at sequence can be reused.
    // If this is false, it also resets the internal feed count 
    // anticipating that the subsequent fill will add new rows.
    Bool checkPrior(const GBTFeed &feed, Int sequence);

     // Set the time of the previously filled rows
    void setTime(Double time, Double interval);
    
    // get the feed id corresponding to physical feed id indicated
    Int feedId(Int whichFeed) const;

    // get the srfeed id corresponding to feedId - feed id is the
    // FEED_ID in the most recently filled block of feed information.
    Int srfeedId(Int feedId) const;

    // Get the value filled in to the GBT_FEED_NAME column for the
    // indicated FEED_ID value.
    String gbtFeedName(Int feedId) const;

    // Returns True if any of the filled FEED_IDs are beam-switched pairs
    Bool hasBeamSwitched() const {return itsHasBeamSwitched;}

    // Set the hasBeamSwitched flag
    void setHasBeamSwitched(Bool hasBeamSwitched) {itsHasBeamSwitched = hasBeamSwitched;}
    
    // flush the underlying MS subtable
    void flush() {itsMSFeed->flush();}
    
private:
    MSFeed *itsMSFeed;
    MSFeedColumns *itsMSFeedCols;
    
    // a map between the feed number in the IF table and feed id here
    SimpleOrderedMap<Int, Int> itsFeedMap;

    // remember where the last filled feed set was
    Int itsLastFeedsStart, itsLastFeedsEnd, itsMaxFeedID;

    //# GBT-specific cols in the FEED table
    ScalarColumn<Int> itsGBTSRFeedCol;
    ScalarColumn<String> itsGBTFeedNameCol, itsGBTTrckBeamCol, itsGBTRxNameCol;
    ArrayColumn<String> itsGBTRcptrsCol;

    //# keep the raw SRFEED info around for comparision time
    Vector<Int> itsLastSrfeeds;

    Bool itsHasBeamSwitched, itsSRFeedSet;

    // initialize the above for the first time
    void init(MSFeed &msFeed);

    // add a row with a new feed id given this information
    // return the new feed ID just added.
    Int addFeed(Double time, Double interval, const Matrix<Double> &offsets, 
		const Vector<String> &polTypes, const String &feedName,
		Int srFeedId, const String &trckBeam, const String &rxName,
		const Vector<String> &rcptrNames);

    // undefined and unavailable
    GBTMSFeedFiller(const GBTMSFeedFiller &other);
    void operator=(const GBTMSFeedFiller &other);
};

#endif
