//# GBTDCRFiller: A GBTBackendFiller for DCR data
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

#ifndef NRAO_GBTDCRFILLER_H
#define NRAO_GBTDCRFILLER_H

//#! Includes go here
#include <casa/aips.h>
#include <nrao/FITS/GBTScanLogReader.h>
#include <nrao/FITS/GBTAntennaFile.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/ScalarColumn.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
template <class T> class Block;
} //# NAMESPACE CASA - END

class GBTBackendTable;
class GBTMSTypeFiller;
class GBTMSTypeDataFiller;
class GBTMSTypeSampFiller;
class GBTMSTypeStateFiller;
class GBTIFFiller;
class GBTGOFiller;
class GBTRcvrCalFiller;
class GBTStateTable;

#include <casa/namespace.h>

// <summary>
// A GBTBackendFiller for DCR data
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
// This fills GBT DCR data to a MS, hence GBTDCRFiller.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// There exist DCR data at the GBT which needs to be filled.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> Probably many things.
// </todo>

class GBTDCRFiller : public GBTBackendFiller
{
public:
    // construct one
    GBTDCRFiller(const String &msName, const Regex &object);
    
    ~GBTDCRFiller();

    virtual GBTScanLogReader::BACKENDS type() {return GBTScanLogReader::DCR;}

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
    GBTBackendTable *dcrTab_p;
    String msName_p;
    Regex object_p;

    ScalarColumn<Int> ifId_p, gbtStateId_p, samplerId_p;

    // the associated tables
    GBTMSTypeDataFiller *typeDataTab_p;
    GBTMSTypeFiller *typeTab_p;
    GBTMSTypeStateFiller *typeStateTab_p;
    GBTMSTypeSampFiller *typeSampTab_p;

    Table *gbtPointingTab_p, *gbtFocusTab_p;

    // IF filler
    GBTIFFiller *ifFiller_p;

    // ANTENNA file
    GBTAntennaFile antennaFile_p;

    // RX CAL INFO files filler
    GBTRcvrCalFiller *rxCalFiller_p;

    // GO (user interface) filler
    GBTGOFiller *GOFiller_p;

    void attachColumns();

    // unset and unavailable
    GBTDCRFiller();
    GBTDCRFiller(const GBTDCRFiller &);
    GBTDCRFiller &operator=(const GBTDCRFiller &);
};

#endif

