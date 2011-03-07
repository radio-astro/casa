
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
 * File CScanIntent.cpp
 */
#include <sstream>
#include <CScanIntent.h>
#include <string>
using namespace std;

	
const std::string& CScanIntent::sAMPLI_CAL = "AMPLI_CAL";
	
const std::string& CScanIntent::sAMPLI_CURVE = "AMPLI_CURVE";
	
const std::string& CScanIntent::sANTENNA_POSITIONS = "ANTENNA_POSITIONS";
	
const std::string& CScanIntent::sATMOSPHERE = "ATMOSPHERE";
	
const std::string& CScanIntent::sBANDPASS = "BANDPASS";
	
const std::string& CScanIntent::sDELAY = "DELAY";
	
const std::string& CScanIntent::sFOCUS = "FOCUS";
	
const std::string& CScanIntent::sHOLOGRAPHY = "HOLOGRAPHY";
	
const std::string& CScanIntent::sPHASE_CAL = "PHASE_CAL";
	
const std::string& CScanIntent::sPHASE_CURVE = "PHASE_CURVE";
	
const std::string& CScanIntent::sPOINTING = "POINTING";
	
const std::string& CScanIntent::sPOINTING_MODEL = "POINTING_MODEL";
	
const std::string& CScanIntent::sPOLARIZATION = "POLARIZATION";
	
const std::string& CScanIntent::sSKYDIP = "SKYDIP";
	
const std::string& CScanIntent::sTARGET = "TARGET";
	
const std::string& CScanIntent::sCALIBRATION = "CALIBRATION";
	
const std::string& CScanIntent::sLAST = "LAST";
	
const std::string& CScanIntent::sOFFLINE_PROCESSING = "OFFLINE_PROCESSING";
	
const std::string& CScanIntent::sFOCUS_X = "FOCUS_X";
	
const std::string& CScanIntent::sFOCUS_Y = "FOCUS_Y";
	
const std::string& CScanIntent::sSIDEBAND_RATIO = "SIDEBAND_RATIO";
	
const std::string& CScanIntent::sOPTICAL_POINTING = "OPTICAL_POINTING";
	
const std::string& CScanIntent::sWVR_CAL = "WVR_CAL";
	
const std::string& CScanIntent::sASTRO_HOLOGRAPHY = "ASTRO_HOLOGRAPHY";
	
const std::string& CScanIntent::sBEAM_MAP = "BEAM_MAP";
	
const std::string& CScanIntent::sUNSPECIFIED = "UNSPECIFIED";
	
const std::vector<std::string> CScanIntent::sScanIntentSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CScanIntent::sAMPLI_CAL);
    
    enumSet.insert(enumSet.end(), CScanIntent::sAMPLI_CURVE);
    
    enumSet.insert(enumSet.end(), CScanIntent::sANTENNA_POSITIONS);
    
    enumSet.insert(enumSet.end(), CScanIntent::sATMOSPHERE);
    
    enumSet.insert(enumSet.end(), CScanIntent::sBANDPASS);
    
    enumSet.insert(enumSet.end(), CScanIntent::sDELAY);
    
    enumSet.insert(enumSet.end(), CScanIntent::sFOCUS);
    
    enumSet.insert(enumSet.end(), CScanIntent::sHOLOGRAPHY);
    
    enumSet.insert(enumSet.end(), CScanIntent::sPHASE_CAL);
    
    enumSet.insert(enumSet.end(), CScanIntent::sPHASE_CURVE);
    
    enumSet.insert(enumSet.end(), CScanIntent::sPOINTING);
    
    enumSet.insert(enumSet.end(), CScanIntent::sPOINTING_MODEL);
    
    enumSet.insert(enumSet.end(), CScanIntent::sPOLARIZATION);
    
    enumSet.insert(enumSet.end(), CScanIntent::sSKYDIP);
    
    enumSet.insert(enumSet.end(), CScanIntent::sTARGET);
    
    enumSet.insert(enumSet.end(), CScanIntent::sCALIBRATION);
    
    enumSet.insert(enumSet.end(), CScanIntent::sLAST);
    
    enumSet.insert(enumSet.end(), CScanIntent::sOFFLINE_PROCESSING);
    
    enumSet.insert(enumSet.end(), CScanIntent::sFOCUS_X);
    
    enumSet.insert(enumSet.end(), CScanIntent::sFOCUS_Y);
    
    enumSet.insert(enumSet.end(), CScanIntent::sSIDEBAND_RATIO);
    
    enumSet.insert(enumSet.end(), CScanIntent::sOPTICAL_POINTING);
    
    enumSet.insert(enumSet.end(), CScanIntent::sWVR_CAL);
    
    enumSet.insert(enumSet.end(), CScanIntent::sASTRO_HOLOGRAPHY);
    
    enumSet.insert(enumSet.end(), CScanIntent::sBEAM_MAP);
    
    enumSet.insert(enumSet.end(), CScanIntent::sUNSPECIFIED);
        
    return enumSet;
}

	

	
	
