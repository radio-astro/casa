
#ifndef CInvalidatingCondition_H
#define CInvalidatingCondition_H

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
 * File CInvalidatingCondition.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace InvalidatingConditionMod
{
  enum InvalidatingCondition
  { 
    ANTENNA_DISCONNECT ,
    ANTENNA_MOVE ,
    ANTENNA_POWER_DOWN ,
    RECEIVER_EXCHANGE ,
    RECEIVER_POWER_DOWN 
  };
  typedef InvalidatingCondition &InvalidatingCondition_out;
} 
#endif

using namespace std;

class CInvalidatingCondition {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the InvalidatingCondition enumeration.  
	
	static const std::string& sANTENNA_DISCONNECT;
	
	static const std::string& sANTENNA_MOVE;
	
	static const std::string& sANTENNA_POWER_DOWN;
	
	static const std::string& sRECEIVER_EXCHANGE;
	
	static const std::string& sRECEIVER_POWER_DOWN;
	
    static const std::vector<std::string> sInvalidatingConditionSet();	 

	

	
	// Explanations associated with the InvalidatingCondition Enumeration.
		
	static const std::string& hANTENNA_DISCONNECT;
		
	static const std::string& hANTENNA_MOVE;
		
	static const std::string& hANTENNA_POWER_DOWN;
		
	static const std::string& hRECEIVER_EXCHANGE;
		
	static const std::string& hRECEIVER_POWER_DOWN;
		
	static const std::vector<std::string> hInvalidatingConditionSet();
   	

   	// Is an integer number associated with the InvalidatingCondition enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the InvalidatingCondition enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  InvalidatingCondition enumeration.
	static std::string name(const InvalidatingConditionMod::InvalidatingCondition& f);
    static std::string toString(const InvalidatingConditionMod::InvalidatingCondition& f) { return name(f); }

	

	
	// Get the help text associated with the specified InvalidatingCondition enumeration.
	static std::string help(const InvalidatingConditionMod::InvalidatingCondition& f);
   	
   	
   	// Create a InvalidatingCondition enumeration object by specifying its name.
   	static InvalidatingConditionMod::InvalidatingCondition newInvalidatingCondition(const std::string& name);
   	
   	// Create a InvalidatingCondition enumeration object by specifying its name.
 	static InvalidatingConditionMod::InvalidatingCondition literal(const std::string& name);
 	
    // Create a InvalidatingCondition enumeration object by specifying its position index (0 based).
 	static InvalidatingConditionMod::InvalidatingCondition from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CInvalidatingCondition();
    CInvalidatingCondition(const CInvalidatingCondition&);
    CInvalidatingCondition& operator=(const CInvalidatingCondition&);
};
 
#endif /*!CInvalidatingCondition_H*/
