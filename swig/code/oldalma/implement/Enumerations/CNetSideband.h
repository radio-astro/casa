
#ifndef CNetSideband_H
#define CNetSideband_H

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
 * File CNetSideband.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace NetSidebandMod
{
  enum NetSideband
  { 
    NOSB ,
    LSB ,
    USB ,
    DSB 
  };
  typedef NetSideband &NetSideband_out;
} 
#endif

using namespace std;

class CNetSideband {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the NetSideband enumeration.  
	
	static const std::string& sNOSB;
	
	static const std::string& sLSB;
	
	static const std::string& sUSB;
	
	static const std::string& sDSB;
	
    static const std::vector<std::string> sNetSidebandSet();	 

	

	
	// Explanations associated with the NetSideband Enumeration.
		
	static const std::string& hNOSB;
		
	static const std::string& hLSB;
		
	static const std::string& hUSB;
		
	static const std::string& hDSB;
		
	static const std::vector<std::string> hNetSidebandSet();
   	

   	// Is an integer number associated with the NetSideband enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the NetSideband enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  NetSideband enumeration.
	static std::string name(const NetSidebandMod::NetSideband& f);
    static std::string toString(const NetSidebandMod::NetSideband& f) { return name(f); }

	

	
	// Get the help text associated with the specified NetSideband enumeration.
	static std::string help(const NetSidebandMod::NetSideband& f);
   	
   	
   	// Create a NetSideband enumeration object by specifying its name.
   	static NetSidebandMod::NetSideband newNetSideband(const std::string& name);
   	
   	// Create a NetSideband enumeration object by specifying its name.
 	static NetSidebandMod::NetSideband literal(const std::string& name);
 	
    // Create a NetSideband enumeration object by specifying its position index (0 based).
 	static NetSidebandMod::NetSideband from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CNetSideband();
    CNetSideband(const CNetSideband&);
    CNetSideband& operator=(const CNetSideband&);
};
 
#endif /*!CNetSideband_H*/
