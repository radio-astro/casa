//# CTEnums.cc: Implementation of CTEnums.h
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

#include <synthesis/CalTables/CTEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

// Static data initialization
SimpleOrderedMap <Int, String> CTEnums::theirFieldMap ("");
SimpleOrderedMap <Int, DataType> CTEnums::theirTypeMap (TpOther);

//----------------------------------------------------------------------------

void CTEnums::initMaps ()
{
// Initialize the static map containing the field names.
// Skip this step if already initialized.
//
  if (!theirFieldMap.ndefined()) {
    theirFieldMap.define (ANTENNA1, "ANTENNA1");
    theirFieldMap.define (ANTENNA2, "ANTENNA2");
    theirFieldMap.define (SCAN_NUMBER, "SCAN_NUMBER");
    theirFieldMap.define (TIME, "TIME");
    theirFieldMap.define (TIME_EXTRA_PREC, "TIME_EXTRA_PREC");
    theirFieldMap.define (INTERVAL, "INTERVAL");
    theirFieldMap.define (ARRAY_ID, "ARRAY_ID");
    theirFieldMap.define (FIELD_ID, "FIELD_ID");
    theirFieldMap.define (OBSERVATION_ID, "OBSERVATION_ID");
    theirFieldMap.define (SPECTRAL_WINDOW_ID, "SPECTRAL_WINDOW_ID");

    theirFieldMap.define (CPARAM, "CPARAM");
    theirFieldMap.define (FPARAM, "FPARAM");
    theirFieldMap.define (PARAMERR, "PARAMERR");
    theirFieldMap.define (FLAG, "FLAG");
    theirFieldMap.define (SNR, "SNR");
    theirFieldMap.define (WEIGHT, "WEIGHT");
    
  };

// Initialize the static map containing the basic field data types
// Skip this step if already initialized.
//
  if (!theirTypeMap.ndefined()) {
    theirTypeMap.define (ANTENNA1, TpInt);
    theirTypeMap.define (ANTENNA2, TpInt);
    theirTypeMap.define (SCAN_NUMBER, TpInt);
    theirTypeMap.define (TIME, TpDouble);
    theirTypeMap.define (TIME_EXTRA_PREC, TpDouble);
    theirTypeMap.define (INTERVAL, TpDouble);
    theirTypeMap.define (ARRAY_ID, TpInt);
    theirTypeMap.define (FIELD_ID, TpInt);
    theirTypeMap.define (OBSERVATION_ID, TpInt);
    theirTypeMap.define (SPECTRAL_WINDOW_ID, TpInt);

    theirTypeMap.define (CPARAM, TpComplex);
    theirTypeMap.define (FPARAM, TpFloat);
    theirTypeMap.define (PARAMERR, TpFloat);
    theirTypeMap.define (FLAG, TpBool);
    theirTypeMap.define (SNR, TpFloat);
    theirTypeMap.define (WEIGHT, TpFloat);
    
  };

};

//----------------------------------------------------------------------------

String CTEnums::fieldName (Int enumField)
{
// Static function to look up the field name:
// Inputs:
//    enumField   Int     Field enumeration.
// Outputs:
//    fieldName   String  Field name.
// Exceptions:
//    Exception if invalid field enumeration.
//
  // Initialize map if empty
  if (!theirFieldMap.ndefined()) initMaps();
  
  // Return the column name
  return theirFieldMap (enumField);
};

//----------------------------------------------------------------------------

Block<String> CTEnums::fieldNames (const Vector<Int>& enumFields)
{
// Static function to look up a set of field names:
// Inputs:
//    enumFields  const Vector<Int>&     Field enumerations.
// Outputs:
//    fieldNames  Block<String>          Field names.
// Exceptions:
//    Exception if invalid field enumeration.
//
  // Return the column names
  uInt nFields = enumFields.nelements();
  Block<String> names(nFields);
  for (uInt i=0; i < nFields; i++) {
    names[i] = fieldName (enumFields(i));
  };
  return names;
};

//----------------------------------------------------------------------------

DataType CTEnums::basicType (Int enumField)
{
// Static function to look up the basic field data type:
// Inputs:
//    enumField   Int        Field enumeration.
// Outputs:
//    basicType   DataType   Basic data type
// Exceptions:
//    Exception if invalid field enumeration.
//
  // Initialize map if empty
  if (!theirTypeMap.ndefined()) initMaps();
  
  // Return the column name
  return theirTypeMap (enumField);
};

//----------------------------------------------------------------------------







} //# NAMESPACE CASA - END

