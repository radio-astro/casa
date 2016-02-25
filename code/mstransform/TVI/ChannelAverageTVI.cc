//# ChannelAverageTVI.h: This file contains the implementation of the ChannelAverageTVI class.
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

#include <mstransform/TVI/ChannelAverageTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVI class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
ChannelAverageTVI::ChannelAverageTVI(	ViImplementation2 * inputVii,
										const Record &configuration):
										DecimationTVI (inputVii,configuration)
{
	// Parse and check configuration parameters
	// Note: if a constructor finishes by throwing an exception, the memory
	// associated with the object itself is cleaned up — there is no memory leak.
	if (not parseConfiguration(configuration))
	{
		throw AipsError("Error parsing ChannelAverageTVI configuration");
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::flag(Cube<Bool>& flagCube) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure Transformation Engine
	LogicalANDKernel<Bool> kernel;
	uInt width = spwChanbinMap_p[inputSPW];
	DecimationTransformEngine<Bool> transformer(&(kernel),width);

	// Transform data
	transformFreqAxis(vb->flagCube(),flagCube,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::floatData (Cube<Float> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure Transformation Engine
	WeightedChannelAverageKernel<Float> kernel;
	uInt width = spwChanbinMap_p[inputSPW];
	DecimationTransformEngine<Float> transformer(&(kernel),width);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	DataCubeHolder<Float> weightCubeHolder(vb->weightSpectrum());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);
	auxiliaryData.add(MS::WEIGHT_SPECTRUM,weightCubeHolder);

	// Transform data
	transformFreqAxis(vb->visCubeFloat(),vis,auxiliaryData,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::visibilityObserved (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure Transformation Engine
	WeightedChannelAverageKernel<Complex> kernel;
	uInt width = spwChanbinMap_p[inputSPW];
	DecimationTransformEngine<Complex> transformer(&(kernel),width);

	// Get weightSpectrum from sigmaSpectrum
	Cube<Float> weightSpFromSigmaSp;
	weightSpFromSigmaSp.resize(vb->sigmaSpectrum().shape(),False);
	weightSpFromSigmaSp = vb->sigmaSpectrum(); // = Operator makes a copy
	arrayTransformInPlace (weightSpFromSigmaSp,sigmaToWeight);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	DataCubeHolder<Float> weightCubeHolder(weightSpFromSigmaSp);
	auxiliaryData.add(MS::FLAG,flagCubeHolder);
	auxiliaryData.add(MS::WEIGHT_SPECTRUM,weightCubeHolder);

	// Transform data
	transformFreqAxis(vb->visCube(),vis,auxiliaryData,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::visibilityCorrected (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure Transformation Engine
	WeightedChannelAverageKernel<Complex> kernel;
	uInt width = spwChanbinMap_p[inputSPW];
	DecimationTransformEngine<Complex> transformer(&(kernel),width);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	DataCubeHolder<Float> weightCubeHolder(vb->weightSpectrum());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);
	auxiliaryData.add(MS::WEIGHT_SPECTRUM,weightCubeHolder);

	// Transform data
	transformFreqAxis(vb->visCubeCorrected(),vis,auxiliaryData,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::visibilityModel (Cube<Complex> & vis) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure Transformation Engine
	WeightedChannelAverageKernel<Complex> kernel;
	uInt width = spwChanbinMap_p[inputSPW];
	DecimationTransformEngine<Complex> transformer(&(kernel),width);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	DataCubeHolder<Float> weightCubeHolder(vb->weightSpectrum());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);
	auxiliaryData.add(MS::WEIGHT_SPECTRUM,weightCubeHolder);

	// Transform data
	transformFreqAxis(vb->visCubeModel(),vis,auxiliaryData,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::weightSpectrum(Cube<Float> &weightSp) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure Transformation Engine
	ChannelAccumulationKernel<Float> kernel;
	uInt width = spwChanbinMap_p[inputSPW];
	DecimationTransformEngine<Float> transformer(&(kernel),width);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);

	// Transform data
	transformFreqAxis(vb->weightSpectrum(),weightSp,auxiliaryData,transformer);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVI::sigmaSpectrum(Cube<Float> &sigmaSp) const
{
	// Get input VisBuffer and SPW
	VisBuffer2 *vb = getVii()->getVisBuffer();
	Int inputSPW = vb->spectralWindows()(0);

	// Configure Transformation Engine
	ChannelAccumulationKernel<Float> kernel;
	uInt width = spwChanbinMap_p[inputSPW];
	DecimationTransformEngine<Float> transformer(&(kernel),width);

	// Get weightSpectrum from sigmaSpectrum
	Cube<Float> weightSpFromSigmaSp;
	weightSpFromSigmaSp.resize(vb->sigmaSpectrum().shape(),False);
	weightSpFromSigmaSp = vb->sigmaSpectrum(); // = Operator makes a copy
	arrayTransformInPlace (weightSpFromSigmaSp,sigmaToWeight);

	// Configure auxiliary data
	DataCubeMap auxiliaryData;
	DataCubeHolder<Bool> flagCubeHolder(vb->flagCube());
	auxiliaryData.add(MS::FLAG,flagCubeHolder);

	// Transform data
	transformFreqAxis(weightSpFromSigmaSp,sigmaSp,auxiliaryData,transformer);

	// Transform back from weight format to sigma format
	arrayTransformInPlace (sigmaSp,weightToSigma);

	return;
}

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVIFactory class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
ChannelAverageTVIFactory::ChannelAverageTVIFactory (Record &configuration,
													ViImplementation2 *inputVii)
{
	inputVii_p = inputVii;
	configuration_p = configuration;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * ChannelAverageTVIFactory::createVi(VisibilityIterator2 *) const
{
	return new ChannelAverageTVI(inputVii_p,configuration_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * ChannelAverageTVIFactory::createVi() const
{
	return new ChannelAverageTVI(inputVii_p,configuration_p);
}

//////////////////////////////////////////////////////////////////////////
// WeightedChannelAverageKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void WeightedChannelAverageKernel<T>::kernel(	Vector<T> &inputVector,
																Vector<T> &outputVector,
																DataCubeMap &auxiliaryData,
																uInt startInputPos,
																uInt outputPos,
																uInt width)
{
	T avg = 0;
	T normalization = 0;
	uInt inputPos = 0;
	Vector<Bool> &inputFlagVector = auxiliaryData.getVector<Bool>(MS::FLAG);
	Vector<Float> &inputWeightVector = auxiliaryData.getVector<Float>(MS::WEIGHT_SPECTRUM);
	Bool accumulatorFlag = inputFlagVector(startInputPos);

	for (uInt sample_i=0;sample_i<width;sample_i++)
	{
		// Get input index
		inputPos = startInputPos + sample_i;

		// True/True or False/False
		if (accumulatorFlag == inputFlagVector(inputPos))
		{
			normalization += inputWeightVector(inputPos);
			avg += inputVector(inputPos)*inputWeightVector(inputPos);
		}
		// True/False: Reset accumulation when accumulator switches from flagged to unflag
		else if ( (accumulatorFlag == True) and (inputFlagVector(inputPos) == False) )
		{
			accumulatorFlag = False;
			normalization = inputWeightVector(inputPos);
			avg = inputVector(inputPos)*inputWeightVector(inputPos);
		}
	}


	// Apply normalization factor
	if (normalization > 0)
	{
		avg /= normalization;
		outputVector(outputPos) = avg;
	}
	// If all weights are zero set accumulatorFlag to True
	else
	{
		accumulatorFlag = True;
		outputVector(outputPos) = 0; // If all weights are zero then the avg is 0 too
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// LogicalANDKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void LogicalANDKernel<T>::kernel(	Vector<T> &inputVector,
													Vector<T> &outputVector,
													DataCubeMap &,
													uInt startInputPos,
													uInt outputPos,
													uInt width)
{
	Bool outputFlag = True;
	for (uInt sample_i=0;sample_i<width;sample_i++)
	{
		if (not inputVector(startInputPos+sample_i))
		{
			outputFlag = False;
			break;
		}
	}

	outputVector(outputPos) = outputFlag;

	return;
}

//////////////////////////////////////////////////////////////////////////
// ChannelAccumulationKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void ChannelAccumulationKernel<T>::kernel(	Vector<T> &inputVector,
																Vector<T> &outputVector,
																DataCubeMap &auxiliaryData,
																uInt startInputPos,
																uInt outputPos,
																uInt width)
{
	T acc = 0;
	uInt inputPos = 0;
	Vector<Bool> &inputFlagVector = auxiliaryData.getVector<Bool>(MS::FLAG);
	Bool accumulatorFlag = inputFlagVector(startInputPos);

	for (uInt sample_i=0;sample_i<width;sample_i++)
	{
		// Get input index
		inputPos = startInputPos + sample_i;

		// True/True or False/False
		if (accumulatorFlag == inputFlagVector(inputPos))
		{
			acc += inputVector(inputPos);
		}
		// True/False: Reset accumulation when accumulator switches from flagged to unflag
		else if ( (accumulatorFlag == True) and (inputFlagVector(inputPos) == False) )
		{
			accumulatorFlag = False;
			acc = inputVector(inputPos);
		}
	}


	outputVector(outputPos) = acc;

	return;
}

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


