//# LineFinderUtilsTest_GTest.cc: this defines unit tests of
//# LineFinderUtils using google test framework
//#
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
#include <iostream>
#include <list>
#include <cassert>

#include <gtest/gtest.h>

#include <casa/Arrays/Vector.h>

#include <libsakura/sakura.h>
#include <singledish/SingleDish/LineFindingUtils.h>
#include <singledish/SingleDish/test/SingleDishTestUtil.h>

#define NUM_IN 20

using namespace casa;
using namespace std;
using namespace test_utility;

typedef pair<size_t,size_t> LineRange;
typedef list<LineRange> LineRangeList;

class LineFindingUtilsTest : public ::testing::Test {
protected:
  LineFindingUtilsTest() : verbose_(false) {}

  virtual void SetUp() {
    // Initialize sakura
    LIBSAKURA_SYMBOL(Status) status = LIBSAKURA_SYMBOL(Initialize)(nullptr,
								   nullptr);
    ASSERT_EQ(LIBSAKURA_SYMBOL(Status_kOK), status);
  }

  virtual void TearDown() {
    // Clean-up sakura
    LIBSAKURA_SYMBOL(CleanUp)();
  }

  void RunBinning(size_t const bin_size, size_t const offset, bool keepsize,
		  size_t const num_data, float const *in_data,
		  bool const *in_mask, size_t const num_bin,
		  float const *bin_data, bool const *bin_mask) {
    cout << "[Binning]" << endl;
    cout << "Bin = " << bin_size << ", offset = " << offset
	 << ", keepsize = " << (keepsize ? "true" : "false") << endl;
    if (verbose_) {
      PrintArray<float>("input data", num_data, in_data);
      PrintArray<bool>("input mask", num_data, in_mask);
    }
    float out_data[num_bin];
    bool out_mask[num_bin];
    size_t nout = LineFinderUtils::binDataAndMask<float>(num_data, in_data, in_mask, bin_size, num_bin, out_data, out_mask, offset, keepsize);
    if (verbose_) {
      PrintArray<float>("binned data", num_bin, out_data);
      PrintArray<bool>("binned mask", num_bin, out_mask);
    }
    EXPECT_EQ(num_bin, nout);
    for (size_t i = 0; i < nout; ++i) {
      EXPECT_FLOAT_EQ(out_data[i], bin_data[i]);
      EXPECT_EQ(out_mask[i], bin_mask[i]);
    }
  }

  void RunMaskToRangeList(size_t num_mask, bool const *mask,
			  LineRangeList &ref_lines) {
    LineRangeList lines;
    if (verbose_) {
      PrintArray("input_mask", num_mask, mask);
    }
    LineFinderUtils::maskToRangesList(num_mask, mask, lines);
    if (verbose_) {
      PrintLine(lines);
    }
    AssertLineRanges(lines, ref_lines);
  }

  void RunMergeIntTwoLists(LineRangeList from, LineRangeList &to,
			   LineRangeList &reference) {
    if (verbose_) {
      cout << "[Merge overlapping ranges in two array]" << endl;
      cout << "FROM" << endl;
      PrintLine(from);
      cout << "TO" << endl;
      PrintLine(to);
    }
    LineFinderUtils::mergeOverlapInTwoLists(to, from);
    if (verbose_) {
      cout << "OUTPUT" << endl;
      PrintLine(to);
    }
    AssertLineRanges(to, reference);
  }

  void RunMergeGapByFalse(size_t const maxgap, LineRangeList &lines,
			  LineRangeList &ref_lines) {
    bool mask[NUM_IN];
    size_t const num_data = ELEMENTSOF(mask);
    for (size_t i = 0; i < num_data; ++i) {
      mask[i] = (i>lines.front().second && i<lines.back().first)? false : true;
    }
    if (verbose_) {
      PrintArray("mask", num_data, mask);
      cout << "[input line ranges]" << endl;
      PrintLine(lines);
    }
    LineFinderUtils::mergeGapByFalse(num_data, mask, maxgap, lines);
    if (verbose_) {
      cout << "[input line ranges]" << endl;
      PrintLine(lines);      
    }
    AssertLineRanges(lines, ref_lines);
  }