const std::string& CScanIntent::hAMPLI_CAL = "Amplitude calibration scan";
	
const std::string& CScanIntent::hAMPLI_CURVE = "Amplitude calibration scan, calibration curve to be derived";
	
const std::string& CScanIntent::hANTENNA_POSITIONS = "Antenna positions measurement";
	
const std::string& CScanIntent::hATMOSPHERE = "Atmosphere calibration scan";
	
const std::string& CScanIntent::hBANDPASS = "Bandpass calibration scan";
	
const std::string& CScanIntent::hDELAY = "Delay calibration scan";
	
const std::string& CScanIntent::hFOCUS = "Focus calibration scan. Z coordinate to be derived";
	
const std::string& CScanIntent::hHOLOGRAPHY = "Holography calibration scan";
	
const std::string& CScanIntent::hPHASE_CAL = "Phase calibration scan";
	
const std::string& CScanIntent::hPHASE_CURVE = "Phase calibration scan; phase calibration curve to be derived";
	
const std::string& CScanIntent::hPOINTING = "Pointing calibration scan";
	
const std::string& CScanIntent::hPOINTING_MODEL = "Pointing calibration scan; pointing model to be derived";
	
const std::string& CScanIntent::hPOLARIZATION = "Polarization calibration scan";
	
const std::string& CScanIntent::hSKYDIP = "Skydip calibration scan";
	
const std::string& CScanIntent::hTARGET = "Target source scan";
	
const std::string& CScanIntent::hCALIBRATION = "Generic calibration scan";
	
const std::string& CScanIntent::hLAST = "Last calibration scan of a group";
	
const std::string& CScanIntent::hOFFLINE_PROCESSING = "Calibration to be processed off-line";
	
const std::string& CScanIntent::hFOCUS_X = "Focus calibration scan; X focus coordinate to be derived";
	
const std::string& CScanIntent::hFOCUS_Y = "Focus calibration scan; Y focus coordinate to be derived";
	
const std::string& CScanIntent::hSIDEBAND_RATIO = "";
	
const std::string& CScanIntent::hOPTICAL_POINTING = "Data from the optical telescope are used to measure poitning offsets.";
	
const std::string& CScanIntent::hWVR_CAL = "Data from the water vapor radiometers (and correlation data) are used to derive their calibration parameters.";
	
const std::string& CScanIntent::hASTRO_HOLOGRAPHY = "Correlation data on a celestial calibration source are used to derive antenna surface parameters.";
	
const std::string& CScanIntent::hBEAM_MAP = "Data on a celestial calibration source are used to derive a map of the primary beam.";
	
const std::string& CScanIntent::hUNSPECIFIED = "Unspecified scan intent";
	
