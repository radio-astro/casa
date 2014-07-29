//# GBTScanLogReader.cc:  GBTScanLogReader reads and iterates through a GBT ScanLog
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
//# $Id$

//# Includes

#include <casa/sstream.h>

#include <nrao/FITS/GBTScanLogReader.h>
#include <nrao/FITS/GBTFITSBase.h>
#include <nrao/GBTFillers/GBTDAPFillerBase.h>

#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <fits/FITS/FITSDateUtil.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicMath/Math.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>
#include <casa/OS/RegularFile.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRow.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>

GBTScanLogReader::GBTScanLogReader(const File& scanlog, Int minscan, Int maxscan)
  : scanLogFile_p(scanlog), rootdir_p(""), scanLog_p(scanlog.path().expandedName()),
    dmjd_p(0.0), nextDmjd_p(0.0), workingDmjd_p(0.0), timeStamp_p(1900, 1, 1.0),
    nextTimeStamp_p(1900, 1, 1.0), scan_p(-1), nextScan_p(-1), workingScan_p(-1),
    minScan_p(minscan), maxScan_p(maxscan),
    backends_p(1), nextBackends_p(1), workingBackends_p(1), 
    backendFiles_p(1), nextBackendFiles_p(1), workingBackendFiles_p(1), 
    dapFiles_p(1), nextDapFiles_p(1), workingDapFiles_p(1),
    rxCalFiles_p(1), nextRxCalFiles_p(1), workingRxCalFiles_p(1),
    itsDCRFile(""), itsHOLOFile(""), itsSPFile(""),
    itsNextDCRFile(""), itsNextHOLOFile(""), itsNextSPFile(""), 
    itsACSFiles(0), itsNextACSFiles(0),
    backendCount_p(0), dapCount_p(0), rxCalCount_p(0),
    dowarn_p(False), thisStartRow_p(-1),
    nextStartRow_p(-1), currow_p(0), ifManagerFile_p(""), nextIFManagerFile_p(""), 
    antennaFile_p(""), nextAntennaFile_p(""), masterStateSet_p(False),
    goFile_p(""), goFileName_p(""), nextGoFileName_p(""), goFileSet_p(False),
    lo1Aname_p(""), nextLO1Aname_p(""), nextLO1Bname_p(""), 
    lo1Aset_p(False), lo1Bset_p(False), lo1Bchecked_p(False), lo1Bname_p(""),
    itsBaseVer(0), itsDeviceVer(0)
{
    // determine versions
    itsBaseVer = itsDeviceVer = 0;
    if (scanLog_p.primaryKeywords().fieldNumber("FITSVER") >= 0) {
	String fitsver = scanLog_p.primaryKeywords().asString("FITSVER");
	String leading = fitsver.before(".");
	String trailing = fitsver.after(".");
	itsBaseVer = GBTFITSBase::stringToInt(leading);
	itsDeviceVer = GBTFITSBase::stringToInt(trailing);
    }

    doScanCheck();
    init();
}

GBTScanLogReader::~GBTScanLogReader()
{
  delete currow_p;
  currow_p = 0;
}

Bool GBTScanLogReader::reopen(Bool keepPosition)
{
  return reopen(scanLogFile_p, keepPosition);
}

Bool GBTScanLogReader::reopen(const File &scanlog, Bool keepPosition)
{
  Int startrow = 0;
  if (scanLogFile_p.path().baseName() != scanlog.path().baseName()) {
    keepPosition = False;
    startrow = 0;
  } else if (keepPosition == True) {
    // restart from this scans starting row number
    startrow = thisStartRow_p;
  }
  scanLogFile_p = scanlog;
  scanLog_p.reopen(scanLogFile_p.path().expandedName());

  workingScan_p = -1;
  nextDmjd_p = 0.0;

  if (startrow == 0) {
    thisStartRow_p = -1;
    nextStartRow_p = -1;
    nextScan_p = -1;
  }
  return init(startrow);
}

