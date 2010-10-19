//# GBTBackendFiller.h: this is a base class for GBT backend fillers
//# Copyright (C) 1999,2000,2001,2002,2003
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

#ifndef NRAO_GBTBACKENDFILLER_H
#define NRAO_GBTBACKENDFILLER_H

//#! Includes go here

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <nrao/FITS/GBTStateTable.h>
#include <nrao/FITS/GBTScanLogReader.h>
#include <nrao/FITS/GBTGOFile.h>
#include <casa/Quanta/MVTime.h>
#include <nrao/GBTFillers/GBTFeedDDFiller.h>
#include <nrao/GBTFillers/GBTLO1DAPFiller.h>
#include <nrao/GBTFillers/GBTMSAntennaFiller.h>
#include <nrao/GBTFillers/GBTMSDataDescFiller.h>
#include <nrao/GBTFillers/GBTMSFeedFiller.h>
#include <nrao/GBTFillers/GBTMSFieldFiller.h>
#include <nrao/GBTFillers/GBTMSHistoryFiller.h>
#include <nrao/GBTFillers/GBTMSObservationFiller.h>
#include <nrao/GBTFillers/GBTMSPointingFiller.h>
#include <nrao/GBTFillers/GBTMSPolarizationFiller.h>
#include <nrao/GBTFillers/GBTMSProcessorFiller.h>
#include <nrao/GBTFillers/GBTMSSourceFiller.h>
#include <nrao/GBTFillers/GBTMSSpecWinFiller.h>
#include <nrao/GBTFillers/GBTMSStateFiller.h>
#include <nrao/GBTFillers/GBTMSSysCalFiller.h>
#include <nrao/GBTFillers/GBTMSWeatherFiller.h>
#include <nrao/GBTFillers/GBTDAPFiller.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/TableLock.h>
#include <casa/BasicSL/String.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MeasurementSet;
class String;
class TableDesc;
class DataManager;
class MSMainColumns;

template<class T> class Block;
} //# NAMESPACE CASA - END

class GBTLO1File;
class GBTMSFillOptions;

#include <casa/namespace.h>

// <summary>
// A base class for GBT backend fillers.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> Understand the GBT FITS files to be filled.
//   <li> <linkto class=MeasurementSet>MeasurementSet</linkto>
//   <li> <linkto class=GBTScanLogReader>GBTScanLogReader</linkto>
//   <li> The several GBTMS*Filler classes for filling the subtables.
// </prerequisite>
//
// <etymology>
// The GBT MeasurementSet filler is driven by the contents of the
// backend data to be filled.  This class is the base class for
// these backend fillers for GBT data, hence GBTBackendFiller.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// It should be possible to write a new filler by deriving from this
// class and plug the new class into the filler DO and have the
// DO understand that new backend.  It is also required that
// the GBTScanLogReader be updated to recognize this new backend.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li> Probably many things remain to be done.
// </todo>

class GBTBackendFiller
{
public:
    // construct an empty filler, openMS or createMS must be
    // called before this is a useful filler.
    GBTBackendFiller();

    virtual ~GBTBackendFiller();

    // static function giving the default TableLock to be used to
    // open all MS tables
    static TableLock tableLock() 
    {return TableLock(TableLock::PermanentLockingWait);}

