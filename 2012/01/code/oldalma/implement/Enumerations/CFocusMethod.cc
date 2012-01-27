
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
 * File CFocusMethod.cpp
 */
#include <sstream>
#include <CFocusMethod.h>
#include <string>
using namespace std;

	
const std::string& CFocusMethod::sTHREE_POINT = "THREE_POINT";
	
const std::string& CFocusMethod::sFIVE_POINT = "FIVE_POINT";
	
const std::vector<std::string> CFocusMethod::sFocusMethodSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CFocusMethod::sTHREE_POINT);
    
    enumSet.insert(enumSet.end(), CFocusMethod::sFIVE_POINT);
        
    return enumSet;
}

	

	
	
const std::string& CFocusMethod::hTHREE_POINT = "Three-point measurement";
	
const std::string& CFocusMethod::hFIVE_POINT = "Five-point measurement";
	
const std::vector<std::string> CFocusMethod::hFocusMethodSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CFocusMethod::hTHREE_POINT);
    
    enumSet.insert(enumSet.end(), CFocusMethod::hFIVE_POINT);
        
    return enumSet;
}
   	

std::string CFocusMethod::name(const FocusMethodMod::FocusMethod& f) {
    switch (f) {
    
    case FocusMethodMod::THREE_POINT:
      return CFocusMethod::sTHREE_POINT;
    
    case FocusMethodMod::FIVE_POINT:
      return CFocusMethod::sFIVE_POINT;
    	
    }
    return std::string("");
}

	

	
std::string CFocusMethod::help(const FocusMethodMod::FocusMethod& f) {
    switch (f) {
    
    case FocusMethodMod::THREE_POINT:
      return CFocusMethod::hTHREE_POINT;
    
    case FocusMethodMod::FIVE_POINT:
      return CFocusMethod::hFIVE_POINT;
    	
    }
    return std::string("");
}
   	

FocusMethodMod::FocusMethod CFocusMethod::newFocusMethod(const std::string& name) {
		
    if (name == CFocusMethod::sTHREE_POINT) {
        return FocusMethodMod::THREE_POINT;
    }
    	
    if (name == CFocusMethod::sFIVE_POINT) {
        return FocusMethodMod::FIVE_POINT;
    }
    
    throw badString(name);
}

FocusMethodMod::FocusMethod CFocusMethod::literal(const std::string& name) {
		
    if (name == CFocusMethod::sTHREE_POINT) {
        return FocusMethodMod::THREE_POINT;
    }
    	
    if (name == CFocusMethod::sFIVE_POINT) {
        return FocusMethodMod::FIVE_POINT;
    }
    
    throw badString(name);
}

FocusMethodMod::FocusMethod CFocusMethod::from_int(unsigned int i) {
	vector<string> names = sFocusMethodSet();
	if (i >= names.size()) throw badInt(i);
	return newFocusMethod(names.at(i));
}

	

string CFocusMethod::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'FocusMethod'.";
}

string CFocusMethod::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'FocusMethod'.";
	return oss.str();
}

