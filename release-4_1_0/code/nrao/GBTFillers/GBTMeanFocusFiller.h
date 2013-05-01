//# GBTMeanFocusFiller: Fill the MEAN_FOCUS table from the ANTENNA file
//# Copyright (C) 2001
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

#ifndef NRAO_GBTMEANFOCUSFILLER_H
#define NRAO_GBTMEANFOCUSFILLER_H

#include <casa/aips.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>

//# Forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MeasurementSet;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

class GBTAntennaFile;

// <summary>
// Fill the pointing model record from a GBTAntennaFile.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> 
// </prerequisite>
//
// <etymology>
// 
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
// <thrown>
//    <li>
//    <li>
// </thrown>
//

class GBTMeanFocusFiller
{
public:
    // Attach this to the indicated MeasurementSet
    GBTMeanFocusFiller(MeasurementSet &ms);

    ~GBTMeanFocusFiller();

    // fill using the indicated antenna file and the specific time and interval
    void fill(Double time, Double interval, GBTAntennaFile &antennaFile);

    // the most recently filled row - always last in the table
    Int meanFocusId() {return (itsTable ? (Int(itsTable->nrow())-1) : -1);}

    // flush this table
    void flush() {itsTable->flush();}
private:
    Table *itsTable;

    // The columns being filled - depends on the optics type
    ScalarColumn<String> itsOpticsType;

    // prime focus
    ScalarColumn<Double> itsPFfocus, itsPFrotation, itsPFx;

    // Gregorian (use "SR" because that is what is in the ANTENNA table)
    ScalarColumn<Double> itsSRxp, itsSRyp, itsSRzp, itsSRxt, itsSRyt, itsSRzt;

    // undefined an inaccessable
    GBTMeanFocusFiller();
    GBTMeanFocusFiller(const GBTMeanFocusFiller &other);
    GBTMeanFocusFiller &operator=(const GBTMeanFocusFiller &other);

};

#endif


