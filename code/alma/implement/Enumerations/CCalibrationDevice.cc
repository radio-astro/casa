
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 * 
 * /////////////////////////////////////////////////////////////////
 * // WARNING!  DO NOT MODIFY THIS FILE!                          //
 * //  ---------------------------------------------------------  //
 * // | This is generated code!  Do not modify this file.       | //
 * // | Any changes will be lost when the file is re-generated. | //
 * //  ---------------------------------------------------------  //
 * /////////////////////////////////////////////////////////////////
 *
 * File CCalibrationDevice.cpp
 */
#include <sstream>
#include <CCalibrationDevice.h>
#include <string>
using namespace std;

	
const std::string& CCalibrationDevice::sAMBIENT_LOAD = "AMBIENT_LOAD";
	
const std::string& CCalibrationDevice::sCOLD_LOAD = "COLD_LOAD";
	
const std::string& CCalibrationDevice::sHOT_LOAD = "HOT_LOAD";
	
const std::string& CCalibrationDevice::sNOISE_TUBE_LOAD = "NOISE_TUBE_LOAD";
	
const std::string& CCalibrationDevice::sQUARTER_WAVE_PLATE = "QUARTER_WAVE_PLATE";
	
const std::string& CCalibrationDevice::sSOLAR_FILTER = "SOLAR_FILTER";
	
const std::string& CCalibrationDevice::sNONE = "NONE";
	
const std::vector<std::string> CCalibrationDevice::sCalibrationDeviceSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::sAMBIENT_LOAD);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::sCOLD_LOAD);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::sHOT_LOAD);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::sNOISE_TUBE_LOAD);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::sQUARTER_WAVE_PLATE);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::sSOLAR_FILTER);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::sNONE);
        
    return enumSet;
}

	

	
	
const std::string& CCalibrationDevice::hAMBIENT_LOAD = "An absorbing load at the ambient temperature.";
	
const std::string& CCalibrationDevice::hCOLD_LOAD = "A cooled absorbing load.";
	
const std::string& CCalibrationDevice::hHOT_LOAD = "A heated absorbing load.";
	
const std::string& CCalibrationDevice::hNOISE_TUBE_LOAD = "A noise tube.";
	
const std::string& CCalibrationDevice::hQUARTER_WAVE_PLATE = "A transparent plate that introduces a 90-degree phase difference between othogonal polarizations.";
	
const std::string& CCalibrationDevice::hSOLAR_FILTER = "An optical attenuator (to protect receiver from solar heat).";
	
const std::string& CCalibrationDevice::hNONE = "No device, the receiver looks at the sky (through the telescope).";
	
const std::vector<std::string> CCalibrationDevice::hCalibrationDeviceSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::hAMBIENT_LOAD);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::hCOLD_LOAD);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::hHOT_LOAD);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::hNOISE_TUBE_LOAD);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::hQUARTER_WAVE_PLATE);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::hSOLAR_FILTER);
    
    enumSet.insert(enumSet.end(), CCalibrationDevice::hNONE);
        
    return enumSet;
}
   	

