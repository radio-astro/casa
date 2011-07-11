//# CalTable.h: Calibration table access and creation
//# Copyright (C) 1996,1997,1998,1999,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef CALIBRATION_CALTABLEFROMAIPS_H
#define CALIBRATION_CALTABLEFROMAIPS_H

#include <calibration/CalTables/CalTable.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// CalTableFromAIPS: Calibration table creation from AIPS FITS file
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration" and "table" and "AIPS".
// </etymology>
//
// <synopsis>
// The CalTable classes provide basic access to calibration tables.
// This child class of CalTable adds the possibility to construct
// a caltable from an AIPS FITS file.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// Provide way to transform calibrations perfromed in AIPS to be
// transferred into CASA, e.g., for VLBI analysis.
// </motivation>
//
// <todo asof="10/05/07">
// </todo>

class CalTableFromAIPS : public CalTable
{
 public:
    // Default destructor
    ~CalTableFromAIPS();

    // Construct from a specified FITS file name and table name.
    CalTableFromAIPS(const String& fitsFileName, const String& tableName);

 protected:

 private:

 };


} //# NAMESPACE CASA - END

#endif
   
