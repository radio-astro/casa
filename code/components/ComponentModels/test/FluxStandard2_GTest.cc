//# FluxStandard2_GTest.cc: implementation of FluxStandard2 google test   
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
#include <casa/System/Aipsrc.h>
#include <components/ComponentModels/test/FluxStandard2_GTest.h>
#include <measures/Measures/MFrequency.h>
#include <limits>
typedef std::numeric_limits< double > dbl;

using namespace casa;
using namespace std; 

namespace test {

NewFluxStandardTest::NewFluxStandardTest()
  : fluxUsed(4), foundStd(false)
{
};
 
NewFluxStandardTest::~NewFluxStandardTest() {};

void NewFluxStandardTest::SetUp() 
{
  mtime = MEpoch(Quantity(56293.0,"d")); //2013-01-01
  //mtime = MEpoch(Quantity(54832.0,"d")); //2009-01-01
  srcDir = MDirection(MVDirection(Quantity(0.0,"rad"),Quantity(0.0,"rad")), MDirection::J2000);
};

Bool NewFluxStandardTest::modelExists(String tablename) {
  String stdPath = "data/nrao/VLA/standards/";
  Bool dataExists = Aipsrc::findDir(foundModelPath, stdPath+tablename);
  return dataExists;
};

void NewFluxStandardTest::TearDown() {};
//
//MatchStandardTest
MatchStandardTest::MatchStandardTest(): matchedStandard(false) {};

MatchStandardTest::~MatchStandardTest() {};
void MatchStandardTest::SetUp() 
{
  flxStdName = std::tr1::get<0>(GetParam());
  expFlxStdEnum = std::tr1::get<1>(GetParam());
}

void MatchStandardTest::TearDown() {};

AltSrcNameTest::AltSrcNameTest() {};
AltSrcNameTest::~AltSrcNameTest() {};
void AltSrcNameTest::SetUp() 
{
  coeffsTbName = "PerleyButler2013Coeffs";
  flxStdName = std::tr1::get<0>(GetParam());
  srcName = std::tr1::get<1>(GetParam());
  freq = std::tr1::get<2>(GetParam());
  expFlxStdEnum = std::tr1::get<3>(GetParam());
}

void AltSrcNameTest::TearDown() { };

SetInterpMethodTest::SetInterpMethodTest() {};
SetInterpMethodTest::~SetInterpMethodTest() {};
void SetInterpMethodTest::SetUp()
{
  NewFluxStandardTest::SetUp();
  coeffsTbName = "PerleyButler2013Coeffs";
  interpMethod = std::tr1::get<0>(GetParam());
  expFlxVal = std::tr1::get<1>(GetParam());
}
void SetInterpMethodTest::TearDown() {};

FluxValueTest::FluxValueTest() {};
FluxValueTest::~FluxValueTest() {};
void FluxValueTest::SetUp() 
{
  NewFluxStandardTest::SetUp();
  flxStdName = std::tr1::get<0>(GetParam());
  srcName = std::tr1::get<1>(GetParam());
  freq = std::tr1::get<2>(GetParam());
  expFlxStdEnum = std::tr1::get<3>(GetParam());
  expFlxVal = std::tr1::get<4>(GetParam());
}
void FluxValueTest::TearDown() {};


//Define parameter sets to be tested
// short param set just for the match check
// (note that for this srcname, freq, and expflux  are dummy values)
std::tr1::tuple<String,FluxStandard::FluxScale> flxStdParams[] =
{
  make_tuple("Perley-Butler 2013", FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Scaife-Heald 2012", FluxStandard::SCAIFE_HEALD_2012)
};

// for testing alternative source names
std::tr1::tuple<String,String,Double,FluxStandard::FluxScale> flxStdParamsAltSrcNames[] =
{
// 2. P-B 2013, 7. Scaife-Heald 2012 
//3C Name B1950 Name J2000 Name Alt. J2000 Name  standards*
//            3C48    0134+329   0137+331   J0137+3309       2,7
//            3C123   0433+295   0437+296   J0437+2940       2 
//            3C138   0518+165   0521+166   J0521+1638       2
//            3C147   0538+498   0542+498   J0542+4951       2,7
//            3C196   0809+483   0813+482   J0813+4813       2,7 
//            3C286   1328+307   1331+305   J1331+3030       2,7
//            3C295   1409+524   1411+522   J1411+5212       2,7
//            3C380   1828+487   1829+487   J1829+4845       7
  //3c48
  make_tuple("Perley-Butler 2013","3C48", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0134+329", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0137+331", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","J0137+3309", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Scaife-Heald 2012","3C48", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","0134+329", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","0137+331", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","J0137+3309", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  //3c123
  make_tuple("Perley-Butler 2013","3C123", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0433+295", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0437+296", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","J0437+2940", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  //3c138
  make_tuple("Perley-Butler 2013","3C138", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0518+165", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0521+166", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","J0521+1638", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  //3c147
  make_tuple("Perley-Butler 2013","3C147", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0538+498", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0542+498", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","J0542+4951", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Scaife-Heald 2012","3C147", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","0538+498", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","0542+498", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","J0542+4951", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  //3C196   0809+483   0813+482   J0813+4813       
  make_tuple("Perley-Butler 2013","3C196", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0809+483", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","0813+482", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","J0813+4813", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Scaife-Heald 2012","3C196", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","0809+483", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","0813+482", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","J0813+4813", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  //3C286   1328+307   1331+305   J1331+3030       1
  make_tuple("Perley-Butler 2013","3C286", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","1328+307", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","1331+305", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","J1331+3030", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Scaife-Heald 2012","3C286", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","1328+307", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","1331+305", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","J1331+3030", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  //3C295   1409+524   1411+522   J1411+5212       1,3,4,5,6         
  make_tuple("Perley-Butler 2013","3C295", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","1409+524", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","1411+522", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Perley-Butler 2013","J1411+5212", 2.0, FluxStandard::PERLEY_BUTLER_2013),
  make_tuple("Scaife-Heald 2012","3C295", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","1409+524", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","1411+522", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","J1411+5212", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  //3C380   1828+487   1829+487   J1829+4845       7
  make_tuple("Scaife-Heald 2012","3C380", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","1828+487", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","1829+487", 0.2, FluxStandard::SCAIFE_HEALD_2012),
  make_tuple("Scaife-Heald 2012","J1829+4845", 0.2, FluxStandard::SCAIFE_HEALD_2012)
};

// for setInterpMethod test
std::tr1::tuple<String,Double> flxStdParamsVarSrc[] =
{
  //for Freq = 2GHz at epoch 2013.01.01
  make_tuple("nearest",  12.157566070556641),
  make_tuple("linear", 12.121262550354004),
  make_tuple("cubic", 12.0489845275878918),
  make_tuple("spline", 12.119579315185547)
};

//for Flux Value test
std::tr1::tuple<String,String,Double,FluxStandard::FluxScale,Double> flxStdParamsFull[] =
{
//            3C48    0134+329   0137+331   J0137+3309       2,7
//            3C123   0433+295   0437+296   J0437+2940       2 
//            3C138   0518+165   0521+166   J0521+1638       2
//            3C147   0538+498   0542+498   J0542+4951       2,7
//            3C196   0809+483   0813+482   J0813+4813       2,7 
//            3C286   1328+307   1331+305   J1331+3030       2,7
//            3C295   1409+524   1411+522   J1411+5212       2,7
//            3C380   1828+487   1829+487   J1829+4845       7
  make_tuple("Perley-Butler 2013","3C48", 2.0, FluxStandard::PERLEY_BUTLER_2013, 12.119579315185547),
  make_tuple("Perley-Butler 2013","3C48", 20.0, FluxStandard::PERLEY_BUTLER_2013, 1.3227671384811401),
  make_tuple("Perley-Butler 2013","3C123", 2.0, FluxStandard::PERLEY_BUTLER_2013, 35.962474822998047),
  make_tuple("Perley-Butler 2013","3C123", 20.0, FluxStandard::PERLEY_BUTLER_2013, 3.7041726112365723),
  make_tuple("Perley-Butler 2013","3C138", 2.0, FluxStandard::PERLEY_BUTLER_2013, 7.1939225196838379),
  make_tuple("Perley-Butler 2013","3C138", 20.0, FluxStandard::PERLEY_BUTLER_2013, 1.5115476846694946),
  make_tuple("Perley-Butler 2013","3C147", 2.0, FluxStandard::PERLEY_BUTLER_2013, 16.785844802856445),
  make_tuple("Perley-Butler 2013","3C147", 20.0, FluxStandard::PERLEY_BUTLER_2013, 1.9164166450500488),
  make_tuple("Perley-Butler 2013","3C196", 2.0, FluxStandard::PERLEY_BUTLER_2013, 10.469700813293457),
  make_tuple("Perley-Butler 2013","3C196", 20.0, FluxStandard::PERLEY_BUTLER_2013, 0.85275018215179443),
  make_tuple("Perley-Butler 2013","3C286", 2.0, FluxStandard::PERLEY_BUTLER_2013, 12.53865909576416),
  make_tuple("Perley-Butler 2013","3C286", 20.0, FluxStandard::PERLEY_BUTLER_2013, 2.7294557094573975),
  make_tuple("Perley-Butler 2013","3C295", 2.0, FluxStandard::PERLEY_BUTLER_2013, 16.616117477416992),
  make_tuple("Perley-Butler 2013","3C295", 20.0, FluxStandard::PERLEY_BUTLER_2013, 1.1097481250762939),
  make_tuple("Scaife-Heald 2012","3C48", 0.03, FluxStandard::SCAIFE_HEALD_2012, 65.291659535926982),
  make_tuple("Scaife-Heald 2012","3C48", 0.3, FluxStandard::SCAIFE_HEALD_2012, 45.89225204709156),
  make_tuple("Scaife-Heald 2012","3C147", 0.03, FluxStandard::SCAIFE_HEALD_2012, 14.383075784917214),
  make_tuple("Scaife-Heald 2012","3C147", 0.3, FluxStandard::SCAIFE_HEALD_2012, 55.083336564420961),
  make_tuple("Scaife-Heald 2012","3C196", 0.03, FluxStandard::SCAIFE_HEALD_2012, 226.12882796186065),
  make_tuple("Scaife-Heald 2012","3C196", 0.3, FluxStandard::SCAIFE_HEALD_2012, 50.018346586474074),
  make_tuple("Scaife-Heald 2012","3C286", 0.03, FluxStandard::SCAIFE_HEALD_2012, 42.316362172505563),
  make_tuple("Scaife-Heald 2012","3C286", 0.3, FluxStandard::SCAIFE_HEALD_2012, 24.512981691947271),
  make_tuple("Scaife-Heald 2012","3C295", 0.03, FluxStandard::SCAIFE_HEALD_2012, 92.395842606321992),
  make_tuple("Scaife-Heald 2012","3C295", 0.3, FluxStandard::SCAIFE_HEALD_2012, 63.225661243858283),
  make_tuple("Scaife-Heald 2012","3C380", 0.03, FluxStandard::SCAIFE_HEALD_2012, 265.81625769557746),
  make_tuple("Scaife-Heald 2012","3C380", 0.3, FluxStandard::SCAIFE_HEALD_2012, 45.454985119727866)
};

INSTANTIATE_TEST_CASE_P(checkStandardMatch, MatchStandardTest,::testing::ValuesIn(flxStdParams));
TEST_P(MatchStandardTest, checkStandardMatch)
{
  matchedStandard  = FluxStandard::matchStandard(flxStdName, flxStdEnum, flxStdDesc); 
  EXPECT_TRUE(matchedStandard);
  EXPECT_EQ(flxStdEnum, expFlxStdEnum);
}

INSTANTIATE_TEST_CASE_P(checkAltSrcNames,AltSrcNameTest,::testing::ValuesIn(flxStdParamsAltSrcNames));
TEST_P(AltSrcNameTest, checkAltSrcNames)
{
  if (modelExists(coeffsTbName)) {
    fluxStd.reset(new FluxStandard(expFlxStdEnum));
    mfreq = MFrequency(Quantity(freq,"GHz"));
    fluxStd->setInterpMethod("spline");
    foundStd = fluxStd->compute(srcName, srcDir, mfreq, mtime, returnFlux, returnFluxErr);
    EXPECT_TRUE(foundStd);
  }
  else {
    cout <<" The model parameter table, "<<foundModelPath<<" does not seem to exist. Skip this test"<<endl;
  }
}

INSTANTIATE_TEST_CASE_P(checkInterpolation, SetInterpMethodTest,::testing::ValuesIn(flxStdParamsVarSrc));
TEST_P(SetInterpMethodTest, checkInterpolation)
{
  // calc values were checked against calculation against a python script to be consistent at the tolerance level of  
  Double tol = 0.00001;
  srcName="3C48";
  if (modelExists(coeffsTbName)) {
    fluxStd.reset(new FluxStandard(FluxStandard::PERLEY_BUTLER_2013));
    mfreq = MFrequency(Quantity(2.0,"GHz")); 
    cerr<<"set interpolation to "<<interpMethod<<endl;
    fluxStd->setInterpMethod(interpMethod);
    foundStd = fluxStd->compute(srcName, srcDir, mfreq, mtime, returnFlux, returnFluxErr);
    returnFlux.value(fluxUsed);
    EXPECT_TRUE(foundStd);
    //EXPECT_DOUBLE_EQ(expFlxVal,fluxUsed[0]);
    EXPECT_NEAR(expFlxVal,fluxUsed[0],tol);
    //cerr<<setprecision(dbl::max_digits10)<<" fluxUsed[0]="<<fluxUsed[0]<<" for srcName="<<srcName<<" freq="<<mfreq<<endl; 
  }
  else {
    cout <<" The model parameter table, "<<foundModelPath<<" does not seem to exist. Skip this test"<<endl;
  }
}


INSTANTIATE_TEST_CASE_P(ckeckFluxValues,FluxValueTest,::testing::ValuesIn(flxStdParamsFull));
TEST_P(FluxValueTest, checkFluxValues)
{
  // calc values were checked against calculation against a python script to be consistent at the tolerance level of  
  Double tol = 0.00001;
  fluxStd.reset(new FluxStandard(expFlxStdEnum));
  if (expFlxStdEnum == FluxStandard::PERLEY_BUTLER_2013) {
    coeffsTbName = "PerleyButler2013Coeffs";
  }
  else if (expFlxStdEnum == FluxStandard::SCAIFE_HEALD_2012) {
    coeffsTbName = "ScaifeHeald2012Coeffs";
  }
  if (modelExists(coeffsTbName)) {
    mfreq = MFrequency(Quantity(freq,"GHz")); 
    fluxStd->setInterpMethod("spline");
    foundStd = fluxStd->compute(srcName, srcDir, mfreq, mtime, returnFlux, returnFluxErr);
    returnFlux.value(fluxUsed);
    EXPECT_TRUE(foundStd);
    //EXPECT_DOUBLE_EQ(expFlxVal,fluxUsed[0]);
    EXPECT_NEAR(expFlxVal,fluxUsed[0],tol);
    //cerr<<setprecision(dbl::max_digits10)<<flxStdName<<" fluxUsed[0]="<<fluxUsed[0]<<" for srcName="<<srcName<<" freq="<<freq<<endl; 
  }
  else {
    cout <<" The model parameter table,"<<foundModelPath<<" does not seem to exist. Skip this test"<<endl;
  }
}

}//end namespace test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
