//# SingleDishSkyCal.cc: implements SingleDishSkyCal
//# Copyright (C) 2003
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

//# Includes
#include <iostream>
#include <sstream>
#include <iomanip>

#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/MaskArrMath.h>
#include <tables/TaQL/TableParse.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSState.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSIter.h>
#include <synthesis/MeasurementComponents/SingleDishSkyCal.h>
#include <synthesis/CalTables/CTGlobals.h>
#include <synthesis/CalTables/CTMainColumns.h>
#include <synthesis/CalTables/CTInterface.h>
#include <ms/MSSel/MSSelection.h>
#include <ms/MSSel/MSSelectionTools.h>

// Debug Message Handling
// if SDCALSKY_DEBUG is defined, the macro debuglog and
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
//#define SDCALSKY_DEBUG

namespace {
struct NullLogger {};

template<class T>
inline NullLogger &operator<<(NullLogger &logger, T /*value*/) {
  return logger;
}

#ifndef SDCALSKY_DEBUG
NullLogger nulllogger;
#endif
}

#ifdef SDCALSKY_DEBUG
  #define debuglog std::cout
  #define debugpost std::endl
#else
  #define debuglog nulllogger
  #define debugpost 0
#endif

// Local Functions
namespace {
inline casa::Vector<casa::uInt> getOffStateIdList(casa::String const &msName) {
  casa::MeasurementSet ms(msName);
  casa::String taql("SELECT FLAG_ROW FROM $1 WHERE UPPER(OBS_MODE) ~ m/^OBSERVE_TARGET#OFF_SOURCE/");
  casa::Table stateSel = casa::tableCommand(taql, ms.state());
  casa::Vector<casa::uInt> stateIdList = stateSel.rowNumbers();
  debuglog << "stateIdList[" << stateIdList.nelements() << "]=";
  for (size_t i = 0; i < stateIdList.nelements(); ++i) {
    debuglog << stateIdList[i] << " ";
  }
  debuglog << debugpost;
  return stateIdList;
}

template<class T>
inline std::string toString(casa::Vector<T> const &v) {
  std::ostringstream oss;
  oss << "[";
  std::string delimiter = "";
  for (size_t i = 0; i < v.nelements(); ++i) {
    oss << delimiter << v[i];
    delimiter = ",";
  }
  oss << "]";
  return oss.str();
}
  
inline casa::String configureTaqlString(casa::String const &msName, casa::Vector<casa::uInt> stateIdList) {
  std::ostringstream oss;
  oss << "SELECT FROM " << msName << " WHERE ANTENNA1 == ANTENNA2 && STATE_ID IN "
      << toString(stateIdList)
      << " ORDER BY FIELD_ID, ANTENNA1, FEED1, DATA_DESC_ID, TIME";
  return casa::String(oss);
}

inline void fillNChanParList(casa::MeasurementSet const &ms, casa::Vector<casa::Int> &nChanParList) {
  casa::MSSpectralWindow const &msspw = ms.spectralWindow();
  casa::ROScalarColumn<casa::Int> nchanCol(msspw, "NUM_CHAN");
  debuglog << "nchanCol=" << toString(nchanCol.getColumn()) << debugpost;
  nChanParList = nchanCol.getColumn()(casa::Slice(0,nChanParList.nelements()));
  debuglog << "nChanParList=" << nChanParList << debugpost;
}

inline void updateWeight(casa::NewCalTable &ct) {
  casa::CTMainColumns ctmc(ct);
  casa::ROArrayColumn<casa::Double> chanWidthCol(ct.spectralWindow(), "CHAN_WIDTH");
  casa::Vector<casa::Int> chanWidth(chanWidthCol.nrow());
  for (size_t irow = 0; irow < chanWidthCol.nrow(); ++irow) {
    casa::Double chanWidthVal = chanWidthCol(irow)(casa::IPosition(1,0));
    chanWidth[irow] = abs(chanWidthVal);
  }
  for (size_t irow = 0; irow < ct.nrow(); ++irow) {
    casa::Int spwid = ctmc.spwId()(irow);
    casa::Double width = chanWidth[spwid];
    casa::Double interval = ctmc.interval()(irow);
    casa::Float weight = width * interval;
    casa::Matrix<casa::Float> weightMat(ctmc.fparam().shape(irow), weight);
    ctmc.weight().put(irow, weightMat);
  }
}

class DataColumnAccessor {
public:
  DataColumnAccessor(casa::Table const &ms,
		     casa::String const colName="DATA")
    : dataCol_(ms, colName) {
  }
  casa::Matrix<casa::Float> operator()(size_t irow) {
    return casa::real(dataCol_(irow));
  }
private:
  DataColumnAccessor() {}
  casa::ROArrayColumn<casa::Complex> dataCol_;
};    

class FloatDataColumnAccessor {
public:
  FloatDataColumnAccessor(casa::Table const &ms)
    : dataCol_(ms, "FLOAT_DATA") {
  }
  casa::Matrix<casa::Float> operator()(size_t irow) {
    return dataCol_(irow);
  }
private:
  FloatDataColumnAccessor() {}
  casa::ROArrayColumn<casa::Float> dataCol_;
};

inline  casa::Int nominalDataDesc(casa::String const &msName, casa::Int const ant)
{
  casa::Int goodDataDesc = -1;
  casa::MeasurementSet ms(msName);
  casa::ROScalarColumn<casa::Int> col(ms.spectralWindow(), "NUM_CHAN");
  casa::Vector<casa::Int> nchanList = col.getColumn();
  size_t numSpw = col.nrow();
  casa::Vector<casa::Int> spwMap(numSpw);
  col.attach(ms.dataDescription(), "SPECTRAL_WINDOW_ID");
  for (size_t i = 0; i < col.nrow(); ++i) {
    spwMap[col(i)] = i;
  }
  casa::ROScalarColumn<casa::String> obsModeCol(ms.state(), "OBS_MODE");
  casa::Regex regex("^OBSERVE_TARGET#ON_SOURCE");
  for (size_t ispw = 0; ispw < numSpw; ++ispw) {
    if (nchanList[ispw] == 4) {
      // this should be WVR
      continue;
    }
    else {
      std::ostringstream oss;
      oss << "SELECT FROM $1 WHERE ANTENNA1 == " << ant
	  << " && ANTENNA2 == " << ant << " && DATA_DESC_ID == " << spwMap[ispw]
	  << " ORDER BY STATE_ID";
      casa::MeasurementSet msSel(casa::tableCommand(oss.str(), ms));
      col.attach(msSel, "STATE_ID");
      casa::Vector<casa::Int> stateIdList = col.getColumn();
      casa::Int previousStateId = -1;
      for (size_t i = 0; i < msSel.nrow(); ++i) {
	casa::Int stateId = stateIdList[i];
	if (stateId != previousStateId) {
	  casa::String obsmode = obsModeCol(stateId);
	  if (regex.match(obsmode.c_str(), obsmode.size()) != casa::String::npos) {
	    goodDataDesc = spwMap[ispw];
	    break;
	  }
	}
	previousStateId = stateId;
      }
    }

    if (goodDataDesc >= 0)
      break;
  }
  return goodDataDesc;
}

inline casa::Vector<size_t> detectGap(casa::Vector<casa::Double> timeList)
{
  size_t n = timeList.size();
  casa::Vector<casa::Double> timeInterval = timeList(casa::Slice(1, n-1)) - timeList(casa::Slice(0, n-1));
  casa::Double medianTime = casa::median(timeInterval);
  casa::Double const threshold = medianTime * 5.0;
  casa::Vector<size_t> gapIndexList(casa::IPosition(1, n/2 + 2), new size_t[n/2+2], casa::TAKE_OVER);
  gapIndexList[0] = 0;
  size_t gapIndexCount = 1;
  for (size_t i = 0; i < timeInterval.size(); ++i) {
    if (timeInterval[i] > threshold) {
      gapIndexList[gapIndexCount] = i + 1;
      gapIndexCount++;
    }
  }
  if (gapIndexList[gapIndexCount] != n) {
    gapIndexList[gapIndexCount] = n;
    gapIndexCount++;
  }
  debuglog << "Detected " << gapIndexCount << " gaps." << debugpost;
  casa::Vector<size_t> ret(casa::IPosition(1, gapIndexCount), gapIndexList.data(), casa::COPY);
  debuglog << "gapList=" << toString(ret) << debugpost;
  return ret;
}

struct DefaultRasterEdgeDetector
{
  static size_t N(size_t numData, casa::Float const /*fraction*/, casa::Int const /*num*/)
  {
    debuglog << "DefaultRasterEdgeDetector" << debugpost;
    return max((size_t)1, static_cast<size_t>(sqrt(numData + 1) - 1));
  }
};

struct FixedNumberRasterEdgeDetector
{
  static size_t N(size_t /*numData*/, casa::Float const /*fraction*/, casa::Int const num)
  {
    debuglog << "FixedNumberRasterEdgeDetector" << debugpost;
    if (num < 0) {
      throw casa::AipsError("Negative number of edge points.");
    }
    return (size_t)num;
  }
};

struct FixedFractionRasterEdgeDetector
{
  static casa::Int N(size_t numData, casa::Float const fraction, casa::Int const /*num*/)
  {
    debuglog << "FixedFractionRasterEdgeDetector" << debugpost;
    return max((size_t)1, static_cast<size_t>(numData * fraction));
  }
};

template<class Detector>
inline casa::Vector<casa::Double> detectEdge(casa::Vector<casa::Double> timeList, casa::Double const interval, casa::Float const fraction, casa::Int const num)
{
  // storage for time range for edges (at head and tail)
  // [(start of head edge), (end of head edge),
  //  (start of tail edge), (end of tail edge)]
  casa::Vector<casa::Double> edgeList(4);
  size_t numList = timeList.size();
  size_t numEdge = Detector::N(numList, fraction, num);
  debuglog << "numEdge = " << numEdge << debugpost;
  if (numEdge == 0) {
    throw casa::AipsError("Zero edge points.");
  }
  else if (timeList.size() > numEdge * 2) {
    edgeList[0] = timeList[0] - 0.5 * interval;
    edgeList[1] = timeList[numEdge-1] + 0.5 * interval;
    edgeList[2] = timeList[numList-numEdge] - 0.5 * interval;
    edgeList[3] = timeList[numList-1] + 0.5 * interval;
  }
  else {
    std::ostringstream oss;
    oss << "Too many edge points (" << 2.0 * numEdge << " out of "
        << timeList.size() << " points)";
    throw casa::AipsError(oss.str());
    // edgeList[0] = timeList[0] - 0.5 * interval;
    // edgeList[1] = timeList[numList-1] + 0.5 * interval;
    // edgeList[2] = edgeList[0];
    // edgeList[3] = edgeList[2];
  }
  return edgeList;
}
  
inline casa::Vector<casa::String> detectRaster(casa::String const &msName,
					       casa::Int const ant,
					       casa::Float const fraction,
					       casa::Int const num)
{
  casa::Int dataDesc = nominalDataDesc(msName, ant);
  debuglog << "nominal DATA_DESC_ID=" << dataDesc << debugpost;
  assert(dataDesc >= 0);
  if (dataDesc < 0) {
    return casa::Vector<casa::String>();
  }

  std::ostringstream oss;
  oss << "SELECT FROM " << msName << " WHERE ANTENNA1 == " << ant
      << " && ANTENNA2 == " << ant << " && FEED1 == 0 && FEED2 == 0"
      << " && DATA_DESC_ID == " << dataDesc
      << " ORDER BY TIME";
  debuglog << "detectRaster: taql=" << oss.str() << debugpost;
  casa::MeasurementSet msSel(casa::tableCommand(oss.str()));
  casa::ROScalarColumn<casa::Double> timeCol(msSel, "TIME");
  casa::ROScalarColumn<casa::Double> intervalCol(msSel, "INTERVAL");
  casa::Vector<casa::Double> timeList = timeCol.getColumn();
  casa::Double interval = casa::min(intervalCol.getColumn());
  casa::Vector<size_t> gapList = detectGap(timeList);
  casa::Vector<casa::String> edgeAsTimeRange(gapList.size() * 2);
  typedef casa::Vector<casa::Double> (*DetectorFunc)(casa::Vector<casa::Double>, casa::Double const,  casa::Float const, casa::Int const);
  DetectorFunc detect = NULL;
  if (num > 0) {
    detect = detectEdge<FixedNumberRasterEdgeDetector>;
  }
  else if (fraction > 0.0) {
    detect = detectEdge<FixedFractionRasterEdgeDetector>;
  }
  else {
    detect = detectEdge<DefaultRasterEdgeDetector>;
  }
  for (size_t i = 0; i < gapList.size()-1; ++i) {
    size_t startRow = gapList[i];
    size_t endRow = gapList[i+1];
    size_t len = endRow - startRow;
    debuglog << "startRow=" << startRow << ", endRow=" << endRow << debugpost;
    casa::Vector<casa::Double> oneRow = timeList(casa::Slice(startRow, len));
    casa::Vector<casa::Double> edgeList = detect(oneRow, interval, fraction, num);
    std::ostringstream s;
    s << std::setprecision(16) << "TIME BETWEEN " << edgeList[0] << " AND " << edgeList[1];
    edgeAsTimeRange[2*i] = s.str();
    s.str("");
    s << std::setprecision(16) << "TIME BETWEEN " << edgeList[2] << " AND " << edgeList[3];
    edgeAsTimeRange[2*i+1] = s.str();
    debuglog << "Resulting selection: (" << edgeAsTimeRange[2*i] << ") || ("
	     << edgeAsTimeRange[2*i+1] << ")" << debugpost;
  }
  return edgeAsTimeRange;
}

// Formula for weight scaling factor, WF
// 1. only one OFF spectrum is used (i.e. no interpolation)
//
//     sigma = sqrt(sigma_ON^2 + sigma_OFF^2)
//           = sigma_ON * sqrt(1 + tau_ON / tau_OFF)
//
//     weight = 1 / sigma^2
//            = 1 / sigma_ON^2 * tau_OFF / (tau_ON + tau_OFF)
//            = weight_ON * tau_OFF / (tau_ON + tau_OFF)
//
//     WF = tau_OFF / (tau_ON + tau_OFF)
//
struct SimpleWeightScalingScheme
{
  static casa::Float SimpleScale(casa::Double exposureOn, casa::Double exposureOff)
  {
    return exposureOff / (exposureOn + exposureOff);
  }
};
// 2. two OFF spectrum is used (linear interpolation)
//
//     sigma_OFF = {(t_OFF2 - t_ON)^2 * sigma_OFF1^2
//                    + (t_ON - t_OFF1)^2 * sigma_OFF2^2}
//                  / (t_OFF2 - t_OFF1)^2
//
//     sigma = sqrt(sigma_ON^2 + sigma_OFF^2)
//           = sigma_ON * sqrt(1 + tau_ON / (t_OFF2 - t_OFF1)^2
//                              * {(t_OFF2 - t_ON)^2 / tau_OFF1
//                                  + (t_ON - t_OFF1)^2 / tau_OFF2})
//
//     weight = weight_ON / (1 + tau_ON / (t_OFF2 - t_OFF1)^2
//                            * {(t_OFF2 - t_ON)^2 / tau_OFF1
//                                + (t_ON - t_OFF1)^2 / tau_OFF2})
//
//     WF = 1.0 / (1 + tau_ON / (t_OFF2 - t_OFF1)^2
//                  * {(t_OFF2 - t_ON)^2 / tau_OFF1
//                      + (t_ON - t_OFF1)^2 / tau_OFF2})
//
struct LinearWeightScalingScheme : public SimpleWeightScalingScheme
{
  static casa::Float InterpolateScale(casa::Double timeOn, casa::Double exposureOn,
                                     casa::Double timeOff1, casa::Double exposureOff1,
                                     casa::Double timeOff2, casa::Double exposureOff2)
  {
    casa::Double dt = timeOff2 - timeOff1;
    casa::Double dt1 = timeOn - timeOff1;
    casa::Double dt2 = timeOff2 - timeOn;
    return 1.0f / (1.0f + exposureOn / (dt * dt)
                   * (dt2 * dt2 / exposureOff1 + dt1 * dt1 / exposureOff2));
  }
};

// 3. two OFF spectrum is used (nearest interpolation)
//
// formulation is same as case 1.
//
struct NearestWeightScalingScheme : public SimpleWeightScalingScheme
{
  static casa::Float InterpolateScale(casa::Double timeOn, casa::Double exposureOn,
                                     casa::Double timeOff1, casa::Double exposureOff1,
                                     casa::Double timeOff2, casa::Double exposureOff2)
  {
    casa::Double dt1 = abs(timeOn - timeOff1);
    casa::Double dt2 = abs(timeOff2 - timeOn);
    return (dt1 <= dt2) ?
      SimpleScale(exposureOn, exposureOff1)
      : SimpleScale(exposureOn, exposureOff2);
  }
};

}

