//# GBTFITSBase.h: A base class for GBT FITS files - primary keywords.
//# Copyright (C) 2001,2002
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

#ifndef NRAO_GBTFITSBASE_H
#define NRAO_GBTFITSBASE_H

//#! Includes go here

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <fits/FITS/FITSTable.h>
#include <measures/Measures/MEpoch.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
//# Forward Declarations

// <summary>
// A base class for GBT FITS files - primary keywords.
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
// GBT FITS files have several required keywords common to all of them.
// This class provides access to those fields and provides default values
// for older FITS files where the required keywords were not present.
// Specific classes for specific types of GBT FITS files are derived 
// from this class.  Only the record of keywords from the primary HDU are
// decoded here.
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

class GBTFITSBase
{
public:
    // This constructs a GBTFITSBase that has no primary keywords.
    GBTFITSBase();

    // Copy constructor.
    GBTFITSBase(const GBTFITSBase &other);

    virtual ~GBTFITSBase() {;}

    // Assignment operator, copy semantics.
    GBTFITSBase &operator=(const GBTFITSBase &other);

    // attach this object to a new FITS file.  If there are problems with the
    // file, the return value is False and appropriate warnings are sent to the
    // logger.  The resulting object is not attached to any file.
    virtual Bool attach(const String &fitsFile) = 0;

    // Is this object attached to a FITS file.  This base class is never
    // attached to a file.
    virtual Bool isAttached() const {return False;}

    // The name of the attached FITS file.  Returns an empty string if
    // it is not attached.
    virtual const String &file() const = 0;

    // Attach this base class to a new primary keyword record.  This
    // resets all of the internals (including usedPrimaryKeys()).
    // Usually, a derived classes implementation of attach() will 
    // invoke this function at some point.  Returns false if any of
    // the expected keywords for this FITSVER were not found.  If
    // scanlog is True, then the more limited set of keywords for the
    // scanlog will be expected.
    virtual Bool setPrimaryKeys(const Record &keys, Bool scanlog = False);

    // The record containing the primary keywords.
    virtual const Record &primaryKeys() const {return itsKeys;}

    // A vector indicating which keywords in primaryKeys() have been used
    // so (either by this base class or by a derived class).  The index here
    // is the fieldNumber of the desired key.  If that element is True, its
    // value has been used.  Standard keywords required by the FITS standard
    // (e.g. SIMPLE, BITPIX, NAXIS, EXTEND, and END) are always marked as
    // used.
    virtual const Vector<Bool> &usedPrimaryKeys() const {return itsUsedKeys;}

    // Mark the indicated keyword as being used.  Returns False if the keyword
    // is not found.
    virtual Bool markAsUsed(const String &keywordName);

    // Returns a record containing just the unused keywords.
    virtual Record unusedPrimaryKeys() const;

    // ORIGIN - where this file originated.
    // Default value: "NRAO Green Bank"
    virtual const String &origin() const {return itsOrigin;}

    // INSTRUME - device or program of origin.
    // Default value: empty string.
    virtual const String &instrume() const {return itsInstrume;}

    // GBTMCVER - telescope control software release
    // Default value: empty string.
    virtual const String &mcVersion() const {return itsMCVersion;}

    // FITSVER - FITS definition version for this device.
    // Default value: "0.0"
    virtual const String &fitsVers() const {return itsFitsVer;}

    // Leading integer (before decimal) in FITSVER - version of base class.
    virtual Int baseVersion() const {return itsBaseVer;}

    // Trailing integer (after decimal) in FITS - version of device.
    virtual Int deviceVersion() const {return itsDeviceVer;}

    // DATEBLD - time program was linked
    // Default value: empty string.
    virtual const String &datebld() const {return itsDatebld;}

    // SIMULATE - is the instrument in simulate mode?
    // Default value: F
    virtual Bool simulate() const {return itsSimulate;}

    // DATE-OBS - Manager parameter startTime
    // Default value: MEpoch(0, MEpoch::Ref(MEpoch::UTC))
    // This also uses the TIMESYS keyword when present (UTC is assumed if not)
    virtual const MEpoch &dateObs() const {return itsDateObs;}

    // TELESCOP - green bank telescope
    // Default value: GBT. 
    // The string NRAO_GBT is changed to GBT for use within AIPS++.
    virtual const String &telescop() const {return itsTelescop;}

    // OBJECT - source
    // Default value: empty string.
    virtual const String &object() const {return itsObject;}

    // PROJID - project ID
    // Default value: empty string.
    virtual const String &projid() const {return itsProjid;}

    // OBSID - scan id
    // Default value: empty string.
    virtual const String &obsid() const {return itsObsid;}

    // SCAN - scan number
    // Default value: -1
    virtual Int scan() const {return itsScan;}

    // utility method to convert strings to integers
    static Int stringToInt(const String &str);

    // utility method to convert strings to doubles
    static Double stringToDouble(const String &str);

    // utility to construct a scratch table from a FITSTable.
    // It is the responsibility of the calling entity to delete
    // the returned pointer.
    static Table *tableFromFITS(FITSTable &fits);
private:
    Record itsKeys;
    Vector<Bool> itsUsedKeys;

    String itsOrigin, itsInstrume, itsMCVersion, itsFitsVer, itsDatebld,
	itsTelescop, itsObject, itsProjid, itsObsid;
    Int itsBaseVer, itsDeviceVer, itsScan;
    Bool itsSimulate;
    MEpoch itsDateObs;

    // fill in the default values, clean up internals
    void init();
};

#endif


