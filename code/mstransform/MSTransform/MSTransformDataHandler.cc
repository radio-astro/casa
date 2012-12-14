//# MSTransformDataHandler.h: This file contains the implementation of the MSTransformDataHandler class.
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

#include <mstransform/MSTransform/MSTransformDataHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace tvf
{
	Double wtToSigma(Double wt)
	{
		return wt > 0.0 ? 1.0 / sqrt(wt) : -1.0;
	}

	Bool False = False;
	Bool True = False;
}

/////////////////////////////////////////////
/// MSTransformDataHandler implementation //
/////////////////////////////////////////////

// -----------------------------------------------------------------------
// Default constructor
// -----------------------------------------------------------------------
MSTransformDataHandler::MSTransformDataHandler()
{
	initialize();
	return;
}


// -----------------------------------------------------------------------
// Configuration constructor
// -----------------------------------------------------------------------
MSTransformDataHandler::MSTransformDataHandler(Record configuration)
{
	initialize();
	configure(configuration);
	return;
}


// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
MSTransformDataHandler::~MSTransformDataHandler()
{
	if (visibilityIterator_p) delete visibilityIterator_p;
	if (splitter_p) delete splitter_p;

	return;
}

// -----------------------------------------------------------------------
// Method to initialize members to default values
// -----------------------------------------------------------------------
void MSTransformDataHandler::initialize()
{
	// Input-Output MS parameters
	inpMsName_p = String("");
	outMsName_p = String("");

	// Data selection parameters
	arraySelection_p = String("");
	fieldSelection_p = String("");
	scanSelection_p = String("");
	timeSelection_p = String("");
	spwSelection_p = String("");
	baselineSelection_p = String("");
	uvwSelection_p = String("");
	polarizationSelection_p = String("");
	scanIntentSelection_p = String("");
	observationSelection_p = String("");

	// Output MS structure parameters
	datacolumn_p = String("corrected");
	tileShape_p.resize(1,False);
	tileShape_p(0) = 0;

	// Spectral transformation parameters
	combinespws_p = False;
	chanSpec_p.resize(1,False);
	chanSpec_p(0) = 1;

	// Time transformation parameters
	timeBin_p = 0.0;
	timespan_p = String("");

	// MS-related members
	splitter_p = NULL;
	inputMs_p = NULL;
	selectedInputMs_p = NULL;
	outputMs_p = NULL;
	inputMsCols_p = NULL;
	outputMsCols_p = NULL;

	// VI/VB related members
	sortColumns_p = Block<Int>(7);
	sortColumns_p[0] = MS::OBSERVATION_ID;
	sortColumns_p[1] = MS::ARRAY_ID;
	sortColumns_p[2] = MS::SCAN_NUMBER;
	sortColumns_p[3] = MS::STATE_ID;
	sortColumns_p[4] = MS::FIELD_ID;
	sortColumns_p[5] = MS::DATA_DESC_ID;
	sortColumns_p[6] = MS::TIME;
	visibilityIterator_p = NULL;

	// Output MS structure related members
	fillFlagCategory_p = False;
	fillWeightSpectrum_p = False;
	correctedToData_p = True;
	dataColMap_p.clear();

	// Spectral transformation members
	inputOutputWeightChannelMap_p.clear();
	baselineMap_p.clear();
	combinedSpw_p = spwInfo();

	return;
}

// -----------------------------------------------------------------------
// Method to configure (or re-configure) members
// -----------------------------------------------------------------------
void MSTransformDataHandler::configure(Record &configuration)
{
	int exists = 0;

	// Input-Output Ms paramters
	exists = configuration.fieldNumber ("inputms");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("inputms"), inpMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Input file name is " << inpMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("outputms");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("outputms"), outMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Output file name is " << outMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("datacolumn");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("datacolumn"), datacolumn_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Data column is " << datacolumn_p << LogIO::POST;
	}

	// Transformation parameters
	exists = configuration.fieldNumber ("combinespws");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("combinespws"), combinespws_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Combine Spectral Windows is " << combinespws_p << LogIO::POST;
	}


	// Data selection parameters
	exists = configuration.fieldNumber ("array");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("array"), arraySelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " array selection is " << arraySelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("field");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("field"), fieldSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " field selection is " << fieldSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("scan");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("scan"), scanSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " scan selection is " << scanSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("timerange");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("timerange"), timeSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " timerange selection is " << timeSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("spw");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("spw"), spwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " spw selection is " << spwSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("antenna");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("antenna"), baselineSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " antenna selection is " << baselineSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("uvrange");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("uvrange"), uvwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " uvrange selection is " << uvwSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("correlation");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("correlation"), polarizationSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " correlation selection is " << polarizationSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("observation");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("observation"), observationSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<" observation selection is " << observationSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("intent");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("intent"), scanIntentSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << " scan intent selection is " << scanIntentSelection_p << LogIO::POST;
	}


	return;
}

