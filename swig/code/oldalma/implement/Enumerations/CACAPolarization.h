
#ifndef CACAPolarization_H
#define CACAPolarization_H

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
 * File CACAPolarization.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace ACAPolarizationMod
{
  enum ACAPolarization
  { 
    ACA_STANDARD ,
    ACA_XX_YY_SUM ,
    ACA_XX_50 ,
    ACA_YY_50 
  };
  typedef ACAPolarization &ACAPolarization_out;
} 
#endif

using namespace std;

class CACAPolarization {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the ACAPolarization enumeration.  
	
	static const std::string& sACA_STANDARD;
	
	static const std::string& sACA_XX_YY_SUM;
	
	static const std::string& sACA_XX_50;
	
	static const std::string& sACA_YY_50;
	
    static const std::vector<std::string> sACAPolarizationSet();	 

	

	
	// Explanations associated with the ACAPolarization Enumeration.
		
	static const std::string& hACA_STANDARD;
		
	static const std::string& hACA_XX_YY_SUM;
		
	static const std::string& hACA_XX_50;
		
	static const std::string& hACA_YY_50;
		
	static const std::vector<std::string> hACAPolarizationSet();
   	

   	// Is an integer number associated with the ACAPolarization enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the ACAPolarization enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  ACAPolarization enumeration.
	static std::string name(const ACAPolarizationMod::ACAPolarization& f);
    static std::string toString(const ACAPolarizationMod::ACAPolarization& f) { return name(f); }

	

	
	// Get the help text associated with the specified ACAPolarization enumeration.
	static std::string help(const ACAPolarizationMod::ACAPolarization& f);
   	
   	
   	// Create a ACAPolarization enumeration object by specifying its name.
   	static ACAPolarizationMod::ACAPolarization newACAPolarization(const std::string& name);
   	
   	// Create a ACAPolarization enumeration object by specifying its name.
 	static ACAPolarizationMod::ACAPolarization literal(const std::string& name);
 	
    // Create a ACAPolarization enumeration object by specifying its position index (0 based).
 	static ACAPolarizationMod::ACAPolarization from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CACAPolarization();
    CACAPolarization(const CACAPolarization&);
    CACAPolarization& operator=(const CACAPolarization&);
};
 
#endif /*!CACAPolarization_H*/
