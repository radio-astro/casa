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

namespace test 
{

class EphemObjFluxStandardTest: public FluxStandardTest 
{

public:

    EphemObjFluxStandardTest();
    EphemObjFluxStandardTest(casacore::Int nspw);
    virtual ~EphemObjFluxStandardTest();
    
protected:
    
    virtual void SetUp();
    virtual void TearDown();
    casacore::Bool ephemExists(); 
    casacore::String foundEphemPath; 
    casacore::String flxStdName;          
    casacore::String srcName;
    casacore::String flxStdDesc;
    casacore::Double freq;
    casacore::MFrequency mfreq;
    casacore::MEpoch mtime;
    casacore::MDirection fieldDir;
    casa::FluxStandard::FluxScale flxStdEnum;
    casacore::Vector<casacore::Double> fluxUsed;
    casacore::Bool foundStd;
    casa::Flux<casacore::Double> returnFlux, returnFluxErr;
    casacore::Vector<casacore::Vector<casacore::MFrequency> > spws;
    casacore::Vector<casacore::String> tempCLs;
    casacore::Vector<casacore::Vector<casa::Flux<casacore::Double> >> returnFluxes, returnFluxErrs;
    casa::ComponentList* cl;
    // expected values
    casa::FluxStandard::FluxScale expFlxStdEnum;
};

class MatchStandardTest:public EphemObjFluxStandardTest {

public:
    
    MatchStandardTest();
    virtual ~MatchStandardTest();

protected:
  
    virtual void SetUp();
    virtual void TearDown();
    casacore::Bool matchedStandard;
    casacore::String flxStdDesc;

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
};

class TwoChanSpwFluxValTest: public EphemObjFluxStandardTest,
       public ::testing::WithParamInterface<std::string>
{

public:
  
    TwoChanSpwFluxValTest();
    virtual ~TwoChanSpwFluxValTest();
   
protected:
    
    virtual void SetUp();
    virtual void TearDown();
    
};

}// end namespace test


#endif /* COMPONENTS_COMPONENTMODELS_TEST_EPHEMOBJFLUXSTANDARD_GTEST_H */
