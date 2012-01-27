//# GBTIFFiller: A filler for the GBT IF Manager FITS files.
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

#ifndef NRAO_GBTIFFILLER_H
#define NRAO_GBTIFFILLER_H

#include <nrao/GBTFillers/GBTSimpleTable.h>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/SimOrdMap.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/String.h>

//# Forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class ColumnsIndex;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
//  A filler for the GBT IF Manager FITS files.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> Stuff
// </prerequisite>
//
// <etymology>
// This fills a subtable of a MS using the contents of the IF fits file.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// It is necessary to have access to the frequency and polarization information 
// during each scan for use in filling other parts of the MS.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//

class GBTIFFiller
{
public:
    // makes a new NRAO_GBT_IF subtable if one is not already present in 
    // parent, otherwise it opens that one up as is.  The backend string is 
    // used throughout to indicate which backend device this corresponds to.
    // Each backend filler will have its own GBTIFFiller object.  Only rows 
    // appropriate for that backend are saved to the subtable.
    GBTIFFiller(Table &parent, const String &backend);

    virtual ~GBTIFFiller();

    // are all of the expected columns present - any SEVERE error results in
    // this being set to False
    virtual Bool isValid() const {return itsValid;}

    // fill the subtable using the indicated IF FITS file.  The return value
    // is the value of the IF_ID in the subtable used when filling from this file.
    virtual Int fill(const String &fileName);

    // Indicate which row number in the most recently filled IF FITS file matches
    // this combination of bank and port.  A return value of -1 indicates
    // that no match was found.  A LogIO::SEVERE message is emitted if more than
    // one match is found.  This should not happen, but it also would be bad to
    // throw an exception in that case since the rest of the filling process should
    // proceed.
    Int whichRow(const String &bank, Int port) const;

    // Return the column information from the most recently filled table.
    // <group>
    virtual const ROScalarColumn<Int> &feed() const {return itsFeedCol;}
    virtual const ROScalarColumn<Int> &srfeed1() const {return itsSRFeed1Col;}
    virtual const ROScalarColumn<Int> &srfeed2() const {return itsSRFeed2Col;}
    virtual const ROScalarColumn<Int> &highCal() const {return itsHighCalCol;}
    virtual const ROScalarColumn<String> &receiver() const {return itsReceiverCol;}
    virtual const ROScalarColumn<String> &receptor() const {return itsReceptorCol;}
    virtual const ROScalarColumn<String> &loCircuit() const {return itsLOCircuit;}
    virtual const ROScalarColumn<String> &loComponent() const {return itsLOComponent;}
    virtual const ROScalarColumn<String> &sideband() const {return itsSidebandCol;}
    virtual const ROScalarColumn<String> &polarize() const {return itsPolarizeCol;}
    virtual const ROScalarColumn<Float> &centerIF() const {return itsCenterIfCol;}
    virtual const ROScalarColumn<Float> &centerSky() const {return itsCenterSkyCol;}
    virtual const ROScalarColumn<Float> &bandwidth() const {return itsBandwidthCol;}
    virtual const ROScalarColumn<Double> &sffMultiplier() const {return itsSffMultiplierCol;}
    virtual const ROScalarColumn<Double> &sffSideband() const {return itsSffSidebandCol;}
    virtual const ROScalarColumn<Double> &sffOffset() const {return itsSffOffsetCol;}
    virtual const ROScalarColumn<String> &bank() const {return itsBankCol;}
    // </group>

    // Get the feed ID and receptor ID for a given row number, returns False if
    // the row number is not valid.  The feedIds start from 0 for each receiver
    // in the table.  The receptorIds start from 0 for each feed.  Also returns
    // the number of possible receptors for that feed.  These feedIds are used
    // in coordinating the filling of the data rows and may not be the same
    // as the FEED_ID in the FEED table, which corresponds to physical feeds - 
    // if multiple IFs are used, each IF/physical FEED has a unique FEED_ID here.
    Bool feedIds(Int whichRow, Int &feedId, Int &receptorId, Int &nReceptors) const;

    // return a reference to the current table
    const Table &currentTable() const { return itsCurrTable;}

    // return the index number of the underying simple table
    Int index() { return (itsMSTable) ? itsMSTable->index() : -1;}

    // get the backend used at construction time
    const String &backend() const { return itsBackend;}

    // Associate an IF frequency (for use in the sky frequency formula)
    // a number of channels and the reference channel at which IF
    // is valid with a specific bank and port in this table.
    // Returns False if bank and port are not found.  deltaFreq gives
    // the frequency increment per channel with increasing channel
    // number.  The bandwidth here should not be used because it
    // may go to zero if the IF manager determines that that signal
    // path has zero bandwidth at any point.  Instead, rely on the
    // backend.  The only exception is that if deltaFreq is zero,
    // use the bandwidth()/nchan here. That should only be used for
    // the DCR.
    Bool associateIF(const String &bank, Int port, 
		     Double ifFreq, Double refChan, Double deltaFreq,
		     Int nchan);

    // Get the associated IF frequency, nchan, refChan, and delta
    // frequency for the given bank and port.  Returns False if not found.
    Bool getIF(const String &bank, Int port, 
	       Double &ifFreq, Double &refChan, Double &deltaFreq,
	       Int &nchan);

    // flush the underlying simple table
    void flush() { itsMSTable->flush();}
private:
    GBTSimpleTable *itsMSTable;
    Table itsCurrTable;

    mutable ColumnsIndex *itsIndex;

    mutable RecordFieldPtr<String> itsBankKey;
    mutable RecordFieldPtr<Int> itsPortKey;

    mutable Int itsLastRow;

    String itsBackend;
    Bool itsUseBank;

    Bool itsSingleReceptor;

    ROScalarColumn<String> itsReceiverCol, itsReceptorCol, itsLOCircuit,
	itsLOComponent, itsSidebandCol, itsPolarizeCol, itsBankCol;

    ROScalarColumn<Int> itsFeedCol, itsSRFeed1Col, itsSRFeed2Col, itsHighCalCol;

    ROScalarColumn<Float> itsCenterIfCol, itsCenterSkyCol, itsBandwidthCol;
    
    ROScalarColumn<Double> itsSffMultiplierCol, itsSffSidebandCol, itsSffOffsetCol;

    SimpleOrderedMap<String, Int> itsMaxFeedIdMap;
    SimpleOrderedMap<Int, Int> itsFeedIdMap;

    Block<Int> itsFeedIDs;
    Block<Int> itsReceptorIDs;
    Block<Int> itsNReceptors;

    Vector<Double> itsIFs, itsRefChans, itsDeltaFreqs;
    Vector<Int> itsNchans;

    Bool itsValid;

    void unknownReceiver(const String &rec, Int whichRow, Int &feedId);

    // digest table to get feedIds and receptor ids for each feed on each receiver
    void digestTable();

    // unavailable, undefined
    GBTIFFiller();
    GBTIFFiller(const GBTIFFiller &);
    GBTIFFiller &operator=(const GBTIFFiller &);
};

#endif


