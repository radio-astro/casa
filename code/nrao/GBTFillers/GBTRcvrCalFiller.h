//# GBTRcvrCalFiller: A filler for GBT receiver TCAL tables.
//# Copyright (C) 2001,2002
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

#ifndef NRAO_GBTRCVRCALFILLER_H
#define NRAO_GBTRCVRCALFILLER_H

#include <casa/aips.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableColumn.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN
template <class T> class Matrix;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// A filler for GBT receiver TCAL tables.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> GBT DAP FITS files
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
// <thrown>
//    <li>
//    <li>
// </thrown>
//

class GBTRcvrCalFiller
{
public:
    // Fill to any NRAO_GBT_RCVRCAL table already attached to parent or
    // create a new one to fill to.
    GBTRcvrCalFiller(Table &parent);

    ~GBTRcvrCalFiller() {;}

    // fill using the table name supplied.  If that table name has already
    // been filled, nothing new will be added to the table being filled.
    // In either case, the table returned by lastFilled will be a reference
    // to those rows filled from the indicated fileName.
    Bool fill(const String &fileName);

    // return a reference to the rows filled by the fileName used in the
    // most recent call to fill
    const Table &lastFilled() {return itsLastFilled;}

    // return the filenames used in the last call to fill
    const String &lastFileName() {return itsLastFileName;}

    // interpolate to given arrays from the measured TCAL and TRX
    // vectors given the indicated frequency vector, receiver name,
    // feed name, and polarizations vector
    void interpolate(const String &receiverName, 
		     const Vector<String> &feedName,
		     const Vector<String> &polarizations,
		     const Vector<Double> &frequencies,
		     String &testDate,
		     Matrix<Float> &tcal, Matrix<Float> &trx,
		     Bool useHighCal);

    // flush the underlying table
    void flush() {itsTable.flush();}
private:
    // the table being filled
    Table itsTable;

    // a reference to the portion of itsTable most filled from the
    // file pointed to by itsLastFileName
    Table itsLastFilled;

    String itsLastFileName;

    // the columns being filled
    // scalar columns from the primary HDU - use ISM as storage manager
    TableColumn fileNameCol, receiver, dateObs, minFreq, maxFreq, nBeam,
	tauzenit, etal, apereff, beameff;

    // scalar columns from the individual table headers - use std SM
    TableColumn testDate, receptor, feed, polarize, bandwdth, engineer,
	tech;

    // vector columns from the rows - each cell from one row
    ArrayColumn<Float> frequency, trx, tcalLo, tcalHi;

    void putScalarString(const String &fieldName, const TableRecord &rec,
			 Int rownr, const String &defval, TableColumn &col,
			 const String &altName=String());
    void putScalarFloat(const String &fieldName, const TableRecord &rec,
			Int rownr, Float scale, Float defval,
			TableColumn &col, const String &altName=String(),
			Float altScale=1.0);
    void putScalarInt(const String &fieldName, const TableRecord &rec,
		      Int rownr, Int defval, TableColumn &col,
		      const String &altName=String());

    // undefined an inaccessible
    GBTRcvrCalFiller();
    GBTRcvrCalFiller(const GBTRcvrCalFiller &other);
    GBTRcvrCalFiller &operator=(const GBTRcvrCalFiller &other);
};

#endif


