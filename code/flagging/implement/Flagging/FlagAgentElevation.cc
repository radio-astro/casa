//# FlagAgentElevation.cc: This file contains the implementation of the FlagAgentElevation class.
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

#include <flagging/Flagging/FlagAgentElevation.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Definition of static members for common pre-processing
vector< vector<Double> > FlagAgentElevation::antennaPointingMap_p;
casa::async::Mutex FlagAgentElevation::staticMembersMutex_p;
vector<bool> FlagAgentElevation::startedProcessing_p;
bool FlagAgentElevation::preProcessingDone_p = false;
uShort FlagAgentElevation::nAgents_p = 0;


FlagAgentElevation::FlagAgentElevation(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube):
		FlagAgentBase(dh,config,ROWS_PREPROCESS_BUFFER,writePrivateFlagCube)
{
	setAgentParameters(config);

	// Request loading antenna pointing map to FlagDataHandler
	flagDataHandler_p->setMapAntennaPointing(true);

	// FlagAgentElevation counters and ids to handle static variables
	staticMembersMutex_p.acquirelock();
	agentNumber_p = nAgents_p;
	nAgents_p += 1;
	staticMembersMutex_p.unlock();
}

FlagAgentElevation::~FlagAgentElevation()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void
FlagAgentElevation::setAgentParameters(Record config)
{
	int exists;

	exists = config.fieldNumber ("lowerlimit");
	if (exists >= 0)
	{
		lowerlimit_p = config.asDouble("lowerlimit");
	}
	else
	{
		lowerlimit_p = 0.0;
	}

	*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " lowerlimit is " << lowerlimit_p << LogIO::POST;

	exists = config.fieldNumber ("upperlimit");
	if (exists >= 0)
	{
		upperlimit_p = config.asDouble("upperlimit");
	}
	else
	{
		upperlimit_p = 90.0;
	}

	*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " upperlimit is " << upperlimit_p << LogIO::POST;


	return;
}

void
FlagAgentElevation::preProcessBuffer(const VisBuffer &visBuffer)
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
FlagAgentElevation::preProcessBufferCore(const VisBuffer &visBuffer)
{
	Vector<Double> time = visBuffer.time();
	uInt nRows = time.size();
	antennaPointingMap_p.clear();
	antennaPointingMap_p.reserve(nRows);
	for (uInt row_i=0;row_i<nRows;row_i++)
	{
		Vector<MDirection> azimuth_elevation = visBuffer.azel(time[row_i]);
		Int ant1 = visBuffer.antenna1()[row_i];
		Int ant2 = visBuffer.antenna1()[row_i];

		double antenna1_elevation = azimuth_elevation[ant1].getAngle("deg").getValue()[1];
		double antenna2_elevation = azimuth_elevation[ant2].getAngle("deg").getValue()[1];

		vector<Double> item(2);
		item[0] = antenna1_elevation;
		item[1] = antenna2_elevation;
		antennaPointingMap_p.push_back(item);
	}

	return;
}

void
FlagAgentElevation::computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
{
    double antenna1_elevation = antennaPointingMap_p.at(row).at(0);
    double antenna2_elevation = antennaPointingMap_p.at(row).at(1);

    if ( antenna1_elevation < lowerlimit_p ||
         antenna2_elevation < lowerlimit_p ||
         antenna1_elevation > upperlimit_p ||
         antenna2_elevation > upperlimit_p )
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


