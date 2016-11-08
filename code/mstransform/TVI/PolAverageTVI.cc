//# PolAverageTVI.h: This file contains the implementation of the PolAverageTVI class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $
#include <mstransform/TVI/PolAverageTVI.h>

#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Containers/Record.h>
#include <casacore/casa/Exceptions/Error.h>
#include <casacore/casa/Arrays/ArrayIter.h>
#include <casacore/measures/Measures/Stokes.h>

#include <msvis/MSVis/VisBufferComponents2.h>

using namespace casacore;

namespace {
template<class T>
inline T replaceFlaggedDataWithZero(T v, Bool b) {
  return ((b) ? T(0) : v);
}

struct StokesTransformation {
public:
  template<class T>
  inline static void transformData(Cube<T> const &dataIn,
      Cube<Bool> const &flagIn, Cube<T> &dataOut) {
    if (dataIn.empty()) {
      dataOut.resize();
      return;
    }
    auto const cubeShape = dataIn.shape();
    IPosition const newShape(3, 1, cubeShape[1], cubeShape[2]);
    size_t const npol = cubeShape[0];
    size_t const nchan = cubeShape[1];
    size_t const nrow = cubeShape[2];
    size_t const nelem = cubeShape[1] * cubeShape[2];
    Cube<T> transformedData(newShape, T(0));
    Cube<Float> weightSum(newShape, 0.0f);

    for (size_t i = 0; i < npol; ++i) {
      IPosition start(3, i, 0, 0);
      IPosition end(3, i, nchan - 1, nrow - 1);
      auto dslice = dataIn(start, end);
      auto fslice = flagIn(start, end);
      Array<Float> weight(dslice.shape());
      Array<T> weightedData(dslice.shape());
      arrayContTransform(dslice, fslice, weightedData, replaceFlaggedDataWithZero<T>);
      arrayContTransform(fslice, weight, [](Bool b) {
        return ((b) ? 0.0f : 1.0f);
      });
      transformedData += weightedData;
      weightSum += weight;
    }

    // transformedData, transformedFlag and nAccumulated should be contiguous array
    auto p_tdata = transformedData.data();
    auto p_wsum = weightSum.data();

    for (size_t i = 0; i < nelem; ++i) {
      if (p_wsum[i] > 0.0) {
        p_tdata[i] /= T(p_wsum[i]);
      }
    }

    dataOut.reference(transformedData);
  }

  static inline void AccumulateWeight(Float const wt, Double &wtsum) {
    wtsum += 1.0 / wt;
  }

  static inline void NormalizeWeight(Double const wtsum, Float &wt) {
    wt = 4.0 / wtsum;
  }
};

struct GeometricTransformation {
  template<class T>
  inline static void transformData(Cube<T> const &dataIn,
      Cube<Bool> const &flagIn, Cube<Float> const &weightIn, Cube<T> &dataOut) {
    if (dataIn.empty()) {
      dataOut.resize();
      return;
    }
    auto const cubeShape = dataIn.shape();
    IPosition const newShape(3, 1, cubeShape[1], cubeShape[2]);
    Cube<T> transformedData(newShape, T(0));
    Cube<Bool> transformedFlag(newShape, True);
    Cube<Float> weightSum(newShape, 0.0f);
    size_t const npol = cubeShape[0];
    size_t const nchan = cubeShape[1];
    size_t const nrow = cubeShape[2];
    size_t const nelem = cubeShape[1] * cubeShape[2];


    for (size_t i = 0; i < npol; ++i) {
      IPosition start(3, i, 0, 0);
      IPosition end(3, i, nchan - 1, nrow - 1);
      auto const dslice = dataIn(start, end);
      auto const wslice = weightIn(start, end);
      auto const fslice = flagIn(start, end);
      Array<Float> weight(dslice.shape());
      Array<T> weightedData(dslice.shape());
      arrayContTransform(dslice * wslice, fslice, weightedData, replaceFlaggedDataWithZero<T>);
      arrayContTransform(wslice, fslice, weight, replaceFlaggedDataWithZero<Float>);
      transformedData += weightedData;
      weightSum += weight;
    }

    // transformedData, transformedFlag and nAccumulated should be contiguous array
    T *p_tdata = transformedData.data();
    Float *p_wsum = weightSum.data();
    for (size_t i = 0; i < nelem; ++i) {
      if (p_wsum[i] > 0.0) {
        p_tdata[i] /= T(p_wsum[i]);
      }
    }

    dataOut.reference(transformedData);
  }

