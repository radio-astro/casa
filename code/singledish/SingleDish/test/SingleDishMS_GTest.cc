//# SingleDishMS_GTest.cc: this defines unit tests of
//# SingleDishMS using google test framework
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
#include <singledish/SingleDish/SingleDishMS.h>
#include <singledish/SingleDish/test/SingleDishTestUtil.h>

using namespace casacore;
using namespace casacore;
using namespace casacore;
using namespace casacore;
using namespace casacore;
using namespace casacore;
using namespace casacore;
using namespace casa;
using namespace std;
using namespace test_utility;

class SingleDishMSTest : public ::testing::Test {
protected:
  SingleDishMSTest() : verbose_(false) {}

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

  bool verbose_;
};

/* NOTE no test has defined yet */

int main (int nArgs, char * args []) {
    ::testing::InitGoogleTest(& nArgs, args);
    cout << "SingleDishMS test " << endl;
    return RUN_ALL_TESTS();
}
