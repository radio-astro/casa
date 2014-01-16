//# Partition.h: this defines Partition which creates an MS that is a subset of
//# an MS without any changes to the subtables.
//# 
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <ms/MeasurementSets/MSMainEnums.h>
//#include <synthesis/MSVis/VisIterator.h>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
//#include <casa/Utilities/CountedPtr.h>
#include <synthesis/MSVis/SubMS.h>

#ifndef MSVIS_PARTITION_H
namespace casa { //# NAMESPACE CASA - BEGIN

#define MSVIS_PARTITION_H

// <summary>
// Partition makes a subset of an existing MS without remapping any indices
// or resizing any subtables (unlike SubMS).
// </summary>

// <visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSet
//   <li> SubMS
// </prerequisite>
//
// <etymology>
// Partition an MS.
// </etymology>
//
// <synopsis>
// The order of operations (as in ms::partition()) is:
//      ctor
//      setmsselect
//      selectTime
//      makePartition
// </synopsis>

template<class T> class ROArrayColumn;
//Bool isAllColumns(const Vector<MS::PredefinedColumns>& colNames);

class Partition
{
public:
  Partition(String& theMS, Table::TableOption option = Table::Old);
  
  // construct from an MS
  Partition(MeasurementSet& ms);

  virtual ~Partition();
  
  // Change or Set the MS this MSSelector refers to.
  void setMS(MeasurementSet& ms);

  // Select spw and channels for each spw.
  // It returns true on success and false on failure.
  Bool selectSpw(const String& spwstr);

  //select Time and time averaging or regridding
  //void selectTime();

  //select stuff using msselection syntax ...time is left out
  // call it separately with timebin
  Bool setmsselect(const String& spw="", const String& field="", 
		   const String& baseline="", const String& scan="",
                   const String& uvrange="", const String& taql="", 
		   const String& subarray="", const String& intent="",
                   const String& obs="");

  // Select source or field
  Bool selectSource(const Vector<Int>& fieldid);
  
  // Select Antennas to split out  
  void selectAntenna(Vector<Int>& antennaids, Vector<String>& antennaSel)
  {
    antennaSel_p = SubMS::pickAntennas(antennaId_p, antennaSelStr_p,
				       antennaids, antennaSel);
  } 

  // Select array IDs to use.
  void selectArray(const String& subarray);

  //select time parameters
  void selectTime(Double timeBin=-1.0, String timerng="");

  //void selectSource(Vector<String> sourceid);

  //Method to set if a phase Center rotation is needed
  //void setPhaseCenter(Int fieldid, MDirection& newPhaseCenter);

  //Method to make the partition.
  //
  //TileShape of size 1 can have 2 values [0], and [1] ...these are used in to
  //determine the tileshape by using MSTileLayout. Otherwise it has to be a
  //vector size 3 e.g [4, 15, 351] => a tile shape of 4 stokes, 15 channels 351
  //rows.
  //
  // combine sets combine_p.  (Columns to ignore while time averaging.)
  //
  Bool makePartition(String& outname, String& whichDataCol,
		     const Vector<Int>& tileShape=Vector<Int>(1, 0),
		     const String& combine="");

  //Method to make a scratch partition and even in memory if posssible
  //Useful if temporary subselection/averaging is necessary
  // It'll be in memory if the basic output ms is less than half of 
  // memory reported by HostInfo unless forced to by user...
  virtual MeasurementSet* makeScratchPartition(const Vector<MS::PredefinedColumns>& whichDataCols, 
  				   const Bool forceInMemory=False);
  // In this form whichDataCol gets passed to parseColumnNames().
  virtual MeasurementSet* makeScratchPartition(const String& whichDataCol, 
  				   const Bool forceInMemory=False);

  // This sets up a default new ms
  // Declared static as it can be called directly outside of Partition.
  // Therefore it is not dependent on any member variable.
  static MeasurementSet* setupMS(const String& msname, const MeasurementSet& inms,
				 const Int nchan, const Int npol, const String& telescop,
                                 const Vector<MS::PredefinedColumns>& colNamesTok,
  				 const Int obstype=0);

