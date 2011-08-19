
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
 * File CWindowFunction.cpp
 */
#include <sstream>
#include <CWindowFunction.h>
#include <string>
using namespace std;

	
const std::string& CWindowFunction::sUNIFORM = "UNIFORM";
	
const std::string& CWindowFunction::sHANNING = "HANNING";
	
const std::string& CWindowFunction::sHAMMING = "HAMMING";
	
const std::string& CWindowFunction::sBARTLETT = "BARTLETT";
	
const std::string& CWindowFunction::sBLACKMANN = "BLACKMANN";
	
const std::string& CWindowFunction::sBLACKMANN_HARRIS = "BLACKMANN_HARRIS";
	
const std::string& CWindowFunction::sWELCH = "WELCH";
	
const std::vector<std::string> CWindowFunction::sWindowFunctionSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CWindowFunction::sUNIFORM);
    
    enumSet.insert(enumSet.end(), CWindowFunction::sHANNING);
    
    enumSet.insert(enumSet.end(), CWindowFunction::sHAMMING);
    
    enumSet.insert(enumSet.end(), CWindowFunction::sBARTLETT);
    
    enumSet.insert(enumSet.end(), CWindowFunction::sBLACKMANN);
    
    enumSet.insert(enumSet.end(), CWindowFunction::sBLACKMANN_HARRIS);
    
    enumSet.insert(enumSet.end(), CWindowFunction::sWELCH);
        
    return enumSet;
}

	

	
	
const std::string& CWindowFunction::hUNIFORM = "No windowing";
	
const std::string& CWindowFunction::hHANNING = "Raised cosine: 0.5*(1-cos(x)) where x = 2*pi*i/(N-1)";
	
const std::string& CWindowFunction::hHAMMING = "The classic Hamming window is Wm(x) = 0.54 - 0.46*cos(x). This is generalized to Wm(x) = beta - (1-beta)*cos(x) where beta can take any value in the range [0,1]. Beta=0.5 corresponds to the Hanning window.";
	
const std::string& CWindowFunction::hBARTLETT = "The Bartlett (triangular) window is 1 - |x/pi|, where x = 2*pi*i/(N-1)";
	
const std::string& CWindowFunction::hBLACKMANN = "The window function is: Wb(x) = (0.5 - Beta) - 0.5*cos(Xj) + Beta*cos(2Xj), where Xj=2*pi*j/(N-1). The classic Blackman window is given by Beta=0.08";
	
const std::string& CWindowFunction::hBLACKMANN_HARRIS = "The BLACKMANN_HARRIS window is 1.0 - 1.36109*cos(x) + 0.39381*cos(2x) - 0.032557*cos(3x), where x = 2*pi*i/(N-1)";
	
const std::string& CWindowFunction::hWELCH = "The Welch window (parabolic) is 1 - (2*i/N)^2";
	
const std::vector<std::string> CWindowFunction::hWindowFunctionSet() {
    std::vector<std::string> enumSet;
    
    enumSet.insert(enumSet.end(), CWindowFunction::hUNIFORM);
    
    enumSet.insert(enumSet.end(), CWindowFunction::hHANNING);
    
    enumSet.insert(enumSet.end(), CWindowFunction::hHAMMING);
    
    enumSet.insert(enumSet.end(), CWindowFunction::hBARTLETT);
    
    enumSet.insert(enumSet.end(), CWindowFunction::hBLACKMANN);
    
    enumSet.insert(enumSet.end(), CWindowFunction::hBLACKMANN_HARRIS);
    
    enumSet.insert(enumSet.end(), CWindowFunction::hWELCH);
        
    return enumSet;
}
   	

