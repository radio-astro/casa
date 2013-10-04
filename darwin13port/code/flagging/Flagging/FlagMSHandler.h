//# FlagMSHandler.h: This file contains the interface definition of the FlagMSHandler class.
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

#ifndef FlagMSHandler_H_
#define FlagMSHandler_H_

#include <flagging/Flagging/FlagDataHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Flag Data Handler class definition
class FlagMSHandler: public FlagDataHandler
{

public:

	// Default constructor
	// NOTE: Time interval 0 groups all time steps together in one chunk.
	FlagMSHandler(string msname, uShort iterationApproach = SUB_INTEGRATION, Double timeInterval = 0);

	// Default destructor
	~FlagMSHandler();

	// Open Measurement Set
	bool open();

	// Close Measurement Set
	bool close();

	// Generate selected Measurement Set
	bool selectData();

	// Parse MSSelection expression
	bool parseExpression(MSSelection &parser);

	// Generate Visibility Iterator
	bool generateIterator();

	// Move to next chunk
	bool nextChunk();

	// Move to next buffer
	bool nextBuffer();

	// Write flag cube into MS
	bool flushFlags();

	// Provide table name (for flag version)
	String getTableName();

	// Check if a given column is available in the MS
	// (typically for checking existence of CORRECTED_DATA
	bool checkIfColumnExists(String column);

	// Signal true when a progress summary has to be printed
	bool summarySignal();

	// Get the MS PROCESSOR sub-table
	bool processorTable();

	// Get a Float visCube and return a Complex one
	Cube<Complex>& weightVisCube();

	Cube<Complex> weight_spectrum_p;

private:

	// Mapping functions
	virtual void generateScanStartStopMap();

	// Swap MS to check what is the maximum RAM memory needed
	void preSweep();

	// Apply channel selection for asyn or normal iterator
	// NOTE: We always have to do this, even if there is no SPW:channel selection
	void applyChannelSelection(vi::VisibilityIterator2 *visIter);


	// Measurement set section
	MeasurementSet *selectedMeasurementSet_p;
	MeasurementSet *originalMeasurementSet_p;

	// RO Visibility Iterator
	vi::VisibilityIterator2 *visibilityIterator_p;

};

} //# NAMESPACE CASA - END

#endif /* FlagMSHandler_H_ */
