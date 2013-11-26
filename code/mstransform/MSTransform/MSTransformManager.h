//# MSTransformManager.h: This file contains the interface definition of the MSTransformManager class.
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

#ifndef MSTransformManager_H_
#define MSTransformManager_H_

// To handle configuration records
#include <casacore/casa/Containers/Record.h>

// To handle variant parameters
#include <stdcasa/StdCasa/CasacSupport.h>

// Measurement Set Selection
#include <ms/MeasurementSets/MSSelection.h>

// Data handling
#include <mstransform/MSTransform/MSTransformDataHandler.h>

// Regridding
#include <mstransform/MSTransform/MSTransformRegridder.h>

// VisibityIterator / VisibilityBuffer framework
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/ViFrequencySelection.h>

// TVI framework
#include <synthesis/MSVis/AveragingVi2Factory.h>

// To get observatory position from observatory name
#include <measures/Measures/MeasTable.h>

// To post formatted msgs via ostringstream
#include <iomanip>

// To apply hanning smooth
#include <scimath/Mathematics/Smooth.h>

// To apply fft shift
#include <scimath/Mathematics/FFTServer.h>

// To apply 1D interpolations
#include <scimath/Mathematics/InterpolateArray1D.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// MS Transform Framework utilities
namespace MSTransformations
{
	// Returns 1/sqrt(wt) or -1, depending on whether wt is positive..
	Double wtToSigma(Double wt);
}

// Forward declarations
struct spwInfo;
struct channelContribution;

// Map definition
typedef map<MS::PredefinedColumns,MS::PredefinedColumns> dataColMap;
typedef map< pair< pair<Int,Int> , Int >,vector<uInt> > baselineMap;
typedef map<Int,map<uInt, uInt > > inputSpwChanMap;
typedef map<Int,vector < channelContribution > >  inputOutputChanFactorMap;
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
		return CHAN_FREQ-0.5*CHAN_WIDTH;
	}

	Double overlap(const channelInfo& other) const
	{

		// The other channel completely covers this channel
		if ((lowerBound() <= other.lowerBound()) and (upperBound() >= other.upperBound()))
		{
			return 1.0;
		}
		// The other channel is completely covered by this channel
		else if ((lowerBound() >= other.lowerBound()) and (upperBound() <= other.upperBound()))
		{
			return CHAN_WIDTH/other.CHAN_WIDTH;
		}
		// Lower end of this channel is overlapping with the other channel
		else if (lowerBound() < other.lowerBound() && other.lowerBound() < upperBound() && upperBound() < other.upperBound())
		{
			return (upperBound()-other.lowerBound())/other.CHAN_WIDTH;
		}
		// Upper end of this channel is overlapping with the other channel
		else if (other.lowerBound() < lowerBound() && lowerBound() < other.upperBound() && other.upperBound() < upperBound())
		{
			return (other.upperBound()-lowerBound())/other.CHAN_WIDTH;
		}
		else
		{
			return 0.0;
		}

	}
};

struct channelContribution {

	Int inpSpw;
	uInt inpChannel;
	uInt outChannel;
	Double weight;
	Bool flag;

	channelContribution()
	{
		inpSpw = 0;
		inpChannel = 0;
		outChannel = 0;
		weight = 0;
		flag = False;
	}

	channelContribution(Int inputSpw, uInt inputChannel, uInt outputChannel,Double fraction)
	{
		inpSpw = inputSpw;
		inpChannel = inputChannel;
		outChannel = outputChannel;
		weight = fraction;
		flag = True;
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
		reset(chanFreq,chanWidth);
	}

	void reset(Vector<Double> &chanFreq,Vector<Double> &chanWidth)
	{
		initialize(chanFreq.size());
		CHAN_FREQ = chanFreq;
		CHAN_WIDTH = chanWidth;
		update();
	}

