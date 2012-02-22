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

FlagAgentClipping::FlagAgentClipping(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube, Bool flag):
		FlagAgentBase(dh,config,IN_ROWS,writePrivateFlagCube,flag)
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
	logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

	int exists;

	// First check basic configuration to see what modules are plugged in the check
	exists = config.fieldNumber ("clipzeros");
	if (exists >= 0)
	{
		clipzeros_p = config.asBool("clipzeros");
	}
	else
	{
		clipzeros_p = False;
	}

	*logger_p << logLevel_p << " clipzeros is " << clipzeros_p << LogIO::POST;


	exists = config.fieldNumber ("clipoutside");
	if (exists >= 0)
	{
		clipoutside_p = config.asBool("clipoutside");
	}
	else
	{
		clipoutside_p = True;
	}

	*logger_p << logLevel_p << " clipoutside is " << clipoutside_p << LogIO::POST;


	exists = config.fieldNumber ("clipminmax");
	if (exists >= 0)
	{
		Array<Double> cliprange = config.asArrayDouble("clipminmax");
		if (cliprange.size()==2)
		{
			Bool deleteIt = False;
			clipmin_p = cliprange.getStorage(deleteIt)[0];
			clipmax_p = cliprange.getStorage(deleteIt)[1];
			*logger_p << logLevel_p << " clipmin is " << clipmin_p << LogIO::POST;
			*logger_p << logLevel_p << " clipmax is " << clipmax_p << LogIO::POST;

			clipminmax_p = true;
		}
		else
		{
			clipminmax_p = false;
			*logger_p << logLevel_p << " clipminmax range not provided" << LogIO::POST;
		}
	}
	else
	{
		clipminmax_p = false;
		*logger_p << logLevel_p << " clipminmax range not provided" << LogIO::POST;
	}


	// Then, point to the function covering for all the modules selected
	if (clipminmax_p)
	{
		if (clipoutside_p)
		{
			if (clipzeros_p)
			{
				checkVis_p = &FlagAgentClipping::checkVisForClipOutsideAndZeros;
				*logger_p << logLevel_p << " Clipping outside [" <<  clipmin_p << "'" <<  clipmax_p << "], plus NaNs and zeros" << LogIO::POST;
			}
			else
			{
				checkVis_p = &FlagAgentClipping::checkVisForClipOutside;
				*logger_p << logLevel_p << " Clipping outside [" <<  clipmin_p << "'" <<  clipmax_p << "], plus NaNs" << LogIO::POST;
			}
		}
		else
		{
			if (clipzeros_p)
			{
				checkVis_p = &FlagAgentClipping::checkVisForClipInsideAndZeros;
				*logger_p << logLevel_p << " Clipping inside [" <<  clipmin_p << "'" <<  clipmax_p << "], plus NaNs and zeros" << LogIO::POST;
			}
			else
			{
				checkVis_p = &FlagAgentClipping::checkVisForClipInside;
				*logger_p << logLevel_p << " Clipping inside [" <<  clipmin_p << "'" <<  clipmax_p << "], plus NaNs" << LogIO::POST;
			}
		}
	}
	else
	{
		if (clipzeros_p)
		{
			checkVis_p = &FlagAgentClipping::checkVisForNaNsAndZeros;
			*logger_p << logLevel_p << " Clipping range not provided, clipping NaNs and zeros" << LogIO::POST;
		}
		else
		{
			checkVis_p = &FlagAgentClipping::checkVisForNaNs;
			*logger_p << logLevel_p << " Clipping range not provided, clipping only NaNs" << LogIO::POST;
		}
	}


	// Finally, get channel average which is a neutral parameter
	exists = config.fieldNumber ("channelavg");
	if (exists >= 0)
	{
		channelavg_p = config.asBool("channelavg");
	}
	else
	{
		channelavg_p = False;
	}

	*logger_p << logLevel_p << " channelavg is " << channelavg_p << LogIO::POST;


	return;
}

bool
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

		// If visExpression is out of range we flag the entire row
		if (nAverage > 0)
		{
			visExpression /= nAverage;
			if ((*this.*checkVis_p)(visExpression))
			{
				for (chan_i=0;chan_i<nChannels;chan_i++)
				{
					flags.applyFlag(chan_i,row);
				}
				visBufferFlags_p += flags.flagsPerRow();
			}
		}
	}
	else
	{
		for (chan_i=0;chan_i<nChannels;chan_i++)
		{
			visExpression = visibilities(chan_i,row);
			if ((*this.*checkVis_p)(visExpression))
			{
				/*
				Complex complexVis = visibilities.correlationProduct(0,chan_i,row);
				cout 	<< " row=" << row
						<< " chan=" << chan_i
						<< " vis.real=" << complexVis.real()
						<< " vis.imag=" << complexVis.imag()
						<< " visExpression=" << visExpression << endl;
						*/
				flags.applyFlag(chan_i,row);
				visBufferFlags_p += flags.nSelectedCorrelations();
			}
		}
	}

	return false;
}

bool
FlagAgentClipping::checkVisForClipOutside(Float visExpression)
{
	if ((visExpression >  clipmax_p) or (visExpression <  clipmin_p))
	{
		return true;
	}
	else
	{
		return isNaN(visExpression);
	}
}

bool
FlagAgentClipping::checkVisForClipInside(Float visExpression)
{
	if ((visExpression <=  clipmax_p) and (visExpression >=  clipmin_p))
	{
		return true;
	}
	else
	{
		return isNaN(visExpression);
	}
}

bool
FlagAgentClipping::checkVisForClipOutsideAndZeros(Float visExpression)
{
	if ((visExpression >  clipmax_p) or (visExpression <  clipmin_p))
	{
		return true;
	}
	else if ((clipzeros_p) and isZero(visExpression))
	{
		return true;
	}
	else
	{
		return isNaN(visExpression);
	}
}

bool
FlagAgentClipping::checkVisForClipInsideAndZeros(Float visExpression)
{
	if ((visExpression <=  clipmax_p) and (visExpression >=  clipmin_p))
	{
		return true;
	}
	else if ((clipzeros_p) and isZero(visExpression))
	{
		return true;
	}
	else
	{
		return isNaN(visExpression);
	}
}

bool
FlagAgentClipping::checkVisForNaNs(Float visExpression)
{
	return isNaN(visExpression);
}

bool
FlagAgentClipping::checkVisForNaNsAndZeros(Float visExpression)
{
	return (isZero(visExpression) or isNaN(visExpression));
}

} //# NAMESPACE CASA - END


