//# SDMaskHandler_GTest.h: definition  of SDMaskHandler google test
//#
//# Copyright (C) 2015
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
//# 51 Franklin Street, Fifth FloorBoston, MA 02110-1335, USA
//#

#ifndef SYNTHESIS_SDMASKHANDLER_TEST_SDMASKHANDLER_GTEST_H
#define SYNTHESIS_SDMASKHANDLER_TEST_SDMASKHANDLER_GTEST_H

#include <gtest/gtest.h>

#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Array.h>
#include <coordinates/Coordinates/CoordinateSystem.h>


using namespace casa;

namespace test {

//Base class for SDMaskHandler unit tests
class SDMaskHandlerTest: public ::testing::Test {

public:

    SDMaskHandlerTest();
    virtual ~SDMaskHandlerTest();
protected:
    virtual void SetUp();
    virtual void TearDown();
    //
    // box mask cube image generator (only create mask for stoke I) 
    void generateBoxMaskImage(String imagename, Int imsize, Int nchan, IPosition blc, IPosition trc);

    String outMaskName;
    CoordinateSystem csys;
};

//Test methods that involve ImageInterface
class ImageInterfaceTest: public SDMaskHandlerTest {

public:
     ImageInterfaceTest();
     virtual ~ImageInterfaceTest();

protected:
     void testMakeMaskByThreshold();
     void testRegionToMaskImage();
     void testRegionText();
     void testCopyMask();

};

}//end namespace test

#endif /* SYNTHESIS_SDMASKHANDLER_TEST_SDMASKHANDLER_GTEST_H */
