//# MSCheckerFixture.cc
//# Copyright (C) 1998,1999,2000,2001
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

#include <msvis/MSVis/test/MSCheckerTest.h>

#include <casacore/casa/OS/EnvVar.h>

#include <memory>

using namespace casacore;
using namespace casacore;

using namespace std;

namespace test {

MSCheckerTest::MSCheckerTest() {
	String *parts = new String[2];
	split(EnvironmentVariable::get("CASAPATH"), parts, 2, String(" "));
	String datadir = parts[0] + "/data/";
	_dataPath = datadir + "regression/unittest/MSMetaData/checker.ms";
}

MSCheckerTest::~MSCheckerTest() {}

void MSCheckerTest::SetUp() {

	MeasurementSet ms(_dataPath);
	testMS.reset(new MeasurementSet(ms.copyToMemoryTable("")));
}

void MSCheckerTest::TearDown() {
	testMS.reset(nullptr);
}

}
