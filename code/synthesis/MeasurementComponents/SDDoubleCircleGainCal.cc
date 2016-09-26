/*
 * SDDoubleCircleGainCal.cpp
 *
 *  Created on: Jun 3, 2016
 *      Author: nakazato
 */

#include <synthesis/MeasurementComponents/SDDoubleCircleGainCal.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/MeasurementComponents/SDDoubleCircleGainCalImpl.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <synthesis/Utilities/PointingDirectionCalculator.h>
#include <synthesis/Utilities/PointingDirectionProjector.h>
#include <msvis/MSVis/VisSet.h>

#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/casa/Quanta/Quantum.h>
#include <casacore/casa/Quanta/QuantumHolder.h>
#include <casacore/tables/TaQL/TableParse.h>
#include <casacore/measures/TableMeasures/ScalarQuantColumn.h>
#include <casacore/measures/TableMeasures/ArrayQuantColumn.h>
#include <casacore/ms/MeasurementSets/MSIter.h>

#include <iostream>
#include <sstream>
#include <map>

// Debug Message Handling
// if SDGAIN_DEBUG is defined, the macro debuglog and
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
//#define SDGAINDBLC_DEBUG

namespace {
struct NullLogger {
};

template<class T>
inline NullLogger &operator<<(NullLogger &logger, T /*value*/) {
  return logger;
}
}

#ifdef SDGAINDBLC_DEBUG
#define debuglog std::cerr
#define debugpost std::endl
#else
::NullLogger nulllogger;
#define debuglog nulllogger
#define debugpost 0
#endif

namespace {
inline void fillNChanParList(casacore::String const &msName,
casacore::Vector<casacore::Int> &nChanParList) {
  casacore::MeasurementSet const ms(msName);
  casacore::MSSpectralWindow const &msspw = ms.spectralWindow();
  casacore::ROScalarColumn<casa::Int> nchanCol(msspw, "NUM_CHAN");
  debuglog<< "nchanCol=" << nchanCol.getColumn() << debugpost;
  nChanParList = nchanCol.getColumn()(casa::Slice(0, nChanParList.nelements()));
}

template<class T>
inline casacore::String toString(casacore::Vector<T> const &v) {
  std::ostringstream oss;
  oss << "[";
  std::string delimiter = "";
  for (size_t i = 0; i < v.nelements(); ++i) {
    oss << delimiter << v[i];
    delimiter = ",";
  }
  oss << "]";
  return casacore::String(oss);
}

inline casacore::String selectOnSourceAutoCorr(casacore::MeasurementSet const &ms) {
  debuglog<< "selectOnSource" << debugpost;
  casa::String taqlForState(
      "SELECT FLAG_ROW FROM $1 WHERE UPPER(OBS_MODE) ~ m/^OBSERVE_TARGET#ON_SOURCE/");
  casa::Table stateSel = casacore::tableCommand(taqlForState, ms.state());
  casa::Vector<casa::uInt> stateIdList = stateSel.rowNumbers();
  debuglog<< "stateIdList = " << stateIdList << debugpost;
  std::ostringstream oss;
  oss << "SELECT FROM $1 WHERE ANTENNA1 == ANTENNA2 && STATE_ID IN "
      << toString(stateIdList)
      << " ORDER BY FIELD_ID, ANTENNA1, FEED1, DATA_DESC_ID, TIME";
  return casacore::String(oss);
}

class DataColumnAccessor {
public:
  DataColumnAccessor(casacore::Table const &ms,
  casacore::String const colName = "DATA") :
      dataCol_(ms, colName) {
  }
  casacore::Matrix<casacore::Float> operator()(size_t irow) {
    return casacore::real(dataCol_(irow));
  }
  casacore::Cube<casacore::Float> getColumn() {
    return casacore::real(dataCol_.getColumn());
  }
private:
  DataColumnAccessor() {
  }
  casacore::ROArrayColumn<casacore::Complex> dataCol_;
};

class FloatDataColumnAccessor {
public:
  FloatDataColumnAccessor(casacore::Table const &ms) :
      dataCol_(ms, "FLOAT_DATA") {
  }
  casacore::Matrix<casacore::Float> operator()(size_t irow) {
    return dataCol_(irow);
  }
  casacore::Cube<casacore::Float> getColumn() {
    return dataCol_.getColumn();
  }
private:
  FloatDataColumnAccessor() {
  }
  casa::ROArrayColumn<casa::Float> dataCol_;
};

inline bool isEphemeris(casa::String const &name) {
  // Check if given name is included in MDirection types
  casa::Int nall, nextra;
  const casa::uInt *typ;
  auto *types = casa::MDirection::allMyTypes(nall, nextra, typ);
  auto start_extra = nall - nextra;
  auto capital_name = name;
  capital_name.upcase();

  for (auto i = start_extra; i < nall; ++i) {
    if (capital_name == types[i]) {
      return true;
    }
  }

  return false;
}

inline void updateWeight(casa::NewCalTable &ct) {
  casa::CTMainColumns ctmc(ct);

  // simply copy FPARAM
  for (size_t irow = 0; irow < ct.nrow(); ++irow) {
    ctmc.weight().put(irow, real(ctmc.cparam()(irow)));
  }
}

casacore::Double rad2arcsec(casacore::Double value_in_rad) {
  return casacore::Quantity(value_in_rad, "rad").getValue("arcsec");
}
}

