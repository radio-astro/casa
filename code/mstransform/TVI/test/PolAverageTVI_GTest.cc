//# PolAverageTVI_GTest:   test of polarization averaging TVIs
//# Copyright (C) 1995,1999,2000,2001,2016
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <mstransform/TVI/PolAverageTVI.h>

#include <casacore/casa/aips.h>
#include <casacore/casa/OS/EnvVar.h>
#include <casacore/casa/OS/File.h>
#include <casacore/casa/OS/RegularFile.h>
#include <casacore/casa/OS/SymLink.h>
#include <casacore/casa/OS/Directory.h>
#include <casacore/casa/OS/DirectoryIterator.h>
#include <casacore/casa/Exceptions/Error.h>
#include <casacore/casa/iostream.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/iomanip.h>
#include <casacore/casa/Containers/Record.h>

#include <msvis/MSVis/VisibilityIteratorImpl2.h>

#include <gtest/gtest.h>
#include <memory>
#include <typeinfo>

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

namespace {
string GetCasaDataPath() {
  if (casacore::EnvironmentVariable::isDefined("CASAPATH")) {
    string casapath = casacore::EnvironmentVariable::get("CASAPATH");
    size_t endindex = casapath.find(" ");
    if (endindex != string::npos) {
      string casaroot = casapath.substr(0, endindex);
      cout << "casaroot = " << casaroot << endl;
      return (casaroot + "/data/");
    } else {
      cout << "hit npos" << endl;
      return "/data/";
    }
  } else {
    cout << "CASAPATH is not defined" << endl;
    return "";
  }
}

template<class T>
struct VerboseDeleterForNew {
  void operator()(T *p) {
    cout << "Destructing " << typeid(p).name() << endl;
    delete p;
  }
};

template<class T>
struct Filler {
  static void FillArrayReference(MeasurementSet const &ms,
      String const &columnName, Vector<uInt> const &rowIds, Array<T> &data) {
    cout << "Start " << __func__ << endl;
    ArrayColumn<T> col(ms, columnName);
    RefRows rows(rowIds);
    col.getColumnCells(rows, data);
  }
  static void FillScalarReference(MeasurementSet const &ms,
      String const &columnName, Vector<uInt> const &rowIds, Vector<T> &data) {
    ScalarColumn<T> col(ms, columnName);
    RefRows rows(rowIds);
//    Vector<T> vref(data);
    col.getColumnCells(rows, data);
  }
  static void FillWeightSp(MeasurementSet const &ms, Vector<uInt> const &rowIds,
      Array<Float> &weightSp) {
    // weightSp must be resized to appropriate shape
    if (ms.tableDesc().isColumn("WEIGHT_SPECTRUM")) {
      Filler<Float>::FillArrayReference(ms, "WEIGHT_SPECTRUM", rowIds,
          weightSp);
    } else {
      Array<Float> weight;
      Filler<Float>::FillArrayReference(ms, "WEIGHT", rowIds, weight);
      auto const wshape(weight.shape());
      ASSERT_EQ(wshape.size(), (uInt )2);
      ASSERT_EQ(wshape[0], weightSp.shape()[0]);
      ASSERT_EQ(wshape[1], weightSp.shape()[2]);
      Matrix<Float> wtMat(weight);
      Cube<Float> wtCube(weightSp);
      cout << "wtCube.shape() = " << wtCube.shape() << " weightSp.shape() = "
          << weightSp.shape() << endl;
      size_t nPol = wshape[0];
      size_t nRow = wshape[1];
      cout << "wtMat.shape() = " << wtMat.shape() << " nrow = " << wtMat.nrow()
          << " ncol = " << wtMat.ncolumn() << endl;
      for (size_t i = 0; i < nPol; ++i) {
        for (size_t j = 0; j < nRow; ++j) {
          cout << "wtCube.yzPlane(" << i << ").shape() = "
              << wtCube.yzPlane(i).shape() << endl;
          cout << "wtCube.yzPlane(i).column(j).shape() = "
              << wtCube.yzPlane(i).column(j).shape() << endl;
          cout << "wtMat(i, j) = " << wtMat(i, j) << endl;
          wtCube.yzPlane(i).column(j) = wtMat(i, j);
        }
      }
    }
  }
};

struct ValidatorUtil {
  template<class T>
  static void ValidateArray(Array<T> const &ref, Array<T> const &result) {
    ASSERT_TRUE(ref.conform(result));
    Bool b1, b2;
    auto const p_ref = ref.getStorage(b1);
    auto const p_result = result.getStorage(b2);
    size_t arraySize = ref.size();
    for (size_t i = 0; i < arraySize; ++i) {
      ValidateScalar(p_ref[i], p_result[i]);
    }
  }
private:
  template<class T>
  static void ValidateScalar(T const ref, T const result) {
    EXPECT_EQ(ref, result);
  }

};

template<>
void ValidatorUtil::ValidateScalar<Float>(Float const ref, Float const result) {
  EXPECT_FLOAT_EQ(ref, result);
}

template<>
void ValidatorUtil::ValidateScalar<Complex>(Complex const ref,
    Complex const result) {
  EXPECT_FLOAT_EQ(ref.real(), result.real());
  EXPECT_FLOAT_EQ(ref.imag(), result.imag());
}

template<class CollapserImpl>
struct ValidatorBase {
  template<class T>
  static void CollapseData(Array<T> const &baseData, Array<Bool> const &baseFlag,
      Array<Float> const &baseWeight, Array<T> &result) {
    ASSERT_EQ(baseData.shape(), baseWeight.shape());
    ASSERT_EQ(baseData.ndim(), (uInt )3);
    IPosition const baseShape = baseData.shape();
    IPosition shape(baseShape);
    shape[0] = 1;
    result.resize(shape);
    result = (T) 0;
    size_t nPol = baseShape[0];
    size_t nChan = baseShape[1];
    size_t nRow = baseShape[2];
    Array<Float> weightSum(shape, 0.0f);
    for (size_t i = 0; i < nPol; ++i) {
      IPosition start(3, i, 0, 0);
      IPosition end(3, i, nChan - 1, nRow - 1);
      Array<T> dSlice = baseData(start, end);
      auto fSlice = baseFlag(start, end);
      Array<Float> weight(dSlice.shape());
      CollapserImpl::SetWeight(start, end, baseWeight, weight);
      Bool b1, b2, b3, b4, b5;
      T *p_r = result.getStorage(b1);
      Float *p_s = weightSum.getStorage(b2);
      T const *p_d = dSlice.getStorage(b3);
      Float const *p_w = weight.getStorage(b4);
      Bool const *p_f = fSlice.getStorage(b5);
      for (size_t j = 0; j < nChan * nRow; ++j) {
        if (!p_f[j]) {
          p_r[j] += p_d[j] * p_w[j];
          p_s[j] += p_w[j];
        }
      }
      result.putStorage(p_r, b1);
      weightSum.putStorage(p_s, b2);
      dSlice.freeStorage(p_d, b3);
      weight.freeStorage(p_w, b4);
      fSlice.freeStorage(p_f, b5);
    }
    result /= weightSum;
  }

