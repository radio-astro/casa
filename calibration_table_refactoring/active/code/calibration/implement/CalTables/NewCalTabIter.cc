//# NewCalTabIter.cc: Implementation of NewCalTabIter.h
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

#include <calibration/CalTables/NewCalTabIter.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Arrays.h>
#include <casa/OS/Timer.h>
#include <calibration/CalTables/NewCalTableEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

NewCalTabIter::NewCalTabIter(NewCalTable tab, const Block<String>& sortcol) :
  ti_(NULL),
  inct_(NULL),
  iMainCols_(NULL)
{
  //  cout << "Creating TableIterator..." << endl;

  ti_=new TableIterator(tab,sortcol);

  // Attach initial accessors:
  inct_= new NewCalTable(ti_->table());
  iMainCols_ = new NewCalMainColumns(*inct_);
  /*
  cout << "inct_->nrow() = " << inct_->nrow() << endl;
  cout << "this->nrow() = " << this->nrow() << endl;
  cout << "iMainCols_->spwId() = " << iMainCols_->spwId().getColumn() << endl;
  cout << "iMainCols_->spwId()(0) = " << iMainCols_->spwId()(0) << endl;
  cout << "spw0 = " << this->spw0() << endl;

  cout << "done." << endl;
  */
};

//----------------------------------------------------------------------------

NewCalTabIter::~NewCalTabIter()
{
  if (ti_!=NULL) delete ti_;
  if (iMainCols_!=NULL) delete iMainCols_;
  if (inct_!=NULL) delete inct_;
};

void NewCalTabIter::next() { 
  // Advance the TableIterator
  ti_->next();
  // Attach accessors:
  if (iMainCols_!=NULL) delete iMainCols_;
  if (inct_!=NULL) delete inct_;
  inct_= new NewCalTable(ti_->table());
  iMainCols_ = new NewCalMainColumns(*inct_);
};

Double NewCalTabIter::time0() { return iMainCols_->time()(0); };
Vector<Double> NewCalTabIter::time() { return iMainCols_->time().getColumn(); };
void NewCalTabIter::time(Vector<Double>& v) { iMainCols_->time().getColumn(v); };

Int NewCalTabIter::field0() { return iMainCols_->fieldId()(0); };
Vector<Int> NewCalTabIter::field() { return iMainCols_->fieldId().getColumn(); };
void NewCalTabIter::field(Vector<Int>& v) { iMainCols_->fieldId().getColumn(v); };

Int NewCalTabIter::spw0() { return iMainCols_->spwId()(0); };
Vector<Int> NewCalTabIter::spw() { return iMainCols_->spwId().getColumn(); };
void NewCalTabIter::spw(Vector<Int>& v) { iMainCols_->spwId().getColumn(v); };

Int NewCalTabIter::scan0() { return iMainCols_->scanNo()(0); };
Vector<Int> NewCalTabIter::scan() { return iMainCols_->scanNo().getColumn(); };
void NewCalTabIter::scan(Vector<Int>& v) { iMainCols_->scanNo().getColumn(v); };

Vector<Int> NewCalTabIter::antenna1() { return iMainCols_->antenna1().getColumn(); };
void NewCalTabIter::antenna1(Vector<Int>& v) { iMainCols_->antenna1().getColumn(v); };
Vector<Int> NewCalTabIter::antenna2() { return iMainCols_->antenna2().getColumn(); };
void NewCalTabIter::antenna2(Vector<Int>& v) { iMainCols_->antenna2().getColumn(v); };

Cube<Float> NewCalTabIter::param() { return iMainCols_->param().getColumn(); };
void NewCalTabIter::param(Cube<Float>& c) { iMainCols_->param().getColumn(c); };
Cube<Float> NewCalTabIter::paramErr() { return iMainCols_->paramerr().getColumn(); };
void NewCalTabIter::paramErr(Cube<Float>& c) { iMainCols_->paramerr().getColumn(c); };

Cube<Float> NewCalTabIter::snr() { return iMainCols_->snr().getColumn(); };
void NewCalTabIter::snr(Cube<Float>& c) { iMainCols_->snr().getColumn(c); };
Cube<Float> NewCalTabIter::wt() { return iMainCols_->weight().getColumn(); };
void NewCalTabIter::wt(Cube<Float>& c) { iMainCols_->weight().getColumn(c); };

Cube<Bool> NewCalTabIter::flag() { return iMainCols_->flag().getColumn(); };
void NewCalTabIter::flag(Cube<Bool>& c) { iMainCols_->flag().getColumn(c); };




} //# NAMESPACE CASA - END
