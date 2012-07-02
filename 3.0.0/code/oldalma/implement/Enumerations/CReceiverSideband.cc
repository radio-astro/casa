
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
 * File CReceiverSideband.cpp
 */
#include <sstream>
#include <CReceiverSideband.h>
#include <string>
using namespace std;

	
const std::string& CReceiverSideband::sNOSB = "NOSB";
	
const std::string& CReceiverSideband::sDSB = "DSB";
	
const std::string& CReceiverSideband::sSSB = "SSB";
	
const std::string& CReceiverSideband::sTSB = "TSB";
	
const std::vector<std::string> CReceiverSideband::sReceiverSidebandSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CReceiverSideband::sNOSB);
    
    enumSet.insert(enumSet.end(), CReceiverSideband::sDSB);
    
    enumSet.insert(enumSet.end(), CReceiverSideband::sSSB);
    
    enumSet.insert(enumSet.end(), CReceiverSideband::sTSB);
        
    return enumSet;
}

	

	
	
const std::string& CReceiverSideband::hNOSB = "Direct output signal (no frequency conversion)";
	
const std::string& CReceiverSideband::hDSB = "DOuble side-band ouput";
	
const std::string& CReceiverSideband::hSSB = "Single-side band receiver.";
	
const std::string& CReceiverSideband::hTSB = "Eeceiver with dual-output";
	
const std::vector<std::string> CReceiverSideband::hReceiverSidebandSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CReceiverSideband::hNOSB);
    
    enumSet.insert(enumSet.end(), CReceiverSideband::hDSB);
    
    enumSet.insert(enumSet.end(), CReceiverSideband::hSSB);
    
    enumSet.insert(enumSet.end(), CReceiverSideband::hTSB);
        
    return enumSet;
}
   	

std::string CReceiverSideband::name(const ReceiverSidebandMod::ReceiverSideband& f) {
    switch (f) {
    
    case ReceiverSidebandMod::NOSB:
      return CReceiverSideband::sNOSB;
    
    case ReceiverSidebandMod::DSB:
      return CReceiverSideband::sDSB;
    
    case ReceiverSidebandMod::SSB:
      return CReceiverSideband::sSSB;
    
    case ReceiverSidebandMod::TSB:
      return CReceiverSideband::sTSB;
    	
    }
    return std::string("");
}

	

	
std::string CReceiverSideband::help(const ReceiverSidebandMod::ReceiverSideband& f) {
    switch (f) {
    
    case ReceiverSidebandMod::NOSB:
      return CReceiverSideband::hNOSB;
    
    case ReceiverSidebandMod::DSB:
      return CReceiverSideband::hDSB;
    
    case ReceiverSidebandMod::SSB:
      return CReceiverSideband::hSSB;
    
    case ReceiverSidebandMod::TSB:
      return CReceiverSideband::hTSB;
    	
    }
    return std::string("");
}
   	

ReceiverSidebandMod::ReceiverSideband CReceiverSideband::newReceiverSideband(const std::string& name) {
		
    if (name == CReceiverSideband::sNOSB) {
        return ReceiverSidebandMod::NOSB;
    }
    	
    if (name == CReceiverSideband::sDSB) {
        return ReceiverSidebandMod::DSB;
    }
    	
    if (name == CReceiverSideband::sSSB) {
        return ReceiverSidebandMod::SSB;
    }
    	
    if (name == CReceiverSideband::sTSB) {
        return ReceiverSidebandMod::TSB;
    }
    
    throw badString(name);
}

ReceiverSidebandMod::ReceiverSideband CReceiverSideband::literal(const std::string& name) {
		
    if (name == CReceiverSideband::sNOSB) {
        return ReceiverSidebandMod::NOSB;
    }
    	
    if (name == CReceiverSideband::sDSB) {
        return ReceiverSidebandMod::DSB;
    }
    	
    if (name == CReceiverSideband::sSSB) {
        return ReceiverSidebandMod::SSB;
    }
    	
    if (name == CReceiverSideband::sTSB) {
        return ReceiverSidebandMod::TSB;
    }
    
    throw badString(name);
}

ReceiverSidebandMod::ReceiverSideband CReceiverSideband::from_int(unsigned int i) {
	vector<string> names = sReceiverSidebandSet();
	if (i >= names.size()) throw badInt(i);
	return newReceiverSideband(names.at(i));
}

	

string CReceiverSideband::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'ReceiverSideband'.";
}

string CReceiverSideband::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'ReceiverSideband'.";
	return oss.str();
}

