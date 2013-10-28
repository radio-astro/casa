//# MSTransformManager.cc: This file contains the implementation of the MSTransformManager class.
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

#include <mstransform/MSTransform/MSTransformManager.h>

namespace casa { //# NAMESPACE CASA - BEGIN

/////////////////////////////////////////////
////// MS Transform Framework utilities /////
/////////////////////////////////////////////
namespace MSTransformations
{
	Double wtToSigma(Double wt)
	{
		return wt > 0.0 ? 1.0 / sqrt(wt) : -1.0;
	}

	Bool False = False;
	Bool True = False;
	Unit Hz(String("Hz"));

	enum InterpolationMethod {
	    // nearest neighbour
	    nearestNeighbour,
	    // linear
	    linear,
	    // cubic
	    cubic,
	    // cubic spline
	    spline,
	    // fft shift
	    fftshift
	  };

	enum WeightingSetup {
		spectrum,
		flags,
		flat
	};
}

/////////////////////////////////////////////
/// MSTransformManager implementation ///
/////////////////////////////////////////////

// -----------------------------------------------------------------------
// Default constructor
// -----------------------------------------------------------------------
MSTransformManager::MSTransformManager()
{
	initialize();
	return;
}


// -----------------------------------------------------------------------
// Configuration constructor
// -----------------------------------------------------------------------
MSTransformManager::MSTransformManager(Record configuration)
{
	initialize();
	configure(configuration);
	return;
}


// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
MSTransformManager::~MSTransformManager()
{
	if (channelSelector_p) delete channelSelector_p;
	if (visibilityIterator_p) delete visibilityIterator_p;
	if (dataHandler_p) delete dataHandler_p;
	if (phaseCenterPar_p) delete phaseCenterPar_p;

	return;
}

// -----------------------------------------------------------------------
// Method to initialize members to default values
// -----------------------------------------------------------------------
void MSTransformManager::initialize()
{
	// MS specification parameters
	inpMsName_p = String("");
	outMsName_p = String("");
	datacolumn_p = String("CORRECTED");
	realmodelcol_p = False;
	tileShape_p.resize(1,False);
	//TileShape of size 1 can have 2 values [0], and [1] ...these are used in to
	//determine the tileshape by using MSTileLayout. Otherwise it has to be a
	//vector size 3 e.g [4, 15, 351] => a tile shape of 4 stokes, 15 channels 351
	//rows.
	tileShape_p(0) = 0;

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

	// Input-Output index maps
	inputOutputObservationIndexMap_p.clear();
	inputOutputArrayIndexMap_p.clear();
	inputOutputScanIndexMap_p.clear();
	inputOutputScanIntentIndexMap_p.clear();
	inputOutputFieldIndexMap_p.clear();
	inputOutputSPWIndexMap_p.clear();
	inputOutputDDIndexMap_p.clear();
	inputOutputAntennaIndexMap_p.clear();
	outputInputSPWIndexMap_p.clear();
//	outputInputDDIndexMap_p.clear();

	// Frequency transformation parameters
	nspws_p = 1;
	ddiStart_p = 0;
	combinespws_p = False;
	channelAverage_p = False;
	hanningSmooth_p = False;
	refFrameTransformation_p = False;
	freqbin_p = Vector<Int>(1,-1);
	useweights_p = "flags";
	weightmode_p = MSTransformations::flags;
	interpolationMethodPar_p = String("linear");	// Options are: nearest, linear, cubic, spline, fftshift
	phaseCenterPar_p = new casac::variant("");
	userPhaseCenter_p = False;
	restFrequency_p = String("");
	outputReferenceFramePar_p = String("");			// Options are: LSRK, LSRD, BARY, GALACTO, LGROUP, CMB, GEO, or TOPO

	// Frequency specification parameters
	mode_p = String("channel"); 					// Options are: channel, frequency, velocity
	start_p = String("0");
	width_p = String("1");
	nChan_p = -1;									// -1 means use all the input channels
	velocityType_p = String("radio");				// When mode is velocity options are: optical, radio

	// Time transformation parameters
	timeAverage_p = False;
	timeBin_p = 0.0;
	timespan_p = String("");
	timeAvgOptions_p = vi::AveragingOptions(vi::AveragingOptions::Nothing);
	maxuvwdistance_p = 0;
	// minbaselines_p = 0;

	// Weight Spectrum parameters
	usewtspectrum_p = False;

	// MS-related members
	dataHandler_p = NULL;
	inputMs_p = NULL;
	selectedInputMs_p = NULL;
	outputMs_p = NULL;
	selectedInputMsCols_p = NULL;
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
	channelSelector_p = NULL;

	// Output MS structure related members
	fillFlagCategory_p = False;
	inputWeightSpectrumAvailable_p = False;
	correctedToData_p = False;
	dataColMap_p.clear();
	mainColumn_p = MS::CORRECTED_DATA;

	// Frequency transformation members
	chansPerOutputSpw_p = 0;
	tailOfChansforLastSpw_p = 0;
	interpolationMethod_p = MSTransformations::linear;
	baselineMap_p.clear();
	rowIndex_p.clear();
	spwChannelMap_p.clear();
	inputOutputSpwMap_p.clear();
	inputOutputChanFactorMap_p.clear();
	freqbinMap_p.clear();
	numOfInpChanMap_p.clear();
	numOfSelChanMap_p.clear();
	numOfOutChanMap_p.clear();
	numOfCombInputChanMap_p.clear();
	numOfCombInterChanMap_p.clear();
	weightFactorMap_p.clear();
	sigmaFactorMap_p.clear();

	// Reference frame transformation members
	fftShiftEnabled_p = False;
	fftShift_p = 0;

	// Weight Spectrum members
	inputWeightSpectrumAvailable_p = False;
	combinationOfSPWsWithDifferentExposure_p = False;

	// Transformations - related function pointers
	transformCubeOfDataComplex_p = NULL;
	transformCubeOfDataFloat_p = NULL;
	fillWeightsPlane_p = NULL;
	setWeightsPlaneByReference_p = NULL;
	setWeightStripeByReference_p = NULL;
	transformStripeOfDataComplex_p = NULL;
	transformStripeOfDataFloat_p = NULL;
	averageKernelComplex_p = NULL;
	averageKernelFloat_p = NULL;

	// I/O related function pointers
	writeOutputPlanesComplex_p = NULL;
	writeOutputPlanesFloat_p = NULL;
	writeOutputFlagsPlane_p = NULL;
	writeOutputFlagsPlaneSlices_p = NULL;
	writeOutputFlagsPlaneReshapedSlices_p = NULL;

	// Buffer handling members
	bufferMode_p = False;
	nRowsToAdd_p = 0;

	return;
}

// -----------------------------------------------------------------------
// Method to parse the configuration parameters
// -----------------------------------------------------------------------
void MSTransformManager::configure(Record &configuration)
{
	parseMsSpecParams(configuration);
	parseDataSelParams(configuration);
	parseFreqTransParams(configuration);
	parseChanAvgParams(configuration);
	parseRefFrameTransParams(configuration);
	parseTimeAvgParams(configuration);

	return;
}

// -----------------------------------------------------------------------
// Method to parse input/output MS specification
// -----------------------------------------------------------------------
void MSTransformManager::parseMsSpecParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("inputms");
	if (exists >= 0)
	{
		configuration.get (exists, inpMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Input file name is " << inpMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("buffermode");
	if (exists >= 0)
	{
		configuration.get (exists, bufferMode_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Buffer mode is on " << LogIO::POST;
	}

	if (bufferMode_p)
	{
		datacolumn_p = "ALL";

		exists = configuration.fieldNumber ("outputms");
		if (exists >= 0)
		{
			configuration.get (exists, outMsName_p);
		}
	}
	else
	{
		exists = configuration.fieldNumber ("outputms");
		if (exists >= 0)
		{
			configuration.get (exists, outMsName_p);
			logger_p 	<< LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
						<< "Output file name is " << outMsName_p << LogIO::POST;
		}

		exists = configuration.fieldNumber ("datacolumn");
		if (exists >= 0)
		{
			configuration.get (exists, datacolumn_p);
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Data column is " << datacolumn_p << LogIO::POST;
		}

		exists = configuration.fieldNumber ("realmodelcol");
		if (exists >= 0)
		{
			configuration.get (exists, realmodelcol_p);
			if (realmodelcol_p)
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
						<< "MODEL column will be made real in the output MS " << LogIO::POST;
			}
		}

		exists = configuration.fieldNumber ("usewtspectrum");
		if (exists >= 0)
		{
			configuration.get (exists, usewtspectrum_p);
			if (usewtspectrum_p)
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
						<< "WEIGHT_SPECTRUM will be written in output MS " << LogIO::POST;
			}
		}

		exists = configuration.fieldNumber ("tileshape");
		if (exists >= 0)
		{
			if ( configuration.type(exists) == casa::TpInt )
			{
				Int mode;
				configuration.get (exists, mode);
				tileShape_p = Vector<Int>(1,mode);
			}
			else if ( configuration.type(exists) == casa::TpArrayInt)
			{
				configuration.get (exists, tileShape_p);
			}

			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Tile shape is " << tileShape_p << LogIO::POST;
		}
	}

	return;
}

// -----------------------------------------------------------------------
// Method to parse the data selection parameters
// -----------------------------------------------------------------------
void MSTransformManager::parseDataSelParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("array");
	if (exists >= 0)
	{
		configuration.get (exists, arraySelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "array selection is " << arraySelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("field");
	if (exists >= 0)
	{
		configuration.get (exists, fieldSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "field selection is " << fieldSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("scan");
	if (exists >= 0)
	{
		configuration.get (exists, scanSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "scan selection is " << scanSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("timerange");
	if (exists >= 0)
	{
		configuration.get (exists, timeSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "timerange selection is " << timeSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("spw");
	if (exists >= 0)
	{
		configuration.get (exists, spwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "spw selection is " << spwSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("antenna");
	if (exists >= 0)
	{
		configuration.get (exists, baselineSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "antenna selection is " << baselineSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("uvrange");
	if (exists >= 0)
	{
		configuration.get (exists, uvwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "uvrange selection is " << uvwSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("correlation");
	if (exists >= 0)
	{
		configuration.get (exists, polarizationSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "correlation selection is " << polarizationSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("observation");
	if (exists >= 0)
	{
		configuration.get (exists, observationSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<<"observation selection is " << observationSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("intent");
	if (exists >= 0)
	{
		configuration.get (exists, scanIntentSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "scan intent selection is " << scanIntentSelection_p << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to parse the channel average parameters
// -----------------------------------------------------------------------
void MSTransformManager::parseChanAvgParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("chanaverage");
	if (exists >= 0)
	{
		configuration.get (exists, channelAverage_p);
		if (channelAverage_p)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Channel average is activated" << LogIO::POST;
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}

	exists = configuration.fieldNumber ("chanbin");
	if (exists >= 0)
	{
		if ( configuration.type(exists) == casa::TpInt )
		{
			Int freqbin;
			configuration.get (exists, freqbin);
			freqbin_p = Vector<Int>(1,freqbin);
		}
		else if ( configuration.type(exists) == casa::TpArrayInt)
		{
			configuration.get (exists, freqbin_p);
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Wrong format for chanbin parameter (only Int and arrayInt are supported) " << LogIO::POST;
		}

		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Channel bin is " << freqbin_p << LogIO::POST;
	}
	else
	{
		logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Channel average is activated but no chanbin parameter provided " << LogIO::POST;
		channelAverage_p = False;
		return;
	}

	exists = configuration.fieldNumber ("useweights");
	if (exists >= 0)
	{
		configuration.get (exists, useweights_p);

		useweights_p.downcase();

		if (useweights_p == "flags")
		{
			weightmode_p = MSTransformations::flags;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Using FLAGS as weights for channel average" << LogIO::POST;
		}
		else if (useweights_p == "spectrum")
		{
			weightmode_p = MSTransformations::spectrum;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Using WEIGHT_SPECTRUM as weights for channel average" << LogIO::POST;
		}
		else
		{
			weightmode_p = MSTransformations::flags;
			useweights_p = String("flags");
		}
	}


	return;
}

// -----------------------------------------------------------------------
// Method to parse the frequency transformation parameters
// -----------------------------------------------------------------------
void MSTransformManager::parseFreqTransParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("combinespws");
	if (exists >= 0)
	{
		configuration.get (exists, combinespws_p);

		if (combinespws_p)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Combine Spectral Windows is activated" << LogIO::POST;
		}
	}

	exists = configuration.fieldNumber ("ddistart");
	if (exists >= 0)
	{
		configuration.get (exists, ddiStart_p);
		if (ddiStart_p > 0)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "DDI start is " << ddiStart_p << LogIO::POST;
		}
		else
		{
			ddiStart_p = 0;
		}

	}

	exists = configuration.fieldNumber ("hanning");
	if (exists >= 0)
	{
		configuration.get (exists, hanningSmooth_p);

		if (hanningSmooth_p)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Hanning Smooth is activated" << LogIO::POST;
		}
	}

	return;
}

// -----------------------------------------------------------------------
// Method to parse the reference frame transformation parameters
// -----------------------------------------------------------------------
void MSTransformManager::parseRefFrameTransParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("regridms");
	if (exists >= 0)
	{
		configuration.get (exists, refFrameTransformation_p);

		if (refFrameTransformation_p)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Regrid MS is activated"<< LogIO::POST;
		}
		else
		{
			return;
		}
	}

	exists = configuration.fieldNumber ("phasecenter");
	if (exists >= 0)
	{
		//If phase center is a simple numeric value then it is taken
		// as a FIELD_ID otherwise it is converted to a MDirection
        if( configuration.type(exists) == TpInt )
        {
        	int fieldIdForPhaseCenter = -1;
    		configuration.get (exists, fieldIdForPhaseCenter);
    		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
    				<< "Field Id for phase center is " << fieldIdForPhaseCenter << LogIO::POST;
    		if (phaseCenterPar_p) delete phaseCenterPar_p;
    		phaseCenterPar_p = new casac::variant(fieldIdForPhaseCenter);
        }
        else
        {
        	String phaseCenter("");
    		configuration.get (exists, phaseCenter);
    		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
    				<< "Phase center is " << phaseCenter << LogIO::POST;
    		if (phaseCenterPar_p) delete phaseCenterPar_p;
    		phaseCenterPar_p = new casac::variant(phaseCenter);
        }
	}

	exists = configuration.fieldNumber ("restfreq");
	if (exists >= 0)
	{
		configuration.get (exists, restFrequency_p);
		if (!restFrequency_p.empty())
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Rest frequency is " << restFrequency_p << LogIO::POST;
		}
	}

	exists = configuration.fieldNumber ("outframe");
	if (exists >= 0)
	{
		configuration.get (exists, outputReferenceFramePar_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Output reference frame is " << outputReferenceFramePar_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("interpolation");
	if (exists >= 0)
	{
		configuration.get (exists, interpolationMethodPar_p);

		if (interpolationMethodPar_p.contains("nearest"))
		{
			interpolationMethod_p = MSTransformations::nearestNeighbour;
		}
		else if (interpolationMethodPar_p.contains("linear"))
		{
			interpolationMethod_p = MSTransformations::linear;
		}
		else if (interpolationMethodPar_p.contains("cubic"))
		{
			interpolationMethod_p = MSTransformations::cubic;
		}
		else if (interpolationMethodPar_p.contains("spline"))
		{
			interpolationMethod_p = MSTransformations::spline;
		}
		else if (interpolationMethodPar_p.contains("fftshift"))
		{
			fftShiftEnabled_p = True;
			interpolationMethod_p = MSTransformations::linear;
		}
		else
		{
			logger_p << LogIO::SEVERE << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Interpolation method " << interpolationMethodPar_p  << " not available " << LogIO::POST;
		}

		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Interpolation method is " << interpolationMethodPar_p  << LogIO::POST;
	}
	else
	{
		interpolationMethod_p = MSTransformations::linear;
	}

	exists = configuration.fieldNumber ("nspw");
	if (exists >= 0)
	{
		configuration.get (exists, nspws_p);

		if (nspws_p > 1)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Number of output SPWs is " << nspws_p << LogIO::POST;
			combinespws_p = True;
		}
		else
		{
			nspws_p = 1;
		}
	}

	parseFreqSpecParams(configuration);

	return;
}

// -----------------------------------------------------------------------
// Method to parse the frequency selection specification
// -----------------------------------------------------------------------
void MSTransformManager::parseFreqSpecParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("mode");
	if (exists >= 0)
	{
		configuration.get (exists, mode_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Mode is " << mode_p<< LogIO::POST;

		if ((mode_p == "frequency") or (mode_p == "velocity"))
		{
			start_p = String("");
			width_p = String("");
		}
	}

	exists = configuration.fieldNumber ("nchan");
	if (exists >= 0)
	{
		configuration.get (exists, nChan_p);
		if (nspws_p > 1)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Number of channels per output spw is " << nChan_p << LogIO::POST;
			nChan_p *=  nspws_p;
		}
		else
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Number of channels is " << nChan_p<< LogIO::POST;
		}
	}

	exists = configuration.fieldNumber ("start");
	if (exists >= 0)
	{
		configuration.get (exists, start_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Start is " << start_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("width");
	if (exists >= 0)
	{
		configuration.get (exists, width_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Width is " << width_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("veltype");
	if ((exists >= 0) and (mode_p == "velocity"))
	{
		configuration.get (exists, velocityType_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Velocity type is " << velocityType_p << LogIO::POST;
	}

	return;
}

void MSTransformManager::parseTimeAvgParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("timeaverage");
	if (exists >= 0)
	{
		configuration.get (exists, timeAverage_p);

		if (timeAverage_p)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Time average is activated" << LogIO::POST;
		}
	}
	else
	{
		return;
	}

	if (timeAverage_p)
	{
		exists = configuration.fieldNumber ("timebin");
		if (exists >= 0)
		{
			String timebin;
			configuration.get (exists, timebin);
			timeBin_p=casaQuantity(timebin).get("s").getValue();
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Time bin is " << timeBin_p << LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Time average is activated but no timebin parameter provided " << LogIO::POST;
			timeAverage_p = False;
			return;
		}

		exists = configuration.fieldNumber ("timespan");
		if (exists >= 0)
		{
			configuration.get (exists, timespan_p);

			if (!timespan_p.contains("scan") and !timespan_p.contains("state"))
			{
				timespan_p = String("");
			}
			else
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
						<< "Time span is " << timespan_p << LogIO::POST;
			}
		}

		exists = configuration.fieldNumber ("maxuvwdistance");
		if (exists >= 0)
		{
			configuration.get (exists, maxuvwdistance_p);

			if (maxuvwdistance_p > 0)
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
						<< "Maximum UV distance for baseline-dependent time average is: " << maxuvwdistance_p << LogIO::POST;
			}
		}

		/*
		exists = configuration.fieldNumber ("minbaselines");
		if (exists >= 0)
		{
			configuration.get (exists, minbaselines_p);

			if (minbaselines_p > 0)
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
						<< "Minimum number of baselines for time average: " << minbaselines_p << LogIO::POST;
			}
		}
		*/
	}

	return;
}

// -----------------------------------------------------------------------
// Method to open the input MS, select the data and create the
// structure of the output MS filling the auxiliary tables.
// -----------------------------------------------------------------------
void MSTransformManager::open()
{
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
			<< "Select data" << LogIO::POST;

	dataHandler_p = new MSTransformDataHandler(inpMsName_p,Table::Old);

	// WARNING: Input MS is re-set at the end of a successful MSTransformDataHandler::makeMSBasicStructure,
	// call therefore we have to use the selected MS always
	inputMs_p = dataHandler_p->getInputMS();
	// Note: We always get the input number of channels because we don't know if pre-averaging will be necessary
	getInputNumberOfChannels();

	// Once the input MS is opened we can get the selection indexes,
	// in this way we also validate the selection parameters
	checkDataColumnsToFill();
	initDataSelectionParams();

	// Determine channel specification for output MS
	Vector<Int> chanSpec;
	Bool spectralRegridding = combinespws_p or refFrameTransformation_p;
	if (channelAverage_p and !spectralRegridding)
	{
		chanSpec =  freqbin_p;
	}
	else
	{
		chanSpec = Vector<Int>(1,1);
	}

	// Set-up splitter object
	const String dummyExpr = String("");
	dataHandler_p->setmsselect((const String)spwSelection_p,
							(const String)fieldSelection_p,
							(const String)baselineSelection_p,
							(const String)scanSelection_p,
							(const String)uvwSelection_p,
							dummyExpr, // taqlExpr
							chanSpec,
							(const String)arraySelection_p,
							(const String)polarizationSelection_p,
							(const String)scanIntentSelection_p,
							(const String)observationSelection_p);

	dataHandler_p->selectTime(timeBin_p,timeSelection_p);

	// Create output MS structure
	if (not bufferMode_p)
	{
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Create output MS structure" << LogIO::POST;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Create transformed MS Subtables to be stored in memory" << LogIO::POST;
	}


	//jagonzal (CAS-5174)dataHandler_p->fillMainTable_p = False;
	Bool selectionOk = False;
	if (not bufferMode_p)
	{
		selectionOk = dataHandler_p->makeMSBasicStructure(outMsName_p,datacolumn_p,tileShape_p,timespan_p);
	}
	else
	{
		selectionOk = dataHandler_p->makeMSBasicStructure(outMsName_p,datacolumn_p,tileShape_p,timespan_p,Table::Scratch);
	}

	if (!selectionOk)
	{
		logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Combination of selection ranges produces a NullSelection" << LogIO::POST;
		throw(MSSelectionNullSelection("MSSelectionNullSelection : The selected table has zero rows."));
	}

	// jagonzal (CAS-5076): Reindex state column when there is scan selection
	map<Int, Int> stateRemapper = dataHandler_p->getStateRemapper();
    std::map<Int, Int>::iterator stateRemapperIter;
    for (	stateRemapperIter = stateRemapper.begin();
    		stateRemapperIter != stateRemapper.end();
    		stateRemapperIter++)
    {
    	inputOutputScanIntentIndexMap_p[stateRemapperIter->first] = stateRemapperIter->second;

    	logger_p << LogIO::DEBUG1 << LogOrigin("MSTransformManager", __FUNCTION__)
    			<< "State " << stateRemapperIter->first << " mapped to " << stateRemapperIter->second << LogIO::POST;
    }

    // jagonzal (CAS-5349): Reindex antenna columns when there is antenna selection
    Vector<Int> antennaRemapper = dataHandler_p->getAntennaRemapper();
    if (!baselineSelection_p.empty())
    {
    	for (uInt oldIndex=0;oldIndex<antennaRemapper.size();oldIndex++)
    	{
    		inputOutputAntennaIndexMap_p[oldIndex] = antennaRemapper[oldIndex];
    	}
    }


	selectedInputMs_p = dataHandler_p->getSelectedInputMS();
	outputMs_p = dataHandler_p->getOutputMS();
	selectedInputMsCols_p = dataHandler_p->getSelectedInputMSColumns();
	outputMsCols_p = dataHandler_p->getOutputMSColumns();

	return;
}

// -----------------------------------------------------------------------
// Method to close the output MS
// -----------------------------------------------------------------------
void MSTransformManager::close()
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
// Check configuration and input MS characteristics to determine run parameters
// -----------------------------------------------------------------------
void MSTransformManager::setup()
{
	// Check if we really need to combine SPWs
	if (combinespws_p)
	{
		uInt nInputSpws = outputMs_p->spectralWindow().nrow();
		if (nInputSpws < 2)
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "There is only one selected SPW, no need to combine " << LogIO::POST;
			combinespws_p = False;
		}
	}

	// Regrid SPW subtable
	if (combinespws_p)
	{
		initRefFrameTransParams();
		regridAndCombineSpwSubtable();
		reindexSourceSubTable();
		reindexDDISubTable();
		reindexFeedSubTable();
		reindexSysCalSubTable();
		reindexFreqOffsetSubTable();
	}
	else if (refFrameTransformation_p)
	{
		initRefFrameTransParams();
		regridSpwSubTable();
	}

	//// Determine the frequency transformation methods to use ////


	// Cube level
	if (combinespws_p)
	{
		transformCubeOfDataComplex_p = &MSTransformManager::combineCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformManager::combineCubeOfData;
	}
	else if (refFrameTransformation_p)
	{
		transformCubeOfDataComplex_p = &MSTransformManager::regridCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformManager::regridCubeOfData;
	}
	else if (channelAverage_p)
	{
		transformCubeOfDataComplex_p = &MSTransformManager::averageCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformManager::averageCubeOfData;
	}
	else if (hanningSmooth_p)
	{
		transformCubeOfDataComplex_p = &MSTransformManager::smoothCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformManager::smoothCubeOfData;
	}
	else if (nspws_p > 1)
	{
		transformCubeOfDataComplex_p = &MSTransformManager::separateCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformManager::separateCubeOfData;
	}
	else
	{
		transformCubeOfDataComplex_p = &MSTransformManager::copyCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformManager::copyCubeOfData;
	}

	Bool spectralRegridding = combinespws_p or refFrameTransformation_p;

	// Vector level
	if (channelAverage_p and !hanningSmooth_p and !spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformManager::average;
		transformStripeOfDataFloat_p = &MSTransformManager::average;
	}
	else if (!channelAverage_p and hanningSmooth_p and !spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformManager::smooth;
		transformStripeOfDataFloat_p = &MSTransformManager::smooth;
	}
	else if (!channelAverage_p and !hanningSmooth_p and spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformManager::regrid;
		transformStripeOfDataFloat_p = &MSTransformManager::regrid;
	}
	else if (channelAverage_p and hanningSmooth_p and !spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformManager::averageSmooth;
		transformStripeOfDataFloat_p = &MSTransformManager::averageSmooth;
	}
	else if (channelAverage_p and !hanningSmooth_p and spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformManager::averageRegrid;
		transformStripeOfDataFloat_p = &MSTransformManager::averageRegrid;
	}
	else if (!channelAverage_p and hanningSmooth_p and spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformManager::smoothRegrid;
		transformStripeOfDataFloat_p = &MSTransformManager::smoothRegrid;
	}
	else if (channelAverage_p and hanningSmooth_p and spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformManager::averageSmoothRegrid;
		transformStripeOfDataFloat_p = &MSTransformManager::averageSmoothRegrid;
	}

	// Averaging kernel
	if (inputWeightSpectrumAvailable_p)
	{
		setWeightBasedTransformations(weightmode_p);
	}
	else
	{
		weightmode_p = MSTransformations::flags;
		setWeightBasedTransformations(weightmode_p);

		if (useweights_p == "spectrum")
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
			<< "Requested column for weighted channel average WEIGHT_SPECTRUM not present in input MS. Will use FLAGS as default" << LogIO::POST;
		}
	}

	// Set Regridding kernel
	if (fftShiftEnabled_p)
	{
		if (combinespws_p)
		{
			regridCoreComplex_p = &MSTransformManager::interpol1Dfftshift;
			regridCoreFloat_p = &MSTransformManager::interpol1Dfftshift;
		}
		else
		{
			regridCoreComplex_p = &MSTransformManager::fftshift;
			regridCoreFloat_p = &MSTransformManager::fftshift;
		}
	}
	else
	{
		regridCoreComplex_p = &MSTransformManager::interpol1D;
		regridCoreFloat_p = &MSTransformManager::interpol1D;
	}

	//// Determine the frequency transformation methods to use ////

	// Drop channels with non-uniform width when doing only channel average
	if (channelAverage_p and !refFrameTransformation_p and !combinespws_p )
	{
		dropNonUniformWidthChannels();
	}

	// Get number of output channels (needed by chan avg and separate SPWs when there is only 1 selected SPW)
	if (channelAverage_p or (nspws_p>1 and !combinespws_p))
	{
		getOutputNumberOfChannels();
	}

	// Determine weight and sigma factors when either auto or user channel average is set
	if (channelAverage_p)
	{
		calculateWeightAndSigmaFactors();
	}


	if (nspws_p > 1)
	{
		uInt totalNumberOfOutputChannels = 0;
		if (combinespws_p)
		{
			totalNumberOfOutputChannels = inputOutputSpwMap_p[0].second.NUM_CHAN;
		}
		// jagonzal: This is the case when there is only one input SPW and there is no need to combine
		else
		{
			uInt spwId = 0;
	    	if (outputInputSPWIndexMap_p.size()>0)
	    	{
	    		spwId = outputInputSPWIndexMap_p[0];
	    	}
	    	else
	    	{
	    		spwId = 0;
	    	}

	    	totalNumberOfOutputChannels = numOfOutChanMap_p[spwId];
		}


		chansPerOutputSpw_p = totalNumberOfOutputChannels / nspws_p;
		if (totalNumberOfOutputChannels % nspws_p)
		{
			chansPerOutputSpw_p += 1;
			tailOfChansforLastSpw_p = totalNumberOfOutputChannels - chansPerOutputSpw_p*(nspws_p-1);
		}
		else
		{
			tailOfChansforLastSpw_p = chansPerOutputSpw_p;
		}

		writeOutputPlanesComplex_p = &MSTransformManager::writeOutputPlanesInSlices;
		writeOutputPlanesFloat_p = &MSTransformManager::writeOutputPlanesInSlices;

		separateSpwSubtable();

		// CAS-5404. DDI sub-table has to be re-indexed after separating SPW sub-table
		reindexDDISubTable();
	}
	else
	{
		writeOutputPlanesComplex_p = &MSTransformManager::writeOutputPlanesInBlock;
		writeOutputPlanesFloat_p = &MSTransformManager::writeOutputPlanesInBlock;
	}


	// Check what columns have to filled
	checkFillFlagCategory();
	checkFillWeightSpectrum();

	// Generate Iterator
	setIterationApproach();
	generateIterator();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::setWeightBasedTransformations(uInt mode)
{

	if (mode == MSTransformations::spectrum)
	{
		averageKernelComplex_p = &MSTransformManager::weightAverageKernel;
		averageKernelFloat_p = &MSTransformManager::weightAverageKernel;

		fillWeightsPlane_p = &MSTransformManager::fillWeightsPlane;
		normalizeWeightsPlane_p = &MSTransformManager::normalizeWeightsPlane;
		setWeightsPlaneByReference_p = &MSTransformManager::setWeightsPlaneByReference;
		setWeightStripeByReference_p = &MSTransformManager::setWeightStripeByReference;
	}
	else
	{
		fillWeightsPlane_p = &MSTransformManager::dontfillWeightsPlane;
		normalizeWeightsPlane_p = &MSTransformManager::dontNormalizeWeightsPlane;
		setWeightsPlaneByReference_p = &MSTransformManager::dontsetWeightsPlaneByReference;
		setWeightStripeByReference_p = &MSTransformManager::dontSetWeightStripeByReference;

		if (mode == MSTransformations::flags)
		{
			averageKernelComplex_p = &MSTransformManager::flagAverageKernel;
			averageKernelFloat_p = &MSTransformManager::flagAverageKernel;
		}
		else
		{
			averageKernelComplex_p = &MSTransformManager::simpleAverageKernel;
			averageKernelFloat_p = &MSTransformManager::simpleAverageKernel;
		}
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::initDataSelectionParams()
{
	MSSelection mssel;

	if (!observationSelection_p.empty())
	{
		mssel.setObservationExpr(observationSelection_p);
		Vector<Int> observationList = mssel.getObservationList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Selected Observations Ids are " << observationList << LogIO::POST;

		for (uInt index=0; index < observationList.size(); index++)
		{
			inputOutputObservationIndexMap_p[observationList(index)] = index;
		}
	}

	if (!arraySelection_p.empty())
	{
		mssel.setArrayExpr(arraySelection_p);
		Vector<Int> arrayList = mssel.getSubArrayList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Selected Arrays Ids are " << arrayList << LogIO::POST;

		for (uInt index=0; index < arrayList.size(); index++)
		{
			inputOutputArrayIndexMap_p[arrayList(index)] = index;
		}
	}

	if (!scanSelection_p.empty())
	{
		mssel.setScanExpr(scanSelection_p);
		Vector<Int> scanList = mssel.getScanList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Selected Scans Ids are " << scanList << LogIO::POST;

		for (uInt index=0; index < scanList.size(); index++)
		{
			inputOutputScanIndexMap_p[scanList(index)] = index;
		}
	}

	if (!scanIntentSelection_p.empty())
	{
		mssel.setStateExpr(scanIntentSelection_p);
		Vector<Int> scanIntentList = mssel.getStateObsModeList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Selected Scans Intents Ids are " << scanIntentList << LogIO::POST;

		for (uInt index=0; index < scanIntentList.size(); index++)
		{
			inputOutputScanIntentIndexMap_p[scanIntentList(index)] = index;
		}
	}

	if (!fieldSelection_p.empty())
	{
		mssel.setFieldExpr(fieldSelection_p);
		Vector<Int> fieldList = mssel.getFieldList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Selected Fields Ids are " << fieldList << LogIO::POST;

		for (uInt index=0; index < fieldList.size(); index++)
		{
			inputOutputFieldIndexMap_p[fieldList(index)] = index;
		}
	}

	if (!spwSelection_p.empty())
	{
		mssel.setSpwExpr(spwSelection_p);
		Matrix<Int> spwchan = mssel.getChanList(inputMs_p);

		// Get the DD IDs of this spw selection
		Vector<Int> spwddi = mssel.getSPWDDIDList(inputMs_p);

		// Take into account the polarization selections
		if (!polarizationSelection_p.empty()){
			mssel.setPolnExpr(polarizationSelection_p);
			Vector<Int> polddi = mssel.getDDIDList(inputMs_p);
			if (polddi.size() > 0){
				// make an intersection
				Vector<Int> commonDDI = set_intersection(spwddi, polddi);
				uInt nddids = commonDDI.size();
				if (nddids > 0){
					spwddi.resize(nddids);
					for (uInt ii = 0; ii < nddids; ++ii){
						spwddi[ii] = commonDDI[ii];
					}
				}
			}
		}

		uInt nddi = spwddi.size();
		Int ddid;
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Selected SPWs Ids are " << spwchan << LogIO::POST;
		// Example of MS with repeated SPW ID in DD table:
		// DD	POL		SPW
		//	0	0		0 (RR)
		//	1	1		0 (LL)
		//	2	2		1 (RR,LL)
		//	3	3		2 (RR,LL,RL,LR)
		// example of selection: spw=0,1 correlation=RR, so selected DDs are
		//	DD	POL		SPW
		//	0	0		0
		//	2	2		1

		// Do the mapping of DD between input and output
		for(uInt selection_ii=0;selection_ii<nddi;selection_ii++)
		{
			// Get dd id and set the input-output dd map
			// This will only be used to write the DD ids in the main table
			ddid = spwddi[selection_ii];
			inputOutputDDIndexMap_p[ddid] = selection_ii + ddiStart_p;
		}

	    IPosition shape = spwchan.shape();
	    uInt nSelections = shape[0];
		Int spw,channelStart,channelStop,channelStep,channelWidth;
		if (channelSelector_p == NULL) channelSelector_p = new vi::FrequencySelectionUsingChannels();

		// Do the spw mapping between input and output
		for(uInt selection_i=0;selection_i<nSelections;selection_i++)
		{
			// Get spw id and set the input-output spw map
			spw = spwchan(selection_i,0);
			inputOutputSPWIndexMap_p[spw] = selection_i + ddiStart_p;

			outputInputSPWIndexMap_p[selection_i] = spw;

			// Set the channel selection ()
			channelStart = spwchan(selection_i,1);
			channelStop = spwchan(selection_i,2);
			channelStep = spwchan(selection_i,3);
			channelWidth = channelStop-channelStart+1;
			numOfSelChanMap_p[spw] = channelWidth;
			channelSelector_p->add (spw, channelStart, channelWidth,channelStep);
		}
	}

	// jagonzal: must fill numOfSelChanMap_p
	else
	{
		spwSelection_p = "*";
		mssel.setSpwExpr(spwSelection_p);
		Matrix<Int> spwchan = mssel.getChanList(inputMs_p);

	    IPosition shape = spwchan.shape();
	    uInt nSelections = shape[0];
		Int spw,channelStart,channelStop,channelStep,channelWidth;
		for(uInt selection_i=0;selection_i<nSelections;selection_i++)
		{
			// Get spw id and set the input-output spw map
			spw = spwchan(selection_i,0);

			// Set the channel selection ()
			channelStart = spwchan(selection_i,1);
			channelStop = spwchan(selection_i,2);
			channelStep = spwchan(selection_i,3);
			channelWidth = channelStop-channelStart+1;
			numOfSelChanMap_p[spw] = channelWidth;
		}

		spwSelection_p = "";
	}

	// If we have channel average we have to populate the freqbin map
	if (channelAverage_p)
	{
		if (!spwSelection_p.empty())
		{
			mssel.setSpwExpr(spwSelection_p);
		}
		else
		{
			mssel.setSpwExpr("*");
		}

		Vector<Int> spwList = mssel.getSpwList(inputMs_p);

		if (freqbin_p.size() == 1)
		{
			for (uInt spw_i=0;spw_i<spwList.size();spw_i++)
			{
				freqbinMap_p[spwList(spw_i)] = freqbin_p(0);
			}
		}
		else
		{
			if (spwList.size() != freqbin_p.size())
			{
				logger_p << LogIO::SEVERE << LogOrigin("MSTransformManager", __FUNCTION__)
						<< "Number of frequency bins ( "
						<< freqbin_p.size() << " ) different from number of selected spws ( "
						<< spwList.size() << " )" << LogIO::POST;
			}
			else
			{
				for (uInt spw_i=0;spw_i<spwList.size();spw_i++)
				{
					freqbinMap_p[spwList(spw_i)] = freqbin_p(spw_i);
				}
			}
		}
	}

	if (!polarizationSelection_p.empty())
	{
		mssel.setPolnExpr(polarizationSelection_p);
		Vector <Vector <Slice> > correlationSlices;
		mssel.getCorrSlices(correlationSlices,inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Selected correlations are " << correlationSlices << LogIO::POST;

		if (channelSelector_p == NULL) channelSelector_p = new vi::FrequencySelectionUsingChannels();

		channelSelector_p->addCorrelationSlices(correlationSlices);

		// Get the DDs related to the polarization selection
		// when there is no spw selection
		if (spwSelection_p.empty()){
			Vector<Int> polddids = mssel.getDDIDList(inputMs_p);

			// Make the in/out DD mapping
			uInt nddids = polddids.size();
			Int dd;
			for(uInt ii=0;ii<nddids;ii++)
			{
				// Get dd id and set the input-output dd map
				dd = polddids[ii];
				inputOutputDDIndexMap_p[dd] = ii + ddiStart_p;
			}

		}

	}

	return;
}

// -----------------------------------------------------------------------
// Method to initialize the reference frame transformation parameters
// -----------------------------------------------------------------------
void MSTransformManager::initRefFrameTransParams()
{
    // Determine input reference frame from the first row in the SPW sub-table of the output (selected) MS
	MSSpectralWindow spwTable = outputMs_p->spectralWindow();
	MSSpWindowColumns spwCols(spwTable);
    inputReferenceFrame_p = MFrequency::castType(spwCols.measFreqRef()(0));

    // Parse output reference frame
    if(outputReferenceFramePar_p.empty())
    {
    	outputReferenceFrame_p = inputReferenceFrame_p;
    }
    else if(!MFrequency::getType(outputReferenceFrame_p, outputReferenceFramePar_p))
    {
    	logger_p << LogIO::SEVERE << LogOrigin("MSTransformManager", __FUNCTION__)
    			<< "Problem parsing output reference frame:" << outputReferenceFramePar_p  << LogIO::POST;
    }

    // Determine observatory position from the first row in the observation sub-table of the output (selected) MS
    MSObservation observationTable = outputMs_p->observation();
    MSObservationColumns observationCols(observationTable);
    String observatoryName = observationCols.telescopeName()(0);
    MeasTable::Observatory(observatoryPosition_p,observatoryName);
    observatoryPosition_p=MPosition::Convert(observatoryPosition_p, MPosition::ITRF)();

	// Determine phase center
    if (phaseCenterPar_p->type() == casac::variant::INT)
    {
    	Int fieldIdForPhaseCenter = phaseCenterPar_p->toInt();

    	MSField fieldTable = selectedInputMs_p->field();
    	if (fieldIdForPhaseCenter >= (Int)fieldTable.nrow() || fieldIdForPhaseCenter < 0)
    	{
    		logger_p << LogIO::SEVERE << LogOrigin("MSTransformManager", __FUNCTION__)
    				<< "Selected FIELD_ID to determine phase center does not exists " << LogIO::POST;
    	}
    	else
    	{
    		MSFieldColumns fieldCols(fieldTable);
    		phaseCenter_p = fieldCols.phaseDirMeasCol()(fieldIdForPhaseCenter)(IPosition(1,0));
    	}

    	userPhaseCenter_p = True;
    }
    else
    {
    	String phaseCenter = phaseCenterPar_p->toString(True);

    	// Determine phase center from the first row in the FIELD sub-table of the output (selected) MS
    	if (phaseCenter.empty())
    	{
    		MSField fieldTable = outputMs_p->field();
    		MSFieldColumns fieldCols(fieldTable);
    		phaseCenter_p = fieldCols.phaseDirMeasCol()(0)(IPosition(1,0));

    		userPhaseCenter_p = False;
    	}
    	// Parse phase center
    	else
    	{
        	if(!casaMDirection(phaseCenter, phaseCenter_p))
        	{
        		logger_p << LogIO::SEVERE << LogOrigin("MSTransformManager", __FUNCTION__)
        				<< "Cannot interpret phase center " << phaseCenter << LogIO::POST;
        		return;
        	}

        	userPhaseCenter_p = True;
    	}
    }

    // Determine observation time from the first row in the selected MS
    referenceTime_p = selectedInputMsCols_p->timeMeas()(0);

	return;
}

// -----------------------------------------------------------------------
// Method to re-grid each SPW separately in the SPW sub-table
// It also sets the input/output frequency arrays to be used by the interpolations
// -----------------------------------------------------------------------
void MSTransformManager::regridSpwSubTable()
{
	// Access Spectral Window sub-table
	MSSpectralWindow spwTable = outputMs_p->spectralWindow();
    uInt nInputSpws = spwTable.nrow();
    MSSpWindowColumns spwCols(spwTable);

    // Access columns which have to be modified
    ArrayColumn<Double> chanFreqCol = spwCols.chanFreq();
    ArrayColumn<Double> chanWidthCol = spwCols.chanWidth();
    ArrayColumn<Double> effectiveBWCol = spwCols.effectiveBW();
    ArrayColumn<Double> resolutionCol = spwCols.resolution();
    ScalarColumn<Int> numChanCol = spwCols.numChan();
    ScalarColumn<Double> refFrequencyCol = spwCols.refFrequency();
    ScalarColumn<Double> totalBandwidthCol = spwCols.totalBandwidth();
    ScalarColumn<Int> measFreqRefCol = spwCols.measFreqRef();

    Int spwId;
    for(uInt spw_idx=0; spw_idx<nInputSpws; spw_idx++)
    {
    	if (outputInputSPWIndexMap_p.size()>0)
    	{
    		spwId = outputInputSPWIndexMap_p[spw_idx];
    	}
    	else
    	{
    		spwId = spw_idx;
    	}

    	logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
    			<< "Regridding SPW with Id " <<  spwId << LogIO::POST;

    	// Get input frequencies and widths
    	Vector<Double> originalChanFreq(chanFreqCol(spw_idx));
    	Vector<Double> originalChanWidth(chanWidthCol(spw_idx));

    	// Calculate output SPW
        Vector<Double> regriddedCHAN_FREQ;
        Vector<Double> regriddedCHAN_WIDTH;
        Vector<Double> inputCHAN_FREQ;
        Vector<Double> inputCHAN_WIDTH;
        regridSpwAux(spwId,originalChanFreq,originalChanWidth,inputCHAN_FREQ,inputCHAN_WIDTH,regriddedCHAN_FREQ,regriddedCHAN_WIDTH,string("Input"));
        spwInfo inputSpw(inputCHAN_FREQ,inputCHAN_WIDTH);
        spwInfo outputSpw(regriddedCHAN_FREQ,regriddedCHAN_WIDTH);

        // Set the output SPW characteristics in the SPW sub-table
        numChanCol.put(spw_idx,outputSpw.NUM_CHAN);
        chanFreqCol.put(spw_idx, outputSpw.CHAN_FREQ);
        chanWidthCol.put(spw_idx,  outputSpw.CHAN_WIDTH);
        effectiveBWCol.put(spw_idx, outputSpw.EFFECTIVE_BW);
        resolutionCol.put(spw_idx, outputSpw.RESOLUTION);
        refFrequencyCol.put(spw_idx,outputSpw.REF_FREQUENCY);
        totalBandwidthCol.put(spw_idx,outputSpw.TOTAL_BANDWIDTH);
        measFreqRefCol.put(spw_idx,outputReferenceFrame_p);

        // Add input-output SPW pair to map
    	inputOutputSpwMap_p[spwId] = std::make_pair(inputSpw,outputSpw);

    	// Prepare frequency transformation engine for the reference time
    	if (fftShiftEnabled_p)
    	{
    		MFrequency::Ref inputFrameRef(inputReferenceFrame_p,
    				MeasFrame(phaseCenter_p, observatoryPosition_p, referenceTime_p));
    		MFrequency::Ref outputFrameRef(outputReferenceFrame_p,
    				MeasFrame(phaseCenter_p, observatoryPosition_p, referenceTime_p));
    		refTimeFreqTransEngine_p = MFrequency::Convert(MSTransformations::Hz, inputFrameRef, outputFrameRef);

    	    for(uInt chan_idx=0; chan_idx<inputOutputSpwMap_p[spwId].first.CHAN_FREQ.size(); chan_idx++)
    	    {
    	    	inputOutputSpwMap_p[spwId].first.CHAN_FREQ_aux[chan_idx] =
    	    			refTimeFreqTransEngine_p(inputOutputSpwMap_p[spwId].first.CHAN_FREQ[chan_idx]).
    	    			get(MSTransformations::Hz).getValue();
    	    }
    	}
    }

    // Flush changes
    outputMs_p->flush(True);

	return;
}

// -----------------------------------------------------------------------
// Method to combine and re-grid the SPW sub-table
// It also sets the input/output frequency arrays to be used by the interpolations
// -----------------------------------------------------------------------
void MSTransformManager::regridAndCombineSpwSubtable()
{
	/// Determine input SPW structure ////////////////////

	// Access Spectral Window sub-table
	MSSpectralWindow spwTable = outputMs_p->spectralWindow();
    uInt nInputSpws = spwTable.nrow();
    MSSpWindowColumns spwCols(spwTable);

    // Access columns which have to be modified
    ArrayColumn<Double> chanFreqCol = spwCols.chanFreq();
    ArrayColumn<Double> chanWidthCol = spwCols.chanWidth();
    ArrayColumn<Double> effectiveBWCol = spwCols.effectiveBW();
    ArrayColumn<Double> resolutionCol = spwCols.resolution();
    ScalarColumn<Int> numChanCol = spwCols.numChan();
    ScalarColumn<Double> refFrequencyCol = spwCols.refFrequency();
    ScalarColumn<Double> totalBandwidthCol = spwCols.totalBandwidth();
    ScalarColumn<Int> measFreqRefCol = spwCols.measFreqRef();

    // Create list of input channels
    vector<channelInfo> inputChannels;
    for(uInt spw_idx=0; spw_idx<nInputSpws; spw_idx++)
    {
    	Vector<Double> originalChanFreq(chanFreqCol(spw_idx));
    	Vector<Double> originalChanWidth(chanWidthCol(spw_idx));
    	Vector<Double> inputEffectiveBW(effectiveBWCol(spw_idx));
    	Vector<Double> inputResolution(resolutionCol(spw_idx));
    	uInt nChannels = originalChanFreq.size();

    	for (uInt chan_idx=0;chan_idx<nChannels;chan_idx++)
    	{
    		channelInfo channelInfo_idx;
    		if (outputInputSPWIndexMap_p.size())
    		{
    			channelInfo_idx.SPW_id = outputInputSPWIndexMap_p[spw_idx];
    		}
    		else
    		{
    			channelInfo_idx.SPW_id = spw_idx;
    		}

    		channelInfo_idx.inpChannel = chan_idx;
    		channelInfo_idx.CHAN_FREQ = originalChanFreq(chan_idx);
    		channelInfo_idx.CHAN_WIDTH = originalChanWidth(chan_idx);
    		channelInfo_idx.EFFECTIVE_BW = inputEffectiveBW(chan_idx);
    		channelInfo_idx.RESOLUTION = inputResolution(chan_idx);

    		inputChannels.push_back(channelInfo_idx);
    	}
    }

    // Sort input channels
    sort (inputChannels.begin(), inputChannels.end());

    /// Determine combined SPW structure ///////////////////

	// Determine combined SPWs
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
			<< "Calculate combined SPW frequencies" << LogIO::POST;

    Vector<Double> combinedCHAN_FREQ;
    Vector<Double> combinedCHAN_WIDTH;
    MSTransformRegridder::combineSpws(logger_p,outMsName_p,Vector<Int>(1,-1),combinedCHAN_FREQ,combinedCHAN_WIDTH,True);

	// Create list of combined channels
	vector<channelInfo> combinedChannels;
	uInt nCombinedChannels = combinedCHAN_FREQ.size();
	for (uInt chan_idx=0;chan_idx<nCombinedChannels;chan_idx++)
	{
		channelInfo channelInfo_idx;
		channelInfo_idx.SPW_id = 0;
		channelInfo_idx.inpChannel = chan_idx;
		channelInfo_idx.CHAN_FREQ = combinedCHAN_FREQ(chan_idx);
		channelInfo_idx.CHAN_WIDTH = combinedCHAN_WIDTH(chan_idx);
		channelInfo_idx.EFFECTIVE_BW = combinedCHAN_WIDTH(chan_idx);
		channelInfo_idx.RESOLUTION = combinedCHAN_WIDTH(chan_idx);
		combinedChannels.push_back(channelInfo_idx);
	}

	// Find list of input overlapping channels per combined channel
	Double overlap = 0;
	inputOutputChanFactorMap_p.clear();
	vector<channelInfo>::iterator inputChanIter;
	vector<channelInfo>::iterator combChanIter;
	for (combChanIter = combinedChannels.begin(); combChanIter != combinedChannels.end(); combChanIter++)
	{
		for (inputChanIter = inputChannels.begin(); inputChanIter != inputChannels.end(); inputChanIter++)
		{
			overlap = combChanIter->overlap(*inputChanIter);
			if (overlap)
			{
				inputChanIter->outChannel = combChanIter->inpChannel;
				inputOutputChanFactorMap_p[inputChanIter->outChannel].
				push_back(channelContribution(inputChanIter->SPW_id,inputChanIter->inpChannel,inputChanIter->outChannel,overlap));
			}
		}
	}

	/// Calculate output SPW ///////////////////////////////
    Vector<Double> regriddedCHAN_FREQ;
    Vector<Double> regriddedCHAN_WIDTH;
    Vector<Double> inputCHAN_FREQ;
    Vector<Double> inputCHAN_WIDTH;
    regridSpwAux(0,combinedCHAN_FREQ,combinedCHAN_WIDTH,inputCHAN_FREQ,inputCHAN_WIDTH,regriddedCHAN_FREQ,regriddedCHAN_WIDTH,string("Combined"));
    spwInfo inputSpw(inputCHAN_FREQ,inputCHAN_WIDTH);
    spwInfo outputSpw(regriddedCHAN_FREQ,regriddedCHAN_WIDTH);

    /// Modify SPW subtable ////////////////////////////////

    logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
    		<< "Write output SPW subtable " << LogIO::POST;

    // Delete combined SPWs (reverse to preserve row number)
    uInt rowsToDelete = nInputSpws-1;
    for(Int spw_idx=rowsToDelete; spw_idx>0; spw_idx--)
    {
    	spwTable.removeRow(spw_idx);
    }

    // Set the output SPW characteristics in the SPW sub-table
    numChanCol.put(0,outputSpw.NUM_CHAN);
    chanFreqCol.put(0, outputSpw.CHAN_FREQ);
    chanWidthCol.put(0,  outputSpw.CHAN_WIDTH);
    effectiveBWCol.put(0, outputSpw.EFFECTIVE_BW);
    resolutionCol.put(0, outputSpw.RESOLUTION);
    refFrequencyCol.put(0,outputSpw.REF_FREQUENCY);
    totalBandwidthCol.put(0,outputSpw.TOTAL_BANDWIDTH);
    measFreqRefCol.put(0,outputReferenceFrame_p);

    // Flush changes
    outputMs_p->flush(True);


    /// Add input-output SPW pair to map ///////////////////
	inputOutputSpwMap_p[0] = std::make_pair(inputSpw,outputSpw);

	// Prepare frequency transformation engine for the reference time
	if (fftShiftEnabled_p)
	{
		MFrequency::Ref inputFrameRef(inputReferenceFrame_p,
				MeasFrame(phaseCenter_p, observatoryPosition_p, referenceTime_p));
		MFrequency::Ref outputFrameRef(outputReferenceFrame_p,
				MeasFrame(phaseCenter_p, observatoryPosition_p, referenceTime_p));
		refTimeFreqTransEngine_p = MFrequency::Convert(MSTransformations::Hz, inputFrameRef, outputFrameRef);

	    for(uInt chan_idx=0; chan_idx<inputOutputSpwMap_p[0].first.CHAN_FREQ.size(); chan_idx++)
	    {
	    	inputOutputSpwMap_p[0].first.CHAN_FREQ_aux[chan_idx] =
	    			refTimeFreqTransEngine_p(inputOutputSpwMap_p[0].first.CHAN_FREQ[chan_idx]).
	    			get(MSTransformations::Hz).getValue();
	    }
	}

	return;
}


// -----------------------------------------------------------------------
// Auxiliary method common whenever re-gridding is necessary (with or without combining the SPWs)
// -----------------------------------------------------------------------
void MSTransformManager::regridSpwAux(	Int spwId,
										Vector<Double> &originalCHAN_FREQ,
										Vector<Double> &originalCHAN_WIDTH,
										Vector<Double> &inputCHAN_FREQ,
										Vector<Double> &inputCHAN_WIDTH,
										Vector<Double> &regriddedCHAN_FREQ,
										Vector<Double> &regriddedCHAN_WIDTH,
										string msg)
{

    // Print characteristics of input SPW
	ostringstream oss;
	oss << msg;
    oss 	<< " SPW: " << std::setw(5) << originalCHAN_FREQ.size()
    		<< " channels, first channel = "
    		<< std::setprecision(9) << std::setw(14) << std::scientific
    		<< originalCHAN_FREQ(0) << " Hz"
    		<< ", last channel = "
    		<< std::setprecision(9) << std::setw(14) << std::scientific
    		<< originalCHAN_FREQ(originalCHAN_FREQ.size() -1) << " Hz";
    logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) << oss.str() << LogIO::POST;


	// Apply channel average if necessary
	if (freqbinMap_p.find(spwId) != freqbinMap_p.end())
	{
		calculateIntermediateFrequencies(spwId,originalCHAN_FREQ,originalCHAN_WIDTH,inputCHAN_FREQ,inputCHAN_WIDTH);

		oss.str("");
		oss.clear();
		oss 	<< "Averaged SPW: " << std::setw(5) << inputCHAN_WIDTH.size()
            				<< " channels, first channel = "
            				<< std::setprecision(9) << std::setw(14) << std::scientific
            				<< inputCHAN_FREQ(0) << " Hz"
            				<< ", last channel = "
            				<< std::setprecision(9) << std::setw(14) << std::scientific
            				<< inputCHAN_FREQ(inputCHAN_WIDTH.size() -1) << " Hz";
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
            				<< oss.str() << LogIO::POST;
	}
	else
	{
		numOfCombInputChanMap_p[spwId] = originalCHAN_FREQ.size();
		inputCHAN_FREQ = originalCHAN_FREQ;
		inputCHAN_WIDTH = originalCHAN_WIDTH;
	}

	// Re-grid the output SPW to be uniform and change reference frame
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
    					<< "Calculate frequencies in output reference frame " << LogIO::POST;

	MSTransformRegridder::calcChanFreqs(	logger_p,
											regriddedCHAN_FREQ,
											regriddedCHAN_WIDTH,
											inputCHAN_FREQ,
											inputCHAN_WIDTH,
											phaseCenter_p,
											inputReferenceFrame_p,
											referenceTime_p,
											observatoryPosition_p,
											mode_p,
											nChan_p,
											start_p,
											width_p,
											restFrequency_p,
											outputReferenceFramePar_p,
											velocityType_p,
											True // verbose
											);

	// Check if pre-averaging step is necessary
	if (freqbinMap_p.find(spwId) == freqbinMap_p.end())
	{

		Vector<Double> tmpCHAN_FREQ;
		Vector<Double> tmpCHAN_WIDTH;
		MSTransformRegridder::calcChanFreqs(	logger_p,
												tmpCHAN_FREQ,
												tmpCHAN_WIDTH,
												originalCHAN_FREQ,
												originalCHAN_WIDTH,
												phaseCenter_p,
												inputReferenceFrame_p,
												referenceTime_p,
												observatoryPosition_p,
												String("channel"),
												-1,
												String("0"),
												String("1"),
												restFrequency_p,
												outputReferenceFramePar_p,
												velocityType_p,
												False // verbose
												);

		Double avgCombinedWidth = 0;
		for (uInt chanIdx=0;chanIdx<tmpCHAN_WIDTH.size();chanIdx++)
		{
			avgCombinedWidth += tmpCHAN_WIDTH(chanIdx);
		}
		avgCombinedWidth /= tmpCHAN_WIDTH.size();

		Double avgRegriddedWidth = 0;
		for (uInt chanIdx=0;chanIdx<regriddedCHAN_WIDTH.size();chanIdx++)
		{
			avgRegriddedWidth += regriddedCHAN_WIDTH(chanIdx);
		}
		avgRegriddedWidth /= regriddedCHAN_WIDTH.size();

		Int width =  (Int)floor(abs(avgRegriddedWidth/avgCombinedWidth) + 0.001);

		if ((width >= 2) and  2*width <= originalCHAN_WIDTH.size())
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
	        					<< "Ratio between input and output width is " << avgRegriddedWidth/avgCombinedWidth
	        					<< ", setting pre-channel average width to " << width << LogIO::POST;
			channelAverage_p = True;
			freqbinMap_p[spwId] = width;

			// Calculate averaged frequencies
			calculateIntermediateFrequencies(spwId,originalCHAN_FREQ,originalCHAN_WIDTH,inputCHAN_FREQ,inputCHAN_WIDTH);

			oss.str("");
			oss.clear();
			oss 	<< "Averaged SPW: " << std::setw(5) << inputCHAN_WIDTH.size()
	            				<< " channels, first channel = "
	            				<< std::setprecision(9) << std::setw(14) << std::scientific
	            				<< inputCHAN_FREQ(0) << " Hz"
	            				<< ", last channel = "
	            				<< std::setprecision(9) << std::setw(14) << std::scientific
	            				<< inputCHAN_FREQ(inputCHAN_WIDTH.size() -1) << " Hz";
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
	            				<< oss.str() << LogIO::POST;
		}
	}

	// Print characteristics of output SPW
	oss.str("");
	oss.clear();
	oss 	<< "Output SPW: " << std::setw(5) << regriddedCHAN_FREQ.size()
			<< " channels, first channel = "
			<< std::setprecision(9) << std::setw(14) << std::scientific
			<< regriddedCHAN_FREQ(0) << " Hz"
			<< ", last channel = "
			<< std::setprecision(9) << std::setw(14) << std::scientific
			<< regriddedCHAN_FREQ(regriddedCHAN_FREQ.size()-1) << " Hz";
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
	    				<< oss.str() << LogIO::POST;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::separateSpwSubtable()
{
	// Access Spectral Window sub-table
	MSSpectralWindow spwTable = outputMs_p->spectralWindow();
    MSSpWindowColumns spwCols(spwTable);

    // Access columns which have to be separated
    ArrayColumn<Double> chanFreqCol = spwCols.chanFreq();
    Vector<Double> chanFreq = chanFreqCol(0);
    ArrayColumn<Double> chanWidthCol = spwCols.chanWidth();
    Vector<Double> chanWidth = chanWidthCol(0);
    ArrayColumn<Double> effectiveBWCol = spwCols.chanWidth();
    Vector<Double> effectiveBW = effectiveBWCol(0);
    ArrayColumn<Double> resolutionCol = spwCols.resolution();
    Vector<Double> resolution = resolutionCol(0);

    // Resize columns to be separated
    if (tailOfChansforLastSpw_p)
    {
    	uInt nInChannels = chanFreq.size();
    	uInt nOutChannels = nspws_p*chansPerOutputSpw_p;
    	uInt newChannels = nOutChannels-nInChannels;
    	Double lastFreq = chanFreq(chanFreq.size()-1);
    	Double lastWidth = chanWidth(chanFreq.size()-1);
    	Double lastEffectiveBW = effectiveBW(chanFreq.size()-1);
    	Double lastResolution = resolution(chanFreq.size()-1);

    	chanFreq.resize(nOutChannels,True);
    	chanWidth.resize(nOutChannels,True);
    	effectiveBW.resize(nOutChannels,True);
    	resolution.resize(nOutChannels,True);

    	uInt outIndex;
    	for (uInt newChanIdx = 0; newChanIdx<newChannels; newChanIdx++)
    	{
    		outIndex = nInChannels+newChanIdx;
    		chanFreq(outIndex) = lastFreq + (newChanIdx+1)*lastWidth;
    		chanWidth(outIndex) = lastWidth;
    		effectiveBW(outIndex) = lastEffectiveBW;
    		resolution(outIndex) = lastResolution;
    	}
    }

    // Calculate bandwidth per output spw
    Double totalBandwidth = chanWidth(0)*chansPerOutputSpw_p;

    uInt rowIndex = 1;
    for (uInt spw_i=0; spw_i<nspws_p; spw_i++)
    {
    	// Add row
    	spwTable.addRow(1,True);

    	// Prepare slice
    	Slice slice(chansPerOutputSpw_p*spw_i,chansPerOutputSpw_p);

    	// Columns that can be just copied
    	spwCols.measFreqRef().put(rowIndex,spwCols.measFreqRef()(0));
    	spwCols.flagRow().put(rowIndex,spwCols.flagRow()(0));
    	spwCols.freqGroup().put(rowIndex,spwCols.freqGroup()(0));
    	spwCols.freqGroupName().put(rowIndex,spwCols.freqGroupName()(0));
		spwCols.ifConvChain().put(rowIndex,spwCols.ifConvChain()(0));
		spwCols.name().put(rowIndex,spwCols.name()(0));
		spwCols.netSideband().put(rowIndex,spwCols.netSideband()(0));

		// Array columns that have to be modified
		Slice range(chansPerOutputSpw_p*spw_i,chansPerOutputSpw_p);
    	spwCols.chanFreq().put(rowIndex,chanFreq(range));
    	spwCols.chanWidth().put(rowIndex,chanWidth(range));
    	spwCols.effectiveBW().put(rowIndex,effectiveBW(range));
    	spwCols.resolution().put(rowIndex,resolution(range));

    	// Scalar columns that have to be modified
		spwCols.numChan().put(rowIndex,chansPerOutputSpw_p);
		spwCols.totalBandwidth().put(rowIndex,totalBandwidth);
    	spwCols.refFrequency().put(rowIndex,chanFreq(range)(0));

    	rowIndex += 1;
    }

    // Remove first row
    spwTable.removeRow(0);

    // Flush changes
    outputMs_p->flush(True);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::calculateIntermediateFrequencies(	Int spwId,
																Vector<Double> &inputChanFreq,
																Vector<Double> &inputChanWidth,
																Vector<Double> &intermediateChanFreq,
																Vector<Double> &intermediateChanWidth)
{
	uInt mumOfInterChan = inputChanFreq.size() / freqbinMap_p[spwId];
	uInt lastChannelWidth = inputChanFreq.size() % freqbinMap_p[spwId];
	if (lastChannelWidth > 0)
	{
		mumOfInterChan += 1;
	}
	numOfCombInputChanMap_p[spwId] = inputChanFreq.size();
	numOfCombInterChanMap_p[spwId] = mumOfInterChan;
	intermediateChanFreq.resize(mumOfInterChan,False);
	intermediateChanWidth.resize(mumOfInterChan,False);
	simpleAverage(freqbinMap_p[spwId], inputChanFreq, intermediateChanFreq);
	simpleAverage(freqbinMap_p[spwId], inputChanWidth, intermediateChanWidth);

	for (uInt chanIdx=0;chanIdx<mumOfInterChan;chanIdx++)
	{
		intermediateChanWidth[chanIdx] *= freqbinMap_p[spwId];
	}

	if (lastChannelWidth > 0)
	{
		intermediateChanWidth[mumOfInterChan-1] /= freqbinMap_p[spwId];
		intermediateChanWidth[mumOfInterChan-1] *= lastChannelWidth;
	}

    return;
}

// -----------------------------------------------------------------------
// Method to set all the elements of a scalar column to a given value
// -----------------------------------------------------------------------
void MSTransformManager::reindexColumn(ScalarColumn<Int> &inputCol, Int value)
{
	for(uInt idx=0; idx<inputCol.nrow(); idx++)
	{
		inputCol.put(idx,value);
	}

	return;
}

// -----------------------------------------------------------------------
// Method to re-index Spectral Window column in Source sub-table
// -----------------------------------------------------------------------
void MSTransformManager::reindexSourceSubTable()
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
    	logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
    			<< "No SOURCE sub-table found " << LogIO::POST;
    }

    return;
}

// -----------------------------------------------------------------------
// Method to re-index Spectral Window column in DDI sub-table
// -----------------------------------------------------------------------
void MSTransformManager::reindexDDISubTable()
{
    if(Table::isReadable(outputMs_p->dataDescriptionTableName()) and !outputMs_p->dataDescription().isNull())
    {
    	// Access DDI sub-table
    	MSDataDescription ddiTable = outputMs_p->dataDescription();
    	MSDataDescColumns ddiCols(ddiTable);

    	// Add a new row for each of the separated SPWs
    	uInt rowIndex = ddiCols.nrow();
    	for (uInt spw_i=0; spw_i<nspws_p; spw_i++)
    	{
    		// Add row
    		ddiTable.addRow(1,True);

    	    // Copy polID and flagRow from the first original SPW
    		ddiCols.polarizationId().put(rowIndex,ddiCols.polarizationId()(0));
    		ddiCols.flagRow().put(rowIndex,ddiCols.flagRow()(0));

    		// Set SPW id separately
    		ddiCols.spectralWindowId().put(rowIndex,ddiStart_p+spw_i);

    		rowIndex += 1;
    	}

        // Delete the old rows
    	uInt rowsToDelete = ddiCols.nrow()-nspws_p;
    	for(uInt rowsDeleted=0; rowsDeleted<rowsToDelete; rowsDeleted++)
    	{
    		ddiTable.removeRow(0);
    	}

        // Flush changes
        outputMs_p->flush(True);

    }
    else
    {
    	logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
    			<< "No DATA_DESCRIPTION sub-table found " << LogIO::POST;
    }
}

// -----------------------------------------------------------------------
// Method to re-index Spectral Window column in Feed sub-table
// -----------------------------------------------------------------------
void MSTransformManager::reindexFeedSubTable()
{
    if(Table::isReadable(outputMs_p->feedTableName()) and !outputMs_p->feed().isNull())
    {
    	MSFeed msSubtable = outputMs_p->feed();
    	MSFeedColumns tableCols(msSubtable);
    	ScalarColumn<Int> spwCol = tableCols.spectralWindowId();
    	Vector<Int> newSpw(spwCol.nrow(),0);
    	spwCol.putColumn(newSpw);

        // Flush changes
        outputMs_p->flush(True);
    }
    else
    {
    	logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
    			 "No FEED sub-table found " << LogIO::POST;
    }

	return;
}

// -----------------------------------------------------------------------
// Method to re-index Spectral Window column in SysCal sub-table
// -----------------------------------------------------------------------
void MSTransformManager::reindexSysCalSubTable()
{
    if(Table::isReadable(outputMs_p->sysCalTableName()) and !outputMs_p->sysCal().isNull())
    {
    	MSSysCal msSubtable = outputMs_p->sysCal();
    	MSSysCalColumns tableCols(msSubtable);
    	ScalarColumn<Int> spwCol = tableCols.spectralWindowId();
    	Vector<Int> newSpw(spwCol.nrow(),0);
    	spwCol.putColumn(newSpw);

        // Flush changes
        outputMs_p->flush(True);
    }
    else
    {
    	logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
    			<< "No SYSCAL sub-table found " << LogIO::POST;
    }

	return;
}

// -----------------------------------------------------------------------
// Method to re-index Spectral Window column in FreqOffset sub-table
// -----------------------------------------------------------------------
void MSTransformManager::reindexFreqOffsetSubTable()
{
    if(Table::isReadable(outputMs_p->freqOffsetTableName()) and !outputMs_p->freqOffset().isNull())
    {
    	MSFreqOffset msSubtable = outputMs_p->freqOffset();
    	MSFreqOffsetColumns tableCols(msSubtable);
    	ScalarColumn<Int> spwCol = tableCols.spectralWindowId();
    	Vector<Int> newSpw(spwCol.nrow(),0);
    	spwCol.putColumn(newSpw);

        // Flush changes
        outputMs_p->flush(True);
    }
    else
    {
    	logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
    			<< "No FREQ_OFF sub-table found " << LogIO::POST;
    }

    return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::getInputNumberOfChannels()
{
	// Access spectral window sub-table
	MSSpectralWindow spwTable = inputMs_p->spectralWindow();
    uInt nInputSpws = spwTable.nrow();
    MSSpWindowColumns spwCols(spwTable);
    ScalarColumn<Int> numChanCol = spwCols.numChan();

    // Get number of output channels per input spw
    for(uInt spw_idx=0; spw_idx<nInputSpws; spw_idx++)
    {
    	numOfInpChanMap_p[spw_idx] = numChanCol(spw_idx);
    }

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::dropNonUniformWidthChannels()
{
	// Access spectral window sub-table
	MSSpectralWindow spwTable = outputMs_p->spectralWindow();
	uInt nInputSpws = spwTable.nrow();
	MSSpWindowColumns spwCols(spwTable);
    ArrayColumn<Double> chanFreqCol = spwCols.chanFreq();
    ArrayColumn<Double> chanWidthCol = spwCols.chanWidth();
    ArrayColumn<Double> effectiveBWCol = spwCols.effectiveBW();
    ArrayColumn<Double> resolutionCol = spwCols.resolution();
    ScalarColumn<Int> numChanCol = spwCols.numChan();
    ScalarColumn<Double> totalBandwidthCol = spwCols.totalBandwidth();

	uInt nChans;
	Int spwId;
	for(uInt spw_idx=0; spw_idx<nInputSpws; spw_idx++)
	{
		nChans = numChanCol(spw_idx);
		Vector<Double> widthVector = chanWidthCol(spw_idx);

    	if (outputInputSPWIndexMap_p.size()>0)
    	{
    		spwId = outputInputSPWIndexMap_p[spw_idx];
    	}
    	else
    	{
    		spwId = spw_idx;
    	}

		uInt nchanInAvg = floor((widthVector(nChans-1) / (widthVector(0) / freqbinMap_p[spwId])) + 0.5);

		if (nchanInAvg < freqbinMap_p[spwId])
		{
			logger_p 	<< LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
						<< "Not enough channels to populate last averaged channel from SPW " << spwId
						<< " with an uniform width of " << widthVector(0) << " Hz" << endl
						<< "The resulting channel would have width of only " << widthVector(nChans-1) << " Hz." << endl
						<< "The channel will be dropped in order to have an uniform grid."
						<< LogIO::POST;

			// Number of channels is reduced in 1
			numChanCol.put(spw_idx, nChans-1);

			// Total BW has to be reduced to account for the dropped channel
			totalBandwidthCol.put(spw_idx, totalBandwidthCol(spw_idx)-widthVector(nChans-1));

			// We have to drop the last element from width
			widthVector.resize(nChans-1,True);
			chanWidthCol.put(spw_idx, widthVector);

			// We have to drop the last element from effective BW
			// NOTE: Effective BW is not always equal to width
			Vector<Double> effectiveBWVector = effectiveBWCol(spw_idx);
			effectiveBWVector.resize(nChans-1,True);
			effectiveBWCol.put(spw_idx, effectiveBWVector);

			// We have to drop the last element from resolution
			// NOTE: Resolution is not always equal to width
			Vector<Double> resolutionVector = resolutionCol(spw_idx);
			resolutionVector.resize(nChans-1,True);
			resolutionCol.put(spw_idx, resolutionVector);

			// We have to drop the last element from channel Frequency
			Vector<Double> frequencyVector = chanFreqCol(spw_idx);
			frequencyVector.resize(nChans-1,True);
			chanFreqCol.put(spw_idx, frequencyVector);

			// Update output number of channels
			inputOutputSpwMap_p[spw_idx].second.resize(nChans-1);
		}
	}

	// Flush changes
	outputMs_p->flush(True);

	return;
}


// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::getOutputNumberOfChannels()
{
	if (refFrameTransformation_p or combinespws_p)
	{
		map<Int,Int>::iterator iter;
		for(iter = numOfSelChanMap_p.begin(); iter != numOfSelChanMap_p.end(); iter++)
		{
			if (freqbinMap_p.find(iter->first) == freqbinMap_p.end())
			{
				// When doing only re-gridding, maybe not all SPWs require pre-averaging
				if (not combinespws_p)
				{
					freqbinMap_p[iter->first] = 1;
				}
				// When combining SPWs all of them get the same freqbin
				else
				{
					freqbinMap_p[iter->first] = freqbinMap_p[0];
				}
			}

			// Note: This will truncate the result, but it is ok because we are dropping the last channel
			numOfOutChanMap_p[iter->first] = numOfSelChanMap_p[iter->first] / freqbinMap_p[iter->first];
		}
	}
	else
	{
		// Access spectral window sub-table
		MSSpectralWindow spwTable = outputMs_p->spectralWindow();
	    uInt nInputSpws = spwTable.nrow();
	    MSSpWindowColumns spwCols(spwTable);
	    ScalarColumn<Int> numChanCol = spwCols.numChan();
	    ArrayColumn<Double> chanFreqCol = spwCols.chanFreq();

	    // Get number of output channels per input spw
	    Int spwId;
	    for(uInt spw_idx=0; spw_idx<nInputSpws; spw_idx++)
	    {
	    	if (outputInputSPWIndexMap_p.size()>0)
	    	{
	    		spwId = outputInputSPWIndexMap_p[spw_idx];
	    	}
	    	else
	    	{
	    		spwId = spw_idx;
	    	}

	    	numOfOutChanMap_p[spwId] = numChanCol(spw_idx);
	    }
	}


	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::calculateWeightAndSigmaFactors()
{
	map<Int,Int>::iterator iter;
	for(iter = numOfSelChanMap_p.begin(); iter != numOfSelChanMap_p.end(); iter++)
	{
		weightFactorMap_p[iter->first] = (Float)numOfSelChanMap_p[iter->first] /
										(Float)numOfInpChanMap_p[iter->first];
		sigmaFactorMap_p[iter->first] = 1./sqrt((Float)numOfSelChanMap_p[iter->first] /
										(Float)numOfOutChanMap_p[iter->first]);
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check if flag category has to be filled
// -----------------------------------------------------------------------
void MSTransformManager::checkFillFlagCategory()
{
	fillFlagCategory_p = False;
	if (!selectedInputMsCols_p->flagCategory().isNull() && selectedInputMsCols_p->flagCategory().isDefined(0))
	{
		fillFlagCategory_p = True;
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Optional column FLAG_CATEGORY found in input MS will be written to output MS" << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check if weight spectrum column has to be filled
// -----------------------------------------------------------------------
void MSTransformManager::checkFillWeightSpectrum()
{
	inputWeightSpectrumAvailable_p = False;
	if (!selectedInputMsCols_p->weightSpectrum().isNull() && selectedInputMsCols_p->weightSpectrum().isDefined(0))
	{
		inputWeightSpectrumAvailable_p = True;
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Optional column WEIGHT_SPECTRUM found in input MS will be written to output MS" << LogIO::POST;
	}

	// jagonzal: I don't want to spend so much resources transforming and writing synthetic WEIGHT_SPECTRUM
	/*
	else
	{
    	logger_p 	<< LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
    				<< "WEIGHT_SPECTRUM column not present." << endl
    				<< " Will fill output WEIGHT_SPECTRUM column using input WEIGHT column" << LogIO::POST;
	}
	*/

	return;
}

// -----------------------------------------------------------------------
// Method to check which data columns have to be filled
// -----------------------------------------------------------------------
void MSTransformManager::checkDataColumnsToFill()
{
	dataColMap_p.clear();
	Bool mainColSet=False;
	Bool modelDataChecked = False;
	if (datacolumn_p.contains("ALL"))
	{
		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::DATA] = MS::DATA;

			if (not bufferMode_p)
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
									"Adding DATA column to output MS "<< LogIO::POST;
			}
			else
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
									"DATA column present in input MS will be available from MSTransformBuffer "<< LogIO::POST;
			}

			timeAvgOptions_p |= vi::AveragingOptions::AverageObserved;
			// timeAvgOptions_p |= vi::AveragingOptions::ObservedUseNoWeights;
		}

		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::CORRECTED_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::CORRECTED_DATA] = MS::CORRECTED_DATA;

			if (not bufferMode_p)
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
									"Adding CORRECTED_DATA column to output MS "<< LogIO::POST;
			}
			else
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
									"CORRECTED_DATA column present in input MS will be available from MSTransformBuffer "<< LogIO::POST;
			}

			timeAvgOptions_p |= vi::AveragingOptions::AverageCorrected;
			timeAvgOptions_p |= vi::AveragingOptions::CorrectedUseWeights;
		}

		if ((inputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA))) or realmodelcol_p)
		{
			modelDataChecked = True;
			if (!mainColSet)
			{
				mainColumn_p = MS::MODEL_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::MODEL_DATA] = MS::MODEL_DATA;

			if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
			{
				if (not bufferMode_p)
				{
					logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
										"Adding MODEL_DATA column to output MS "<< LogIO::POST;
				}
				else
				{
					logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
										"MODEL_DATA column present in input MS will be available from MSTransformBuffer "<< LogIO::POST;
				}
			}
			else
			{
				if (not bufferMode_p)
				{
					logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
										"Adding MODEL_DATA column to output MS from input virtual MODEL_DATA column "<< LogIO::POST;
				}
				else
				{
					logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
										"MODEL_DATA column present in input MS will be available from MSTransformBuffer "<< LogIO::POST;
				}
			}

			timeAvgOptions_p |= vi::AveragingOptions::AverageModel;
			timeAvgOptions_p |= vi::AveragingOptions::ModelUseNoWeights;
		}

		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::FLOAT_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;

			// TODO: FLOAT_DATA is not yet supported by TVI
			// timeAvgOptions_p |= vi::AveragingOptions::AverageFloatData;
		}

		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::LAG_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;

			// TODO: LAG_DATA is not yet supported by TVI
			// timeAvgOptions_p |= vi::AveragingOptions::AverageLagData;
		}
	}
	else if (datacolumn_p.contains("FLOAT_DATA,DATA"))
	{
		Bool mainColSet=False;

		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;

			timeAvgOptions_p = vi::AveragingOptions(vi::AveragingOptions::AverageObserved);
			// timeAvgOptions_p = vi::AveragingOptions(vi::AveragingOptions::ObservedUseNoWeights);
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::FLOAT_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;

			// TODO: FLOAT_DATA is not yet supported by TVI
			// timeAvgOptions_p |= vi::AveragingOptions::AverageFloatData;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"FLOAT_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("FLOAT_DATA"))
	{
		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::FLOAT_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;

			// TODO: FLOAT_DATA is not yet supported by TVI
			// timeAvgOptions_p |= vi::AveragingOptions::AverageFloatData;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"FLOAT_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("LAG_DATA,DATA"))
	{
		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;

			timeAvgOptions_p |= vi::AveragingOptions::AverageObserved;
			// timeAvgOptions_p = vi::AveragingOptions(vi::AveragingOptions::ObservedUseNoWeights);
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::LAG_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;

			// TODO: LAG_DATA is not yet supported by TVI
			// timeAvgOptions_p |= vi::AveragingOptions::AverageLagData;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"LAG_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("LAG_DATA"))
	{
		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::LAG_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;

			// TODO: LAG_DATA is not yet supported by TVI
			// timeAvgOptions_p |= vi::AveragingOptions::AverageLagData;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"LAG_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("DATA"))
	{
		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;

			timeAvgOptions_p |= vi::AveragingOptions::AverageObserved;
			// timeAvgOptions_p |= vi::AveragingOptions::ObservedUseNoWeights;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("CORRECTED"))
	{
		if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::CORRECTED_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::CORRECTED_DATA] = MS::DATA;
			correctedToData_p = True;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
								"Adding DATA column to output MS as DATA from input CORRECTED_DATA column"<< LogIO::POST;

			timeAvgOptions_p |= vi::AveragingOptions::AverageCorrected;
			timeAvgOptions_p |= vi::AveragingOptions::CorrectedUseWeights;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"CORRECTED_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("MODEL"))
	{

		if ((inputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA))) or realmodelcol_p)
		{
			modelDataChecked = True;
			if (!mainColSet)
			{
				mainColumn_p = MS::MODEL_DATA;
				mainColSet = True;
			}

			dataColMap_p[MS::MODEL_DATA] = MS::DATA;
			if (inputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
									"Adding MODEL_DATA column to output MS as DATA "<< LogIO::POST;
			}
			else
			{
				logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
							"Adding MODEL_DATA column to output MS as DATA from input virtual MODEL_DATA column"<< LogIO::POST;
			}

			timeAvgOptions_p |= vi::AveragingOptions::AverageModel;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"MODEL_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else
	{
		logger_p << LogIO::SEVERE << LogOrigin("MSTransformManager", __FUNCTION__) <<
				"Requested data column " << datacolumn_p <<
				" is not supported, possible choices are ALL,DATA,CORRECTED,MODEL,FLOAT_DATA,LAG_DATA" << LogIO::POST;
	}

	if ((realmodelcol_p) and (!modelDataChecked))
	{
		dataColMap_p[MS::MODEL_DATA] = MS::MODEL_DATA;
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__) <<
					"Adding MODEL_DATA column to output MS from input virtual MODEL_DATA column"<< LogIO::POST;
		datacolumn_p += String(",MODEL");
	}

	return;
}

// -----------------------------------------------------------------------
// Method to determine sort columns order
// -----------------------------------------------------------------------
void MSTransformManager::setIterationApproach()
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
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Combining data through scans for time average " << LogIO::POST;
	}

	if (!timespan_p.contains("state"))
	{
		sortColumns_p[sortColumnIndex] =  MS::STATE_ID;
		sortColumnIndex += 1;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Combining data through state for time average" << LogIO::POST;
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
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformManager", __FUNCTION__)
				<< "Combining data from selected spectral windows" << LogIO::POST;
	}

	sortColumns_p[sortColumnIndex] =  MS::TIME;

	return;
}

// -----------------------------------------------------------------------
// Method to generate the initial iterator
// -----------------------------------------------------------------------
void MSTransformManager::generateIterator()
{

	if (timeAverage_p)
	{
		if (maxuvwdistance_p > 0) timeAvgOptions_p |= vi::AveragingOptions::BaselineDependentAveraging;
		vi::AveragingParameters parameters (timeBin_p, 0, vi::SortColumns (sortColumns_p,false), timeAvgOptions_p, maxuvwdistance_p);
		visibilityIterator_p = new vi::VisibilityIterator2 (vi::AveragingVi2Factory (parameters, selectedInputMs_p));
		visibilityIterator_p->setWeightScaling (vi::WeightScaling::generateUnityWeightScaling());
	}
	else
	{
		visibilityIterator_p = new vi::VisibilityIterator2(*selectedInputMs_p,vi::SortColumns (sortColumns_p,false),
		                                                   false, NULL,timeBin_p);
	}

	if (channelSelector_p != NULL) visibilityIterator_p->setFrequencySelection(*channelSelector_p);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::setupBufferTransformations(vi::VisBuffer2 *vb)
{
	// Calculate number of rows to add to the output MS depending on the combination parameters
	uInt rowsToAdd = 0;

	if (combinespws_p)
	{
		// Fill baseline map using as key Ant1,Ant2,Scan and State,
		// Which are the elements that can be combined in one chunk
		baselineMap_p.clear();
		Vector<Int> antenna1 = vb->antenna1();
		Vector<Int> antenna2 = vb->antenna2();
		Vector<Int> scan = vb->scan();
		Vector<Int> state = vb->stateId();
		Int relativeTimeInMiliseconds = 0;
		for (uInt row=0;row<antenna1.size();row++)
		{
			pair<Int,Int> baseline = std::make_pair(antenna1(row),antenna2(row));
			relativeTimeInMiliseconds = (Int)floor(1E3*(vb->time()(row) - vb->time()(0)));
			baselineMap_p[std::make_pair(baseline,relativeTimeInMiliseconds)].push_back(row);
		}

		rowsToAdd = baselineMap_p.size();

		// Fill row index vector with to the first row for every element in the baseline map
		uInt rowIndex = 0;
		rowIndex_p.clear();
		for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
		{
			rowIndex_p.push_back((iter->second)[0]);
			rowIndex ++;
		}
	}
	else
	{
		rowsToAdd = vb->nRows();
	}

	// Initialize reference frame transformation parameters
	if (refFrameTransformation_p)
	{
		initFrequencyTransGrid(vb);
	}

	// Calculate total number for rows to add
	nRowsToAdd_p = rowsToAdd*nspws_p;

    return;

}

// -----------------------------------------------------------------------
// Fill output MS with data from an input VisBuffer
// -----------------------------------------------------------------------
void MSTransformManager::fillOutputMs(vi::VisBuffer2 *vb)
{
	setupBufferTransformations(vb);

	if (not bufferMode_p)
	{
		// Create RowRef object to fill new rows
		uInt currentRows = outputMs_p->nrow();
		RefRows rowRef( currentRows, currentRows + nRowsToAdd_p/nspws_p - 1);

		// Add new rows to output MS
		outputMs_p->addRow(nRowsToAdd_p,False);

		// Fill new rows
	    fillDataCols(vb,rowRef);
		fillIdCols(vb,rowRef);
	}

    return;
}

// -----------------------------------------------------------------------
// Method to initialize the input frequency grid to change reference frame
// -----------------------------------------------------------------------
void MSTransformManager::initFrequencyTransGrid(vi::VisBuffer2 *vb)
{
	// NOTE (jagonzal): According to dpetry the difference between times is negligible but he recommends to use TIME
	//                  However it does not cross-validate unless we use timeMeas from the MS columns
	ScalarMeasColumn<MEpoch> mainTimeMeasCol = selectedInputMsCols_p->timeMeas();
	MEpoch currentRowTime = mainTimeMeasCol(vb->rowIds()(0));

	MFrequency::Ref inputFrameRef = MFrequency::Ref(inputReferenceFrame_p,
													MeasFrame(vb->phaseCenter(), observatoryPosition_p, currentRowTime));

	MFrequency::Ref outputFrameRef;
	if (userPhaseCenter_p)
	{
		outputFrameRef = MFrequency::Ref(outputReferenceFrame_p,
				MeasFrame(phaseCenter_p, observatoryPosition_p, currentRowTime));
	}
	else
	{
		outputFrameRef = MFrequency::Ref(outputReferenceFrame_p,
				MeasFrame(vb->phaseCenter(), observatoryPosition_p, currentRowTime));
	}

	/*
	ostringstream oss;
	oss.precision(20);
	oss <<  "inputReferenceFrame_p=" << inputFrameRef << endl;
	oss <<  "outputReferenceFrame_p=" << outputFrameRef << endl;
	logger_p << LogIO::NORMAL << oss.str() << LogIO::POST;
	*/


	freqTransEngine_p = MFrequency::Convert(MSTransformations::Hz, inputFrameRef, outputFrameRef);

	Int spwIndex = 0;
	if (not combinespws_p)
	{
		// jagonzal : This is actually not necessary because we
		// pass the original SPWId down to the interpol1D method
		/*
		if (inputOutputSPWIndexMap_p.size())
		{
			Int originalSPWid = vb->spectralWindows()(0);
			spwIndex = inputOutputSPWIndexMap_p[originalSPWid];
		}
		*/
		spwIndex = vb->spectralWindows()(0);
	}


	if (fftShiftEnabled_p)
	{
		uInt centralChan = inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ.size()/2;

		Double oldCentralFrequencyBeforeRegridding = inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ[centralChan];
		Double newCentralFrequencyBeforeRegriddingAtCurrentTime =
				freqTransEngine_p(oldCentralFrequencyBeforeRegridding).get(MSTransformations::Hz).getValue();
		Double newCentralFrequencyBeforeRegriddingAtReferenceTime = inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ_aux[centralChan];
		Double absoluteShift = newCentralFrequencyBeforeRegriddingAtCurrentTime - newCentralFrequencyBeforeRegriddingAtReferenceTime;

		Double chanWidth = inputOutputSpwMap_p[spwIndex].second.CHAN_FREQ[1] - inputOutputSpwMap_p[spwIndex].second.CHAN_FREQ[0];
		Double bandwidth = inputOutputSpwMap_p[spwIndex].second.CHAN_FREQ[inputOutputSpwMap_p[spwIndex].second.NUM_CHAN-1] - inputOutputSpwMap_p[spwIndex].second.CHAN_FREQ[0];
		bandwidth += chanWidth;

		fftShift_p = - absoluteShift / bandwidth;

		/*
		ostringstream oss;
		oss.precision(20);
		oss <<  "centralChan=" << centralChan << endl;
		oss <<  "oldCentralFrequencyBeforeRegridding=" << oldCentralFrequencyBeforeRegridding << endl;
		oss <<  "newCentralFrequencyBeforeRegriddingAtCurrentTime=" << newCentralFrequencyBeforeRegriddingAtCurrentTime << endl;
		oss <<  "newCentralFrequencyBeforeRegriddingAtReferenceTime=" << newCentralFrequencyBeforeRegriddingAtReferenceTime << endl;
		oss <<  "absoluteShift=" << absoluteShift << endl;
		oss <<  "xout[iDone][0]=" << inputOutputSpwMap_p[spwIndex].second.CHAN_FREQ[0] << endl;
		oss <<  "xout[iDone][1]=" << inputOutputSpwMap_p[spwIndex].second.CHAN_FREQ[1] << endl;
		oss <<  "chanWidth=" << chanWidth<< endl;
		oss <<  "xout[iDone][endChan]=" << inputOutputSpwMap_p[spwIndex].second.CHAN_FREQ[inputOutputSpwMap_p[spwIndex].second.NUM_CHAN-1] << endl;
		oss <<  "bandwidth (xout[iDone][endChan] - xout[iDone][0] + chanWidth)" << bandwidth << endl;
		oss <<  "chanwidth-spwInfo " << inputOutputSpwMap_p[spwIndex].second.CHAN_WIDTH[0] << endl;
		oss <<  "bandwidth-spwInfo " << inputOutputSpwMap_p[spwIndex].second.TOTAL_BANDWIDTH << endl;
		oss <<  "fftShift_p=" << fftShift_p << endl;
		logger_p << LogIO::NORMAL << oss.str() << LogIO::POST;
		*/

	}
	else
	{
	    for(uInt chan_idx=0; chan_idx<inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ.size(); chan_idx++)
	    {
	    	inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ_aux[chan_idx] =
	    	freqTransEngine_p(inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ[chan_idx]).get(MSTransformations::Hz).getValue();
	    }
	}

	return;
}

// ----------------------------------------------------------------------------------------
// Fill auxiliary (meta data) columns which don't depend on the SPW (merely consist of Ids)
// ----------------------------------------------------------------------------------------
void MSTransformManager::fillIdCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
	// Declare common auxiliary variables
	RefRows absoluteRefRows(rowRef.firstRow(),rowRef.firstRow()+nRowsToAdd_p-1);
	Vector<Int> tmpVectorInt(nRowsToAdd_p,0);
	Vector<Double> tmpVectorDouble(nRowsToAdd_p,0.0);
	Vector<Bool> tmpVectorBool(nRowsToAdd_p,False);

	// Special case for Data description Id
	if (transformDDIVector(vb->dataDescriptionIds(),tmpVectorInt))
	{
		outputMsCols_p->dataDescId().putColumnCells(absoluteRefRows,tmpVectorInt);
	}
	else
	{
		outputMsCols_p->dataDescId().putColumnCells(absoluteRefRows,vb->dataDescriptionIds());
	}

	// Re-indexable Columns
	transformAndWriteReindexableVector(	vb->observationId(),tmpVectorInt,True,
										inputOutputObservationIndexMap_p,
										outputMsCols_p->observationId(),absoluteRefRows);
	transformAndWriteReindexableVector(	vb->arrayId(),tmpVectorInt,True,
										inputOutputArrayIndexMap_p,
										outputMsCols_p->arrayId(),absoluteRefRows);
	transformAndWriteReindexableVector(	vb->fieldId(),tmpVectorInt,True,
										inputOutputFieldIndexMap_p,
										outputMsCols_p->fieldId(),absoluteRefRows);
	transformAndWriteReindexableVector(	vb->stateId(),tmpVectorInt,!timespan_p.contains("state"),
										inputOutputScanIntentIndexMap_p,
										outputMsCols_p->stateId(),absoluteRefRows);
	transformAndWriteReindexableVector(	vb->antenna1(),tmpVectorInt,False,
										inputOutputAntennaIndexMap_p,
										outputMsCols_p->antenna1(),absoluteRefRows);
	transformAndWriteReindexableVector(	vb->antenna2(),tmpVectorInt,False,
										inputOutputAntennaIndexMap_p,
										outputMsCols_p->antenna2(),absoluteRefRows);

	// Not Re-indexable Columns
	transformAndWriteNotReindexableVector(vb->scan(),tmpVectorInt,!timespan_p.contains("scan"),outputMsCols_p->scanNumber(),absoluteRefRows);
	transformAndWriteNotReindexableVector(vb->processorId(),tmpVectorInt,False,outputMsCols_p->processorId(),absoluteRefRows);
	transformAndWriteNotReindexableVector(vb->feed1(),tmpVectorInt,False,outputMsCols_p->feed1(),absoluteRefRows);
	transformAndWriteNotReindexableVector(vb->feed2(),tmpVectorInt,False,outputMsCols_p->feed2(),absoluteRefRows);
	transformAndWriteNotReindexableVector(vb->time(),tmpVectorDouble,False,outputMsCols_p->time(),absoluteRefRows);
	transformAndWriteNotReindexableVector(vb->timeCentroid(),tmpVectorDouble,False,outputMsCols_p->timeCentroid(),absoluteRefRows);
	transformAndWriteNotReindexableVector(vb->timeInterval(),tmpVectorDouble,False,outputMsCols_p->interval(),absoluteRefRows);

	// Special case for vectors that have to be averaged
	if (combinespws_p)
	{
		mapAndAverageVector(vb->flagRow(),tmpVectorBool);
		outputMsCols_p->flagRow().putColumnCells(absoluteRefRows, tmpVectorBool);

		// jagonzal: We average exposures by default, if they are the same we obtain the same results
		mapAndAverageVector(vb->exposure(),tmpVectorDouble);
		outputMsCols_p->exposure().putColumnCells(absoluteRefRows, tmpVectorDouble);
	}
	else
	{
		outputMsCols_p->flagRow().putColumnCells(absoluteRefRows, vb->flagRow());
		outputMsCols_p->exposure().putColumnCells(absoluteRefRows, vb->exposure());
	}

	if (combinespws_p)
	{
		Matrix<Double> tmpUvw(IPosition(2,3,rowRef.nrows()),0.0);
		Matrix<Float> tmpMatrixFloat(IPosition(2,vb->nCorrelations(),rowRef.nrows()),0.0);

		mapMatrix(vb->uvw(),tmpUvw);
		writeMatrix(tmpUvw,outputMsCols_p->uvw(),rowRef,nspws_p);

		if (channelAverage_p)
		{
			mapScaleAndAverageMatrix(vb->weight(),tmpMatrixFloat,weightFactorMap_p,vb->spectralWindows());
			writeMatrix(tmpMatrixFloat,outputMsCols_p->weight(),rowRef,nspws_p);
		}
		else
		{
			// jagonzal: According to dpetry we have to copy weights from the first SPW
			// This is justified since the rows to be combined _must_ be from the
			// same baseline and therefore have the same UVW coordinates in the MS (in meters).
			// They could therefore be regarded to also have the same WEIGHT, at least to
			// a good approximation.
			mapMatrix(vb->weight(),tmpMatrixFloat);
			writeMatrix(tmpMatrixFloat,outputMsCols_p->weight(),rowRef,nspws_p);
		}


		// Sigma must be redefined to 1/weight when corrected data becomes data
		if (correctedToData_p)
		{
			arrayTransformInPlace(tmpMatrixFloat, MSTransformations::wtToSigma);
			outputMsCols_p->sigma().putColumnCells(rowRef, tmpMatrixFloat);
			writeMatrix(tmpMatrixFloat,outputMsCols_p->sigma(),rowRef,nspws_p);
		}
		else
		{
			if (channelAverage_p)
			{
				mapScaleAndAverageMatrix(vb->sigma(),tmpMatrixFloat,sigmaFactorMap_p,vb->spectralWindows());
				outputMsCols_p->sigma().putColumnCells(rowRef, tmpMatrixFloat);
				writeMatrix(tmpMatrixFloat,outputMsCols_p->sigma(),rowRef,nspws_p);
			}
			else
			{
				// jagonzal: According to dpetry we have to copy weights from the first SPW
				// This is justified since the rows to be combined _must_ be from the
				// same baseline and therefore have the same UVW coordinates in the MS (in meters).
				// They could therefore be regarded to also have the same WEIGHT, at least to
				// a good approximation.
				mapMatrix(vb->sigma(),tmpMatrixFloat);
				writeMatrix(tmpMatrixFloat,outputMsCols_p->sigma(),rowRef,nspws_p);
			}
		}
	}
	else
	{
		writeMatrix(vb->uvw(),outputMsCols_p->uvw(),rowRef,nspws_p);

		Matrix<Float> weights = vb->weight();
		if (channelAverage_p)
		{
			if ( (weightFactorMap_p.find(vb->spectralWindows()(0))  != weightFactorMap_p.end()) and
					(weightFactorMap_p[vb->spectralWindows()(0)] != 1) )
			{
				weights *= weightFactorMap_p[vb->spectralWindows()(0)];
			}
		}
		writeMatrix(weights,outputMsCols_p->weight(),rowRef,nspws_p);

		// Sigma must be redefined to 1/weight when corrected data becomes data
		if (correctedToData_p)
		{
			arrayTransformInPlace(weights, MSTransformations::wtToSigma);
			writeMatrix(weights,outputMsCols_p->sigma(),rowRef,nspws_p);
		}
		else
		{
			Matrix<Float> sigma = vb->sigma();
			if (channelAverage_p)
			{
				if ( (sigmaFactorMap_p.find(vb->spectralWindows()(0)) != sigmaFactorMap_p.end()) and
						(sigmaFactorMap_p[vb->spectralWindows()(0)] != 1) )
				{
					sigma *= sigmaFactorMap_p[vb->spectralWindows()(0)];
				}
			}
			writeMatrix(sigma,outputMsCols_p->sigma(),rowRef,nspws_p);
		}
	}

	return;
}

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
template <class T> void MSTransformManager::transformAndWriteNotReindexableVector(	const Vector<T> &inputVector,
																					Vector<T> &outputVector,
																					Bool constant,
																					ScalarColumn<T> &outputCol,
																					RefRows &rowReference)
{
	Bool transformed = transformNotReindexableVector(inputVector,outputVector,constant);

	if (transformed)
	{
		outputCol.putColumnCells(rowReference, outputVector);
	}
	else
	{
		outputCol.putColumnCells(rowReference, inputVector);
	}

	return;
};

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
template <class T> void MSTransformManager::transformAndWriteReindexableVector(	const Vector<T> &inputVector,
															Vector<T> &outputVector,
															Bool constant,
															map<Int,Int> &inputOutputIndexMap,
															ScalarColumn<T> &outputCol,
															RefRows &rowReference)
{
	Bool transformed = transformReindexableVector(inputVector,outputVector,constant,inputOutputIndexMap);

	if (transformed)
	{
		outputCol.putColumnCells(rowReference, outputVector);
	}
	else
	{
		outputCol.putColumnCells(rowReference, inputVector);
	}

	return;
};

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
Bool MSTransformManager::transformDDIVector(const Vector<Int> &inputVector,Vector<Int> &outputVector)
{
	Bool transformed = True;

	if (combinespws_p)
	{
		if (nspws_p <2)
		{
			outputVector = ddiStart_p;
		}
		else
		{
			uInt absoluteIndex = 0;
			for (uInt index=0; index<rowIndex_p.size();index++)
			{
				for (uInt spwIndex=0;spwIndex < nspws_p; spwIndex++)
				{
					outputVector(absoluteIndex) = ddiStart_p + spwIndex;
					absoluteIndex += 1;
				}
			}
		}
	}
	else
	{
		transformed = transformReindexableVector(inputVector,outputVector,True,inputOutputDDIndexMap_p);
	}

	return transformed;
}

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
template <class T> Bool MSTransformManager::transformNotReindexableVector(	const Vector<T> &inputVector,
																			Vector<T> &outputVector,
																			Bool constant)
{
	Bool transformed = True;

	if (combinespws_p)
	{
		if (constant)
		{
			outputVector = inputVector(0);
		}
		else
		{
			mapVector(inputVector,outputVector);
		}
	}
	else
	{
		transformed = False;
	}

	return transformed;
};

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
template <class T> Bool MSTransformManager::transformReindexableVector(	const Vector<T> &inputVector,
																		Vector<T> &outputVector,
																		Bool constant,
																		map<Int,Int> &inputOutputIndexMap)
{
	Bool transformed = True;

	if (inputOutputIndexMap.size())
	{
		if (constant)
		{
			outputVector = inputOutputIndexMap[inputVector(0)];
		}
		else if (combinespws_p)
		{
			mapAndReindexVector(inputVector,outputVector,inputOutputIndexMap);
		}
		else
		{
			reindexVector(inputVector,outputVector,inputOutputIndexMap);
		}
	}
	else
	{
		transformed = transformNotReindexableVector(inputVector,outputVector,constant);
	}

	return transformed;
};

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
template <class T> void MSTransformManager::mapAndReindexVector(	const Vector<T> &inputVector,
																	Vector<T> &outputVector,
																	map<Int,Int> &inputOutputIndexMap)
{
	if (nspws_p <2)
	{
		for (uInt index=0; index<rowIndex_p.size();index++)
		{
			outputVector(index) = inputOutputIndexMap[inputVector(rowIndex_p[index])];
		}
	}
	else
	{
		uInt absoluteIndex = 0;
		for (uInt index=0; index<rowIndex_p.size();index++)
		{
			for (uInt spwIndex=0;spwIndex < nspws_p; spwIndex++)
			{
				outputVector(absoluteIndex) = inputOutputIndexMap[inputVector(rowIndex_p[index])];
				absoluteIndex += 1;
			}
		}
	}

	return;
}

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
template <class T> void MSTransformManager::reindexVector(	const Vector<T> &inputVector,
															Vector<T> &outputVector,
															map<Int,Int> &inputOutputIndexMap)
{
	if (nspws_p <2)
	{
		for (uInt index=0; index<inputVector.shape()[0];index++)
		{
			outputVector(index) = inputOutputIndexMap[inputVector(index)];
		}
	}
	else
	{
		uInt absoluteIndex = 0;
		for (uInt index=0; index<inputVector.shape()[0];index++)
		{
			for (uInt spwIndex=0;spwIndex < nspws_p; spwIndex++)
			{
				outputVector(absoluteIndex) = inputOutputIndexMap[inputVector(index)];
				absoluteIndex += 1;
			}
		}
	}

	return;
};

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
template <class T> void MSTransformManager::mapVector(	const Vector<T> &inputVector,
														Vector<T> &outputVector)
{
	if (nspws_p < 2)
	{
		for (uInt index=0; index<rowIndex_p.size();index++)
		{
			outputVector(index) = inputVector(rowIndex_p[index]);
		}
	}
	else
	{
		uInt absoluteIndex = 0;
		for (uInt index=0; index<rowIndex_p.size();index++)
		{
			for (uInt spwIndex=0;spwIndex < nspws_p; spwIndex++)
			{
				outputVector(absoluteIndex) = inputVector(rowIndex_p[index]);
				absoluteIndex += 1;
			}
		}
	}


	return;
}

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
void MSTransformManager::mapAndAverageVector(	const Vector<Double> &inputVector,
												Vector<Double> &outputVector)
{
	Double average;
	vector<uInt> baselineRows;
	uInt row, counts, absoluteIndex = 0;
	for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
	{
		// Get baseline rows vector
		baselineRows = iter->second;

		// Compute combined value from each SPW
		counts = 0;

		for (vector<uInt>::iterator iter = baselineRows.begin();iter != baselineRows.end(); iter++)
		{
			row = *iter;
			if (counts == 0)
			{
				average = inputVector(row);
			}
			else
			{
				average += inputVector(row);
			}

			counts += 1;
		}

		// Normalize value
		if (counts) average /= counts;

		// Set value in output vector
		for (uInt spwIndex=0;spwIndex < nspws_p; spwIndex++)
		{
			outputVector(absoluteIndex) = average;
			absoluteIndex += 1;
		}
	}

	return;
}

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------
void MSTransformManager::mapAndAverageVector(	const Vector<Bool> &inputVector,
												Vector<Bool> &outputVector)
{
	Bool average;
	vector<uInt> baselineRows;
	uInt row, counts, absoluteIndex = 0;
	for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
	{
		// Get baseline rows vector
		baselineRows = iter->second;

		// Compute combined value from each SPW
		counts = 0;

		for (vector<uInt>::iterator iter = baselineRows.begin();iter != baselineRows.end(); iter++)
		{
			row = *iter;
			if (counts == 0)
			{
				average = inputVector(row);
			}
			else
			{
				average &= inputVector(row);
			}
		}

		// Set value in output vector
		for (uInt spwIndex=0;spwIndex < nspws_p; spwIndex++)
		{
			outputVector(absoluteIndex) = average;
			absoluteIndex += 1;
		}
	}

	return;
}

// ------------------------------------------------------------------------------------
// Fill the data from an input matrix with shape [nCol,nBaselinesxnSPWsxnScans/nStates]
// into an output matrix with shape [nCol,nBaselinesxnScans/nStates]
// ------------------------------------------------------------------------------------
template <class T> void MSTransformManager::mapMatrix(	const Matrix<T> &inputMatrix,Matrix<T> &outputMatrix)
{
	// Get number of columns
	uInt nCols = outputMatrix.shape()(0);

	for (uInt index=0; index<rowIndex_p.size();index++)
	{
		for (uInt col = 0; col < nCols; col++)
		{
			outputMatrix(col,index) = inputMatrix(col,rowIndex_p[index]);
		}
	}

	return;
}

// -----------------------------------------------------------------------------------
// Fill the data from an input matrix with shape [nCol,nBaselinesxnSPWs] into an
// output matrix with shape [nCol,nBaselines] accumulating the averaging from all SPWS
// -----------------------------------------------------------------------------------
template <class T> void MSTransformManager::mapAndAverageMatrix(	const Matrix<T> &inputMatrix,
																		Matrix<T> &outputMatrix,
																		Bool convolveFlags,
																		vi::VisBuffer2 *vb)
{
	// Get number of columns
	uInt nCols = outputMatrix.shape()(0);

    // Access FLAG_ROW in case we need to convolute the average
	Vector<Bool> flags;
	if (convolveFlags) flags = vb->flagRow();

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
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::mapScaleAndAverageMatrix(	const Matrix<T> &inputMatrix,
																		Matrix<T> &outputMatrix,
																		map<Int,T> scaleMap,
																		Vector<Int> spws)
{
	// Reset output Matrix
	outputMatrix = 0;

	// Get number of columns
	uInt nCols = outputMatrix.shape()(0);

    // Fill output array with the combined data from each SPW
	Int spw;
	uInt row;
	uInt baseline_index = 0;
	vector<uInt> baselineRows;
	T contributionFactor, normalizingFactor;
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
			spw = spws(row);
			if (scaleMap.find(spw) != scaleMap.end())
			{
				contributionFactor = scaleMap[spw];
			}
			else
			{
				contributionFactor = 1;
			}


			for (uInt col = 0; col < nCols; col++)
			{
				outputMatrix(col,baseline_index) += contributionFactor*inputMatrix(col,row);
			}

			// normalizingFactor += contributionFactor;
			normalizingFactor += 1;
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

// ----------------------------------------------------------------------------------------
// Fill main (data) columns which have to be combined together to produce bigger SPWs
// ----------------------------------------------------------------------------------------
void MSTransformManager::fillDataCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
	// First of all we fill WEIGHT_SPECTRUM because it might be needed by channel average
    if (inputWeightSpectrumAvailable_p)
    {
    	// Unset all the weights-based operations
    	setWeightBasedTransformations(MSTransformations::flat);

		// Transform weights
    	transformCubeOfData(vb,rowRef,vb->weightSpectrum(),outputMsCols_p->weightSpectrum(),NULL);

    	// Reset all the weights-based operations
    	setWeightBasedTransformations(weightmode_p);
    }
    else if (combinespws_p or usewtspectrum_p)
    {
    	// Fill WEIGHT_SPECTRUM with WEIGHTS
		Matrix<Float> inputWeightPlane = vb->weight();
		IPosition PolChanRow = vb->flagCube().shape();
		weightSpectrumCube_p.resize(PolChanRow,False);
		Double scaleFactor = 1.0/PolChanRow(1);
		for (uInt row=0; row < PolChanRow(2); row++)
		{
			Matrix<Float> inputWeightSpectrumPlane = weightSpectrumCube_p.xyPlane(row);
			for (uInt pol = 0; pol < PolChanRow(0); pol++)
			{
				inputWeightSpectrumPlane.row(pol) = scaleFactor*inputWeightPlane(pol,row);
			}
		}

		if (usewtspectrum_p)
		{
			// Unset all the weights-based operations
			setWeightBasedTransformations(MSTransformations::flat);

			// jagonzal: I don't want to spend so much resources transforming and writing synthetic WEIGHT_SPECTRUM
			transformCubeOfData(vb,rowRef,weightSpectrumCube_p,outputMsCols_p->weightSpectrum(),NULL);

			// Reset all the weights-based operations
			setWeightBasedTransformations(weightmode_p);
		}
    }

	ArrayColumn<Bool> *outputFlagCol=NULL;
	for (dataColMap::iterator iter = dataColMap_p.begin();iter != dataColMap_p.end();iter++)
	{
		switch (iter->first)
		{
			case MS::DATA:
			{
				if (mainColumn_p == MS::DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				transformCubeOfData(vb,rowRef,vb->visCube(),outputMsCols_p->data(), outputFlagCol);

				break;
			}
			case MS::CORRECTED_DATA:
			{
				if (mainColumn_p == MS::CORRECTED_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				if (iter->second == MS::DATA)
				{
					transformCubeOfData(vb,rowRef,vb->visCubeCorrected(),outputMsCols_p->data(), outputFlagCol);
				}
				else
				{
					transformCubeOfData(vb,rowRef,vb->visCubeCorrected(),outputMsCols_p->correctedData(), outputFlagCol);
				}

				break;
			}
			case MS::MODEL_DATA:
			{
				if (mainColumn_p == MS::MODEL_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				if (iter->second == MS::DATA)
				{
					transformCubeOfData(vb,rowRef,vb->visCubeModel(),outputMsCols_p->data(), outputFlagCol);
				}
				else
				{
					transformCubeOfData(vb,rowRef,vb->visCubeModel(),outputMsCols_p->modelData(), outputFlagCol);
				}
				break;
			}
			case MS::FLOAT_DATA:
			{
				if (mainColumn_p == MS::FLOAT_DATA)
				{
					outputFlagCol = &(outputMsCols_p->flag());
				}
				else
				{
					outputFlagCol = NULL;
				}

				transformCubeOfData(vb,rowRef,vb->visCubeFloat(),outputMsCols_p->floatData(), outputFlagCol);

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

    // Special case for flag category
    if (fillFlagCategory_p)
    {
    	if (combinespws_p)
    	{
            IPosition shapeFlagCategory = vb->flagCategory().shape();
            shapeFlagCategory(3) = rowRef.nrow();
            Array<Bool> flagCategory(shapeFlagCategory,
            		const_cast<Bool*>(vb->flagCategory().getStorage(MSTransformations::False)),SHARE);
        	outputMsCols_p->flagCategory().putColumnCells(rowRef, flagCategory);
    	}
    	else
    	{
        	outputMsCols_p->flagCategory().putColumnCells(rowRef, vb->flagCategory());
    	}
    }

	return;
}

// -----------------------------------------------------------------------
// Generic method to write a Matrix from a VisBuffer into a ArrayColumn
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::writeMatrix(	const Matrix<T> &inputMatrix,
																ArrayColumn<T> &outputCol,
																RefRows &rowRef,
																uInt nBlocks)
{
	if (nBlocks == 1)
	{
		 outputCol.putColumnCells(rowRef, inputMatrix);
	}
	else
	{
		uInt offset = 0;
		for (uInt block_i=0;block_i<nBlocks;block_i++)
		{
			uInt startRow_i = rowRef.firstRow()+offset;
			RefRows rowRef_i(startRow_i, startRow_i+inputMatrix.shape()(1)-1);
		    outputCol.putColumnCells(rowRef_i, inputMatrix);
		    offset += inputMatrix.shape()(1);
		}
	}
	return;
}

// -----------------------------------------------------------------------
// Generic method to write a Cube from a VisBuffer into a ArrayColumn
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::writeCube(	const Cube<T> &inputCube,
															ArrayColumn<T> &outputCol,
															RefRows &rowRef)
{
	IPosition shape = inputCube.shape();
	shape(2) = rowRef.nrows();
    Array<T> outputArray(shape,const_cast<T*>(inputCube.getStorage(MSTransformations::False)),SHARE);
    outputCol.putColumnCells(rowRef, outputArray);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::transformCubeOfData(	vi::VisBuffer2 *vb,
													RefRows &rowRef,
													const Cube<Complex> &inputDataCube,
													ArrayColumn<Complex> &outputDataCol,
													ArrayColumn<Bool> *outputFlagCol)
{
	(*this.*transformCubeOfDataComplex_p)(vb,rowRef,inputDataCube,outputDataCol,outputFlagCol);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::transformCubeOfData(	vi::VisBuffer2 *vb,
													RefRows &rowRef,
													const Cube<Float> &inputDataCube,
													ArrayColumn<Float> &outputDataCol,
													ArrayColumn<Bool> *outputFlagCol)
{
	(*this.*transformCubeOfDataFloat_p)(vb,rowRef,inputDataCube,outputDataCol,outputFlagCol);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::copyCubeOfData(	vi::VisBuffer2 *vb,
																RefRows &rowRef,
																const Cube<T> &inputDataCube,
																ArrayColumn<T> &outputDataCol,
																ArrayColumn<Bool> *outputFlagCol)
{
	writeCube(inputDataCube,outputDataCol,rowRef);
	if (outputFlagCol != NULL)
	{
		writeCube(vb->flagCube(),*outputFlagCol,rowRef);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::combineCubeOfData(	vi::VisBuffer2 *vb,
																	RefRows &rowRef,
																	const Cube<T> &inputDataCube,
																	ArrayColumn<T> &outputDataCol,
																	ArrayColumn<Bool> *outputFlagCol)
{
	// Write flag column too?
	if (outputFlagCol != NULL)
	{
		writeOutputFlagsPlaneSlices_p = &MSTransformManager::writeOutputFlagsPlaneSlices;
		writeOutputFlagsPlaneReshapedSlices_p = &MSTransformManager::writeOutputFlagsPlaneReshapedSlices;
		writeOutputFlagsPlane_p = &MSTransformManager::writeOutputFlagsPlane;
	}
	else
	{
		writeOutputFlagsPlaneSlices_p = &MSTransformManager::dontWriteOutputFlagsPlaneSlices;
		writeOutputFlagsPlaneReshapedSlices_p = &MSTransformManager::dontWriteOutputPlaneReshapedSlices;
		writeOutputFlagsPlane_p = &MSTransformManager::dontWriteOutputFlagsPlane;
	}

	// Get input flag and weight cubes
	const Cube<Bool> inputFlagCube = vb->flagCube();
	Cube<Float> inputWeightsCubeLocal;
	if (inputWeightSpectrumAvailable_p)
	{
		inputWeightsCubeLocal = Cube<Float>(	vb->weightSpectrum().shape(),
												const_cast<Float*>(vb->weightSpectrum().getStorage(MSTransformations::False)),
												SHARE);
	}
	else
	{
		inputWeightsCubeLocal = Cube<Float>(	weightSpectrumCube_p.shape(),
												const_cast<Float*>(weightSpectrumCube_p.getStorage(MSTransformations::False)),
												SHARE);
	}

	// Get input SPWs and exposures
	Vector<Int> spws = vb->spectralWindows();
	Vector<Double> exposures = vb->exposure();

	// Get input cube shape
	IPosition inputCubeShape = inputDataCube.shape();
	uInt nInputCorrelations = inputCubeShape(0);
	uInt nInputChannels = inputCubeShape(1);

	// Initialize input planes
	IPosition inputPlaneShape(2,nInputCorrelations, numOfCombInputChanMap_p[0]);
	Matrix<Double> normalizingFactorPlane(inputPlaneShape);
	Matrix<T> inputPlaneData(inputPlaneShape);
	Matrix<Bool> inputPlaneFlags(inputPlaneShape,False);
	Matrix<Float> inputPlaneWeights(inputPlaneShape);

	// Initialize output planes
	IPosition outputPlaneShape(2,nInputCorrelations, inputOutputSpwMap_p[0].second.NUM_CHAN);
	Matrix<T> outputPlaneData(outputPlaneShape);
	Matrix<Bool> outputPlaneFlags(outputPlaneShape);

	Int spw = 0;
	Double weight;
	uInt inputChannel;
	Bool inputChanelFlag;
	Double normalizingFactor;
	uInt row = 0, baseline_index = 0;
	vector<uInt> baselineRows;
	map<Int, uInt> spwRowMap;
	map<Int, uInt>::iterator spwRowMapIter;
	map<Int, uInt> spwFractionCountsMap;
	Bool unityContributors = False;
	vector< channelContribution > contributions;
	vector< channelContribution >::iterator contributionsIter;
	map < Int , map < uInt, Bool > > removeContributionsMap;

	Bool combinationOfSPWsWithDifferentExposure = False;
	Double exposure = 0;

	for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
	{
		// Initialize input plane
		inputPlaneData = 0.0;

		// Initialize weights plane
		inputPlaneWeights = 0.0;

		// Initialize normalizing factor plane
		normalizingFactorPlane = 0.0;

		// Fill input plane to benefit from contiguous access to the input cube
		baselineRows = iter->second;

		// Create spw-row map for this baseline and initialize detection of SPWs with different exposure
		spwRowMap.clear();
		if (combinationOfSPWsWithDifferentExposure_p)
		{
			combinationOfSPWsWithDifferentExposure = True;
			addWeightSpectrumContribution_p = &MSTransformManager::addWeightSpectrumContribution;
			for (vector<uInt>::iterator iter = baselineRows.begin();iter != baselineRows.end(); iter++)
			{
				row = *iter;
				spw = spws(row);
				spwRowMap[spw]=row;
			}
		}
		else
		{
			exposure = exposures(*baselineRows.begin());
			combinationOfSPWsWithDifferentExposure = False;
			for (vector<uInt>::iterator iter = baselineRows.begin();iter != baselineRows.end(); iter++)
			{
				row = *iter;
				spw = spws(row);
				spwRowMap[spw]=row;

				if (abs(exposure - exposures(row)) > FLT_EPSILON)
				{
					combinationOfSPWsWithDifferentExposure = True;
				}
			}

			if (combinationOfSPWsWithDifferentExposure)
			{
				combinationOfSPWsWithDifferentExposure_p = True;
				addWeightSpectrumContribution_p = &MSTransformManager::addWeightSpectrumContribution;
				if (inputWeightSpectrumAvailable_p)
				{
					logger_p 	<< LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
								<< "Detected combination of SPWs with different EXPOSURE "<< endl
								<< "Will use WEIGHT_SPECTRUM to combine them "<< endl
								<< LogIO::POST;
				}
				else
				{
					logger_p 	<< LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
								<< "Detected combination of SPWs with different EXPOSURE "<< endl
								<< "Will use WEIGHT to combine them (WEIGHT_SPECTRUM not available)"<< endl
								<< LogIO::POST;
				}
			}
			else
			{
				addWeightSpectrumContribution_p = &MSTransformManager::dontAddWeightSpectrumContribution;
			}
		}

		for (uInt outputChannel = 0; outputChannel < numOfCombInputChanMap_p[0]; outputChannel++)
		{
			contributions = inputOutputChanFactorMap_p[outputChannel];

			for (uInt pol = 0; pol < inputDataCube.shape()(0); pol++)
			{
				spwFractionCountsMap.clear();
				unityContributors = False;

				// Go through list of contributors for this output channel and polarization and gather flags info
				for (contributionsIter = contributions.begin(); contributionsIter != contributions.end(); contributionsIter++)
				{
					inputChannel = contributionsIter->inpChannel;
					weight = contributionsIter->weight;

					// Add WEIGHT_SPECTRUM to the contribution
					(*this.*addWeightSpectrumContribution_p)(weight,pol,inputChannel,row,inputWeightsCubeLocal);

					// Find row for this input channel
					spw = contributionsIter->inpSpw;
					spwRowMapIter = spwRowMap.find(spw);
					if (spwRowMapIter != spwRowMap.end())
					{
						row = spwRowMap[spw];

						// Fill flags info
						inputChanelFlag = inputFlagCube(pol,inputChannel,row);
						contributionsIter->flag = inputChanelFlag;

						// Count input channel if it is not flagged and has non-unity overlapping fraction
						if (weight<1.0)
						{
							if (!inputChanelFlag) spwFractionCountsMap[spw] += 1;
						}
						// Count if we have valid unity contributors, otherwise we don't discard non-unity contributors
						else
						{
							unityContributors = True;
						}
					}
					else
					{
						// Fill flags info
						contributionsIter->flag = True;
					}
				}

				// Remove contributions from SPWs with odd numbers of contributors with non-unity
				// overlap fraction which could influence the averaging asymmetrically
				for (contributionsIter = contributions.begin(); contributionsIter != contributions.end(); contributionsIter++)
				{
					inputChannel = contributionsIter->inpChannel;
					weight = contributionsIter->weight;
					spw = contributionsIter->inpSpw;

					// Find row for this input channel
					if (!contributionsIter->flag)
					{
						// jagonzal: Caution, accessing the map populates it!!!
						row = spwRowMap[spw];

						if ((spwFractionCountsMap[spw] % 2 == 0) or (weight >= 1.0) or (!unityContributors))
						{
							inputPlaneData(pol,outputChannel) += weight*inputDataCube(pol,inputChannel,row);
							normalizingFactorPlane(pol,outputChannel) += weight;
							(*this.*fillWeightsPlane_p)(pol,inputChannel,outputChannel,row,inputWeightsCubeLocal,inputPlaneWeights,weight);

							/*
							cout 	<< "outChan=" << outputChannel
									<< " inpChan=" << inputChannel
									<< " spw=" << spw
									<< " fraction count=" << spwFractionCountsMap[spw]
									<< " unity contributors=" << unityContributors
									<< " weight=" << weight
									<< " value=" << inputDataCube(pol,inputChannel,row)
									<< " flag=" << inputFlagCube(pol,inputChannel,row)
									<< " OK" << endl;
							*/
						}
					}
				}
			}
		}

		// Normalize combined data and determine input plane flags
		inputPlaneFlags = False;
		for (uInt outputChannel = 0; outputChannel < numOfCombInputChanMap_p[0]; outputChannel++)
		{
			for (uInt pol = 0; pol < nInputCorrelations; pol++)
			{
				normalizingFactor = normalizingFactorPlane(pol,outputChannel);
				if (normalizingFactor >= 1)
				{
					inputPlaneData(pol,outputChannel) /= normalizingFactorPlane(pol,outputChannel);

					// Normalize weights plane
					(*this.*normalizeWeightsPlane_p)(pol,outputChannel,inputPlaneWeights,normalizingFactorPlane);
				}
				else if (normalizingFactor > 0)
				{
					inputPlaneData(pol,outputChannel) /= normalizingFactorPlane(pol,outputChannel);
					inputPlaneFlags(pol,outputChannel) = True;
				}
				else
				{
					inputPlaneFlags(pol,outputChannel) = True;
				}
			}
		}

		// Initialize output flags plane
		outputPlaneFlags = False;

		// Transform input planes and write them
		transformAndWritePlaneOfData(	0,rowRef.firstRow()+baseline_index*nspws_p,
										inputPlaneData,inputPlaneFlags,inputPlaneWeights,
										outputPlaneData,outputPlaneFlags,outputDataCol,outputFlagCol);

		baseline_index += 1;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::addWeightSpectrumContribution(	Double &weight,
															uInt &pol,
															uInt &inputChannel,
															uInt &row,
															Cube<Float> &inputWeightsCube)
{
	weight *= inputWeightsCube(pol,inputChannel,row);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::dontAddWeightSpectrumContribution(	Double &weight,
																uInt &pol,
																uInt &inputChannel,
																uInt &row,
																Cube<Float> &inputWeightsCube)
{
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::fillWeightsPlane(	uInt pol,
												uInt inputChannel,
												uInt outputChannel,
												uInt inputRow,
												const Cube<Float> &inputWeightsCube,
												Matrix<Float> &inputWeightsPlane,
												Double factor)
{
	inputWeightsPlane(pol,outputChannel) += factor*inputWeightsCube(pol,inputChannel,inputRow);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::normalizeWeightsPlane(	uInt pol,
													uInt outputChannel,
													Matrix<Float> &inputPlaneWeights,
													Matrix<Double> &normalizingFactorPlane)
{
	inputPlaneWeights(pol,outputChannel) /= normalizingFactorPlane(pol,outputChannel);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::averageCubeOfData(	vi::VisBuffer2 *vb,
																	RefRows &rowRef,
																	const Cube<T> &inputDataCube,
																	ArrayColumn<T> &outputDataCol,
																	ArrayColumn<Bool> *outputFlagCol)
{
	// Get input spw and flag and weight cubes
	Int inputSpw = vb->spectralWindows()(0);
	const Cube<Bool> inputFlagsCube = vb->flagCube();
	const Cube<Float> inputWeightsCube = vb->weightSpectrum();

	// Define output plane shape
	IPosition outputPlaneShape = IPosition(2,inputDataCube.shape()(0), numOfOutChanMap_p[inputSpw]);

	transformAndWriteCubeOfData(	inputSpw, rowRef,
									inputDataCube, inputFlagsCube, inputWeightsCube,
									outputPlaneShape, outputDataCol, outputFlagCol);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::smoothCubeOfData(	vi::VisBuffer2 *vb,
																	RefRows &rowRef,
																	const Cube<T> &inputDataCube,
																	ArrayColumn<T> &outputDataCol,
																	ArrayColumn<Bool> *outputFlagCol)
{
	// Get input spw and flag cube
	Int inputSpw = vb->spectralWindows()(0);
	const Cube<Bool> inputFlagsCube = vb->flagCube();
	const Cube<Float> inputWeightsCube = vb->weightSpectrum();

	// Define output plane shape
	IPosition outputPlaneShape = IPosition(2,inputDataCube.shape()(0), inputDataCube.shape()(1));

	// Transform cube
	transformAndWriteCubeOfData(	inputSpw, rowRef,
									inputDataCube, inputFlagsCube, inputWeightsCube,
									outputPlaneShape, outputDataCol, outputFlagCol);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::regridCubeOfData(	vi::VisBuffer2 *vb,
																	RefRows &rowRef,
																	const Cube<T> &inputDataCube,
																	ArrayColumn<T> &outputDataCol,
																	ArrayColumn<Bool> *outputFlagCol)
{
	// Get input spw and flag cube
	Int inputSpw = vb->spectralWindows()(0);
	const Cube<Bool> inputFlagsCube = vb->flagCube();
	const Cube<Float> inputWeightsCube = vb->weightSpectrum();

	// Define output plane shape
	IPosition outputPlaneShape = IPosition(2,inputDataCube.shape()(0), inputOutputSpwMap_p[inputSpw].second.NUM_CHAN);

	// Transform cube
	transformAndWriteCubeOfData(	inputSpw, rowRef,
									inputDataCube, inputFlagsCube, inputWeightsCube,
									outputPlaneShape, outputDataCol, outputFlagCol);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::transformAndWriteCubeOfData(	Int inputSpw,
																				RefRows &rowRef,
																				const Cube<T> &inputDataCube,
																				const Cube<Bool> &inputFlagsCube,
																				const Cube<Float> &inputWeightsCube,
																				IPosition &outputPlaneShape,
																				ArrayColumn<T> &outputDataCol,
																				ArrayColumn<Bool> *outputFlagCol)
{
	// Write flag column too?
	if (outputFlagCol != NULL)
	{
		writeOutputFlagsPlaneSlices_p = &MSTransformManager::writeOutputFlagsPlaneSlices;
		writeOutputFlagsPlaneReshapedSlices_p = &MSTransformManager::writeOutputFlagsPlaneReshapedSlices;
		writeOutputFlagsPlane_p = &MSTransformManager::writeOutputFlagsPlane;
	}
	else
	{
		writeOutputFlagsPlaneSlices_p = &MSTransformManager::dontWriteOutputFlagsPlaneSlices;
		writeOutputFlagsPlaneReshapedSlices_p = &MSTransformManager::dontWriteOutputPlaneReshapedSlices;
		writeOutputFlagsPlane_p = &MSTransformManager::dontWriteOutputFlagsPlane;
	}

	// Get input number of rows
	uInt nInputRows = inputDataCube.shape()(2);

	// Initialize input planes
	Matrix<T> inputPlaneData;
	Matrix<Bool> inputPlaneFlags;
	Matrix<Float> inputPlaneWeights;

	// Initialize output planes
	Matrix<T> outputPlaneData(outputPlaneShape);
	Matrix<Bool> outputPlaneFlags(outputPlaneShape);

	// Iterate row by row in order to extract a plane
	for (uInt rowIndex=0; rowIndex < nInputRows; rowIndex++)
	{
		// Initialize output flags plane
		outputPlaneFlags = False;

		// Fill input planes by reference
		inputPlaneData = inputDataCube.xyPlane(rowIndex);
		inputPlaneFlags = inputFlagsCube.xyPlane(rowIndex);
		(*this.*setWeightsPlaneByReference_p)(rowIndex,inputWeightsCube,inputPlaneWeights);

		// Transform input planes and write them
		transformAndWritePlaneOfData(	inputSpw,rowRef.firstRow()+rowIndex*nspws_p,
										inputPlaneData,inputPlaneFlags,inputPlaneWeights,
										outputPlaneData,outputPlaneFlags,outputDataCol,outputFlagCol);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::separateCubeOfData(	vi::VisBuffer2 *vb,
																	RefRows &rowRef,
																	const Cube<T> &inputDataCube,
																	ArrayColumn<T> &outputDataCol,
																	ArrayColumn<Bool> *outputFlagCol)
{
	// Write flag column too?
	if (outputFlagCol != NULL)
	{
		writeOutputFlagsPlaneSlices_p = &MSTransformManager::writeOutputFlagsPlaneSlices;
		writeOutputFlagsPlaneReshapedSlices_p = &MSTransformManager::writeOutputFlagsPlaneReshapedSlices;
		writeOutputFlagsPlane_p = &MSTransformManager::writeOutputFlagsPlane;
	}
	else
	{
		writeOutputFlagsPlaneSlices_p = &MSTransformManager::dontWriteOutputFlagsPlaneSlices;
		writeOutputFlagsPlaneReshapedSlices_p = &MSTransformManager::dontWriteOutputPlaneReshapedSlices;
		writeOutputFlagsPlane_p = &MSTransformManager::dontWriteOutputFlagsPlane;
	}

	// Get input flags, spw and number of rows
	Int inputSpw = vb->spectralWindows()(0);
	uInt nInputRows = inputDataCube.shape()(2);
	const Cube<Bool> inputFlagsCube = vb->flagCube();

	// Initialize input planes
	Matrix<T> inputPlaneData;
	Matrix<Bool> inputPlaneFlags;

	// Iterate row by row in order to extract a plane
	for (uInt rowIndex=0; rowIndex < nInputRows; rowIndex++)
	{
		// Fill input planes by reference
		inputPlaneData = inputDataCube.xyPlane(rowIndex);
		inputPlaneFlags = inputFlagsCube.xyPlane(rowIndex);

		// Directly write output plane
		writeOutputPlanes(	rowRef.firstRow()+rowIndex*nspws_p,
							inputPlaneData,inputPlaneFlags,
							outputDataCol,*outputFlagCol);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::setWeightsPlaneByReference(	uInt inputRow,
															const Cube<Float> &inputWeightsCube,
															Matrix<Float> &inputWeightsPlane)
{
	inputWeightsPlane = inputWeightsCube.xyPlane(inputRow);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::transformAndWritePlaneOfData(	Int inputSpw,
																				uInt row,
																				Matrix<T> &inputDataPlane,
																				Matrix<Bool> &inputFlagsPlane,
																				Matrix<Float> &inputWeightsPlane,
																				Matrix<T> &outputDataPlane,
																				Matrix<Bool> &outputFlagsPlane,
																				ArrayColumn<T> &outputDataCol,
																				ArrayColumn<Bool> *outputFlagCol)
{
	// Get input number of correlations
	uInt nCorrs = inputDataPlane.shape()(0);

	// Get output plane shape
	IPosition outputPlaneShape = outputDataPlane.shape();

	// Initialize vectors
	Vector<T> inputDataStripe;
	Vector<Bool> inputFlagsStripe;
	Vector<Float> inputWeightsStripe;
	Vector<T> outputDataStripe;
	Vector<Bool> outputFlagsStripe;

	// Iterate correlation by correlation in order to extract a vector
	for (uInt corrIndex=0; corrIndex < nCorrs; corrIndex++)
	{
		// Fill input stripes by reference
		inputDataStripe.reference(inputDataPlane.row(corrIndex));
		inputFlagsStripe.reference(inputFlagsPlane.row(corrIndex));
		(*this.*setWeightStripeByReference_p)(corrIndex,inputWeightsPlane,inputWeightsStripe);

		// Fill output stripes by reference
		outputDataStripe.reference(outputDataPlane.row(corrIndex));
		outputFlagsStripe.reference(outputFlagsPlane.row(corrIndex));

		transformStripeOfData(inputSpw,inputDataStripe,inputFlagsStripe,
				inputWeightsStripe,outputDataStripe,outputFlagsStripe);
	}

	// Write output planes
	writeOutputPlanes(row,outputDataPlane,outputFlagsPlane,outputDataCol,*outputFlagCol);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::writeOutputPlanes(	uInt row,
												Matrix<Complex> &outputDataPlane,
												Matrix<Bool> &outputFlagsPlane,
												ArrayColumn<Complex> &outputDataCol,
												ArrayColumn<Bool> &outputFlagCol)
{
	(*this.*writeOutputPlanesComplex_p)(row,outputDataPlane,outputFlagsPlane,outputDataCol,outputFlagCol);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::writeOutputPlanes(	uInt row,
												Matrix<Float> &outputDataPlane,
												Matrix<Bool> &outputFlagsPlane,
												ArrayColumn<Float> &outputDataCol,
												ArrayColumn<Bool> &outputFlagCol)
{
	(*this.*writeOutputPlanesFloat_p)(row,outputDataPlane,outputFlagsPlane,outputDataCol,outputFlagCol);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::writeOutputPlanesInBlock(	uInt row,
																			Matrix<T> &outputDataPlane,
																			Matrix<Bool> &outputFlagsPlane,
																			ArrayColumn<T> &outputDataCol,
																			ArrayColumn<Bool> &outputFlagCol)
{
	IPosition outputPlaneShape = outputDataPlane.shape();
	outputDataCol.setShape(row,outputPlaneShape);
	outputDataCol.put(row, outputDataPlane);
	(*this.*writeOutputFlagsPlane_p)(outputFlagsPlane,outputFlagCol, outputPlaneShape, row);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::writeOutputFlagsPlane(	Matrix<Bool> &outputPlane,
													ArrayColumn<Bool> &outputCol,
													IPosition &outputPlaneShape,
													uInt &outputRow)
{
	outputCol.setShape(outputRow,outputPlaneShape);
	outputCol.put(outputRow, outputPlane);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::writeOutputPlanesInSlices(	uInt row,
																			Matrix<T> &outputDataPlane,
																			Matrix<Bool> &outputFlagsPlane,
																			ArrayColumn<T> &outputDataCol,
																			ArrayColumn<Bool> &outputFlagCol)
{
	IPosition outputPlaneShape = outputDataPlane.shape();
	uInt nCorrs = outputPlaneShape(0);
	IPosition outputPlaneShape_i(2,nCorrs,chansPerOutputSpw_p);
	Slice sliceX(0,nCorrs);

	uInt spw_i;
	uInt nspws = nspws_p-1;
	for (spw_i=0;spw_i<nspws;spw_i++)
	{
		uInt outRow = row+spw_i;
		Slice sliceY(chansPerOutputSpw_p*spw_i,chansPerOutputSpw_p);
		writeOutputPlaneSlices(outputDataPlane,outputDataCol,sliceX,sliceY,outputPlaneShape_i,outRow);
		(*this.*writeOutputFlagsPlaneSlices_p)(	outputFlagsPlane,outputFlagCol,
												sliceX,sliceY,outputPlaneShape_i,outRow);
	}

	uInt outRow = row+spw_i;
	Slice sliceY(chansPerOutputSpw_p*spw_i,tailOfChansforLastSpw_p);
	writeOutputPlaneReshapedSlices(outputDataPlane,outputDataCol,sliceX,sliceY,outputPlaneShape_i,outRow);
	(*this.*writeOutputFlagsPlaneReshapedSlices_p)(	outputFlagsPlane,outputFlagCol,
													sliceX,sliceY,outputPlaneShape_i,outRow);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::writeOutputFlagsPlaneSlices(	Matrix<Bool> &outputPlane,
															ArrayColumn<Bool> &outputCol,
															Slice &sliceX,
															Slice &sliceY,
															IPosition &outputPlaneShape,
															uInt &outputRow)
{
	writeOutputPlaneSlices(outputPlane,outputCol,sliceX,sliceY,outputPlaneShape,outputRow);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::writeOutputFlagsPlaneReshapedSlices(	Matrix<Bool> &outputPlane,
																	ArrayColumn<Bool> &outputCol,
																	Slice &sliceX,
																	Slice &sliceY,
																	IPosition &outputPlaneShape,
																	uInt &outputRow)
{
	writeOutputPlaneReshapedSlices(outputPlane,outputCol,sliceX,sliceY,outputPlaneShape,outputRow);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::writeOutputPlaneSlices(	Matrix<T> &outputPlane,
																		ArrayColumn<T> &outputCol,
																		Slice &sliceX,
																		Slice &sliceY,
																		IPosition &outputPlaneShape,
																		uInt &outputRow)
{
	Matrix<T> outputPlane_i = outputPlane(sliceX,sliceY);
	outputCol.setShape(outputRow,outputPlaneShape);
	outputCol.put(outputRow, outputPlane_i);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::writeOutputPlaneReshapedSlices(	Matrix<T> &outputPlane,
																				ArrayColumn<T> &outputCol,
																				Slice &sliceX,
																				Slice &sliceY,
																				IPosition &outputPlaneShape,
																				uInt &outputRow)
{
	Matrix<T> outputPlane_i = outputPlane(sliceX,sliceY);
	outputPlane_i.resize(outputPlaneShape,True);
	outputCol.setShape(outputRow,outputPlaneShape);
	outputCol.put(outputRow, outputPlane_i);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::setWeightStripeByReference(	uInt corrIndex,
															Matrix<Float> &inputWeightsPlane,
															Vector<Float> &inputWeightsStripe)
{
	inputWeightsStripe.reference(inputWeightsPlane.row(corrIndex));
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::transformStripeOfData(	Int inputSpw,
													Vector<Complex> &inputDataStripe,
													Vector<Bool> &inputFlagsStripe,
													Vector<Float> &inputWeightsStripe,
													Vector<Complex> &outputDataStripe,
													Vector<Bool> &outputFlagsStripe)
{
	(*this.*transformStripeOfDataComplex_p)(	inputSpw,inputDataStripe,inputFlagsStripe,
												inputWeightsStripe,outputDataStripe,outputFlagsStripe);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::transformStripeOfData(	Int inputSpw,
													Vector<Float> &inputDataStripe,
													Vector<Bool> &inputFlagsStripe,
													Vector<Float> &inputWeightsStripe,
													Vector<Float> &outputDataStripe,
													Vector<Bool> &outputFlagsStripe)
{
	(*this.*transformStripeOfDataFloat_p)(	inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe,
											outputDataStripe,outputFlagsStripe);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::average(	Int inputSpw,
															Vector<T> &inputDataStripe,
															Vector<Bool> &inputFlagsStripe,
															Vector<Float> &inputWeightsStripe,
															Vector<T> &outputDataStripe,
															Vector<Bool> &outputFlagsStripe)
{
	uInt width = freqbinMap_p[inputSpw];
	uInt startChan = 0;
	uInt outChanIndex = 0;
	uInt tail = inputDataStripe.size() % width;
	uInt limit = inputDataStripe.size() - tail;
	while (startChan < limit)
	{
		averageKernel(	inputDataStripe,inputFlagsStripe,inputWeightsStripe,
						outputDataStripe,outputFlagsStripe,startChan,outChanIndex,width);
		startChan += width;
		outChanIndex += 1;
	}

	// jagonzal: The last channel is dropped when there are not enough input channels
	//           to populate it only when there is no regridding afterwards
	if (tail and (outChanIndex <= outputDataStripe.size()-1) )
	{
		averageKernel(	inputDataStripe,inputFlagsStripe,inputWeightsStripe,
						outputDataStripe,outputFlagsStripe,startChan,outChanIndex,tail);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void  MSTransformManager::simpleAverage(	uInt width,
																Vector<T> &inputData,
																Vector<T> &outputData)
{
	// Dummy variables
	Vector<Bool> inputFlags,outputFlags;
	Vector<Float> inputWeights;

	uInt startChan = 0;
	uInt outChanIndex = 0;
	uInt tail = inputData.size() % width;
	uInt limit = inputData.size() - tail;
	while (startChan < limit)
	{
		simpleAverageKernel(inputData,inputFlags,inputWeights,outputData,outputFlags,startChan,outChanIndex,width);
		startChan += width;
		outChanIndex += 1;
	}

	// jagonzal: The last channel is dropped when there are not enough input channels
	//           to populate it only when there is no regridding afterwards
	if (tail and (outChanIndex <= outputData.size()-1) )
	{
		simpleAverageKernel(inputData,inputFlags,inputWeights,outputData,outputFlags,startChan,outChanIndex,tail);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::averageKernel(	Vector<Complex> &inputData,
											Vector<Bool> &inputFlags,
											Vector<Float> &inputWeights,
											Vector<Complex> &outputData,
											Vector<Bool> &outputFlags,
											uInt startInputPos,
											uInt outputPos,
											uInt width)
{
	(*this.*averageKernelComplex_p)(	inputData,inputFlags,inputWeights,
										outputData,outputFlags,startInputPos,outputPos,width);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::averageKernel(	Vector<Float> &inputData,
											Vector<Bool> &inputFlags,
											Vector<Float> &inputWeights,
											Vector<Float> &outputData,
											Vector<Bool> &outputFlags,
											uInt startInputPos,
											uInt outputPos,
											uInt width)
{
	(*this.*averageKernelFloat_p)(	inputData,inputFlags,inputWeights,
									outputData,outputFlags,startInputPos,outputPos,width);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::simpleAverageKernel(Vector<T> &inputData,
																	Vector<Bool> &inputFlags,
																	Vector<Float> &inputWeights,
																	Vector<T> &outputData,
																	Vector<Bool> &outputFlags,
																	uInt startInputPos,
																	uInt outputPos,
																	uInt width)
{
	uInt pos = startInputPos + 1;
	uInt counts = 1;
	T avg = inputData(startInputPos);
	while (pos < width)
	{
		avg += inputData(pos);
		counts += 1;
		pos += 1;
	}

	if (counts > 0)
	{
		avg /= counts;
	}

	outputData(outputPos) = avg;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::flagAverageKernel(	Vector<T> &inputData,
																	Vector<Bool> &inputFlags,
																	Vector<Float> &inputWeights,
																	Vector<T> &outputData,
																	Vector<Bool> &outputFlags,
																	uInt startInputPos,
																	uInt outputPos,
																	uInt width)
{
	uInt samples = 1;
	uInt pos = startInputPos + 1;
	uInt counts = !inputFlags(startInputPos);
	T avg = inputData(startInputPos)*(!inputFlags(startInputPos));
	while (samples < width)
	{
		avg += inputData(pos)*(!inputFlags(pos));
		counts += (!inputFlags(pos));
		samples += 1;
		pos += 1;
	}

	if (counts > 0)
	{
		avg /= counts;
	}
	else
	{
		outputFlags(outputPos) = True;
	}

	outputData(outputPos) = avg;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::weightAverageKernel(	Vector<T> &inputData,
																		Vector<Bool> &inputFlags,
																		Vector<Float> &inputWeights,
																		Vector<T> &outputData,
																		Vector<Bool> &outputFlags,
																		uInt startInputPos,
																		uInt outputPos,
																		uInt width)
{
	uInt samples = 1;
	uInt pos = startInputPos + 1;
	Float counts = inputWeights(startInputPos);
	T avg = inputData(startInputPos)*inputWeights(startInputPos);
	while (samples < width)
	{
		avg += inputData(pos)*inputWeights(pos);
		counts += inputWeights(pos);
		samples += 1;
		pos += 1;
	}

	if (counts > 0)
	{
		avg /= counts;
	}
	else
	{
		outputFlags(outputPos) = True;
	}

	outputData(outputPos) = avg;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::smooth(	Int inputSpw,
														Vector<T> &inputDataStripe,
														Vector<Bool> &inputFlagsStripe,
														Vector<Float> &inputWeightsStripe,
														Vector<T> &outputDataStripe,
														Vector<Bool> &outputFlagsStripe)
{
    Smooth<T>::hanning(	outputDataStripe, 		// the output data
    					outputFlagsStripe, 		// the output flags
    					inputDataStripe, 		// the input data
    					inputFlagsStripe, 		// the input flags
    					False);					// A good data point has its flag set to False

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::regrid(	Int inputSpw,
														Vector<T> &inputDataStripe,
														Vector<Bool> &inputFlagsStripe,
														Vector<Float> &inputWeightsStripe,
														Vector<T> &outputDataStripe,
														Vector<Bool> &outputFlagsStripe)
{

	regridCore(	inputSpw,
				inputDataStripe,
				inputFlagsStripe,
				inputWeightsStripe,
				outputDataStripe,
				outputFlagsStripe);

    /*
	ostringstream oss;
	oss.precision(20);
	oss << "inputDataStripe= " << inputDataStripe << endl;
	oss << "inputFlagsStripe= " << inputFlagsStripe << endl;
	oss << "inputFreq= " << inputOutputSpwMap_p[inputSpw].first.CHAN_FREQ_aux << endl;
	oss << "outputFreq= " << inputOutputSpwMap_p[inputSpw].second.CHAN_FREQ << endl;
	oss << "outputDataStripe= " << outputDataStripe << endl;
	oss << "outputFlagsStripe= " << outputFlagsStripe << endl;
	logger_p << LogIO::NORMAL << oss.str() << LogIO::POST;
	*/

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::regridCore(	Int inputSpw,
											Vector<Complex> &inputDataStripe,
											Vector<Bool> &inputFlagsStripe,
											Vector<Float> &inputWeightsStripe,
											Vector<Complex> &outputDataStripe,
											Vector<Bool> &outputFlagsStripe)
{

	(*this.*regridCoreComplex_p)(	inputSpw,
									inputDataStripe,
									inputFlagsStripe,
									inputWeightsStripe,
									outputDataStripe,
									outputFlagsStripe);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::regridCore(	Int inputSpw,
											Vector<Float> &inputDataStripe,
											Vector<Bool> &inputFlagsStripe,
											Vector<Float> &inputWeightsStripe,
											Vector<Float> &outputDataStripe,
											Vector<Bool> &outputFlagsStripe)
{
	(*this.*regridCoreFloat_p)(	inputSpw,
								inputDataStripe,
								inputFlagsStripe,
								inputWeightsStripe,
								outputDataStripe,
								outputFlagsStripe);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::fftshift(	Int inputSpw,
										Vector<Complex> &inputDataStripe,
										Vector<Bool> &inputFlagsStripe,
										Vector<Float> &inputWeightsStripe,
										Vector<Complex> &outputDataStripe,
										Vector<Bool> &outputFlagsStripe)
{
	fFFTServer_p.fftshift(outputDataStripe,
    					outputFlagsStripe,
    					(const Vector<Complex>)inputDataStripe,
    					(const Vector<Bool>)inputFlagsStripe,
    					(const uInt)0, // In vectors axis 0 is the only dimension
    					(const Double)fftShift_p,
    					MSTransformations::False, // A good data point has its flag set to False
    					MSTransformations::False);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformManager::fftshift(	Int inputSpw,
										Vector<Float> &inputDataStripe,
										Vector<Bool> &inputFlagsStripe,
										Vector<Float> &inputWeightsStripe,
										Vector<Float> &outputDataStripe,
										Vector<Bool> &outputFlagsStripe)
{
    fFFTServer_p.fftshift(outputDataStripe,
    					outputFlagsStripe,
    					(const Vector<Float>)inputDataStripe,
    					(const Vector<Bool>)inputFlagsStripe,
    					(const uInt)0, // In vectors axis 0 is the only dimension
    					(const Double)fftShift_p,
    					MSTransformations::False); // A good data point has its flag set to False
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::interpol1D(	Int inputSpw,
															Vector<T> &inputDataStripe,
															Vector<Bool> &inputFlagsStripe,
															Vector<Float> &inputWeightsStripe,
															Vector<T> &outputDataStripe,
															Vector<Bool> &outputFlagsStripe)
{
	InterpolateArray1D<Double,T>::interpolate(	outputDataStripe, // Output data
	    										outputFlagsStripe, // Output flags
	    										inputOutputSpwMap_p[inputSpw].second.CHAN_FREQ, // Out chan freq
	    										inputOutputSpwMap_p[inputSpw].first.CHAN_FREQ_aux, // In chan freq
	    										inputDataStripe, // Input data
	    										inputFlagsStripe, // Input Flags
	    										interpolationMethod_p, // Interpolation method
	    										False, // A good data point has its flag set to False
	    										False // If False extrapolated data points are set flagged
							    				);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::interpol1Dfftshift(	Int inputSpw,
																	Vector<T> &inputDataStripe,
																	Vector<Bool> &inputFlagsStripe,
																	Vector<Float> &inputWeightsStripe,
																	Vector<T> &outputDataStripe,
																	Vector<Bool> &outputFlagsStripe)
{
	Vector<T> regriddedDataStripe(inputDataStripe.shape(),T());
	Vector<Bool> regriddedFlagsStripe(inputFlagsStripe.shape(),MSTransformations::False);

	// This linear interpolation provides an uniform grid (pre-condition to apply fftshift)
	interpol1D(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe,regriddedDataStripe,regriddedFlagsStripe);

	// fftshift takes care of time
	fftshift(inputSpw,regriddedDataStripe,regriddedFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::averageSmooth(	Int inputSpw,
																Vector<T> &inputDataStripe,
																Vector<Bool> &inputFlagsStripe,
																Vector<Float> &inputWeightsStripe,
																Vector<T> &outputDataStripe,
																Vector<Bool> &outputFlagsStripe)
{
	Vector<T> averagedDataStripe(outputDataStripe.shape(),T());
	Vector<Bool> averagedFlagsStripe(outputFlagsStripe.shape(),MSTransformations::False);

	average(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe, averagedDataStripe,averagedFlagsStripe);

	smooth(inputSpw,averagedDataStripe,averagedFlagsStripe, inputWeightsStripe, outputDataStripe,outputFlagsStripe);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::averageRegrid(	Int inputSpw,
																Vector<T> &inputDataStripe,
																Vector<Bool> &inputFlagsStripe,
																Vector<Float> &inputWeightsStripe,
																Vector<T> &outputDataStripe,
																Vector<Bool> &outputFlagsStripe)
{
	Vector<T> averagedDataStripe(numOfCombInterChanMap_p[inputSpw],T());
	Vector<Bool> averagedFlagsStripe(numOfCombInterChanMap_p[inputSpw],MSTransformations::False);

	average(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe, averagedDataStripe,averagedFlagsStripe);

	regrid(inputSpw,averagedDataStripe,averagedFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::smoothRegrid(	Int inputSpw,
																Vector<T> &inputDataStripe,
																Vector<Bool> &inputFlagsStripe,
																Vector<Float> &inputWeightsStripe,
																Vector<T> &outputDataStripe,
																Vector<Bool> &outputFlagsStripe)
{
	Vector<T> smoothedDataStripe(inputDataStripe.shape(),T());
	Vector<Bool> smoothedFlagsStripe(inputFlagsStripe.shape(),MSTransformations::False);

	smooth(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe,smoothedDataStripe,smoothedFlagsStripe);

	regrid(inputSpw,smoothedDataStripe,smoothedFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MSTransformManager::averageSmoothRegrid(Int inputSpw,
																	Vector<T> &inputDataStripe,
																	Vector<Bool> &inputFlagsStripe,
																	Vector<Float> &inputWeightsStripe,
																	Vector<T> &outputDataStripe,
																	Vector<Bool> &outputFlagsStripe)
{
	Vector<T> averageSmoothedDataStripe(numOfCombInterChanMap_p[inputSpw],T());
	Vector<Bool> averageSmoothedFlagsStripe(numOfCombInterChanMap_p[inputSpw],MSTransformations::False);

	averageSmooth(	inputSpw,inputDataStripe,inputFlagsStripe,
					inputWeightsStripe,averageSmoothedDataStripe,averageSmoothedFlagsStripe);

	regrid(	inputSpw,averageSmoothedDataStripe,averageSmoothedFlagsStripe,
			inputWeightsStripe,outputDataStripe,outputFlagsStripe);

	return;
}
template Bool MSTransformManager::transformNotReindexableVector(	const Vector<double> &inputVector, Vector<double> &outputVector, Bool constant);
template Bool MSTransformManager::transformNotReindexableVector(	const Vector<int> &inputVector, Vector<int> &outputVector, Bool constant);

} //# NAMESPACE CASA - END
