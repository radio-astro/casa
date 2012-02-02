
#ifndef CWindowFunction_H
#define CWindowFunction_H

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
 * File CWindowFunction.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace WindowFunctionMod
{
  enum WindowFunction
  { 
    UNIFORM ,
    HANNING ,
    HAMMING ,
    BARTLETT ,
    BLACKMANN ,
    BLACKMANN_HARRIS ,
    WELCH 
  };
  typedef WindowFunction &WindowFunction_out;
} 
#endif

using namespace std;

class CWindowFunction {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the WindowFunction enumeration.  
	
	static const std::string& sUNIFORM;
	
	static const std::string& sHANNING;
	
	static const std::string& sHAMMING;
	
	static const std::string& sBARTLETT;
	
	static const std::string& sBLACKMANN;
	
	static const std::string& sBLACKMANN_HARRIS;
	
	static const std::string& sWELCH;
	
    static const std::vector<std::string> sWindowFunctionSet();	 

	

	
	// Explanations associated with the WindowFunction Enumeration.
		
	static const std::string& hUNIFORM;
		
	static const std::string& hHANNING;
		
	static const std::string& hHAMMING;
		
	static const std::string& hBARTLETT;
		
	static const std::string& hBLACKMANN;
		
	static const std::string& hBLACKMANN_HARRIS;
		
	static const std::string& hWELCH;
		
	static const std::vector<std::string> hWindowFunctionSet();
   	

   	// Is an integer number associated with the WindowFunction enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the WindowFunction enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  WindowFunction enumeration.
	static std::string name(const WindowFunctionMod::WindowFunction& f);
    static std::string toString(const WindowFunctionMod::WindowFunction& f) { return name(f); }

	

	
	// Get the help text associated with the specified WindowFunction enumeration.
	static std::string help(const WindowFunctionMod::WindowFunction& f);
   	
   	
   	// Create a WindowFunction enumeration object by specifying its name.
   	static WindowFunctionMod::WindowFunction newWindowFunction(const std::string& name);
   	
   	// Create a WindowFunction enumeration object by specifying its name.
 	static WindowFunctionMod::WindowFunction literal(const std::string& name);
 	
    // Create a WindowFunction enumeration object by specifying its position index (0 based).
 	static WindowFunctionMod::WindowFunction from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CWindowFunction();
    CWindowFunction(const CWindowFunction&);
    CWindowFunction& operator=(const CWindowFunction&);
};
 
#endif /*!CWindowFunction_H*/
