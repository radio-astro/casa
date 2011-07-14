//# RONewCalMainColumns.cc: Implementation of RONewCalMainColumns.h
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
//# $Id$
//----------------------------------------------------------------------------

#include <calibration/CalTables/RONewCalMainColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

RONewCalMainColumns::RONewCalMainColumns(const NewCalTable& calTable) 
{
// Construct from a calibration table
// Input:
//    calTable         const NewCalTable&                Calibration table
// Output to private data:
//    time_p           ROScalarColumn<Double>&        Time
//    timeMeas_p       ROScalarMeasColumn<MEpoch>&    Time as a Measure
//    timeEP_p         ROScalarColumn<Double>&        Extended precision time
//    timeEPQuant_p    ROScalarQuantColumn<Double>&   EP time as Quantum
//    interval_p       ROScalarColumn<Double>&        Time interval
//    intervalQuant_p  ROScalarQuantColumn<Double>&   Time interval as Quantum
//    antenna1_p       ROScalarColumn<Int>&           Antenna 1
//    antenna2_p       ROScalarColumn<Int>&           Antenna 2
//    fieldId_p        ROScalarColumn<Int>&           Field id.
//    spwId_p          ROScalarColumn<Int>&           Spectral Window id.
//    scanNo_p         ROScalarColumn<Int>&           Scan no.
//    param_p          ROArrayColumn<Float>&          Solution params
//    paramerr_p       ROArrayColumn<Float>&          Solution param error
//    flag_p           ROArrayColumn<Bool>&           flag
//    snr_p            ROArrayColumn<Float>&          Solution SNR
//    weight_p         ROArrayColumn<Float>&          Solution weight
//
  // Attach all the column accessors (including required and
  // optional columns)
  attach (calTable, time_p, NCT::TIME);
  attach (calTable, timeMeas_p, NCT::TIME);
  attach (calTable, timeEP_p, NCT::TIME_EXTRA_PREC, True);
  attach (calTable, timeEPQuant_p, NCT::TIME_EXTRA_PREC, True);
  attach (calTable, interval_p, NCT::INTERVAL);
  attach (calTable, intervalQuant_p, NCT::INTERVAL);
  attach (calTable, antenna1_p, NCT::ANTENNA1);
  attach (calTable, antenna2_p, NCT::ANTENNA2);
  attach (calTable, fieldId_p, NCT::FIELD_ID);
  attach (calTable, spwId_p, NCT::SPECTRAL_WINDOW_ID);
  attach (calTable, scanNo_p, NCT::SCAN_NUMBER);
  attach (calTable, param_p, NCT::PARAM);
  attach (calTable, paramerr_p, NCT::PARAMERR);
  attach (calTable, flag_p, NCT::FLAG);
  attach (calTable, snr_p, NCT::SNR);
  attach (calTable, weight_p, NCT::WEIGHT);
};

//----------------------------------------------------------------------------

void RONewCalMainColumns::attach (const NewCalTable& calTable, 
				   ROTableColumn& tabCol, 
				   NewCalTableEnums::colDef colEnum, 
				   const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         const NewCalTable&         Calibration table
//    tabCol           ROTableColumn&             Table column accessor
//    colEnum          NewCalTableEnums::colDef   Column enum
//    optional         const Bool&                True if optional column
// Output to private data:
//
  // Convert to column name
  String colName = NCT::fieldName (colEnum);

  // Check if an optional column
  if (optional) {
    if (calTable.tableDesc().isColumn(colName)) {
      tabCol.attach (calTable, colName);
    };
  } else {
    tabCol.attach (calTable, colName);
  };
};

//----------------------------------------------------------------------------

void RONewCalMainColumns::attach (const NewCalTable& calTable, 
				   ROArrayMeasColumn<MFrequency>& tabCol, 
				   NewCalTableEnums::colDef colEnum, 
				   const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         const NewCalTable&               Calibration table
//    tabCol           ROArrayMeasColumn<MFrequency>&   Table measures 
//                                                      column accessor
//    colEnum          NewCalTableEnums::colDef         Column enum
//    optional         const Bool&                      True if optional column
// Output to private data:
//
  // Convert to column name
  String colName = NCT::fieldName (colEnum);

  // Check if an optional column
  if (optional) {
    if (calTable.tableDesc().isColumn(colName)) {
      tabCol.attach (calTable, colName);
    };
  } else {
    tabCol.attach (calTable, colName);
  };
};

//----------------------------------------------------------------------------

void RONewCalMainColumns::attach (const NewCalTable& calTable, 
				   ROArrayMeasColumn<MEpoch>& tabCol, 
				   NewCalTableEnums::colDef colEnum, 
				   const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         const NewCalTable&               Calibration table
//    tabCol           ROArrayMeasColumn<MEpoch>&       Table measures 
//                                                      column accessor
//    colEnum          NewCalTableEnums::colDef         Column enum
//    optional         const Bool&                      True if optional column
// Output to private data:
//
  // Convert to column name
  String colName = NCT::fieldName (colEnum);

  // Check if an optional column
  if (optional) {
    if (calTable.tableDesc().isColumn(colName)) {
      tabCol.attach (calTable, colName);
    };
  } else {
    tabCol.attach (calTable, colName);
  };
};

//----------------------------------------------------------------------------

void RONewCalMainColumns::attach (const NewCalTable& calTable, 
				   ROArrayMeasColumn<MDirection>& tabCol, 
				   NewCalTableEnums::colDef colEnum, 
				   const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         const NewCalTable&               Calibration table
//    tabCol           ROArrayMeasColumn<MDirection>&   Table measures 
//                                                      column accessor
//    colEnum          NewCalTableEnums::colDef         Column enum
//    optional         const Bool&                      True if optional column
// Output to private data:
//
  // Convert to column name
  String colName = NCT::fieldName (colEnum);

  // Check if an optional column
  if (optional) {
    if (calTable.tableDesc().isColumn(colName)) {
      tabCol.attach (calTable, colName);
    };
  } else {
    tabCol.attach (calTable, colName);
  };
};

//----------------------------------------------------------------------------

void RONewCalMainColumns::attach (const NewCalTable& calTable, 
				   ROScalarMeasColumn<MEpoch>& tabCol, 
				   NewCalTableEnums::colDef colEnum, 
				   const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         const NewCalTable&           Calibration table
//    tabCol           ROScalarMeasColumn<MEpoch>&  Table measures column 
//                                                  accessor
//    colEnum          NewCalTableEnums::colDef     Column enum
//    optional         const Bool&                  True if optional column
// Output to private data:
//
  // Convert to column name
  String colName = NCT::fieldName (colEnum);

  // Check if an optional column
  if (optional) {
    if (calTable.tableDesc().isColumn(colName)) {
      tabCol.attach (calTable, colName);
    };
  } else {
    tabCol.attach (calTable, colName);
  };
};

//----------------------------------------------------------------------------

void RONewCalMainColumns::attach (const NewCalTable& calTable, 
				   ROScalarQuantColumn<Double>& tabCol, 
				   NewCalTableEnums::colDef colEnum, 
				   const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable    const NewCalTable&             Calibration table
//    tabCol      ROScalarQuantColumn<Double>&   Scalar quantum column accessor
//    colEnum     NewCalTableEnums::colDef       Column enum
//    optional    const Bool&                    True if optional column
// Output to private data:
//
  // Convert to column name
  String colName = NCT::fieldName (colEnum);

  // Check if an optional column
  if (optional) {
    if (calTable.tableDesc().isColumn(colName)) {
      tabCol.attach (calTable, colName);
    };
  } else {
    tabCol.attach (calTable, colName);
  };
};
};
