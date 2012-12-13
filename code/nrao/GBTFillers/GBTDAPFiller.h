//# GBTDAPFiller: A generic filler for GBT Data Associated Parameter FITS files.
//# Copyright (C) 1999,2000,2001
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

#ifndef NRAO_GBTDAPFILLER_H
#define NRAO_GBTDAPFILLER_H

#include <nrao/GBTFillers/GBTDAPFillerBase.h>

#include <casa/Containers/Block.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableColumn.h>
#include <casa/BasicSL/String.h>


//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class TableRow;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// A generic filler for GBT Data Associated Parameter FITS files.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> GBT DAP FITS files
//   <li> GBTDAPFillerBase.h
// </prerequisite>
//
// <etymology>
// This fills GBT DAP FITS file contents into a table which is part of
// an AIPS++ MeasurementSet.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// It is necessary to fill GBT DAP files to a MS.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//

class GBTDAPFiller : public GBTDAPFillerBase
{
public:
    // makes new one if not already present in parent, otherwise
    // it opens that one up as is
    GBTDAPFiller(const String &device, Table &parent);

    virtual ~GBTDAPFiller();

    // the device name used at construction
    const String &device() const {return device_p;}

    virtual Int prepare(const String &fileName, const String &manager,
			const String &sampler, const MVTime &startTime);


    // fill to the standard table attached to the parent used at construction
    virtual void fill();

    // flush this to disk
    void flush() {tab_p->flush();}

protected:
    // derived classes need read-only access to the underlying table
    Table &tab() { return *tab_p;}

private:
    String device_p;

    Table *tab_p;

    // samper, manager and time columns
    TableColumn samplerCol_p, managerCol_p, timeCol_p, intervalCol_p;

    // The is where the TableRow appropriate for each DAP is stored
    PtrBlock<TableRow *> daprow_p;
    // and how many of them are really in use at any given point
    Int ndap_p;

    // attach the table columns
    void attachColumns();

    // unavailable, undefined
    GBTDAPFiller();
    GBTDAPFiller(const GBTDAPFiller &);
    GBTDAPFiller &operator=(const GBTDAPFiller &);
};


#endif


