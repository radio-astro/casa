#include <iostream>
#include <casa/Utilities/GenSort.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <tables/Tables/ScalarColumn.h>
#include "STIdxIter.h"

namespace asap {

IndexIterator::IndexIterator( IPosition &shape ) 
  : niter_m( 0 )
{
  nfield_m = shape.nelements() ;
  prod_m.resize( nfield_m ) ;
  idx_m.resize( nfield_m ) ;
  prod_m[nfield_m-1] = 1 ;
  idx_m[nfield_m-1] = 0 ;
  for ( Int i = nfield_m-2 ; i >= 0 ; i-- ) {
    prod_m[i] = shape[i+1] * prod_m[i+1] ;
    idx_m[i] = 0 ;
  }
  maxiter_m = prod_m[0] * shape[0] ;
//   cout << "maxiter_m=" << maxiter_m << endl ;
}

Bool IndexIterator::pastEnd() 
{
  return niter_m >= maxiter_m ;
}

void IndexIterator::next()
{
  niter_m++ ;
  uInt x = niter_m ;
  for ( Int i = 0 ; i < nfield_m ; i++ ) {
    idx_m[i] = x / prod_m[i] ;
    //cout << "i=" << i << ": prod=" << prod_m[i] 
    //     << ", idx=" << idx_m[i] << endl ;
    x %= prod_m[i] ;
  }
}

ArrayIndexIterator::ArrayIndexIterator( Matrix<uInt> &arr,
                                        vector< vector<uInt> > idlist )
  : arr_m( arr ),
    pos_m( 1 )
{
  ncol_m = arr_m.ncolumn() ;
  nrow_m = arr_m.nrow() ;
  vector< vector<uInt> > l = idlist ;
  if ( l.size() != ncol_m ) {
    l.resize( ncol_m ) ;
    for ( uInt i = 0 ; i < ncol_m ; i++ ) {
      Vector<uInt> a( arr_m.column( i ).copy() ) ;
      uInt n = genSort( a, Sort::Ascending, Sort::QuickSort|Sort::NoDuplicates ) ;
      a.resize(n,True) ;
      a.tovector( l[i] ) ;
    }
  }
  idxlist_m = l ;
  IPosition shape( ncol_m ) ;
  for ( uInt i = 0 ; i < ncol_m ; i++ ) {
    shape[i] = idxlist_m[i].size() ;
  }
//   cout << "shape=" << shape << endl ;
  iter_m = new IndexIterator( shape ) ;
  current_m.resize( ncol_m ) ;
}

ArrayIndexIterator::~ArrayIndexIterator()
{
  delete iter_m ;
}

Vector<uInt> ArrayIndexIterator::current() 
{
  Block<uInt> idx = iter_m->current() ;
  for ( uInt i = 0 ; i < ncol_m ; i++ )
    current_m[i] = idxlist_m[i][idx[i]] ;
  return current_m ;
}

Bool ArrayIndexIterator::pastEnd()
{
  return iter_m->pastEnd() ;
}

ArrayIndexIteratorNormal::ArrayIndexIteratorNormal( Matrix<uInt> &arr,
                                                    vector< vector<uInt> > idlist )
  : ArrayIndexIterator( arr, idlist )
{
  storage_m.resize( nrow_m ) ;
}

void ArrayIndexIteratorNormal::next() 
{
  iter_m->next() ;
}

Vector<uInt> ArrayIndexIteratorNormal::getRows( StorageInitPolicy policy )
{
  Vector<uInt> v = current() ;
  uInt len = 0 ;
  uInt *p = storage_m.storage() ;
  for ( uInt i = 0 ; i < nrow_m ; i++ ) {
    if ( allEQ( v, arr_m.row( i ) ) ) {
      *p = i ;
      len++ ;
      p++ ;
    }
  }
  pos_m[0] = len ;
  p = storage_m.storage() ;
  return Vector<uInt>( pos_m, p, policy ) ;
}

ArrayIndexIteratorAcc::ArrayIndexIteratorAcc( Matrix<uInt> &arr,
                                              vector< vector<uInt> > idlist )
  : ArrayIndexIterator( arr, idlist )
{
  // storage_m layout
  // length: ncol_m * (nrow_m + 1)
  // 0~nrow_m-1: constant temporary storage that indicates whole rows
  // nrow_m~2*nrow_m-1: temporary storage for column 0
  // 2*nrow_m~3*nrow_m-1: temporary storage for column 1
  // ...
  storage_m.resize( arr_m.nelements()+nrow_m ) ;
  // initialize constant temporary storage
  uInt *p = storage_m.storage() ;
  for ( uInt i = 0 ; i < nrow_m ; i++ ) {
    *p = i ;
    p++ ;
  }
  // len_m layout
  // len[0]: length of temporary storage that incidates whole rows (constant)
  // len[1]: number of matched row for column 0 selection
  // len[2]: number of matched row for column 1 selection
  // ...
  len_m.resize( ncol_m+1 ) ;
  p = len_m.storage() ;
  for ( uInt i = 0 ; i < ncol_m+1 ; i++ ) {
    *p = nrow_m ;
    p++ ;
//     cout << "len[" << i << "]=" << len_m[i] << endl ;
  }
  // skip_m layout
  // skip_m[0]: True if column 0 is filled by unique value
  // skip_m[1]: True if column 1 is filled by unique value
  // ...
  skip_m.resize( ncol_m ) ;
  for ( uInt i = 0 ; i < ncol_m ; i++ ) {
    skip_m[i] = (Bool)(idxlist_m[i].size()==1) ;
//     cout << "skip_m[" << i << "]=" << skip_m[i] << endl ;
  }
  prev_m = iter_m->current() ;
  p = prev_m.storage() ;
  for ( uInt i = 0 ; i < ncol_m ; i++ ) {
    *p = *p - 1 ;
    p++ ;
  }
//   cout << "prev_m=" << Vector<uInt>(IPosition(1,ncol_m),prev_m.storage()) << endl ;
}

void ArrayIndexIteratorAcc::next() 
{
  prev_m = iter_m->current() ;
  iter_m->next() ;
}

Vector<uInt> ArrayIndexIteratorAcc::getRows( StorageInitPolicy policy )
{
  Block<uInt> v = iter_m->current() ;
  Int c = isChanged( v ) ;
//   cout << "v=" << Vector<uInt>(IPosition(1,v.nelements()),v.storage(),SHARE) << endl ;
//   cout << "c=" << c << endl ;
  if ( c > ncol_m-1 ) {
    pos_m[0] = len_m[ncol_m] ;
    Int offset = ncol_m - 1 ;
    while( offset >= 0 && skip_m[offset] )
      offset-- ;
    offset++ ;
//     cout << "offset=" << offset << endl ;
    return Vector<uInt>( pos_m, storage_m.storage()+offset*nrow_m, policy ) ;
  }
  Int offset = c - 1 ;
  while( offset >= 0 && skip_m[offset] )
    offset-- ;
  offset++ ;
//   cout << "offset = " << offset << endl ;
  uInt *base = storage_m.storage() + offset * nrow_m ;
//   cout << "len_m[c+1]=" << len_m[c+1] << endl ;
//   cout << "base=" << Vector<uInt>(IPosition(1,len_m[c+1]),base,SHARE) << endl ;
  for ( Int i = c ; i < ncol_m ; i++ ) {
    base = updateStorage( i, base, idxlist_m[i][v[i]] ) ;
//     cout << "len_m[" << i << "]=" << len_m[i] << endl ;
//     cout << "base=" << Vector<uInt>(IPosition(1,len_m[i]),base,SHARE) << endl ;
  }
  pos_m[0] = len_m[ncol_m] ;
//   cout << "pos_m=" << pos_m << endl ;
//   cout << "ret=" << Vector<uInt>( pos_m, base, policy ) << endl ;
  return Vector<uInt>( pos_m, base, policy ) ;
}

Int ArrayIndexIteratorAcc::isChanged( Block<uInt> &idx )
{
  Int i = 0 ;
  while( i < ncol_m && idx[i] == prev_m[i] )
    i++ ;
  return i ;
}

uInt *ArrayIndexIteratorAcc::updateStorage( Int &icol,
                                            uInt *base,
                                            uInt &v )
{
  // CAUTION: 
  // indexes for storage_m and len_m differ from index for skip_m by 1
  // (skip_m[0] corresponds to storage_m[1] and len_m[1]) since there 
  // is additional temporary storage at first segment in storage_m 
  uInt *p ;
  if ( skip_m[icol] ) {
    // skip update, just update len_m[icol] and pass appropriate pointer
//     cout << "skip " << icol << endl ;
    p = base ;
    len_m[icol+1] = len_m[icol] ;
  }
  else {
//     cout << "update " << icol << endl ;
    uInt len = 0 ;
    p = storage_m.storage() + (icol+1) * nrow_m ;
    uInt *work = p ;
    Bool b ;
    const uInt *arr_p = arr_m.getStorage( b ) ;
    long offset = 0 ;
    // warr_p points a first element of (icol)-th column
    const uInt *warr_p = arr_p + icol * nrow_m ;
    for ( uInt i = 0 ; i < len_m[icol] ; i++ ) {
      // increment warr_p by (*(base)-*(base-1))
      warr_p += *base - offset ;
      // check if target element is equal to value specified
      if ( *warr_p == v ) {
        // then, add current index to storage_m
        //       cout << "add " << *base << endl ;
        *work = *base ;
        len++ ;
        work++ ;
      }
      // update offset 
      offset = *base ;
      // next index
      base++ ;
    }
    arr_m.freeStorage( arr_p, b ) ;
    len_m[icol+1] = len ;
  }
  return p ;
}

STIdxIter::STIdxIter()
{
  iter_m = 0 ;
}

STIdxIter::STIdxIter( const string &name,
                      const vector<string> &cols ) 
{
}

STIdxIter::STIdxIter( const CountedPtr<Scantable> &s,
                      const vector<string> &cols )
{
}

STIdxIter::~STIdxIter()
{
  if ( iter_m != 0 )
    delete iter_m ;
}

vector<uInt> STIdxIter::tovector( Vector<uInt> v )
{
  vector<uInt> ret ;
  v.tovector( ret ) ;
  return ret ;
}

Vector<uInt> STIdxIter::getRows( StorageInitPolicy policy )
{ 
  return iter_m->getRows( policy ) ; 
} 

STIdxIterNormal::STIdxIterNormal()
  : STIdxIter()
{ 
}

STIdxIterNormal::STIdxIterNormal( const string &name,
                                  const vector<string> &cols ) 
  : STIdxIter( name, cols )
{
  Table t( name, Table::Old ) ;
  init( t, cols ) ;
}

STIdxIterNormal::STIdxIterNormal( const CountedPtr<Scantable> &s,
                                  const vector<string> &cols )
  : STIdxIter( s, cols )
{
  init( s->table(), cols ) ;
}

STIdxIterNormal::~STIdxIterNormal()
{
}

void STIdxIterNormal::init( Table &t, 
                            const vector<string> &cols )
{
  uInt ncol = cols.size() ;
  uInt nrow = t.nrow() ;
  Matrix<uInt> arr( nrow, ncol ) ;
  ROScalarColumn<uInt> col ;
  Vector<uInt> v ;
  for ( uInt i = 0 ; i < ncol ; i++ ) {
    col.attach( t, cols[i] ) ;
    v.reference( arr.column( i ) ) ;
    col.getColumn( v ) ;
  }
  iter_m = new ArrayIndexIteratorNormal( arr ) ;
}

STIdxIterAcc::STIdxIterAcc()
  : STIdxIter()
{ 
}

STIdxIterAcc::STIdxIterAcc( const string &name,
                            const vector<string> &cols ) 
  : STIdxIter( name, cols )
{
  Table t( name, Table::Old ) ;
  init( t, cols ) ;
}

STIdxIterAcc::STIdxIterAcc( const CountedPtr<Scantable> &s,
                            const vector<string> &cols )
  : STIdxIter( s, cols )
{
  init( s->table(), cols ) ;
}

STIdxIterAcc::~STIdxIterAcc()
{
}

void STIdxIterAcc::init( Table &t, 
                         const vector<string> &cols )
{
  uInt ncol = cols.size() ;
  uInt nrow = t.nrow() ;
  // array shape here is as follows if cols=["BEAMNO","POLNO","IFNO"]:
  // [[B0,B1,B2,...,BN],
  //  [P0,P1,P2,...,PN],
  //  [I0,I1,I2,...,IN]]
  // order of internal storage is
  // [B0,B1,B2,..,BN,P0,P1,P2,...,PN,I0,I1,I2,...,IN] 
  Matrix<uInt> arr( nrow, ncol ) ;
  Vector<uInt> v ;
  ROScalarColumn<uInt> col ;
  for ( uInt i = 0 ; i < ncol ; i++ ) {
    col.attach( t, cols[i] ) ;
    v.reference( arr.column( i ) ) ;
    col.getColumn( v ) ;
  }
  iter_m = new ArrayIndexIteratorAcc( arr ) ;
}

STIdxIterExAcc::STIdxIterExAcc()
  : STIdxIter(),
    srctypeid_m( -1 ),
    srcnameid_m( -1 )
{ 
}

STIdxIterExAcc::STIdxIterExAcc( const string &name,
                                const vector<string> &cols ) 
  : STIdxIter( name, cols ),
    srctypeid_m( -1 ),
    srcnameid_m( -1 )
{
  Table t( name, Table::Old ) ;
  init( t, cols ) ;
}

STIdxIterExAcc::STIdxIterExAcc( const CountedPtr<Scantable> &s,
                                const vector<string> &cols )
  : STIdxIter( s, cols ),
    srctypeid_m( -1 ),
    srcnameid_m( -1 )
{
  init( s->table(), cols ) ;
}

STIdxIterExAcc::~STIdxIterExAcc()
{
}

void STIdxIterExAcc::init( Table &t, 
                         const vector<string> &cols )
{
  uInt ncol = cols.size() ;
  uInt nrow = t.nrow() ;
  // array shape here is as follows if cols=["BEAMNO","POLNO","IFNO"]:
  // [[B0,B1,B2,...,BN],
  //  [P0,P1,P2,...,PN],
  //  [I0,I1,I2,...,IN]]
  // order of internal storage is
  // [B0,B1,B2,..,BN,P0,P1,P2,...,PN,I0,I1,I2,...,IN] 
  Matrix<uInt> arr( nrow, ncol ) ;
  Vector<uInt> v ;
  ROScalarColumn<uInt> col ;
  ROScalarColumn<String> strCol ;
  ROScalarColumn<Int> intCol ;
  for ( uInt i = 0 ; i < ncol ; i++ ) {
    v.reference( arr.column( i ) ) ;
    if ( cols[i] == "SRCTYPE" ) {
      intCol.attach( t, cols[i] ) ;
      Vector<Int> srctype = intCol.getColumn() ;
      processIntCol( srctype, v, srctype_m ) ;
      srctypeid_m = i ;
    }
    else if ( cols[i] == "SRCNAME" ) {
      strCol.attach( t, cols[i] ) ;
      Vector<String> srcname = strCol.getColumn() ;
      processStrCol( srcname, v, srcname_m ) ;
      srcnameid_m = i ;
    }
    else {
      col.attach( t, cols[i] ) ;
      col.getColumn( v ) ;
    }
  }
  iter_m = new ArrayIndexIteratorAcc( arr ) ;
}

void STIdxIterExAcc::processIntCol( Vector<Int> &in,
                                    Vector<uInt> &out,
                                    Block<Int> &val )
{
  convertArray( out, in ) ;
} 

void STIdxIterExAcc::processStrCol( Vector<String> &in,
                                    Vector<uInt> &out,
                                    Block<String> &val )
{
  uInt len = in.nelements() ;
  Vector<String> tmp = in.copy() ;
  uInt n = genSort( tmp, Sort::Ascending, Sort::QuickSort|Sort::NoDuplicates ) ;
  val.resize( n ) ;
  for ( uInt i = 0 ; i < n ; i++ ) {
    val[i] = tmp[i] ;
//     cout << "val[" << i << "]=" << val[i] << endl ;
  }
  if ( n == 1 ) {
    //cout << "n=1" << endl ;
    out = 0 ;
  }
  else if ( n == 2 ) {
    //cout << "n=2" << endl ;
    for ( uInt i = 0 ; i < len ; i++ ) {
      out[i] = (in[i] == val[0]) ? 0 : 1 ;
    }
  }
  else {
    //cout << "n=" << n << endl ;
    map<String,uInt> m ;
    for ( uInt i = 0 ; i < n ; i++ )
      m[val[i]] = i ;
    for ( uInt i = 0 ; i < len ; i++ ) {
      out[i] = m[in[i]] ;
    }
  }
} 

Int STIdxIterExAcc::getSrcType()
{
  if ( srctypeid_m >= 0 ) 
    return (Int)(iter_m->current()[srctypeid_m]) ;
  else
    return -999 ;
}

String STIdxIterExAcc::getSrcName()
{
  if ( srcname_m.nelements() > 0 )
    return srcname_m[iter_m->current()[srcnameid_m]] ;
  else
    return "" ;
}

} // namespace
