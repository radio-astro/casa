//# NewCalMainColumns.cc: Implementation of NewCalMainColumns.h
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

#include <calibration/CalTables/NewCalMainColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

NewCalMainColumns::NewCalMainColumns(NewCalTable& calTable) 
{
// Construct from a new calibration table
// Input:
// Input:
//    calTable         const NewCalTable&           Calibration table
// Output to private data:
//    time_p           ScalarColumn<Double>&        Time
//    timeMeas_p       ScalarMeasColumn<MEpoch>&    Time as a Measure
//    timeEP_p         ScalarColumn<Double>&        Extended precision time
//    timeEPQuant_p    ScalarQuantColumn<Double>&   EP time as Quantum
//    interval_p       ScalarColumn<Double>&        Time interval
//    intervalQuant_p  ScalarQuantColumn<Double>&   Time interval as Quantum
//    antenna1_p       ScalarColumn<Int>&           Antenna 1
//    antenna2_p       ScalarColumn<Int>&           Antenna 2
//    fieldId_p        ScalarColumn<Int>&           Field id.
//    spwId_p          ScalarColumn<Int>&           Spectral window id.
//    scanNo_p         ScalarColumn<Int>&           Scan no.
//    param_p          ArrayColumn<Float>&          Cal solution params.
//    paramerr_p       ArrayColumn<Float>&          Cal solution error
//    flag_p           ArrayColumn<Bool>&           Ref. dir. as Measure
//    snr_p            ArrayColumn<Float>&          Solution SNR
//    weight_p         ArrayColumn<Float>&          Solution weight
//
  // Attach all the column accessors (including required and
  // optional columns)
  attach (calTable, time_p, NCT::TIME);
  attach (calTable, timeMeas_p, NCT::TIME);
  attach (calTable, timeEP_p, NCT::TIME_EXTRA_PREC, True);
  attach (calTable, timeEPQuant_p, NCT::TIME_EXTRA_PREC, True);
  attach (calTable, interval_p, NCT::INTERVAL);
  attach (calTable, intervalQuant_p, NCT::INTERVAL);
  attach (calTable, fieldId_p, NCT::FIELD_ID);
  attach (calTable, spwId_p, NCT::SPECTRAL_WINDOW_ID);
  attach (calTable, antenna1_p, NCT::ANTENNA1);
  attach (calTable, antenna2_p, NCT::ANTENNA2);
  attach (calTable, scanNo_p, NCT::SCAN_NUMBER);

  attach (calTable, param_p, NCT::PARAM);
  attach (calTable, paramerr_p, NCT::PARAMERR);
  attach (calTable, flag_p, NCT::FLAG);
  attach (calTable, snr_p, NCT::SNR);
  attach (calTable, weight_p, NCT::WEIGHT);

};

//----------------------------------------------------------------------------

void NewCalMainColumns::attach (NewCalTable& calTable, 
				 TableColumn& tabCol, 
				 NewCalTableEnums::colDef colEnum, 
				 const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         CalTable&                  Calibration table
//    tabCol           TableColumn&               Table column accessor
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

void NewCalMainColumns::attach (NewCalTable& calTable, 
				 ArrayMeasColumn<MEpoch>& tabCol, 
				 NewCalTableEnums::colDef colEnum, 
				 const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         NewCalTable&                  Calibration table
//    tabCol           ArrayMeasColumn<MEpoch>&      Table measures column 
//                                                   accessor
//    colEnum          NewCalTableEnums::colDef      Column enum
//    optional         const Bool&                   True if optional column
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

void NewCalMainColumns::attach (NewCalTable& calTable, 
				 ArrayMeasColumn<MFrequency>& tabCol, 
				 NewCalTableEnums::colDef colEnum, 
				 const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         CalTable&                     Calibration table
//    tabCol           ArrayMeasColumn<MFrequency>&  Table measures column 
//                                                   accessor
//    colEnum          NewCalTableEnums::colDef            Column enum
//    optional         const Bool&                   True if optional column
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

void NewCalMainColumns::attach (NewCalTable& calTable, 
				 ArrayMeasColumn<MDirection>& tabCol, 
				 NewCalTableEnums::colDef colEnum, 
				 const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         NewCalTable&                  Calibration table
//    tabCol           ArrayMeasColumn<MDirection>&  Table measures column 
//                                                   accessor
//    colEnum          NewCalTableEnums::colDef            Column enum
//    optional         const Bool&                   True if optional column
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

void NewCalMainColumns::attach (NewCalTable& calTable, 
				 ScalarMeasColumn<MEpoch>& tabCol, 
				 NewCalTableEnums::colDef colEnum, 
				 const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable         CalTable&                   Calibration table
//    tabCol           ScalarMeasColumn<MEpoch>&   Table measures column 
//                                                 accessor
//    colEnum          NewCalTableEnums::colDef          Column enum
//    optional         const Bool&                 True if optional column
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

void NewCalMainColumns::attach (NewCalTable& calTable, 
				 ScalarQuantColumn<Double>& tabCol, 
				 NewCalTableEnums::colDef colEnum, 
				 const Bool& optional)
{
// Attach a column accessor to the calibration table
// Input:
//    calTable    CalTable&                    Calibration table
//    tabCol      ScalarQuantColumn<Double>&   Table measures column accessor
//    colEnum     NewCalTableEnums::colDef           Column enum
//    optional    const Bool&                  True if optional column
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


} //# NAMESPACE CASA - END

