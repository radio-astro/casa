
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
 * File CAxisName.cpp
 */
#include <sstream>
#include <CAxisName.h>
#include <string>
using namespace std;

	
const std::string& CAxisName::sTIM = "TIM";
	
const std::string& CAxisName::sBAL = "BAL";
	
const std::string& CAxisName::sANT = "ANT";
	
const std::string& CAxisName::sBAB = "BAB";
	
const std::string& CAxisName::sSPW = "SPW";
	
const std::string& CAxisName::sSIB = "SIB";
	
const std::string& CAxisName::sSUB = "SUB";
	
const std::string& CAxisName::sBIN = "BIN";
	
const std::string& CAxisName::sAPC = "APC";
	
const std::string& CAxisName::sSPP = "SPP";
	
const std::string& CAxisName::sPOL = "POL";
	
const std::string& CAxisName::sHOL = "HOL";
	
const std::vector<std::string> CAxisName::sAxisNameSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CAxisName::sTIM);
    
    enumSet.insert(enumSet.end(), CAxisName::sBAL);
    
    enumSet.insert(enumSet.end(), CAxisName::sANT);
    
    enumSet.insert(enumSet.end(), CAxisName::sBAB);
    
    enumSet.insert(enumSet.end(), CAxisName::sSPW);
    
    enumSet.insert(enumSet.end(), CAxisName::sSIB);
    
    enumSet.insert(enumSet.end(), CAxisName::sSUB);
    
    enumSet.insert(enumSet.end(), CAxisName::sBIN);
    
    enumSet.insert(enumSet.end(), CAxisName::sAPC);
    
    enumSet.insert(enumSet.end(), CAxisName::sSPP);
    
    enumSet.insert(enumSet.end(), CAxisName::sPOL);
    
    enumSet.insert(enumSet.end(), CAxisName::sHOL);
        
    return enumSet;
}

	

	
	
const std::string& CAxisName::hTIM = "Time axis";
	
const std::string& CAxisName::hBAL = "Baseline number axis";
	
const std::string& CAxisName::hANT = "ANtenna number axis";
	
const std::string& CAxisName::hBAB = "Baseband axis";
	
const std::string& CAxisName::hSPW = "SPectral window number axis";
	
const std::string& CAxisName::hSIB = "Sideband axis.";
	
const std::string& CAxisName::hSUB = "Subband axis";
	
const std::string& CAxisName::hBIN = "Bin axis";
	
const std::string& CAxisName::hAPC = "Atmosphere phase correction axis";
	
const std::string& CAxisName::hSPP = "Spectral data axis";
	
const std::string& CAxisName::hPOL = "Polarization axes (Stokes parameters)";
	
const std::string& CAxisName::hHOL = "Holography axis.";
	
const std::vector<std::string> CAxisName::hAxisNameSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CAxisName::hTIM);
    
    enumSet.insert(enumSet.end(), CAxisName::hBAL);
    
    enumSet.insert(enumSet.end(), CAxisName::hANT);
    
    enumSet.insert(enumSet.end(), CAxisName::hBAB);
    
    enumSet.insert(enumSet.end(), CAxisName::hSPW);
    
    enumSet.insert(enumSet.end(), CAxisName::hSIB);
    
    enumSet.insert(enumSet.end(), CAxisName::hSUB);
    
    enumSet.insert(enumSet.end(), CAxisName::hBIN);
    
    enumSet.insert(enumSet.end(), CAxisName::hAPC);
    
    enumSet.insert(enumSet.end(), CAxisName::hSPP);
    
    enumSet.insert(enumSet.end(), CAxisName::hPOL);
    
    enumSet.insert(enumSet.end(), CAxisName::hHOL);
        
    return enumSet;
}
   	

