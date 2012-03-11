//# FlagCalTableHandler.h: This file contains the implementation of the FlagCalTableHandler class.
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

#include <flagging/Flagging/FlagCalTableHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//////////////////////////////////////////
/// FlagCalTableHandler implementation ///
//////////////////////////////////////////

// -----------------------------------------------------------------------
// Default constructor
// -----------------------------------------------------------------------
FlagCalTableHandler::FlagCalTableHandler(string msname, uShort iterationApproach, Double timeInterval):
		FlagDataHandler(msname,iterationApproach,timeInterval)
{
	selectedCalTable_p = NULL;
	originalCalTable_p = NULL;
	calTableInterface_p = NULL;
	calBuffer_p = NULL;
	calIter_p = NULL;
}


// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
FlagCalTableHandler::~FlagCalTableHandler()
{
	if (calBuffer_p) delete calBuffer_p;
	if (calIter_p) delete calIter_p;
	if (calTableInterface_p) delete calTableInterface_p;
	if (selectedCalTable_p) delete selectedCalTable_p;
	if (originalCalTable_p) delete originalCalTable_p;
}


// -----------------------------------------------------------------------
// Open CalTable
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::open()
{
	if (originalCalTable_p) delete originalCalTable_p;
	originalCalTable_p = new NewCalTable(msname_p,Table::Update);

	// Read field names
	ROMSFieldColumns *fieldSubTable = new ROMSFieldColumns(originalCalTable_p->field());
	fieldNames_p = new Vector<String>(fieldSubTable->name().getColumn());
	*logger_p << LogIO::DEBUG1 << "Field names are " << *fieldNames_p << LogIO::POST;

	// Read antenna names and diameters from Antenna table
	ROMSAntennaColumns *antennaSubTable = new ROMSAntennaColumns(originalCalTable_p->antenna());
	antennaNames_p = new Vector<String>(antennaSubTable->name().getColumn());
	antennaDiameters_p = new Vector<Double>(antennaSubTable->dishDiameter().getColumn());
	antennaPositions_p = new ROScalarMeasColumn<MPosition>(antennaSubTable->positionMeas());
	*logger_p << LogIO::DEBUG1 << "There are " << antennaNames_p->size() << " antennas with names: " << *antennaNames_p << LogIO::POST;

	return true;
}


// -----------------------------------------------------------------------
// Close CalTable
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::close()
{
	if (selectedCalTable_p)
	{
		selectedCalTable_p->flush();
		selectedCalTable_p->relinquishAutoLocks(True);
		selectedCalTable_p->unlock();
	}

	return true;
}


// -----------------------------------------------------------------------
// Generate selected CalTable
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::selectData()
{
	logger_p->origin(LogOrigin("FlagCalTableHandler",__FUNCTION__,WHERE));

	if (calTableInterface_p) delete calTableInterface_p;
	calTableInterface_p = new CTInterface(*originalCalTable_p);

	if (measurementSetSelection_p) delete measurementSetSelection_p;
	const String dummyExpr = String("");
	measurementSetSelection_p = new MSSelection();
	measurementSetSelection_p->reset(*calTableInterface_p,
									MSSelection::PARSE_LATE,
									(const String)timeSelection_p,
									(const String)baselineSelection_p,
									(const String)fieldSelection_p,
									(const String)spwSelection_p,
									(const String)uvwSelection_p,
									dummyExpr, // taqlExpr
									(const String)polarizationSelection_p,
									(const String)scanSelection_p,
									(const String)arraySelection_p,
									(const String)scanIntentSelection_p,
									(const String)observationSelection_p);


	if (selectedCalTable_p) delete selectedCalTable_p;
	TableExprNode ten = measurementSetSelection_p->toTableExprNode(calTableInterface_p);
	selectedCalTable_p = new NewCalTable();
	Bool madeSelection = getSelectedTable(*selectedCalTable_p,*originalCalTable_p,ten,String(""));
	if (madeSelection == False) selectedCalTable_p = originalCalTable_p;

	// Check if selected CalTable has rows...
	if (selectedCalTable_p->nrow() == 0)
	{
		*logger_p << LogIO::WARN << "Selected CalTable doesn't have any rows " << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL 	<< "Original CalTable has "
									<< originalCalTable_p->nrow()
									<< " rows, and selected CalTable has "
									<< selectedCalTable_p->nrow()
									<< " rows" << LogIO::POST;
	}

	return true;
}


