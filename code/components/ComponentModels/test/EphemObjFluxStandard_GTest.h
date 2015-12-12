//# EphemObjFluxStandard_GTest.h: definition  of FluxStandard google test            
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

#ifndef COMPONENTS_COMPONENTMODELS_TEST_EPHEMOBJFLUXSTANDARD_GTEST_H
#define COMPONENTS_COMPONENTMODELS_TEST_EPHEMOBJFLUXSTANDARD_GTEST_H

#include <memory>
#include <gtest/gtest.h>

#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/FluxStandard.h>
#include <components/ComponentModels/test/FluxStandardTest.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>

using namespace casa;

namespace test 
{

class EphemObjFluxStandardTest: public FluxStandardTest 
{

public:

    EphemObjFluxStandardTest();
    virtual ~EphemObjFluxStandardTest();
    
protected:
    
    virtual void SetUp();
    virtual void TearDown();
    Bool ephemExists(); 

    String foundEphemPath; 
    String flxStdName;          
    String srcName;
    String flxStdDesc;
    Double freq;
    MFrequency mfreq;
    MEpoch mtime;
    MDirection srcDir;
    FluxStandard::FluxScale flxStdEnum;
    Vector<Double> fluxUsed;
    Flux<Double> returnFlux, returnFluxErr;
    Bool foundStd;
    // expected values
    FluxStandard::FluxScale expFlxStdEnum;
};

class MatchStandardTest:public EphemObjFluxStandardTest {

public:
    
    MatchStandardTest();
    virtual ~MatchStandardTest();

protected:
  
    virtual void SetUp();
    virtual void TearDown();
    Bool matchedStandard;
    String flxStdDesc;

}; 

class SingleChanSpwFluxValTest: public EphemObjFluxStandardTest, 
       public ::testing::WithParamInterface<std::string>
{

public:

    SingleChanSpwFluxValTest();
    virtual ~SingleChanSpwFluxValTest();
    
protected:
    
    virtual void SetUp();
    virtual void TearDown();
    Vector<Vector<MFrequency> > spws;
    Double expFlxVal;
    const MDirection fieldDir;
    Vector<String> tempCLs;
    Vector<Vector<Flux<Double> >> returnFluxes, returnFluxErrs;
    ComponentList* cl;
};
}// end namespace test


#endif /* COMPONENTS_COMPONENTMODELS_TEST_EPHEMOBJFLUXSTANDARD_GTEST_H */
