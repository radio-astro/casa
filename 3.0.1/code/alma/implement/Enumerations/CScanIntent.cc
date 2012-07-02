
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
 * File CScanIntent.cpp
 */
#include <sstream>
#include <CScanIntent.h>
#include <string>
using namespace std;


int CScanIntent::version() {
	return ScanIntentMod::version;
	}
	
string CScanIntent::revision () {
	return ScanIntentMod::revision;
}

unsigned int CScanIntent::size() {
	return 19;
	}
	
	
const std::string& CScanIntent::sCALIBRATE_AMPLI = "CALIBRATE_AMPLI";
	
const std::string& CScanIntent::sCALIBRATE_ATMOSPHERE = "CALIBRATE_ATMOSPHERE";
	
const std::string& CScanIntent::sCALIBRATE_BANDPASS = "CALIBRATE_BANDPASS";
	
const std::string& CScanIntent::sCALIBRATE_DELAY = "CALIBRATE_DELAY";
	
const std::string& CScanIntent::sCALIBRATE_FLUX = "CALIBRATE_FLUX";
	
const std::string& CScanIntent::sCALIBRATE_FOCUS = "CALIBRATE_FOCUS";
	
const std::string& CScanIntent::sCALIBRATE_FOCUS_X = "CALIBRATE_FOCUS_X";
	
const std::string& CScanIntent::sCALIBRATE_FOCUS_Y = "CALIBRATE_FOCUS_Y";
	
const std::string& CScanIntent::sCALIBRATE_PHASE = "CALIBRATE_PHASE";
	
const std::string& CScanIntent::sCALIBRATE_POINTING = "CALIBRATE_POINTING";
	
const std::string& CScanIntent::sCALIBRATE_POLARIZATION = "CALIBRATE_POLARIZATION";
	
const std::string& CScanIntent::sCALIBRATE_SIDEBAND_RATIO = "CALIBRATE_SIDEBAND_RATIO";
	
const std::string& CScanIntent::sCALIBRATE_WVR = "CALIBRATE_WVR";
	
const std::string& CScanIntent::sDO_SKYDIP = "DO_SKYDIP";
	
const std::string& CScanIntent::sMAP_ANTENNA_SURFACE = "MAP_ANTENNA_SURFACE";
	
const std::string& CScanIntent::sMAP_PRIMARY_BEAM = "MAP_PRIMARY_BEAM";
	
const std::string& CScanIntent::sOBSERVE_TARGET = "OBSERVE_TARGET";
	
const std::string& CScanIntent::sTEST = "TEST";
	
const std::string& CScanIntent::sUNSPECIFIED = "UNSPECIFIED";
	
const std::vector<std::string> CScanIntent::names() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_AMPLI);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_ATMOSPHERE);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_BANDPASS);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_DELAY);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_FLUX);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_FOCUS);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_FOCUS_X);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_FOCUS_Y);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_PHASE);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_POINTING);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_POLARIZATION);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_SIDEBAND_RATIO);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATE_WVR);
    
    enumSet.insert(enumSet.end(), CScanIntent::sDO_SKYDIP);
    
    enumSet.insert(enumSet.end(), CScanIntent::sMAP_ANTENNA_SURFACE);
    
    enumSet.insert(enumSet.end(), CScanIntent::sMAP_PRIMARY_BEAM);
    
    enumSet.insert(enumSet.end(), CScanIntent::sOBSERVE_TARGET);
    
    enumSet.insert(enumSet.end(), CScanIntent::sTEST);
    
    enumSet.insert(enumSet.end(), CScanIntent::sUNSPECIFIED);
        
    return enumSet;
}

