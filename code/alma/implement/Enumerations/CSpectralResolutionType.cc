
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
 * File CSpectralResolutionType.cpp
 */
#include <sstream>
#include <CSpectralResolutionType.h>
#include <string>
using namespace std;

	
const std::string& CSpectralResolutionType::sCHANNEL_AVERAGE = "CHANNEL_AVERAGE";
	
const std::string& CSpectralResolutionType::sBASEBAND_WIDE = "BASEBAND_WIDE";
	
const std::string& CSpectralResolutionType::sFULL_RESOLUTION = "FULL_RESOLUTION";
	
const std::vector<std::string> CSpectralResolutionType::sSpectralResolutionTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSpectralResolutionType::sCHANNEL_AVERAGE);
    
    enumSet.insert(enumSet.end(), CSpectralResolutionType::sBASEBAND_WIDE);
    
    enumSet.insert(enumSet.end(), CSpectralResolutionType::sFULL_RESOLUTION);
        
    return enumSet;
}

	

	
	
const std::string& CSpectralResolutionType::hCHANNEL_AVERAGE = "";
	
const std::string& CSpectralResolutionType::hBASEBAND_WIDE = "";
	
const std::string& CSpectralResolutionType::hFULL_RESOLUTION = "";
	
const std::vector<std::string> CSpectralResolutionType::hSpectralResolutionTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSpectralResolutionType::hCHANNEL_AVERAGE);
    
    enumSet.insert(enumSet.end(), CSpectralResolutionType::hBASEBAND_WIDE);
    
    enumSet.insert(enumSet.end(), CSpectralResolutionType::hFULL_RESOLUTION);
        
    return enumSet;
}
   	

std::string CSpectralResolutionType::name(const SpectralResolutionTypeMod::SpectralResolutionType& f) {
    switch (f) {
    
    case SpectralResolutionTypeMod::CHANNEL_AVERAGE:
      return CSpectralResolutionType::sCHANNEL_AVERAGE;
    
    case SpectralResolutionTypeMod::BASEBAND_WIDE:
      return CSpectralResolutionType::sBASEBAND_WIDE;
    
    case SpectralResolutionTypeMod::FULL_RESOLUTION:
      return CSpectralResolutionType::sFULL_RESOLUTION;
    	
    }
    return std::string("");
}

	

	
std::string CSpectralResolutionType::help(const SpectralResolutionTypeMod::SpectralResolutionType& f) {
    switch (f) {
    
    case SpectralResolutionTypeMod::CHANNEL_AVERAGE:
      return CSpectralResolutionType::hCHANNEL_AVERAGE;
    
    case SpectralResolutionTypeMod::BASEBAND_WIDE:
      return CSpectralResolutionType::hBASEBAND_WIDE;
    
    case SpectralResolutionTypeMod::FULL_RESOLUTION:
      return CSpectralResolutionType::hFULL_RESOLUTION;
    	
    }
    return std::string("");
}
   	

SpectralResolutionTypeMod::SpectralResolutionType CSpectralResolutionType::newSpectralResolutionType(const std::string& name) {
		
    if (name == CSpectralResolutionType::sCHANNEL_AVERAGE) {
        return SpectralResolutionTypeMod::CHANNEL_AVERAGE;
    }
    	
    if (name == CSpectralResolutionType::sBASEBAND_WIDE) {
        return SpectralResolutionTypeMod::BASEBAND_WIDE;
    }
    	
    if (name == CSpectralResolutionType::sFULL_RESOLUTION) {
        return SpectralResolutionTypeMod::FULL_RESOLUTION;
    }
    
    throw badString(name);
}

SpectralResolutionTypeMod::SpectralResolutionType CSpectralResolutionType::literal(const std::string& name) {
		
    if (name == CSpectralResolutionType::sCHANNEL_AVERAGE) {
        return SpectralResolutionTypeMod::CHANNEL_AVERAGE;
    }
    	
    if (name == CSpectralResolutionType::sBASEBAND_WIDE) {
        return SpectralResolutionTypeMod::BASEBAND_WIDE;
    }
    	
    if (name == CSpectralResolutionType::sFULL_RESOLUTION) {
        return SpectralResolutionTypeMod::FULL_RESOLUTION;
    }
    
    throw badString(name);
}

SpectralResolutionTypeMod::SpectralResolutionType CSpectralResolutionType::from_int(unsigned int i) {
	vector<string> names = sSpectralResolutionTypeSet();
	if (i >= names.size()) throw badInt(i);
	return newSpectralResolutionType(names.at(i));
}

	

string CSpectralResolutionType::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'SpectralResolutionType'.";
}

string CSpectralResolutionType::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'SpectralResolutionType'.";
	return oss.str();
}

