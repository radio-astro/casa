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
		FlagAgentBase(dh,config,ROWS_PREPROCESS_BUFFER,false)
{
	arrayId = 0;
	fieldId = 0;
	spw = 0;
	scan = 0;
	observationId = 0;

	arrayId_str = String("");;
	fieldId_str = String("");;
	spw_str = String("");;
	scan_str = String("");;
	observationId_str = String("");;

	accumTotalFlags = 0;
	accumTotalCount = 0;
	spwChannelCounts = False;
	spwPolarizationCounts = False;

	setAgentParameters(config);

	// Request loading polarization map to FlagDataHandler
	flagDataHandler_p->setMapPolarizations(true);
}

FlagAgentSummary::~FlagAgentSummary()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void
FlagAgentSummary::setAgentParameters(Record config)
{

	int exists;

	exists = config.fieldNumber ("spwchan");
	if (exists >= 0)
	{
		spwChannelCounts = config.asBool("spwchan");
	}
	else
	{
		spwChannelCounts = False;
	}

	if (spwChannelCounts)
	{
		*logger_p << LogIO::NORMAL << "FlagAgentSummary::" << __FUNCTION__ << " Spw-Channel count activated " << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentSummary::" << __FUNCTION__ << " Spw-Channel count deactivated " << LogIO::POST;
	}

	exists = config.fieldNumber ("spwcorr");
	if (exists >= 0)
	{
		spwPolarizationCounts = config.asBool("spwcorr");
	}
	else
	{
		spwPolarizationCounts = False;
	}

	if (spwPolarizationCounts)
	{
		*logger_p << LogIO::NORMAL << "FlagAgentSummary::" << __FUNCTION__ << " Spw-Correlation count activated " << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentSummary::" << __FUNCTION__ << " Spw-Correlation count deactivated " << LogIO::POST;
	}

	return;
}

void
FlagAgentSummary::preProcessBuffer(VisBuffer &visBuffer)
{
	arrayId = visBuffer.arrayId();
	stringstream arrayId_stringStream;
	arrayId_stringStream << arrayId;
	arrayId_str = arrayId_stringStream.str();

	fieldId = visBuffer.fieldId();
	stringstream fieldId_stringStream;
	fieldId_stringStream << fieldId;
	fieldId_str = fieldId_stringStream.str();

	spw = visBuffer.spectralWindow();
	stringstream spw_stringStream;
	spw_stringStream << spw;
	spw_str = spw_stringStream.str();

	// TODO: This is not generic but in all the iteration modes provided
	// by the FlagDataHandler scan and observation are constant over rows
	scan = visBuffer.scan()[0];
	stringstream scan_stringStream;
	scan_stringStream << scan;
	scan_str = scan_stringStream.str();

	observationId = visBuffer.observationId()[0];
	stringstream observationId_stringStream;
	observationId_stringStream << observationId;
	observationId_str = observationId_stringStream.str();

	return;
}