  static void CollapseWeight(Array<Float> const &baseWeight,
      Array<Float> &result) {
    IPosition const baseShape = baseWeight.shape();
    IPosition resultShape(baseShape);
    resultShape[0] = 1;
    result.resize(resultShape);
    result = 0;
    size_t nPol = baseShape[0];
    for (size_t i = 0; i < nPol; ++i) {
      IPosition start(baseShape.size(), 0);
      start[0] = i;
      IPosition end(baseShape);
      end -= 1;
      end[0] = i;
      auto wslice = baseWeight(start, end);
      CollapserImpl::AccumulateWeight(wslice, result);
    }
    CollapserImpl::NormalizeWeight(result);
  }

  template<class T>
  static void ValidateDataColumn(Array<T> const &data, MeasurementSet const &ms,
      String const &columnName, Vector<uInt> const &rowIds) {
    cout << "Start " << __func__ << endl;
    Array<T> baseData;
    Array<Float> baseWeight;
    Array<Bool> baseFlag;
    Filler<T>::FillArrayReference(ms, columnName, rowIds, baseData);
    Filler<Bool>::FillArrayReference(ms, "FLAG", rowIds, baseFlag);
    baseWeight.resize(baseData.shape());
    Filler<Float>::FillWeightSp(ms, rowIds, baseWeight);
    Cube<T> ref(data.shape());
    CollapseData(baseData, baseFlag, baseWeight, ref);
    ValidatorUtil::ValidateArray<T>(ref, data);
  }

