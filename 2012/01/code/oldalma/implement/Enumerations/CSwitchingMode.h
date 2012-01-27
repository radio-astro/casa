
#ifndef CSwitchingMode_H
#define CSwitchingMode_H

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
 * File CSwitchingMode.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace SwitchingModeMod
{
  enum SwitchingMode
  { 
    NO_SWITCHING ,
    LOAD_SWITCHING ,
    POSITION_SWITCHING ,
    PHASE_SWITCHING ,
    FREQUENCY_SWITCHING ,
    NUTATOR_SWITCHING ,
    CHOPPER_WHEEL 
  };
  typedef SwitchingMode &SwitchingMode_out;
} 
#endif

using namespace std;

class CSwitchingMode {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the SwitchingMode enumeration.  
	
	static const std::string& sNO_SWITCHING;
	
	static const std::string& sLOAD_SWITCHING;
	
	static const std::string& sPOSITION_SWITCHING;
	
	static const std::string& sPHASE_SWITCHING;
	
	static const std::string& sFREQUENCY_SWITCHING;
	
	static const std::string& sNUTATOR_SWITCHING;
	
	static const std::string& sCHOPPER_WHEEL;
	
    static const std::vector<std::string> sSwitchingModeSet();	 

	

	
	// Explanations associated with the SwitchingMode Enumeration.
		
	static const std::string& hNO_SWITCHING;
		
	static const std::string& hLOAD_SWITCHING;
		
	static const std::string& hPOSITION_SWITCHING;
		
	static const std::string& hPHASE_SWITCHING;
		
	static const std::string& hFREQUENCY_SWITCHING;
		
	static const std::string& hNUTATOR_SWITCHING;
		
	static const std::string& hCHOPPER_WHEEL;
		
	static const std::vector<std::string> hSwitchingModeSet();
   	

   	// Is an integer number associated with the SwitchingMode enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the SwitchingMode enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  SwitchingMode enumeration.
	static std::string name(const SwitchingModeMod::SwitchingMode& f);
    static std::string toString(const SwitchingModeMod::SwitchingMode& f) { return name(f); }

	

	
	// Get the help text associated with the specified SwitchingMode enumeration.
	static std::string help(const SwitchingModeMod::SwitchingMode& f);
   	
   	
   	// Create a SwitchingMode enumeration object by specifying its name.
   	static SwitchingModeMod::SwitchingMode newSwitchingMode(const std::string& name);
   	
   	// Create a SwitchingMode enumeration object by specifying its name.
 	static SwitchingModeMod::SwitchingMode literal(const std::string& name);
 	
    // Create a SwitchingMode enumeration object by specifying its position index (0 based).
 	static SwitchingModeMod::SwitchingMode from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CSwitchingMode();
    CSwitchingMode(const CSwitchingMode&);
    CSwitchingMode& operator=(const CSwitchingMode&);
};
 
#endif /*!CSwitchingMode_H*/