  template<class T>
  inline static void transformData(Cube<T> const &dataIn,
      Cube<Bool> const &flagIn, Matrix<Float> const &weightIn,
      Cube<T> &dataOut) {
    cout << "start " << __func__ << endl;
    if (dataIn.empty()) {
      dataOut.resize();
      return;
    }
    auto const cubeShape = dataIn.shape();
    IPosition const newShape(3, 1, cubeShape[1], cubeShape[2]);
    Cube<T> transformedData(newShape, T(0));
    Cube<Float> weightSum(newShape, 0.0f);
    auto const npol = dataIn.shape()[0];
    auto const nchan = dataIn.shape()[1];
    auto const nrow = dataIn.shape()[2];
    auto const nelem = nchan * nrow;

    for (ssize_t i = 0; i < npol; ++i) {
      for (ssize_t j = 0; j < nrow; ++j) {
        IPosition start(3, i, 0, j);
        IPosition end(3, i, nchan - 1, j);
        auto dslice = dataIn(start, end);
        auto fslice = flagIn(start, end);
        auto w = weightIn(i, j);
        Array<Float> weight(dslice.shape());
        Array<T> weightedData(dslice.shape());
        arrayContTransform(dslice * w, fslice, weightedData, replaceFlaggedDataWithZero<T>);
        arrayContTransform(fslice, weight, [&w](Bool b) {
          return ((b) ? 0.0f: w);
        });
        transformedData += weightedData;
        weightSum += weight;
      }
    }

    // transformedData, transformedFlag and nAccumulated should be contiguous array
    T *p_tdata = transformedData.data();
    Float *p_wsum = weightSum.data();

    for (ssize_t i = 0; i < nelem; ++i) {
      if (p_wsum[i] > 0.0) {
        p_tdata[i] /= T(p_wsum[i]);
      }
    }

    dataOut.reference(transformedData);
    cout << "end " << __func__ << endl;
  }

  static inline void AccumulateWeight(Float const wt, Double &wtsum) {
    wtsum += wt;
  }

