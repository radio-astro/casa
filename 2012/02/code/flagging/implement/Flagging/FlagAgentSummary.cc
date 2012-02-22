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

	arrayId_str = String("");
	fieldId_str = String("");
	spw_str = String("");
	scan_str = String("");
	observationId_str = String("");

	accumTotalFlags = 0;
	accumTotalCount = 0;
	spwChannelCounts = False;
	spwPolarizationCounts = False;

	setAgentParameters(config);

	// Request loading polarization map to FlagDataHandler
	flagDataHandler_p->setMapPolarizations(true);

	// Request pre-loading array,field,spw, scan, observation, antenna1, antenna2
	flagDataHandler_p->preLoadColumn(VisBufferComponents::ArrayId);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::FieldId);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Scan);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::ObservationId);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::SpW);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant1);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant2);
}

FlagAgentSummary::~FlagAgentSummary()
{
	// Compiler automagically calls FlagAgentBase::~FlagAgentBase()
}

void
FlagAgentSummary::setAgentParameters(Record config)
{
        logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

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
		*logger_p << LogIO::NORMAL << " Spw-Channel count activated " << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << " Spw-Channel count deactivated " << LogIO::POST;
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
		*logger_p << LogIO::NORMAL << " Spw-Correlation count activated " << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << " Spw-Correlation count deactivated " << LogIO::POST;
	}

	return;
}

void
FlagAgentSummary::preProcessBuffer(const VisBuffer &visBuffer)
{
	arrayId = visBuffer.arrayId();
	stringstream arrayId_stringStream;
	arrayId_stringStream << arrayId;
	arrayId_str = arrayId_stringStream.str();

	fieldId = visBuffer.fieldId();
	// Transform fieldId into field name using the corresponding subtable
	fieldId_str = flagDataHandler_p->fieldNames_p->operator()(fieldId);

	spw = visBuffer.spectralWindow();
	stringstream spw_stringStream;
	spw_stringStream << spw;
	spw_str = spw_stringStream.str();

	observationId = visBuffer.observationId()[0];
	stringstream observationId_stringStream;
	observationId_stringStream << observationId;
	observationId_str = observationId_stringStream.str();

	return;
}

bool
FlagAgentSummary::computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
{
	Int antenna1 = visBuffer.antenna1()[row];
	Int antenna2 = visBuffer.antenna2()[row];
	String antenna1Name = flagDataHandler_p->antennaNames_p->operator()(antenna1);
	String antenna2Name = flagDataHandler_p->antennaNames_p->operator()(antenna2);
    String baseline = antenna1Name + "&&" + antenna2Name;

    // Get scan for each particular row to cover for the "combine scans" case
	scan = visBuffer.scan()[row];
	stringstream scan_stringStream;
	scan_stringStream << scan;
	scan_str = scan_stringStream.str();

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
			flag = flags.getModifiedFlags(pol_i,channel_i,row);
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

	accumTotalFlags += rowFlags;
	accumTotalCount += rowTotal;

	return false;
}

FlagReport
FlagAgentSummary::getReport()
{
  // // Later, this function should return 'views' if they have been asked for.
  // // In this case, make a 'list' type FlagReport, and do an addReport().
  //
  //         // Make the flagreport list
  //         FlagReport summarylist("list");  
  //
  //         // Add the standard summary dictionary as a report of type 'summary'
  //         summarylist.addReport( FlagReport("summary", agentName_p, getResult())  );
  //
  //         // Make a report for a view, and add it to the list
  //         FlagReport viewrep("plotline",agentName_p,"title","xaxis","yaxis")
  //         viewrep.addData(xdata,ydata,"label");
  //         summarylist.addReport( viewRep );        
  //
  //         return summarylist;
  //
  // Note : Calculate these extra views only if the user has asked for it.
  //

  return FlagReport("summary", agentName_p, getResult());
}

