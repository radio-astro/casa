
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
 * File CHolographyChannelType.cpp
 */
#include <sstream>
#include <CHolographyChannelType.h>
#include <string>
using namespace std;

	
const std::string& CHolographyChannelType::sQ2 = "Q2";
	
const std::string& CHolographyChannelType::sQR = "QR";
	
const std::string& CHolographyChannelType::sQS = "QS";
	
const std::string& CHolographyChannelType::sR2 = "R2";
	
const std::string& CHolographyChannelType::sRS = "RS";
	
const std::string& CHolographyChannelType::sS2 = "S2";
	
const std::vector<std::string> CHolographyChannelType::sHolographyChannelTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::sQ2);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::sQR);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::sQS);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::sR2);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::sRS);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::sS2);
        
    return enumSet;
}

	

	
	
const std::string& CHolographyChannelType::hQ2 = "Quadrature channel auto-product";
	
const std::string& CHolographyChannelType::hQR = "Quadrature channel times Reference channel cross-product";
	
const std::string& CHolographyChannelType::hQS = "Quadrature channel times Signal channel cross-product";
	
const std::string& CHolographyChannelType::hR2 = "Reference channel auto-product";
	
const std::string& CHolographyChannelType::hRS = "Reference channel times Signal channel cross-product";
	
const std::string& CHolographyChannelType::hS2 = "Signal channel auto-product";
	
const std::vector<std::string> CHolographyChannelType::hHolographyChannelTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::hQ2);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::hQR);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::hQS);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::hR2);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::hRS);
    
    enumSet.insert(enumSet.end(), CHolographyChannelType::hS2);
        
    return enumSet;
}
   	

std::string CHolographyChannelType::name(const HolographyChannelTypeMod::HolographyChannelType& f) {
    switch (f) {
    
    case HolographyChannelTypeMod::Q2:
      return CHolographyChannelType::sQ2;
    
    case HolographyChannelTypeMod::QR:
      return CHolographyChannelType::sQR;
    
    case HolographyChannelTypeMod::QS:
      return CHolographyChannelType::sQS;
    
    case HolographyChannelTypeMod::R2:
      return CHolographyChannelType::sR2;
    
    case HolographyChannelTypeMod::RS:
      return CHolographyChannelType::sRS;
    
    case HolographyChannelTypeMod::S2:
      return CHolographyChannelType::sS2;
    	
    }
    return std::string("");
}

	

	
std::string CHolographyChannelType::help(const HolographyChannelTypeMod::HolographyChannelType& f) {
    switch (f) {
    
    case HolographyChannelTypeMod::Q2:
      return CHolographyChannelType::hQ2;
    
    case HolographyChannelTypeMod::QR:
      return CHolographyChannelType::hQR;
    
    case HolographyChannelTypeMod::QS:
      return CHolographyChannelType::hQS;
    
    case HolographyChannelTypeMod::R2:
      return CHolographyChannelType::hR2;
    
    case HolographyChannelTypeMod::RS:
      return CHolographyChannelType::hRS;
    
    case HolographyChannelTypeMod::S2:
      return CHolographyChannelType::hS2;
    	
    }
    return std::string("");
}
   	

HolographyChannelTypeMod::HolographyChannelType CHolographyChannelType::newHolographyChannelType(const std::string& name) {
		
    if (name == CHolographyChannelType::sQ2) {
        return HolographyChannelTypeMod::Q2;
    }
    	
    if (name == CHolographyChannelType::sQR) {
        return HolographyChannelTypeMod::QR;
    }
    	
    if (name == CHolographyChannelType::sQS) {
        return HolographyChannelTypeMod::QS;
    }
    	
    if (name == CHolographyChannelType::sR2) {
        return HolographyChannelTypeMod::R2;
    }
    	
    if (name == CHolographyChannelType::sRS) {
        return HolographyChannelTypeMod::RS;
    }
    	
    if (name == CHolographyChannelType::sS2) {
        return HolographyChannelTypeMod::S2;
    }
    
    throw badString(name);
}

HolographyChannelTypeMod::HolographyChannelType CHolographyChannelType::literal(const std::string& name) {
		
    if (name == CHolographyChannelType::sQ2) {
        return HolographyChannelTypeMod::Q2;
    }
    	
    if (name == CHolographyChannelType::sQR) {
        return HolographyChannelTypeMod::QR;
    }
    	
    if (name == CHolographyChannelType::sQS) {
        return HolographyChannelTypeMod::QS;
    }
    	
    if (name == CHolographyChannelType::sR2) {
        return HolographyChannelTypeMod::R2;
    }
    	
    if (name == CHolographyChannelType::sRS) {
        return HolographyChannelTypeMod::RS;
    }
    	
    if (name == CHolographyChannelType::sS2) {
        return HolographyChannelTypeMod::S2;
    }
    
    throw badString(name);
}

HolographyChannelTypeMod::HolographyChannelType CHolographyChannelType::from_int(unsigned int i) {
	vector<string> names = sHolographyChannelTypeSet();
	if (i >= names.size()) throw badInt(i);
	return newHolographyChannelType(names.at(i));
}

	

string CHolographyChannelType::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'HolographyChannelType'.";
}

string CHolographyChannelType::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'HolographyChannelType'.";
	return oss.str();
}

