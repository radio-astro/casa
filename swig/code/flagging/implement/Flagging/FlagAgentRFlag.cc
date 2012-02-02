//# FlagAgentRFlag.cc: This file contains the implementation of the FlagAgentRFlag class.
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

#include <flagging/Flagging/FlagAgentRFlag.h>

namespace casa { //# NAMESPACE CASA - BEGIN


FlagAgentRFlag::FlagAgentRFlag(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube, Bool flag):
		FlagAgentBase(dh,config,ANTENNA_PAIRS,writePrivateFlagCube,flag)
{
	setAgentParameters(config);

	// Request loading polarization map to FlagDataHandler
	flagDataHandler_p->setMapPolarizations(true);
}

FlagAgentRFlag::~FlagAgentRFlag()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void FlagAgentRFlag::setAgentParameters(Record config)
{
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
	int exists;

	exists = config.fieldNumber ("half_nchan");
	if (exists >= 0)
	{
		half_nchan_p = atoi(config.asString("half_nchan").c_str());
	}
	else
	{
		half_nchan_p = 1;
	}

	*logger_p << logLevel_p << " half_nchan is " << half_nchan_p << LogIO::POST;

	exists = config.fieldNumber ("half_ntime");
	if (exists >= 0)
	{
		half_ntime_p = atoi(config.asString("half_ntime").c_str());
	}
	else
	{
		half_ntime_p = 1;
	}

	*logger_p << logLevel_p << " half_ntime is " << half_ntime_p << LogIO::POST;

	return;
}

bool
FlagAgentRFlag::computeAntennaPairFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows)
{
	// Set logger origin
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

	// Get flag cube size
	IPosition flagCubeShape = flags.shape();
	Int nPols,nChannels,nTimesteps;
	flags.shape(nPols, nChannels, nTimesteps);

	// Prepare sliding window
	Matrix<Float> slidingWindow(half_nchan_p*2+1,half_ntime_p*2+1);
	Int slidingWindow_timestep_idx, slidingWindow_chan_idx;

	// Iterate over flag cube and grow flags
	for (Int timestep_i=half_ntime_p;timestep_i<nTimesteps-half_ntime_p;timestep_i++)
	{
		for (Int chan_j=half_nchan_p;chan_j<nChannels-half_nchan_p;chan_j++)
		{
			for (Int pol_k=0;pol_k<nPols;pol_k++)
			{
				// Inner iteration to fill sliding window
				for (Int timestep_inner=timestep_i-half_ntime_p;timestep_inner<timestep_i+half_ntime_p;timestep_inner++)
				{
					slidingWindow_timestep_idx = timestep_inner - (timestep_i-half_ntime_p);
					for (Int chan_inner=chan_j-half_nchan_p;chan_inner<chan_j+half_nchan_p;chan_inner++)
					{
						slidingWindow_chan_idx = chan_inner - (chan_j-half_nchan_p);
						slidingWindow(slidingWindow_chan_idx,slidingWindow_timestep_idx) = visibilities(chan_inner,timestep_inner);
					}
				}
			}
		}
	}

	return false;
}

} //# NAMESPACE CASA - END


