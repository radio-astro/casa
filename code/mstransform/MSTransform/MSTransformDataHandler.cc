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
	    spline
	  };
}

/////////////////////////////////////////////
/// MSTransformDataHandler implementation ///
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
	if (channelSelector_p) delete channelSelector_p;
	if (visibilityIterator_p) delete visibilityIterator_p;
	if (splitter_p) delete splitter_p;
	if (phaseCenterPar_p) delete phaseCenterPar_p;

	return;
}

// -----------------------------------------------------------------------
// Method to initialize members to default values
// -----------------------------------------------------------------------
void MSTransformDataHandler::initialize()
{
	// MS specification parameters
	inpMsName_p = String("");
	outMsName_p = String("");
	datacolumn_p = String("CORRECTED");
	tileShape_p.resize(1,False);
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
	outputInputSPWIndexMap_p.clear();

	// Frequency transformation parameters
	ddiStart_p = 0;
	combinespws_p = False;
	channelAverage_p = False;
	hanningSmooth_p = False;
	refFrameTransformation_p = False;
	freqbin_p = Vector<Int>(1,-1);
	useweights_p = "flags";
	interpolationMethodPar_p = String("linear");	// Options are: nearest, linear, cubic, spline, fftshift
	phaseCenterPar_p = new casac::variant("");
	restFrequency_p = String("");
	outputReferenceFramePar_p = String("");			// Options are: LSRK, LSRD, BARY, GALACTO, LGROUP, CMB, GEO, or TOPO

	// Frequency specification parameters
	mode_p = String("channel"); 					// Options are: channel, frequency, velocity
	start_p = String("0");
	width_p = String("1");
	nChan_p = -1;									// -1 means use all the input channels
	velocityType_p = String("radio");				// When mode is velocity options are: optical, radio

	// Time transformation parameters
	timeBin_p = 0.0;
	timespan_p = String("");

	// MS-related members
	splitter_p = NULL;
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
	fillWeightSpectrum_p = False;
	correctedToData_p = False;
	dataColMap_p.clear();
	mainColumn_p = MS::CORRECTED_DATA;

	// Frequency transformation members
	baselineMap_p.clear();
	rowIndex_p.clear();
	spwChannelMap_p.clear();
	inputOutputSpwMap_p.clear();
	freqbinMap_p.clear();
	numOfInpChanMap_p.clear();
	numOfSelChanMap_p.clear();
	numOfOutChanMap_p.clear();
	numOfCombInputChanMap_p.clear();
	numOfCombInterChanMap_p.clear();
	transformStripeOfDataComplex_p = NULL;
	transformStripeOfDataFloat_p = NULL;
	transformCubeOfDataComplex_p = NULL;
	transformCubeOfDataFloat_p = NULL;
	averageKernelComplex_p = NULL;
	fillWeightsPlane_p = NULL;
	setWeightsPlaneByReference_p = NULL;
	setWeightStripeByReference_p = NULL;

	return;
}

// -----------------------------------------------------------------------
// Method to parse the configuration parameters
// -----------------------------------------------------------------------
void MSTransformDataHandler::configure(Record &configuration)
{
	parseMsSpecParams(configuration);
	parseDataSelParams(configuration);
	parseFreqTransParams(configuration);
	parseChanAvgParams(configuration);
	parseRefFrameTransParams(configuration);

	return;
}