// -----------------------------------------------------------------------
// Method to open the input MS, select the data and create the
// structure of the output MS filling the auxiliary tables.
// -----------------------------------------------------------------------
void MSTransformDataHandler::open()
{
	splitter_p = new SubMS(inpMsName_p,Table::Update);

	inputMs_p = &(splitter_p->ms_p);

	const String dummyExpr = String("");
	splitter_p->setmsselect((const String)spwSelection_p,
							(const String)fieldSelection_p,
							(const String)baselineSelection_p,
							(const String)scanIntentSelection_p,
							(const String)uvwSelection_p,
							dummyExpr, // taqlExpr
							chanSpec_p,
							(const String)arraySelection_p,
							(const String)polarizationSelection_p,
							(const String)scanIntentSelection_p,
							(const String)observationSelection_p);

	splitter_p->selectTime(timeBin_p,timeSelection_p);

	splitter_p->fillMainTable_p = False;

	splitter_p->makeSubMS(outMsName_p,datacolumn_p,tileShape_p,timespan_p);

	selectedInputMs_p = &(splitter_p->mssel_p);

	outputMs_p = &(splitter_p->msOut_p);

	inputMsCols_p = splitter_p->mscIn_p;

	outputMsCols_p = splitter_p->msc_p;

	if (combinespws_p)
	{
		combineSpwSubtable();
		reindexSourceSubTable();
		reindexDDISubTable();
		reindexFeedSubTable();
		reindexSysCalSubTable();
		reindexFreqOffsetSubTable();
	}
}

