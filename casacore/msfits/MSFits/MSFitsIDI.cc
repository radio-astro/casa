//# MSFitsIDI.cc: Implementation of MSFitsIDI.h
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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
//# $Id: 
//----------------------------------------------------------------------------

#include <msfits/MSFits/MSFitsIDI.h>
#include <msfits/MSFits/FitsIDItoMS.h>
#include <fits/FITS/fitsio.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Utilities/Regex.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/File.h>
#include <casa/OS/Directory.h>
#include <casa/IO/TapeIO.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSTileLayout.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/TiledColumnStMan.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <tables/Tables/SetupNewTab.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

MSFitsIDI::MSFitsIDI(const Path& tapeDevice, const String& msOut, 
		     const Bool& overWrite, const Int& obsType) :
  itsDataSource(""),
  itsDeviceType(FITS::Tape9),
  itsMSOut(""),
  itsMS(0),
  itsMSExists(False),
  itsOverWrite(False),
  itsSelectedFiles(0)
{
// Construct from a tape device and output MS file name
// Input:
//    tapeDevice           const String&      Tape device name
//    msOut                const String&      Output MS name
//    overWrite            const Bool&        True if existing MS is to 
//                                            be overwritten
// Output to private data:
//    itsDataSource        String             Tape name or input file name
//    itsDeviceType        FITS::DeviceType   FITS device type (disk or tape)
//    itsMSOut             String             Output MS name
//    itsMS                MeasurementSet*    Pointer to output MS
//    itsMSExists          Bool               True if output MS already exists
//    itsOverWrite         Bool               True if existing MS is to 
//                                            be overwritten
//    itsSelectedFiles     Vector<Int>        Input file numbers selected
//    itsAllFilesSelected  Bool               True if all files selected
//
  init(tapeDevice.absoluteName(), FITS::Tape9, msOut, overWrite, obsType);
//
};

//----------------------------------------------------------------------------

MSFitsIDI::MSFitsIDI(const String& inFile, const String& msOut, 
		     const Bool& overWrite, const Int& obsType) :
  itsDataSource(""),
  itsDeviceType(FITS::Disk),
  itsMSOut(""),
  itsMS(0),
  itsMSExists(False),
  itsOverWrite(False),
  itsSelectedFiles(0)
{
// Construct from an input FITS-IDI file name and an output MS file name
// Input:
//    inFile               const String&      Input FITS-IDI file name
//    msOut                const String&      Output MS name
//    overWrite            const Bool&        True if existing MS is to 
//                                            be overwritten
// Output to private data:
//    itsDataSource        String             Tape name or input file name
//    itsDeviceType        FITS::DeviceType   FITS device type (disk or tape)
//    itsMSOut             String             Output MS name
//    itsMS                MeasurementSet*    Pointer to output MS
//    itsMSExists          Bool               True if output MS already exists
//    itsOverWrite         Bool               True if existing MS is to 
//                                            be overwritten
//    itsSelectedFiles     Vector<Int>        Input file numbers selected
//    itsAllFilesSelected  Bool               True if all files selected
//
  init(inFile, FITS::Disk, msOut, overWrite, obsType);
//
};

//----------------------------------------------------------------------------

MSFitsIDI::~MSFitsIDI()
{
// Default desctructor
// Output to private data:
//    itsMS                MeasurementSet*    Pointer to output MS
//
  if (itsMS) {
    delete (itsMS);
  };
};

//----------------------------------------------------------------------------

void MSFitsIDI::selectFiles(const Vector<Int>& files)
{
// Select input tape files by number (1-relative)
// Input:
//    files                const Vector<Int>  List of selected file numbers
// Output to private data:
//    itsSelectedFiles     Vector<Int>        Input file numbers selected
//    itsAllFilesSelected  Bool               True if all files selected
//
  itsSelectedFiles.resize(files.nelements());
  itsSelectedFiles = files;
  if (itsSelectedFiles.nelements() > 0) {
    itsAllFilesSelected = False;
  };
};

//----------------------------------------------------------------------------

