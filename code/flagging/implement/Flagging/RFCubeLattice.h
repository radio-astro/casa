//# RFCubeLattice.h: this defines RFCubeLattice
//# Copyright (C) 2000,2001
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
//# $Id$
#ifndef FLAGGING_RFCUBELATTICE_H
#define FLAGGING_RFCUBELATTICE_H
    
#include <casa/Arrays/Matrix.h> 
#include <lattices/Lattices/TempLattice.h> 
#include <lattices/Lattices/LatticeIterator.h> 
#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// RFCubeLatticeIterator: iterator over a cubic buffer
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> std::vector, Matrix
// </prerequisite>
//
// <synopsis>
// See RFCubeLattice, below
// </synopsis>
//
// <templating arg=T>
//    <li> same as Matrix
// </templating>
//
// <todo asof="2001/04/16">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>

template<class T>
class RFCubeLattice;

template<class T> class RFCubeLatticeIterator
{
  private:
    std::vector<Matrix<T> > *lattice;

    Matrix<T> curs;

    unsigned int iter_pos;

    void update_curs();
  
  public:
    // default constructor creates empty iterator
    RFCubeLatticeIterator();
    
    // creates and attaches to lattice
    RFCubeLatticeIterator(std::vector<Matrix<T> > *lat);
    
    // destructor
    ~RFCubeLatticeIterator();

    // resets the lattice iterator to beginning, returns cursor
    Matrix<T> * reset();
    
    // advances internal iterator to specified slot along the Z axis, returns cursor
    Matrix<T> * advance( uInt iz );
    
    // returns position of internal iterator
    uInt position ()                 
      { return iter_pos; }
    
    //  returns internal cursor
    Matrix<T> * cursor();
    
    // returns element at i,j of cursor
    T & operator () ( uInt i,uInt j );

    void flush_curs();
};


// <summary>
// RFCubeLatice: a cubic lattice
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> TempLattice
// </prerequisite>
//
// <synopsis>
// RFCubeLattice is a [NX,NY,NZ] vector of Matrices which 
// is iterated over the Z axis. 
// While a vector of Matrices may not be localized in memory, it has the
// advantage that the total amount of memory allocated can exceed
// the available RAM, which is probably not possible if allocated as a
// single giant block.
//
// The buffer is no longer implemented using a TempLattice because the
// template parameter to TempLattice is restricted to certain types, and
// cannot be boost::dynamic_bitset<>. Besides, TempLattice is currently(?)
// *not* well implemented: it creates TempLattice disk files although most
// of the RAM is free.
//
// If more memory than avilable RAM is requested, swapping will occur.
// The underlying OS probably knows better when to swap!
//
// </synopsis>
//
// <motivation>
// Many flagging agents make use of cubic lattices (typically, to maintain
// [NCHAN,NIFR,NTIME] cubes of something) in an identical way. This class
// provides a clean and convenient interface to the basic functions.
// </motivation>
//
// <templating arg=T>
//    <li> same as Matrix
// </templating>
//
// <todo asof="2001/04/16">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>

template<class T> class RFCubeLattice
{
protected:
  IPosition                 lat_shape,tile_shape,iter_shape;
  std::vector<Matrix<T> >   lat;
  RFCubeLatticeIterator<T>  iter;
  Bool                      valid;

public:
// default constructor creates empty cube
  RFCubeLattice();
// creates NX x NY x NZ cube
  RFCubeLattice( uInt nx,uInt ny,uInt nz,Int maxmem=-1 );
// creates NX x NY x NZ cube and fills with initial value
  RFCubeLattice( uInt nx,uInt ny,uInt nz,const T &init_val,Int maxmem=-1 );
// destructor
  ~RFCubeLattice();

// creates NX x NY x NZ cube
// tile_mb is the tile size, in MB (when using paging)
  void init ( uInt nx,uInt ny,uInt nz,Int maxmem=-1,Int tile_mb=2 );
// creates NX x NY x NZ cube and fills with initial value
// tile_mb is the tile size, in MB (when using paging)
  void init ( uInt nx,uInt ny,uInt nz,const T &init_val,Int maxmem=-1,Int tile_mb=2 );
// destroys cube
  void cleanup ();
// returns size of cube
  static uInt estimateMemoryUse ( uInt nx,uInt ny,uInt nz )
        { return nx*ny*nz*sizeof(T)/(1024*1024) + 1; }

// resets the lattice iterator to beginning. 
  Matrix<T> * reset( Bool will_read=True,
                     Bool will_write=True );  
  
// advances internal iterator to specified slot along the Z axis, returns cursor
  Matrix<T> * advance( Int iz )   { return iter.advance(iz); };
  
// returns position of internal iterator
  Int position ()                 { return iter.position(); }
  
// returns shape
  const IPosition & shape ()      { return lat_shape; }

//  returns internal cursor
  Matrix<T> * cursor()              { return iter.cursor(); }
  
// returns element at i,j of cursor
  T & operator () ( uInt i,uInt j )  { return (*iter.cursor())(i,j); }
  
// provides access to lattice itself  
  std::vector<Matrix<T> > & lattice()    { return lat; }

// provides access to iterator  
  RFCubeLatticeIterator<T> & iterator()    { return iter; }

// creates a new iterator for this lattice
  RFCubeLatticeIterator<T>  newIter();
};



} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <flagging/Flagging/RFCubeLattice.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
