//# GBTMSDataDescFiller.h: GBTMSDataDescFiller fills the MSDataDescription table for GBT fillers
//# Copyright (C) 2000,2001
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

#ifndef NRAO_GBTMSDATADESCFILLER_H
#define NRAO_GBTMSDATADESCFILLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/RecordField.h>
#include <tables/Tables/TableColumn.h>
#include <ms/MeasurementSets/MSDataDescription.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSDataDescColumns;
class Table;
class ColumnsIndex;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSDataDescFiller fills the MSDataDescription table for GBT fillers
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
// This class puts GBT data (fills) into the MSDataDescription table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSDataDescription
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSDataDescription are made.
// This filler keeps a certain number of the most recent data descriptionn IDs and
// the information that makes them unique in a cache.  If a new set of
// information matches that cached information, that ID is reused, otherwise
// new rows are added to the MSDataDescription table.
//
// It will often be the case that several spectral window ids are filled at
// the same time, hence it is necessary to fill several data descriptions at
// the same time.  During a fill, these are associated with a specific receiver id.
//
// The data description Ids are available to be used by 
// other GBTMS*Fillers as other subtables are filled in turn.
//
// Until the GBT polarization and spectral window information is fully available,
// the data description also includes a NS_GBT_RECEIVER_ID column.  This column
// is a key into the GBT_type_* tables.
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

class GBTMSDataDescFiller
{
public:

    // The default ctor.  No MSDataDescriotion table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSDataDescription has been created.
    GBTMSDataDescFiller();

    // A filler attached to an MSDataDescription table
    GBTMSDataDescFiller(MSDataDescription &msDataDesc);

    ~GBTMSDataDescFiller();

    // attach it to an MSDataDescription
    void attach(MSDataDescription &msDataDesc);

    // fill using the given polarization ID and spectral window ids
    void fill(Int polarizationId, const Vector<Int> &specWinIds, 
	      const Vector<Int> &receiverIds = Vector<Int>());

    Int fill(Int polarizationId, Int specWinId);

    // What is the data description ID associated with the indicated receiver?
    // Returns -1 if there is no association yet
    Int dataDescriptionId(uInt whichReceiver) const;

    // Return a vector of the most recently filled data description ids
    const Vector<Int> &dataDescriptionIds() const {return dataDescIds_p;}

    // The number of receivers which currently have known data description ids
    // from the last fill().
    Int nrec() const {return receiverMap_p.nelements();}

    // set the cache size - max  number of data description ids and their
    // associate information to keep, this always
    // re-initializes the cache from the DATA_DESCRIPTION table making it 
    // potentially expensive to use.
    void setCacheSize(uInt newCacheSize);

    // get the cache size
    uInt getCacheSize() const {return cacheSize_p;}

    // flush the underlying MS subtable
    void flush() {msDataDesc_p->flush();}

private:
    MSDataDescription *msDataDesc_p;
    MSDataDescColumns *msDataDescCols_p;

    // The current Ids
    Vector<Int> dataDescIds_p;

    // The map from receiver number to data description ID for the
    // current set of Ids
    // dataDescIds_p(receiverMap_p[whichReceiver]);
    Block<Int> receiverMap_p;

    // the table holding the cache, its temporary
    Table *theCache_p;

    // the columns in the cache
    TableColumn idCacheCol_p, spWinIdCacheCol_p, polIdCacheCol_p, receiverIdCacheCol_p;

    // The NS_GBT_RECEIVER_ID column in the actual table
    TableColumn receiverIdCol_p;

    // The ColumnsIndex for the cache
    ColumnsIndex *cacheIndx_p;

    // The pointers to the fields in the index key
    RecordFieldPtr<Int> spWinIdKey_p, polIdKey_p, receiverIdKey_p;

    // The next row number to get trashed in the cache when a new ID is generated
    uInt nextCacheRow_p;

    // The maximum number of rows in the cache (cache size)
    uInt cacheSize_p;

    // initialize the above for the first time
    void init(MSDataDescription &msDataDesc);

    // check and optionally fill a row from the index returns the Id
    Int checkAndFill(Int polarizationId, Int specWinId, Int receiverId);
    

    // undefined and unavailable
    GBTMSDataDescFiller(const GBTMSDataDescFiller &other);
    void operator=(const GBTMSDataDescFiller &other);
};

#endif