namespace casa { //# NAMESPACE CASA - BEGIN
//
// SingleDishSkyCal
//
  
// Constructor
SingleDishSkyCal::SingleDishSkyCal(VisSet& vs)
  : VisCal(vs),
    SolvableVisCal(vs),
    currAnt_(-1),
    engineC_(vs.numberSpw(), NULL),
    engineF_(vs.numberSpw(), NULL),
    currentSky_(vs.numberSpw(), NULL),
    currentSkyOK_(vs.numberSpw(), NULL)
{
  debuglog << "SingleDishSkyCal::SingleDishSkyCal(VisSet& vs)" << debugpost;
  append() = False;

  initializeSky();
}
  
SingleDishSkyCal::SingleDishSkyCal(const Int& nAnt)
  : VisCal(nAnt),
    SolvableVisCal(nAnt),
    currAnt_(-1),
    engineC_(1, NULL),
    engineF_(1, NULL),
    currentSky_(1, NULL),
    currentSkyOK_(1, NULL)
{
  debuglog << "SingleDishSkyCal::SingleDishSkyCal(const Int& nAnt)" << debugpost;
  append() = False;

  initializeSky();
}

// Destructor
SingleDishSkyCal::~SingleDishSkyCal()
{
  debuglog << "SingleDishSkyCal::~SingleDishSkyCal()" << debugpost;

  finalizeSky();
}

void SingleDishSkyCal::guessPar(VisBuffer& /*vb*/)
{
}

void SingleDishSkyCal::differentiate( CalVisBuffer & /*cvb*/)
{
}

void SingleDishSkyCal::differentiate(VisBuffer& /*vb*/, Cube<Complex>& /*V*/,     
                                     Array<Complex>& /*dV*/, Matrix<Bool>& /*Vflg*/)
{
}

void SingleDishSkyCal::accumulate(SolvableVisCal* /*incr*/,
                                  const Vector<Int>& /*fields*/)
{
}

void SingleDishSkyCal::diffSrc(VisBuffer& /*vb*/, Array<Complex>& /*dV*/)
{
}

void SingleDishSkyCal::fluxscale(const String& /*outfile*/,
                                 const Vector<Int>& /*refFieldIn*/,
                                 const Vector<Int>& /*tranFieldIn*/,
                                 const Vector<Int>& /*inRefSpwMap*/,
                                 const Vector<String>& /*fldNames*/,
                                 const Float& /*inGainThres*/,
                                 const String& /*antSel*/,
                                 const String& /*timerangeSel*/,
                                 const String& /*scanSel*/,
                                 fluxScaleStruct& /*oFluxScaleStruct*/,
                                 const String& /*oListFile*/,
                                 const Bool& /*incremental*/,
                                 const Int& /*fitorder*/,
                                 const Bool& /*display*/)
{
}

void SingleDishSkyCal::listCal(const Vector<Int> /*ufldids*/, const Vector<Int> /*uantids*/,
                               const Matrix<Int> /*uchanids*/,
                               //const Int& /*spw*/, const Int& /*chan*/,
                               const String& /*listfile*/, const Int& /*pagerows*/)
{
}

void SingleDishSkyCal::setApply(const Record& apply)
{
  // Override interp
  // default frequency interpolation option is 'linearflag'
  Record applyCopy(apply);
  if (applyCopy.isDefined("interp")) {
    String interp = applyCopy.asString("interp");
    if (!interp.contains(',')) {
      //fInterpType() = "linearflag";
      String newInterp = interp + ",linearflag";
      applyCopy.define("interp", newInterp);
    }
  }
  else {
    applyCopy.define("interp", "linear,linearflag");
  }
  
  // call parent method
  SolvableVisCal::setApply(applyCopy);
}
  
template<class Accessor>
void SingleDishSkyCal::traverseMS(MeasurementSet const &ms) {
  Int cols[] = {MS::FIELD_ID, MS::ANTENNA1, MS::FEED1,
		MS::DATA_DESC_ID};
  Int *colsp = cols;
  Block<Int> sortCols(4, colsp, False);
  MSIter msIter(ms, sortCols, 0.0, False, False);
  for (msIter.origin(); msIter.more(); msIter++) {
    Table const current = msIter.table();
    uInt nrow = current.nrow();
    debuglog << "nrow = " << nrow << debugpost;
    if (nrow == 0) {
      debuglog << "Skip" << debugpost;
      continue;
    }
    Int ispw = msIter.spectralWindowId();
    if (nChanParList()[ispw] == 4) {
      // Skip WVR
      debuglog << "Skip " << ispw
	       << "(nchan " << nChanParList()[ispw] << ")"
	       << debugpost;
      continue;
    }
    debuglog << "Process " << ispw
	     << "(nchan " << nChanParList()[ispw] << ")"
	     << debugpost;
    
    Int ifield = msIter.fieldId();
    ROScalarColumn<Int> antennaCol(current, "ANTENNA1");
    currAnt_ = antennaCol(0);
    ROScalarColumn<Int> feedCol(current, "FEED1");
    debuglog << "FIELD_ID " << msIter.fieldId()
	     << " ANTENNA1 " << currAnt_
	     << " FEED1 " << feedCol(0)
	     << " DATA_DESC_ID " << msIter.dataDescriptionId()
	     << debugpost;
    ROScalarColumn<Double> timeCol(current, "TIME");
    ROScalarColumn<Double> exposureCol(current, "EXPOSURE");
    ROScalarColumn<Double> intervalCol(current, "INTERVAL");
    Accessor dataCol(current);
    ROArrayColumn<Bool> flagCol(current, "FLAG");
    ROScalarColumn<Bool> flagRowCol(current, "FLAG_ROW");
    Vector<Double> timeList = timeCol.getColumn();
    Vector<Double> exposure = exposureCol.getColumn();
    Vector<Double> interval = intervalCol.getColumn();
    Vector<Double> timeInterval(timeList.nelements());
    Slice slice1(0, nrow - 1);
    Slice slice2(1, nrow - 1);
    timeInterval(slice1) = timeList(slice2) - timeList(slice1);
    timeInterval[nrow-1] = DBL_MAX;
    IPosition cellShape = flagCol.shape(0);
    size_t nelem = cellShape.product();
    Matrix<Float> dataSum(cellShape, new Float[nelem], TAKE_OVER);
    Matrix<Float> weightSum(cellShape, new Float[nelem], TAKE_OVER);
    dataSum = 0.0f;
    weightSum = 0.0f;
    Matrix<Bool> resultMask(cellShape, new Bool[nelem], TAKE_OVER);
    resultMask = True;
    Vector<Bool> flagRow = flagRowCol.getColumn();
    Double threshold = 1.1;
    Double timeCentroid = 0.0;
    size_t numSpectra = 0;
    Double effectiveExposure = 0.0;
    for (uInt i = 0; i < nrow; ++i) {
      if (flagRow(i)) {
	continue;
      }

      numSpectra++;
      timeCentroid += timeList[i];
      effectiveExposure += exposure[i];
      
      Matrix<Bool> mask = !flagCol(i);
      MaskedArray<Float> mdata(dataCol(i), mask);
      MaskedArray<Float> weight(Matrix<Float>(mdata.shape(), exposure[i]), mask);
      dataSum += mdata * weight;
      weightSum += weight;

      Double gap = 2.0 * timeInterval[i] /
	(interval[i] + interval[(i < nrow-1)?i+1:i]);
      if (gap > threshold) {
        debuglog << "flush accumulated data at row " << i << debugpost;
	// Here we can safely use data() since internal storeage
	// is contiguous
	Float *data_ = dataSum.data();
	const Float *weight_ = weightSum.data();
	Bool *flag_ = resultMask.data();
	for (size_t j = 0; j < dataSum.nelements(); ++j) {
	  if (weight_[j] == 0.0f) {
	    data_[j] = 0.0;
	    flag_[j] = False;
	  }
	  else {
	    data_[j] /= weight_[j];
	  }
	}

	currSpw() = ispw;
	currField() = ifield;
	timeCentroid /= (Double)numSpectra;
	refTime() = timeCentroid;
	interval_ = effectiveExposure;

	debuglog << "spw " << ispw << ": solveAllRPar.shape=" << solveAllRPar().shape() << " nPar=" << nPar() << " nChanPar=" << nChanPar() << " nElem=" << nElem() << debugpost;
	
	solveAllRPar() = dataSum.addDegenerate(1);
	solveAllParOK() = resultMask.addDegenerate(1);
	solveAllParErr() = 0.1; // TODO: this is tentative
	solveAllParSNR() = 1.0; // TODO: this is tentative

	keepNCT();

	dataSum = 0.0f;
	weightSum = 0.0f;
	resultMask = True;
	timeCentroid = 0.0;
	numSpectra = 0;
	effectiveExposure = 0.0;
      }
    }
  }
}

void SingleDishSkyCal::keepNCT() {
  // Call parent to do general stuff
  //  This adds nElem() rows
  SolvableVisCal::keepNCT();

  // We are adding to the most-recently added rows
  RefRows rows(ct_->nrow()-nElem(),ct_->nrow()-1,1);
  Vector<Int> ant(nElem(), currAnt_);

  // update ANTENNA1 and ANTENNA2 with appropriate value
  CTMainColumns ncmc(*ct_);
  ncmc.antenna1().putColumnCells(rows,ant);
  ncmc.antenna2().putColumnCells(rows,ant);

  // update INTERVAL
  ncmc.interval().putColumnCells(rows,interval_);
}    

void SingleDishSkyCal::initSolvePar()
{
  debuglog << "SingleDishSkyCal::initSolvePar()" << debugpost;
  for (Int ispw=0;ispw<nSpw();++ispw) {
    
    currSpw()=ispw;

    switch(parType()) {
    case VisCalEnum::REAL: {
      solveAllRPar().resize(nPar(),nChanPar(),nElem());
      solveAllRPar()=0.0;
      solveRPar().reference(solveAllRPar());
      break;
    }
    default: {
      throw(AipsError("Internal error(Calibrater Module): Unsupported parameter type "
		      "found in SingleDishSkyCal::initSolvePar()"));
      break;
    }
    }//switch

    solveAllParOK().resize(solveAllRPar().shape());
    solveAllParErr().resize(solveAllRPar().shape());
    solveAllParSNR().resize(solveAllRPar().shape());
    solveAllParOK()=True;
    solveAllParErr()=0.0;
    solveAllParSNR()=0.0;
    solveParOK().reference(solveAllParOK());
    solveParErr().reference(solveAllParErr());
    solveParSNR().reference(solveAllParSNR());
  }
  currSpw()=0;
  currAnt_ = 0;

  interval_.resize(nElem());
}

void SingleDishSkyCal::syncMeta2(const vi::VisBuffer2& vb)
{
  // call method in parent class
  VisCal::syncMeta2(vb);

  // fill interval array with exposure
  interval_.reference(vb.exposure());
  debuglog << "SingleDishSkyCal::syncMeta2 interval_= " << interval_ << debugpost;
}

void SingleDishSkyCal::syncCalMat(const Bool &/*doInv*/)
{
  debuglog << "SingleDishSkyCal::syncCalMat" << debugpost;
  debuglog << "nAnt()=" << nAnt() << ", nElem()=" << nElem() << ", nBln()=" << nBln() << debugpost;
  debuglog << "Spw " << currSpw() << "nchanmat, ncalmat" << nChanMat() << ", " << nCalMat() << debugpost;
  debuglog << "nChanPar = " << nChanPar() << debugpost;
  currentSky().resize(2, nChanMat(), nCalMat());
  currentSky().unique();
  currentSkyOK().resize(currentSky().shape());
  currentSkyOK().unique();
  debuglog << "currentSkyOK.shape()=" << currentSkyOK().shape() << debugpost;

  // sky data from caltable
  debuglog << "currRPar().shape()=" << currRPar().shape() << debugpost;
  if (currRPar().shape().product() > 0) {
    debuglog << "currRPar() = " << currRPar().xzPlane(0) << debugpost;
  }

  convertArray(currentSky(), currRPar());
  currentSkyOK() = currParOK();
  debuglog << "currentTime() = " << setprecision(16) << currTime() << debugpost;
  debuglog << "currentSky() = " << currentSky().xzPlane(0) << debugpost;
  debuglog << "currentSkyOK() = " << currentSkyOK().xzPlane(0) << debugpost;

  // weight calibration
  if (calWt())
    syncWtScale();

  debuglog << "SingleDishSkyCal::syncCalMat DONE" << debugpost;
}
  
void SingleDishSkyCal::syncDiffMat()
{
  debuglog << "SingleDishSkyCal::syncDiffMat()" << debugpost;
}

void SingleDishSkyCal::syncWtScale()
{
  debuglog << "syncWtScale" << debugpost;
  
  // allocate necessary memory 
  currWtScale().resize(currentSky().shape());

  // Calculate the weight scaling
  if (tInterpType() == "nearest") {
    calcWtScale<NearestWeightScalingScheme>();
  }
  else {
    calcWtScale<LinearWeightScalingScheme>();
  }
  
  debuglog << "syncWtScale DONE" << debugpost;
}

template<class ScalingScheme>
void SingleDishSkyCal::calcWtScale()
{
  debuglog << "calcWtScale<ScalingScheme>" << debugpost;
  CTInterface cti(*ct_);
  MSSelection mss;
  mss.setFieldExpr(String::toString(currField()));
  mss.setSpwExpr(String::toString(currSpw()));
  mss.setObservationExpr(String::toString(currObs()));
  for (Int iAnt = 0; iAnt < nAnt(); ++iAnt) {
    mss.setAntennaExpr(String::toString(iAnt) + "&&&");
    TableExprNode ten = mss.toTableExprNode(&cti);
    NewCalTable temp;
    try {
      getSelectedTable(temp, *ct_, ten, "");
    } catch (AipsError x) {
      //logSink() << LogIO::WARN
      //          << "Failed to calculate Weight Scale. Set to 1." << LogIO::POST;
      //currWtScale() = 1.0f;
      continue;
    }
    temp = temp.sort("TIME");
    ROScalarColumn<Double> col(temp, "TIME");
    Vector<Double> timeCol = col.getColumn();
    col.attach(temp, "INTERVAL");
    Vector<Double> intervalCol = col.getColumn();
    size_t nrow = timeCol.size();
    debuglog << "timeCol = " << timeCol << debugpost;
    debuglog << "intervalCol = " << intervalCol << debugpost;
    debuglog << "iAnt " << iAnt << " temp.nrow()=" << temp.nrow() << debugpost;
    if (currTime() < timeCol[0]) {
      debuglog << "Use nearest OFF weight (0)" << debugpost;
      currWtScale().xyPlane(iAnt) = ScalingScheme::SimpleScale(interval_[iAnt], intervalCol[0]);
    }
    else if (currTime() > timeCol[nrow-1]) {
      debuglog << "Use nearest OFF weight (" << nrow-1 << ")" << debugpost;
      currWtScale().xyPlane(iAnt) = ScalingScheme::SimpleScale(interval_[iAnt], intervalCol[nrow-1]);
    }
    else {
      debuglog << "Use interpolated OFF weight" << debugpost;
      for (size_t irow = 0; irow < nrow ; ++irow) {
        if (currTime() == timeCol[irow]) {
          currWtScale().xyPlane(iAnt) = ScalingScheme::SimpleScale(interval_[iAnt], intervalCol[irow]);
          break;
        }
        else if (currTime() < timeCol[irow]) {
          currWtScale().xyPlane(iAnt) = ScalingScheme::InterpolateScale(currTime(), interval_[iAnt],
                                                                       timeCol[irow-1], intervalCol[irow-1],
                                                                       timeCol[irow], intervalCol[irow]);
          break;
        }
      }
    }
  }
  debuglog << "currWtScale() = " << currWtScale().xzPlane(0) << debugpost;

  debuglog << "calcWtScale<ScalingScheme> DONE" << debugpost;
}
  
Float SingleDishSkyCal::calcPowerNorm(Array<Float>& /*amp*/, const Array<Bool>& /*ok*/)
{
  return 0.0f;
}

void SingleDishSkyCal::applyCal(VisBuffer& /*vb*/, Cube<Complex>& /*Vout*/, Bool /*trial*/)
{
  throw AipsError("Single dish calibration doesn't support applyCal. Please use applyCal2");
}

void SingleDishSkyCal::applyCal2(vi::VisBuffer2 &vb, Cube<Complex> &Vout, Cube<Float> &Wout,
                                 Bool trial)
{
  debuglog << "SingleDishSkyCal::applycal2" << debugpost;
  debuglog << "nrow, nchan=" << vb.nRows() << "," << vb.nChannels() << debugpost;
  debuglog << "antenna1: " << vb.antenna1() << debugpost;
  debuglog << "antenna2: " << vb.antenna2() << debugpost;
  debuglog << "spw: " << vb.spectralWindows() << debugpost;

  // References to VB2's contents' _data_
  Vector<Bool> flagRv(vb.flagRow());
  Vector<Int>  a1v(vb.antenna1());
  Vector<Int>  a2v(vb.antenna2());
  Cube<Bool> flagCube(vb.flagCube());
  Cube<Complex> visCube(Vout);
  ArrayIterator<Float> wt(Wout,2);
  Matrix<Float> wtmat;

  // Data info/indices
  Int* dataChan;
  Bool* flagR=&flagRv(0);
  Int* a1=&a1v(0);
  Int* a2=&a2v(0);
  
  // iterate rows
  Int nRow=vb.nRows();
  Int nChanDat=vb.nChannels();
  Vector<Int> dataChanv(vb.getChannelNumbers(0));  // All rows have same chans
  //    cout << currSpw() << " startChan() = " << startChan() << " nChanMat() = " << nChanMat() << " nChanDat="<<nChanDat <<endl;

  // setup calibration engine
  engineC().setNumChannel(nChanDat);
  engineC().setNumPolarization(vb.nCorrelations());

  debuglog << "typesize=" << engineC().typesize() << debugpost;

  // Matrix slice of visCube
  // TODO: storage must be aligned for future use
  Matrix<Complex> visCubeSlice;
  Matrix<Bool> flagCubeSlice;
  
  for (Int row=0; row<nRow; row++,flagR++,a1++,a2++) {
    debuglog << "spw: " << currSpw() << " antenna: " << *a1 << debugpost;
    assert(*a1 == *a2);
    
    // Solution channel registration
    Int solCh0(0);
    dataChan=&dataChanv(0);
      
    // If cal _parameters_ are not freqDep (e.g., a delay)
    //  the startChan() should be the same as the first data channel
    if (freqDepMat() && !freqDepPar())
      startChan()=(*dataChan);

    // Solution and data array registration
    engineC().sync(currentSky()(0,solCh0,*a1), currentSkyOK()(0,solCh0,*a1));
    visCubeSlice.reference(visCube.xyPlane(row));
    flagCubeSlice.reference(flagCube.xyPlane(row));

    if (trial) {
      // only update flag info
      engineC().flag(flagCubeSlice);
    }
    else {
      // apply calibration
      engineC().apply(visCubeSlice, flagCubeSlice);
    }
    
    // If requested, update the weights
    if (!trial && calWt()) {
      wtmat.reference(wt.array());
      updateWt2(wtmat,*a1);
    }
    
    if (!trial)
      wt.next();
    
  }
}

void SingleDishSkyCal::selfGatherAndSolve(VisSet& vs, VisEquation& /*ve*/)
{
  debuglog << "SingleDishSkyCal::selfGatherAndSolve()" << debugpost;

  debuglog << "nspw = " << nSpw() << debugpost;
  fillNChanParList(MeasurementSet(msName()), nChanParList());
  debuglog << "nChanParList=" << ::toString(nChanParList()) << debugpost;

  // Create a new caltable to fill up
  createMemCalTable();

  // Setup shape of solveAllRPar
  nElem() = 1;
  initSolvePar();

  // Pick up OFF spectra using STATE_ID
  debuglog << "configure data selection for specific calibration mode" << debugpost;
  String taql = configureSelection(); 
  debuglog << "taql = \"" << taql << "\"" << debugpost;
  MeasurementSet msSel(tableCommand(taql, vs.iter().ms()));
  debuglog << "msSel.nrow()=" << msSel.nrow() << debugpost;
  if (msSel.nrow() == 0) {
    throw AipsError("No reference integration in the data.");
  }
  String dataColName = (msSel.tableDesc().isColumn("FLOAT_DATA")) ? "FLOAT_DATA" : "DATA";

  if (msSel.tableDesc().isColumn("FLOAT_DATA")) {
    traverseMS<FloatDataColumnAccessor>(msSel);
  }
  else {
    traverseMS<DataColumnAccessor>(msSel);
  }

  assignCTScanField(*ct_, msName());

  // update weight without Tsys
  // formula is chanWidth [Hz] * interval [sec]
  updateWeight(*ct_);

  // store caltable
  storeNCT();
}

void SingleDishSkyCal::initializeSky()
{
  debuglog << "SingleDishSkyCal::initializeSky()" << debugpost;
  for (Int ispw=0;ispw<nSpw(); ispw++) {
    currentSky_[ispw] = new Cube<Complex>();
    currentSkyOK_[ispw] = new Cube<Bool>();
    engineC_[ispw] = new SkyCal<Complex, Complex>();
  }
}

void SingleDishSkyCal::finalizeSky()
{
  for (Int ispw=0;ispw<nSpw(); ispw++) {
    if (currentSky_[ispw]) delete currentSky_[ispw];
    if (currentSkyOK_[ispw]) delete currentSkyOK_[ispw];
    if (engineC_[ispw]) delete engineC_[ispw];
    if (engineF_[ispw]) delete engineF_[ispw];
  }

}

void SingleDishSkyCal::updateWt2(Matrix<Float> &weight, const Int &antenna1)
{
  // apply weight scaling factor
  Matrix<Float> const factor = currWtScale().xyPlane(antenna1);
  debuglog << "factor.shape() = " << factor.shape() << debugpost;
  debuglog << "weight.shape() = " << weight.shape() << debugpost;
  debuglog << "weight = " << weight << debugpost;
  if (weight.shape() == factor.shape()) {
    weight *= factor;
  }
  else if (weight.shape() == IPosition(2,factor.shape()[0],1)) {
    weight *= factor(Slice(0,factor.shape()[0]),Slice(0,1));
  }
  else {
    throw AipsError("Shape mismatch between input weight and weight scaling factor");
  }
}

//
// SingleDishPositionSwitchCal
//
  
// Constructor
SingleDishPositionSwitchCal::SingleDishPositionSwitchCal(VisSet& vs)
  : VisCal(vs),
    SingleDishSkyCal(vs)
{
  debuglog << "SingleDishPositionSwitchCal::SingleDishPositionSwitchCal(VisSet& vs)" << debugpost;
}
  
SingleDishPositionSwitchCal::SingleDishPositionSwitchCal(const Int& nAnt)
  : VisCal(nAnt),
    SingleDishSkyCal(nAnt)
{
  debuglog << "SingleDishPositionSwitchCal::SingleDishPositionSwitchCal(const Int& nAnt)" << debugpost;
}

// Destructor
SingleDishPositionSwitchCal::~SingleDishPositionSwitchCal()
{
  debuglog << "SingleDishPositionSwitchCal::~SingleDishPositionSwitchCal()" << debugpost;
}

String SingleDishPositionSwitchCal::configureSelection()
{
  Vector<uInt> stateIdList = getOffStateIdList(msName());
  std::ostringstream oss;
  oss << "SELECT FROM $1 WHERE ANTENNA1 == ANTENNA2 && STATE_ID IN "
      << ::toString(stateIdList)
      << " ORDER BY FIELD_ID, ANTENNA1, FEED1, DATA_DESC_ID, TIME";
  return String(oss.str());  
}

//
// SingleDishRasterCal
//
  
// Constructor
SingleDishRasterCal::SingleDishRasterCal(VisSet& vs)
  : VisCal(vs),
    SingleDishSkyCal(vs),
    fraction_(0.1),
    numEdge_(-1)
{
  debuglog << "SingleDishRasterCal::SingleDishRasterCal(VisSet& vs)" << debugpost;
}
  
SingleDishRasterCal::SingleDishRasterCal(const Int& nAnt)
  : VisCal(nAnt),
    SingleDishSkyCal(nAnt)
{
  debuglog << "SingleDishRasterCal::SingleDishRasterCal(const Int& nAnt)" << debugpost;
}

// Destructor
SingleDishRasterCal::~SingleDishRasterCal()
{
  debuglog << "SingleDishRasterCal::~SingleDishRasterCal()" << debugpost;
}

void SingleDishRasterCal::setSolve(const Record& solve)
{
  // edge detection parameter for otfraster mode
  if (solve.isDefined("fraction")) {
    fraction_ = solve.asFloat("fraction");
  }
  if (solve.isDefined("numedge")) {
    numEdge_ = solve.asInt("numedge");
  }

  logSink() << "fraction=" << fraction_ << endl
            << "numedge=" << numEdge_ << LogIO::POST;
  
  // call parent setSolve
  SolvableVisCal::setSolve(solve);
}
  
String SingleDishRasterCal::configureSelection()
{
  debuglog << "SingleDishRasterCal::configureSelection" << debugpost;
  const Record specify;
  std::ostringstream oss;
  oss << "SELECT FROM $1 WHERE ";
  String delimiter = "";
  // for (Int iant = 0; iant < nAnt(); ++iant) {
  //   Vector<String> timeRangeList = detectRaster(msName(), iant, fraction_, numEdge_);
  //   debuglog << "timeRangeList=" << ::toString(timeRangeList) << debugpost;
  //   oss << delimiter;
  //   oss << "(ANTENNA1 == " << iant << " && ANTENNA2 == " << iant << " && (";
  //   String separator = "";
  //   for (size_t i = 0; i < timeRangeList.size(); ++i) {
  //     if (timeRangeList[i].size() > 0) { 
  //   	oss << separator << "(" << timeRangeList[i] << ")";
  //   	separator = " || ";
  //     }
  //   }
  //   oss << "))";
  //   debuglog << "oss.str()=" << oss.str() << debugpost;
  //   delimiter = " || ";
  // }
  // use ANTENNA 0 for reference antenna
  Vector<String> timeRangeList = detectRaster(msName(), 0, fraction_, numEdge_);
  debuglog << "timeRangeList=" << ::toString(timeRangeList) << debugpost;
  oss << delimiter;
  oss << "(ANTENNA1 == ANTENNA2 && (";
  String separator = "";
  for (size_t i = 0; i < timeRangeList.size(); ++i) {
    if (timeRangeList[i].size() > 0) { 
      oss << separator << "(" << timeRangeList[i] << ")";
      separator = " || ";
    }
  }
  oss << "))";
  debuglog << "oss.str()=" << oss.str() << debugpost;
  
  oss //<< ")"
      << " ORDER BY FIELD_ID, ANTENNA1, FEED1, DATA_DESC_ID, TIME";
  return String(oss);  
}

//
// SingleDishOtfCal
//
  
// Constructor
SingleDishOtfCal::SingleDishOtfCal(VisSet& vs)
  : VisCal(vs),
    SingleDishSkyCal(vs)
{
  debuglog << "SingleDishOtfCal::SingleDishOtfCal(VisSet& vs)" << debugpost;
}
  
SingleDishOtfCal::SingleDishOtfCal(const Int& nAnt)
  : VisCal(nAnt),
    SingleDishSkyCal(nAnt)
{
  debuglog << "SingleDishOtfCal::SingleDishOtfCal(const Int& nAnt)" << debugpost;
}

// Destructor
SingleDishOtfCal::~SingleDishOtfCal()
{
  debuglog << "SingleDishOtfCal::~SingleDishOtfCal()" << debugpost;
}

String SingleDishOtfCal::configureSelection()
{
  throw AipsError("Single-dish OTF calibration is not implemented yet");
}


} //# NAMESPACE CASA - END