// -----------------------------------------------------------------------
// Parse MSSelection expression
// -----------------------------------------------------------------------
void
FlagCalTableHandler::parseExpression(MSSelection &parser)
{
	parser.toTableExprNode(calTableInterface_p);
	return;
}


// -----------------------------------------------------------------------
// Generate CalIter with a given sort order and time interval
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::generateIterator()
{
	// Generate CalIterator
	if (calIter_p) delete calIter_p;
	calIter_p = new CTIter(*selectedCalTable_p,getSortColumns(sortOrder_p));

	// Attach to CalBuffer
	if (calBuffer_p) delete calBuffer_p;
	calBuffer_p = new CTBuffer(calIter_p);

	// Put VisBuffer wrapper around CalBuffer
	if (visibilityBuffer_p) delete visibilityBuffer_p;
	visibilityBuffer_p = new VisBufferAutoPtr();
	visibilityBuffer_p->set(calBuffer_p);

	iteratorGenerated_p = true;
	return true;
}


// -----------------------------------------------------------------------
// Translate sorting columns from Block<Int> format to Block<string> format
// -----------------------------------------------------------------------
Block<String>
FlagCalTableHandler::getSortColumns(Block<Int> intCols)
{
	Block<String> strCols(2);
	strCols[0] = "FIELD_ID";
	strCols[1] = "SPECTRAL_WINDOW_ID";

	return strCols;
}


// -----------------------------------------------------------------------
// Move to next chunk
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::nextChunk()
{
	logger_p->origin(LogOrigin("FlagCalTableHandler",__FUNCTION__,WHERE));

	msCounts_p += chunkCounts_p;
	chunkCounts_p = 0;
	bool moreChunks = false;
	if (stopIteration_p)
	{
		moreChunks = false;
	}
	else
	{
		if (!chunksInitialized_p)
		{
			if (!iteratorGenerated_p) generateIterator();
			calIter_p->reset();
			chunksInitialized_p = true;
			buffersInitialized_p = false;
			chunkNo++;
			bufferNo = 0;
			moreChunks = true;
		}
		else
		{
			calIter_p->next();

			if (!calIter_p->pastEnd())
			{
				buffersInitialized_p = false;
				moreChunks = true;
				chunkNo++;
				bufferNo = 0;
			}
		}
	}

	if (!moreChunks)
	{
		*logger_p << LogIO::NORMAL << "==================================================================================== " << LogIO::POST;
	}

	return moreChunks;
}


