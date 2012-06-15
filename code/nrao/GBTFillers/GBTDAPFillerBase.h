//# GBTDAPFillerBase: A base class for GBT Data Associated Parameter fillers
//# Copyright (C) 2000
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

#ifndef NRAO_GBTDAPFILLERBASE_H
#define NRAO_GBTDAPFILLERBASE_H

#include <nrao/FITS/GBTDAPFile.h>

#include <casa/aips.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/Quanta/MVTime.h>
#include <tables/Tables/TableDesc.h>
#include <casa/BasicSL/String.h>

#include <fits/FITS/FITSTable.h>

#include <casa/namespace.h>

// <summary>
// A base class for GBT Data Associated Parameter fillers
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> GBT DAP FITS files
// </prerequisite>
//
// <etymology>
// This provides the underlying mechanisms for filling GBT DAP FITS
// files from the same device but many different samplers and managers.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Various DAP fillers share their handling of the FITS files.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//

class GBTDAPFillerBase
{
public:
    // A virtual destructor is needed so that it will use the
    // actual destructor in the derived class.
    virtual ~GBTDAPFillerBase();

    // report the device name
    virtual const String &device() const = 0;

    // this should do what it can to fill the data
    virtual void fill() = 0;

    // prepare the filler to use the indicated manager, sampler, and filename
    // starting with the given startTime.  The return value is the ID of
    // this sampler/manager combo.  A negative return value indicates a problem.
    virtual Int prepare(const String &fileName, const String &manager,
			const String &sampler, const MVTime &startTime);

    // A useful function to decode a filename into a device, manager, and sampler
    static Bool parseName(const String &fileName, 
			  String &device, String &manager,
			  String &sampler);
protected:
    // (Required) default constructor
    GBTDAPFillerBase();

    // (Required) copy constructor.
    GBTDAPFillerBase(const GBTDAPFillerBase &other);

    // (Required) copy assignment.
    GBTDAPFillerBase &operator=(const GBTDAPFillerBase &other);

    // how many rows are ready to be filled
    Int rowsToFill() {return rowsToFill_p;}

    // Get the ID of the DAP holding the current row waiting to be filled
    Int currentId();

    // return the current row waiting to be filled
    const Record &currentRow();

    // return the current sampler
    const String &currentSampler();

    // return the current manager
    const String &currentManager();

    // return the current interval
    Double currentInterval();

    // return the current time
    const MVTime &currentTime() {return ctime_p;}

    // advance to the next row to be filled - this decrements rowsToFill by 1
    // when rowsToFill reaches zero, next() has no effect
    void next();

    // get the TableDesc of the indicated sampler/manager ID - the value returned
    // by prepare
    TableDesc tableDesc(Int whichId) {return FITSTabular::tableDesc(*dapblock_p[whichId]);}

    // get the currentRow of the indicated sampler/manager ID - the value returned
    // by prepare
    const Record &dapRecord(Int whichId) {return dapblock_p[whichId]->unhandledFields();}

private:
    // how man rows are ready to be filled
    Int rowsToFill_p;

    // this maps the manager+sampler string to an integer
    SimpleOrderedMap<String, Int> dapmap_p;

    // this is where the DAPs for each manager+sampler string are held
    PtrBlock<GBTDAPFile *> dapblock_p;

    // This indicates that a particular DAP file is ready to fill
    // This is set to True on a successfull prepare and set to False when filled
    Block<Int> readyToFill_p;

    // how many active DAPs do we have
    uInt ndap_p;

    // which one is the next row waiting to be filled
    Int currentPtr_p;
    MVTime ctime_p;

    // and empty record, to be returned as needed
    Record emptyRecord_p;
    String emptyString_p;

    // find the minimum currentTime from the known DAP files
    MVTime minTime() const;
};


#endif


