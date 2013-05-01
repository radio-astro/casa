//# FlagAgentShadow.h: This file contains the interface definition of the FlagAgentShadow class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef FlagAgentShadow_H_
#define FlagAgentShadow_H_

#include <flagging/Flagging/FlagAgentBase.h>

#include <measures/Measures/MeasFrame.h>
//#include <casa/Quanta/MVAngle.h>
//#include <casa/Quanta/MVTime.h>
//#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/Muvw.h>
#include <measures/Measures/MCuvw.h>
#include <casa/Quanta/MVuvw.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/MVDirection.h>
#include <measures/Measures.h>
#include <casa/Utilities/DataType.h>

#include <casa/Containers/Record.h>


namespace casa { //# NAMESPACE CASA - BEGIN

class FlagAgentShadow : public FlagAgentBase {

public:

	FlagAgentShadow(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube = false, Bool flag = true);
	~FlagAgentShadow();

protected:

	// Common functionality for each visBuffer (don't repeat at the row level)
	void preProcessBuffer(const vi::VisBuffer2 &visBuffer);

	// Common preProcessing code for the single/multiple agent cases
	void preProcessBufferCore(const vi::VisBuffer2 &visBuffer);

	// Compute flags afor a given mapped visibility point
	bool computeRowFlags(const vi::VisBuffer2 &visBuffer, FlagMapper &flags, uInt row);

	// Parse configuration parameters
	void setAgentParameters(Record config);

private:

        // Function to compute antenna UVW values for the current timestep
        Bool computeAntUVW(const vi::VisBuffer2 &vb, Int rownr);
        // Function to compute shadowed antennas, given a list of antenna UVWs.
        void calculateShadowedAntennas(const vi::VisBuffer2 &visBuffer, Int rownr);
        // Function to decide if the 'behind' antenna is shadowed or not, for one baseline
        void decideBaselineShadow(Double uvDistance, Double w, Int antenna1, Int antenna2);
        // Function to return baseline index.
        uInt baselineIndex(uInt nAnt, uInt a1, uInt a2);
 
	/// Input parameters ///
	Double shadowTolerance_p;
        Record additionalAntennas_p;

        // Copies of antenna-information lists, containing extra antennas if specified.
        ///Vector<String> shadowAntennaNames_p;
        Vector<Double> shadowAntennaDiameters_p;
        Vector<MPosition> shadowAntennaPositions_p;

	// Declaration of static members for common pre-processing
	uShort agentNumber_p;
	static vector<Int> shadowedAntennas_p;
	static casa::async::Mutex staticMembersMutex_p;
	static vector<bool> startedProcessing_p;
	static bool preProcessingDone_p;
	static uShort nAgents_p;
        
        // Private variables that change with each timestep
        Matrix<Double> uvwAnt_p;
        Double currTime_p;

        Bool firststep_p; // helper variable to control a debug print statement
    
};

} //# NAMESPACE CASA - END

#endif /* FlagAgentShadow_H_ */

