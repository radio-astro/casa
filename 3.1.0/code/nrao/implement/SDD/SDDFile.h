//# SDDFile.h: a class for accessing unipops SDD data files
//# Copyright (C) 1999,2001
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

#ifndef NRAO_SDDFILE_H
#define NRAO_SDDFILE_H

//#! Includes go here
#include <casa/aips.h>
#include <nrao/SDD/SDDIndex.h>
#include <casa/Containers/Block.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>

#include <casa/namespace.h>
//# Forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
} //# NAMESPACE CASA - END

class SDDBootStrap;
class SDDHeader;

#include <casa/iosfwd.h>

#include <casa/namespace.h>
// <summary>
// a class for accessing unipops SDD data files
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

class SDDFile {
public:
    // These file options are straight out of Table.  They
    // should be made more general.
    enum FileOption {Old=1,                 // existing table
		     New,                   // create table
		     NewNoReplace,          // create table (may not exist)
		     Scratch,               // scratch table
		     Update,                // update existing table
		     Delete};               // delete table


    // creates the parts, but nothing is attached to any file
    SDDFile();
    // open the indicated file, with the indicated options
    SDDFile(const String& fileName, FileOption option = Old);
    // Copy constructor
    SDDFile(const SDDFile& other);

    ~SDDFile();

    // assignment operator
    SDDFile& operator=(const SDDFile& other);

    // attach to a file
    void attach(const String& fileName, FileOption option = Old);

    // return a reference to the BootStrap
    SDDBootStrap& bootStrap() {return *boot_p;}

    // return a reference to the Index
    const SDDIndex& index() const {return *index_p;}
    SDDIndex& index() {return *index_p;}

    const SDDIndexRep& index(uInt entry) const 
    {return (*index_p)(entry); }
    SDDIndexRep& index(uInt entry) {return (*index_p)(entry); }

    // the number of vectors at a specific entry
    uInt nvectors(uInt entry) const { return nvectors_p[entry]; }

    // check if a specific index entry is in use
    Bool inUse(uInt entry) const { return ( index_p->inUse(entry)); }

    // return a reference to the Header at location i
    SDDHeader& header(uInt location);

    // get the data vector at the indicated location in the index
    // If there is more than one vector at that location (for OTF and PZ
    // data at the 12m), get the one indicated.  If the arguments are 
    // invalid, getData returns False and does NOT alter the array argument
    Bool getData(Array<Float>& data, uInt location, uInt which=0);

    // The following are specific to OTF data
    // they are the single values appropriate for a given location and vector
    // for non-OTF rows, these return 0 for the offsets (time, raOffset, decOffset),
    // and the header values for az and el
    Float time(uInt location, uInt which);
    Float raOffset(uInt location, uInt which);
    Float decOffset(uInt location, uInt which);
    Float az(uInt location, uInt which);
    Float el(uInt location, uInt which);

    // return a reference to the header/data corresponding to scan & subscan
    SDDHeader& header(uInt scan, uInt subscan);

    // update the bootstrap and index if the file on disk has changed
    // this also needs to reset the rowMap and make sure the "current" values
    // point at appropriate things.  If the "current" values are no longer
    // available, reset to the first thing that is available.
    // The return values are the same as the SDDIndex functions they use
    Bool fullUpdate();
    Bool incrementalUpdate();

private:
    SDDBootStrap* boot_p;
    SDDIndex* index_p;
    SDDHeader* header_p;

    Vector<Float> data_p;

    Double cosdec_p;
    Vector<Float> time_p;
    Vector<Float> raOffset_p;
    Vector<Float> decOffset_p;
    Vector<Float> az_p;
    Vector<Float> el_p;

    CountedPtr<fstream> file_p;

    // The cache of number of vectors per entry
    Block<uInt> nvectors_p;

    // For all of the current pointers, if < 0 they point at nothing valid
    // It is always true that the header and various data vectors are always from
    // the same location, only the row number may vary.
    // the current index entry location loaded here for the header and OTF data
    Int currentHeader_p;
    // This is the offset into this location that corresponds to this row
    Int currentOffset_p;

    FileOption option_p;
    String* fileName_p;

    void seek(uInt record=0);

    uInt fileMode(FileOption option) const;

    void loadLocation(uInt location);
    void resetCache();
    void init_nvectors();
};

#endif
