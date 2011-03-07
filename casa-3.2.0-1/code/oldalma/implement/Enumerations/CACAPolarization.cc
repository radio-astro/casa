
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
 * File CACAPolarization.cpp
 */
#include <sstream>
#include <CACAPolarization.h>
#include <string>
using namespace std;

	
const std::string& CACAPolarization::sACA_STANDARD = "ACA_STANDARD";
	
const std::string& CACAPolarization::sACA_XX_YY_SUM = "ACA_XX_YY_SUM";
	
const std::string& CACAPolarization::sACA_XX_50 = "ACA_XX_50";
	
const std::string& CACAPolarization::sACA_YY_50 = "ACA_YY_50";
	
const std::vector<std::string> CACAPolarization::sACAPolarizationSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CACAPolarization::sACA_STANDARD);
    
    enumSet.insert(enumSet.end(), CACAPolarization::sACA_XX_YY_SUM);
    
    enumSet.insert(enumSet.end(), CACAPolarization::sACA_XX_50);
    
    enumSet.insert(enumSet.end(), CACAPolarization::sACA_YY_50);
        
    return enumSet;
}

	

	
	
const std::string& CACAPolarization::hACA_STANDARD = "Data product is the standard way (it is a standard observed Stokes parameter)";
	
const std::string& CACAPolarization::hACA_XX_YY_SUM = "ACA has calculated I by averaging XX and YY";
	
const std::string& CACAPolarization::hACA_XX_50 = "ACA has averaged XX and XX delayed by half a FFT period";
	
const std::string& CACAPolarization::hACA_YY_50 = "ACA has averaged YY and YY delayed by half a FFT period";
	
const std::vector<std::string> CACAPolarization::hACAPolarizationSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CACAPolarization::hACA_STANDARD);
    
    enumSet.insert(enumSet.end(), CACAPolarization::hACA_XX_YY_SUM);
    
    enumSet.insert(enumSet.end(), CACAPolarization::hACA_XX_50);
    
    enumSet.insert(enumSet.end(), CACAPolarization::hACA_YY_50);
        
    return enumSet;
}
   	

std::string CACAPolarization::name(const ACAPolarizationMod::ACAPolarization& f) {
    switch (f) {
    
    case ACAPolarizationMod::ACA_STANDARD:
      return CACAPolarization::sACA_STANDARD;
    
    case ACAPolarizationMod::ACA_XX_YY_SUM:
      return CACAPolarization::sACA_XX_YY_SUM;
    
    case ACAPolarizationMod::ACA_XX_50:
      return CACAPolarization::sACA_XX_50;
    
    case ACAPolarizationMod::ACA_YY_50:
      return CACAPolarization::sACA_YY_50;
    	
    }
    return std::string("");
}

	

	
std::string CACAPolarization::help(const ACAPolarizationMod::ACAPolarization& f) {
    switch (f) {
    
    case ACAPolarizationMod::ACA_STANDARD:
      return CACAPolarization::hACA_STANDARD;
    
    case ACAPolarizationMod::ACA_XX_YY_SUM:
      return CACAPolarization::hACA_XX_YY_SUM;
    
    case ACAPolarizationMod::ACA_XX_50:
      return CACAPolarization::hACA_XX_50;
    
    case ACAPolarizationMod::ACA_YY_50:
      return CACAPolarization::hACA_YY_50;
    	
    }
    return std::string("");
}
   	

ACAPolarizationMod::ACAPolarization CACAPolarization::newACAPolarization(const std::string& name) {
		
    if (name == CACAPolarization::sACA_STANDARD) {
        return ACAPolarizationMod::ACA_STANDARD;
    }
    	
    if (name == CACAPolarization::sACA_XX_YY_SUM) {
        return ACAPolarizationMod::ACA_XX_YY_SUM;
    }
    	
    if (name == CACAPolarization::sACA_XX_50) {
        return ACAPolarizationMod::ACA_XX_50;
    }
    	
    if (name == CACAPolarization::sACA_YY_50) {
        return ACAPolarizationMod::ACA_YY_50;
    }
    
    throw badString(name);
}

ACAPolarizationMod::ACAPolarization CACAPolarization::literal(const std::string& name) {
		
    if (name == CACAPolarization::sACA_STANDARD) {
        return ACAPolarizationMod::ACA_STANDARD;
    }
    	
    if (name == CACAPolarization::sACA_XX_YY_SUM) {
        return ACAPolarizationMod::ACA_XX_YY_SUM;
    }
    	
    if (name == CACAPolarization::sACA_XX_50) {
        return ACAPolarizationMod::ACA_XX_50;
    }
    	
    if (name == CACAPolarization::sACA_YY_50) {
        return ACAPolarizationMod::ACA_YY_50;
    }
    
    throw badString(name);
}

ACAPolarizationMod::ACAPolarization CACAPolarization::from_int(unsigned int i) {
	vector<string> names = sACAPolarizationSet();
	if (i >= names.size()) throw badInt(i);
	return newACAPolarization(names.at(i));
}

	

string CACAPolarization::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'ACAPolarization'.";
}

string CACAPolarization::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'ACAPolarization'.";
	return oss.str();
}

