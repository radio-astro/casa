//# FlagCalTableHandler.h: This file contains the interface definition of the FlagCalTableHandler class.
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

#ifndef FlagCalTableHandler_H_
#define FlagCalTableHandler_H_

#include <flagging/Flagging/FlagDataHandler.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTInterface.h>
#include <synthesis/CalTables/CTIter.h>
#include <synthesis/CalTables/CalBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class CTBuffer: public VisBuffer
{

public:
	CTBuffer(CTIter *calIter);
	~CTBuffer();

	Int fieldId() const;
	Int& spectralWindow();
	Vector<Int>& scan();
	Vector<Double>& time();
	Vector<Int>& antenna1();
	Vector<Int>& antenna2();
	Cube<Bool>& flagCube();
	Vector<Int>& channel();
	Vector<Double>& frequency();
	Int& nRow();
	Int nRowChunk() const;
	Int& nChannel();
	Int& nCorr();


	// Convenient public methods for compatibility with MS-like interface
	Vector<Int>& observationId();

	// Methods for efficient synchronization with CTIter
	void invalidate();

private:

	CTIter *calIter_p;
	Int field0_p;
	Int spw0_p;
	Vector<Int> field_p;
	Vector<Int> spw_p;
	Vector<Int> scan_p;
	Vector<Double> time_p;
	Vector<Int> antenna1_p;
	Vector<Int> antenna2_p;
	Cube<Bool> flagCube_p;
	Vector<Int> channel_p;
	Vector<Double> frequency_p;
	Int nRow_p;
	Int nRowChunk_p;
	Int nChannel_p;
	Int nCorr_p;

	// Convenient members for compatibility with MS-like interface
	Vector<Int> observationId_p;

	// Members for efficient synchronization with CTIter
	Bool CTfieldIdOK_p;
	Bool CTspectralWindowOK_p;
	Bool CTscanOK_p;
	Bool CTtimeOK_p;
	Bool CTantenna1OK_p;
	Bool CTantenna2OK_p;
	Bool CTflagCubeOk_p;
	Bool CTobservationIdOK_p;
	Bool CTchannelOK_p;
	Bool CTfrequencyOK_p;
	Bool CTnRowOK_p;
	Bool CTnRowChunkOK_p;
	Bool CTnChannelOK_p;
	Bool CTnCorr_pOK_p;

};

// Flag Data Handler class definition
class FlagCalTableHandler: public FlagDataHandler
{

public:

	// Default constructor
	// NOTE: Time interval 0 groups all time steps together in one chunk.
	FlagCalTableHandler(string msname, uShort iterationApproach = SUB_INTEGRATION, Double timeInterval = 0);

	// Default destructor
	~FlagCalTableHandler();

	// Open CalTable
	bool open();

	// Close CalTable
	bool close();

	// Generate selected CalTable
	bool selectData();

	// Parse MSSelection expression
	void parseExpression(MSSelection &parser);

	// Generate CalIter
	bool generateIterator();

	// Move to next chunk
	bool nextChunk();

	// Move to next buffer
	bool nextBuffer();

	// Write flag cube into MS
	bool flushFlags();

	// Provide table name (for flag version)
	String getTableName();

private:

	// Mapping functions
	void generatePolarizationsMap();

	// Convenient private methods for compatibility with MS-like interface
	Block<String> getSortColumns(Block<Int> intCols);

	// CatTable selection objects
	NewCalTable *selectedCalTable_p;
	NewCalTable *originalCalTable_p;
	CTInterface *calTableInterface_p;

	// Calibration Table iterator and buffer
	CTIter *calIter_p;
	CTBuffer *calBuffer_p;

};

} //# NAMESPACE CASA - END

#endif /* FlagCalTableHandler_H_ */
