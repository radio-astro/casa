//# CTIter.h: Iterating interfaces for modern CalTables
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

#ifndef CALIBRATION_CALTABITER_H
#define CALIBRATION_CALTABITER_H

#include <casa/aips.h>
#include <tables/Tables/TableIter.h>
#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTMainColumns.h>
#include <synthesis/CalTables/CTColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// ROCTIter: Read-only iterating interface to New Calibration tables
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
// The ROCTIter class provide basic iterated access to the contents
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

class ROCTIter
{
public:
  // Constructor/Destructor 
  ROCTIter(NewCalTable tab,const Block<String>& sortcol);
  virtual ~ROCTIter();
  
  // Iteration operators
  inline void reset() { ti_->reset(); this->attach(); };
  inline Bool pastEnd() { return ti_->pastEnd(); };
  void next();
  void next0();

  // Return the current table iteration
  NewCalTable table() const { return NewCalTable(ti_->table()); };

  Int nrow() const { return ti_->table().nrow(); };

  // Column accessors
  // Those methods that return scalars for data coordinates 
  //  (e.g., thisTime(), thisField(), etc.) return the first element
  //  of the corresponding vector of values in the current 
  //  iteration.  This makes the most sense when the corresponding
  //  column is a sort column in the iteration, such that the contents
  //  of the vector is in fact a unique value.  (TBD: return -1 from
  //  scalar methods when column is not unique?)

  Double thisTime() const;
  Vector<Double> time() const;
  void time(Vector<Double>& v) const;

  Int thisField() const;
  Vector<Int> field() const;
  void field(Vector<Int>& v) const;

  Int thisSpw() const;
  Vector<Int> spw() const;
  void spw(Vector<Int>& v) const;

  Int thisScan() const;
  Vector<Int> scan() const;
  void scan(Vector<Int>& v) const;

  Int thisObs() const;
  Vector<Int> obs() const;
  void obs(Vector<Int>& v) const;

  Int thisAntenna1() const;
  Vector<Int> antenna1() const;
  void antenna1(Vector<Int>& v) const;
  Int thisAntenna2() const;
  Vector<Int> antenna2() const;
  void antenna2(Vector<Int>& v) const;

  Cube<Complex> cparam() const;
  void cparam(Cube<Complex>& c) const;

  Cube<Float> fparam() const;
  void fparam(Cube<Float>& c) const;

  Cube<Float> casfparam(String what="") const;
  void casfparam(Cube<Float>& casf, String what="") const;


  Cube<Float> paramErr() const;
  void paramErr(Cube<Float>& c) const;

  Cube<Float> snr() const;
  void snr(Cube<Float>& c) const;

  Cube<Float> wt() const;
  void wt(Cube<Float>& c) const;

  Cube<Bool> flag() const;
  void flag(Cube<Bool>& c) const;

  Int nchan() const;
  Vector<Int> chan() const;
  void chan(Vector<Int>& v) const;
  Vector<Double> freq() const;
  void freq(Vector<Double>& v) const;

 protected:

  // Attach accessors
  virtual void attach();
  
 private:

  // Prohibit public use of copy, assignment
  ROCTIter (const ROCTIter& other);
  ROCTIter& operator= (const ROCTIter& other);

  // Data:

  // Remember the sort columns...
  Vector<String> sortCols_;

  // If true, spw is unique per iteration, and it is
  //   safe to access channel axis info
  Bool singleSpw_;

  // The parent NewCalTable (Table) object 
  //  (stays in scope for the life of the CTIter)
  NewCalTable parentNCT_;

  // Access to subtables (e.g., for frequencies)
  ROCTColumns calCol_;

  // The underlying TableIterator
  TableIterator *ti_;

  // Per-iteration table
  NewCalTable *inct_;

  // Per-iteration columns
  ROCTMainColumns *iROCTMainCols_;


};

// Writable version (limited to certain 'columns')
class CTIter : public ROCTIter
{
public:
  // Constructor/Destructor 
  CTIter(NewCalTable tab,const Block<String>& sortcol);
  virtual ~CTIter();

  // Set fieldid
  void setfield(Int fieldid);

  // Set scan number
  void setscan(Int scan);

  // Set obsid
  void setobs(Int obs);

  // Set antenna2 (e.g., used for setting refant)
  void setantenna2(const Vector<Int>& a2);

  // Set the flags
  void setflag(const Cube<Bool>& flag);

  // Set the parameters
  void setfparam(const Cube<Float>& f);
  void setcparam(const Cube<Complex>& c);

protected:

  // Attach writable column access
  virtual void attach();

private:

  // Prohibit public use of copy, assignment
  CTIter (const CTIter& other);
  CTIter& operator= (const CTIter& other);

  // Per-iteration table
  NewCalTable *irwnct_;

  // Writable column access
  CTMainColumns *iRWCTMainCols_;

};


} //# NAMESPACE CASA - END

#endif
