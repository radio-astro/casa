//# NewCalMainRecord.cc: Implementation of NewCalMainRecord.h
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

#include <calibration/CalTables/NewCalMainRecord.h>
#include <calibration/CalTables/NewCalTableEnums.h>
#include <casa/Arrays.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

NewCalMainRecord::NewCalMainRecord() : itsRecord()
{
// Null constructor
// Output to private data:
//    itsRecord        Record          Underlying record object
//
};

//----------------------------------------------------------------------------

NewCalMainRecord::NewCalMainRecord (const Record& inpRec) : itsRecord (inpRec)
{
// Construct from an input record
// Input:
//    inpRec           Record          Input record
// Output to private data:
//    itsRecord        Record          Underlying record object
//
};

//----------------------------------------------------------------------------

const Record& NewCalMainRecord::record()
{
// Return as record
// Output:
//    record           const Record&   Return underlying record object
// Input from private data:
//    itsRecord        Record          Underlying record object
//
  return itsRecord;
};

//----------------------------------------------------------------------------

void NewCalMainRecord::addRec (const Record& newRec)
{
// Add to underlying record object
// Input:
//    newRec           Record          Record to be added
// Output to private data:
//    itsRecord        Record          Underlying record object
//
  itsRecord.merge (newRec, RecordInterface::OverwriteDuplicates);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineTime (const Double& time)
{
// Define the TIME field value (expressed as a Double)
// Input:
//    time             const Double&         TIME value
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define (NCT::fieldName (NCT::TIME), time);
  addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineTime (const MEpoch& time)
{
// Define the TIME field value (expressed as an MEpoch)
// Input:
//    time             const MEpoch&         TIME value
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define (NCT::fieldName (NCT::TIME), time.getValue().get());
  addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineInterval (const Double& interval)
{
// Define the INTERVAL field value
// Input:
//    interval         const Double&         INTERVAL value
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define (NCT::fieldName (NCT::INTERVAL), interval);
  addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineAntenna1 (const Int& antenna1)
{
// Define the ANTENNA1 field value
// Input:
//    antenna1         const Int&            ANTENNA1 value
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define (NCT::fieldName (NCT::ANTENNA1), antenna1);
  addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineAntenna2 (const Int& antenna2)
{
// Define the ANTENNA2 field value
// Input:
//    antenna2         const Int&            ANTENNA2 value
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define (NCT::fieldName (NCT::ANTENNA2), antenna2);
  addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineFieldId (const Int& fieldId)
{
// Define the FIELD_ID field value
// Input:
//    fieldId          const Int&            FIELD_ID value
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define (NCT::fieldName (NCT::FIELD_ID), fieldId);
  addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineSpwId (const Int& spwId)
{
// Define the SPECTRAL_WINDOW_ID field value
// Input:
//    spwId          const Int&            SPECTRAL_WINDOW_ID value
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define (NCT::fieldName (NCT::SPECTRAL_WINDOW_ID), spwId);
      addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineScanNo (const Int& scanNo)
{
// Define the SCAN_NUMBER field value
// Input:
//    scanNo           const Int&            SCAN_NUMBER value
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define (NCT::fieldName (NCT::SCAN_NUMBER), scanNo);
  addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineParam (const Array<Float>& param)
{
// Define the PARAM field value
// Input:
//    param           const Array<Float>&    cal solution param values
// Output to private data:
//    itsRecord        Record                Underlying record object
//
  Record newRec;
  newRec.define ("PARAM", param);
  addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineParamerr (const Array<Float>& paramerr)
{
// Define the PARAMERR field value
// Input:
//    paramerr         const Array<Float>&   cal solution param err values
// Output to private data:
//    itsRecord        Record                Underlying record object
//
   Record newRec;
   newRec.define ("PARAMERR", paramerr);
   addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineFlag (const Array<Bool>& flag)
{
// Define the FLAG field value
// Input:
//    flag             const Array<Float>&   cal solution param err values
// Output to private data:
//    itsRecord        Record                Underlying record object
//
   Record newRec;
   newRec.define ("FLAG", flag);
   addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineSnr (const Array<Float>& snr)
{
// Define the SNR field value
// Input:
//    SNR             const Array<Float>&    cal solution snr values
// Output to private data:
//    itsRecord        Record                Underlying record object
//
   Record newRec;
   newRec.define ("SNR", snr);
   addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::defineWeight (const Array<Float>& weight)
{
// Define the Weight field value
// Input:
//    weight           const Array<Float>&  cal solution weight values
// Output to private data:
//    itsRecord        Record                Underlying record object
//
   Record newRec;
   newRec.define ("Weight", weight);
   addRec (newRec);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getTime (Double& time)
{
// Get the TIME field value
// Output:
//    time             Double&               TIME value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get (NCT::fieldName (NCT::TIME), time);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getInterval (Double& interval)
{
// Get the INTERVAL field value
// Output:
//    interval         Double&               INTERVAL value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get (NCT::fieldName (NCT::INTERVAL), interval);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getAntenna1 (Int& antenna1)
{
// Get the ANTENNA1 field value
// Output:
//    antenna1         Int&                  ANTENNA1 value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get (NCT::fieldName (NCT::ANTENNA1), antenna1);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getAntenna2 (Int& antenna2)
{
// Get the ANTENNA2 field value
// Output:
//    antenna2         Int&                  ANTENNA2 value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get (NCT::fieldName (NCT::ANTENNA2), antenna2);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getFieldId (Int& fieldId)
{
// Get the FIELD_ID field value
// Output:
//    fieldId          Int&                  FIELD_ID value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get (NCT::fieldName (NCT::FIELD_ID), fieldId);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getSpwId (Int& spwId)
{
// Get the SPECTRAL_WINDOW_ID field value
// Output:
//    spwId          Int&                  SPECTRAL_WINDOW_ID value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get (NCT::fieldName (NCT::SPECTRAL_WINDOW_ID), spwId);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getScanNo (Int& scanNo)
{
// Get the SCAN_NUMBER field value
// Output:
//    scanNo           Int&                  SCAN_NUMBER value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get (NCT::fieldName (NCT::SCAN_NUMBER), scanNo);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getParam (Array<Float>& param)
{
// Get the PARAM field value
// Output:
//    param           Array<Float>&                  PARAM value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get ("PARAM", param);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getParamerr (Array<Float>& paramerr)
{
// Get the PARAMERR field value
// Output:
//    paramerr        Array<Float>&          paramerr value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get ("PARAMERR", paramerr);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getFlag (Array<Bool>& flag)
{
// Get the FLAG field value
// Output:
//    flag        Array<Bool>&          flag value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get ("FLAG", flag);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getSnr (Array<Float>& snr)
{
// Get the FLAG field value
// Output:
//    snr        Array<Float>&          snr value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get ("SNR", snr);
};

//----------------------------------------------------------------------------

void NewCalMainRecord::getWeight (Array<Float>& weight)
{
// Get the WEIGHT field value
// Output:
//    weight        Array<Float>&          weight value
// Input from private data:
//    itsRecord        Record                Underlying record object
//
  record().get ("WEIGHT", weight);
};


} //# NAMESPACE CASA - END

