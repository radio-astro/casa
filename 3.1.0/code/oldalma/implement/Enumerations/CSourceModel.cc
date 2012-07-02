
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
 * File CSourceModel.cpp
 */
#include <sstream>
#include <CSourceModel.h>
#include <string>
using namespace std;

	
const std::string& CSourceModel::sGAUSSIAN = "GAUSSIAN";
	
const std::string& CSourceModel::sPOINT = "POINT";
	
const std::string& CSourceModel::sDISK = "DISK";
	
const std::vector<std::string> CSourceModel::sSourceModelSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSourceModel::sGAUSSIAN);
    
    enumSet.insert(enumSet.end(), CSourceModel::sPOINT);
    
    enumSet.insert(enumSet.end(), CSourceModel::sDISK);
        
    return enumSet;
}

	

	
	
const std::string& CSourceModel::hGAUSSIAN = "Gaussian source";
	
const std::string& CSourceModel::hPOINT = "Point Source";
	
const std::string& CSourceModel::hDISK = "Uniform Disk";
	
const std::vector<std::string> CSourceModel::hSourceModelSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CSourceModel::hGAUSSIAN);
    
    enumSet.insert(enumSet.end(), CSourceModel::hPOINT);
    
    enumSet.insert(enumSet.end(), CSourceModel::hDISK);
        
    return enumSet;
}
   	

std::string CSourceModel::name(const SourceModelMod::SourceModel& f) {
    switch (f) {
    
    case SourceModelMod::GAUSSIAN:
      return CSourceModel::sGAUSSIAN;
    
    case SourceModelMod::POINT:
      return CSourceModel::sPOINT;
    
    case SourceModelMod::DISK:
      return CSourceModel::sDISK;
    	
    }
    return std::string("");
}

	

	
std::string CSourceModel::help(const SourceModelMod::SourceModel& f) {
    switch (f) {
    
    case SourceModelMod::GAUSSIAN:
      return CSourceModel::hGAUSSIAN;
    
    case SourceModelMod::POINT:
      return CSourceModel::hPOINT;
    
    case SourceModelMod::DISK:
      return CSourceModel::hDISK;
    	
    }
    return std::string("");
}
   	

SourceModelMod::SourceModel CSourceModel::newSourceModel(const std::string& name) {
		
    if (name == CSourceModel::sGAUSSIAN) {
        return SourceModelMod::GAUSSIAN;
    }
    	
    if (name == CSourceModel::sPOINT) {
        return SourceModelMod::POINT;
    }
    	
    if (name == CSourceModel::sDISK) {
        return SourceModelMod::DISK;
    }
    
    throw badString(name);
}

SourceModelMod::SourceModel CSourceModel::literal(const std::string& name) {
		
    if (name == CSourceModel::sGAUSSIAN) {
        return SourceModelMod::GAUSSIAN;
    }
    	
    if (name == CSourceModel::sPOINT) {
        return SourceModelMod::POINT;
    }
    	
    if (name == CSourceModel::sDISK) {
        return SourceModelMod::DISK;
    }
    
    throw badString(name);
}

SourceModelMod::SourceModel CSourceModel::from_int(unsigned int i) {
	vector<string> names = sSourceModelSet();
	if (i >= names.size()) throw badInt(i);
	return newSourceModel(names.at(i));
}

	

string CSourceModel::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'SourceModel'.";
}

string CSourceModel::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'SourceModel'.";
	return oss.str();
}

