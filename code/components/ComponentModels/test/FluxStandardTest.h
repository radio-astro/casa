//# FluxStandard.h: definition  of FluxStandard google test base class           
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

#ifndef COMPONENTS_COMPONENTMODELS_TEST_FLUXSTANDARDTEST_H
#define COMPONENTS_COMPONENTMODELS_TEST_FLUXSTANDARDTEST_H

#include <memory>
#include <gtest/gtest.h>

#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <components/ComponentModels/FluxStandard.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>

namespace test 
{

//Base class for FluxStandard unit tests
class FluxStandardTest: public ::testing::Test  
{

public:

    FluxStandardTest();
    virtual ~FluxStandardTest();

protected:

    virtual void SetUp();
    virtual void TearDown();
    casacore::String fluxScaleName;
    std::unique_ptr<casa::FluxStandard> fluxStd;
// things to test
// compute (2 signatures)
// computeCL
// setInterpMethod
// makeComponentList (2 signatures)
// matchStandard 
// standardName
//
};

}// end namespace test

#endif /* COMPONENTS_COMPONENTMODELS_TEST_FLUXSTANDARDTEST_H */
