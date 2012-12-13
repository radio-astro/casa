//# GBTMSFeedFiller.cc: GBTMSFeedFiller fills the MSFeed table for GBT fillers
//# Copyright (C) 2000,2001,2002,2003
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

//# Includes

#include <nrao/GBTFillers/GBTMSFeedFiller.h>

#include <nrao/FITS/GBTAntennaFile.h>
#include <nrao/GBTFillers/GBTFeed.h>
#include <nrao/GBTFillers/GBTIFFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicMath/Math.h>
#include <ms/MeasurementSets/MSFeedColumns.h>
#include <casa/Utilities/Assert.h>

GBTMSFeedFiller::GBTMSFeedFiller()
    : itsMSFeed(0), itsMSFeedCols(0), itsFeedMap(0), 
      itsLastFeedsStart(-1), itsLastFeedsEnd(-1),
      itsMaxFeedID(-1), itsHasBeamSwitched(False), itsSRFeedSet(False)
{;}


GBTMSFeedFiller::GBTMSFeedFiller(MSFeed &msFeed)
    : itsMSFeed(0), itsMSFeedCols(0), itsFeedMap(0), 
      itsLastFeedsStart(-1), itsLastFeedsEnd(-1),
      itsMaxFeedID(-1), itsHasBeamSwitched(False), itsSRFeedSet(False)
{init(msFeed);}

GBTMSFeedFiller::~GBTMSFeedFiller()
{
    delete itsMSFeed;
    itsMSFeed = 0;

    delete itsMSFeedCols;
    itsMSFeedCols = 0;
}

void GBTMSFeedFiller::attach(MSFeed &msFeed)
{init(msFeed);}

void GBTMSFeedFiller::fill()
{
    itsHasBeamSwitched = False;
    if (itsMSFeed->nrow() == 0) {
	addFeed(0.0, 0.0, Matrix<Double>(2,1,0.0), 
		Vector<String>(1,""), "", -1, "", "", Vector<String>(1,""));
	itsLastFeedsStart = itsLastFeedsEnd = 0;
    }
}

Int GBTMSFeedFiller::fill(const GBTFeed &feed, Int sequence)
{
    Int result = -1;
    if (feed.numReceptors() > 0) {
	if (sequence >= 0) {
	    // assume its the same and just reuse it at the same location
	    result = itsMSFeedCols->feedId()(sequence+itsLastFeedsStart);
	} else {
	    result = addFeed(-1.0, -1.0, feed.offsets(),feed.polType(),
			     feed.name(), feed.srFeedId(), feed.trckbeam(), 
			     feed.receiver(), feed.receptorNames());
	    
	}
    }
    itsFeedMap(feed.fitsFeedId()) = result;
    return result;
}

Bool GBTMSFeedFiller::checkPrior(const GBTFeed &feed, Int sequence)
{
    Int feedRow = sequence+itsLastFeedsStart;
    Bool result = itsLastFeedsStart >= 0 && feedRow <= itsLastFeedsEnd;
    if (result) {
	result = 
	    Int(feed.polType().nelements()) == itsMSFeedCols->numReceptors()(feedRow) &&
	    allEQ(feed.polType(), itsMSFeedCols->polarizationType()(feedRow)) &&
	    allEQ(feed.offsets(), itsMSFeedCols->beamOffset()(feedRow)) &&
	    feed.name() == itsGBTFeedNameCol(feedRow) &&
	    feed.trckbeam() == itsGBTTrckBeamCol(feedRow) &&
	    feed.receiver() == itsGBTRxNameCol(feedRow) &&
	    allEQ(feed.receptorNames(), itsGBTRcptrsCol(feedRow));
    }
    if (!result) itsMaxFeedID = -1;
    return result;
}

Int GBTMSFeedFiller::feedId(Int whichFeed) const {
    return (itsFeedMap.isDefined(whichFeed) ? 
	    itsFeedMap(whichFeed) : itsFeedMap.defaultVal());
}

