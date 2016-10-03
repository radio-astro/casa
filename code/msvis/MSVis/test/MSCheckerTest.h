//# MSCheckerFixture.h
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

#ifndef CODE_MSVIS_MSVIS_TEST_MSCHECKERFIXTURE_H_
#define CODE_MSVIS_MSVIS_TEST_MSCHECKERFIXTURE_H_

#include <gtest/gtest.h>

#include <casacore/casa/BasicSL/String.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>

#include <memory>

using namespace std;
using namespace casacore;

namespace test {

class MSCheckerTest : public ::testing::Test {

public:
	MSCheckerTest();
	virtual ~MSCheckerTest();

	virtual void SetUp();

	virtual void TearDown();

	unique_ptr<casacore::MeasurementSet> testMS;

private:
	casacore::String _dataPath;

};

#endif /* CODE_MSVIS_MSVIS_TEST_MSCHECKERFIXTURE_H_ */

}
