//# GBTAntennaDAPFiller: A GBTDAPFiller for antenna DAPs
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

#ifndef NRAO_GBTANTENNADAPFILLER_H
#define NRAO_GBTANTENNADAPFILLER_H

#include <nrao/GBTFillers/GBTDAPFiller.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableColumn.h>

// <summary>
//  A GBTDAPFiller for antenna DAPs.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> GBT DAP FITS files
//   <li> GBTAntennaDAPFillerBase.h
// </prerequisite>
//
// <etymology>
// This fills GBT DAP FITS file from the Antenna device into a subtable of 
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
// It is necessary to have access to the sky position information during each scan
// for use in filling other parts of the MS.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//

#include <casa/namespace.h>

class GBTAntennaDAPFiller : public GBTDAPFiller
{
public:
    // makes new one if not already present in parent, otherwise
    // it opens that one up as is
    GBTAntennaDAPFiller(const String &device, Table &parent);

    virtual ~GBTAntennaDAPFiller();

    virtual Int prepare(const String &fileName, const String &manager,
			const String &sampler, const MVTime &startTime);

    // fill to the standard table attached to the parent used at construction
    virtual void fill();

    // Return the sky position information.  These columns are NOT attached
    // until the DAP has been filled.  They will also not be attached if the
    // skyPosition sampler is not know to the filler.  Only the most recently
    // filled skyPosition information will be available.
    // <group>
    virtual const ROTableColumn &j2000RA() {return j2000ra_p;}
    virtual const ROTableColumn &j2000DEC() {return j2000dec_p;}
    virtual const ROTableColumn &time() {return time_p;}
    // </group>
private:
    Table *skyTable_p;
    ROTableColumn j2000ra_p, j2000dec_p, time_p;

    Bool hasSkyPosition_p;

    void cleanup();

    // unavailable, undefined
    GBTAntennaDAPFiller();
    GBTAntennaDAPFiller(const GBTAntennaDAPFiller &);
    GBTAntennaDAPFiller &operator=(const GBTAntennaDAPFiller &);
};


#endif


