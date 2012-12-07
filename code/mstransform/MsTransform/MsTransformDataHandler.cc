//# MsTransformDataHandler.h: This file contains the implementation of the MsTransformDataHandler class.
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

#include <mstransform/MsTransform/MsTransformDataHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace tvf
{
	Double wtToSigma(Double wt)
	{
		return wt > 0.0 ? 1.0 / sqrt(wt) : -1.0;
	}
}

/////////////////////////////////////////////
/// MsTransformDataHandler implementation //
/////////////////////////////////////////////

// -----------------------------------------------------------------------
// Default constructor
// -----------------------------------------------------------------------
MsTransformDataHandler::MsTransformDataHandler()
{
	initialize();
	return;
}


// -----------------------------------------------------------------------
// Configuration constructor
// -----------------------------------------------------------------------
MsTransformDataHandler::MsTransformDataHandler(Record configuration)
{
	initialize();
	configure(configuration);
	return;
}


// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
MsTransformDataHandler::~MsTransformDataHandler()
{
	if (visibilityIterator_p) delete visibilityIterator_p;
	if (splitter_p) delete splitter_p;

	return;
}

// -----------------------------------------------------------------------
// Method to initialize members to default values
// -----------------------------------------------------------------------
void MsTransformDataHandler::initialize()
{
	inputMs_p = NULL;
	selectedInputMs_p = NULL;
	inputMsCols_p = NULL;
	visibilityIterator_p = NULL;

	splitter_p = NULL;
	outputMs_p = NULL;
	outputMsCols_p = NULL;

	inpMsName_p = String("");
	outMsName_p = String("");

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

	// By default operate on data column
	colname_p = String("data");

	// By default don't combine across scan/state
	combine_p = String("");

	// By default don't provide a tile shape specification
	tileShape_p.resize(1,False);
	tileShape_p(0) = 0;

	// By default don't perform channel average
	chanSpec_p.resize(1,False);
	chanSpec_p(0) = 1;

	// By default don't perform time average
	timeBin_p = 0.0;

	// Default sort order (don't combine any data)
	sortColumns_p = Block<Int>(7);
	sortColumns_p[0] = MS::OBSERVATION_ID;
	sortColumns_p[1] = MS::ARRAY_ID;
	sortColumns_p[2] = MS::SCAN_NUMBER;
	sortColumns_p[3] = MS::STATE_ID;
	sortColumns_p[4] = MS::FIELD_ID;
	sortColumns_p[5] = MS::DATA_DESC_ID;
	sortColumns_p[6] = MS::TIME;

	fillFlagCategory_p = False;
	fillWeightSpectrum_p = False;
	correctedToData_p = False;
	dataColMap_p.clear();
	baselineMap_p.clear();

	return;
}

// -----------------------------------------------------------------------
// Method to configure (or re-configure) members
// -----------------------------------------------------------------------
void MsTransformDataHandler::configure(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("inputms");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("inputms"), inpMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Input file name is " << inpMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("outputms");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("outputms"), outMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Output file name is " << outMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("spw");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("spw"), spwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Spectral Window selection is " << spwSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("timebin");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("timebin"), timeBin_p);
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Time interval is " << timeBin_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("combine");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("combine"), combine_p);
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Combine axes are " << combine_p << LogIO::POST;
	}


	return;
}

// -----------------------------------------------------------------------
// Method to open the input MS, select the data and create the
// structure of the output MS filling the auxiliary tables.
// -----------------------------------------------------------------------
void MsTransformDataHandler::open()
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

	splitter_p->makeSubMS(outMsName_p,colname_p,tileShape_p,combine_p);

	selectedInputMs_p = &(splitter_p->mssel_p);

	outputMs_p = &(splitter_p->msOut_p);

	inputMsCols_p = splitter_p->mscIn_p;

	outputMsCols_p = splitter_p->msc_p;

	return;
}