void
FlagAgentSummary::computeRowFlags(VisBuffer &visBuffer, FlagMapper &flags, uInt row)
{
	Int antenna1 = visBuffer.antenna1()[row];
	Int antenna2 = visBuffer.antenna2()[row];
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
	Int pol_i = 0;;
	vector<uInt64> polarizationsBreakdownFlags;
	for (pol_i=0;pol_i < nPolarizations;pol_i++)
	{
		polarizationsBreakdownFlags.push_back(0);
	}

	// Iterate trough channels
	Bool flag;
	Int channel_i = 0;
	uInt64 rowFlags = 0;
	uInt64 channelFlags = 0;
	for (channel_i=0;channel_i<nChannels;channel_i++)
	{
		channelFlags = 0;
		for (pol_i=0;pol_i < nPolarizations;pol_i++)
		{
			flag = flags.getModifiedFlags(polarizations[pol_i],channel_i,row);
			channelFlags += flag;
			polarizationsBreakdownFlags[pol_i] += flag;
		}
		rowFlags += channelFlags;

		if (spwChannelCounts)
		{
			accumChanneltotal[spw][channel_i] += nPolarizations;
			accumChannelflags[spw][channel_i] += channelFlags;
		}
	}

	// Update polarization counts
	polarizationIndexMap *toPolarizationIndexMap = flagDataHandler_p->getPolarizationIndexMap();
	String polarization_str;
	for (pol_i=0;pol_i < nPolarizations;pol_i++)
	{
		polarization_str = (*toPolarizationIndexMap)[polarizations[pol_i]];
		accumtotal["correlation"][polarization_str] += nChannels;
		accumflags["correlation"][polarization_str] += polarizationsBreakdownFlags[pol_i];

		if (spwPolarizationCounts)
		{
			accumPolarizationtotal[spw][polarization_str] += nChannels;
			accumPolarizationflags[spw][polarization_str] += polarizationsBreakdownFlags[pol_i];
		}
	}

	// Update row counts
	accumtotal["array"][arrayId_str] += rowTotal;
	accumflags["array"][arrayId_str] += rowFlags;

	accumtotal["field"][fieldId_str] += rowTotal;
	accumflags["field"][fieldId_str] += rowFlags;

	accumtotal["spw"][spw_str] += rowTotal;
	accumflags["spw"][spw_str] += rowFlags;

	accumtotal["scan"][scan_str] += rowTotal;
	accumflags["scan"][scan_str] += rowFlags;

	accumtotal["observation"][observationId_str] += rowTotal;
	accumflags["observation"][observationId_str] += rowFlags;

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

	if (spwChannelCounts)
	{
		for (map<Int, map<uInt, uInt64> >::iterator key1 = accumChanneltotal.begin();key1 != accumChanneltotal.end();key1++)
		{
			Record stats_key1;
			for (map<uInt, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
			{
				Record stats_key2;

				stats_key2.define("flagged", (uInt) accumChannelflags[key1->first][key2->first]);
				stats_key2.define("total", (uInt) key2->second);
				stats_key1.defineRecord(String(key2->first), stats_key2);

				*logger_p 	<< LogIO::NORMAL << "FlagAgentSummary::" << __FUNCTION__
						<< " Spw:" << key1->first << " Channel:" << key2->first
						<< " flagged: " <<  (uInt) accumChannelflags[key1->first][key2->first]
						<< " total: " <<  (uInt) key2->second
						<< LogIO::POST;
			}

			result.defineRecord(String(key1->first), stats_key1);
		}
	}

	if (spwPolarizationCounts)
	{
		for (map<Int, map<string, uInt64> >::iterator key1 = accumPolarizationtotal.begin();key1 != accumPolarizationtotal.end();key1++)
		{
			Record stats_key1;
			for (map<string, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
			{
				Record stats_key2;

				stats_key2.define("flagged", (uInt) accumPolarizationflags[key1->first][key2->first]);
				stats_key2.define("total", (uInt) key2->second);
				stats_key1.defineRecord(key2->first, stats_key2);

				*logger_p 	<< LogIO::NORMAL << "FlagAgentSummary::" << __FUNCTION__
						<< " Spw:" << key1->first << " Correlation:" << key2->first
						<< " flagged: " <<  (uInt) accumPolarizationflags[key1->first][key2->first]
						<< " total: " <<  (uInt) key2->second
						<< LogIO::POST;
			}

			result.defineRecord(String(key1->first), stats_key1);
		}
	}

	for (map<string, map<string, uInt64> >::iterator key1 = accumtotal.begin();key1 != accumtotal.end();key1++)
	{
		Record stats_key1;
		for (map<string, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
		{
			Record stats_key2;

			stats_key2.define("flagged", (uInt) accumflags[key1->first][key2->first]);
			stats_key2.define("total", (uInt) key2->second);
			stats_key1.defineRecord(key2->first, stats_key2);

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


