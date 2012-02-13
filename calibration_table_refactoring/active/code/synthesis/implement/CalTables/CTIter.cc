//# CTIter.cc: Implementation of CTIter.h
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
//# $Id: NewCalTable.cc 15602 2011-07-14 00:03:34Z tak.tsutsumi $
//----------------------------------------------------------------------------

#include <synthesis/CalTables/CTIter.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Arrays.h>
#include <casa/OS/Timer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

ROCTIter::ROCTIter(NewCalTable tab, const Block<String>& sortcol) :
  sortCols_(sortcol,sortcol.nelements()),
  singleSpw_(False),
  calCol_(tab),
  ti_(NULL),
  inct_(NULL),
  iMainCols_(NULL)
{

  ti_=new TableIterator(tab,sortcol);

  // Attach initial accessors:
  attach();

  //  cout << "CTIter sort columns (" << sortCols_.nelements() << "): " << sortCols_ << endl;

  // If SPW a sort column, then 
  singleSpw_=anyEQ(sortCols_,String("SPECTRAL_WINDOW_ID"));
  
  /*
  cout << "singleSpw_ = " << boolalpha << singleSpw_ << endl;
  cout << "inct_->nrow() = " << inct_->nrow() << endl;
  cout << "this->nrow() = " << this->nrow() << endl;
  cout << "iMainCols_->spwId() = " << iMainCols_->spwId().getColumn() << endl;
  cout << "iMainCols_->spwId()(0) = " << iMainCols_->spwId()(0) << endl;
  cout << "thisSpw() = " << this->thisSpw() << endl;

  cout << "done." << endl;
  */
};

//----------------------------------------------------------------------------

ROCTIter::~ROCTIter()
{
  if (ti_!=NULL) delete ti_;
  if (iMainCols_!=NULL) delete iMainCols_;
  if (inct_!=NULL) delete inct_;
};

void ROCTIter::next() { 
  // Advance the TableIterator
  ti_->next();

  this->attach();

};

Double ROCTIter::thisTime() const { return iMainCols_->time()(0); };
Vector<Double> ROCTIter::time() const { return iMainCols_->time().getColumn(); };
void ROCTIter::time(Vector<Double>& v) const { iMainCols_->time().getColumn(v); };

Int ROCTIter::thisField() const { return iMainCols_->fieldId()(0); };
Vector<Int> ROCTIter::field() const { return iMainCols_->fieldId().getColumn(); };
void ROCTIter::field(Vector<Int>& v) const { iMainCols_->fieldId().getColumn(v); };

Int ROCTIter::thisSpw() const { return iMainCols_->spwId()(0); };
Vector<Int> ROCTIter::spw() const { return iMainCols_->spwId().getColumn(); };
void ROCTIter::spw(Vector<Int>& v) const { iMainCols_->spwId().getColumn(v); };

Int ROCTIter::thisScan() const { return iMainCols_->scanNo()(0); };
Vector<Int> ROCTIter::scan() const { return iMainCols_->scanNo().getColumn(); };
void ROCTIter::scan(Vector<Int>& v) const { iMainCols_->scanNo().getColumn(v); };

Int ROCTIter::thisAntenna1() const { return iMainCols_->antenna1()(0); };
Vector<Int> ROCTIter::antenna1() const { return iMainCols_->antenna1().getColumn(); };
void ROCTIter::antenna1(Vector<Int>& v) const { iMainCols_->antenna1().getColumn(v); };
Int ROCTIter::thisAntenna2() const { return iMainCols_->antenna2()(0); };
Vector<Int> ROCTIter::antenna2() const { return iMainCols_->antenna2().getColumn(); };
void ROCTIter::antenna2(Vector<Int>& v) const { iMainCols_->antenna2().getColumn(v); };

Cube<Complex> ROCTIter::param() const { return iMainCols_->param().getColumn(); };
void ROCTIter::param(Cube<Complex>& c) const { iMainCols_->param().getColumn(c); };
Cube<Float> ROCTIter::paramErr() const { return iMainCols_->paramerr().getColumn(); };
void ROCTIter::paramErr(Cube<Float>& c) const { iMainCols_->paramerr().getColumn(c); };

Cube<Float> ROCTIter::snr() const { return iMainCols_->snr().getColumn(); };
void ROCTIter::snr(Cube<Float>& c) const { iMainCols_->snr().getColumn(c); };
Cube<Float> ROCTIter::wt() const { return iMainCols_->weight().getColumn(); };
void ROCTIter::wt(Cube<Float>& c) const { iMainCols_->weight().getColumn(c); };

Cube<Bool> ROCTIter::flag() const { return iMainCols_->flag().getColumn(); };
void ROCTIter::flag(Cube<Bool>& c) const { iMainCols_->flag().getColumn(c); };

Vector<Int> ROCTIter::chan() const {
  Vector<Int> chans;
  this->chan(chans);
  return chans;
}
void ROCTIter::chan(Vector<Int>& v) const {
  if (singleSpw_) {
    v.resize(calCol_.spectralWindow().numChan()(this->thisSpw()));
    // TBD: observe channel selection here:
    indgen(v);
  }
  else
    // more than one spw per iteration...
    throw(AipsError("Please sort by spw."));
}

Vector<Double> ROCTIter::freq() const {
  Vector<Double> freqs;
  this->freq(freqs);
  return freqs;
}

void ROCTIter::freq(Vector<Double>& v) const {
  if (singleSpw_) {
    v.resize();
    calCol_.spectralWindow().chanFreq().get(this->thisSpw(),v);
  }
  else
    // more than one spw per iteration...
    throw(AipsError("Please sort by spw."));
}

void ROCTIter::attach() {
  // Attach accessors:
  if (iMainCols_!=NULL) delete iMainCols_;
  if (inct_!=NULL) delete inct_;
  inct_= new NewCalTable(ti_->table());
  iMainCols_ = new ROCTMainColumns(*inct_);
}


} //# NAMESPACE CASA - END
