
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
 * File CCalCurveType.cpp
 */
#include <sstream>
#include <CCalCurveType.h>
#include <string>
using namespace std;

	
const std::string& CCalCurveType::sAMPLITUDE = "AMPLITUDE";
	
const std::string& CCalCurveType::sPHASE = "PHASE";
	
const std::vector<std::string> CCalCurveType::sCalCurveTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCalCurveType::sAMPLITUDE);
    
    enumSet.insert(enumSet.end(), CCalCurveType::sPHASE);
        
    return enumSet;
}

	

	
	
const std::string& CCalCurveType::hAMPLITUDE = "Calibration curve is Amplitude";
	
const std::string& CCalCurveType::hPHASE = "Calibration curve is phase";
	
const std::vector<std::string> CCalCurveType::hCalCurveTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCalCurveType::hAMPLITUDE);
    
    enumSet.insert(enumSet.end(), CCalCurveType::hPHASE);
        
    return enumSet;
}
   	

std::string CCalCurveType::name(const CalCurveTypeMod::CalCurveType& f) {
    switch (f) {
    
    case CalCurveTypeMod::AMPLITUDE:
      return CCalCurveType::sAMPLITUDE;
    
    case CalCurveTypeMod::PHASE:
      return CCalCurveType::sPHASE;
    	
    }
    return std::string("");
}

	

	
std::string CCalCurveType::help(const CalCurveTypeMod::CalCurveType& f) {
    switch (f) {
    
    case CalCurveTypeMod::AMPLITUDE:
      return CCalCurveType::hAMPLITUDE;
    
    case CalCurveTypeMod::PHASE:
      return CCalCurveType::hPHASE;
    	
    }
    return std::string("");
}
   	

CalCurveTypeMod::CalCurveType CCalCurveType::newCalCurveType(const std::string& name) {
		
    if (name == CCalCurveType::sAMPLITUDE) {
        return CalCurveTypeMod::AMPLITUDE;
    }
    	
    if (name == CCalCurveType::sPHASE) {
        return CalCurveTypeMod::PHASE;
    }
    
    throw badString(name);
}

CalCurveTypeMod::CalCurveType CCalCurveType::literal(const std::string& name) {
		
    if (name == CCalCurveType::sAMPLITUDE) {
        return CalCurveTypeMod::AMPLITUDE;
    }
    	
    if (name == CCalCurveType::sPHASE) {
        return CalCurveTypeMod::PHASE;
    }
    
    throw badString(name);
}

CalCurveTypeMod::CalCurveType CCalCurveType::from_int(unsigned int i) {
	vector<string> names = sCalCurveTypeSet();
	if (i >= names.size()) throw badInt(i);
	return newCalCurveType(names.at(i));
}

	

string CCalCurveType::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'CalCurveType'.";
}

string CCalCurveType::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'CalCurveType'.";
	return oss.str();
}

