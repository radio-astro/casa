//# GBTSPFiller: A GBTBackendFiller for Spectral Processor data
//# Copyright (C) 2000,2001,2002
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

#ifndef NRAO_GBTSPFILLER_H
#define NRAO_GBTSPFILLER_H

//#! Includes go here
#include <casa/aips.h>
#include <nrao/FITS/GBTAntennaFile.h>
#include <nrao/FITS/GBTScanLogReader.h>
#include <nrao/GBTFillers/GBTBackendFiller.h>

#include <casa/Utilities/Regex.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/ScalarColumn.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
template <class T> class Block;
template <class T> class RORecordFieldPtr;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

class GBTBackendTable;
class GBTMSTypeFiller;
class GBTMSTypeDataFiller;
class GBTMSTypeSampFiller;
class GBTMSTypeStateFiller;
class GBTIFFiller;
class GBTGOFiller;
class GBTRcvrCalFiller;
class GBTStateTable;

// <summary>
// A GBTBackendFiller for Spectral Processor data
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=GBTBackendFiller>GBTBackendFIller</linkto>
//   <li> The GBT Spectral Processor FITS file format.
// </prerequisite>
//
// <etymology>
// This fills GBT Spectral Processor data to a MS, hence GBTSPFiller.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// There exist Spectral Processor data at the GBT which needs to be filled.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> Probably many things.
// </todo>

class GBTSPFiller : public GBTBackendFiller
{
public:
    // construct one
    GBTSPFiller(const String &msName, const Regex &object);
    
    ~GBTSPFiller();

    virtual GBTScanLogReader::BACKENDS type() {return GBTScanLogReader::SP;}

    // fill a given backend file and DAP files
    virtual Bool fill(const String &backendFile,
		      const Block<String> &dapFiles,
		      const String &ifManagerFile,
		      const Block<String> &rxCalInfoFiles,
		      const GBTGOFile &goFile,
		      const String &antennaFile,
		      const GBTStateTable &masterState,
		      const GBTLO1File &LO1A,
		      const GBTLO1File &LO1B,
		      const GBTMSFillOptions &fillOptions);

private:
    GBTBackendTable *spTab_p;
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

    void handleColumns();

    // extract the start time at the current row of the data table
    MVTime rowTime(const RORecordFieldPtr<Int> &utdateField,
		   const RORecordFieldPtr<Double> &utcstartField,
		   Double tdur);

    // unset and unavailable
    GBTSPFiller();
    GBTSPFiller(const GBTSPFiller &);
    GBTSPFiller &operator=(const GBTSPFiller &);
};

#endif