const std::vector<std::string> CScanIntent::hScanIntentSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CScanIntent::hAMPLI_CAL);
    
    enumSet.insert(enumSet.end(), CScanIntent::hAMPLI_CURVE);
    
    enumSet.insert(enumSet.end(), CScanIntent::hANTENNA_POSITIONS);
    
    enumSet.insert(enumSet.end(), CScanIntent::hATMOSPHERE);
    
    enumSet.insert(enumSet.end(), CScanIntent::hBANDPASS);
    
    enumSet.insert(enumSet.end(), CScanIntent::hDELAY);
    
    enumSet.insert(enumSet.end(), CScanIntent::hFOCUS);
    
    enumSet.insert(enumSet.end(), CScanIntent::hHOLOGRAPHY);
    
    enumSet.insert(enumSet.end(), CScanIntent::hPHASE_CAL);
    
    enumSet.insert(enumSet.end(), CScanIntent::hPHASE_CURVE);
    
    enumSet.insert(enumSet.end(), CScanIntent::hPOINTING);
    
    enumSet.insert(enumSet.end(), CScanIntent::hPOINTING_MODEL);
    
    enumSet.insert(enumSet.end(), CScanIntent::hPOLARIZATION);
    
    enumSet.insert(enumSet.end(), CScanIntent::hSKYDIP);
    
    enumSet.insert(enumSet.end(), CScanIntent::hTARGET);
    
    enumSet.insert(enumSet.end(), CScanIntent::hCALIBRATION);
    
    enumSet.insert(enumSet.end(), CScanIntent::hLAST);
    
    enumSet.insert(enumSet.end(), CScanIntent::hOFFLINE_PROCESSING);
    
    enumSet.insert(enumSet.end(), CScanIntent::hFOCUS_X);
    
    enumSet.insert(enumSet.end(), CScanIntent::hFOCUS_Y);
    
    enumSet.insert(enumSet.end(), CScanIntent::hSIDEBAND_RATIO);
    
    enumSet.insert(enumSet.end(), CScanIntent::hOPTICAL_POINTING);
    
    enumSet.insert(enumSet.end(), CScanIntent::hWVR_CAL);
    
    enumSet.insert(enumSet.end(), CScanIntent::hASTRO_HOLOGRAPHY);
    
    enumSet.insert(enumSet.end(), CScanIntent::hBEAM_MAP);
    
    enumSet.insert(enumSet.end(), CScanIntent::hUNSPECIFIED);
        
    return enumSet;
}
   	

