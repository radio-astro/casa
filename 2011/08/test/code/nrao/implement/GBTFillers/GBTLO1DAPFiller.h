//# GBTLO1DAPFiller: A GBTDAPFiller for LO1 DAPs
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

#ifndef NRAO_GBTLO1DAPFILLER_H
#define NRAO_GBTLO1DAPFILLER_H

#include <nrao/GBTFillers/GBTDAPFiller.h>

#include <casa/Arrays/Vector.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>

//# Forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
//  A GBTDAPFiller for LO1 DAPs.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> GBT DAP FITS files
//   <li> GBTLO1DAPFillerBase.h
// </prerequisite>
//
// <etymology>
// This fills GBT DAP FITS file from the LO1 device into a subtable of 
// the MS.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// It is necessary to have access to the frequency information during each scan
// for use in filling other parts of the MS.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//

class GBTLO1DAPFiller : public GBTDAPFiller
{
public:
    // makes new one if not already present in parent, otherwise
    // it opens that one up as is
    GBTLO1DAPFiller(const String &device, Table &parent);

    virtual ~GBTLO1DAPFiller();

    virtual Int prepare(const String &fileName, const String &manager,
			const String &sampler, const MVTime &startTime);

    // fill to the standard table attached to the parent used at construction
    virtual void fill();

    // Return the frequency for the given the phase numbers, given the time and interval
    // Time is in MJD seconds UT and interval is in seconds.
    // These values are based on the results of the most recent fill.
    // A value of 0.0 is returned for all phases if there are no rows within the
    // specified time interval
    const Vector<Double> &frequency(Double time, Double interval, Int nphases);

    // return the value of the frequency column in the first row
    Double initialFreq() { return ((nrow()>0) ? itsFrequencyColumn.asdouble(0) : 0.0); }

    // how many rows are in the underlying table
    uInt nrow() {return ( itsLO1Table ? itsLO1Table->nrow() : 0);}

private:
    Table *itsLO1Table;
    ROScalarColumn<Double> itsFrequencyColumn, itsTimeColumn;
    ROScalarColumn<Int> itsPhaseColumn;

    Int itsCachePtr, itsCacheStart, itsCacheEnd, itsCacheSize;
    Bool itsHasPhaseState;

    Vector<Double> itsFreqs;

    Vector<Double> itsTimeCache, itsFreqCache;
    Vector<Int> itsPhaseCache;

    void cleanup();
    void nextCache();
    void prevCache();

    Bool atCacheEnd() {return itsFrequencyColumn.isNull() ||
			   itsCacheEnd >= (Int(itsFrequencyColumn.nrow())-1);}
    Bool atCacheStart() {return itsFrequencyColumn.isNull() || itsCacheStart <= 0;}
    Bool atEnd() {return itsCachePtr >= (Int(itsFreqCache.nelements())-1) && atCacheEnd();}
    Bool atStart() {return itsCachePtr <= 0 && atCacheStart();}

    // unavailable, undefined
    GBTLO1DAPFiller();
    GBTLO1DAPFiller(const GBTLO1DAPFiller &);
    GBTLO1DAPFiller &operator=(const GBTLO1DAPFiller &);
};

#endif