void MSTransformDataHandler::combineSpwSubtable()
{
	// Access Spectral Window sub-table
	MSSpectralWindow spwTable = outputMs_p->spectralWindow();
    Int nInputSpws = spwTable.nrow();
    MSSpWindowColumns spwCols(spwTable);

    // Access columns which have to be combined
    ArrayColumn<Double> chanFreqCol = spwCols.chanFreq();
    ArrayColumn<Double> chanWidthCol = spwCols.chanWidth();
    ArrayColumn<Double> effectiveBWCol = spwCols.effectiveBW();
    ArrayColumn<Double> resolutionCol = spwCols.resolution();

    // Create list of input channels
    vector<channelInfo> inputChannels;
    for(uInt spw_idx=0; spw_idx<nInputSpws; spw_idx++)
    {
    	Vector<Double> inputChanFreq(chanFreqCol(spw_idx));
    	Vector<Double> inputChanWidth(chanWidthCol(spw_idx));
    	Vector<Double> inputEffectiveBW(effectiveBWCol(spw_idx));
    	Vector<Double> inputResolution(resolutionCol(spw_idx));
    	uInt nChannels = inputChanFreq.size();

    	for (uInt chan_idx=0;chan_idx<nChannels;chan_idx++)
    	{
    		channelInfo channelInfo_idx;
    		channelInfo_idx.SPW_id = spw_idx;
    		channelInfo_idx.inpChannel = chan_idx;
    		channelInfo_idx.CHAN_FREQ = inputChanFreq(chan_idx);
    		channelInfo_idx.CHAN_WIDTH = inputChanWidth(chan_idx);
    		channelInfo_idx.EFFECTIVE_BW = inputEffectiveBW(chan_idx);
    		channelInfo_idx.RESOLUTION = inputResolution(chan_idx);

    		inputChannels.push_back(channelInfo_idx);
    	}
    }

    // Sort input channels
    sort (inputChannels.begin(), inputChannels.end());

    // Produce output-input channel map
    map< uInt,vector<uInt> > outputInputChannelMap;

    // Dump first input channel index into first output channel list
    uInt outputChannelIdx = 0;
    outputInputChannelMap[outputChannelIdx].clear();
    outputInputChannelMap[outputChannelIdx].push_back(0);

    // Collect list of input channels indexes related to each output channel
    for (uInt inputChannels_idx=1; inputChannels_idx<inputChannels.size(); inputChannels_idx++)
    {
    	if (inputChannels[inputChannels_idx-1].overlap(inputChannels[inputChannels_idx]))
    	{
    		outputInputChannelMap[outputChannelIdx].push_back(inputChannels_idx);
    	}
    	else
    	{
    		outputChannelIdx += 1;
    		outputInputChannelMap[outputChannelIdx].clear();
    		outputInputChannelMap[outputChannelIdx].push_back(inputChannels_idx);
    	}
    }

    // Merge input channels to produce each of the output channels
    outputChannelIdx = 0;
    vector<uInt> inputChannelsIndexes;
    Double newChanWidth, newChanFreq;
    channelInfo lowFreqChan, highFreqChan;
    uInt nOutputChannels = outputInputChannelMap.size();
    combinedSpw_p = spwInfo(nOutputChannels);
    for (map< uInt,vector<uInt> >::iterator map_iter=outputInputChannelMap.begin();map_iter!=outputInputChannelMap.end();map_iter++)
    {
    	inputChannelsIndexes = map_iter->second;

    	// At this point we assume that the channels are sorted by frequency
    	if (inputChannelsIndexes.size()>1)
    	{
    		lowFreqChan = inputChannels.at(inputChannelsIndexes.front());
    		highFreqChan = inputChannels.at(inputChannelsIndexes.back());

    		// Determine output channel frequency and width
        	newChanWidth = highFreqChan.upperBound()-lowFreqChan.lowerBound();
        	newChanFreq = lowFreqChan.lowerBound() + 0.5*newChanWidth;

        	// Calculate the normalizing factor for each input channel
        	Double normalizeFactor = 0;;
        	for (uInt inputChannel_idx=0; inputChannel_idx<inputChannelsIndexes.size(); inputChannel_idx++)
        	{
        		lowFreqChan = inputChannels.at(inputChannelsIndexes.at(inputChannel_idx));
        		normalizeFactor += lowFreqChan.CHAN_WIDTH;
        	}

        	// Fill input-output channel weight map
        	for (uInt inputChannel_idx=0; inputChannel_idx!=inputChannelsIndexes.size(); inputChannel_idx++)
        	{
        		lowFreqChan = inputChannels.at(inputChannelsIndexes.at(inputChannel_idx));
        		inputOutputWeightChannelMap_p[lowFreqChan.SPW_id][lowFreqChan.inpChannel] = std::make_pair(outputChannelIdx,lowFreqChan.CHAN_WIDTH/normalizeFactor);
        	}
    	}
    	else
    	{
    		lowFreqChan = inputChannels.at(inputChannelsIndexes.front());

    		// Determine output channel frequency and width
        	newChanWidth = lowFreqChan.CHAN_WIDTH;
        	newChanFreq = lowFreqChan.CHAN_FREQ;

        	// Fill input-output channel weight map
    		lowFreqChan = inputChannels.at(inputChannelsIndexes.front());
    		inputOutputWeightChannelMap_p[lowFreqChan.SPW_id][lowFreqChan.inpChannel] = std::make_pair(outputChannelIdx,1);
    	}

    	combinedSpw_p.CHAN_FREQ(outputChannelIdx) = newChanFreq;
    	combinedSpw_p.CHAN_WIDTH(outputChannelIdx) = newChanWidth;
    	combinedSpw_p.EFFECTIVE_BW(outputChannelIdx) = newChanWidth;
    	combinedSpw_p.RESOLUTION(outputChannelIdx) = newChanWidth;
    	outputChannelIdx += 1;
    }

    // Delete combined SPWs (reverse to preserve row number)
    uInt rowsToDelete = spwTable.nrow()-1;
    for(Int spw_idx=rowsToDelete; spw_idx>0; spw_idx--)
    {
    	spwTable.removeRow(spw_idx);
    }

    // Redefine the combined columns in new first SPW
    chanFreqCol.put(0, combinedSpw_p.CHAN_FREQ);
    chanWidthCol.put(0,  combinedSpw_p.CHAN_WIDTH);
    effectiveBWCol.put(0, combinedSpw_p.EFFECTIVE_BW);
    resolutionCol.put(0, combinedSpw_p.RESOLUTION);

    // Set output SPW number of channels, reference frequency and total bandwidth in new first SPW
    combinedSpw_p.update();
    ScalarColumn<Int> numChanCol = spwCols.numChan();
    ScalarColumn<Double> refFrequencyCol = spwCols.refFrequency();
    ScalarColumn<Double> totalBandwidthCol = spwCols.totalBandwidth();
    numChanCol.put(0,combinedSpw_p.NUM_CHAN);
    refFrequencyCol.put(0,combinedSpw_p.REF_FREQUENCY);
    totalBandwidthCol.put(0,combinedSpw_p.TOTAL_BANDWIDTH);

    // Flush changes
    outputMs_p->flush(True);

	return;
}

// -----------------------------------------------------------------------
// Re-index Spectral Window column in Source sub-table
// -----------------------------------------------------------------------
void MSTransformDataHandler::reindexSourceSubTable()
{
    if(Table::isReadable(outputMs_p->sourceTableName()) and !outputMs_p->source().isNull())
    {
    	MSSource msSubtable = outputMs_p->source();
    	MSSourceColumns tableCols(msSubtable);
       	ScalarColumn<Int> spwCol = tableCols.spectralWindowId();
        reindexColumn(spwCol,0);

        // Flush changes
        outputMs_p->flush(True);
    }
    else
    {
    	logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "No SOURCE sub-table found " << LogIO::POST;
    }

    return;
}

