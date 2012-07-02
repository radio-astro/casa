
#ifndef CSpectralResolutionType_H
#define CSpectralResolutionType_H

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
 * File CSpectralResolutionType.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace SpectralResolutionTypeMod
{
  enum SpectralResolutionType
  { 
    CHANNEL_AVERAGE ,
    BASEBAND_WIDE ,
    FULL_RESOLUTION 
  };
  typedef SpectralResolutionType &SpectralResolutionType_out;
} 
#endif

using namespace std;

class CSpectralResolutionType {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the SpectralResolutionType enumeration.  
	
	static const std::string& sCHANNEL_AVERAGE;
	
	static const std::string& sBASEBAND_WIDE;
	
	static const std::string& sFULL_RESOLUTION;
	
    static const std::vector<std::string> sSpectralResolutionTypeSet();	 

	

	
	// Explanations associated with the SpectralResolutionType Enumeration.
		
	static const std::string& hCHANNEL_AVERAGE;
		
	static const std::string& hBASEBAND_WIDE;
		
	static const std::string& hFULL_RESOLUTION;
		
	static const std::vector<std::string> hSpectralResolutionTypeSet();
   	

   	// Is an integer number associated with the SpectralResolutionType enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the SpectralResolutionType enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  SpectralResolutionType enumeration.
	static std::string name(const SpectralResolutionTypeMod::SpectralResolutionType& f);
    static std::string toString(const SpectralResolutionTypeMod::SpectralResolutionType& f) { return name(f); }

	

	
	// Get the help text associated with the specified SpectralResolutionType enumeration.
	static std::string help(const SpectralResolutionTypeMod::SpectralResolutionType& f);
   	
   	
   	// Create a SpectralResolutionType enumeration object by specifying its name.
   	static SpectralResolutionTypeMod::SpectralResolutionType newSpectralResolutionType(const std::string& name);
   	
   	// Create a SpectralResolutionType enumeration object by specifying its name.
 	static SpectralResolutionTypeMod::SpectralResolutionType literal(const std::string& name);
 	
    // Create a SpectralResolutionType enumeration object by specifying its position index (0 based).
 	static SpectralResolutionTypeMod::SpectralResolutionType from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CSpectralResolutionType();
    CSpectralResolutionType(const CSpectralResolutionType&);
    CSpectralResolutionType& operator=(const CSpectralResolutionType&);
};
 
#endif /*!CSpectralResolutionType_H*/
