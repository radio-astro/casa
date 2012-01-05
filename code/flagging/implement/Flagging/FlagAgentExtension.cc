//# FlagAgenExtension.cc: This file contains the implementation of the FlagAgenExtension.cc class.
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

#include <flagging/Flagging/FlagAgentExtension.h>

namespace casa { //# NAMESPACE CASA - BEGIN

FlagAgentExtension::FlagAgentExtension(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube):
		FlagAgentBase(dh,config,ANTENNA_PAIRS,writePrivateFlagCube)
{
	setAgentParameters(config);

	// Request loading polarization map to FlagDataHandler
	flagDataHandler_p->setMapPolarizations(true);
}

FlagAgentExtension::~FlagAgentExtension()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void FlagAgentExtension::setAgentParameters(Record config)
{
        logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
	int exists;

	exists = config.fieldNumber ("extendpols");
	if (exists >= 0)
	{
		extendpols_p = config.asBool("extendpols");
	}
	else
	{
		extendpols_p = False;
	}
	*logger_p << LogIO::NORMAL << " extendpols is " << extendpols_p << LogIO::POST;


	exists = config.fieldNumber ("growtime");
	if (exists >= 0)
	{
		growtime_p = config.asDouble("growtime");
	}
	else
	{
		growtime_p = 50.0;
	}
	*logger_p << LogIO::NORMAL << " growtime is " << growtime_p << LogIO::POST;


	exists = config.fieldNumber ("growfreq");
	if (exists >= 0)
	{
		growfreq_p = config.asDouble("growfreq");
	}
	else
	{
		growfreq_p = 50.0;
	}
	*logger_p << LogIO::NORMAL << " growfreq is " << growfreq_p << LogIO::POST;


	exists = config.fieldNumber ("growaround");
	if (exists >= 0)
	{
		growaround_p = config.asBool("growaround");
	}
	else
	{
		growaround_p = True;
	}
	*logger_p << LogIO::NORMAL << " growaround is " << growaround_p << LogIO::POST;


	exists = config.fieldNumber ("flagneartime");
	if (exists >= 0)
	{
		flagneartime_p = config.asBool("flagneartime");
	}
	else
	{
		flagneartime_p = False;
	}
	*logger_p << LogIO::NORMAL << " flagneartime is " << flagneartime_p << LogIO::POST;


	exists = config.fieldNumber ("flagnearfreq");
	if (exists >= 0)
	{
		flagnearfreq_p = config.asBool("flagnearfreq");
	}
	else
	{
		flagnearfreq_p = False;
	}
	*logger_p << LogIO::NORMAL << " flagnearfreq is " << flagnearfreq_p << LogIO::POST;


	return;
}

void
FlagAgentExtension::computeAntennaPairFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows)
{
        logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
	IPosition flagCubeShape = flags.shape();
	Int nPols,nChannels,nTimesteps;
	flags.shape(nPols, nChannels, nTimesteps);

	*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__
			<< " antenna1=" << antenna1
			<< " antenna2=" << antenna2
			<< " nPols=" << nPols
			<< " nChannels=" << nChannels
			<< " nTimesteps=" << nTimesteps
			<< LogIO::POST;

	// Create the growing flag cube setting all flags initially to false
	Cube<Bool> *growFlags = new Cube<Bool>(nPols, nChannels, nTimesteps,false);

	// Figure out what flags have to be extended
	bool flag;
	uInt extendTimestep,extendChannel,extendPol,extendAround;
	for (Int timestep_i=0;timestep_i<nTimesteps;timestep_i++)
	{
		for (Int chan_j=0;chan_j<nChannels;chan_j++)
		{
			for (Int pol_k=0;pol_k<nPols;pol_k++)
			{
				flag = flags.getModifiedFlags(pol_k,chan_j,timestep_i);

				// If the point is flagged we may extend in the time,frequency or polarization direction
				if (flag)
				{
					if (flagneartime_p)
					{
						// Check if we can extend to the next timestep
						extendTimestep = timestep_i + 1;
						if (extendTimestep < nTimesteps) growFlags->operator()(pol_k,chan_j,extendTimestep) = true;

						// Check if we can extend to the previous timestep
						extendTimestep = timestep_i - 1;
						if (extendTimestep >= 0) growFlags->operator()(pol_k,chan_j,extendTimestep) = true;
					}

					if (flagnearfreq_p)
					{
						// Check if we can extend to the next frequency
						extendChannel = chan_j + 1;
						if (extendChannel < nChannels) growFlags->operator()(pol_k,extendChannel,timestep_i) = true;

						// Check if we can extend to the previous frequency
						extendChannel = chan_j - 1;
						if (extendChannel >= 0) growFlags->operator()(pol_k,extendChannel,timestep_i) = true;
					}

					if (extendpols_p)
					{
						for (Int pol_inner=0;pol_inner<nPols;pol_inner++)
						{
							growFlags->operator()(pol_inner,chan_j,timestep_i) = true;
						}
					}
				}
				// Only if the point is not flagged, and grow around is activated, we check the nearest neighbours
				else if (growaround_p)
				{
					extendAround = 0;
					for (Int timestep_inner=timestep_i-1;timestep_inner<=timestep_i+1;timestep_inner++)
					{
						// Calculate number of flagged neighbours
						for (Int chan_inner=chan_j-1;chan_inner<=chan_j+1;chan_inner++)
						{
							if (	(timestep_inner>=0) and
									(timestep_inner<nTimesteps) and
									(chan_inner>=0) and
									(chan_inner<nChannels))
							{
								extendAround += flags.getModifiedFlags(pol_k,chan_inner,timestep_inner);
							}
						}

						// Extend flags if we have more than 4 flagged  neighbours
						if (extendAround>4) growFlags->operator()(pol_k,chan_j,timestep_i) = true;
					}
				}
			}
		}
	}

	// Apply extended flags to the common modified flag cube
	for (uInt timestep_i=0;timestep_i<nTimesteps;timestep_i++)
	{
		for (uInt chan_j=0;chan_j<nChannels;chan_j++)
		{
			for (uInt pol_k=0;pol_k<nPols;pol_k++)
			{
				if (growFlags->operator()(pol_k,chan_j,timestep_i))
				{
					flags.setModifiedFlags(pol_k,chan_j,timestep_i);
					visBufferFlags_p += 1;
				}
			}
		}
	}

	// Delete growing flag cube
	delete growFlags;

	return;
}

} //# NAMESPACE CASA - END


