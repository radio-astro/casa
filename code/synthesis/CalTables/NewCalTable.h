//# NewCalTable.h: Calibration table access and creation
//# Copyright (C) 2011
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef CALIBRATION_NEWCALTABLE_H
#define CALIBRATION_NEWCALTABLE_H

#include <casa/aips.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ExprNode.h>
#include <casa/Containers/Record.h>
#include <casa/OS/Path.h>
#include <casa/Utilities/Sort.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSHistory.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSHistoryColumns.h>
#include <synthesis/CalTables/CTDesc.h>
#include <synthesis/CalTables/CTMainRecord.h>
#include <synthesis/CalTables/CTMainColumns.h>
#include <synthesis/CalTables/VisCalEnum.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// NewCalTable: New Calibration table access and creation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTableDesc">CalTableDesc</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration" and "table".
// </etymology>
//
// <synopsis>
// The NewCalTable classes provide basic access to calibration tables. This 
// includes the creation of calibration tables, sorting and selection
// capabilities, and basic data access. Specializations for baseline-based,
// time-variable and solvable VisJones types, and sub-types, are provided 
// through inheritance.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// This class is used by other calibration table accessors and iterators.
// </motivation>
//
// <todo asof="11/06/10">
// </todo>


// Typedefs for subtable objects
typedef MSField CTField;
typedef MSObservation CTObservation;
typedef MSAntenna CTAntenna;
typedef MSSpectralWindow CTSpectralWindow;
typedef MSHistory CTHistory;
typedef MSFieldColumns CTFieldColumns;
typedef MSObservationColumns CTObservationColumns;
typedef MSAntennaColumns CTAntennaColumns;
typedef MSSpWindowColumns CTSpWindowColumns;
typedef MSHistoryColumns CTHistoryColumns;
typedef ROMSFieldColumns ROCTFieldColumns;
typedef ROMSObservationColumns ROCTObservationColumns;
typedef ROMSAntennaColumns ROCTAntennaColumns;
typedef ROMSSpWindowColumns ROCTSpWindowColumns;
typedef ROMSHistoryColumns ROCTHistoryColumns;

class NewCalTable : public Table
{
 public:
   // Default null constructor, and destructor
   NewCalTable();
   virtual ~NewCalTable();

   // Construct from a specified table name, calibration table descriptor 
   // and table access option. Used for creating new tables.
   NewCalTable (const String& tableName, CTDesc& ctableDesc,
		Table::TableOption access = Table::New, 
		Table::TableType ttype = Table::Plain);

   // Construct from setupNewTable 
   NewCalTable (SetupNewTable& newTab, uInt nrow = 0, Bool initialize = False);
   
   // Create an empty NewCalTable conveniently
   NewCalTable(String tableName,VisCalEnum::VCParType parType,
	       String typeName,String msName,Bool doSingleChan);

   // Construct from a specified table name, and access option. Used
   // for accessing existing tables.
   NewCalTable (const String& tableName, 
		Table::TableOption access = Table::Old, 
		Table::TableType ttype = Table::Memory);

   static NewCalTable createCT(const String& tableName, 
			       Table::TableOption access, 
			       Table::TableType ttype, 
			       Bool doBackCompat=True);

   static NewCalTable* createCTptr(const String& tableName, 
				   Table::TableOption access, 
				   Table::TableType ttype, 
				   Bool doBackCompat=True);



   // Construct from an existing table object
   NewCalTable (const Table& table);

   // Copy constructor
   NewCalTable (const NewCalTable& other);

   // Create a vanilla NewCalTable from shapes (for testing purposes)
   //  Default rTime is 2012/01/06/12:00:00
   NewCalTable(String tableName,String CorF,
	       Int nObs=1, Int nScanPerObs=1,Int nTimePerScan=1,
	       Int nAnt=1, Int nSpw=1, Vector<Int> nChan=Vector<Int>(1,1), 
	       Int nFld=1, 
	       Double rTime=0.0, Double tint=0.0,
	       Bool disk=False, Bool verbose=False);

   // Assignment operator
   NewCalTable& operator= (const NewCalTable& other);

   // Handle backward compatibility
   static Bool CTBackCompat(const String& caltable);

   // Set the TableInfo record
   void setTableInfo();

