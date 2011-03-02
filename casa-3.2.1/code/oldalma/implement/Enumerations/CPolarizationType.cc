
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
 * File CPolarizationType.cpp
 */
#include <sstream>
#include <CPolarizationType.h>
#include <string>
using namespace std;

	
const std::string& CPolarizationType::sR = "R";
	
const std::string& CPolarizationType::sL = "L";
	
const std::string& CPolarizationType::sX = "X";
	
const std::string& CPolarizationType::sY = "Y";
	
const std::vector<std::string> CPolarizationType::sPolarizationTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CPolarizationType::sR);
    
    enumSet.insert(enumSet.end(), CPolarizationType::sL);
    
    enumSet.insert(enumSet.end(), CPolarizationType::sX);
    
    enumSet.insert(enumSet.end(), CPolarizationType::sY);
        
    return enumSet;
}

	

	
	
const std::string& CPolarizationType::hR = "Right-handed Circular";
	
const std::string& CPolarizationType::hL = "Left-handed Circular";
	
const std::string& CPolarizationType::hX = "X linear";
	
const std::string& CPolarizationType::hY = "Y linear";
	
const std::vector<std::string> CPolarizationType::hPolarizationTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CPolarizationType::hR);
    
    enumSet.insert(enumSet.end(), CPolarizationType::hL);
    
    enumSet.insert(enumSet.end(), CPolarizationType::hX);
    
    enumSet.insert(enumSet.end(), CPolarizationType::hY);
        
    return enumSet;
}
   	

std::string CPolarizationType::name(const PolarizationTypeMod::PolarizationType& f) {
    switch (f) {
    
    case PolarizationTypeMod::R:
      return CPolarizationType::sR;
    
    case PolarizationTypeMod::L:
      return CPolarizationType::sL;
    
    case PolarizationTypeMod::X:
      return CPolarizationType::sX;
    
    case PolarizationTypeMod::Y:
      return CPolarizationType::sY;
    	
    }
    return std::string("");
}

	

	
std::string CPolarizationType::help(const PolarizationTypeMod::PolarizationType& f) {
    switch (f) {
    
    case PolarizationTypeMod::R:
      return CPolarizationType::hR;
    
    case PolarizationTypeMod::L:
      return CPolarizationType::hL;
    
    case PolarizationTypeMod::X:
      return CPolarizationType::hX;
    
    case PolarizationTypeMod::Y:
      return CPolarizationType::hY;
    	
    }
    return std::string("");
}
   	

PolarizationTypeMod::PolarizationType CPolarizationType::newPolarizationType(const std::string& name) {
		
    if (name == CPolarizationType::sR) {
        return PolarizationTypeMod::R;
    }
    	
    if (name == CPolarizationType::sL) {
        return PolarizationTypeMod::L;
    }
    	
    if (name == CPolarizationType::sX) {
        return PolarizationTypeMod::X;
    }
    	
    if (name == CPolarizationType::sY) {
        return PolarizationTypeMod::Y;
    }
    
    throw badString(name);
}

PolarizationTypeMod::PolarizationType CPolarizationType::literal(const std::string& name) {
		
    if (name == CPolarizationType::sR) {
        return PolarizationTypeMod::R;
    }
    	
    if (name == CPolarizationType::sL) {
        return PolarizationTypeMod::L;
    }
    	
    if (name == CPolarizationType::sX) {
        return PolarizationTypeMod::X;
    }
    	
    if (name == CPolarizationType::sY) {
        return PolarizationTypeMod::Y;
    }
    
    throw badString(name);
}

PolarizationTypeMod::PolarizationType CPolarizationType::from_int(unsigned int i) {
	vector<string> names = sPolarizationTypeSet();
	if (i >= names.size()) throw badInt(i);
	return newPolarizationType(names.at(i));
}

	

string CPolarizationType::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'PolarizationType'.";
}

string CPolarizationType::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'PolarizationType'.";
	return oss.str();
}

