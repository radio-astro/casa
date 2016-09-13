//# tVisCal.cc: Tests the VisCal framework
//# Copyright (C) 1995,1999,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

//#include <casa/aips.h>
//#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
//#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
#include <synthesis/MeasurementComponents/FringeJones.h>

#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casa::vi;

class FringeJonesTest : public ::testing::Test {

public:

  virtual void SetUp() {
    nAnt=5;
    nSpw=1;
  }

  Int nAnt,nSpw;

};


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST_F(FringeJonesTest, Basic) {

  VisCal *fringe = new FringeJones("<noms>",nAnt,nSpw);
  fringe->setApply();

  ASSERT_EQ(VisCalEnum::JONES,fringe->matrixType());
  ASSERT_EQ(VisCal::K,fringe->type());
  ASSERT_EQ(String("Fringe Jones"),fringe->typeName());
  ASSERT_EQ(6,fringe->nPar());
  ASSERT_EQ(False,fringe->freqDepPar());
  ASSERT_EQ(True,fringe->freqDepMat());
  ASSERT_EQ(False,fringe->freqDepCalWt());
  ASSERT_EQ(True,fringe->timeDepMat());
  ASSERT_EQ(True,fringe->isApplied());
  ASSERT_EQ(True,fringe->isSolvable());

  delete fringe;
}
