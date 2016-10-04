//# tVisCalGlobals.cc: Tests the VisCalGlobals functions
//# Copyright (C) 1995,1999,2000,2001,2016
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
#include <casa/Arrays/Slice.h>
#include <casa/OS/Timer.h>
#include <casacore/casa/OS/EnvVar.h>
#include <casacore/casa/OS/Path.h>

#include <synthesis/MeasurementComponents/VisCalGlobals.h>

#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::viscal;

class VisCalGlobalsTest : public :: testing::Test {

public:
  Bool compareSlices(Slice s1,Slice s2) {
    return (s1.start()==s2.start() &&
	    s1.length()==s2.length() &&
	    s1.inc()==s2.inc());
  }

};


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST_F(VisCalGlobalsTest, getCalSliceByTypeTest) {

  //cout << "calParSliceByType(G JONES,amp, ) = " << calParSliceByType("G JONES","amp","") << endl;
  ASSERT_TRUE(compareSlices(Slice(0,2,1),calParSliceByType("G JONES","amp","")));
  //cout << "calParSliceByType(G JONES,amp, ) = " << calParSliceByType("G JONES","phase","") << endl;
  ASSERT_TRUE(compareSlices(Slice(0,2,1),calParSliceByType("G JONES","amp","")));
  //cout << "calParSliceByType(G JONES,amp, ) = " << calParSliceByType("G JONES","real","") << endl;
  ASSERT_TRUE(compareSlices(Slice(0,2,1),calParSliceByType("G JONES","amp","")));
  //cout << "calParSliceByType(G JONES,amp, ) = " << calParSliceByType("G JONES","imag","") << endl;
  ASSERT_TRUE(compareSlices(Slice(0,2,1),calParSliceByType("G JONES","amp","")));
  //cout << "calParSliceByType(G JONES,amp,R) = " << calParSliceByType("G JONES","amp","R") << endl;
  ASSERT_TRUE(compareSlices(Slice(0,1,1),calParSliceByType("G JONES","amp","R")));
  //cout << "calParSliceByType(G JONES,amp,L) = " << calParSliceByType("G JONES","amp","L") << endl;
  ASSERT_TRUE(compareSlices(Slice(1,1,1),calParSliceByType("G JONES","amp","L")));

  //cout << "calParSliceByType(EVLASWP,gainamp, ) = " << calParSliceByType("EVLASWP","gainamp","") << endl;
  ASSERT_TRUE(compareSlices(Slice(0,2,2),calParSliceByType("EVLASWP","gainamp","")));
  //cout << "calParSliceByType(EVLASWP,Tsys, ) = " << calParSliceByType("EVLASWP","Tsys","") << endl;
  ASSERT_TRUE(compareSlices(Slice(1,2,2),calParSliceByType("EVLASWP","Tsys","")));
  //cout << "calParSliceByType(EVLASWP,gainamp, ) = " << calParSliceByType("EVLASWP","gainamp","R") << endl;
  ASSERT_TRUE(compareSlices(Slice(0,1,2),calParSliceByType("EVLASWP","gainamp","R")));
  //cout << "calParSliceByType(EVLASWP,Tsys, ) = " << calParSliceByType("EVLASWP","Tsys","R") << endl;
  ASSERT_TRUE(compareSlices(Slice(1,1,2),calParSliceByType("EVLASWP","Tsys","R")));
  //cout << "calParSliceByType(EVLASWP,gainamp, ) = " << calParSliceByType("EVLASWP","gainamp","L") << endl;
  ASSERT_TRUE(compareSlices(Slice(2,1,2),calParSliceByType("EVLASWP","gainamp","L")));
  //cout << "calParSliceByType(EVLASWP,Tsys, ) = " << calParSliceByType("EVLASWP","Tsys","L") << endl;
  ASSERT_TRUE(compareSlices(Slice(3,1,2),calParSliceByType("EVLASWP","Tsys","L")));


  /*
  Cube<Int>  c(4,1,10);
  indgen(c);

  Cube<Int> d;
  d.assign(calParSliceByType("EVLASWP","gainamp",""),Slice(),Slice()));

  cout << "c = " << c << endl;
  cout << "d = " << d  << endl;


  cout << boolalpha << d.contiguousStorage() << endl;
  */

}

/*
TEST_F(VisCalGlobalsTest, getCalSliceTest) {

  String *casapath = new String[2];
  split(EnvironmentVariable::get("CASAPATH"), casapath, 2, String(" "));
  // Use of Path().absoluteName() absorbs relative stuff in casapath
  String mspath(Path(casapath[0]+"/data/regression/unittest/flagdata/Four_ants_3C286.ms").absoluteName());


  NewCalTable ct("test.G",VisCalEnum::COMPLEX,"G Jones",mspath,True);
  ct.writeToDisk("test.G");

  cout << "calParSlice(test.G,amp, ) = " << calParSlice("test.G","amp","") << endl;
  ASSERT_TRUE(compareSlices(Slice(0,2,1),calParSlice("test.G","amp","")));

}
*/
