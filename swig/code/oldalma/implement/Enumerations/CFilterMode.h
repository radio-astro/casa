
#ifndef CFilterMode_H
#define CFilterMode_H

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
 * File CFilterMode.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace FilterModeMod
{
  enum FilterMode
  { 
    FILTER_NA ,
    FILTER_TDM ,
    FILTER_TFB 
  };
  typedef FilterMode &FilterMode_out;
} 
#endif

using namespace std;

class CFilterMode {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the FilterMode enumeration.  
	
	static const std::string& sFILTER_NA;
	
	static const std::string& sFILTER_TDM;
	
	static const std::string& sFILTER_TFB;
	
    static const std::vector<std::string> sFilterModeSet();	 

	

	
	// Explanations associated with the FilterMode Enumeration.
		
	static const std::string& hFILTER_NA;
		
	static const std::string& hFILTER_TDM;
		
	static const std::string& hFILTER_TFB;
		
	static const std::vector<std::string> hFilterModeSet();
   	

   	// Is an integer number associated with the FilterMode enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the FilterMode enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  FilterMode enumeration.
	static std::string name(const FilterModeMod::FilterMode& f);
    static std::string toString(const FilterModeMod::FilterMode& f) { return name(f); }

	

	
	// Get the help text associated with the specified FilterMode enumeration.
	static std::string help(const FilterModeMod::FilterMode& f);
   	
   	
   	// Create a FilterMode enumeration object by specifying its name.
   	static FilterModeMod::FilterMode newFilterMode(const std::string& name);
   	
   	// Create a FilterMode enumeration object by specifying its name.
 	static FilterModeMod::FilterMode literal(const std::string& name);
 	
    // Create a FilterMode enumeration object by specifying its position index (0 based).
 	static FilterModeMod::FilterMode from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CFilterMode();
    CFilterMode(const CFilterMode&);
    CFilterMode& operator=(const CFilterMode&);
};
 
#endif /*!CFilterMode_H*/
