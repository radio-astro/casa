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

    spwChannelCounts = False;
    spwPolarizationCounts = False;
    baselineCounts = False;
    fieldCounts = False;

    setAgentParameters(config);

    currentSummary = NULL;
    fieldSummaryMap.clear();
    if (fieldCounts)
    {
    	currentSummary = NULL;
    }
    else
    {
    	currentSummary = new summary();
    }

    // Request loading polarization map to FlagDataHandler
    flagDataHandler_p->setMapPolarizations(true);

    // Request pre-loading array,field,spw, scan, observation, antenna1, antenna2
    flagDataHandler_p->preLoadColumn(vi::ArrayId);
    flagDataHandler_p->preLoadColumn(vi::FieldId);
    flagDataHandler_p->preLoadColumn(vi::Scan);
    flagDataHandler_p->preLoadColumn(vi::ObservationId);
    flagDataHandler_p->preLoadColumn(vi::SpectralWindows);
    flagDataHandler_p->preLoadColumn(vi::Antenna1);
    flagDataHandler_p->preLoadColumn(vi::Antenna2);
    //flagDataHandler_p->preLoadColumn(vi::Freq);
}

FlagAgentSummary::~FlagAgentSummary()
{
	if (fieldCounts)
	{
		std::map<std::string, summary* >::iterator iter;
		for(iter = fieldSummaryMap.begin(); iter != fieldSummaryMap.end(); iter++)
		{
			delete iter->second;
		}
		fieldSummaryMap.clear();
	}
	else
	{
		delete currentSummary;
	}

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
        if( config.type(exists) != TpBool )
        {
            throw( AipsError ( "Parameter 'spwchan' must be of type 'bool'" ) );
        }

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
        if( config.type(exists) != TpBool )
        {
            throw( AipsError ( "Parameter 'spwcorr' must be of type 'bool'" ) );
        }

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

    exists = config.fieldNumber ("basecnt");
    if (exists >= 0)
    {
        if( config.type(exists) != TpBool )
        {
            throw( AipsError ( "Parameter 'basecnt' must be of type 'bool'" ) );
        }

        baselineCounts = config.asBool("basecnt");
    }
    else
    {
        baselineCounts = False;
    }

    if (baselineCounts)
    {
        *logger_p << LogIO::NORMAL << " Baseline count activated " << LogIO::POST;
    }
    else
    {
        *logger_p << LogIO::NORMAL << " Baseline count deactivated " << LogIO::POST;
    }

    exists = config.fieldNumber ("fieldcnt");
    if (exists >= 0)
    {
        if( config.type(exists) != TpBool )
        {
            throw( AipsError ( "Parameter 'fieldcnt' must be of type 'bool'" ) );
        }

        fieldCounts = config.asBool("fieldcnt");
    }
    else
    {
    	fieldCounts = False;
    }

    if (fieldCounts)
    {
        *logger_p << LogIO::NORMAL << " Field breakdown activated " << LogIO::POST;
    }
    else
    {
        *logger_p << LogIO::NORMAL << " Field breakdown deactivated " << LogIO::POST;
    }

    return;
}

void
FlagAgentSummary::preProcessBuffer(const vi::VisBuffer2 &visBuffer)
{
    arrayId = visBuffer.arrayId()(0);
    stringstream arrayId_stringStream;
    arrayId_stringStream << arrayId;
    arrayId_str = arrayId_stringStream.str();

    fieldId = visBuffer.fieldId()(0);
    // Transform fieldId into field name using the corresponding subtable
    fieldId_str = flagDataHandler_p->fieldNames_p->operator()(fieldId);

    spw = visBuffer.spectralWindows()(0);
    stringstream spw_stringStream;
    spw_stringStream << spw;
    spw_str = spw_stringStream.str();

    observationId = visBuffer.observationId()[0];
    stringstream observationId_stringStream;
    observationId_stringStream << observationId;
    observationId_str = observationId_stringStream.str();

    // Read in channel-frequencies.
    // RVU : I'm not sure if this should go here, or in the FlagDataHandler so that all agents get it.
    Vector<Double> flist(visBuffer.getFrequencies(0,MFrequency::TOPO));
    for(Int i=0;i<(Int) flist.nelements();i++)
        frequencyList[spw].push_back(flist[i]);

    if (fieldCounts)
    {
        if (fieldSummaryMap.find(fieldId_str) != fieldSummaryMap.end())
        {
        	currentSummary = fieldSummaryMap[fieldId_str];
        }
        else
        {
        	fieldSummaryMap[fieldId_str] = new summary();
        	currentSummary = fieldSummaryMap[fieldId_str];
        }
    }

    return;
}

