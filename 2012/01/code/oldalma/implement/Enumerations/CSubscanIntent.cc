
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
 * File CSubscanIntent.cpp
 */
#include <sstream>
#include <CSubscanIntent.h>
#include <string>
using namespace std;

	
const std::string& CSubscanIntent::sHOLOGRAPHY_RASTER = "HOLOGRAPHY_RASTER";
	
const std::string& CSubscanIntent::sHOLOGRAPHY_PHASECAL = "HOLOGRAPHY_PHASECAL";
	
const std::string& CSubscanIntent::sUNSPECIFIED = "UNSPECIFIED";
	
const std::string& CSubscanIntent::sIN_FOCUS = "IN_FOCUS";
	
const std::string& CSubscanIntent::sOUT_OF_FOCUS = "OUT_OF_FOCUS";
	
const std::string& CSubscanIntent::sON_SOURCE = "ON_SOURCE";
	
const std::string& CSubscanIntent::sOFF_SOURCE = "OFF_SOURCE";
	
const std::string& CSubscanIntent::sMIXED_FOCUS = "MIXED_FOCUS";
	
const std::string& CSubscanIntent::sMIXED_POINTING = "MIXED_POINTING";
	
const std::string& CSubscanIntent::sREFERENCE = "REFERENCE";
	
const std::string& CSubscanIntent::sRASTER = "RASTER";
	
const std::vector<std::string> CSubscanIntent::sSubscanIntentSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sHOLOGRAPHY_RASTER);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sHOLOGRAPHY_PHASECAL);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sUNSPECIFIED);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sIN_FOCUS);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sOUT_OF_FOCUS);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sON_SOURCE);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sOFF_SOURCE);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sMIXED_FOCUS);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sMIXED_POINTING);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sREFERENCE);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::sRASTER);
        
    return enumSet;
}

	

	
	
const std::string& CSubscanIntent::hHOLOGRAPHY_RASTER = "The scanning subscans in a holography raster map.";
	
const std::string& CSubscanIntent::hHOLOGRAPHY_PHASECAL = "The boresight reference measurement for a holography raster map";
	
const std::string& CSubscanIntent::hUNSPECIFIED = "Unspecified";
	
const std::string& CSubscanIntent::hIN_FOCUS = "In Focus measurement";
	
const std::string& CSubscanIntent::hOUT_OF_FOCUS = "out of focus measurement";
	
const std::string& CSubscanIntent::hON_SOURCE = "on-source measurement";
	
const std::string& CSubscanIntent::hOFF_SOURCE = "off-source measurement";
	
const std::string& CSubscanIntent::hMIXED_FOCUS = "Focus measurement, some antennas in focus, some not";
	
const std::string& CSubscanIntent::hMIXED_POINTING = "Pointing measurement, some antennas are on -ource, some off-source";
	
const std::string& CSubscanIntent::hREFERENCE = "reference measurement";
	
const std::string& CSubscanIntent::hRASTER = "The scanning subscans in an astronomy raster map.";
	
const std::vector<std::string> CSubscanIntent::hSubscanIntentSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hHOLOGRAPHY_RASTER);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hHOLOGRAPHY_PHASECAL);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hUNSPECIFIED);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hIN_FOCUS);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hOUT_OF_FOCUS);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hON_SOURCE);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hOFF_SOURCE);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hMIXED_FOCUS);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hMIXED_POINTING);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hREFERENCE);
    
    enumSet.insert(enumSet.end(), CSubscanIntent::hRASTER);
        
    return enumSet;
}
   	

std::string CSubscanIntent::name(const SubscanIntentMod::SubscanIntent& f) {
    switch (f) {
    
    case SubscanIntentMod::HOLOGRAPHY_RASTER:
      return CSubscanIntent::sHOLOGRAPHY_RASTER;
    
    case SubscanIntentMod::HOLOGRAPHY_PHASECAL:
      return CSubscanIntent::sHOLOGRAPHY_PHASECAL;
    
    case SubscanIntentMod::UNSPECIFIED:
      return CSubscanIntent::sUNSPECIFIED;
    
    case SubscanIntentMod::IN_FOCUS:
      return CSubscanIntent::sIN_FOCUS;
    
    case SubscanIntentMod::OUT_OF_FOCUS:
      return CSubscanIntent::sOUT_OF_FOCUS;
    
    case SubscanIntentMod::ON_SOURCE:
      return CSubscanIntent::sON_SOURCE;
    
    case SubscanIntentMod::OFF_SOURCE:
      return CSubscanIntent::sOFF_SOURCE;
    
    case SubscanIntentMod::MIXED_FOCUS:
      return CSubscanIntent::sMIXED_FOCUS;
    
    case SubscanIntentMod::MIXED_POINTING:
      return CSubscanIntent::sMIXED_POINTING;
    
    case SubscanIntentMod::REFERENCE:
      return CSubscanIntent::sREFERENCE;
    
    case SubscanIntentMod::RASTER:
      return CSubscanIntent::sRASTER;
    	
    }
    return std::string("");
}

	

	
std::string CSubscanIntent::help(const SubscanIntentMod::SubscanIntent& f) {
    switch (f) {
    
    case SubscanIntentMod::HOLOGRAPHY_RASTER:
      return CSubscanIntent::hHOLOGRAPHY_RASTER;
    
    case SubscanIntentMod::HOLOGRAPHY_PHASECAL:
      return CSubscanIntent::hHOLOGRAPHY_PHASECAL;
    
    case SubscanIntentMod::UNSPECIFIED:
      return CSubscanIntent::hUNSPECIFIED;
    
    case SubscanIntentMod::IN_FOCUS:
      return CSubscanIntent::hIN_FOCUS;
    
    case SubscanIntentMod::OUT_OF_FOCUS:
      return CSubscanIntent::hOUT_OF_FOCUS;
    
    case SubscanIntentMod::ON_SOURCE:
      return CSubscanIntent::hON_SOURCE;
    
    case SubscanIntentMod::OFF_SOURCE:
      return CSubscanIntent::hOFF_SOURCE;
    
    case SubscanIntentMod::MIXED_FOCUS:
      return CSubscanIntent::hMIXED_FOCUS;
    
    case SubscanIntentMod::MIXED_POINTING:
      return CSubscanIntent::hMIXED_POINTING;
    
    case SubscanIntentMod::REFERENCE:
      return CSubscanIntent::hREFERENCE;
    
    case SubscanIntentMod::RASTER:
      return CSubscanIntent::hRASTER;
    	
    }
    return std::string("");
}
   	