   // initialize reference to subtables
   void createSubTables();
   void attachSubTables();
   void clearSubtables();

   void copyMemCalSubtables(const NewCalTable& other);
   void copyMemCalSubtable(const Table& otherSubtable, Table& subTable);

   // Complex parameters?  (else Float)
   Bool isComplex();

   // Report the polarization basis (from header record)
   String polBasis();

   // Get a row from cal_main
   Record getRowMain (const Int& jrow);
    
   // Put a row to cal_main
   void putRowMain (const Int& jrow, CTMainRecord& tableRec);

   // Get rows of the data from the main table
   //   void fillRowsMain ();

   // Set Meta data info from a parent MS 
   void setMetaInfo (const String& msName);

   //validate input table desc. over required caltable desc
   static Bool conformant(const TableDesc& tabDesc);

   // save to disk
   void writeToDisk(const String& tableName); 

   CTObservation& observation() {return observation_p;}
   CTAntenna& antenna() {return antenna_p;}
   CTField& field() {return field_p;}
   CTSpectralWindow& spectralWindow() {return spectralWindow_p;}
   CTHistory& history() {return history_p;}
   const CTObservation& observation() const {return observation_p;}
   const CTAntenna& antenna() const {return antenna_p;}
   const CTField& field() const {return field_p;}
   const CTSpectralWindow& spectralWindow() const {return spectralWindow_p;}
   const CTHistory& history() const {return history_p;}

   // Provide RO access to 
   //   inline ROCTMainColumns mcols() const {return ROCTMainColumns(*this);};

   static Complex NCTtestvalueC(Int iant,Int ispw,Double ich,Double time,Double refTime,Double tint);
   static Float NCTtestvalueF(Int iant,Int ispw,Double ich,Double time,Double refTime,Double tint);

   // Fill in antenna-based solutions
   //  (obsId-less version calls obsId-capable version w/ obsId=0)
   void fillAntBasedMainRows(uInt nrows, 
			     Double time,Double interval,
			     Int fieldId,uInt spwId,Int scanNo,
			     const Vector<Int>& ant1list, Int refant,
			     const Cube<Complex>& cparam,
			     const Cube<Bool>& flag,
			     const Cube<Float>& paramErr,
			     const Cube<Float>& snr);
   void fillAntBasedMainRows(uInt nrows, 
			     Double time,Double interval,
			     Int fieldId,uInt spwId,Int obsId,Int scanNo,
			     const Vector<Int>& ant1list, Int refant,
			     const Cube<Complex>& cparam,
			     const Cube<Bool>& flag,
			     const Cube<Float>& paramErr,
			     const Cube<Float>& snr);

   // Reset spw freq info
   void setSpwFreqs(Int spw, const Vector<Double>& freq, 
		    const Vector<Double>& chanwidth=Vector<Double>());

   // Set FLAG_ROW in SPECTRAL_WINDOW subtable for spws absent in MAIN
   void flagAbsentSpws();

   // Merge SPW subtable rows from another NewCalTable
   void mergeSpwMetaInfo(const NewCalTable& other);

   // Add a line to the HISTORY table
   void addHistoryMessage(String app="",String message="none");

 private:

   // Services for generic test table ctor
   //  Default rTime is 2012/01/06/12:00:00
   void fillGenericContents(Int nObs=1, Int nScanPerObs=1, Int nTimePerScan=1,
			    Int nAnt=1, Int nSpw=1, Vector<Int> nChan=Vector<Int>(1,1), 
			    Int nFld=1, 
			    Double rTime=0.0, Double tint=0.0,
			    Bool verbose=False);
   void fillGenericObs(Int nObs);
   void fillGenericField(Int nFld);
   void fillGenericAntenna(Int nAnt);
   void fillGenericSpw(Int nSpw,Vector<Int>& nChan);
     
   // Force Spw subtable to be all nchan=1
   //  (very basic; uses chan n/2 freq)
   void makeSpwSingleChan();

   // Handle pre-4.1 caltables that don't have OBS_ID
   //  (by adding a phoney one with a single OBS_ID
   void addPhoneyObs();

   // The subtables
   CTObservation observation_p;
   CTAntenna antenna_p;
   CTField field_p;
   CTSpectralWindow spectralWindow_p;
   CTHistory history_p;

 };

} //# NAMESPACE CASA - END

#endif
