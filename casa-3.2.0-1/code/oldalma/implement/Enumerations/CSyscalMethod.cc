
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
 * File CSyscalMethod.cpp
 */
#include <sstream>
#include <CSyscalMethod.h>
#include <string>
using namespace std;

	
const std::string& CSyscalMethod::sTEMPERATURE_SCALE = "TEMPERATURE_SCALE";
	
const std::string& CSyscalMethod::sSKYDIP = "SKYDIP";
	
const std::vector<std::string> CSyscalMethod::sSyscalMethodSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSyscalMethod::sTEMPERATURE_SCALE);
    
    enumSet.insert(enumSet.end(), CSyscalMethod::sSKYDIP);
        
    return enumSet;
}

	

	
	
const std::string& CSyscalMethod::hTEMPERATURE_SCALE = "Use single direction data to compute ta* scale";
	
const std::string& CSyscalMethod::hSKYDIP = "Use a skydip (observing the sky at various elevations) to get atmospheric opacity";
	
const std::vector<std::string> CSyscalMethod::hSyscalMethodSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSyscalMethod::hTEMPERATURE_SCALE);
    
    enumSet.insert(enumSet.end(), CSyscalMethod::hSKYDIP);
        
    return enumSet;
}
   	

std::string CSyscalMethod::name(const SyscalMethodMod::SyscalMethod& f) {
    switch (f) {
    
    case SyscalMethodMod::TEMPERATURE_SCALE:
      return CSyscalMethod::sTEMPERATURE_SCALE;
    
    case SyscalMethodMod::SKYDIP:
      return CSyscalMethod::sSKYDIP;
    	
    }
    return std::string("");
}

	

	
std::string CSyscalMethod::help(const SyscalMethodMod::SyscalMethod& f) {
    switch (f) {
    
    case SyscalMethodMod::TEMPERATURE_SCALE:
      return CSyscalMethod::hTEMPERATURE_SCALE;
    
    case SyscalMethodMod::SKYDIP:
      return CSyscalMethod::hSKYDIP;
    	
    }
    return std::string("");
}
   	

SyscalMethodMod::SyscalMethod CSyscalMethod::newSyscalMethod(const std::string& name) {
		
    if (name == CSyscalMethod::sTEMPERATURE_SCALE) {
        return SyscalMethodMod::TEMPERATURE_SCALE;
    }
    	
    if (name == CSyscalMethod::sSKYDIP) {
        return SyscalMethodMod::SKYDIP;
    }
    
    throw badString(name);
}

SyscalMethodMod::SyscalMethod CSyscalMethod::literal(const std::string& name) {
		
    if (name == CSyscalMethod::sTEMPERATURE_SCALE) {
        return SyscalMethodMod::TEMPERATURE_SCALE;
    }
    	
    if (name == CSyscalMethod::sSKYDIP) {
        return SyscalMethodMod::SKYDIP;
    }
    
    throw badString(name);
}

SyscalMethodMod::SyscalMethod CSyscalMethod::from_int(unsigned int i) {
	vector<string> names = sSyscalMethodSet();
	if (i >= names.size()) throw badInt(i);
	return newSyscalMethod(names.at(i));
}

	

string CSyscalMethod::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'SyscalMethod'.";
}

string CSyscalMethod::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'SyscalMethod'.";
	return oss.str();
}