  // Same as above except allowing manual tileshapes
  static MeasurementSet* setupMS(const String& msname, const MeasurementSet& inms,
				 const Int nchan, const Int npol,
                                 const Vector<MS::PredefinedColumns>& colNamesTok,
  				 const Vector<Int>& tileShape=Vector<Int>(1,0));
  
  void verifyColumns(const MeasurementSet& ms, const Vector<MS::PredefinedColumns>& colNames);
private:
  //method that returns the selected ms (?! - but it's Boolean - RR)
  Bool makeSelection();

  // (Sub)table fillers.
  Bool fillAllTables(const Vector<MS::PredefinedColumns>& colNames);
  Bool fillMainTable(const Vector<MS::PredefinedColumns>& colNames);

  //  Bool writeDiffSpwShape(const Vector<MS::PredefinedColumns>& colNames);
  Bool fillAccessoryMainCols();

  // *** Private member functions ***
  Bool getDataColumn(ROArrayColumn<Complex>& data,
                     const MS::PredefinedColumns colName);
  Bool getDataColumn(ROArrayColumn<Float>& data,
                     const MS::PredefinedColumns colName);
  Bool putDataColumn(MSColumns& msc, ROArrayColumn<Complex>& data,
                     const MS::PredefinedColumns datacol,
                     const Bool writeToDataCol=False);
  Bool putDataColumn(MSColumns& msc, ROArrayColumn<Float>& data,
                     const MS::PredefinedColumns datacol,
                     const Bool writeToDataCol=False);

  // This method uses VisIter for efficient copy mode data transfer
  Bool copyDataFlagsWtSp(const Vector<MS::PredefinedColumns>& colNames,
                         const Bool writeToDataCol);

  // Used in a couple of places to estimate how much memory to grab.
  Double n_bytes() {return mssel_p.nrow() * maxnchan_p * maxncorr_p *
                           sizeof(Complex);}

  // Read the input, time average it to timeBin_p, and write the output.
  Bool doTimeAver(const Vector<MS::PredefinedColumns>& dataColNames);

  // Fills mapper[ntok] with a map from dataColumn indices to ArrayColumns in
  // the output.  mapper must have ntok slots!
  static void getDataColMap(MSMainColumns* msc, ArrayColumn<Complex>* mapper,
			    uInt ntok,
			    const Vector<MS::PredefinedColumns>& colEnums);

  // *** Member variables ***

  // Initialized* by ctors.  (Maintain order both here and in ctors.)
  //  * not necessarily to anything useful.
  MeasurementSet ms_p, mssel_p;
  MSMainColumns * msc_p;		// columns of msOut_p
  ROMSColumns * mscIn_p;
  Bool   antennaSel_p;		// Selecting by antenna?
  Double timeBin_p;
  String scanString_p,          // Selects scans by #number#.  Historically named.
         intentString_p,        // Selects scans by string.  scanString_p was taken.
         obsString_p,           // String for observationID selection.
         uvrangeString_p, taqlString_p;
  String timeRange_p, arrayExpr_p, corrString_p;
  String combine_p;          // Should time averaging not split bins by
                             // scan #, observation, and/or state ID?
                             // Must be lowercase at all times.
  Int maxnchan_p,    // The maximum # of channels and correlations for each
      maxncorr_p;    // selected DDID.  (Int because NUM_CHAN and NUM_CORR
                     // are Int instead of uInt.)

  // Uninitialized by ctors.
  MeasurementSet msOut_p;
  Vector<Int> spw_p;      // Selected spw numbers
  Vector<Int> fieldid_p;
  Vector<String> antennaSelStr_p;
  Vector<Int> antennaId_p;
  Vector<Int> arrayId_p;
  Matrix<Double> selTimeRanges_p;
};

} //# NAMESPACE CASA - END

#endif

