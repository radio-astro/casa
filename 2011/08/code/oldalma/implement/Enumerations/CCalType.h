
#ifndef CCalType_H
#define CCalType_H

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
 * File CCalType.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace CalTypeMod
{
  enum CalType
  { 
    CAL_AMPLI ,
    CAL_ATMOSPHERE ,
    CAL_BANDPASS ,
    CAL_CURVE ,
    CAL_DELAY ,
    CAL_FLUX ,
    CAL_FOCUS ,
    CAL_FOCUS_MODEL ,
    CAL_GAIN ,
    CAL_HOLOGRAPHY ,
    CAL_PHASE ,
    CAL_POINTING ,
    CAL_POINTING_MODEL ,
    CAL_POSITION ,
    CAL_PRIMARY_BEAM ,
    CAL_SEEING ,
    CAL_WVR 
  };
  typedef CalType &CalType_out;
} 
#endif

using namespace std;

class CCalType {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the CalType enumeration.  
	
	static const std::string& sCAL_AMPLI;
	
	static const std::string& sCAL_ATMOSPHERE;
	
	static const std::string& sCAL_BANDPASS;
	
	static const std::string& sCAL_CURVE;
	
	static const std::string& sCAL_DELAY;
	
	static const std::string& sCAL_FLUX;
	
	static const std::string& sCAL_FOCUS;
	
	static const std::string& sCAL_FOCUS_MODEL;
	
	static const std::string& sCAL_GAIN;
	
	static const std::string& sCAL_HOLOGRAPHY;
	
	static const std::string& sCAL_PHASE;
	
	static const std::string& sCAL_POINTING;
	
	static const std::string& sCAL_POINTING_MODEL;
	
	static const std::string& sCAL_POSITION;
	
	static const std::string& sCAL_PRIMARY_BEAM;
	
	static const std::string& sCAL_SEEING;
	
	static const std::string& sCAL_WVR;
	
    static const std::vector<std::string> sCalTypeSet();	 

	

	
	// Explanations associated with the CalType Enumeration.
		
	static const std::string& hCAL_AMPLI;
		
	static const std::string& hCAL_ATMOSPHERE;
		
	static const std::string& hCAL_BANDPASS;
		
	static const std::string& hCAL_CURVE;
		
	static const std::string& hCAL_DELAY;
		
	static const std::string& hCAL_FLUX;
		
	static const std::string& hCAL_FOCUS;
		
	static const std::string& hCAL_FOCUS_MODEL;
		
	static const std::string& hCAL_GAIN;
		
	static const std::string& hCAL_HOLOGRAPHY;
		
	static const std::string& hCAL_PHASE;
		
	static const std::string& hCAL_POINTING;
		
	static const std::string& hCAL_POINTING_MODEL;
		
	static const std::string& hCAL_POSITION;
		
	static const std::string& hCAL_PRIMARY_BEAM;
		
	static const std::string& hCAL_SEEING;
		
	static const std::string& hCAL_WVR;
		
	static const std::vector<std::string> hCalTypeSet();
   	

   	// Is an integer number associated with the CalType enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the CalType enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  CalType enumeration.
	static std::string name(const CalTypeMod::CalType& f);
    static std::string toString(const CalTypeMod::CalType& f) { return name(f); }

	

	
	// Get the help text associated with the specified CalType enumeration.
	static std::string help(const CalTypeMod::CalType& f);
   	
   	
   	// Create a CalType enumeration object by specifying its name.
   	static CalTypeMod::CalType newCalType(const std::string& name);
   	
   	// Create a CalType enumeration object by specifying its name.
 	static CalTypeMod::CalType literal(const std::string& name);
 	
    // Create a CalType enumeration object by specifying its position index (0 based).
 	static CalTypeMod::CalType from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CCalType();
    CCalType(const CCalType&);
    CCalType& operator=(const CCalType&);
};
 
#endif /*!CCalType_H*/
