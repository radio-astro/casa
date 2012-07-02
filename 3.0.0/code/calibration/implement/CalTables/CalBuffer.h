//# CalBuffer.h: Calibration table buffer
//# Copyright (C) 1996,1997,1998,2001,2003
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

#ifndef CALIBRATION_CALBUFFER_H
#define CALIBRATION_CALBUFFER_H

#include <calibration/CalTables/CalIterBase.h>
#include <calibration/CalTables/CalTable.h>
#include <calibration/CalTables/CalMainColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// CalBuffer: Calibration table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
//   <li> <linkto class="CalIterBase">CalIterBase</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration table" and "buffer"
// </etymology>
//
// <synopsis>
// The CalBuffer class holds a buffer, optinally connected to a 
// calibration table iterator (as derived from CalIterBase). 
// Specializations for baseline-based, time-variable and solvable 
// VisJones calibration table formats are provided through 
// inheritance.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate calibration table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class CalBuffer 
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  CalBuffer();

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  CalBuffer (CalIterBase& calIter);

  // Default destructor
  virtual ~CalBuffer();

  // Invalidate the current calibration buffer. This signals
  // that a re-read is required as the iterator has advanced.
  virtual void invalidate();

  // Write the current buffer at the end of a specified cal table (NYI)
  virtual void append (CalTable& calTable) {};

  // Data field accessors
  Vector<Double>& time();
  //  Vector<MEpoch>& timeMeas();
  Vector<Double>& timeEP();
  Vector<Double>& interval();
  Vector<Int>& antenna1();
  Vector<Int>& feed1();
  Vector<Int>& fieldId();
  Vector<Int>& arrayId();
  Vector<Int>& obsId();
  Vector<Int>& scanNo();
  Vector<Int>& processorId();
  Vector<Int>& stateId();
  Vector<Int>& phaseId();
  Vector<Int>& pulsarBin();
  Vector<Int>& pulsarGateId();
  Vector<Int>& freqGrp();
  Vector<String>& freqGrpName();
  Vector<String>& fieldName();
  Vector<String>& fieldCode();
  Vector<String>& sourceName();
  Vector<String>& sourceCode();
  Vector<Int>& calGrp();
  Array<Complex>& gain();
  Array<Int>& refAnt();
  Array<Int>& refFeed();
  Array<Int>& refReceptor();
  Array<Double>& refFreq();
  Vector<Int>& measFreqRef();
  Array<Double>& refDir();
  Vector<Int>& measDirRef();
  Vector<Int>& calDescId();
  Vector<Int>& calHistoryId(); 

 protected:

 private:
  // True if connected to underlying iterator
  Bool connectedToIter_p;

  // Ptr to cal main columns accessor
  CalMainColumns* calMainCol_p;

  // Buffer fields
  Vector<Double> time_p;
  Vector<MEpoch> timeMeas_p;
  Vector<Double> timeEP_p;
  Vector<Double> interval_p;
  Vector<Int> antenna1_p;
  Vector<Int> feed1_p;
  Vector<Int> fieldId_p;
  Vector<Int> arrayId_p;
  Vector<Int> obsId_p;
  Vector<Int> scanNo_p;
  Vector<Int> processorId_p;
  Vector<Int> stateId_p;
  Vector<Int> phaseId_p;
  Vector<Int> pulsarBin_p;
  Vector<Int> pulsarGateId_p;
  Vector<Int> freqGrp_p;
  Vector<String> freqGrpName_p;
  Vector<String> fieldName_p;
  Vector<String> fieldCode_p;
  Vector<String> sourceName_p;
  Vector<String> sourceCode_p;
  Vector<Int> calGrp_p;
  Array<Complex> gain_p;
  Array<Int> refAnt_p;
  Array<Int> refFeed_p;
  Array<Int> refReceptor_p;
  Array<Double> refFreq_p;
  Vector<Int> measFreqRef_p;
  Array<Double> refDir_p;
  Vector<Int> measDirRef_p;
  Vector<Int> calDescId_p;
  Vector<Int> calHistoryId_p;

  // Buffer field status flags
  Bool timeOK_p;
  Bool timeMeasOK_p;
  Bool timeEPOK_p;
  Bool intervalOK_p;
  Bool antenna1OK_p;
  Bool feed1OK_p;
  Bool fieldIdOK_p;
  Bool arrayIdOK_p;
  Bool obsIdOK_p;
  Bool scanNoOK_p;
  Bool processorIdOK_p;
  Bool stateIdOK_p;
  Bool phaseIdOK_p;
  Bool pulsarBinOK_p;
  Bool pulsarGateIdOK_p;
  Bool freqGrpOK_p;
  Bool freqGrpNameOK_p;
  Bool fieldNameOK_p;
  Bool fieldCodeOK_p;
  Bool sourceNameOK_p;
  Bool sourceCodeOK_p;
  Bool calGrpOK_p;
  Bool gainOK_p;
  Bool refAntOK_p;
  Bool refFeedOK_p;
  Bool refReceptorOK_p;
  Bool refFreqOK_p;
  Bool measFreqRefOK_p;
  Bool refDirOK_p;
  Bool measDirRefOK_p;
  Bool calDescIdOK_p;
  Bool calHistoryIdOK_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



