//# CTMainRecord.h: Calibration table record access and creation
//# Copyright (C) 1996,1997,1998,2001,2011
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
//# $Id: 

#ifndef CALIBRATION_CTMAINRECORD_H
#define CALIBRATION_CTMAINRECORD_H

#include <casa/aips.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Containers/Record.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// CTMainRecord: Calibration table record access and creation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="Record">Record</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration main table" and "record".
// </etymology>
//
// <synopsis>
// The CTMainRecord classes allow access to, and creation of, records
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
// Encapsulate access to calibration table records.
// </motivation>
//
// <todo asof="98/01/01">
// (i) Deal with non-standard columns.
// </todo>

class CTMainRecord
{
 public:
   // Default null constructor, and destructor
   CTMainRecord();
   ~CTMainRecord() {};

   // Construct from an existing record
   CTMainRecord (const Record& inpRec);

   // Return as record
   const Record& record();

   // Field accessors
   // a) define
   void defineTime (const Double& time);
   void defineTime (const MEpoch& time);
   void defineFieldId (const Int& fieldId);
   void defineSpwId (const Int& spwId);
   void defineAntenna1 (const Int& antenna1);
   void defineAntenna2 (const Int& antenna2);
   void defineInterval (const Double& interval);
   void defineScanNo (const Int& scanNo);
   void defineParam (const Array<Complex>& param);
   void defineParamerr (const Array<Float>& paramerr);
   void defineFlag (const Array<Bool>& flag);
   void defineSnr (const Array<Float>& snr);
   void defineWeight (const Array<Float>& weight);

   // b) get
   void getTime (Double& time);
   void getFieldId (Int& fieldId);
   void getSpwId (Int& spwId);
   void getAntenna1 (Int& antenna1);
   void getAntenna2 (Int& antenna2);
   void getInterval (Double& interval);
   void getScanNo (Int& scanNo);
   void getParam (Array<Complex>& param);
   void getParamerr (Array<Float>& paramerr);
   void getFlag (Array<Bool>& flag);
   void getSnr (Array<Float>& snr);
   void getWeight (Array<Float>& weight);

 protected:
   // Add to itsRecord
   void addRec (const Record& newRec);

 private:
   Record itsRecord;

 };


} //# NAMESPACE CASA - END

#endif
   
  


