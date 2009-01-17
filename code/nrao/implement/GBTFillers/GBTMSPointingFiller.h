//# GBTMSPointingFiller.h: GBTMSPointingFiller fills the MSPointing table for GBT fillers
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

#ifndef NRAO_GBTMSPOINTINGFILLER_H
#define NRAO_GBTMSPOINTINGFILLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/SimOrdMap.h>
#include <scimath/Mathematics/StatAcc.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/MVTime.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <ms/MeasurementSets/MSPointing.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class MSPointingColumns;
class MeasurementSet;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

class GBTAntennaDAPFiller;
class GBTAntennaFile;
class GBTPointModelFiller;
class GBTMeanFocusFiller;

// <summary>
// GBTMSPointingFiller fills the MSPointing table for GBT fillers
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
// This class puts GBT data (fills) into the MSPointing table.
// </etymology>
//
// <synopsis>
// This class puts the appropriate values for the GBT in an MSPointing
// table (which has been previously created as part of a MeasurementSet).
// On each fill() any necessary additions to the MSPointing are made.  
// Each fill() adds a new row to the MSPointing table.
// 
// The pointingId is available to be used by 
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

class GBTMSPointingFiller
{
public:

    // The default ctor.  No MSPointing table has been attached yet.
    // Calling fill on such a filler will throw an exception.
    // It is present so that this object can be instantiated before
    // an MSPointing has been created.
    GBTMSPointingFiller();

    // A filler attached to an MSPointing table
    GBTMSPointingFiller(MSPointing &msPointing);

    ~GBTMSPointingFiller();

    // attach it to an MSPointing
    void attach(MSPointing &msPointing);

    // Use the indicated antenna DAP filler to get the antenna positions
    // from.  This class is not responsible for deleting this pointer.
    void setAntennaDAPFiller(GBTAntennaDAPFiller *antennaDAPFiller);

    // Use the indicated antenna file to get the antenna positions.
    // This class is not responsible for deleting this pointer.
    // If there is an attached antenna file, this always takes precendence
    // over any attached antenna DAP filler.
    void setAntennaFile(GBTAntennaFile *antennaFile);

    // fill using the indicated sourceId and using the times to get the
    // appropriate antenna positions at those times to be stored in the MSPointing table.
    // Each fill() call adds times.nelements() rows to MSPointing IF there are antenna
    // positions, otherwise just a single row will be added.  This assumes that
    // same integration interval applies to all times in a given fill.
    void fill(MeasurementSet &ms, Int antennaId, const Vector<Double> &times, Double interval);

    // this is the row numbers of the most recently "filled" MSPointing row
    // it returns an empty vector if nothing has been filled yet
    const Vector<Int> &pointingId() const {return pointingId_p;}

    // flush the underlying MS subtable and the model and focus fillers
  void flush();

private:
    MSPointing *msPointing_p;
    MSPointingColumns *msPointingCols_p;

    Vector<Int> pointingId_p;

    GBTAntennaDAPFiller *antDAPFiller_p;
    GBTAntennaFile *antennaFile_p;

    GBTPointModelFiller *modelFiller_p;
    // this ultimately belongs in the FEED table, I think.
    GBTMeanFocusFiller *focusFiller_p;

    // NRAO_GBT_USER_DIRECTION column - this is a ArrayMeasColumn<MDirection>
    // where the number of elements is given by NUM_POLY as with the DIRECTION
    // column but where the reference frame may vary from row to row.
    ArrayMeasColumn<MDirection> userDirection_p;

    // hacks until I know how to really set the user-defined coordinate system
    ScalarColumn<Double> userEquinox_p;
    ScalarColumn<Bool> userDefined_p;

    // NRAO_GBT_MEAN_FOCUS_ID column, id to mean focus table
    ScalarColumn<Int> meanFocusId_p;

    StatAcc<Double> raStatAcc_p, decStatAcc_p;

    // to be used when filling the TIME column
    MVTime j2000_p;

    // initialize the above for the first time
    void init(MSPointing &msPointing);

    // undefined and unavailable
    GBTMSPointingFiller(const GBTMSPointingFiller &other);
    void operator=(const GBTMSPointingFiller &other);
};

#endif


