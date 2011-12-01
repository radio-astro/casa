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

FlagAgentShadow::FlagAgentShadow(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube):
		FlagAgentBase(dh,config,ROWS,writePrivateFlagCube)
{
	setAgentParameters(config);

	// Request loading antenna1,antenna2 and uvw
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant1);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant2);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Uvw);
}

FlagAgentShadow::~FlagAgentShadow()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void
FlagAgentShadow::setAgentParameters(Record config)
{
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

	*logger_p << LogIO::NORMAL << "FlagAgentShadow::" << __FUNCTION__ << " diameter is " << antennaDiameter_p << LogIO::POST;

	return;
}

void
FlagAgentShadow::computeRowFlags(VisBuffer &visBuffer, FlagMapper &flags, uInt row)
{
	// First check if this row corresponds to autocorrelation
	// (Antennas don't shadow themselves)
	Int antenna1 = visBuffer.antenna1()[row];
	Int antenna2 = visBuffer.antenna2()[row];
	if (antenna1 == antenna2) return;

	// Get antenna diameter
	Double antennaDiameter1,antennaDiameter2;
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
	Double antennaDistance = (antennaDiameter1+antennaDiameter2)*(antennaDiameter1+antennaDiameter2)/4.0;

	// Compute uv distance
	Double u = visBuffer.uvw()(row)(0);
	Double v = visBuffer.uvw()(row)(1);
	Double uvDistance = u*u + v*v;

	// Check if the baseline is shadowed
	if (uvDistance < antennaDistance)
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