// -----------------------------------------------------------------------
// Close the output MS
// -----------------------------------------------------------------------
void MsTransformDataHandler::close()
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
void MsTransformDataHandler::checkFillFlagCategory()
{
	fillFlagCategory_p = False;
	if (!inputMsCols_p->flagCategory().isNull() && inputMsCols_p->flagCategory().isDefined(0))
	{
		fillFlagCategory_p = True;
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Optional column FLAG_CATEGORY found in input MS will be written to output MS" << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check if weight spectrum column has to be filled
// -----------------------------------------------------------------------
void MsTransformDataHandler::checkFillWeightSpectrum()
{
	fillWeightSpectrum_p = False;
	if (!inputMsCols_p->weightSpectrum().isNull() && inputMsCols_p->weightSpectrum().isDefined(0))
	{
		fillWeightSpectrum_p = True;
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Optional column WEIGHT_SPECTRUM found in input MS will be written to output MS" << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check which data columns have to be filled
// -----------------------------------------------------------------------
void MsTransformDataHandler::checkDataColumnsToFill()
{
	dataColMap_p.clear();
	if (colname_p.contains("all"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			dataColMap_p[MS::CORRECTED_DATA] = MS::CORRECTED_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding CORRECTED_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
		{
			dataColMap_p[MS::MODEL_DATA] = MS::MODEL_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding MODEL_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("float_data,data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"FLOAT_DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("float_data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"FLOAT_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("lag_data,data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"LAG_DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("lag_data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"LAG_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("corrected"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			dataColMap_p[MS::CORRECTED_DATA] = MS::DATA;
			correctedToData_p = True;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS from input CORRECTED_DATA column"<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"CORRECTED_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("model"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
		{
			dataColMap_p[MS::MODEL_DATA] = MS::DATA;
			dataColMap_p[MS::CORRECTED_DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS from input MODEL_DATA column"<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("MsTransformDataHandler", __FUNCTION__) <<
					"MODEL_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}

	return;
}

// -----------------------------------------------------------------------
// Determine sort columns order
// -----------------------------------------------------------------------
void MsTransformDataHandler::setIterationApproach()
{
	uInt nSortColumns = 7;

	if (combine_p.contains("scan")) nSortColumns -= 1;
	if (combine_p.contains("state")) nSortColumns -= 1;
	if (combine_p.contains("spw")) nSortColumns -= 1;

	sortColumns_p = Block<Int>(nSortColumns);
	uInt sortColumnIndex = 0;

	sortColumns_p[0] = MS::OBSERVATION_ID;
	sortColumnIndex += 1;

	sortColumns_p[1] = MS::ARRAY_ID;
	sortColumnIndex += 1;

	if (!combine_p.contains("scan"))
	{
		sortColumns_p[sortColumnIndex] =  MS::SCAN_NUMBER;
		sortColumnIndex += 1;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Combining data through scans for time average " << LogIO::POST;
	}

	if (!combine_p.contains("state"))
	{
		sortColumns_p[sortColumnIndex] =  MS::STATE_ID;
		sortColumnIndex += 1;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Combining data through state for time average" << LogIO::POST;
	}

	sortColumns_p[sortColumnIndex] = MS::FIELD_ID;
	sortColumnIndex += 1;

	if (!combine_p.contains("spw"))
	{
		sortColumns_p[sortColumnIndex] =  MS::DATA_DESC_ID;
		sortColumnIndex += 1;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__) << "Combining data from selected spectral windows" << LogIO::POST;
	}

	sortColumns_p[sortColumnIndex] =  MS::TIME;

	return;
}

// -----------------------------------------------------------------------
// Generate the initial iterator
// -----------------------------------------------------------------------
void MsTransformDataHandler::generateIterator()
{
	visibilityIterator_p = new vi::VisibilityIterator2(*selectedInputMs_p,sortColumns_p,false,NULL,false,timeBin_p);
	return;
}

// -----------------------------------------------------------------------
// Check configuration and input MS characteristics to determine run parameters
// -----------------------------------------------------------------------
void MsTransformDataHandler::setup()
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
void MsTransformDataHandler::fillOutputMs(vi::VisBuffer2 *vb)
{
	// Calculate number of rows to add to the output MS depending on the combination parameters
	uInt rowsToAdd = 0;
	if (combine_p.contains("spw"))
	{
		baselineMap_p.clear();
		Vector<Int> antenna1 = vb->antenna1();
		Vector<Int> antenna2 = vb->antenna2();
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

	logger_p << LogIO::NORMAL << LogOrigin("MsTransformDataHandler", __FUNCTION__)
			<< "Adding " << rowsToAdd << " rows to output MS from "
			<<  vb->nRows() << " rows in input MS selection" << LogIO::POST;

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
void MsTransformDataHandler::fillIdCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
    // For row-constant columns we have to create new vectors
	Vector<Int> tmpInt(rowRef.nrow(),0);

	tmpInt.set(vb->arrayId());
    outputMsCols_p->arrayId().putColumnCells(rowRef, tmpInt);

    tmpInt.set(vb->fieldId());
    outputMsCols_p->fieldId().putColumnCells(rowRef, tmpInt);

    tmpInt.set(vb->dataDescriptionId());
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

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MsTransformDataHandler::writeVector(const Vector<T> &inputVector,ScalarColumn<T> &outputarray, RefRows &rowRef)
{
	Bool deleteIt = false;
	IPosition shapeScalarColumns(1,rowRef.nrows());

    Array<T> outputArray(shapeScalarColumns,const_cast<T*>(inputVector.getStorage(deleteIt)),SHARE);
    outputarray.putColumnCells(rowRef, outputArray);

	return;
}


// ----------------------------------------------------------------------------------------
// Fill main (data) columns which have to be combined together to produce bigger SPWs
// ----------------------------------------------------------------------------------------
void MsTransformDataHandler::fillDataCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
	for (dataColMap::iterator iter = dataColMap_p.begin();iter != dataColMap_p.end();iter++)
	{
		switch (iter->first)
		{
			case MS::DATA:
			{
				writeCube(vb->visCube(),outputMsCols_p->data(),rowRef);
				break;
			}
			case MS::CORRECTED_DATA:
			{
				if (iter->second == MS::DATA)
				{
					writeCube(vb->visCubeCorrected(),outputMsCols_p->data(),rowRef);
				}
				else
				{
					writeCube(vb->visCubeCorrected(),outputMsCols_p->correctedData(),rowRef);
				}
				break;
			}
			case MS::MODEL_DATA:
			{
				if (iter->second == MS::DATA)
				{
					writeCube(vb->visCubeModel(),outputMsCols_p->data(),rowRef);
				}
				else
				{
					writeCube(vb->visCubeModel(),outputMsCols_p->modelData(),rowRef);
				}
				break;
			}
			case MS::FLOAT_DATA:
			{
				writeCube(vb->visCubeFloat(),outputMsCols_p->floatData(),rowRef);
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

	// Flag cube has to be re-shaped
    writeCube(vb->flagCube(),outputMsCols_p->flag(),rowRef);

	// Weight Spectrum has to be re-shaped
    if (fillWeightSpectrum_p)
    {
    	writeCube(vb->weightSpectrum(),outputMsCols_p->weightSpectrum(),rowRef);
    }

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
template <class T> void MsTransformDataHandler::writeCube(const Cube<T> &inputCube,ArrayColumn<T> &outputarray,RefRows &rowRef)
{
	// Get input cube shape
	IPosition inputCubeShape = inputCube.shape();
	uInt nInputCorrelations = inputCubeShape(0);
	uInt nInputChannels = inputCubeShape(1);

	// Create output plane
	baselineMap::iterator iter = baselineMap_p.begin();
	vector<uInt> baselineRows = iter->second;
	uInt nOutputChannels = nInputChannels*baselineRows.size();
	IPosition planeShape(2,nInputCorrelations, nOutputChannels);
	Matrix<T> outputPlane(planeShape,T());

	uInt baseline_index = 0, channelOffset = 0;
	for (baselineMap::iterator iter = baselineMap_p.begin(); iter != baselineMap_p.end(); iter++)
	{
		// Fill output plane with values from input cube
		channelOffset = 0;
		for (vector<uInt>::iterator row = baselineRows.begin();row != baselineRows.end(); row++)
		{
			for (uInt chan = 0; chan < nInputChannels; chan++)
			{
				uInt outputChannel = chan + channelOffset;
				for (uInt pol = 0; pol < nInputCorrelations; pol++)
				{
					outputPlane(pol,outputChannel) = inputCube(pol,chan,*row);
				}
			}
			channelOffset += nInputChannels;
		}

		// Write output plane
		outputarray.setShape(rowRef.firstRow()+baseline_index,planeShape);
		outputarray.put(rowRef.firstRow()+baseline_index, outputPlane);
		baseline_index += 1;
	}

	return;
}

// ----------------------------------------------------------------------------------------
// Fill auxiliary (meta data) columns which depend on the SPW and have to be merged
// ----------------------------------------------------------------------------------------
void MsTransformDataHandler::fillAuxCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
    Bool deleteIt = false;
    IPosition shapeScalarColumns(1,rowRef.nrows());

    // TODO: UVW has to be re-calculated because it depends on the frequency
    IPosition shapeUvw(2,3,rowRef.nrow());
	Array<Double> uvw(shapeUvw,const_cast<Double*>(vb->uvw().getStorage(deleteIt)),SHARE);
    outputMsCols_p->uvw().putColumnCells(rowRef, uvw);

    // TODO: FLAG_ROW has to be re-calculated because different SPWs may have different flags
	Array<Bool> flagRow(shapeScalarColumns,const_cast<Bool*>(vb->flagRow().getStorage(deleteIt)),SHARE);
    outputMsCols_p->flagRow().putColumnCells(rowRef, flagRow);

	// TODO: Weight Mat has to be re-calculated because different SPWs may have different weights
	IPosition shapeWeights = vb->weightMat().shape();
	shapeWeights(1) = rowRef.nrows();
	Array<Float> weight(shapeWeights,const_cast<Float*>(vb->weightMat().getStorage(deleteIt)),SHARE);
	outputMsCols_p->weight().putColumnCells(rowRef, weight);


	if (correctedToData_p)
	{
		arrayTransformInPlace(weight, tvf::wtToSigma);
		outputMsCols_p->sigma().putColumnCells(rowRef, weight);
	}
	else
	{
		// TODO: Sigma has to be re-calculated because different SPWs may have different sigma
		IPosition shapeSigma = vb->sigmaMat().shape();
		shapeSigma(1) = rowRef.nrows();
		Array<Float> sigma(shapeSigma,const_cast<Float*>(vb->sigmaMat().getStorage(deleteIt)),SHARE);
		outputMsCols_p->sigma().putColumnCells(rowRef, sigma);
	}

    // TODO: FLAG has to be re-calculated because different SPWs may have different flag categories
    if (fillFlagCategory_p)
    {
        IPosition shapeFlagCategory = vb->flagCategory().shape();
        shapeFlagCategory(3) = rowRef.nrow();
        Array<Bool> flagCategory(shapeFlagCategory,const_cast<Bool*>(vb->flagCategory().getStorage(deleteIt)),SHARE);
    	outputMsCols_p->flagCategory().putColumnCells(rowRef, flagCategory);
    }

    return;
}


} //# NAMESPACE CASA - END