    // what type of backend is this
    virtual GBTScanLogReader::BACKENDS type() = 0;

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
		      const GBTMSFillOptions &fillOptions) = 0;

    // prepare the appropriate DAP filler based on the indicated file name
    virtual Bool prepareDAPfillers(const String &dapfile,
				   const MVTime &startTime);
    // fill all of the prepared DAP files
    virtual void fillDAP();

    // get the MS this filler is associated with
    virtual MeasurementSet &ms() { return *ms_p;}

    // open the MS, this returns False if an MS of that name
    // does not exist or can not be opened.
    // The holography filler has useFeedDDFiller = False here.
    virtual Bool openMS(const String &msName, Bool useFeedDDFiller = True);

    // create and open the MS, using the given TableDesc
    // for the Main table of the MS.  Also supply a data manager
    // an the main table column names it should be bound to.
    // When spectralLine is true the following optional columns
    // are added to the indicated tables:
    // <li> SYSCAL: TCAL_SPECTRUM, TRX, and TRX_FLAG
    // <li> SOURCE: REST_FREQUENCY
    virtual Bool createMS(const String &msName, 
			  const TableDesc &mainTD,
			  const DataManager &dm, 
			  const Vector<String> &dmColNames,
			  const IPosition &defaultTileShape,
			  const GBTMSFillOptions &fillOptions,
			  Int nrows = 0,
			  Bool spectralLine = False,
			  Bool useFeedDDFiller = True);

    // Supply a second data manager and its columns (this
    // is used when a LAGS column is required).
    // Returns false if the MS can not be created.
    // Always uses the GBTFeedDDFiller.
    // Only the holography filler has useFeedDDFiller = False.
    virtual Bool createMS(const String &msName, 
			  const TableDesc &mainTD,
			  const DataManager &dm, 
			  const Vector<String> &dmColNames,
			  const IPosition &defaultTileShape,
			  const GBTMSFillOptions &fillOptions,
			  const DataManager &dm2,
			  const Vector<String> &dm2ColNames,
			  Int nrows = 0,
			  Bool spectralLine = False,
			  Bool useFeedDDFiller = True);

    // create and open the MS, no special data manager columns
    // Only the holography filler has useFeedDDFiller = False.
    virtual Bool createMS(const String &msName, const TableDesc &mainTD,
			  const GBTMSFillOptions &fillOptions,
			  Int nrows = 0,
			  Bool useFeedDDFiller = True);

    virtual Bool hasMS() { return hasMS_p;}

    // Flush the MS and all subtables known here.  
    virtual void flush();

    // re-attach to the same MS - forces MSMainColumns to be redone
    virtual void reattach();

    // return the main table columns
    virtual MSMainColumns &cols() { return *mscols_p;}

    // The several sub-table filler.  feedFiller, dataDescFiller,
    // polarizationFiller and specWinFiller are all not attached
    // in favor of feedDDFiller unless useFeedDDFiller is False
    // in openMS or createMS.
    // <group>
    virtual GBTFeedDDFiller &feedDDFiller() { return feedDDFiller_p;}
    virtual GBTMSAntennaFiller &antennaFiller() { return antennaFiller_p;}
    virtual GBTMSDataDescFiller &dataDescFiller() { return dataDescFiller_p;}
    virtual GBTMSFeedFiller &feedFiller() { return feedFiller_p;}
    virtual GBTMSFieldFiller &fieldFiller() { return fieldFiller_p;}
    virtual GBTMSHistoryFiller &historyFiller() { return historyFiller_p;}
    virtual GBTMSObservationFiller &observationFiller() { return observationFiller_p;}
    virtual GBTMSPointingFiller &pointingFiller() { return pointingFiller_p;}
    virtual GBTMSPolarizationFiller &polarizationFiller() { return polarizationFiller_p;}
    virtual GBTMSProcessorFiller &processorFiller() { return processorFiller_p;}
    virtual GBTMSSourceFiller &sourceFiller() { return sourceFiller_p;}
    virtual GBTMSSpecWinFiller &specWinFiller() { return specWinFiller_p;}
    virtual GBTMSStateFiller &stateFiller() { return stateFiller_p;}
    virtual GBTMSSysCalFiller &sysCalFiller() { return sysCalFiller_p;}
    virtual GBTMSWeatherFiller &weatherFiller() { return weatherFiller_p;}
    // </group>

    // return the last time in the MS when it was opened
    // all filling must happen for times on or after this time
    virtual MVTime &startTime() { return startTime_p;}

    // return a reference to the LO1 filler, by pointer for now.  This pointer
    // should not be deleted.  It will be a null pointer if there is no LO1
    // manager associated with this filler yet.
    GBTLO1DAPFiller *lo1Filler() { return (lo1DAPFiller_p >= 0) ? 
				       (dynamic_cast<GBTLO1DAPFiller *>(dapFillers_p[lo1DAPFiller_p])) : 0;}

    // The calibration columns
    // <group>
    ArrayColumn<Complex> &modelData() {return modelData_p;}
    ArrayColumn<Complex> &correctedData() {return correctedData_p;}
    // </group>
private:
    // The MeasurementSet
    MeasurementSet *ms_p;
    Bool hasMS_p;

    // the main table columns
    MSMainColumns *mscols_p;

    // The various subtable fillers
    GBTFeedDDFiller feedDDFiller_p;
    GBTMSAntennaFiller antennaFiller_p;
    GBTMSDataDescFiller dataDescFiller_p;
    GBTMSFeedFiller feedFiller_p;
    GBTMSFieldFiller fieldFiller_p;
    GBTMSHistoryFiller historyFiller_p;
    GBTMSObservationFiller observationFiller_p;
    GBTMSPointingFiller pointingFiller_p;
    GBTMSPolarizationFiller polarizationFiller_p;
    GBTMSProcessorFiller processorFiller_p;
    GBTMSSourceFiller sourceFiller_p;
    GBTMSSpecWinFiller specWinFiller_p;
    GBTMSStateFiller stateFiller_p;
    GBTMSSysCalFiller sysCalFiller_p;
    GBTMSWeatherFiller weatherFiller_p;

    // calibration columns
    ArrayColumn<Complex> modelData_p, correctedData_p;

    // map device name string to the appropriate GBTDAPFiller
    SimpleOrderedMap<String, Int> deviceMap_p;
    // the block containing the pointers to the various GBTDAPFillers
    Block<GBTDAPFiller *> dapFillers_p;
    // this indicates which ones are ready to fill (the flag is
    // set to true in prepareDAPfillers and set to False in fillDAP
    Block<Bool> readyToFill_p;
    // this points to the antenna DAP filler when one exists
    Int antennaDAPFiller_p;
    // this points to the LO1 DAP filler when one exists
    Int lo1DAPFiller_p;

    MVTime startTime_p;

    // initialize the sub-table fillers once the MS has been
    // re-opened or constructed
    void initSubTableFillers(Bool useFeedDDFiller);

    // add calibration columns
    void addCalSet(const IPosition &defaultTileShape,
		   Bool compress);

    // initialize calibration columns by setting their
    // shape and their initial default values.  This is
    // only used when data already has been filled.
    void initCalSet();

    // attach the calibration columns
    void attachCalSet();

    // undefined and unavailable
    GBTBackendFiller(const GBTBackendFiller &other);
    GBTBackendFiller &operator=(const GBTBackendFiller &other);

};


#endif

