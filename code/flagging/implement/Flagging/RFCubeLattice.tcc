//# RFCubeLattice.cc: this defines RFCubeLattice
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
#include <lattices/Lattices/LatticeStepper.h>
#include <flagging/Flagging/RFCubeLattice.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template<class T> RFCubeLatticeIterator<T>::RFCubeLatticeIterator ()
{
  iter_pos = 0;
  curs = Matrix<T>();
  lattice = NULL;
}

template<class T> RFCubeLatticeIterator<T>::RFCubeLatticeIterator(std::vector<Matrix<T> > *lat)
{
  iter_pos = 0;
  lattice = lat;
  update_curs();
}

template<class T> RFCubeLatticeIterator<T>::~RFCubeLatticeIterator()
{
  flush_curs();
}

/* Update cursor from buffer */
template<class T> void RFCubeLatticeIterator<T>::update_curs()
{
  if (lattice != NULL && iter_pos < lattice->size()) {
    //curs.resize((*lattice)[iter_pos].shape());
    curs = (*lattice)[iter_pos];
  }
}

/* Write cursor back to buffer */
template<class T> void RFCubeLatticeIterator<T>::flush_curs()
{
  if (lattice != NULL && iter_pos < lattice->size()) {
    (*lattice)[iter_pos] = curs;
  }
}

template<class T> Matrix<T> * RFCubeLatticeIterator<T>::reset()
{
  if (iter_pos != 0) flush_curs();
  iter_pos = 0;
  update_curs();
  return cursor();
}

template<class T> Matrix<T> * RFCubeLatticeIterator<T>::advance(uInt t1)
{
  if (iter_pos != t1) flush_curs();
  iter_pos = t1;
  update_curs();
  return cursor();
}

template<class T> Matrix<T> * RFCubeLatticeIterator<T>::cursor()
{
  return &curs;
  //return &((*lattice)[iter_pos]);
}

template<class T> T & RFCubeLatticeIterator<T>::operator()(uInt i,uInt j)
{ 
  return curs(i, j);
  //return (*lattice)[iter_pos](i, j);
}




template<class T> RFCubeLattice<T>::RFCubeLattice ()
{
}
template<class T> RFCubeLattice<T>::RFCubeLattice ( uInt nchan,uInt nifr,uInt ntime,Int maxmem )
{
  init(nchan,nifr,ntime,maxmem);
}
template<class T> RFCubeLattice<T>::RFCubeLattice ( uInt nchan,uInt nifr,uInt ntime,const T &init_val,Int maxmem )
{
  init(nchan,nifr,ntime,init_val,maxmem);
}
template<class T> RFCubeLattice<T>::~RFCubeLattice ()
{
  cleanup();
}

template<class T> void 
RFCubeLattice<T>::init(uInt nchan,
                       uInt nifr,
                       uInt ntime,
                       Int maxmem,
                       Int tile_mb)
{
  lat_shape = IPosition(3,nchan,nifr,ntime);

  // iterator is one plane of lattice
  iter_shape = IPosition(3,nchan,nifr,1);

  // select a tile size
  uInt tilesize = tile_mb*1024*1024, 
      planesize = iter_shape.product()*sizeof(T),
      ntile = (uInt)(tilesize/(Float)planesize+.2);
  tile_shape = IPosition(3,nchan,nifr,ntile);
  //  cerr<<"Using "<<ntile<<" planes ("<<tile_shape.product()*sizeof(T)/(1024*1024.)<<"MB) tile\n";

  //  lat = TempLattice<T>( TiledShape(lat_shape,iter_shape),maxmem );
  lat = std::vector<Matrix<T> >(ntime);
  for (unsigned i = 0; i < ntime; i++) {
    lat[i] = Matrix<T>(IPosition(2, nchan, nifr));
  }

  iter = RFCubeLatticeIterator<T>(&lat);
}

template<class T> RFCubeLatticeIterator<T> RFCubeLattice<T>::newIter()
{
  return RFCubeLatticeIterator<T>(&lat);
}

template<class T> void RFCubeLattice<T>::init(uInt nchan,
                                              uInt nifr,
                                              uInt ntime,
                                              const T &init_val,
                                              Int maxmem,
                                              Int tile_mb)
{
  init(nchan,nifr,ntime,maxmem,tile_mb);
  //  lat.set(init_val);
  for (unsigned i = 0; i < ntime; i++) {
    lat[i].set(init_val);
  }
}

template<class T> void RFCubeLattice<T>::cleanup ()
{
  iter.flush_curs();
  iter.cursor()->resize(IPosition(2, 0, 0));
  iter = RFCubeLatticeIterator<T>();
  lat.resize(0);
  lat_shape.resize(0);
}

template<class T> Matrix<T> * RFCubeLattice<T>::reset ( Bool r,Bool w )
{
  return iter.reset();
}


} //# NAMESPACE CASA - END

