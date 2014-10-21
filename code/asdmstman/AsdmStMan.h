//# AsdmStMan.h: Storage Manager for the main table of a raw ASDM MS
//# Copyright (C) 2012
//# Associated Universities, Inc. Washington DC, USA.
//# (c) European Southern Observatory, 2012
//# Copyright by ESO (in the framework of the ALMA collaboration)
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
//# $Id: AsdmStMan.h 18108 2011-05-27 07:52:39Z broekema $

#ifndef ASDM_ASDMSTMAN_H
#define ASDM_ASDMSTMAN_H

//# Includes
#include <asdmstman/AsdmIndex.h>
#include <tables/Tables/DataManager.h>
#include <casa/IO/LargeFiledesIO.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>

namespace casa {

//# Forward Declarations.
class AsdmColumn;

// <summary>
// The Storage Manager for the main table of a raw ASDM MS
// </summary>

// <use visibility=export>

// <reviewed reviewer="UNKNOWN" date="before2004/08/25" tests="tAsdmStMan.cc">
// </reviewed>

// <prerequisite>
//# Classes you should understand before using this one.
//   <li> The Table Data Managers concept as described in module file
//        <linkto module="Tables:Data Managers">Tables.h</linkto>
// </prerequisite>

// <etymology>
// AsdmStMan is the data manager which stores the data for a ASDM MS.
// </etymology>

// <synopsis>
// AsdmStMan is a specific storage manager for the main table of a ASDM MS.
// For performance purposes the raw data from the correlator is directly
// written to a disk file. However, to be able to use the data directly as a
// MeasurementSet, this specific storage manager is created offering access to
// all mandatory columns in the main table of the MS.
//
// Similar to other storage managers, the AsdmStMan files need to be part of
// the table directory. There are two files:
// <ul>
//  <li> The meta file contains the meta data describing baselines, start time,
//       integration time, etc. It needs to be written as an AipsIO file.
//       The meta info should also tell the endianness of the data file.
//  <li> The data file consists of NSEQ data blocks each containing:
//   <ul>
//    <li> 4-byte sequence number defining the time stamp.
//    <li> Complex data with shape [npol,nchan,nbasel].
//    <li> Unsigned short nr of samples used in each data point. It has shape
//         [nchan,nbasel]. It defines WEIGHT_SPECTRUM and FLAG.
//    <li> Filler bytes to align the blocks as given in the meta info.
//   </ul>
//   The sequence numbers are ascending, but there can be holes due to
//   missing time stamps.
// </ul>
// The first versions of the data file can only handle regularly shaped data
// with equal integration times. A future version might be able to deal with
// varying integration times (depending on baseline length).
//
// Most of the MS columns (like DATA_DESC_ID) are not stored in the data file;
// usually they map to the value 0. This is also true for the UVW column, so
// the UVW coordinates need to be added to the table in a separate step because
// the online system does not have the resources to do it.
//
// All columns are readonly with the exception of DATA.
// </synopsis>

// <motivation>
// The common Table storage managers are too slow for the possibly high
// output rate of the ASDM correlator.
// </motivation>

// <example>
// The following example shows how to create a table and how to attach
// the storage manager to some columns.
// <srcblock>
//   SetupNewTable newtab("name.data", tableDesc, Table::New);
//   AsdmStMan stman;                     // define storage manager
//   newtab.bindColumn ("DATA", stman);    // bind column to st.man.
//   newtab.bindColumn ("FLAG", stman);    // bind column to st.man.
//   Table tab(newtab);                    // actually create table
// </srcblock>
// </example>

//# <todo asof="$DATE:$">
//# A List of bugs, limitations, extensions or planned refinements.
//# </todo>


class AsdmStMan : public DataManager
{
public:
    // Create a Asdm storage manager with the given name.
    // If no name is used, it is set to "AsdmStMan"
  explicit AsdmStMan (const String& dataManagerName = "AsdmStMan");

  // Create a Asdm storage manager with the given name.
  // The specifications are part of the record (as created by dataManagerSpec).
  AsdmStMan (const String& dataManagerName, const Record& spec);
  
  ~AsdmStMan();

  // Clone this object.
  virtual DataManager* clone() const;
  
  // Get the type name of the data manager (i.e. AsdmStMan).
  virtual String dataManagerType() const;
  
  // Get the name given to the storage manager (in the constructor).
  virtual String dataManagerName() const;
  
  // Record a record containing data manager specifications.
  virtual Record dataManagerSpec() const;

  // Is this a regular storage manager?
  // It is regular if it allows addition of rows and writing dara in them.
  // <br>We need to return False here.
  virtual Bool isRegular() const;

  // The storage manager can add rows, but does nothing.
  virtual Bool canAddRow() const;
  
  // The storage manager cannot delete rows.
  virtual Bool canRemoveRow() const;
  
  // The storage manager can add columns, which does not really do something.
  virtual Bool canAddColumn() const;
  
  // Columns can be removed, but it does not do anything at all.
  virtual Bool canRemoveColumn() const;
  