  void AssertLineRanges(LineRangeList &data, LineRangeList &reference) {
    // test number of data
    EXPECT_EQ(data.size(), reference.size());
    // test line ranges
    LineRangeList::iterator outiter = data.begin();
    LineRangeList::iterator refiter = reference.begin();
    for (;outiter!=data.end() && refiter!=reference.end();
	 ++outiter, ++refiter) {
      EXPECT_EQ((*outiter).first, (*refiter).first);
      EXPECT_EQ((*outiter).second, (*refiter).second);
    }
  }

  void SimpleInit(size_t const num_data, float *data, bool *mask) {
    float dummyf[num_data];
    bool dummyb[num_data];
    float *mydata = (data==nullptr ? dummyf : data);
    bool *mymask = (mask==nullptr ? dummyb : mask);
    for (size_t i = 0; i<num_data; ++i) {
      mymask[i] = (i%4 != 0) ? true : false;
      mydata[i] = (mymask[i] ? static_cast<float>(i) : 1000.0); // set large values to flagged elements.
    }
  }
  void CalcAbsDiff(float value, size_t const num_data, float *data) {
    for (size_t i = 0; i<num_data; ++i) {
      data[i] = fabs(data[i]-value);
    }
  }

  //private:
  bool verbose_;

}; // end of LineFindingUtils test class

TEST_F(LineFindingUtilsTest, BinDataAndMask) {
  float data[NUM_IN];
  bool mask[ELEMENTSOF(data)];
  size_t const num_data = ELEMENTSOF(data);
  SimpleInit(num_data, data, mask);

  { // Test with offset > 0
    size_t const bin_size = 3;
    size_t const offset = 1;
    size_t const num_out = (num_data-offset)/bin_size;
    bool const keepsize=false;
    float ref_data[] = {2.0, 0.0, 0.0, 0.0, 14,0, 0.0 };
    bool ref_mask[] = {true, false, false, false, true, false};
    assert(ELEMENTSOF(ref_data)==num_out);
    assert(ELEMENTSOF(ref_mask)==num_out);
    RunBinning(bin_size, offset, keepsize, num_data, data, mask,
		num_out, ref_data, ref_mask);
  }
  { // Test keep size
    size_t const bin_size = 3;
    size_t const offset = 0;
    bool const keepsize=true;
    float ref_data[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 10.0, 10.0, 10.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			18.5, 18.5};
    bool ref_mask[] = {false, false, false, false, false, false,
		       false, false, false, true, true, true,
		       false, false, false, false, false, false,
		       true, true};
    assert(ELEMENTSOF(ref_data)==num_data);
    assert(ELEMENTSOF(ref_mask)==num_data);
    RunBinning(bin_size, offset, keepsize, num_data, data, mask,
		num_data, ref_data, ref_mask);
  }
}

/*
  Test LineFinderUtils::calculateMAD
  The function calculates Median Absolute Deviation (MAD) of a data array.
  mask (true is valid) is taken into account when calculating the median.
  MAD is calculated regardless of the mask value of corresponding element.
*/
TEST_F(LineFindingUtilsTest, culculateMAD) {
    // Demonstrate the use of Vector instead of float[]
    Vector<float> data(NUM_IN);
    Vector<float> mad(data.size());
    Vector<bool> in_mask(data.size());
    Vector<bool> out_mask(data.size());
    size_t const num_data = data.size();
    float* dptr = data.data();
    bool* mptr = in_mask.data();
    SimpleInit(num_data, dptr, mptr);
    float const ref_mad [] = {990.0, 9.0, 8.0, 7.0, 990.0, 5.0, 4.0, 3.0,
			      990.0, 1.0, 0.0, 1.0, 990.0, 3.0, 4.0, 5.0,
			      990.0, 7.0, 8.0, 9.0};
    assert(ELEMENTSOF(ref_mad)==mad.size());
    if (verbose_) {
      PrintArray("input_data", num_data, dptr);
      PrintArray("input_mask", num_data, mptr);
    }
    LineFinderUtils::calculateMAD(num_data,dptr,mptr,mad.data());
    if (verbose_) {
      PrintArray<float>("MAD", num_data, mad.data());
    }
    for (size_t i = 0; i < num_data; ++i) {
      EXPECT_FLOAT_EQ(mad.data()[i], ref_mad[i]);
    }
}

