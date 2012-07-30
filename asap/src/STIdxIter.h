#ifndef _ASAP_INDEX_ITERATOR_H_ 
#define _ASAP_INDEX_ITERATOR_H_ 

#include <vector>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/IPosition.h>
#include <casa/BasicSL/String.h>

#include "Scantable.h"

using namespace std ;
using namespace casa ;

namespace asap {
class IndexIterator 
{
public:
  IndexIterator( IPosition &shape ) ;
  Block<uInt> current() { return idx_m ; } ;
  Bool pastEnd() ;
  void next() ;
private:
  uInt nfield_m ;
  Block<uInt> prod_m ;
  Block<uInt> idx_m ;
  uInt niter_m ;
  uInt maxiter_m ;
} ;

class ArrayIndexIterator
{
public:
  ArrayIndexIterator( Matrix<uInt> &arr, 
                      vector< vector<uInt> > idlist=vector< vector<uInt> >() ) ;
  virtual ~ArrayIndexIterator() ;
  Vector<uInt> current() ;
  Bool pastEnd() ;
  virtual void next() = 0 ;
  virtual Vector<uInt> getRows( StorageInitPolicy policy=COPY ) = 0 ;
protected:
  IndexIterator *iter_m ;
  uInt nrow_m ;
  uInt ncol_m ;
  Block<uInt> storage_m ;
  Matrix<uInt> arr_m ;
  IPosition pos_m ;
  Vector<uInt> current_m ;
  vector< vector<uInt> > idxlist_m ;
} ;

class ArrayIndexIteratorNormal : public ArrayIndexIterator
{
public:
  ArrayIndexIteratorNormal( Matrix<uInt> &arr, 
                            vector< vector<uInt> > idlist=vector< vector<uInt> >() ) ;
  void next() ;
  Vector<uInt> getRows( StorageInitPolicy policy=COPY ) ;
} ;

class ArrayIndexIteratorAcc : public ArrayIndexIterator
{
public:
  ArrayIndexIteratorAcc( Matrix<uInt> &arr, 
                         vector< vector<uInt> > idlist=vector< vector<uInt> >() ) ;
  void next() ;
  Vector<uInt> getRows( StorageInitPolicy policy=COPY ) ;
private:
  Int isChanged( Block<uInt> &idx ) ;
  uInt *updateStorage( Int &icol, uInt *base, uInt &v ) ;

  Block<uInt> prev_m ;
  Block<uInt> len_m ;
  Block<Bool> skip_m ;
} ;

class STIdxIter
{ 
public:
  STIdxIter() ;
  STIdxIter( const string &name,
                             const vector<string> &cols ) ;
  STIdxIter( const CountedPtr<Scantable> &s,
                          const vector<string> &cols ) ; 
  virtual ~STIdxIter() ;
  vector<uInt> currentSTL() { return tovector( iter_m->current() ) ; } ;
  Vector<uInt> current() { return iter_m->current() ; } ;
  Bool pastEnd() { return iter_m->pastEnd() ; } ;
  void next() { iter_m->next() ; } ;
  vector<uInt> getRowsSTL() { return tovector( iter_m->getRows() ) ; } ;
  // !!!you should not use policy=TAKE_OVER since it causes problem!!!
  Vector<uInt> getRows( StorageInitPolicy policy=COPY ) ;
protected:
  ArrayIndexIterator *iter_m ;
  virtual void init( Table &t,
                     const vector<string> &cols ) = 0 ;
private:
  vector<uInt> tovector( Vector<uInt> v ) ;
} ;

class STIdxIterNormal : public STIdxIter
{
public:
  STIdxIterNormal() ;
  STIdxIterNormal( const string &name,
                   const vector<string> &cols ) ;
  STIdxIterNormal( const CountedPtr<Scantable> &s,
                   const vector<string> &cols ) ;
  ~STIdxIterNormal() ;
protected:
  void init( Table &t,
             const vector<string> &cols ) ;
} ;

class STIdxIterAcc : public STIdxIter
{
public:
  STIdxIterAcc() ;
  STIdxIterAcc( const string &name,
                const vector<string> &cols ) ;
  STIdxIterAcc( const CountedPtr<Scantable> &s,
                const vector<string> &cols ) ;
  ~STIdxIterAcc() ;
protected:
  virtual void init( Table &t,
                     const vector<string> &cols ) ;
} ;

class STIdxIterExAcc : public STIdxIter
{
public:
  STIdxIterExAcc() ;
  STIdxIterExAcc( const string &name,
                  const vector<string> &cols ) ;
  STIdxIterExAcc( const CountedPtr<Scantable> &s,
                  const vector<string> &cols ) ;
  ~STIdxIterExAcc() ;
  Int getSrcType() ;
  String getSrcName() ;
protected:
  virtual void init( Table &t,
                     const vector<string> &cols ) ;
private:
  void processIntCol( Vector<Int> &in,
                      Vector<uInt> &out,
                      Block<Int> &val ) ;
  void processStrCol( Vector<String> &in,
                      Vector<uInt> &out,
                      Block<String> &val ) ;
  Block<Int> srctype_m ;
  Block<String> srcname_m ;
  Int srctypeid_m ;
  Int srcnameid_m ;
} ;

} // namespace
#endif /* _ASAP_INDEX_ITERATOR_H_ */
