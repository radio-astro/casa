//# DecimationTVI.h: This file contains the implementation of the DecimationTVI class.
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

#include <mstransform/TVI/DecimationTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// DecimationTVI class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
DecimationTVI::DecimationTVI(	ViImplementation2 * inputVii,
								const Record &configuration):
								FreqAxisTVI (inputVii,configuration)
{
	// Parse and check configuration parameters
	// Note: if a constructor finishes by throwing an exception, the memory
	// associated with the object itself is cleaned up — there is no memory leak.
	if (not parseConfiguration(configuration))
	{
		throw AipsError("Error parsing DecimationTVI configuration");
	}

	initialize();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool DecimationTVI::parseConfiguration(const Record &configuration)
{
	int exists = 0;
	Bool ret = True;

	// Parse chanbin parameter (mandatory)
	exists = configuration.fieldNumber ("chanbin");
	if (exists >= 0)
	{
		if ( configuration.type(exists) == casa::TpInt )
		{
			Int freqbin;
			configuration.get (exists, freqbin);
			chanbin_p = Vector<Int>(spwInpChanIdxMap_p.size(),freqbin);
		}
		else if ( configuration.type(exists) == casa::TpArrayInt)
		{
			configuration.get (exists, chanbin_p);
		}
		else
		{
			ret = False;
			logger_p << LogIO::SEVERE << LogOrigin("DecimationTVI", __FUNCTION__)
					<< "Wrong format for chanbin parameter (only Int and arrayInt are supported) "
					<< LogIO::POST;
		}

		logger_p << LogIO::NORMAL << LogOrigin("DecimationTVI", __FUNCTION__)
				<< "Channel bin is " << chanbin_p << LogIO::POST;
	}
	else
	{
		ret = False;
		logger_p << LogIO::SEVERE << LogOrigin("DecimationTVI", __FUNCTION__)
				<< "chanbin parameter not found in configuration "
				<< LogIO::POST;
	}

	// Check consistency between chanbin vector and selected SPW/Chan map
	if (chanbin_p.size() !=  spwInpChanIdxMap_p.size())
	{
		ret = False;
		logger_p << LogIO::SEVERE << LogOrigin("DecimationTVI", __FUNCTION__)
				<< "Number of elements in chanbin vector does not match number of selected SPWs"
				<< LogIO::POST;
	}

	return ret;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void DecimationTVI::initialize()
{
	// Populate nchan input-output maps
	Int spw;
	uInt spw_idx = 0;
	map<Int,vector<Int> >::iterator iter;
	for(iter=spwInpChanIdxMap_p.begin();iter!=spwInpChanIdxMap_p.end();iter++)
	{
		spw = iter->first;

		// Make sure that chanbin is greater than 1
		if ((uInt)chanbin_p(spw_idx) <= 1)
		{
			logger_p << LogIO::DEBUG1 << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Selected chanbin for spw " << spw
					<< " set to 1 fallbacks to the default number of"
					<< " existing/selected channels: " << iter->second.size()
					<< LogIO::POST;

			spwChanbinMap_p[spw] = iter->second.size();
		}
		// Make sure that chanbin does not exceed number of selected channels
		else if ((uInt)chanbin_p(spw_idx) > iter->second.size())
		{
			logger_p << LogIO::WARN << LogOrigin("MSTransformManager", __FUNCTION__)
					<< "Number of selected channels " << iter->second.size()
					<< " for SPW " << spw
					<< " is smaller than specified chanbin " << chanbin_p(spw_idx) << endl
					<< "Setting chanbin to " << iter->second.size()
					<< " for SPW " << spw
					<< LogIO::POST;
			spwChanbinMap_p[spw] = iter->second.size();
		}
		else
		{
			spwChanbinMap_p[spw] = chanbin_p(spw_idx);
		}

		// Calculate number of output channels per spw
		spwOutChanNumMap_p[spw] = spwInpChanIdxMap_p[spw].size() / spwChanbinMap_p[spw];
		if (spwInpChanIdxMap_p[spw].size() % spwChanbinMap_p[spw] > 0) spwOutChanNumMap_p[spw] += 1;

		spw_idx++;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Vector<Double> DecimationTVI::getFrequencies (	Double time,
													Int frameOfReference,
													Int spectralWindowId,
													Int msId) const
{
	// Get frequencies from input VI
	Vector<Double> inputFrequencies = getVii()->getFrequencies(time,frameOfReference,
																spectralWindowId,msId);

	// Produce output (transformed) frequencies
	Vector<Double> outputFrecuencies(spwOutChanNumMap_p[spectralWindowId]);

	// Configure Transformation Engine
	DataCubeMap auxiliaryData;
	PlainDecimationKernel<Double> kernel;
	uInt width = spwChanbinMap_p[spectralWindowId];
	DecimationTransformEngine<Double> transformer(&(kernel),width);

	// Transform data
	transformer.transform(inputFrequencies,outputFrecuencies,auxiliaryData);

	return outputFrecuencies;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void DecimationTVI::writeFlag (const Cube<Bool> & flag)
{
	// Create a flag cube with the input VI shape
	Cube<Bool> propagatedFlagCube;
	propagatedFlagCube = getVii()->getVisBuffer()->flagCube();

	// Propagate flags from the input cube to the propagated flag cube
	propagateChanAvgFlags(flag,propagatedFlagCube);

	// Pass propagated flag cube downstream for further propagation and/or writting
	getVii()->writeFlag(propagatedFlagCube);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void DecimationTVI::propagateChanAvgFlags (const Cube<Bool> &transformedFlagCube,
												Cube<Bool> &propagatedFlagCube)
{
	// Get current SPW and chanbin
	VisBuffer2 *inputVB = getVii()->getVisBuffer();
	Int inputSPW = inputVB->spectralWindows()(0);
	uInt width = spwChanbinMap_p[inputSPW];

	// Get propagated (input) shape
	IPosition inputShape = propagatedFlagCube.shape();
	size_t nCorr = inputShape(0);
	size_t nChan = inputShape(1);
	size_t nRows = inputShape(2);

	// Get transformed (output) shape
	IPosition transformedShape = transformedFlagCube.shape();
	size_t nTransChan = transformedShape(1);

	// Map input-output channel
	uInt binCounts = 0;
	uInt transformedIndex = 0;
	Vector<uInt> inputOutputChan(nChan);
	for (size_t chan_i =0;chan_i<nChan;chan_i++)
	{
		binCounts += 1;

		if (binCounts > width)
		{
			binCounts = 1;
			transformedIndex += 1;
		}

		inputOutputChan(chan_i) = transformedIndex;
	}

	// Propagate chan-avg flags
	uInt outChan;
	for (size_t row_i =0;row_i<nRows;row_i++)
	{
		for (size_t chan_i =0;chan_i<nChan;chan_i++)
		{
			outChan = inputOutputChan(chan_i);
			if (outChan < nTransChan) // outChan >= nChan  may happen when channels are dropped
			{
				for (size_t corr_i =0;corr_i<nCorr;corr_i++)
				{
					if (transformedFlagCube(corr_i,outChan,row_i)) propagatedFlagCube(corr_i,chan_i,row_i) = True;
				}
			}
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////
// PlainDecimationKernel class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template<class T> void PlainDecimationKernel<T>::kernel(	Vector<T> &inputVector,
															Vector<T> &outputVector,
															DataCubeMap &,
															uInt startInputPos,
															uInt outputPos,
															uInt width)
{
	uInt pos = startInputPos + 1;
	uInt counts = 1;
	T avg = inputVector(startInputPos);
	while (counts < width)
	{
		avg += inputVector(pos);
		counts += 1;
		pos += 1;
	}

	if (counts > 0)
	{
		avg /= counts;
	}

	outputVector(outputPos) = avg;

	return;
}


} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


