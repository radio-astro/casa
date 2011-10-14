//# FlagAgentTimeFreqCrop.cc: This file contains the implementation of the FlagAgentTimeFreqCrop class.
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

#include <flagging/Flagging/FlagAgentTimeFreqCrop.h>

namespace casa { //# NAMESPACE CASA - BEGIN

FlagAgentTimeFreqCrop::FlagAgentTimeFreqCrop(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube):
		FlagAgentBase(dh,config,ANTENNA_PAIRS,writePrivateFlagCube)
{
	setAgentParameters(config);
}

FlagAgentTimeFreqCrop::~FlagAgentTimeFreqCrop()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void FlagAgentTimeFreqCrop::setAgentParameters(Record config)
{
	int exists;

	exists = config.fieldNumber ("time_amp_cutoff");
	if (exists >= 0)
	{
		T_TOL_p = atof(config.asString("time_amp_cutoff").c_str());
	}
	else
	{
		T_TOL_p = 4.0;
	}

	*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__ << " time_amp_cutoff is " << T_TOL_p << LogIO::POST;

	exists = config.fieldNumber ("freq_amp_cutoff");
	if (exists >= 0)
	{
		F_TOL_p = atof(config.asString("freq_amp_cutoff").c_str());
	}
	else
	{
		F_TOL_p = 3.0;
	}

	*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__ << " freq_amp_cutoff is " << F_TOL_p << LogIO::POST;

	exists = config.fieldNumber ("maxnpieces");
	if (exists >= 0)
	{
		MaxNPieces_p = atoi(config.asString("maxnpieces").c_str());

		if ((MaxNPieces_p<1) or (MaxNPieces_p>9))
		{
			*logger_p << LogIO::WARN << "FlagAgentTimeFreqCrop::" << __FUNCTION__ <<
					" Unsupported maxnpieces: " <<
					MaxNPieces_p << ", using 7 by default. Supported values: 1-9" << LogIO::POST;
			MaxNPieces_p = 7;
		}
	}
	else
	{
		MaxNPieces_p = 7;
	}

	*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__ << " maxnpieces is " << MaxNPieces_p << LogIO::POST;

	exists = config.fieldNumber ("timefit");
	if (exists >= 0)
	{
		timeFitType_p = config.asString("timefit");
		if ((timeFitType_p.compare("line") != 0) and (timeFitType_p.compare("poly") != 0))
		{
			*logger_p << LogIO::WARN << "FlagAgentTimeFreqCrop::" << __FUNCTION__ <<
					" Unsupported timefit: " <<
					timeFitType_p << ", using line by default. Supported modes: line,poly" << LogIO::POST;
			timeFitType_p = "line";
		}

	}
	else
	{
		timeFitType_p = "line";
	}

	*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__ << " timefit is " << timeFitType_p << LogIO::POST;

	exists = config.fieldNumber ("freqfit");
	if (exists >= 0)
	{
		freqFitType_p = config.asString("freqfit");
		if ((freqFitType_p.compare("line") != 0) and (freqFitType_p.compare("poly") != 0))
		{
			*logger_p << LogIO::WARN << "FlagAgentTimeFreqCrop::" << __FUNCTION__ <<
					" Unsupported freqfit " <<
					freqFitType_p << ", using line by default. Supported modes: line,poly" << LogIO::POST;
			freqFitType_p = "line";
		}
	}
	else
	{
		freqFitType_p = "poly";
	}

	*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__ << " freqfit is " << freqFitType_p << LogIO::POST;

	exists = config.fieldNumber ("flagdimension");
	if (exists >= 0)
	{
		flagDimension_p = config.asString("flagdimension");
		if ((flagDimension_p.compare("time") != 0) and (flagDimension_p.compare("freq") != 0)
				and (flagDimension_p.compare("timefreq") != 0) and (flagDimension_p.compare("freqtime") != 0))
		{
			*logger_p << LogIO::WARN << "FlagAgentTimeFreqCrop::" << __FUNCTION__ <<
					" Unsupported flagdimension " <<
					flagDimension_p << ", using line by default. Supported modes: time,freq,timefreq,freqtime" << LogIO::POST;
			flagDimension_p = "freqtime";
		}
	}
	else
	{
		flagDimension_p = "freqtime";
	}

	*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__ << " flagdimension is " << flagDimension_p << LogIO::POST;

	exists = config.fieldNumber ("halfwin");
	if (exists >= 0)
	{
		halfWin_p = atoi(config.asString("halfwin").c_str());
		if ((halfWin_p < 1) or (halfWin_p > 3))
		{
			*logger_p << LogIO::WARN << "FlagAgentTimeFreqCrop::" << __FUNCTION__ <<
					" Unsupported halfWin_p " <<
					halfWin_p << ", using 1 by default. Supported values: 1,2,3" << LogIO::POST;
			halfWin_p = 1;
		}
	}
	else
	{
		halfWin_p = 1;
	}

	*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__ << " halfwin is " << halfWin_p << LogIO::POST;

	exists = config.fieldNumber ("usewindowstats");
	if (exists >= 0)
	{
		winStats_p = config.asString("usewindowstats");
		if ((winStats_p.compare("none") != 0) and (winStats_p.compare("sum") != 0)
				and (winStats_p.compare("std") != 0) and (winStats_p.compare("both") != 0))
		{
			*logger_p << LogIO::WARN << "FlagAgentTimeFreqCrop::" << __FUNCTION__ <<
					" Unsupported usewindowstats " <<
					winStats_p << ", using none by default. Supported modes: none,sum,std,both" << LogIO::POST;
			winStats_p = "none";
		}
	}
	else
	{
		winStats_p = "none";
	}

	*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__ << " usewindowstats is " << winStats_p << LogIO::POST;

	return;
}

void
FlagAgentTimeFreqCrop::computeAntennaPairFlags(VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2)
{

	IPosition flagCubeShape = visibilities.shape();

	// Some logging info
	if (multiThreading_p)
	{
		*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__
				<<  " Thread Id " << threadId_p << ":" << nThreads_p
				<<  " Processing [freq,time] data cube for baseline ("
				<< antenna1 << "," << antenna2 << ") with shape " << flagCubeShape << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentTimeFreqCrop::" << __FUNCTION__
				<<  " Processing [freq,time] data cube for baseline ("
				<< antenna1 << "," << antenna2 << ") with shape " << flagCubeShape << LogIO::POST;
	}

	uInt nChannels,nRows;
	nChannels = flagCubeShape(0);
	nRows = flagCubeShape(1);
	Float vis;
	uInt row_i,chan_i;
	for (row_i=0;row_i<nRows;row_i++)
	{
		for (chan_i=0;chan_i<nChannels;chan_i++)
		{
			// Get mapped visibility value
			vis = visibilities(chan_i,row_i);

			// ... RFI detection algorithm ...

			// Set flags in all correlations involved
			flags.applyFlag(chan_i,row_i);
		}
	}
	return;
}


} //# NAMESPACE CASA - END


