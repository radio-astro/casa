
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
 * File CBasebandName.cpp
 */
#include <sstream>
#include <CBasebandName.h>
#include <string>
using namespace std;

	
const std::string& CBasebandName::sBB_1 = "BB_1";
	
const std::string& CBasebandName::sBB_2 = "BB_2";
	
const std::string& CBasebandName::sBB_3 = "BB_3";
	
const std::string& CBasebandName::sBB_4 = "BB_4";
	
const std::string& CBasebandName::sBB_5 = "BB_5";
	
const std::string& CBasebandName::sBB_6 = "BB_6";
	
const std::string& CBasebandName::sBB_7 = "BB_7";
	
const std::string& CBasebandName::sBB_8 = "BB_8";
	
const std::vector<std::string> CBasebandName::sBasebandNameSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CBasebandName::sBB_1);
    
    enumSet.insert(enumSet.end(), CBasebandName::sBB_2);
    
    enumSet.insert(enumSet.end(), CBasebandName::sBB_3);
    
    enumSet.insert(enumSet.end(), CBasebandName::sBB_4);
    
    enumSet.insert(enumSet.end(), CBasebandName::sBB_5);
    
    enumSet.insert(enumSet.end(), CBasebandName::sBB_6);
    
    enumSet.insert(enumSet.end(), CBasebandName::sBB_7);
    
    enumSet.insert(enumSet.end(), CBasebandName::sBB_8);
        
    return enumSet;
}

	

	
	
const std::string& CBasebandName::hBB_1 = "Baseband one";
	
const std::string& CBasebandName::hBB_2 = "Baseband two";
	
const std::string& CBasebandName::hBB_3 = "Baseband three";
	
const std::string& CBasebandName::hBB_4 = "Baseband four";
	
const std::string& CBasebandName::hBB_5 = "Baseband five (not ALMA)";
	
const std::string& CBasebandName::hBB_6 = "Baseband six (not ALMA)";
	
const std::string& CBasebandName::hBB_7 = "Baseband seven (not ALMA)";
	
const std::string& CBasebandName::hBB_8 = "Baseband eight (not ALMA)";
	
const std::vector<std::string> CBasebandName::hBasebandNameSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CBasebandName::hBB_1);
    
    enumSet.insert(enumSet.end(), CBasebandName::hBB_2);
    
    enumSet.insert(enumSet.end(), CBasebandName::hBB_3);
    
    enumSet.insert(enumSet.end(), CBasebandName::hBB_4);
    
    enumSet.insert(enumSet.end(), CBasebandName::hBB_5);
    
    enumSet.insert(enumSet.end(), CBasebandName::hBB_6);
    
    enumSet.insert(enumSet.end(), CBasebandName::hBB_7);
    
    enumSet.insert(enumSet.end(), CBasebandName::hBB_8);
        
    return enumSet;
}
   	

std::string CBasebandName::name(const BasebandNameMod::BasebandName& f) {
    switch (f) {
    
    case BasebandNameMod::BB_1:
      return CBasebandName::sBB_1;
    
    case BasebandNameMod::BB_2:
      return CBasebandName::sBB_2;
    
    case BasebandNameMod::BB_3:
      return CBasebandName::sBB_3;
    
    case BasebandNameMod::BB_4:
      return CBasebandName::sBB_4;
    
    case BasebandNameMod::BB_5:
      return CBasebandName::sBB_5;
    
    case BasebandNameMod::BB_6:
      return CBasebandName::sBB_6;
    
    case BasebandNameMod::BB_7:
      return CBasebandName::sBB_7;
    
    case BasebandNameMod::BB_8:
      return CBasebandName::sBB_8;
    	
    }
    return std::string("");
}

	

	
std::string CBasebandName::help(const BasebandNameMod::BasebandName& f) {
    switch (f) {
    
    case BasebandNameMod::BB_1:
      return CBasebandName::hBB_1;
    
    case BasebandNameMod::BB_2:
      return CBasebandName::hBB_2;
    
    case BasebandNameMod::BB_3:
      return CBasebandName::hBB_3;
    
    case BasebandNameMod::BB_4:
      return CBasebandName::hBB_4;
    
    case BasebandNameMod::BB_5:
      return CBasebandName::hBB_5;
    
    case BasebandNameMod::BB_6:
      return CBasebandName::hBB_6;
    
    case BasebandNameMod::BB_7:
      return CBasebandName::hBB_7;
    
    case BasebandNameMod::BB_8:
      return CBasebandName::hBB_8;
    	
    }
    return std::string("");
}
   	

BasebandNameMod::BasebandName CBasebandName::newBasebandName(const std::string& name) {
		
    if (name == CBasebandName::sBB_1) {
        return BasebandNameMod::BB_1;
    }
    	
    if (name == CBasebandName::sBB_2) {
        return BasebandNameMod::BB_2;
    }
    	
    if (name == CBasebandName::sBB_3) {
        return BasebandNameMod::BB_3;
    }
    	
    if (name == CBasebandName::sBB_4) {
        return BasebandNameMod::BB_4;
    }
    	
    if (name == CBasebandName::sBB_5) {
        return BasebandNameMod::BB_5;
    }
    	
    if (name == CBasebandName::sBB_6) {
        return BasebandNameMod::BB_6;
    }
    	
    if (name == CBasebandName::sBB_7) {
        return BasebandNameMod::BB_7;
    }
    	
    if (name == CBasebandName::sBB_8) {
        return BasebandNameMod::BB_8;
    }
    
    throw badString(name);
}

BasebandNameMod::BasebandName CBasebandName::literal(const std::string& name) {
		
    if (name == CBasebandName::sBB_1) {
        return BasebandNameMod::BB_1;
    }
    	
    if (name == CBasebandName::sBB_2) {
        return BasebandNameMod::BB_2;
    }
    	
    if (name == CBasebandName::sBB_3) {
        return BasebandNameMod::BB_3;
    }
    	
    if (name == CBasebandName::sBB_4) {
        return BasebandNameMod::BB_4;
    }
    	
    if (name == CBasebandName::sBB_5) {
        return BasebandNameMod::BB_5;
    }
    	
    if (name == CBasebandName::sBB_6) {
        return BasebandNameMod::BB_6;
    }
    	
    if (name == CBasebandName::sBB_7) {
        return BasebandNameMod::BB_7;
    }
    	
    if (name == CBasebandName::sBB_8) {
        return BasebandNameMod::BB_8;
    }
    
    throw badString(name);
}

BasebandNameMod::BasebandName CBasebandName::from_int(unsigned int i) {
	vector<string> names = sBasebandNameSet();
	if (i >= names.size()) throw badInt(i);
	return newBasebandName(names.at(i));
}

	

string CBasebandName::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'BasebandName'.";
}

string CBasebandName::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'BasebandName'.";
	return oss.str();
}

