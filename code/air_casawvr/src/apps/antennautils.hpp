/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version September 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file antennautils.hpp

   Utilities related to antenna positions, etc
*/
#ifndef _LIBAIR_APPS_ANTENNAUTILS_HPP__
#define _LIBAIR_APPS_ANTENNAUTILS_HPP__

#include <set>
#include <boost/numeric/ublas/matrix.hpp>

namespace LibAIR2 {

  /** \brief Set of antenna numbers
   */
  typedef std::set<size_t> AntSet;

  typedef std::set< std::pair<double, size_t> > AntSetD;


  /** A set of weights and antenna numbers
   */
  typedef std::set< std::pair<double, size_t> > AntSetWeight;

  /** Antennas are represented by rows and the three cartesian
      coordinate of positions in columns
  */
  typedef boost::numeric::ublas::matrix<double> antpos_t;

  /** \brief Return antennas and their distances to antenna i while
      avoiding antennas in flag
   */
  AntSetD antsDist(const antpos_t &pos,
		   size_t i,
		   const AntSet &flag);

  /** \brief Return n nearest antennas to antenna i while avoiding
      antennas in flag
   */
  AntSet nearestAnts(const antpos_t &pos,
		     size_t i,
		     const AntSet &flag,
		     size_t n);

  /** \brief Return nearest n antennas and their weights 
   */
  AntSetWeight linNearestAnt(const antpos_t &pos,
			     size_t i,
			     const AntSet &flag,
			     size_t n);


}

#endif
