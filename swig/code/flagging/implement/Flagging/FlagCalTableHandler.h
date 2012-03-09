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

namespace casa { //# NAMESPACE CASA - BEGIN

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
	void generateScanStartStopMap();

};

} //# NAMESPACE CASA - END

#endif /* FlagCalTableHandler_H_ */