std::string CScanIntent::name(const ScanIntentMod::ScanIntent& f) {
    switch (f) {
    
    case ScanIntentMod::AMPLI_CAL:
      return CScanIntent::sAMPLI_CAL;
    
    case ScanIntentMod::AMPLI_CURVE:
      return CScanIntent::sAMPLI_CURVE;
    
    case ScanIntentMod::ANTENNA_POSITIONS:
      return CScanIntent::sANTENNA_POSITIONS;
    
    case ScanIntentMod::ATMOSPHERE:
      return CScanIntent::sATMOSPHERE;
    
    case ScanIntentMod::BANDPASS:
      return CScanIntent::sBANDPASS;
    
    case ScanIntentMod::DELAY:
      return CScanIntent::sDELAY;
    
    case ScanIntentMod::FOCUS:
      return CScanIntent::sFOCUS;
    
    case ScanIntentMod::HOLOGRAPHY:
      return CScanIntent::sHOLOGRAPHY;
    
    case ScanIntentMod::PHASE_CAL:
      return CScanIntent::sPHASE_CAL;
    
    case ScanIntentMod::PHASE_CURVE:
      return CScanIntent::sPHASE_CURVE;
    
    case ScanIntentMod::POINTING:
      return CScanIntent::sPOINTING;
    
    case ScanIntentMod::POINTING_MODEL:
      return CScanIntent::sPOINTING_MODEL;
    
    case ScanIntentMod::POLARIZATION:
      return CScanIntent::sPOLARIZATION;
    
    case ScanIntentMod::SKYDIP:
      return CScanIntent::sSKYDIP;
    
    case ScanIntentMod::TARGET:
      return CScanIntent::sTARGET;
    
    case ScanIntentMod::CALIBRATION:
      return CScanIntent::sCALIBRATION;
    
    case ScanIntentMod::LAST:
      return CScanIntent::sLAST;
    
    case ScanIntentMod::OFFLINE_PROCESSING:
      return CScanIntent::sOFFLINE_PROCESSING;
    
    case ScanIntentMod::FOCUS_X:
      return CScanIntent::sFOCUS_X;
    
    case ScanIntentMod::FOCUS_Y:
      return CScanIntent::sFOCUS_Y;
    
    case ScanIntentMod::SIDEBAND_RATIO:
      return CScanIntent::sSIDEBAND_RATIO;
    
    case ScanIntentMod::OPTICAL_POINTING:
      return CScanIntent::sOPTICAL_POINTING;
    
    case ScanIntentMod::WVR_CAL:
      return CScanIntent::sWVR_CAL;
    
    case ScanIntentMod::ASTRO_HOLOGRAPHY:
      return CScanIntent::sASTRO_HOLOGRAPHY;
    
    case ScanIntentMod::BEAM_MAP:
      return CScanIntent::sBEAM_MAP;
    
    case ScanIntentMod::UNSPECIFIED:
      return CScanIntent::sUNSPECIFIED;
    	
    }
    return std::string("");
}

	

	
std::string CScanIntent::help(const ScanIntentMod::ScanIntent& f) {
    switch (f) {
    
    case ScanIntentMod::AMPLI_CAL:
      return CScanIntent::hAMPLI_CAL;
    
    case ScanIntentMod::AMPLI_CURVE:
      return CScanIntent::hAMPLI_CURVE;
    
    case ScanIntentMod::ANTENNA_POSITIONS:
      return CScanIntent::hANTENNA_POSITIONS;
    
    case ScanIntentMod::ATMOSPHERE:
      return CScanIntent::hATMOSPHERE;
    
    case ScanIntentMod::BANDPASS:
      return CScanIntent::hBANDPASS;
    
    case ScanIntentMod::DELAY:
      return CScanIntent::hDELAY;
    
    case ScanIntentMod::FOCUS:
      return CScanIntent::hFOCUS;
    
    case ScanIntentMod::HOLOGRAPHY:
      return CScanIntent::hHOLOGRAPHY;
    
    case ScanIntentMod::PHASE_CAL:
      return CScanIntent::hPHASE_CAL;
    
    case ScanIntentMod::PHASE_CURVE:
      return CScanIntent::hPHASE_CURVE;
    
    case ScanIntentMod::POINTING:
      return CScanIntent::hPOINTING;
    
    case ScanIntentMod::POINTING_MODEL:
      return CScanIntent::hPOINTING_MODEL;
    
    case ScanIntentMod::POLARIZATION:
      return CScanIntent::hPOLARIZATION;
    
    case ScanIntentMod::SKYDIP:
      return CScanIntent::hSKYDIP;
    
    case ScanIntentMod::TARGET:
      return CScanIntent::hTARGET;
    
    case ScanIntentMod::CALIBRATION:
      return CScanIntent::hCALIBRATION;
    
    case ScanIntentMod::LAST:
      return CScanIntent::hLAST;
    
    case ScanIntentMod::OFFLINE_PROCESSING:
      return CScanIntent::hOFFLINE_PROCESSING;
    
    case ScanIntentMod::FOCUS_X:
      return CScanIntent::hFOCUS_X;
    
    case ScanIntentMod::FOCUS_Y:
      return CScanIntent::hFOCUS_Y;
    
    case ScanIntentMod::SIDEBAND_RATIO:
      return CScanIntent::hSIDEBAND_RATIO;
    
    case ScanIntentMod::OPTICAL_POINTING:
      return CScanIntent::hOPTICAL_POINTING;
    
    case ScanIntentMod::WVR_CAL:
      return CScanIntent::hWVR_CAL;
    
    case ScanIntentMod::ASTRO_HOLOGRAPHY:
      return CScanIntent::hASTRO_HOLOGRAPHY;
    
    case ScanIntentMod::BEAM_MAP:
      return CScanIntent::hBEAM_MAP;
    
    case ScanIntentMod::UNSPECIFIED:
      return CScanIntent::hUNSPECIFIED;
    	
    }
    return std::string("");
}
   	