void GBTScanLogReader::next()
{
  if (more()) {
    // copy the next values to the current values and reset the
    // next values
    dmjd_p = nextDmjd_p;
    timeStamp_p = nextTimeStamp_p;
    scan_p = nextScan_p;

    backends_p = nextBackends_p;
    backendFiles_p = nextBackendFiles_p;
    dapFiles_p = nextDapFiles_p;
    rxCalFiles_p = nextRxCalFiles_p;
    allFiles_p = nextAllFiles_p;
    startingLine_p = nextStartingLine_p;
    finishedLine_p = nextFinishedLine_p;

    itsDCRFile = itsNextDCRFile;
    itsSPFile = itsNextSPFile;
    itsHOLOFile = itsNextHOLOFile;
    itsACSFiles.resize(itsNextACSFiles.nelements());
    itsACSFiles = itsNextACSFiles;

    thisStartRow_p = nextStartRow_p;

    ifManagerFile_p = nextIFManagerFile_p;

    goFileSet_p = False;
    goFileName_p = nextGoFileName_p;

    lo1Aset_p = False;
    lo1Aname_p = nextLO1Aname_p;
    lo1Bset_p = False;
    lo1Bchecked_p = False;
    lo1Bname_p = nextLO1Bname_p;

    antennaFile_p = nextAntennaFile_p;

    masterStateSet_p = False;

    // this will at least temporarily signal that there are no more scans
    nextScan_p = -1;
    
    digestScan();

    // final check - this takes care of the case where the next fields when next() were called
    // point to a scan not in the range - in that case, next will be called at least twice.
    checkScanNumber();
  } else {
      // the next is after the end - set the scan number to reflect that
      scan_p = -1;
  }
}

Bool GBTScanLogReader::init(Int startRow)
{
  Bool result = True;
  scan_p = -1;
  nextScan_p = -1;
  workingScan_p = -1;
  backendCount_p = 0;
  dapCount_p = 0;
  rxCalCount_p = 0;
  goFileSet_p = lo1Aset_p = lo1Bset_p = lo1Bchecked_p = masterStateSet_p = False;

  delete currow_p;
  currow_p = 0;

  rootdir_p = scanLogFile_p.path().dirName();

  MVTime::setFormat(MVTime::FITS);
  if (scanLog_p.isValid()) {
    currow_p = new Record(scanLog_p.description());
    AlwaysAssert(currow_p, AipsError);
  } else {
    currow_p = new Record;
  }
  AlwaysAssert(currow_p, AipsError);
  if (!scanLog_p.pastEnd()) {
    *currow_p = scanLog_p.currentRow();
    // sanity checks
    if (!(currow_p->fieldNumber("DMJD") >= 0 ||
	  currow_p->fieldNumber("DATE-OBS") >= 0) ||
	currow_p->fieldNumber("SCAN") <0  ||
	currow_p->fieldNumber("FILEPATH") < 0) {
      throw(AipsError("GBTScanLogReader: one or more of the DMJD, DATE-OBS, SCAN, or FILEPATH columns is missing"));
    }

    if (currow_p->fieldNumber("DATE-OBS") >= 0) {
	dateObsField_p.attachToRecord(*currow_p, "DATE-OBS");
    } else {
	dmjdField_p.attachToRecord(*currow_p, "DMJD");
    }
    scanField_p.attachToRecord(*currow_p, "SCAN");
    filePathField_p.attachToRecord(*currow_p, "FILEPATH");

    // move scanLog_p to the indicated startRow
    if (startRow != 0) {
      if (scanLog_p.isValid()) {
	scanLog_p.move(startRow);
      }
      // did the correct move take place
      result = scanLog_p.rownr() == startRow;
      if (result) *currow_p = scanLog_p.currentRow();
    }


    if (result) {
	nextScan_p = -1;
	digestScan();
	// that possibly put something into nextScan et al, use
	// next to move that to the scan_p et al and to see if there
	// might be more after that.
	next();
    }
  }
  return result;
}