Int GBTMSFeedFiller::srfeedId(Int feedId) const {
    Int result = feedId;
    if (hasBeamSwitched()) {
	// find FEED_ID 
	for (Int i=itsLastFeedsStart;i<=itsLastFeedsEnd;i++) {
	    if (itsMSFeedCols->feedId()(i) == feedId) {
		result = itsGBTSRFeedCol(i);
	    }
	}
    } 
    return result;
}
    

void GBTMSFeedFiller::init(MSFeed &msFeed)
{
    if (itsMSFeed) delete itsMSFeed;
    itsMSFeed = new MSFeed(msFeed);
    AlwaysAssert(itsMSFeed, AipsError);

    if (itsMSFeedCols) delete itsMSFeedCols;
    itsMSFeedCols = new MSFeedColumns(msFeed);
    AlwaysAssert(itsMSFeedCols, AipsError);

    // attach the non-standard columns
    itsGBTSRFeedCol.attach(*itsMSFeed, "GBT_SRFEED_ID");
    itsGBTFeedNameCol.attach(*itsMSFeed, "GBT_FEED_NAME");
    itsGBTTrckBeamCol.attach(*itsMSFeed, "GBT_TRCKBEAM");
    itsGBTRxNameCol.attach(*itsMSFeed, "GBT_RECEIVER");
    itsGBTRcptrsCol.attach(*itsMSFeed, "GBT_RECEPTORS");

    itsLastFeedsStart = itsLastFeedsEnd = itsMaxFeedID = -1;

    if (itsMSFeed->nrow() > 0) {
	Int thisRow = itsMSFeed->nrow() - 1;
	Double lastTime = itsMSFeedCols->time()(thisRow);
	itsMaxFeedID = itsMSFeedCols->feedId()(thisRow);
	itsLastFeedsEnd = itsLastFeedsStart = thisRow;
	while(thisRow > 0) {
	    thisRow--;
	    if (itsMSFeedCols->time()(thisRow) != lastTime) {
		thisRow = 0;
	    } else {
		itsMaxFeedID = 
		    max(itsMaxFeedID, itsMSFeedCols->feedId()(thisRow));
		itsLastFeedsStart = thisRow;
	    }
	}
	itsLastSrfeeds.resize(itsLastFeedsEnd-itsLastFeedsStart+1);
	// first pass, get the original feed associated with each FEED_ID
	Vector<Int> feeds(itsLastSrfeeds.nelements(),-1);
	for (uInt i=0;i<itsLastSrfeeds.nelements();i++) {
	    thisRow = i + itsLastFeedsStart;
	    feeds[itsMSFeedCols->feedId()(thisRow)] =
		atoi((itsGBTFeedNameCol(thisRow)).chars());
	}
	// second pass, convert srfeed id to its original feed
	for (uInt i=0;i<itsLastSrfeeds.nelements();i++) {
	    thisRow = i + itsLastFeedsStart;
	    Int thisSRID = itsGBTSRFeedCol(thisRow);
	    if (thisSRID >= 0) {
		itsLastSrfeeds[i] = feeds[thisSRID];
	    }
	}
    }
}