std::string CCalibrationDevice::name(const CalibrationDeviceMod::CalibrationDevice& f) {
    switch (f) {
    
    case CalibrationDeviceMod::AMBIENT_LOAD:
      return CCalibrationDevice::sAMBIENT_LOAD;
    
    case CalibrationDeviceMod::COLD_LOAD:
      return CCalibrationDevice::sCOLD_LOAD;
    
    case CalibrationDeviceMod::HOT_LOAD:
      return CCalibrationDevice::sHOT_LOAD;
    
    case CalibrationDeviceMod::NOISE_TUBE_LOAD:
      return CCalibrationDevice::sNOISE_TUBE_LOAD;
    
    case CalibrationDeviceMod::QUARTER_WAVE_PLATE:
      return CCalibrationDevice::sQUARTER_WAVE_PLATE;
    
    case CalibrationDeviceMod::SOLAR_FILTER:
      return CCalibrationDevice::sSOLAR_FILTER;
    
    case CalibrationDeviceMod::NONE:
      return CCalibrationDevice::sNONE;
    	
    }
    return std::string("");
}

	

	
std::string CCalibrationDevice::help(const CalibrationDeviceMod::CalibrationDevice& f) {
    switch (f) {
    
    case CalibrationDeviceMod::AMBIENT_LOAD:
      return CCalibrationDevice::hAMBIENT_LOAD;
    
    case CalibrationDeviceMod::COLD_LOAD:
      return CCalibrationDevice::hCOLD_LOAD;
    
    case CalibrationDeviceMod::HOT_LOAD:
      return CCalibrationDevice::hHOT_LOAD;
    
    case CalibrationDeviceMod::NOISE_TUBE_LOAD:
      return CCalibrationDevice::hNOISE_TUBE_LOAD;
    
    case CalibrationDeviceMod::QUARTER_WAVE_PLATE:
      return CCalibrationDevice::hQUARTER_WAVE_PLATE;
    
    case CalibrationDeviceMod::SOLAR_FILTER:
      return CCalibrationDevice::hSOLAR_FILTER;
    
    case CalibrationDeviceMod::NONE:
      return CCalibrationDevice::hNONE;
    	
    }
    return std::string("");
}
   	

CalibrationDeviceMod::CalibrationDevice CCalibrationDevice::newCalibrationDevice(const std::string& name) {
		
    if (name == CCalibrationDevice::sAMBIENT_LOAD) {
        return CalibrationDeviceMod::AMBIENT_LOAD;
    }
    	
    if (name == CCalibrationDevice::sCOLD_LOAD) {
        return CalibrationDeviceMod::COLD_LOAD;
    }
    	
    if (name == CCalibrationDevice::sHOT_LOAD) {
        return CalibrationDeviceMod::HOT_LOAD;
    }
    	
    if (name == CCalibrationDevice::sNOISE_TUBE_LOAD) {
        return CalibrationDeviceMod::NOISE_TUBE_LOAD;
    }
    	
    if (name == CCalibrationDevice::sQUARTER_WAVE_PLATE) {
        return CalibrationDeviceMod::QUARTER_WAVE_PLATE;
    }
    	
    if (name == CCalibrationDevice::sSOLAR_FILTER) {
        return CalibrationDeviceMod::SOLAR_FILTER;
    }
    	
    if (name == CCalibrationDevice::sNONE) {
        return CalibrationDeviceMod::NONE;
    }
    
    throw badString(name);
}

CalibrationDeviceMod::CalibrationDevice CCalibrationDevice::literal(const std::string& name) {
		
    if (name == CCalibrationDevice::sAMBIENT_LOAD) {
        return CalibrationDeviceMod::AMBIENT_LOAD;
    }
    	
    if (name == CCalibrationDevice::sCOLD_LOAD) {
        return CalibrationDeviceMod::COLD_LOAD;
    }
    	
    if (name == CCalibrationDevice::sHOT_LOAD) {
        return CalibrationDeviceMod::HOT_LOAD;
    }
    	
    if (name == CCalibrationDevice::sNOISE_TUBE_LOAD) {
        return CalibrationDeviceMod::NOISE_TUBE_LOAD;
    }
    	
    if (name == CCalibrationDevice::sQUARTER_WAVE_PLATE) {
        return CalibrationDeviceMod::QUARTER_WAVE_PLATE;
    }
    	
    if (name == CCalibrationDevice::sSOLAR_FILTER) {
        return CalibrationDeviceMod::SOLAR_FILTER;
    }
    	
    if (name == CCalibrationDevice::sNONE) {
        return CalibrationDeviceMod::NONE;
    }
    
    throw badString(name);
}

CalibrationDeviceMod::CalibrationDevice CCalibrationDevice::from_int(unsigned int i) {
	vector<string> names = sCalibrationDeviceSet();
	if (i >= names.size()) throw badInt(i);
	return newCalibrationDevice(names.at(i));
}

	

string CCalibrationDevice::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'CalibrationDevice'.";
}

string CCalibrationDevice::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'CalibrationDevice'.";
	return oss.str();
}

