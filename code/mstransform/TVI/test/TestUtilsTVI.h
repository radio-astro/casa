//# TestUtilsTVI.h This file contains the interface definition of the TestUtilsTVI class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef TestUtilsTVI_H_
#define TestUtilsTVI_H_

// Google test
#include <gtest/gtest.h>

// casacore containers
#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Containers/Record.h>

// VI/VB framework
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisibilityIterator2.h>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// FreqAxisTVITest class
//////////////////////////////////////////////////////////////////////////
class FreqAxisTVITest: public ::testing::Test {

public:

	FreqAxisTVITest();
	FreqAxisTVITest(Record configuration);
    virtual ~FreqAxisTVITest();

    void SetUp();
    void TearDown();
    Bool getTestResult() {return testResult_p;}

protected:

    void init(Record &configuration);
    virtual void generateTestFile() = 0;
    virtual void generateReferenceFile() = 0;
    virtual void initTestConfiguration(Record &configuration) = 0;
    virtual void initReferenceConfiguration(Record &configuration) = 0;

    Bool autoMode_p;
    Bool testResult_p;
    String inpFile_p;
    String testFile_p;
    String referenceFile_p;
    Record refConfiguration_p;
    Record testConfiguration_p;
};


//////////////////////////////////////////////////////////////////////////
// Convenience methods
//////////////////////////////////////////////////////////////////////////
template <class T> Bool compareVector(	const Char* column,
										const Vector<T> &inp,
										const Vector<T> &ref,
										Float tolerance = FLT_EPSILON);

template <class T> Bool compareMatrix(	const Char* column,
										const Matrix<T> &inp,
										const Matrix<T> &ref,
										Float tolerance = FLT_EPSILON);

template <class T> Bool compareCube(const Char* column,
									const Cube<T> &inp,
									const Cube<T> &ref,
									Float tolerance = FLT_EPSILON);

Bool copyTestFile(String &path,String &filename,String &outfilename);

Bool compareVisibilityIterators(VisibilityIterator2 &testTVI,
								VisibilityIterator2 &refTVI,
								VisBufferComponents2 &columns,
								Float tolerance = FLT_EPSILON);

void flagEachOtherChannel(VisibilityIterator2 &vi);

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* TestUtilsTVI_H_ */