Record
FlagAgentSummary::getResult()
{
        logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
	Record result;

	if (spwChannelCounts)
	{
		Record stats_key1;

		for (map<Int, map<uInt, uInt64> >::iterator key1 = accumChanneltotal.begin();key1 != accumChanneltotal.end();key1++)
		{
			// Transform spw id into string
			stringstream spw_stringStream;
			spw_stringStream << key1->first;

			for (map<uInt, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
			{
				Record stats_key2;

				stats_key2.define("flagged", (Double) accumChannelflags[key1->first][key2->first]);
				stats_key2.define("total", (Double) key2->second);

				// Transform channel id into string
				stringstream channel_stringStream;
				channel_stringStream << key2->first;

				// Construct spw:channel string as first key
				stats_key1.defineRecord(spw_stringStream.str() + ":" + channel_stringStream.str(), stats_key2);
				// Calculate percentage flagged
				stringstream percentage;
				percentage.precision(3);percentage.fixed;
				if( key2->second > 0 )
				{
				        percentage << " (" << 100.0 * 
                                                               (Double) accumChannelflags[key1->first][key2->first]/
				                               (Double) key2->second << "%)";
				}

				*logger_p 	<< LogIO::NORMAL 
						<< " Spw:" << key1->first << " Channel:" << key2->first
						<< " flagged: " <<  (Double) accumChannelflags[key1->first][key2->first]
						<< " total: " <<  (Double) key2->second
				                << percentage.str() 
						<< LogIO::POST;
			}

		}

		result.defineRecord("spw:channel", stats_key1);
	}

	if (spwPolarizationCounts)
	{
		Record stats_key1;

		for (map<Int, map<string, uInt64> >::iterator key1 = accumPolarizationtotal.begin();key1 != accumPolarizationtotal.end();key1++)
		{
			// Transform spw id into string
			stringstream spw_stringStream;
			spw_stringStream << key1->first;

			for (map<string, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
			{
				Record stats_key2;

				stats_key2.define("flagged", (Double) accumPolarizationflags[key1->first][key2->first]);
				stats_key2.define("total", (Double) key2->second);

				// Construct spw:correlation string as first key (Polarization already comes as a string)
				stats_key1.defineRecord(spw_stringStream.str() + ":" + key2->first, stats_key2);

				// Calculate percentage flagged
				stringstream percentage;
				percentage.precision(3);percentage.fixed;
				if( key2->second > 0 )
				{
				  percentage << " (" << 100.0 * 
                                                        (Double) accumPolarizationflags[key1->first][key2->first]/
				    (Double) key2->second << "%)";
				}

				*logger_p 	<< LogIO::NORMAL 
						<< " Spw:" << key1->first << " Correlation:" << key2->first
						<< " flagged: " <<  (Double) accumPolarizationflags[key1->first][key2->first]
						<< " total: " <<  (Double) key2->second
				                << percentage.str()
						<< LogIO::POST;
			}
		}

		result.defineRecord("spw:correlation", stats_key1);
	}

	for (map<string, map<string, uInt64> >::iterator key1 = accumtotal.begin();key1 != accumtotal.end();key1++)
	{
		Record stats_key1;
		for (map<string, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
		{
			Record stats_key2;

			stats_key2.define("flagged", (Double) accumflags[key1->first][key2->first]);
			stats_key2.define("total", (Double) key2->second);
			stats_key1.defineRecord(key2->first, stats_key2);

			// Calculate percentage flagged
			stringstream percentage;
			percentage.precision(3);percentage.fixed;
			if( key2->second > 0 )
			{
			  percentage << " (" << 100.0 * 
                                                 (Double) accumflags[key1->first][key2->first] /
			                         (Double) key2->second << "%)";
			}

			*logger_p 	<< LogIO::NORMAL 
					<< " " << key1->first << " " << key2->first
					<< " flagged: " <<  (Double) accumflags[key1->first][key2->first]
					<< " total: " <<  (Double) key2->second
			                << percentage.str()
					<< LogIO::POST;
		}

		result.defineRecord(key1->first, stats_key1);
	}

	result.define("flagged", (Double) accumTotalFlags);
	result.define("total"  , (Double) accumTotalCount);

        // Calculate percentage flagged
	stringstream percentage;
	percentage.precision(3);percentage.fixed;
	if( accumTotalCount > 0 )
	{
 	      percentage << " (" << 100.0 * 
                                     (Double) accumTotalFlags / 
		                     (Double) accumTotalCount << "%)";
	}
	*logger_p 	<< LogIO::NORMAL 
			<< " Total Flagged: " <<  (Double) accumTotalFlags
			<< " Total Counts: " <<  (Double) accumTotalCount
			<< percentage.str()
			<< LogIO::POST;

	return result;
}

} //# NAMESPACE CASA - END


