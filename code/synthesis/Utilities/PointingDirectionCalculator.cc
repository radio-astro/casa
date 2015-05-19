//# PointingDirectionCalculator.cc: Implementation of PointingDirectionCalculator.h
//# All helper functions of imager moved here for readability
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
#include <cassert>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <synthesis/Utilities/PointingDirectionCalculator.h>

#include <casa/aipstype.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Containers/Block.h>
#include <casa/Utilities/BinarySearch.h>
#include <casa/Logging/LogIO.h>
#include <tables/TaQL/ExprNode.h>
#include <ms/MSSel/MSSelection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>

using namespace casacore;
using namespace std;

// Debug Message Handling
// if DIRECTIONCALC_DEBUG is defined, the macro debuglog and
// debugpost point standard output stream (std::cout and
// std::endl so that debug messages are sent to standard
// output. Otherwise, these macros basically does nothing.
// "Do nothing" behavior is implemented in NullLogger
// and its associating << operator below.
//
// Usage:
// Similar to standard output stream.
//
//   debuglog << "Any message" << any_value << debugpost;
//
//#define DIRECTIONCALC_DEBUG

namespace {
struct NullLogger {
};

template<class T>
inline NullLogger &operator<<(NullLogger &logger, T /*value*/) {
    return logger;
}

#ifndef DIRECTIONCALC_DEBUG
NullLogger nulllogger;
#endif
}

#ifdef DIRECTIONCALC_DEBUG
#define debuglog cout << "PointingDirectionCalculator::DEBUG "
#define debugpost endl
#else
#define debuglog nulllogger
#define debugpost 0
#endif

namespace {
#define ARRAY_DIRECTION(ColumnName) \
inline MDirection ColumnName ## Accessor(ROMSPointingColumns &pointingColumns, uInt rownr) { \
    return pointingColumns.ColumnName ## Meas(rownr); \
}

#define SCALAR_DIRECTION(ColumnName) \
inline MDirection ColumnName ## Accessor(ROMSPointingColumns &pointingColumns, uInt rownr) { \
    return pointingColumns.ColumnName ## Meas()(rownr); \
}

ARRAY_DIRECTION(direction)
ARRAY_DIRECTION(target)
ARRAY_DIRECTION(pointingOffset)
ARRAY_DIRECTION(sourceOffset)
SCALAR_DIRECTION(encoder)

// working function for moving source correction
// convertToAzel must be configured with moving source direction and
// proper reference frame. Also, convertToCelestial must refer proper
// reference frame.
inline void performMovingSourceCorrection(
        CountedPtr<MDirection::Convert> &convertToAzel,
        CountedPtr<MDirection::Convert> &convertToCelestial,
        Vector<Double> &direction) {
    // moving source handling
    // If moving source is specified, output direction list is always
    // offset from reference position of moving source
    MDirection srcAzel = (*convertToAzel)();
    MDirection srcDirection = (*convertToCelestial)(srcAzel);
    Vector<Double> srcDirectionVal = srcDirection.getAngle("rad").getValue();
    direction -= srcDirectionVal;
}

inline void skipMovingSourceCorrection(
        CountedPtr<MDirection::Convert> &/*convertToAzel*/,
        CountedPtr<MDirection::Convert> &/*convertToCelestial*/,
        Vector<Double> &/*direction*/) {
    // do nothing
}
} // anonymous namespace

