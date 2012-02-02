
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
 * File CAtmPhaseCorrection.cpp
 */
#include <sstream>
#include <CAtmPhaseCorrection.h>
#include <string>
using namespace std;

	
const std::string& CAtmPhaseCorrection::sAP_UNCORRECTED = "AP_UNCORRECTED";
	
const std::string& CAtmPhaseCorrection::sAP_CORRECTED = "AP_CORRECTED";
	
const std::string& CAtmPhaseCorrection::sAP_MIXED = "AP_MIXED";
	
const std::vector<std::string> CAtmPhaseCorrection::sAtmPhaseCorrectionSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CAtmPhaseCorrection::sAP_UNCORRECTED);
    
    enumSet.insert(enumSet.end(), CAtmPhaseCorrection::sAP_CORRECTED);
    
    enumSet.insert(enumSet.end(), CAtmPhaseCorrection::sAP_MIXED);
        
    return enumSet;
}

	

	
	
const std::string& CAtmPhaseCorrection::hAP_UNCORRECTED = "Data has no WVR phase correction";
	
const std::string& CAtmPhaseCorrection::hAP_CORRECTED = "Data phases have been corrected using WVR data";
	
const std::string& CAtmPhaseCorrection::hAP_MIXED = "Data phases have been corrected on some baselines";
	
const std::vector<std::string> CAtmPhaseCorrection::hAtmPhaseCorrectionSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CAtmPhaseCorrection::hAP_UNCORRECTED);
    
    enumSet.insert(enumSet.end(), CAtmPhaseCorrection::hAP_CORRECTED);
    
    enumSet.insert(enumSet.end(), CAtmPhaseCorrection::hAP_MIXED);
        
    return enumSet;
}
   	

std::string CAtmPhaseCorrection::name(const AtmPhaseCorrectionMod::AtmPhaseCorrection& f) {
    switch (f) {
    
    case AtmPhaseCorrectionMod::AP_UNCORRECTED:
      return CAtmPhaseCorrection::sAP_UNCORRECTED;
    
    case AtmPhaseCorrectionMod::AP_CORRECTED:
      return CAtmPhaseCorrection::sAP_CORRECTED;
    
    case AtmPhaseCorrectionMod::AP_MIXED:
      return CAtmPhaseCorrection::sAP_MIXED;
    	
    }
    return std::string("");
}

	

	
std::string CAtmPhaseCorrection::help(const AtmPhaseCorrectionMod::AtmPhaseCorrection& f) {
    switch (f) {
    
    case AtmPhaseCorrectionMod::AP_UNCORRECTED:
      return CAtmPhaseCorrection::hAP_UNCORRECTED;
    
    case AtmPhaseCorrectionMod::AP_CORRECTED:
      return CAtmPhaseCorrection::hAP_CORRECTED;
    
    case AtmPhaseCorrectionMod::AP_MIXED:
      return CAtmPhaseCorrection::hAP_MIXED;
    	
    }
    return std::string("");
}
   	

AtmPhaseCorrectionMod::AtmPhaseCorrection CAtmPhaseCorrection::newAtmPhaseCorrection(const std::string& name) {
		
    if (name == CAtmPhaseCorrection::sAP_UNCORRECTED) {
        return AtmPhaseCorrectionMod::AP_UNCORRECTED;
    }
    	
    if (name == CAtmPhaseCorrection::sAP_CORRECTED) {
        return AtmPhaseCorrectionMod::AP_CORRECTED;
    }
    	
    if (name == CAtmPhaseCorrection::sAP_MIXED) {
        return AtmPhaseCorrectionMod::AP_MIXED;
    }
    
    throw badString(name);
}

AtmPhaseCorrectionMod::AtmPhaseCorrection CAtmPhaseCorrection::literal(const std::string& name) {
		
    if (name == CAtmPhaseCorrection::sAP_UNCORRECTED) {
        return AtmPhaseCorrectionMod::AP_UNCORRECTED;
    }
    	
    if (name == CAtmPhaseCorrection::sAP_CORRECTED) {
        return AtmPhaseCorrectionMod::AP_CORRECTED;
    }
    	
    if (name == CAtmPhaseCorrection::sAP_MIXED) {
        return AtmPhaseCorrectionMod::AP_MIXED;
    }
    
    throw badString(name);
}

AtmPhaseCorrectionMod::AtmPhaseCorrection CAtmPhaseCorrection::from_int(unsigned int i) {
	vector<string> names = sAtmPhaseCorrectionSet();
	if (i >= names.size()) throw badInt(i);
	return newAtmPhaseCorrection(names.at(i));
}

	

string CAtmPhaseCorrection::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'AtmPhaseCorrection'.";
}

string CAtmPhaseCorrection::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'AtmPhaseCorrection'.";
	return oss.str();
}

