//# GBTAntennaFile.h: digest the FITS file holding GBT Antenna info
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

#ifndef NRAO_GBTANTENNAFILE_H
#define NRAO_GBTANTENNAFILE_H

//#! Includes go here

#include <nrao/FITS/GBTFITSBase.h>

#include <casa/Containers/Record.h>
#include <casa/Containers/SimOrdMap.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MEpoch;
template <class T> class Matrix;
} //# NAMESPACE CASA - END


// <summary>
// GBTAntennaFile digests the FITS file holding GBT Antenna info
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
// </prerequisite>
//
// <etymology>
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
//
// <thrown>
//#! A list of exceptions thrown if errors are discovered in the function.
//#! This tag will appear in the body of the header file, preceding the
//#! declaration of each function which throws an exception.
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

class GBTAntennaFile : public GBTFITSBase
{
public:

    // This constructs a GBTAntennaFile that is not attached to any FITS file.
    // The indicated default values are returned for the function calls.
    GBTAntennaFile();

    // Construct a GBTAntennaFile attached to the indicated FITS file.
    // Warnings are sent to the logger if there is a problem with the file.
    // In such a case, the resulting object will not be attached to any
    // file.  In this way, such a file will be ignored and filling can
    // proceed - having notified the user of the problesm with that file.
    GBTAntennaFile(const String &antennaFile);

    // Copy constructor.
    GBTAntennaFile(const GBTAntennaFile &other);

    ~GBTAntennaFile();

    // Assignment operator, copy semantics.
    GBTAntennaFile &operator=(const GBTAntennaFile &other);

    // attach this object to a new antenna file.  If there are problems with the
    // file, the return value is False and appropriate warnings are sent to the
    // logger.  The resulting object is not attached to any file.
    Bool attach(const String &antennaFile);

    // Is this object attached to a FITS file.
    Bool isAttached() const {return itsAttached;}

    // The name of the attached FITS file.  Returns an empty string if
    // it is not attached.
    const String &file() const {return itsFileName;}

    // The value of the EXTNAME keyword for the ANTPOS* HDU.  This indicates 
    // the type of optics in use for this scan.  Possible values current include 
    // ANTPOSPF (prime focus), ANTPOSGR (Gregorian) and ANTPOSST (stow).  Defaults 
    // to an empty string if not attached.
    const String &opticsType() const {return itsExtname;}

    // The position as determined by the SITE* keywords.
    // This defaults to the value from MeasTable::Observatory for "GBT"
    const MPosition &position() const {return itsPosition;}

    // A record containing all of the keywords related to the pointing model.
    // This is all of the keywords not otherwise used plus an OPTICS_TYPE keyword
    // which holds the opticsType() for completeness.  Returns an empty record
    // if no file is attached.
    const Record &pointingModel() const {return itsPointingKeywords;}

    // compares the otherPointingModel record to this one.  Returns True if they
    // both contain exactly the same fields, with exactly the same types and values.
    Bool equalPointingModel(const RecordInterface &other) const;

    // The EQUINOX value, defaults to 2000.0 if not attached.  This is the equinox at
    // the start of the scan.
    Double equinox() const {return itsEquinox;}

    // The appropriate MDirection::Types given the RADECSYS keyword value.
    // Returns MDirection::DEFAULT if no file is attached as well as if
    // RADECSYS is "USER".  Use isUserType() to check if this is a USER 
    // defined coordinate system.  At this point, I don't know what to do with
    // such a coordinate system.
    MDirection::Types type() const {return itsCoordType;}

    // return the raw RADECSYS keyword value
    const String &radesys() const {return itsRadesys;}

    // return the raw INDICSYS keyword value
    const String &indicsys() const {return itsIndicsys;}

    // Returns TRUE if the INDICSYS keyword is "USER".
    Bool isUserCoordSys() const {return itsIsUser;}

    // Get the mean pointing values at the given time using the indicated
    // interval.  This finds the mean values in that interval centered on time.
    // Returns False if there is no table attached or if the interval falls
    // off of the end of the table.  In that case, the values are from
    // the nearest value at that particular end of the table.
    // If the requested interval falls completely between adjacent rows in
    // this table, a simple linear interpolation is used.
    Bool getPointing(Double time, Double interval,
		     MDirection &j2000, MDirection &user);

    // Fill the entire pointing information (RAJ2000, DECJ2000, MNT_AZ, MNT_EL,
    // MAJOR and MINOR, and REFRACT) to the indicated Table.  Columns are added as necessary.
    // The incremental storage manager is used.  The DMJD column is converted to
    // a TIME column of MEpoch in seconds.  Returns False if no file is attached.
    // Only Epochs after the last value of any TIME column in pointingTable will
    // be filled.
    Bool fillPointingTable(Table &pointingTable);

    // Get the mean focus values for the prime focus optics at the given
    // time using the indicated interval..  
    // Returns False if there is no table attached or if the interval falls off
    // the end of the table or if the optics type is not that of prime focus.
    // If the requested interval falls completely between adjacent rows in the
    // table, a simple linear interpolation is used.
    Bool getPrimeFocus(Double time, Double interval, Quantity &focus, Quantity &rotation,
		       Quantity &x);

