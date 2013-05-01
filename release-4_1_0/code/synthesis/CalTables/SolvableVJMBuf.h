//# SolvableVJMBuf.h: SolvableVisJones calibration main table buffer
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

#ifndef CALIBRATION_SOLVABLEVJMBUF_H
#define CALIBRATION_SOLVABLEVJMBUF_H

#include <synthesis/CalTables/TimeVarVJMBuf.h>
#include <synthesis/CalTables/SolvableVJMCol.h>
#include <synthesis/CalTables/SolvableVJTable.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// SolvableVisJonesMBuf: SolvableVisJones calibration main table buffer
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
// The SolvableVisJonesMBuf class holds a buffer, optionally connected to a 
// SolvableVisJones calibration main table iterator (of base type 
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
// Encapsulate SolvableVisJones calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class SolvableVisJonesMBuf : public TimeVarVisJonesMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  SolvableVisJonesMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  SolvableVisJonesMBuf (const Vector<Int>& calIndices, 
			const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  SolvableVisJonesMBuf (CalIterBase& calIter);

  // Write the current buffer at the end of a specified cal table;
  // returns the number of rows appended
  virtual Int append (CalTable& calTable);

  // Maximum number of rows in the calibration buffer
  virtual Int nRow();

  // Data field accessors
  virtual Vector<Bool>& totalSolnOk();
  virtual Vector<Float>& totalFit();
  virtual Vector<Float>& totalFitWgt();
  virtual Array<Bool>& solnOk();
  virtual Array<Float>& fit();
  virtual Array<Float>& fitWgt();
  virtual Array<Bool>& flag();
  virtual Array<Float>& snr();

 protected:
  // Factory method to create a columns accessor object of the appropriate type
  virtual SolvableVisJonesMCol* newCalMainCol (CalTable& calTable) {
    return new 
      SolvableVisJonesMCol(dynamic_cast<SolvableVisJonesTable&>(calTable));};

  // Calibration table columns accessor
  virtual SolvableVisJonesMCol* calMainCol() 
    {return dynamic_cast<SolvableVisJonesMCol*>(CalMainBuffer::calMainCol());};

  // Invalidate the current cache.
  virtual void invalidate();

  // <group>
  // Fill the cal buffer attribute columns in an empty cal buffer,
  // after the cal indices have been set using fillIndices(). The
  // cal indices, specified as enums from class MSCalEnums, are excluded
  // as non-attribute columns
  //
  // Use a visibility buffer to define the attribute values (NYI)
  virtual void fillAttributes(const Vector<Int>& /*calIndices*/,
			      const VisBuffer& /*vb*/) {};
  //
  // Set default attribute values
  virtual void fillAttributes(const Vector<Int>& calIndices);
  // </group>

 private:
  // Buffer fields
  Vector<Bool> totalSolnOk_p;
  Vector<Float> totalFit_p;
  Vector<Float> totalFitWgt_p;
  Array<Bool> solnOk_p;
  Array<Float> fit_p;
  Array<Float> fitWgt_p;
  Array<Bool> flag_p;
  Array<Float> snr_p;

  // Buffer field status flags
  Bool totalSolnOkOK_p;
  Bool totalFitOK_p;
  Bool totalFitWgtOK_p;
  Bool solnOkOK_p;
  Bool fitOK_p;
  Bool fitWgtOK_p;
  Bool flagOK_p;
  Bool snrOK_p;
};

// <summary> 
// GJonesMBuf: GJones calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="CalMainBuffer">CalMainBuffer</linkto> module
// <li> <linkto class="SolvableVisJonesMBuf">SolvableVisJonesMBuf</linkto> module
// </prerequisite>
//
// <etymology>
// From "GJones matrix", "cal main table" and "buffer"
// </etymology>
//
// <synopsis>
// The GJonesMBuf class holds a buffer, optionally connected to a 
// GJones calibration main table iterator (of base type CalIterBase). 
// The GJones matrix contains electronic gain correction terms.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate GJones calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class GJonesMBuf : public SolvableVisJonesMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  GJonesMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  GJonesMBuf (const Vector<Int>& calIndices, 
	      const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  GJonesMBuf (CalIterBase& calIter);

  // Update the calibration solution in each of a set of buffer rows
  virtual Bool fillMatchingRows (const Vector<Int>& matchingRows,
				 const String& sFreqGrpName,
				 const Complex& sGain,
				 const MFrequency& sRefFreq,
				 const Int& sRefAnt);
 protected:

 private:
};

// <summary> 
// DJonesMBuf: DJones calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="CalMainBuffer">CalMainBuffer</linkto> module
// <li> <linkto class="SolvableVisJonesMBuf">SolvableVisJonesMBuf</linkto> module
// </prerequisite>
//
// <etymology>
// From "DJones matrix", "cal main table" and "buffer"
// </etymology>
//
// <synopsis>
// The DJonesMBuf class holds a buffer, optionally connected to a 
// DJones calibration main table iterator (of base type CalIterBase). 
// The DJones matrix contains instrumental polarization corrections.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate DJones calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class DJonesMBuf : public SolvableVisJonesMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  DJonesMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  DJonesMBuf (const Vector<Int>& calIndices, 
	      const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  DJonesMBuf (CalIterBase& calIter);

 protected:

 private:
};

// <summary> 
// TJonesMBuf: TJones calibration main table buffer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="CalMainBuffer">CalMainBuffer</linkto> module
// <li> <linkto class="SolvableVisJonesMBuf">SolvableVisJonesMBuf</linkto> module
// </prerequisite>
//
// <etymology>
// From "TJones matrix", "cal main table" and "buffer"
// </etymology>
//
// <synopsis>
// The TJonesMBuf class holds a buffer, optionally connected to a 
// TJones calibration main table iterator (of base type CalIterBase). 
// The TJones matrix contains atmospheric corrections.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Encapsulate TJones calibration main table data buffers.
// </motivation>
//
// <todo asof="01/08/01">
// (i) Deal with non-standard columns.
// </todo>

class TJonesMBuf : public SolvableVisJonesMBuf
{
 public:
  // Default constructor. No connection to an underlying
  // calibration table iterator in this case.
  TJonesMBuf();

  // Construct from a set of calibration buffer indices and
  // their specified values. Non-index columns will be set 
  // to default values, and there is no connection to an 
  // underlying calibration table iterator in this case.
  TJonesMBuf (const Vector<Int>& calIndices, 
	      const Block<Vector<Int> >& indexValues);

  // Construct from a calibration table iterator. The calibration
  // buffer will remain synchronized with the iterator.
  TJonesMBuf (CalIterBase& calIter);

 protected:

 private:
};


} //# NAMESPACE CASA - END

#endif
   
  