namespace casa {
PointingDirectionCalculator::PointingDirectionCalculator(
        MeasurementSet const &ms) :
        originalMS_(new MeasurementSet(ms)), selectedMS_(), pointingTable_(), pointingColumns_(), timeColumn_(), intervalColumn_(), antennaColumn_(), directionColumnName_(), accessor_(
        NULL), antennaPosition_(), referenceEpoch_(), referenceFrame_(
                referenceEpoch_, antennaPosition_), directionConvert_(
        NULL), directionType_(MDirection::J2000), movingSource_(NULL), movingSourceConvert_(
        NULL), movingSourceCorrection_(NULL), antennaBoundary_(), numAntennaBoundary_(
                0), pointingTimeUTC_(), lastTimeStamp_(-1.0), lastAntennaIndex_(
                -1), pointingTableIndexCache_(0), shape_(
                PointingDirectionCalculator::COLUMN_MAJOR) {
    accessor_ = directionAccessor;

    Block<String> sortColumns(2);
    sortColumns[0] = "ANTENNA1";
    sortColumns[1] = "TIME";
    selectedMS_ = new MeasurementSet(originalMS_->sort(sortColumns));

    init();

    // set default output direction reference frame
    setFrame("J2000");

    // set default direction column name
    setDirectionColumn("DIRECTION");
}

void PointingDirectionCalculator::init() {
    // attach column
    timeColumn_.attach(*selectedMS_, "TIME");
    intervalColumn_.attach(*selectedMS_, "INTERVAL");
    antennaColumn_.attach(*selectedMS_, "ANTENNA1");

    // initial setup
    debuglog << "inspectAntenna" << debugpost;
    inspectAntenna();
    debuglog << "done" << debugpost;

    resetAntennaPosition(antennaColumn_(0));
}

void PointingDirectionCalculator::selectData(String const &antenna,
        String const &spw, String const &field, String const &time,
        String const &scan, String const &feed, String const &intent,
        String const &observation, String const &uvrange,
        String const &msselect) {
    // table selection
    MSSelection thisSelection;
    thisSelection.setAntennaExpr(antenna);
    thisSelection.setSpwExpr(spw);
    thisSelection.setFieldExpr(field);
    thisSelection.setTimeExpr(time);
    thisSelection.setScanExpr(scan);
    thisSelection.setStateExpr(intent);
    thisSelection.setObservationExpr(observation);
    thisSelection.setUvDistExpr(uvrange);
    thisSelection.setTaQLExpr(msselect);

    TableExprNode exprNode = thisSelection.getTEN(&(*originalMS_));

    // sort by ANTENNA1 and TIME for performance reason
    Block<String> sortColumns(2);
    sortColumns[0] = "ANTENNA1";
    sortColumns[1] = "TIME";
    if (exprNode.isNull()) {
        debuglog << "NULL selection" << debugpost;
        selectedMS_ = new MeasurementSet(originalMS_->sort(sortColumns));
    } else {
        debuglog << "Sort of selection" << debugpost;
        MeasurementSet tmp = (*originalMS_)(exprNode);
        selectedMS_ = new MeasurementSet(tmp.sort(sortColumns));
    }
    debuglog << "selectedMS_->nrow() = " << selectedMS_->nrow() << debugpost;
    if (selectedMS_->nrow() == 0) {
        stringstream ss;
        ss << "Selected MS is empty for given selection: " << endl;
        if (!antenna.empty()) {
            ss << "\tantenna \"" << antenna << "\"" << endl;
        }
        if (!spw.empty()) {
            ss << "\tspw \"" << spw << "\"" << endl;
        }
        if (!field.empty()) {
            ss << "\tfield \"" << field << "\"" << endl;
        }
        if (!time.empty()) {
            ss << "\ttime \"" << time << "\"" << endl;
        }
        if (!scan.empty()) {
            ss << "\tscan \"" << scan << "\"" << endl;
        }
        if (!feed.empty()) {
            ss << "\tfeed \"" << feed << "\"" << endl;
        }
        if (!intent.empty()) {
            ss << "\tintent \"" << intent << "\"" << endl;
        }
        if (!observation.empty()) {
            ss << "\tobservation \"" << observation << "\"" << endl;
        }
        if (!uvrange.empty()) {
            ss << "\tuvrange \"" << uvrange << "\"" << endl;
        }
        if (!msselect.empty()) {
            ss << "\tmsselect \"" << msselect << "\"" << endl;
        }

        throw AipsError(ss.str());
    }

    init();

    debuglog << "done selectdata" << debugpost;
}

void PointingDirectionCalculator::configureMovingSourceCorrection() {
    if (!movingSource_.null() || directionColumnName_.contains("OFFSET")) {
        movingSourceCorrection_ = performMovingSourceCorrection;
    } else {
        movingSourceCorrection_ = skipMovingSourceCorrection;
    }
}

void PointingDirectionCalculator::setDirectionColumn(String const &columnName) {
    String columnNameUpcase = columnName;
    columnNameUpcase.upcase();
    if (!(originalMS_->pointing().tableDesc().isColumn(columnNameUpcase))) {
        stringstream ss;
        ss << "Column \"" << columnNameUpcase
                << "\" doesn't exist in POINTING table.";
        throw AipsError(ss.str());
    }

    directionColumnName_ = columnNameUpcase;

    if (directionColumnName_ == "DIRECTION") {
        accessor_ = directionAccessor;
    } else if (directionColumnName_ == "TARGET") {
        accessor_ = targetAccessor;
    } else if (directionColumnName_ == "POINTING_OFFSET") {
        accessor_ = pointingOffsetAccessor;
    } else if (directionColumnName_ == "SOURCE_OFFSET") {
        accessor_ = sourceOffsetAccessor;
    } else if (directionColumnName_ == "ENCODER") {
        accessor_ = encoderAccessor;
    } else {
        stringstream ss;
        ss << "Column \"" << columnNameUpcase << "\" is not supported.";
        throw AipsError(ss.str());
    }

    configureMovingSourceCorrection();
}

void PointingDirectionCalculator::setFrame(String const frameType) {
    Bool status = MDirection::getType(directionType_, frameType);
    if (!status) {
        LogIO os(LogOrigin("PointingDirectionCalculator", "setFrame", WHERE));
        os << LogIO::WARN << "Conversion of frame string \"" << frameType
                << "\" into direction type enum failed. Use J2000."
                << LogIO::POST;
        directionType_ = MDirection::J2000;
    }

    // create conversion engine
    MDirection nominalInputMeasure = accessor_(*pointingColumns_, 0);
    MDirection::Ref outReference(directionType_, referenceFrame_);
    directionConvert_ = new MDirection::Convert(nominalInputMeasure,
            outReference);
    const MEpoch *e = dynamic_cast<const MEpoch *>(referenceFrame_.epoch());
    const MPosition *p =
            dynamic_cast<const MPosition *>(referenceFrame_.position());
    debuglog << "Conversion Setup: Epoch "
            << e->get("s").getValue() << " " << e->getRefString() << " Position "
            << p->get("m").getValue() << " " << p->getRefString()
            << debugpost;
}

void PointingDirectionCalculator::setDirectionListMatrixShape(
        PointingDirectionCalculator::MatrixShape const shape) {
    shape_ = shape;
}

void PointingDirectionCalculator::setMovingSource(String const sourceName) {
    MDirection sourceDirection(Quantity(0.0, "deg"), Quantity(90.0, "deg"));
    sourceDirection.setRefString(sourceName);
    setMovingSource(sourceDirection);
}

void PointingDirectionCalculator::setMovingSource(
        MDirection const &sourceDirection) {
    movingSource_ = dynamic_cast<MDirection *>(sourceDirection.clone());

    // create conversion engine for moving source
    MDirection::Ref refAzel(MDirection::AZEL, referenceFrame_);
    movingSourceConvert_ = new MDirection::Convert(*movingSource_, refAzel);

    configureMovingSourceCorrection();
}

Matrix<Double> PointingDirectionCalculator::getDirection() {
    assert(!selectedMS_.null());

    uInt const nrow = selectedMS_->nrow();
    debuglog << "selectedMS_->nrow() = " << nrow << debugpost;
    Vector<Double> outDirectionFlattened(2 * nrow);
    // column major data offset and increment for outDirectionFlattened,
    // and output matrix shape
    uInt offset = nrow;
    uInt increment = 1;
    // matrix shape: number of rows is nrow and number of columns is 2
    IPosition outShape(2, nrow, 2);
    if (shape_ == PointingDirectionCalculator::ROW_MAJOR) {
        // column major specific offset, increment and output shape
        offset = 1;
        increment = 2;
        // matrix shape: number of rows is 2 and number of columns is nrow
        outShape = IPosition(2, 2, nrow);
    }

    for (uInt i = 0; i < numAntennaBoundary_ - 1; ++i) {
        uInt start = antennaBoundary_[i];
        uInt end = antennaBoundary_[i + 1];
        uInt currentAntenna = antennaColumn_(start);
        resetAntennaPosition(currentAntenna);
        debuglog << "antenna " << currentAntenna << " start " << start
                << " end " << end << debugpost;
        uInt const nrowPointing = pointingTimeUTC_.nelements();
        debuglog << "nrowPointing = " << nrowPointing << debugpost;
        debuglog << "pointingTimeUTC = " << min(pointingTimeUTC_) << "~"
        << max(pointingTimeUTC_) << debugpost;
        for (uInt j = start; j < end; ++j) {
            debuglog << "start index " << j << debugpost;
            Vector<Double> direction = doGetDirection(j);
            debuglog << "index for lat: " << (j * increment)
                    << " (cf. outDirectionFlattened.nelements()="
                    << outDirectionFlattened.nelements() << ")" << debugpost;
            debuglog << "index for lon: " << (offset + j * increment)
                    << debugpost;
            outDirectionFlattened[j * increment] = direction[0];
            outDirectionFlattened[offset + j * increment] = direction[1];
        }
        debuglog << "done antenna " << currentAntenna << debugpost;
    }
    debuglog << "done getDirection" << debugpost;
    return Matrix < Double > (outShape, outDirectionFlattened.data());
}

Vector<Double> PointingDirectionCalculator::doGetDirection(uInt irow) {
    debuglog << "doGetDirection(" << irow << ")" << debugpost;
    Double currentTime =
            timeColumn_.convert(irow, MEpoch::UTC).get("s").getValue();
    resetTime(currentTime);

    // search and interpolate if necessary
    Bool exactMatch;
    uInt const nrowPointing = pointingTimeUTC_.nelements();
    // pointingTableIndexCache_ is not so effective in terms of performance
    // simple binary search may be enough,
    Int index = binarySearch(exactMatch, pointingTimeUTC_, currentTime,
            nrowPointing, 0);
    debuglog << "binarySearch result " << index << debugpost;
//    uInt n = nrowPointing - pointingTableIndexCache_;
//    Int lower = pointingTableIndexCache_;
//    debuglog << "do binarySearch n=" << n << " lower=" << lower
//            << " nrowPointing=" << nrowPointing << " cache="
//            << pointingTableIndexCache_ << debugpost;
//    Int index = binarySearch(exactMatch, pointingTimeUTC_, currentTime, n,
//            lower);
//    debuglog << "binarySearch result " << index << debugpost;
//    if (!exactMatch && lower > 0 && index == lower) {
//        // maybe out of range
//        n = nrowPointing - n;
//        lower = 0;
//        debuglog << "do second binarySearch n=" << n << " lower=" << lower
//                << " nrowPointing=" << nrowPointing << " cache="
//                << pointingTableIndexCache_ << debugpost;
//        index = binarySearch(exactMatch, pointingTimeUTC_, currentTime, n,
//                lower);
//        debuglog << "second binarySearch result " << index << debugpost;
//    }
//    pointingTableIndexCache_ = (uInt) max((Int) 0, (Int) (index - 1));
    debuglog << "Time " << setprecision(16) << currentTime << " idx=" << index
            << debugpost;
    MDirection direction;
    assert(accessor_ != NULL);
    if (exactMatch) {
        debuglog << "exact match" << debugpost;
        direction = accessor_(*pointingColumns_, index);
    } else if (index <= 0) {
        debuglog << "take 0th row" << debugpost;
        direction = accessor_(*pointingColumns_, 0);
    } else if (index >= (Int) (nrowPointing - 1)) {
        debuglog << "take final row" << debugpost;
        direction = accessor_(*pointingColumns_, nrowPointing - 1);
//            } else if (currentInterval > pointingIntervalColumn(index)) {
//                // Sampling rate of pointing < data dump rate
//                // nearest interpolation
//                debuglog << "nearest interpolation" << debugpost;
//                Double dt1 = abs(pointingTimeUTC[index] - currentTime);
//                Double dt2 = abs(currentTime - pointingTimeUTC[index - 1]);
//                if (dt1 >= dt2) {
//                    // midpoint takes the value at index - 1
//                    direction = accessor_(*pointingColumns_, index - 1);
//                } else {
//                    direction = accessor_(*pointingColumns_, index);
//                }
    } else {
        debuglog << "linear interpolation " << debugpost;
        // Sampling rate of pointing > data dump rate (fast scan)
        // linear interpolation
        Double t0 = pointingTimeUTC_[index - 1];
        Double t1 = pointingTimeUTC_[index];
        Double dt = t1 - t0;
        debuglog << "Interpolate between " << setprecision(16) << index - 1
                << " (" << t0 << ") and " << index << " (" << t1 << ")"
                << debugpost;
        MDirection dir1 = accessor_(*pointingColumns_, index - 1);
        MDirection dir2 = accessor_(*pointingColumns_, index);
        String dirRef1 = dir1.getRefString();
        String dirRef2 = dir2.getRefString();
        MDirection::Types refType1, refType2;
        MDirection::getType(refType1, dirRef1);
        MDirection::getType(refType2, dirRef2);
        debuglog << "dirRef1 = " << dirRef1 << " ("
                << MDirection::showType(refType1) << ")" << debugpost;
        if (dirRef1 != dirRef2) {
            MeasFrame referenceFrameLocal((pointingColumns_->timeMeas())(index),
                    *(referenceFrame_.position()));
            dir2 = MDirection::Convert(dir2,
                    MDirection::Ref(refType1, referenceFrameLocal))();
        }
        Vector<Double> dirVal1 = dir1.getAngle("rad").getValue();
        Vector<Double> dirVal2 = dir2.getAngle("rad").getValue();
        Vector<Double> scanRate = dirVal2 - dirVal1;
        Vector<Double> interpolated = dirVal1
                + scanRate * (currentTime - t0) / dt;
        direction = MDirection(Quantum<Vector<Double> >(interpolated, "rad"),
                refType1);
    }
    debuglog << "direction = "
            << direction.getAngle("rad").getValue() << " (unit rad reference frame "
            << direction.getRefString()
            << ")" << debugpost;
    Vector<Double> outVal(2);
    if (direction.getRefString() == MDirection::showType(directionType_)) {
        outVal = direction.getAngle("rad").getValue();
    } else {
        MDirection converted = (*directionConvert_)(direction);
        outVal = converted.getAngle("rad").getValue();
        debuglog << "converted = " << outVal << "(unit rad reference frame "
                << converted.getRefString() << ")" << debugpost;
    }

    // moving source correction
    assert(movingSourceCorrection_ != NULL);
    movingSourceCorrection_(movingSourceConvert_, directionConvert_, outVal);

    return outVal;
}

Vector<Double> PointingDirectionCalculator::getDirection(uInt i) {
    if (i >= selectedMS_->nrow()) {
        stringstream ss;
        ss << "Out of range row index: " << i << " (nrow for selected MS "
                << getNrowForSelectedMS() << ")" << endl;
        throw AipsError(ss.str());
    }
    debuglog << "start row " << i << debugpost;
    Int currentAntennaIndex = antennaColumn_(i);
    debuglog << "currentAntennaIndex = " << currentAntennaIndex
            << " lastAntennaIndex_ = " << lastAntennaIndex_ << debugpost;
    Double currentTime =
            timeColumn_.convert(i, MEpoch::UTC).get("s").getValue();
    resetAntennaPosition(currentAntennaIndex);
    debuglog << "currentTime = " << currentTime << " lastTimeStamp_ = "
            << lastTimeStamp_ << debugpost;
    if (currentTime != lastTimeStamp_) {
        resetTime(i);
    }
    debuglog << "doGetDirection" << debugpost;
    Vector<Double> direction = doGetDirection(i);
    return direction;
}

Vector<uInt> PointingDirectionCalculator::getRowId() {
    return selectedMS_->rowNumbers();
}

uInt PointingDirectionCalculator::getRowId(uInt i) {
    return selectedMS_->rowNumbers()[i];
}

void PointingDirectionCalculator::inspectAntenna() {
    // selectedMS_ must be sorted by ["ANTENNA1", "TIME"]
    antennaBoundary_.resize(selectedMS_->antenna().nrow() + 1);
    antennaBoundary_ = -1;
    Int count = 0;
    antennaBoundary_[count] = 0;
    ++count;
    Vector<Int> antennaList = antennaColumn_.getColumn();
    uInt nrow = antennaList.nelements();
    Int lastAnt = antennaList[0];
    for (uInt i = 0; i < nrow; ++i) {
        if (antennaList[i] != lastAnt) {
            antennaBoundary_[count] = i;
            ++count;
            lastAnt = antennaList[i];
        }
    }
    antennaBoundary_[count] = nrow;
    ++count;
    numAntennaBoundary_ = count;
    debuglog << "antennaBoundary_=" << antennaBoundary_ << debugpost;
    debuglog << "numAntennaBoundary_=" << numAntennaBoundary_ << debugpost;
}

void PointingDirectionCalculator::initPointingTable(Int const antennaId) {
    if (!pointingTable_.null() && !pointingColumns_.null()
            && pointingTable_->nrow() > 0
            && pointingColumns_->antennaId()(0) == antennaId) {
        // no need to update
        return;
    }
    debuglog << "update pointing table for antenna " << antennaId << debugpost;
    MSPointing original = selectedMS_->pointing();
    MSPointing selected = original(original.col("ANTENNA_ID") == antennaId);
    if (selected.nrow() == 0) {
        debuglog << "no rows for antenna " << antennaId << " try -1"
                << debugpost;
        // try ANTENNA_ID == -1
        selected = original(original.col("ANTENNA_ID") == -1);
        assert(selected.nrow() > 0);
        if (selected.nrow() == 0) {
            stringstream ss;
            ss << "Internal Error: POINTING table has no entry for antenna "
                    << antennaId << "." << endl;
            throw AipsError(ss.str());
        }
    }
    debuglog << "selected pointing rows " << selected.nrow() << debugpost;
    pointingTable_ = new MSPointing(selected.sort("TIME"));

    // attach columns
    pointingColumns_ = new ROMSPointingColumns(*pointingTable_);

    // initialize pointingTimeUTC_
    uInt const nrowPointing = pointingTable_->nrow();
    pointingTimeUTC_.resize(nrowPointing);
    ROScalarMeasColumn<MEpoch> pointingTimeColumn =
            pointingColumns_->timeMeas();
    for (uInt i = 0; i < nrowPointing; ++i) {
        MEpoch e = pointingTimeColumn(i);
        if (e.getRefString() == MEpoch::showType(MEpoch::UTC)) {
            pointingTimeUTC_[i] = e.get("s").getValue();
        } else {
            pointingTimeUTC_[i] =
                    MEpoch::Convert(e, MEpoch::UTC)().get("s").getValue();
        }
    }

    // reset index cache for pointing table
    pointingTableIndexCache_ = 0;

    debuglog << "done initPointingTable" << debugpost;
}

void PointingDirectionCalculator::resetAntennaPosition(Int const antennaId) {
    MSAntenna antennaTable = selectedMS_->antenna();
    uInt nrow = antennaTable.nrow();
    if (antennaId < 0 || (Int) nrow <= antennaId) {
        stringstream ss;
        ss << "Internal Error: Invalid ANTENNA_ID is specified (" << antennaId
                << ")." << endl;
        throw AipsError(ss.str());
    } else if (antennaId != lastAntennaIndex_ || lastAntennaIndex_ == -1) {
        ScalarMeasColumn < MPosition
                > antennaPositionColumn(antennaTable, "POSITION");
        antennaPosition_ = antennaPositionColumn(antennaId);
        debuglog << "antenna position: "
                << antennaPosition_.getRefString() << " "
                << setprecision(16) << antennaPosition_.get("m").getValue() << debugpost;
        referenceFrame_.resetPosition(antennaPosition_);

        initPointingTable(antennaId);

        lastAntennaIndex_ = antennaId;
    }
}

void PointingDirectionCalculator::resetTime(Double const timestamp) {
    debuglog << "resetTime(Double " << timestamp << ")" << debugpost;
    debuglog << "lastTimeStamp_ = " << lastTimeStamp_ << " timestamp = "
            << timestamp << debugpost;
    if (timestamp != lastTimeStamp_ || lastTimeStamp_ < 0.0) {
        referenceEpoch_ = MEpoch(Quantity(timestamp, "s"), MEpoch::UTC);
        referenceFrame_.resetEpoch(referenceEpoch_);

        lastTimeStamp_ = timestamp;
    }
}

}  //# NAMESPACE CASA - END
