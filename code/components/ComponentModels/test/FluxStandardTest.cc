//# FluxStandardTest.cc: implementation of FluxStandard google test base class  
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
#include <components/ComponentModels/test/FluxStandardTest.h>
#include <measures/Measures/MFrequency.h>
#include <limits>
typedef std::numeric_limits< double > dbl;

using namespace casacore;
using namespace casa;
using namespace std; 

namespace test {


FluxStandardTest::FluxStandardTest() {};
FluxStandardTest::~FluxStandardTest() {};

void FluxStandardTest::SetUp() {};

void FluxStandardTest::TearDown() {};
// things to test
// // compute (2 signatures)
// // computeCL
// // setInterpMethod
// // makeComponentList (2 signatures)
// // matchStandard
// // standardName
// //
// };
//
//
}
