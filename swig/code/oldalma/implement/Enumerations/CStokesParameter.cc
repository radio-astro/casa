
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
 * File CStokesParameter.cpp
 */
#include <sstream>
#include <CStokesParameter.h>
#include <string>
using namespace std;

	
const std::string& CStokesParameter::sI = "I";
	
const std::string& CStokesParameter::sQ = "Q";
	
const std::string& CStokesParameter::sU = "U";
	
const std::string& CStokesParameter::sV = "V";
	
const std::string& CStokesParameter::sRR = "RR";
	
const std::string& CStokesParameter::sRL = "RL";
	
const std::string& CStokesParameter::sLR = "LR";
	
const std::string& CStokesParameter::sLL = "LL";
	
const std::string& CStokesParameter::sXX = "XX";
	
const std::string& CStokesParameter::sXY = "XY";
	
const std::string& CStokesParameter::sYX = "YX";
	
const std::string& CStokesParameter::sYY = "YY";
	
const std::string& CStokesParameter::sRX = "RX";
	
const std::string& CStokesParameter::sRY = "RY";
	
const std::string& CStokesParameter::sLX = "LX";
	
const std::string& CStokesParameter::sLY = "LY";
	
const std::string& CStokesParameter::sXR = "XR";
	
const std::string& CStokesParameter::sXL = "XL";
	
const std::string& CStokesParameter::sYR = "YR";
	
const std::string& CStokesParameter::sYL = "YL";
	
const std::string& CStokesParameter::sPP = "PP";
	
const std::string& CStokesParameter::sPQ = "PQ";
	
const std::string& CStokesParameter::sQP = "QP";
	
const std::string& CStokesParameter::sQQ = "QQ";
	
const std::string& CStokesParameter::sRCIRCULAR = "RCIRCULAR";
	
const std::string& CStokesParameter::sLCIRCULAR = "LCIRCULAR";
	
const std::string& CStokesParameter::sLINEAR = "LINEAR";
	
const std::string& CStokesParameter::sPTOTAL = "PTOTAL";
	
const std::string& CStokesParameter::sPLINEAR = "PLINEAR";
	
const std::string& CStokesParameter::sPFTOTAL = "PFTOTAL";
	
const std::string& CStokesParameter::sPFLINEAR = "PFLINEAR";
	
const std::string& CStokesParameter::sPANGLE = "PANGLE";
	
const std::vector<std::string> CStokesParameter::sStokesParameterSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CStokesParameter::sI);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sQ);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sU);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sV);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sRR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sRL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sLR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sLL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sXX);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sXY);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sYX);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sYY);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sRX);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sRY);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sLX);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sLY);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sXR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sXL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sYR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sYL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sPP);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sPQ);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sQP);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sQQ);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sRCIRCULAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sLCIRCULAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sLINEAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sPTOTAL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sPLINEAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sPFTOTAL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sPFLINEAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::sPANGLE);
        
    return enumSet;
}

	

	
	
const std::string& CStokesParameter::hI = "";
	
const std::string& CStokesParameter::hQ = "";
	
const std::string& CStokesParameter::hU = "";
	
const std::string& CStokesParameter::hV = "";
	
const std::string& CStokesParameter::hRR = "";
	
const std::string& CStokesParameter::hRL = "";
	
const std::string& CStokesParameter::hLR = "";
	
const std::string& CStokesParameter::hLL = "";
	
const std::string& CStokesParameter::hXX = "Linear correlation product";
	
const std::string& CStokesParameter::hXY = "";
	
const std::string& CStokesParameter::hYX = "";
	
const std::string& CStokesParameter::hYY = "";
	
const std::string& CStokesParameter::hRX = "Mixed correlation product";
	
const std::string& CStokesParameter::hRY = "Mixed correlation product";
	
const std::string& CStokesParameter::hLX = "Mixed LX product";
	
const std::string& CStokesParameter::hLY = "Mixed LY correlation product";
	
const std::string& CStokesParameter::hXR = "Mixed XR correlation product";
	
const std::string& CStokesParameter::hXL = "Mixed XL correlation product";
	
const std::string& CStokesParameter::hYR = "Mixed YR correlation product";
	
const std::string& CStokesParameter::hYL = "Mixel YL correlation product";
	
const std::string& CStokesParameter::hPP = "";
	
const std::string& CStokesParameter::hPQ = "";
	
const std::string& CStokesParameter::hQP = "";
	
const std::string& CStokesParameter::hQQ = "";
	
const std::string& CStokesParameter::hRCIRCULAR = "";
	
const std::string& CStokesParameter::hLCIRCULAR = "";
	
const std::string& CStokesParameter::hLINEAR = "single dish polarization type";
	
const std::string& CStokesParameter::hPTOTAL = "Polarized intensity ((Q^2+U^2+V^2)^(1/2))";
	
