//# PointingDirectionCalculator.h: Does for MSes various fixes which do not involve calibrating.
//# Copyright (C) 2008
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
//# $Id$
//#

#ifndef _SYNTHESIS_POINTING_DIRECTION_CALCULATOR_H_
#define _SYNTHESIS_POINTING_DIRECTION_CALCULATOR_H_

#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Utilities/CountedPtr.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSPointing.h>
#include <ms/MeasurementSets/MSPointingColumns.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <tables/Tables/ScalarColumn.h>

namespace casa {

class PointingDirectionCalculator {
public:
    enum MatrixShape {
        COLUMN_MAJOR, ROW_MAJOR
    };
    PointingDirectionCalculator(casa::MeasurementSet const &ms);

    ~PointingDirectionCalculator() {
    }

    void selectData(casa::String const &antenna = "", casa::String const &spw =
            "", casa::String const &field = "", casa::String const &time = "",
            casa::String const &scan = "", casa::String const &feed = "",
            casa::String const &intent = "", casa::String const &observation =
                    "", casa::String const &uvrange = "",
            casa::String const &msselect = "");
    void setDirectionColumn(casa::String const &columnName = "DIRECTION");
    void setFrame(casa::String const frameType);
    void setDirectionListMatrixShape(
            PointingDirectionCalculator::MatrixShape const shape);
    void setMovingSource(casa::String const sourceName);
    void setMovingSource(casa::MDirection const &sourceDirection);

    casa::Matrix<Double> getDirection();
    casa::Vector<Double> getDirection(uInt i);
    casa::Vector<uInt> getRowId();
    uInt getRowId(uInt i);

private:
    void init();
    void initPointingTable(Int const antennaId);
    void resetAntennaPosition(Int antennaId);
    void resetTime(uInt rownr);
    void inspectAntenna();

    // table access stuff
    CountedPtr<casa::MeasurementSet> originalMS_;
    CountedPtr<casa::MeasurementSet> selectedMS_;
    CountedPtr<casa::MSPointing> pointingTable_;
    CountedPtr<casa::ROMSPointingColumns> pointingColumns_;
    casa::ROScalarMeasColumn<casa::MEpoch> timeColumn_;
    casa::ROScalarColumn<Double> intervalColumn_;
    casa::ROScalarColumn<Int> antennaColumn_;
    casa::String directionColumnName_;
    casa::MDirection (*accessor_)(casa::ROMSPointingColumns &pointingColumns, uInt rownr);

    // conversion stuff
    casa::MPosition antennaPosition_;
    casa::MEpoch referenceEpoch_;
    casa::MeasFrame referenceFrame_;
    CountedPtr<casa::MDirection::Convert> directionConvert_;
    casa::MDirection::Types directionType_;
    CountedPtr<casa::MDirection> movingSource_;

    // other
    Vector<uInt> antennaBoundary_;
    uInt numAntennaBoundary_;
    Double lastTimeStamp_;
    Int lastAntennaIndex_;
    uInt pointingTableIndexCache_;
    PointingDirectionCalculator::MatrixShape shape_;

    // privatize  default constructor
    PointingDirectionCalculator();
}
;

} //# NAMESPACE CASA - END

#endif /* _SYNTHESIS_POINTING_DIRECTION_CALCULATOR_H_ */