  static inline void NormalizeWeight(Double const wtsum, Float &wt) {
    wt = wtsum;
  }
};

inline Float weight2Sigma(Float x) {
  return 1.0 / sqrt(x);
}

template<class WeightHandler>
inline void transformWeight(Array<Float> const &weightIn,
    Array<Float> &weightOut) {
  cout << "start " << __func__ << endl;
  if (weightIn.empty()) {
    cout << "input weight is empty" << endl;
    weightOut.resize();
    return;
  }
  IPosition const shapeIn = weightIn.shape();
  IPosition shapeOut(shapeIn);
  // set length of polarization axis to 1
  shapeOut[0] = 1;
  cout << "shapeIn = " << shapeIn << " shapeOut = " << shapeOut << endl;

  // initialization
  weightOut.resize(shapeOut);
  weightOut = 0.0f;

  ssize_t numPol = shapeIn[0];
  Int64 numElemPerPol = shapeOut.product();
  cout << "numElemPerPol = " << numElemPerPol << endl;
  cout << "numPol = " << numPol << endl;

  Bool b;
  Float const *p_wIn = weightIn.getStorage(b);
  Float *p_wOut = weightOut.data();

  for (Int64 i = 0; i < numElemPerPol; ++i) {
    ssize_t offsetIndex = i * numPol;
    Double sum = 0.0;
    for (ssize_t j = 0; j < numPol; ++j) {
      //sum += p_wOut[offsetIndex + j];
      WeightHandler::AccumulateWeight(p_wIn[offsetIndex + j], sum);
    }
    //p_wOut[i] = sum;
    WeightHandler::NormalizeWeight(sum, p_wOut[i]);
  }

  weightIn.freeStorage(p_wIn, b);
}
} // anonymous namespace

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi { //# NAMESPACE VI - BEGIN
//////////
// Base Class
// PolAverageTVI
/////////
PolAverageTVI::PolAverageTVI(ViImplementation2 *inputVII) :
    TransformingVi2(inputVII) {
}

PolAverageTVI::~PolAverageTVI() {
}

void PolAverageTVI::origin() {
  TransformingVi2::origin();

  // Synchronize own VisBuffer
  configureNewSubchunk();
}

void PolAverageTVI::next() {
  TransformingVi2::next();

  // Synchronize own VisBuffer
  configureNewSubchunk();
}

void PolAverageTVI::corrType(Vector<Int> & corrTypes) const {
  // Always return (Stokes::I)
  Vector<Int> myCorrTypes(1, (Int) Stokes::I);
  corrTypes.reference(myCorrTypes);
}

void PolAverageTVI::flag(Cube<Bool> & flags) const {
  auto const vb = getVii()->getVisBuffer();
  Cube<Bool> originalFlags = vb->flagCube();
  auto const cubeShape = originalFlags.shape();
  IPosition const newShape(3, 1, cubeShape[1], cubeShape[2]);
  Cube<Bool> transformedFlags(newShape, True);
  ArrayIterator<Bool> iter(originalFlags, IPosition(1, 0), False);
  auto transformedFlagsSlice = transformedFlags.yzPlane(0);
  while (!iter.pastEnd()) {
    transformedFlagsSlice &= iter.array();
    iter.next();
  }
  flags.reference(transformedFlags);
}

void PolAverageTVI::flag(Matrix<Bool> & flags) const {
  Cube<Bool> transformedFlags;
  flag(transformedFlags);

  flags.reference(transformedFlags.yzPlane(0));
}

void PolAverageTVI::jonesC(
    Vector<SquareMatrix<Complex, 2> > & /*cjones*/) const {
  // TODO
  throw AipsError("PolAverageTVI::jonesC should not be called.");
}

void PolAverageTVI::sigma(Matrix<Float> & sigmat) const {
  weight(sigmat);
  arrayTransformInPlace(sigmat, ::weight2Sigma);
}

void PolAverageTVI::visibilityCorrected(Cube<Complex> & vis) const {
  if (getVii()->existsColumn(VisBufferComponent2::VisibilityCorrected)) {
    Cube<Complex> dataCube;
    getVii()->visibilityCorrected(dataCube);
    Cube<Bool> flagCube;
    getVii()->flag(flagCube);
    transformComplexData(dataCube, flagCube, vis);
  } else {
    vis.resize();
  }
}

void PolAverageTVI::visibilityModel(Cube<Complex> & vis) const {
  if (getVii()->existsColumn(VisBufferComponent2::VisibilityModel)) {
    Cube<Complex> dataCube;
    getVii()->visibilityModel(dataCube);
    Cube<Bool> flagCube;
    getVii()->flag(flagCube);
    transformComplexData(dataCube, flagCube, vis);
  } else {
    vis.resize();
  }
}

void PolAverageTVI::visibilityObserved(Cube<Complex> & vis) const {
  if (getVii()->existsColumn(VisBufferComponent2::VisibilityObserved)) {
    Cube<Complex> dataCube;
    getVii()->visibilityObserved(dataCube);
    Cube<Bool> flagCube;
    getVii()->flag(flagCube);
    transformComplexData(dataCube, flagCube, vis);
  } else {
    vis.resize();
  }
}

void PolAverageTVI::floatData(casacore::Cube<casacore::Float> & fcube) const {
  if (getVii()->existsColumn(VisBufferComponent2::FloatData)) {
    Cube<Float> dataCube;
    getVii()->floatData(dataCube);
    Cube<Bool> flagCube;
    getVii()->flag(flagCube);
    transformFloatData(dataCube, flagCube, fcube);
  } else {
    fcube.resize();
  }
}

IPosition PolAverageTVI::visibilityShape() const {
  IPosition cubeShape = getVii()->visibilityShape();
  // Length of polarization (Stokes) axis is always 1 after polarizaton averaging
  cubeShape[0] = 1;
  return cubeShape;
}

void PolAverageTVI::weight(Matrix<Float> & wtmat) const {
  // TODO
  Matrix<Float> wtmatOrg;
  getVii()->weight(wtmatOrg);
  transformWeight(wtmatOrg, wtmat);
}

void PolAverageTVI::weightSpectrum(Cube<Float> & wtsp) const {
  // TODO
  if (weightSpectrumExists()) {
    Cube<Float> wtspOrg;
    getVii()->weightSpectrum(wtspOrg);
    transformWeight(wtspOrg, wtsp);
  } else {
    wtsp.resize();
  }
}

void PolAverageTVI::sigmaSpectrum(Cube<Float> & wtsp) const {
  if (sigmaSpectrumExists()) {
    // sigma = (weight)^-1/2
    weightSpectrum(wtsp);
    arrayTransformInPlace(wtsp, ::weight2Sigma);
  } else {
    wtsp.resize();
  }
}

const VisImagingWeight & PolAverageTVI::getImagingWeightGenerator() const {
  // TODO
  throw AipsError(
      "PolAverageTVI::getImagingWeightGenerator should not be called.");
}

Vector<Int> PolAverageTVI::getCorrelations() const {
  // Always return (Stokes::I)
  return Vector<Int>(1, Stokes::I);
}

Vector<Stokes::StokesTypes> PolAverageTVI::getCorrelationTypesDefined() const {
  // Always return (Stokes::I)
  return Vector<Stokes::StokesTypes>(1, Stokes::I);
}

Vector<Stokes::StokesTypes> PolAverageTVI::getCorrelationTypesSelected() const {
  // Always return (Stokes::I)
  return Vector<Stokes::StokesTypes>(1, Stokes::I);
}

//////////
// GeometricPolAverageTVI
/////////
GeometricPolAverageTVI::GeometricPolAverageTVI(ViImplementation2 *inputVII) :
    PolAverageTVI(inputVII) {
}

GeometricPolAverageTVI::~GeometricPolAverageTVI() {
}

void GeometricPolAverageTVI::transformComplexData(Cube<Complex> const &dataIn,
    Cube<Bool> const &flagIn, Cube<Complex> &dataOut) const {
  transformData(dataIn, flagIn, dataOut);
}

void GeometricPolAverageTVI::transformFloatData(Cube<Float> const &dataIn,
    Cube<Bool> const &flagIn, Cube<Float> &dataOut) const {
  transformData(dataIn, flagIn, dataOut);
}

void GeometricPolAverageTVI::transformWeight(Array<Float> const &weightIn,
    Array<Float> &weightOut) const {
  ::transformWeight<GeometricTransformation>(weightIn, weightOut);
}

template<class T>
void GeometricPolAverageTVI::transformData(Cube<T> const &dataIn,
    Cube<Bool> const &flagIn, Cube<T> &dataOut) const {
  if (weightSpectrumExists()) {
    Cube<Float> weightSp;
    getVii()->weightSpectrum(weightSp);
    ::GeometricTransformation::transformData<T>(dataIn, flagIn, weightSp,
        dataOut);
  } else {
    Matrix<Float> weightMat;
    getVii()->weight(weightMat);
    ::GeometricTransformation::transformData<T>(dataIn, flagIn, weightMat,
        dataOut);
  }
}

//////////
// StokesPolAverageTVI
/////////
StokesPolAverageTVI::StokesPolAverageTVI(ViImplementation2 *inputVII) :
    PolAverageTVI(inputVII) {
}

StokesPolAverageTVI::~StokesPolAverageTVI() {
}

void StokesPolAverageTVI::transformComplexData(Cube<Complex> const &dataIn,
    Cube<Bool> const &flagIn, Cube<Complex> &dataOut) const {
  transformData(dataIn, flagIn, dataOut);
}

void StokesPolAverageTVI::transformFloatData(Cube<Float> const &dataIn,
    Cube<Bool> const &flagIn, Cube<Float> &dataOut) const {
  transformData(dataIn, flagIn, dataOut);
}

void StokesPolAverageTVI::transformWeight(Array<Float> const &weightIn,
    Array<Float> &weightOut) const {
  ::transformWeight<StokesTransformation>(weightIn, weightOut);
}

template<class T>
void StokesPolAverageTVI::transformData(Cube<T> const &dataIn,
    Cube<Bool> const &flagIn, Cube<T> &dataOut) const {
  ::StokesTransformation::transformData<T>(dataIn, flagIn, dataOut);
}

//////////
// PolAverageTVIFactory
/////////
PolAverageVi2Factory::PolAverageVi2Factory(
casacore::Record const &configuration, ViImplementation2 *inputVII) :
    inputVII_p(inputVII), mode_(AveragingMode::DEFAULT) {
  inputVII_p = inputVII;

  if (configuration.isDefined("mode")) {
    String mode = configuration.asString("mode");
    mode.downcase();
    if (mode == "geometric") {
      mode_ = AveragingMode::GEOMETRIC;
    } else if (mode == "stokes") {
      mode_ = AveragingMode::STOKES;
    } else if (mode == "default") {
      mode_ = AveragingMode::DEFAULT;
    } else {
      mode_ = AveragingMode::NUM_MODES;
    }
  }
}

PolAverageVi2Factory::~PolAverageVi2Factory() {
}

ViImplementation2 * PolAverageVi2Factory::createVi() const {
  if (mode_ == AveragingMode::GEOMETRIC) {
    return new GeometricPolAverageTVI(inputVII_p);
  } else if (mode_ == AveragingMode::STOKES) {
    return new StokesPolAverageTVI(inputVII_p);
  }

  throw AipsError("Invalid Averaging Mode for PolAverageTVI.");

  return nullptr;
}

} // # NAMESPACE VI - END
} // #NAMESPACE CASA - END
