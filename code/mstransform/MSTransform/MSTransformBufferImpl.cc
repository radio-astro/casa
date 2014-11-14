//# MSTransformBufferImpl.h: This file contains the implementation of the MSTransformBufferImpl.h class.
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

#include <mstransform/MSTransform/MSTransformBufferImpl.h>

namespace casa
{

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformBufferImpl::MSTransformBufferImpl(MSTransformManager *manager)
{
	manager_p = manager;

	MSSpectralWindow spwTable = manager_p->outputMs_p->spectralWindow();
	MSSpWindowColumns spwCols(spwTable);
	spwFrequencies_p.reference(spwCols.chanFreq());

	rowIdOffset_p = 0;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformBufferImpl::resetState()
{
	observationIdOk_p = False;
	arrayIdOk_p = False;
	scanOk_p = False;
	stateIdOk_p = False;
	fieldIdOk_p = False;
	dataDescIdOk_p = False;
	spectralWindowsOk_p = False;
	processorIdOk_p = False;
	antenna1Ok_p = False;
	antenna2Ok_p = False;
	feed1Ok_p = False;
	feed2Ok_p = False;
	flagRowOk_p = False;
	timeOk_p = False;
	timeCentroidOk_p = False;
	timeIntervalOk_p = False;
	exposureOk_p = False;
	uvwOk_p = False;
	weightOk_p = False;
	sigmaOk_p = False;
	flagCubeOk_p = False;
	visCubeOk_p = False;
	visCubeCorrectedOk_p = False;
	visCubeModelOk_p = False;
	visCubeFloatOk_p = False;
	weightSpectrumOk_p = False;
	feedPaOk_p = False;
	parangOk_p = False;
	azelOk_p = False;
	frequenciesOk_p = False;
	channelNumbersOk_p = False;
	rowIdsOk_p = False;
	shapeOk_p = False;
	nRowsOk_p = False;
	nChannelsOk_p = False;
	nCorrelationsOk_p = False;
	nAntennasOk_p = False;

	observationIdTransformed_p = False;
	arrayIdTransformed_p = False;
	scanTransformed_p = False;
	stateIdTransformed_p = False;
	fieldIdTransformed_p = False;
	dataDescIdTransformed_p = False;
	spectralWindowsTransformed_p = False;
	processorIdTransformed_p = False;
	antenna1Transformed_p = False;
	antenna2Transformed_p = False;
	feed1Transformed_p = False;
	feed2Transformed_p = False;
	flagRowTransformed_p = False;
	timeTransformed_p = False;
	timeCentroidTransformed_p = False;
	timeIntervalTransformed_p = False;
	exposureTransformed_p = False;
	feedPaOk_p = False;
	parangOk_p = False;
	azelOk_p = False;
	frequenciesTransformed_p = False;
	channelNumbersTransformed_p = False;
	rowIdsTransformed_p = False;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::dataDescriptionIds () const
{
	if (not dataDescIdOk_p)
	{
		getShape();
		dataDescriptionIds_p.resize(nRows_p,False);

		dataDescIdTransformed_p = manager_p->transformDDIVector(manager_p->getVisBuffer()->dataDescriptionIds(),
																dataDescriptionIds_p);

		if (not dataDescIdTransformed_p)
		{
			dataDescriptionIds_p = manager_p->getVisBuffer()->dataDescriptionIds();
		}

		dataDescIdOk_p = True;
	}

	return dataDescriptionIds_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::spectralWindows () const
{
	if (not spectralWindowsOk_p)
	{
		dataDescriptionIds();

		getShape();
		spectralWindows_p.resize(nRows_p,False);

		if (not dataDescIdTransformed_p)
		{
			spectralWindows_p = manager_p->getVisBuffer()->spectralWindows();
		}
		else
		{
			for (uInt rowIdx = 0; rowIdx<nRows_p;rowIdx++)
			{
				spectralWindows_p(rowIdx) = dataDescriptionIds_p(rowIdx);
			}

			spectralWindowsTransformed_p = True;
		}

		spectralWindowsOk_p = True;
	}

	return spectralWindows_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::observationId () const
{
	if (not observationIdOk_p)
	{
		getShape();
		observationId_p.resize(nRows_p,False);

		observationIdTransformed_p = manager_p->transformReindexableVector(	manager_p->getVisBuffer()->observationId(),
																			observationId_p,
																			True,
																			manager_p->inputOutputObservationIndexMap_p);
		observationIdOk_p = True;
	}

	if (not observationIdTransformed_p)
	{
		return manager_p->getVisBuffer()->observationId();
	}

	return observationId_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::arrayId () const
{
	if (not arrayIdOk_p)
	{
		getShape();
		arrayId_p.resize(nRows_p,False);

		arrayIdTransformed_p = manager_p->transformReindexableVector(	manager_p->getVisBuffer()->arrayId(),
																		arrayId_p,
																		True,
																		manager_p->inputOutputArrayIndexMap_p);
		arrayIdOk_p = True;
	}

	if (not arrayIdTransformed_p)
	{
		return manager_p->getVisBuffer()->arrayId();
	}

	return arrayId_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::fieldId () const
{
	if (not fieldIdOk_p)
	{
		getShape();
		fieldId_p.resize(nRows_p,False);

		fieldIdTransformed_p = manager_p->transformReindexableVector(	manager_p->getVisBuffer()->fieldId(),
																		fieldId_p,
																		True,
																		manager_p->inputOutputFieldIndexMap_p);
		fieldIdOk_p = True;
	}

	if (not fieldIdTransformed_p)
	{
		return manager_p->getVisBuffer()->fieldId();
	}

	return fieldId_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::stateId () const
{
	if (not stateIdOk_p)
	{
		getShape();
		stateId_p.resize(nRows_p,False);

		stateIdTransformed_p = manager_p->transformReindexableVector(	manager_p->getVisBuffer()->stateId(),
																		stateId_p,
																		!manager_p->timespan_p.contains("state"),
																		manager_p->inputOutputScanIntentIndexMap_p);
		stateIdOk_p = True;
	}

	if (not stateIdTransformed_p)
	{
		return manager_p->getVisBuffer()->stateId();
	}


	return stateId_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::antenna1 () const
{
	if (not antenna1Ok_p)
	{
		getShape();
		antenna1_p.resize(nRows_p,False);

		antenna1Transformed_p = manager_p->transformReindexableVector(	manager_p->getVisBuffer()->antenna1(),
																		antenna1_p,
																		False,
																		manager_p->inputOutputAntennaIndexMap_p);
		antenna1Ok_p = True;
	}

	if (not antenna1Transformed_p)
	{
		return manager_p->getVisBuffer()->antenna1();
	}

	return antenna1_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::antenna2 () const
{
	if (not antenna2Ok_p)
	{
		getShape();
		antenna2_p.resize(nRows_p,False);

		antenna2Transformed_p = manager_p->transformReindexableVector(	manager_p->getVisBuffer()->antenna2(),
																		antenna2_p,
																		False,
																		manager_p->inputOutputAntennaIndexMap_p);
		antenna2Ok_p = True;
	}

	if (not antenna2Transformed_p)
	{
		return manager_p->getVisBuffer()->antenna2();
	}

	return antenna2_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::scan () const
{
	if (not scanOk_p)
	{
		getShape();
		scan_p.resize(nRows_p,False);

		scanTransformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->scan(),
																		scan_p,
																		!manager_p->timespan_p.contains("scan"));
		scanOk_p = True;
	}

	if (not scanTransformed_p)
	{
		return manager_p->getVisBuffer()->scan();
	}

	return scan_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::processorId () const
{
	if (not processorIdOk_p)
	{
		getShape();
		processorId_p.resize(nRows_p,False);

		processorIdTransformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->processorId(),
																				processorId_p,
																				False);
		processorIdOk_p = True;
	}

	if (not processorIdTransformed_p)
	{
		return manager_p->getVisBuffer()->processorId();
	}

	return processorId_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::feed1 () const
{
	if (not feed1Ok_p)
	{
		getShape();
		feed1_p.resize(nRows_p,False);

		feed1Transformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->feed1(),
																		feed1_p,
																		False);
		feed1Ok_p = True;
	}

	if (not feed1Transformed_p)
	{
		return manager_p->getVisBuffer()->feed1();
	}

	return feed1_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::feed2 () const
{
	if (not feed2Ok_p)
	{
		getShape();
		feed2_p.resize(nRows_p,False);

		feed2Transformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->feed2(),
																		feed2_p,
																		False);
		feed2Ok_p = True;
	}

	if (not feed2Transformed_p)
	{
		return manager_p->getVisBuffer()->feed2();
	}

	return feed2_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Double> & MSTransformBufferImpl::time () const
{
	if (not timeOk_p)
	{
		getShape();
		time_p.resize(nRows_p,False);

		timeTransformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->time(),
																		time_p,
																		False);
		timeOk_p = True;
	}

	if (not timeTransformed_p)
	{
		return manager_p->getVisBuffer()->time();
	}

	return time_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Double> & MSTransformBufferImpl::timeCentroid () const
{
	if (not timeCentroidOk_p)
	{
		getShape();
		timeCentroid_p.resize(nRows_p,False);

		timeCentroidTransformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->timeCentroid(),
																				timeCentroid_p,
																				False);
		timeCentroidOk_p = True;
	}

	if (not timeCentroidTransformed_p)
	{
		return manager_p->getVisBuffer()->timeCentroid();
	}

	return timeCentroid_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Double> & MSTransformBufferImpl::timeInterval () const
{
	if (not timeIntervalOk_p)
	{
		getShape();
		timeInterval_p.resize(nRows_p,False);

		timeIntervalTransformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->timeInterval(),
																				timeInterval_p,
																				False);
		timeIntervalOk_p = True;
	}

	if (not timeIntervalTransformed_p)
	{
		return manager_p->getVisBuffer()->timeInterval();
	}

	return timeInterval_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Double> & MSTransformBufferImpl::exposure () const
{
	if (not exposureOk_p)
	{
		getShape();
		exposure_p.resize(nRows_p,False);

		if (manager_p->combinespws_p)
		{
			manager_p->mapAndAverageVector(	manager_p->getVisBuffer()->exposure(),exposure_p);
			exposureTransformed_p = True;
		}
		else
		{
			exposureTransformed_p = False;
		}

		exposureOk_p = True;
	}

	if (not exposureTransformed_p)
	{
		return manager_p->getVisBuffer()->exposure();
	}

	return exposure_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Bool> & MSTransformBufferImpl::flagRow () const
{
	if (not flagRowOk_p)
	{
		getShape();
		flagRow_p.resize(nRows_p,False);

		if (manager_p->combinespws_p)
		{
			manager_p->mapAndAverageVector(	manager_p->getVisBuffer()->flagRow(),flagRow_p);
			flagRowTransformed_p = True;
		}
		else
		{
			flagRowTransformed_p = False;
		}

		flagRowOk_p = True;
	}

	if (not flagRowTransformed_p)
	{
		return manager_p->getVisBuffer()->flagRow();
	}

	return flagRow_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Matrix<Double> & MSTransformBufferImpl::uvw () const
{
	if (not uvwOk_p)
	{

		uvwOk_p = True;
	}

	return uvw_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Matrix<Float> & MSTransformBufferImpl::weight () const
{
	if (not weightOk_p)
	{
		weightOk_p = True;
	}

	return weight_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Matrix<Float> & MSTransformBufferImpl::sigma () const
{
	if (not sigmaOk_p)
	{
		sigmaOk_p = True;
	}

	return sigma_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Cube<Bool> & MSTransformBufferImpl::flagCube () const
{
	if (not manager_p->spectrumReshape_p)
	{
		return manager_p->getVisBuffer()->flagCube();
	}
	else if (not flagCubeOk_p)
	{
		if (manager_p->dataColumnAvailable_p)
		{
			visCube();
		}
		else if (manager_p->correctedDataColumnAvailable_p)
		{
			visCubeCorrected();
		}
		else if (manager_p->modelDataColumnAvailable_p)
		{
			visCubeModel();
		}
		else if (manager_p->floatDataColumnAvailable_p)
		{
			visCubeFloat();
		}
		else
		{
			manager_p->logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformBufferImpl", __FUNCTION__)
					<< "No data columns available to provide a transformed FlagCube" << LogIO::POST;
		}

		flagCubeOk_p = True;
	}

	return flagCube_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Cube<Complex> & MSTransformBufferImpl::visCube () const
{
	if ( (not manager_p->dataColumnAvailable_p) and (not manager_p->getVisBuffer()->modelDataIsVirtual()) )
	{
		manager_p->logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformBufferImpl", __FUNCTION__)
				<< "visCube requested but DATA column not present in input MS" << LogIO::POST;
	}

	if (not manager_p->spectrumReshape_p)
	{
		return manager_p->getVisBuffer()->visCube();
	}
	else if (not visCubeOk_p)
	{
		visCube_p.resize(getShape(),False);
		manager_p->visCube_p = &visCube_p;

		flagCube_p.resize(getShape(),False);
		manager_p->flagCube_p = &flagCube_p;

		RefRows dummyRefRows(0,0);
		ArrayColumn<Complex> dummyDataCol;

		manager_p->dataBuffer_p = MSTransformations::visCube;
		manager_p->transformCubeOfData(	manager_p->getVisBuffer(),
										dummyRefRows,
										manager_p->getVisBuffer()->visCube(),
										dummyDataCol,
										NULL,
										weightSpectrum_p);
		flagCubeOk_p = True;
		visCubeOk_p = True;
	}

	return visCube_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Cube<Complex> & MSTransformBufferImpl::visCubeCorrected () const
{
	if (not manager_p->correctedDataColumnAvailable_p)
	{
		manager_p->logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformBufferImpl", __FUNCTION__)
				<< "visCubeCorrected requested but CORRECTED_DATA column not present in input MS" << LogIO::POST;
	}

	if (not manager_p->spectrumReshape_p)
	{
		return manager_p->getVisBuffer()->visCubeCorrected();
	}
	else if (not visCubeCorrectedOk_p)
	{
		visCubeCorrected_p.resize(getShape(),False);
		manager_p->visCubeCorrected_p = &visCubeCorrected_p;

		flagCube_p.resize(getShape(),False);
		manager_p->flagCube_p = &flagCube_p;

		RefRows dummyRefRows(0,0);
		ArrayColumn<Complex> dummyDataCol;

		manager_p->dataBuffer_p = MSTransformations::visCubeCorrected;
		manager_p->transformCubeOfData(	manager_p->getVisBuffer(),
											dummyRefRows,
											manager_p->getVisBuffer()->visCubeCorrected(),
											dummyDataCol,
											NULL,
											weightSpectrum_p);
		flagCubeOk_p = True;
		visCubeCorrectedOk_p = True;
	}

	return visCubeCorrected_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Cube<Complex> & MSTransformBufferImpl::visCubeModel () const
{
	if (not manager_p->modelDataColumnAvailable_p)
	{
		manager_p->logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformBufferImpl", __FUNCTION__)
				<< "visCubeModel requested but MODEL_DATA column not present in input MS" << LogIO::POST;
	}

	if (not manager_p->spectrumReshape_p)
	{
		return manager_p->getVisBuffer()->visCubeModel();
	}
	else if (not visCubeModelOk_p)
	{
		visCubeModel_p.resize(getShape(),False);
		manager_p->visCubeModel_p = &visCubeModel_p;

		flagCube_p.resize(getShape(),False);
		manager_p->flagCube_p = &flagCube_p;

		RefRows dummyRefRows(0,0);
		ArrayColumn<Complex> dummyDataCol;

		manager_p->dataBuffer_p = MSTransformations::visCubeModel;
		manager_p->transformCubeOfData(	manager_p->getVisBuffer(),
										dummyRefRows,
										manager_p->getVisBuffer()->visCubeModel(),
										dummyDataCol,
										NULL,
										weightSpectrum_p);
		flagCubeOk_p = True;
		visCubeModelOk_p= True;
	}

	return visCubeModel_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Cube<Float> & MSTransformBufferImpl::visCubeFloat () const
{
	if (not manager_p->floatDataColumnAvailable_p)
	{
		manager_p->logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformBufferImpl", __FUNCTION__)
				<< "visCubeFloat requested but FLOAT_DATA column not present in input MS" << LogIO::POST;
	}

	if (not manager_p->spectrumReshape_p)
	{
		return manager_p->getVisBuffer()->visCubeFloat();
	}
	else if (not visCubeFloatOk_p)
	{
		visCubeFloat_p.resize(getShape(),False);
		manager_p->visCubeFloat_p = &visCubeFloat_p;

		flagCube_p.resize(getShape(),False);
		manager_p->flagCube_p = &flagCube_p;

		RefRows dummyRefRows(0,0);
		ArrayColumn<Float> dummyDataCol;

		manager_p->dataBuffer_p = MSTransformations::visCubeFloat;
		manager_p->transformCubeOfData(	manager_p->getVisBuffer(),
										dummyRefRows,
										manager_p->getVisBuffer()->visCubeFloat(),
										dummyDataCol,
										NULL,
										weightSpectrum_p);
		flagCubeOk_p = True;
		visCubeFloatOk_p = True;
	}

	return visCubeFloat_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Cube<Float> & MSTransformBufferImpl::weightSpectrum () const
{
	if (not weightSpectrumOk_p)
	{
		if (manager_p->inputWeightSpectrumAvailable_p)
		{
			if (not manager_p->spectrumReshape_p)
			{
				return manager_p->getVisBuffer()->weightSpectrum();
			}
			else
			{
				weightSpectrum_p.resize(getShape(),False);
				manager_p->weightSpectrum_p = &weightSpectrum_p;

				RefRows dummyRefRows(0,0);
				ArrayColumn<Float> dummyDataCol;

				// Don't propagate auxiliary weight spectrum and use cumSum for average
		    	manager_p->propagateWeights(False);
		    	manager_p->setChannelAverageKernel(MSTransformations::cumSum);

		    	// Transform weights
				manager_p->dataBuffer_p = MSTransformations::weightSpectrum;
				manager_p->transformCubeOfData(	manager_p->getVisBuffer(),
												dummyRefRows,
												manager_p->getVisBuffer()->weightSpectrum(),
												dummyDataCol,
												NULL,
												weightSpectrum_p);

				// Go back to normal
		    	manager_p->propagateWeights(manager_p->propagateWeights_p);
		    	manager_p->setChannelAverageKernel(manager_p->weightmode_p);
			}
		}
		// Fill WEIGHT_SPECTRUM with transformed WEIGHTS
		else
		{
			weightSpectrum_p.resize(getShape(),False);
			for (uInt row=0; row < nRows_p; row++)
			{
				Double scaleFactor = 1.0/nChannels_p;
				Matrix<Float> weightSpectrumPlane = weightSpectrum_p.xyPlane(row);
				for (uInt pol = 0; pol < nCorrelations_p; pol++)
				{
					weightSpectrumPlane.row(pol) = scaleFactor*weight_p(pol,row);
				}
			}
		}

		weightSpectrumOk_p = True;
	}

	return weightSpectrum_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Array<Bool> & MSTransformBufferImpl::flagCategory () const
{
	if (not manager_p->inputFlagCategoryAvailable_p)
	{
		manager_p->logger_p << LogIO::EXCEPTION << LogOrigin("MSTransformBufferImpl", __FUNCTION__)
				<< "FlagCategory requested but FLAG_CATEGORY column not present in input MS" << LogIO::POST;
	}

	if (not manager_p->spectrumReshape_p)
	{
		return manager_p->getVisBuffer()->flagCategory();
	}
	else if (not flagCategoryOk_p)
	{
		uInt nCategories = manager_p->getVisBuffer()->flagCategory().shape()(2); // [nC,nF,nCategories,nR]
		IPosition flagCategoryShape(4,nCorrelations_p,nChannels_p,nCategories,nRows_p);
		flagCategory_p.resize(flagCategoryShape,False);
		flagCategoryOk_p = True;
	}

	return flagCategory_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Float> & MSTransformBufferImpl::feedPa (Double time) const
{
	/*
	if (not feedPaOk_p)
	{
		getShape();
		feedPa_p.resize(nRows_p,False);

		feedPaTransformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->feedPa(time),
																		feedPa_p,
																		False);
		feedPaOk_p = True;
	}

	if (not feedPaTransformed_p)
	{
		return manager_p->getVisBuffer()->feedPa(time);
	}

	return feedPa_p;
	*/

	return manager_p->getVisBuffer()->feedPa(time);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Float MSTransformBufferImpl::parang0 (Double time) const
{
	return manager_p->getVisBuffer()->parang0(time);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Float> & MSTransformBufferImpl::parang(Double time) const
{
	/*
	if (not parangOk_p)
	{
		getShape();
		parang_p.resize(nRows_p,False);

		parangTransformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->parang(time),
																		parang_p,
																		False);
		parangOk_p = True;
	}

	if (not parangTransformed_p)
	{
		return manager_p->getVisBuffer()->parang(time);
	}

	return parang_p;
	*/

	return manager_p->getVisBuffer()->parang(time);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MDirection MSTransformBufferImpl::azel0 (Double time) const
{
	return manager_p->getVisBuffer()->azel0(time);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<MDirection> & MSTransformBufferImpl::azel(Double time) const
{
	if (not azelOk_p)
	{
		getShape();
		azel_p.resize(nRows_p,False);

		azelTransformed_p = manager_p->transformNotReindexableVector(	manager_p->getVisBuffer()->azel(time),
																		azel_p,
																		False);
		azelOk_p = True;
	}

	if (not azelTransformed_p)
	{
		return manager_p->getVisBuffer()->azel(time);
	}

	return azel_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Double MSTransformBufferImpl::hourang(Double time) const
{
	return manager_p->getVisBuffer()->hourang(time);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Vector<Int> MSTransformBufferImpl::getCorrelationTypes () const
{
	return manager_p->getVisBuffer()->getCorrelationTypes();
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Double MSTransformBufferImpl::getFrequency (Int rowInBuffer, Int frequencyIndex, Int frame) const
{
	Bool newFrequencies = 	manager_p->combinespws_p ||
							manager_p->refFrameTransformation_p ||
							manager_p->channelAverage_p ||
							(manager_p->nspws_p > 1);

	if (not newFrequencies)
	{
		return manager_p->getVisBuffer()->getFrequency (rowInBuffer,frequencyIndex,frame);
	}

	spectralWindows();
	Vector<Double> frequencies = spwFrequencies_p(spectralWindows_p(rowInBuffer));
	return frequencies(frequencyIndex);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Double> & MSTransformBufferImpl::getFrequencies (Int rowInBuffer,Int frame) const
{
	if (not frequenciesOk_p)
	{
		Bool newFrequencies = 	manager_p->combinespws_p ||
								manager_p->refFrameTransformation_p ||
								manager_p->channelAverage_p ||
								(manager_p->nspws_p > 1);

		if (not newFrequencies)
		{
			frequenciesTransformed_p = False;
		}
		else
		{
			getShape();
			frequencies_p.resize(nChannels_p,False);
			Vector<Double> frequencies = spwFrequencies_p(spectralWindows_p(rowInBuffer));

			for (uInt chanIdx = 0; chanIdx<nChannels_p;chanIdx++)
			{
				frequencies_p(chanIdx) = frequencies(chanIdx);
			}

			frequenciesTransformed_p = True;
		}

		frequenciesOk_p = True;
	}

	if (not frequenciesTransformed_p)
	{
		return manager_p->getVisBuffer()->getFrequencies(rowInBuffer,frame);
	}

	return frequencies_p;

}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Int MSTransformBufferImpl::getChannelNumber (Int rowInBuffer, Int frequencyIndex) const
{
	Bool newFrequencies = 	manager_p->combinespws_p ||
							manager_p->refFrameTransformation_p ||
							manager_p->channelAverage_p ||
							(manager_p->nspws_p > 1);

	if (not newFrequencies)
	{
		return manager_p->getVisBuffer()->getChannelNumber (rowInBuffer,frequencyIndex);
	}

	return frequencyIndex;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<Int> & MSTransformBufferImpl::getChannelNumbers (Int rowInBuffer) const
{
	if (not channelNumbersOk_p)
	{
		Bool newFrequencies = 	manager_p->combinespws_p ||
								manager_p->refFrameTransformation_p ||
								manager_p->channelAverage_p ||
								(manager_p->nspws_p > 1);

		if (not newFrequencies)
		{
			channelNumbersTransformed_p = False;
		}
		else
		{
			getShape();
			channelNumbers_p.resize(nChannels_p,False);

			for (uInt chanIdx = 0; chanIdx<nChannels_p;chanIdx++)
			{
				channelNumbers_p(chanIdx) = chanIdx;
			}

			channelNumbersTransformed_p = True;
		}

		channelNumbersOk_p = True;
	}

	if (not channelNumbersTransformed_p)
	{
		return manager_p->getVisBuffer()->getChannelNumbers(rowInBuffer);
	}

	return channelNumbers_p;
}

const Vector<uInt> & MSTransformBufferImpl::rowIds () const
{
	if (not rowIdsOk_p)
	{
		Bool newFrequencies = 	manager_p->combinespws_p ||
								manager_p->refFrameTransformation_p ||
								manager_p->channelAverage_p ||
								(manager_p->nspws_p > 1);

		if (not newFrequencies)
		{
			rowIdsTransformed_p = False;
		}
		else
		{
			getShape();
			rowIds_p.resize(nRows_p,False);

			for (uInt rowIdx = 0; rowIdx<nRows_p;rowIdx++)
			{
				rowIds_p(rowIdx) = rowIdOffset_p + rowIdx;
			}

			rowIdsTransformed_p = True;
		}

		rowIdsOk_p = True;
	}

	if (not rowIdsTransformed_p)
	{
		return manager_p->getVisBuffer()->rowIds();
	}

	return rowIds_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
IPosition MSTransformBufferImpl::getShape () const
{
	if (not shapeOk_p)
	{
		shape_p = manager_p->getShape();
		nRows_p = shape_p(2);
		nChannels_p = shape_p(1);
		nCorrelations_p = shape_p(0);
		nAntennas_p = manager_p->getVisBuffer()->nAntennas();
		shapeOk_p = True;
	}

	return shape_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Int MSTransformBufferImpl::nRows () const
{
	if (not nRowsOk_p)
	{
		getShape();
		nRowsOk_p = True;
	}

	return nRows_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Int MSTransformBufferImpl::nChannels () const
{
	if (not nChannelsOk_p)
	{
		getShape();
		nChannelsOk_p = True;
	}

	return nChannels_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Int MSTransformBufferImpl::nCorrelations () const
{
	if (not nCorrelationsOk_p)
	{
		getShape();
		nCorrelationsOk_p  = True;
	}

	return nCorrelations_p;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Int MSTransformBufferImpl::nAntennas () const
{
	if (not nAntennasOk_p)
	{
		getShape();
		nAntennasOk_p = True;
	}

	return nAntennas_p;
}

} //# NAMESPACE CASA - END


