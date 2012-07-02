
#ifndef CAccumMode_H
#define CAccumMode_H

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
 * File CAccumMode.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace AccumModeMod
{
  enum AccumMode
  { 
    ALMA_FAST ,
    ALMA_NORMAL 
  };
  typedef AccumMode &AccumMode_out;
} 
#endif

using namespace std;

class CAccumMode {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the AccumMode enumeration.  
	
	static const std::string& sALMA_FAST;
	
	static const std::string& sALMA_NORMAL;
	
    static const std::vector<std::string> sAccumModeSet();	 

	

	
	// Explanations associated with the AccumMode Enumeration.
		
	static const std::string& hALMA_FAST;
		
	static const std::string& hALMA_NORMAL;
		
	static const std::vector<std::string> hAccumModeSet();
   	

   	// Is an integer number associated with the AccumMode enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the AccumMode enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  AccumMode enumeration.
	static std::string name(const AccumModeMod::AccumMode& f);
    static std::string toString(const AccumModeMod::AccumMode& f) { return name(f); }

	

	
	// Get the help text associated with the specified AccumMode enumeration.
	static std::string help(const AccumModeMod::AccumMode& f);
   	
   	
   	// Create a AccumMode enumeration object by specifying its name.
   	static AccumModeMod::AccumMode newAccumMode(const std::string& name);
   	
   	// Create a AccumMode enumeration object by specifying its name.
 	static AccumModeMod::AccumMode literal(const std::string& name);
 	
    // Create a AccumMode enumeration object by specifying its position index (0 based).
 	static AccumModeMod::AccumMode from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CAccumMode();
    CAccumMode(const CAccumMode&);
    CAccumMode& operator=(const CAccumMode&);
};
 
#endif /*!CAccumMode_H*/