ScanIntentMod::ScanIntent CScanIntent::newScanIntent(const std::string& name) {
		
    if (name == CScanIntent::sAMPLI_CAL) {
        return ScanIntentMod::AMPLI_CAL;
    }
    	
    if (name == CScanIntent::sAMPLI_CURVE) {
        return ScanIntentMod::AMPLI_CURVE;
    }
    	
    if (name == CScanIntent::sANTENNA_POSITIONS) {
        return ScanIntentMod::ANTENNA_POSITIONS;
    }
    	
    if (name == CScanIntent::sATMOSPHERE) {
        return ScanIntentMod::ATMOSPHERE;
    }
    	
    if (name == CScanIntent::sBANDPASS) {
        return ScanIntentMod::BANDPASS;
    }
    	
    if (name == CScanIntent::sDELAY) {
        return ScanIntentMod::DELAY;
    }
    	
    if (name == CScanIntent::sFOCUS) {
        return ScanIntentMod::FOCUS;
    }
    	
    if (name == CScanIntent::sHOLOGRAPHY) {
        return ScanIntentMod::HOLOGRAPHY;
    }
    	
    if (name == CScanIntent::sPHASE_CAL) {
        return ScanIntentMod::PHASE_CAL;
    }
    	
    if (name == CScanIntent::sPHASE_CURVE) {
        return ScanIntentMod::PHASE_CURVE;
    }
    	
    if (name == CScanIntent::sPOINTING) {
        return ScanIntentMod::POINTING;
    }
    	
    if (name == CScanIntent::sPOINTING_MODEL) {
        return ScanIntentMod::POINTING_MODEL;
    }
    	
    if (name == CScanIntent::sPOLARIZATION) {
        return ScanIntentMod::POLARIZATION;
    }
    	
    if (name == CScanIntent::sSKYDIP) {
        return ScanIntentMod::SKYDIP;
    }
    	
    if (name == CScanIntent::sTARGET) {
        return ScanIntentMod::TARGET;
    }
    	
    if (name == CScanIntent::sCALIBRATION) {
        return ScanIntentMod::CALIBRATION;
    }
    	
    if (name == CScanIntent::sLAST) {
        return ScanIntentMod::LAST;
    }
    	
    if (name == CScanIntent::sOFFLINE_PROCESSING) {
        return ScanIntentMod::OFFLINE_PROCESSING;
    }
    	
    if (name == CScanIntent::sFOCUS_X) {
        return ScanIntentMod::FOCUS_X;
    }
    	
    if (name == CScanIntent::sFOCUS_Y) {
        return ScanIntentMod::FOCUS_Y;
    }
    	
    if (name == CScanIntent::sSIDEBAND_RATIO) {
        return ScanIntentMod::SIDEBAND_RATIO;
    }
    	
    if (name == CScanIntent::sOPTICAL_POINTING) {
        return ScanIntentMod::OPTICAL_POINTING;
    }
    	
    if (name == CScanIntent::sWVR_CAL) {
        return ScanIntentMod::WVR_CAL;
    }
    	
    if (name == CScanIntent::sASTRO_HOLOGRAPHY) {
        return ScanIntentMod::ASTRO_HOLOGRAPHY;
    }
    	
    if (name == CScanIntent::sBEAM_MAP) {
        return ScanIntentMod::BEAM_MAP;
    }
    	
    if (name == CScanIntent::sUNSPECIFIED) {
        return ScanIntentMod::UNSPECIFIED;
    }
    
    throw badString(name);
}