TEST_F(LineFindingUtilsTest, MaskByAThreshold) {
    float const threshold = 5.0;
    float data[NUM_IN];
    bool in_mask[ELEMENTSOF(data)];
    bool out_mask[ELEMENTSOF(data)];
    size_t const num_data = ELEMENTSOF(data);
    SimpleInit(num_data, data, in_mask);
    // mad array
    CalcAbsDiff(10.0, num_data, data);
    bool ref_mask[] = {false, true, true, true, false, true, false, false,
		       false, false, false, false, false, false, false, true,
		       false, true, true, true };
    assert(ELEMENTSOF(ref_mask)==ELEMENTSOF(out_mask));
    cout << "threshold = " << threshold << endl;
    if (verbose_) {
      PrintArray("input_data", num_data, data);
      PrintArray("input_mask", num_data, in_mask);
    }
    LineFinderUtils::createMaskByAThreshold(num_data, data, in_mask, threshold, out_mask);
    if (verbose_) {
      PrintArray("output_mask", num_data, out_mask);
    }
    for (size_t i = 0; i < num_data; ++i) {
      EXPECT_EQ(out_mask[i], ref_mask[i]);
    }
}

TEST_F(LineFindingUtilsTest, SignByAThreshold) {
    float const threshold = 5.0;
    float data[NUM_IN];
    int8_t signval[ELEMENTSOF(data)];
    size_t const num_data = ELEMENTSOF(data);
    SimpleInit(num_data, data, nullptr);
    // mad array
    CalcAbsDiff(10.0, num_data, data);
    int8_t ref_sign[] = {+1, +1, +1, +1, +1, 0, -1, -1,
			 +1, -1, -1, -1, +1, -1, -1, 0,
			 +1, +1, +1, +1};
    assert(ELEMENTSOF(ref_sign)==ELEMENTSOF(signval));
    cout << "threshold = " << threshold << endl;
    if (verbose_) {
      PrintArray("input_data", num_data, data);
    }
    LineFinderUtils::createSignByAThreshold(num_data, data,
					    threshold,signval);
    if (verbose_) {
      PrintArray("signs", num_data, signval);
    }
    for (size_t i = 0; i < num_data; ++i) {
      EXPECT_EQ(signval[i], ref_sign[i]);
    }
}

TEST_F(LineFindingUtilsTest, MaskToRangeList) {
    constexpr size_t kOffset = 3;
    bool mask[NUM_IN+kOffset];
    size_t const num_data = NUM_IN;
    SimpleInit(ELEMENTSOF(mask), nullptr, mask);
    {// [[1,3], [5,7], [9,11], [13,15], [17,19]]
      LineRangeList ref_lines = {LineRange(1,3), LineRange(5,7),
				 LineRange(9,11), LineRange(13,15),
				 LineRange(17,19)};
      RunMaskToRangeList(num_data, mask, ref_lines);      
    }
    {// [[0,0], [2,4], [6,8], [10,12], [14,16], [18,19]]
      bool *offset_mask = &mask[kOffset];
      static_assert(ELEMENTSOF(mask) >= num_data+kOffset, "data length error");
      LineRangeList ref_lines = {LineRange(0,0), LineRange(2,4),
				 LineRange(6,8), LineRange(10,12),
				 LineRange(14,16), LineRange(18,19)};
      RunMaskToRangeList(num_data, offset_mask, ref_lines);      
    }
}

TEST_F(LineFindingUtilsTest, RejectRange) {
  {
    LineRangeList lines = {LineRange(0,2), LineRange(5,25),
			   LineRange(30,49), LineRange(55,80)};
    LineRangeList ref_lines = {LineRange(0,2), LineRange(30,49)};
    if (verbose_) {
      PrintLine(lines);
    }
    LineFinderUtils::rejectWideRange(20,lines);
    cout << "[Regect wide (>20)]" << endl;
    if (verbose_) {
      PrintLine(lines);
    }
    AssertLineRanges(lines, ref_lines);
  }
  {
    LineRangeList lines = {LineRange(0,2), LineRange(5,6),
			   LineRange(20,20), LineRange(72,80)};
    LineRangeList ref_lines = {LineRange(0,2), LineRange(72,80)};
    if (verbose_) {
      PrintLine(lines);
    }
    LineFinderUtils::rejectNarrowRange(3,lines);
    cout << "[Regect narrow (<3)]" << endl;
    if (verbose_) {
      PrintLine(lines);
    }
    AssertLineRanges(lines, ref_lines);
  }
}

