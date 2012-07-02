
#ifndef CReceiverSideband_H
#define CReceiverSideband_H

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
 * File CReceiverSideband.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace ReceiverSidebandMod
{
  enum ReceiverSideband
  { 
    NOSB ,
    DSB ,
    SSB ,
    TSB 
  };
  typedef ReceiverSideband &ReceiverSideband_out;
} 
#endif

using namespace std;

class CReceiverSideband {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the ReceiverSideband enumeration.  
	
	static const std::string& sNOSB;
	
	static const std::string& sDSB;
	
	static const std::string& sSSB;
	
	static const std::string& sTSB;
	
    static const std::vector<std::string> sReceiverSidebandSet();	 

	

	
	// Explanations associated with the ReceiverSideband Enumeration.
		
	static const std::string& hNOSB;
		
	static const std::string& hDSB;
		
	static const std::string& hSSB;
		
	static const std::string& hTSB;
		
	static const std::vector<std::string> hReceiverSidebandSet();
   	

   	// Is an integer number associated with the ReceiverSideband enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the ReceiverSideband enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  ReceiverSideband enumeration.
	static std::string name(const ReceiverSidebandMod::ReceiverSideband& f);
    static std::string toString(const ReceiverSidebandMod::ReceiverSideband& f) { return name(f); }

	

	
	// Get the help text associated with the specified ReceiverSideband enumeration.
	static std::string help(const ReceiverSidebandMod::ReceiverSideband& f);
   	
   	
   	// Create a ReceiverSideband enumeration object by specifying its name.
   	static ReceiverSidebandMod::ReceiverSideband newReceiverSideband(const std::string& name);
   	
   	// Create a ReceiverSideband enumeration object by specifying its name.
 	static ReceiverSidebandMod::ReceiverSideband literal(const std::string& name);
 	
    // Create a ReceiverSideband enumeration object by specifying its position index (0 based).
 	static ReceiverSidebandMod::ReceiverSideband from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CReceiverSideband();
    CReceiverSideband(const CReceiverSideband&);
    CReceiverSideband& operator=(const CReceiverSideband&);
};
 
#endif /*!CReceiverSideband_H*/
