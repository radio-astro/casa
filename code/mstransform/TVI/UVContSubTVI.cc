//# UVContSubTVI.h: This file contains the implementation of the UVContSubTVI class.
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

#include <mstransform/TVI/UVContSubTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// UVContSubTVI class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
UVContSubTVI::UVContSubTVI(	ViImplementation2 * inputVii,
								const Record &configuration):
								FreqAxisTVI (inputVii,configuration)
{
	fitOrder_p = 0;
	want_cont_p = False;
	fitspw_p = String("");
	inputFrequencyMap_p.clear();

	// Parse and check configuration parameters
	// Note: if a constructor finishes by throwing an exception, the memory
	// associated with the object itself is cleaned up — there is no memory leak.
	if (not parseConfiguration(configuration))
	{
		throw AipsError("Error parsing UVContSubTVI configuration");
	}

	initialize();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool UVContSubTVI::parseConfiguration(const Record &configuration)
{
	int exists = 0;
	Bool ret = True;

	exists = configuration.fieldNumber ("fitorder");
	if (exists >= 0)
	{
		configuration.get (exists, fitOrder_p);

		logger_p 	<< LogIO::NORMAL << LogOrigin("UVContSubTVI", __FUNCTION__)
					<< "Fit order is " << fitOrder_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("want_cont");
	if (exists >= 0)
	{
		configuration.get (exists, want_cont_p);

		logger_p 	<< LogIO::NORMAL << LogOrigin("UVContSubTVI", __FUNCTION__)
					<< "want_cont is " << want_cont_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("fitspw");
	if (exists >= 0)
	{
		configuration.get (exists, fitspw_p);

		logger_p 	<< LogIO::NORMAL << LogOrigin("UVContSubTVI", __FUNCTION__)
					<< "Line-free channel selection is " << fitspw_p << LogIO::POST;
	}

	return ret;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVI::initialize()
{
	// Populate nchan input-output maps
	Int spw;
	uInt spw_idx = 0;
	map<Int,vector<Int> >::iterator iter;
	for(iter=spwInpChanIdxMap_p.begin();iter!=spwInpChanIdxMap_p.end();iter++)
	{
		spw = iter->first;
		spwOutChanNumMap_p[spw] = spwInpChanIdxMap_p[spw].size();

		spw_idx++;
	}

	// Initialize fitting model (we assume there is always enough data for the requested order)
	Polynomial<AutoDiff<Float> > poly(fitOrder_p);
	fitter_p.setFunction(poly);

	if (fitspw_p.size() > 0)
	{
		// Parse line-free channel selection
		MSSelection mssel;
		mssel.setSpwExpr(fitspw_p);
		Matrix<Int> spwchan = mssel.getChanList(&(inputVii_p->ms()));

		// Create line-free channel map
	    uInt nSelections = spwchan.shape()[0];
	    map<Int,vector<Int> > lineFreeChannelMap;
		Int channelStart,channelStop,channelStep;
		for(uInt selection_i=0;selection_i<nSelections;selection_i++)
		{
			spw = spwchan(selection_i,0);
			channelStart = spwchan(selection_i,1);
			channelStop = spwchan(selection_i,2);
			channelStep = spwchan(selection_i,3);

			if (lineFreeChannelMap.find(spw) == lineFreeChannelMap.end())
			{
				lineFreeChannelMap[spw].clear(); // Accessing the vector creates it
			}

			for (Int inpChan=channelStart;inpChan<=channelStop;inpChan += channelStep)
			{
				lineFreeChannelMap[spw].push_back(inpChan);
			}
		}


		// Create line-free channel mask
		uInt selChan;
		for(iter=spwInpChanIdxMap_p.begin();iter!=spwInpChanIdxMap_p.end();iter++)
		{
			spw = iter->first;
			if (lineFreeChannelMaskMap_p.find(spw) == lineFreeChannelMaskMap_p.end())
			{
				lineFreeChannelMaskMap_p[spw] = Vector<Bool>(spwInpChanIdxMap_p[spw].size(),True);
				for (uInt selChanIdx=0;selChanIdx<lineFreeChannelMap[spw].size();selChanIdx++)
				{
					selChan = lineFreeChannelMap[spw][selChanIdx];
					lineFreeChannelMaskMap_p[spw](selChan) = False;
				}
			}
			spw_idx++;
		}

	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVI::floatData (Cube<Float> & vis) const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Transform data
	transformDataCube(vb->visCubeFloat(),vb->weightSpectrum(),True,vis);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVI::visibilityObserved (Cube<Complex> & vis) const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Transform data
	transformDataCube(vb->visCube(),vb->sigmaSpectrum(),False,vis);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVI::visibilityCorrected (Cube<Complex> & vis) const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();


	// Transform data
	transformDataCube(vb->visCubeCorrected(),vb->weightSpectrum(),True,vis);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVI::visibilityModel (Cube<Complex> & vis) const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Transform data
	transformDataCube(vb->visCubeModel(),vb->weightSpectrum(),True,vis);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void UVContSubTVI::transformDataCube(	const Cube<T> &inputVis,
														const Cube<Float> &inputSigma,
														Bool sigmaAsWeight,
														Cube<T> &outputVis) const
{
	// Get input VisBuffer
	VisBuffer2 *vb = getVii()->getVisBuffer();

	// Convert input frequencies to Float (required for the templated fitter class LinearFitSVD<Float>)
	Int spwId = vb->spectralWindows()(0);
	if (inputFrequencyMap_p.find(spwId) == inputFrequencyMap_p.end())
	{
		const Vector<Double> &inputFrequencies = vb->getFrequencies(0);
		inputFrequencyMap_p[spwId] = Matrix<Float>(fitOrder_p==0? 1:fitOrder_p,inputFrequencies.size(),0);

		Double midfreq,lofreq,hifreq,freqscale;
		lofreq = inputFrequencies(0);
		hifreq = inputFrequencies(inputFrequencies.size()-1);
		midfreq = 0.5 * (lofreq + hifreq);
		freqscale = 1.0 / (hifreq - midfreq);
		// First row is always input frequencies
		for (uInt chan_idx = 0; chan_idx < inputFrequencies.size(); chan_idx++)
		{
			//inputFrequencyMap_p[spwId](0,chan_idx) = inputFrequencies(chan_idx);
			//inputFrequencyMap_p[spwId](0,chan_idx) = -1.0 + chan_idx*(2.0/(inputFrequencies.size()-1));
			inputFrequencyMap_p[spwId](0,chan_idx) = freqscale*(inputFrequencies(chan_idx)-midfreq);
		}

		// For fit order 2 and above we calculate pows
		for (uInt order_idx=2;order_idx<=fitOrder_p;order_idx++)
		{
			for (uInt chan_idx = 0; chan_idx < inputFrequencies.size(); chan_idx++)
			{
				inputFrequencyMap_p[spwId](order_idx-1,chan_idx) = pow(inputFrequencyMap_p[spwId](0,chan_idx),order_idx);
			}
		}
	}

	// Get input line-free channel mask
	Vector<Bool> *lineFreeChannelMask;
	if (lineFreeChannelMaskMap_p.find(spwId) != lineFreeChannelMaskMap_p.end())
	{
		lineFreeChannelMask = &(lineFreeChannelMaskMap_p[spwId]);
	}
	else
	{
		lineFreeChannelMask = NULL;
	}

	// Reshape output data before passing it to the DataCubeHolder
	outputVis.resize(getVisBufferConst()->getShape(),False);

	// Gather input data
	DataCubeMap inputData;
	DataCubeHolder<T> inputVisCubeHolder(inputVis);
	DataCubeHolder<Float> inputSigmaCubeHolder(inputSigma);
	DataCubeHolder<Bool> inputFlagCubeHolder(vb->flagCube());
	inputData.add(MS::DATA,inputVisCubeHolder);
	inputData.add(MS::SIGMA,inputSigmaCubeHolder);
	inputData.add(MS::FLAG,inputFlagCubeHolder);

	// Gather output data
	DataCubeMap outputData;
	DataCubeHolder<T> outputVisCubeHolder(outputVis);
	outputData.add(MS::DATA,outputVisCubeHolder);

	// Configure transformation engine and transform data
	fitter_p.asWeight(sigmaAsWeight); // We are using sigma spectrum
	if (want_cont_p)
	{
		UVContEstimationKernel<T> kernel(fitOrder_p,&fitter_p,&(inputFrequencyMap_p[spwId]),lineFreeChannelMask);
		UVContSubTransformEngine<T> transformer(&kernel,&inputData,&outputData);
		transformFreqAxis2(vb->getShape(),transformer);
	}
	else
	{
		UVContSubtractionKernel<T> kernel(fitOrder_p,&fitter_p,&(inputFrequencyMap_p[spwId]),lineFreeChannelMask);
		UVContSubTransformEngine<T> transformer(&kernel,&inputData,&outputData);
		transformFreqAxis2(vb->getShape(),transformer);
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// UVContSubTVIFactory class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
UVContSubTVIFactory::UVContSubTVIFactory (	Record &configuration,
											ViImplementation2 *inputVii)
{
	inputVii_p = inputVii;
	configuration_p = configuration;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * UVContSubTVIFactory::createVi(VisibilityIterator2 *) const
{
	return new UVContSubTVI(inputVii_p,configuration_p);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
vi::ViImplementation2 * UVContSubTVIFactory::createVi() const
{
	return new UVContSubTVI(inputVii_p,configuration_p);
}

//////////////////////////////////////////////////////////////////////////
// UVContSubTransformEngine class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> UVContSubTransformEngine<T>::UVContSubTransformEngine(	UVContSubKernel<T> *kernel,
																			DataCubeMap *inputData,
																			DataCubeMap *outputData):
															FreqAxisTransformEngine2<T>(inputData,outputData)
{
	uvContSubKernel_p = kernel;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void UVContSubTransformEngine<T>::transform(	)
{
	uvContSubKernel_p->setDebug(debug_p);
	uvContSubKernel_p->kernel(inputData_p,outputData_p);
}

//////////////////////////////////////////////////////////////////////////
// UVContSubKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> UVContSubKernel<T>::UVContSubKernel(	uInt fitOrder,
														LinearFitSVD<Float> *fitter,
														Matrix<Float> *freqPows,
														Vector<Bool> *lineFreeChannelMask)
{
	fitter_p = fitter;
	fitOrder_p = fitOrder;
	freqPows_p = freqPows;
	frequencies_p.reference(freqPows->row(0));
	lineFreeChannelMask_p = lineFreeChannelMask != NULL? lineFreeChannelMask : NULL;
	debug_p = False;
}


//////////////////////////////////////////////////////////////////////////
// UVContSubtractionKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> UVContSubtractionKernel<T>::UVContSubtractionKernel(	uInt fitOrder,
																		LinearFitSVD<Float> *fitter,
																		Matrix<Float> *freqPows,
																		Vector<Bool> *lineFreeChannelMask):
																		UVContSubKernel<T>(fitOrder,fitter,
																				freqPows,lineFreeChannelMask)
{

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void UVContSubtractionKernel<T>::kernel(	DataCubeMap *inputData,
															DataCubeMap *outputData)
{
	// Get input/output data
	Vector<T> &outputVector = outputData->getVector<T>(MS::DATA);
	Vector<T> &inputVector = inputData->getVector<T>(MS::DATA);

	// Apply line-free channel mask
	Vector<Bool> &inputFlags = inputData->getVector<Bool>(MS::FLAG);
	if (lineFreeChannelMask_p != NULL) inputFlags |= *lineFreeChannelMask_p;

	// Calculate number of valid data points and adapt fit
	size_t validPoints = nfalse(inputFlags);
	if (validPoints > 0)
	{
		Bool restoreDefaultPoly = False;
		uInt tmpFitOrder = fitOrder_p;

		// Reduce fit order to match number of valid points
		if (validPoints <= fitOrder_p)
		{
			fitOrder_p = validPoints-1;
			Polynomial<AutoDiff<Float> > poly(fitOrder_p);
			fitter_p->setFunction(poly);
			restoreDefaultPoly = True;
		}

		// Get weights
		Vector<Float> &inputSigma = inputData->getVector<Float>(MS::SIGMA);

		// Convert flags to mask
		Vector<Bool> mask = !inputFlags;

		// Calculate and subtract continuum
		kernelCore(inputVector,mask,inputSigma,outputVector);

		// Go back to default fit order to match number of valid points
		if (restoreDefaultPoly)
		{
			fitOrder_p = tmpFitOrder;
			Polynomial<AutoDiff<Float> > poly(fitOrder_p);
			fitter_p->setFunction(poly);
		}
	}
	else
	{
		// Continuum is 0, therefore output data equals input data
		outputVector = inputVector;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void UVContSubtractionKernel<T>::kernelCore(	Vector<Complex> &inputVector,
																Vector<Bool> &inputFlags,
																Vector<Float> &inputSigma,
																Vector<Complex> &outputVector)
{
	// Fit for imaginary and real components separately
	Vector<Float> realCoeff;
	Vector<Float> imagCoeff;
	realCoeff = fitter_p->fit(frequencies_p, real(inputVector), inputSigma, &inputFlags);
	imagCoeff = fitter_p->fit(frequencies_p, imag(inputVector), inputSigma, &inputFlags);

	// Fill output data
	outputVector = inputVector;
	outputVector -= Complex(realCoeff(0),imagCoeff(0));
	for (uInt order_idx = 1; order_idx <= fitOrder_p; order_idx++)
	{
		Complex coeff(realCoeff(order_idx),imagCoeff(order_idx));
		for (uInt chan_idx=0; chan_idx < outputVector.size(); chan_idx++)
		{
			outputVector(chan_idx) -= ((*freqPows_p)(order_idx-1,chan_idx))*coeff;
		}
	}

	// jagonzal: Debug code
	/*
	if (debug_p)
	{
		LogIO logger;
		logger << "frequencies_p=" << frequencies_p << LogIO::POST;
		logger << "inputSigma=" << inputSigma << LogIO::POST;
		logger << "inputFlags=" << inputFlags << LogIO::POST;
		logger << "real(inputVector)=" << real(inputVector) << LogIO::POST;
		logger << "realCoeff=" << realCoeff << LogIO::POST;
		logger << "imag(inputVector)=" << imag(inputVector) << LogIO::POST;
		logger << "imagCoeff=" << imagCoeff << LogIO::POST;
		logger << "outputVector=" << outputVector << LogIO::POST;
	}
	*/


	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void UVContSubtractionKernel<T>::kernelCore(	Vector<Float> &inputVector,
																Vector<Bool> &inputFlags,
																Vector<Float> &inputSigma,
																Vector<Float> &outputVector)
{
	// Fit model
	Vector<Float> coeff;
	coeff = fitter_p->fit(frequencies_p, inputVector, inputSigma, &inputFlags);

	// Fill output data
	outputVector = inputVector;
	outputVector -= coeff(0);
	for (uInt order_idx = 1; order_idx <= fitOrder_p; order_idx++)
	{
		for (uInt chan_idx=0; chan_idx < outputVector.size(); chan_idx++)
		{
			outputVector(chan_idx) -= ((*freqPows_p)(order_idx-1,chan_idx))*coeff(order_idx);
		}
	}

	return;
}


//////////////////////////////////////////////////////////////////////////
// UVContEstimationKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> UVContEstimationKernel<T>::UVContEstimationKernel(	uInt fitOrder,
																		LinearFitSVD<Float> *fitter,
																		Matrix<Float> *freqPows,
																		Vector<Bool> *lineFreeChannelMask):
																		UVContSubKernel<T>(fitOrder,fitter,
																				freqPows,lineFreeChannelMask)
{

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void UVContEstimationKernel<T>::kernel(	DataCubeMap *inputData,
															DataCubeMap *outputData)
{
	// Get input/output data
	Vector<T> &outputVector = outputData->getVector<T>(MS::DATA);
	Vector<T> &inputVector = inputData->getVector<T>(MS::DATA);

	// Apply line-free channel mask
	Vector<Bool> &inputFlags = inputData->getVector<Bool>(MS::FLAG);
	if (lineFreeChannelMask_p != NULL) inputFlags |= *lineFreeChannelMask_p;

	// Calculate number of valid data points and adapt fit
	size_t validPoints = nfalse(inputFlags);
	if (validPoints > 0)
	{
		Bool restoreDefaultPoly = False;
		uInt tmpFitOrder = fitOrder_p;

		// Reduce fit order to match number of valid points
		if (validPoints <= fitOrder_p)
		{
			fitOrder_p = validPoints-1;
			Polynomial<AutoDiff<Float> > poly(fitOrder_p);
			fitter_p->setFunction(poly);
			restoreDefaultPoly = True;
		}

		// Get weights
		Vector<Float> &inputSigma = inputData->getVector<Float>(MS::SIGMA);

		// Convert flags to mask
		Vector<Bool> mask = !inputFlags;

		// Calculate and subtract continuum
		kernelCore(inputVector,mask,inputSigma,outputVector);

		// Go back to default fit order to match number of valid points
		if (restoreDefaultPoly)
		{
			fitOrder_p = tmpFitOrder;
			Polynomial<AutoDiff<Float> > poly(fitOrder_p);
			fitter_p->setFunction(poly);
		}
	}
	else
	{
		// Continuum is 0, therefore output data equals input data
		outputVector = 0;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void UVContEstimationKernel<T>::kernelCore(	Vector<Complex> &inputVector,
																Vector<Bool> &inputFlags,
																Vector<Float> &inputSigma,
																Vector<Complex> &outputVector)
{
	// Fit for imaginary and real components separately
	Vector<Float> realCoeff;
	Vector<Float> imagCoeff;
	realCoeff = fitter_p->fit(frequencies_p, real(inputVector), inputSigma, &inputFlags);
	imagCoeff = fitter_p->fit(frequencies_p, imag(inputVector), inputSigma, &inputFlags);

	// Fill output data
	outputVector = Complex(realCoeff(0),imagCoeff(0));
	for (uInt order_idx = 1; order_idx <= fitOrder_p; order_idx++)
	{
		Complex coeff(realCoeff(order_idx),imagCoeff(order_idx));
		for (uInt chan_idx=0; chan_idx < outputVector.size(); chan_idx++)
		{
			outputVector(chan_idx) += ((*freqPows_p)(order_idx-1,chan_idx))*coeff;
		}
	}

	// jagonzal: Debug code
	/*
	if (debug_p)
	{
		LogIO logger;
		logger << "frequencies_p=" << frequencies_p << LogIO::POST;
		logger << "inputSigma=" << inputSigma << LogIO::POST;
		logger << "inputFlags=" << inputFlags << LogIO::POST;
		logger << "real(inputVector)=" << real(inputVector) << LogIO::POST;
		logger << "realCoeff=" << realCoeff << LogIO::POST;
		logger << "imag(inputVector)=" << imag(inputVector) << LogIO::POST;
		logger << "imagCoeff=" << imagCoeff << LogIO::POST;
		logger << "outputVector=" << outputVector << LogIO::POST;
	}
	*/


	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void UVContEstimationKernel<T>::kernelCore(	Vector<Float> &inputVector,
																Vector<Bool> &inputFlags,
																Vector<Float> &inputSigma,
																Vector<Float> &outputVector)
{
	// Fit model
	Vector<Float> coeff;
	coeff = fitter_p->fit(frequencies_p, inputVector, inputSigma, &inputFlags);

	// Fill output data
	outputVector = coeff(0);
	for (uInt order_idx = 1; order_idx <= fitOrder_p; order_idx++)
	{
		for (uInt chan_idx=0; chan_idx < outputVector.size(); chan_idx++)
		{
			outputVector(chan_idx) += ((*freqPows_p)(order_idx-1,chan_idx))*coeff(order_idx);
		}
	}

	return;
}

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


