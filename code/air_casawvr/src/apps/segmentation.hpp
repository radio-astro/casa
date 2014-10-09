/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version March 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file segmentation.hpp
   
   Segmentation of observations into pieces over which the phase
   correction coefficients do not vary too much
*/
#ifndef _LIBAIR_APPS_SEGMENTATIO_HPP__
#define _LIBAIR_APPS_SEGMENTATIO_HPP__

#include <vector>
#include <set>
#include <cstddef>

namespace LibAIR2 {

  /** Compute the first and last maintable row for each dwell on a
      field

      \note Does this function correctly handle data which is not
      exactly time orderred?
      \bug This function probably doesn't work as expected on typical
      MS with data which is not fully time-ordereed
   */
  void fieldSegments(const std::vector<double> &time,
		     const std::vector<int> &fieldID,
		     std::vector<std::pair<double, double> > &res);

  /** Like fieldSegments, but "tie" some fields/sources together,
      which means that transition from one to another in same set is
      not considered a break point
   */
  void fieldSegmentsTied(const std::vector<double> &time,
			 const std::vector<int> &fieldID,
			 const std::vector<std::set<size_t> > &tied,
			 std::vector<std::pair<double, double> > &res);

  /** Time intervals over which fields are observed

      \param fieldselect All fields in this set are include
   */
  void fieldTimes(const std::vector<double> &time,
		  const std::vector<int> &fieldID,
		  const std::vector<size_t> &spw,
		  const std::set<size_t>  &fieldselect,
		  size_t spwselect,
		  std::vector<std::pair<double, double> > &res);

}

#endif
