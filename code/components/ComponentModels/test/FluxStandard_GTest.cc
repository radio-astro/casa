//# FluxStandard_GTest.cc: implementation of FluxStandard google test   
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
#include <components/ComponentModels/test/FluxStandard_GTest.h>
#include <measures/Measures/MFrequency.h>
#include <limits>
typedef std::numeric_limits< double > dbl;

using namespace casacore;
using namespace casa;
using namespace std; 

namespace test {

OldFluxStandardTest::OldFluxStandardTest()
  : fluxUsed(4), foundStd(false)
{
};
 
OldFluxStandardTest::~OldFluxStandardTest() {};

void OldFluxStandardTest::SetUp() 
{
  //mtime = new MEpoch(Quantity(56293.0,"d")); 
  //srcDir = new MDirection(MVDirection(Quantity(0.0,"rad"),Quantity(0.0,"rad")), MDirection::J2000);
  mtime = MEpoch(Quantity(56293.0,"d")); 
  srcDir = MDirection(MVDirection(Quantity(0.0,"rad"),Quantity(0.0,"rad")), MDirection::J2000);
};

void OldFluxStandardTest::TearDown()
{
  //delete mtime;
  //delete srcDir;
  //mtime = NULL;
  //srcDir = NULL;
}

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
  flxStdName = std::tr1::get<0>(GetParam());
  srcName = std::tr1::get<1>(GetParam());
  freq = std::tr1::get<2>(GetParam());
  expFlxStdEnum = std::tr1::get<3>(GetParam());
}

void AltSrcNameTest::TearDown() { };

