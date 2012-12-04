//# TransformVisDataHandler.h: This file contains the implementation of the TransformVisDataHandler class.
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

#include <transformvis/Transformvis/TransformVisDataHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace tvf
{
	Double wtToSigma(Double wt)
	{
		return wt > 0.0 ? 1.0 / sqrt(wt) : -1.0;
	}
}

/////////////////////////////////////////////
/// TransformVisDataHandler implementation //
/////////////////////////////////////////////

// -----------------------------------------------------------------------
// Default constructor
// -----------------------------------------------------------------------
TransformVisDataHandler::TransformVisDataHandler()
{
	initialize();
	return;
}


// -----------------------------------------------------------------------
// Configuration constructor
// -----------------------------------------------------------------------
TransformVisDataHandler::TransformVisDataHandler(Record configuration)
{
	initialize();
	configure(configuration);
	return;
}


// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
TransformVisDataHandler::~TransformVisDataHandler()
{
	if (visibilityIterator_p) delete visibilityIterator_p;
	if (splitter_p) delete splitter_p;

	return;
}

// -----------------------------------------------------------------------
// Method to initialize members to default values
// -----------------------------------------------------------------------
void TransformVisDataHandler::initialize()
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

	return;
}

// -----------------------------------------------------------------------
// Method to configure (or re-configure) members
// -----------------------------------------------------------------------
void TransformVisDataHandler::configure(Record &configuration)
{
	int exists = 0;

	exists = configuration.fieldNumber ("inputms");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("inputms"), inpMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Input file name is " << inpMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("outputms");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("outputms"), outMsName_p);
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Output file name is " << outMsName_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("spw");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("spw"), spwSelection_p);
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Spectral Window selection is " << spwSelection_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("timebin");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("timebin"), timeBin_p);
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Time interval is " << timeBin_p << LogIO::POST;
	}

	exists = configuration.fieldNumber ("combine");
	if (exists >= 0)
	{
		configuration.get (configuration.fieldNumber ("combine"), combine_p);
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Combine axes are " << combine_p << LogIO::POST;
	}


	return;
}