std::string CAxisName::name(const AxisNameMod::AxisName& f) {
    switch (f) {
    
    case AxisNameMod::TIM:
      return CAxisName::sTIM;
    
    case AxisNameMod::BAL:
      return CAxisName::sBAL;
    
    case AxisNameMod::ANT:
      return CAxisName::sANT;
    
    case AxisNameMod::BAB:
      return CAxisName::sBAB;
    
    case AxisNameMod::SPW:
      return CAxisName::sSPW;
    
    case AxisNameMod::SIB:
      return CAxisName::sSIB;
    
    case AxisNameMod::SUB:
      return CAxisName::sSUB;
    
    case AxisNameMod::BIN:
      return CAxisName::sBIN;
    
    case AxisNameMod::APC:
      return CAxisName::sAPC;
    
    case AxisNameMod::SPP:
      return CAxisName::sSPP;
    
    case AxisNameMod::POL:
      return CAxisName::sPOL;
    
    case AxisNameMod::HOL:
      return CAxisName::sHOL;
    	
    }
    return std::string("");
}

	

	
std::string CAxisName::help(const AxisNameMod::AxisName& f) {
    switch (f) {
    
    case AxisNameMod::TIM:
      return CAxisName::hTIM;
    
    case AxisNameMod::BAL:
      return CAxisName::hBAL;
    
    case AxisNameMod::ANT:
      return CAxisName::hANT;
    
    case AxisNameMod::BAB:
      return CAxisName::hBAB;
    
    case AxisNameMod::SPW:
      return CAxisName::hSPW;
    
    case AxisNameMod::SIB:
      return CAxisName::hSIB;
    
    case AxisNameMod::SUB:
      return CAxisName::hSUB;
    
    case AxisNameMod::BIN:
      return CAxisName::hBIN;
    
    case AxisNameMod::APC:
      return CAxisName::hAPC;
    
    case AxisNameMod::SPP:
      return CAxisName::hSPP;
    
    case AxisNameMod::POL:
      return CAxisName::hPOL;
    
    case AxisNameMod::HOL:
      return CAxisName::hHOL;
    	
    }
    return std::string("");
}
   	

AxisNameMod::AxisName CAxisName::newAxisName(const std::string& name) {
		
    if (name == CAxisName::sTIM) {
        return AxisNameMod::TIM;
    }
    	
    if (name == CAxisName::sBAL) {
        return AxisNameMod::BAL;
    }
    	
    if (name == CAxisName::sANT) {
        return AxisNameMod::ANT;
    }
    	
    if (name == CAxisName::sBAB) {
        return AxisNameMod::BAB;
    }
    	
    if (name == CAxisName::sSPW) {
        return AxisNameMod::SPW;
    }
    	
    if (name == CAxisName::sSIB) {
        return AxisNameMod::SIB;
    }
    	
    if (name == CAxisName::sSUB) {
        return AxisNameMod::SUB;
    }
    	
    if (name == CAxisName::sBIN) {
        return AxisNameMod::BIN;
    }
    	
    if (name == CAxisName::sAPC) {
        return AxisNameMod::APC;
    }
    	
    if (name == CAxisName::sSPP) {
        return AxisNameMod::SPP;
    }
    	
    if (name == CAxisName::sPOL) {
        return AxisNameMod::POL;
    }
    	
    if (name == CAxisName::sHOL) {
        return AxisNameMod::HOL;
    }
    
    throw badString(name);
}

AxisNameMod::AxisName CAxisName::literal(const std::string& name) {
		
    if (name == CAxisName::sTIM) {
        return AxisNameMod::TIM;
    }
    	
    if (name == CAxisName::sBAL) {
        return AxisNameMod::BAL;
    }
    	
    if (name == CAxisName::sANT) {
        return AxisNameMod::ANT;
    }
    	
    if (name == CAxisName::sBAB) {
        return AxisNameMod::BAB;
    }
    	
    if (name == CAxisName::sSPW) {
        return AxisNameMod::SPW;
    }
    	
    if (name == CAxisName::sSIB) {
        return AxisNameMod::SIB;
    }
    	
    if (name == CAxisName::sSUB) {
        return AxisNameMod::SUB;
    }
    	
    if (name == CAxisName::sBIN) {
        return AxisNameMod::BIN;
    }
    	
    if (name == CAxisName::sAPC) {
        return AxisNameMod::APC;
    }
    	
    if (name == CAxisName::sSPP) {
        return AxisNameMod::SPP;
    }
    	
    if (name == CAxisName::sPOL) {
        return AxisNameMod::POL;
    }
    	
    if (name == CAxisName::sHOL) {
        return AxisNameMod::HOL;
    }
    
    throw badString(name);
}

AxisNameMod::AxisName CAxisName::from_int(unsigned int i) {
	vector<string> names = sAxisNameSet();
	if (i >= names.size()) throw badInt(i);
	return newAxisName(names.at(i));
}

	

string CAxisName::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'AxisName'.";
}

string CAxisName::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'AxisName'.";
	return oss.str();
}

