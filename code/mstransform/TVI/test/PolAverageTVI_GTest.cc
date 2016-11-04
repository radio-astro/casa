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

struct GeometricAverageValidator {
  static String GetMode() { return "geometric"; }
  static String GetTypePrefix() { return "GeometricPolAverage("; }
};

struct StokesAverageValidator {
  static String GetMode() { return "stokes"; }
  static String GetTypePrefix() { return "StokesPolAverage("; }
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

    // MS property via VI
    auto ms = vi->ms();
    uInt const nRowMs = ms.nrow();
    auto const desc = ms.tableDesc();
    auto const correctedExists = desc.isColumn("CORRECTED_DATA");
    auto const modelExists = desc.isColumn("MODEL_DATA");
    auto const dataExists = desc.isColumn("DATA");
    auto const floatExists = desc.isColumn("FLOAT_DATA");
    cout << "MS Property" << endl;
    cout << "\tMS Name: \"" << ms.tableName() << "\"" << endl;
    cout << "\tNumber of Rows: " << nRowMs << endl;
    cout << "\tNumber of Spws: " << vi->nSpectralWindows() << endl;
    cout << "\tNumber of Polarizations: " << vi->nPolarizationIds() << endl;
    cout << "\tNumber of DataDescs: " << vi->nDataDescriptionIds() << endl;
    cout << "\tChannelized Weight Exists? "
        << (vi->weightSpectrumExists() ? "True" : "False") << endl;
    //cout << "\tChannelized Sigma Exists? " << (vi->sigmaSpectrumExists() ? "True" : "False") << endl;

