
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
 * File CProcessorSubType.cpp
 */
#include <sstream>
#include <CProcessorSubType.h>
#include <string>
using namespace std;

	
const std::string& CProcessorSubType::sALMA_CORRELATOR_MODE = "ALMA_CORRELATOR_MODE";
	
const std::string& CProcessorSubType::sSQUARE_LAW_DETECTOR = "SQUARE_LAW_DETECTOR";
	
const std::string& CProcessorSubType::sHOLOGRAPHY = "HOLOGRAPHY";
	
const std::vector<std::string> CProcessorSubType::sProcessorSubTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CProcessorSubType::sALMA_CORRELATOR_MODE);
    
    enumSet.insert(enumSet.end(), CProcessorSubType::sSQUARE_LAW_DETECTOR);
    
    enumSet.insert(enumSet.end(), CProcessorSubType::sHOLOGRAPHY);
        
    return enumSet;
}

	

	
	
const std::string& CProcessorSubType::hALMA_CORRELATOR_MODE = "ALMA_CORRELATOR_MODE";
	
const std::string& CProcessorSubType::hSQUARE_LAW_DETECTOR = "SQUARE_LAW_DETECTOR";
	
const std::string& CProcessorSubType::hHOLOGRAPHY = "HOLOGRAPHY";
	
const std::vector<std::string> CProcessorSubType::hProcessorSubTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CProcessorSubType::hALMA_CORRELATOR_MODE);
    
    enumSet.insert(enumSet.end(), CProcessorSubType::hSQUARE_LAW_DETECTOR);
    
    enumSet.insert(enumSet.end(), CProcessorSubType::hHOLOGRAPHY);
        
    return enumSet;
}
   	

std::string CProcessorSubType::name(const ProcessorSubTypeMod::ProcessorSubType& f) {
    switch (f) {
    
    case ProcessorSubTypeMod::ALMA_CORRELATOR_MODE:
      return CProcessorSubType::sALMA_CORRELATOR_MODE;
    
    case ProcessorSubTypeMod::SQUARE_LAW_DETECTOR:
      return CProcessorSubType::sSQUARE_LAW_DETECTOR;
    
    case ProcessorSubTypeMod::HOLOGRAPHY:
      return CProcessorSubType::sHOLOGRAPHY;
    	
    }
    return std::string("");
}

	

	
std::string CProcessorSubType::help(const ProcessorSubTypeMod::ProcessorSubType& f) {
    switch (f) {
    
    case ProcessorSubTypeMod::ALMA_CORRELATOR_MODE:
      return CProcessorSubType::hALMA_CORRELATOR_MODE;
    
    case ProcessorSubTypeMod::SQUARE_LAW_DETECTOR:
      return CProcessorSubType::hSQUARE_LAW_DETECTOR;
    
    case ProcessorSubTypeMod::HOLOGRAPHY:
      return CProcessorSubType::hHOLOGRAPHY;
    	
    }
    return std::string("");
}
   	

ProcessorSubTypeMod::ProcessorSubType CProcessorSubType::newProcessorSubType(const std::string& name) {
		
    if (name == CProcessorSubType::sALMA_CORRELATOR_MODE) {
        return ProcessorSubTypeMod::ALMA_CORRELATOR_MODE;
    }
    	
    if (name == CProcessorSubType::sSQUARE_LAW_DETECTOR) {
        return ProcessorSubTypeMod::SQUARE_LAW_DETECTOR;
    }
    	
    if (name == CProcessorSubType::sHOLOGRAPHY) {
        return ProcessorSubTypeMod::HOLOGRAPHY;
    }
    
    throw badString(name);
}

ProcessorSubTypeMod::ProcessorSubType CProcessorSubType::literal(const std::string& name) {
		
    if (name == CProcessorSubType::sALMA_CORRELATOR_MODE) {
        return ProcessorSubTypeMod::ALMA_CORRELATOR_MODE;
    }
    	
    if (name == CProcessorSubType::sSQUARE_LAW_DETECTOR) {
        return ProcessorSubTypeMod::SQUARE_LAW_DETECTOR;
    }
    	
    if (name == CProcessorSubType::sHOLOGRAPHY) {
        return ProcessorSubTypeMod::HOLOGRAPHY;
    }
    
    throw badString(name);
}

ProcessorSubTypeMod::ProcessorSubType CProcessorSubType::from_int(unsigned int i) {
	vector<string> names = sProcessorSubTypeSet();
	if (i >= names.size()) throw badInt(i);
	return newProcessorSubType(names.at(i));
}

	

string CProcessorSubType::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'ProcessorSubType'.";
}

string CProcessorSubType::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'ProcessorSubType'.";
	return oss.str();
}

