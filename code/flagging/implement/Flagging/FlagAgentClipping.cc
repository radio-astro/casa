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
}

FlagAgentClipping::~FlagAgentClipping()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void
FlagAgentClipping::setAgentParameters(Record config)
{
	int exists;

	exists = config.fieldNumber ("clipmin");
	if (exists >= 0)
	{
		clipmin_p = atof(config.asString("clipmin").c_str());
	}
	else
	{
		clipmin_p = 0.0;
	}

	*logger_p << LogIO::NORMAL << "FlagAgentClipping::" << __FUNCTION__ << " clipmin is " << clipmin_p << LogIO::POST;

	exists = config.fieldNumber ("clipmax");
	if (exists >= 0)
	{
		clipmax_p = atof(config.asString("clipmax").c_str());
	}
	else
	{
		clipmax_p = 1.5;
	}

	*logger_p << LogIO::NORMAL << "FlagAgentClipping::" << __FUNCTION__ << " clipmax is " << clipmax_p << LogIO::POST;


	return;
}

void
FlagAgentClipping::computeInRowFlags(VisMapper &visibilities,FlagMapper &flags,uInt channel, uInt row)
{
	Float visExpression = visibilities(channel,row);
	if ((visExpression <  clipmin_p) or (visExpression >  clipmin_p))
	{
		flags.applyFlag(channel,row);
	}

	return;
}



} //# NAMESPACE CASA - END


