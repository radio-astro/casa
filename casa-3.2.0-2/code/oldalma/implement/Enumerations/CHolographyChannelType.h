
#ifndef CHolographyChannelType_H
#define CHolographyChannelType_H

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
 * File CHolographyChannelType.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace HolographyChannelTypeMod
{
  enum HolographyChannelType
  { 
    Q2 ,
    QR ,
    QS ,
    R2 ,
    RS ,
    S2 
  };
  typedef HolographyChannelType &HolographyChannelType_out;
} 
#endif

using namespace std;

class CHolographyChannelType {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the HolographyChannelType enumeration.  
	
	static const std::string& sQ2;
	
	static const std::string& sQR;
	
	static const std::string& sQS;
	
	static const std::string& sR2;
	
	static const std::string& sRS;
	
	static const std::string& sS2;
	
    static const std::vector<std::string> sHolographyChannelTypeSet();	 

	

	
	// Explanations associated with the HolographyChannelType Enumeration.
		
	static const std::string& hQ2;
		
	static const std::string& hQR;
		
	static const std::string& hQS;
		
	static const std::string& hR2;
		
	static const std::string& hRS;
		
	static const std::string& hS2;
		
	static const std::vector<std::string> hHolographyChannelTypeSet();
   	

   	// Is an integer number associated with the HolographyChannelType enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the HolographyChannelType enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  HolographyChannelType enumeration.
	static std::string name(const HolographyChannelTypeMod::HolographyChannelType& f);
    static std::string toString(const HolographyChannelTypeMod::HolographyChannelType& f) { return name(f); }

	

	
	// Get the help text associated with the specified HolographyChannelType enumeration.
	static std::string help(const HolographyChannelTypeMod::HolographyChannelType& f);
   	
   	
   	// Create a HolographyChannelType enumeration object by specifying its name.
   	static HolographyChannelTypeMod::HolographyChannelType newHolographyChannelType(const std::string& name);
   	
   	// Create a HolographyChannelType enumeration object by specifying its name.
 	static HolographyChannelTypeMod::HolographyChannelType literal(const std::string& name);
 	
    // Create a HolographyChannelType enumeration object by specifying its position index (0 based).
 	static HolographyChannelTypeMod::HolographyChannelType from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CHolographyChannelType();
    CHolographyChannelType(const CHolographyChannelType&);
    CHolographyChannelType& operator=(const CHolographyChannelType&);
};
 
#endif /*!CHolographyChannelType_H*/
