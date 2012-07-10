//# GBTFeedDDFiller: This class fills the FEED and DATA_DESC tables
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

#ifndef NRAO_GBTFEEDDDFILLER_H
#define NRAO_GBTFEEDDDFILLER_H

#include <nrao/GBTFillers/GBTCorrelation.h>
#include <nrao/GBTFillers/GBTFeed.h>
#include <nrao/GBTFillers/GBTIF.h>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/SimOrdMap.h>

#include <nrao/GBTFillers/GBTMSDataDescFiller.h>
#include <nrao/GBTFillers/GBTMSFeedFiller.h>
#include <nrao/GBTFillers/GBTMSPolarizationFiller.h>
#include <nrao/GBTFillers/GBTMSSpecWinFiller.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MeasurementSet;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

class GBTBackendTable;
class GBTIFFiller;
class GBTAntennaFile;
class GBTLO1File;

// <summary>
// This class fills the FEED and DATA_DESCRIPTION tables for GBT data.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tGBTFeedDDFIller.cc" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSet FEED table
//   <li> MeasurementSet DATA_DESCRIPTION, POLARIZATION, and SPECTRAL_WINDOW tables.
//   <li> GBT FITS files
// </prerequisite>
//
// <etymology>
// This class takes the GBT backend FITS file, IF manager file and Antenna manager
// file for a scan, digests them so that each sampler is associated with a FEED
// and IF.  At that point, the FEED, POLARIZATION, SPECTRAL_WINDOW, and DATA_DESCRIPTION
// tables can all be filled.  
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//

class GBTFeedDDFiller
{
public:
    // Default constructor, not associated with any parent MeasurementSet.
    // This is useful only when this object must exist before the MS does.
    // An MS can be attached later.  If fill is called before the MS is
    // attached, it will return False.
    GBTFeedDDFiller();

    // Construct this attached to the associated MS.
    GBTFeedDDFiller(MeasurementSet &ms);

    ~GBTFeedDDFiller();

    // attach this to a MS, breaks any existing attachments
    void attach(MeasurementSet &ms);

    Bool isAttached() {return itsAttached;}

    // digest the scan using these tables/files and fill to the
    // appropriate sub-tables.  Returns False if there is no MS
    // attached.  Selection on a specific backend has already
    // occured in the GBTIFFiller setup which should correspond
    // to the backend that sampler is from.  If
    // continuum is True then the doppler info from the LO1 tables will
    // be ignored in the GBTIF constructor.  Also, at this time 
    // each correlation will contain just one receptor.
    // If bankCheck is true then checks occur which are only appropriate
    // to the multi-bank ACS case.
    Bool fill(const GBTBackendTable &backend, const GBTIFFiller &iffiller,
	      const GBTAntennaFile &antFile, const GBTLO1File &lo1aFile,
	      const GBTLO1File &lo1bFile, const String &swtchsig,
	      Bool continuum = False, Bool bankCheck = False);

    // The number of feeds just filled.
    uInt nfeeds() {return itsNfeeds;}

    // Number of states for things just filled.
    uInt nstates() {return itsNstate;}

    // Get a specific GBTFeed - 0 through nfeeds()-1
    const GBTFeed &feed(uInt whichFeed) const {return *(itsFeeds[whichFeed]);}

    // Get a specific GBTFeed to possibley modify - 0 through nfeeds()-1
    GBTFeed &feed(uInt whichFeed) {return *(itsFeeds[whichFeed]);}

    // Set the midpoint and interval for this scan - generally this
    // isn't known until after most of this information is filled
    // and need.  This only affects the FEED subtable.
    void setTime(Double time, Double interval) {itsFeedFiller.setTime(time, interval);}

    // flush all associated fillers
    void flush();

    // Does this most recently filled data contain any beam switched data?
    Bool hasBeamSwitched() {return itsFeedFiller.hasBeamSwitched();}

    // These functions allow one to easily iterate through and get the IDs
    // and sampler row numbers for each of the correlations and states
    // involved in all feeds in the most recently filled data.
    // <group>
    // Reset the iterator to the top.
    void origin() const;
    // The total number of correlations, all feeds.
    uInt ncorr() const {return itsNcorr;}
    // Move to the next correlation.  Can do this ncorr() times after origin().
    void next() const;
    // The current feed
    const GBTFeed &currFeed() const {return *itsCurrFeedPtr;}
    // The current IF
    const GBTIF &currIF() const {return *itsCurrIFPtr;}
    // The current correlation
    const GBTCorrelation &currCorr() const {return *itsCurrCorrPtr;}
    // </group> 
private:
    Bool itsAttached;
    //# itsNfeeds is the true number of feeds in use in itsFeeds
    uInt itsNfeeds, itsNcorr, itsNstate;
    Block<GBTFeed *> itsFeeds;

    //# this associates a feed index as seen in the IF manager file
    //# with a specific feed in itsFeeds.
    SimpleOrderedMap<Int, Int> itsFeedMap;

    GBTMSFeedFiller itsFeedFiller;
    GBTMSPolarizationFiller itsPolFiller;
    GBTMSSpecWinFiller itsSWFiller;
    GBTMSDataDescFiller itsDDFiller;

    mutable uInt itsCurrFeed, itsCurrIF, itsCurrCorr,
	itsCurrNif, itsCurrNcorr, itsCurrCount;
    mutable GBTFeed * itsCurrFeedPtr;
    mutable GBTIF *itsCurrIFPtr;
    mutable GBTCorrelation *itsCurrCorrPtr;

    void clear();

    //# these do the actual work in fill
    void digestFeeds(const GBTAntennaFile &antFile, const GBTIFFiller &iffiller,
		     const String &swtchsig);
    void digestSampler(const GBTBackendTable &backend, const GBTIFFiller &iffiller,
		       const GBTLO1File &lo1aFile, const GBTLO1File &lo1bFile,
		       Bool continuum, Bool bankCheck);
    void fillMSSW();
    void findOtherBanks(const GBTIFFiller &iffiller);

    //# Undefined and unavailable
    GBTFeedDDFiller(const GBTFeedDDFiller &);
    GBTFeedDDFiller &operator=(const GBTFeedDDFiller &);
};


#endif


