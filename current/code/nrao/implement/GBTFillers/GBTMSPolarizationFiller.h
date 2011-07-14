//# GBTMSPolarizationFiller.h: GBTMSPolarizationFiller fills the MSPolarization table for GBT fillers
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

#ifndef NRAO_GBTMSPOLARIZATIONFILLER_H
#define NRAO_GBTMSPOLARIZATIONFILLER_H

#include <casa/aips.h>
#include <casa/Containers/RecordField.h>
#include <ms/MeasurementSets/MSPolarization.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class ColumnsIndex;
class MSPolarizationColumns;
template <class T> class Matrix;
template <class T> class Vector;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTMSPolarizationFiller fills the MSPolarization table for GBT fillers
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
// This class puts GBT data (fills) into the MSPolarization table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSPolarization
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSPolarization are made.  
// Currently this depends only on the shape of the data being filled as
// there is not yet any polarization information available from the GBT
// data associated paramters FITS files.  Existing rows are re-used
// whenever possible.
//
// The polarizationId is available to be used by the
// other GBTMS*Fillers as other subtables are filled in turn.
//
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

class GBTMSPolarizationFiller
{
public:

    // The default ctor.  No MSPolarization table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSPolarization has been created.
    GBTMSPolarizationFiller();

    // A filler attached to an MSPolarization table
    GBTMSPolarizationFiller(MSPolarization &msPolarization);

    ~GBTMSPolarizationFiller();

    // attach it to an MSPolarization
    void attach(MSPolarization &msPolarization);

    // fill - GBT MSPolarization table has one row.
    void fill(Int ncorr, const Vector<Int> &corrType, const Matrix<Int> &corrProduct);

    // this is the row number of the most recently "filled" MSPolarization row
    // it returns -1 if nothing has been filled yet
    Int polarizationId() const {return polarizationId_p;}

    // flush the underlying MS subtable
    void flush() {msPolarization_p->flush();}

private:
    ColumnsIndex *index_p;
    RecordFieldPtr<Int> ncorrKey_p;
    MSPolarization *msPolarization_p;
    MSPolarizationColumns *msPolCols_p;

    Int polarizationId_p;

    // initialize the above for the first time
    void init(MSPolarization &msPolarization);

    // undefined and unavailable
    GBTMSPolarizationFiller(const GBTMSPolarizationFiller &other);
    void operator=(const GBTMSPolarizationFiller &other);
};

#endif


