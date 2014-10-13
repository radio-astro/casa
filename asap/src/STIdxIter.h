#ifndef _ASAP_INDEX_ITERATOR_H_ 
#define _ASAP_INDEX_ITERATOR_H_ 

#include <vector>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/IPosition.h>
#include <casa/BasicSL/String.h>

#include <casa/Utilities/Sort.h>

#include "Scantable.h"

using namespace std ;
using namespace casa ;

namespace {
vector<string> split(const string &str, char delim)
{
  vector<string> result;
  size_t current = 0;
  size_t found;
  while ((found = str.find_first_of(delim, current)) != string::npos) {
    result.push_back(string(str, current, found - current));
    current = found + 1;
  }
  result.push_back(string(str, current, str.size() - current));
  return result;
}
} // anonymous namespace

namespace asap {
class STIdxIter2
{ 
public:
  template<class T>
  static void Iterate(T &processor, const string cols_list)
  {
    vector<string> cols = split(cols_list, ',');
    // for (vector<string>::iterator i = cols.begin(); i != cols.end(); ++i)
    //   cout << *i << endl;
    STIdxIter2 iter(processor.target(), cols);
    STSelector sel ;
    while ( !iter.pastEnd() ) {
      const Record current = iter.currentValue() ;
      Vector<uInt> rows = iter.getRows( SHARE ) ;
      // any process
      processor.Process(cols, current, rows);
      // go next
      iter.next() ;
    }    
  }
  STIdxIter2() ;
  STIdxIter2( const string &name,
                             const vector<string> &cols ) ;
  STIdxIter2( const CountedPtr<Scantable> &s,
                          const vector<string> &cols ) ; 
  virtual ~STIdxIter2() ;
  Record currentValue();
  Bool pastEnd() ;
  void next() ;
  Vector<uInt> getRows(StorageInitPolicy policy=COPY) ;
  vector<uInt> getRowsSTL() { return tovector( getRows() ) ; } ;
  virtual void init();
private:
  vector<uInt> tovector(Vector<uInt> v);
  void addSortKey(const string &name);
  template<class T, DataType U> void addColumnToKey(const string &name);
  void addColumnToKeyTpString(const string &name);
  void deallocate();
  vector<string> cols_;
  Table table_;
  uInt counter_;
  uInt num_iter_;
  uInt num_row_;
  Sort sorter_;
  Vector<uInt> index_;
  Vector<uInt> unique_;
  vector<void*> pointer_;
  vector<Vector<String> > string_storage_;
} ;

} // namespace
#endif /* _ASAP_INDEX_ITERATOR_H_ */
