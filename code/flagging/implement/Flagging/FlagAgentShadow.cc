//# FlagAgentShadow.cc: This file contains the implementation of the FlagAgentShadow class.
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

#include <flagging/Flagging/FlagAgentShadow.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Definition of static members for common pre-processing
vector<Int> FlagAgentShadow::shadowedAntennas_p;
casa::async::Mutex FlagAgentShadow::staticMembersMutex_p;
vector<bool> FlagAgentShadow::startedProcessing_p;
bool FlagAgentShadow::preProcessingDone_p = false;
uShort FlagAgentShadow::nAgents_p = 0;

FlagAgentShadow::FlagAgentShadow(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube):
		FlagAgentBase(dh,config,ROWS_PREPROCESS_BUFFER,writePrivateFlagCube)
{
	setAgentParameters(config);

	// Set preProcessingDone_p static member to false
	preProcessingDone_p = false;

	// Request loading antenna1,antenna2 and uvw
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant1);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant2);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Uvw);

	// FlagAgentShadow counters and ids to handle static variables
	staticMembersMutex_p.acquirelock();
	agentNumber_p = nAgents_p;
	nAgents_p += 1;
	staticMembersMutex_p.unlock();
}

FlagAgentShadow::~FlagAgentShadow()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()

	// NOTE: The following is necessary because the static variables
	// persist even if all the instances of the class were deleted!
	staticMembersMutex_p.acquirelock();
	agentNumber_p = nAgents_p;
	nAgents_p -= 1;
	staticMembersMutex_p.unlock();
}

void
FlagAgentShadow::setAgentParameters(Record config)
{
        logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
	int exists;

	exists = config.fieldNumber ("diameter");
	if (exists >= 0)
	{
		antennaDiameter_p = config.asDouble("diameter");
	}
	else
	{
		antennaDiameter_p = -1.0;
	}

	*logger_p << LogIO::NORMAL << " diameter is " << antennaDiameter_p << LogIO::POST;

	return;
}

void
FlagAgentShadow::preProcessBuffer(const VisBuffer &visBuffer)
{
	if (nAgents_p > 1)
	{
		staticMembersMutex_p.acquirelock();

		if (!preProcessingDone_p)
		{
			// Reset processing state variables
			if (startedProcessing_p.size() != nAgents_p) startedProcessing_p.resize(nAgents_p,false);
			for (vector<bool>::iterator iter = startedProcessing_p.begin();iter != startedProcessing_p.end();iter++)
			{
				*iter = false;
			}

			// Do actual pre-processing
			preProcessBufferCore(visBuffer);

			// Mark pre-processing as done so that other agents don't redo it
			preProcessingDone_p = true;
		}

		staticMembersMutex_p.unlock();
	}
	else
	{
		preProcessBufferCore(visBuffer);
	}

	return;
}
void
FlagAgentShadow::preProcessBufferCore(const VisBuffer &visBuffer)
{
	Vector<Int> antenna1list =  visBuffer.antenna1();
	Vector<Int> antenna2list =  visBuffer.antenna2();
	shadowedAntennas_p.clear();

	Double u,v,w, uvDistance;
	Int antenna1, antenna2;
	Double antennaDiameter1,antennaDiameter2, antennaDistance;
	for (Int row_i=0;row_i<antenna1list.size();row_i++)
	{
		// Retrieve antenna ids
		antenna1 = antenna1list[row_i];
		antenna2 = antenna2list[row_i];

		// Check if this row corresponds to autocorrelation (Antennas don't shadow themselves)
		if (antenna1 == antenna2) continue;

		// Get antenna diameter
		if (antennaDiameter_p>0)
		{
			antennaDiameter1 = antennaDiameter_p;
			antennaDiameter2 = antennaDiameter_p;
		}
		else
		{
			Vector<Double> *antennaDiameters = flagDataHandler_p->antennaDiameters_p;
			antennaDiameter1 = (*antennaDiameters)[antenna1];
			antennaDiameter2 = (*antennaDiameters)[antenna2];
		}

		// Compute effective distance for shadowing
		antennaDistance = (antennaDiameter1+antennaDiameter2)*(antennaDiameter1+antennaDiameter2)/4.0;

		// Compute uv distance
		u = visBuffer.uvw()(row_i)(0);
		v = visBuffer.uvw()(row_i)(1);
		w = visBuffer.uvw()(row_i)(2);
		uvDistance = u*u + v*v;

		// Check if one of the antennas can be shadowed
		if (uvDistance < antennaDistance)
		{
			if (w>0)
			{
				if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) == shadowedAntennas_p.end())
				{
					shadowedAntennas_p.push_back(antenna1);
				}
			}
			else
			{
				if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) == shadowedAntennas_p.end())
				{
					shadowedAntennas_p.push_back(antenna2);
				}
			}
		}
	}
}

void
FlagAgentShadow::computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
{
	// Flag row if either antenna1 or antenna2 are in the list of shadowed antennas
	Int antenna1 = visBuffer.antenna1()[row];
	Int antenna2 = visBuffer.antenna2()[row];
	if (	(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) != shadowedAntennas_p.end()) or
			(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) != shadowedAntennas_p.end()) )
	{
    	IPosition flagCubeShape = flags.shape();
    	uInt nChannels = flagCubeShape(0);
    	for (uInt chan_i=0;chan_i<nChannels;chan_i++)
    	{
    		flags.applyFlag(chan_i,row);
    	}
    	visBufferFlags_p += flags.flagsPerRow();
	}

	if ((nAgents_p > 1) and preProcessingDone_p)
	{
		startedProcessing_p[agentNumber_p] = true;
		if (std::find (startedProcessing_p.begin(), startedProcessing_p.end(), false) == startedProcessing_p.end())
		{
			preProcessingDone_p = false;
		}
	}

	return;
}

} //# NAMESPACE CASA - END