Int  GBTMSFeedFiller::addFeed(Double time, Double interval,
			      const Matrix<Double> &offsets, 
			      const Vector<String> &polTypes,
			      const String &feedName,
			      Int srFeedId,
			      const String &trckBeam,
			      const String &rxName,
			      const Vector<String> &rcptrNames)
{
    itsMSFeed->addRow();
    itsMaxFeedID++;
    uInt rownr = itsMSFeed->nrow()-1;
    itsMSFeedCols->antennaId().put(rownr, 0);
    itsMSFeedCols->beamId().put(rownr, -1);
    itsMSFeedCols->beamOffset().put(rownr, offsets);
    itsMSFeedCols->feedId().put(rownr, itsMaxFeedID);
    itsMSFeedCols->interval().put(rownr, interval);
    Int nrec = polTypes.nelements();
    itsMSFeedCols->numReceptors().put(rownr, nrec);
    Matrix<Complex> polResponse(nrec, nrec);
    polResponse = 0.0;
    polResponse.diagonal() = 1.0;
    itsMSFeedCols->polResponse().put(rownr, polResponse);
    itsMSFeedCols->polarizationType().put(rownr, polTypes);
    itsMSFeedCols->position().put(rownr, Vector<Double>(3,0.0));
    itsMSFeedCols->receptorAngle().put(rownr, Vector<Double>(nrec,0.0));
    itsMSFeedCols->spectralWindowId().put(rownr, -1);
    itsMSFeedCols->time().put(rownr, time);

    itsGBTFeedNameCol.put(rownr, feedName);
    itsGBTTrckBeamCol.put(rownr, trckBeam);
    itsGBTSRFeedCol.put(rownr, srFeedId);
    itsGBTRxNameCol.put(rownr, rxName);
    itsGBTRcptrsCol.put(rownr, rcptrNames);
    return itsMaxFeedID;
}

void GBTMSFeedFiller::setTime(Double time, Double interval)
{
    if (itsLastFeedsEnd != (Int(itsMSFeedCols->nrow())-1)) {
	// new rows have been added, need to update these
	itsLastFeedsStart = itsLastFeedsEnd+1;
	itsLastFeedsEnd = itsMSFeedCols->nrow()-1;
	// also set GBT_SRFEED_ID correctly if necessary
	if (!itsSRFeedSet) {
	    for (Int i=itsLastFeedsStart;i<=itsLastFeedsEnd;i++) {
		Int rawSRFeed = itsGBTSRFeedCol(i);
		if (rawSRFeed >= 0) {
		    itsGBTSRFeedCol.put(i, itsFeedMap(rawSRFeed));
		}
	    }
	}
    }
    Double midTime = itsMSFeedCols->time()(itsLastFeedsStart);
    Double tint = itsMSFeedCols->interval()(itsLastFeedsStart);
    if (tint < 0.5) {
	// place holding - just replace it
	midTime = time;
	tint = interval;
    } else if (tint != 0.0) {
	// average it
	Double minTime = midTime - tint/2.0;
	Double maxTime = time + interval/2.0;
	midTime = (minTime + maxTime) / 2.0;
	tint = (maxTime - minTime);
    }
    if (tint != 0.0) {
	for (Int i=itsLastFeedsStart;i<=itsLastFeedsEnd;i++) {
	    itsMSFeedCols->time().put(i,midTime);
	    itsMSFeedCols->interval().put(i,tint);
	}
    } // else it already has no time-dependence, don't bother adding one in
}

String GBTMSFeedFiller::gbtFeedName(Int feedid) const
{
    String result;
    for (Int i=itsLastFeedsStart;i<=itsLastFeedsEnd;i++) {
 	if (itsMSFeedCols->feedId()(i) == feedid) {
 	    result = itsGBTFeedNameCol(i);
 	}
    }
    return result;
}

Int GBTMSFeedFiller::srFeedId(Int sequence)
{
    Int result = -1;
    if (sequence >= 0) {
	result = itsGBTSRFeedCol(sequence+itsLastFeedsStart);
    }
    return result;
}

void GBTMSFeedFiller::setSRFeedId(Int feedId, Int srFeedId)
{
    // any attempt here disables setting it in setTime
    itsSRFeedSet = True;
    if (itsMSFeedCols->nrow() > 0) {
	// look for feedId
	Int currEnd = itsMSFeedCols->nrow();
	Int currStart = itsLastFeedsStart;
	if (itsLastFeedsEnd != (currEnd-1)) {
	    currStart = itsLastFeedsEnd + 1;
	}
	for (Int i=currStart;i<currEnd;i++) {
	    if (itsMSFeedCols->feedId()(i) == feedId) {
		itsGBTSRFeedCol.put(i,srFeedId);
		break;
	    }
	}
    }
}
