
#ifndef CStokesParameter_H
#define CStokesParameter_H

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
 * File CStokesParameter.h
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>
#include <vector>
#ifndef WITHOUT_ACS
#include <almaEnumerations_IFC.h>
#else
namespace StokesParameterMod
{
  enum StokesParameter
  { 
    I ,
    Q ,
    U ,
    V ,
    RR ,
    RL ,
    LR ,
    LL ,
    XX ,
    XY ,
    YX ,
    YY ,
    RX ,
    RY ,
    LX ,
    LY ,
    XR ,
    XL ,
    YR ,
    YL ,
    PP ,
    PQ ,
    QP ,
    QQ ,
    RCIRCULAR ,
    LCIRCULAR ,
    LINEAR ,
    PTOTAL ,
    PLINEAR ,
    PFTOTAL ,
    PFLINEAR ,
    PANGLE 
  };
  typedef StokesParameter &StokesParameter_out;
} 
#endif

using namespace std;

class CStokesParameter {
  public:
  	static string badString(const string& name) ;
  	static string badInt(unsigned int i) ;
  	
	// Names associated with the StokesParameter enumeration.  
	
	static const std::string& sI;
	
	static const std::string& sQ;
	
	static const std::string& sU;
	
	static const std::string& sV;
	
	static const std::string& sRR;
	
	static const std::string& sRL;
	
	static const std::string& sLR;
	
	static const std::string& sLL;
	
	static const std::string& sXX;
	
	static const std::string& sXY;
	
	static const std::string& sYX;
	
	static const std::string& sYY;
	
	static const std::string& sRX;
	
	static const std::string& sRY;
	
	static const std::string& sLX;
	
	static const std::string& sLY;
	
	static const std::string& sXR;
	
	static const std::string& sXL;
	
	static const std::string& sYR;
	
	static const std::string& sYL;
	
	static const std::string& sPP;
	
	static const std::string& sPQ;
	
	static const std::string& sQP;
	
	static const std::string& sQQ;
	
	static const std::string& sRCIRCULAR;
	
	static const std::string& sLCIRCULAR;
	
	static const std::string& sLINEAR;
	
	static const std::string& sPTOTAL;
	
	static const std::string& sPLINEAR;
	
	static const std::string& sPFTOTAL;
	
	static const std::string& sPFLINEAR;
	
	static const std::string& sPANGLE;
	
    static const std::vector<std::string> sStokesParameterSet();	 

	

	
	// Explanations associated with the StokesParameter Enumeration.
		
	static const std::string& hI;
		
	static const std::string& hQ;
		
	static const std::string& hU;
		
	static const std::string& hV;
		
	static const std::string& hRR;
		
	static const std::string& hRL;
		
	static const std::string& hLR;
		
	static const std::string& hLL;
		
	static const std::string& hXX;
		
	static const std::string& hXY;
		
	static const std::string& hYX;
		
	static const std::string& hYY;
		
	static const std::string& hRX;
		
	static const std::string& hRY;
		
	static const std::string& hLX;
		
	static const std::string& hLY;
		
	static const std::string& hXR;
		
	static const std::string& hXL;
		
	static const std::string& hYR;
		
	static const std::string& hYL;
		
	static const std::string& hPP;
		
	static const std::string& hPQ;
		
	static const std::string& hQP;
		
	static const std::string& hQQ;
		
	static const std::string& hRCIRCULAR;
		
	static const std::string& hLCIRCULAR;
		
	static const std::string& hLINEAR;
		
	static const std::string& hPTOTAL;
		
	static const std::string& hPLINEAR;
		
	static const std::string& hPFTOTAL;
		
	static const std::string& hPFLINEAR;
		
	static const std::string& hPANGLE;
		
	static const std::vector<std::string> hStokesParameterSet();
   	

   	// Is an integer number associated with the StokesParameter enumeration?
    static bool isNumber() { return false; }
   	
   	// Is a help text associated with the StokesParameter enumeration?
    static bool isHelp() { return true; }
    
    // Get the string name associated with the specified  StokesParameter enumeration.
	static std::string name(const StokesParameterMod::StokesParameter& f);
    static std::string toString(const StokesParameterMod::StokesParameter& f) { return name(f); }

	

	
	// Get the help text associated with the specified StokesParameter enumeration.
	static std::string help(const StokesParameterMod::StokesParameter& f);
   	
   	
   	// Create a StokesParameter enumeration object by specifying its name.
   	static StokesParameterMod::StokesParameter newStokesParameter(const std::string& name);
   	
   	// Create a StokesParameter enumeration object by specifying its name.
 	static StokesParameterMod::StokesParameter literal(const std::string& name);
 	
    // Create a StokesParameter enumeration object by specifying its position index (0 based).
 	static StokesParameterMod::StokesParameter from_int(unsigned int i);	
 	
	

  private:
    /* Not Implemented.  This is a pure static class. */
    CStokesParameter();
    CStokesParameter(const CStokesParameter&);
    CStokesParameter& operator=(const CStokesParameter&);
};
 
#endif /*!CStokesParameter_H*/
