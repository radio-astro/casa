
#ifndef CStationType_H
#define CStationType_H

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
 * File CStationType.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace StationTypeMod
{
  enum StationType
  { 
    ANTENNA_PAD ,
    MAINTENANCE_PAD ,
    WEATHER_STATION 
  };
  typedef StationType &StationType_out;
} 
#endif

using namespace std;

class CStationType {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the StationType enumeration.  
	
	static const std::string& sANTENNA_PAD;
	
	static const std::string& sMAINTENANCE_PAD;
	
	static const std::string& sWEATHER_STATION;
	
    static const std::vector<std::string> sStationTypeSet();	 

	

	
	// Explanations associated with the StationType Enumeration.
		
	static const std::string& hANTENNA_PAD;
		
	static const std::string& hMAINTENANCE_PAD;
		
	static const std::string& hWEATHER_STATION;
		
	static const std::vector<std::string> hStationTypeSet();
   	

   	// Is an integer number associated with the StationType enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the StationType enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  StationType enumeration.
	static std::string name(const StationTypeMod::StationType& f);
    static std::string toString(const StationTypeMod::StationType& f) { return name(f); }

	

	
	// Get the help text associated with the specified StationType enumeration.
	static std::string help(const StationTypeMod::StationType& f);
   	
   	
   	// Create a StationType enumeration object by specifying its name.
   	static StationTypeMod::StationType newStationType(const std::string& name);
   	
   	// Create a StationType enumeration object by specifying its name.
 	static StationTypeMod::StationType literal(const std::string& name);
 	
    // Create a StationType enumeration object by specifying its position index (0 based).
 	static StationTypeMod::StationType from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CStationType();
    CStationType(const CStationType&);
    CStationType& operator=(const CStationType&);
};
 
#endif /*!CStationType_H*/
