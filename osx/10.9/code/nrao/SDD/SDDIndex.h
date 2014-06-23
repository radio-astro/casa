//# SDDIndex.h: classes contining SDDFile index information
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

#ifndef NRAO_SDDINDEX_H
#define NRAO_SDDINDEX_H

#include <casa/aips.h>
#include <nrao/SDD/SDDBootStrap.h>
#include <nrao/SDD/SDDBlock.h>
#include <casa/OS/Time.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <casa/Utilities/Fallible.h>

#include <casa/namespace.h>
// <summary>
// SDDIndexRep is the individual index field in an SDDIndex
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

class SDDIndexRep {
public:
    // construct an empty index entry
    SDDIndexRep();

    // construct one from another
    SDDIndexRep(const SDDIndexRep& other);

    ~SDDIndexRep() {;}

    // assinment operator, true copy
    SDDIndexRep& operator=(const SDDIndexRep& other);

    // operator== returns True if EVERY value of this is the same as other
    Bool operator==(const SDDIndexRep& other) const;
    // operator!= returns False if ANY value of this is not the same as other
    Bool operator!=(const SDDIndexRep& other) const;

    // returns True if this IndexRep has changed
    // remains True until a clearChangeFlag has been called
    Bool hasChanged() const { return hasChanged_p;}
    void clearChangeFlag() { hasChanged_p = False;}

    // these function return the values stored in this index entry
    // The starting record for this scan - 0 relative
    uInt firstRecord() const { return firstRecord_p; }
    void setFirstRecord(uInt recnum) 
    { hasChanged_p = True; firstRecord_p = recnum;}
    // The total number of records used for this scan
    uInt nRecords() const { return nRecords_p; }
    void setnRecords(uInt nrecs) 
    { hasChanged_p = True; nRecords_p = nrecs;}

    // the Coordinates of this scan in the indicated coordinate system
    // Units are radians.  The coordinate system is the 8 character
    // unipops coordinate string.
    Float hCoord() const { return hCoord_p; }
    void sethCoord(Float hpos) { hasChanged_p = True; hCoord_p = hpos; }
    Float vCoord() const { return vCoord_p; }
    void setvCoord(Float vpos) { hasChanged_p = True; vCoord_p = vpos; }
    String coordSys() const { return posCodeMap_p[coordSys_p]; }
    uShort coordCode() const { return coordSys_p; }
    // If there are not exactly 8 chars in the new coordSys or
    // the coordSys string is unrecognized, it is set to the null string
    void setcoordSys(const String& new_coordSys);
    void setcoordSys(uShort new_coordCode);

    // The source name, must be less than 16 characters
    String source() const { return source_p; }
    // If there are more than 16 characters, they are not used
    void setsource(const String& new_source);

    // The scan number, must be less than 100000
    uInt scan() const { return scan_p; }
    // If it is out of range, it gets set to zero (unused)
    void setscan(uInt newScan);

    // The subscan number, must be between 1 and 99
    uInt subScan() const { return subScan_p; }
    // If it is out of range, it and scan are set to zero (unused)
    void setsubScan(uInt newSubScan);

    //  For LINE scans (isLine() is True) freqResolution and restFrequency
    //  are available.  For CONT scans (isLine() is False), slewRate and
    // intTimePerPoint are available.
    // If a value is not-available - an IEEE NaN is returned.
    // If an attempt is made to set something that isn't otherwise available,
    // that will switch the type (i.e. setting slewRate while isLine() is True
    // changes isLine() to False).  Any values currently stored in the now
    // unavailable locations are lost (i.e. if slewRate is set, any value in
    // freqResolution and restFrequency is permanently lost).
    Float freqResolution() const { return freqRes_p; }
    void setfreqResolution(Float new_freqRes);
    Float slewRate() const { return slewRate_p; }
    void setslewRate(Float new_slewRate);
    Double restFrequency() const { return restFreq_p; }
    void setrestFrequency(Double new_restFreq);
    Double intTimePerPoint() const { return intTime_p; }
    void setintTimePerPoint(Double new_intTime);

    // LST in hours
    Float lst() const { return lst_p; }
    void setlst(float new_lst) { hasChanged_p = True; lst_p = new_lst; }

    // UT data as a Time object
    Time utDate() const { return utDate_p; }
    void setutDate(const Time& new_utDate) 
    { hasChanged_p = True; utDate_p = new_utDate;}

    // The 4 character observing mode
    String mode() const { return obsModeMap_p[mode_p]; }
    uShort modeCode() const { return mode_p; }
    // If the mode is not recognized it is set to the null string
    void setmode(const String& new_mode);
    void setmode(uShort new_modeCode);