namespace casa {
SDDoubleCircleGainCal::SDDoubleCircleGainCal(VisSet &vs) :
    VisCal(vs),             // virtual base
    VisMueller(vs),         // virtual base
    GJones(vs), central_disk_size_(0.0), smooth_(True) {
}

SDDoubleCircleGainCal::SDDoubleCircleGainCal(const MSMetaInfoForCal& msmc) :
    VisCal(msmc),             // virtual base
    VisMueller(msmc),         // virtual base
    GJones(msmc), central_disk_size_(0.0), smooth_(True) {
}

SDDoubleCircleGainCal::~SDDoubleCircleGainCal() {
}

void SDDoubleCircleGainCal::setSolve() {
  central_disk_size_ = 0.0;
  smooth_ = True;

  // call parent setSolve
  SolvableVisCal::setSolve();
}

void SDDoubleCircleGainCal::setSolve(const Record &solve) {
  // parameters for double circle gain calibration
  if (solve.isDefined("smooth")) {
    smooth_ = solve.asBool("smooth");
  }
  if (solve.isDefined("radius")) {
    String size_string = solve.asString("radius");
    QuantumHolder qh;
    String error;
    qh.fromString(error, size_string);
    Quantity q = qh.asQuantity();
    central_disk_size_ = q.getValue("rad");
  }

  logSink() << LogIO::DEBUGGING << "smooth=" << smooth_ << LogIO::POST;
  logSink() << LogIO::DEBUGGING << "central disk size=" << central_disk_size_
      << " rad" << "(" << rad2arcsec(central_disk_size_) << " arcsec)"
      << LogIO::POST;

  if (central_disk_size_ < 0.0) {
    logSink() << "Negative central disk size is given" << LogIO::EXCEPTION;
  }

  // call parent setSolve
  SolvableVisCal::setSolve(solve);
}

String SDDoubleCircleGainCal::solveinfo() {
  ostringstream o;
  o << typeName()
      << ": " << calTableName()
      << " smooth=" << (smooth_?"True":"False") << endl
      << " radius=" << central_disk_size_;
  if (central_disk_size_ == 0.0) {
    o << " (half of primary beam will be used)";
  }
  o << endl;
  return String(o);
}

void SDDoubleCircleGainCal::keepNCT() {
  debuglog << "SDDoubleCircleGainCal::keepNCT" << debugpost;
  // Call parent to do general stuff
  GJones::keepNCT();

  if (prtlev()>4)
    cout << " SVJ::keepNCT" << endl;

  // Set proper antenna id
  Vector<Int> a1(nAnt());
  a1 = currAnt_;
  //indgen(a1);

  debuglog << "antenna is " << a1 << debugpost;

  // We are adding to the most-recently added rows
  RefRows rows(ct_->nrow()-nElem(),ct_->nrow()-1,1);

  // Write to table
  CTMainColumns ncmc(*ct_);
  ncmc.antenna1().putColumnCells(rows,a1);
}

void SDDoubleCircleGainCal::selfGatherAndSolve(VisSet& vs,
    VisEquation& /* ve */) {
  SDDoubleCircleGainCalImpl sdcalib;
  debuglog<< "SDDoubleCircleGainCal::selfGatherAndSolve()" << debugpost;

  // TODO: implement pre-application of single dish caltables

  debuglog<< "nspw = " << nSpw() << debugpost;
  fillNChanParList(msName(), nChanParList());
  debuglog<< "nChanParList=" << nChanParList() << debugpost;

  // Create a new caltable to fill up
  createMemCalTable();

  // Setup shape of solveAllRPar
  nElem() = 1;
  initSolvePar();

  // Pick up OFF spectra using STATE_ID
  auto const msSel = vs.iter().ms();
  debuglog<< "configure data selection for specific calibration mode" << debugpost;
  auto const taql = selectOnSourceAutoCorr(msSel);
  debuglog<< "taql = \"" << taql << "\"" << debugpost;
  MeasurementSet msOnSource(tableCommand(taql, msSel));
  logSink() << LogIO::DEBUGGING << "msSel.nrow()=" << msSel.nrow()
      << " msOnSource.nrow()=" << msOnSource.nrow() << LogIO::POST;
  if (msOnSource.nrow() == 0) {
    throw AipsError("No reference integration in the data.");
  }
  String dataColName =
      (msOnSource.tableDesc().isColumn("FLOAT_DATA")) ? "FLOAT_DATA" : "DATA";
  logSink() << LogIO::DEBUGGING << "dataColName = " << dataColName << LogIO::POST;

  if (msOnSource.tableDesc().isColumn("FLOAT_DATA")) {
    executeDoubleCircleGainCal<FloatDataColumnAccessor>(msOnSource);
  } else {
    executeDoubleCircleGainCal<DataColumnAccessor>(msOnSource);
  }

  //assignCTScanField(*ct_, msName());

  // update weight
  updateWeight(*ct_);

  // store caltable
  storeNCT();
}

template<class Accessor>
void SDDoubleCircleGainCal::executeDoubleCircleGainCal(
    MeasurementSet const &ms) {
  logSink() << LogOrigin("SDDoubleCircleGainCal", __FUNCTION__, WHERE);
  // setup worker class
  SDDoubleCircleGainCalImpl worker;

  Int smoothingSize = -1;// use default smoothing size
  worker.setCentralRegion(central_disk_size_);
  if (smooth_) {
    worker.setSmoothing(smoothingSize);
  } else {
    worker.unsetSmoothing();
  }

  ROArrayColumn<Double> uvwColumn(ms, "UVW");
  Matrix<Double> uvw = uvwColumn.getColumn();
  debuglog<< "uvw.shape " << uvw.shape() << debugpost;

  // make a map between SOURCE_ID and source NAME
  auto const &sourceTable = ms.source();
  ROScalarColumn<Int> idCol(sourceTable,
      sourceTable.columnName(MSSource::MSSourceEnums::SOURCE_ID));
  ROScalarColumn<String> nameCol(sourceTable,
      sourceTable.columnName(MSSource::MSSourceEnums::NAME));
  std::map<Int, String> sourceMap;
  for (uInt irow = 0; irow < sourceTable.nrow(); ++irow) {
    auto sourceId = idCol(irow);
    if (sourceMap.find(sourceId) == sourceMap.end()) {
      sourceMap[sourceId] = nameCol(irow);
    }
  }

  // make a map between FIELD_ID and SOURCE_ID
  auto const &fieldTable = ms.field();
  idCol.attach(fieldTable,
      fieldTable.columnName(MSField::MSFieldEnums::SOURCE_ID));
  ROArrayMeasColumn<MDirection> dirCol(fieldTable, "REFERENCE_DIR");
  std::map<Int, Int> fieldMap;
  for (uInt irow = 0; irow < fieldTable.nrow(); ++irow) {
    auto sourceId = idCol(irow);
    fieldMap[static_cast<Int>(irow)] = sourceId;
  }

  // access to subtable columns
  ROScalarQuantColumn<Double> antennaDiameterColumn(ms.antenna(),
      "DISH_DIAMETER");
  ROArrayQuantColumn<Double> observingFrequencyColumn(ms.spectralWindow(),
      "CHAN_FREQ");

  // traverse MS
  Int cols[] = {MS::FIELD_ID, MS::ANTENNA1, MS::FEED1, MS::DATA_DESC_ID};
  Int *colsp = cols;
  Block<Int> sortCols(4, colsp, False);
  MSIter msIter(ms, sortCols, 0.0, False, False);
  for (msIter.origin(); msIter.more(); msIter++) {
    MeasurementSet const currentMS = msIter.table();

    uInt nrow = currentMS.nrow();
    debuglog<< "nrow = " << nrow << debugpost;
    if (nrow == 0) {
      debuglog<< "Skip" << debugpost;
      continue;
    }
    Int ispw = msIter.spectralWindowId();
    if (nChanParList()[ispw] == 4) {
      // Skip WVR
      debuglog<< "Skip " << ispw
      << "(nchan " << nChanParList()[ispw] << ")"
      << debugpost;
      continue;
    }
    logSink() << "Process spw " << ispw
        << "(nchan " << nChanParList()[ispw] << ")" << LogIO::POST;

    Int ifield = msIter.fieldId();
    ROScalarColumn<Int> antennaCol(currentMS, "ANTENNA1");
    //currAnt_ = antennaCol(0);
    Int iantenna = antennaCol(0);
    ROScalarColumn<Int> feedCol(currentMS, "FEED1");
    debuglog<< "FIELD_ID " << msIter.fieldId()
    << " ANTENNA1 " << iantenna//currAnt_
    << " FEED1 " << feedCol(0)
    << " DATA_DESC_ID " << msIter.dataDescriptionId()
    << debugpost;

    // setup PointingDirectionCalculator
    PointingDirectionCalculator pcalc(currentMS);
    pcalc.setDirectionColumn("DIRECTION");
    pcalc.setFrame("J2000");
    pcalc.setDirectionListMatrixShape(PointingDirectionCalculator::ROW_MAJOR);
    debuglog<< "SOURCE_ID " << fieldMap[ifield] << " SOURCE_NAME " << sourceMap[fieldMap[ifield]] << debugpost;
    auto const isEphem = ::isEphemeris(sourceMap[fieldMap[ifield]]);
    Matrix<Double> offset_direction;
    if (isEphem) {
      pcalc.setMovingSource(sourceMap[fieldMap[ifield]]);
      offset_direction = pcalc.getDirection();
    } else {
      pcalc.unsetMovingSource();
      Matrix<Double> direction = pcalc.getDirection();

      // absolute coordinate -> offset from center
      OrthographicProjector projector(1.0f);
      projector.setDirection(direction);
      Vector<MDirection> md = dirCol.convert(ifield, MDirection::J2000);
      //logSink() << "md.shape() = " << md.shape() << LogIO::POST;
      auto const qd = md[0].getAngle("rad");
      auto const d = qd.getValue();
      auto const lat = d[0];
      auto const lon = d[1];
      logSink() << "reference coordinate: lat = " << lat << " lon = " << lon << LogIO::POST;
      projector.setReferencePixel(0.0, 0.0);
      projector.setReferenceCoordinate(lat, lon);
      offset_direction = projector.project();
      auto const pixel_size = projector.pixel_size();
      // convert offset_direction from pixel to radian
      offset_direction *= pixel_size;
    }
//    debuglog<< "offset_direction = " << offset_direction << debugpost;
//    Double const *direction_p = offset_direction.data();
//    for (size_t i = 0; i < 10; ++i) {
//      debuglog<< "offset_direction[" << i << "]=" << direction_p[i] << debugpost;
//    }

    ROScalarColumn<Double> timeCol(currentMS, "TIME");
    Vector<Double> time = timeCol.getColumn();
    Accessor dataCol(currentMS);
    Cube<Float> data = dataCol.getColumn();
    ROArrayColumn<Bool> flagCol(currentMS, "FLAG");
    Cube<Bool> flag = flagCol.getColumn();
//    debuglog<< "data = " << data << debugpost;

    Vector<Double> gainTime;
    Cube<Float> gain;
    Cube<Bool> gain_flag;

    // tell some basic information to worker object
    Quantity antennaDiameterQuant = antennaDiameterColumn(iantenna);
    worker.setAntennaDiameter(antennaDiameterQuant.getValue("m"));
    debuglog<< "antenna diameter = " << worker.getAntennaDiameter() << "m" << debugpost;
    Vector<Quantity> observingFrequencyQuant = observingFrequencyColumn(ispw);
    Double meanFrequency = 0.0;
    auto numChan = observingFrequencyQuant.nelements();
    debuglog<< "numChan = " << numChan << debugpost;
    assert(numChan > 0);
    if (numChan % 2 == 0) {
      meanFrequency = (observingFrequencyQuant[numChan / 2 - 1].getValue("Hz")
          + observingFrequencyQuant[numChan / 2].getValue("Hz")) / 2.0;
    } else {
      meanFrequency = observingFrequencyQuant[numChan / 2].getValue("Hz");
    }
    //debuglog << "mean frequency " << meanFrequency.getValue() << " [" << meanFrequency.getFullUnit() << "]" << debugpost;
    debuglog<< "mean frequency " << meanFrequency << debugpost;
    worker.setObservingFrequency(meanFrequency);
    debuglog<< "observing frequency = " << worker.getObservingFrequency() / 1e9 << "GHz" << debugpost;
    Double primaryBeamSize = worker.getPrimaryBeamSize();
    debuglog<< "primary beam size = " << rad2arcsec(primaryBeamSize) << " arcsec" << debugpost;

    auto const effective_radius = worker.getRadius();
    logSink() << "effective radius of the central region = " << effective_radius << " arcsec"
        << " (" << rad2arcsec(effective_radius) << " arcsec)"<< LogIO::POST;
    if (worker.isSmoothingActive()) {
      auto const effective_smoothing_size = worker.getEffectiveSmoothingSize();
      logSink() << "smoothing activated. effective size = " << effective_smoothing_size << LogIO::POST;
    }
    else {
      logSink() << "smoothing deactivated." << LogIO::POST;
    }

    // execute calibration
    worker.calibrate(data, flag, time, offset_direction, gainTime, gain, gain_flag);
    //debuglog<< "gain_time = " << gain_time << debugpost;
    //debuglog<<"gain = " << gain << debugpost;
    size_t numGain = gainTime.nelements();
    debuglog<< "number of gain " << numGain << debugpost;

    currSpw() = ispw;
    currField() = ifield;
    currAnt_ = iantenna;

    solveAllParErr() = 0.1; // TODO
    solveAllParSNR() = 1.0; // TODO

    size_t numCorr = gain.shape()[0];
    Slice corrSlice(0, numCorr);
    Slice chanSlice(0, numChan);
    for (size_t i = 0; i < numGain; ++i) {
      refTime() = gainTime[i];
      //solveAllCPar() = gain(corrSlice, chanSlice, Slice(i, 1));
      convertArray(solveAllCPar(), gain(corrSlice, chanSlice, Slice(i, 1)));
      solveAllParOK() = !gain_flag(corrSlice, chanSlice, Slice(i, 1));

      keepNCT();
    }

  }
}
}
