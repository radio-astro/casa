//# GBTIFLOHelper.h: GBTIFLOHelper helps the GBT fillers use the IF and LO data
//# Copyright (C) 2001,2002,2003
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

#ifndef NRAO_GBTIFLOHELPER_H
#define NRAO_GBTIFLOHELPER_H

//#! Includes go here
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/BasicSL/String.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class Table;
} //# NAMESPACE CASA - END

class GBTIFFiller;
class GBTLO1File;
class GBTLO1DAPFiller;

#include <casa/namespace.h>

// <summary>
// GBTIFLOHelper helps the GBT filler use the IF and LO FITS files.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> GBTIFFiller
//   <li> GBTLO1DAPFiller
//   <li> Contents of the GBT IF and LO FITS files
// </prerequisite>
//
// <etymology>
// The GBT fillers need to take the IF and LO fits files plus their receiver
// tables and other information to arrive at feeds, spectral windows, and
// polarizations as well as groupings of data so that they can be filled into
// the MS appropriately.
// </etymology>
//
// <synopsis>
// Currently, this is only useful for SpectralProcessor data.  It needs to be
// generalized as much as possible.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// The code in the SP filler that this is derived from was getting sufficiently
// large and confusing that it was difficult to see the forest for the trees.
// Moving it here served to encapsulate it all in one place, hopefully making
// it easier to modify, understand, and reuse.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> This needs to work for the other backends as well.
// </todo>

class GBTIFLOHelper
{
public:
    // the ifFiller, lo1File and lo1Handler are used directly here.  If lo1File is a 
    // non-null pointer, it will be used in preference to lo1Handler.  Neither should
    // be deleted by the calling object until this object is no longer going to be
    // used.
    GBTIFLOHelper(GBTIFFiller *ifFiller,
		  const GBTLO1File *lo1AFile, 
		  const GBTLO1File *lo1BFile, 
		  GBTLO1DAPFiller *lo1Handler,
		  const Table &samplerTable,
		  const Table &portTable,
		  uInt nphases, uInt nchannels,
		  const String &dcrBank = "");

    ~GBTIFLOHelper();

    // number of independent feeds in use in the sampler table
    uInt nfeeds() const {return itsSamplerFeeds.nelements();}

    // map from sampler feed number (starts from 0 through (nfeeds()-1)
    // to feedNumber (from iffiller).  feedNumber should be used
    // as the whichFeed argument to the other methods here.
    uInt whichFeed(uInt samplerFeed) {return itsSamplerFeeds[samplerFeed];}

     // the feed IDs as returned by GBTIFFiller
    Int feedId(uInt whichFeed) const {return itsFeedIds[whichFeed];}

    // row(s) in receiver table for each feed.  The validity of whichFeed is not checked.
    const Block<uInt> &receiverRows(uInt whichFeed) const {return *(itsRcvrRows[whichFeed]);}

    // Polarization information for the indicated feed.  The validity of whichFeed
    // is not checked.
    // <group>
    const Vector<Int> &corrType(uInt whichFeed) const {return itsCorrType[whichFeed];}
    const Array<Int> &corrProduct(uInt whichFeed) const {return itsCorrProduct[whichFeed];}
    // </group>

    // Feed information for the indicated feed.  The validity of whichFeed is
    // not checked.
    // <group>
    // From the "polarize" column
    const Vector<String> &polarizationType(uInt whichFeed) const {return itsPolType[whichFeed];}
    // From the "receiver" column
    const String &receiverName(uInt whichFeed) const {return itsRxName[whichFeed];}
    // From the "receptor" column
    const Vector<String> &feedName(uInt whichFeed) const {return itsFeedName[whichFeed];}
    // From the "feed" column if present, returns -1 if not.
    Int feed(uInt whichFeed) const {return itsFeed[whichFeed];}
    // From the "srfeed1" column if present, returns -1 if not.
    Int srfeed1(uInt whichFeed) const {return itsSrfeed1[whichFeed];}
    // From the "srfeed2" column if present, returns -1 if not.
    Int srfeed2(uInt whichFeed) const {return itsSrfeed2[whichFeed];}
    // From the "high_cal" column if present, returns -1 if not.
    Int highCal(uInt whichFeed) const {return itsHighCal[whichFeed];}
    // From the "bank" column if present, returns "" if not.
    const String &bank(uInt whichFeed) const {return itsBank[whichFeed];}
    // Also from the "bank" column, but for the appropriate switched feed,
    // this returns the bank of other feed involved if any switching is
    // going on AND this is from the ACS.
    const String &srbank(uInt whichFeed) const {return itsSRBank[whichFeed];}
    // The feedName corresponding to srbank
    const Vector<String> &srFeedName(uInt whichFeed) const 
    {return itsSRFeedName[whichFeed];}
    // </group>

    // Spectral information.  The validity of whichFeed and whichPhase is not checked.
    // Currently the SPECTRAL_WINDOW filler constructs the frequency axis 
    // from these linear values, although that could move here if appropriate.
    // <group>
    // all of the nphases values are returned together for the first timestamp in
    // the LO1 table.  Also set are the refframe (reference frame),
    // doptype (velocity definition) and souvel (source velocity in m/s) arguments.
    const Vector<Double> &centerFreqs(uInt whichFeed, MFrequency::Types &refframe,
				      MDoppler::Types &doptype, Double &souvel);
    // channel spacing is independent of phase and time
    Double deltaFreq(uInt whichFeed) const {return itsDeltaFreq[whichFeed];}

    // reference channel is independent of phase and time
    // Channels are numbered from 0 in this scheme.
    Double refChan(uInt /*whichFeed*/) const {return itsRefChan;}
    // </group>

    // The LO1 file in use for each feed
    const GBTLO1File &lo1File(uInt whichFeed) const {return *(itsLO1Files[whichFeed]);}

    // The shape of the FLOAT_DATA column for the indicated feed.
    // The value of whichFeed is not checked.
    const IPosition &shape(uInt whichFeed) const {return itsShapes[whichFeed];}
private:
    uInt itsNfeeds, itsNphases, itsNchannels;
    Double itsRefChan;
    PtrBlock<const GBTLO1File *> itsLO1Files;
    GBTLO1DAPFiller *itsLO1Filler;

    PtrBlock<Block<uInt> *> itsRcvrRows;


    Block<Double> itsT0CenterFreq;
    Block<Double> itsSFFMultiplier;

    Block<Int> itsFeedIds;

    Block<Vector<Int> > itsCorrType;
    Block<Vector<Double> > itsCenterFreq;
    Block<Vector<String> > itsPolType;
    Block<String> itsRxName, itsBank, itsSRBank;
    Block<Vector<String> > itsFeedName, itsSRFeedName;
    Block<Int> itsFeed, itsSrfeed1, itsSrfeed2, itsHighCal;

    Block<Array<Int> > itsCorrProduct;

    Block<Double> itsDeltaFreq;

    Block<IPosition> itsShapes;

    Vector<Double> itsLO1offset;

    Vector<uInt> itsSamplerFeeds;

    // function to try and ensure that SFF_sideband, SFF_multiplier, SFF_offset, initial LO1,
    // center_IF and center_Sky are all consistent.
    void ifParms(GBTIFFiller &ifFiller,
		 Int whichRow, Double &sffSideband, Double &sffMultiplier, Double &sffOffset);

    // undefined and unavailable
    GBTIFLOHelper();
    GBTIFLOHelper(const GBTIFLOHelper &);
    GBTIFLOHelper &operator=(const GBTIFLOHelper &);

};

#endif