// -----------------------------------------------------------------------
// Method to parse input/output MS specification
// -----------------------------------------------------------------------
void MSTransformDataHandler::parseMsSpecParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("inputms");
	if (exists >= 0)
	{
		configuration.get (exists, inpMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Input file name is " << inpMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("outputms");
	if (exists >= 0)
	{
		configuration.get (exists, outMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Output file name is " << outMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("datacolumn");
	if (exists >= 0)
	{
		configuration.get (exists, datacolumn_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Data column is " << datacolumn_p << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to parse the data selection parameters
// -----------------------------------------------------------------------
void MSTransformDataHandler::parseDataSelParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("array");
	if (exists >= 0)
	{
		configuration.get (exists, arraySelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "array selection is " << arraySelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("field");
	if (exists >= 0)
	{
		configuration.get (exists, fieldSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "field selection is " << fieldSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("scan");
	if (exists >= 0)
	{
		configuration.get (exists, scanSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "scan selection is " << scanSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("timerange");
	if (exists >= 0)
	{
		configuration.get (exists, timeSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "timerange selection is " << timeSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("spw");
	if (exists >= 0)
	{
		configuration.get (exists, spwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "spw selection is " << spwSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("antenna");
	if (exists >= 0)
	{
		configuration.get (exists, baselineSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "antenna selection is " << baselineSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("uvrange");
	if (exists >= 0)
	{
		configuration.get (exists, uvwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "uvrange selection is " << uvwSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("correlation");
	if (exists >= 0)
	{
		configuration.get (exists, polarizationSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "correlation selection is " << polarizationSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("observation");
	if (exists >= 0)
	{
		configuration.get (exists, observationSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<"observation selection is " << observationSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("intent");
	if (exists >= 0)
	{
		configuration.get (exists, scanIntentSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "scan intent selection is " << scanIntentSelection_p << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to parse the channel average parameters
// -----------------------------------------------------------------------
void MSTransformDataHandler::parseChanAvgParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("freqaverage");
	if (exists >= 0)
	{
		configuration.get (exists, channelAverage_p);
		if (channelAverage_p)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Frequency average is activated" << LogIO::POST;
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

	exists = configuration.fieldNumber ("freqbin");
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
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Wrong format for freqbin parameter (only Int and arrayInt are supported) " << LogIO::POST;
		}

		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Frequency bin is " << freqbin_p << LogIO::POST;
	}
	else
	{
		logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Frequency average is activated but no freqbin parameter is provided " << LogIO::POST;
	}

	exists = configuration.fieldNumber ("useweights");
	if (exists >= 0)
	{
		configuration.get (exists, useweights_p);

		useweights_p.downcase();

		if (useweights_p == "flags")
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Using FLAGS as weights for frequency average" << LogIO::POST;
		}
		else if (useweights_p == "weights")
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Using WEIGHT_SPECTRUM as weights for frequency average" << LogIO::POST;
		}
		else
		{
			useweights_p = String("flags");
		}
	}


	return;
}

// -----------------------------------------------------------------------
// Method to parse the frequency transformation parameters
// -----------------------------------------------------------------------
void MSTransformDataHandler::parseFreqTransParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("combinespws");
	if (exists >= 0)
	{
		configuration.get (exists, combinespws_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Combine Spectral Windows is " << combinespws_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("ddistart");
	if (exists >= 0)
	{
		configuration.get (exists, ddiStart_p);
		if (ddiStart_p > 0)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "DDI start is " << ddiStart_p << LogIO::POST;
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
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Hanning Smooth is " << hanningSmooth_p << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to parse the reference frame transformation parameters
// -----------------------------------------------------------------------
void MSTransformDataHandler::parseRefFrameTransParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("regridms");
	if (exists >= 0)
	{
		configuration.get (exists, refFrameTransformation_p);

		if (refFrameTransformation_p)
		{
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Regrid MS is activated"<< LogIO::POST;
		}
		else
		{
			return;
		}
	}

	exists = configuration.fieldNumber ("phasecenter");
	if (exists >= 0)
	{
		//If phase center is a simple numeric value then it is taken as a FIELD_ID otherwise it is converted to a MDirection
        if( configuration.type(exists) == TpInt )
        {
        	int fieldIdForPhaseCenter = -1;
    		configuration.get (exists, fieldIdForPhaseCenter);
    		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Field Id for phase center is " << fieldIdForPhaseCenter << LogIO::POST;
    		if (phaseCenterPar_p) delete phaseCenterPar_p;
    		phaseCenterPar_p = new casac::variant(fieldIdForPhaseCenter);
        }
        else
        {
        	String phaseCenter("");
    		configuration.get (exists, phaseCenter);
    		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Phase center is " << phaseCenter << LogIO::POST;
    		if (phaseCenterPar_p) delete phaseCenterPar_p;
    		phaseCenterPar_p = new casac::variant(phaseCenter);
        }
	}

	exists = configuration.fieldNumber ("restfreq");
	if (exists >= 0)
	{
		configuration.get (exists, restFrequency_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Rest frequency is " << restFrequency_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("outframe");
	if (exists >= 0)
	{
		configuration.get (exists, outputReferenceFramePar_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Output reference frame is " << outputReferenceFramePar_p << LogIO::POST;
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
		else
		{
			logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Interpolation method " << interpolationMethodPar_p  << " not available " << LogIO::POST;
		}

		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Interpolation method is " << interpolationMethodPar_p  << LogIO::POST;
	}
	else
	{
		interpolationMethod_p = MSTransformations::linear;
	}

	parseFreqSpecParams(configuration);

	return;
}

// -----------------------------------------------------------------------
// Method to parse the frequency selection specification
// -----------------------------------------------------------------------
void MSTransformDataHandler::parseFreqSpecParams(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("mode");
	if (exists >= 0)
	{
		configuration.get (exists, mode_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Mode is " << mode_p<< LogIO::POST;

		if ((mode_p == "frequency") or (mode_p == "channel"))
		{
			start_p = String("");
			width_p = String("");
		}
	}

	exists = configuration.fieldNumber ("nchan");
	if (exists >= 0)
	{
		configuration.get (exists, nChan_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Number of channels is " << nChan_p<< LogIO::POST;
	}

	exists = configuration.fieldNumber ("start");
	if (exists >= 0)
	{
		configuration.get (exists, start_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Start is " << start_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("width");
	if (exists >= 0)
	{
		configuration.get (exists, width_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Width is " << width_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("veltype");
	if (exists >= 0)
	{
		configuration.get (exists, velocityType_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Velocity type is " << velocityType_p << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to open the input MS, select the data and create the
// structure of the output MS filling the auxiliary tables.
// -----------------------------------------------------------------------
void MSTransformDataHandler::open()
{
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Select data" << LogIO::POST;

	splitter_p = new SubMS(inpMsName_p,Table::Old);

	// WARNING: Input MS is re-set at the end of a successful SubMS::makeSubMS call, therefore we have to use the selected MS always
	inputMs_p = &(splitter_p->ms_p);

	// Once the input MS is opened we can get the selection indexes, in this way we also validate the selection parameters
	initDataSelectionParams();
	getInputNumberOfChannels();

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

	// Set-up SubMS object
	const String dummyExpr = String("");
	splitter_p->setmsselect((const String)spwSelection_p,
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

	splitter_p->selectTime(timeBin_p,timeSelection_p);

	// Create output MS structure
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Create output MS structure" << LogIO::POST;

	Bool selectionOk = False;
	splitter_p->fillMainTable_p = False;
	selectionOk = splitter_p->makeSubMS(outMsName_p,datacolumn_p,tileShape_p,timespan_p);
	if (!selectionOk)
	{
		logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Combination of selection ranges produces a NullSelection" << LogIO::POST;
		throw(MSSelectionNullSelection("MSSelectionNullSelection : The selected table has zero rows."));
	}

	selectedInputMs_p = &(splitter_p->mssel_p);
	outputMs_p = &(splitter_p->msOut_p);
	selectedInputMsCols_p = splitter_p->mscIn_p;
	outputMsCols_p = splitter_p->msc_p;

	return;
}

// -----------------------------------------------------------------------
// Method to close the output MS
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
// Check configuration and input MS characteristics to determine run parameters
// -----------------------------------------------------------------------
void MSTransformDataHandler::setup()
{
	// Check if we really need to combine SPWs
	if (combinespws_p)
	{
		uInt nInputSpws = outputMs_p->spectralWindow().nrow();
		if (nInputSpws < 2)
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "There is only one selected SPW, no need to combine " << LogIO::POST;
			combinespws_p = False;
		}
	}

	//// Determine the frequency transformation methods to use ////


	// Cube level
	if (combinespws_p)
	{
		transformCubeOfDataComplex_p = &MSTransformDataHandler::combineCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformDataHandler::combineCubeOfData;
	}
	else if (refFrameTransformation_p)
	{
		transformCubeOfDataComplex_p = &MSTransformDataHandler::regridCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformDataHandler::regridCubeOfData;
	}
	else if (channelAverage_p)
	{
		transformCubeOfDataComplex_p = &MSTransformDataHandler::averageCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformDataHandler::averageCubeOfData;
	}
	else if (hanningSmooth_p)
	{
		transformCubeOfDataComplex_p = &MSTransformDataHandler::smoothCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformDataHandler::smoothCubeOfData;
	}
	else
	{
		transformCubeOfDataComplex_p = &MSTransformDataHandler::copyCubeOfData;
		transformCubeOfDataFloat_p = &MSTransformDataHandler::copyCubeOfData;
	}

	Bool spectralRegridding = combinespws_p or refFrameTransformation_p;

	// Vector level
	if (channelAverage_p and !hanningSmooth_p and !spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformDataHandler::average;
		transformStripeOfDataFloat_p = &MSTransformDataHandler::average;
	}
	else if (!channelAverage_p and hanningSmooth_p and !spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformDataHandler::smooth;
		transformStripeOfDataFloat_p = &MSTransformDataHandler::smooth;
	}
	else if (!channelAverage_p and !hanningSmooth_p and spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformDataHandler::regrid;
		transformStripeOfDataFloat_p = &MSTransformDataHandler::regrid;
	}
	else if (channelAverage_p and hanningSmooth_p and !spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformDataHandler::averageSmooth;
		transformStripeOfDataFloat_p = &MSTransformDataHandler::averageSmooth;
	}
	else if (channelAverage_p and !hanningSmooth_p and spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformDataHandler::averageRegrid;
		transformStripeOfDataFloat_p = &MSTransformDataHandler::averageRegrid;
	}
	else if (!channelAverage_p and hanningSmooth_p and spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformDataHandler::smoothRegrid;
		transformStripeOfDataFloat_p = &MSTransformDataHandler::smoothRegrid;
	}
	else if (channelAverage_p and hanningSmooth_p and spectralRegridding)
	{
		transformStripeOfDataComplex_p = &MSTransformDataHandler::averageSmoothRegrid;
		transformStripeOfDataFloat_p = &MSTransformDataHandler::averageSmoothRegrid;
	}

	// Averaging kernel
	if ((useweights_p == "weights") and (fillWeightSpectrum_p))
	{
		averageKernelComplex_p = &MSTransformDataHandler::weightAverageKernel;
		averageKernelFloat_p = &MSTransformDataHandler::weightAverageKernel;

		fillWeightsPlane_p = &MSTransformDataHandler::fillWeightsPlane;
		setWeightsPlaneByReference_p = &MSTransformDataHandler::setWeightsPlaneByReference;
		setWeightStripeByReference_p = &MSTransformDataHandler::setWeightStripeByReference;
	}
	else
	{
		averageKernelComplex_p = &MSTransformDataHandler::flagAverageKernel;
		averageKernelFloat_p = &MSTransformDataHandler::flagAverageKernel;

		fillWeightsPlane_p = &MSTransformDataHandler::dontfillWeightsPlane;
		setWeightsPlaneByReference_p = &MSTransformDataHandler::dontsetWeightsPlaneByReference;
		setWeightStripeByReference_p = &MSTransformDataHandler::dontSetWeightStripeByReference;

		if (useweights_p == "weights")
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Requested column for weighted channel average WEIGHT_SPECTRUM not present in input MS" << LogIO::POST;
		}
	}

	//// Determine the frequency transformation methods to use ////


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

	// Determine weight and sigma factors
	getOutputNumberOfChannels();
	calculateWeightAndSigmaFactors();

	// Check what columns have to filled
	checkFillFlagCategory();
	checkDataColumnsToFill();
	checkFillWeightSpectrum();

	// Generate Iterator
	setIterationApproach();
	generateIterator();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::initDataSelectionParams()
{
	MSSelection mssel;

	if (!observationSelection_p.empty())
	{
		mssel.setObservationExpr(observationSelection_p);
		Vector<Int> observationList = mssel.getObservationList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Selected Observations Ids are " << observationList << LogIO::POST;

		for (uInt index=0; index < observationList.size(); index++)
		{
			inputOutputObservationIndexMap_p[observationList(index)] = index;
		}
	}

	if (!arraySelection_p.empty())
	{
		mssel.setArrayExpr(arraySelection_p);
		Vector<Int> arrayList = mssel.getSubArrayList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Selected Arrays Ids are " << arrayList << LogIO::POST;

		for (uInt index=0; index < arrayList.size(); index++)
		{
			inputOutputArrayIndexMap_p[arrayList(index)] = index;
		}
	}

	if (!scanSelection_p.empty())
	{
		mssel.setScanExpr(scanSelection_p);
		Vector<Int> scanList = mssel.getScanList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Selected Scans Ids are " << scanList << LogIO::POST;

		for (uInt index=0; index < scanList.size(); index++)
		{
			inputOutputScanIndexMap_p[scanList(index)] = index;
		}
	}

	if (!scanIntentSelection_p.empty())
	{
		mssel.setStateExpr(scanIntentSelection_p);
		Vector<Int> scanIntentList = mssel.getStateObsModeList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Selected Scans Intents Ids are " << scanIntentList << LogIO::POST;

		for (uInt index=0; index < scanIntentList.size(); index++)
		{
			inputOutputScanIntentIndexMap_p[scanIntentList(index)] = index;
		}
	}

	if (!fieldSelection_p.empty())
	{
		mssel.setFieldExpr(fieldSelection_p);
		Vector<Int> fieldList = mssel.getFieldList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Selected Fields Ids are " << fieldList << LogIO::POST;

		for (uInt index=0; index < fieldList.size(); index++)
		{
			inputOutputFieldIndexMap_p[fieldList(index)] = index;
		}
	}

	if (!spwSelection_p.empty())
	{
		mssel.setSpwExpr(spwSelection_p);
		Matrix<Int> spwchan = mssel.getChanList(inputMs_p);
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Selected SPWs Ids are " << spwchan << LogIO::POST;

	    IPosition shape = spwchan.shape();
	    uInt nSelections = shape[0];
		Int spw,channelStart,channelStop,channelStep,channelWidth;
		if (channelSelector_p == NULL) channelSelector_p = new vi::FrequencySelectionUsingChannels();
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
				logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Number of frequency bins ( "
						<< freqbin_p.size() << " ) different from number of selected spws ( " << spwList.size() << " )" << LogIO::POST;
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
		logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Selected correlations are " << correlationSlices << LogIO::POST;

		if (channelSelector_p == NULL) channelSelector_p = new vi::FrequencySelectionUsingChannels();

		channelSelector_p->addCorrelationSlices(correlationSlices);
	}

	return;
}

// -----------------------------------------------------------------------
// Method to initialize the reference frame transformation parameters
// -----------------------------------------------------------------------
void MSTransformDataHandler::initRefFrameTransParams()
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
    	logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Problem parsing output reference frame:" << outputReferenceFramePar_p  << LogIO::POST;
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
    		logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Selected FIELD_ID to determine phase center does not exists " << LogIO::POST;
    	}
    	else
    	{
    		MSFieldColumns fieldCols(fieldTable);
    		phaseCenter_p = fieldCols.phaseDirMeasCol()(fieldIdForPhaseCenter)(IPosition(1,0));
    	}
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
    	}
    	// Parse phase center
    	else
    	{
        	if(!casaMDirection(phaseCenter, phaseCenter_p))
        	{
        		logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Cannot interpret phase center " << phaseCenter << LogIO::POST;
        		return;
        	}
    	}
    }

    // Determine observation time from the first row in the selected MS
    observationTime_p = selectedInputMsCols_p->timeMeas()(0);

	return;
}

// -----------------------------------------------------------------------
// Method to re-grid each SPW separately in the SPW sub-table
// It also sets the input/output frequency arrays to be used by the interpolations
// -----------------------------------------------------------------------
void MSTransformDataHandler::regridSpwSubTable()
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

    	logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Regridding SPW with Id " <<  spwId << LogIO::POST;

    	// Get input frequencies and widths
    	Vector<Double> inputChanFreq(chanFreqCol(spw_idx));
    	Vector<Double> inputChanWidth(chanWidthCol(spw_idx));

        // Print characteristics of input SPW
        ostringstream oss;
        oss 	<< "Input SPW: " << std::setw(5) << inputChanFreq.size()
        		<< " channels, first channel = " << std::setprecision(9) << std::setw(14) << std::scientific << inputChanFreq(0) << " Hz"
        		<< ", last channel = " << std::setprecision(9) << std::setw(14) << std::scientific << inputChanFreq(inputChanFreq.size() -1) << " Hz";
        logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << oss.str() << LogIO::POST;

        // Create input SPW structure
        spwInfo inputSpw;
    	if (channelAverage_p)
    	{
    		Vector<Double> intermediateChanFreq;
    		Vector<Double> intermediateChanWidth;
    		calculateIntermediateFrequencies(spwId,inputChanFreq,inputChanWidth,intermediateChanFreq,intermediateChanWidth);
        	inputSpw = spwInfo(intermediateChanFreq,intermediateChanWidth);

            oss.str("");
            oss.clear();
            oss 	<< "Averaged SPW: " << std::setw(5) << intermediateChanWidth.size()
            		<< " channels, first channel = " << std::setprecision(9) << std::setw(14) << std::scientific << intermediateChanFreq(0) << " Hz"
            		<< ", last channel = " << std::setprecision(9) << std::setw(14) << std::scientific << intermediateChanFreq(intermediateChanWidth.size() -1) << " Hz";
            logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << oss.str() << LogIO::POST;
    	}
    	else
    	{
    		numOfCombInputChanMap_p[spwId] = inputChanFreq.size();
    		inputSpw = spwInfo(inputChanFreq,inputChanWidth);
    	}

        // Calculate output channels and widths
        Vector<Double> regriddedCHAN_FREQ;
        Vector<Double> regriddedCHAN_WIDTH;
    	SubMS::calcChanFreqs(	logger_p,
    							regriddedCHAN_FREQ,
    							regriddedCHAN_WIDTH,
    							inputChanFreq,
    							inputChanWidth,
    							phaseCenter_p,
    							inputReferenceFrame_p,
    							observationTime_p,
    							observatoryPosition_p,
    							mode_p,
    							nChan_p,
    							start_p,
    							width_p,
    							restFrequency_p,
    							outputReferenceFramePar_p,
    							velocityType_p,
    							False // verbose
            				);

        // Create output SPW structure
        spwInfo outputSpw = spwInfo(regriddedCHAN_FREQ,regriddedCHAN_WIDTH);

        // Set the output SPW characteristics in the SPW sub-table
        numChanCol.put(spw_idx,outputSpw.NUM_CHAN);
        chanFreqCol.put(spw_idx, outputSpw.CHAN_FREQ);
        chanWidthCol.put(spw_idx,  outputSpw.CHAN_WIDTH);
        effectiveBWCol.put(spw_idx, outputSpw.EFFECTIVE_BW);
        resolutionCol.put(spw_idx, outputSpw.RESOLUTION);
        refFrequencyCol.put(spw_idx,outputSpw.REF_FREQUENCY);
        totalBandwidthCol.put(spw_idx,outputSpw.TOTAL_BANDWIDTH);

        // Print characteristics of output SPW
        oss.str("");
        oss.clear();
        oss 	<< "Output SPW: " << std::setw(5) << outputSpw.NUM_CHAN
        		<< " channels, first channel = " << std::setprecision(9) << std::setw(14) << std::scientific << outputSpw.CHAN_FREQ(0) << " Hz"
        		<< ", last channel = " << std::setprecision(9) << std::setw(14) << std::scientific << outputSpw.CHAN_FREQ(outputSpw.NUM_CHAN -1) << " Hz";
        logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << oss.str() << LogIO::POST;

        // Add input-output SPW pair to map
    	inputOutputSpwMap_p[spwId] = std::make_pair(inputSpw,outputSpw);
    }

    // Flush changes
    outputMs_p->flush(True);

	return;
}

// -----------------------------------------------------------------------
// Method to combine and re-grid the SPW sub-table
// It also sets the input/output frequency arrays to be used by the interpolations
// -----------------------------------------------------------------------
void MSTransformDataHandler::regridAndCombineSpwSubtable()
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

    // Map unsorted input channels
    spwChannelMap_p.clear();
    Vector<Double> inputFrequencies(inputChannels.size(),0);
    Vector<Double> inputWidths(inputChannels.size(),0);
    for (uInt inputChannels_idx=0; inputChannels_idx<inputChannels.size(); inputChannels_idx++)
    {
    	inputChannels.at(inputChannels_idx).outChannel = inputChannels_idx;
    	spwChannelMap_p[inputChannels.at(inputChannels_idx).SPW_id][inputChannels.at(inputChannels_idx).inpChannel] = inputChannels_idx;
    	inputFrequencies(inputChannels_idx) = inputChannels.at(inputChannels_idx).CHAN_FREQ;
    	inputWidths(inputChannels_idx) = inputChannels.at(inputChannels_idx).CHAN_WIDTH;
    }

    // Print characteristics of input SPW
    ostringstream oss;
    oss 	<< "Input SPW: " << std::setw(5) << inputFrequencies.size()
    		<< " channels, first channel = " << std::setprecision(9) << std::setw(14) << std::scientific << inputFrequencies(0) << " Hz"
    		<< ", last channel = " << std::setprecision(9) << std::setw(14) << std::scientific << inputFrequencies(inputFrequencies.size()-1) << " Hz";
    logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << oss.str() << LogIO::POST;

    // Create input SPW structure
    spwInfo inputSpw;
	if (channelAverage_p)
	{
		Vector<Double> intermediateChanFreq;
		Vector<Double> intermediateChanWidth;
		calculateIntermediateFrequencies(0,inputFrequencies,inputWidths,intermediateChanFreq,intermediateChanWidth);
    	inputSpw = spwInfo(intermediateChanFreq,intermediateChanWidth);

        oss.str("");
        oss.clear();
        oss 	<< "Averaged SPW: " << std::setw(5) << intermediateChanWidth.size()
        		<< " channels, first channel = " << std::setprecision(9) << std::setw(14) << std::scientific << intermediateChanFreq(0) << " Hz"
        		<< ", last channel = " << std::setprecision(9) << std::setw(14) << std::scientific << intermediateChanFreq(intermediateChanWidth.size() -1) << " Hz";
        logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << oss.str() << LogIO::POST;
	}
	else
	{
		numOfCombInputChanMap_p[0] = inputFrequencies.size();
		inputSpw = spwInfo(inputFrequencies,inputWidths);
	}

    /// Determine output SPW structure ///////////////////

	// Determine combined SPWs
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Calculate combined SPW frequencies" << LogIO::POST;
	SubMS combiner(*outputMs_p);
    Vector<Double> combinedCHAN_FREQ;
    Vector<Double> combinedCHAN_WIDTH;
	combiner.combineSpws(Vector<Int>(1,-1),True,combinedCHAN_FREQ,combinedCHAN_WIDTH,True);

	/*
	// Debugging info
    ostringstream debug_oss;
    debug_oss <<  " phaseCenter_p=" << phaseCenter_p << endl;
    debug_oss <<  " inputReferenceFrame_p=" << inputReferenceFrame_p << endl;
    debug_oss <<  " observationTime_p=" << observationTime_p << endl;
    debug_oss <<  " observatoryPosition_p=" << observatoryPosition_p << endl;
    debug_oss <<  " mode_p=" << mode_p << endl;
    debug_oss <<  " nChan_p=" << nChan_p << endl;
    debug_oss <<  " start_p=" << start_p << endl;
    debug_oss <<  " width_p=" << width_p << endl;
    debug_oss <<  " restFrequency_p=" << restFrequency_p << endl;
    debug_oss <<  " outputReferenceFramePar_p=" << outputReferenceFramePar_p << endl;
    debug_oss <<  " velocityType_p=" << velocityType_p << endl;
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << debug_oss.str() << LogIO::POST;
	*/

	// Re-grid the output SPW to be uniform and change reference frame
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Calculate frequencies in output reference frame " << LogIO::POST;
    Vector<Double> regriddedCHAN_FREQ;
    Vector<Double> regriddedCHAN_WIDTH;
    SubMS::calcChanFreqs(	logger_p,
    						regriddedCHAN_FREQ,
    						regriddedCHAN_WIDTH,
    						combinedCHAN_FREQ,
    						combinedCHAN_WIDTH,
    						phaseCenter_p,
    						inputReferenceFrame_p,
    						observationTime_p,
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

	// Create output SPW structure
	spwInfo outputSpw = spwInfo(regriddedCHAN_FREQ,regriddedCHAN_WIDTH);

    // Print characteristics of output SPW
    oss.str("");
    oss.clear();
    oss 	<< "Output SPW: " << std::setw(5) << outputSpw.NUM_CHAN
    		<< " channels, first channel = " << std::setprecision(9) << std::setw(14) << std::scientific << outputSpw.CHAN_FREQ(0) << " Hz"
    		<< ", last channel = " << std::setprecision(9) << std::setw(14) << std::scientific << outputSpw.CHAN_FREQ(outputSpw.NUM_CHAN -1) << " Hz";
    logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << oss.str() << LogIO::POST;

    /// Determine output SPW structure ///////////////////

    // Add input-output SPW pair to map
	inputOutputSpwMap_p[0] = std::make_pair(inputSpw,outputSpw);

    /// Modify SPW subtable ///////////////////

    logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) << "Write output SPW subtable " << LogIO::POST;

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

    // Flush changes
    outputMs_p->flush(True);

	return;
}

void MSTransformDataHandler::calculateIntermediateFrequencies(Int spwId,Vector<Double> &inputChanFreq,Vector<Double> &inputChanWidth,Vector<Double> &intermediateChanFreq,Vector<Double> &intermediateChanWidth)
{
	uInt mumOfInterChan = inputChanFreq.size() / freqbinMap_p[spwId];
	if (mumOfInterChan % freqbinMap_p[spwId]) mumOfInterChan += 1;
	numOfCombInputChanMap_p[spwId] = inputChanFreq.size();
	numOfCombInterChanMap_p[spwId] = mumOfInterChan;
	intermediateChanFreq.resize(mumOfInterChan,False);
	intermediateChanWidth.resize(mumOfInterChan,False);
	simpleAverage(freqbinMap_p[spwId], inputChanFreq, intermediateChanFreq);
	simpleAverage(freqbinMap_p[spwId], inputChanWidth, intermediateChanWidth);

    return;
}

// -----------------------------------------------------------------------
// Method to set all the elements of a scalar column to a given value
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
// Method to re-index Spectral Window column in Source sub-table
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
// Method to re-index Spectral Window column in DDI sub-table
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
// Method to re-index Spectral Window column in Feed sub-table
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
// Method to re-index Spectral Window column in SysCal sub-table
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
// Method to re-index Spectral Window column in FreqOffset sub-table
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

    return;
}

Bool MSTransformDataHandler::mergeSpwSubTables(Vector<String> filenames)
{
	String filename_0 = filenames(0);
	MeasurementSet ms_0(filename_0,Table::Update);
	MSSpectralWindow spwTable_0 = ms_0.spectralWindow();
	MSSpWindowColumns spwCols_0(spwTable_0);

	uInt rowIndex = spwTable_0.nrow();

	for (uInt subms_index=1;subms_index < filenames.size();subms_index++)
	{
		String filename_i = filenames(subms_index);
		MeasurementSet ms_i(filename_i);
		MSSpectralWindow spwTable_i = ms_i.spectralWindow();
		MSSpWindowColumns spwCols_i(spwTable_i);

		spwTable_0.addRow(spwTable_i.nrow());

		for (uInt subms_row_index=0;subms_row_index<spwTable_i.nrow();subms_row_index++)
		{
			spwCols_0.measFreqRef().put(rowIndex,spwCols_i.measFreqRef()(subms_row_index));
			spwCols_0.chanFreq().put(rowIndex,spwCols_i.chanFreq()(subms_row_index));
			spwCols_0.refFrequency().put(rowIndex,spwCols_i.refFrequency()(subms_row_index));
			spwCols_0.chanWidth().put(rowIndex,spwCols_i.chanWidth()(subms_row_index));
			spwCols_0.effectiveBW().put(rowIndex,spwCols_i.effectiveBW()(subms_row_index));
			spwCols_0.resolution().put(rowIndex,spwCols_i.resolution()(subms_row_index));
			spwCols_0.flagRow().put(rowIndex,spwCols_i.flagRow()(subms_row_index));
			spwCols_0.freqGroup().put(rowIndex,spwCols_i.freqGroup()(subms_row_index));
			spwCols_0.freqGroupName().put(rowIndex,spwCols_i.freqGroupName()(subms_row_index));
			spwCols_0.ifConvChain().put(rowIndex,spwCols_i.ifConvChain()(subms_row_index));
			spwCols_0.name().put(rowIndex,spwCols_i.name()(subms_row_index));
			spwCols_0.netSideband().put(rowIndex,spwCols_i.netSideband()(subms_row_index));
			spwCols_0.numChan().put(rowIndex,spwCols_i.numChan()(subms_row_index));
			spwCols_0.totalBandwidth().put(rowIndex,spwCols_i.totalBandwidth()(subms_row_index));

			rowIndex += 1;
		}
	}

	ms_0.flush(True);

	return True;
}

void MSTransformDataHandler::getInputNumberOfChannels()
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

void MSTransformDataHandler::getOutputNumberOfChannels()
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

	return;
}


void MSTransformDataHandler::calculateWeightAndSigmaFactors()
{
	map<Int,Int>::iterator iter;
	for(iter = numOfSelChanMap_p.begin(); iter != numOfSelChanMap_p.end(); iter++)
	{
		weightFactorMap_p[iter->first] = (Float)numOfSelChanMap_p[iter->first] / (Float)numOfInpChanMap_p[iter->first];
		sigmaFactorMap_p[iter->first] = 1./sqrt((Float)numOfSelChanMap_p[iter->first] / (Float)numOfOutChanMap_p[iter->first]);
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check if flag category has to be filled
// -----------------------------------------------------------------------
void MSTransformDataHandler::checkFillFlagCategory()
{
	fillFlagCategory_p = False;
	if (!selectedInputMsCols_p->flagCategory().isNull() && selectedInputMsCols_p->flagCategory().isDefined(0))
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
	if (!selectedInputMsCols_p->weightSpectrum().isNull() && selectedInputMsCols_p->weightSpectrum().isDefined(0))
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
	Bool mainColSet=False;
	if (datacolumn_p.contains("ALL"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::CORRECTED_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::CORRECTED_DATA] = MS::CORRECTED_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding CORRECTED_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::MODEL_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::MODEL_DATA] = MS::MODEL_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding MODEL_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::FLOAT_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::LAG_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
	}
	else if (datacolumn_p.contains("FLOAT_DATA,DATA"))
	{
		Bool mainColSet=False;

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::FLOAT_DATA;
				mainColSet = True;
			}
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
	else if (datacolumn_p.contains("FLOAT_DATA"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::FLOAT_DATA;
				mainColSet = True;
			}
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
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::LAG_DATA;
				mainColSet = True;
			}
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
	else if (datacolumn_p.contains("LAG_DATA"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			if (!mainColSet)
			{
				mainColumn_p = MS::LAG_DATA;
				mainColSet = True;
			}
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
			if (!mainColSet)
			{
				mainColumn_p = MS::DATA;
				mainColSet = True;
			}
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
			if (!mainColSet)
			{
				mainColumn_p = MS::CORRECTED_DATA;
				mainColSet = True;
			}
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
			if (!mainColSet)
			{
				mainColumn_p = MS::MODEL_DATA;
				mainColSet = True;
			}
			dataColMap_p[MS::MODEL_DATA] = MS::DATA;
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
// Method to determine sort columns order
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
// Method to generate the initial iterator
// -----------------------------------------------------------------------
void MSTransformDataHandler::generateIterator()
{
	visibilityIterator_p = new vi::VisibilityIterator2(*selectedInputMs_p,sortColumns_p,false,NULL,false,timeBin_p);
	if (channelSelector_p != NULL) visibilityIterator_p->setFrequencySelection(*channelSelector_p);
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
		// Fill baseline map using as key Ant1,Ant2,Scan and State,
		// Which are the elements that can be combined in one chunk
		baselineMap_p.clear();
		Vector<Int> antenna1 = vb->antenna1();
		Vector<Int> antenna2 = vb->antenna2();
		Vector<Int> scan = vb->scan();
		Vector<Int> state = vb->stateId();
		for (uInt row=0;row<antenna1.size();row++)
		{
			pair<Int,Int> baseline = std::make_pair(antenna1(row),antenna2(row));
			pair<Int,Int> scanState = std::make_pair(scan(row),state(row));
			baselineMap_p[std::make_pair(baseline,scanState)].push_back(row);
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

		// Initialize reference frame transformation parameters
		if (refFrameTransformation_p)
		{
			initFrequencyTransGrid(vb);
		}
	}
	else
	{
		rowsToAdd = vb->nRows();
	}

	uInt currentRows = outputMs_p->nrow();
	RefRows rowRef( currentRows, currentRows + rowsToAdd - 1);

	/*
	logger_p << LogIO::NORMAL << LogOrigin("MSTransformDataHandler", __FUNCTION__)
			<< "Filling output MS with " << rowsToAdd << " rows from input " << vb->nRows() << " rows"  << LogIO::POST;
	*/

	outputMs_p->addRow(rowsToAdd,True);

	fillIdCols(vb,rowRef);
    fillDataCols(vb,rowRef);

    return;

}

// -----------------------------------------------------------------------
// Method to initialize the input frequency grid to change reference frame
// -----------------------------------------------------------------------
void MSTransformDataHandler::initFrequencyTransGrid(vi::VisBuffer2 *vb)
{
	// Phase center is fixed for a given fieldId, which is constant along a chunk of data
	phaseCenter_p = vb->phaseCenter();

	// NOTE (jagonzal): According to dpetry the difference between times is negligible but he recommends to use TIME
	Double snapshotTime = vb->time()(0);
	observationTime_p = MEpoch(Quantity(snapshotTime,"s"), MEpoch::UTC);

	MFrequency::Ref inputFrameRef = MFrequency::Ref(inputReferenceFrame_p, MeasFrame(phaseCenter_p, observatoryPosition_p, observationTime_p));
	freqTransEngine_p = MFrequency::Convert(MSTransformations::Hz, inputFrameRef, outputReferenceFrame_p);

	Int spwIndex = 0;
	if (not combinespws_p)
	{
		Int originalSPWid = vb->spectralWindows()(0);
		spwIndex = inputOutputSPWIndexMap_p[originalSPWid];
	}

    for(uInt chan_idx=0; chan_idx<inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ.size(); chan_idx++)
    {
    	inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ_aux[chan_idx] = freqTransEngine_p(inputOutputSpwMap_p[spwIndex].first.CHAN_FREQ[chan_idx]).get(MSTransformations::Hz).getValue();
    }

	return;
}

// ----------------------------------------------------------------------------------------
// Fill auxiliary (meta data) columns which don't depend on the SPW (merely consist of Ids)
// ----------------------------------------------------------------------------------------
void MSTransformDataHandler::fillIdCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
	// Declare common auxiliary variables
	Vector<Int> tmpVectorInt(rowRef.nrow(),0);

	if (combinespws_p)
	{
		// Declare extra auxiliary variables
		Vector<Double> tmpVectorDouble(rowRef.nrow(),0.0);
		Vector<Bool> tmpVectorBool(rowRef.nrow(),False);
		Matrix<Float> tmpMatrixFloat(IPosition(2,vb->nCorrelations(),rowRef.nrow()),0.0);

		// Observation
		mapAndReindexVector(vb->observationId(),tmpVectorInt,inputOutputObservationIndexMap_p,True);
		outputMsCols_p->observationId().putColumnCells(rowRef,tmpVectorInt);

		// Array Id
		mapAndReindexVector(vb->arrayId(),tmpVectorInt,inputOutputArrayIndexMap_p,True);
		outputMsCols_p->arrayId().putColumnCells(rowRef,tmpVectorInt);

		// Field Id
		mapAndReindexVector(vb->fieldId(),tmpVectorInt,inputOutputFieldIndexMap_p,True);
		outputMsCols_p->fieldId().putColumnCells(rowRef,tmpVectorInt);

		// Scan -> SCAN is not re-indexed in old split
		// mapAndReindexVector(vb->scan(),tmpVectorInt,inputOutputScanIndexMap_p,!timespan_p.contains("scan"));
		// outputMsCols_p->scanNumber().putColumnCells(rowRef,tmpVectorInt);

		// State
		mapAndReindexVector(vb->stateId(),tmpVectorInt,inputOutputScanIntentIndexMap_p,!timespan_p.contains("state"));
		outputMsCols_p->stateId().putColumnCells(rowRef,tmpVectorInt);

		// Spectral Window
		tmpVectorInt = ddiStart_p;
		outputMsCols_p->dataDescId().putColumnCells(rowRef,tmpVectorInt);

		// Non re-indexable vector columns
		mapVector(vb->scan(),tmpVectorInt);
		outputMsCols_p->scanNumber().putColumnCells(rowRef,tmpVectorInt);
		mapVector(vb->antenna1(),tmpVectorInt);
		outputMsCols_p->antenna1().putColumnCells(rowRef,tmpVectorInt);
		mapVector(vb->antenna2(),tmpVectorInt);
		outputMsCols_p->antenna2().putColumnCells(rowRef,tmpVectorInt);
		mapVector(vb->feed1(),tmpVectorInt);
		outputMsCols_p->feed1().putColumnCells(rowRef,tmpVectorInt);
		mapVector(vb->feed2(),tmpVectorInt);
		outputMsCols_p->feed2().putColumnCells(rowRef,tmpVectorInt);
		mapVector(vb->processorId(),tmpVectorInt);
		outputMsCols_p->processorId().putColumnCells(rowRef,tmpVectorInt);
		mapVector(vb->time(),tmpVectorDouble);
		outputMsCols_p->time().putColumnCells(rowRef,tmpVectorDouble);
		mapVector(vb->timeCentroid(),tmpVectorDouble);
		outputMsCols_p->timeCentroid().putColumnCells(rowRef,tmpVectorDouble);
		mapVector(vb->timeInterval(),tmpVectorDouble);
		outputMsCols_p->interval().putColumnCells(rowRef,tmpVectorDouble);
		mapVector(vb->exposure(),tmpVectorDouble);
		outputMsCols_p->exposure().putColumnCells(rowRef,tmpVectorDouble);

		// Non re-indexable matrix columns
		Matrix<Double> tmpUvw(IPosition(2,3,rowRef.nrow()),0.0);
		mapMatrix(vb->uvw(),tmpUvw);
		outputMsCols_p->uvw().putColumnCells(rowRef,tmpUvw);

		// Averaged vector columns
		mapAndAverageVector(vb->flagRow(),tmpVectorBool);
		outputMsCols_p->flagRow().putColumnCells(rowRef,tmpVectorBool);

		// Averaged matrix columns
		mapScaleAndAverageMatrix(vb->weight(),tmpMatrixFloat,weightFactorMap_p,vb->spectralWindows());
		outputMsCols_p->weight().putColumnCells(rowRef,tmpMatrixFloat);

		// Sigma must be redefined to 1/weight when corrected data becomes data
		if (correctedToData_p)
		{
			arrayTransformInPlace(tmpMatrixFloat, MSTransformations::wtToSigma);
			outputMsCols_p->sigma().putColumnCells(rowRef, tmpMatrixFloat);
		}
		else
		{
			mapScaleAndAverageMatrix(vb->sigma(),tmpMatrixFloat,sigmaFactorMap_p,vb->spectralWindows());
			outputMsCols_p->sigma().putColumnCells(rowRef, tmpMatrixFloat);
		}
	}
	else
	{
		// Observation
		if (inputOutputObservationIndexMap_p.size())
		{
			reindexVector(vb->observationId(),tmpVectorInt,inputOutputObservationIndexMap_p,True);
			outputMsCols_p->observationId().putColumnCells(rowRef,tmpVectorInt);
		}
		else
		{
			outputMsCols_p->observationId().putColumnCells(rowRef,vb->observationId());
		}

		// Array
		if (inputOutputArrayIndexMap_p.size())
		{
			reindexVector(vb->arrayId(),tmpVectorInt,inputOutputArrayIndexMap_p,True);
			outputMsCols_p->arrayId().putColumnCells(rowRef,tmpVectorInt);
		}
		else
		{
			outputMsCols_p->arrayId().putColumnCells(rowRef,vb->arrayId());
		}

		// Field
		if (inputOutputFieldIndexMap_p.size())
		{
			reindexVector(vb->fieldId(),tmpVectorInt,inputOutputFieldIndexMap_p,True);
			outputMsCols_p->fieldId().putColumnCells(rowRef,tmpVectorInt);
		}
		else
		{
			outputMsCols_p->fieldId().putColumnCells(rowRef,vb->fieldId());
		}

		// Scan -> SCAN is not re-indexed in old split
		/*
		if (inputOutputScanIndexMap_p.size())
		{
			reindexVector(vb->scan(),tmpVectorInt,inputOutputScanIndexMap_p,!timespan_p.contains("scan"));
			outputMsCols_p->scanNumber().putColumnCells(rowRef,tmpVectorInt);
		}
		else
		{
			outputMsCols_p->scanNumber().putColumnCells(rowRef,vb->scan());
		}
		*/

		// State
		if (inputOutputScanIntentIndexMap_p.size())
		{
			reindexVector(vb->stateId(),tmpVectorInt,inputOutputScanIntentIndexMap_p,!timespan_p.contains("state"));
			outputMsCols_p->stateId().putColumnCells(rowRef,tmpVectorInt);
		}
		else
		{
			outputMsCols_p->stateId().putColumnCells(rowRef,vb->stateId());
		}

		// Spectral Window
		if (inputOutputSPWIndexMap_p.size())
		{
			reindexVector(vb->spectralWindows(),tmpVectorInt,inputOutputSPWIndexMap_p,True);
			outputMsCols_p->dataDescId().putColumnCells(rowRef,tmpVectorInt);
		}
		else
		{
			outputMsCols_p->dataDescId().putColumnCells(rowRef,vb->spectralWindows());
		}

		// Non re-indexable columns
		outputMsCols_p->scanNumber().putColumnCells(rowRef,vb->scan());
		outputMsCols_p->antenna1().putColumnCells(rowRef,vb->antenna1());
		outputMsCols_p->antenna2().putColumnCells(rowRef,vb->antenna2());
		outputMsCols_p->feed1().putColumnCells(rowRef,vb->feed1());
		outputMsCols_p->feed2().putColumnCells(rowRef,vb->feed2());
		outputMsCols_p->processorId().putColumnCells(rowRef,vb->processorId());
		outputMsCols_p->time().putColumnCells(rowRef,vb->time());
		outputMsCols_p->timeCentroid().putColumnCells(rowRef,vb->timeCentroid());
		outputMsCols_p->interval().putColumnCells(rowRef,vb->timeInterval());
		outputMsCols_p->exposure().putColumnCells(rowRef,vb->exposure());
		outputMsCols_p->uvw().putColumnCells(rowRef,vb->uvw());
		outputMsCols_p->flagRow().putColumnCells(rowRef,vb->flagRow());

		Matrix<Float> weights = vb->weight();
		if (weightFactorMap_p[vb->spectralWindows()(0)] != 1)
		{
			weights *= weightFactorMap_p[vb->spectralWindows()(0)];
		}
		outputMsCols_p->weight().putColumnCells(rowRef, weights);

		// Sigma must be redefined to 1/weight when corrected data becomes data
		if (correctedToData_p)
		{
			arrayTransformInPlace(weights, MSTransformations::wtToSigma);
			outputMsCols_p->sigma().putColumnCells(rowRef, weights);
		}
		else
		{
			Matrix<Float> sigma = vb->sigma();
			if (sigmaFactorMap_p[vb->spectralWindows()(0)] != 1)
			{
				sigma *= sigmaFactorMap_p[vb->spectralWindows()(0)];
			}
			outputMsCols_p->sigma().putColumnCells(rowRef, sigma);
		}
	}

	return;
}

// -----------------------------------------------------------------------
// Fill the output vector with an input scalar
// re-indexed according to the data selection
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::fillAndReindexScalar(T inputScalar, Vector<T> &outputVector, map<Int,Int> &inputOutputIndexMap)
{
	if (inputOutputIndexMap.size())
	{
		outputVector = inputOutputIndexMap[inputScalar];
	}
	else
	{
		outputVector = inputScalar;
	}

	return;
}

// -----------------------------------------------------------------------
// Fill the output vector with the data from an input vector mapped in
// (SPW,Scan,State) tuples, and re-indexed according to the data selection
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::mapAndReindexVector(const Vector<T> &inputVector, Vector<T> &outputVector, map<Int,Int> &inputOutputIndexMap, Bool constant)
{
	if (constant)
	{
		fillAndReindexScalar(inputVector(0),outputVector,inputOutputIndexMap);
	}
	else
	{
		if (inputOutputIndexMap.size())
		{
			for (uInt index=0; index<rowIndex_p.size();index++)
			{
				outputVector(index) = inputOutputIndexMap[inputVector(rowIndex_p[index])];
			}
		}
		else
		{
			mapVector(inputVector,outputVector);
		}
	}

	return;
}

// -----------------------------------------------------------------------
// Fill the output vector with the data from an input
// vector re-indexed according to the data selection
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::reindexVector(const Vector<T> &inputVector, Vector<T> &outputVector, map<Int,Int> &inputOutputIndexMap, Bool constant)
{
	if (constant)
	{
		T value = inputOutputIndexMap[inputVector(0)];
		outputVector = value;
	}
	else
	{
		for (uInt index=0; index<inputVector.shape()[0];index++)
		{
			outputVector(index) = inputOutputIndexMap[inputVector(index)];
		}
	}

	return;
}

// ------------------------------------------------------------------------------------
// Fill the data from an input vector with shape [nBaselinesxnSPWsxnScans/nStates]
// into an output vector with shape [nBaselinesxnScans/nStates]
// ------------------------------------------------------------------------------------
template <class T> void MSTransformDataHandler::mapVector(const Vector<T> &inputVector, Vector<T> &outputVector)
{
	for (uInt index=0; index<rowIndex_p.size();index++)
	{
		outputVector(index) = inputVector(rowIndex_p[index]);
	}

	return;
}

// ------------------------------------------------------------------------------------
// Fill the data from an input matrix with shape [nCol,nBaselinesxnSPWsxnScans/nStates]
// into an output matrix with shape [nCol,nBaselinesxnScans/nStates]
// ------------------------------------------------------------------------------------
template <class T> void MSTransformDataHandler::mapMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix)
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
// Fill the data from an input vector with shape [nBaselinesxnSPWs] into an
// output vector with shape [nBaselines] averaging the values from all SPWS
// -----------------------------------------------------------------------------------
template <class T> void MSTransformDataHandler::mapAndAverageVector(const Vector<T> &inputVector, Vector<T> &outputVector,Bool convolveFlags,vi::VisBuffer2 *vb)
{
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

// -----------------------------------------------------------------------------------
// Fill the data from an input matrix with shape [nCol,nBaselinesxnSPWs] into an
// output matrix with shape [nCol,nBaselines] accumulating the averaging from all SPWS
// -----------------------------------------------------------------------------------
template <class T> void MSTransformDataHandler::mapAndAverageMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix,Bool convolveFlags,vi::VisBuffer2 *vb)
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

template <class T> void MSTransformDataHandler::mapScaleAndAverageMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix,map<Int,T> scaleMap, Vector<Int> spws)
{
	// Get number of columns
	uInt nCols = outputMatrix.shape()(0);

    // Fill output array with the combined data from each SPW
	Int spw;
	uInt row;
	uInt baseline_index = 0;
	vector<uInt> baselineRows;
	T normalizingFactor, contributionFactor;
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
			contributionFactor = scaleMap[spw];

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

// ----------------------------------------------------------------------------------------
// Fill main (data) columns which have to be combined together to produce bigger SPWs
// ----------------------------------------------------------------------------------------
void MSTransformDataHandler::fillDataCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
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


    if (fillWeightSpectrum_p)
    {
    	averageKernelFloat_p = &MSTransformDataHandler::simpleAverageKernel;
    	transformCubeOfData(vb,rowRef,vb->weightSpectrum(),outputMsCols_p->weightSpectrum(),NULL);
    	if (useweights_p == "weights")
    	{
    		averageKernelFloat_p = &MSTransformDataHandler::weightAverageKernel;
    	}
    	else
    	{
    		averageKernelFloat_p = &MSTransformDataHandler::flagAverageKernel;
    	}
    }

    // Special case for flag category
    if (fillFlagCategory_p)
    {
    	if (combinespws_p)
    	{
            IPosition shapeFlagCategory = vb->flagCategory().shape();
            shapeFlagCategory(3) = rowRef.nrow();
            Array<Bool> flagCategory(shapeFlagCategory,const_cast<Bool*>(vb->flagCategory().getStorage(MSTransformations::False)),SHARE);
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
// Generic method to write a Vector from a VisBuffer into a ScalarColumn
// -----------------------------------------------------------------------
template <class T> void MSTransformDataHandler::writeVector(const Vector<T> &inputVector,ScalarColumn<T> &outputCol, RefRows &rowRef)
{
	IPosition shape = inputVector.shape();
	shape(0) = rowRef.nrows();
    Array<T> outputArray(shape,const_cast<T*>(inputVector.getStorage(MSTransformations::False)),SHARE);
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
    Array<T> outputArray(shape,const_cast<T*>(inputMatrix.getStorage(MSTransformations::False)),SHARE);
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
    Array<T> outputArray(shape,const_cast<T*>(inputCube.getStorage(MSTransformations::False)),SHARE);
    outputCol.putColumnCells(rowRef, outputArray);

	return;
}

void MSTransformDataHandler::transformCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<Complex> &inputDataCube,ArrayColumn<Complex> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
{
	(*this.*transformCubeOfDataComplex_p)(vb,rowRef,inputDataCube,outputDataCol,outputFlagCol);
	return;
}

void MSTransformDataHandler::transformCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<Float> &inputDataCube,ArrayColumn<Float> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
{
	(*this.*transformCubeOfDataFloat_p)(vb,rowRef,inputDataCube,outputDataCol,outputFlagCol);
	return;
}

template <class T> void MSTransformDataHandler::copyCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
{
	writeCube(inputDataCube,outputDataCol,rowRef);
	if (outputFlagCol != NULL)
	{
		writeCube(vb->flagCube(),*outputFlagCol,rowRef);
	}

	return;
}

template <class T> void MSTransformDataHandler::combineCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
{
	// Get input flag and weight cubes
	const Cube<Bool> inputFlagCube = vb->flagCube();
	const Cube<Float> inputWeightsCube = vb->weightSpectrum();

	// Get input SPWs
	Vector<Int> spws = vb->spectralWindows();

	// Get input cube shape
	IPosition inputCubeShape = inputDataCube.shape();
	uInt nInputCorrelations = inputCubeShape(0);
	uInt nInputChannels = inputCubeShape(1);

	// Initialize input planes
	IPosition inputPlaneShape(2,nInputCorrelations, numOfCombInputChanMap_p[0]);
	Matrix<T> inputPlaneData(inputPlaneShape);
	Matrix<Bool> inputPlaneFlags(inputPlaneShape);
	Matrix<Float> inputPlaneWeights(inputPlaneShape);

	// Initialize output planes
	IPosition outputPlaneShape(2,nInputCorrelations, inputOutputSpwMap_p[0].second.NUM_CHAN);
	Matrix<T> outputPlaneData(outputPlaneShape);
	Matrix<Bool> outputPlaneFlags(outputPlaneShape);

	Int spw = 0;
	uInt row = 0, baseline_index = 0, outputChannel = 0;
	vector<uInt> baselineRows;
	for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
	{
		// Fill input plane to benefit from contiguous access to the input cube
		baselineRows = iter->second;
		for (vector<uInt>::iterator iter = baselineRows.begin();iter != baselineRows.end(); iter++)
		{
			row = *iter;
			spw = spws(row);

			// Fill input planes
			uInt outputChannel;
			for (uInt inputChannel = 0; inputChannel < inputDataCube.shape()(1); inputChannel++)
			{
				outputChannel = spwChannelMap_p[spw][inputChannel];
				for (uInt pol = 0; pol < inputDataCube.shape()(0); pol++)
				{
					inputPlaneData(pol,outputChannel) = inputDataCube(pol,inputChannel,row);
					inputPlaneFlags(pol,outputChannel) = inputFlagCube(pol,inputChannel,row);
					(*this.*fillWeightsPlane_p)(pol,inputChannel,outputChannel,row,inputWeightsCube,inputPlaneWeights);
				}
			}
		}

		// Initialize output flags plane
		outputPlaneFlags = False;

		// Transform input planes and write them
		transformAndWritePlaneOfData(0,rowRef.firstRow()+baseline_index,inputPlaneData,inputPlaneFlags,inputPlaneWeights,outputPlaneData,outputPlaneFlags,outputDataCol,outputFlagCol);

		baseline_index += 1;
	}
}


void MSTransformDataHandler::fillWeightsPlane(uInt pol, uInt inputChannel, uInt outputChannel, uInt inputRow, const Cube<Float> &inputWeightsCube, Matrix<Float> &inputWeightsPlane)
{
	inputWeightsPlane(pol,outputChannel) = inputWeightsCube(pol,inputChannel,inputRow);

	return;
}

template <class T> void MSTransformDataHandler::averageCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
{
	// Get input spw and flag and weight cubes
	Int inputSpw = vb->spectralWindows()(0);
	const Cube<Bool> inputFlagsCube = vb->flagCube();
	const Cube<Float> inputWeightsCube = vb->weightSpectrum();

	// Define output plane shape
	IPosition outputPlaneShape = IPosition(2,inputDataCube.shape()(0), numOfOutChanMap_p[inputSpw]);

	transformAndWriteCubeOfData(inputSpw, rowRef, inputDataCube, inputFlagsCube, inputWeightsCube, outputPlaneShape, outputDataCol, outputFlagCol);

	return;
}

template <class T> void MSTransformDataHandler::smoothCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
{
	// Get input spw and flag cube
	Int inputSpw = vb->spectralWindows()(0);
	const Cube<Bool> inputFlagsCube = vb->flagCube();
	const Cube<Float> inputWeightsCube = vb->weightSpectrum();

	// Define output plane shape
	IPosition outputPlaneShape = IPosition(2,inputDataCube.shape()(0), inputDataCube.shape()(1));

	// Transform cube
	transformAndWriteCubeOfData(inputSpw, rowRef, inputDataCube, inputFlagsCube, inputWeightsCube, outputPlaneShape, outputDataCol, outputFlagCol);

	return;
}

template <class T> void MSTransformDataHandler::regridCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
{
	// Get input spw and flag cube
	Int inputSpw = vb->spectralWindows()(0);
	const Cube<Bool> inputFlagsCube = vb->flagCube();
	const Cube<Float> inputWeightsCube = vb->weightSpectrum();

	// Define output plane shape
	IPosition outputPlaneShape = IPosition(2,inputDataCube.shape()(0), inputOutputSpwMap_p[inputSpw].second.NUM_CHAN);

	// Transform cube
	transformAndWriteCubeOfData(inputSpw, rowRef, inputDataCube, inputFlagsCube, inputWeightsCube, outputPlaneShape, outputDataCol, outputFlagCol);

	return;
}

template <class T> void MSTransformDataHandler::transformAndWriteCubeOfData(Int inputSpw, RefRows &rowRef, const Cube<T> &inputDataCube, const Cube<Bool> &inputFlagsCube, const Cube<Float> &inputWeightsCube, IPosition &outputPlaneShape, ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
{
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
		transformAndWritePlaneOfData(inputSpw,rowRef.firstRow()+rowIndex,inputPlaneData,inputPlaneFlags,inputPlaneWeights,outputPlaneData,outputPlaneFlags,outputDataCol,outputFlagCol);
	}

	return;
}

void MSTransformDataHandler::setWeightsPlaneByReference(uInt inputRow, const Cube<Float> &inputWeightsCube, Matrix<Float> &inputWeightsPlane)
{
	inputWeightsPlane = inputWeightsCube.xyPlane(inputRow);

	return;
}

template <class T> void MSTransformDataHandler::transformAndWritePlaneOfData(Int inputSpw, uInt row, Matrix<T> &inputDataPlane,Matrix<Bool> &inputFlagsPlane, Matrix<Float> &inputWeightsPlane, Matrix<T> &outputDataPlane,Matrix<Bool> &outputFlagsPlane, ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol)
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

		transformStripeOfData(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);
	}

	// Write output planes
	outputDataCol.setShape(row,outputPlaneShape);
	outputDataCol.put(row, outputDataPlane);

	if (outputFlagCol != NULL)
	{
		outputFlagCol->setShape(row,outputPlaneShape);
		outputFlagCol->put(row, outputFlagsPlane);
	}

	return;
}

void MSTransformDataHandler::setWeightStripeByReference(uInt corrIndex,Matrix<Float> &inputWeightsPlane, Vector<Float> &inputWeightsStripe)
{
	inputWeightsStripe.reference(inputWeightsPlane.row(corrIndex));
	return;
}

void MSTransformDataHandler::transformStripeOfData(Int inputSpw, Vector<Complex> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe, Vector<Complex> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
	(*this.*transformStripeOfDataComplex_p)(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);
	return;
}

void MSTransformDataHandler::transformStripeOfData(Int inputSpw, Vector<Float> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe, Vector<Float> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
	(*this.*transformStripeOfDataFloat_p)(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);
	return;
}

template <class T> void MSTransformDataHandler::average(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
	uInt width = freqbinMap_p[inputSpw];
	uInt startChan = 0;
	uInt outChanIndex = 0;
	uInt tail = inputDataStripe.size() % width;
	uInt limit = inputDataStripe.size() - tail;
	while (startChan < limit)
	{
		averageKernel(inputDataStripe,inputFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe,startChan,outChanIndex,width);
		startChan += width;
		outChanIndex += 1;
	}

	if (tail)
	{
		averageKernel(inputDataStripe,inputFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe,startChan,outChanIndex,tail);
	}

	return;
}

template <class T> void  MSTransformDataHandler::simpleAverage(uInt width, Vector<T> &inputData, Vector<T> &outputData)
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

	if (tail)
	{
		simpleAverageKernel(inputData,inputFlags,inputWeights,outputData,outputFlags,startChan,outChanIndex,width);
	}

	return;
}

void MSTransformDataHandler::averageKernel(Vector<Complex> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<Complex> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width)
{
	(*this.*averageKernelComplex_p)(inputData,inputFlags,inputWeights,outputData,outputFlags,startInputPos,outputPos,width);
	return;
}

void MSTransformDataHandler::averageKernel(Vector<Float> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<Float> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width)
{
	(*this.*averageKernelFloat_p)(inputData,inputFlags,inputWeights,outputData,outputFlags,startInputPos,outputPos,width);
	return;
}

template <class T> void MSTransformDataHandler::simpleAverageKernel(Vector<T> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<T> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width)
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

template <class T> void MSTransformDataHandler::flagAverageKernel(Vector<T> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<T> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width)
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


template <class T> void MSTransformDataHandler::weightAverageKernel(Vector<T> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<T> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width)
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

template <class T> void MSTransformDataHandler::smooth(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
    Smooth<T>::hanning(	outputDataStripe, 		// the output data
    					outputFlagsStripe, 		// the output flags
    					inputDataStripe, 		// the input data
    					inputFlagsStripe, 		// the input flags
    					False);					// A good data point has its flag set to False

	return;
}

template <class T> void MSTransformDataHandler::regrid(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
    InterpolateArray1D<Double,T>::interpolate(	outputDataStripe, // Output data
    											outputFlagsStripe, // Output flags
    											inputOutputSpwMap_p[inputSpw].second.CHAN_FREQ, // Output channel frequencies
    											inputOutputSpwMap_p[inputSpw].first.CHAN_FREQ_aux, // Input channel frequencies
    											inputDataStripe, // Input data
    											inputFlagsStripe, // Input Flags
    											interpolationMethod_p, // Interpolation method
    											False, // A good data point has its flag set to False
    											False // If False extrapolated data points are set flagged
						    					);

	return;
}

template <class T> void MSTransformDataHandler::averageSmooth(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
	Vector<T> averagedDataStripe(outputDataStripe.shape(),T());
	Vector<Bool> averagedFlagsStripe(outputFlagsStripe.shape(),MSTransformations::False);

	average(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe, averagedDataStripe,averagedFlagsStripe);

	smooth(inputSpw,averagedDataStripe,averagedFlagsStripe, inputWeightsStripe, outputDataStripe,outputFlagsStripe);

	return;
}

template <class T> void MSTransformDataHandler::averageRegrid(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
	Vector<T> averagedDataStripe(numOfCombInterChanMap_p[inputSpw],T());
	Vector<Bool> averagedFlagsStripe(numOfCombInterChanMap_p[inputSpw],MSTransformations::False);

	average(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe, averagedDataStripe,averagedFlagsStripe);

	regrid(inputSpw,averagedDataStripe,averagedFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);

	return;
}

template <class T> void MSTransformDataHandler::smoothRegrid(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
	Vector<T> smoothedDataStripe(inputDataStripe.shape(),T());
	Vector<Bool> smoothedFlagsStripe(inputFlagsStripe.shape(),MSTransformations::False);

	smooth(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe,smoothedDataStripe,smoothedFlagsStripe);

	regrid(inputSpw,smoothedDataStripe,smoothedFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);

	return;
}

template <class T> void MSTransformDataHandler::averageSmoothRegrid(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe)
{
	Vector<T> averageSmoothedDataStripe(numOfCombInterChanMap_p[inputSpw],T());
	Vector<Bool> averageSmoothedFlagsStripe(numOfCombInterChanMap_p[inputSpw],MSTransformations::False);

	averageSmooth(inputSpw,inputDataStripe,inputFlagsStripe,inputWeightsStripe,averageSmoothedDataStripe,averageSmoothedFlagsStripe);

	regrid(inputSpw,averageSmoothedDataStripe,averageSmoothedFlagsStripe,inputWeightsStripe,outputDataStripe,outputFlagsStripe);

	return;
}


} //# NAMESPACE CASA - END
