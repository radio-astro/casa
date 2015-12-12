//# EphemObjFluxStandard_GTest.cc: implementation of EphemObjFluxStandard google test   
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
//
#include <casa/aips.h>
#include <casa/System/Aipsrc.h>
#include <casa/OS/Path.h>
#include <components/ComponentModels/test/EphemObjFluxStandard_GTest.h>
#include <measures/Measures/MFrequency.h>
#include <limits>
#include <map>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/TwoSidedShape.h>
typedef std::numeric_limits< double > dbl;

using namespace casa;
using namespace std; 

namespace test {

EphemObjFluxStandardTest::EphemObjFluxStandardTest()
  : fluxUsed(4), foundStd(false)
{
};
 
EphemObjFluxStandardTest::~EphemObjFluxStandardTest() {};

void EphemObjFluxStandardTest::SetUp() 
{
  flxStdName = "Butler-JPL-Horizons 2010"; 
  expFlxStdEnum = FluxStandard::SS_JPL_BUTLER; 
  //mtime = MEpoch(Quantity(56293.0,"d")); 
  //srcDir = MDirection(MVDirection(Quantity(0.0,"rad"),Quantity(0.0,"rad")), MDirection::J2000);
};

void EphemObjFluxStandardTest::TearDown()
{
}

Bool EphemObjFluxStandardTest::ephemExists() {
  Bool dataExists = Aipsrc::findDir(foundEphemPath, "data/ephemerides/JPL-Horizons");
  return dataExists;
}
  
//MatchStandardTest
MatchStandardTest::MatchStandardTest(): matchedStandard(false) {};

MatchStandardTest::~MatchStandardTest() {};
void MatchStandardTest::SetUp() 
{
   EphemObjFluxStandardTest::SetUp();
};

void MatchStandardTest::TearDown() {};

//SingleChanSpwFluxValTest
SingleChanSpwFluxValTest::SingleChanSpwFluxValTest():
                            spws(2),tempCLs(2),returnFluxes(2), returnFluxErrs(2)
{
};

SingleChanSpwFluxValTest::~SingleChanSpwFluxValTest() {};

void SingleChanSpwFluxValTest::SetUp() 
{
  EphemObjFluxStandardTest::SetUp();
  Double mjd = 56000.1;
  mtime = MEpoch(MVEpoch(Quantity(mjd, "d")), MEpoch::Ref(MEpoch::UTC));
  spws[0].resize(1);
  spws[1].resize(1);
  spws[0][0] = MFrequency(Quantity(115.0,"GHz"));
  spws[1][0] = MFrequency(Quantity(345.0,"GHz"));
  returnFluxes[0].resize(1);
  returnFluxes[1].resize(1);
  returnFluxErrs[0].resize(1);
  returnFluxErrs[1].resize(1);
};

void SingleChanSpwFluxValTest::TearDown() 
{
  delete cl;
  for (uInt i=0; i< spws.nelements(); i++) {
     if(tempCLs[i] != ""){
            if(Table::canDeleteTable(tempCLs[i]))
              Table::deleteTable(tempCLs[i]);
     }
  }
};

// Parameters to be tested
// src list
vector<string> srcs  = {"Venus",
                        "Mars", 
                        "Ceres", 
                        "Jupiter", 
                        "Ganymede", 
                        "Titan", 
                        "Uranus",
                        "Neptune",
                        "Triton",
                        "Pluto",
                        "Vesta"};
//expected ang. diam.
map<string, Double> expAngDiam = {
  {"Venus",9.99015e-05},
  {"Mars", 6.64545e-05},
  {"Ceres",1.70353e-06},
  {"Jupiter",0.000164169},
  {"Ganymede",6.25104e-06},
  {"Titan",3.87696e-06},
  {"Uranus",1.60618e-05},
  {"Neptune",1.06361e-05},
  {"Triton",5.84913e-07},
  {"Pluto",4.92412e-07},
  {"Vesta",1.02053e-06}
};

// expected flux densities at 
// 115 and 345GHz
map<string, vector<Double> > expFlux = {
  {"Venus",{1056.06,7525.55}},
  {"Mars",{292.084,2559.98}},
  {"Ceres",{0.152118,1.32408}},
  {"Jupiter",{1438.55,12529.0}},
  {"Ganymede",{1.33757,11.4395}},
  {"Titan",{0.35435,2.96232}},
  {"Uranus",{9.74463,57.3713}},
  {"Neptune",{4.32918,24.7793}},
  {"Triton",{0.00385483, 0.0297936}},
  {"Pluto",{0.00250031,0.0190617}},
  {"Vesta",{0.052266,0.454264}}
};

TEST_F(MatchStandardTest, checkStandardMatch)
{
  matchedStandard  = FluxStandard::matchStandard(flxStdName, flxStdEnum, flxStdDesc); 
  EXPECT_TRUE(matchedStandard);
  EXPECT_EQ(flxStdEnum, expFlxStdEnum);
};

INSTANTIATE_TEST_CASE_P(checkFlux, SingleChanSpwFluxValTest, ::testing::ValuesIn(srcs));
TEST_P(SingleChanSpwFluxValTest, checkFlux)
{
  // calc values were checked against calculation against a python script to be consistent at the tolerance level of  
  Double tol = 1.0e-5;
  if (ephemExists()) {
     //cerr<<"OK ephem data exists! ephemDataFullPath="<<foundEphemPath<<endl;
     String ssobjname(GetParam());
     fluxStd.reset(new FluxStandard(expFlxStdEnum));
     fluxStd->computeCL(ssobjname, spws, mtime, fieldDir, returnFluxes, returnFluxErrs, tempCLs, "setjy_");
     // test ang diameter
     for(uInt spwInd = 0; spwInd < spws.nelements(); spwInd++) {
       if(spwInd == 0 ) {
         cl = new ComponentList(Path(tempCLs[0]));
         //cerr<<" tempCLs[0] = "<<tempCLs[0]<<endl;
         Double angDiam = ((TwoSidedShape*)(cl->getShape(0)))->majorAxisInRad();
         //cerr<<ssobjname<<": major axis="<<angDiam<<endl;
         //cerr<<" exp value ="<<expAngDiam[ssobjname]<<endl;
         EXPECT_NEAR(expAngDiam[ssobjname], angDiam,tol); 
       }
       returnFluxes[spwInd][0].value(fluxUsed);
       //cerr<<"calculated f="<<fluxUsed[0]<<" expected="<<(expFlux[ssobjname])[spwInd]<<endl;
       EXPECT_TRUE(fabs(fluxUsed[0]/expFlux[ssobjname][spwInd]-1.0) < tol); 
     } 
  }
  else {
     cout <<" The ephemeris database does not exist in "<<foundEphemPath<<" . Skip this test"<<endl;
  }
};

}//end namespace test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