const std::string& CStokesParameter::hPLINEAR = "Linearly Polarized intensity ((Q^2+U^2)^(1/2))";
	
const std::string& CStokesParameter::hPFTOTAL = "Polarization Fraction (Ptotal/I)";
	
const std::string& CStokesParameter::hPFLINEAR = "Linear Polarization Fraction (Plinear/I)";
	
const std::string& CStokesParameter::hPANGLE = "Linear Polarization Angle (0.5 arctan(U/Q)) (in radians)";
	
const std::vector<std::string> CStokesParameter::hStokesParameterSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CStokesParameter::hI);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hQ);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hU);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hV);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hRR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hRL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hLR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hLL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hXX);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hXY);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hYX);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hYY);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hRX);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hRY);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hLX);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hLY);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hXR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hXL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hYR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hYL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hPP);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hPQ);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hQP);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hQQ);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hRCIRCULAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hLCIRCULAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hLINEAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hPTOTAL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hPLINEAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hPFTOTAL);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hPFLINEAR);
    
    enumSet.insert(enumSet.end(), CStokesParameter::hPANGLE);
        
    return enumSet;
}
   	

std::string CStokesParameter::name(const StokesParameterMod::StokesParameter& f) {
    switch (f) {
    
    case StokesParameterMod::I:
      return CStokesParameter::sI;
    
    case StokesParameterMod::Q:
      return CStokesParameter::sQ;
    
    case StokesParameterMod::U:
      return CStokesParameter::sU;
    
    case StokesParameterMod::V:
      return CStokesParameter::sV;
    
    case StokesParameterMod::RR:
      return CStokesParameter::sRR;
    
    case StokesParameterMod::RL:
      return CStokesParameter::sRL;
    
    case StokesParameterMod::LR:
      return CStokesParameter::sLR;
    
    case StokesParameterMod::LL:
      return CStokesParameter::sLL;
    
    case StokesParameterMod::XX:
      return CStokesParameter::sXX;
    
    case StokesParameterMod::XY:
      return CStokesParameter::sXY;
    
    case StokesParameterMod::YX:
      return CStokesParameter::sYX;
    
    case StokesParameterMod::YY:
      return CStokesParameter::sYY;
    
    case StokesParameterMod::RX:
      return CStokesParameter::sRX;
    
    case StokesParameterMod::RY:
      return CStokesParameter::sRY;
    
    case StokesParameterMod::LX:
      return CStokesParameter::sLX;
    
    case StokesParameterMod::LY:
      return CStokesParameter::sLY;
    
    case StokesParameterMod::XR:
      return CStokesParameter::sXR;
    
    case StokesParameterMod::XL:
      return CStokesParameter::sXL;
    
    case StokesParameterMod::YR:
      return CStokesParameter::sYR;
    
    case StokesParameterMod::YL:
      return CStokesParameter::sYL;
    
    case StokesParameterMod::PP:
      return CStokesParameter::sPP;
    
    case StokesParameterMod::PQ:
      return CStokesParameter::sPQ;
    
    case StokesParameterMod::QP:
      return CStokesParameter::sQP;
    
    case StokesParameterMod::QQ:
      return CStokesParameter::sQQ;
    
    case StokesParameterMod::RCIRCULAR:
      return CStokesParameter::sRCIRCULAR;
    
    case StokesParameterMod::LCIRCULAR:
      return CStokesParameter::sLCIRCULAR;
    
    case StokesParameterMod::LINEAR:
      return CStokesParameter::sLINEAR;
    
    case StokesParameterMod::PTOTAL:
      return CStokesParameter::sPTOTAL;
    
    case StokesParameterMod::PLINEAR:
      return CStokesParameter::sPLINEAR;
    
    case StokesParameterMod::PFTOTAL:
      return CStokesParameter::sPFTOTAL;
    
    case StokesParameterMod::PFLINEAR:
      return CStokesParameter::sPFLINEAR;
    
    case StokesParameterMod::PANGLE:
      return CStokesParameter::sPANGLE;
    	
    }
    return std::string("");
}

	

	
std::string CStokesParameter::help(const StokesParameterMod::StokesParameter& f) {
    switch (f) {
    
    case StokesParameterMod::I:
      return CStokesParameter::hI;
    
    case StokesParameterMod::Q:
      return CStokesParameter::hQ;
    
    case StokesParameterMod::U:
      return CStokesParameter::hU;
    
    case StokesParameterMod::V:
      return CStokesParameter::hV;
    
    case StokesParameterMod::RR:
      return CStokesParameter::hRR;
    
    case StokesParameterMod::RL:
      return CStokesParameter::hRL;
    
    case StokesParameterMod::LR:
      return CStokesParameter::hLR;
    
    case StokesParameterMod::LL:
      return CStokesParameter::hLL;
    
    case StokesParameterMod::XX:
      return CStokesParameter::hXX;
    
    case StokesParameterMod::XY:
      return CStokesParameter::hXY;
    
    case StokesParameterMod::YX:
      return CStokesParameter::hYX;
    
    case StokesParameterMod::YY:
      return CStokesParameter::hYY;
    
    case StokesParameterMod::RX:
      return CStokesParameter::hRX;
    
    case StokesParameterMod::RY:
      return CStokesParameter::hRY;
    
    case StokesParameterMod::LX:
      return CStokesParameter::hLX;
    
    case StokesParameterMod::LY:
      return CStokesParameter::hLY;
    
    case StokesParameterMod::XR:
      return CStokesParameter::hXR;
    
    case StokesParameterMod::XL:
      return CStokesParameter::hXL;
    
    case StokesParameterMod::YR:
      return CStokesParameter::hYR;
    
    case StokesParameterMod::YL:
      return CStokesParameter::hYL;
    
    case StokesParameterMod::PP:
      return CStokesParameter::hPP;
    
    case StokesParameterMod::PQ:
      return CStokesParameter::hPQ;
    
    case StokesParameterMod::QP:
      return CStokesParameter::hQP;
    
    case StokesParameterMod::QQ:
      return CStokesParameter::hQQ;
    
    case StokesParameterMod::RCIRCULAR:
      return CStokesParameter::hRCIRCULAR;
    
    case StokesParameterMod::LCIRCULAR:
      return CStokesParameter::hLCIRCULAR;
    
    case StokesParameterMod::LINEAR:
      return CStokesParameter::hLINEAR;
    
    case StokesParameterMod::PTOTAL:
      return CStokesParameter::hPTOTAL;
    
    case StokesParameterMod::PLINEAR:
      return CStokesParameter::hPLINEAR;
    
    case StokesParameterMod::PFTOTAL:
      return CStokesParameter::hPFTOTAL;
    
    case StokesParameterMod::PFLINEAR:
      return CStokesParameter::hPFLINEAR;
    
    case StokesParameterMod::PANGLE:
      return CStokesParameter::hPANGLE;
    	
    }
    return std::string("");
}
   	

