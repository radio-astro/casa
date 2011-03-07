
#ifndef CSubscanIntent_H
#define CSubscanIntent_H

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
 * File CSubscanIntent.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace SubscanIntentMod
{
  enum SubscanIntent
  { 
    HOLOGRAPHY_RASTER ,
    HOLOGRAPHY_PHASECAL ,
    UNSPECIFIED ,
    IN_FOCUS ,
    OUT_OF_FOCUS ,
    ON_SOURCE ,
    OFF_SOURCE ,
    MIXED_FOCUS ,
    MIXED_POINTING ,
    REFERENCE ,
    RASTER 
  };
  typedef SubscanIntent &SubscanIntent_out;
} 
#endif

using namespace std;

class CSubscanIntent {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the SubscanIntent enumeration.  
	
	static const std::string& sHOLOGRAPHY_RASTER;
	
	static const std::string& sHOLOGRAPHY_PHASECAL;
	
	static const std::string& sUNSPECIFIED;
	
	static const std::string& sIN_FOCUS;
	
	static const std::string& sOUT_OF_FOCUS;
	
	static const std::string& sON_SOURCE;
	
	static const std::string& sOFF_SOURCE;
	
	static const std::string& sMIXED_FOCUS;
	
	static const std::string& sMIXED_POINTING;
	
	static const std::string& sREFERENCE;
	
	static const std::string& sRASTER;
	
    static const std::vector<std::string> sSubscanIntentSet();	 

	

	
	// Explanations associated with the SubscanIntent Enumeration.
		
	static const std::string& hHOLOGRAPHY_RASTER;
		
	static const std::string& hHOLOGRAPHY_PHASECAL;
		
	static const std::string& hUNSPECIFIED;
		
	static const std::string& hIN_FOCUS;
		
	static const std::string& hOUT_OF_FOCUS;
		
	static const std::string& hON_SOURCE;
		
	static const std::string& hOFF_SOURCE;
		
	static const std::string& hMIXED_FOCUS;
		
	static const std::string& hMIXED_POINTING;
		
	static const std::string& hREFERENCE;
		
	static const std::string& hRASTER;
		
	static const std::vector<std::string> hSubscanIntentSet();
   	

   	// Is an integer number associated with the SubscanIntent enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the SubscanIntent enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  SubscanIntent enumeration.
	static std::string name(const SubscanIntentMod::SubscanIntent& f);
    static std::string toString(const SubscanIntentMod::SubscanIntent& f) { return name(f); }

	

	
	// Get the help text associated with the specified SubscanIntent enumeration.
	static std::string help(const SubscanIntentMod::SubscanIntent& f);
   	
   	
   	// Create a SubscanIntent enumeration object by specifying its name.
   	static SubscanIntentMod::SubscanIntent newSubscanIntent(const std::string& name);
   	
   	// Create a SubscanIntent enumeration object by specifying its name.
 	static SubscanIntentMod::SubscanIntent literal(const std::string& name);
 	
    // Create a SubscanIntent enumeration object by specifying its position index (0 based).
 	static SubscanIntentMod::SubscanIntent from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CSubscanIntent();
    CSubscanIntent(const CSubscanIntent&);
    CSubscanIntent& operator=(const CSubscanIntent&);
};
 
#endif /*!CSubscanIntent_H*/
