//# GBTStateTable.h: Class for dealing with the STATE table in GBT FITS files
//# Copyright (C) 2002,2003
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

#ifndef NRAO_GBTSTATETABLE_H
#define NRAO_GBTSTATETABLE_H

//#! Includes go here

#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class Table;
class FITSTable;
} //# NAMESPACE CASA - END


// <summary>
// Class for dealing with the STATE table in GBT FITS files.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
//
// <thrown>
//#! A list of exceptions thrown if errors are discovered in the function.
//#! This tag will appear in the body of the header file, preceding the
//#! declaration of each function which throws an exception.
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

class GBTStateTable
{
public:
    // This constructs a GBTStateTable that is not attached to any table.
    // reattach must be used to make such an object useful.
    GBTStateTable();

    // Make a GBTStateTable attached to the indicated FITSTable
    GBTStateTable(FITSTable &fitstab);

    // uses copy semantics
    GBTStateTable(const GBTStateTable &other);

    ~GBTStateTable();

    // uses copy semantics
    GBTStateTable &operator=(const GBTStateTable &other);

    // reattach this object to a new FITSTable.  If there are problems with the
    // file, the return value is False and appropriate warnings are sent to the
    // logger.  The resulting object is not attached to any FITSTable.
    Bool reattach(FITSTable &fitstab);
    
    // detach from any table.
    void detach();

    // Is this object attached to a FITSTable.
    Bool isAttached() const {return itsAttached;}

    // the STATE table converted to a table
    const Table &table() const {return *itsTab;}

    // the value of the MASTER keyword in this table
    const String &master() const {return itsMaster;}

    // the value of the NUMPHASE keyword in this table
    Int numphase() const {return itsNumphase;}

    // the value of the SWPERIOD keyword in this table
    Double swperiod() const {return itsSwperiod;}

    // The SIGREF values - converted to the MS convention
    // T if signal being observed, F if reference.
    // In original FITS, 0 -> signal and 1 -> reference
    const Vector<Bool> &sigref() const {return itsSigref;}

    // The CAL values - converted to the MS convention
    // T if CAL noise being observed, F if not.
    // In original FITS, 0->no cal, 1->cal.
    const Vector<Bool> &cal() const {return itsCal;}

    // Digest an ACT_STATE table.  Return False if one of the following rules
    // are violated:
    // 1) The changing columns must be either ICAL or ECAL and ISIGREF or ESIGREF.
    // 2) Each row of STATE must have one and only one matching row in ACT_STATE.
    // If the above are true, the return value will be true and mixedSignals will be False.
    // states will contain the STATE row numbers corresponding to the rows of ACT_STATE
    // (hence states will have the same number of elements as rows in ACT_STATE).
    // For matching rows, the row number of the corresponding STATE row will appear
    // in the location for that row in ACT_STATE (i.e. in order).  ACT_STATE rows with
    // no match in STATE will be indicated by a value of -1 at that location in states.
    // Data corresponding to that ACT_STATE row should all be zero (not checked here,
    // obviously).  Hence states is resized so that it has the same number of rows as
    // the ACT_STATE table.
    // mixedSignals = T is not necessarily fatal. If multipleCal and multipleSigref are
    // both false, that is okay.   In that case, mixedSignals = True imples that there
    // was a non-zero value in both an E and I column but not both *CAL or *SIGREF columns.
    // If oldCalState is True, then this implies that the ECAL and ICAL columns in the
    // ACT_STATE table have the wrong sense (i.e. 0->on and 1->off) instead of the
    // sense in the STATE table (0->off and 1->on).  It is expected that this will
    // be fixed in a future version of the ACS FITS file.  It is up to the
    // GBTACSFiller class to decide how to set that argument.
    Bool digestActState(const Table &actState, Vector<Int> &states,
			Bool &mixedSignals, Bool &multipleCal, Bool &multipleSigref,
			Bool oldCalState) const;
private:
    Table *itsTab;

    Bool itsAttached;

    String itsMaster;
    Int itsNumphase;
    Double itsSwperiod;

    Vector<Bool> itsSigref, itsCal;

    Int itsBaseVer, itsDeviceVer;

    Bool examineColumn(const Table &actState, const String &colName,
		       Vector<Int> &colValue, Int factor,
		       Bool oldCalState) const;
};

#endif


