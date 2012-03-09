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

}


// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
FlagCalTableHandler::~FlagCalTableHandler()
{

}


// -----------------------------------------------------------------------
// Open CalTable
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::open()
{
	return true;
}


// -----------------------------------------------------------------------
// Close CalTable
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::close()
{
	return true;
}


// -----------------------------------------------------------------------
// Generate selected CalTable
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::selectData()
{
	return true;
}


// -----------------------------------------------------------------------
// Parse MSSelection expression
// -----------------------------------------------------------------------
void
FlagCalTableHandler::parseExpression(MSSelection &parser)
{
	return;
}


// -----------------------------------------------------------------------
// Generate CalIter with a given sort order and time interval
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::generateIterator()
{
	return true;
}


// -----------------------------------------------------------------------
// Move to next chunk
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::nextChunk()
{
	return false;
}


// -----------------------------------------------------------------------
// Move to next buffer
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::nextBuffer()
{
	return false;
}


// -----------------------------------------------------------------------
// Flush flags to CalTable
// -----------------------------------------------------------------------
bool
FlagCalTableHandler::flushFlags()
{
	return true;
}


// -----------------------------------------------------------------------
// Flush flags to CalTable
// -----------------------------------------------------------------------
String
FlagCalTableHandler::getTableName()
{
	return String("CalTable");
}


// -----------------------------------------------------------------------
// Generate scan start stop map
// -----------------------------------------------------------------------
void
FlagCalTableHandler::generateScanStartStopMap()
{
	return;
}

} //# NAMESPACE CASA - END

