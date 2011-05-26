//# GBTScanLogReader: GBTScanLogReader reads and iterates through a GBT ScanLog
//# Copyright (C) 1999,2000,2001,2002,2003
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

#ifndef NRAO_GBTSCANLOGREADER_H
#define NRAO_GBTSCANLOGREADER_H

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/RecordField.h>
#include <nrao/FITS/GBTStateTable.h>
#include <fits/FITS/FITSTable.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/File.h>
#include <casa/Quanta/MVTime.h>
#include <tables/Tables/Table.h>
#include <casa/BasicSL/String.h>

#include <nrao/FITS/GBTGOFile.h>
#include <nrao/FITS/GBTLO1File.h>

#include <casa/namespace.h>
//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class Record;
} //# NAMESPACE CASA - END


// <summary>
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
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
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>

class GBTScanLogReader
{
public:
    // the types of backends
    enum BACKENDS {ANY=0, DCR, HOLOGRAPHY, SP, ACS,
		   UNKNOWN, NUM_BACKENDS=UNKNOWN};

    // construct this using the indicated ScanLog FITS file
    GBTScanLogReader(const File& scanlog, Int minscan=-1,
		     Int maxscan=LONG_MAX);

    ~GBTScanLogReader();

    // inquire about the health of the underling table
    Bool isValid() {return scanLog_p.isValid();}

    // reopen using the given file.  If scanlog is the same
    // file then it will be repositioned at the same current
    // position (same scan) as it currently is if keepPosition
    // is True.  This assumes that nothing in the scanlog has 
    // changed up until that point.  If scanlog is a different
    // file or if keepPosition is False, then this resets 
    // everything to the state it would have been immediately
    // after construction using the given scanlog.
    Bool reopen(const File &scanlog, Bool keepPosition=True);

    // reopen using the same file
    Bool reopen(Bool keepPosition=True);

    // Toggle the emission of warning messages.  This object starts
    // off initially being silent (no warning messages)
    // regarding missing files or scans without backends.
    // This is desirable so that warnings aren't issued until
    // real data is seen to arrive and a fill is started.
    void warn(Bool dowarn = True) {dowarn_p = dowarn;}

    // move the pointer to the next group in the log.
    // When next has been called and the file is at the end, the
    // scan number will be < 0
    void next();

    // the current scan number, this will be negative if
    // the file is empty
    Int scan() const {return scan_p;}

    // the indicated DMJD
    const MVTime &dmjd() const {return dmjd_p;}

    // The timestamp from the file names (uses the backend file
    // file name, which should be consistent with the others, although
    // there is no consistency check here).
    const MVTime &timeStamp() const {return timeStamp_p;}

    // return the next time stamp
    const MVTime &nextTimeStamp() const { return nextTimeStamp_p;}

    //# update the pointer to re-read the same scan log
    //# not yet implemented
    //# void update();

    // are there more groups (T) or is this pointed at the end (F)
    Bool more() const {return nextScan_p >= 0;}

    // return the name of the DCR backend FITS file in this scan
    // This string has zero length if there is no DCR data in
    // this scan.  There should never be more than one DCR
    // file in a scan but if there is, a warning is issued and
    // the last one seen is used.
    const String &dcrfile() const {return itsDCRFile;}

    // return the name of the HOLO backend FITS file in this scan
    // This string has zero length if there is no HOLO data in
    // this scan.  There should never be more than one HOLO
    // file in a scan but if there is, a warning is issued and
    // the last one seen is used.
    const String &holofile() const {return itsHOLOFile;}

    // return the name of the SP backend FITS file in this scan
    // This string has zero length if there is no SP data in
    // this scan.  There should never be more than one SP
    // file in a scan but if there is, a warning is issued and
    // the last one seen is used.
    const String &spfile() const {return itsSPFile;}

    // return the names of the ACS backend FITS file in this scan
    // This vector has zero elements if there are no ACS files
    // in thie scan.
    const Vector<String> &acsfiles() const {return itsACSFiles;}

    // return the list of backend types in this group
    const Block<Int>& backends() const {return backends_p;}

    // The names of the current backend files
    const Block<String>& backendFiles() const {return backendFiles_p;}

    // The names of all of the current DAP files
    const Block<String>& dapFiles() const {return dapFiles_p;}

    // The name of the IF manager file - there should only be one in a scan.
    // When more than one is seen, the first seen is used and a warning
    // message is sent to the logger.  If no IF manager file is present in
    // the scan, this string will be empty.
    const String &ifManagerFile() const {return ifManagerFile_p;}

    // The GBTGOFile from the FITS file written by the user interface - there should
    // be only one in a scan.  When more than one is seen, the first is used
    // and a warning message is sent to the logger.  If no user FITS file is
    // present in the scan, this GBTGOFile will not be attached.
    const GBTGOFile &GOFile() const {if (!goFileSet_p) attachGO(); return goFile_p;}

    // The GBTLO1File for LO1A.  There should be only one in a scan.  When more
    // than one is seen, the first is used and a warning message is sent to the 
    // logger.  If no LO1A file is seen, then this GBTLO1File will not be attached.
    const GBTLO1File &LO1A() const {if (!lo1Aset_p) attachLO1A(); return itsLO1A_p;}

    // The GBTLO1File for LO1B.  There should be only one in a scan.  When more
    // than one is seen, the first is used and a warning message is sent to the 
    // logger.  If no LO1B is seen, then this GBTLO1File will not be attached.
    const GBTLO1File &LO1B() const {if (!lo1Bset_p) attachLO1B(); return itsLO1B_p;}