    // Get the mean focus values for the gregorian optics at the given
    // time using the indicated interval..  
    // Returns False if there is no table attached or if the interval falls off
    // the end of the table or if the optics type is not that of prime focus.
    // If the requested interval falls completely between adjacent rows in the
    // table, a simple linear interpolation is used.
    Bool getGregorianFocus(Double time, Double interval, 
			   Quantity &xp, Quantity &yp, Quantity &zp,
			   Quantity &xt, Quantity &yt, Quantity &zt);

    // Fill the entire focus information to the indicated Table.  Columns are addded
    // as necessary.  The incremental storage manager is used.  The DJMD column is
    // converted to a TIME column of MEpoch in seconds.  Returns False if no file
    // is attached.  Only times after the last value of any TIME column in 
    // focusTable will be filled. 
    Bool fillFocusTable(Table &focusTable);

    // return the beam offset table - defauls to an empty table
    const Table &beamOffsetTable() const { return itsBeamOffTable ? *itsBeamOffTable : emptyTable;}

    // For a given physical feed number, return the corresponding offsets
    // srfeed1 and srfeed2, and name.  Returns -1 if that feed isn't found.
    Bool getFeedInfo(Int whichFeed, Double &xeloffset, Double &eloffset,
		     Int &srfeed1, Int &srfeed2, String &feedName) const;

    // how many physical feeds are present
    uInt nfeeds() const { return itsRowFromFeed.ndefined();}

    // get feed value from number, starting from 0 through (nfeeds-1), this
    // can be used in getFeedInfo - returns -1 if outside of the range
    Int getFeedValue(uInt feedNumber) const
    { return (feedNumber < nfeeds()) ? itsRowFromFeed.getKey(feedNumber) : -1; }

    // return the tracking beam name
    const String trckbeam() const { return itsTrckBeam;}

private:
    //# it seems necessary to put the antenna table contents into a scratch table
    Table *itsTable;

    //# the Beam Offset table
    Table *itsBeamOffTable;

    //# and empty table, in case there is no beam offset table present
    Table emptyTable;

    //# name of Antenna file
    String itsFileName;

    //# is it attached?
    Bool itsAttached;

    //# keyword based fields
    String itsExtname;
    MPosition itsPosition;
    Record itsPointingKeywords;
    Double itsEquinox;
    MDirection::Types itsCoordType;
    Bool itsIsUser;
    String itsRadesys;
    String itsIndicsys;

    //# pointing columns
    ROScalarColumn<Double> itsDMJD, itsRAJ2000, itsDECJ2000, itsMAJOR, itsMINOR,
	itsMNT_AZ, itsMNT_EL, itsREFRACT;
    //# Units for those columns
    Unit itsDMJDunit, itsRAunit, itsDECunit, itsMAJORunit, itsMINORunit,
	itsMNT_AZunit, itsMNT_ELunit, itsREFRACTunit;

    //# focus columns, not all of these will be used
    ROScalarColumn<Double> itsFOCUS, itsROTATION, itsX, itsXP, itsYP, itsZP, 
	itsXT, itsYT, itsZT;
    //# Units for those columns
    Unit itsFOCUSunit, itsROTATIONunit, itsXunit, itsXPunit, itsYPunit, itsZPunit,
	itsXTunit, itsYTunit, itsZTunit;

    Double itsLastTime, itsLastInterval, itsLastStartTime, itsLastEndTime;
    uInt itsLastStart, itsLastEnd, itsLastReturnedStart, itsLastReturnedEnd;

    Double itsSecondsPerDay;

    SimpleOrderedMap<Int, Int> itsRowFromFeed;

    ScalarColumn<String> itsFeedNameCol;
    ScalarColumn<Double> itsXELOffCol, itsELOffCol;
    ScalarColumn<Int> itsSR1Col, itsSR2Col;

    String itsTrckBeam;

    // set the default values for internals
    void setDefaults();

    // digest the EXTNAME=ANTPOS* 
    void digestAntPosTab(FITSTable &antpostab, const String &extname);

    // digest the BEAM_OFFSETS table
    void digestBeamOffTab(FITSTable &beamofftab);

    // find the first and last row numbers given the
    // indicated time and interval - this assumes that this is attached
    // and that there are rows in the output table
    Bool findTimeRange(Double time, Double interval, uInt &startRow, uInt &endRow);

    // get the mean value of the indicated column over the indicate row range.
    // canWrap should be true if this is an angle which might go through 360/0
    Double getMean(ROScalarColumn<Double> &col, uInt startRow, uInt endRow,
		   Bool canWrap=False);

    // get the interpolated value of the indicated column.  Value is found assuming
    // a linear intepolation between the values at the low and high row with the
    // desired value being that at low+fraction. canWrap should be true if this
    // is an angle which might go through 360/0.
    Double getInterpolated(ROScalarColumn<Double> &col, uInt low, uInt high, 
			   Double fraction, Bool canWrap=False);
};

#endif