// -----------------------------------------------------------------------
// Re-index Spectral Window column in DDI sub-table
// -----------------------------------------------------------------------
void MSTransformDataHandler::reindexDDISubTable()
{
    if(Table::isReadable(outputMs_p->dataDescriptionTableName()) and !outputMs_p->dataDescription().isNull())
    {
    	MSDataDescription msSubtable = outputMs_p->dataDescription();
    	MSDataDescColumns tableCols(msSubtable);

        // Delete all rows except for the first one
        uInt rowsToDelete = tableCols.nrow()-1;
        for(uInt row_idx=rowsToDelete; row_idx>0; row_idx--)
        {
        	msSubtable.removeRow(row_idx);
        }

        // Set SPW in the remaining row
        ScalarColumn<Int> spwCol = tableCols.spectralWindowId();
        spwCol.put(0,0);

        // Flush changes
        outputMs_p->flush(True);
    }
    else
    {
    	logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "No DATA_DESCRIPTION sub-table found " << LogIO::POST;
    }
}

// -----------------------------------------------------------------------
// Re-index Spectral Window column in Feed sub-table
// -----------------------------------------------------------------------
void MSTransformDataHandler::reindexFeedSubTable()
{
    if(Table::isReadable(outputMs_p->feedTableName()) and !outputMs_p->feed().isNull())
    {
    	MSFeed msSubtable = outputMs_p->feed();
    	MSFeedColumns tableCols(msSubtable);
    	ScalarColumn<Int> spwCol = tableCols.spectralWindowId();
    	reindexColumn(spwCol,0);

        // Flush changes
        outputMs_p->flush(True);
    }
    else
    {
    	logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "No FEED sub-table found " << LogIO::POST;
    }

	return;
}

// -----------------------------------------------------------------------
// Re-index Spectral Window column in SysCal sub-table
// -----------------------------------------------------------------------
void MSTransformDataHandler::reindexSysCalSubTable()
{
    if(Table::isReadable(outputMs_p->sysCalTableName()) and !outputMs_p->sysCal().isNull())
    {
    	MSSysCal msSubtable = outputMs_p->sysCal();
    	MSSysCalColumns tableCols(msSubtable);
    	ScalarColumn<Int> spwCol = tableCols.spectralWindowId();
    	reindexColumn(spwCol,0);

        // Flush changes
        outputMs_p->flush(True);
    }
    else
    {
    	logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "No SYSCAL sub-table found " << LogIO::POST;
    }

	return;
}

// -----------------------------------------------------------------------
// Re-index Spectral Window column in FreqOffset sub-table
// -----------------------------------------------------------------------
void MSTransformDataHandler::reindexFreqOffsetSubTable()
{
    if(Table::isReadable(outputMs_p->freqOffsetTableName()) and !outputMs_p->freqOffset().isNull())
    {
    	MSFreqOffset msSubtable = outputMs_p->freqOffset();
    	MSFreqOffsetColumns tableCols(msSubtable);
    	ScalarColumn<Int> spwCol = tableCols.spectralWindowId();
    	reindexColumn(spwCol,0);

        // Flush changes
        outputMs_p->flush(True);
    }
    else
    {
    	logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "No FREQ_OFF sub-table found " << LogIO::POST;
    }
}

// -----------------------------------------------------------------------
// Set all the elements of a scalar column to a given value
// -----------------------------------------------------------------------
void MSTransformDataHandler::reindexColumn(ScalarColumn<Int> &inputCol, Int value)
{
	for(uInt idx=0; idx<inputCol.nrow(); idx++)
	{
		inputCol.put(idx,value);
	}

	return;
}

