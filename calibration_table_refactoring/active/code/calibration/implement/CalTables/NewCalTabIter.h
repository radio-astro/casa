//# NewCalTabIter.h: Iterating interface for NewCalTables
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#ifndef CALIBRATION_NEWCALTABITER_H
#define CALIBRATION_NEWCALTABITER_H

#include <casa/aips.h>
#include <tables/Tables/TableIter.h>
#include <calibration/CalTables/NewCalTable.h>
#include <calibration/CalTables/NewCalMainColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// NewCalTabIter: Iterating interface to New Calibration tables
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTableDesc">NewCalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration", "table", and "iteration".
// </etymology>
//
// <synopsis>
// The NewCalTabIter class provide basic iterated access to the contents
// of a NewCalTable.  The iteration is initialized according to sort 
// columns specified in the constructor, and access to the contents
// of each iteration is provided via a NewCalTable object (referencing
// the iterated subset of the original NewCalTable) or methods that return
// Vectors or Arrays of the contents of specific columns.  For the latter,
// the Vectors/Arrays are generated only on-demand.
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// This class is used by calibration table clients to access calibration tables
//  in a piecemeal fashion, e.g., for plotting, interpolation, etc.
// </motivation>
//
// <todo asof="11/06/10">
// </todo>

class NewCalTabIter
{
public:
  // Constructor/Destructor 
  NewCalTabIter(NewCalTable tab,const Block<String>& sortcol);
  ~NewCalTabIter();
  
  // Iteration operators
  inline void reset() { ti_->reset(); };
  inline Bool pastEnd() { return ti_->pastEnd(); };
  void next();

  // Return the current table iteration
  NewCalTable table() { return NewCalTable(ti_->table()); };

  Int nrow() { return ti_->table().nrow(); };

  // Column accessors
  // Those methods that return scalars for data coordinates 
  //  (e.g., time0(), field0(), etc.) return the first element
  //  of the corresponding vector of values in the current 
  //  iteration.  This makes the most sense when the corresponding
  //  column is a sort column in the iteration, such that the contents
  //  of the vector is in fact a unique value.  (TBD: return -1 from
  //  scalar methods when column is not unique?)

  Double time0();
  Vector<Double> time();
  void time(Vector<Double>& v);

  Int field0();
  Vector<Int> field();
  void field(Vector<Int>& v);

  Int spw0();
  Vector<Int> spw();
  void spw(Vector<Int>& v);

  Int scan0();
  Vector<Int> scan();
  void scan(Vector<Int>& v);

  // TBD: scalar versions
  Vector<Int> antenna1();
  void antenna1(Vector<Int>& v);
  Vector<Int> antenna2();
  void antenna2(Vector<Int>& v);

  // TBD: support for Complex...
  Cube<Float> param();
  void param(Cube<Float>& c);
  Cube<Float> paramErr();
  void paramErr(Cube<Float>& c);

  Cube<Float> snr();
  void snr(Cube<Float>& c);

  Cube<Float> wt();
  void wt(Cube<Float>& c);

  Cube<Bool> flag();
  void flag(Cube<Bool>& c);

 private:

  // Prohibit public use of copy, assignment
  NewCalTabIter (const NewCalTabIter& other);
  NewCalTable& operator= (const NewCalTable& other);

  // Data:

  // The underlying TableIterator
  TableIterator *ti_;

  // Per-iteration table
  NewCalTable *inct_;

  // Per-iteration columns
  NewCalMainColumns *iMainCols_;


};


} //# NAMESPACE CASA - END

#endif