  // Make the object from the type name string.
  // This function gets registered in the DataManager "constructor" map.
  // The caller has to delete the object.
  static DataManager* makeObject (const String& aDataManType,
                                  const Record& spec);

  // Register the class name and the static makeObject "constructor".
  // This will make the engine known to the table system.
  static void registerClass();


  // Get the data shape.
  IPosition getShape (uInt rownr);

  // Get data.
  void getData (uInt rownr, Complex* buf);

  uInt getAsdmStManVersion() const
    { return itsVersion; }

  // access the references to the ASDM BDFs
  void getBDFNames(Block<String>& bDFNames);

  // overwrite the BDFNames (Block needs to have same size as original,
  // returns False otherwise)
  Bool setBDFNames(Block<String>& bDFNames);

  // overwrite the index with the information presently stored in the
  // data manager
  void writeIndex();

private:
  // Copy constructor cannot be used.
  AsdmStMan (const AsdmStMan& that);

  // Assignment cannot be used.
  AsdmStMan& operator= (const AsdmStMan& that);
  
  // Flush and optionally fsync the data.
  // It does nothing, and returns False.
  virtual Bool flush (AipsIO&, Bool doFsync);
  
  // Let the storage manager create files as needed for a new table.
  // This allows a column with an indirect array to create its file.
  virtual void create (uInt nrrow);
  
  // Open the storage manager file for an existing table.
  virtual void open (uInt nrrow, AipsIO&); //# should never be called

  // Prepare the columns (needed for UvwColumn).
  virtual void prepare();

  // Resync the storage manager with the new file contents.
  // It does nothing.
  virtual void resync (casa::uInt nrrow);

  // Reopen the storage manager files for read/write.
  // It does nothing.
  virtual void reopenRW();
  
  // The data manager will be deleted (because all its columns are
  // requested to be deleted).
  // So clean up the things needed (e.g. delete files).
  virtual void deleteManager();

  // Add rows to the storage manager.
  // It cannot do it, so it does nothing.
  // This function will be called, because this storage manager is not the
  // only one used in an ASDM MS.
  virtual void addRow (uInt nrrow);
  
  // Delete a row from all columns.
  // It cannot do it, so throws an exception.
  virtual void removeRow (uInt rowNr);
  
  // Do the final addition of a column.
  // It won't do anything.
  virtual void addColumn (DataManagerColumn*);
  
  // Remove a column from the data file.
  // It won't do anything.
  virtual void removeColumn (DataManagerColumn*);
  
  // Create a column in the storage manager on behalf of a table column.
  // The caller has to delete the newly created object.
  // <group>
  // Create a scalar column.
  virtual DataManagerColumn* makeScalarColumn (const String& aName,
					       int aDataType,
					       const String& aDataTypeID);
  // Create a direct array column.
  virtual DataManagerColumn* makeDirArrColumn (const String& aName,
					       int aDataType,
					       const String& aDataTypeID);
  // Create an indirect array column.
  virtual DataManagerColumn* makeIndArrColumn (const String& aName,
					       int aDataType,
					       const String& aDataTypeID);
  // </group>

  // Initialize by reading the index file and opening the BDFs.
  void init();

  // Close the currently open BDF file.
  void closeBDF();

  // Return the entry number in the index containing the row.
  uInt searchIndex (Int64 rownr);

  // Return the index block containing the row.
  // It sets itsIndexEntry to that block.
  const AsdmIndex& findIndex (Int64 rownr);

  // Get data from the buffer.
  // <group>
  void getShort (const AsdmIndex&, Complex* buf, uInt bl, uInt spw);
  void getInt   (const AsdmIndex&, Complex* buf, uInt bl, uInt spw);
  void getFloat (const AsdmIndex&, Complex* buf, uInt bl, uInt spw);
  void getAuto  (const AsdmIndex&, Complex* buf, uInt bl);
  // </group>


  // set transposeBLNum_v
  void setTransposeBLNum(uInt nBl);

  //# Declare member variables.
  // Name of data manager.
  String itsDataManName;
  // The column objects.
  vector<AsdmColumn*>    itsColumns;
  Block<String>          itsBDFNames;
  LargeFiledesIO*        itsBDF;
  int                    itsFD;
  int                    itsOpenBDF;
  Int64                  itsFileOffset;
  Bool   itsDoSwap;       //# True = byte-swapping is needed
  Record itsSpec;         //# Data manager properties
  uInt   itsVersion;      //# Version of AsdmStMan MeasurementSet
  //# Fields to keep track of last block accessed.
  Int64  itsStartRow;     //# First row of data block
  Int64  itsEndRow;       //# First row of next data block
  uInt   itsIndexEntry;   //# Index entry number of current data block
  vector<char>      itsData;
  vector<AsdmIndex> itsIndex;
  vector<Int64>     itsIndexRows;

  uInt              itsNBl;
  vector<uInt>      itsTransposeBLNum_v;
};


} //# end namespace

#endif