    // For RECORDS data (currently only available for GB data)
    // this returns the record number for the indexed data.
    // A negative number implies that this is not records data
    // and also turns off intPhase (loosing any current value)
    Int record() const { return record_p; }
    void setrecord(Int new_record);
    // Also for RECORDS data, this returns the integration phase
    // number for the indexed data.  It must be less than 64.
    // A negative number implies that this is not records data
    // and also turns off record (loosing any current value)
    // Attempting to set this to an invalid number (>=64) is
    // equivalent to setting it to a negative number.
    Int intPhase() const { return intPhase_p; }
    void setintPhase(Int new_intPhase);

    // Returns TRUE if this indexes spectral line data
    // otherwise False
    Bool isLine() const { return isLine_p; }
    
    // change this entry to be a spectral line entry
    // This looses any current value of slewRate or intTimePerPoint
    void setLine();
    // change this entry to be a continuum entry
    // This looses any current value of freqResolution or restFrequency
    void setCont();

private:
    Bool isLine_p, hasChanged_p;
    uShort coordSys_p, mode_p;
    uInt firstRecord_p, nRecords_p, scan_p, subScan_p;
    Int record_p, intPhase_p;
    Float hCoord_p, vCoord_p, freqRes_p, slewRate_p, lst_p;
    Double restFreq_p, intTime_p;
    String source_p;
    Time utDate_p;

    void init_maps();

    static Block<String> posCodeMap_p;
    static Block<String> obsModeMap_p;
};
    

// <summary>
// SDDIndex is the index of an SDDFile, a set of SDDIndexReps
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

class SDDIndex{
public:
    // construct index from an SDDbootStrap - reads from file if present
    SDDIndex(const SDDBootStrap& bs);
    // copy constructor
    SDDIndex(const SDDIndex& other);

    // clean up the pointers
    ~SDDIndex();

    // assignment operator, true copy
    SDDIndex& operator=(const SDDIndex& other);

    // For both types of update, if the boostrap block is the same,
    // nothing actually changes in the index.  Each returns True if an
    // actual update occured and false if things remain the same.
    // do a full update from the disk file
    Bool fullUpdate();
    // do an incremental update : assumes that the file has mearly grown
    // Attempts to verify that assumption by checking the values of the last
    // 8 known index entries with their counterparts on disk and verifying
    // that they are still the same. 
    // Return True if incremental was the correct model (or nothing has changed)
    // Returns False if incremental was NOT the correct model, a fullUpdate 
    // could then be tried if desirable
    Bool incrementalUpdate();

    // attach to a new file via an SDDBooStrap
    void attach(const SDDBootStrap& bs);

    // write the full index to the attached file
    void write();
    // write nEntries entries starting with firstEntry
    void write(uInt firstEntry, uInt nEntries = 1);

    // Return a reference to SDDIndexRep
    SDDIndexRep& operator()(uInt entry);
    const SDDIndexRep& operator()(uInt entry) const;

    // Return a reference to the Bootstrap
    const SDDBootStrap& bootstrap() const { return bootstrap_p;}
    SDDBootStrap& bootstrap() { return bootstrap_p;}

    // max entry in use, invalid if non in use
    Fallible<uInt> maxEntryInUse() const;
    // number of available entries
    uInt nrEntry() const;
    // check if an entry is in use (non-zero scan number)
    Bool inUse(uInt entry) const;

    // set everything in an entry to 0
    void clear(uInt entry);

    // switch version
    void setVersion(SDDBootStrap::SDD_VERSION newVersion);
    
private:
    enum ShortFields { OLD_FIRST = 0,
		       OLD_LAST  = 1,
		       OLD_MAGIC = 2,     // not used
		       OLD_POSCODE = 3,
		       OBSMODE = 28,
		       RECORD_PHASE = 29,
		       NEW_POSCODE = 30};

    enum IntFields { NEW_FIRST = 0,
		     NEW_LAST  = 1};

    enum FloatFields { HCOORD = 2,
		       VCOORD = 3,
		       SCAN_NUMBER = 8,
		       FRES_SLEWRATE = 9,
		       LST = 12,
		       UTDATE = 13};

    enum DoubleFields { RESTF_INTTIME = 5 };

    enum CharFields { SOURCE = 16};
		       
    Block<SDDIndexRep> *index_p;
    SDDBlock sddBlock;
    SDDBootStrap bootstrap_p;
    uInt currentRecord_p;
    Bool needsFlushed_p;

    void flush();
    void readEntry(SDDIndexRep& rep, uInt entry);
    void writeEntry(SDDIndexRep& rep, uInt entry);
    
    Char& valueRef(CharFields field, uInt entry, uInt position);
    const Char& valueRef(CharFields field, uInt entry, uInt position) const;
    Short& valueRef(ShortFields field, uInt entry);
    const Short& valueRef(ShortFields field, uInt entry) const;
    Int& valueRef(IntFields field, uInt entry);
    const Int& valueRef(IntFields field, uInt entry) const;
    float& valueRef(FloatFields field, uInt entry);
    const float& valueRef(FloatFields field, uInt entry) const;
    double& valueRef(DoubleFields field, uInt entry);
    const double& valueRef(DoubleFields field, uInt entry) const;

    
    
};

#endif

