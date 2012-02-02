
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
 * File CCorrelatorCalibration.cpp
 */
#include <sstream>
#include <CCorrelatorCalibration.h>
#include <string>
using namespace std;

	
const std::string& CCorrelatorCalibration::sNONE = "NONE";
	
const std::string& CCorrelatorCalibration::sACA_OBSERVE_CALIBATOR = "ACA_OBSERVE_CALIBATOR";
	
const std::string& CCorrelatorCalibration::sACA_CALIBRATE_CALIBRATOR = "ACA_CALIBRATE_CALIBRATOR";
	
const std::string& CCorrelatorCalibration::sACA_HFSC_REFRESH_CALIBRATOR = "ACA_HFSC_REFRESH_CALIBRATOR";
	
const std::string& CCorrelatorCalibration::sACA_OBSERVE_TARGET = "ACA_OBSERVE_TARGET";
	
const std::string& CCorrelatorCalibration::sACA_CALIBATE_TARGET = "ACA_CALIBATE_TARGET";
	
const std::string& CCorrelatorCalibration::sACA_HFSC_REFRESH_TARGET = "ACA_HFSC_REFRESH_TARGET";
	
const std::string& CCorrelatorCalibration::sACA_CORRELATOR_CALIBRATION = "ACA_CORRELATOR_CALIBRATION";
	
const std::string& CCorrelatorCalibration::sACA_REAL_OBSERVATION = "ACA_REAL_OBSERVATION";
	
const std::string& CCorrelatorCalibration::sBL_CALC_TFB_SCALING_FACTORS = "BL_CALC_TFB_SCALING_FACTORS";
	
const std::vector<std::string> CCorrelatorCalibration::sCorrelatorCalibrationSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sNONE);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sACA_OBSERVE_CALIBATOR);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sACA_CALIBRATE_CALIBRATOR);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sACA_HFSC_REFRESH_CALIBRATOR);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sACA_OBSERVE_TARGET);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sACA_CALIBATE_TARGET);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sACA_HFSC_REFRESH_TARGET);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sACA_CORRELATOR_CALIBRATION);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sACA_REAL_OBSERVATION);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::sBL_CALC_TFB_SCALING_FACTORS);
        
    return enumSet;
}

	

	
	
const std::string& CCorrelatorCalibration::hNONE = "No internal correlator calibration";
	
const std::string& CCorrelatorCalibration::hACA_OBSERVE_CALIBATOR = "Specific ACA Correlator calibration";
	
const std::string& CCorrelatorCalibration::hACA_CALIBRATE_CALIBRATOR = "Specific ACA Correlator calibration";
	
const std::string& CCorrelatorCalibration::hACA_HFSC_REFRESH_CALIBRATOR = "Specific ACA Correlator calibration";
	
const std::string& CCorrelatorCalibration::hACA_OBSERVE_TARGET = "Specific ACA Correlator calibration";
	
const std::string& CCorrelatorCalibration::hACA_CALIBATE_TARGET = "Specific ACA Correlator calibration";
	
const std::string& CCorrelatorCalibration::hACA_HFSC_REFRESH_TARGET = "Specific ACA Correlator calibration";
	
const std::string& CCorrelatorCalibration::hACA_CORRELATOR_CALIBRATION = "Specific ACA Correlator calibration";
	
const std::string& CCorrelatorCalibration::hACA_REAL_OBSERVATION = "Specific ACA Correlator calibration";
	
const std::string& CCorrelatorCalibration::hBL_CALC_TFB_SCALING_FACTORS = "Specific ACA Correlator calibration";
	
const std::vector<std::string> CCorrelatorCalibration::hCorrelatorCalibrationSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hNONE);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hACA_OBSERVE_CALIBATOR);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hACA_CALIBRATE_CALIBRATOR);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hACA_HFSC_REFRESH_CALIBRATOR);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hACA_OBSERVE_TARGET);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hACA_CALIBATE_TARGET);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hACA_HFSC_REFRESH_TARGET);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hACA_CORRELATOR_CALIBRATION);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hACA_REAL_OBSERVATION);
    
    enumSet.insert(enumSet.end(), CCorrelatorCalibration::hBL_CALC_TFB_SCALING_FACTORS);
        
    return enumSet;
}
   	