// -----------------------------------------------------------------------
// Move to next buffer
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::nextBuffer()
{
	bool moreBuffers = false;
	if (stopIteration_p)
	{
		moreBuffers = false;
	}
	else
	{
		if (!buffersInitialized_p)
		{
			visibilityBuffer_p->get()->invalidate();
			if (!asyncio_enabled_p) preFetchColumns();
			if (mapPolarizations_p) generatePolarizationsMap();
			buffersInitialized_p = true;
			flushFlags_p = false;
			flushFlagRow_p = false;
			bufferNo++;

			moreBuffers = true;
		}
		else
		{
			// In CalTables there is only one iteration level
			moreBuffers = false;
		}
	}

	// Print chunk characteristics
	if (moreBuffers)
	{
		logger_p->origin(LogOrigin("FlagCalTableHandler",__FUNCTION__,WHERE));

		// Get flag  (WARNING: We have to modify the shape of the cube before re-assigning it)
		Cube<Bool> curentFlagCube= visibilityBuffer_p->get()->flagCube();
		modifiedFlagCube_p.resize(curentFlagCube.shape());
		modifiedFlagCube_p = curentFlagCube;
		originalFlagCube_p.resize(curentFlagCube.shape());
		originalFlagCube_p = curentFlagCube;

		// Get flag row (WARNING: We have to modify the shape of the cube before re-assigning it)
		// NOTE: There is no FlagRow in CalTables yet, but we have it here for compatibility reasons
		modifiedFlagRow_p.resize(visibilityBuffer_p->get()->nRow());
		originalFlagRow_p.resize(visibilityBuffer_p->get()->nRow());

		// Compute total number of flags per buffer to be used for generating the agents stats
		chunkCounts_p += curentFlagCube.shape().product();

		// Print chunk characteristics
		if (bufferNo == 1)
		{
			Vector<Int> scan = visibilityBuffer_p->get()->scan();
			String corrs = "[ ";
			for (uInt corr_i=0;corr_i<visibilityBuffer_p->get()->nCorr();corr_i++)
			{
				corrs += (*polarizationIndexMap_p)[corr_i] + " ";
			}
			corrs += "]";

			*logger_p << LogIO::NORMAL <<
					"------------------------------------------------------------------------------------ " << LogIO::POST;
			*logger_p << "Chunk = " << chunkNo <<
					", Scan = " << scan[0] << "~" << scan[scan.size()-1] <<
					", Field = " << visibilityBuffer_p->get()->fieldId() << " (" << fieldNames_p->operator()(visibilityBuffer_p->get()->fieldId()) << ")"
					", Spw = " << visibilityBuffer_p->get()->spectralWindow() <<
					", Channels = " << visibilityBuffer_p->get()->nChannel() <<
					", Corrs = " << corrs <<
					", Total Rows = " << visibilityBuffer_p->get()->nRowChunk() << LogIO::POST;
		}
	}

	return moreBuffers;
}

void
FlagCalTableHandler::generatePolarizationsMap()
{
	Cube<Bool> curentFlagCube= visibilityBuffer_p->get()->flagCube();

	if (polarizationIndexMap_p) delete polarizationIndexMap_p;
	polarizationIndexMap_p = new polarizationIndexMap();
	for (uInt corr_i=0;corr_i<curentFlagCube.shape()[0];corr_i++)
	{
		stringstream corrStream;
		corrStream << corr_i;
		(*polarizationIndexMap_p)[corr_i] = "Corr-" + corrStream.str();
	}

	return;
}


// -----------------------------------------------------------------------
// Flush flags to CalTable
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::flushFlags()
{
	if (flushFlags_p)
	{
		calIter_p->setflag(modifiedFlagCube_p);
		flushFlags_p = false;
	}

	return true;
}


// -----------------------------------------------------------------------
// Flush flags to CalTable
// -----------------------------------------------------------------------
String
FlagCalTableHandler::getTableName()
{
	return originalCalTable_p->tableName();
}


//////////////////////////////////////////
//////// CTBuffer implementation ////////
//////////////////////////////////////////

CTBuffer::CTBuffer(CTIter *calIter): calIter_p(calIter)
{
	invalidate();
	CTnRowOK_p = False;
	nRow();
}

CTBuffer::~CTBuffer()
{

}

Int CTBuffer::fieldId() const
{
	return  calIter_p->field()[0];
}

Int& CTBuffer::spectralWindow()
{
	if (!CTspectralWindowOK_p)
	{
		Vector<Int> tmp = calIter_p->spw();
		spw_p.resize(tmp.size(),False);
		spw_p = tmp;
		spw0_p = spw_p[0];
		CTspectralWindowOK_p = True;
	}

	return spw0_p;
}

Vector<Int>& CTBuffer::scan()
{
	if (!CTscanOK_p)
	{
		Vector<Int> tmp = calIter_p->scan();
		scan_p.resize(tmp.size(),False);
		scan_p = tmp;
		CTscanOK_p = True;
	}

	return scan_p;
}

