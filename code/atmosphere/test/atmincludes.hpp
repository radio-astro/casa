// Copyright (2008) Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
// 
// This file is part of AATM
//
// AATM is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AATM is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AATM.  If not, see <http://www.gnu.org/licenses/>.
//
// Comments regarding this example welcome at: 
// Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
//

// These are all the includes required for functioning of
// ATM. Unfortunately, due to the design of ATM, this list is somewhat
// long and includes the use of "using namespace std;". 

#ifndef _AATM_TEST_AATMINCLUDES_HPP__
#define _AATM_TEST_AATMINCLUDES_HPP__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

// Unfortunately the using statement below is required because of poor
// desgin of the header files
using namespace std;

#include "ATMRefractiveIndexProfile.h"
#include "ATMPercent.h"
#include "ATMPressure.h"
#include "ATMNumberDensity.h"
#include "ATMMassDensity.h"
#include "ATMTemperature.h"
#include "ATMLength.h"
#include "ATMInverseLength.h"
#include "ATMOpacity.h"  
#include "ATMHumidity.h"
#include "ATMFrequency.h"
#include "ATMWaterVaporRadiometer.h"
#include "ATMWVRMeasurement.h"
#include "ATMProfile.h"
#include "ATMSpectralGrid.h"
#include "ATMRefractiveIndex.h"
#include "ATMSkyStatus.h"
#include "ATMAngle.h"


#endif
