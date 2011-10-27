//# FlagAgentSummary.cc: This file contains the implementation of the FlagAgentSummary class.
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

#include <flagging/Flagging/FlagAgentSummary.h>

namespace casa { //# NAMESPACE CASA - BEGIN

FlagAgentSummary::FlagAgentSummary(FlagDataHandler *dh, Record config):
		FlagAgentBase(dh,config,ROWS,false)
{
	setAgentParameters(config);
	accumTotalFlags = 0;
	accumTotalCount = 0;
	spwChannelCounts = False;
	spwPolarizationCounts = False;
}

FlagAgentSummary::~FlagAgentSummary()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void
FlagAgentSummary::setAgentParameters(Record config)
{


	return;
}

void
FlagAgentSummary::computeRowFlags(FlagMapper &flags, uInt row)
{
	Int arrayId = visibilityBuffer_p->get()->arrayId();
	Int fieldId = visibilityBuffer_p->get()->fieldId();
	Int spw = visibilityBuffer_p->get()->spectralWindow();
	Int scan = visibilityBuffer_p->get()->scan()[row];
	Int observationId = visibilityBuffer_p->get()->observationId()[row];

	stringstream arrayId_str, fieldId_str, spw_str, scan_str, observationId_str;
	arrayId_str << arrayId;
	fieldId_str << fieldId;
	spw_str << spw;
	scan_str << scan;
	observationId_str << observationId;

	Int antenna1 = visibilityBuffer_p->get()->antenna1()[row];
	Int antenna2 = visibilityBuffer_p->get()->antenna2()[row];
	String antenna1Name = flagDataHandler_p->antennaNames_p->operator()(antenna1);
	String antenna2Name = flagDataHandler_p->antennaNames_p->operator()(antenna2);
    String baseline = antenna1Name + "&&" + antenna2Name;

    // Compute totals
	Int nChannels,nRows;
	flags.shape(nChannels,nRows);
    vector<uInt> polarizations = flags.getSelectedCorrelations();
	Int nPolarizations = polarizations.size();
    uInt64 rowTotal = nChannels*nPolarizations;

	// Initialize polarization counts
	uInt pol_i = 0;;
	vector<uInt64> polarizationsBreakdownFlags;
	for (pol_i=0;pol_i < nPolarizations;pol_i++)
	{
		polarizationsBreakdownFlags.push_back(0);
	}

	// Iterate trough channels
	Bool flag;
	uInt channel_i = 0;
	uInt64 rowFlags = 0;
	uInt64 channelFlags = 0;
	for (channel_i=0;channel_i<nChannels;channel_i++)
	{
		channelFlags = 0;
		for (pol_i=0;pol_i < polarizations.size();pol_i++)
		{
			flag = flags(polarizations[pol_i],channel_i,row);
			channelFlags += flag;
			polarizationsBreakdownFlags[pol_i] += flag;
		}
		rowFlags += channelFlags;

		if (spwChannelCounts)
		{
			accumChannelflags[spw][channel_i] += channelFlags;
			accumChanneltotal[spw][channel_i] += nPolarizations;
		}
	}

	// Update polarization counts
	polarizationIndexMap *toPolarizationIndexMap = flagDataHandler_p->getPolarizationIndexMap();
	String polarization_str;
	for (pol_i=0;pol_i < polarizations.size();pol_i++)
	{
		polarization_str = (*toPolarizationIndexMap)[polarizations[pol_i]];
		accumtotal["correlation"][polarization_str] += nChannels;
		accumflags["correlation"][polarization_str] += polarizationsBreakdownFlags[pol_i];

		if (spwPolarizationCounts)
		{
			accumPolarizationflags[spw][polarization_str] += nChannels;
			accumPolarizationtotal[spw][polarization_str] += polarizationsBreakdownFlags[pol_i];
		}
	}

	// Update row counts
	accumtotal["array"][arrayId_str.str()] += rowTotal;
	accumflags["array"][arrayId_str.str()] += rowFlags;

	accumtotal["field"][fieldId_str.str()] += rowTotal;
	accumflags["field"][fieldId_str.str()] += rowFlags;

	accumtotal["spw"][spw_str.str()] += rowTotal;
	accumflags["spw"][spw_str.str()] += rowFlags;

	accumtotal["scan"][scan_str.str()] += rowTotal;
	accumflags["scan"][scan_str.str()] += rowFlags;

	accumtotal["observation"][observationId_str.str()] += rowTotal;
	accumflags["observation"][observationId_str.str()] += rowFlags;

	accumtotal["antenna"][antenna1Name] += rowTotal;
	accumflags["antenna"][antenna1Name] += rowFlags;

	if (antenna1 != antenna2)
	{
		accumtotal["antenna"][antenna2Name] += rowTotal;
		accumflags["antenna"][antenna2Name] += rowFlags;
	}

	accumtotal["baseline"][baseline] += rowTotal;
	accumflags["baseline"][baseline] += rowFlags;

	return;
}

Record
FlagAgentSummary::getResult()
{
	Record result;
	result.define("flagged", (uInt) accumTotalFlags);
	result.define("total"  , (uInt) accumTotalCount);

	for (map<string, map<string, uInt64> >::iterator key1 = accumtotal.begin();key1 != accumtotal.end();key1++)
	{
		Record stats_key1;
		for (map<string, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
		{
			Record stats_key2;

			stats_key2.define("flagged", (uInt) accumflags[key1->first][key2->first]);
			stats_key2.define("total", (uInt) key2->second);
			stats_key1.defineRecord(key1->first, stats_key2);

			*logger_p 	<< LogIO::NORMAL << "FlagAgentSummary::" << __FUNCTION__
					<< " " << key1->first << " " << key2->first
					<< " flagged: " <<  (uInt) accumflags[key1->first][key2->first]
					<< " total: " <<  (uInt) key2->second
					<< LogIO::POST;
		}

		result.defineRecord(key1->first, stats_key1);
	}

	return result;
}

} //# NAMESPACE CASA - END