void GBTScanLogReader::digestScan()
{
  // this should not be called unless nextScan_p == -1
  AlwaysAssert(nextScan_p == -1, AipsError);

  // unused arguments we need later, put here to get out of the way
  String device, manager, sampler;

  nextIFManagerFile_p = "";
  nextAntennaFile_p = "";
  nextGoFileName_p = "";
  nextLO1Aname_p = nextLO1Bname_p = "";

  nextStartingLine_p = nextFinishedLine_p = "";

  // advance until the scan number is in the desired range
  while (!scanLog_p.pastEnd() && 
	 (*scanField_p < minScan_p || *scanField_p > maxScan_p)) {
      scanLog_p.next();
      *currow_p = scanLog_p.currentRow();
  }

  // don't bother to do anything unless there's more to be done
  if (!scanLog_p.pastEnd()) {
    Bool finished=False;
    // I believe that this always restarts at the top of a scan
    // given the way reopen works
    nextStartRow_p = scanLog_p.rownr();
    if (workingScan_p < 0) {
	workingScan_p = *scanField_p;
	if (dateObsField_p.isAttached()) {
	    // convert from string - assume UTC
	    MVTime dateObsTime;
	    MEpoch::Types system;
	    FITSDateUtil::fromFITS(dateObsTime, system, 
				   *dateObsField_p, "UTC");
	    workingDmjd_p = dateObsTime.day();
	} else {
	    workingDmjd_p = *dmjdField_p;
	}
      // don't bother even doing anything with this scan if the
      // time hasn't advanced since the last one
      if (workingDmjd_p <= nextDmjd_p) {
	finished = True;
	// emit a log message
	if (dowarn_p) {
	  MVTime::setFormat(MVTime::FITS);
	  LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
	  os << LogIO::WARN << WHERE
	     << "The scan start time has not increased for scan " 
	     << workingScan_p << endl;
	  os << "  this time = " << workingDmjd_p 
	     << "  previous time = " << nextDmjd_p << endl;
	  os << "  This scan will be skipped." << scanLogFile_p.path().originalName()
	     << LogIO::POST;
	}
	workingScan_p = -1;
      }
    }

    String fileName = *filePathField_p;

    if (fileName.contains("STARTING")) {
	// we may already be done, advance to the next line and see
      scanLog_p.next();
      if (scanLog_p.pastEnd()) finished = True;
      *currow_p = scanLog_p.currentRow();
      fileName = *filePathField_p; 
    }

    if (fileName.contains("FINISHED") || fileName == "COMPLETE") {
      finished = True;
    } 

    // and now the file names
    uInt fileCount = 0;
    while (!finished) {
      // we have something to decipher
      File aFile(Path::addDirectory(fileName, rootdir_p));
      // it needs to exist, be a regular file and be readable
      // before we will even think about considering it
      if (!aFile.exists() || !aFile.isRegular() || !aFile.isReadable()) {
	// issue a warning and skip this file
	if (dowarn_p) {
	  MVTime::setFormat(MVTime::FITS);
	  LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
	  os << LogIO::WARN << WHERE
	     << "The file " << fileName
	     << " does not exist or has the wrong permissions."
	     << endl;
	  os << " referenced in scan " << workingScan_p 
	     << " at time " << workingDmjd_p << endl;
	  os << " This file will be ignored." << LogIO::POST;
	}
      } else {
	// append this to nextAllFiles_p
	if (fileCount >= nextAllFiles_p.nelements()) {
	    // double size or make it at least fileCount+1 large
	    Int nextSize = max(fileCount+1, nextAllFiles_p.nelements()*2);
	    nextAllFiles_p.resize(nextSize, True, True);
	}
	nextAllFiles_p[fileCount++] = aFile.path().absoluteName();

	// extract the directory name where this file is, the last in the path
	String dirName(Path(aFile.path().dirName()).baseName());
	// do we recognize this as a valid backend
	// eventually this will be to simply check that this is Data, but
	// we also want to check on backend types.

	BACKENDS thisType = type(dirName);
	if (thisType != UNKNOWN) {
	  // we have a new backend file
	  // resize if necessary
	  if (backendCount_p >= Int(workingBackendFiles_p.nelements())) {
	    workingBackendFiles_p.resize(workingBackendFiles_p.nelements()*2);
	    workingBackends_p.resize(workingBackends_p.nelements()*2);
	  }
	  workingBackendFiles_p[backendCount_p] = aFile.path().absoluteName();
	  workingBackends_p[backendCount_p] = thisType;
	  backendCount_p++;
	} else {
	  // Does it seem to be an IF file
	  if (dirName == "IF") {
	      if (nextIFManagerFile_p != "") {
		  // this scan already has an IF manager file, emit a warning
		  LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
		  os << LogIO::WARN << WHERE
		     << "Scan " << workingScan_p 
		     << "has more than one IF manager file" << endl;
		  os << "The first IF manager file will be used : " << nextIFManagerFile_p << endl;
		  os << "This IF manager file will be ignored : " << aFile.path().absoluteName() 
		     << LogIO::POST;
	      } else {
		  nextIFManagerFile_p = aFile.path().absoluteName();
	      }
	  } else if (dirName == "GO" || dirName == "Glish") {
	      // does it seem to be a GO file
	      // old versions called this directory "Glish"
	      if (nextGoFileName_p.length() > 0) {
		  // this scan already has a GO file, emit a warning
		  LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
		  os << LogIO::WARN << WHERE
		     << "Scan " << workingScan_p 
		     << "has more than one GO FITS file" << endl;
		  os << "The first GO file will be used : " 
		     << nextGoFileName_p << endl;
		  os << "This GO file will be ignored : " 
		     << aFile.path().absoluteName() 
		     << LogIO::POST;
	      } else {
		  nextGoFileName_p = aFile.path().absoluteName();
	      }
	  } else if (dirName == "Antenna") {
	      // does it seem to be an Antenna file
	      if (nextAntennaFile_p != "") {
		  // this scan already has an Antenna file, emit a warning
		  LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
		  os << LogIO::WARN << WHERE
		     << "Scan " << workingScan_p
		     << "has more than one Antenna FITS file" << endl;
		  os << "The first Antenna file will be used : " 
		     << nextAntennaFile_p << endl;
		  os << "This Antenna file will be ignored : " 
		     << aFile.path().absoluteName()
		     << LogIO::POST;
	      } else {
		  nextAntennaFile_p = aFile.path().absoluteName();
	      }
	  } else if (dirName == "LO1A" || dirName == "LO1") {
	      // simply ignore this in early versions
	      if (itsBaseVer > 0) {
		  // does it seem to be an LO1A file
		  if (nextLO1Aname_p.length() > 0) {
		      // this scan already has an LO1A file, emit a warning
		      LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
		      os << LogIO::WARN << WHERE
			 << "Scan " << workingScan_p
			 << "has more than one LO1A FITS file" << endl;
		      os << "The first LO1A file will be used : "
			 << nextLO1Aname_p << endl;
		      os << "This LO1A file will be ignored : " 
			 << aFile.path().absoluteName()
			 << LogIO::POST;
		  } else {
		      nextLO1Aname_p = aFile.path().absoluteName();
		  }
	      }
	  } else if (dirName == "LO1B" || dirName == "Testtone") {
	      // simply ignore this file in early versions
	      if (itsBaseVer > 0) {
		  // does it seem to be an LO1B file
		  if (nextLO1Bname_p.length() > 0) {
		      // this scan already has a LO1B file, emit a warning
		      LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
		      os << LogIO::WARN << WHERE
			 << "Scan " << workingScan_p
			 << "has more than one LO1B FITS file" << endl;
		      os << "The first LO1B file will be used : "
			 << nextLO1Bname_p << endl;
		      os << "This LO1B file will be ignored : " 
			 << aFile.path().absoluteName()
			 << LogIO::POST;
		  } else {
		      nextLO1Bname_p = aFile.path().absoluteName();
		  }
	      }
	  } else if (GBTDAPFillerBase::parseName(aFile.path().absoluteName(),
						 device, manager, sampler)) {
	      // appears to be a valid DAP file name
	      if (dapCount_p >= Int(workingDapFiles_p.nelements())) {
		  workingDapFiles_p.resize(workingDapFiles_p.nelements()*2);
	      }
	      workingDapFiles_p[dapCount_p] = aFile.path().absoluteName();
	      dapCount_p++;
	  } else {
	      // either its a Rcvr* file containing tcal info or its a new type
	      Path dirPath(aFile.path().dirName());
	      String thisDir = dirPath.baseName();
	      if (thisDir.matches(Regex("^Rcvr.*"))) {
		  if (rxCalCount_p >= Int(workingRxCalFiles_p.nelements())) {
		      workingRxCalFiles_p.
			  resize(workingRxCalFiles_p.nelements()*2);
		  }
		  workingRxCalFiles_p[rxCalCount_p] = 
		      aFile.path().absoluteName();
		  rxCalCount_p++;
	      } // unknown type - quietly ignore
	  }
	}
      }
      scanLog_p.next();
      if (scanLog_p.pastEnd()) finished = True;
      *currow_p = scanLog_p.currentRow();
      fileName = *filePathField_p; 
      if (fileName.contains("STARTING")) {
	  nextStartingLine_p = fileName;
	  // we may be done, advance to the next line and see
	  scanLog_p.next();
	  if (scanLog_p.pastEnd()) finished = True;
	  *currow_p = scanLog_p.currentRow();
	  fileName = *filePathField_p; 
      }

      if (fileName.contains("FINISHED") || fileName == "COMPLETE") {
	  nextFinishedLine_p = fileName;
	  finished = True;
      } 
    }
    // we seem to be finished, lets see how things look
    nextAllFiles_p.resize(fileCount,True,True);
    if (workingScan_p >= 0) {
      // appears to still be valid
      if (fileName.contains("FINISHED") || fileName == "COMPLETE") {
	// and it looks like the end
	if (backendCount_p > 0) {
	  // looks good, extract the file time stamp from the first
	  // backend file name, the file name is
	  // of the form YYYY_MM_DD_HH:MM:SS.fits, do this semi-blindly,
	  // convert the first two _ to -
	  // convert the third _ to T 
	  // and pass the result up through the SS to MVTime for conversion
	  Path ptmp(workingBackendFiles_p[0]);
	  String baseName(ptmp.baseName());
	  String timeStamp(baseName.at(0,4)+"-"+baseName.at(5,2)+"-" +
			   baseName.at(8,2)+"T"+baseName.at(11,8));
	  istringstream iostr(timeStamp.chars());
	  iostr >> nextTimeStamp_p;

	  // and move the other stuff over to the next slot
	  nextDmjd_p = workingDmjd_p;
	  nextScan_p = workingScan_p;
	  nextBackends_p = workingBackends_p;
	  nextBackendFiles_p = workingBackendFiles_p;
	  nextBackends_p.resize(backendCount_p, True);
	  nextBackendFiles_p.resize(backendCount_p, True);
	  nextDapFiles_p = workingDapFiles_p;
	  nextDapFiles_p.resize(dapCount_p, True);
	  nextRxCalFiles_p = workingRxCalFiles_p;
	  nextRxCalFiles_p.resize(rxCalCount_p, True);

	  // pick out specific file names and types - ACS files take
	  // two passes, first to count and second to assemble.
	  // Also watch for unexpected duplications in the other types.
	  Bool hasDCR, hasSP, hasHolo;
	  Int acsCount;
	  hasDCR=hasSP=hasHolo=False;
	  acsCount=0;
	  for (uInt i=0;i<nextBackends_p.nelements();i++) {
	      switch (nextBackends_p[i]) {
	      case DCR:
		  if (hasDCR) {
		      // duplicate, warn
		      LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
		      os << LogIO::WARN << WHERE
			 << "Scan " << nextScan_p
			 << "has more than one DCR FITS file" << endl;
		      os << "The first DCR file will be used : "
			 << itsNextDCRFile << endl;
		      os << "This DCR file will be ignored : " 
			 << nextBackends_p[i]
			 << LogIO::POST;
		  } else {
		      itsNextDCRFile = nextBackendFiles_p[i];
		      hasDCR = True;
		  }
		  break;
	      case HOLOGRAPHY:
		  if (hasHolo) {
		      // duplicate, warn
		      LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
		      os << LogIO::WARN << WHERE
			 << "Scan " << nextScan_p
			 << "has more than one Holography FITS file" << endl;
		      os << "The first Holography file will be used : "
			 << itsHOLOFile << endl;
		      os << "This Holography file will be ignored : " 
			 << nextBackends_p[i]
			 << LogIO::POST;
		  } else {
		      itsNextHOLOFile = nextBackendFiles_p[i];
		      hasHolo=True;
		  }
		  break;
	      case SP:
		  if (hasSP) {
		      // duplicate, warn
		      LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
		      os << LogIO::WARN << WHERE
			 << "Scan " << nextScan_p
			 << "has more than one SpectralProcessor FITS file" << endl;
		      os << "The first SpectralProcessor file will be used : "
			 << itsNextSPFile << endl;
		      os << "This SpectralProcessor file will be ignored : " 
			 << nextBackends_p[i]
			 << LogIO::POST;
		  } else {
		      itsNextSPFile = nextBackendFiles_p[i];
		      hasSP=True;
		  }
		  break;
	      case ACS:
		  acsCount++;
		  break;
	      default:
		  // unknown backend - do nothing here
		  break;		  
	      }
	  }
	  // second pass to get any acs files
	  itsNextACSFiles.resize(acsCount);
	  if (acsCount > 0) {
	      acsCount = 0;
	      for (uInt i=0;i<nextBackends_p.nelements();i++) {
		  if (nextBackends_p[i] == ACS) {
		      itsNextACSFiles[acsCount++] = nextBackendFiles_p[i];
		  }
	      }
	  }
	  if (!hasDCR) itsNextDCRFile = "";
	  if (!hasSP) itsNextSPFile = "";
	  if (!hasHolo) itsNextHOLOFile = "";
	} else {
	  // no valid backend files, emit a warning
	  if (dowarn_p) {
	    MVTime::setFormat(MVTime::FITS);
	    LogIO os(LogOrigin("GBTScanLogReader","digestScan()"));
	    os << LogIO::WARN << WHERE
	       << "Scan " << workingScan_p << " at time " << workingDmjd_p 
	       << " has no valid backend files" << endl;
	    os << "This scan will be skipped"
	       << LogIO::POST;
	  }
	  workingScan_p = -1;
	}
      } else {
	// this should ONLY be possible if we past the end of the file
	AlwaysAssert(scanLog_p.pastEnd(), AipsError);
	// if we're past the end of the file then this means that we've caught
	// this file at some intermediate state in its writing and we should
	// just return to wait for an update to arrive	
	return;
      }
    } 
    
    // some clean up here
    backendCount_p = 0;
    dapCount_p = 0;
    rxCalCount_p = 0;

    // advance to end or the next non "COMPLETE" line
    while (!scanLog_p.pastEnd() && !fileName.contains("FINISHED") && fileName != "COMPLETE") {
      scanLog_p.next();
      *currow_p = scanLog_p.currentRow();
      fileName = *filePathField_p;
    }

    if ((fileName.contains("FINISHED") || fileName == "COMPLETE") && !scanLog_p.pastEnd()) {
      scanLog_p.next();
      *currow_p = scanLog_p.currentRow();
      if (workingScan_p < 0) {
	// this was NOT a valid scan, try it again
	digestScan();
      } else {
	// this WAS a valid scan, clean up
	workingScan_p = -1;
      }
    }
  }
}

