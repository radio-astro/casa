
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
 * File CAccumMode.cpp
 */
#include <sstream>
#include <CAccumMode.h>
#include <string>
using namespace std;

	
const std::string& CAccumMode::sALMA_FAST = "ALMA_FAST";
	
const std::string& CAccumMode::sALMA_NORMAL = "ALMA_NORMAL";
	
const std::vector<std::string> CAccumMode::sAccumModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CAccumMode::sALMA_FAST);
    
    enumSet.insert(enumSet.end(), CAccumMode::sALMA_NORMAL);
        
    return enumSet;
}

	

	
	
const std::string& CAccumMode::hALMA_FAST = "1 ms dump time, available only for autocorrelation";
	
const std::string& CAccumMode::hALMA_NORMAL = "16ms dump time, available for both autocorrelation and cross-orrelation";
	
const std::vector<std::string> CAccumMode::hAccumModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CAccumMode::hALMA_FAST);
    
    enumSet.insert(enumSet.end(), CAccumMode::hALMA_NORMAL);
        
    return enumSet;
}
   	

std::string CAccumMode::name(const AccumModeMod::AccumMode& f) {
    switch (f) {
    
    case AccumModeMod::ALMA_FAST:
      return CAccumMode::sALMA_FAST;
    
    case AccumModeMod::ALMA_NORMAL:
      return CAccumMode::sALMA_NORMAL;
    	
    }
    return std::string("");
}

	

	
std::string CAccumMode::help(const AccumModeMod::AccumMode& f) {
    switch (f) {
    
    case AccumModeMod::ALMA_FAST:
      return CAccumMode::hALMA_FAST;
    
    case AccumModeMod::ALMA_NORMAL:
      return CAccumMode::hALMA_NORMAL;
    	
    }
    return std::string("");
}
   	

AccumModeMod::AccumMode CAccumMode::newAccumMode(const std::string& name) {
		
    if (name == CAccumMode::sALMA_FAST) {
        return AccumModeMod::ALMA_FAST;
    }
    	
    if (name == CAccumMode::sALMA_NORMAL) {
        return AccumModeMod::ALMA_NORMAL;
    }
    
    throw badString(name);
}

AccumModeMod::AccumMode CAccumMode::literal(const std::string& name) {
		
    if (name == CAccumMode::sALMA_FAST) {
        return AccumModeMod::ALMA_FAST;
    }
    	
    if (name == CAccumMode::sALMA_NORMAL) {
        return AccumModeMod::ALMA_NORMAL;
    }
    
    throw badString(name);
}

AccumModeMod::AccumMode CAccumMode::from_int(unsigned int i) {
	vector<string> names = sAccumModeSet();
	if (i >= names.size()) throw badInt(i);
	return newAccumMode(names.at(i));
}

	

string CAccumMode::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'AccumMode'.";
}

string CAccumMode::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'AccumMode'.";
	return oss.str();
}