  static void ValidateWeightColumn(Array<Float> const &weight,
      MeasurementSet const &ms, String const &columnName,
      Vector<uInt> const &rowIds) {
    Array<Float> baseWeight;
    Filler<Float>::FillArrayReference(ms, columnName, rowIds, baseWeight);
    Array<Float> ref;
    CollapseWeight(baseWeight, ref);
    ValidatorUtil::ValidateArray(ref, weight);
  }

  static void ValidateData(Array<Complex> const &data, MeasurementSet const &ms,
      Vector<uInt> const &rowIds) {
    ValidateDataColumn(data, ms, "DATA", rowIds);
  }

  static void ValidateCorrected(Cube<Complex> const &data,
      MeasurementSet const &ms, Vector<uInt> const &rowIds) {
    ValidateDataColumn(data, ms, "CORRECTED_DATA", rowIds);
  }

  static void ValidateModel(Cube<Complex> const &data, MeasurementSet const &ms,
      Vector<uInt> const &rowIds) {
    ValidateDataColumn(data, ms, "MODEL_DATA", rowIds);
  }

  static void ValidateFloat(Cube<Float> const &data, MeasurementSet const &ms,
      Vector<uInt> const &rowIds) {
    ValidateDataColumn(data, ms, "FLOAT_DATA", rowIds);
  }

  static void ValidateFlag(Cube<Bool> const &flag, MeasurementSet const &ms,
      Vector<uInt> const &rowIds) {
    Array<Bool> baseFlag;
    Filler<Bool>::FillArrayReference(ms, "FLAG", rowIds, baseFlag);
    Cube<Bool> ref(flag.shape(), True);
    IPosition const baseShape = baseFlag.shape();
    ASSERT_EQ(baseShape.size(), (uInt )3);
    ASSERT_EQ(baseShape[1], flag.shape()[1]);
    ASSERT_EQ(baseShape[2], flag.shape()[2]);
    size_t nPol = baseShape[0];
    size_t nChan = baseShape[1];
    size_t nRow = baseShape[2];
    for (size_t i = 0; i < nPol; ++i) {
      IPosition start(3, i, 0, 0);
      IPosition end(3, i, nChan - 1, nRow - 1);
      auto fslice = baseFlag(start, end);
      ref &= fslice;
    }
    ValidatorUtil::ValidateArray(ref, flag);
  }

  static void ValidateFlagRow(Vector<Bool> const &flag,
      MeasurementSet const &ms, Vector<uInt> const &rowIds) {
    Vector<Bool> ref;
    Filler<Bool>::FillScalarReference(ms, "FLAG_ROW", rowIds, ref);
    ASSERT_EQ(flag.shape(), ref.shape());
    EXPECT_TRUE(allEQ(flag, ref));
  }

  static void ValidateWeight(Matrix<Float> const &weight,
      MeasurementSet const &ms, Vector<uInt> const &rowIds) {
    ValidateWeightColumn(weight, ms, "WEIGHT", rowIds);
  }

  static void ValidateWeightSp(Cube<Float> const &weight,
      MeasurementSet const &ms, Vector<uInt> const &rowIds) {
    if (ms.tableDesc().isColumn("WEIGHT_SPECTRUM")) {
      // WEIGHT_SPECTRUM exists
      ValidateWeightColumn(weight, ms, "WEIGHT_SPECTRUM", rowIds);
    } else {
      // only WEIGHT exists
      Matrix<Float> scalarWeight = weight.xzPlane(0);
      ValidateWeightColumn(scalarWeight, ms, "WEIGHT", rowIds);
      IPosition const weightShape = weight.shape();
      size_t nPol = weightShape[0];
      size_t nChan = weightShape[1];
      size_t nRow = weightShape[2];
      for (size_t i = 0; i < nPol; ++i) {
        for (size_t j = 0; j < nRow; ++j) {
          IPosition start(3, i, 0, j);
          IPosition end(3, i, nChan - 1, j);
          auto wslice = weight(start, end);
          auto wref = scalarWeight(i, j);
          EXPECT_TRUE(allEQ(wref, wslice));
        }
      }
    }
  }
};

struct GeometricAverageValidator: public ValidatorBase<GeometricAverageValidator> {
  static String GetMode() {
    return "geometric";
  }

