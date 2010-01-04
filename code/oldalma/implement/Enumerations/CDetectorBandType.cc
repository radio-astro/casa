
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
 * File CDetectorBandType.cpp
 */
#include <sstream>
#include <CDetectorBandType.h>
#include <string>
using namespace std;

	
const std::string& CDetectorBandType::sBASEBAND = "BASEBAND";
	
const std::string& CDetectorBandType::sDOWN_CONVERTER = "DOWN_CONVERTER";
	
const std::string& CDetectorBandType::sHOLOGRAPHY_RECEIVER = "HOLOGRAPHY_RECEIVER";
	
const std::vector<std::string> CDetectorBandType::sDetectorBandTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CDetectorBandType::sBASEBAND);
    
    enumSet.insert(enumSet.end(), CDetectorBandType::sDOWN_CONVERTER);
    
    enumSet.insert(enumSet.end(), CDetectorBandType::sHOLOGRAPHY_RECEIVER);
        
    return enumSet;
}

	

	
	
const std::string& CDetectorBandType::hBASEBAND = "Detector in Baseband Processor";
	
const std::string& CDetectorBandType::hDOWN_CONVERTER = "Detector in Down - Converter";
	
const std::string& CDetectorBandType::hHOLOGRAPHY_RECEIVER = "Detector in Holography Receiver";
	
const std::vector<std::string> CDetectorBandType::hDetectorBandTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CDetectorBandType::hBASEBAND);
    
    enumSet.insert(enumSet.end(), CDetectorBandType::hDOWN_CONVERTER);
    
    enumSet.insert(enumSet.end(), CDetectorBandType::hHOLOGRAPHY_RECEIVER);
        
    return enumSet;
}
   	

std::string CDetectorBandType::name(const DetectorBandTypeMod::DetectorBandType& f) {
    switch (f) {
    
    case DetectorBandTypeMod::BASEBAND:
      return CDetectorBandType::sBASEBAND;
    
    case DetectorBandTypeMod::DOWN_CONVERTER:
      return CDetectorBandType::sDOWN_CONVERTER;
    
    case DetectorBandTypeMod::HOLOGRAPHY_RECEIVER:
      return CDetectorBandType::sHOLOGRAPHY_RECEIVER;
    	
    }
    return std::string("");
}

	

	
std::string CDetectorBandType::help(const DetectorBandTypeMod::DetectorBandType& f) {
    switch (f) {
    
    case DetectorBandTypeMod::BASEBAND:
      return CDetectorBandType::hBASEBAND;
    
    case DetectorBandTypeMod::DOWN_CONVERTER:
      return CDetectorBandType::hDOWN_CONVERTER;
    
    case DetectorBandTypeMod::HOLOGRAPHY_RECEIVER:
      return CDetectorBandType::hHOLOGRAPHY_RECEIVER;
    	
    }
    return std::string("");
}
   	

DetectorBandTypeMod::DetectorBandType CDetectorBandType::newDetectorBandType(const std::string& name) {
		
    if (name == CDetectorBandType::sBASEBAND) {
        return DetectorBandTypeMod::BASEBAND;
    }
    	
    if (name == CDetectorBandType::sDOWN_CONVERTER) {
        return DetectorBandTypeMod::DOWN_CONVERTER;
    }
    	
    if (name == CDetectorBandType::sHOLOGRAPHY_RECEIVER) {
        return DetectorBandTypeMod::HOLOGRAPHY_RECEIVER;
    }
    
    throw badString(name);
}

DetectorBandTypeMod::DetectorBandType CDetectorBandType::literal(const std::string& name) {
		
    if (name == CDetectorBandType::sBASEBAND) {
        return DetectorBandTypeMod::BASEBAND;
    }
    	
    if (name == CDetectorBandType::sDOWN_CONVERTER) {
        return DetectorBandTypeMod::DOWN_CONVERTER;
    }
    	
    if (name == CDetectorBandType::sHOLOGRAPHY_RECEIVER) {
        return DetectorBandTypeMod::HOLOGRAPHY_RECEIVER;
    }
    
    throw badString(name);
}

DetectorBandTypeMod::DetectorBandType CDetectorBandType::from_int(unsigned int i) {
	vector<string> names = sDetectorBandTypeSet();
	if (i >= names.size()) throw badInt(i);
	return newDetectorBandType(names.at(i));
}

	

string CDetectorBandType::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'DetectorBandType'.";
}

string CDetectorBandType::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'DetectorBandType'.";
	return oss.str();
}