SubscanIntentMod::SubscanIntent CSubscanIntent::newSubscanIntent(const std::string& name) {
		
    if (name == CSubscanIntent::sHOLOGRAPHY_RASTER) {
        return SubscanIntentMod::HOLOGRAPHY_RASTER;
    }
    	
    if (name == CSubscanIntent::sHOLOGRAPHY_PHASECAL) {
        return SubscanIntentMod::HOLOGRAPHY_PHASECAL;
    }
    	
    if (name == CSubscanIntent::sUNSPECIFIED) {
        return SubscanIntentMod::UNSPECIFIED;
    }
    	
    if (name == CSubscanIntent::sIN_FOCUS) {
        return SubscanIntentMod::IN_FOCUS;
    }
    	
    if (name == CSubscanIntent::sOUT_OF_FOCUS) {
        return SubscanIntentMod::OUT_OF_FOCUS;
    }
    	
    if (name == CSubscanIntent::sON_SOURCE) {
        return SubscanIntentMod::ON_SOURCE;
    }
    	
    if (name == CSubscanIntent::sOFF_SOURCE) {
        return SubscanIntentMod::OFF_SOURCE;
    }
    	
    if (name == CSubscanIntent::sMIXED_FOCUS) {
        return SubscanIntentMod::MIXED_FOCUS;
    }
    	
    if (name == CSubscanIntent::sMIXED_POINTING) {
        return SubscanIntentMod::MIXED_POINTING;
    }
    	
    if (name == CSubscanIntent::sREFERENCE) {
        return SubscanIntentMod::REFERENCE;
    }
    	
    if (name == CSubscanIntent::sRASTER) {
        return SubscanIntentMod::RASTER;
    }
    
    throw badString(name);
}

SubscanIntentMod::SubscanIntent CSubscanIntent::literal(const std::string& name) {
		
    if (name == CSubscanIntent::sHOLOGRAPHY_RASTER) {
        return SubscanIntentMod::HOLOGRAPHY_RASTER;
    }
    	
    if (name == CSubscanIntent::sHOLOGRAPHY_PHASECAL) {
        return SubscanIntentMod::HOLOGRAPHY_PHASECAL;
    }
    	
    if (name == CSubscanIntent::sUNSPECIFIED) {
        return SubscanIntentMod::UNSPECIFIED;
    }
    	
    if (name == CSubscanIntent::sIN_FOCUS) {
        return SubscanIntentMod::IN_FOCUS;
    }
    	
    if (name == CSubscanIntent::sOUT_OF_FOCUS) {
        return SubscanIntentMod::OUT_OF_FOCUS;
    }
    	
    if (name == CSubscanIntent::sON_SOURCE) {
        return SubscanIntentMod::ON_SOURCE;
    }
    	
    if (name == CSubscanIntent::sOFF_SOURCE) {
        return SubscanIntentMod::OFF_SOURCE;
    }
    	
    if (name == CSubscanIntent::sMIXED_FOCUS) {
        return SubscanIntentMod::MIXED_FOCUS;
    }
    	
    if (name == CSubscanIntent::sMIXED_POINTING) {
        return SubscanIntentMod::MIXED_POINTING;
    }
    	
    if (name == CSubscanIntent::sREFERENCE) {
        return SubscanIntentMod::REFERENCE;
    }
    	
    if (name == CSubscanIntent::sRASTER) {
        return SubscanIntentMod::RASTER;
    }
    
    throw badString(name);
}

SubscanIntentMod::SubscanIntent CSubscanIntent::from_int(unsigned int i) {
	vector<string> names = sSubscanIntentSet();
	if (i >= names.size()) throw badInt(i);
	return newSubscanIntent(names.at(i));
}

	

string CSubscanIntent::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'SubscanIntent'.";
}

string CSubscanIntent::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'SubscanIntent'.";
	return oss.str();
}

