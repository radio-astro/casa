
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
 * File CSidebandProcessingMode.cpp
 */
#include <sstream>
#include <CSidebandProcessingMode.h>
#include <string>
using namespace std;

	
const std::string& CSidebandProcessingMode::sNONE = "NONE";
	
const std::string& CSidebandProcessingMode::sPHASE_SWITCH_SEPARATION = "PHASE_SWITCH_SEPARATION";
	
const std::string& CSidebandProcessingMode::sFREQUENCY_OFFSET_SEPARATION = "FREQUENCY_OFFSET_SEPARATION";
	
const std::string& CSidebandProcessingMode::sPHASE_SWITCH_REJECTION = "PHASE_SWITCH_REJECTION";
	
const std::string& CSidebandProcessingMode::sFREQUENCY_OFFSET_REJECTION = "FREQUENCY_OFFSET_REJECTION";
	
const std::vector<std::string> CSidebandProcessingMode::sSidebandProcessingModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::sNONE);
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::sPHASE_SWITCH_SEPARATION);
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::sFREQUENCY_OFFSET_SEPARATION);
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::sPHASE_SWITCH_REJECTION);
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::sFREQUENCY_OFFSET_REJECTION);
        
    return enumSet;
}

	

	
	
const std::string& CSidebandProcessingMode::hNONE = "No processing";
	
const std::string& CSidebandProcessingMode::hPHASE_SWITCH_SEPARATION = "Side band separation using 90-degree phase switching";
	
const std::string& CSidebandProcessingMode::hFREQUENCY_OFFSET_SEPARATION = "Side band separation using offsets of first ans second oscillators";
	
const std::string& CSidebandProcessingMode::hPHASE_SWITCH_REJECTION = "Side band rejection 90-degree phase switching";
	
const std::string& CSidebandProcessingMode::hFREQUENCY_OFFSET_REJECTION = "Side band rejection using offsets of first and second oscillators";
	
const std::vector<std::string> CSidebandProcessingMode::hSidebandProcessingModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::hNONE);
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::hPHASE_SWITCH_SEPARATION);
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::hFREQUENCY_OFFSET_SEPARATION);
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::hPHASE_SWITCH_REJECTION);
    
    enumSet.insert(enumSet.end(), CSidebandProcessingMode::hFREQUENCY_OFFSET_REJECTION);
        
    return enumSet;
}
   	

std::string CSidebandProcessingMode::name(const SidebandProcessingModeMod::SidebandProcessingMode& f) {
    switch (f) {
    
    case SidebandProcessingModeMod::NONE:
      return CSidebandProcessingMode::sNONE;
    
    case SidebandProcessingModeMod::PHASE_SWITCH_SEPARATION:
      return CSidebandProcessingMode::sPHASE_SWITCH_SEPARATION;
    
    case SidebandProcessingModeMod::FREQUENCY_OFFSET_SEPARATION:
      return CSidebandProcessingMode::sFREQUENCY_OFFSET_SEPARATION;
    
    case SidebandProcessingModeMod::PHASE_SWITCH_REJECTION:
      return CSidebandProcessingMode::sPHASE_SWITCH_REJECTION;
    
    case SidebandProcessingModeMod::FREQUENCY_OFFSET_REJECTION:
      return CSidebandProcessingMode::sFREQUENCY_OFFSET_REJECTION;
    	
    }
    return std::string("");
}

	

	
std::string CSidebandProcessingMode::help(const SidebandProcessingModeMod::SidebandProcessingMode& f) {
    switch (f) {
    
    case SidebandProcessingModeMod::NONE:
      return CSidebandProcessingMode::hNONE;
    
    case SidebandProcessingModeMod::PHASE_SWITCH_SEPARATION:
      return CSidebandProcessingMode::hPHASE_SWITCH_SEPARATION;
    
    case SidebandProcessingModeMod::FREQUENCY_OFFSET_SEPARATION:
      return CSidebandProcessingMode::hFREQUENCY_OFFSET_SEPARATION;
    
    case SidebandProcessingModeMod::PHASE_SWITCH_REJECTION:
      return CSidebandProcessingMode::hPHASE_SWITCH_REJECTION;
    
    case SidebandProcessingModeMod::FREQUENCY_OFFSET_REJECTION:
      return CSidebandProcessingMode::hFREQUENCY_OFFSET_REJECTION;
    	
    }
    return std::string("");
}
   	

SidebandProcessingModeMod::SidebandProcessingMode CSidebandProcessingMode::newSidebandProcessingMode(const std::string& name) {
		
    if (name == CSidebandProcessingMode::sNONE) {
        return SidebandProcessingModeMod::NONE;
    }
    	
    if (name == CSidebandProcessingMode::sPHASE_SWITCH_SEPARATION) {
        return SidebandProcessingModeMod::PHASE_SWITCH_SEPARATION;
    }
    	
    if (name == CSidebandProcessingMode::sFREQUENCY_OFFSET_SEPARATION) {
        return SidebandProcessingModeMod::FREQUENCY_OFFSET_SEPARATION;
    }
    	
    if (name == CSidebandProcessingMode::sPHASE_SWITCH_REJECTION) {
        return SidebandProcessingModeMod::PHASE_SWITCH_REJECTION;
    }
    	
    if (name == CSidebandProcessingMode::sFREQUENCY_OFFSET_REJECTION) {
        return SidebandProcessingModeMod::FREQUENCY_OFFSET_REJECTION;
    }
    
    throw badString(name);
}

SidebandProcessingModeMod::SidebandProcessingMode CSidebandProcessingMode::literal(const std::string& name) {
		
    if (name == CSidebandProcessingMode::sNONE) {
        return SidebandProcessingModeMod::NONE;
    }
    	
    if (name == CSidebandProcessingMode::sPHASE_SWITCH_SEPARATION) {
        return SidebandProcessingModeMod::PHASE_SWITCH_SEPARATION;
    }
    	
    if (name == CSidebandProcessingMode::sFREQUENCY_OFFSET_SEPARATION) {
        return SidebandProcessingModeMod::FREQUENCY_OFFSET_SEPARATION;
    }
    	
    if (name == CSidebandProcessingMode::sPHASE_SWITCH_REJECTION) {
        return SidebandProcessingModeMod::PHASE_SWITCH_REJECTION;
    }
    	
    if (name == CSidebandProcessingMode::sFREQUENCY_OFFSET_REJECTION) {
        return SidebandProcessingModeMod::FREQUENCY_OFFSET_REJECTION;
    }
    
    throw badString(name);
}

SidebandProcessingModeMod::SidebandProcessingMode CSidebandProcessingMode::from_int(unsigned int i) {
	vector<string> names = sSidebandProcessingModeSet();
	if (i >= names.size()) throw badInt(i);
	return newSidebandProcessingMode(names.at(i));
}

	

string CSidebandProcessingMode::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'SidebandProcessingMode'.";
}

string CSidebandProcessingMode::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'SidebandProcessingMode'.";
	return oss.str();
}

