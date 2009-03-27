//# SDDOnLine.h: a class which encapsulates on-line SDD data files at the 140ft
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

#ifndef NRAO_SDDONLINE_H
#define NRAO_SDDONLINE_H

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
// a class for accessing on-line SDD data files at the 140ft
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

class SDDOnLine {
public:
    SDDOnLine();
    SDDOnLine(const String& sddFileName);
    SDDOnLine(const SDDOnLine& other);

    ~SDDOnLine();

    // attach to two new files
    void attach(const String& sddFileName);

    // the things we need from the SDDFile
    // The index rep for a specific rownr
    const SDDIndexRep& index(uInt rownr) const;

    // The header for a specific rownr
    const SDDHeader& header(uInt rownr) const;

    // The data array for a specific rownr
    Bool getData(Array<Float>& data, uInt rownr) const;

    // The timeStamp for a particular row
    Int timeStamp(uInt rownr) const;

    // the offStamp for a particular row
    Int offStamp(uInt rownr) const;

    // This just sees if there is a need to sync
    Bool needToSync() const;

    // sync with what is on disk, return the number of rows 
    Int sync();

    uInt nrow() { return nrow_p; }

private:
    SDDOnLine operator=(const SDDOnLine&);

    Block<Int> timeStamp_p;
    SimpleOrderedMap<uInt, Int> offMap_p;

    String sddFileName_p;
    SDDFile *sddFile_p;

    uInt nrow_p;

    Block<uInt> rowMap_p;

    void init();
    void appendMaps(uInt startSDDEntry);
};

#endif
