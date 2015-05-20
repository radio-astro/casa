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
    PointingDirectionCalculator(MeasurementSet const &ms);

    ~PointingDirectionCalculator() {
    }

    void selectData(String const &antenna = "", String const &spw =
            "", String const &field = "", String const &time = "",
            String const &scan = "", String const &feed = "",
            String const &intent = "", String const &observation =
                    "", String const &uvrange = "",
            String const &msselect = "");
    void setDirectionColumn(String const &columnName = "DIRECTION");
    void setFrame(String const frameType);
    void setDirectionListMatrixShape(
            PointingDirectionCalculator::MatrixShape const shape);
    void setMovingSource(String const sourceName);
    void setMovingSource(MDirection const &sourceDirection);

    uInt getNrowForSelectedMS() {return selectedMS_->nrow();}
    MDirection::Types const &getDirectionType() {return directionType_;}
    MDirection const &getMovingSourceDirection() {return *movingSource_;}
    Matrix<Double> getDirection();
    Vector<Double> getDirection(uInt irow);
    Vector<uInt> getRowId();
    uInt getRowId(uInt i);

private:
    void init();
    void initPointingTable(Int const antennaId);
    void resetAntennaPosition(Int const antennaId);
    void resetTime(Double const timestamp);
    void inspectAntenna();
    void configureMovingSourceCorrection();
    Vector<Double> doGetDirection(uInt irow);

    // table access stuff
    CountedPtr<MeasurementSet> originalMS_;
    CountedPtr<MeasurementSet> selectedMS_;
    CountedPtr<MSPointing> pointingTable_;
    CountedPtr<ROMSPointingColumns> pointingColumns_;
    ROScalarMeasColumn<MEpoch> timeColumn_;
    ROScalarColumn<Double> intervalColumn_;
    ROScalarColumn<Int> antennaColumn_;
    String directionColumnName_;
    MDirection (*accessor_)(ROMSPointingColumns &pointingColumns,
            uInt rownr);

    // conversion stuff
    MPosition antennaPosition_;
    MEpoch referenceEpoch_;
    MeasFrame referenceFrame_;
    CountedPtr<MDirection::Convert> directionConvert_;
    MDirection::Types directionType_;
    CountedPtr<MDirection> movingSource_;
    CountedPtr<MDirection::Convert> movingSourceConvert_;
    void (*movingSourceCorrection_)(
            CountedPtr<MDirection::Convert> &convertToAzel,
            CountedPtr<MDirection::Convert> &convertToCelestial,
            Vector<Double> &direction);

    // other
    Vector<uInt> antennaBoundary_;
    uInt numAntennaBoundary_;
    Vector<Double> pointingTimeUTC_;
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