std::string CCorrelatorCalibration::name(const CorrelatorCalibrationMod::CorrelatorCalibration& f) {
    switch (f) {
    
    case CorrelatorCalibrationMod::NONE:
      return CCorrelatorCalibration::sNONE;
    
    case CorrelatorCalibrationMod::ACA_OBSERVE_CALIBATOR:
      return CCorrelatorCalibration::sACA_OBSERVE_CALIBATOR;
    
    case CorrelatorCalibrationMod::ACA_CALIBRATE_CALIBRATOR:
      return CCorrelatorCalibration::sACA_CALIBRATE_CALIBRATOR;
    
    case CorrelatorCalibrationMod::ACA_HFSC_REFRESH_CALIBRATOR:
      return CCorrelatorCalibration::sACA_HFSC_REFRESH_CALIBRATOR;
    
    case CorrelatorCalibrationMod::ACA_OBSERVE_TARGET:
      return CCorrelatorCalibration::sACA_OBSERVE_TARGET;
    
    case CorrelatorCalibrationMod::ACA_CALIBATE_TARGET:
      return CCorrelatorCalibration::sACA_CALIBATE_TARGET;
    
    case CorrelatorCalibrationMod::ACA_HFSC_REFRESH_TARGET:
      return CCorrelatorCalibration::sACA_HFSC_REFRESH_TARGET;
    
    case CorrelatorCalibrationMod::ACA_CORRELATOR_CALIBRATION:
      return CCorrelatorCalibration::sACA_CORRELATOR_CALIBRATION;
    
    case CorrelatorCalibrationMod::ACA_REAL_OBSERVATION:
      return CCorrelatorCalibration::sACA_REAL_OBSERVATION;
    
    case CorrelatorCalibrationMod::BL_CALC_TFB_SCALING_FACTORS:
      return CCorrelatorCalibration::sBL_CALC_TFB_SCALING_FACTORS;
    	
    }
    return std::string("");
}

	

	
std::string CCorrelatorCalibration::help(const CorrelatorCalibrationMod::CorrelatorCalibration& f) {
    switch (f) {
    
    case CorrelatorCalibrationMod::NONE:
      return CCorrelatorCalibration::hNONE;
    
    case CorrelatorCalibrationMod::ACA_OBSERVE_CALIBATOR:
      return CCorrelatorCalibration::hACA_OBSERVE_CALIBATOR;
    
    case CorrelatorCalibrationMod::ACA_CALIBRATE_CALIBRATOR:
      return CCorrelatorCalibration::hACA_CALIBRATE_CALIBRATOR;
    
    case CorrelatorCalibrationMod::ACA_HFSC_REFRESH_CALIBRATOR:
      return CCorrelatorCalibration::hACA_HFSC_REFRESH_CALIBRATOR;
    
    case CorrelatorCalibrationMod::ACA_OBSERVE_TARGET:
      return CCorrelatorCalibration::hACA_OBSERVE_TARGET;
    
    case CorrelatorCalibrationMod::ACA_CALIBATE_TARGET:
      return CCorrelatorCalibration::hACA_CALIBATE_TARGET;
    
    case CorrelatorCalibrationMod::ACA_HFSC_REFRESH_TARGET:
      return CCorrelatorCalibration::hACA_HFSC_REFRESH_TARGET;
    
    case CorrelatorCalibrationMod::ACA_CORRELATOR_CALIBRATION:
      return CCorrelatorCalibration::hACA_CORRELATOR_CALIBRATION;
    
    case CorrelatorCalibrationMod::ACA_REAL_OBSERVATION:
      return CCorrelatorCalibration::hACA_REAL_OBSERVATION;
    
    case CorrelatorCalibrationMod::BL_CALC_TFB_SCALING_FACTORS:
      return CCorrelatorCalibration::hBL_CALC_TFB_SCALING_FACTORS;
    	
    }
    return std::string("");
}
   	

