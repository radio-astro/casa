
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
 * File CNetSideband.cpp
 */
#include <sstream>
#include <CNetSideband.h>
#include <string>
using namespace std;

	
const std::string& CNetSideband::sNOSB = "NOSB";
	
const std::string& CNetSideband::sLSB = "LSB";
	
const std::string& CNetSideband::sUSB = "USB";
	
const std::string& CNetSideband::sDSB = "DSB";
	
const std::vector<std::string> CNetSideband::sNetSidebandSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CNetSideband::sNOSB);
    
    enumSet.insert(enumSet.end(), CNetSideband::sLSB);
    
    enumSet.insert(enumSet.end(), CNetSideband::sUSB);
    
    enumSet.insert(enumSet.end(), CNetSideband::sDSB);
        
    return enumSet;
}

	

	
	
const std::string& CNetSideband::hNOSB = "No side band (no frequency conversion)";
	
const std::string& CNetSideband::hLSB = "Lower side band";
	
const std::string& CNetSideband::hUSB = "Upper side band";
	
const std::string& CNetSideband::hDSB = "Double side band";
	
const std::vector<std::string> CNetSideband::hNetSidebandSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CNetSideband::hNOSB);
    
    enumSet.insert(enumSet.end(), CNetSideband::hLSB);
    
    enumSet.insert(enumSet.end(), CNetSideband::hUSB);
    
    enumSet.insert(enumSet.end(), CNetSideband::hDSB);
        
    return enumSet;
}
   	

std::string CNetSideband::name(const NetSidebandMod::NetSideband& f) {
    switch (f) {
    
    case NetSidebandMod::NOSB:
      return CNetSideband::sNOSB;
    
    case NetSidebandMod::LSB:
      return CNetSideband::sLSB;
    
    case NetSidebandMod::USB:
      return CNetSideband::sUSB;
    
    case NetSidebandMod::DSB:
      return CNetSideband::sDSB;
    	
    }
    return std::string("");
}

	

	
std::string CNetSideband::help(const NetSidebandMod::NetSideband& f) {
    switch (f) {
    
    case NetSidebandMod::NOSB:
      return CNetSideband::hNOSB;
    
    case NetSidebandMod::LSB:
      return CNetSideband::hLSB;
    
    case NetSidebandMod::USB:
      return CNetSideband::hUSB;
    
    case NetSidebandMod::DSB:
      return CNetSideband::hDSB;
    	
    }
    return std::string("");
}
   	

NetSidebandMod::NetSideband CNetSideband::newNetSideband(const std::string& name) {
		
    if (name == CNetSideband::sNOSB) {
        return NetSidebandMod::NOSB;
    }
    	
    if (name == CNetSideband::sLSB) {
        return NetSidebandMod::LSB;
    }
    	
    if (name == CNetSideband::sUSB) {
        return NetSidebandMod::USB;
    }
    	
    if (name == CNetSideband::sDSB) {
        return NetSidebandMod::DSB;
    }
    
    throw badString(name);
}

NetSidebandMod::NetSideband CNetSideband::literal(const std::string& name) {
		
    if (name == CNetSideband::sNOSB) {
        return NetSidebandMod::NOSB;
    }
    	
    if (name == CNetSideband::sLSB) {
        return NetSidebandMod::LSB;
    }
    	
    if (name == CNetSideband::sUSB) {
        return NetSidebandMod::USB;
    }
    	
    if (name == CNetSideband::sDSB) {
        return NetSidebandMod::DSB;
    }
    
    throw badString(name);
}

NetSidebandMod::NetSideband CNetSideband::from_int(unsigned int i) {
	vector<string> names = sNetSidebandSet();
	if (i >= names.size()) throw badInt(i);
	return newNetSideband(names.at(i));
}

	

string CNetSideband::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'NetSideband'.";
}

string CNetSideband::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'NetSideband'.";
	return oss.str();
}

