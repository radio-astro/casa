//# MSTransformDataHandler.h: This file contains the interface definition of the MSTransformDataHandler class.
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

#ifndef MSTransformDataHandler_H_
#define MSTransformDataHandler_H_

// To handle configuration records
#include <casacore/casa/Containers/Record.h>

// To handle variant parameters
#include <stdcasa/StdCasa/CasacSupport.h>

// Measurement Set Selection
#include <ms/MeasurementSets/MSSelection.h>

// To use Sub-Ms class
#include <synthesis/MSVis/SubMS.h>

// VisibityIterator / VisibilityBuffer framework
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/ViFrequencySelection.h>

// To get observatory position from observatory name
#include <measures/Measures/MeasTable.h>

// To post formatted msgs via ostringstream
#include <iomanip>

// To apply hanning smooth
#include <scimath/Mathematics/Smooth.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// MS Transform Framework utilities
namespace MSTransformations
{
	// Returns 1/sqrt(wt) or -1, depending on whether wt is positive..
	Double wtToSigma(Double wt);
}

// Forward declarations
struct spwInfo;

// Map definition
typedef map<MS::PredefinedColumns,MS::PredefinedColumns> dataColMap;
typedef map< pair< pair<Int,Int> , pair<Int,Int> >,vector<uInt> > baselineMap;
typedef map<Int,map<uInt, uInt > > inputSpwChanMap;
typedef map<Int,pair < spwInfo, spwInfo > > inputOutputSpwMap;

// Struct definition
struct channelInfo {

	Int SPW_id;
	uInt inpChannel;
	uInt outChannel;
	Double CHAN_FREQ;
	Double CHAN_WIDTH;
	Double EFFECTIVE_BW;
	Double RESOLUTION;

	channelInfo()
	{
		SPW_id = -1;
		inpChannel = 0;
		outChannel = 0;

		CHAN_FREQ = -1;
		CHAN_WIDTH = -1;
		EFFECTIVE_BW = -1;
		RESOLUTION = -1;
	}

