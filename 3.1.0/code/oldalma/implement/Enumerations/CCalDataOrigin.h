
#ifndef CCalDataOrigin_H
#define CCalDataOrigin_H

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
 * File CCalDataOrigin.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace CalDataOriginMod
{
  enum CalDataOrigin
  { 
    TOTAL_POWER ,
    WVR ,
    CHANNEL_AVERAGE_AUTO ,
    CHANNEL_AVERAGE_CROSS ,
    FULL_RESOLUTION_AUTO ,
    FULL_RESOLUTION_CROSS ,
    OPTICAL_POINTING ,
    HOLOGRAPHY 
  };
  typedef CalDataOrigin &CalDataOrigin_out;
} 
#endif

using namespace std;

class CCalDataOrigin {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the CalDataOrigin enumeration.  
	
	static const std::string& sTOTAL_POWER;
	
	static const std::string& sWVR;
	
	static const std::string& sCHANNEL_AVERAGE_AUTO;
	
	static const std::string& sCHANNEL_AVERAGE_CROSS;
	
	static const std::string& sFULL_RESOLUTION_AUTO;
	
	static const std::string& sFULL_RESOLUTION_CROSS;
	
	static const std::string& sOPTICAL_POINTING;
	
	static const std::string& sHOLOGRAPHY;
	
    static const std::vector<std::string> sCalDataOriginSet();	 

	

	
	// Explanations associated with the CalDataOrigin Enumeration.
		
	static const std::string& hTOTAL_POWER;
		
	static const std::string& hWVR;
		
	static const std::string& hCHANNEL_AVERAGE_AUTO;
		
	static const std::string& hCHANNEL_AVERAGE_CROSS;
		
	static const std::string& hFULL_RESOLUTION_AUTO;
		
	static const std::string& hFULL_RESOLUTION_CROSS;
		
	static const std::string& hOPTICAL_POINTING;
		
	static const std::string& hHOLOGRAPHY;
		
	static const std::vector<std::string> hCalDataOriginSet();
   	

   	// Is an integer number associated with the CalDataOrigin enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the CalDataOrigin enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  CalDataOrigin enumeration.
	static std::string name(const CalDataOriginMod::CalDataOrigin& f);
    static std::string toString(const CalDataOriginMod::CalDataOrigin& f) { return name(f); }

	

	
	// Get the help text associated with the specified CalDataOrigin enumeration.
	static std::string help(const CalDataOriginMod::CalDataOrigin& f);
   	
   	
   	// Create a CalDataOrigin enumeration object by specifying its name.
   	static CalDataOriginMod::CalDataOrigin newCalDataOrigin(const std::string& name);
   	
   	// Create a CalDataOrigin enumeration object by specifying its name.
 	static CalDataOriginMod::CalDataOrigin literal(const std::string& name);
 	
    // Create a CalDataOrigin enumeration object by specifying its position index (0 based).
 	static CalDataOriginMod::CalDataOrigin from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CCalDataOrigin();
    CCalDataOrigin(const CCalDataOrigin&);
    CCalDataOrigin& operator=(const CCalDataOrigin&);
};
 
#endif /*!CCalDataOrigin_H*/
