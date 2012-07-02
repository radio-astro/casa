//# EPointMBuf.h: SolvableVisJones calibration main table buffer
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

#ifndef CALIBRATION_EPOINTMBUF_H
#define CALIBRATION_EPOINTMBUF_H

#include <calibration/CalTables/TimeVarVJMBuf.h>
#include <calibration/CalTables/EPointMCol.h>
#include <calibration/CalTables/SolvableVJTable.h>
namespace casa {
// <summary> 
// EPointMBuf: EPoint calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="CalMainBuffer">CalMainBuffer</linkto> module
// <li> <linkto class="TimeVarVisJonesMBuf">TimeVarVisJonesMBuf</linkto> module
// </prerequisite>
//
// <etymology>
// From "solvable visibility Jones matrix", "cal main table" and "buffer"
// </etymology>
//
// <synopsis>
// The EPointMBuf class holds a buffer, optionally connected to a 
// EPoint calibration main table iterator (of base type 
// CalIterBase). Specializations for different solvable Jones matrix
// types are provided through inheritance.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate EPoint calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class EPointMBuf : public TimeVarVisJonesMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  EPointMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  EPointMBuf (const Vector<Int>& calIndices, 
			const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  EPointMBuf (CalIterBase& calIter);

  // Write the current buffer at the end of a specified cal table;
  // returns the number of rows appended
  virtual Int append (CalTable& calTable);

  // Maximum number of rows in the calibration buffer
  virtual Int nRow();

  // Data field accessors
  virtual Array<Float>& pointingOffset();

 protected:
  // Factory method to create a columns accessor object of the appropriate type
  virtual EPointMCol* newCalMainCol (CalTable& calTable) {
    return new 
      EPointMCol(dynamic_cast<EPointTable&>(calTable));};

  // Calibration table columns accessor
  virtual EPointMCol* calMainCol() 
    {return dynamic_cast<EPointMCol*>(CalMainBuffer::calMainCol());};

  // Invalidate the current cache.
  virtual void invalidate();

  // <group>
  // Fill the cal buffer attribute columns in an empty cal buffer,
  // after the cal indices have been set using fillIndices(). The
  // cal indices, specified as enums from class MSCalEnums, are excluded
  // as non-attribute columns
  //
  // Use a visibility buffer to define the attribute values (NYI)
  virtual void fillAttributes(const Vector<Int>& calIndices,
			      const VisBuffer& vb) {};
  //
  // Set default attribute values
  virtual void fillAttributes(const Vector<Int>& calIndices);
  // </group>

 private:
  // Buffer fields
  Array<Float> pointingOffset_p;

  // Buffer field status flags
  Bool pointingOffsetOK_p;
};
}
#endif