	bool operator<(const channelInfo& right_operand) const
	{
		if (CHAN_FREQ<right_operand.CHAN_FREQ)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	Double upperBound() const
	{
		return CHAN_FREQ+0.5*CHAN_WIDTH;
	}

	Double lowerBound() const
	{
		return CHAN_FREQ+0.5*CHAN_WIDTH;
	}

	bool overlap(const channelInfo& other) const
	{
		if (CHAN_FREQ<other.CHAN_FREQ)
		{
			if (upperBound() > other.lowerBound())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (other.upperBound() > lowerBound())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
};

struct spwInfo {

	spwInfo()
	{
		initialize(0);
	}

	spwInfo(uInt nChannels)
	{
		initialize(nChannels);
	}

	spwInfo(Vector<Double> &chanFreq,Vector<Double> &chanWidth)
	{
		initialize(chanFreq.size());
		CHAN_FREQ = chanFreq;
		CHAN_WIDTH = chanWidth;
		update();
	}

	void initialize(uInt nChannels)
	{
		NUM_CHAN = nChannels;
		CHAN_FREQ.resize(nChannels);
		CHAN_WIDTH.resize(nChannels);
		EFFECTIVE_BW.resize(nChannels);
		RESOLUTION.resize(nChannels);
		CHAN_FREQ_aux.resize(nChannels);
		TOTAL_BANDWIDTH = 0;
		REF_FREQUENCY = 0;
		upperBound = 0;
		lowerBound = 0;
	}

	void update()
	{
		upperBound = CHAN_FREQ(NUM_CHAN-1)+0.5*CHAN_FREQ(NUM_CHAN-1);
		lowerBound = CHAN_FREQ(0)-0.5*CHAN_FREQ(0);
		TOTAL_BANDWIDTH = upperBound - lowerBound;
		REF_FREQUENCY = CHAN_FREQ(0);

		CHAN_FREQ_aux = CHAN_FREQ;
		EFFECTIVE_BW = CHAN_WIDTH;
		RESOLUTION = CHAN_WIDTH;
	}

	uInt NUM_CHAN;
	Vector<Double> CHAN_FREQ;
	Vector<Double> CHAN_WIDTH;
	Vector<Double> EFFECTIVE_BW;
	Vector<Double> RESOLUTION;
	Vector<Double> CHAN_FREQ_aux;
	Double TOTAL_BANDWIDTH;
	Double REF_FREQUENCY;
	Double upperBound;
	Double lowerBound;
};

//  MSTransformDataHandler definition
class MSTransformDataHandler
{

public:

	MSTransformDataHandler();
	MSTransformDataHandler(Record configuration);

	~MSTransformDataHandler();

	void initialize();
	void configure(Record &configuration);

	void open();
	void setup();
	void close();

	vi::VisibilityIterator2 * getVisIter() {return visibilityIterator_p;}

	void fillOutputMs(vi::VisBuffer2 *vb);

	// To consilidate several SPW subtables
	static Bool mergeSpwSubTables(Vector<String> filenames);
	static Bool mergeDDISubTables(Vector<String> filenames);


protected:

	void parseMsSpecParams(Record &configuration);
	void parseDataSelParams(Record &configuration);
	void parseFreqTransParams(Record &configuration);
	void parseChanAvgParams(Record &configuration);
	void parseRefFrameTransParams(Record &configuration);
	void parseFreqSpecParams(Record &configuration);

	// From input MS
	void initDataSelectionParams();
	void getInputNumberOfChannels();

	// To re-grid SPW subtable
	void initRefFrameTransParams();
	void regridSpwSubTable();
	void regridAndCombineSpwSubtable();
	void reindexColumn(ScalarColumn<Int> &inputCol, Int value);
	void reindexSourceSubTable();
	void reindexDDISubTable();
	void reindexFeedSubTable();
	void reindexSysCalSubTable();
	void reindexFreqOffsetSubTable();

	// From output MS
	void getOutputNumberOfChannels();

	// For channel averaging and selection
	void calculateIntermediateFrequencies(Int spwId,Vector<Double> &inputChanFreq,Vector<Double> &inputChanWidth,Vector<Double> &intermediateChanFreq,Vector<Double> &intermediateChanWidth);
	void calculateWeightAndSigmaFactors();

	// From selected MS
	void checkFillFlagCategory();
	void checkFillWeightSpectrum();

	// Iterator set-up
	void checkDataColumnsToFill();
	void setIterationApproach();
	void generateIterator();

	void initFrequencyTransGrid(vi::VisBuffer2 *vb);
	void fillIdCols(vi::VisBuffer2 *vb,RefRows &rowRef);
	void fillDataCols(vi::VisBuffer2 *vb,RefRows &rowRef);

	// To transform re-indexable columns
	template <class T> void fillAndReindexScalar(T inputScalar, Vector<T> &outputVector, map<Int,Int> &inputOutputIndexMap);
	template <class T> void mapAndReindexVector(const Vector<T> &inputVector, Vector<T> &outputVector, map<Int,Int> &inputOutputIndexMap, Bool constant=False);
	template <class T> void reindexVector(const Vector<T> &inputVector, Vector<T> &outputVector, map<Int,Int> &inputOutputIndexMap, Bool constant=False);

	// To transform non re-indexable columns
	template <class T> void mapVector(const Vector<T> &inputVector, Vector<T> &outputVector);
	template <class T> void mapMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix);
	template <class T> void mapAndAverageVector(const Vector<T> &inputVector, Vector<T> &outputVector,Bool convolveFlags=False,vi::VisBuffer2 *vb=NULL);
	template <class T> void mapAndAverageMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix,Bool convolveFlags=False,vi::VisBuffer2 *vb=NULL);
	template <class T> void mapScaleAndAverageMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix,map<Int,T> scaleMap, Vector<Int> spws);

	// When no transformations are needed, and the only combination axis is SPW
	template <class T> void writeVector(const Vector<T> &inputVector,ScalarColumn<T> &outputCol, RefRows &rowRef, uInt nBlocks);
	template <class T> void writeMatrix(const Matrix<T> &inputMatrix,ArrayColumn<T> &outputCol, RefRows &rowRef, uInt nBlocks);
	template <class T> void writeCube(const Cube<T> &inputCube,ArrayColumn<T> &outputCol, RefRows &rowRef);

