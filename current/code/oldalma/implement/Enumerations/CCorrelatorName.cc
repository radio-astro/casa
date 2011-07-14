
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
 * File CCorrelatorName.cpp
 */
#include <sstream>
#include <CCorrelatorName.h>
#include <string>
using namespace std;

	
const std::string& CCorrelatorName::sALMA_ACA = "ALMA_ACA";
	
const std::string& CCorrelatorName::sALMA_BASELINE = "ALMA_BASELINE";
	
const std::string& CCorrelatorName::sALMA_BASELINE_ATF = "ALMA_BASELINE_ATF";
	
const std::string& CCorrelatorName::sALMA_BASELINE_PROTO_OSF = "ALMA_BASELINE_PROTO_OSF";
	
const std::string& CCorrelatorName::sHERSCHEL = "HERSCHEL";
	
const std::string& CCorrelatorName::sIRAM_PDB = "IRAM_PDB";
	
const std::string& CCorrelatorName::sIRAM_30M_VESPA = "IRAM_30M_VESPA";
	
const std::string& CCorrelatorName::sIRAM_WILMA = "IRAM_WILMA";
	
const std::vector<std::string> CCorrelatorName::sCorrelatorNameSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCorrelatorName::sALMA_ACA);
    
    enumSet.insert(enumSet.end(), CCorrelatorName::sALMA_BASELINE);
    
    enumSet.insert(enumSet.end(), CCorrelatorName::sALMA_BASELINE_ATF);
    
    enumSet.insert(enumSet.end(), CCorrelatorName::sALMA_BASELINE_PROTO_OSF);
    
    enumSet.insert(enumSet.end(), CCorrelatorName::sHERSCHEL);
    
    enumSet.insert(enumSet.end(), CCorrelatorName::sIRAM_PDB);
    
    enumSet.insert(enumSet.end(), CCorrelatorName::sIRAM_30M_VESPA);
    
    enumSet.insert(enumSet.end(), CCorrelatorName::sIRAM_WILMA);
        
    return enumSet;
}

	

	

std::string CCorrelatorName::name(const CorrelatorNameMod::CorrelatorName& f) {
    switch (f) {
    
    case CorrelatorNameMod::ALMA_ACA:
      return CCorrelatorName::sALMA_ACA;
    
    case CorrelatorNameMod::ALMA_BASELINE:
      return CCorrelatorName::sALMA_BASELINE;
    
    case CorrelatorNameMod::ALMA_BASELINE_ATF:
      return CCorrelatorName::sALMA_BASELINE_ATF;
    
    case CorrelatorNameMod::ALMA_BASELINE_PROTO_OSF:
      return CCorrelatorName::sALMA_BASELINE_PROTO_OSF;
    
    case CorrelatorNameMod::HERSCHEL:
      return CCorrelatorName::sHERSCHEL;
    
    case CorrelatorNameMod::IRAM_PDB:
      return CCorrelatorName::sIRAM_PDB;
    
    case CorrelatorNameMod::IRAM_30M_VESPA:
      return CCorrelatorName::sIRAM_30M_VESPA;
    
    case CorrelatorNameMod::IRAM_WILMA:
      return CCorrelatorName::sIRAM_WILMA;
    	
    }
    return std::string("");
}

	

	

CorrelatorNameMod::CorrelatorName CCorrelatorName::newCorrelatorName(const std::string& name) {
		
    if (name == CCorrelatorName::sALMA_ACA) {
        return CorrelatorNameMod::ALMA_ACA;
    }
    	
    if (name == CCorrelatorName::sALMA_BASELINE) {
        return CorrelatorNameMod::ALMA_BASELINE;
    }
    	
    if (name == CCorrelatorName::sALMA_BASELINE_ATF) {
        return CorrelatorNameMod::ALMA_BASELINE_ATF;
    }
    	
    if (name == CCorrelatorName::sALMA_BASELINE_PROTO_OSF) {
        return CorrelatorNameMod::ALMA_BASELINE_PROTO_OSF;
    }
    	
    if (name == CCorrelatorName::sHERSCHEL) {
        return CorrelatorNameMod::HERSCHEL;
    }
    	
    if (name == CCorrelatorName::sIRAM_PDB) {
        return CorrelatorNameMod::IRAM_PDB;
    }
    	
    if (name == CCorrelatorName::sIRAM_30M_VESPA) {
        return CorrelatorNameMod::IRAM_30M_VESPA;
    }
    	
    if (name == CCorrelatorName::sIRAM_WILMA) {
        return CorrelatorNameMod::IRAM_WILMA;
    }
    
    throw badString(name);
}

CorrelatorNameMod::CorrelatorName CCorrelatorName::literal(const std::string& name) {
		
    if (name == CCorrelatorName::sALMA_ACA) {
        return CorrelatorNameMod::ALMA_ACA;
    }
    	
    if (name == CCorrelatorName::sALMA_BASELINE) {
        return CorrelatorNameMod::ALMA_BASELINE;
    }
    	
    if (name == CCorrelatorName::sALMA_BASELINE_ATF) {
        return CorrelatorNameMod::ALMA_BASELINE_ATF;
    }
    	
    if (name == CCorrelatorName::sALMA_BASELINE_PROTO_OSF) {
        return CorrelatorNameMod::ALMA_BASELINE_PROTO_OSF;
    }
    	
    if (name == CCorrelatorName::sHERSCHEL) {
        return CorrelatorNameMod::HERSCHEL;
    }
    	
    if (name == CCorrelatorName::sIRAM_PDB) {
        return CorrelatorNameMod::IRAM_PDB;
    }
    	
    if (name == CCorrelatorName::sIRAM_30M_VESPA) {
        return CorrelatorNameMod::IRAM_30M_VESPA;
    }
    	
    if (name == CCorrelatorName::sIRAM_WILMA) {
        return CorrelatorNameMod::IRAM_WILMA;
    }
    
    throw badString(name);
}

CorrelatorNameMod::CorrelatorName CCorrelatorName::from_int(unsigned int i) {
	vector<string> names = sCorrelatorNameSet();
	if (i >= names.size()) throw badInt(i);
	return newCorrelatorName(names.at(i));
}

	

string CCorrelatorName::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'CorrelatorName'.";
}

string CCorrelatorName::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'CorrelatorName'.";
	return oss.str();
}

