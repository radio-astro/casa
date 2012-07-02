
#ifndef CSyscalMethod_H
#define CSyscalMethod_H

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
 * File CSyscalMethod.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace SyscalMethodMod
{
  enum SyscalMethod
  { 
    TEMPERATURE_SCALE ,
    SKYDIP 
  };
  typedef SyscalMethod &SyscalMethod_out;
} 
#endif

using namespace std;

class CSyscalMethod {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the SyscalMethod enumeration.  
	
	static const std::string& sTEMPERATURE_SCALE;
	
	static const std::string& sSKYDIP;
	
    static const std::vector<std::string> sSyscalMethodSet();	 

	

	
	// Explanations associated with the SyscalMethod Enumeration.
		
	static const std::string& hTEMPERATURE_SCALE;
		
	static const std::string& hSKYDIP;
		
	static const std::vector<std::string> hSyscalMethodSet();
   	

   	// Is an integer number associated with the SyscalMethod enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the SyscalMethod enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  SyscalMethod enumeration.
	static std::string name(const SyscalMethodMod::SyscalMethod& f);
    static std::string toString(const SyscalMethodMod::SyscalMethod& f) { return name(f); }

	

	
	// Get the help text associated with the specified SyscalMethod enumeration.
	static std::string help(const SyscalMethodMod::SyscalMethod& f);
   	
   	
   	// Create a SyscalMethod enumeration object by specifying its name.
   	static SyscalMethodMod::SyscalMethod newSyscalMethod(const std::string& name);
   	
   	// Create a SyscalMethod enumeration object by specifying its name.
 	static SyscalMethodMod::SyscalMethod literal(const std::string& name);
 	
    // Create a SyscalMethod enumeration object by specifying its position index (0 based).
 	static SyscalMethodMod::SyscalMethod from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CSyscalMethod();
    CSyscalMethod(const CSyscalMethod&);
    CSyscalMethod& operator=(const CSyscalMethod&);
};
 
#endif /*!CSyscalMethod_H*/
