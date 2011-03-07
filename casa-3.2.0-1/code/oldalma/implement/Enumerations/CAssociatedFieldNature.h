
#ifndef CAssociatedFieldNature_H
#define CAssociatedFieldNature_H

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
 * File CAssociatedFieldNature.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace AssociatedFieldNatureMod
{
  enum AssociatedFieldNature
  { 
    ON ,
    OFF ,
    PHASE_REFERENCE 
  };
  typedef AssociatedFieldNature &AssociatedFieldNature_out;
} 
#endif

using namespace std;

class CAssociatedFieldNature {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the AssociatedFieldNature enumeration.  
	
	static const std::string& sON;
	
	static const std::string& sOFF;
	
	static const std::string& sPHASE_REFERENCE;
	
    static const std::vector<std::string> sAssociatedFieldNatureSet();	 

	
	// Integer values associated with the AssociatedFieldNature enumeration.
		
	static const int iON;
		
	static const int iOFF;
		
	static const int iPHASE_REFERENCE;
		
	static const std::vector<int> iAssociatedFieldNatureSet();
   	

	
	// Explanations associated with the AssociatedFieldNature Enumeration.
		
	static const std::string& hON;
		
	static const std::string& hOFF;
		
	static const std::string& hPHASE_REFERENCE;
		
	static const std::vector<std::string> hAssociatedFieldNatureSet();
   	

   	// Is an integer number associated with the AssociatedFieldNature enumeration?
    static bool isNumber() { return true; }
   	
   	// Is a help text associated with the AssociatedFieldNature enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  AssociatedFieldNature enumeration.
	static std::string name(const AssociatedFieldNatureMod::AssociatedFieldNature& f);
    static std::string toString(const AssociatedFieldNatureMod::AssociatedFieldNature& f) { return name(f); }

	
	// Get the integer number associated with the specified AssociatedFieldNature enumeration.
    static int number(const AssociatedFieldNatureMod::AssociatedFieldNature& f);
   	

	
	// Get the help text associated with the specified AssociatedFieldNature enumeration.
	static std::string help(const AssociatedFieldNatureMod::AssociatedFieldNature& f);
   	
   	
   	// Create a AssociatedFieldNature enumeration object by specifying its name.
   	static AssociatedFieldNatureMod::AssociatedFieldNature newAssociatedFieldNature(const std::string& name);
   	
   	// Create a AssociatedFieldNature enumeration object by specifying its name.
 	static AssociatedFieldNatureMod::AssociatedFieldNature literal(const std::string& name);
 	
    // Create a AssociatedFieldNature enumeration object by specifying its position index (0 based).
 	static AssociatedFieldNatureMod::AssociatedFieldNature from_int(unsigned int i);	
 	
	
   	// Create a AssociatedFieldNature enumeration object by specifying its integer value.
    static AssociatedFieldNatureMod::AssociatedFieldNature newAssociatedFieldNature(int number);
    

  private:
    /* Not Implemented.  This is a pure static class. */
    CAssociatedFieldNature();
    CAssociatedFieldNature(const CAssociatedFieldNature&);
    CAssociatedFieldNature& operator=(const CAssociatedFieldNature&);
};
 
#endif /*!CAssociatedFieldNature_H*/