// -----------------------------------------------------------------------
// Method to open the input MS, select the data and create the
// structure of the output MS filling the auxiliary tables.
// -----------------------------------------------------------------------
void TransformVisDataHandler::open()
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
void TransformVisDataHandler::close()
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
void TransformVisDataHandler::checkFillFlagCategory()
{
	fillFlagCategory_p = False;
	if (!inputMsCols_p->flagCategory().isNull() && inputMsCols_p->flagCategory().isDefined(0))
	{
		fillFlagCategory_p = True;
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Optional column FLAG_CATEGORY found in input MS will be written to output MS" << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check if weight spectrum column has to be filled
// -----------------------------------------------------------------------
void TransformVisDataHandler::checkFillWeightSpectrum()
{
	fillWeightSpectrum_p = False;
	if (!inputMsCols_p->weightSpectrum().isNull() && inputMsCols_p->weightSpectrum().isDefined(0))
	{
		fillWeightSpectrum_p = True;
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Optional column WEIGHT_SPECTRUM found in input MS will be written to output MS" << LogIO::POST;
	}

	return;
}

// -----------------------------------------------------------------------
// Method to check which data columns have to be filled
// -----------------------------------------------------------------------
void TransformVisDataHandler::checkDataColumnsToFill()
{
	dataColMap_p.clear();
	if (colname_p.contains("all"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			dataColMap_p[MS::CORRECTED_DATA] = MS::CORRECTED_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding CORRECTED_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
		{
			dataColMap_p[MS::MODEL_DATA] = MS::MODEL_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding MODEL_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("float_data,data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"FLOAT_DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("float_data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::FLOAT_DATA] = MS::FLOAT_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding FLOAT_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"FLOAT_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("lag_data,data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"LAG_DATA column requested but not available in input MS "<< LogIO::POST;
		}

		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("lag_data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::LAG_DATA)))
		{
			dataColMap_p[MS::LAG_DATA] = MS::LAG_DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding LAG_DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"LAG_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("data"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::DATA)))
		{
			dataColMap_p[MS::DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS "<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("corrected"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::CORRECTED_DATA)))
		{
			dataColMap_p[MS::CORRECTED_DATA] = MS::DATA;
			correctedToData_p = True;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS from input CORRECTED_DATA column"<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"CORRECTED_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}
	else if (colname_p.contains("model"))
	{
		if (selectedInputMs_p->tableDesc().isColumn(MS::columnName(MS::MODEL_DATA)))
		{
			dataColMap_p[MS::MODEL_DATA] = MS::DATA;
			dataColMap_p[MS::CORRECTED_DATA] = MS::DATA;
			logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
								"Adding DATA column to output MS from input MODEL_DATA column"<< LogIO::POST;
		}
		else
		{
			logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__) <<
					"MODEL_DATA column requested but not available in input MS "<< LogIO::POST;
		}
	}

	return;
}

// -----------------------------------------------------------------------
// Determine sort columns order
// -----------------------------------------------------------------------
void TransformVisDataHandler::setIterationApproach()
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
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Combining data through scans for time average " << LogIO::POST;
	}

	if (!combine_p.contains("state"))
	{
		sortColumns_p[sortColumnIndex] =  MS::STATE_ID;
		sortColumnIndex += 1;
	}
	else
	{
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Combining data through state for time average" << LogIO::POST;
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
		logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__) << "Combining data from selected spectral windows" << LogIO::POST;
	}

	sortColumns_p[sortColumnIndex] =  MS::TIME;

	return;
}

// -----------------------------------------------------------------------
// Generate the initial iterator
// -----------------------------------------------------------------------
void TransformVisDataHandler::generateIterator()
{
	visibilityIterator_p = new vi::VisibilityIterator2(*selectedInputMs_p,sortColumns_p,false,NULL,false,timeBin_p);
	return;
}

// -----------------------------------------------------------------------
// Check configuration and input MS characteristics to determine run parameters
// -----------------------------------------------------------------------
void TransformVisDataHandler::setup()
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
void TransformVisDataHandler::fillOutputMs(vi::VisBuffer2 *vb)
{
	// Calculate number of rows to add to the output MS depending on the combination parameters
	uInt rowsToAdd = 0;
	if (combine_p.contains("spw"))
	{
		Vector<Int> ddis = vb->dataDescriptionIds();
		map<Int,uInt> spwRowCounters;
		for (uInt index = 0;index<ddis.size();index++)
		{
			Int spw = ddis(index);
			if (spwRowCounters.find(spw) != spwRowCounters.end())
			{
				spwRowCounters[spw] += 1;
			}
			else
			{
				spwRowCounters[spw] = 1;
			}
		}

		rowsToAdd = spwRowCounters[ddis(0)];
		for (map<Int,uInt>::iterator iter = spwRowCounters.begin();iter != spwRowCounters.end();iter++)
		{
			if (iter->second != rowsToAdd)
			{
				logger_p << LogIO::WARN << LogOrigin("TransformVisDataHandler", __FUNCTION__)
						<< "Data Description " << iter->first
						<< " has different number of rows from the rest, skipping" << LogIO::POST;
			}
		}
	}
	else
	{
		rowsToAdd = vb->nRows();
	}

	logger_p << LogIO::NORMAL << LogOrigin("TransformVisDataHandler", __FUNCTION__)
			<< "Adding " << rowsToAdd << " rows to output MS from "
			<<  vb->nRows() << " rows in input MS selection" << LogIO::POST;

	uInt currentRows = outputMs_p->nrow();
	RefRows rowRef( currentRows, currentRows + rowsToAdd - 1);

	outputMs_p->addRow(rowsToAdd,True);

    fillAuxCols(vb,rowRef);
    fillDataCols(vb,rowRef);

    return;

}


// -----------------------------------------------------------------------
// Fill auxiliary (meta data) columns
// -----------------------------------------------------------------------
void TransformVisDataHandler::fillAuxCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
    Bool deleteIt = false;

    // For row-constant columns we have to create new vectors
	Vector<Int> tmpInt(rowRef.nrow(),0);

	tmpInt.set(vb->arrayId());
    outputMsCols_p->arrayId().putColumnCells(rowRef, tmpInt);

    tmpInt.set(vb->fieldId());
    outputMsCols_p->fieldId().putColumnCells(rowRef, tmpInt);

    tmpInt.set(vb->dataDescriptionId());
	outputMsCols_p->dataDescId().putColumnCells(rowRef, tmpInt);


	// For row-variable columns we use re-sized vectors re-using the storage
    IPosition shapeScalarColumns(1,rowRef.nrows());

    Array<Int> antenna1(shapeScalarColumns,const_cast<Int*>(vb->antenna1().getStorage(deleteIt)),SHARE);
    outputMsCols_p->antenna1().putColumnCells(rowRef, antenna1);

    Array<Int> antenna2(shapeScalarColumns,const_cast<Int*>(vb->antenna2().getStorage(deleteIt)),SHARE);
	outputMsCols_p->antenna2().putColumnCells(rowRef, antenna2);

	Array<Int> feed1(shapeScalarColumns,const_cast<Int*>(vb->feed1().getStorage(deleteIt)),SHARE);
    outputMsCols_p->feed1().putColumnCells(rowRef, feed1);

	Array<Int> feed2(shapeScalarColumns,const_cast<Int*>(vb->feed2().getStorage(deleteIt)),SHARE);
    outputMsCols_p->feed2().putColumnCells(rowRef, feed2);

	Array<Int> processorId(shapeScalarColumns,const_cast<Int*>(vb->processorId().getStorage(deleteIt)),SHARE);
    outputMsCols_p->processorId().putColumnCells(rowRef, processorId);

	Array<Int> observationId(shapeScalarColumns,const_cast<Int*>(vb->observationId().getStorage(deleteIt)),SHARE);
    outputMsCols_p->observationId().putColumnCells(rowRef, observationId);

	Array<Int> scan(shapeScalarColumns,const_cast<Int*>(vb->scan().getStorage(deleteIt)),SHARE);
    outputMsCols_p->scanNumber().putColumnCells(rowRef, scan);

	Array<Int> stateId(shapeScalarColumns,const_cast<Int*>(vb->stateId().getStorage(deleteIt)),SHARE);
    outputMsCols_p->stateId().putColumnCells(rowRef, stateId);

	Array<Double> time(shapeScalarColumns,const_cast<Double*>(vb->time().getStorage(deleteIt)),SHARE);
    outputMsCols_p->time().putColumnCells(rowRef, time);

	Array<Double> timeCentroid(shapeScalarColumns,const_cast<Double*>(vb->timeCentroid().getStorage(deleteIt)),SHARE);
    outputMsCols_p->timeCentroid().putColumnCells(rowRef, timeCentroid);

	Array<Double> timeInterval(shapeScalarColumns,const_cast<Double*>(vb->timeInterval().getStorage(deleteIt)),SHARE);
    outputMsCols_p->interval().putColumnCells(rowRef, timeInterval);

	Array<Double> exposure(shapeScalarColumns,const_cast<Double*>(vb->exposure().getStorage(deleteIt)),SHARE);
    outputMsCols_p->exposure().putColumnCells(rowRef, exposure);

    // TODO: UVW has to be re-calculated because it depends on the frequency
    IPosition shapeUvw(2,3,rowRef.nrow());
	Array<Double> uvw(shapeUvw,const_cast<Double*>(vb->uvw().getStorage(deleteIt)),SHARE);
    outputMsCols_p->uvw().putColumnCells(rowRef, uvw);

    // TODO: FLAG_ROW has to be re-calculated because different SPWs may have different flags
	Array<Bool> flagRow(shapeScalarColumns,const_cast<Bool*>(vb->flagRow().getStorage(deleteIt)),SHARE);
    outputMsCols_p->flagRow().putColumnCells(rowRef, flagRow);

    // TODO: FLAG has to be re-calculated because different SPWs may have different flags
    IPosition shapeFlags = vb->flagCube().shape();
    shapeFlags(2) = rowRef.nrow();
	Array<Bool> flag(shapeFlags,const_cast<Bool*>(vb->flag().getStorage(deleteIt)),SHARE);
    outputMsCols_p->flag().putColumnCells(rowRef, flag);

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

// -----------------------------------------------------------------------
// Fill main (data) columns
// -----------------------------------------------------------------------
void TransformVisDataHandler::fillDataCols(vi::VisBuffer2 *vb,RefRows &rowRef)
{
    Bool deleteIt = false;

	for (dataColMap::iterator iter = dataColMap_p.begin();iter != dataColMap_p.end();iter++)
	{
		switch (iter->first)
		{
			case MS::DATA:
			{
				IPosition shapeCube = vb->visCube().shape();
				shapeCube(1) = (shapeCube(2)/rowRef.nrow())*shapeCube(1);
				shapeCube(2) = rowRef.nrow();
				Array<Complex> visCube(shapeCube,const_cast<Complex*>(vb->visCube().getStorage(deleteIt)),SHARE);
				outputMsCols_p->data().putColumnCells(rowRef, visCube);
				break;
			}
			case MS::CORRECTED_DATA:
			{
				if (iter->second == MS::DATA)
				{
					IPosition shapeCube = vb->visCubeCorrected().shape();
					shapeCube(1) = (shapeCube(2)/rowRef.nrow())*shapeCube(1);
					shapeCube(2) = rowRef.nrow();
					Array<Complex> visCube(shapeCube,const_cast<Complex*>(vb->visCubeCorrected().getStorage(deleteIt)),SHARE);
					outputMsCols_p->data().putColumnCells(rowRef, visCube);
				}
				else
				{
					IPosition shapeCube = vb->visCubeCorrected().shape();
					shapeCube(1) = (shapeCube(2)/rowRef.nrow())*shapeCube(1);
					shapeCube(2) = rowRef.nrow();
					Array<Complex> visCube(shapeCube,const_cast<Complex*>(vb->visCubeCorrected().getStorage(deleteIt)),SHARE);
					outputMsCols_p->correctedData().putColumnCells(rowRef, visCube);
				}
				break;
			}
			case MS::MODEL_DATA:
			{
				if (iter->second == MS::DATA)
				{
					IPosition shapeCube = vb->visCubeCorrected().shape();
					shapeCube(1) = (shapeCube(2)/rowRef.nrow())*shapeCube(1);
					shapeCube(2) = rowRef.nrow();
					Array<Complex> visCube(shapeCube,const_cast<Complex*>(vb->visCubeModel().getStorage(deleteIt)),SHARE);
					outputMsCols_p->data().putColumnCells(rowRef, visCube);
				}
				else
				{
					IPosition shapeCube = vb->visCubeCorrected().shape();
					shapeCube(1) = (shapeCube(2)/rowRef.nrow())*shapeCube(1);
					shapeCube(2) = rowRef.nrow();
					Array<Complex> visCube(shapeCube,const_cast<Complex*>(vb->visCubeModel().getStorage(deleteIt)),SHARE);
					outputMsCols_p->modelData().putColumnCells(rowRef, visCube);
				}
				break;
			}
			case MS::FLOAT_DATA:
			{
				IPosition shapeCube = vb->visCubeFloat().shape();
				shapeCube(1) = (shapeCube(2)/rowRef.nrow())*shapeCube(1);
				shapeCube(2) = rowRef.nrow();
				Array<Float> visCube(shapeCube,const_cast<Float*>(vb->visCubeFloat().getStorage(deleteIt)),SHARE);
				outputMsCols_p->floatData().putColumnCells(rowRef, visCube);
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
		IPosition shapeSigma = vb->sigmaMat().shape();
		shapeSigma(1) = rowRef.nrows();
		Array<Float> sigma(shapeSigma,const_cast<Float*>(vb->sigmaMat().getStorage(deleteIt)),SHARE);
		outputMsCols_p->sigma().putColumnCells(rowRef, sigma);
	}


    if (fillWeightSpectrum_p)
    {
		IPosition shapeWeightSpectrum= vb->weightSpectrum().shape();
		shapeWeightSpectrum(2) = rowRef.nrows();
		Array<Float> weightSpectrum(shapeWeightSpectrum,const_cast<Float*>(vb->weightSpectrum().getStorage(deleteIt)),SHARE);
		outputMsCols_p->weightSpectrum().putColumnCells(rowRef, weightSpectrum);
    }

	return;
}


} //# NAMESPACE CASA - END
