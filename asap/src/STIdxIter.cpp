#include <assert.h>
#include <iostream>
#include <casa/Utilities/GenSort.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Utilities/DataType.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableRecord.h>
#include "STIdxIter.h"

namespace asap {
STIdxIter2::STIdxIter2()
  : cols_(),
    table_(),
    counter_(0),
    num_iter_(0),
    num_row_(0),
    sorter_(),
    index_(),
    unique_(),
    pointer_(),
    string_storage_()
{
}

STIdxIter2::STIdxIter2( const string &name,
                      const vector<string> &cols )
  : cols_(cols),
    table_(name, Table::Old),
    counter_(0),
    num_iter_(0),
    num_row_(0),
    sorter_(),
    index_(),
    unique_(),
    pointer_(),
    string_storage_()
{
  init();
}

STIdxIter2::STIdxIter2( const CountedPtr<Scantable> &s,
                      const vector<string> &cols )
  : cols_(cols),
    table_(s->table()),
    counter_(0),
    num_iter_(0),
    num_row_(0),
    sorter_(),
    index_(),
    unique_(),
    pointer_(),
    string_storage_()
{
  init();
}

STIdxIter2::~STIdxIter2()
{
  deallocate();
}

void STIdxIter2::deallocate()
{
  for (vector<void*>::iterator i = pointer_.begin(); i != pointer_.end(); ++i) {
    free(*i);
  }
}

Record STIdxIter2::currentValue() {
  assert(counter_ < num_iter_);
  Vector<String> cols(cols_.size());
  for (uInt i = 0; i < cols.nelements(); ++i) {
    cols[i] = cols_[i];
  }
  const ROTableRow row(table_, cols);
  const TableRecord rec = row.get(index_[unique_[counter_]]);
  return Record(rec);
}

Bool STIdxIter2::pastEnd() {
  return counter_ >= num_iter_;
}

void STIdxIter2::next() {
  counter_++;
}

vector<uInt> STIdxIter2::tovector( Vector<uInt> v )
{
  vector<uInt> ret ;
  v.tovector( ret ) ;
  return ret ;
}

Vector<uInt> STIdxIter2::getRows( StorageInitPolicy policy )
{
  assert(num_iter_ >= 1);
  assert(counter_ < num_iter_);
  if (counter_ == num_iter_ - 1) {
    uInt start = unique_[counter_];
    uInt num_row = num_row_ - start;
    Vector<uInt> rows(IPosition(1, num_row), &(index_.data()[start]), policy);
    return rows;
  }
  else {
    uInt start = unique_[counter_];
    uInt end = unique_[counter_ + 1];
    uInt num_row = end - start;
    Vector<uInt> rows(IPosition(1, num_row), &(index_.data()[start]), policy);
    return rows;
  }
} 

void STIdxIter2::init()
{
  num_row_ = table_.nrow();
  for (uInt i = 0; i < cols_.size(); ++i) {
    addSortKey(cols_[i]);
  }
  sorter_.sort(index_, num_row_);
  num_iter_ = sorter_.unique(unique_, index_);
  // cout << "num_row_ = " << num_row_ << endl
  //      << "num_iter_ = " << num_iter_ << endl;
  // cout << "unique_ = " << unique_ << endl;
  // cout << "index_ = " << index_ << endl;
}

void STIdxIter2::addSortKey(const string &name)
{
  const ColumnDesc &desc = table_.tableDesc().columnDesc(name);
  const DataType dtype = desc.trueDataType();
  switch (dtype) {
  case TpUInt:
    addColumnToKey<uInt, TpUInt>(name);
    break;
  case TpInt:
    addColumnToKey<Int, TpInt>(name);
    break;
  case TpFloat:
    addColumnToKey<Float, TpFloat>(name);
    break;
  case TpDouble:
    addColumnToKey<Double, TpDouble>(name);
    break;
  case TpComplex:
    addColumnToKey<Complex, TpComplex>(name);
    break;
  case TpString:
    addColumnToKeyTpString(name);
    break;
  default:
    deallocate();
    stringstream oss;
    oss << name << ": data type is not supported" << endl;
    throw(AipsError(oss.str()));
  }
}

template<class T, DataType U>
void STIdxIter2::addColumnToKey(const string &name)
{
  void *raw_storage = malloc(sizeof(T) * num_row_);
  T *storage = reinterpret_cast<T*>(raw_storage);
  Vector<T> array(IPosition(1, num_row_), storage, SHARE); 
  ROScalarColumn<T> col(table_, name);
  col.getColumn(array);
  sorter_.sortKey(storage, U, 0, Sort::Ascending);
  pointer_.push_back(raw_storage);
}

void STIdxIter2::addColumnToKeyTpString(const string &name)
{
  ROScalarColumn<String> col(table_, name);
  String *storage = new String[num_row_];
  Vector<String> array(IPosition(1, num_row_), storage, TAKE_OVER);
  col.getColumn(array);
  sorter_.sortKey(storage, TpString, 0, Sort::Ascending);
  string_storage_.push_back(array);
}

} // namespace

