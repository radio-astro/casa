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

FlagAgentElevation::FlagAgentElevation(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube):
		FlagAgentBase(dh,config,ROWS,writePrivateFlagCube)
{
	setAgentParameters(config);

	// Request loading antenna pointing map to FlagDataHandler
	flagDataHandler_p->setMapAntennaPointing(true);
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

	*logger_p << LogIO::NORMAL << "FlagAgentElevation::" << __FUNCTION__ << " lowerlimit is " << lowerlimit_p << LogIO::POST;

	exists = config.fieldNumber ("upperlimit");
	if (exists >= 0)
	{
		upperlimit_p = config.asDouble("upperlimit");
	}
	else
	{
		upperlimit_p = 90.0;
	}

	*logger_p << LogIO::NORMAL << "FlagAgentElevation::" << __FUNCTION__ << " upperlimit is " << upperlimit_p << LogIO::POST;


	return;
}

void
FlagAgentElevation::computeRowFlags(VisBuffer &visBuffer, FlagMapper &flags, uInt row)
{
    double antenna1_elevation = flagDataHandler_p->getMapAntennaPointing()->at(row).at(0);
    double antenna2_elevation = flagDataHandler_p->getMapAntennaPointing()->at(row).at(1);

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
    }

	return;
}

} //# NAMESPACE CASA - END


