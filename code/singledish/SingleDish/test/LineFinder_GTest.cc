//# LineFinder_GTest.cc: this defines unit tests of
//# LineFinder using google test framework
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
#include <cmath>
#include <random>

#include <gtest/gtest.h>

#include <casa/Arrays/Vector.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include <libsakura/sakura.h>
#include <singledish/SingleDish/LineFinder.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <singledish/SingleDish/test/SingleDishTestUtil.h>

using namespace casa;
using namespace std;
using namespace test_utility;

typedef pair<size_t,size_t> LineRange;
typedef list<LineRange> LineRangeList;

/*
 * A struct to generate a deterministic normalized uniform random number sequence.
 * The sequence is platform independent.
 *
 * Usage:
 *    NormalizedUniformRandomGenerator<float> uniform_rand = {1111}; //with seed
 *    for (size_t i=0; i<10; ++i) {
 *        cout << uniform_rand.generate() << endl;
 *    }
 */
template <typename DataType>
struct NormalizedUniformRandomGenerator {
  NormalizedUniformRandomGenerator(int seed=1111) {
    mt.seed(seed);
  }
  DataType generate() {
    return static_cast<DataType>(mt())/static_cast<DataType>(mt.max());
  }
  mt19937 mt;
};

/*
 * A helper class to generate deterministic random number sequence in normal destribution.
 * The sequence is platform independent.
 *
 * Usage:
 *     NormalizedUniformRandomGenerator<float> uniform_rand;
 *     BoxMuller<float> normal_rand(0.0, 1.0);
 *     for (size_t i=0; i<10;++i) {
 *         cout << normal_rand.generate(&uniform_rand) << endl;
 *     }
 */
template<typename DataType>
class BoxMuller {
public:
  BoxMuller(DataType const mean, DataType const stddev) : mean_(mean), stddev_(stddev){
  }
  ~BoxMuller() {}
  DataType generate(NormalizedUniformRandomGenerator<DataType> *generator) {
    if (cashed) {
      cashed = false;
      return cashed_value;
    }
    else {
      DataType const urand0 = generator->generate();
      DataType const urand1 = generator->generate();
      cashed_value = mean_ + stddev_ * sqrt(-2*log(1-urand0))*sin(k2PI*urand1);
      cashed = true;
      return mean_ + stddev_ * sqrt(-2*log(1-urand0))*cos(k2PI*urand1);
    }
  }

private:
  DataType mean_;
  DataType stddev_;
  DataType cashed_value = 0.0;
  bool cashed = false;
  const DataType k2PI = 8*atan(1); //2*PI
};

class LineFinderTest : public ::testing::Test {
protected:
  LineFinderTest() : verbose_(false) {}

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

  void AssertLineRanges(LineRangeList &data, LineRangeList &reference) {
    // test number of data
    EXPECT_EQ(data.size(), reference.size());
    // test line ranges
    LineRangeList::iterator outiter = data.begin();
    LineRangeList::iterator refiter = reference.begin();
    for (;outiter!=data.end() && refiter!=reference.end();
	 ++outiter, ++refiter) {
      EXPECT_EQ((*refiter).first, (*outiter).first);
      EXPECT_EQ((*refiter).second, (*outiter).second);
    }
  }

  bool verbose_;
};

TEST_F(LineFinderTest, LineFinding) {
  auto GaussElement = [] (size_t const i, float const center, float const peak, float const width) {
    return peak * exp( -0.5 * pow( (static_cast<float>(i)-center)/width, 2) );
  };
  constexpr size_t kNumData = 1024;
  float data[kNumData];
  bool mask[ELEMENTSOF(data)];
  size_t const num_data = kNumData;
  pair<size_t,size_t> edge(20,10);
  float const peak = 5.0;
  float const width = 10.0;
  float const stddev = 0.5;
//  mt19937 mt(1111);
//  normal_distribution<float> rand(0.0, stddev);
  NormalizedUniformRandomGenerator<float> urand = {3333};
  BoxMuller<float> nrand(0.0, stddev);
  // two gaussian + random noize
  for (size_t i = 0; i<kNumData; ++i) {
    data[i] = nrand.generate(&urand)
      + GaussElement(i, kNumData*0.5, peak, width)
      + GaussElement(i, kNumData*0.75, peak*2.0, width/2.0);
    mask[i] = true;
  }
  // edge
  for (size_t i=0; i<edge.first; ++i) {
    data[i] += 50.0;
  }
  // masked channels
  for (size_t i=kNumData/4; i<kNumData/4+10; ++i) {
    data[i] += 20.0;
    mask[i] = false;
  }

  LineRangeList line_list = \
    linefinder::MADLineFinder(num_data, data, mask, 3.0, 5, 10, kNumData/5, 4, edge);
  if (verbose_) {
    cout << "[Line finding result]" << endl;
    PrintLine(line_list);
  }
  LineRangeList line_ref = {LineRange(490, 533), LineRange(754, 781)};
  AssertLineRanges(line_list, line_ref);
}

/*
 * An example of using read-only data in data/regression/
 * Not testing anything.
 */
TEST_F(LineFinderTest, LineFindingST) {
    string table_name = GetCasaDataPath()
      + "regression/unittest/singledish/sd_analytic_type3-1.asap";

    size_t row_idx = 0;
    cout << "Table: " << table_name << endl;
    cout << "idx: " << row_idx << endl;
    Table mytab(table_name, Table::Old);
    assert(row_idx<mytab.nrow());
    ScalarColumn<unsigned int> flagRCol(mytab, "FLAGROW");
    assert(flagRCol.get(row_idx)==0);
    ArrayColumn<float> specCol(mytab, "SPECTRA");
    ArrayColumn<uint8_t> flagCol(mytab, "FLAGTRA");
    Vector<float> specvec(specCol.get(row_idx));
    Vector<uint8_t> flagvec(flagCol.get(row_idx));
    size_t num_data(specvec.nelements());
    cout << "nchan: " << num_data << endl;
    Vector<float> data(num_data);
    Vector<bool> mask(num_data);
    for (size_t i=0; i<num_data; ++i) {
      data[i] = specvec[i];
      mask[i] = (flagvec[i]==static_cast<uint8_t>(0));
    }
    pair<size_t,size_t> edge(5,10);
    LineRangeList line_list = \
      linefinder::MADLineFinder(num_data, data.data(), mask.data(), 5.0, 10.0, 3, 1000, 4, edge);
    cout << "[Line finding result]" << endl;
    PrintLine(line_list);
}

int main (int nArgs, char * args []) {
    ::testing::InitGoogleTest(& nArgs, args);
    cout << "LineFinder test " << endl;
    return RUN_ALL_TESTS();
}