FluxValueTest::FluxValueTest() {};
FluxValueTest::~FluxValueTest() {};
void FluxValueTest::SetUp() 
{
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
  make_tuple("Stevens-Reynolds 2016", FluxStandard::STEVENS_REYNOLDS_2016),
  make_tuple("Perley-Butler 2010", FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Baars", FluxStandard::BAARS),
  make_tuple("Perley-Taylor 99", FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 95", FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley 90", FluxStandard::PERLEY_90)
};

// for testing alternative source names
std::tr1::tuple<String,String,Double,FluxStandard::FluxScale> flxStdParamsAltSrcNames[] =
{
// 1. P-B 2010, 3. Baars, 4. P-T 99, 5. P-T 95, 6. Perly 90
//3C48    0134+329   0137+331   J0137+3309       1,3,4,5,6       
//3C138   0518+165   0521+166   J0521+1638       1,3,4,5,6           
//3C147   0538+498   0542+498   J0542+4951       1,3,4,5,6       
//3C196   0809+483   0813+482   J0813+4813       1                 
//3C286   1328+307   1331+305   J1331+3030       1,3,4,5,6         
//3C295   1409+524   1411+522   J1411+5212       1,3,4,5,6         
//  -     1934-638      -       J1939-6342       1,3,4,5,6,8 
  //3c48
  make_tuple("Perley-Butler 2010","3C48", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","0134+329", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","0137+331", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","J0137+3309", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Baars","3C48", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","0134+329", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","0137+331", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","J0137+3309", 2.0, FluxStandard::BAARS),
  make_tuple("Perley-Taylor 99","3C48", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","0134+329", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","0137+331", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","J0137+3309", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 95","3C48", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","0134+329", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","0137+331", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","J0137+3309", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley 90","3C48", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","0134+329", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","0137+331", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","J0137+3309", 2.0, FluxStandard::PERLEY_90),
  //3c138
  make_tuple("Perley-Butler 2010","3C138", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","0518+165", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","0521+166", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","J0521+1638", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Baars","3C138", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","0518+165", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","0521+166", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","J0521+1638", 2.0, FluxStandard::BAARS),
  make_tuple("Perley-Taylor 99","3C138", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","0518+165", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","0521+166", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","J0521+1638", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 95","3C138", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","0518+165", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","0521+166", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","J0521+1638", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley 90","3C138", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","0518+165", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","0521+166", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","J0521+1638", 2.0, FluxStandard::PERLEY_90),
  //3c147
  make_tuple("Perley-Butler 2010","3C147", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","0538+498", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","0542+498", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","J0542+4951", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Baars","3C147", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","0538+498", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","0542+498", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","J0542+4951", 2.0, FluxStandard::BAARS),
  make_tuple("Perley-Taylor 99","3C147", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","0538+498", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","0542+498", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","J0542+4951", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 95","3C147", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","0538+498", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","0542+498", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","J0542+4951", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley 90","3C147", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","0538+498", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","0542+498", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","J0542+4951", 2.0, FluxStandard::PERLEY_90),
  //3C196   0809+483   0813+482   J0813+4813       1
  make_tuple("Perley-Butler 2010","3C196", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","0809+483", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","0813+482", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","J0813+4813", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  //3C286   1328+307   1331+305   J1331+3030       1,3,4,5,6         
  make_tuple("Perley-Butler 2010","3C286", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","1328+307", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","1331+305", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","J1331+3030", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Baars","3C286", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","1328+307", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","1331+305", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","J1331+3030", 2.0, FluxStandard::BAARS),
  make_tuple("Perley-Taylor 99","3C286", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","1328+307", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","1331+305", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","J1331+3030", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 95","3C286", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","1328+307", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","1331+305", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","J1331+3030", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley 90","3C286", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","1328+307", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","1331+305", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","J1331+3030", 2.0, FluxStandard::PERLEY_90),
  //
  //3C295   1409+524   1411+522   J1411+5212       1,3,4,5,6         
  make_tuple("Perley-Butler 2010","3C295", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","1409+524", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","1411+522", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","J1411+5212", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Baars","3C295", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","1409+524", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","1411+522", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","J1411+5212", 2.0, FluxStandard::BAARS),
  make_tuple("Perley-Taylor 99","3C295", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","1409+524", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","1411+522", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","J1411+5212", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 95","3C295", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","1409+524", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","1411+522", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","J1411+5212", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley 90","3C295", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","1409+524", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","1411+522", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","J1411+5212", 2.0, FluxStandard::PERLEY_90),
  //1934-638   J1939-6342       1,3,4,5,6 
  make_tuple("Stevens-Reynolds 2016","1934-638", 2.0, FluxStandard::STEVENS_REYNOLDS_2016),
  make_tuple("Perley-Butler 2010","1934-638", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Perley-Butler 2010","J1939-6342", 2.0, FluxStandard::PERLEY_BUTLER_2010),
  make_tuple("Baars","1934-638", 2.0, FluxStandard::BAARS),
  make_tuple("Baars","J1939-6342", 2.0, FluxStandard::BAARS),
  make_tuple("Perley-Taylor 99","1934-638", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 99","J1939-6342", 2.0, FluxStandard::PERLEY_TAYLOR_99),
  make_tuple("Perley-Taylor 95","1934-638", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley-Taylor 95","J1939-6342", 2.0, FluxStandard::PERLEY_TAYLOR_95),
  make_tuple("Perley 90","1934-638", 2.0, FluxStandard::PERLEY_90),
  make_tuple("Perley 90","J1939-6342", 2.0, FluxStandard::PERLEY_90)
};

std::tr1::tuple<String,String,Double,FluxStandard::FluxScale,Double> flxStdParamsFull[] =
{
// 1. P-B 2010, 3. Baars, 4. P-T 99, 5. P-T 95, 6. Perly 90, 8. Stevens-Reynolds 2016 
/***
3C48    0134+329   0137+331   J0137+3309       1,2,3,4,5,6         
3C138   0518+165   0521+166   J0521+1638       1,2,3,4,5,6           
3C147   0538+498   0542+498   J0542+4951       1,2,3,4,5,6         
3C286   1328+307   1331+305   J1331+3030       1,2,3,4,5,6         
3C295   1409+524   1411+522   J1411+5212       1,2,3,4,5,6         
  -     1934-638      -       J1939-6342       1,3,4,5,6,8 
***/
  make_tuple("Stevens-Reynolds 2016","1934-638", 2.0, FluxStandard::STEVENS_REYNOLDS_2016, 12.950267126896309),
  make_tuple("Stevens-Reynolds 2016","1934-638", 20.0, FluxStandard::STEVENS_REYNOLDS_2016, 0.92788142182644617),
  make_tuple("Perley-Butler 2010","3C48", 2.0, FluxStandard::PERLEY_BUTLER_2010, 12.147971835306278),
  make_tuple("Perley-Butler 2010","3C48", 20.0, FluxStandard::PERLEY_BUTLER_2010, 1.4208694621518749),
  make_tuple("Perley-Butler 2010","3C138", 2.0, FluxStandard::PERLEY_BUTLER_2010, 7.2096240634653155),
  make_tuple("Perley-Butler 2010","3C138", 20.0, FluxStandard::PERLEY_BUTLER_2010, 1.7118680993464588),
  make_tuple("Perley-Butler 2010","3C147", 2.0, FluxStandard::PERLEY_BUTLER_2010, 16.884719037509679),
  make_tuple("Perley-Butler 2010","3C147", 20.0, FluxStandard::PERLEY_BUTLER_2010, 2.0824813770436577),
  make_tuple("Perley-Butler 2010","3C196", 2.0, FluxStandard::PERLEY_BUTLER_2010, 10.374114100448065),
  make_tuple("Perley-Butler 2010","3C196", 20.0, FluxStandard::PERLEY_BUTLER_2010, 0.87143393935843783),
  make_tuple("Perley-Butler 2010","3C286", 2.0, FluxStandard::PERLEY_BUTLER_2010,12.467388477230747),
  make_tuple("Perley-Butler 2010","3C286", 20.0, FluxStandard::PERLEY_BUTLER_2010,2.807986176998104),
  make_tuple("Perley-Butler 2010","3C295", 2.0, FluxStandard::PERLEY_BUTLER_2010,16.402280452401566),
  make_tuple("Perley-Butler 2010","3C295", 20.0, FluxStandard::PERLEY_BUTLER_2010,1.1347195694663186),
  make_tuple("Perley-Butler 2010","J1939-6342", 2.0, FluxStandard::PERLEY_BUTLER_2010,12.950267126896309),
  make_tuple("Perley-Butler 2010","J1939-6342", 20.0, FluxStandard::PERLEY_BUTLER_2010,0.9316588798090486),
  make_tuple("Baars","3C48", 2.0, FluxStandard::BAARS, 11.901862734549713),
  make_tuple("Baars","3C48", 20.0, FluxStandard::BAARS, 1.2517877204986338),
  make_tuple("Baars","3C138", 2.0, FluxStandard::BAARS, 6.8047800215651995),
  make_tuple("Baars","3C138", 20.0, FluxStandard::BAARS, 1.2757234214326791),
  make_tuple("Baars","3C147", 2.0, FluxStandard::BAARS, 17.24048203942839),
  make_tuple("Baars","3C147", 20.0, FluxStandard::BAARS, 1.9265059745354762),
  make_tuple("Baars","3C286", 2.0, FluxStandard::BAARS, 12.379346119135585),
  make_tuple("Baars","3C286", 20.0, FluxStandard::BAARS, 2.7672110161123444),
  make_tuple("Baars","3C295", 2.0, FluxStandard::BAARS, 16.285387274618959),
  make_tuple("Baars","3C295", 20.0, FluxStandard::BAARS, 1.0771416832732676),
  make_tuple("Baars","J1939-6342", 2.0, FluxStandard::BAARS, 14.507709268835908),
  make_tuple("Baars","J1939-6342", 20.0, FluxStandard::BAARS, 0.54344717204516524),
  make_tuple("Perley-Taylor 99","3C48", 2.0, FluxStandard::PERLEY_TAYLOR_99, 12.016065248498386),
  make_tuple("Perley-Taylor 99","3C48", 20.0, FluxStandard::PERLEY_TAYLOR_99, 1.2716220764539476),
  make_tuple("Perley-Taylor 99","3C138", 2.0, FluxStandard::PERLEY_TAYLOR_99, 6.755573067574753),
  make_tuple("Perley-Taylor 99","3C138", 20.0, FluxStandard::PERLEY_TAYLOR_99, 1.1567636529166294),
  make_tuple("Perley-Taylor 99","3C147", 2.0, FluxStandard::PERLEY_TAYLOR_99, 16.907566726317707),
  make_tuple("Perley-Taylor 99","3C147", 20.0, FluxStandard::PERLEY_TAYLOR_99, 2.0221556697518288),
  make_tuple("Perley-Taylor 99","3C286", 2.0, FluxStandard::PERLEY_TAYLOR_99, 12.424290466215524),
  make_tuple("Perley-Taylor 99","3C286", 20.0, FluxStandard::PERLEY_TAYLOR_99, 2.7615579712777598),
  make_tuple("Perley-Taylor 99","3C295", 2.0, FluxStandard::PERLEY_TAYLOR_99, 16.267365667368523),
  make_tuple("Perley-Taylor 99","3C295", 20.0, FluxStandard::PERLEY_TAYLOR_99, 1.0940748669447304),
  make_tuple("Perley-Taylor 99","J1939-6342", 2.0, FluxStandard::PERLEY_TAYLOR_99, 12.950267126896309),
  make_tuple("Perley-Taylor 99","J1939-6342", 20.0, FluxStandard::PERLEY_TAYLOR_99, 0.9316588798090486),
  make_tuple("Perley-Taylor 95","3C48", 2.0, FluxStandard::PERLEY_TAYLOR_95, 12.133615321238402),
  make_tuple("Perley-Taylor 95","3C48", 20.0, FluxStandard::PERLEY_TAYLOR_95, 1.3294254585519367),
  make_tuple("Perley-Taylor 95","3C138", 2.0, FluxStandard::PERLEY_TAYLOR_95, 6.7773748524664432),
  make_tuple("Perley-Taylor 95","3C138", 20.0, FluxStandard::PERLEY_TAYLOR_95, 1.2237802305297349),
  make_tuple("Perley-Taylor 95","3C147", 2.0, FluxStandard::PERLEY_TAYLOR_95, 16.862979591567914),
  make_tuple("Perley-Taylor 95","3C147", 20.0, FluxStandard::PERLEY_TAYLOR_95, 2.1317289572360321),
  make_tuple("Perley-Taylor 95","3C286", 2.0, FluxStandard::PERLEY_TAYLOR_95, 12.475252761482698),
  make_tuple("Perley-Taylor 95","3C286", 20.0, FluxStandard::PERLEY_TAYLOR_95, 2.7378442967334431),
  make_tuple("Perley-Taylor 95","3C295", 2.0, FluxStandard::PERLEY_TAYLOR_95, 16.282388892493458),
  make_tuple("Perley-Taylor 95","3C295", 20.0, FluxStandard::PERLEY_TAYLOR_95, 1.0870668356765618),
  make_tuple("Perley-Taylor 95","J1939-6342", 2.0, FluxStandard::PERLEY_TAYLOR_95, 12.950272533179664),
  make_tuple("Perley-Taylor 95","J1939-6342", 20.0, FluxStandard::PERLEY_TAYLOR_95, 1.0828214962498031),
  make_tuple("Perley 90","3C48", 2.0, FluxStandard::PERLEY_90, 12.233939206358027),
  make_tuple("Perley 90","3C48", 20.0, FluxStandard::PERLEY_90, 1.3130735022715823),
  make_tuple("Perley 90","3C138", 2.0, FluxStandard::PERLEY_90, 6.8047800215651995),
  make_tuple("Perley 90","3C138", 20.0, FluxStandard::PERLEY_90, 1.2757234214326791),
  make_tuple("Perley 90","3C147", 2.0, FluxStandard::PERLEY_90, 16.684500263849291),
  make_tuple("Perley 90","3C147", 20.0, FluxStandard::PERLEY_90, 1.8243620970271477),
  make_tuple("Perley 90","3C286", 2.0, FluxStandard::PERLEY_90, 12.405538830933223),
  make_tuple("Perley 90","3C286", 20.0, FluxStandard::PERLEY_90, 2.7513947248923971),
  make_tuple("Perley 90","3C295", 2.0, FluxStandard::PERLEY_90, 16.285387274618959),
  make_tuple("Perley 90","3C295", 20.0, FluxStandard::PERLEY_90, 1.0771416832732676),
  make_tuple("Perley 90","J1939-6342", 2.0, FluxStandard::PERLEY_90, 12.950272533179664),
  make_tuple("Perley 90","J1939-6342", 20.0, FluxStandard::PERLEY_90, 1.0828214962498031)
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
  fluxStd.reset(new FluxStandard(expFlxStdEnum));
  mfreq = MFrequency(Quantity(freq,"GHz"));
  foundStd = fluxStd->compute(srcName, srcDir, mfreq, mtime, returnFlux, returnFluxErr);
  EXPECT_TRUE(foundStd);
}
INSTANTIATE_TEST_CASE_P(ckeckFluxValues,FluxValueTest,::testing::ValuesIn(flxStdParamsFull));
TEST_P(FluxValueTest, checkFluxValues)
{
  // calc values were checked against calculation against a python script to be consistent at the tolerance level of  
  Double tol = 0.00001;
  fluxStd.reset(new FluxStandard(expFlxStdEnum));
  mfreq = MFrequency(Quantity(freq,"GHz")); 
  foundStd = fluxStd->compute(srcName, srcDir, mfreq, mtime, returnFlux, returnFluxErr);
  returnFlux.value(fluxUsed);
  EXPECT_TRUE(foundStd);
  //EXPECT_DOUBLE_EQ(expFlxVal,fluxUsed[0]);
  EXPECT_NEAR(expFlxVal,fluxUsed[0],tol);
  //cerr<<setprecision(dbl::max_digits10)<<" fluxUsed[0]="<<fluxUsed[0]<<" for srcName="<<srcName<<" freq="<<freq<<endl; 
}

}//end namespace test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
