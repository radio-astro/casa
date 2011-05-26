
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
 * File CPointingModelMode.cpp
 */
#include <sstream>
#include <CPointingModelMode.h>
#include <string>
using namespace std;

	
const std::string& CPointingModelMode::sRADIO = "RADIO";
	
const std::string& CPointingModelMode::sOPTICAL = "OPTICAL";
	
const std::vector<std::string> CPointingModelMode::sPointingModelModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CPointingModelMode::sRADIO);
    
    enumSet.insert(enumSet.end(), CPointingModelMode::sOPTICAL);
        
    return enumSet;
}

	

	
	
const std::string& CPointingModelMode::hRADIO = "Radio pointing model";
	
const std::string& CPointingModelMode::hOPTICAL = "Optical Pointing Model";
	
const std::vector<std::string> CPointingModelMode::hPointingModelModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CPointingModelMode::hRADIO);
    
    enumSet.insert(enumSet.end(), CPointingModelMode::hOPTICAL);
        
    return enumSet;
}
   	

std::string CPointingModelMode::name(const PointingModelModeMod::PointingModelMode& f) {
    switch (f) {
    
    case PointingModelModeMod::RADIO:
      return CPointingModelMode::sRADIO;
    
    case PointingModelModeMod::OPTICAL:
      return CPointingModelMode::sOPTICAL;
    	
    }
    return std::string("");
}

	

	
std::string CPointingModelMode::help(const PointingModelModeMod::PointingModelMode& f) {
    switch (f) {
    
    case PointingModelModeMod::RADIO:
      return CPointingModelMode::hRADIO;
    
    case PointingModelModeMod::OPTICAL:
      return CPointingModelMode::hOPTICAL;
    	
    }
    return std::string("");
}
   	

PointingModelModeMod::PointingModelMode CPointingModelMode::newPointingModelMode(const std::string& name) {
		
    if (name == CPointingModelMode::sRADIO) {
        return PointingModelModeMod::RADIO;
    }
    	
    if (name == CPointingModelMode::sOPTICAL) {
        return PointingModelModeMod::OPTICAL;
    }
    
    throw badString(name);
}

PointingModelModeMod::PointingModelMode CPointingModelMode::literal(const std::string& name) {
		
    if (name == CPointingModelMode::sRADIO) {
        return PointingModelModeMod::RADIO;
    }
    	
    if (name == CPointingModelMode::sOPTICAL) {
        return PointingModelModeMod::OPTICAL;
    }
    
    throw badString(name);
}

PointingModelModeMod::PointingModelMode CPointingModelMode::from_int(unsigned int i) {
	vector<string> names = sPointingModelModeSet();
	if (i >= names.size()) throw badInt(i);
	return newPointingModelMode(names.at(i));
}

	

string CPointingModelMode::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'PointingModelMode'.";
}

string CPointingModelMode::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'PointingModelMode'.";
	return oss.str();
}