StokesParameterMod::StokesParameter CStokesParameter::newStokesParameter(const std::string& name) {
		
    if (name == CStokesParameter::sI) {
        return StokesParameterMod::I;
    }
    	
    if (name == CStokesParameter::sQ) {
        return StokesParameterMod::Q;
    }
    	
    if (name == CStokesParameter::sU) {
        return StokesParameterMod::U;
    }
    	
    if (name == CStokesParameter::sV) {
        return StokesParameterMod::V;
    }
    	
    if (name == CStokesParameter::sRR) {
        return StokesParameterMod::RR;
    }
    	
    if (name == CStokesParameter::sRL) {
        return StokesParameterMod::RL;
    }
    	
    if (name == CStokesParameter::sLR) {
        return StokesParameterMod::LR;
    }
    	
    if (name == CStokesParameter::sLL) {
        return StokesParameterMod::LL;
    }
    	
    if (name == CStokesParameter::sXX) {
        return StokesParameterMod::XX;
    }
    	
    if (name == CStokesParameter::sXY) {
        return StokesParameterMod::XY;
    }
    	
    if (name == CStokesParameter::sYX) {
        return StokesParameterMod::YX;
    }
    	
    if (name == CStokesParameter::sYY) {
        return StokesParameterMod::YY;
    }
    	
    if (name == CStokesParameter::sRX) {
        return StokesParameterMod::RX;
    }
    	
    if (name == CStokesParameter::sRY) {
        return StokesParameterMod::RY;
    }
    	
    if (name == CStokesParameter::sLX) {
        return StokesParameterMod::LX;
    }
    	
    if (name == CStokesParameter::sLY) {
        return StokesParameterMod::LY;
    }
    	
    if (name == CStokesParameter::sXR) {
        return StokesParameterMod::XR;
    }
    	
    if (name == CStokesParameter::sXL) {
        return StokesParameterMod::XL;
    }
    	
    if (name == CStokesParameter::sYR) {
        return StokesParameterMod::YR;
    }
    	
    if (name == CStokesParameter::sYL) {
        return StokesParameterMod::YL;
    }
    	
    if (name == CStokesParameter::sPP) {
        return StokesParameterMod::PP;
    }
    	
    if (name == CStokesParameter::sPQ) {
        return StokesParameterMod::PQ;
    }
    	
    if (name == CStokesParameter::sQP) {
        return StokesParameterMod::QP;
    }
    	
    if (name == CStokesParameter::sQQ) {
        return StokesParameterMod::QQ;
    }
    	
    if (name == CStokesParameter::sRCIRCULAR) {
        return StokesParameterMod::RCIRCULAR;
    }
    	
    if (name == CStokesParameter::sLCIRCULAR) {
        return StokesParameterMod::LCIRCULAR;
    }
    	
    if (name == CStokesParameter::sLINEAR) {
        return StokesParameterMod::LINEAR;
    }
    	
    if (name == CStokesParameter::sPTOTAL) {
        return StokesParameterMod::PTOTAL;
    }
    	
    if (name == CStokesParameter::sPLINEAR) {
        return StokesParameterMod::PLINEAR;
    }
    	
    if (name == CStokesParameter::sPFTOTAL) {
        return StokesParameterMod::PFTOTAL;
    }
    	
    if (name == CStokesParameter::sPFLINEAR) {
        return StokesParameterMod::PFLINEAR;
    }
    	
    if (name == CStokesParameter::sPANGLE) {
        return StokesParameterMod::PANGLE;
    }
    
    throw badString(name);
}

