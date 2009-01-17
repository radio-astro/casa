//# GBTBackendTable.h:  GBT backend tables with AIPS++ Look and Feel.
//# Copyright (C) 1998,1999,2001,2002,2003
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

#ifndef NRAO_GBTBACKENDTABLE_H
#define NRAO_GBTBACKENDTABLE_H

#include <casa/aips.h>
#include <nrao/FITS/GBTStateTable.h>
#include <nrao/FITS/GBTScanLogReader.h>
#include <fits/FITS/FITSTable.h>
#include <casa/Arrays/Vector.h>
#include <fits/FITS/CopyRecord.h>

#include <casa/namespace.h>
// forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
class Table;
} //# NAMESPACE CASA - END


// <summary>
// Attach a FITSTabular to a GBT backend FITS file
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// GBTBackendTable is a FITSTabular which is attached to a GBT FITS 
// backend file.   GBT FITS backend files contain a DATA table and 
// several anciliary tables that correspond to the axes of the data 
// column in the DATA table.  
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="1995/06/01">
//   <li>
// </todo>
class GBTBackendTable : public FITSTabular
{
public:
    // Default constructor, not attached to any specific file
    GBTBackendTable();
    
    // Construct it from a file name
    GBTBackendTable(const String &fileName);
    ~GBTBackendTable();

    // Attach this GBTBackendTable to a new file name.
    // when resync is True, if fileName is the same as the
    // currently opened file, if there is one, then this just
    // closes and reopens the FITS file setting things right
    // back to the next row in the table, if there is one
    virtual Bool reopen(const String &fileName, Bool resync);

    // the reopen from FITSTabular, uses reopen here with resync=True
    virtual Bool reopen(const String &fileName) {return reopen(fileName, True);}
    
    // Return the type of this backend
    GBTScanLogReader::BACKENDS type() const {return itsType;}

    // return the name of the underlying table used at construction
    virtual const String &name() const {return dataTable_p->name();}

    // is this a valid GBTBackendTable?
    virtual Bool isValid() const {return isValid_p;}

    // The number of elements along the STATE axis
    virtual uInt nstate() const {return itsNstate;}

    // The number of elements along the frequency axis (defaults to 1
    // for non-spectral line data) for a specific sampler.
    // For most backends, this is a constant.  For the ACS
    // backend, if there are more than one banks involved, it
    // may vary with sampler.  The default implementation is
    // to return a constant.
    virtual uInt nchan(uInt whichSampler) const {return itsNchan;}

    // The number of elements along the sampler axis.
    virtual uInt nsamp() const {return itsNsamp;}

    // The bandwidth for the indicated sampler.  If this is < 0 then that
    // signals that it is not known here and it should be fetched from
    // the IF BANDWDTH column.
    virtual Double bw(uInt whichSampler) const {return itsBW[whichSampler];}

    // The center IF for the indicated sampler.  If this is < 0 then that
    // signals that it is not known here and it should be fetched from
    // the IF CENTER_IF column.
    virtual Double centerIF(uInt whichSampler) const {return itsCenterIF[whichSampler];}

    // Indicates whether frequency increases with channel number for the
    // given sampler.
    virtual Bool increases(uInt whichSampler) const {return itsIncreases[whichSampler];}
 
    // these keywords associated with the FITS data table.
    virtual const TableRecord &keywords() const {return keywords_p;}

    virtual const RecordDesc &description() const {return dataTable_p->description();}
    virtual const Record &units() const {return dataTable_p->units();}
    virtual const Record &displayFormats() const {return dataTable_p->displayFormats();}
    virtual const Record &nulls() const {return dataTable_p->nulls();}

    // is this now past the end of the FITS table?
    virtual Bool pastEnd() const {return dataTable_p->pastEnd();}

    // advance to the next row
    virtual void next() {
	dataTable_p->next();
	if (fieldCopier_p) fieldCopier_p->copy();
    }
    // return the current row
    virtual const Record &currentRow() const {return dataTable_p->currentRow();}

    // the unhandled keywords
    virtual const Record &unhandledKeywords() 
	{ 
	    if (!unhandledKeys_p) initUnhandledKeys();
	    return *unhandledKeys_p;
	}
    // mark a keyword as handled
    virtual void handleKeyword(const String &kwname);

    // the unhandled fields in currentRow
    virtual const Record &unhandledFields()
	{ 
	    if (!unhandledFields_p) initUnhandledFields();
	    return *unhandledFields_p;
	}
    // mark a field in currentRow as being handled
    virtual void handleField(const String &fieldname);

    // the ancilliary tables - empty tables when not present
    virtual const Table &sampler() const {return *sampler_p;}
    virtual const GBTStateTable &state() const {return state_p;}
    virtual const Table &actState() const {return *actState_p;}
    virtual const Table &port() const {return *port_p;}

    virtual uInt nrow() const {return dataTable_p->nrow();}
    virtual uInt rownr() const {return dataTable_p->rownr();}

    // FITSVER - FITS definition version for this device.
    // Default value: "0.0"
    virtual const String &fitsVers() const {return itsFitsVer;}

    // Leading integer (before decimal) in FITSVER - version of base class.
    virtual Int baseVersion() const {return itsBaseVer;}

    // Trailing integer (after decimal) in FITS - version of device.
    virtual Int deviceVersion() const {return itsDeviceVer;}

private:
    GBTScanLogReader::BACKENDS itsType;

    FITSTable *dataTable_p;
    Table *sampler_p, *actState_p, *port_p;
    GBTStateTable state_p;

    TableRecord keywords_p;
    Record *unhandledKeys_p, *unhandledFields_p;

    CopyRecordToRecord *fieldCopier_p;

    Vector<Int> keyMap_p, fieldMap_p;
    Vector<Double> itsBW, itsCenterIF;
    Vector<Bool> itsIncreases;

    Bool isValid_p;

    uInt itsNstate, itsNchan, itsNsamp;

    String itsFitsVer;
    Int itsBaseVer, itsDeviceVer;

    void clear_self(Bool resync=False);

    // construct a Table from a FITSTable, it is the responsiblity
    // of the calling entity to delete the returned pointer
    // the constructed table is a Scratch table using the
    // AipsIO storage manager
    Table *tableFromFITS(FITSTable &fits);

    void initUnhandledKeys();
    void initUnhandledFields();

    // Undefined and inaccessible.
    GBTBackendTable(const GBTBackendTable &);
    GBTBackendTable &operator=(const GBTBackendTable &);
};


#endif
