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
    PointingDirectionCalculator(casacore::MeasurementSet const &ms);

    ~PointingDirectionCalculator() {
    }

    void selectData(casacore::String const &antenna = "", casacore::String const &spw =
            "", casacore::String const &field = "", casacore::String const &time = "",
            casacore::String const &scan = "", casacore::String const &feed = "",
            casacore::String const &intent = "", casacore::String const &observation =
                    "", casacore::String const &uvrange = "",
            casacore::String const &msselect = "");
    void setDirectionColumn(casacore::String const &columnName = "DIRECTION");
    void setFrame(casacore::String const frameType);
    void setDirectionListMatrixShape(
            PointingDirectionCalculator::MatrixShape const shape);
    void setMovingSource(casacore::String const sourceName);
    void setMovingSource(casacore::MDirection const &sourceDirection);
    void unsetMovingSource();

    casacore::uInt getNrowForSelectedMS() {return selectedMS_->nrow();}
    casacore::MDirection::Types const &getDirectionType() {return directionType_;}
    casacore::MDirection const &getMovingSourceDirection() {return *movingSource_;}
    casacore::Matrix<casacore::Double> getDirection();
    casacore::Vector<casacore::Double> getDirection(casacore::uInt irow);
    casacore::Vector<casacore::uInt> getRowId();
    casacore::uInt getRowId(casacore::uInt i);

private:
    void init();
    void initPointingTable(casacore::Int const antennaId);
    void resetAntennaPosition(casacore::Int const antennaId);
    void resetTime(casacore::Double const timestamp);
    void inspectAntenna();
    void configureMovingSourceCorrection();
    casacore::Vector<casacore::Double> doGetDirection(casacore::uInt irow);

    // table access stuff
    casacore::CountedPtr<casacore::MeasurementSet> originalMS_;
    casacore::CountedPtr<casacore::MeasurementSet> selectedMS_;
    casacore::CountedPtr<casacore::MSPointing> pointingTable_;
    casacore::CountedPtr<casacore::ROMSPointingColumns> pointingColumns_;
    casacore::ROScalarMeasColumn<casacore::MEpoch> timeColumn_;
    casacore::ROScalarColumn<casacore::Double> intervalColumn_;
    casacore::ROScalarColumn<casacore::Int> antennaColumn_;
    casacore::String directionColumnName_;
    casacore::MDirection (*accessor_)(casacore::ROMSPointingColumns &pointingColumns,
            casacore::uInt rownr);

    // conversion stuff
    casacore::MPosition antennaPosition_;
    casacore::MEpoch referenceEpoch_;
    casacore::MeasFrame referenceFrame_;
    casacore::CountedPtr<casacore::MDirection::Convert> directionConvert_;
    casacore::MDirection::Types directionType_;
    casacore::CountedPtr<casacore::MDirection> movingSource_;
    casacore::CountedPtr<casacore::MDirection::Convert> movingSourceConvert_;
    void (*movingSourceCorrection_)(
            casacore::CountedPtr<casacore::MDirection::Convert> &convertToAzel,
            casacore::CountedPtr<casacore::MDirection::Convert> &convertToCelestial,
            casacore::Vector<casacore::Double> &direction);

    // other
    casacore::Vector<casacore::uInt> antennaBoundary_;
    casacore::uInt numAntennaBoundary_;
    casacore::Vector<casacore::Double> pointingTimeUTC_;
    casacore::Double lastTimeStamp_;
    casacore::Int lastAntennaIndex_;
    casacore::uInt pointingTableIndexCache_;
    PointingDirectionCalculator::MatrixShape shape_;

    // privatize  default constructor
    PointingDirectionCalculator();
}
;

} //# NAMESPACE CASA - END

#endif /* _SYNTHESIS_POINTING_DIRECTION_CALCULATOR_H_ */
