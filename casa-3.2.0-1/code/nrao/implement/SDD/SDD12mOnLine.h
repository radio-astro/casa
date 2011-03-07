//# SDD12mOnLine.h: a class which encapsulates on-line SDD data files at the 12-m
//# Copyright (C) 1999
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

#ifndef NRAO_SDD12MONLINE_H
#define NRAO_SDD12MONLINE_H

//#! Includes go here
#include <casa/aips.h>
#include <nrao/SDD/SDDFile.h>
#include <nrao/SDD/SDDIndex.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/Containers/OrderedPair.h>
#include <casa/Containers/Block.h>

#include <casa/namespace.h>
// <summary>
// a class for accessing on-line SDD data files at the 12-m
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SDDFile
//   <li> SDDIndex
//   <li> SDDHeader
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

class SDD12mOnLine {
public:
    SDD12mOnLine();
    SDD12mOnLine(const String& dataFileName, const String& gainFileName);
    SDD12mOnLine(const SDD12mOnLine& other);

    ~SDD12mOnLine();

    // attach to two new files
    void attach(const String& dataFileName, const String& gainFileName);

    // the things we need from the SDDFiles
    // The index rep for a specific rownr
    const SDDIndexRep& index(uInt rownr) const;

    // The header for a specific rownr
    // values will be correctly modified for OTF data here
    // for HSOURCE, VSOURCE, AZ, EL, and UTIME
    const SDDHeader& header(uInt rownr) const;

    // The data array for a specific rownr
    Bool getData(Array<Float>& data, uInt rownr) const;

    // The timeStamp for a particular row
    Int timeStamp(uInt rownr) const;

    // the gainStamp for a particular row
    Int gainStamp(uInt rownr) const;

    // the offStamp for a particular row
    Int offStamp(uInt rownr) const;

    // The OTF offset for a particular row
    // For non-otf data, these should return appropriate values
    // 0 for time, ra, dec and the actual AZ and EL for az el
    Float time(uInt rownr) const;
    Float raOffset(uInt rownr) const;
    Float decOffset(uInt rownr) const;
    Float az(uInt rownr) const;
    Float el(uInt rownr) const;

    // This returns the full range of ra values appropriate for the scan number
    // associated with this row
    Float raRange(uInt rownr) const;

    // This just sees if there is a need to sync
    Bool needToSync() const;

    // sync with what is on disk, return the number of rows 
    Int sync();

    uInt nrow() { return nrow_p; }

private:
    SDD12mOnLine operator=(const SDD12mOnLine&);

    Block<Int> timeStamp_p;
    SimpleOrderedMap<uInt, Int> offMap_p;
    SimpleOrderedMap<uInt, Int> gainMap_p;

    String dataFileName_p, gainFileName_p;
    SDDFile *dataFile_p, *gainFile_p;

    uInt nrow_p;

    PtrBlock<SDDFile*> fileMap_p;
    Block< OrderedPair<uInt, uInt> > rowMap_p;

    void init();
    void appendMaps(uInt startDataEntry, uInt startGainEntry);
};

#endif
