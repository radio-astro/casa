
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
 * File CSwitchingMode.cpp
 */
#include <sstream>
#include <CSwitchingMode.h>
#include <string>
using namespace std;

	
const std::string& CSwitchingMode::sNO_SWITCHING = "NO_SWITCHING";
	
const std::string& CSwitchingMode::sLOAD_SWITCHING = "LOAD_SWITCHING";
	
const std::string& CSwitchingMode::sPOSITION_SWITCHING = "POSITION_SWITCHING";
	
const std::string& CSwitchingMode::sPHASE_SWITCHING = "PHASE_SWITCHING";
	
const std::string& CSwitchingMode::sFREQUENCY_SWITCHING = "FREQUENCY_SWITCHING";
	
const std::string& CSwitchingMode::sNUTATOR_SWITCHING = "NUTATOR_SWITCHING";
	
const std::string& CSwitchingMode::sCHOPPER_WHEEL = "CHOPPER_WHEEL";
	
const std::vector<std::string> CSwitchingMode::sSwitchingModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSwitchingMode::sNO_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::sLOAD_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::sPOSITION_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::sPHASE_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::sFREQUENCY_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::sNUTATOR_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::sCHOPPER_WHEEL);
        
    return enumSet;
}

	

	
	
const std::string& CSwitchingMode::hNO_SWITCHING = "No switching";
	
const std::string& CSwitchingMode::hLOAD_SWITCHING = "Receiver beam is switched between sky and load";
	
const std::string& CSwitchingMode::hPOSITION_SWITCHING = "Antenna (main reflector) pointing direction  is switched ";
	
const std::string& CSwitchingMode::hPHASE_SWITCHING = "90 degrees phase switching  (switching mode used for sideband separation or rejection with DSB receivers)";
	
const std::string& CSwitchingMode::hFREQUENCY_SWITCHING = "LO frequency is switched (definition context sensitive: fast if cycle shrorter than the integration duration, slow if e.g. step one step per subscan)";
	
const std::string& CSwitchingMode::hNUTATOR_SWITCHING = "Switching between different directions by nutating the sub-reflector";
	
const std::string& CSwitchingMode::hCHOPPER_WHEEL = "Switching using a chopper wheel";
	
const std::vector<std::string> CSwitchingMode::hSwitchingModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSwitchingMode::hNO_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::hLOAD_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::hPOSITION_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::hPHASE_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::hFREQUENCY_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::hNUTATOR_SWITCHING);
    
    enumSet.insert(enumSet.end(), CSwitchingMode::hCHOPPER_WHEEL);
        
    return enumSet;
}
   	

std::string CSwitchingMode::name(const SwitchingModeMod::SwitchingMode& f) {
    switch (f) {
    
    case SwitchingModeMod::NO_SWITCHING:
      return CSwitchingMode::sNO_SWITCHING;
    
    case SwitchingModeMod::LOAD_SWITCHING:
      return CSwitchingMode::sLOAD_SWITCHING;
    
    case SwitchingModeMod::POSITION_SWITCHING:
      return CSwitchingMode::sPOSITION_SWITCHING;
    
    case SwitchingModeMod::PHASE_SWITCHING:
      return CSwitchingMode::sPHASE_SWITCHING;
    
    case SwitchingModeMod::FREQUENCY_SWITCHING:
      return CSwitchingMode::sFREQUENCY_SWITCHING;
    
    case SwitchingModeMod::NUTATOR_SWITCHING:
      return CSwitchingMode::sNUTATOR_SWITCHING;
    
    case SwitchingModeMod::CHOPPER_WHEEL:
      return CSwitchingMode::sCHOPPER_WHEEL;
    	
    }
    return std::string("");
}

	

	
std::string CSwitchingMode::help(const SwitchingModeMod::SwitchingMode& f) {
    switch (f) {
    
    case SwitchingModeMod::NO_SWITCHING:
      return CSwitchingMode::hNO_SWITCHING;
    
    case SwitchingModeMod::LOAD_SWITCHING:
      return CSwitchingMode::hLOAD_SWITCHING;
    
    case SwitchingModeMod::POSITION_SWITCHING:
      return CSwitchingMode::hPOSITION_SWITCHING;
    
    case SwitchingModeMod::PHASE_SWITCHING:
      return CSwitchingMode::hPHASE_SWITCHING;
    
    case SwitchingModeMod::FREQUENCY_SWITCHING:
      return CSwitchingMode::hFREQUENCY_SWITCHING;
    
    case SwitchingModeMod::NUTATOR_SWITCHING:
      return CSwitchingMode::hNUTATOR_SWITCHING;
    
    case SwitchingModeMod::CHOPPER_WHEEL:
      return CSwitchingMode::hCHOPPER_WHEEL;
    	
    }
    return std::string("");
}
   	

SwitchingModeMod::SwitchingMode CSwitchingMode::newSwitchingMode(const std::string& name) {
		
    if (name == CSwitchingMode::sNO_SWITCHING) {
        return SwitchingModeMod::NO_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sLOAD_SWITCHING) {
        return SwitchingModeMod::LOAD_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sPOSITION_SWITCHING) {
        return SwitchingModeMod::POSITION_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sPHASE_SWITCHING) {
        return SwitchingModeMod::PHASE_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sFREQUENCY_SWITCHING) {
        return SwitchingModeMod::FREQUENCY_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sNUTATOR_SWITCHING) {
        return SwitchingModeMod::NUTATOR_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sCHOPPER_WHEEL) {
        return SwitchingModeMod::CHOPPER_WHEEL;
    }
    
    throw badString(name);
}

SwitchingModeMod::SwitchingMode CSwitchingMode::literal(const std::string& name) {
		
    if (name == CSwitchingMode::sNO_SWITCHING) {
        return SwitchingModeMod::NO_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sLOAD_SWITCHING) {
        return SwitchingModeMod::LOAD_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sPOSITION_SWITCHING) {
        return SwitchingModeMod::POSITION_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sPHASE_SWITCHING) {
        return SwitchingModeMod::PHASE_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sFREQUENCY_SWITCHING) {
        return SwitchingModeMod::FREQUENCY_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sNUTATOR_SWITCHING) {
        return SwitchingModeMod::NUTATOR_SWITCHING;
    }
    	
    if (name == CSwitchingMode::sCHOPPER_WHEEL) {
        return SwitchingModeMod::CHOPPER_WHEEL;
    }
    
    throw badString(name);
}

SwitchingModeMod::SwitchingMode CSwitchingMode::from_int(unsigned int i) {
	vector<string> names = sSwitchingModeSet();
	if (i >= names.size()) throw badInt(i);
	return newSwitchingMode(names.at(i));
}

	

string CSwitchingMode::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'SwitchingMode'.";
}

string CSwitchingMode::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'SwitchingMode'.";
	return oss.str();
}