bool
FlagAgentSummary::computeRowFlags(const vi::VisBuffer2 &visBuffer, FlagMapper &flags, uInt row)
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
    vector< vector<uInt> > polarizations = flags.getSelectedCorrelations();
    Int nPolarizations = polarizations.size();
    uInt64 rowTotal = nChannels*nPolarizations;

    // Initialize polarization counts
    Int pol_i = 0;;
    vector<uInt64> polarizationsBreakdownFlags;
    for (pol_i=0;pol_i < nPolarizations;pol_i++)
    {
        polarizationsBreakdownFlags.push_back(0);
    }

    // Iterate through channels
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
        	currentSummary->accumChanneltotal[spw][channel_i] += nPolarizations;
            currentSummary->accumChannelflags[spw][channel_i] += channelFlags;
        }
    }

    // Update polarization counts
    polarizationIndexMap *toPolarizationIndexMap = flagDataHandler_p->getPolarizationIndexMap();
    String polarization_str;
    for (pol_i=0;pol_i < nPolarizations;pol_i++)
    {
        polarization_str = (*toPolarizationIndexMap)[polarizations[pol_i][0]];
        currentSummary->accumtotal["correlation"][polarization_str] += nChannels;
        currentSummary->accumflags["correlation"][polarization_str] += polarizationsBreakdownFlags[pol_i];

        if (spwPolarizationCounts)
        {
        	currentSummary->accumPolarizationtotal[spw][polarization_str] += nChannels;
            currentSummary->accumPolarizationflags[spw][polarization_str] += polarizationsBreakdownFlags[pol_i];
        }
    }

    // Update row counts
    currentSummary->accumtotal["array"][arrayId_str] += rowTotal;
    currentSummary->accumflags["array"][arrayId_str] += rowFlags;

    currentSummary->accumtotal["field"][fieldId_str] += rowTotal;
    currentSummary->accumflags["field"][fieldId_str] += rowFlags;

    currentSummary->accumtotal["spw"][spw_str] += rowTotal;
    currentSummary->accumflags["spw"][spw_str] += rowFlags;

    currentSummary->accumtotal["scan"][scan_str] += rowTotal;
    currentSummary->accumflags["scan"][scan_str] += rowFlags;

    currentSummary->accumtotal["observation"][observationId_str] += rowTotal;
    currentSummary->accumflags["observation"][observationId_str] += rowFlags;

    currentSummary->accumtotal["antenna"][antenna1Name] += rowTotal;
    currentSummary->accumflags["antenna"][antenna1Name] += rowFlags;

    if (antenna1 != antenna2)
    {
    	currentSummary->accumtotal["antenna"][antenna2Name] += rowTotal;
        currentSummary->accumflags["antenna"][antenna2Name] += rowFlags;
    }

    if ( baselineCounts )
    {
    	currentSummary->accumtotal["baseline"][baseline] += rowTotal;
        currentSummary->accumflags["baseline"][baseline] += rowFlags;
        currentSummary->accumAntScantotal[antenna1][scan] += rowTotal;
        currentSummary->accumAntScanflags[antenna1][scan] += rowFlags;
        if (antenna1 != antenna2)
        {
        	currentSummary->accumAntScantotal[antenna2][scan] += rowTotal;
            currentSummary->accumAntScanflags[antenna2][scan] += rowFlags;
        }
    }

    currentSummary->accumTotalFlags += rowFlags;
    currentSummary->accumTotalCount += rowTotal;

    return false;
}

