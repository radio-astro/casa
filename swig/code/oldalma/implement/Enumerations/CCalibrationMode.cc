
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
 * File CCalibrationMode.cpp
 */
#include <sstream>
#include <CCalibrationMode.h>
#include <string>
using namespace std;

	
const std::string& CCalibrationMode::sHOLOGRAPHY = "HOLOGRAPHY";
	
const std::string& CCalibrationMode::sINTERFEROMETRY = "INTERFEROMETRY";
	
const std::string& CCalibrationMode::sOPTICAL = "OPTICAL";
	
const std::string& CCalibrationMode::sRADIOMETRY = "RADIOMETRY";
	
const std::string& CCalibrationMode::sWVR = "WVR";
	
const std::vector<std::string> CCalibrationMode::sCalibrationModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCalibrationMode::sHOLOGRAPHY);
    
    enumSet.insert(enumSet.end(), CCalibrationMode::sINTERFEROMETRY);
    
    enumSet.insert(enumSet.end(), CCalibrationMode::sOPTICAL);
    
    enumSet.insert(enumSet.end(), CCalibrationMode::sRADIOMETRY);
    
    enumSet.insert(enumSet.end(), CCalibrationMode::sWVR);
        
    return enumSet;
}

	

	
	
const std::string& CCalibrationMode::hHOLOGRAPHY = "Holography receiver";
	
const std::string& CCalibrationMode::hINTERFEROMETRY = "interferometry";
	
const std::string& CCalibrationMode::hOPTICAL = "Optical telescope";
	
const std::string& CCalibrationMode::hRADIOMETRY = "total power";
	
const std::string& CCalibrationMode::hWVR = "water vapour radiometry receiver";
	
const std::vector<std::string> CCalibrationMode::hCalibrationModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCalibrationMode::hHOLOGRAPHY);
    
    enumSet.insert(enumSet.end(), CCalibrationMode::hINTERFEROMETRY);
    
    enumSet.insert(enumSet.end(), CCalibrationMode::hOPTICAL);
    
    enumSet.insert(enumSet.end(), CCalibrationMode::hRADIOMETRY);
    
    enumSet.insert(enumSet.end(), CCalibrationMode::hWVR);
        
    return enumSet;
}
   	

std::string CCalibrationMode::name(const CalibrationModeMod::CalibrationMode& f) {
    switch (f) {
    
    case CalibrationModeMod::HOLOGRAPHY:
      return CCalibrationMode::sHOLOGRAPHY;
    
    case CalibrationModeMod::INTERFEROMETRY:
      return CCalibrationMode::sINTERFEROMETRY;
    
    case CalibrationModeMod::OPTICAL:
      return CCalibrationMode::sOPTICAL;
    
    case CalibrationModeMod::RADIOMETRY:
      return CCalibrationMode::sRADIOMETRY;
    
    case CalibrationModeMod::WVR:
      return CCalibrationMode::sWVR;
    	
    }
    return std::string("");
}

	

	
std::string CCalibrationMode::help(const CalibrationModeMod::CalibrationMode& f) {
    switch (f) {
    
    case CalibrationModeMod::HOLOGRAPHY:
      return CCalibrationMode::hHOLOGRAPHY;
    
    case CalibrationModeMod::INTERFEROMETRY:
      return CCalibrationMode::hINTERFEROMETRY;
    
    case CalibrationModeMod::OPTICAL:
      return CCalibrationMode::hOPTICAL;
    
    case CalibrationModeMod::RADIOMETRY:
      return CCalibrationMode::hRADIOMETRY;
    
    case CalibrationModeMod::WVR:
      return CCalibrationMode::hWVR;
    	
    }
    return std::string("");
}
   	

CalibrationModeMod::CalibrationMode CCalibrationMode::newCalibrationMode(const std::string& name) {
		
    if (name == CCalibrationMode::sHOLOGRAPHY) {
        return CalibrationModeMod::HOLOGRAPHY;
    }
    	
    if (name == CCalibrationMode::sINTERFEROMETRY) {
        return CalibrationModeMod::INTERFEROMETRY;
    }
    	
    if (name == CCalibrationMode::sOPTICAL) {
        return CalibrationModeMod::OPTICAL;
    }
    	
    if (name == CCalibrationMode::sRADIOMETRY) {
        return CalibrationModeMod::RADIOMETRY;
    }
    	
    if (name == CCalibrationMode::sWVR) {
        return CalibrationModeMod::WVR;
    }
    
    throw badString(name);
}

CalibrationModeMod::CalibrationMode CCalibrationMode::literal(const std::string& name) {
		
    if (name == CCalibrationMode::sHOLOGRAPHY) {
        return CalibrationModeMod::HOLOGRAPHY;
    }
    	
    if (name == CCalibrationMode::sINTERFEROMETRY) {
        return CalibrationModeMod::INTERFEROMETRY;
    }
    	
    if (name == CCalibrationMode::sOPTICAL) {
        return CalibrationModeMod::OPTICAL;
    }
    	
    if (name == CCalibrationMode::sRADIOMETRY) {
        return CalibrationModeMod::RADIOMETRY;
    }
    	
    if (name == CCalibrationMode::sWVR) {
        return CalibrationModeMod::WVR;
    }
    
    throw badString(name);
}

CalibrationModeMod::CalibrationMode CCalibrationMode::from_int(unsigned int i) {
	vector<string> names = sCalibrationModeSet();
	if (i >= names.size()) throw badInt(i);
	return newCalibrationMode(names.at(i));
}

	

string CCalibrationMode::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'CalibrationMode'.";
}

string CCalibrationMode::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'CalibrationMode'.";
	return oss.str();
}

