
#ifndef CBasebandName_H
#define CBasebandName_H

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
 * File CBasebandName.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace BasebandNameMod
{
  enum BasebandName
  { 
    BB_1 ,
    BB_2 ,
    BB_3 ,
    BB_4 ,
    BB_5 ,
    BB_6 ,
    BB_7 ,
    BB_8 
  };
  typedef BasebandName &BasebandName_out;
} 
#endif

using namespace std;

class CBasebandName {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the BasebandName enumeration.  
	
	static const std::string& sBB_1;
	
	static const std::string& sBB_2;
	
	static const std::string& sBB_3;
	
	static const std::string& sBB_4;
	
	static const std::string& sBB_5;
	
	static const std::string& sBB_6;
	
	static const std::string& sBB_7;
	
	static const std::string& sBB_8;
	
    static const std::vector<std::string> sBasebandNameSet();	 

	

	
	// Explanations associated with the BasebandName Enumeration.
		
	static const std::string& hBB_1;
		
	static const std::string& hBB_2;
		
	static const std::string& hBB_3;
		
	static const std::string& hBB_4;
		
	static const std::string& hBB_5;
		
	static const std::string& hBB_6;
		
	static const std::string& hBB_7;
		
	static const std::string& hBB_8;
		
	static const std::vector<std::string> hBasebandNameSet();
   	

   	// Is an integer number associated with the BasebandName enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the BasebandName enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  BasebandName enumeration.
	static std::string name(const BasebandNameMod::BasebandName& f);
    static std::string toString(const BasebandNameMod::BasebandName& f) { return name(f); }

	

	
	// Get the help text associated with the specified BasebandName enumeration.
	static std::string help(const BasebandNameMod::BasebandName& f);
   	
   	
   	// Create a BasebandName enumeration object by specifying its name.
   	static BasebandNameMod::BasebandName newBasebandName(const std::string& name);
   	
   	// Create a BasebandName enumeration object by specifying its name.
 	static BasebandNameMod::BasebandName literal(const std::string& name);
 	
    // Create a BasebandName enumeration object by specifying its position index (0 based).
 	static BasebandNameMod::BasebandName from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CBasebandName();
    CBasebandName(const CBasebandName&);
    CBasebandName& operator=(const CBasebandName&);
};
 
#endif /*!CBasebandName_H*/
