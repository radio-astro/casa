//# SDDBootStrap.h: this defines SDDBootStrap, which encapsulates the SDD bootstrap block.
//# Copyright (C) 1995,1999,2001
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

#ifndef NRAO_SDDBOOTSTRAP_H
#define NRAO_SDDBOOTSTRAP_H

#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/fstream.h>

#include <casa/namespace.h>
//# Forward Declarations 

class SDDBlock;

// <summary> 
// Class encapsulates user-options for a bootstrap block from an SDD file
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> SDDFile
//   <li> SDDIndex
//   <li> SDDData
//   <li> The SDD data format
// </prerequisite>
//
// <etymology>
// SDD is the Single Dish Data format used by UniPOPS.  It is also the
// on-line data format at the NRAO 12-m.  The bootstrap block is the first
// 512 bytes of an SDD file.  It contains information that describes the
// file and allows the rest of the file to be read correctly.  This class
// encapsulates that data.
// </etymology>
//
// <synopsis> 
// This class encapsulates the SDD bootstrap block.  
// It can be initialzed from a CountedPtr<fstream> object.  
// In order to be written
// to disk it must be attached to a CountedPtr<fstream> object.  
// The class correctly positions the fstream to the
// start of the file.  The information in the bootstrap is retrieved via
// member functions.  Most of the information in the bootstrap can be
// set.  The class does basic sanity checking on the information in the
// bootstrap.  It is expected that this class will be used with other
// SDD classes (most users will simply use SDDFile).
// The hasChanged() function checks the internal copy of the bootstrap block with
// the block on disk (if attached) and returns True if the file has changed
// (i.e. whats on disk does NOT match this object) and False if it has
// not changed.
// </synopsis> 
//
// <example>
// Create an SDDBootStrap from a file on disk.
// Print some information, set some values, write it out to the same file.
// <srcblock>
//     fstream* file_ptr = new fstream("file_name", ios::nocreate);
//     CountedPtr<fstream> file(file_ptr);
//     SDDBootStrap bs(file);
//     cout << "Number of index records : " << bs.nIndexRec() << endl;
//     cout << "Number of data records : " << bs.nDataRec() << endl;
//     cout << "Maximum index entry in use : " << bs.maxEntryUsed() << endl;
//
//     // set some new values
//     bs.setNDataRec(bs.nDataRec()+5);
//     if (!bs.setMaxEntryUsed(bs.maxEntryUsed()+1)) {
//        cout << "unable to set max entry used" << endl;
//        // do something appropriate here
//     }
//
//     // write it out
//     bs.write();
// </srcblock>
// </example>
//
// <motivation>
// The SDD file format is used by UniPOPS and the on-line data
// system at the NRAO 12-m.  It is necessary to read this type of
// data directly into aips++.  Specifically, a table storage manager
// can be written to make an SDD file look like a table.  Underneath that
// storage manager are the SDD classes to encapsulate the data.
// </motivation>
//
//
// <todo asof="">
// <li> I'm still not happy with the update mechanism
// <li> Need to be more explicit about RO versus RW or is that info 
// available directly from the fstream?
// </todo>

class SDDBootStrap
{
public: 

    enum TYPE_OF_SDD { DATA = 0,      // normal data file
		       RECORDS = 1 }; // individual records file

    enum SDD_VERSION { ORIGINAL = 0,  // used short integers, limited file size
		       CURRENT = 1 }; // uses long integers


    // Default constructor : empty CURRENT version
    // NOT attached to a file
    SDDBootStrap();
    // create from an fstream ptr in CountedPtr<fstream>
    SDDBootStrap(CountedPtr<fstream> streamPtr);
    // from an existing SDDBootStrap, copies all values
    SDDBootStrap(const SDDBootStrap& other);

    ~SDDBootStrap();
    
    // The assignment operator, does a full copy
    SDDBootStrap& operator=(const SDDBootStrap& other);

    // Attach
    void attach(CountedPtr<fstream> newStreamPtr);
    // write it out, must already be attached
    void write();

    // Get the CountedPtr<fstream> in use here
    CountedPtr<fstream>& theStream() { return theFile_p; }

    // These just return the values.
    uInt nIndexRec() const {return nIndexRec_p;}
    uInt nDataRec() const {return nDataRec_p;}
    uInt bytesPerRecord() const {return bytesPerRec_p;}
    uInt bytesPerEntry() const {return bytesPerEntry_p;}
    uInt maxEntryUsed() const {return maxEntryUsed_p;}
    uInt counter() const {return counter_p;}
    uInt type() const {return type_p;}
    uInt version() const {return version_p;}

    // Maximum index entries that can be used, based on
    // the values in the bootstrap
    uInt maxEntries() const 
    {return ((nIndexRec_p-1) * bytesPerRec_p / bytesPerEntry_p);}

    // the numebr of entries per record
    uInt entriesPerRecord() const
    {return bytesPerRec_p / bytesPerEntry_p;}

    // Set things that are allowed to be set
    void setNIndexRec(uInt newValue) {nIndexRec_p = newValue;}
    void setNDataRec(uInt newValue) {nDataRec_p = newValue;}
    void setBytesPerEntry(uInt newValue) {bytesPerEntry_p = newValue;}
    void setVersion(SDD_VERSION newValue) {version_p = newValue;}
    void setType(TYPE_OF_SDD newValue) {type_p = newValue;}

    // this requires a sanity check to ensure it is less than maxEntries
    Bool setMaxEntryUsed(uInt newValue);

    // This returns True if the bootstrap on disk differs from the
    // one here.  It does not change the bootstrap on disk.
    Bool hasChanged();

    // sync() re-reads the values from disk.  Any internal values
    // will be forgotten.
    void sync();

private:
    // this enum describes where things are in the bootstrap block
    // for ORIGINAL version files, these are short (2-byte) unsigned integers
    // for CURRENT version files, these are long (4-byte) unsigned integers
    // all unused space is filled with zeros (both ORIGINAL and CURRECT).
    enum { N_INDEX_REC     = 0,   // Number of index records
	   N_DATA_REC      = 1,   // Number of data records
	   BYTES_PER_REC   = 2,   // Number of bytes/records, must be 512
	   BYTES_PER_ENTRY = 3,   // Number of bytes/index-entry
	   MAX_ENTRY_USED  = 4,   // Number (origin=1) of 
				  // largest index entry in use
	   COUNTER         = 5,   // incremented by 1 whenever the file
				  // changes, wraps back to 0 at max
	                          // unsigned 4-byte integer
	   TYPE            = 6,   // either DATA or RECORDS
	   VERSION         = 7};  // either ORIGINAL or CURRENT
    uInt nIndexRec_p, nDataRec_p, bytesPerRec_p, bytesPerEntry_p,
	maxEntryUsed_p, counter_p, type_p, version_p;

    SDDBlock* bs_p;
    CountedPtr<fstream> theFile_p;
};

inline Bool SDDBootStrap::setMaxEntryUsed(uInt newValue) 
{
    if (newValue <= maxEntries()) maxEntryUsed_p = newValue;
    return (newValue == maxEntryUsed_p);
}


#endif
