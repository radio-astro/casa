//# GBTHoloFiller: A GBTBackendFiller for Holography data
//# Copyright (C) 1999,2000,2001,2002
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
//#
//# $Id$

#ifndef NRAO_GBTHOLOFILLER_H
#define NRAO_GBTHOLOFILLER_H

//#! Includes go here
 #include <casa/aips.h>
#include <nrao/FITS/GBTAntennaFile.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Containers/Block.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Regex.h>
#include <tables/Tables/ScalarColumn.h>

#include <casa/namespace.h>

//# Forward Declarations
class GBTBackendTable;
class GBTMSTypeFiller;
class GBTMSTypeDataFiller;
class GBTIFFiller;
class GBTGOFiller;
class GBTRcvrCalFiller;
class GBTStateTable;

// <summary>
// A GBTBackendFiller for Holography data
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=GBTBackendFiller>GBTBackendFIller</linkto>
//   <li> The GBT DCR FITS file format.
// </prerequisite>
//
// <etymology>
// This fills GBT Holography data to a MS, hence GBTHoloFiller.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// There exist holography data at the GBT which needs to be filled.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> probably many things.
// </todo>

class GBTHoloFiller : public GBTBackendFiller
{
public:
    // construct one
    GBTHoloFiller(const String &msName, const Regex &object);
    
    ~GBTHoloFiller();

    virtual GBTScanLogReader::BACKENDS type() {return GBTScanLogReader::HOLOGRAPHY;}

    // fill a given backend file and DAP files
    virtual Bool fill(const String &backendFile,
		      const Block<String> &dapFiles,
		      const String &ifManagerFile,
		      const Block<String> &rxCalInfoFiles,
		      const GBTGOFile &GOFile,
		      const String &antennaFile,
		      const GBTStateTable &masterState,
		      const GBTLO1File &LO1A,
		      const GBTLO1File &LO1B,
		      const GBTMSFillOptions &fillOptions);

private:
    GBTBackendTable *holoTab_p;
    String msName_p;
    Regex object_p;

    // the associated tables
    GBTMSTypeDataFiller *typeDataTab_p;
    GBTMSTypeFiller *typeTab_p;

    // the backend ID column
    ScalarColumn<Int> ifId_p;

    Table *gbtPointingTab_p, *gbtFocusTab_p;

    // IF manager filler
    GBTIFFiller *ifFiller_p;

    // ANTENNA file
    GBTAntennaFile antennaFile_p;

    // RX CAL INFO files filler
    GBTRcvrCalFiller *rxCalFiller_p;

    // GO (user interface) filler
    GBTGOFiller *GOFiller_p;

    void attachColumns();

    // unset and unavailable
    GBTHoloFiller();
    GBTHoloFiller(const GBTHoloFiller &);
    GBTHoloFiller &operator=(const GBTHoloFiller &);
};

#endif