  static String GetTypePrefix() {
    return "GeometricPolAverage(";
  }

  static void SetWeight(IPosition const &start, IPosition const &end,
      Array<Float> const &baseWeight, Array<Float> &weight) {
    weight = baseWeight(start, end);
  }

  static void AccumulateWeight(Array<Float> const &weight,
      Array<Float> &result) {
    result += weight;
  }

  static void NormalizeWeight(Array<Float> &/*result*/) {
  }
};

struct StokesAverageValidator: public ValidatorBase<StokesAverageValidator> {
  static String GetMode() {
    return "stokes";
  }

  static String GetTypePrefix() {
    return "StokesPolAverage(";
  }

  static void SetWeight(IPosition const &/*start*/, IPosition const &/*end*/,
      Array<Float> const &/*baseWeight*/, Array<Float> &weight) {
    weight = 1.0f;
  }

  static void AccumulateWeight(Array<Float> const &weight,
      Array<Float> &result) {
    result += 1.0f / weight;
  }

  static void NormalizeWeight(Array<Float> &result) {
    result = 4.0f / result;
  }
};
} // anonymous namespace

class PolAverageTVITest: public ::testing::Test {
public:
  virtual void SetUp() {
//    my_data_name_ = "analytic_spectra.ms";
    my_data_name_ = "analytic_type1.bl.ms";
    my_ms_name_ = "polaverage_test.ms";
    std::string const data_path = ::GetCasaDataPath()
        + "/regression/unittest/tsdbaseline/";
//    + "/regression/unittest/singledish/";

    copyDataFromRepository(data_path);
    ASSERT_TRUE(File(my_data_name_).exists());
    deleteTable(my_ms_name_);
  }

  virtual void TearDown() {
    cleanup();
  }
protected:
  std::string my_ms_name_;
  std::string my_data_name_;

  VisibilityIterator2 *ManufactureVIAtFactory(String const &mode) {
    // create read-only VI impl
    Block<MeasurementSet const *> const mss(1,
        new MeasurementSet(my_data_name_, Table::Old));
    SortColumns defaultSortColumns;

    std::unique_ptr<ViImplementation2> inputVii(
        new VisibilityIteratorImpl2(mss, defaultSortColumns, 0.0, VbPlain,
            False));

    Record modeRec;
    if (mode.size() > 0) {
      modeRec.define("mode", mode);
    }

    PolAverageVi2Factory factory(modeRec, inputVii.get());

    std::unique_ptr<VisibilityIterator2> vi;
    try {
      vi.reset(new VisibilityIterator2(factory));
    } catch (...) {
      cout << "Failed to create VI at factory" << endl;
      throw;
    }

    cout << "Created VI type \"" << vi->ViiType() << "\"" << endl;

    // vi will be responsible for releasing inputVii so unique_ptr
    // should release the ownership here
    inputVii.release();

    return vi.release();
  }

  void TestFactory(String const &mode, String const &expectedClassName) {

    cout << "Mode \"" << mode << "\" expected class name \""
        << expectedClassName << "\"" << endl;

    if (expectedClassName.size() > 0) {
      std::unique_ptr<VisibilityIterator2> vi(ManufactureVIAtFactory(mode));

      // Verify type string
      String viiType = vi->ViiType();
      EXPECT_TRUE(viiType.startsWith(expectedClassName));
    } else {
      cout << "Creation of VI via factory will fail" << endl;
      // exception must be thrown
      EXPECT_THROW( {
            std::unique_ptr<VisibilityIterator2> vi(ManufactureVIAtFactory(mode)); //new VisibilityIterator2(factory));
          },
          AipsError)<< "The process must throw AipsError";
    }
  }

