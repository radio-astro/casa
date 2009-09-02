
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
 * File CSchedulerMode.cpp
 */
#include <sstream>
#include <CSchedulerMode.h>
#include <string>
using namespace std;

	
const std::string& CSchedulerMode::sDYNAMIC = "DYNAMIC";
	
const std::string& CSchedulerMode::sINTERACTIVE = "INTERACTIVE";
	
const std::string& CSchedulerMode::sMANUAL = "MANUAL";
	
const std::string& CSchedulerMode::sQUEUED = "QUEUED";
	
const std::vector<std::string> CSchedulerMode::sSchedulerModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSchedulerMode::sDYNAMIC);
    
    enumSet.insert(enumSet.end(), CSchedulerMode::sINTERACTIVE);
    
    enumSet.insert(enumSet.end(), CSchedulerMode::sMANUAL);
    
    enumSet.insert(enumSet.end(), CSchedulerMode::sQUEUED);
        
    return enumSet;
}

	

	
	
const std::string& CSchedulerMode::hDYNAMIC = "Dynamic scheduling";
	
const std::string& CSchedulerMode::hINTERACTIVE = "Interactive scheduling";
	
const std::string& CSchedulerMode::hMANUAL = "Manual scheduling";
	
const std::string& CSchedulerMode::hQUEUED = "Queued scheduling";
	
const std::vector<std::string> CSchedulerMode::hSchedulerModeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSchedulerMode::hDYNAMIC);
    
    enumSet.insert(enumSet.end(), CSchedulerMode::hINTERACTIVE);
    
    enumSet.insert(enumSet.end(), CSchedulerMode::hMANUAL);
    
    enumSet.insert(enumSet.end(), CSchedulerMode::hQUEUED);
        
    return enumSet;
}
   	

std::string CSchedulerMode::name(const SchedulerModeMod::SchedulerMode& f) {
    switch (f) {
    
    case SchedulerModeMod::DYNAMIC:
      return CSchedulerMode::sDYNAMIC;
    
    case SchedulerModeMod::INTERACTIVE:
      return CSchedulerMode::sINTERACTIVE;
    
    case SchedulerModeMod::MANUAL:
      return CSchedulerMode::sMANUAL;
    
    case SchedulerModeMod::QUEUED:
      return CSchedulerMode::sQUEUED;
    	
    }
    return std::string("");
}

	

	
std::string CSchedulerMode::help(const SchedulerModeMod::SchedulerMode& f) {
    switch (f) {
    
    case SchedulerModeMod::DYNAMIC:
      return CSchedulerMode::hDYNAMIC;
    
    case SchedulerModeMod::INTERACTIVE:
      return CSchedulerMode::hINTERACTIVE;
    
    case SchedulerModeMod::MANUAL:
      return CSchedulerMode::hMANUAL;
    
    case SchedulerModeMod::QUEUED:
      return CSchedulerMode::hQUEUED;
    	
    }
    return std::string("");
}
   	

SchedulerModeMod::SchedulerMode CSchedulerMode::newSchedulerMode(const std::string& name) {
		
    if (name == CSchedulerMode::sDYNAMIC) {
        return SchedulerModeMod::DYNAMIC;
    }
    	
    if (name == CSchedulerMode::sINTERACTIVE) {
        return SchedulerModeMod::INTERACTIVE;
    }
    	
    if (name == CSchedulerMode::sMANUAL) {
        return SchedulerModeMod::MANUAL;
    }
    	
    if (name == CSchedulerMode::sQUEUED) {
        return SchedulerModeMod::QUEUED;
    }
    
    throw badString(name);
}

SchedulerModeMod::SchedulerMode CSchedulerMode::literal(const std::string& name) {
		
    if (name == CSchedulerMode::sDYNAMIC) {
        return SchedulerModeMod::DYNAMIC;
    }
    	
    if (name == CSchedulerMode::sINTERACTIVE) {
        return SchedulerModeMod::INTERACTIVE;
    }
    	
    if (name == CSchedulerMode::sMANUAL) {
        return SchedulerModeMod::MANUAL;
    }
    	
    if (name == CSchedulerMode::sQUEUED) {
        return SchedulerModeMod::QUEUED;
    }
    
    throw badString(name);
}

SchedulerModeMod::SchedulerMode CSchedulerMode::from_int(unsigned int i) {
	vector<string> names = sSchedulerModeSet();
	if (i >= names.size()) throw badInt(i);
	return newSchedulerMode(names.at(i));
}

	

string CSchedulerMode::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'SchedulerMode'.";
}

string CSchedulerMode::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'SchedulerMode'.";
	return oss.str();
}