StokesParameterMod::StokesParameter CStokesParameter::literal(const std::string& name) {
		
    if (name == CStokesParameter::sI) {
        return StokesParameterMod::I;
    }
    	
    if (name == CStokesParameter::sQ) {
        return StokesParameterMod::Q;
    }
    	
    if (name == CStokesParameter::sU) {
        return StokesParameterMod::U;
    }
    	
    if (name == CStokesParameter::sV) {
        return StokesParameterMod::V;
    }
    	
    if (name == CStokesParameter::sRR) {
        return StokesParameterMod::RR;
    }
    	
    if (name == CStokesParameter::sRL) {
        return StokesParameterMod::RL;
    }
    	
    if (name == CStokesParameter::sLR) {
        return StokesParameterMod::LR;
    }
    	
    if (name == CStokesParameter::sLL) {
        return StokesParameterMod::LL;
    }
    	
    if (name == CStokesParameter::sXX) {
        return StokesParameterMod::XX;
    }
    	
    if (name == CStokesParameter::sXY) {
        return StokesParameterMod::XY;
    }
    	
    if (name == CStokesParameter::sYX) {
        return StokesParameterMod::YX;
    }
    	
    if (name == CStokesParameter::sYY) {
        return StokesParameterMod::YY;
    }
    	
    if (name == CStokesParameter::sRX) {
        return StokesParameterMod::RX;
    }
    	
    if (name == CStokesParameter::sRY) {
        return StokesParameterMod::RY;
    }
    	
    if (name == CStokesParameter::sLX) {
        return StokesParameterMod::LX;
    }
    	
    if (name == CStokesParameter::sLY) {
        return StokesParameterMod::LY;
    }
    	
    if (name == CStokesParameter::sXR) {
        return StokesParameterMod::XR;
    }
    	
    if (name == CStokesParameter::sXL) {
        return StokesParameterMod::XL;
    }
    	
    if (name == CStokesParameter::sYR) {
        return StokesParameterMod::YR;
    }
    	
    if (name == CStokesParameter::sYL) {
        return StokesParameterMod::YL;
    }
    	
    if (name == CStokesParameter::sPP) {
        return StokesParameterMod::PP;
    }
    	
    if (name == CStokesParameter::sPQ) {
        return StokesParameterMod::PQ;
    }
    	
    if (name == CStokesParameter::sQP) {
        return StokesParameterMod::QP;
    }
    	
    if (name == CStokesParameter::sQQ) {
        return StokesParameterMod::QQ;
    }
    	
    if (name == CStokesParameter::sRCIRCULAR) {
        return StokesParameterMod::RCIRCULAR;
    }
    	
    if (name == CStokesParameter::sLCIRCULAR) {
        return StokesParameterMod::LCIRCULAR;
    }
    	
    if (name == CStokesParameter::sLINEAR) {
        return StokesParameterMod::LINEAR;
    }
    	
    if (name == CStokesParameter::sPTOTAL) {
        return StokesParameterMod::PTOTAL;
    }
    	
    if (name == CStokesParameter::sPLINEAR) {
        return StokesParameterMod::PLINEAR;
    }
    	
    if (name == CStokesParameter::sPFTOTAL) {
        return StokesParameterMod::PFTOTAL;
    }
    	
    if (name == CStokesParameter::sPFLINEAR) {
        return StokesParameterMod::PFLINEAR;
    }
    	
    if (name == CStokesParameter::sPANGLE) {
        return StokesParameterMod::PANGLE;
    }
    
    throw badString(name);
}

StokesParameterMod::StokesParameter CStokesParameter::from_int(unsigned int i) {
	vector<string> names = sStokesParameterSet();
	if (i >= names.size()) throw badInt(i);
	return newStokesParameter(names.at(i));
}

	

string CStokesParameter::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'StokesParameter'.";
}

string CStokesParameter::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'StokesParameter'.";
	return oss.str();
}

