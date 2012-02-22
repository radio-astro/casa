
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
 * File CPrimitiveDataType.cpp
 */
#include <sstream>
#include <CPrimitiveDataType.h>
#include <string>
using namespace std;

	
const std::string& CPrimitiveDataType::sBOOL_TYPE = "BOOL_TYPE";
	
const std::string& CPrimitiveDataType::sBYTE_TYPE = "BYTE_TYPE";
	
const std::string& CPrimitiveDataType::sSHORT_TYPE = "SHORT_TYPE";
	
const std::string& CPrimitiveDataType::sINT_TYPE = "INT_TYPE";
	
const std::string& CPrimitiveDataType::sLONGLONG_TYPE = "LONGLONG_TYPE";
	
const std::string& CPrimitiveDataType::sUNSIGNED_SHORT_TYPE = "UNSIGNED_SHORT_TYPE";
	
const std::string& CPrimitiveDataType::sUNSIGNED_INT_TYPE = "UNSIGNED_INT_TYPE";
	
const std::string& CPrimitiveDataType::sUNSIGNED_LONGLONG_TYPE = "UNSIGNED_LONGLONG_TYPE";
	
const std::string& CPrimitiveDataType::sFLOAT_TYPE = "FLOAT_TYPE";
	
const std::string& CPrimitiveDataType::sDOUBLE_TYPE = "DOUBLE_TYPE";
	
const std::string& CPrimitiveDataType::sSTRING_TYPE = "STRING_TYPE";
	
const std::vector<std::string> CPrimitiveDataType::sPrimitiveDataTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sBOOL_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sBYTE_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sSHORT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sINT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sLONGLONG_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sUNSIGNED_SHORT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sUNSIGNED_INT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sUNSIGNED_LONGLONG_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sFLOAT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sDOUBLE_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::sSTRING_TYPE);
        
    return enumSet;
}

	

	
	
const std::string& CPrimitiveDataType::hBOOL_TYPE = "";
	
const std::string& CPrimitiveDataType::hBYTE_TYPE = "";
	
const std::string& CPrimitiveDataType::hSHORT_TYPE = "";
	
const std::string& CPrimitiveDataType::hINT_TYPE = "";
	
const std::string& CPrimitiveDataType::hLONGLONG_TYPE = "";
	
const std::string& CPrimitiveDataType::hUNSIGNED_SHORT_TYPE = "";
	
const std::string& CPrimitiveDataType::hUNSIGNED_INT_TYPE = "";
	
const std::string& CPrimitiveDataType::hUNSIGNED_LONGLONG_TYPE = "";
	
const std::string& CPrimitiveDataType::hFLOAT_TYPE = "";
	
const std::string& CPrimitiveDataType::hDOUBLE_TYPE = "";
	
const std::string& CPrimitiveDataType::hSTRING_TYPE = "";
	
const std::vector<std::string> CPrimitiveDataType::hPrimitiveDataTypeSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hBOOL_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hBYTE_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hSHORT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hINT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hLONGLONG_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hUNSIGNED_SHORT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hUNSIGNED_INT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hUNSIGNED_LONGLONG_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hFLOAT_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hDOUBLE_TYPE);
    
    enumSet.insert(enumSet.end(), CPrimitiveDataType::hSTRING_TYPE);
        
    return enumSet;
}
   	

