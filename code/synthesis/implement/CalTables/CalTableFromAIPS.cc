//# CalTable.cc: Implementation of CalTable.h
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
//----------------------------------------------------------------------------

#include <synthesis/CalTables/CalTableFromAIPS.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Arrays.h>
#include <casa/Arrays/ArrayMath.h>
#include <synthesis/MSVis/MSCalEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

CalTableFromAIPS::~CalTableFromAIPS()
{
// Default desctructor
//    itsMainTable         Table*           Ptr to cal_main Table object
//    itsDescTable         Table*           Ptr to cal_desc Table object
//    itsHistoryTable      Table*           Ptr to cal_history Table object
// are already destroyed by the virtual base class constructor ~CalTable()

};

//----------------------------------------------------------------------------

CalTableFromAIPS::CalTableFromAIPS (const String& fitsFileName, const String& tableName)
{
// Construct from a AIPS FITS file name and a cal table name

  // Stephen Bourke's code to go here
};


} //# NAMESPACE CASA - END

