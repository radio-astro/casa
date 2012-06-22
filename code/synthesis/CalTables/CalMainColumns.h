//# CalMainColumns.h: Calibration table cal_main column access
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

#ifndef CALIBRATION_CALMAINCOLUMNS_H
#define CALIBRATION_CALMAINCOLUMNS_H

#include <casa/aips.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDirection.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <measures/TableMeasures/TableMeasColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ScalarQuantColumn.h>
#include <synthesis/CalTables/CalTable.h>
#include <synthesis/MSVis/MSCalEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// ROCalMainColumns: Read-only cal_main calibration table column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "read-only", "calibration main table" and "columns".
// </etymology>
//
// <synopsis>
// The ROCalMainColumns class allows read-only access to columns
// in the main calibration table. Specializations for baseline-based,
// time-variable and solvable VisJones calibration table record types
// are provided through inheritance.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate access to calibration table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class ROCalMainColumns 
{
 public:
  // Construct from a calibration table
  ROCalMainColumns (const CalTable& calTable);

  // Default destructor
  virtual ~ROCalMainColumns() {};

  // Read-only column accessors
  const ROScalarColumn<Double>& time() const {return time_p;};
  const ROScalarMeasColumn<MEpoch>& timeMeas() const {return timeMeas_p;};
  const ROScalarColumn<Double>& timeEP() const {return timeEP_p;};
  const ROScalarQuantColumn<Double>& timeEPQuant() const 
    {return timeEPQuant_p;};
  const ROScalarColumn<Double>& interval() const {return interval_p;};
  const ROScalarQuantColumn<Double>& intervalQuant() const
    {return intervalQuant_p;};
  const ROScalarColumn<Int>& antenna1() const {return antenna1_p;};
  const ROScalarColumn<Int>& feed1() const {return feed1_p;};
  const ROScalarColumn<Int>& fieldId() const {return fieldId_p;};
  const ROScalarColumn<Int>& arrayId() const {return arrayId_p;};
  const ROScalarColumn<Int>& obsId() const {return obsId_p;};
  const ROScalarColumn<Int>& scanNo() const {return scanNo_p;};
  const ROScalarColumn<Int>& processorId() const {return processorId_p;};
  const ROScalarColumn<Int>& stateId() const {return stateId_p;};
  const ROScalarColumn<Int>& phaseId() const {return phaseId_p;};
  const ROScalarColumn<Int>& pulsarBin() const {return pulsarBin_p;};
  const ROScalarColumn<Int>& pulsarGateId() const {return pulsarGateId_p;};
  const ROScalarColumn<Int>& freqGrp() const {return freqGrp_p;};
  const ROScalarColumn<String>& freqGrpName() const {return freqGrpName_p;};
  const ROScalarColumn<String>& fieldName() const {return fieldName_p;};
  const ROScalarColumn<String>& fieldCode() const {return fieldCode_p;};
  const ROScalarColumn<String>& sourceName() const {return sourceName_p;};
  const ROScalarColumn<String>& sourceCode() const {return sourceCode_p;};
  const ROScalarColumn<Int>& calGrp() const {return calGrp_p;};
  const ROArrayColumn<Complex>& gain() const {return gain_p;};
  const ROArrayColumn<Int>& refAnt() const {return refAnt_p;};
  const ROArrayColumn<Int>& refFeed() const {return refFeed_p;};
  const ROArrayColumn<Int>& refReceptor() const {return refReceptor_p;};
  const ROArrayColumn<Double>& refFreq() const {return refFreq_p;};
  const ROArrayMeasColumn<MFrequency>& refFreqMeas() const 
    {return refFreqMeas_p;};
  const ROScalarColumn<Int>& measFreqRef() const {return measFreqRef_p;};
  const ROArrayColumn<Double>& refDir() const {return refDir_p;};
  const ROArrayMeasColumn<MDirection>& refDirMeas() const 
    {return refDirMeas_p;};
  const ROScalarColumn<Int>& measDirRef() const {return measDirRef_p;};
  const ROScalarColumn<Int>& calDescId() const {return calDescId_p;};
  const ROScalarColumn<Int>& calHistoryId() const {return calHistoryId_p;};

 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  ROCalMainColumns() {};

  // Return a CalTable as a Table reference. Utilizes friendship
  // relationship with class CalTable.
  const Table& asTable(const CalTable& calTable) 
    {return calTable.calMainAsTable();}

  // Attach a table column accessor
  void attach (const CalTable& calTable, ROTableColumn& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (const CalTable& calTable, 
	       ROArrayMeasColumn<MEpoch>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (const CalTable& calTable, 
	       ROArrayMeasColumn<MFrequency>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (const CalTable& calTable, 
	       ROArrayMeasColumn<MDirection>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (const CalTable& calTable, ROScalarMeasColumn<MEpoch>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (const CalTable& calTable, ROScalarQuantColumn<Double>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);

 private:
  // Prohibit copy constructor and assignment operator 
  ROCalMainColumns (const ROCalMainColumns&);
  ROCalMainColumns& operator= (const ROCalMainColumns&);

  // Private column accessors
  ROScalarColumn<Double> time_p;
  ROScalarMeasColumn<MEpoch> timeMeas_p;
  ROScalarColumn<Double> timeEP_p;
  ROScalarQuantColumn<Double> timeEPQuant_p;
  ROScalarColumn<Double> interval_p;
  ROScalarQuantColumn<Double> intervalQuant_p;
  ROScalarColumn<Int> antenna1_p;
  ROScalarColumn<Int> feed1_p;
  ROScalarColumn<Int> fieldId_p;
  ROScalarColumn<Int> arrayId_p;
  ROScalarColumn<Int> obsId_p;
  ROScalarColumn<Int> scanNo_p;
  ROScalarColumn<Int> processorId_p;
  ROScalarColumn<Int> stateId_p;
  ROScalarColumn<Int> phaseId_p;
  ROScalarColumn<Int> pulsarBin_p;
  ROScalarColumn<Int> pulsarGateId_p;
  ROScalarColumn<Int> freqGrp_p;
  ROScalarColumn<String> freqGrpName_p;
  ROScalarColumn<String> fieldName_p;
  ROScalarColumn<String> fieldCode_p;
  ROScalarColumn<String> sourceName_p;
  ROScalarColumn<String> sourceCode_p;
  ROScalarColumn<Int> calGrp_p;
  ROArrayColumn<Complex> gain_p;
  ROArrayColumn<Int> refAnt_p;
  ROArrayColumn<Int> refFeed_p;
  ROArrayColumn<Int> refReceptor_p;
  ROArrayColumn<Double> refFreq_p;
  ROArrayMeasColumn<MFrequency> refFreqMeas_p;
  ROScalarColumn<Int> measFreqRef_p;
  ROArrayColumn<Double> refDir_p;
  ROArrayMeasColumn<MDirection> refDirMeas_p;
  ROScalarColumn<Int> measDirRef_p;
  ROScalarColumn<Int> calDescId_p;
  ROScalarColumn<Int> calHistoryId_p;
};

// <summary> 
// CalMainColumns: Read-write cal_main calibration table column access
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration main table" and "columns".
// </etymology>
//
// <synopsis>
// The CalMainColumns class allows read-write access to columns
// in the main calibration table. Specializations for baseline-based,
// time-variable and solvable VisJones calibration table record types
// are provided through inheritance.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate access to calibration table columns.
// </motivation>
//
// <todo asof="01/07/01">
// (i) Deal with non-standard columns.
// </todo>

class CalMainColumns 
{
 public:
  // Construct from a calibration table
  CalMainColumns (CalTable& calTable);

  // Default destructor
  virtual ~CalMainColumns() {};

  // Read-write column accessors
  ScalarColumn<Double>& time() {return time_p;};
  ScalarMeasColumn<MEpoch>& timeMeas() {return timeMeas_p;};
  ScalarColumn<Double>& timeEP() {return timeEP_p;};
  ScalarQuantColumn<Double>& timeEPQuant() {return timeEPQuant_p;};
  ScalarColumn<Double>& interval() {return interval_p;};
  ScalarQuantColumn<Double>& intervalQuant() {return intervalQuant_p;};
  ScalarColumn<Int>& antenna1() {return antenna1_p;};
  ScalarColumn<Int>& feed1() {return feed1_p;};
  ScalarColumn<Int>& fieldId() {return fieldId_p;};
  ScalarColumn<Int>& arrayId() {return arrayId_p;};
  ScalarColumn<Int>& obsId() {return obsId_p;};
  ScalarColumn<Int>& scanNo() {return scanNo_p;};
  ScalarColumn<Int>& processorId() {return processorId_p;};
  ScalarColumn<Int>& stateId() {return stateId_p;};
  ScalarColumn<Int>& phaseId() {return phaseId_p;};
  ScalarColumn<Int>& pulsarBin() {return pulsarBin_p;};
  ScalarColumn<Int>& pulsarGateId() {return pulsarGateId_p;};
  ScalarColumn<Int>& freqGrp() {return freqGrp_p;};
  ScalarColumn<String>& freqGrpName() {return freqGrpName_p;};
  ScalarColumn<String>& fieldName() {return fieldName_p;};
  ScalarColumn<String>& fieldCode() {return fieldCode_p;};
  ScalarColumn<String>& sourceName() {return sourceName_p;};
  ScalarColumn<String>& sourceCode() {return sourceCode_p;};
  ScalarColumn<Int>& calGrp() {return calGrp_p;};
  ArrayColumn<Complex>& gain() {return gain_p;};
  ArrayColumn<Int>& refAnt() {return refAnt_p;};
  ArrayColumn<Int>& refFeed() {return refFeed_p;};
  ArrayColumn<Int>& refReceptor() {return refReceptor_p;};
  ArrayColumn<Double>& refFreq() {return refFreq_p;};
  ArrayMeasColumn<MFrequency>& refFreqMeas() {return refFreqMeas_p;};
  ScalarColumn<Int>& measFreqRef() {return measFreqRef_p;};
  ArrayColumn<Double>& refDir() {return refDir_p;};
  ArrayMeasColumn<MDirection>& refDirMeas() {return refDirMeas_p;};
  ScalarColumn<Int>& measDirRef() {return measDirRef_p;};
  ScalarColumn<Int>& calDescId() {return calDescId_p;};
  ScalarColumn<Int>& calHistoryId() {return calHistoryId_p;}; 
  
 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  CalMainColumns() {};

  // Return a CalTable as a Table reference. Utilizes friendship
  // relationship with class CalTable.
  Table& asTable(CalTable& calTable) {return calTable.calMainAsTable();}

  // Attach a table column accessor
  void attach (CalTable& calTable, TableColumn& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (CalTable& calTable, ArrayMeasColumn<MEpoch>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (CalTable& calTable, ArrayMeasColumn<MFrequency>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (CalTable& calTable, ArrayMeasColumn<MDirection>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (CalTable& calTable, ScalarMeasColumn<MEpoch>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);
  void attach (CalTable& calTable, ScalarQuantColumn<Double>& tabCol, 
	       MSCalEnums::colDef colEnum, const Bool& optional = False);

 private:
  // Prohibit copy constructor and assignment operator 
  CalMainColumns (const CalMainColumns&);
  CalMainColumns& operator= (const CalMainColumns&);

  // Private column accessors
  ScalarColumn<Double> time_p;
  ScalarMeasColumn<MEpoch> timeMeas_p;
  ScalarColumn<Double> timeEP_p;
  ScalarQuantColumn<Double> timeEPQuant_p;
  ScalarColumn<Double> interval_p;
  ScalarQuantColumn<Double> intervalQuant_p;
  ScalarColumn<Int> antenna1_p;
  ScalarColumn<Int> feed1_p;
  ScalarColumn<Int> fieldId_p;
  ScalarColumn<Int> arrayId_p;
  ScalarColumn<Int> obsId_p;
  ScalarColumn<Int> scanNo_p;
  ScalarColumn<Int> processorId_p;
  ScalarColumn<Int> stateId_p;
  ScalarColumn<Int> phaseId_p;
  ScalarColumn<Int> pulsarBin_p;
  ScalarColumn<Int> pulsarGateId_p;
  ScalarColumn<Int> freqGrp_p;
  ScalarColumn<String> freqGrpName_p;
  ScalarColumn<String> fieldName_p;
  ScalarColumn<String> fieldCode_p;
  ScalarColumn<String> sourceName_p;
  ScalarColumn<String> sourceCode_p;
  ScalarColumn<Int> calGrp_p;
  ArrayColumn<Complex> gain_p;
  ArrayColumn<Int> refAnt_p;
  ArrayColumn<Int> refFeed_p;
  ArrayColumn<Int> refReceptor_p;
  ArrayColumn<Double> refFreq_p;
  ArrayMeasColumn<MFrequency> refFreqMeas_p;
  ScalarColumn<Int> measFreqRef_p;
  ArrayColumn<Double> refDir_p;
  ArrayMeasColumn<MDirection> refDirMeas_p;
  ScalarColumn<Int> measDirRef_p;
  ScalarColumn<Int> calDescId_p;
  ScalarColumn<Int> calHistoryId_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



