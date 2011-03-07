
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
 * File CAssociatedFieldNature.cpp
 */
#include <sstream>
#include <CAssociatedFieldNature.h>
#include <string>
using namespace std;

	
const std::string& CAssociatedFieldNature::sON = "ON";
	
const std::string& CAssociatedFieldNature::sOFF = "OFF";
	
const std::string& CAssociatedFieldNature::sPHASE_REFERENCE = "PHASE_REFERENCE";
	
const std::vector<std::string> CAssociatedFieldNature::sAssociatedFieldNatureSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::sON);
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::sOFF);
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::sPHASE_REFERENCE);
        
    return enumSet;
}

	
	
const int CAssociatedFieldNature::iON = 0;
	
const int CAssociatedFieldNature::iOFF = 1;
	
const int CAssociatedFieldNature::iPHASE_REFERENCE = 2;
	
const std::vector<int> CAssociatedFieldNature::iAssociatedFieldNatureSet() {
    std::vector<int> enumSet;
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::iON);
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::iOFF);
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::iPHASE_REFERENCE);
        
    return enumSet;
}
   	

	
	
const std::string& CAssociatedFieldNature::hON = "The associated field is used as ON source data";
	
const std::string& CAssociatedFieldNature::hOFF = "The associated field is used as OFF source data";
	
const std::string& CAssociatedFieldNature::hPHASE_REFERENCE = "The associated field is used as Phase reference data";
	
const std::vector<std::string> CAssociatedFieldNature::hAssociatedFieldNatureSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::hON);
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::hOFF);
    
    enumSet.insert(enumSet.end(), CAssociatedFieldNature::hPHASE_REFERENCE);
        
    return enumSet;
}
   	

std::string CAssociatedFieldNature::name(const AssociatedFieldNatureMod::AssociatedFieldNature& f) {
    switch (f) {
    
    case AssociatedFieldNatureMod::ON:
      return CAssociatedFieldNature::sON;
    
    case AssociatedFieldNatureMod::OFF:
      return CAssociatedFieldNature::sOFF;
    
    case AssociatedFieldNatureMod::PHASE_REFERENCE:
      return CAssociatedFieldNature::sPHASE_REFERENCE;
    	
    }
    return std::string("");
}

	
int CAssociatedFieldNature::number(const AssociatedFieldNatureMod::AssociatedFieldNature& f) {
    switch (f) {
    
    case AssociatedFieldNatureMod::ON:
      return CAssociatedFieldNature::iON;
    
    case AssociatedFieldNatureMod::OFF:
      return CAssociatedFieldNature::iOFF;
    
    case AssociatedFieldNatureMod::PHASE_REFERENCE:
      return CAssociatedFieldNature::iPHASE_REFERENCE;
    	
    }
    return -1;
}
   	

	
std::string CAssociatedFieldNature::help(const AssociatedFieldNatureMod::AssociatedFieldNature& f) {
    switch (f) {
    
    case AssociatedFieldNatureMod::ON:
      return CAssociatedFieldNature::hON;
    
    case AssociatedFieldNatureMod::OFF:
      return CAssociatedFieldNature::hOFF;
    
    case AssociatedFieldNatureMod::PHASE_REFERENCE:
      return CAssociatedFieldNature::hPHASE_REFERENCE;
    	
    }
    return std::string("");
}
   	

AssociatedFieldNatureMod::AssociatedFieldNature CAssociatedFieldNature::newAssociatedFieldNature(const std::string& name) {
		
    if (name == CAssociatedFieldNature::sON) {
        return AssociatedFieldNatureMod::ON;
    }
    	
    if (name == CAssociatedFieldNature::sOFF) {
        return AssociatedFieldNatureMod::OFF;
    }
    	
    if (name == CAssociatedFieldNature::sPHASE_REFERENCE) {
        return AssociatedFieldNatureMod::PHASE_REFERENCE;
    }
    
    throw badString(name);
}

AssociatedFieldNatureMod::AssociatedFieldNature CAssociatedFieldNature::literal(const std::string& name) {
		
    if (name == CAssociatedFieldNature::sON) {
        return AssociatedFieldNatureMod::ON;
    }
    	
    if (name == CAssociatedFieldNature::sOFF) {
        return AssociatedFieldNatureMod::OFF;
    }
    	
    if (name == CAssociatedFieldNature::sPHASE_REFERENCE) {
        return AssociatedFieldNatureMod::PHASE_REFERENCE;
    }
    
    throw badString(name);
}

AssociatedFieldNatureMod::AssociatedFieldNature CAssociatedFieldNature::from_int(unsigned int i) {
	vector<string> names = sAssociatedFieldNatureSet();
	if (i >= names.size()) throw badInt(i);
	return newAssociatedFieldNature(names.at(i));
}

	
AssociatedFieldNatureMod::AssociatedFieldNature CAssociatedFieldNature::newAssociatedFieldNature(int number) {
    switch(number) {
    
    case CAssociatedFieldNature::iON:
        return AssociatedFieldNatureMod::ON;
    
    case CAssociatedFieldNature::iOFF:
        return AssociatedFieldNatureMod::OFF;
    
    case CAssociatedFieldNature::iPHASE_REFERENCE:
        return AssociatedFieldNatureMod::PHASE_REFERENCE;
    
    }
    throw;
}
   	

string CAssociatedFieldNature::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'AssociatedFieldNature'.";
}

string CAssociatedFieldNature::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'AssociatedFieldNature'.";
	return oss.str();
}