Vector<Double>& CTBuffer::time()
{
	if (!CTtimeOK_p)
	{
		Vector<Double> tmp = calIter_p->time();
		time_p.resize(tmp.size(),False);
		time_p = tmp;
		CTtimeOK_p = True;
	}

	return time_p;
}

Vector<Int>& CTBuffer::antenna1()
{
	if (!CTantenna1OK_p)
	{
		Vector<Int> tmp = calIter_p->antenna1();
		antenna1_p.resize(tmp.size(),False);
		antenna1_p = tmp;
		CTantenna1OK_p = True;
	}

	return antenna1_p;
}

Vector<Int>& CTBuffer::antenna2()
{
	if (!CTantenna2OK_p)
	{
		Vector<Int> tmp = calIter_p->antenna2();
		antenna2_p.resize(tmp.size(),False);
		antenna2_p = tmp;
		CTantenna2OK_p = True;
	}

	return antenna2_p;
}

Cube<Bool>& CTBuffer::flagCube()
{
	if (!CTflagCubeOk_p)
	{
		Cube<Bool> tmp = calIter_p->flag();
		flagCube_p.resize(tmp.shape(),False);
		flagCube_p = tmp;
		CTflagCubeOk_p = True;

		// Also fill shapes (nCorr cannot be retrieved at access time because of the VisBuffer nrowChunk signature, which is const)
		nRowChunk_p = flagCube_p.shape()[2];
		nChannel_p = flagCube_p.shape()[1];
		nCorr_p = flagCube_p.shape()[0];
	}

	return flagCube_p;
}

Vector<Int>& CTBuffer::observationId()
{
	if (!CTobservationIdOK_p)
	{
		if (!CTflagCubeOk_p) flagCube();
		observationId_p.resize(flagCube_p.shape()[2]);
		observationId_p[0] = 0;
		CTobservationIdOK_p = True;
	}

	return observationId_p;
}

Vector<Int>& CTBuffer::channel()
{
	if (!CTchannelOK_p)
	{
		Vector<Int> tmp = calIter_p->chan();
		channel_p.resize(tmp.size(),False);
		channel_p = tmp;
		CTchannelOK_p = True;
	}

	return channel_p;
}

Vector<Double>& CTBuffer::frequency()
{
	if (!CTchannelOK_p)
	{
		Vector<Double> tmp = calIter_p->freq();
		frequency_p.resize(tmp.size(),False);
		frequency_p = tmp;
		CTchannelOK_p = True;
	}

	return frequency_p;
}

Int& CTBuffer::nRow()
{
	if (!CTnRowOK_p)
	{
		nRow_p = calIter_p->nrow();
	}

	return nRow_p;
}

Int CTBuffer::nRowChunk() const
{
	return nRowChunk_p;
}

Int& CTBuffer::nChannel()
{
	if (!CTnChannelOK_p)
	{
		if (!CTflagCubeOk_p) flagCube();
		nChannel_p = flagCube_p.shape()[1];
	}

	return nChannel_p;
}

Int& CTBuffer::nCorr()
{
	if (!CTnCorr_pOK_p)
	{
		if (!CTflagCubeOk_p) flagCube();
		nCorr_p = flagCube_p.shape()[0];
	}

	return nCorr_p;
}

void CTBuffer::invalidate()
{
	CTfieldIdOK_p = False;
	CTspectralWindowOK_p = False;
	CTscanOK_p = False;
	CTtimeOK_p = False;
	CTantenna1OK_p = False;
	CTantenna2OK_p = False;
	CTflagCubeOk_p = False;
	CTobservationIdOK_p = False;
	CTchannelOK_p = False;
	CTfrequencyOK_p = False;
	CTnRowChunkOK_p = False;
	CTnChannelOK_p = False;
	CTnCorr_pOK_p = False;

	return;
}

} //# NAMESPACE CASA - END

