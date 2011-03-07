
#ifndef CPointingMethod_H
#define CPointingMethod_H

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
 * File CPointingMethod.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace PointingMethodMod
{
  enum PointingMethod
  { 
    THREE_POINT ,
    FOUR_POINT ,
    FIVE_POINT ,
    CROSS ,
    CIRCLE 
  };
  typedef PointingMethod &PointingMethod_out;
} 
#endif

using namespace std;

class CPointingMethod {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the PointingMethod enumeration.  
	
	static const std::string& sTHREE_POINT;
	
	static const std::string& sFOUR_POINT;
	
	static const std::string& sFIVE_POINT;
	
	static const std::string& sCROSS;
	
	static const std::string& sCIRCLE;
	
    static const std::vector<std::string> sPointingMethodSet();	 

	

	
	// Explanations associated with the PointingMethod Enumeration.
		
	static const std::string& hTHREE_POINT;
		
	static const std::string& hFOUR_POINT;
		
	static const std::string& hFIVE_POINT;
		
	static const std::string& hCROSS;
		
	static const std::string& hCIRCLE;
		
	static const std::vector<std::string> hPointingMethodSet();
   	

   	// Is an integer number associated with the PointingMethod enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the PointingMethod enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  PointingMethod enumeration.
	static std::string name(const PointingMethodMod::PointingMethod& f);
    static std::string toString(const PointingMethodMod::PointingMethod& f) { return name(f); }

	

	
	// Get the help text associated with the specified PointingMethod enumeration.
	static std::string help(const PointingMethodMod::PointingMethod& f);
   	
   	
   	// Create a PointingMethod enumeration object by specifying its name.
   	static PointingMethodMod::PointingMethod newPointingMethod(const std::string& name);
   	
   	// Create a PointingMethod enumeration object by specifying its name.
 	static PointingMethodMod::PointingMethod literal(const std::string& name);
 	
    // Create a PointingMethod enumeration object by specifying its position index (0 based).
 	static PointingMethodMod::PointingMethod from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CPointingMethod();
    CPointingMethod(const CPointingMethod&);
    CPointingMethod& operator=(const CPointingMethod&);
};
 
#endif /*!CPointingMethod_H*/
