
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
 * File CFilterMode.cpp
 */
#include <sstream>
#include <CFilterMode.h>
#include <string>
using namespace std;

	
const std::string& CFilterMode::sFILTER_NA = "FILTER_NA";
	
const std::string& CFilterMode::sFILTER_TDM = "FILTER_TDM";
	
const std::string& CFilterMode::sFILTER_TFB = "FILTER_TFB";
	
const std::vector<std::string> CFilterMode::sFilterModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CFilterMode::sFILTER_NA);
    
    enumSet.insert(enumSet.end(), CFilterMode::sFILTER_TDM);
    
    enumSet.insert(enumSet.end(), CFilterMode::sFILTER_TFB);
        
    return enumSet;
}

	

	
	
const std::string& CFilterMode::hFILTER_NA = " Not Applicable (2 antenna prototype). The Tunable Filter Banks are not implemented";
	
const std::string& CFilterMode::hFILTER_TDM = "Time Division Mode. In this mode the Tunable Filter banks are bypassed";
	
const std::string& CFilterMode::hFILTER_TFB = "The Tunable Filter Bank is implemented and used";
	
const std::vector<std::string> CFilterMode::hFilterModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CFilterMode::hFILTER_NA);
    
    enumSet.insert(enumSet.end(), CFilterMode::hFILTER_TDM);
    
    enumSet.insert(enumSet.end(), CFilterMode::hFILTER_TFB);
        
    return enumSet;
}
   	

std::string CFilterMode::name(const FilterModeMod::FilterMode& f) {
    switch (f) {
    
    case FilterModeMod::FILTER_NA:
      return CFilterMode::sFILTER_NA;
    
    case FilterModeMod::FILTER_TDM:
      return CFilterMode::sFILTER_TDM;
    
    case FilterModeMod::FILTER_TFB:
      return CFilterMode::sFILTER_TFB;
    	
    }
    return std::string("");
}

	

	
std::string CFilterMode::help(const FilterModeMod::FilterMode& f) {
    switch (f) {
    
    case FilterModeMod::FILTER_NA:
      return CFilterMode::hFILTER_NA;
    
    case FilterModeMod::FILTER_TDM:
      return CFilterMode::hFILTER_TDM;
    
    case FilterModeMod::FILTER_TFB:
      return CFilterMode::hFILTER_TFB;
    	
    }
    return std::string("");
}
   	

FilterModeMod::FilterMode CFilterMode::newFilterMode(const std::string& name) {
		
    if (name == CFilterMode::sFILTER_NA) {
        return FilterModeMod::FILTER_NA;
    }
    	
    if (name == CFilterMode::sFILTER_TDM) {
        return FilterModeMod::FILTER_TDM;
    }
    	
    if (name == CFilterMode::sFILTER_TFB) {
        return FilterModeMod::FILTER_TFB;
    }
    
    throw badString(name);
}

FilterModeMod::FilterMode CFilterMode::literal(const std::string& name) {
		
    if (name == CFilterMode::sFILTER_NA) {
        return FilterModeMod::FILTER_NA;
    }
    	
    if (name == CFilterMode::sFILTER_TDM) {
        return FilterModeMod::FILTER_TDM;
    }
    	
    if (name == CFilterMode::sFILTER_TFB) {
        return FilterModeMod::FILTER_TFB;
    }
    
    throw badString(name);
}

FilterModeMod::FilterMode CFilterMode::from_int(unsigned int i) {
	vector<string> names = sFilterModeSet();
	if (i >= names.size()) throw badInt(i);
	return newFilterMode(names.at(i));
}

	

string CFilterMode::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'FilterMode'.";
}

string CFilterMode::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'FilterMode'.";
	return oss.str();
}

