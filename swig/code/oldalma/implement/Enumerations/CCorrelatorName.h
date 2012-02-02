
#ifndef CCorrelatorName_H
#define CCorrelatorName_H

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
 * File CCorrelatorName.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace CorrelatorNameMod
{
  enum CorrelatorName
  { 
    ALMA_ACA ,
    ALMA_BASELINE ,
    ALMA_BASELINE_ATF ,
    ALMA_BASELINE_PROTO_OSF ,
    HERSCHEL ,
    IRAM_PDB ,
    IRAM_30M_VESPA ,
    IRAM_WILMA 
  };
  typedef CorrelatorName &CorrelatorName_out;
} 
#endif

using namespace std;

class CCorrelatorName {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the CorrelatorName enumeration.  
	
	static const std::string& sALMA_ACA;
	
	static const std::string& sALMA_BASELINE;
	
	static const std::string& sALMA_BASELINE_ATF;
	
	static const std::string& sALMA_BASELINE_PROTO_OSF;
	
	static const std::string& sHERSCHEL;
	
	static const std::string& sIRAM_PDB;
	
	static const std::string& sIRAM_30M_VESPA;
	
	static const std::string& sIRAM_WILMA;
	
    static const std::vector<std::string> sCorrelatorNameSet();	 

	

	

   	// Is an integer number associated with the CorrelatorName enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the CorrelatorName enumeration?
    static bool isHelp() { return false; }
    
    // Get the string name associated with the specified  CorrelatorName enumeration.
	static std::string name(const CorrelatorNameMod::CorrelatorName& f);
    static std::string toString(const CorrelatorNameMod::CorrelatorName& f) { return name(f); }

	

	
   	
   	// Create a CorrelatorName enumeration object by specifying its name.
   	static CorrelatorNameMod::CorrelatorName newCorrelatorName(const std::string& name);
   	
   	// Create a CorrelatorName enumeration object by specifying its name.
 	static CorrelatorNameMod::CorrelatorName literal(const std::string& name);
 	
    // Create a CorrelatorName enumeration object by specifying its position index (0 based).
 	static CorrelatorNameMod::CorrelatorName from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CCorrelatorName();
    CCorrelatorName(const CCorrelatorName&);
    CCorrelatorName& operator=(const CCorrelatorName&);
};
 
#endif /*!CCorrelatorName_H*/