FlagReport
FlagAgentSummary::getReport()
{
    // Make the flagreport list
    FlagReport summarylist("list");

    // Add the standard summary dictionary as a report of type 'summary'
//    summarylist.addReport( FlagReport("summary", agentName_p, getResult())  );
    summarylist.addReport( FlagReport("summary", summaryName_p, getResult())  );

    //////// Note : Calculate extra views only if the user has asked for it.
    /////// If returning only summary report, do the following.
    ////////  return FlagReport("summary", agentName_p, getResult());

    // Add a list of reports from the flag-count dictionary
    summarylist.addReport ( buildFlagCountPlots() );

    //         // Make a report (or a list of them )for a view, and add it to the list
    //         FlagReport viewrep("plotline",agentName_p,"title","xaxis","yaxis")
    //         viewrep.addData(xdata,ydata,"label");
    //         summarylist.addReport( viewRep );
    //
    return summarylist;
}

FlagReport
FlagAgentSummary::buildFlagCountPlots()
{
    FlagReport countRepList("list");

    // (1) Plot of fraction flagged vs frequency (only if spwchan==True)
    if( spwChannelCounts )
    {
        pair<string, double> freqUnit("GHz",1e+9);

        FlagReport subRep1 = FlagReport("plotpoints",summaryName_p,"Percentage Flagged",
                "Frequency ("+freqUnit.first+")", "% Flagged");

        for (map<Int, map<uInt, uInt64> >::iterator key1 = currentSummary->accumChanneltotal.begin();
                key1 != currentSummary->accumChanneltotal.end();
                key1++)
        {
            Int nCh=currentSummary->accumChanneltotal[key1->first].size();

            Vector<Float> freqVals(nCh), flagPercent(nCh);
            uInt chCount=0;
            for (map<uInt, uInt64>::const_iterator key2 = key1->second.begin();
                    key2 != key1->second.end();
                    key2++)
            {
                // read the frequency value for this channel.
                freqVals[chCount] = frequencyList[key1->first][key2->first] / freqUnit.second;

                // calculate the percentage flagged for this channel
                if( key2->second > 0 )
                {
                    flagPercent[chCount] = 100.0 *
                            (Double) currentSummary->accumChannelflags[key1->first][key2->first] /
                            (Double) key2->second;
                }
                else
                {
                    flagPercent[chCount] = 0.0;
                }

                // Increment channel counter
                chCount++;
            }

            subRep1.addData("line", freqVals,flagPercent,"",Vector<Float>(),
                    "spw"+String::toString(key1->first));

        }

        countRepList.addReport( subRep1 );
    }

    // (2) Plot of fraction flagged vs antenna-position
    Int nAnt=currentSummary->accumtotal["antenna"].size();
    if(nAnt>0) // Perhaps put a parameter to control this ?
    {
        Vector<Float> antPosX(nAnt), antPosY(nAnt), radius(nAnt);
        Int antCount=0;
        const Vector<double> xyzOrigin = (flagDataHandler_p->antennaPositions_p->operator()(0))
                                                                               .getValue().getValue();

        FlagReport subRep2 = FlagReport("plotpoints",summaryName_p,"Percentage Flagged",
                "X meters (ITRF)", "Y meters (ITRF)");

        for (map<std::string, uInt64>::const_iterator antkey = currentSummary->accumtotal["antenna"].begin();
                antkey != currentSummary->accumtotal["antenna"].end();
                antkey++)
        {
            Int antId = 0; //antCount; // this needs to find the antenna-id for the antenna name.... aaaaah.
            for(antId=0; antId<(Int) flagDataHandler_p->antennaNames_p->nelements(); antId++)
            {
                if( flagDataHandler_p->antennaNames_p->operator()(antId)
                == String(antkey->first) ) break;
            }

            const Vector<double> xyz = (flagDataHandler_p->antennaPositions_p->operator()(antId))
                                                                                .getValue().getValue();
            antPosX[antCount] = xyz[0]-xyzOrigin[0];
            antPosY[antCount] = xyz[1]-xyzOrigin[1];
            radius[antCount] = 200.0 *
                    (Double) currentSummary->accumflags["antenna"][antkey->first]/
                    (Double) antkey->second;
            antCount++;
        }
        subRep2.addData("scatter", antPosX,antPosY,"circle",radius,"");
        countRepList.addReport( subRep2 );
    }

    // (3) Plot of fraction flagged vs baseline-length
    Int nBase=currentSummary->accumtotal["baseline"].size();
    if(nBase>0 && baselineCounts==True) // Perhaps put a parameter to control this ?
    {
        Vector<Float> baselineLength(nBase), flagFraction(nBase);
        Int baseCount=0;
        FlagReport subRep3 = FlagReport("plotpoints",summaryName_p,"Percentage Flagged per baseline",
                "Baseline Length (m)", "% Flagged");

        for (map<std::string, uInt64>::const_iterator basekey = currentSummary->accumtotal["baseline"].begin();
                basekey != currentSummary->accumtotal["baseline"].end();
                basekey++)
        {
            Int antId1 = 0, antId2=0;
            String antName1,antName2;
            antName1 = antName2 =  String(basekey->first);
            antName1 = antName1.before("&&");
            antName2 = antName2.after("&&");
            for(Int antId=0; antId<(Int) flagDataHandler_p->antennaNames_p->nelements(); antId++)
            {
                if( flagDataHandler_p->antennaNames_p->operator()(antId) == antName1 ) antId1 = antId;
                if( flagDataHandler_p->antennaNames_p->operator()(antId) == antName2 ) antId2 = antId;
            }

            const Vector<double> xyz1 = (flagDataHandler_p->antennaPositions_p->operator()(antId1))
                                                                                 .getValue().getValue();
            const Vector<double> xyz2 = (flagDataHandler_p->antennaPositions_p->operator()(antId2))
                                                                                 .getValue().getValue();
            baselineLength[baseCount] = sqrt( fabs( (xyz1[0]-xyz2[0])*(xyz1[0]-xyz2[0]) +
                    (xyz1[1]-xyz2[1])*(xyz1[1]-xyz2[1]) +
                    (xyz1[2]-xyz2[2])*(xyz1[2]-xyz2[2]) ) );
            flagFraction[baseCount] = 100.0 *
                    (Double) currentSummary->accumflags["baseline"][basekey->first]/
                    (Double) basekey->second;
            baseCount++;
        }
        subRep3.addData("scatter", baselineLength,flagFraction,"",Vector<Float>(),"");
        countRepList.addReport( subRep3 );
    }

    // jagonzal: CAS-3450
    if(nBase>0 && baselineCounts==True)
    {
        Int totalNAnt = flagDataHandler_p->antennaNames_p->size();
        // Add ant1xant2 summary views
        FlagReport subRep4 = FlagReport("plotraster",summaryName_p,"% Flagged per baseline", "Antenna 1", "Antenna 2");
        Array<Float> ant1ant2View( IPosition(2, totalNAnt, totalNAnt) , (Float)0);
        std::pair<Int,Int> ant1ant2;
        Float percentageFlagged;
        for (map<std::string, uInt64>::const_iterator	basekey = currentSummary->accumtotal["baseline"].begin();
                basekey != currentSummary->accumtotal["baseline"].end();
                basekey++)
        {
            ant1ant2 = flagDataHandler_p->baselineToAnt1Ant2_p[basekey->first];
            percentageFlagged = (Float)100*((Double)currentSummary->accumflags["baseline"][basekey->first] /
            		(Double)currentSummary->accumtotal["baseline"][basekey->first]);
            ant1ant2View(IPosition(2, ant1ant2.first, ant1ant2.second)) = percentageFlagged;
            ant1ant2View(IPosition(2, ant1ant2.second, ant1ant2.first)) = percentageFlagged;

        }

        subRep4.addData(ant1ant2View);
        countRepList.addReport( subRep4 );

        // Add ant1xscan summary views
        FlagReport subRep5 = FlagReport("plotraster",summaryName_p,"% Flagged per antenna and scan", "Scan relative index", "% Flagged per antenna");

        // NOTE: We need to handle directly the storage array, because it seems that the dimension steps are switched
        Array<Float> antScanView( IPosition(2, currentSummary->accumflags["scan"].size(),totalNAnt) , (Float)0);
        Bool deleteIt = False;
        Float* antScanViewPtr = antScanView.getStorage(deleteIt);


        uInt scanIdx,antennaIdx = 0;
        for (map<Int, std::map<Int, uInt64> >::const_iterator	antkey = currentSummary->accumAntScantotal.begin();
                antkey != currentSummary->accumAntScantotal.end();
                antkey++)
        {
            scanIdx = 0;
            for (map<Int, uInt64>::const_iterator	scankey = antkey->second.begin();
                    scankey != antkey->second.end();
                    scankey++)
            {
                percentageFlagged = (Float)100*((Double)currentSummary->accumAntScanflags[antkey->first][scankey->first] /
                		(Double)currentSummary->accumAntScantotal[antkey->first][scankey->first]);
                antScanViewPtr[totalNAnt*scanIdx + antkey->first] = percentageFlagged;
                scanIdx += 1;
            }
            antennaIdx += 1;
        }

        subRep5.addData(antScanView);
        countRepList.addReport( subRep5 );
    }


    return countRepList;
}