std::string CWindowFunction::name(const WindowFunctionMod::WindowFunction& f) {
    switch (f) {
    
    case WindowFunctionMod::UNIFORM:
      return CWindowFunction::sUNIFORM;
    
    case WindowFunctionMod::HANNING:
      return CWindowFunction::sHANNING;
    
    case WindowFunctionMod::HAMMING:
      return CWindowFunction::sHAMMING;
    
    case WindowFunctionMod::BARTLETT:
      return CWindowFunction::sBARTLETT;
    
    case WindowFunctionMod::BLACKMANN:
      return CWindowFunction::sBLACKMANN;
    
    case WindowFunctionMod::BLACKMANN_HARRIS:
      return CWindowFunction::sBLACKMANN_HARRIS;
    
    case WindowFunctionMod::WELCH:
      return CWindowFunction::sWELCH;
    	
    }
    return std::string("");
}

	

	
std::string CWindowFunction::help(const WindowFunctionMod::WindowFunction& f) {
    switch (f) {
    
    case WindowFunctionMod::UNIFORM:
      return CWindowFunction::hUNIFORM;
    
    case WindowFunctionMod::HANNING:
      return CWindowFunction::hHANNING;
    
    case WindowFunctionMod::HAMMING:
      return CWindowFunction::hHAMMING;
    
    case WindowFunctionMod::BARTLETT:
      return CWindowFunction::hBARTLETT;
    
    case WindowFunctionMod::BLACKMANN:
      return CWindowFunction::hBLACKMANN;
    
    case WindowFunctionMod::BLACKMANN_HARRIS:
      return CWindowFunction::hBLACKMANN_HARRIS;
    
    case WindowFunctionMod::WELCH:
      return CWindowFunction::hWELCH;
    	
    }
    return std::string("");
}
   	

WindowFunctionMod::WindowFunction CWindowFunction::newWindowFunction(const std::string& name) {
		
    if (name == CWindowFunction::sUNIFORM) {
        return WindowFunctionMod::UNIFORM;
    }
    	
    if (name == CWindowFunction::sHANNING) {
        return WindowFunctionMod::HANNING;
    }
    	
    if (name == CWindowFunction::sHAMMING) {
        return WindowFunctionMod::HAMMING;
    }
    	
    if (name == CWindowFunction::sBARTLETT) {
        return WindowFunctionMod::BARTLETT;
    }
    	
    if (name == CWindowFunction::sBLACKMANN) {
        return WindowFunctionMod::BLACKMANN;
    }
    	
    if (name == CWindowFunction::sBLACKMANN_HARRIS) {
        return WindowFunctionMod::BLACKMANN_HARRIS;
    }
    	
    if (name == CWindowFunction::sWELCH) {
        return WindowFunctionMod::WELCH;
    }
    
    throw badString(name);
}

WindowFunctionMod::WindowFunction CWindowFunction::literal(const std::string& name) {
		
    if (name == CWindowFunction::sUNIFORM) {
        return WindowFunctionMod::UNIFORM;
    }
    	
    if (name == CWindowFunction::sHANNING) {
        return WindowFunctionMod::HANNING;
    }
    	
    if (name == CWindowFunction::sHAMMING) {
        return WindowFunctionMod::HAMMING;
    }
    	
    if (name == CWindowFunction::sBARTLETT) {
        return WindowFunctionMod::BARTLETT;
    }
    	
    if (name == CWindowFunction::sBLACKMANN) {
        return WindowFunctionMod::BLACKMANN;
    }
    	
    if (name == CWindowFunction::sBLACKMANN_HARRIS) {
        return WindowFunctionMod::BLACKMANN_HARRIS;
    }
    	
    if (name == CWindowFunction::sWELCH) {
        return WindowFunctionMod::WELCH;
    }
    
    throw badString(name);
}

WindowFunctionMod::WindowFunction CWindowFunction::from_int(unsigned int i) {
	vector<string> names = sWindowFunctionSet();
	if (i >= names.size()) throw badInt(i);
	return newWindowFunction(names.at(i));
}

	

string CWindowFunction::badString(const string& name) {
	return "'"+name+"' does not correspond to any literal in the enumeration 'WindowFunction'.";
}

string CWindowFunction::badInt(unsigned int i) {
	ostringstream oss ;
	oss << "'" << i << "' is out of range for the enumeration 'WindowFunction'.";
	return oss.str();
}