GBTScanLogReader::BACKENDS GBTScanLogReader::type(const String &name)
{

    String tmp(name);
    tmp.upcase();

    BACKENDS result = UNKNOWN;
    if (tmp == "ANY") {
	result = ANY;
    } else if (tmp == "DCR") {
	result = DCR;
    } else if (tmp == "SP" || tmp == "SPECTRALPROCESSOR") {
      result = SP;
    } else if (tmp == "HOLO" || tmp == "HOLOGRAPHY") {
	result = HOLOGRAPHY;
    } else if (tmp == "ACS" || tmp == "SPECTROMETER") {
	result = ACS;
    }
    return result;
}

String GBTScanLogReader::name(GBTScanLogReader::BACKENDS type)
{
    String result;
    switch (type) {
    case ANY:
	result = "ANY";
	break;
    case DCR:
	result = "DCR";
	break;
    case SP:
	result = "SP";
	break;
    case HOLOGRAPHY:
	result = "HOLOGRAPHY";
	break;
    case ACS:
	result = "ACS";
	break;
    default:
	result = "UNKNOWN";
	break;
    }
    return result;
}

void GBTScanLogReader::setStateTable(const String &fileName) const
{
    state_p.detach();
    // verify that fileName exists and is readable
    File thefile(fileName);
    if (thefile.exists() && thefile.isReadable()) {
	// don't open if the file is empty
	RegularFile newfile(fileName);
	if (newfile.size() != 0) {
	    // open the file and initialize the tables, it should
	    // have at least one table
	    Bool opening = True;
	    uInt whichHDU=0;
	    while(opening) {
		whichHDU++;
		FITSTable fitstab(fileName, whichHDU);
		if (fitstab.isValid()) {
		    if (fitstab.keywords().isDefined("EXTNAME")) {
			String extname(fitstab.keywords().asString("EXTNAME"));
			if (extname == "DATA") {
			    // stop here
			    opening = False;
			} else if (extname == "PHASE" || extname == "STATE") {
			    // this is what we are looking for
			    state_p.reattach(fitstab);
			}
		    }
		} else {
		    // there must have been a problem, give up
		    opening = False;
		}
	    }
	}
    }
}