  template<class Validator>
  void TestTVI() {
    // Create VI
    std::unique_ptr<VisibilityIterator2> vi(ManufactureVIAtFactory(Validator::GetMode()));
    ASSERT_TRUE(vi->ViiType().startsWith(Validator::GetTypePrefix()));

    // MS property
    auto ms = vi->ms();
    uInt const nRowMs = ms.nrow();
    uInt const nRowPolarizationTable = ms.polarization().nrow();
    auto const desc = ms.tableDesc();
    auto const correctedExists = desc.isColumn("CORRECTED_DATA");
    auto const modelExists = desc.isColumn("MODEL_DATA");
    auto const dataExists = desc.isColumn("DATA");
    auto const floatExists = desc.isColumn("FLOAT_DATA");
    //auto const weightSpExists = desc.isColumn("WEIGHT_SPECTRUM");
    cout << "MS Property" << endl;
    cout << "\tMS Name: \"" << ms.tableName() << "\"" << endl;
    cout << "\tNumber of Rows: " << nRowMs << endl;
    cout << "\tNumber of Spws: " << vi->nSpectralWindows() << endl;
    cout << "\tNumber of Polarizations: " << vi->nPolarizationIds() << endl;
    cout << "\tNumber of DataDescs: " << vi->nDataDescriptionIds() << endl;
    cout << "\tChannelized Weight Exists? "
    << (vi->weightSpectrumExists() ? "True" : "False") << endl;
    //cout << "\tChannelized Sigma Exists? " << (vi->sigmaSpectrumExists() ? "True" : "False") << endl;

    // mv-VI consistency check
    EXPECT_EQ(nRowPolarizationTable + 1, (uInt)vi->nPolarizationIds());

    // VI iteration
    Vector<uInt> swept(nRowMs, 0);
    uInt nRowChunkSum = 0;
    VisBuffer2 *vb = vi->getVisBuffer();
    vi->originChunks();
    while (vi->moreChunks()) {
      vi->origin();
      Int const nRowChunk = vi->nRowsInChunk();
      nRowChunkSum += nRowChunk;
      cout << "*************************" << endl;
      cout << "*** Start loop on chunk " << vi->getSubchunkId().chunk() << endl;
      cout << "*** Number of Rows: " << nRowChunk << endl;
      cout << "*************************" << endl;

      Int nRowSubchunkSum = 0;

      while (vi->more()) {
        auto subchunk = vi->getSubchunkId();
        cout << "=== Start loop on subchunk " << subchunk.subchunk() << " ===" << endl;

        // cannot use getInterval due to the error
        // "undefined reference to VisibilityIterator2::getInterval"
        // even if the code is liked to libmsvis.so.
        //cout << "Interval: " << vi->getInterval() << endl;

        cout << "Antenna1: " << vb->antenna1() << endl;
        cout << "Antenna2: " << vb->antenna2() << endl;
        cout << "Array Id: " << vb->arrayId() << endl;
        cout << "Data Desc Ids: " << vb->dataDescriptionIds() << endl;
        cout << "Polarization Id: " << vb->polarizationId() << endl;
        cout << "Exposure: " << vb->exposure() << endl;
        cout << "Feed1: " << vb->feed1() << endl;
        cout << "Feed2: " << vb->feed2() << endl;
        cout << "Field Id: " << vb->fieldId() << endl;
        cout << "Flag Row: " << vb->flagRow() << endl;
        cout << "Observation Id: " << vb->observationId() << endl;
        cout << "Processor Id: " << vb->processorId() << endl;
        cout << "Scan: " << vb->scan() << endl;
        cout << "State Id: " << vb->stateId() << endl;
        cout << "Time: " << vb->time() << endl;
        cout << "Time Centroid: " << vb->timeCentroid() << endl;
        cout << "Time Interval: " << vb->timeInterval() << endl;
        auto const corrTypes = vb->correlationTypes();
        cout << "Correlation Types: " << corrTypes << endl;
        //cout << "UVW: " << vb->uvw() << endl;

        cout << "---" << endl;
        Int nRowSubchunk = vb->nRows();
        Vector<uInt> rowIds = vb->rowIds();
        for (auto iter = rowIds.begin(); iter != rowIds.end(); ++iter) {
          swept[*iter] += 1;
        }
        nRowSubchunkSum += nRowSubchunk;
        Int nAnt = vb->nAntennas();
        Int nChan = vb->nChannels();
        Int nCorr = vb->nCorrelations();
        IPosition visShape = vb->getShape();
        cout << "Number of Subchunk Rows: " << nRowSubchunk << endl;
        cout << "Number of Antennas: " << nAnt << endl;
        cout << "Number of Channels: " << nChan << endl;
        cout << "Number of Correlations: " << nCorr << endl;
        cout << "Row Ids: " << rowIds << endl;
        cout << "Spectral Windows: " << vb->spectralWindows() << endl;
        cout << "Visibility Shape: " << visShape << endl;
        cout << "---" << endl;
        Cube<Complex> visCube = vb->visCube();
        cout << "DATA Shape: " << visCube.shape() << endl;
        Cube<Complex> visCubeCorrected = vb->visCubeCorrected();
        cout << "CORRECTED_DATA Shape: " << visCubeCorrected.shape() << endl;
        Cube<Complex> visCubeModel = vb->visCubeModel();
        cout << "MODEL_DATA Shape: " << visCubeModel.shape() << endl;
        Cube<Float> visCubeFloat = vb->visCubeFloat();
        cout << "FLOAT_DATA Shape: " << visCubeFloat.shape() << endl;
        Cube<Bool> flagCube = vb->flagCube();
        cout << "FLAG Shape: " << flagCube.shape() << endl;
        Vector<Bool> flagRow = vb->flagRow();
        cout << "FLAG_ROW Shape: " << flagRow.shape() << endl;
        Matrix<Float> weight = vb->weight();
        cout << "WEIGHT Shape: " << weight.shape() << endl;
        Cube<Float> weightSp = vb->weightSpectrum();
        cout << "WEIGHT_SPECTRUM Shape: " << weightSp.shape() << endl;
        cout << "===" << endl;

        // internal consistency check
        EXPECT_EQ(nRowSubchunk, visShape[2]);
        EXPECT_EQ(nChan, visShape[1]);
        EXPECT_EQ(nCorr, visShape[0]);
        EXPECT_EQ(!dataExists, visCube.empty());
        if (!visCube.empty()) {
          EXPECT_EQ(visShape, visCube.shape());
        }
        EXPECT_EQ(!correctedExists, visCubeCorrected.empty());
        if (!visCubeCorrected.empty()) {
          EXPECT_EQ(visShape, visCubeCorrected.shape());
        }
        EXPECT_EQ(!modelExists, visCubeModel.empty());
        if (!visCubeModel.empty()) {
          EXPECT_EQ(visShape, visCubeModel.shape());
        }
        EXPECT_EQ(!floatExists, visCubeFloat.empty());
        if (!visCubeFloat.empty()) {
          EXPECT_EQ(visShape, visCubeFloat.shape());
        }
        EXPECT_EQ((uInt)1, flagRow.size());
        EXPECT_EQ((ssize_t)1, weight.shape()[0]);
        EXPECT_EQ((ssize_t)nRowSubchunk, weight.shape()[1]);
        // NB: weight spectrum is created on-the-fly based on WEIGHT
        //     so that weightSp is always non-empty.
        //     see VisBufferImpl2::fillWeightSpectrum.
        //EXPECT_EQ(!weightSpExists, weightSp.empty());
        EXPECT_FALSE(weightSp.empty());
        if (!weightSp.empty()) {
          EXPECT_EQ((ssize_t)1, weightSp.shape()[0]);
          EXPECT_EQ((ssize_t)nChan, weightSp.shape()[1]);
          EXPECT_EQ((ssize_t)nRowSubchunk, weightSp.shape()[2]);
        }

        // polarization averaging specific check
        // length of the correlation (polarization) axis must be 1
        ASSERT_EQ((ssize_t)1, visShape[0]);
        // polarization id always points to the row to be appended
        ASSERT_EQ(nRowPolarizationTable, (uInt)vb->polarizationId());
        // correlation type is always I
        ASSERT_EQ((size_t)1, corrTypes.size());
        ASSERT_TRUE(allEQ(corrTypes, (Int)Stokes::I));

        // validation of polarization average
        if (!visCube.empty()) {
          cout << "validate DATA" << endl;
          Validator::ValidateData(visCube, ms, rowIds);
        }
        if (!visCubeCorrected.empty()) {
          cout << "validate CORRECTED_DATA" << endl;
          Validator::ValidateCorrected(visCubeCorrected, ms, rowIds);
        }
        if (!visCubeModel.empty()) {
          cout << "validate MODEL_DATA" << endl;
          Validator::ValidateModel(visCubeModel, ms, rowIds);
        }
        if (!visCubeFloat.empty()) {
          cout << "validate FLOAT_DATA" << endl;
          Validator::ValidateFloat(visCubeFloat, ms, rowIds);
        }
        Validator::ValidateFlag(flagCube, ms, rowIds);
        Validator::ValidateFlagRow(flagRow, ms, rowIds);
        Validator::ValidateWeight(weight, ms, rowIds);
        Validator::ValidateWeightSp(weightSp, ms, rowIds);

        // next round of iteration
        vi->next();
      }

      // chunk-subchunk consistency check
      EXPECT_EQ(nRowChunk, nRowSubchunkSum);

      vi->nextChunk();
    }

    // chunk-ms consistency check
    EXPECT_EQ(nRowMs, nRowChunkSum);

    // iteration check
    EXPECT_TRUE(allEQ(swept, (uInt)1));

  }

private:
  void copyRegular(String const &src, String const &dst) {
    RegularFile r(src);
    r.copy(dst);
  }
  void copySymLink(String const &src, String const &dst) {
    Path p = SymLink(src).followSymLink();
    String actual_src = p.absoluteName();
    File f(actual_src);
    if (f.isRegular()) {
      copyRegular(actual_src, dst);
    } else if (f.isDirectory()) {
      copyDirectory(actual_src, dst);
    }
  }
  void copyDirectory(String const &src, String const &dst) {
    Directory dsrc(src);
    Directory ddst(dst);
    ddst.create();
    DirectoryIterator iter(dsrc);
    while (!iter.pastEnd()) {
      String name = iter.name();
      if (name.contains(".svn")) {
        iter++;
        continue;
      }
      File f = iter.file();
      Path psrc(src);
      Path pdst(dst);
      psrc.append(name);
      String sub_src = psrc.absoluteName();
      pdst.append(name);
      String sub_dst = pdst.absoluteName();
      if (f.isSymLink()) {
        copySymLink(sub_src, sub_dst);
      } else if (f.isRegular()) {
        copyRegular(sub_src, sub_dst);
      } else if (f.isDirectory()) {
        copyDirectory(sub_src, sub_dst);
      }
      iter++;
    }
  }
  void copyDataFromRepository(std::string const &data_dir) {
    if (my_data_name_.size() > 0) {
      std::string full_path = data_dir + my_data_name_;
      std::string work_path = my_data_name_;
      File f(full_path);
      ASSERT_TRUE(f.exists());
      if (f.isSymLink()) {
        copySymLink(full_path, work_path);
      } else if (f.isRegular()) {
        copyRegular(full_path, work_path);
      } else if (f.isDirectory()) {
        copyDirectory(full_path, work_path);
      }
    }
  }
  void cleanup() {
    if (my_data_name_.size() > 0) {
      File f(my_data_name_);
      if (f.isRegular()) {
        RegularFile r(my_data_name_);
        r.remove();
      } else if (f.isDirectory()) {
        Directory d(my_data_name_);
        d.removeRecursive();
      }
    }
    deleteTable(my_ms_name_);
  }
  void deleteTable(std::string const &name) {
    File file(name);
    if (file.exists()) {
      std::cout << "Removing " << name << std::endl;
      Table::deleteTable(name, true);
    }
  }

};

TEST_F(PolAverageTVITest, Factory) {

  TestFactory("default", "GeometricPolAverage");
  TestFactory("Default", "GeometricPolAverage");
  TestFactory("DEFAULT", "GeometricPolAverage");
  TestFactory("geometric", "GeometricPolAverage");
  TestFactory("Geometric", "GeometricPolAverage");
  TestFactory("GEOMETRIC", "GeometricPolAverage");
  TestFactory("stokes", "StokesPolAverage");
  TestFactory("Stokes", "StokesPolAverage");
  TestFactory("STOKES", "StokesPolAverage");
  // empty mode (default)
  TestFactory("", "GeometricPolAverage");
  // invalid mode (throw exception)
  TestFactory("invalid", "");
}

TEST_F(PolAverageTVITest, GeometricAverage) {
  TestTVI<GeometricAverageValidator>();
}

TEST_F(PolAverageTVITest, StokesAverage) {
  TestTVI<StokesAverageValidator>();
}

// TODO: define test on not-well-behaved data
//       such as partially flagged, contains NaN, etc.
// TODO: define test on the data that should not average
//       along polarization axis (IQUV or single polarization)

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  std::cout << "PolAverageTVI test " << std::endl;
  return RUN_ALL_TESTS();
}