Bool MSFitsIDI::fillMS()
{
// Convert the FITS-IDI data to MS format
//
  LogIO os(LogOrigin("MSFitsIDI", "fillMS()", WHERE));
  
  // Delete the MS if it already exits and overwrite selected
  if (itsMSExists && itsOverWrite) {
    Table::deleteTable(itsMSOut);
  };

  // Create a new MS or attach to the existing MS
  if (!itsMSExists || itsOverWrite) {
    createOutputMS();
  } else {
    itsMS = new MeasurementSet(itsMSOut);
  };

  //
  // Tape input: loop over all selected input files
  //
  Bool atEnd = False;
  if (itsDeviceType == FITS::Tape9) {
    uInt fileIndex = 0;
    Int currentFile = 1;
    Int fileno = currentFile;

    while (!atEnd) {
      // Skip to next file selected
      if (itsAllFilesSelected) {
	fileno = currentFile;
      } else {
	atEnd = (fileIndex >= itsSelectedFiles.nelements()-1);
	if (!atEnd) fileno = itsSelectedFiles(fileIndex++);
      };

      if (!atEnd) {
	// Advance tape if necessary
	Int nskip = fileno - currentFile;
	if (nskip > 0) {
	  TapeIO tapeDev(itsDataSource);
	  tapeDev.skip(nskip);
	  currentFile = currentFile + nskip;
	};

	// Read and process the selected input file
	readFITSFile(atEnd);

	// Increment file counter
	currentFile = currentFile + 1;
      };
    }; 
      
    //
    // Disk input:
    //
  } else if (itsDeviceType == FITS::Disk) {
    readFITSFile(atEnd);
  };
  return True;
};

//----------------------------------------------------------------------------

void MSFitsIDI::init(const String& dataSource, 
		     const FITS::FitsDevice& deviceType, const String& msOut,
		     const Bool& overWrite, const Int& obsType) 
{
// Initialization (called by all constructors)
// Input:
//    dataSource    const String&            Input file name or tape device
//    deviceType    const FITS::FitsDevice   FITS device type (tape or disk)
//    msOut         const String&            Output MS name
//    overWrite     const Bool&              True if existing MS is to 
//                                           be overwritten
// Output to private data:
//    itsDataSource        String             Tape name or input file name
//    itsDeviceType        FITS::DeviceType   FITS device type (disk or tape)
//    itsMSOut             String             Output MS name
//    itsMS                MeasurementSet*    Pointer to output MS
//    itsMSExists          Bool               True if output MS already exists
//    itsOverWrite         Bool               True if existing MS is to 
//                                            be overwritten
//    itsSelectedFiles     Vector<Int>        Input file numbers selected
//    itsAllFilesSelected  Bool               True if all files selected
//
  LogIO os(LogOrigin("MSFitsIDI", "init()", WHERE));
  
  // Check for valid FITS-IDI data source
  Path sourcePath(dataSource);
  if (!sourcePath.isValid() || !File(sourcePath).exists() || 
      !File(sourcePath).isReadable()) {
    os << LogIO::SEVERE << "FITS-IDI data source is not readable"
       << LogIO::EXCEPTION;
  };

  itsDataSource = sourcePath.absoluteName();
  itsDeviceType = deviceType;

  // Check for valid output MS specification
  Path msPath(msOut);
  itsMSExists = File(msPath).exists();

  if (itsMSExists && !File(msPath).isWritable()) {
    os << LogIO::SEVERE << "Output MS is not writable" << LogIO::EXCEPTION;
  };

  if (!itsMSExists && !File(msPath).canCreate()) {
    os << LogIO::SEVERE << "Output MS cannot be created" << LogIO::EXCEPTION;
  };
  itsMSOut = msOut;
  itsOverWrite = overWrite;
  itsObsType = obsType;

  // Set remaining default parameters
  itsAllFilesSelected = True;
};

//----------------------------------------------------------------------------