    // VI iteration
    Vector<uInt> swept(nRowMs, 0);
    uInt nRowChunkSum = 0;
    VisBuffer2 *vb = vi->getVisBuffer();
    ViImplementation2 *vii = vi->getImpl();
    vi->originChunks();
    while (vi->moreChunks()) {
      vi->origin();
      Int const nRowChunk = vi->nRowsInChunk();
      nRowChunkSum += nRowChunk;
      cout << "***" << endl;
      cout << "*** Start loop on chunk " << vi->getSubchunkId().chunk() << endl;
      cout << "*** Number of Rows: " << nRowChunk << endl;
      cout << "***" << endl;

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
        IPosition shape = vb->getShape();
        cout << "Number of Subchunk Rows: " << nRowSubchunk << endl;
        cout << "Number of Antennas: " << nAnt << endl;
        cout << "Number of Channels: " << nChan << endl;
        cout << "Number of Correlations: " << nCorr << endl;
        cout << "Row Ids: " << rowIds << endl;
        cout << "Spectral Windows: " << vb->spectralWindows() << endl;
        cout << "Visibility Shape: " << shape << endl;
        cout << "---" << endl;
        IPosition visShape = vii->visibilityShape();
        cout << "Visibility Shape (from VII): " << visShape << endl;
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
        cout << "===" << endl;

        // internal consistency check
        EXPECT_EQ(nRowSubchunk, shape[2]);
        EXPECT_EQ(nChan, shape[1]);
        EXPECT_EQ(nCorr, shape[0]);
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

        // polarization averaging check
        // length of the correlation (polarization) axis must be 1
        ASSERT_EQ((ssize_t)1, visShape[0]);

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

//  // Create VI
//  std::unique_ptr<VisibilityIterator2> vi(ManufactureVIAtFactory("geometric"));
//  ASSERT_TRUE(vi->ViiType().startsWith("GeometricPolAverage("));
//
//  // MS property via VI
//  auto ms = vi->ms();
//  uInt const nRowMs = ms.nrow();
//  auto const desc = ms.tableDesc();
//  auto const correctedExists = desc.isColumn("CORRECTED_DATA");
//  auto const modelExists = desc.isColumn("MODEL_DATA");
//  auto const dataExists = desc.isColumn("DATA");
//  auto const floatExists = desc.isColumn("FLOAT_DATA");
//  cout << "MS Property" << endl;
//  cout << "\tMS Name: \"" << ms.tableName() << "\"" << endl;
//  cout << "\tNumber of Rows: " << nRowMs << endl;
//  cout << "\tNumber of Spws: " << vi->nSpectralWindows() << endl;
//  cout << "\tNumber of Polarizations: " << vi->nPolarizationIds() << endl;
//  cout << "\tNumber of DataDescs: " << vi->nDataDescriptionIds() << endl;
//  cout << "\tChannelized Weight Exists? "
//      << (vi->weightSpectrumExists() ? "True" : "False") << endl;
//  //cout << "\tChannelized Sigma Exists? " << (vi->sigmaSpectrumExists() ? "True" : "False") << endl;
//
//  // VI iteration
//  Vector<uInt> swept(nRowMs, 0);
//  uInt nRowChunkSum = 0;
//  VisBuffer2 *vb = vi->getVisBuffer();
//  ViImplementation2 *vii = vi->getImpl();
//  vi->originChunks();
//  while (vi->moreChunks()) {
//    vi->origin();
//    Int const nRowChunk = vi->nRowsInChunk();
//    nRowChunkSum += nRowChunk;
//    cout << "***" << endl;
//    cout << "*** Start loop on chunk " << vi->getSubchunkId().chunk() << endl;
//    cout << "*** Number of Rows: " << nRowChunk << endl;
//    cout << "***" << endl;
//
//    Int nRowSubchunkSum = 0;
//
//    while (vi->more()) {
//      auto subchunk = vi->getSubchunkId();
//      cout << "=== Start loop on subchunk " << subchunk.subchunk() << " ===" << endl;
//
//      // cannot use getInterval due to the error
//      // "undefined reference to VisibilityIterator2::getInterval"
//      // even if the code is liked to libmsvis.so.
//      //cout << "Interval: " << vi->getInterval() << endl;
//
//      cout << "Antenna1: " << vb->antenna1() << endl;
//      cout << "Antenna2: " << vb->antenna2() << endl;
//      cout << "Array Id: " << vb->arrayId() << endl;
//      cout << "Data Desc Ids: " << vb->dataDescriptionIds() << endl;
//      cout << "Exposure: " << vb->exposure() << endl;
//      cout << "Feed1: " << vb->feed1() << endl;
//      cout << "Feed2: " << vb->feed2() << endl;
//      cout << "Field Id: " << vb->fieldId() << endl;
//      cout << "Flag Row: " << vb->flagRow() << endl;
//      cout << "Observation Id: " << vb->observationId() << endl;
//      cout << "Processor Id: " << vb->processorId() << endl;
//      cout << "Scan: " << vb->scan() << endl;
//      cout << "State Id: " << vb->stateId() << endl;
//      cout << "Time: " << vb->time() << endl;
//      cout << "Time Centroid: " << vb->timeCentroid() << endl;
//      cout << "Time Interval: " << vb->timeInterval() << endl;
//      //cout << "UVW: " << vb->uvw() << endl;
//
//      cout << "---" << endl;
//      Int nRowSubchunk = vb->nRows();
//      Vector<uInt> rowIds = vb->rowIds();
//      for (auto iter = rowIds.begin(); iter != rowIds.end(); ++iter) {
//        swept[*iter] += 1;
//      }
//      nRowSubchunkSum += nRowSubchunk;
//      Int nAnt = vb->nAntennas();
//      Int nChan = vb->nChannels();
//      Int nCorr = vb->nCorrelations();
//      IPosition shape = vb->getShape();
//      cout << "Number of Subchunk Rows: " << nRowSubchunk << endl;
//      cout << "Number of Antennas: " << nAnt << endl;
//      cout << "Number of Channels: " << nChan << endl;
//      cout << "Number of Correlations: " << nCorr << endl;
//      cout << "Row Ids: " << rowIds << endl;
//      cout << "Spectral Windows: " << vb->spectralWindows() << endl;
//      cout << "Visibility Shape: " << shape << endl;
//      cout << "---" << endl;
//      IPosition visShape = vii->visibilityShape();
//      cout << "Visibility Shape (from VII): " << visShape << endl;
//      Cube<Complex> visCube = vb->visCube();
//      cout << "DATA Shape: " << visCube.shape() << endl;
//      Cube<Complex> visCubeCorrected = vb->visCubeCorrected();
//      cout << "CORRECTED_DATA Shape: " << visCubeCorrected.shape() << endl;
//      Cube<Complex> visCubeModel = vb->visCubeModel();
//      cout << "MODEL_DATA Shape: " << visCubeModel.shape() << endl;
//      Cube<Float> visCubeFloat = vb->visCubeFloat();
//      cout << "FLOAT_DATA Shape: " << visCubeFloat.shape() << endl;
//      Cube<Bool> flagCube = vb->flagCube();
//      cout << "FLAG Shape: " << flagCube.shape() << endl;
//      Vector<Bool> flagRow = vb->flagRow();
//      cout << "FLAG_ROW Shape: " << flagRow.shape() << endl;
//      cout << "===" << endl;
//
//      // internal consistency check
//      EXPECT_EQ(nRowSubchunk, shape[2]);
//      EXPECT_EQ(nChan, shape[1]);
//      EXPECT_EQ(nCorr, shape[0]);
//      EXPECT_EQ(!dataExists, visCube.empty());
//      if (!visCube.empty()) {
//        EXPECT_EQ(visShape, visCube.shape());
//      }
//      EXPECT_EQ(!correctedExists, visCubeCorrected.empty());
//      if (!visCubeCorrected.empty()) {
//        EXPECT_EQ(visShape, visCubeCorrected.shape());
//      }
//      EXPECT_EQ(!modelExists, visCubeModel.empty());
//      if (!visCubeModel.empty()) {
//        EXPECT_EQ(visShape, visCubeModel.shape());
//      }
//      EXPECT_EQ(!floatExists, visCubeFloat.empty());
//      if (!visCubeFloat.empty()) {
//        EXPECT_EQ(visShape, visCubeFloat.shape());
//      }
//      EXPECT_EQ((uInt)1, flagRow.size());
//
//      // polarization averaging check
//      // length of the correlation (polarization) axis must be 1
//      ASSERT_EQ((ssize_t)1, visShape[0]);
//
//      // next round of iteration
//      vi->next();
//    }
//
//    // chunk-subchunk consistency check
//    EXPECT_EQ(nRowChunk, nRowSubchunkSum);
//
//    vi->nextChunk();
//  }
//
//  // chunk-ms consistency check
//  EXPECT_EQ(nRowMs, nRowChunkSum);
//
//  // iteration check
//  EXPECT_TRUE(allEQ(swept, (uInt)1));

}

TEST_F(PolAverageTVITest, StokesAverage) {
  TestTVI<StokesAverageValidator>();
//  // Create VI
//  std::unique_ptr<VisibilityIterator2> vi(ManufactureVIAtFactory("stokes"));
//  ASSERT_TRUE(vi->ViiType().startsWith("StokesPolAverage("));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  std::cout << "PolAverageTVI test " << std::endl;
  return RUN_ALL_TESTS();
}