CorrelatorCalibrationMod::CorrelatorCalibration CCorrelatorCalibration::newCorrelatorCalibration(const std::string& name) {
		
    if (name == CCorrelatorCalibration::sNONE) {
        return CorrelatorCalibrationMod::NONE;
    }
    	
    if (name == CCorrelatorCalibration::sACA_OBSERVE_CALIBATOR) {
        return CorrelatorCalibrationMod::ACA_OBSERVE_CALIBATOR;
    }
    	
    if (name == CCorrelatorCalibration::sACA_CALIBRATE_CALIBRATOR) {
        return CorrelatorCalibrationMod::ACA_CALIBRATE_CALIBRATOR;
    }
    	
    if (name == CCorrelatorCalibration::sACA_HFSC_REFRESH_CALIBRATOR) {
        return CorrelatorCalibrationMod::ACA_HFSC_REFRESH_CALIBRATOR;
    }
    	
    if (name == CCorrelatorCalibration::sACA_OBSERVE_TARGET) {
        return CorrelatorCalibrationMod::ACA_OBSERVE_TARGET;
    }
    	
    if (name == CCorrelatorCalibration::sACA_CALIBATE_TARGET) {
        return CorrelatorCalibrationMod::ACA_CALIBATE_TARGET;
    }
    	
    if (name == CCorrelatorCalibration::sACA_HFSC_REFRESH_TARGET) {
        return CorrelatorCalibrationMod::ACA_HFSC_REFRESH_TARGET;
    }
    	
    if (name == CCorrelatorCalibration::sACA_CORRELATOR_CALIBRATION) {
        return CorrelatorCalibrationMod::ACA_CORRELATOR_CALIBRATION;
    }
    	
    if (name == CCorrelatorCalibration::sACA_REAL_OBSERVATION) {
        return CorrelatorCalibrationMod::ACA_REAL_OBSERVATION;
    }
    	
    if (name == CCorrelatorCalibration::sBL_CALC_TFB_SCALING_FACTORS) {
        return CorrelatorCalibrationMod::BL_CALC_TFB_SCALING_FACTORS;
    }
    
    throw badString(name);
}

CorrelatorCalibrationMod::CorrelatorCalibration CCorrelatorCalibration::literal(const std::string& name) {
		
    if (name == CCorrelatorCalibration::sNONE) {
        return CorrelatorCalibrationMod::NONE;
    }
    	
    if (name == CCorrelatorCalibration::sACA_OBSERVE_CALIBATOR) {
        return CorrelatorCalibrationMod::ACA_OBSERVE_CALIBATOR;
    }
    	
    if (name == CCorrelatorCalibration::sACA_CALIBRATE_CALIBRATOR) {
        return CorrelatorCalibrationMod::ACA_CALIBRATE_CALIBRATOR;
    }
    	
    if (name == CCorrelatorCalibration::sACA_HFSC_REFRESH_CALIBRATOR) {
        return CorrelatorCalibrationMod::ACA_HFSC_REFRESH_CALIBRATOR;
    }
    	
    if (name == CCorrelatorCalibration::sACA_OBSERVE_TARGET) {
        return CorrelatorCalibrationMod::ACA_OBSERVE_TARGET;
    }
    	
    if (name == CCorrelatorCalibration::sACA_CALIBATE_TARGET) {
        return CorrelatorCalibrationMod::ACA_CALIBATE_TARGET;
    }
    	
    if (name == CCorrelatorCalibration::sACA_HFSC_REFRESH_TARGET) {
        return CorrelatorCalibrationMod::ACA_HFSC_REFRESH_TARGET;
    }
    	
    if (name == CCorrelatorCalibration::sACA_CORRELATOR_CALIBRATION) {
        return CorrelatorCalibrationMod::ACA_CORRELATOR_CALIBRATION;
    }
    	
    if (name == CCorrelatorCalibration::sACA_REAL_OBSERVATION) {
        return CorrelatorCalibrationMod::ACA_REAL_OBSERVATION;
    }
    	
    if (name == CCorrelatorCalibration::sBL_CALC_TFB_SCALING_FACTORS) {
        return CorrelatorCalibrationMod::BL_CALC_TFB_SCALING_FACTORS;
    }
    
    throw badString(name);
}

CorrelatorCalibrationMod::CorrelatorCalibration CCorrelatorCalibration::from_int(unsigned int i) {
	vector<string> names = sCorrelatorCalibrationSet();
	if (i >= names.size()) throw badInt(i);
	return newCorrelatorCalibration(names.at(i));
}

	

string CCorrelatorCalibration::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'CorrelatorCalibration'.";
}

string CCorrelatorCalibration::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'CorrelatorCalibration'.";
	return oss.str();
}

