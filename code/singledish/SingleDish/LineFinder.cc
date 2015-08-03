//# --------------------------------------------------------------------
//# LineFinder.cc: this defines utility functions of line finding
//# --------------------------------------------------------------------
//# Copyright (C) 2015
//# National Astronomical Observatory of Japan
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
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <string>

#include <libsakura/sakura.h>

#include <singledish/SingleDish/LineFinder.h>
#include <singledish/SingleDish/LineFindingUtils.h>

using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

namespace linefinder {//# NAMESPACE LINEFINDER - BEGIN

void getMask(size_t const num_mask, bool *mask,
	     list<pair<size_t,size_t>>& ranges,
	     bool const invert, bool const initialize)
{
  bool const lineval = (!invert);
  if (initialize) {
    for (size_t i = 0; i<num_mask; ++i) {
      mask[i] = invert;
    }
  }
  for (list<pair<size_t,size_t>>::iterator iter=ranges.begin();
       iter!=ranges.end(); ++iter) {
    AlwaysAssert((*iter).second >= (*iter).first, AipsError);
    for (size_t i=(*iter).first; i<(*iter).second+1 && i<num_mask; ++i) {
      mask[i] = lineval;
    }
  }
}
  
list<pair<size_t,size_t>> MADLineFinder(size_t const num_data,
					float const* data,
					bool * mask,
					float const threshold,
					uint8_t max_iteration,
					size_t const minwidth,
					size_t const maxwidth,
					size_t const avg_limit,
					pair<size_t,size_t> edge)
{
  // value check and edge handling
  size_t min_valid_elem = 2;
  AlwaysAssert(edge.first+edge.second<num_data-min_valid_elem, AipsError);
  AlwaysAssert(maxwidth>minwidth, AipsError);
  for (size_t i=0; i<edge.first; ++i){
    mask[i] = false;
  }
  for (size_t i=num_data-edge.second; i<num_data; ++i){
    mask[i] = false;
  }
  size_t num_valid_elem=LineFinderUtils::countTrue(num_data,mask);
  AlwaysAssert(num_valid_elem >= min_valid_elem+edge.first+edge.second, AipsError);
  // factor for each iteration of binning
  size_t const binfactor = 4;
  size_t average_factor =1;
  //size_t maxgap = num_data;
  list<pair<size_t,size_t>> line_list;
  SakuraAlignedArray<float> binned_data(num_data);
  SakuraAlignedArray<bool> binned_mask(num_data);
  while (true) {
    // Bin spectrum and mask
    size_t const maxwidth_bin = std::max(maxwidth/average_factor,static_cast<size_t>(1));
    //size_t const minwidth_bin = minwidth/average_factor;
    size_t const offset = average_factor/2;
    size_t num_binned = \
      LineFinderUtils::binDataAndMask<float>(num_data, data, mask,
					     average_factor, num_data,
					     binned_data.data,
					     binned_mask.data,
					     offset,false);
    // caluculate MAD array
    SakuraAlignedArray<float> mad_data(num_binned);
    SakuraAlignedArray<bool> line_mask(num_binned);
    SakuraAlignedArray<bool> search_mask(num_binned);
    for (size_t i=0; i<num_binned; ++i) {
	search_mask.data[i] = binned_mask.data[i];
    }
    float prev_mad_threshold = FLT_MAX;
    size_t prev_num_line_chan = 0;
    list<pair<size_t,size_t>> prev_line_list;
    prev_line_list.clear();
    for (size_t iteration=0; iteration < max_iteration; ++iteration) {
      list<pair<size_t,size_t>> new_lines; // lines found in this iteration
      // working data array. sorted after median calculation.
      LineFinderUtils::calculateMAD(num_binned, binned_data.data,
				    search_mask, mad_data.data);
      // use lower 80% of mad_array and use median of the array as a criteria.
      float mad_threshold = threshold*LineFinderUtils::maskedMedian(num_binned, mad_data.data,
								    search_mask,0.8);
//       cout << "mad_threshold = " << mad_threshold << endl;
      if (mad_threshold >= prev_mad_threshold) break; // stop iteration
      LineFinderUtils::createMaskByAThreshold(num_binned, mad_data,
					      binned_mask, mad_threshold,
					      line_mask);
      // channel mask -> mask range
      LineFinderUtils::maskToRangesList(num_binned, line_mask.data, new_lines);
      //rejectByRangeWidth(minwidth_bin,maxwidth_bin,new_lines);
      if ( new_lines.size()>0 ) {
	// merge flagged gaps
	//LineFinderUtils::mergeGapByFalse(num_binned, binned_mask.data,
	//				 maxgap/average_factor, new_lines);
	// extend wing
	SakuraAlignedArray<int8_t> sign(num_binned);
	LineFinderUtils::createSignByAThreshold(num_binned, mad_data.data,
					       mad_threshold, sign.data);
	LineFinderUtils::extendRangeBySign(num_binned, sign.data,
					   binned_mask.data,
					   new_lines);
	// merge overlap
	LineFinderUtils::mergeOverlappingRanges(new_lines);
	// reject by max line width
	LineFinderUtils::rejectWideRange(maxwidth_bin,new_lines);
	// merge lines with small gap
	LineFinderUtils::mergeSmallGapByFraction(0.25,maxwidth_bin,new_lines);
	// de-bin line ranges
	LineFinderUtils::deBinRanges(average_factor, offset, new_lines);
	// reject by min and max line width
	LineFinderUtils::rejectWideRange(maxwidth,new_lines);
	LineFinderUtils::rejectNarrowRange(minwidth, new_lines);
	// update search mask for the next iteration
	for (list<pair<size_t,size_t>>::iterator iter=new_lines.begin();
	     iter!=new_lines.end(); ++iter) {
	  for (size_t i=(*iter).first; i<=(*iter).second && i<num_binned; ++i) {
	    search_mask.data[i] = false;
	  }
	}
	// culculate the number of line channels
	size_t curr_num_line_chan = 0;
	for (list<pair<size_t,size_t>>::iterator iter=new_lines.begin();
	     iter!=new_lines.end(); ++iter) {
	  curr_num_line_chan += ((*iter).second - (*iter).first + 1);
	}
	if (curr_num_line_chan <= prev_num_line_chan)
	  break; // stop iteration
	// save values of current iteration for next loop
	prev_num_line_chan = curr_num_line_chan;
	prev_mad_threshold = mad_threshold;
	prev_line_list.clear();
	prev_line_list.splice(prev_line_list.end(), new_lines);
      }
      else { // no lines found
	break;
      }
    } // end of iteration loop
    // merge line list from different bin level
//     cout << "Final line list for average_factor = " << average_factor << endl;
//     for (list<pair<size_t,size_t>>::iterator iter=prev_line_list.begin();
// 	 iter!=prev_line_list.end(); ++iter) {
//       cout << "[" << (*iter).first << ", " << (*iter).second << "], ";
//     }
//     cout << endl;
    LineFinderUtils::mergeOverlapInTwoLists(line_list, prev_line_list);
    average_factor *= binfactor;
    if (average_factor <= avg_limit)
      continue; // go to the next bin level.
    else
      break;
  } // end of bin level loop
  return line_list;
}




} //# NAMESPACE LINEFINDER - END

} //# NAMESPACE CASA - END
