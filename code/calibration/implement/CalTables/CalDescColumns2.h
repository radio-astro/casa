//# CalDescColumns.h: Calibration table cal_desc column access
//# Copyright (C) 1996,1997,1998,2001,2002,2003
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

#ifndef CALIBRATION_CALDESCCOLUMNS2_H
#define CALIBRATION_CALDESCCOLUMNS2_H

#include <casa/aips.h>
#include <measures/Measures/MFrequency.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <measures/TableMeasures/TableMeasColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>
#include <calibration/CalTables/CalTable2.h>
#include <msvis/MSVis/MSCalEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// ROCalDescColumns: Read-only cal_desc calibration table column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "read-only", "calibration descriptor table" and "columns".
// </etymology>
//
// <synopsis>
// The ROCalDescColumns class allows read-only access to columns
// in the cal_desc calibration sub-table. 
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate read-only access to cal_desc calibration sub-table columns.
// </motivation>
//
// <todo asof="11/01/01">
// (i) Deal with non-standard columns.
// </todo>

class ROCalDescColumns2 
{
 public:
  // Construct from a cal_desc calibration sub-table
  ROCalDescColumns2 (const CalTable2& calTable);

  // Default destructor
  virtual ~ROCalDescColumns2() {};

  // Read-only column accessors
  const ROScalarColumn<Int>& numSpw() const {return numSpw_p;};
  const ROArrayColumn<Int>& numChan() const {return numChan_p;};
  const ROScalarColumn<Int>& numReceptors() const {return numReceptors_p;};
  const ROScalarColumn<Int>& nJones() const {return nJones_p;};
  const ROArrayColumn<Int>& spwId() const {return spwId_p;};
  const ROArrayColumn<Double>& chanFreq() const {return chanFreq_p;};
  const ROArrayMeasColumn<MFrequency>& chanFreqMeas() const 
    {return chanFreqMeas_p;};
  const ROScalarColumn<Int>& measFreqRef() const {return measFreqRef_p;};
  const ROArrayColumn<Double>& chanWidth() const {return chanWidth_p;}
  const ROArrayQuantColumn<Double>& chanWidthQuant() const {
    return chanWidthQuant_p;}
  const ROArrayColumn<Int>& chanRange() const {return chanRange_p;}
  const ROArrayColumn<String>& polznType() const {return polznType_p;}
  const ROScalarColumn<String>& jonesType() const {return jonesType_p;};
  const ROScalarColumn<String>& msName() const {return msName_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  ROCalDescColumns2() {};

  // Return a CalTable cal_desc subtable as a Table reference. 
  // Utilizes friendship relationship with class CalTable.
  const Table& calDescAsTable(const CalTable2& calTable) 
    {return calTable.calDescAsTable();}

  // Attach a table column accessor
  void attach (const CalTable2& calTable, ROTableColumn& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (const CalTable2& calTable, ROTableMeasColumn& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (const CalTable2& calTable, ROArrayQuantColumn<Double>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);

 private:
  // Prohibit copy constructor and assignment operator 
  ROCalDescColumns2 (const ROCalDescColumns2&);
  ROCalDescColumns2& operator= (const ROCalDescColumns2&);

  // Private column accessors
  ROScalarColumn<Int> numSpw_p;
  ROArrayColumn<Int> numChan_p;
  ROScalarColumn<Int> numReceptors_p;
  ROScalarColumn<Int> nJones_p;
  ROArrayColumn<Int> spwId_p;
  ROArrayColumn<Double> chanFreq_p;
  ROArrayMeasColumn<MFrequency> chanFreqMeas_p;
  ROScalarColumn<Int> measFreqRef_p;
  ROArrayColumn<Double> chanWidth_p;
  ROArrayQuantColumn<Double> chanWidthQuant_p;
  ROArrayColumn<Int> chanRange_p;
  ROArrayColumn<String> polznType_p;
  ROScalarColumn<String> jonesType_p;
  ROScalarColumn<String> msName_p;
};

// <summary> 
// CalDescColumns2: Read-write cal_desc calibration table column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration descriptor table" and "columns".
// </etymology>
//
// <synopsis>
// The CalDescColumns2 class allows read-write access to columns
// in the cal_desc calibration sub-table. 
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate access to cal_desc calibration sub-table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class CalDescColumns2 
{
 public:
  // Construct from a calibration table
  CalDescColumns2 (CalTable2& calTable);

  // Default destructor
  virtual ~CalDescColumns2() {};

  // Read-write column accessors
  ScalarColumn<Int>& numSpw() {return numSpw_p;};
  ArrayColumn<Int>& numChan() {return numChan_p;};
  ScalarColumn<Int>& numReceptors() {return numReceptors_p;};
  ScalarColumn<Int>& nJones() {return nJones_p;};
  ArrayColumn<Int>& spwId() {return spwId_p;};
  ArrayColumn<Double>& chanFreq() {return chanFreq_p;};
  ArrayMeasColumn<MFrequency>& chanFreqMeas() {return chanFreqMeas_p;};
  ScalarColumn<Int>& measFreqRef() {return measFreqRef_p;};
  ArrayColumn<Double>& chanWidth() {return chanWidth_p;}
  ArrayQuantColumn<Double>& chanWidthQuant() {return chanWidthQuant_p;}
  ArrayColumn<Int>& chanRange() {return chanRange_p;}
  ArrayColumn<String>& polznType() {return polznType_p;}
  ScalarColumn<String>& jonesType() {return jonesType_p;};
  ScalarColumn<String>& msName() {return msName_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  CalDescColumns2() {};

  // Return a CalTable cal_desc subtable as a Table reference. 
  // Utilizes friendship relationship with class CalTable.
  Table& calDescAsTable(CalTable2& calTable) {return calTable.calDescAsTable();}

  // Attach a table column accessor
  void attach (CalTable2& calTable, ROTableColumn& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (CalTable2& calTable, ROTableMeasColumn& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (CalTable2& calTable, ROArrayQuantColumn<Double>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);

 private:
  // Prohibit copy constructor and assignment operator 
  CalDescColumns2 (const CalDescColumns2&);
  CalDescColumns2& operator= (const CalDescColumns2&);

  // Private column accessors
  ScalarColumn<Int> numSpw_p;
  ArrayColumn<Int> numChan_p;
  ScalarColumn<Int> numReceptors_p;
  ScalarColumn<Int> nJones_p;
  ArrayColumn<Int> spwId_p;
  ArrayColumn<Double> chanFreq_p;
  ArrayMeasColumn<MFrequency> chanFreqMeas_p;
  ScalarColumn<Int> measFreqRef_p;
  ArrayColumn<Double> chanWidth_p;
  ArrayQuantColumn<Double> chanWidthQuant_p;
  ArrayColumn<Int> chanRange_p;
  ArrayColumn<String> polznType_p;
  ScalarColumn<String> jonesType_p;
  ScalarColumn<String> msName_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



