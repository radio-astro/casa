//# FreqAxisTVI.h: This file contains the implementation of the FreqAxisTVI class.
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

#ifndef FreqAxisTVI_CC_
#define FreqAxisTVI_CC_

#include <mstransform/TVI/FreqAxisTVI.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN

//////////////////////////////////////////////////////////////////////////
// FreqAxisTVI class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
FreqAxisTVI::FreqAxisTVI(	ViImplementation2 * inputVii,
							const Record &configuration):
							TransformingVi2 (inputVii)
{
	// Parse and check configuration parameters
	// Note: if a constructor finishes by throwing an exception, the memory
	// associated with the object itself is cleaned up — there is no memory leak.
	if (not parseConfiguration(configuration))
	{
		throw AipsError("Error parsing ChannelAverageTVI configuration");
	}

	initialize();

	// Initialize attached VisBuffer
	setVisBuffer(createAttachedVisBuffer (VbPlain,VbRekeyable));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool FreqAxisTVI::parseConfiguration(const Record &configuration)
{
	int exists = 0;
	Bool ret = True;

	// Parse spw selection (optional)
	exists = configuration.fieldNumber ("spw");
	if (exists >= 0)
	{
		configuration.get (exists, spwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("FreqAxisTVI", __FUNCTION__)
				<< "spw selection is " << spwSelection_p
				<< LogIO::POST;
	}
	else
	{
		spwSelection_p = "*";
	}

	// Get list of selected SPWs and channels
	MSSelection mssel;
	mssel.setSpwExpr(spwSelection_p);
	Matrix<Int> spwchan = mssel.getChanList(&(inputVii_p->ms()));
	logger_p << LogIO::NORMAL << LogOrigin("FreqAxisTVI", __FUNCTION__)
			<< "Selected SPW:Channels are " << spwchan << LogIO::POST;

	// Convert list of selected SPWs/Channels into a map
	spwInpChanIdxMap_p.clear();
    uInt nSelections = spwchan.shape()[0];
	Int spw,channelStart,channelStop,channelStep;
	for(uInt selection_i=0;selection_i<nSelections;selection_i++)
	{
		spw = spwchan(selection_i,0);
		channelStart = spwchan(selection_i,1);
		channelStop = spwchan(selection_i,2);
		channelStep = spwchan(selection_i,3);

		if (spwInpChanIdxMap_p.find(spw) == spwInpChanIdxMap_p.end())
		{
			spwInpChanIdxMap_p[spw].clear(); // Accesing the vector creates it
		}

		for (Int inpChan=channelStart;inpChan<=channelStop;inpChan += channelStep)
		{
			spwInpChanIdxMap_p[spw].push_back(inpChan);
		}
	}

	return ret;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVI::initialize()
{
	// Get list of selected SPWs and channels
	MSSelection mssel;
	mssel.setSpwExpr(spwSelection_p);
	Matrix<Int> spwchan = mssel.getChanList(&(inputVii_p->ms()));
	logger_p << LogIO::NORMAL << LogOrigin("FreqAxisTVI", __FUNCTION__)
			<< "Selected SPW:Channels are " << spwchan << LogIO::POST;

	// Convert list of selected SPWs/Channels into a map
	spwInpChanIdxMap_p.clear();
    uInt nSelections = spwchan.shape()[0];
	Int spw,channelStart,channelStop,channelStep;
	for(uInt selection_i=0;selection_i<nSelections;selection_i++)
	{
		spw = spwchan(selection_i,0);
		channelStart = spwchan(selection_i,1);
		channelStop = spwchan(selection_i,2);
		channelStep = spwchan(selection_i,3);

		if (spwInpChanIdxMap_p.find(spw) == spwInpChanIdxMap_p.end())
		{
			spwInpChanIdxMap_p[spw].clear(); // Accesing the vector creates it
		}

		for (Int inpChan=channelStart;inpChan<=channelStop;inpChan += channelStep)
		{
			spwInpChanIdxMap_p[spw].push_back(inpChan);
		}
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVI::origin()
{
	// Drive underlying ViImplementation2
	getVii()->origin();

	// Synchronize own VisBuffer
	configureNewSubchunk();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVI::next()
{
	// Drive underlying ViImplementation2
	getVii()->next();

	// Synchronize own VisBuffer
	configureNewSubchunk();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool FreqAxisTVI::existsColumn (VisBufferComponent2 id) const
{

	Bool ret;
	switch (id)
	{
		case WeightSpectrum:
		{
			ret = True;
			break;
		}
		case SigmaSpectrum:
		{
			ret = True;
			break;
		}
		default:
		{
			ret = getVii()->existsColumn(id);
			break;
		}
	}

	return ret;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void FreqAxisTVI::transformFreqAxis(	Cube<T> const &inputDataCube,
														Cube<T> &outputDataCube,
														DataCubeMap &auxiliaryData,
														FreqAxisTransformEngine<T> &transformer) const
{
	// Re-shape output data cube
	outputDataCube.resize(getVisBufferConst()->getShape(),False);

	// Get data shape for iteration
	const IPosition &inputShape = inputDataCube.shape();
	uInt nRows = inputShape(2);
	uInt nCorrs = inputShape(0);

	// Initialize input-output planes
	Matrix<T> inputDataPlane;
	Matrix<T> outputDataPlane;

	// Initialize input-output vectors
	Vector<T> inputDataVector;
	Vector<T> outputDataVector;

	for (uInt row=0; row < nRows; row++)
	{
		// Assign input-output planes by reference
		auxiliaryData.setMatrixIndex(row);
		inputDataPlane.reference(inputDataCube.xyPlane(row));
		outputDataPlane.reference(outputDataCube.xyPlane(row));

		for (uInt corr=0; corr < nCorrs; corr++)
		{
			// Assign input-output vectors by reference
			auxiliaryData.setVectorIndex(corr);
			inputDataVector.reference(inputDataPlane.row(corr));
			outputDataVector.reference(outputDataPlane.row(corr));

			// Transform data
			transformer.transform(inputDataVector,outputDataVector,auxiliaryData);
		}
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Vector<Int> FreqAxisTVI::getChannels (Double,Int,Int spectralWindowId,Int) const
{
	Vector<Int> ret(spwOutChanNumMap_p[spectralWindowId]);

	for (uInt chanIdx = 0; chanIdx<spwOutChanNumMap_p[spectralWindowId];chanIdx++)
	{
		ret(chanIdx) = chanIdx;
	}

	return ret;
}


// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVI::flagRow (Vector<Bool> & flagRow) const
{
	// Get flagCube from own VisBuffer
	const Cube<Bool> &flagCube = getVisBufferConst()->flagCube();

	// Calculate output flagRow
	accumulateFlagCube(flagCube,flagRow);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVI::weight (Matrix<Float> & weight) const
{
	// Get flags and weightSpectrum from own VisBuffer
	const Cube<Bool> &flags = getVisBufferConst()->flagCube();
	const Cube<Float> &weightSpectrum = getVisBufferConst()->weightSpectrum();

	// Calculate output weight
	accumulateWeightCube(weightSpectrum,flags,weight);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void FreqAxisTVI::sigma (Matrix<Float> & sigma) const
{

	// Get flags and sigmaSpectrum from own VisBuffer
	const Cube<Bool> &flags = getVisBufferConst()->flagCube();
	const Cube<Float> &sigmaSpectrum = getVisBufferConst()->sigmaSpectrum();

	// Calculate output sigma
	accumulateWeightCube(sigmaSpectrum,flags,sigma);

	return;
}

} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END

#endif /* FreqAxisTVI_CC_ */


