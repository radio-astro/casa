
#ifndef CAntennaMake_H
#define CAntennaMake_H

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
 * File CAntennaMake.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace AntennaMakeMod
{
  enum AntennaMake
  { 
    AEM_12 ,
    MITSUBISHI_7 ,
    MITSUBISHI_12_A ,
    MITSUBISHI_12_B ,
    VERTEX_12_ATF ,
    AEM_12_ATF ,
    VERTEX_12 ,
    IRAM_15 
  };
  typedef AntennaMake &AntennaMake_out;
} 
#endif

using namespace std;

class CAntennaMake {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the AntennaMake enumeration.  
	
	static const std::string& sAEM_12;
	
	static const std::string& sMITSUBISHI_7;
	
	static const std::string& sMITSUBISHI_12_A;
	
	static const std::string& sMITSUBISHI_12_B;
	
	static const std::string& sVERTEX_12_ATF;
	
	static const std::string& sAEM_12_ATF;
	
	static const std::string& sVERTEX_12;
	
	static const std::string& sIRAM_15;
	
    static const std::vector<std::string> sAntennaMakeSet();	 

	

	
	// Explanations associated with the AntennaMake Enumeration.
		
	static const std::string& hAEM_12;
		
	static const std::string& hMITSUBISHI_7;
		
	static const std::string& hMITSUBISHI_12_A;
		
	static const std::string& hMITSUBISHI_12_B;
		
	static const std::string& hVERTEX_12_ATF;
		
	static const std::string& hAEM_12_ATF;
		
	static const std::string& hVERTEX_12;
		
	static const std::string& hIRAM_15;
		
	static const std::vector<std::string> hAntennaMakeSet();
   	

   	// Is an integer number associated with the AntennaMake enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the AntennaMake enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  AntennaMake enumeration.
	static std::string name(const AntennaMakeMod::AntennaMake& f);
    static std::string toString(const AntennaMakeMod::AntennaMake& f) { return name(f); }

	

	
	// Get the help text associated with the specified AntennaMake enumeration.
	static std::string help(const AntennaMakeMod::AntennaMake& f);
   	
   	
   	// Create a AntennaMake enumeration object by specifying its name.
   	static AntennaMakeMod::AntennaMake newAntennaMake(const std::string& name);
   	
   	// Create a AntennaMake enumeration object by specifying its name.
 	static AntennaMakeMod::AntennaMake literal(const std::string& name);
 	
    // Create a AntennaMake enumeration object by specifying its position index (0 based).
 	static AntennaMakeMod::AntennaMake from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CAntennaMake();
    CAntennaMake(const CAntennaMake&);
    CAntennaMake& operator=(const CAntennaMake&);
};
 
#endif /*!CAntennaMake_H*/
