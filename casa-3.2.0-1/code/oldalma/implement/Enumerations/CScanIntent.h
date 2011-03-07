
#ifndef CScanIntent_H
#define CScanIntent_H

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
 * File CScanIntent.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace ScanIntentMod
{
  enum ScanIntent
  { 
    AMPLI_CAL ,
    AMPLI_CURVE ,
    ANTENNA_POSITIONS ,
    ATMOSPHERE ,
    BANDPASS ,
    DELAY ,
    FOCUS ,
    HOLOGRAPHY ,
    PHASE_CAL ,
    PHASE_CURVE ,
    POINTING ,
    POINTING_MODEL ,
    POLARIZATION ,
    SKYDIP ,
    TARGET ,
    CALIBRATION ,
    LAST ,
    OFFLINE_PROCESSING ,
    FOCUS_X ,
    FOCUS_Y ,
    SIDEBAND_RATIO ,
    OPTICAL_POINTING ,
    WVR_CAL ,
    ASTRO_HOLOGRAPHY ,
    BEAM_MAP ,
    UNSPECIFIED 
  };
  typedef ScanIntent &ScanIntent_out;
} 
#endif

using namespace std;

class CScanIntent {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the ScanIntent enumeration.  
	
	static const std::string& sAMPLI_CAL;
	
	static const std::string& sAMPLI_CURVE;
	
	static const std::string& sANTENNA_POSITIONS;
	
	static const std::string& sATMOSPHERE;
	
	static const std::string& sBANDPASS;
	
	static const std::string& sDELAY;
	
	static const std::string& sFOCUS;
	
	static const std::string& sHOLOGRAPHY;
	
	static const std::string& sPHASE_CAL;
	
	static const std::string& sPHASE_CURVE;
	
	static const std::string& sPOINTING;
	
	static const std::string& sPOINTING_MODEL;
	
	static const std::string& sPOLARIZATION;
	
	static const std::string& sSKYDIP;
	
	static const std::string& sTARGET;
	
	static const std::string& sCALIBRATION;
	
	static const std::string& sLAST;
	
	static const std::string& sOFFLINE_PROCESSING;
	
	static const std::string& sFOCUS_X;
	
	static const std::string& sFOCUS_Y;
	
	static const std::string& sSIDEBAND_RATIO;
	
	static const std::string& sOPTICAL_POINTING;
	
	static const std::string& sWVR_CAL;
	
	static const std::string& sASTRO_HOLOGRAPHY;
	
	static const std::string& sBEAM_MAP;
	
	static const std::string& sUNSPECIFIED;
	
    static const std::vector<std::string> sScanIntentSet();	 

	

	
	// Explanations associated with the ScanIntent Enumeration.
		
	static const std::string& hAMPLI_CAL;
		
	static const std::string& hAMPLI_CURVE;
		
	static const std::string& hANTENNA_POSITIONS;
		
	static const std::string& hATMOSPHERE;
		
	static const std::string& hBANDPASS;
		
	static const std::string& hDELAY;
		
	static const std::string& hFOCUS;
		
	static const std::string& hHOLOGRAPHY;
		
	static const std::string& hPHASE_CAL;
		
	static const std::string& hPHASE_CURVE;
		
	static const std::string& hPOINTING;
		
	static const std::string& hPOINTING_MODEL;
		
	static const std::string& hPOLARIZATION;
		
	static const std::string& hSKYDIP;
		
	static const std::string& hTARGET;
		
	static const std::string& hCALIBRATION;
		
	static const std::string& hLAST;
		
	static const std::string& hOFFLINE_PROCESSING;
		
	static const std::string& hFOCUS_X;
		
	static const std::string& hFOCUS_Y;
		
	static const std::string& hSIDEBAND_RATIO;
		
	static const std::string& hOPTICAL_POINTING;
		
	static const std::string& hWVR_CAL;
		
	static const std::string& hASTRO_HOLOGRAPHY;
		
	static const std::string& hBEAM_MAP;
		
	static const std::string& hUNSPECIFIED;
		
	static const std::vector<std::string> hScanIntentSet();
   	

   	// Is an integer number associated with the ScanIntent enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the ScanIntent enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  ScanIntent enumeration.
	static std::string name(const ScanIntentMod::ScanIntent& f);
    static std::string toString(const ScanIntentMod::ScanIntent& f) { return name(f); }

	

	
	// Get the help text associated with the specified ScanIntent enumeration.
	static std::string help(const ScanIntentMod::ScanIntent& f);
   	
   	
   	// Create a ScanIntent enumeration object by specifying its name.
   	static ScanIntentMod::ScanIntent newScanIntent(const std::string& name);
   	
   	// Create a ScanIntent enumeration object by specifying its name.
 	static ScanIntentMod::ScanIntent literal(const std::string& name);
 	
    // Create a ScanIntent enumeration object by specifying its position index (0 based).
 	static ScanIntentMod::ScanIntent from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CScanIntent();
    CScanIntent(const CScanIntent&);
    CScanIntent& operator=(const CScanIntent&);
};
 
#endif /*!CScanIntent_H*/
