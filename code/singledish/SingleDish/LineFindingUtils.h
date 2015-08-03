//# --------------------------------------------------------------------
//# LineFindingUtils.h: this defines utility functions of line finding
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
#ifndef _CASA_LINE_FINDING_UTILS_H_
#define _CASA_LINE_FINDING_UTILS_H_

#include <iostream>
#include <string>
#include <list>

#include <casa/aipstype.h>
#include <casa/Arrays/Vector.h>

#include <casa_sakura/SakuraAlignedArray.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  class LineFinderUtils {
  public:
    /*
      Bin data and mask arrays.
     */
    template <typename DataType> 
      static size_t binDataAndMask(size_t const num_in,
				   DataType const in_data[/*num_in*/],
				   bool const in_mask[/*num_in*/],
				   size_t const bin_size,
				   size_t const num_out,
				   DataType out_data[/*num_out*/],
				   bool out_mask[/*num_out*/],
				   size_t const offset=0,
				   bool const keepsize=false);

    /*
      Calculate median absolute deviation of data_array
     */
    static void calculateMAD(size_t const num_data,
			     float const in_data[/*num_data*/],
			     SakuraAlignedArray<bool> const& in_mask,
			     float mad[/*num_data*/]);

    /*
      Count the number of True elements in a boolean array
     */
    inline static size_t countTrue(size_t num_data, bool const data[/*num_data*/])
    {
      size_t ntrue = 0;
      static_assert(static_cast<size_t>(true)==1, "cast of bool failed");
      static_assert(static_cast<size_t>(false)==0, "cast of bool failed");
      for (size_t i=0; i<num_data; ++i) {
	ntrue += static_cast<size_t>(data[i]);
      }
      return ntrue;
    };

    /*
      create mask by a threshold value
      out_mask[i] = in_mask[i] && (in_data[i] >= threshold)
     */

    static void createMaskByAThreshold(size_t const num_data,
				       SakuraAlignedArray<float> const& in_data,
				       SakuraAlignedArray<bool> const& in_mask,
				       float const threshold,
				       SakuraAlignedArray<bool>& out_mask);
    /*
      create mask by threshold value array
      out_mask[i] = in_mask[i] && (in_data[i] >= threshold_array[i])
     */
/*     template <typename DataType>  */
/*     static void createMaskByThresholds(size_t const num_data, */
/* 				       DataType const in_data[/\*num_data*\/], */
/* 				       bool const in_mask[/\*num_data*\/], */
/* 				       DataType const threshold_array[/\*num_data*\/], */
/* 				       bool out_mask[/\*num_data*\/]); */

    /*
      create sign array by a threshold value
      sign[i] = +1 (in_data[i] > threshold)
              = 0  (in_data[i] == threshold)
	      = -1 (in_data[i] < threshold)
     */
    template <typename DataType>
      inline static void createSignByAThreshold(size_t const num_data,
						DataType const in_data[/*num_data*/],
						DataType const threshold,
						int8_t sign[/*num_data*/])
    {
      for (size_t i = 0; i < num_data; ++i) {
	  sign[i] = signCompare(in_data[i], threshold);
      }
    };

    /*
      create mask by threshold value array
      sign[i] = +1 (in_data[i] > threshold_array[i])
              = 0  (in_data[i] == threshold_array[i])
	      = -1 (in_data[i] < threshold_array[i])
     */
    template <typename DataType>
    inline static void createSignByThresholds(size_t const num_data,
				       DataType const in_data[/*num_data*/],
				       DataType const threshold_array[/*num_data*/],
				       int8_t sign[/*num_data*/])
    {
      for (size_t i = 0; i < num_data; ++i) {
	  sign[i] = signCompare(in_data[i], threshold_array[i]);
      }
    };

    static void deBinRanges(size_t const bin_size, size_t const offset,
			    std::list<std::pair<size_t,size_t>>& range_list);

    /* 
       Extend for line wing
       Line ranges are extended while sign has the same value as the range AND mask=true.
    */
    static void extendRangeBySign(size_t num_sign, int8_t const sign[/*num_sign*/],
				  bool const mask[/*num_sign*/],
				  std::list<std::pair<size_t,size_t>>& range_list);

    /* 
       Get median of an sorted array.
       num_data: the number of elements from which median is calculated
       data: values should be sorted in ascending order.
             Must neighther have Inf nor Nan in elements 0-num_data-1.
       When the number of elements is odd the function returns data[num_data/2].
       Otherwise, average of middle two elements, i.e, (data[num_data/2]+data[num_data/2-1])/2
     */
     template <typename DataType>
         inline static DataType getMedianOfSorted(size_t const num_data,
					   DataType const data[/*num_data*/]) {
       return (data[num_data/2] + data[num_data/2-((num_data+1)%2)])/static_cast<DataType>(2);
     };


    /*
      Get median of an array with mask.
      
     */
     static float maskedMedian(size_t num_data, float const* data,
			       SakuraAlignedArray<bool> const& mask, float fraction=1.0);


    /*
      Convert boolean channel mask to channel index ranges.
     */
     static void maskToRangesList(size_t const num_mask, bool const mask[/*num_mask*/],
				 std::list<std::pair<size_t,size_t>>& out_range);

    /*
      Merge line ranges if the ranges are separated only by flagged
      channels, i.e., mask=false.
     */
    static void mergeGapByFalse(size_t const num_mask, bool const mask[/*num_mask*/],
				size_t const maxgap,
				std::list<std::pair<size_t,size_t>>& range_list);

    /*
      Merge line ranges if the ranges are overlapped.
      This function assumes that range_list is in ascending order of idx
      for both for first and second elements of pairs.
     */
    static void mergeOverlappingRanges(std::list<std::pair<size_t,size_t>>& range_list);

    static void mergeOverlapInTwoLists(std::list<std::pair<size_t,size_t>>& to,
				       std::list<std::pair<size_t,size_t>>& from);
    /*
      Merge line ranges if the gap between lines are smaller than
      a certain fraction of narrower line.
      This function assumes that range_list is in ascending order of idx
      for both for first and second elements of pairs.
     */
    static void mergeSmallGapByFraction(double const fraction,
					size_t const maxwidth,
					std::list<std::pair<size_t,size_t>>& range_list);

    /*
      Remove line ranges smaller than minwidth or larger than maxwidth.
     */
    static void rejectWideRange(size_t const maxwidth,
				std::list<std::pair<size_t,size_t>>& range_list);

    static void rejectNarrowRange(size_t const minwidth,
				  std::list<std::pair<size_t,size_t>>& range_list);
/*     template <typename DataType> */
/*       static void splitLines(DataType const& in_data, bool const& in_mask, */
/* 			     std::list<std::pair<size_t,size_t>> const& in_range, */
/* 			     std::list<std::pair<size_t,size_t>>& out_range); */

  private:
    static size_t mergeARangeToList(std::list<std::pair<size_t,size_t>>& range_list,
				    std::pair<size_t,size_t>& new_range,
				    size_t const cursor=0);

    template <typename DataType>
      inline static int8_t  signCompare(DataType const& data, DataType const& threshold){
	if (data > threshold)
	  return 1;
	else if (data < threshold)
	  return -1;
	else
	  return 0;
    };
};

} //# NAMESPACE CASA - END
  
#endif /* _CASA_LINE_FINDING_UTILS_H_ */
