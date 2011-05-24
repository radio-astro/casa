
#ifndef CReceiverBand_H
#define CReceiverBand_H

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
 * File CReceiverBand.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace ReceiverBandMod
{
  enum ReceiverBand
  { 
    ALMA_RB_01 ,
    ALMA_RB_02 ,
    ALMA_RB_03 ,
    ALMA_RB_04 ,
    ALMA_RB_05 ,
    ALMA_RB_06 ,
    ALMA_RB_07 ,
    ALMA_RB_08 ,
    ALMA_RB_09 ,
    ALMA_RB_10 ,
    ALMA_HOLOGRAPHY_RECEIVER ,
    BURE_01 ,
    BURE_02 ,
    BURE_03 ,
    BURE_04 ,
    UNSPECIFIED 
  };
  typedef ReceiverBand &ReceiverBand_out;
} 
#endif

using namespace std;

class CReceiverBand {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the ReceiverBand enumeration.  
	
	static const std::string& sALMA_RB_01;
	
	static const std::string& sALMA_RB_02;
	
	static const std::string& sALMA_RB_03;
	
	static const std::string& sALMA_RB_04;
	
	static const std::string& sALMA_RB_05;
	
	static const std::string& sALMA_RB_06;
	
	static const std::string& sALMA_RB_07;
	
	static const std::string& sALMA_RB_08;
	
	static const std::string& sALMA_RB_09;
	
	static const std::string& sALMA_RB_10;
	
	static const std::string& sALMA_HOLOGRAPHY_RECEIVER;
	
	static const std::string& sBURE_01;
	
	static const std::string& sBURE_02;
	
	static const std::string& sBURE_03;
	
	static const std::string& sBURE_04;
	
	static const std::string& sUNSPECIFIED;
	
    static const std::vector<std::string> sReceiverBandSet();	 

	

	
	// Explanations associated with the ReceiverBand Enumeration.
		
	static const std::string& hALMA_RB_01;
		
	static const std::string& hALMA_RB_02;
		
	static const std::string& hALMA_RB_03;
		
	static const std::string& hALMA_RB_04;
		
	static const std::string& hALMA_RB_05;
		
	static const std::string& hALMA_RB_06;
		
	static const std::string& hALMA_RB_07;
		
	static const std::string& hALMA_RB_08;
		
	static const std::string& hALMA_RB_09;
		
	static const std::string& hALMA_RB_10;
		
	static const std::string& hALMA_HOLOGRAPHY_RECEIVER;
		
	static const std::string& hBURE_01;
		
	static const std::string& hBURE_02;
		
	static const std::string& hBURE_03;
		
	static const std::string& hBURE_04;
		
	static const std::string& hUNSPECIFIED;
		
	static const std::vector<std::string> hReceiverBandSet();
   	

   	// Is an integer number associated with the ReceiverBand enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the ReceiverBand enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  ReceiverBand enumeration.
	static std::string name(const ReceiverBandMod::ReceiverBand& f);
    static std::string toString(const ReceiverBandMod::ReceiverBand& f) { return name(f); }

	

	
	// Get the help text associated with the specified ReceiverBand enumeration.
	static std::string help(const ReceiverBandMod::ReceiverBand& f);
   	
   	
   	// Create a ReceiverBand enumeration object by specifying its name.
   	static ReceiverBandMod::ReceiverBand newReceiverBand(const std::string& name);
   	
   	// Create a ReceiverBand enumeration object by specifying its name.
 	static ReceiverBandMod::ReceiverBand literal(const std::string& name);
 	
    // Create a ReceiverBand enumeration object by specifying its position index (0 based).
 	static ReceiverBandMod::ReceiverBand from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CReceiverBand();
    CReceiverBand(const CReceiverBand&);
    CReceiverBand& operator=(const CReceiverBand&);
};
 
#endif /*!CReceiverBand_H*/