TEST_F(LineFindingUtilsTest, DeBin) {
    size_t const binsize = 4;
    size_t const offset = 1;  
    LineRangeList lines = {LineRange(0,2), LineRange(5,6),
			   LineRange(20,20), LineRange(72,80)};
    LineRangeList ref_lines = {LineRange(1,9), LineRange(21,25),
			       LineRange(81,81), LineRange(289,321)};
    if (verbose_) {
      PrintLine(lines);
    }
    LineFinderUtils::deBinRanges(binsize, offset, lines);
    cout << "[debin (bin=" << binsize << ", offset=" << offset <<"]" << endl;
    if (verbose_) {
      PrintLine(lines);
    }
    AssertLineRanges(lines, ref_lines);
}

TEST_F(LineFindingUtilsTest, MergeOverlap) {
  LineRangeList lines = {LineRange(5,8), LineRange(4,6),
			 LineRange(12,14), LineRange(11,15),
			 LineRange(20,23), LineRange(23,25)};
  LineRangeList ref_lines = {LineRange(4,8), LineRange(11,15),
			     LineRange(20,25)};
  if (verbose_) {
    PrintLine(lines);
  }
  LineFinderUtils::mergeOverlappingRanges(lines);
  if (verbose_) {
    cout << "Merge overlapped ranges in an input" << endl;
    PrintLine(lines);
  }
  AssertLineRanges(lines, ref_lines);
}


TEST_F(LineFindingUtilsTest, MergeOverlapTwoLists) {
  LineRangeList lines_from = {LineRange(5,10), LineRange(15,20),
			      LineRange(25,30), LineRange(35,40),
			      LineRange(45,50), LineRange(55,60)};
  {
    LineRangeList lines_to = {LineRange(0,2), LineRange(12,13),
			      LineRange(22,27), LineRange(37,39),
			      LineRange(42,52), LineRange(57,62),
			      LineRange(67,69)};
    LineRangeList ref_lines =  {LineRange(0,2), LineRange(5,10),
				LineRange(12,13), LineRange(15,20),
				LineRange(22,30), LineRange(35,40),
				LineRange(42,52), LineRange(55,62),
				LineRange(67,69)};
    RunMergeIntTwoLists(lines_from, lines_to, ref_lines);
  }
  {// multiple overlap ranges
    LineRangeList lines_to = {LineRange(2,17), LineRange(22,42),
			      LineRange(47,62)};
    LineRangeList ref_lines =  {LineRange(2,20), LineRange(22,42),
				LineRange(45,62)};
    RunMergeIntTwoLists(lines_from, lines_to, ref_lines);
  }
}

TEST_F(LineFindingUtilsTest, ExtendRangeBySign) {
    int8_t sign[NUM_IN];
    bool mask[ELEMENTSOF(sign)];
    size_t num_data = ELEMENTSOF(sign);
    for (size_t i = 0; i<num_data; ++i) {
      sign[i] = ((i/6) % 2)==0 ? 1:-1;
      mask[i] = (i % 5)==0 ? false : true;
    }
    LineRangeList lines = {LineRange(2,2), LineRange(7,7), LineRange(12,12)};
    LineRangeList ref_lines = {LineRange(1,4), LineRange(6,9),
			       LineRange(12,14)};
    if (verbose_) {
      PrintArray("sign", num_data, sign);
      PrintArray("mask", num_data, mask);
      cout << "[input line ranges]" << endl;
      PrintLine(lines);
    }
    LineFinderUtils::extendRangeBySign(num_data, sign, mask, lines);
    if (verbose_) {
      cout << "[output line ranges]" << endl;
      PrintLine(lines);      
    }
    AssertLineRanges(lines, ref_lines);
}

TEST_F(LineFindingUtilsTest, MergeGapByFalse) {
  {
    size_t const maxgap = 5;
    LineRangeList lines = {LineRange(5,10), LineRange(16,18)};
    LineRangeList ref_lines = {LineRange(5,18)};
    RunMergeGapByFalse(maxgap, lines, ref_lines);
  }
  { //change max gap to merge. This time lines would not be merged.
    size_t const maxgap = 4;
    LineRangeList lines = {LineRange(5,10), LineRange(16,18)};
    LineRangeList ref_lines = {LineRange(5,10), LineRange(16,18)};
    RunMergeGapByFalse(maxgap, lines, ref_lines);
  }
}

int main (int nArgs, char * args []) {
    ::testing::InitGoogleTest(& nArgs, args);
    cout << "LineFindingUtils test " << endl;
    return RUN_ALL_TESTS();
}
