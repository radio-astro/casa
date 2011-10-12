
#ifndef CProcessorType_H
#define CProcessorType_H

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
 * File CProcessorType.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace ProcessorTypeMod
{
  enum ProcessorType
  { 
    CORRELATOR ,
    RADIOMETER ,
    SPECTROMETER 
  };
  typedef ProcessorType &ProcessorType_out;
} 
#endif

using namespace std;

class CProcessorType {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the ProcessorType enumeration.  
	
	static const std::string& sCORRELATOR;
	
	static const std::string& sRADIOMETER;
	
	static const std::string& sSPECTROMETER;
	
    static const std::vector<std::string> sProcessorTypeSet();	 

	

	
	// Explanations associated with the ProcessorType Enumeration.
		
	static const std::string& hCORRELATOR;
		
	static const std::string& hRADIOMETER;
		
	static const std::string& hSPECTROMETER;
		
	static const std::vector<std::string> hProcessorTypeSet();
   	

   	// Is an integer number associated with the ProcessorType enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the ProcessorType enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  ProcessorType enumeration.
	static std::string name(const ProcessorTypeMod::ProcessorType& f);
    static std::string toString(const ProcessorTypeMod::ProcessorType& f) { return name(f); }

	

	
	// Get the help text associated with the specified ProcessorType enumeration.
	static std::string help(const ProcessorTypeMod::ProcessorType& f);
   	
   	
   	// Create a ProcessorType enumeration object by specifying its name.
   	static ProcessorTypeMod::ProcessorType newProcessorType(const std::string& name);
   	
   	// Create a ProcessorType enumeration object by specifying its name.
 	static ProcessorTypeMod::ProcessorType literal(const std::string& name);
 	
    // Create a ProcessorType enumeration object by specifying its position index (0 based).
 	static ProcessorTypeMod::ProcessorType from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CProcessorType();
    CProcessorType(const CProcessorType&);
    CProcessorType& operator=(const CProcessorType&);
};
 
#endif /*!CProcessorType_H*/
