//# CalDescBuffer.h: Calibration table cal_desc buffer
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

#ifndef CALIBRATION_CALDESCBUFFER_H
#define CALIBRATION_CALDESCBUFFER_H

#include <casa/Arrays/Matrix.h>
#include <calibration/CalTables/CalIterBase.h>
#include <calibration/CalTables/CalTable.h>
#include <calibration/CalTables/CalDescColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// CalDescBuffer: Calibration table cal_desc buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
//   <li> <linkto class="CalIterBase">CalIterBase</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration description table" and "buffer"
// </etymology>
//
// <synopsis>
// The CalDescBuffer class holds a buffer, optionally connected to a 
// calibration table cal_desc iterator (of base type CalIterBase). 
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate calibration table cal_desc data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class CalDescBuffer 
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  CalDescBuffer();

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  CalDescBuffer (CalIterBase& calIter);

  // Default destructor
  virtual ~CalDescBuffer();

  // Invalidate the current calibration table cal_desc buffer. 
  // This signals that a re-read is required as the iterator has advanced.
  virtual void invalidate();

  // Write the current buffer at the end of a specified calibration
  // table cal_desc subtable(NYI)
  virtual void append (CalTable& calTable) {};

  // Data field accessors
  Vector<Int>& numSpw();
  Vector<Int>& numChan();
  Vector<Int>& numReceptors();
  Vector<Int>& nJones();
  Matrix<Int>& spwId();
  Array<MFrequency>& chanFreqMeas();
  Vector<Int>& measFreqRef();
  Array<Quantity>& chanWidthQuant();
  Array<Int>& chanRange();
  Matrix<String>& polznType();
  Vector<String>& jonesType();
  Vector<String>& msName();

 protected:
  // Factory method to create a columns accessor object of the appropriate type
  virtual CalDescColumns* newCalDescCol (CalTable& calTable)
    {return new CalDescColumns (calTable);};

  // Access to the columns accessor object
  virtual CalDescColumns* calDescCol() {return calDescCol_p;};

  // Is the buffer connected to an underlying iterator ?
  Bool connectedToIter() {return connectedToIter_p;};

 private:
  // True if connected to underlying iterator
  Bool connectedToIter_p;

  // Ptr to cal main columns accessor
  CalDescColumns* calDescCol_p;

  // Buffer fields
  Vector<Int> numSpw_p;
  Vector<Int> numChan_p;
  Vector<Int> numReceptors_p;
  Vector<Int> nJones_p;
  Matrix<Int> spwId_p;
  Array<MFrequency> chanFreqMeas_p;
  Vector<Int> measFreqRef_p;
  Array<Quantity> chanWidthQuant_p;
  Array<Int> chanRange_p;
  Matrix<String> polznType_p;
  Vector<String> jonesType_p;
  Vector<String> msName_p;

  // Buffer field status flags
  Bool numSpwOK_p;
  Bool numChanOK_p;
  Bool numReceptorsOK_p;
  Bool nJonesOK_p;
  Bool spwIdOK_p;
  Bool chanFreqMeasOK_p;
  Bool measFreqRefOK_p;
  Bool chanWidthQuantOK_p;
  Bool chanRangeOK_p;
  Bool polznTypeOK_p;
  Bool jonesTypeOK_p;
  Bool msNameOK_p;
};


} //# NAMESPACE CASA - END

#endif