std::string CScanIntent::name(const ScanIntentMod::ScanIntent& f) {
    switch (f) {
    
    case ScanIntentMod::CALIBRATE_AMPLI:
      return CScanIntent::sCALIBRATE_AMPLI;
    
    case ScanIntentMod::CALIBRATE_ATMOSPHERE:
      return CScanIntent::sCALIBRATE_ATMOSPHERE;
    
    case ScanIntentMod::CALIBRATE_BANDPASS:
      return CScanIntent::sCALIBRATE_BANDPASS;
    
    case ScanIntentMod::CALIBRATE_DELAY:
      return CScanIntent::sCALIBRATE_DELAY;
    
    case ScanIntentMod::CALIBRATE_FLUX:
      return CScanIntent::sCALIBRATE_FLUX;
    
    case ScanIntentMod::CALIBRATE_FOCUS:
      return CScanIntent::sCALIBRATE_FOCUS;
    
    case ScanIntentMod::CALIBRATE_FOCUS_X:
      return CScanIntent::sCALIBRATE_FOCUS_X;
    
    case ScanIntentMod::CALIBRATE_FOCUS_Y:
      return CScanIntent::sCALIBRATE_FOCUS_Y;
    
    case ScanIntentMod::CALIBRATE_PHASE:
      return CScanIntent::sCALIBRATE_PHASE;
    
    case ScanIntentMod::CALIBRATE_POINTING:
      return CScanIntent::sCALIBRATE_POINTING;
    
    case ScanIntentMod::CALIBRATE_POLARIZATION:
      return CScanIntent::sCALIBRATE_POLARIZATION;
    
    case ScanIntentMod::CALIBRATE_SIDEBAND_RATIO:
      return CScanIntent::sCALIBRATE_SIDEBAND_RATIO;
    
    case ScanIntentMod::CALIBRATE_WVR:
      return CScanIntent::sCALIBRATE_WVR;
    
    case ScanIntentMod::DO_SKYDIP:
      return CScanIntent::sDO_SKYDIP;
    
    case ScanIntentMod::MAP_ANTENNA_SURFACE:
      return CScanIntent::sMAP_ANTENNA_SURFACE;
    
    case ScanIntentMod::MAP_PRIMARY_BEAM:
      return CScanIntent::sMAP_PRIMARY_BEAM;
    
    case ScanIntentMod::OBSERVE_TARGET:
      return CScanIntent::sOBSERVE_TARGET;
    
    case ScanIntentMod::TEST:
      return CScanIntent::sTEST;
    
    case ScanIntentMod::UNSPECIFIED:
      return CScanIntent::sUNSPECIFIED;
    	
    }
    // Impossible siutation but....who knows with C++ enums
    throw badInt((int) f);
}

ScanIntentMod::ScanIntent CScanIntent::newScanIntent(const std::string& name) {
		
    if (name == CScanIntent::sCALIBRATE_AMPLI) {
        return ScanIntentMod::CALIBRATE_AMPLI;
    }
    	
    if (name == CScanIntent::sCALIBRATE_ATMOSPHERE) {
        return ScanIntentMod::CALIBRATE_ATMOSPHERE;
    }
    	
    if (name == CScanIntent::sCALIBRATE_BANDPASS) {
        return ScanIntentMod::CALIBRATE_BANDPASS;
    }
    	
    if (name == CScanIntent::sCALIBRATE_DELAY) {
        return ScanIntentMod::CALIBRATE_DELAY;
    }
    	
    if (name == CScanIntent::sCALIBRATE_FLUX) {
        return ScanIntentMod::CALIBRATE_FLUX;
    }
    	
    if (name == CScanIntent::sCALIBRATE_FOCUS) {
        return ScanIntentMod::CALIBRATE_FOCUS;
    }
    	
    if (name == CScanIntent::sCALIBRATE_FOCUS_X) {
        return ScanIntentMod::CALIBRATE_FOCUS_X;
    }
    	
    if (name == CScanIntent::sCALIBRATE_FOCUS_Y) {
        return ScanIntentMod::CALIBRATE_FOCUS_Y;
    }
    	
    if (name == CScanIntent::sCALIBRATE_PHASE) {
        return ScanIntentMod::CALIBRATE_PHASE;
    }
    	
    if (name == CScanIntent::sCALIBRATE_POINTING) {
        return ScanIntentMod::CALIBRATE_POINTING;
    }
    	
    if (name == CScanIntent::sCALIBRATE_POLARIZATION) {
        return ScanIntentMod::CALIBRATE_POLARIZATION;
    }
    	
    if (name == CScanIntent::sCALIBRATE_SIDEBAND_RATIO) {
        return ScanIntentMod::CALIBRATE_SIDEBAND_RATIO;
    }
    	
    if (name == CScanIntent::sCALIBRATE_WVR) {
        return ScanIntentMod::CALIBRATE_WVR;
    }
    	
    if (name == CScanIntent::sDO_SKYDIP) {
        return ScanIntentMod::DO_SKYDIP;
    }
    	
    if (name == CScanIntent::sMAP_ANTENNA_SURFACE) {
        return ScanIntentMod::MAP_ANTENNA_SURFACE;
    }
    	
    if (name == CScanIntent::sMAP_PRIMARY_BEAM) {
        return ScanIntentMod::MAP_PRIMARY_BEAM;
    }
    	
    if (name == CScanIntent::sOBSERVE_TARGET) {
        return ScanIntentMod::OBSERVE_TARGET;
    }
    	
    if (name == CScanIntent::sTEST) {
        return ScanIntentMod::TEST;
    }
    	
    if (name == CScanIntent::sUNSPECIFIED) {
        return ScanIntentMod::UNSPECIFIED;
    }
    
    throw badString(name);
}

