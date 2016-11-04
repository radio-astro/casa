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
    size_t const nelem = cubeShape[1] * cubeShape[2];
    Cube<T> transformedData(newShape, T(0));
    Cube<Float> weightSum(newShape, 0.0f);
    // transformedData, transformedFlag and nAccumulated should be contiguous array
    auto p_tdata = transformedData.data();
    auto p_wsum = weightSum.data();
    Bool b0, b1;
    T const *p_data = dataIn.getStorage(b0);
    Bool const *p_flag = flagIn.getStorage(b1);

    for (size_t ipol = 0; ipol < npol; ++ipol) {
      for (size_t i = 0; i < nelem; ++i) {
        auto offsetIndex = nelem * ipol + i;
        if (!p_flag[offsetIndex]) {
          p_tdata[i] += p_data[offsetIndex];
          p_wsum[i] += 1.0f;
        }
      }
    }

    for (size_t i = 0; i < nelem; ++i) {
      if (p_wsum[i] > 0.0) {
        p_tdata[i] /= T(p_wsum[i]);
      }
    }

    dataIn.freeStorage(p_data, b0);
    flagIn.freeStorage(p_flag, b1);

    dataOut.reference(transformedData);
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
    size_t const nelem = cubeShape[1] * cubeShape[2];
    // transformedData, transformedFlag and nAccumulated should be contiguous array
    T *p_tdata = transformedData.data();
    Float *p_wsum = weightSum.data();
    Bool b0, b1, b2;
    T const *p_data = dataIn.getStorage(b0);
    Bool const *p_flag = flagIn.getStorage(b1);
    Float const *p_weight = weightIn.getStorage(b2);

    for (size_t ipol = 0; ipol < npol; ++ipol) {
      for (size_t i = 0; i < nelem; ++i) {
        auto offsetIndex = nelem * ipol + i;
        if (!p_flag[offsetIndex]) {
          p_tdata[i] += p_data[offsetIndex] * p_weight[offsetIndex];
          p_wsum[i] += p_weight[offsetIndex];
        }
      }
    }

    for (size_t i = 0; i < nelem; ++i) {
      if (p_wsum[i] > 0.0) {
        p_tdata[i] /= T(p_wsum[i]);
      }
    }

    dataIn.freeStorage(p_data, b0);
    flagIn.freeStorage(p_flag, b1);
    weightIn.freeStorage(p_weight, b2);

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
    // transformedData, transformedFlag and nAccumulated should be contiguous array
    T *p_tdata = transformedData.data();
    Float *p_wsum = weightSum.data();
    Bool b0, b1, b2;
    T const *p_data = dataIn.getStorage(b0);
    Bool const *p_flag = flagIn.getStorage(b1);
    Float const *p_weight = weightIn.getStorage(b2);

    cout << "weightIn.shape() = " << weightIn.shape() << endl;
    cout << "dataIn.shape() = " << cubeShape << endl;

    for (ssize_t ipol = 0; ipol < npol; ++ipol) {
      for (ssize_t ichan = 0; ichan < nchan; ++ichan) {
        for (ssize_t irow = 0; irow < nrow; ++irow) {
          auto wIndex = ipol * nrow + irow;
          auto dIndex = ipol * (nrow * nchan) + ichan * nrow + irow;
          auto tIndex = ichan * nrow + irow;
          if (!p_flag[dIndex]) {
            p_tdata[tIndex] += p_data[dIndex] * p_weight[wIndex];
            p_wsum[tIndex] += p_weight[wIndex];
          }
        }
      }
    }

    for (ssize_t i = 0; i < nelem; ++i) {
      if (p_wsum[i] > 0.0) {
        p_tdata[i] /= T(p_wsum[i]);
      }
    }

    dataIn.freeStorage(p_data, b0);
    flagIn.freeStorage(p_flag, b1);
    weightIn.freeStorage(p_weight, b2);

    dataOut.reference(transformedData);
    cout << "end " << __func__ << endl;
  }
};

inline Float weight2Sigma(Float x) {
  return 1.0 / sqrt(x);
}
}

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

void PolAverageTVI::jonesC(Vector<SquareMatrix<Complex, 2> > & cjones) const {
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
}

void PolAverageTVI::weightSpectrum(Cube<Float> & wtsp) const {
  // TODO
}

void PolAverageTVI::sigmaSpectrum(Cube<Float> & wtsp) const {
  // sigma = (weight)^-1/2
  weightSpectrum(wtsp);
  arrayTransformInPlace(wtsp, ::weight2Sigma);
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

void GeometricPolAverageTVI::transformWeight(Cube<Float> const &weightIn,
    Cube<Float> &weightOut) const {

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
    cout << "scalar weight" << endl;
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

void StokesPolAverageTVI::transformWeight(Cube<Float> const &weightIn,
    Cube<Float> &weightOut) const {
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
