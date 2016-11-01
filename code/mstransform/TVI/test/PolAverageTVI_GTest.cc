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
} // anonymous namespace

class PolAverageTVITest: public ::testing::Test {
public:
  virtual void SetUp() {
    my_data_name_ = "analytic_spectra.ms";
    my_ms_name_ = "polaverage_test.ms";
    std::string const data_path = ::GetCasaDataPath()
        + "/regression/unittest/singledish/";

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

  void TestFactory(String const &mode, String const &expectedClassName) {
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
    if (expectedClassName.size() > 0) {
      std::unique_ptr<VisibilityIterator2,
          VerboseDeleterForNew<VisibilityIterator2> > vi(
          new VisibilityIterator2(factory));
      // vi will be responsible for releasing inputVii so unique_ptr
      // should release the ownership here
      inputVii.release();
      String viiType = vi->ViiType();
      cout << "Mode \"" << mode << "\": Created VI type \"" << viiType << "\""
          << endl;
      EXPECT_TRUE(viiType.startsWith(expectedClassName));
    } else {
      // exception must be thrown
      EXPECT_THROW( {
        std::unique_ptr<VisibilityIterator2> vi(new VisibilityIterator2(factory));
      },
      AipsError);
    }
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

TEST_F(PolAverageTVITest, FactoryTest) {

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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  std::cout << "PolAverageTVI test " << std::endl;
  return RUN_ALL_TESTS();
}