void MSFitsIDI::readFITSFile(Bool& atEnd)
{
// Read and process the current FITS-IDI input file (on tape or disk)
// Output:
//    atEnd                Bool               True if at EOF
//
  LogIO os(LogOrigin("MSFitsIDI", "readFITSFile()", WHERE));
  atEnd = False;

  // Construct a FitsInput object
  FitsInput infits(itsDataSource.chars(), itsDeviceType);
  if (infits.err() != FitsIO::OK) {
    os << LogIO::SEVERE << "Error reading FITS input" << LogIO::EXCEPTION;
  };

  // Regular expression for trailing blanks
  Regex trailing(" *$");

  // Create a temporary work directory for the sub-tables
  Directory tmpDir(itsMSOut + "_tmp");
  tmpDir.create();

  // Vector of sub-table names
  Vector<String> subTableName;
  Int subTableNr = -1;
  Table maintab;
  
  // Loop over all HDU in the FITS-IDI file
  while (infits.err() == FitsIO::OK && !infits.eof()) {

    // Skip non-binary table HDU's
    if (infits.hdutype() != FITS::BinaryTableHDU) {
      os << LogIO::WARN << "Skipping non-binary table HDU" << LogIO::POST;
      infits.skip_hdu();

    } else if (infits.rectype() == FITS::SpecialRecord) {
      os << LogIO::WARN << "Skipping FITS special record" << LogIO::POST;
      infits.read_sp();

    } else {
      // Process the FITS-IDI input from the position of this binary table
      FITSIDItoMS1 bintab(infits);
      String hduName = bintab.extname();
      hduName = hduName.before(trailing);
      String tableName = itsMSOut;
      if (hduName != "") {
	if (hduName != "UV_DATA") {
	  tableName = tableName + "_tmp/" + hduName;
	  subTableNr++;
	  subTableName.resize(subTableNr+1, True);
	  subTableName(subTableNr) = hduName;
	};

	// Process the FITS-IDI input
	bintab.readFitsFile(tableName);
	if (infits.err() != FitsIO::OK) {
	  os << LogIO::SEVERE << "Error reading FITS input" 
	     << LogIO::EXCEPTION;
	};
      };
    };
  }; // end while

  // Move the subtables in the proper place and add the subtable
  // references to the main table description.
  //
  cout << "Subtables found: " << subTableName << endl;
  // Open the main table to be updated.
  Table msmain (itsMSOut, Table::Update);
  // Loop over all subtables.
  cout << "Nr of subtables = " << subTableNr+1 << endl;
  for (Int isub=0; isub<=subTableNr; isub++) {
    //cout << "renaming subtable " << subTableName(isub) << endl;
    // Open the subtable to be updated.
    if (subTableName(isub)=="ARRAY_GEOMETRY") {
      Table mssub(itsMSOut+"_tmp/"+subTableName(isub)+"/ANTENNA",Table::Update);
      // Rename the subtable.
      mssub.rename (itsMSOut+"/ANTENNA",Table::Update);
      // Attach the subtable to the main table.
      msmain.rwKeywordSet().defineTable("ANTENNA",mssub);
    }
    if (subTableName(isub)=="SOURCE") {
      Table mssub(itsMSOut+"_tmp/"+subTableName(isub)+"/FIELD",Table::Update);
      mssub.rename (itsMSOut+"/FIELD",Table::Update);
      msmain.rwKeywordSet().defineTable("FIELD",mssub);
    }
    if (subTableName(isub)=="FREQUENCY") {
      Table mssub(itsMSOut+"_tmp/"+subTableName(isub)+"/SPECTRAL_WINDOW",Table::Update);
      mssub.rename (itsMSOut+"/SPECTRAL_WINDOW",Table::Update);
      msmain.rwKeywordSet().defineTable("SPECTRAL_WINDOW",mssub);
      
      Table mssub2(itsMSOut+"_tmp/"+subTableName(isub)+"/DATA_DESCRIPTION",Table::Update);
      mssub2.rename (itsMSOut+"/DATA_DESCRIPTION",Table::Update);
      msmain.rwKeywordSet().defineTable("DATA_DESCRIPTION",mssub2);

      Table mssub3(itsMSOut+"_tmp/"+subTableName(isub)+"/POLARIZATION",Table::Update);
      mssub3.rename (itsMSOut+"/POLARIZATION",Table::Update);
      msmain.rwKeywordSet().defineTable("POLARIZATION",mssub3);
      
    }
    if (subTableName(isub)=="ANTENNA") {
      Table mssub(itsMSOut+"_tmp/"+subTableName(isub)+"/FEED",Table::Update);
      mssub.rename (itsMSOut+"/FEED",Table::Update);
      msmain.rwKeywordSet().defineTable("FEED",mssub);
    }
    if (subTableName(isub)=="POINTING_DATA") {
      Table mssub(itsMSOut+"_tmp/"+subTableName(isub)+"/POINTING",Table::Update);
      mssub.rename (itsMSOut+"/POINTING",Table::Update);
      msmain.rwKeywordSet().defineTable("POINTING",mssub);
    }
    
  }
  //tmpDir.remove();
  //commentwas here
};
  