	void transformCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<Complex> &inputDataCube,ArrayColumn<Complex> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);
	void transformCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<Float> &inputDataCube,ArrayColumn<Float> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);
	void (casa::MSTransformDataHandler::*transformCubeOfDataComplex_p)(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<Complex> &inputDataCube,ArrayColumn<Complex> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);
	void (casa::MSTransformDataHandler::*transformCubeOfDataFloat_p)(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<Float> &inputDataCube,ArrayColumn<Float> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);

	template <class T> void copyCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);

	template <class T> void combineCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);
	void fillWeightsPlane(uInt pol, uInt inputChannel, uInt outputChannel, uInt inputRow, const Cube<Float> &inputWeightsCube, Matrix<Float> &inputWeightsPlane);
	void dontfillWeightsPlane(uInt pol, uInt inputChannel, uInt outputChannel, uInt inputRow, const Cube<Float> &inputWeightsCube, Matrix<Float> &inputWeightsPlane) {return;}
	void (casa::MSTransformDataHandler::*fillWeightsPlane_p)(uInt pol, uInt inputChannel, uInt outputChannel, uInt inputRow, const Cube<Float> &inputWeightsCube, Matrix<Float> &inputWeightsPlane);

	template <class T> void averageCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);
	template <class T> void smoothCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);
	template <class T> void regridCubeOfData(vi::VisBuffer2 *vb, RefRows &rowRef, const Cube<T> &inputDataCube,ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);

	template <class T> void transformAndWriteCubeOfData(Int inputSpw, RefRows &rowRef, const Cube<T> &inputDataCube, const Cube<Bool> &inputFlagsCube, const Cube<Float> &inputWeightsCube, IPosition &outputPlaneShape, ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);
	void setWeightsPlaneByReference(uInt inputRow, const Cube<Float> &inputWeightsCube, Matrix<Float> &inputWeightsPlane);
	void dontsetWeightsPlaneByReference(uInt inputRow, const Cube<Float> &inputWeightsCube, Matrix<Float> &inputWeightsPlane) {return;}
	void (casa::MSTransformDataHandler::*setWeightsPlaneByReference_p)(uInt inputRow, const Cube<Float> &inputWeightsCube, Matrix<Float> &inputWeightsPlane);

	template <class T> void transformAndWritePlaneOfData(Int inputSpw, uInt row, Matrix<T> &inputDataPlane,Matrix<Bool> &inputFlagsPlane, Matrix<Float> &inputWeightsPlane, Matrix<T> &outputDataPlane,Matrix<Bool> &outputFlagsPlane, ArrayColumn<T> &outputDataCol, ArrayColumn<Bool> *outputFlagCol);
	void setWeightStripeByReference(uInt corrIndex,Matrix<Float> &inputWeightsPlane, Vector<Float> &inputWeightsStripe);
	void dontSetWeightStripeByReference(uInt corrIndex,Matrix<Float> &inputWeightsPlane, Vector<Float> &inputWeightsStripe) {return;}
	void (casa::MSTransformDataHandler::*setWeightStripeByReference_p)(uInt corrIndex,Matrix<Float> &inputWeightsPlane, Vector<Float> &inputWeightsStripe);

	void transformStripeOfData(Int inputSpw, Vector<Complex> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe, Vector<Complex> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	void transformStripeOfData(Int inputSpw, Vector<Float> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe, Vector<Float> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	void (casa::MSTransformDataHandler::*transformStripeOfDataComplex_p)(Int inputSpw, Vector<Complex> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<Complex> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	void (casa::MSTransformDataHandler::*transformStripeOfDataFloat_p)(Int inputSpw, Vector<Float> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<Float> &outputDataStripe,Vector<Bool> &outputFlagsStripe);

	template <class T> void average(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	template <class T> void simpleAverage(uInt width, Vector<T> &inputData, Vector<T> &outputData);
	void averageKernel(Vector<Complex> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<Complex> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width);
	void averageKernel(Vector<Float> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<Float> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width);
	void (casa::MSTransformDataHandler::*averageKernelComplex_p)(Vector<Complex> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<Complex> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width);
	void (casa::MSTransformDataHandler::*averageKernelFloat_p)(Vector<Float> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<Float> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width);
	template <class T> void simpleAverageKernel(Vector<T> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<T> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width);
	template <class T> void flagAverageKernel(Vector<T> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<T> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width);
	template <class T> void weightAverageKernel(Vector<T> &inputData, Vector<Bool> &inputFlags, Vector<Float> &inputWeights, Vector<T> &outputData, Vector<Bool> &outputFlags, uInt startInputPos, uInt outputPos, uInt width);

	template <class T> void smooth(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	template <class T> void regrid(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	template <class T> void averageSmooth(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	template <class T> void averageRegrid(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	template <class T> void smoothRegrid(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe);
	template <class T> void averageSmoothRegrid(Int inputSpw, Vector<T> &inputDataStripe,Vector<Bool> &inputFlagsStripe, Vector<Float> &inputWeightsStripe,Vector<T> &outputDataStripe,Vector<Bool> &outputFlagsStripe);


	// MS specification parameters
	String inpMsName_p;
	String outMsName_p;
	String datacolumn_p;
	Vector<Int> tileShape_p;

	// Data selection parameters
	String arraySelection_p;
	String fieldSelection_p;
	String scanSelection_p;
	String timeSelection_p;
	String spwSelection_p;
	String baselineSelection_p;
	String uvwSelection_p;
	String polarizationSelection_p;
	String scanIntentSelection_p;
	String observationSelection_p;

	// Input-Output index maps
	map<Int,Int> inputOutputObservationIndexMap_p;
	map<Int,Int> inputOutputArrayIndexMap_p;
	map<Int,Int> inputOutputScanIndexMap_p;
	map<Int,Int> inputOutputScanIntentIndexMap_p;
	map<Int,Int> inputOutputFieldIndexMap_p;
	map<Int,Int> inputOutputSPWIndexMap_p;
	map<Int,Int> outputInputSPWIndexMap_p;

	// Frequency transformation parameters
	Int nspws_p;
	Int ddiStart_p;
	Bool combinespws_p;
	Bool channelAverage_p;
	Bool hanningSmooth_p;
	Bool refFrameTransformation_p;
	Vector<Int> freqbin_p;
	String useweights_p;
	String interpolationMethodPar_p;
	casac::variant *phaseCenterPar_p;
	String restFrequency_p;
	String outputReferenceFramePar_p;

	// Frequency specification parameters
	String mode_p;
	String start_p;
	String width_p;
	int nChan_p;
	String velocityType_p;

	// Time transformation parameters
	Double timeBin_p;
	String timespan_p;

	// MS-related members
	SubMS *splitter_p;
	MeasurementSet *inputMs_p;
	MeasurementSet *selectedInputMs_p;
	MeasurementSet *outputMs_p;
	ROMSColumns *selectedInputMsCols_p;
	MSColumns *outputMsCols_p;

	// VI/VB related members
	Block<Int> sortColumns_p;
	vi::VisibilityIterator2 *visibilityIterator_p;
	vi::FrequencySelectionUsingChannels *channelSelector_p;

	// Output MS structure related members
	Bool fillFlagCategory_p;
	Bool fillWeightSpectrum_p;
	Bool correctedToData_p;
	dataColMap dataColMap_p;
	MSMainEnums::PredefinedColumns mainColumn_p;

	// Frequency transformation members
	baselineMap baselineMap_p;
	vector<uInt> rowIndex_p;
	inputSpwChanMap spwChannelMap_p;
	uInt interpolationMethod_p;
	inputOutputSpwMap inputOutputSpwMap_p;
	map<Int,Int> freqbinMap_p;
	map<Int,Int> numOfInpChanMap_p;
	map<Int,Int> numOfSelChanMap_p;
	map<Int,Int> numOfOutChanMap_p;
	map<Int,Int> numOfCombInputChanMap_p;
	map<Int,Int> numOfCombInterChanMap_p;
	map<Int,Float> weightFactorMap_p;
	map<Int,Float> sigmaFactorMap_p;

	// Reference Frame Transformation members
	MFrequency::Types inputReferenceFrame_p;
	MFrequency::Types outputReferenceFrame_p;
	MPosition observatoryPosition_p;
	MEpoch observationTime_p;
	MDirection phaseCenter_p;
	MFrequency::Convert freqTransEngine_p;

	// Logging
	LogIO logger_p;
};

} //# NAMESPACE CASA - END

#endif /* MSTransformDataHandler_H_ */
