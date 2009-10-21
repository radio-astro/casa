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
  : n_chan(0), n_ifr(0), n_time(0), n_bit(0), n_corr(0)
{
  iter_pos = 0;
  curs = Matrix<T>();
  lattice = NULL;
}

 template<class T> RFCubeLatticeIterator<T>::RFCubeLatticeIterator(std::vector<boost::dynamic_bitset<> > *lat,
								   unsigned nchan, unsigned nifr, 
								   unsigned ntime, unsigned nbit,
								   unsigned ncorr)
   : n_chan(nchan), n_ifr(nifr), n_time(ntime), n_bit(nbit), n_corr(ncorr)

{
  iter_pos = 0;
  lattice = lat;
  update_curs();
}

template<class T> RFCubeLatticeIterator<T>::~RFCubeLatticeIterator()
{
  flush_curs();
}

/* Update cursor from buffer, convert from bitset to Matrix<uInt> */

/*
    The format is like this:

    [ ... | agent1 | agent0 | corrN | ... | corr1 | corr0 ]

    i.e. the length is number of agents + number of correlations.

    Except (don't ask why) if nCorr = 1, the format is

    [ ... | agent1 | agent0 | notUsed | corr0 ]

*/

template<class T> void RFCubeLatticeIterator<T>::update_curs()
{
  if (lattice != NULL && iter_pos < lattice->size()) {
    curs.resize(IPosition(2, n_chan, n_ifr));

    //curs = (*lattice)[iter_pos];

    boost::dynamic_bitset<> &l = (*lattice)[iter_pos];
    
    boost::dynamic_bitset<> val(n_bit);
    if (n_corr == 1) {
      val.resize(n_bit+1);
    }

    for (unsigned ifr = 0; ifr < n_ifr; ifr++) {
      for (unsigned chan = 0; chan < n_chan; chan++) {
	if (n_corr <= 1) {
	  /* write corr0 */
	  unsigned indx = 0 + n_bit*(chan + n_chan*ifr);
	  val[0] = l[indx];

	  /* write agents starting from b[2] */
	  for (unsigned b = 1; b < n_bit; b++) {
	    indx = b + n_bit*(chan + n_chan*ifr);
	    val[b+1] = l[indx];
	  }
	}
	else {
	  unsigned indx;
	  for (unsigned b = 0; b < n_bit; b++) {
	    indx = b + n_bit*(chan + n_chan*ifr);
	    
	    val[b] = l[indx]; 
	  }
        }

        curs(chan, ifr) = val.to_ulong();
      }
    }
  }
}

/* Write cursor back to buffer, convert from Matrix<uInt> to bitset.
   See above for an explanation of the format.
*/
template<class T> void RFCubeLatticeIterator<T>::flush_curs()
{
  if (lattice != NULL && iter_pos < lattice->size()) {

    //(*lattice)[iter_pos] = curs;
    boost::dynamic_bitset<> &l = (*lattice)[iter_pos];
    
    boost::dynamic_bitset<> val(n_bit+1);

    for (unsigned ifr = 0; ifr < n_ifr; ifr++) {
      for (unsigned chan = 0; chan < n_chan; chan++) {

	/* It would be faster to avoid this allocation,
	   and put the bit pattern from curs(chan,ifr) in val
	   in some other way.  There doesn't seem to be a
	   val.set_from_ulong(curs(chan,ifr));
	*/
	   
	val = boost::dynamic_bitset<> (n_bit+1, (unsigned)curs(chan, ifr));

	if (n_corr <= 1) {
	  unsigned indx = 0 + n_bit*(chan + n_chan*ifr);
	  l[indx] = val[0];

	  for (unsigned b = 1; b < n_bit; b++) {
	    indx = b + n_bit*(chan + n_chan*ifr);
	    l[indx] = val[b+1];
	  }
	}
	else {
	  unsigned indx;
	  for (unsigned b = 0; b < n_bit; b++) {
	    indx = b + n_bit*(chan + n_chan*ifr);
	    
	    l[indx] = val[b];
	  }
	}
      }
    }
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
template<class T> RFCubeLattice<T>::RFCubeLattice ( uInt nchan,
                                                    uInt nifr,
                                                    uInt ntime,
                                                    uInt ncorr,
                                                    uInt nAgent,
                                                    Int maxmem )
{
  init(nchan, nifr, ntime, ncorr, nAgent, maxmem);
}
template<class T> RFCubeLattice<T>::RFCubeLattice ( uInt nchan,
                                                    uInt nifr,
                                                    uInt ntime,
                                                    uInt ncorr,
                                                    uInt nAgent,
                                                    const T &init_val,
                                                    Int maxmem )
{
  init(nchan, nifr, ntime, ncorr, nAgent, init_val, maxmem);
}
template<class T> RFCubeLattice<T>::~RFCubeLattice ()
{
  cleanup();
}

template<class T> void 
RFCubeLattice<T>::init(uInt nchan,
                       uInt nifr,
                       uInt ntime,
		       uInt ncorr,
		       uInt nAgent,
                       Int maxmem,
                       Int tile_mb)
{
  n_bit = ncorr + nAgent;

  if (n_bit > 32) {
    stringstream ss;
    ss << 
      "Sorry, too many polarizations (" << ncorr <<
      ") and agents (" << nAgent << "). Max supported number is 32 in total.";
    cerr << ss.str();
    abort();
    throw AipsError(ss.str());
  }

  lat_shape = IPosition(3,nchan,nifr,ntime);

  lat = std::vector<boost::dynamic_bitset<> >(ntime);
  for (unsigned i = 0; i < ntime; i++) {
    //lat[i] = Matrix<T>(IPosition(2, nchan, nifr));
    lat[i] = boost::dynamic_bitset<>(nchan * nifr * (ncorr+nAgent));
  }

  iter = RFCubeLatticeIterator<T>(&lat, nchan, nifr, ntime, ncorr+nAgent, ncorr);
}

template<class T> RFCubeLatticeIterator<T> RFCubeLattice<T>::newIter()
{
  return RFCubeLatticeIterator<T>(&lat, n_chan, n_ifr, n_time, n_bit, n_corr);
}

template<class T> void RFCubeLattice<T>::init(uInt nchan,
                                              uInt nifr,
                                              uInt ntime,
					      uInt ncorr,
					      uInt nAgent,
                                              const T &init_val,
                                              Int maxmem,
                                              Int tile_mb)
{
  n_chan = nchan;
  n_ifr = nifr;
  n_time = ntime;
  n_bit = ncorr + nAgent;
  n_corr = ncorr;
  init(nchan, nifr, ntime, ncorr, nAgent, maxmem, tile_mb);
  //  lat.set(init_val);

  uInt nbits = ncorr + nAgent;

  /* Write init_val to every matrix element.
     See above for description of format */
  boost::dynamic_bitset<> val(nbits+1, (unsigned) init_val);
  for (unsigned i = 0; i < ntime; i++) {
    for (unsigned chan = 0; chan < nchan; chan++) {
      for (unsigned ifr = 0; ifr < nifr; ifr++) {
	if (n_corr <= 1) {
	  unsigned indx = 0 + n_bit*(chan + n_chan*ifr);
	  lat[i][indx] = val[0];

	  for (unsigned b = 1; b < n_bit; b++) {
	    indx = b + n_bit*(chan + n_chan*ifr);
	    lat[i][indx] = val[b+1];
	  }
	}
	else {
	  unsigned indx;
	  for (unsigned b = 0; b < n_bit; b++) {
	    indx = b + n_bit*(chan + n_chan*ifr);
	    
	    lat[i][indx] = val[b];
	  }
	}
      }
    }
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