//----------------------------------------------------------------------------

void MSFitsIDI::createOutputMS()
{
// Create a new, empty output MS
//
  LogIO os(LogOrigin("MSFitsIDI", "createOutputMS()", WHERE));

  // determine MS parameters here
  
  Int nCorr = 1; // dummy for the moment
  Int nchan = 1; // dto.
  String telescop = "EVN"; // dto.

  // set up new MS

  IPosition dataShape(2, nCorr, nchan);
  IPosition tshape = MSTileLayout::tileShape(dataShape, itsObsType, telescop);

  if(tshape.nelements() != 3)
    throw(AipsError("TileShape has to have 3 elememts ") );
  
  // Choose an appropriate tileshape
  //IPosition dataShape(2,nCorr,nchan);
  //IPosition tileShape = MSTileLayout::tileShape(dataShape,obsType, telescop);
  //////////////////
  
  IPosition tileShape(tshape);
  
  // Make the MS table
  TableDesc td = MS::requiredTableDesc();
  
  Vector<String> tiledDataNames;
  
  // Even though we know the data is going to be the same shape throughout I'll
  // still create a column that has a variable shape as this will permit MS's
  // with other shapes to be appended.
  MS::addColumnToDesc(td, MS::DATA, 2);
  String hcolName=String("Tiled")+String("DATA");
  td.defineHypercolumn(hcolName, 3,
		       stringToVector("DATA"));
  tiledDataNames.resize(1);
  tiledDataNames[0] = hcolName;
  
  // add this optional column because there is a 
  // weight per visibility
  MS::addColumnToDesc(td, MS::WEIGHT_SPECTRUM, 2);
  
  //     td.defineHypercolumn("TiledDATA",3,
  //                           stringToVector(MS::columnName(MS::DATA)));
  td.defineHypercolumn("TiledFlag",3,
		       stringToVector(MS::columnName(MS::FLAG)));
  td.defineHypercolumn("TiledFlagCategory",4,
		       stringToVector(MS::columnName(MS::FLAG_CATEGORY)));
  td.defineHypercolumn("TiledWgtSpectrum",3,
		       stringToVector(MS::columnName(MS::WEIGHT_SPECTRUM)));
  td.defineHypercolumn("TiledUVW",2,
		       stringToVector(MS::columnName(MS::UVW)));
  td.defineHypercolumn("TiledWgt",2,
		       stringToVector(MS::columnName(MS::WEIGHT)));
  td.defineHypercolumn("TiledSigma", 2,
		       stringToVector(MS::columnName(MS::SIGMA)));
  
  
  SetupNewTable newtab(itsMSOut, td, Table::New);
    
  uInt cache_val=32768;
  // Set the default Storage Manager to be the Incr one
  IncrementalStMan incrStMan ("ISMData",cache_val);
  newtab.bindAll(incrStMan, True);
  //Override the binding for specific columns
  
  IncrementalStMan incrStMan0("Array_ID",cache_val);
  newtab.bindColumn(MS::columnName(MS::ARRAY_ID), incrStMan0);
  IncrementalStMan incrStMan1("EXPOSURE",cache_val);
  newtab.bindColumn(MS::columnName(MS::EXPOSURE), incrStMan1);
  IncrementalStMan incrStMan2("FEED1",cache_val);
  newtab.bindColumn(MS::columnName(MS::FEED1), incrStMan2);
  IncrementalStMan incrStMan3("FEED2",cache_val);
  newtab.bindColumn(MS::columnName(MS::FEED2), incrStMan3);
  IncrementalStMan incrStMan4("FIELD_ID",cache_val);
  newtab.bindColumn(MS::columnName(MS::FIELD_ID), incrStMan4);
  IncrementalStMan incrStMan5("FLAG_ROW",cache_val/4);
  newtab.bindColumn(MS::columnName(MS::FLAG_ROW), incrStMan5);
  IncrementalStMan incrStMan6("INTERVAL",cache_val);
  newtab.bindColumn(MS::columnName(MS::INTERVAL), incrStMan6);
  IncrementalStMan incrStMan7("OBSERVATION_ID",cache_val);
  newtab.bindColumn(MS::columnName(MS::OBSERVATION_ID), incrStMan7);
  IncrementalStMan incrStMan8("PROCESSOR_ID",cache_val);
  newtab.bindColumn(MS::columnName(MS::PROCESSOR_ID), incrStMan8);
  IncrementalStMan incrStMan9("SCAN_NUMBER",cache_val);
  newtab.bindColumn(MS::columnName(MS::SCAN_NUMBER), incrStMan9);
  IncrementalStMan incrStMan10("STATE_ID",cache_val);
  newtab.bindColumn(MS::columnName(MS::STATE_ID), incrStMan10);
  IncrementalStMan incrStMan11("TIME",cache_val);
  newtab.bindColumn(MS::columnName(MS::TIME), incrStMan11);
  IncrementalStMan incrStMan12("TIME_CENTROID",cache_val);
  newtab.bindColumn(MS::columnName(MS::TIME_CENTROID), incrStMan12);
  
  // Bind ANTENNA1, ANTENNA2 and DATA_DESC_ID to the standardStMan 
  // as they may change sufficiently frequently to make the
  // incremental storage manager inefficient for these columns.
  
  
  StandardStMan aipsStMan0("ANTENNA1", cache_val);
  newtab.bindColumn(MS::columnName(MS::ANTENNA1), aipsStMan0);
  StandardStMan aipsStMan1("ANTENNA2", cache_val);
  newtab.bindColumn(MS::columnName(MS::ANTENNA2), aipsStMan1);
  StandardStMan aipsStMan2("DATA_DESC_ID", cache_val);
  newtab.bindColumn(MS::columnName(MS::DATA_DESC_ID), aipsStMan2);
  
  
  TiledShapeStMan tiledStMan1f("TiledFlag",tileShape);
  TiledShapeStMan tiledStMan1fc("TiledFlagCategory",
				IPosition(4,tileShape(0),tileShape(1),1,
					  tileShape(2)));
  TiledShapeStMan tiledStMan2("TiledWgtSpectrum",tileShape);
  TiledColumnStMan tiledStMan3("TiledUVW",IPosition(2,3,1024));
  TiledShapeStMan tiledStMan4("TiledWgt", 
			      IPosition(2,tileShape(0),tileShape(2)));
  TiledShapeStMan tiledStMan5("TiledSigma", 
			      IPosition(2,tileShape(0),tileShape(2)));
  
  // Bind the DATA, FLAG & WEIGHT_SPECTRUM columns to the tiled stman
    
  TiledShapeStMan tiledStMan1Data("TiledDATA",tileShape);
    
  newtab.bindColumn(MS::columnName(MS::DATA), tiledStMan1Data);

  newtab.bindColumn(MS::columnName(MS::FLAG),tiledStMan1f);
  newtab.bindColumn(MS::columnName(MS::FLAG_CATEGORY),tiledStMan1fc);
  newtab.bindColumn(MS::columnName(MS::WEIGHT_SPECTRUM),tiledStMan2);
  
  newtab.bindColumn(MS::columnName(MS::UVW),tiledStMan3);
  newtab.bindColumn(MS::columnName(MS::WEIGHT),tiledStMan4);
  newtab.bindColumn(MS::columnName(MS::SIGMA),tiledStMan5);
  
   // avoid lock overheads by locking the table permanently
  TableLock lock(TableLock::AutoLocking);
  itsMS = new MeasurementSet (newtab,lock);
  

// Subtables are created later. Hence this is commented out ...
//   // Set up the subtables for the FITS-IDI MS
//   // we make new tables with 0 rows
//   Table::TableOption option=Table::New;
//   ms->createDefaultSubtables(option); 
//   // add the optional Source sub table to allow for 
//   // specification of the rest frequency
//   TableDesc sourceTD=MSSource::requiredTableDesc();
//   SetupNewTable sourceSetup(ms->sourceTableName(),sourceTD,option);
//   ms->rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
// 				 Table(sourceSetup,0));
//   // update the references to the subtable keywords
//   ms->initRefs();
  
  { // Set the TableInfo
    TableInfo& info(itsMS->tableInfo());
    info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
    info.setSubType(String("FITS-IDI"));
    info.readmeAddLine
      ("This is a measurement set Table holding astronomical observations");
  }
    
  return;

};

//----------------------------------------------------------------------------



} //# NAMESPACE CASA - END