ScanIntentMod::ScanIntent CScanIntent::literal(const std::string& name) {
		
    if (name == CScanIntent::sCALIBRATE_AMPLI) {
        return ScanIntentMod::CALIBRATE_AMPLI;
    }
    	
    if (name == CScanIntent::sCALIBRATE_ATMOSPHERE) {
        return ScanIntentMod::CALIBRATE_ATMOSPHERE;
    }
    	
    if (name == CScanIntent::sCALIBRATE_BANDPASS) {
        return ScanIntentMod::CALIBRATE_BANDPASS;
    }
    	
    if (name == CScanIntent::sCALIBRATE_DELAY) {
        return ScanIntentMod::CALIBRATE_DELAY;
    }
    	
    if (name == CScanIntent::sCALIBRATE_FLUX) {
        return ScanIntentMod::CALIBRATE_FLUX;
    }
    	
    if (name == CScanIntent::sCALIBRATE_FOCUS) {
        return ScanIntentMod::CALIBRATE_FOCUS;
    }
    	
    if (name == CScanIntent::sCALIBRATE_FOCUS_X) {
        return ScanIntentMod::CALIBRATE_FOCUS_X;
    }
    	
    if (name == CScanIntent::sCALIBRATE_FOCUS_Y) {
        return ScanIntentMod::CALIBRATE_FOCUS_Y;
    }
    	
    if (name == CScanIntent::sCALIBRATE_PHASE) {
        return ScanIntentMod::CALIBRATE_PHASE;
    }
    	
    if (name == CScanIntent::sCALIBRATE_POINTING) {
        return ScanIntentMod::CALIBRATE_POINTING;
    }
    	
    if (name == CScanIntent::sCALIBRATE_POLARIZATION) {
        return ScanIntentMod::CALIBRATE_POLARIZATION;
    }
    	
    if (name == CScanIntent::sCALIBRATE_SIDEBAND_RATIO) {
        return ScanIntentMod::CALIBRATE_SIDEBAND_RATIO;
    }
    	
    if (name == CScanIntent::sCALIBRATE_WVR) {
        return ScanIntentMod::CALIBRATE_WVR;
    }
    	
    if (name == CScanIntent::sDO_SKYDIP) {
        return ScanIntentMod::DO_SKYDIP;
    }
    	
    if (name == CScanIntent::sMAP_ANTENNA_SURFACE) {
        return ScanIntentMod::MAP_ANTENNA_SURFACE;
    }
    	
    if (name == CScanIntent::sMAP_PRIMARY_BEAM) {
        return ScanIntentMod::MAP_PRIMARY_BEAM;
    }
    	
    if (name == CScanIntent::sOBSERVE_TARGET) {
        return ScanIntentMod::OBSERVE_TARGET;
    }
    	
    if (name == CScanIntent::sTEST) {
        return ScanIntentMod::TEST;
    }
    	
    if (name == CScanIntent::sUNSPECIFIED) {
        return ScanIntentMod::UNSPECIFIED;
    }
    
    throw badString(name);
}

ScanIntentMod::ScanIntent CScanIntent::from_int(unsigned int i) {
	vector<string> names_ = names();
	if (i >= names_.size()) throw badInt(i);
	return newScanIntent(names_.at(i));
}

string CScanIntent::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'ScanIntent'.";
}

string CScanIntent::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'ScanIntent'.";
	return oss.str();
}