Record
FlagAgentSummary::getResult()
{
    logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));

    Record result;

    if (fieldCounts)
    {
    	std::map<std::string, summary* >::iterator iter;
    	for(iter = fieldSummaryMap.begin(); iter != fieldSummaryMap.end(); iter++)
    	{
    		Record subresult;
    		currentSummary = iter->second;
    		getResultCore(subresult);
    		result.defineRecord(iter->first, subresult);
    	}
    }
    else
    {
    	getResultCore(result);
    }

    return result;
}

void
FlagAgentSummary::getResultCore(Record &result)
{
	if (fieldCounts)
	{
		string field = currentSummary->accumtotal["field"].begin()->first;
		string spaces(field.size(),'=');
		*logger_p << LogIO::NORMAL << "======" << spaces << "==========" << LogIO::POST;
	    *logger_p << LogIO::NORMAL << "Field " << field << " breakdown" << LogIO::POST;
		*logger_p << LogIO::NORMAL << "======" << spaces << "==========" << LogIO::POST;
	}

    if (spwChannelCounts)
    {
        Record stats_key1;

        for (map<Int, map<uInt, uInt64> >::iterator key1 = currentSummary->accumChanneltotal.begin();
        		key1 != currentSummary->accumChanneltotal.end();key1++)
        {
            // Transform spw id into string
            stringstream spw_stringStream;
            spw_stringStream << key1->first;

            for (map<uInt, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
            {
                Record stats_key2;

                stats_key2.define("flagged", (Double) currentSummary->accumChannelflags[key1->first][key2->first]);
                stats_key2.define("total", (Double) key2->second);

                // Transform channel id into string
                stringstream channel_stringStream;
                channel_stringStream << key2->first;

                // Construct spw:channel string as first key
                stats_key1.defineRecord(spw_stringStream.str() + ":" + channel_stringStream.str(), stats_key2);
                // Calculate percentage flagged
                stringstream percentage;
                percentage.precision(3);
                //				percentage.fixed;
                if( key2->second > 0 )
                {
                    percentage << " (" << 100.0 *
                            (Double) currentSummary->accumChannelflags[key1->first][key2->first]/
                            (Double) key2->second << "%)";
                }

                *logger_p 	<< LogIO::NORMAL
                        << " Spw:" << key1->first << " Channel:" << key2->first
                        << " flagged: " <<  (Double) currentSummary->accumChannelflags[key1->first][key2->first]
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

        for (map<Int, map<string, uInt64> >::iterator key1 = currentSummary->accumPolarizationtotal.begin();
        		key1 != currentSummary->accumPolarizationtotal.end();key1++)
        {
            // Transform spw id into string
            stringstream spw_stringStream;
            spw_stringStream << key1->first;

            for (map<string, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
            {
                Record stats_key2;

                stats_key2.define("flagged", (Double) currentSummary->accumPolarizationflags[key1->first][key2->first]);
                stats_key2.define("total", (Double) key2->second);

                // Construct spw:correlation string as first key (Polarization already comes as a string)
                stats_key1.defineRecord(spw_stringStream.str() + ":" + key2->first, stats_key2);

                // Calculate percentage flagged
                stringstream percentage;
                percentage.precision(3);
                //				percentage.fixed;
                if( key2->second > 0 )
                {
                    percentage << " (" << 100.0 *
                            (Double) currentSummary->accumPolarizationflags[key1->first][key2->first]/
                            (Double) key2->second << "%)";
                }

                *logger_p 	<< LogIO::NORMAL
                        << " Spw:" << key1->first << " Correlation:" << key2->first
                        << " flagged: " <<  (Double) currentSummary->accumPolarizationflags[key1->first][key2->first]
                                            << " total: " <<  (Double) key2->second
                                            << percentage.str()
                                            << LogIO::POST;
            }
        }

        result.defineRecord("spw:correlation", stats_key1);
    }

    if (baselineCounts)
    {
        Record stats_key1;

        for (map<Int, map<Int, uInt64> >::iterator key1 = currentSummary->accumAntScantotal.begin();
        		key1 != currentSummary->accumAntScantotal.end();key1++)
        {
            // Construct antenna name
            stringstream antenna_stringStream;
            antenna_stringStream << flagDataHandler_p->antennaNames_p->operator()(key1->first);

            for (map<Int, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
            {
                // Construct scan name
                stringstream scan_stringStream;
                scan_stringStream << key2->first;

                Record stats_key2;

                stats_key2.define("flagged", (Double) currentSummary->accumAntScanflags[key1->first][key2->first]);
                stats_key2.define("total", (Double) key2->second);

                // Construct spw:correlation string as first key (Polarization already comes as a string)
                stats_key1.defineRecord(antenna_stringStream.str() + ":" + scan_stringStream.str(), stats_key2);

                // Calculate percentage flagged
                stringstream percentage;
                percentage.precision(3);
                //				percentage.fixed;
                if( key2->second > 0 )
                {
                    percentage << " (" << 100.0 *
                            (Double) currentSummary->accumAntScanflags[key1->first][key2->first]/
                            (Double) key2->second << "%)";
                }

                *logger_p 	<< LogIO::NORMAL
                        << " Spw:" << key1->first << " Correlation:" << key2->first
                        << " flagged: " <<  (Double) currentSummary->accumAntScanflags[key1->first][key2->first]
                                         << " total: " <<  (Double) key2->second
                                         << percentage.str()
                                         << LogIO::POST;
            }
        }

        result.defineRecord("antenna:scan", stats_key1);
    }

    for (map<string, map<string, uInt64> >::iterator key1 = currentSummary->accumtotal.begin();
    		key1 != currentSummary->accumtotal.end();key1++)
    {
        Record stats_key1;
        for (map<string, uInt64>::const_iterator key2 = key1->second.begin();key2 != key1->second.end();key2++)
        {
            Record stats_key2;

            stats_key2.define("flagged", (Double) currentSummary->accumflags[key1->first][key2->first]);
            stats_key2.define("total", (Double) key2->second);
            stats_key1.defineRecord(key2->first, stats_key2);

            // Calculate percentage flagged
            stringstream percentage;
            percentage.precision(3);
            //			percentage.fixed;
            if( key2->second > 0 )
            {
                percentage << " (" << 100.0 *
                        (Double) currentSummary->accumflags[key1->first][key2->first] /
                        (Double) key2->second << "%)";
            }

            *logger_p 	<< LogIO::NORMAL
                    << " " << key1->first << " " << key2->first
                    << " flagged: " <<  (Double) currentSummary->accumflags[key1->first][key2->first]
                                     << " total: " <<  (Double) key2->second
                                     << percentage.str()
                                     << LogIO::POST;
        }

        result.defineRecord(key1->first, stats_key1);
    }

    result.define("flagged", (Double) currentSummary->accumTotalFlags);
    result.define("total"  , (Double) currentSummary->accumTotalCount);

    // Calculate percentage flagged
    stringstream percentage;
    percentage.precision(3);
    //	percentage.fixed;
    if( currentSummary->accumTotalCount > 0 )
    {
        percentage << " (" << 100.0 *
                (Double) currentSummary->accumTotalFlags /
                (Double) currentSummary->accumTotalCount << "%)";
    }
    *logger_p 	<< LogIO::NORMAL
            << " Total Flagged: " <<  (Double) currentSummary->accumTotalFlags
            << " Total Counts: " <<  (Double) currentSummary->accumTotalCount
            << percentage.str()
            << LogIO::POST;

    return;
}

} //# NAMESPACE CASA - END