ScanIntentMod::ScanIntent CScanIntent::literal(const std::string& name) {
		
    if (name == CScanIntent::sAMPLI_CAL) {
        return ScanIntentMod::AMPLI_CAL;
    }
    	
    if (name == CScanIntent::sAMPLI_CURVE) {
        return ScanIntentMod::AMPLI_CURVE;
    }
    	
    if (name == CScanIntent::sANTENNA_POSITIONS) {
        return ScanIntentMod::ANTENNA_POSITIONS;
    }
    	
    if (name == CScanIntent::sATMOSPHERE) {
        return ScanIntentMod::ATMOSPHERE;
    }
    	
    if (name == CScanIntent::sBANDPASS) {
        return ScanIntentMod::BANDPASS;
    }
    	
    if (name == CScanIntent::sDELAY) {
        return ScanIntentMod::DELAY;
    }
    	
    if (name == CScanIntent::sFOCUS) {
        return ScanIntentMod::FOCUS;
    }
    	
    if (name == CScanIntent::sHOLOGRAPHY) {
        return ScanIntentMod::HOLOGRAPHY;
    }
    	
    if (name == CScanIntent::sPHASE_CAL) {
        return ScanIntentMod::PHASE_CAL;
    }
    	
    if (name == CScanIntent::sPHASE_CURVE) {
        return ScanIntentMod::PHASE_CURVE;
    }
    	
    if (name == CScanIntent::sPOINTING) {
        return ScanIntentMod::POINTING;
    }
    	
    if (name == CScanIntent::sPOINTING_MODEL) {
        return ScanIntentMod::POINTING_MODEL;
    }
    	
    if (name == CScanIntent::sPOLARIZATION) {
        return ScanIntentMod::POLARIZATION;
    }
    	
    if (name == CScanIntent::sSKYDIP) {
        return ScanIntentMod::SKYDIP;
    }
    	
    if (name == CScanIntent::sTARGET) {
        return ScanIntentMod::TARGET;
    }
    	
    if (name == CScanIntent::sCALIBRATION) {
        return ScanIntentMod::CALIBRATION;
    }
    	
    if (name == CScanIntent::sLAST) {
        return ScanIntentMod::LAST;
    }
    	
    if (name == CScanIntent::sOFFLINE_PROCESSING) {
        return ScanIntentMod::OFFLINE_PROCESSING;
    }
    	
    if (name == CScanIntent::sFOCUS_X) {
        return ScanIntentMod::FOCUS_X;
    }
    	
    if (name == CScanIntent::sFOCUS_Y) {
        return ScanIntentMod::FOCUS_Y;
    }
    	
    if (name == CScanIntent::sSIDEBAND_RATIO) {
        return ScanIntentMod::SIDEBAND_RATIO;
    }
    	
    if (name == CScanIntent::sOPTICAL_POINTING) {
        return ScanIntentMod::OPTICAL_POINTING;
    }
    	
    if (name == CScanIntent::sWVR_CAL) {
        return ScanIntentMod::WVR_CAL;
    }
    	
    if (name == CScanIntent::sASTRO_HOLOGRAPHY) {
        return ScanIntentMod::ASTRO_HOLOGRAPHY;
    }
    	
    if (name == CScanIntent::sBEAM_MAP) {
        return ScanIntentMod::BEAM_MAP;
    }
    	
    if (name == CScanIntent::sUNSPECIFIED) {
        return ScanIntentMod::UNSPECIFIED;
    }
    
    throw badString(name);
}

ScanIntentMod::ScanIntent CScanIntent::from_int(unsigned int i) {
	vector<string> names = sScanIntentSet();
	if (i >= names.size()) throw badInt(i);
	return newScanIntent(names.at(i));
}

	

string CScanIntent::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'ScanIntent'.";
}

string CScanIntent::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'ScanIntent'.";
	return oss.str();
}

