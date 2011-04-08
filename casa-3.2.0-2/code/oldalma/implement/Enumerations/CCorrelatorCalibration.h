
#ifndef CCorrelatorCalibration_H
#define CCorrelatorCalibration_H

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
 * File CCorrelatorCalibration.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace CorrelatorCalibrationMod
{
  enum CorrelatorCalibration
  { 
    NONE ,
    ACA_OBSERVE_CALIBATOR ,
    ACA_CALIBRATE_CALIBRATOR ,
    ACA_HFSC_REFRESH_CALIBRATOR ,
    ACA_OBSERVE_TARGET ,
    ACA_CALIBATE_TARGET ,
    ACA_HFSC_REFRESH_TARGET ,
    ACA_CORRELATOR_CALIBRATION ,
    ACA_REAL_OBSERVATION ,
    BL_CALC_TFB_SCALING_FACTORS 
  };
  typedef CorrelatorCalibration &CorrelatorCalibration_out;
} 
#endif

using namespace std;

class CCorrelatorCalibration {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the CorrelatorCalibration enumeration.  
	
	static const std::string& sNONE;
	
	static const std::string& sACA_OBSERVE_CALIBATOR;
	
	static const std::string& sACA_CALIBRATE_CALIBRATOR;
	
	static const std::string& sACA_HFSC_REFRESH_CALIBRATOR;
	
	static const std::string& sACA_OBSERVE_TARGET;
	
	static const std::string& sACA_CALIBATE_TARGET;
	
	static const std::string& sACA_HFSC_REFRESH_TARGET;
	
	static const std::string& sACA_CORRELATOR_CALIBRATION;
	
	static const std::string& sACA_REAL_OBSERVATION;
	
	static const std::string& sBL_CALC_TFB_SCALING_FACTORS;
	
    static const std::vector<std::string> sCorrelatorCalibrationSet();	 

	

	
	// Explanations associated with the CorrelatorCalibration Enumeration.
		
	static const std::string& hNONE;
		
	static const std::string& hACA_OBSERVE_CALIBATOR;
		
	static const std::string& hACA_CALIBRATE_CALIBRATOR;
		
	static const std::string& hACA_HFSC_REFRESH_CALIBRATOR;
		
	static const std::string& hACA_OBSERVE_TARGET;
		
	static const std::string& hACA_CALIBATE_TARGET;
		
	static const std::string& hACA_HFSC_REFRESH_TARGET;
		
	static const std::string& hACA_CORRELATOR_CALIBRATION;
		
	static const std::string& hACA_REAL_OBSERVATION;
		
	static const std::string& hBL_CALC_TFB_SCALING_FACTORS;
		
	static const std::vector<std::string> hCorrelatorCalibrationSet();
   	

   	// Is an integer number associated with the CorrelatorCalibration enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the CorrelatorCalibration enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  CorrelatorCalibration enumeration.
	static std::string name(const CorrelatorCalibrationMod::CorrelatorCalibration& f);
    static std::string toString(const CorrelatorCalibrationMod::CorrelatorCalibration& f) { return name(f); }

	

	
	// Get the help text associated with the specified CorrelatorCalibration enumeration.
	static std::string help(const CorrelatorCalibrationMod::CorrelatorCalibration& f);
   	
   	
   	// Create a CorrelatorCalibration enumeration object by specifying its name.
   	static CorrelatorCalibrationMod::CorrelatorCalibration newCorrelatorCalibration(const std::string& name);
   	
   	// Create a CorrelatorCalibration enumeration object by specifying its name.
 	static CorrelatorCalibrationMod::CorrelatorCalibration literal(const std::string& name);
 	
    // Create a CorrelatorCalibration enumeration object by specifying its position index (0 based).
 	static CorrelatorCalibrationMod::CorrelatorCalibration from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CCorrelatorCalibration();
    CCorrelatorCalibration(const CCorrelatorCalibration&);
    CCorrelatorCalibration& operator=(const CCorrelatorCalibration&);
};
 
#endif /*!CCorrelatorCalibration_H*/
