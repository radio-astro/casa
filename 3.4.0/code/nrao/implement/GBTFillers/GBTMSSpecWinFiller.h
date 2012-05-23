//# GBTMSSpecWinFiller.h: GBTMSSpecWinFiller fills the MSSpectralWindow table for GBT fillers
//# Copyright (C) 2000,2001,2002,2003
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

#ifndef NRAO_GBTMSSPECWINFILLER_H
#define NRAO_GBTMSSPECWINFILLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/RecordField.h>
#include <tables/Tables/TableColumn.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <measures/Measures/MFrequency.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSSpWindowColumns;
class Table;
class ColumnsIndex;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSSpecWinFiller fills the MSSpectralWindow table for GBT fillers
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSets
//   <li> GBT FITS files
// </prerequisite>
//
// <etymology>
// This class puts GBT data (fills) into the MSSpectralWindow table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSSpectralWindow
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSSpectralWindow are made.
// This filler keeps a certain number of the most spectral window IDs and
// the information that makes them unique in a cache.  If a new set of
// information matches that cached information, that ID is reused, otherwise
// new rows are added to the MSSpectralWindow table.
//
// In principle, the number of spectral window Id may vary with the
// the number of receivers in the backend (at least, thats the idea so far).
// 
// The specWinId is available to be used by 
// other GBTMS*Fillers as other subtables are filled in turn.
//
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Writing a filler is a daunting task.  There are various dependencies
// between the subtables.  It helps to encapsulate the knowlege and
// task of filling a specific subtable to separate classes.
// </motivation>
//
//#! <thrown>
//#! A list of exceptions thrown if errors are discovered in the function.
//#! This tag will appear in the body of the header file, preceding the
//#! declaration of each function which throws an exception.
//#! </thrown>

class GBTMSSpecWinFiller
{
public:

    // The default ctor.  No MSSpectralWindow table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSSpecWin has been created.
    GBTMSSpecWinFiller();

    // A filler attached to an MSSpectralWindow table
    GBTMSSpecWinFiller(MSSpectralWindow &msSpecWin);

    ~GBTMSSpecWinFiller();

    // attach it to an MSSpectralWindow
    void attach(MSSpectralWindow &msSpecWin);

    // fill using the indicated frequency axis description
    // returns the SPECTRAL_WINDOW_ID associated with this call to fill.
    // The tolerance is in Hz.  If this SW is within tolerance of an
    // already existing SW and the chanWidths are the same, that
    // existing SW will be re-used.  SPWs in the exclude vector will
    // not be used (that is used elsewhere to guarantee unique
    // spws even when they would otherwise match an existing spw).
    Int fill(uInt nfreq, Double centerFreq, Double centerChan, Double chanWidth,
	     MFrequency::Types restFrame = MFrequency::TOPO,
	     Double tolerance=10.0, const Vector<Int> &exclude = Vector<Int>());

    // fill a dummy (no spectral information actually present) row 
    // this is necessary until the IF manager is sufficient to supply 
    // frequency information for all of the backends.
    void fill(uInt nreceivers, uInt nfreq);

    // What is the spectral window ID associated with the indicated receiver
    // Returns -1 if there is no association yet
    Int spectralWindowId(uInt whichReceiver) const;

    // Return a vector of the spectral window Ids for each receiver
    const Vector<Int> &spectralWindowIds() const {return specWinIds_p;}

    // The number of receivers which currently have known spectral window Ids
    // from the last fill().
    Int nrec() const {return specWinIds_p.nelements();}

    Int nfreq() const {return nfreq_p;}

    // set the cache size - max  number of spectral window IDs and their
    // associate information to keep, this always
    // re-initializes the cache from the SPECTRAL_WINDOW table making it 
    // expensive to use.
    void setCacheSize(uInt newCacheSize);

    // get the cache size
    uInt getCacheSize() const {return cacheSize_p;}

    // flush the underlying MS subtable
    void flush() {msSpecWin_p->flush();}

private:
    MSSpectralWindow *msSpecWin_p;
    MSSpWindowColumns *msSpecWinCols_p;

    // The current Ids
    Vector<Int> specWinIds_p;

    // from the last fill
    Int nfreq_p;

    // the table holding the cache, its temporary
    Table *theCache_p;

    // the columns in the cache
    TableColumn idCacheCol_p, nfreqCacheCol_p, obsfreqCacheCol_p,
	freqresCacheCol_p, refchanCacheCol_p, bwCacheCol_p,
	restFrameCacheCol_p;

    // The ColumnsIndex for the cache
    ColumnsIndex *cacheIndx_p;

    // The pointers to the fields in the index key
    RecordFieldPtr<Int> nfreqKey_p, restFrameKey_p;
    RecordFieldPtr<Double> refchanKey_p;

    // The next row number to get trashed in the cache when a new ID is 
    // generated
    uInt nextCacheRow_p;

    // The maximum number of rows in the cache (cache size)
    uInt cacheSize_p;

    // initialize the above for the first time
    void init(MSSpectralWindow &msSpecWin);

    // add a new row to the spectral window table using these value
    // return the new row number
    Int newSpecWin(Int nfreq, Double obsfreq, Double freqres, Double refchan, 
		   Double bw, Int restframe);

    // undefined and unavailable
    GBTMSSpecWinFiller(const GBTMSSpecWinFiller &other);
    void operator=(const GBTMSSpecWinFiller &other);
};

#endif


