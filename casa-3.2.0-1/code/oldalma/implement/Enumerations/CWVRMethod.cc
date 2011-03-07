
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
 * File CWVRMethod.cpp
 */
#include <sstream>
#include <CWVRMethod.h>
#include <string>
using namespace std;

	
const std::string& CWVRMethod::sATM_MODEL = "ATM_MODEL";
	
const std::string& CWVRMethod::sEMPIRICAL = "EMPIRICAL";
	
const std::vector<std::string> CWVRMethod::sWVRMethodSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CWVRMethod::sATM_MODEL);
    
    enumSet.insert(enumSet.end(), CWVRMethod::sEMPIRICAL);
        
    return enumSet;
}

	

	
	
const std::string& CWVRMethod::hATM_MODEL = "WVR data reduction uses ATM model";
	
const std::string& CWVRMethod::hEMPIRICAL = "WVR data reduction optimized using actual phase data";
	
const std::vector<std::string> CWVRMethod::hWVRMethodSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CWVRMethod::hATM_MODEL);
    
    enumSet.insert(enumSet.end(), CWVRMethod::hEMPIRICAL);
        
    return enumSet;
}
   	

std::string CWVRMethod::name(const WVRMethodMod::WVRMethod& f) {
    switch (f) {
    
    case WVRMethodMod::ATM_MODEL:
      return CWVRMethod::sATM_MODEL;
    
    case WVRMethodMod::EMPIRICAL:
      return CWVRMethod::sEMPIRICAL;
    	
    }
    return std::string("");
}

	

	
std::string CWVRMethod::help(const WVRMethodMod::WVRMethod& f) {
    switch (f) {
    
    case WVRMethodMod::ATM_MODEL:
      return CWVRMethod::hATM_MODEL;
    
    case WVRMethodMod::EMPIRICAL:
      return CWVRMethod::hEMPIRICAL;
    	
    }
    return std::string("");
}
   	

WVRMethodMod::WVRMethod CWVRMethod::newWVRMethod(const std::string& name) {
		
    if (name == CWVRMethod::sATM_MODEL) {
        return WVRMethodMod::ATM_MODEL;
    }
    	
    if (name == CWVRMethod::sEMPIRICAL) {
        return WVRMethodMod::EMPIRICAL;
    }
    
    throw badString(name);
}

WVRMethodMod::WVRMethod CWVRMethod::literal(const std::string& name) {
		
    if (name == CWVRMethod::sATM_MODEL) {
        return WVRMethodMod::ATM_MODEL;
    }
    	
    if (name == CWVRMethod::sEMPIRICAL) {
        return WVRMethodMod::EMPIRICAL;
    }
    
    throw badString(name);
}

WVRMethodMod::WVRMethod CWVRMethod::from_int(unsigned int i) {
	vector<string> names = sWVRMethodSet();
	if (i >= names.size()) throw badInt(i);
	return newWVRMethod(names.at(i));
}

	

string CWVRMethod::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'WVRMethod'.";
}

string CWVRMethod::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'WVRMethod'.";
	return oss.str();
}

