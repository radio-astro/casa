//# FlagAgentClipping.cc: This file contains the implementation of the FlagAgentClipping class.
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

#include <flagging/Flagging/FlagAgentClipping.h>

namespace casa { //# NAMESPACE CASA - BEGIN

FlagAgentClipping::FlagAgentClipping(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube):
		FlagAgentBase(dh,config,IN_ROWS,writePrivateFlagCube)
{
	setAgentParameters(config);

	// Request loading polarization map to FlagDataHandler
	flagDataHandler_p->setMapPolarizations(true);
}

FlagAgentClipping::~FlagAgentClipping()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void
FlagAgentClipping::setAgentParameters(Record config)
{
	int exists;

	exists = config.fieldNumber ("clipminmax");
	if (exists >= 0)
	{
		Array<Double> cliprange = config.asArrayDouble("clipminmax");
		Bool deleteIt = False;
		clipmin_p = cliprange.getStorage(deleteIt)[0];
		clipmax_p = cliprange.getStorage(deleteIt)[1];
	}
	else
	{
		clipmin_p = 0.0;
		clipmax_p = 1.5;
	}

	*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " clipmin is " << clipmin_p << LogIO::POST;
	*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " clipmax is " << clipmax_p << LogIO::POST;

	exists = config.fieldNumber ("clipoutside");
	if (exists >= 0)
	{
		clipoutside_p = config.asBool("clipoutside");
	}
	else
	{
		clipoutside_p = True;
	}

	*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " clipoutside is " << clipoutside_p << LogIO::POST;

	exists = config.fieldNumber ("channelavg");
	if (exists >= 0)
	{
		channelavg_p = config.asBool("channelavg");
	}
	else
	{
		channelavg_p = False;
	}

	*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " channelavg is " << channelavg_p << LogIO::POST;


	return;
}

void
FlagAgentClipping::computeInRowFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags, uInt row)
{
	IPosition flagCubeShape= flags.shape();
	uInt nChannels,chan_i;
	nChannels = flagCubeShape(0);
	Float visExpression;
	uInt nAverage;

	if (channelavg_p)
	{
		visExpression = 0;
		nAverage = 0;

		for (chan_i=0;chan_i<nChannels;chan_i++)
		{
			// If none of the correlations involved in the expression
			// are flagged, then take into account this channel
			if (!flags.getModifiedFlags(chan_i,row))
			{
				visExpression += visibilities(chan_i,row);
				nAverage += 1;
			}
		}

		visExpression /= nAverage;

		// If visExpression is out of range we flag the entire row
		if (	((visExpression >  clipmax_p) and clipoutside_p) or
				((visExpression <  clipmin_p) and clipoutside_p) or
				((visExpression <=  clipmax_p) and (visExpression >=  clipmin_p) and !clipoutside_p) or
				isnan(visExpression))
		{
			for (chan_i=0;chan_i<nChannels;chan_i++)
			{
				flags.applyFlag(chan_i,row);
			}
		}

	}
	else
	{
		for (chan_i=0;chan_i<nChannels;chan_i++)
		{
			visExpression = visibilities(chan_i,row);
			if (	((visExpression >  clipmax_p) and clipoutside_p) or
					((visExpression <  clipmin_p) and clipoutside_p) or
					((visExpression <=  clipmax_p) and (visExpression >=  clipmin_p) and !clipoutside_p) or
					isnan(visExpression))
			{
				flags.applyFlag(chan_i,row);
			}
		}
	}

	return;
}

} //# NAMESPACE CASA - END


