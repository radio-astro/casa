
#ifndef CPrimitiveDataType_H
#define CPrimitiveDataType_H

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
 * File CPrimitiveDataType.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace PrimitiveDataTypeMod
{
  enum PrimitiveDataType
  { 
    BOOL_TYPE ,
    BYTE_TYPE ,
    SHORT_TYPE ,
    INT_TYPE ,
    LONGLONG_TYPE ,
    UNSIGNED_SHORT_TYPE ,
    UNSIGNED_INT_TYPE ,
    UNSIGNED_LONGLONG_TYPE ,
    FLOAT_TYPE ,
    DOUBLE_TYPE ,
    STRING_TYPE 
  };
  typedef PrimitiveDataType &PrimitiveDataType_out;
} 
#endif

using namespace std;

class CPrimitiveDataType {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the PrimitiveDataType enumeration.  
	
	static const std::string& sBOOL_TYPE;
	
	static const std::string& sBYTE_TYPE;
	
	static const std::string& sSHORT_TYPE;
	
	static const std::string& sINT_TYPE;
	
	static const std::string& sLONGLONG_TYPE;
	
	static const std::string& sUNSIGNED_SHORT_TYPE;
	
	static const std::string& sUNSIGNED_INT_TYPE;
	
	static const std::string& sUNSIGNED_LONGLONG_TYPE;
	
	static const std::string& sFLOAT_TYPE;
	
	static const std::string& sDOUBLE_TYPE;
	
	static const std::string& sSTRING_TYPE;
	
    static const std::vector<std::string> sPrimitiveDataTypeSet();	 

	

	
	// Explanations associated with the PrimitiveDataType Enumeration.
		
	static const std::string& hBOOL_TYPE;
		
	static const std::string& hBYTE_TYPE;
		
	static const std::string& hSHORT_TYPE;
		
	static const std::string& hINT_TYPE;
		
	static const std::string& hLONGLONG_TYPE;
		
	static const std::string& hUNSIGNED_SHORT_TYPE;
		
	static const std::string& hUNSIGNED_INT_TYPE;
		
	static const std::string& hUNSIGNED_LONGLONG_TYPE;
		
	static const std::string& hFLOAT_TYPE;
		
	static const std::string& hDOUBLE_TYPE;
		
	static const std::string& hSTRING_TYPE;
		
	static const std::vector<std::string> hPrimitiveDataTypeSet();
   	

   	// Is an integer number associated with the PrimitiveDataType enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the PrimitiveDataType enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  PrimitiveDataType enumeration.
	static std::string name(const PrimitiveDataTypeMod::PrimitiveDataType& f);
    static std::string toString(const PrimitiveDataTypeMod::PrimitiveDataType& f) { return name(f); }

	

	
	// Get the help text associated with the specified PrimitiveDataType enumeration.
	static std::string help(const PrimitiveDataTypeMod::PrimitiveDataType& f);
   	
   	
   	// Create a PrimitiveDataType enumeration object by specifying its name.
   	static PrimitiveDataTypeMod::PrimitiveDataType newPrimitiveDataType(const std::string& name);
   	
   	// Create a PrimitiveDataType enumeration object by specifying its name.
 	static PrimitiveDataTypeMod::PrimitiveDataType literal(const std::string& name);
 	
    // Create a PrimitiveDataType enumeration object by specifying its position index (0 based).
 	static PrimitiveDataTypeMod::PrimitiveDataType from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CPrimitiveDataType();
    CPrimitiveDataType(const CPrimitiveDataType&);
    CPrimitiveDataType& operator=(const CPrimitiveDataType&);
};
 
#endif /*!CPrimitiveDataType_H*/
