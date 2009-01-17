//# CalMainBuffer.h: Calibration main table buffer
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

#ifndef CALIBRATION_CALMAINBUFFER_H
#define CALIBRATION_CALMAINBUFFER_H

#include <calibration/CalTables/CalDescBuffer.h>
#include <calibration/CalTables/CalHistoryBuffer.h>
#include <calibration/CalTables/CalTable.h>
#include <calibration/CalTables/CalMainColumns.h>
#include <msvis/MSVis/MSCalEnums.h>
#include <msvis/MSVis/VisBuffer.h>
#include <calibration/CalTables/CalIterBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// CalMainBuffer: Calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
//   <li> <linkto class="CalIterBase">CalIterBase</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration main table" and "buffer"
// </etymology>
//
// <synopsis>
// The CalMainBuffer class holds a buffer, optionally connected to a 
// calibration main table iterator (of base type CalIterBase). 
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
// Encapsulate calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class CalMainBuffer 
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  CalMainBuffer();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values. There is no connection to an underlying 
  // calibration table iterator in this case.
  CalMainBuffer (const Vector<Int>& calIndices, 
		 const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  CalMainBuffer (CalIterBase& calIter);

  // Default destructor
  virtual ~CalMainBuffer();

  // Synchronize the buffer (if attached to an iterator)
  virtual void synchronize();

  // Force an explicit read to cache for all columns 
  // (if attached to an iterator)
  virtual void fillCache();

  // Write the current buffer at the end of a specified cal table;
  // returns the number of rows appended.
  virtual Int append (CalTable& calTable);

  // Maximum number of rows in the calibration buffer
  virtual Int nRow();

  // Accessors for the contained cal_desc and cal_history buffers
  virtual CalDescBuffer& calDescBuffer() {return *calDescBuf_p;};
  virtual CalHistoryBuffer& calHistoryBuffer() {return *calHistBuf_p;};

  // Data field accessors
  virtual Vector<MEpoch>& timeMeas();
  virtual Vector<Quantity>& timeEPQuant();
  virtual Vector<Quantity>& intervalQuant();
  virtual Vector<Int>& antenna1();
  virtual Vector<Int>& feed1();
  virtual Vector<Int>& fieldId();
  virtual Vector<Int>& arrayId();
  virtual Vector<Int>& obsId();
  virtual Vector<Int>& scanNo();
  virtual Vector<Int>& processorId();
  virtual Vector<Int>& stateId();
  virtual Vector<Int>& phaseId();
  virtual Vector<Int>& pulsarBin();
  virtual Vector<Int>& pulsarGateId();
  virtual Vector<Int>& freqGrp();
  virtual Vector<String>& freqGrpName();
  virtual Vector<String>& fieldName();
  virtual Vector<String>& fieldCode();
  virtual Vector<String>& sourceName();
  virtual Vector<String>& sourceCode();
  virtual Vector<Int>& calGrp();
  virtual Array<Complex>& gain();
  virtual Array<Int>& refAnt();
  virtual Array<Int>& refFeed();
  virtual Array<Int>& refReceptor();
  virtual Array<MFrequency>& refFreqMeas();
  virtual Vector<Int>& measFreqRef();
  virtual Array<MDirection>& refDirMeas();
  virtual Vector<Int>& measDirRef();
  virtual Vector<Int>& calDescId();
  virtual Vector<Int>& calHistoryId(); 

  // Generic access to integer columns by MSCalEnums enumeration. Throws
  // an exception if the column is not recognized or is not an integer column.
  virtual Vector<Int>& asVecInt (const MSCalEnums::colDef& calEnum);

  // Generic access to string columns by MSCalEnums enumeration. Throws
  // an exception if the column is not recognized or is not a string column.
  virtual Vector<String>& asVecString (const MSCalEnums::colDef& calEnum);

  // Find the matching rows for a given antenna1 id.
  virtual Vector<Int> matchAntenna1 (const Int& antennaId);

  // Find the matching rows for a given antenna1 id. and field id. pair
  virtual Vector<Int> matchAntenna1AndFieldId (const Int& antennaId,
					       const Int& fldId);

  // Find the matching rows for a given antenna1 id. and freq. group name
  virtual Vector<Int> matchAntenna1AndFreqGrp (const Int& antennaId,
					       const String& frqGrpName);

 protected:
  // Factory method to create a columns accessor object of the appropriate type
  virtual CalMainColumns* newCalMainCol (CalTable& calTable)
    {return new CalMainColumns (calTable);};

  // Access to the columns accessor object
  virtual CalMainColumns* calMainCol() {return calMainCol_p;};

  // Is the buffer connected to an underlying iterator ?
  Bool connectedToIter() {return connectedToIter_p;};

  // Invalidate the current cache. 
  virtual void invalidate();

  // <group>
  // Span an empty cal buffer by taking all permutations of the
  // values of a specified set of cal indices, specified as
  // enums from class MSCalEnums:
  //
  // Use a visibility buffer to define the index values 
  virtual void fillIndices (const Vector<Int>& calIndices,
			    const VisBuffer& vb);

  // Define the index values directly
  virtual void fillIndices (const Vector<Int>& calIndices,
			    const Block<Vector<Int> >& indexValues);
  // </group>

  // <group>
  // Fill the cal buffer attribute columns in an empty cal buffer,
  // after the cal indices have been set using fillIndices(). The 
  // cal indices, specified as enums from class MSCalEnums, are excluded
  // as non-attribute columns.
  //
  // Use a visibility buffer to define the attribute values
  virtual void fillAttributes(const Vector<Int>& calIndices,
			      const VisBuffer& vb);

  // Set default attribute values
  virtual void fillAttributes(const Vector<Int>& calIndices);

  // </group>

  // Utility function to test for membership in a list of calibration enums
  Bool excluded(const MSCalEnums::colDef& calEnum, 
		const Vector<Int>& excludeIndices);

 private:
  // True if connected to underlying iterator
  Bool connectedToIter_p;

  // Ptr to calibration table iterator
  CalIterBase* calIter_p;

  // Ptr to cal main columns accessor
  CalMainColumns* calMainCol_p;

  // Contained sub-table cal_desc and cal_history buffers
  CalDescBuffer* calDescBuf_p;
  CalHistoryBuffer* calHistBuf_p;

  // Buffer fields
  Vector<MEpoch> timeMeas_p;
  Vector<Quantity> timeEPQuant_p;
  Vector<Quantity> intervalQuant_p;
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
  Array<MFrequency> refFreqMeas_p;
  Vector<Int> measFreqRef_p;
  Array<MDirection> refDirMeas_p;
  Vector<Int> measDirRef_p;
  Vector<Int> calDescId_p;
  Vector<Int> calHistoryId_p;

  // Buffer field status flags
  Bool timeMeasOK_p;
  Bool timeEPQuantOK_p;
  Bool intervalQuantOK_p;
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
  Bool refFreqMeasOK_p;
  Bool measFreqRefOK_p;
  Bool refDirMeasOK_p;
  Bool measDirRefOK_p;
  Bool calDescIdOK_p;
  Bool calHistoryIdOK_p;
};


} //# NAMESPACE CASA - END

#endif
   
  



