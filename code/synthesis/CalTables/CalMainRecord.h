//# CalMainRecord.h: Calibration table record access and creation
//# Copyright (C) 1996,1997,1998,2001
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

#ifndef CALIBRATION_CALMAINRECORD_H
#define CALIBRATION_CALMAINRECORD_H

#include <casa/aips.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Containers/Record.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// CalMainRecord: Calibration table record access and creation
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
// The CalMainRecord classes allow access to, and creation of, records
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

class CalMainRecord
{
 public:
   // Default null constructor, and destructor
   CalMainRecord();
   ~CalMainRecord() {};

   // Construct from an existing record
   CalMainRecord (const Record& inpRec);

   // Return as record
   const Record& record();

   // Field accessors
   // a) define
   void defineTime (const Double& time);
   void defineTime (const MEpoch& time);
   void defineTimeEP (const Double& timeEP);
   void defineInterval (const Double& interval);
   void defineAntenna1 (const Int& antenna1);
   void defineFeed1 (const Int& feed1);
   void defineFieldId (const Int& fieldId);
   void defineArrayId (const Int& arrayId);
   void defineObsId (const Int& obsId);
   void defineScanNo (const Int& scanNo);
   void defineProcessorId (const Int& processorId);
   void defineStateId (const Int& stateId);
   void definePhaseId (const Int& phaseId);
   void definePulsarBin (const Int& pulsarBin);
   void definePulsarGateId (const Int& pulsarGateId);
   void defineFreqGrp (const Int& freqGrp);
   void defineFreqGrpName (const String& freqGrpName);
   void defineFieldName (const String& fieldName);
   void defineFieldCode (const String& fieldCode);
   void defineSourceName (const String& sourceName);
   void defineSourceCode (const String& sourceCode);
   void defineCalGrp (const Int& calGrp);
   void defineGain (const Array<Complex>& gain);
   void defineRefAnt (const Array<Int>& refAnt);
   void defineRefFeed (const Array<Int>& refFeed);
   void defineRefReceptor (const Array<Int>& refReceptor);
   void defineRefFreq (const Array<Double>& refFreq);
   void defineMeasFreqRef (const Int& measFreqRef);
   void defineRefDir (const Array<Double>& refDir);
   void defineMeasDirRef (const Int& measDirRef);
   void defineCalDescId (const Int& calDescId);
   void defineCalHistoryId (const Int& calHistoryId);

   // b) get
   void getTime (Double& time);
   void getTimeEP (Double& timeEP);
   void getInterval (Double& interval);
   void getAntenna1 (Int& antenna1);
   void getFeed1 (Int& feed1);
   void getFieldId (Int& fieldId);
   void getArrayId (Int& arrayId);
   void getObsId (Int& obsId);
   void getScanNo (Int& scanNo);
   void getProcessorId (Int& processorId);
   void getStateId (Int& stateId);
   void getPhaseId (Int& phaseId);
   void getPulsarBin (Int& pulsarBin);
   void getPulsarGateId (Int& pulsarGateId);
   void getFreqGrp (Int& freqGrp);
   void getFreqGrpName (String& freqGrpName);
   void getFieldName (String& fieldName);
   void getFieldCode (String& fieldCode);
   void getSourceName (String& sourceName);
   void getSourceCode (String& sourceCode);
   void getCalGrp (Int& calGrp);
   void getGain (Array<Complex>& gain);
   void getRefAnt (Array<Int>& refAnt);
   void getRefFeed (Array<Int>& refFeed);
   void getRefReceptor (Array<Int>& refReceptor);
   void getRefFreq (Array<Double>& refFreq);
   void getMeasFreqRef (Int& measFreqRef);
   void getRefDir (Array<Double>& refDir);
   void getMeasDirRef (Int& measDirRef);
   void getCalDescId (Int& calDescId);
   void getCalHistoryId (Int& calHistoryId);

 protected:
   // Add to itsRecord
   void addRec (const Record& newRec);

   // Field accessors for fit parameters.
   // Used in separate parts of the inheritance tree.
   // a) define
   void dTotalSolnOk (const Bool& totalSolnOk);
   void dTotalFit (const Float& totalFit);
   void dTotalFitWgt (const Float& totalFitWgt);
   void dSolnOk (const Array <Bool>& solnOk);
   void dFit (const Array <Float>& fit);
   void dFitWgt (const Array <Float>& fitWgt);
   void dFlag (const Array <Bool>& flag);
   void dSnr (const Array <Float>& snr);

   // b) get
   void gTotalSolnOk (Bool& totalSolnOk);
   void gTotalFit (Float& totalFit);
   void gTotalFitWgt (Float& totalFitWgt);
   void gSolnOk (Array <Bool>& solnOk);
   void gFit (Array <Float>& fit);
   void gFitWgt (Array <Float>& fitWgt);
   void gFlag (Array <Bool>& flag);
   void gSnr (Array <Float>& snr);

 private:
   Record itsRecord;

 };


} //# NAMESPACE CASA - END

#endif
   
  