std::string CPrimitiveDataType::name(const PrimitiveDataTypeMod::PrimitiveDataType& f) {
    switch (f) {
    
    case PrimitiveDataTypeMod::BOOL_TYPE:
      return CPrimitiveDataType::sBOOL_TYPE;
    
    case PrimitiveDataTypeMod::BYTE_TYPE:
      return CPrimitiveDataType::sBYTE_TYPE;
    
    case PrimitiveDataTypeMod::SHORT_TYPE:
      return CPrimitiveDataType::sSHORT_TYPE;
    
    case PrimitiveDataTypeMod::INT_TYPE:
      return CPrimitiveDataType::sINT_TYPE;
    
    case PrimitiveDataTypeMod::LONGLONG_TYPE:
      return CPrimitiveDataType::sLONGLONG_TYPE;
    
    case PrimitiveDataTypeMod::UNSIGNED_SHORT_TYPE:
      return CPrimitiveDataType::sUNSIGNED_SHORT_TYPE;
    
    case PrimitiveDataTypeMod::UNSIGNED_INT_TYPE:
      return CPrimitiveDataType::sUNSIGNED_INT_TYPE;
    
    case PrimitiveDataTypeMod::UNSIGNED_LONGLONG_TYPE:
      return CPrimitiveDataType::sUNSIGNED_LONGLONG_TYPE;
    
    case PrimitiveDataTypeMod::FLOAT_TYPE:
      return CPrimitiveDataType::sFLOAT_TYPE;
    
    case PrimitiveDataTypeMod::DOUBLE_TYPE:
      return CPrimitiveDataType::sDOUBLE_TYPE;
    
    case PrimitiveDataTypeMod::STRING_TYPE:
      return CPrimitiveDataType::sSTRING_TYPE;
    	
    }
    return std::string("");
}

	

	
std::string CPrimitiveDataType::help(const PrimitiveDataTypeMod::PrimitiveDataType& f) {
    switch (f) {
    
    case PrimitiveDataTypeMod::BOOL_TYPE:
      return CPrimitiveDataType::hBOOL_TYPE;
    
    case PrimitiveDataTypeMod::BYTE_TYPE:
      return CPrimitiveDataType::hBYTE_TYPE;
    
    case PrimitiveDataTypeMod::SHORT_TYPE:
      return CPrimitiveDataType::hSHORT_TYPE;
    
    case PrimitiveDataTypeMod::INT_TYPE:
      return CPrimitiveDataType::hINT_TYPE;
    
    case PrimitiveDataTypeMod::LONGLONG_TYPE:
      return CPrimitiveDataType::hLONGLONG_TYPE;
    
    case PrimitiveDataTypeMod::UNSIGNED_SHORT_TYPE:
      return CPrimitiveDataType::hUNSIGNED_SHORT_TYPE;
    
    case PrimitiveDataTypeMod::UNSIGNED_INT_TYPE:
      return CPrimitiveDataType::hUNSIGNED_INT_TYPE;
    
    case PrimitiveDataTypeMod::UNSIGNED_LONGLONG_TYPE:
      return CPrimitiveDataType::hUNSIGNED_LONGLONG_TYPE;
    
    case PrimitiveDataTypeMod::FLOAT_TYPE:
      return CPrimitiveDataType::hFLOAT_TYPE;
    
    case PrimitiveDataTypeMod::DOUBLE_TYPE:
      return CPrimitiveDataType::hDOUBLE_TYPE;
    
    case PrimitiveDataTypeMod::STRING_TYPE:
      return CPrimitiveDataType::hSTRING_TYPE;
    	
    }
    return std::string("");
}
   	

PrimitiveDataTypeMod::PrimitiveDataType CPrimitiveDataType::newPrimitiveDataType(const std::string& name) {
		
    if (name == CPrimitiveDataType::sBOOL_TYPE) {
        return PrimitiveDataTypeMod::BOOL_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sBYTE_TYPE) {
        return PrimitiveDataTypeMod::BYTE_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sSHORT_TYPE) {
        return PrimitiveDataTypeMod::SHORT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sINT_TYPE) {
        return PrimitiveDataTypeMod::INT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sLONGLONG_TYPE) {
        return PrimitiveDataTypeMod::LONGLONG_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sUNSIGNED_SHORT_TYPE) {
        return PrimitiveDataTypeMod::UNSIGNED_SHORT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sUNSIGNED_INT_TYPE) {
        return PrimitiveDataTypeMod::UNSIGNED_INT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sUNSIGNED_LONGLONG_TYPE) {
        return PrimitiveDataTypeMod::UNSIGNED_LONGLONG_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sFLOAT_TYPE) {
        return PrimitiveDataTypeMod::FLOAT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sDOUBLE_TYPE) {
        return PrimitiveDataTypeMod::DOUBLE_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sSTRING_TYPE) {
        return PrimitiveDataTypeMod::STRING_TYPE;
    }
    
    throw badString(name);
}

PrimitiveDataTypeMod::PrimitiveDataType CPrimitiveDataType::literal(const std::string& name) {
		
    if (name == CPrimitiveDataType::sBOOL_TYPE) {
        return PrimitiveDataTypeMod::BOOL_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sBYTE_TYPE) {
        return PrimitiveDataTypeMod::BYTE_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sSHORT_TYPE) {
        return PrimitiveDataTypeMod::SHORT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sINT_TYPE) {
        return PrimitiveDataTypeMod::INT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sLONGLONG_TYPE) {
        return PrimitiveDataTypeMod::LONGLONG_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sUNSIGNED_SHORT_TYPE) {
        return PrimitiveDataTypeMod::UNSIGNED_SHORT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sUNSIGNED_INT_TYPE) {
        return PrimitiveDataTypeMod::UNSIGNED_INT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sUNSIGNED_LONGLONG_TYPE) {
        return PrimitiveDataTypeMod::UNSIGNED_LONGLONG_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sFLOAT_TYPE) {
        return PrimitiveDataTypeMod::FLOAT_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sDOUBLE_TYPE) {
        return PrimitiveDataTypeMod::DOUBLE_TYPE;
    }
    	
    if (name == CPrimitiveDataType::sSTRING_TYPE) {
        return PrimitiveDataTypeMod::STRING_TYPE;
    }
    
    throw badString(name);
}

PrimitiveDataTypeMod::PrimitiveDataType CPrimitiveDataType::from_int(unsigned int i) {
	vector<string> names = sPrimitiveDataTypeSet();
	if (i >= names.size()) throw badInt(i);
	return newPrimitiveDataType(names.at(i));
}

	

string CPrimitiveDataType::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'PrimitiveDataType'.";
}

string CPrimitiveDataType::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'PrimitiveDataType'.";
	return oss.str();
}

