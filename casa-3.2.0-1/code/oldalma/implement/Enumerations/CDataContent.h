
#ifndef CDataContent_H
#define CDataContent_H

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
 * File CDataContent.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace DataContentMod
{
  enum DataContent
  { 
    CROSS_DATA ,
    AUTO_DATA ,
    ZERO_LAGS ,
    ACTUAL_TIMES ,
    ACTUAL_DURATIONS ,
    WEIGHTS ,
    FLAGS 
  };
  typedef DataContent &DataContent_out;
} 
#endif

using namespace std;

class CDataContent {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the DataContent enumeration.  
	
	static const std::string& sCROSS_DATA;
	
	static const std::string& sAUTO_DATA;
	
	static const std::string& sZERO_LAGS;
	
	static const std::string& sACTUAL_TIMES;
	
	static const std::string& sACTUAL_DURATIONS;
	
	static const std::string& sWEIGHTS;
	
	static const std::string& sFLAGS;
	
    static const std::vector<std::string> sDataContentSet();	 

	

	
	// Explanations associated with the DataContent Enumeration.
		
	static const std::string& hCROSS_DATA;
		
	static const std::string& hAUTO_DATA;
		
	static const std::string& hZERO_LAGS;
		
	static const std::string& hACTUAL_TIMES;
		
	static const std::string& hACTUAL_DURATIONS;
		
	static const std::string& hWEIGHTS;
		
	static const std::string& hFLAGS;
		
	static const std::vector<std::string> hDataContentSet();
   	

   	// Is an integer number associated with the DataContent enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the DataContent enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  DataContent enumeration.
	static std::string name(const DataContentMod::DataContent& f);
    static std::string toString(const DataContentMod::DataContent& f) { return name(f); }

	

	
	// Get the help text associated with the specified DataContent enumeration.
	static std::string help(const DataContentMod::DataContent& f);
   	
   	
   	// Create a DataContent enumeration object by specifying its name.
   	static DataContentMod::DataContent newDataContent(const std::string& name);
   	
   	// Create a DataContent enumeration object by specifying its name.
 	static DataContentMod::DataContent literal(const std::string& name);
 	
    // Create a DataContent enumeration object by specifying its position index (0 based).
 	static DataContentMod::DataContent from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CDataContent();
    CDataContent(const CDataContent&);
    CDataContent& operator=(const CDataContent&);
};
 
#endif /*!CDataContent_H*/