	void initialize(uInt nChannels)
	{
		NUM_CHAN = nChannels;
		CHAN_FREQ.resize(nChannels,False);
		CHAN_WIDTH.resize(nChannels,False);
		EFFECTIVE_BW.resize(nChannels,False);
		RESOLUTION.resize(nChannels,False);
		CHAN_FREQ_aux.resize(nChannels,False);
		TOTAL_BANDWIDTH = 0;
		REF_FREQUENCY = 0;
		upperBound = 0;
		lowerBound = 0;
	}

	void update()
	{
		upperBound = CHAN_FREQ(NUM_CHAN-1)+0.5*CHAN_WIDTH(NUM_CHAN-1);
		lowerBound = CHAN_FREQ(0)-0.5*CHAN_WIDTH(0);
		TOTAL_BANDWIDTH = upperBound - lowerBound;
		REF_FREQUENCY = CHAN_FREQ(0);

		CHAN_FREQ_aux = CHAN_FREQ;
		EFFECTIVE_BW = CHAN_WIDTH;
		RESOLUTION = CHAN_WIDTH;
	}

	void resize(uInt nChannels)
	{
		NUM_CHAN = nChannels;
		CHAN_FREQ.resize(nChannels,True);
		CHAN_WIDTH.resize(nChannels,True);
		EFFECTIVE_BW.resize(nChannels,True);
		RESOLUTION.resize(nChannels,True);
		CHAN_FREQ_aux.resize(nChannels,True);
		update();
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

//  MSTransformManager definition
class MSTransformManager
{

public:

	MSTransformManager();
	MSTransformManager(Record configuration);

	~MSTransformManager();

	void initialize();
	void configure(Record &configuration);

	void open();
	void setup();
	void close();

	vi::VisibilityIterator2 * getVisIter() {return visibilityIterator_p;}

	void fillOutputMs(vi::VisBuffer2 *vb);


protected:

	void parseMsSpecParams(Record &configuration);
	void parseDataSelParams(Record &configuration);
	void parseFreqTransParams(Record &configuration);
	void parseChanAvgParams(Record &configuration);
	void parseRefFrameTransParams(Record &configuration);
	void parseFreqSpecParams(Record &configuration);
	void parseTimeAvgParams(Record &configuration);

	// From input MS
	void initDataSelectionParams();
	void getInputNumberOfChannels();

	// To re-grid SPW subtable
	void initRefFrameTransParams();
	void regridSpwSubTable();
	void regridAndCombineSpwSubtable();
	void regridSpwAux(	Int spwId,
						Vector<Double> &inputCHAN_FREQ,
						Vector<Double> &inputCHAN_WIDTH,
						Vector<Double> &originalCHAN_FREQ,
						Vector<Double> &originalCHAN_WIDTH,
						Vector<Double> &regriddedCHAN_FREQ,
						Vector<Double> &regriddedCHAN_WIDTH,
						string msg);
	void reindexColumn(ScalarColumn<Int> &inputCol, Int value);
	void reindexSourceSubTable();
	void reindexDDISubTable();
	void reindexFeedSubTable();
	void reindexSysCalSubTable();
	void reindexFreqOffsetSubTable();
	void separateSpwSubtable();

	// Setter for the weight-based average
	void setWeightBasedTransformations(uInt mode);

	// Drop channels with non-uniform width when doing channel average
	void dropNonUniformWidthChannels();

	// From output MS
	void getOutputNumberOfChannels();

	// For channel averaging and selection
	void calculateIntermediateFrequencies(	Int spwId,
											Vector<Double> &inputChanFreq,
											Vector<Double> &inputChanWidth,
											Vector<Double> &intermediateChanFreq,
											Vector<Double> &intermediateChanWidth);
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

	template <class T> void fillAndReindexScalar(	T inputScalar,
													Vector<T> &outputVector,
													map<Int,Int> &inputOutputIndexMap);
	template <class T> void mapAndReindexVector(	const Vector<T> &inputVector,
													Vector<T> &outputVector,
													map<Int,Int> &inputOutputIndexMap,
													Bool constant=False);
	template <class T> void reindexVector(	const Vector<T> &inputVector,
											Vector<T> &outputVector,
											map<Int,Int> &inputOutputIndexMap,
											Bool constant=False);

	template <class T> void mapVector(const Vector<T> &inputVector, Vector<T> &outputVector);
	template <class T> void mapMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix);
	template <class T> void mapAndAverageVector(	const Vector<T> &inputVector,
													Vector<T> &outputVector,
													Bool convolveFlags=False,
													vi::VisBuffer2 *vb=NULL);
	template <class T> void mapAndAverageMatrix(	const Matrix<T> &inputMatrix,
													Matrix<T> &outputMatrix,
													Bool convolveFlags=False,
													vi::VisBuffer2 *vb=NULL);
	template <class T> void mapScaleAndAverageMatrix(	const Matrix<T> &inputMatrix,
														Matrix<T> &outputMatrix,
														map<Int,T> scaleMap,
														Vector<Int> spws);

	template <class T> void writeVector(	const Vector<T> &inputVector,
											ScalarColumn<T> &outputCol,
											RefRows &rowRef,
											uInt nBlocks);
	template <class T> void writeRollingVector(	Vector<T> &inputVector,
												ScalarColumn<T> &outputCol,
												RefRows &rowRef,
												uInt nBlocks);
	template <class T> void writeVectorBlock(	const Vector<T> &inputVector,
												ScalarColumn<T> &outputCol,
												RefRows &rowRef,
												uInt offset);
	template <class T> void writeMatrix(	const Matrix<T> &inputMatrix,
											ArrayColumn<T> &outputCol,
											RefRows &rowRef,
											uInt nBlocks);
	template <class T> void writeCube(	const Cube<T> &inputCube,
										ArrayColumn<T> &outputCol,
										RefRows &rowRef);

	void transformCubeOfData(	vi::VisBuffer2 *vb,
								RefRows &rowRef,
								const Cube<Complex> &inputDataCube,
								ArrayColumn<Complex> &outputDataCol,
								ArrayColumn<Bool> *outputFlagCol);
	void transformCubeOfData(	vi::VisBuffer2 *vb,
								RefRows &rowRef,
								const Cube<Float> &inputDataCube,
								ArrayColumn<Float> &outputDataCol,
								ArrayColumn<Bool> *outputFlagCol);
	void (casa::MSTransformManager::*transformCubeOfDataComplex_p)(	vi::VisBuffer2 *vb,
																		RefRows &rowRef,
																		const Cube<Complex> &inputDataCube,
																		ArrayColumn<Complex> &outputDataCol,
																		ArrayColumn<Bool> *outputFlagCol);
	void (casa::MSTransformManager::*transformCubeOfDataFloat_p)(	vi::VisBuffer2 *vb,
																		RefRows &rowRef,
																		const Cube<Float> &inputDataCube,
																		ArrayColumn<Float> &outputDataCol,
																		ArrayColumn<Bool> *outputFlagCol);

	template <class T> void copyCubeOfData(	vi::VisBuffer2 *vb,
											RefRows &rowRef,
											const Cube<T> &inputDataCube,
											ArrayColumn<T> &outputDataCol,
											ArrayColumn<Bool> *outputFlagCol);

	template <class T> void combineCubeOfData(	vi::VisBuffer2 *vb,
												RefRows &rowRef,
												const Cube<T> &inputDataCube,
												ArrayColumn<T> &outputDataCol,
												ArrayColumn<Bool> *outputFlagCol);

	void addWeightSpectrumContribution(	Double &weight,
										uInt &pol,
										uInt &inputChannel,
										uInt &row,
										Cube<Float> &inputWeightsCube);
	void dontAddWeightSpectrumContribution(	Double &weight,
											uInt &pol,
											uInt &inputChannel,
											uInt &row,
											Cube<Float> &inputWeightsCube);
	void (casa::MSTransformManager::*addWeightSpectrumContribution_p)(	Double &weight,
																			uInt &pol,
																			uInt &inputChannel,
																			uInt &row,
																			Cube<Float> &inputWeightsCube);


	void fillWeightsPlane(	uInt pol,
							uInt inputChannel,
							uInt outputChannel,
							uInt inputRow,
							const Cube<Float> &inputWeightsCube,
							Matrix<Float> &inputWeightsPlane,
							Double weight);
	void dontfillWeightsPlane(	uInt pol,
								uInt inputChannel,
								uInt outputChannel,
								uInt inputRow,
								const Cube<Float> &inputWeightsCube,
								Matrix<Float> &inputWeightsPlane,
								Double weight) {return;}
	void (casa::MSTransformManager::*fillWeightsPlane_p)(	uInt pol,
																uInt inputChannel,
																uInt outputChannel,
																uInt inputRow,
																const Cube<Float> &inputWeightsCube,
																Matrix<Float> &inputWeightsPlane,
																Double weight);

	void normalizeWeightsPlane(	uInt pol,
								uInt outputChannel,
								Matrix<Float> &inputPlaneWeights,
								Matrix<Double> &normalizingFactorPlane);
	void dontNormalizeWeightsPlane(	uInt pol,
									uInt outputChannel,
									Matrix<Float> &inputPlaneWeights,
									Matrix<Double> &normalizingFactorPlane) {return;}
	void (casa::MSTransformManager::*normalizeWeightsPlane_p)(	uInt pol,
																	uInt outputChannel,
																	Matrix<Float> &inputPlaneWeights,
																	Matrix<Double> &normalizingFactorPlane);

	template <class T> void averageCubeOfData(	vi::VisBuffer2 *vb,
												RefRows &rowRef,
												const Cube<T> &inputDataCube,
												ArrayColumn<T> &outputDataCol,
												ArrayColumn<Bool> *outputFlagCol);
	template <class T> void smoothCubeOfData(	vi::VisBuffer2 *vb,
												RefRows &rowRef,
												const Cube<T> &inputDataCube,
												ArrayColumn<T> &outputDataCol,
												ArrayColumn<Bool> *outputFlagCol);
	template <class T> void regridCubeOfData(	vi::VisBuffer2 *vb,
												RefRows &rowRef,
												const Cube<T> &inputDataCube,
												ArrayColumn<T> &outputDataCol,
												ArrayColumn<Bool> *outputFlagCol);
	template <class T> void separateCubeOfData(	vi::VisBuffer2 *vb,
												RefRows &rowRef,
												const Cube<T> &inputDataCube,
												ArrayColumn<T> &outputDataCol,
												ArrayColumn<Bool> *outputFlagCol);

	template <class T> void transformAndWriteCubeOfData(	Int inputSpw,
															RefRows &rowRef,
															const Cube<T> &inputDataCube,
															const Cube<Bool> &inputFlagsCube,
															const Cube<Float> &inputWeightsCube,
															IPosition &outputPlaneShape,
															ArrayColumn<T> &outputDataCol,
															ArrayColumn<Bool> *outputFlagCol);


	void setWeightsPlaneByReference(	uInt inputRow,
										const Cube<Float> &inputWeightsCube,
										Matrix<Float> &inputWeightsPlane);
	void dontsetWeightsPlaneByReference(	uInt inputRow,
											const Cube<Float> &inputWeightsCube,
											Matrix<Float> &inputWeightsPlane) {return;}
	void (casa::MSTransformManager::*setWeightsPlaneByReference_p)(	uInt inputRow,
																		const Cube<Float> &inputWeightsCube,
																		Matrix<Float> &inputWeightsPlane);

	template <class T> void transformAndWritePlaneOfData(	Int inputSpw,
															uInt row,
															Matrix<T> &inputDataPlane,
															Matrix<Bool> &inputFlagsPlane,
															Matrix<Float> &inputWeightsPlane,
															Matrix<T> &outputDataPlane,
															Matrix<Bool> &outputFlagsPlane,
															ArrayColumn<T> &outputDataCol,
															ArrayColumn<Bool> *outputFlagCol);
	void setWeightStripeByReference(	uInt corrIndex,
										Matrix<Float> &inputWeightsPlane,
										Vector<Float> &inputWeightsStripe);
	void dontSetWeightStripeByReference(	uInt corrIndex,
											Matrix<Float> &inputWeightsPlane,
											Vector<Float> &inputWeightsStripe) {return;}
	void (casa::MSTransformManager::*setWeightStripeByReference_p)(	uInt corrIndex,
																		Matrix<Float> &inputWeightsPlane,
																		Vector<Float> &inputWeightsStripe);

	void writeOutputPlanes(	uInt row,
							Matrix<Complex> &outputDataPlane,
							Matrix<Bool> &outputFlagsPlane,
							ArrayColumn<Complex> &outputDataCol,
							ArrayColumn<Bool> &outputFlagCol);
	void writeOutputPlanes(	uInt row,
							Matrix<Float> &outputDataPlane,
							Matrix<Bool> &outputFlagsPlane,
							ArrayColumn<Float> &outputDataCol,
							ArrayColumn<Bool> &outputFlagCol);
	void (casa::MSTransformManager::*writeOutputPlanesComplex_p)(	uInt row,
																		Matrix<Complex> &outputDataPlane,
																		Matrix<Bool> &outputFlagsPlane,
																		ArrayColumn<Complex> &outputDataCol,
																		ArrayColumn<Bool> &outputFlagCol);
	void (casa::MSTransformManager::*writeOutputPlanesFloat_p)(	uInt row,
																	Matrix<Float> &outputDataPlane,
																	Matrix<Bool> &outputFlagsPlane,
																	ArrayColumn<Float> &outputDataCol,
																	ArrayColumn<Bool> &outputFlagCol);

	template <class T> void writeOutputPlanesInBlock(	uInt row,
														Matrix<T> &outputDataPlane,
														Matrix<Bool> &outputFlagsPlane,
														ArrayColumn<T> &outputDataCol,
														ArrayColumn<Bool> &outputFlagCol);
	void (casa::MSTransformManager::*writeOutputFlagsPlane_p)(	Matrix<Bool> &outputPlane,
																	ArrayColumn<Bool> &outputCol,
																	IPosition &outputPlaneShape,
																	uInt &outputRow);
	void writeOutputFlagsPlane(	Matrix<Bool> &outputPlane,
								ArrayColumn<Bool> &outputCol,
								IPosition &outputPlaneShape,
								uInt &outputRow);
	void dontWriteOutputFlagsPlane(	Matrix<Bool> &outputPlane,
									ArrayColumn<Bool> &outputCol,
									IPosition &outputPlaneShape,
									uInt &outputRow) {return;}

	template <class T> void writeOutputPlanesInSlices(	uInt row,
														Matrix<T> &outputDataPlane,
														Matrix<Bool> &outputFlagsPlane,
														ArrayColumn<T> &outputDataCol,
														ArrayColumn<Bool> &outputFlagCol);
	template <class T> void writeOutputPlaneSlices(	Matrix<T> &outputPlane,
													ArrayColumn<T> &outputDataCol,
													Slice &sliceX,
													Slice &sliceY,
													IPosition &outputPlaneShape,
													uInt &outputRow);
	template <class T> void writeOutputPlaneReshapedSlices(	Matrix<T> &outputPlane,
															ArrayColumn<T> &outputDataCol,
															Slice &sliceX,
															Slice &sliceY,
															IPosition &outputPlaneShape,
															uInt &outputRow);
	void (casa::MSTransformManager::*writeOutputFlagsPlaneSlices_p)(	Matrix<Bool> &outputPlane,
																			ArrayColumn<Bool> &outputCol,
																			Slice &sliceX,
																			Slice &sliceY,
																			IPosition &outputPlaneShape,
																			uInt &outputRow);
	void writeOutputFlagsPlaneSlices(	Matrix<Bool> &outputPlane,
										ArrayColumn<Bool> &outputCol,
										Slice &sliceX,
										Slice &sliceY,
										IPosition &outputPlaneShape,
										uInt &outputRow);
	void dontWriteOutputFlagsPlaneSlices(	Matrix<Bool> &outputPlane,
											ArrayColumn<Bool> &outputDataCol,
											Slice &sliceX,
											Slice &sliceY,
											IPosition &outputPlaneShape,
											uInt &outputRow) {return;}
	void (casa::MSTransformManager::*writeOutputFlagsPlaneReshapedSlices_p)(	Matrix<Bool> &outputPlane,
																					ArrayColumn<Bool> &outputCol,
																					Slice &sliceX,
																					Slice &sliceY,
																					IPosition &outputPlaneShape,
																					uInt &outputRow);
	void writeOutputFlagsPlaneReshapedSlices(	Matrix<Bool> &outputPlane,
												ArrayColumn<Bool> &outputCol,
												Slice &sliceX,
												Slice &sliceY,
												IPosition &outputPlaneShape,
												uInt &outputRow);
	void dontWriteOutputPlaneReshapedSlices(	Matrix<Bool> &outputPlane,
												ArrayColumn<Bool> &outputDataCol,
												Slice &sliceX,
												Slice &sliceY,
												IPosition &outputPlaneShape,
												uInt &outputRow) {return;}

	void transformStripeOfData(	Int inputSpw,
								Vector<Complex> &inputDataStripe,
								Vector<Bool> &inputFlagsStripe,
								Vector<Float> &inputWeightsStripe,
								Vector<Complex> &outputDataStripe,
								Vector<Bool> &outputFlagsStripe);
	void transformStripeOfData(	Int inputSpw,
								Vector<Float> &inputDataStripe,
								Vector<Bool> &inputFlagsStripe,
								Vector<Float> &inputWeightsStripe,
								Vector<Float> &outputDataStripe,
								Vector<Bool> &outputFlagsStripe);
	void (casa::MSTransformManager::*transformStripeOfDataComplex_p)(	Int inputSpw,
																			Vector<Complex> &inputDataStripe,
																			Vector<Bool> &inputFlagsStripe,
																			Vector<Float> &inputWeightsStripe,
																			Vector<Complex> &outputDataStripe,
																			Vector<Bool> &outputFlagsStripe);
	void (casa::MSTransformManager::*transformStripeOfDataFloat_p)(	Int inputSpw,
																		Vector<Float> &inputDataStripe,
																		Vector<Bool> &inputFlagsStripe,
																		Vector<Float> &inputWeightsStripe,
																		Vector<Float> &outputDataStripe,
																		Vector<Bool> &outputFlagsStripe);

	template <class T> void average(	Int inputSpw,
										Vector<T> &inputDataStripe,
										Vector<Bool> &inputFlagsStripe,
										Vector<Float> &inputWeightsStripe,
										Vector<T> &outputDataStripe,
										Vector<Bool> &outputFlagsStripe);
	template <class T> void simpleAverage(	uInt width,
											Vector<T> &inputData,
											Vector<T> &outputData);
	void averageKernel(	Vector<Complex> &inputData,
						Vector<Bool> &inputFlags,
						Vector<Float> &inputWeights,
						Vector<Complex> &outputData,
						Vector<Bool> &outputFlags,
						uInt startInputPos,
						uInt outputPos,
						uInt width);
	void averageKernel(	Vector<Float> &inputData,
						Vector<Bool> &inputFlags,
						Vector<Float> &inputWeights,
						Vector<Float> &outputData,
						Vector<Bool> &outputFlags,
						uInt startInputPos,
						uInt outputPos,
						uInt width);
	void (casa::MSTransformManager::*averageKernelComplex_p)(	Vector<Complex> &inputData,
																	Vector<Bool> &inputFlags,
																	Vector<Float> &inputWeights,
																	Vector<Complex> &outputData,
																	Vector<Bool> &outputFlags,
																	uInt startInputPos,
																	uInt outputPos,
																	uInt width);
	void (casa::MSTransformManager::*averageKernelFloat_p)(		Vector<Float> &inputData,
																	Vector<Bool> &inputFlags,
																	Vector<Float> &inputWeights,
																	Vector<Float> &outputData,
																	Vector<Bool> &outputFlags,
																	uInt startInputPos,
																	uInt outputPos,
																	uInt width);
	template <class T> void simpleAverageKernel(	Vector<T> &inputData,
													Vector<Bool> &inputFlags,
													Vector<Float> &inputWeights,
													Vector<T> &outputData,
													Vector<Bool> &outputFlags,
													uInt startInputPos,
													uInt outputPos,
													uInt width);
	template <class T> void flagAverageKernel(	Vector<T> &inputData,
												Vector<Bool> &inputFlags,
												Vector<Float> &inputWeights,
												Vector<T> &outputData,
												Vector<Bool> &outputFlags,
												uInt startInputPos,
												uInt outputPos,
												uInt width);
	template <class T> void weightAverageKernel(	Vector<T> &inputData,
													Vector<Bool> &inputFlags,
													Vector<Float> &inputWeights,
													Vector<T> &outputData,
													Vector<Bool> &outputFlags,
													uInt startInputPos,
													uInt outputPos,
													uInt width);
	template <class T> void cumSumKernel(	Vector<T> &inputData,
											Vector<Bool> &inputFlags,
											Vector<Float> &inputWeights,
											Vector<T> &outputData,
											Vector<Bool> &outputFlags,
											uInt startInputPos,
											uInt outputPos,
											uInt width);

	template <class T> void smooth(	Int inputSpw,
									Vector<T> &inputDataStripe,
									Vector<Bool> &inputFlagsStripe,
									Vector<Float> &inputWeightsStripe,
									Vector<T> &outputDataStripe,
									Vector<Bool> &outputFlagsStripe);

	template <class T> void regrid(	Int inputSpw,
									Vector<T> &inputDataStripe,
									Vector<Bool> &inputFlagsStripe,
									Vector<Float> &inputWeightsStripe,
									Vector<T> &outputDataStripe,
									Vector<Bool> &outputFlagsStripe);

	void regridCore(	Int inputSpw,
						Vector<Complex> &inputDataStripe,
						Vector<Bool> &inputFlagsStripe,
						Vector<Float> &inputWeightsStripe,
						Vector<Complex> &outputDataStripe,
						Vector<Bool> &outputFlagsStripe);
	void regridCore(	Int inputSpw,
						Vector<Float> &inputDataStripe,
						Vector<Bool> &inputFlagsStripe,
						Vector<Float> &inputWeightsStripe,
						Vector<Float> &outputDataStripe,
						Vector<Bool> &outputFlagsStripe);

	void (casa::MSTransformManager::*regridCoreComplex_p)(		Int inputSpw,
																	Vector<Complex> &inputDataStripe,
																	Vector<Bool> &inputFlagsStripe,
																	Vector<Float> &inputWeightsStripe,
																	Vector<Complex> &outputDataStripe,
																	Vector<Bool> &outputFlagsStripe);
	void (casa::MSTransformManager::*regridCoreFloat_p)(	Int inputSpw,
																Vector<Float> &inputDataStripe,
																Vector<Bool> &inputFlagsStripe,
																Vector<Float> &inputWeightsStripe,
																Vector<Float> &outputDataStripe,
																Vector<Bool> &outputFlagsStripe);

	void fftshift(	Int inputSpw,
					Vector<Complex> &inputDataStripe,
					Vector<Bool> &inputFlagsStripe,
					Vector<Float> &inputWeightsStripe,
					Vector<Complex> &outputDataStripe,
					Vector<Bool> &outputFlagsStripe);
	void fftshift(	Int inputSpw,
					Vector<Float> &inputDataStripe,
					Vector<Bool> &inputFlagsStripe,
					Vector<Float> &inputWeightsStripe,
					Vector<Float> &outputDataStripe,
					Vector<Bool> &outputFlagsStripe);

	template <class T> void interpol1D(	Int inputSpw,
										Vector<T> &inputDataStripe,
										Vector<Bool> &inputFlagsStripe,
										Vector<Float> &inputWeightsStripe,
										Vector<T> &outputDataStripe,
										Vector<Bool> &outputFlagsStripe);

	template <class T> void interpol1Dfftshift(	Int inputSpw,
												Vector<T> &inputDataStripe,
												Vector<Bool> &inputFlagsStripe,
												Vector<Float> &inputWeightsStripe,
												Vector<T> &outputDataStripe,
												Vector<Bool> &outputFlagsStripe);

	template <class T> void averageSmooth(	Int inputSpw,
											Vector<T> &inputDataStripe,
											Vector<Bool> &inputFlagsStripe,
											Vector<Float> &inputWeightsStripe,
											Vector<T> &outputDataStripe,
											Vector<Bool> &outputFlagsStripe);
	template <class T> void averageRegrid(	Int inputSpw,
											Vector<T> &inputDataStripe,
											Vector<Bool> &inputFlagsStripe,
											Vector<Float> &inputWeightsStripe,
											Vector<T> &outputDataStripe,
											Vector<Bool> &outputFlagsStripe);
	template <class T> void smoothRegrid(	Int inputSpw,
											Vector<T> &inputDataStripe,
											Vector<Bool> &inputFlagsStripe,
											Vector<Float> &inputWeightsStripe,
											Vector<T> &outputDataStripe,
											Vector<Bool> &outputFlagsStripe);
	template <class T> void averageSmoothRegrid(	Int inputSpw,
													Vector<T> &inputDataStripe,
													Vector<Bool> &inputFlagsStripe,
													Vector<Float> &inputWeightsStripe,
													Vector<T> &outputDataStripe,
													Vector<Bool> &outputFlagsStripe);

	// MS specification parameters
	String inpMsName_p;
	String outMsName_p;
	String datacolumn_p;
	Bool realmodelcol_p;
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
	map<Int,Int> inputOutputDDIndexMap_p;
	map<Int,Int> inputOutputAntennaIndexMap_p;
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
	uInt weightmode_p;
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
	Bool timeAverage_p;
	Double timeBin_p;
	String timespan_p;
	vi::AveragingOptions timeAvgOptions_p;
	Double maxuvwdistance_p;
//	uInt minbaselines_p;

	// Weight Spectrum parameters
	Bool usewtspectrum_p;

	// MS-related members
	MSTransformDataHandler *dataHandler_p;
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
	Bool correctedToData_p;
	dataColMap dataColMap_p;
	MSMainEnums::PredefinedColumns mainColumn_p;

	// Frequency transformation members
	uInt chansPerOutputSpw_p;
	uInt tailOfChansforLastSpw_p;
	uInt interpolationMethod_p;
	baselineMap baselineMap_p;
	vector<uInt> rowIndex_p;
	inputSpwChanMap spwChannelMap_p;
	inputOutputSpwMap inputOutputSpwMap_p;
	inputOutputChanFactorMap inputOutputChanFactorMap_p;
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
	MEpoch referenceTime_p;
	MDirection phaseCenter_p;
	Bool userPhaseCenter_p;
	MFrequency::Convert freqTransEngine_p;
	MFrequency::Convert refTimeFreqTransEngine_p;
    FFTServer<Float, Complex> fFFTServer_p;
    Bool fftShiftEnabled_p;
	Double fftShift_p;
	ROScalarMeasColumn<MEpoch> timeMeas_p;

	// Weight Spectrum members
	Bool inputWeightSpectrumAvailable_p;
	Bool combinationOfSPWsWithDifferentExposure_p;
	Cube<Float> weightSpectrumCube_p;

	// Logging
	LogIO logger_p;
};

} //# NAMESPACE CASA - END

#endif /* MSTransformManager_H_ */