    // The name of the Antenna FITS file - there should be only one in a scan.
    // When more than one is seen, the first seen is used and a warning message
    // is sent to the logger.   If no Antenna file is present in the scan,
    // this string will be empty.
    const String &antennaFile() const {return antennaFile_p;}

    // The names of any RX_CAL_INFO files
    const Block<String> &rxCalInfoFiles() const {return rxCalFiles_p;}

    // The master state table as indicated by the state files in the backend
    // files.  The first state file found indicating where the master is
    // will be believed.  If no master is indicated, this table will be null.
    const GBTStateTable &masterState() const {if (!masterStateSet_p) attachMasterState(); return state_p;}

    // The name of the backend file from which the master state came.
    const String &masterStateBackend() const { if (!masterStateSet_p) attachMasterState(); return stateName_p;}

    // return the names of all of the files associated with this scan
    const Block<String> &allFiles() const {if (!lo1Bchecked_p) checkLO1Bname(); return allFiles_p;}

    // return the current minimum scan number
    Int minscan() const { return minScan_p;}

    // set the minimum scan number
    void setMinscan(Int minscan) { minScan_p = minscan; doScanCheck();}
    
    // return the current maximum scan number
    Int maxscan() const { return maxScan_p;}

    // set the maximum scan number
    void setMaxscan(Int maxscan)
    { maxScan_p = maxscan; if (maxScan_p<0) maxScan_p = LONG_MAX; doScanCheck();}

    // recheck and possible reset the current scan to ensure it lies between minscan
    // and maxscan.  Necessary because setting minscan and maxscan involves two calls.
    // This should be done after min and max scan have been set.
    void checkScanNumber() {while (scan()>=0 && (scan() < minscan() || scan() > maxscan())) next();}
    
    // translate a string into a backend type
    static GBTScanLogReader::BACKENDS type(const String &name);

    // translate a type into a name, all lower case
    static String name(const GBTScanLogReader::BACKENDS type);

    // return the primary keywords associated with the underlying FITSTable
    const TableRecord &primaryKeywords() { return scanLog_p.primaryKeywords();}

    // return the STARTING line for this scan
    const String &starting() { return startingLine_p;}

    // return the FINISHED line for this scan
    const String &finished() { return finishedLine_p;}
 
private:
    File scanLogFile_p;
    String rootdir_p;

    FITSTable scanLog_p;
    MVTime dmjd_p, nextDmjd_p, workingDmjd_p,
	timeStamp_p, nextTimeStamp_p;
    Int scan_p, nextScan_p, workingScan_p, minScan_p, maxScan_p;

    Block<Int> backends_p, nextBackends_p, workingBackends_p;
    Block<String> backendFiles_p, nextBackendFiles_p, workingBackendFiles_p,
	dapFiles_p, nextDapFiles_p, workingDapFiles_p;

    Block<String> rxCalFiles_p, nextRxCalFiles_p, workingRxCalFiles_p;
    
    String itsDCRFile, itsHOLOFile, itsSPFile, itsNextDCRFile, 
           itsNextHOLOFile, itsNextSPFile;

    Vector<String> itsACSFiles, itsNextACSFiles;

    mutable Block<String> allFiles_p;
    Block<String> nextAllFiles_p;

    // these counts apply only to the working* blocks.
    Int backendCount_p, dapCount_p, rxCalCount_p;

    Bool dowarn_p;

    // the rownr corresponding to the start of this scan
    // -1 if nothing started yet
    Int thisStartRow_p;
    // the rownr corresponding to the start of the next scan
    // -1 if nothing started yet
    Int nextStartRow_p;

    Record *currow_p;
    RecordFieldPtr<Double> dmjdField_p;
    RecordFieldPtr<Int> scanField_p;
    RecordFieldPtr<String> filePathField_p;
    RecordFieldPtr<String> dateObsField_p;

    String ifManagerFile_p, nextIFManagerFile_p,
	antennaFile_p, nextAntennaFile_p;

    String startingLine_p, nextStartingLine_p;
    String finishedLine_p, nextFinishedLine_p;

    mutable GBTStateTable state_p;
    mutable Bool masterStateSet_p;
    mutable String stateName_p;

    mutable GBTGOFile goFile_p;
    String goFileName_p, nextGoFileName_p;
    mutable Bool goFileSet_p;

    mutable GBTLO1File itsLO1A_p, itsLO1B_p;
    String lo1Aname_p, nextLO1Aname_p, nextLO1Bname_p;
    mutable Bool lo1Aset_p, lo1Bset_p, lo1Bchecked_p;
    mutable String lo1Bname_p;

    Int itsBaseVer, itsDeviceVer;

    // initialize things after scanLog_p exists
    Bool init(Int startrow=0);

    // this is the thing which actually fills out all of the information
    void digestScan();

    // set the state table given a file name
    void setStateTable(const String &fileName) const;

    // ensure that minScan_p <= maxScan_p
    void doScanCheck();

    // attach the GO file - only done as needed
    void attachGO() const;

    // attach the LO1A file - only done as needed
    void attachLO1A() const;

    // attach the LO1B file - only done as needed
    void attachLO1B() const;

    // check for the existance of an un-named LO1B file
    void checkLO1Bname() const;

    // attach master state table - only done as needed
    void attachMasterState() const;

    // unset and unavailable
    GBTScanLogReader();
    GBTScanLogReader(const GBTScanLogReader &);
    GBTScanLogReader &operator=(const GBTScanLogReader &);
};

#endif