// -----------------------------------------------------------------------
// Close the output MS
// -----------------------------------------------------------------------
void MSTransformDataHandler::close()
{
	if (outputMs_p)
	{
		// Flush and unlock MS
		outputMs_p->flush();
		outputMs_p->relinquishAutoLocks(True);
		outputMs_p->unlock();
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check if flag category has to be filled
// -----------------------------------------------------------------------
void MSTransformDataHandler::checkFillFlagCategory()
{
	fillFlagCategory_p = False;
	if (!inputMsCols_p->flagCategory().isNull() && inputMsCols_p->flagCategory().isDefined(0))
	{
		fillFlagCategory_p = True;
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Optional column FLAG_CATEGORY found in input MS will be written to output MS" << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check if weight spectrum column has to be filled
// -----------------------------------------------------------------------
void MSTransformDataHandler::checkFillWeightSpectrum()
{
	fillWeightSpectrum_p = False;
	if (!inputMsCols_p->weightSpectrum().isNull() && inputMsCols_p->weightSpectrum().isDefined(0))
	{
		fillWeightSpectrum_p = True;
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Optional column WEIGHT_SPECTRUM found in input MS will be written to output MS" << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check which data columns have to be filled
// -----------------------------------------------------------------------
void MSTransformDataHandler::checkDataColumnsToFill()
{
	dataColMap_p.clear();
	if (datacolumn_p.contains("ALL"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			dataColMap_p[MS::CORRECTED_DATA] = MS::CORRECTED_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding CORRECTED_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
		{
			dataColMap_p[MS::MODEL_DATA] = MS::MODEL_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding MODEL_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("FLOAT_DATA,DATA"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"FLOAT_DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("FLOAT_DATA"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"FLOAT_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("LAG_DATA,DATA"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"LAG_DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("LAG_DATA"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"LAG_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("DATA"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("CORRECTED"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			dataColMap_p[MS::CORRECTED_DATA] = MS::DATA;
			correctedToData_p = True;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS from input CORRECTED_DATA column"<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"CORRECTED_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("MODEL"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
		{
			dataColMap_p[MS::MODEL_DATA] = MS::DATA;
			dataColMap_p[MS::CORRECTED_DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS from input MODEL_DATA column"<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"MODEL_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else
	{
		logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
				"Requested data column " << datacolumn_p <<
				" is not supported, possible choices are ALL,DATA,CORRECTED,MODEL,FLOAT_DATA,LAG_DATA" << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Determine sort columns order
// -----------------------------------------------------------------------
void MSTransformDataHandler::setIterationApproach()
{
	uInt nSortColumns = 7;

	if (timespan_p.contains("scan")) nSortColumns -= 1;
	if (timespan_p.contains("state")) nSortColumns -= 1;
	if (combinespws_p) nSortColumns -= 1;

	sortColumns_p = Block<Int>(nSortColumns);
	uInt sortColumnIndex = 0;

	sortColumns_p[0] = MS::OBSERVATION_ID;
	sortColumnIndex += 1;

	sortColumns_p[1] = MS::ARRAY_ID;
	sortColumnIndex += 1;

	if (!timespan_p.contains("scan"))
	{
		sortColumns_p[sortColumnIndex] =  MS::SCAN_NUMBER;
		sortColumnIndex += 1;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Combining data through scans for time average " << LogIO::POST;
	}

	if (!timespan_p.contains("state"))
	{
		sortColumns_p[sortColumnIndex] =  MS::STATE_ID;
		sortColumnIndex += 1;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Combining data through state for time average" << LogIO::POST;
	}

	sortColumns_p[sortColumnIndex] = MS::FIELD_ID;
	sortColumnIndex += 1;

	if (!combinespws_p)
	{
		sortColumns_p[sortColumnIndex] =  MS::DATA_DESC_ID;
		sortColumnIndex += 1;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Combining data from selected spectral windows" << LogIO::POST;
	}

	sortColumns_p[sortColumnIndex] =  MS::TIME;

	return;
}

// -----------------------------------------------------------------------
// Generate the initial iterator
// -----------------------------------------------------------------------
void MSTransformDataHandler::generateIterator()
{
	visibilityIterator_p = new vi::VisibilityIterator2(*selectedInputMs_p,sortColumns_p,false,NULL,false,timeBin_p);
	return;
}

// -----------------------------------------------------------------------
// Check configuration and input MS characteristics to determine run parameters
// -----------------------------------------------------------------------
void MSTransformDataHandler::setup()
{
	checkFillFlagCategory();
	checkFillWeightSpectrum();
	checkDataColumnsToFill();
	setIterationApproach();
	generateIterator();

	return;
}

// -----------------------------------------------------------------------
// Fill output MS with data from an input VisBuffer
// -----------------------------------------------------------------------
void MSTransformDataHandler::fillOutputMs(vi::VisBuffer2 *vb)
{
	// Calculate number of rows to add to the output MS depending on the combination parameters
	uInt rowsToAdd = 0;
	if (combinespws_p)
	{
		baselineMap_p.clear();
		Vector<Int> antenna1 = vb->antenna1();
		Vector<Int> antenna2 = vb->antenna2();
		Vector<Int> spws = vb->spectralWindows();
		for (uInt row=0;row<antenna1.size();row++)
		{
			baselineMap_p[std::make_pair(antenna1(row),antenna2(row))].push_back(row);
		}
		rowsToAdd = baselineMap_p.size();
	}
	else
	{
		rowsToAdd = vb->nRows();
	}

	/*
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__)
			<< "Adding " << rowsToAdd << " rows to output MS from "
			<<  vb->nRows() << " rows in input MS selection" << LogIO::POST;
			*/

	uInt currentRows = outputMs_p->nrow();
	RefRows rowRef( currentRows, currentRows + rowsToAdd - 1);

	outputMs_p->addRow(rowsToAdd,True);

	fillIdCols(vb,rowRef);
    fillDataCols(vb,rowRef);

    return;

}


// ----------------------------------------------------------------------------------------
// Fill auxiliary (meta data) columns which don't depend on the SPW (merely consist of Ids)
// ----------------------------------------------------------------------------------------
void MSTransformDataHandler::fillIdCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
    // For row-constant columns we have to create new vectors
	Vector<Int> tmpInt(rowRef.nrow(),0);

	tmpInt.set(vb->arrayId());
    outputMsCols_p->arrayId().putColumnCells(rowRef, tmpInt);

    tmpInt.set(vb->fieldId());
    outputMsCols_p->fieldId().putColumnCells(rowRef, tmpInt);

    if (combinespws_p)
    {
    	tmpInt.set(0);
    }
    else
    {
    	tmpInt.set(vb->spectralWindow());
    }
	outputMsCols_p->dataDescId().putColumnCells(rowRef, tmpInt);


	// For the row-variable columns we can use the VisBuffer vectors
	writeVector(vb->antenna1(),outputMsCols_p->antenna1(),rowRef);
	writeVector(vb->antenna2(),outputMsCols_p->antenna2(),rowRef);
	writeVector(vb->feed1(),outputMsCols_p->feed1(),rowRef);
	writeVector(vb->feed2(),outputMsCols_p->feed2(),rowRef);
	writeVector(vb->processorId(),outputMsCols_p->processorId(),rowRef);
	writeVector(vb->observationId(),outputMsCols_p->observationId(),rowRef);
	writeVector(vb->scan(),outputMsCols_p->scanNumber(),rowRef);
	writeVector(vb->stateId(),outputMsCols_p->stateId(),rowRef);
	writeVector(vb->time(),outputMsCols_p->time(),rowRef);
	writeVector(vb->timeCentroid(),outputMsCols_p->timeCentroid(),rowRef);
	writeVector(vb->timeInterval(),outputMsCols_p->interval(),rowRef);
	writeVector(vb->exposure(),outputMsCols_p->exposure(),rowRef);
	// UVW does not depend on SPW because is given in meters
	writeMatrix(vb->uvw(),outputMsCols_p->uvw(),rowRef);


	// FLAG_ROW has to be re-calculated because different SPWs may have different flags
	if (combinespws_p)
	{
		writeTransformedVector(vb->flagRow(),outputMsCols_p->flagRow(),rowRef,vb,False);
	}
	else
	{
		writeVector(vb->flagRow(),outputMsCols_p->flagRow(),rowRef);
	}


	// Weights can be re-used to calculate sigma
	Matrix<Float> weights;
	if (combinespws_p)
	{
		transformMatrix(vb->weightMat(),weights, vb,True);
		outputMsCols_p->weight().putColumnCells(rowRef, weights);
	}
	else
	{
		weights = vb->weightMat();;
	}


	// Sigma must be redefined to 1/weight when corrected data becomes data
	if (correctedToData_p)
	{
		arrayTransformInPlace(weights, tvf::wtToSigma);
		outputMsCols_p->sigma().putColumnCells(rowRef, weights);
	}
	else
	{
		if (combinespws_p)
		{
			writeTransformedMatrix(vb->sigmaMat(),outputMsCols_p->sigma(),rowRef,vb,True);
		}
		else
		{
			writeMatrix(vb->sigmaMat(),outputMsCols_p->sigma(),rowRef);
		}
	}

	return;
}

// ----------------------------------------------------------------------------------------
// Fill main (data) columns which have to be combined together to produce bigger SPWs
// ----------------------------------------------------------------------------------------
void MSTransformDataHandler::fillDataCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
	for (dataColMap::iterator iter = dataColMap_p.begin();iter != dataColMap_p.end();iter++)
	{
		switch (iter->first)
		{
			case MS::DATA:
			{
				if (combinespws_p)
				{
					writeTransformedCube(vb->visCube(),outputMsCols_p->data(),rowRef,vb);
				}
				else
				{
					writeCube(vb->visCube(),outputMsCols_p->data(),rowRef);
				}
				break;
			}
			case MS::CORRECTED_DATA:
			{
				if (iter->second == MS::DATA)
				{
					if (combinespws_p)
					{
						writeTransformedCube(vb->visCubeCorrected(),outputMsCols_p->data(),rowRef,vb);
					}
					else
					{
						writeCube(vb->visCubeCorrected(),outputMsCols_p->data(),rowRef);
					}
				}
				else
				{
					if (combinespws_p)
					{
						writeTransformedCube(vb->visCubeCorrected(),outputMsCols_p->correctedData(),rowRef,vb);
					}
					else
					{
						writeCube(vb->visCubeCorrected(),outputMsCols_p->correctedData(),rowRef);
					}
				}
				break;
			}
			case MS::MODEL_DATA:
			{
				if (iter->second == MS::DATA)
				{
					if (combinespws_p)
					{
						writeTransformedCube(vb->visCubeModel(),outputMsCols_p->data(),rowRef,vb);
					}
					else
					{
						writeCube(vb->visCubeModel(),outputMsCols_p->data(),rowRef);
					}
				}
				else
				{
					if (combinespws_p)
					{
						writeTransformedCube(vb->visCubeModel(),outputMsCols_p->modelData(),rowRef,vb);
					}
					else
					{
						writeCube(vb->visCubeModel(),outputMsCols_p->modelData(),rowRef);
					}
				}
				break;
			}
			case MS::FLOAT_DATA:
			{
				if (combinespws_p)
				{
					writeTransformedCube(vb->visCubeFloat(),outputMsCols_p->floatData(),rowRef,vb);
				}
				else
				{
					writeCube(vb->visCubeFloat(),outputMsCols_p->floatData(),rowRef);
				}
				break;
			}
			case MS::LAG_DATA:
			{
				// jagonzal: TODO
				break;
			}
			default:
			{
				// jagonzal: TODO
				break;
			}
		}
	}


	if (combinespws_p)
	{
		writeTransformedCube(vb->flagCube(),outputMsCols_p->flag(),rowRef,vb);
	}
	else
	{
		writeCube(vb->flagCube(),outputMsCols_p->flag(),rowRef);
	}


    if (fillWeightSpectrum_p)
    {
    	if (combinespws_p)
    	{
    		writeTransformedCube(vb->weightSpectrum(),outputMsCols_p->weightSpectrum(),rowRef,vb);
    	}
    	else
    	{
    		writeCube(vb->weightSpectrum(),outputMsCols_p->weightSpectrum(),rowRef);
    	}
    }

    // Special case for flag category
    if (fillFlagCategory_p)
    {
    	if (combinespws_p)
    	{
    		// TODO
    	}
    	else
    	{
            IPosition shapeFlagCategory = vb->flagCategory().shape();
            shapeFlagCategory(3) = rowRef.nrow();
            Array<Bool> flagCategory(shapeFlagCategory,const_cast<Bool*>(vb->flagCategory().getStorage(tvf::False)),SHARE);
        	outputMsCols_p->flagCategory().putColumnCells(rowRef, flagCategory);
    	}
    }

	return;
}

// -----------------------------------------------------------------------
// Generic method to write a Vector from a VisBuffer into a ScalarColumn
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::writeVector(const Vector<T> &inputVector,ScalarColumn<T> &outputCol, RefRows &rowRef)
{
	IPosition shape = inputVector.shape();
	shape(0) = rowRef.nrows();
    Array<T> outputArray(shape,const_cast<T*>(inputVector.getStorage(tvf::False)),SHARE);
    outputCol.putColumnCells(rowRef, outputArray);

	return;
}

// -----------------------------------------------------------------------
// Generic method to write a Matrix from a VisBuffer into a ArrayColumn
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::writeMatrix(const Matrix<T> &inputMatrix,ArrayColumn<T> &outputCol, RefRows &rowRef)
{
	IPosition shape = inputMatrix.shape();
	shape(1) = rowRef.nrows();
    Array<T> outputArray(shape,const_cast<T*>(inputMatrix.getStorage(tvf::False)),SHARE);
    outputCol.putColumnCells(rowRef, outputArray);

	return;
}

// -----------------------------------------------------------------------
// Generic method to write a Cube from a VisBuffer into a ArrayColumn
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::writeCube(const Cube<T> &inputCube,ArrayColumn<T> &outputCol, RefRows &rowRef)
{
	IPosition shape = inputCube.shape();
	shape(2) = rowRef.nrows();
    Array<T> outputArray(shape,const_cast<T*>(inputCube.getStorage(tvf::False)),SHARE);
    outputCol.putColumnCells(rowRef, outputArray);

	return;
}

// -----------------------------------------------------------------------
// Fill the data from an input vector with shape [nBaselinesxnSPWs] into an
// output vector with shape [nBaselines] accumulating the values from all SPWS
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::transformVector(const Vector<T> &inputVector, Vector<T> &outputVector, vi::VisBuffer2 *vb, Bool convolveFlags)
{
	// Create output array
	IPosition shape = inputVector.shape();
	shape(0) = baselineMap_p.size();

	// Re-shape output vector but don't copy values
	outputVector.resize(shape,False);

	// Re-initialize output vector
	outputVector = T();

    // Fill output array with the combined data from each SPW
	uInt row;
	uInt baseline_index = 0;
	vector<uInt> baselineRows;
	for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
	{
		// Get baseline rows vector
		baselineRows = iter->second;

		// Compute combined value from each SPW
		for (vector<uInt>::iterator iter = baselineRows.begin();iter != baselineRows.end(); iter++)
		{
			row = *iter;
			outputVector(baseline_index) += inputVector(row);
		}

		baseline_index += 1;
	}

	return;
}

// -----------------------------------------------------------------------
// Fill the data from an input vector with shape [nCol,nBaselinesxnSPWs] into an
// output vector with shape [nCol,nBaselines] accumulating the values from all SPWS
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::transformMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix, vi::VisBuffer2 *vb, Bool convolveFlags)
{
	// Create output array
	IPosition shape = inputMatrix.shape();
	shape(1) = baselineMap_p.size();
	uInt nCols = shape(0);

	// Re-shape output matrix but don't copy values
	outputMatrix.resize(shape,False);

	// Re-initialize output matrix
	outputMatrix = T();

    // Access FLAG_ROW in case we need to convolute the average
    Vector<Bool> flags = vb->flagRow();

    // Fill output array with the combined data from each SPW
	uInt row;
	uInt baseline_index = 0;
	Double normalizingFactor = 0;
	Double contributionFactor = 0;
	vector<uInt> baselineRows;
	for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
	{
		// Get baseline rows vector
		baselineRows = iter->second;

		// Reset normalizing factor
		normalizingFactor = 0;

		// Compute combined value from each SPW
		for (vector<uInt>::iterator iter = baselineRows.begin();iter != baselineRows.end(); iter++)
		{
			row = *iter;
			if (convolveFlags)
			{
				contributionFactor = !flags(row);
			}
			else
			{
				contributionFactor = 1;
			}

			for (uInt col = 0; col < nCols; col++)
			{
				outputMatrix(col,baseline_index) += contributionFactor*inputMatrix(col,row);
			}

			normalizingFactor += contributionFactor;
		}

		// Normalize accumulated value
		if (normalizingFactor>0)
		{
			for (uInt col = 0; col < nCols; col++)
			{
				outputMatrix(col,baseline_index) /= normalizingFactor;
			}
		}

		baseline_index += 1;
	}

	return;
}

// -----------------------------------------------------------------------
// Get and write transformed vector to output MS
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::writeTransformedVector(const Vector<T> &inputVector,ScalarColumn<T> &outputCol,RefRows &rowRef,vi::VisBuffer2 *vb, Bool convolveFlags)
{
	// Get transformed vector
	Vector<T> outputVector;
	transformVector(inputVector, outputVector, vb, convolveFlags);

	// Write transformed vector
	outputCol.putColumnCells(rowRef, outputVector);

	return;
}

// -----------------------------------------------------------------------
// Get and write transformed matrix to output MS
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::writeTransformedMatrix(const Matrix<T> &inputMatrix,ArrayColumn<T> &outputCol,RefRows &rowRef,vi::VisBuffer2 *vb, Bool convolveFlags)
{
	// Get transformed matrix
	Matrix<T> outputMatrix;
	transformMatrix(inputMatrix, outputMatrix, vb, convolveFlags);

	// Write transformed matrix
	outputCol.putColumnCells(rowRef, outputMatrix);

	return;
}

// -----------------------------------------------------------------------
// Fill the data from an input cube with shape [nPol,nChan,nBaselinesxnSPWs] into
// an output cube with shape [nPol,nChanxnSPWs,nBaselines] resolving overlaps
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::writeTransformedCube(const Cube<T> &inputCube,ArrayColumn<T> &outputCol,RefRows &rowRef, vi::VisBuffer2 *vb,Bool convolveFlags)
{
	// Get spectral window
	Vector<Int> spws = vb->spectralWindows();

	// Get input cube shape
	IPosition inputCubeShape = inputCube.shape();
	uInt nInputCorrelations = inputCubeShape(0);
	uInt nInputChannels = inputCubeShape(1);

	// Define output plane shape
	uInt nOutputChannels = combinedSpw_p.NUM_CHAN;
	IPosition planeShape(2,nInputCorrelations, nOutputChannels);

	Int spw = 0;
	Double fraction = 0;
	uInt row = 0, baseline_index = 0, outputChannel = 0;
	vector<uInt> baselineRows;
	for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
	{
		// Initialize output plane
		Matrix<T> outputPlane(planeShape,T());

		// Get baseline rows vector
		baselineRows = iter->second;

		// Fill output plane with values from input cube
		for (vector<uInt>::iterator iter = baselineRows.begin();iter != baselineRows.end(); iter++)
		{
			// Get spectral window for this row
			row = *iter;
			spw = spws(row);

			for (uInt inputChannel = 0; inputChannel < nInputChannels; inputChannel++)
			{
				// Get output channel for this input spw:channel
				outputChannel = inputOutputWeightChannelMap_p[spw][inputChannel].first;
				fraction = inputOutputWeightChannelMap_p[spw][inputChannel].second;

				for (uInt pol = 0; pol < nInputCorrelations; pol++)
				{
					outputPlane(pol,outputChannel) += inputCube(pol,inputChannel,row)*fraction;
				}
			}
		}

		// Write output plane
		outputCol.setShape(rowRef.firstRow()+baseline_index,planeShape);
		outputCol.put(rowRef.firstRow()+baseline_index, outputPlane);
		baseline_index += 1;
	}

	return;
}

} //# NAMESPACE CASA - END