void GBTScanLogReader::doScanCheck() {
    if (maxScan_p < 0) maxScan_p = INT_MAX;
    if (minScan_p > maxScan_p) {
	Int tmp = minScan_p;
	minScan_p = maxScan_p;
	maxScan_p = tmp;
    }
}

void GBTScanLogReader::attachGO() const {
    goFile_p.attach(goFileName_p);
    goFileSet_p = True;
}

void GBTScanLogReader::attachLO1A() const {
    itsLO1A_p.attach(lo1Aname_p);
    lo1Aset_p = True;
}

void GBTScanLogReader::attachLO1B() const {
    if (!lo1Bchecked_p) checkLO1Bname();
    itsLO1B_p.attach(lo1Bname_p);
    lo1Bset_p = True;
}

void GBTScanLogReader::checkLO1Bname() const
{
    if (lo1Bname_p.length() == 0) {
	// sometimes the LO1B file is present even when not in the
	// scan log - try replacing the "/LO1A/" with "/LO1B"
	String aname = lo1Aname_p;
	lo1Bname_p = 
	    aname.before("/LO1A/") + "/LO1B/" + aname.after("/LO1A/");
	if (File(lo1Bname_p).exists()) {
	    // add this to allFiles_p
	    allFiles_p.resize(allFiles_p.nelements()+1, True, True);
	    allFiles_p[allFiles_p.nelements()-1] = lo1Bname_p;
	} else {
	    lo1Bname_p = "";
	}
    }
    lo1Bchecked_p = True;
}

void GBTScanLogReader::attachMasterState() const
{
    // see if there is a master state in the backends
    uInt i = 0;
    Bool found = False;
    stateName_p = "";
    while (!found && i < backends_p.nelements()) {
	setStateTable(backendFiles_p[i]);
	if (state_p.isAttached()) {
	    String master = state_p.master();
	    uInt masterType = type(master);
	    // is this the masterType
	    if (Int(masterType) == backends_p[i]) {
		// yes, we're done
		found = True;
		stateName_p = backendFiles_p[i];
	    } else {
		// no, find it
		uInt lastI = i;
		for (i=0;i<backends_p.nelements();i++) {
		    if (backends_p[i] == Int(masterType)) {
			// we're done, still need to set the state table
			setStateTable(backendFiles_p[i]);
			if (state_p.isAttached()) {
			    found = True;
			    stateName_p = backendFiles_p[i];
			    break;
			}
		    }
		}
		// paranoid protection, just in case something was a lie somewhere
		// we might find the truth if we keep checking
		if (!found) i = lastI;
	    }
	}	  
	i++;
    }
    
    masterStateSet_p = True;
}
