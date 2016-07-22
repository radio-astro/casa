//# tMSMetaInfoForCal.cc: Tests the MS Meta-info services in cal context
//# Copyright (C) 1995,1999,2000,2001
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

//#include <casa/aips.h>
//#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
//#include <msvis/MSVis/VisBuffer2.h>
//#include <casa/Arrays/ArrayMath.h>
//#include <casa/OS/Timer.h>
#include <casa/Arrays/Vector.h>
//#include <casa/Arrays/ArrayIO.h>
#include <casacore/casa/OS/EnvVar.h>
#include <casacore/casa/OS/Path.h>
#include <synthesis/MeasurementComponents/MSMetaInfoForCal.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>

#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casacore;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(MSMetaInfoForCal, NoMS) {

  {
    MSMetaInfoForCal msm("<noms>");

    // Basic MS info
    ASSERT_EQ(False,msm.msOk());
    ASSERT_EQ(String("<noms>"),msm.msname());
    
    // Auto-shaped things
    ASSERT_EQ(4,Int(msm.nAnt()));
    ASSERT_EQ(1,Int(msm.nSpw()));
    ASSERT_EQ(1,Int(msm.nFld()));

    ASSERT_EQ(String("AntennaId1"),msm.antennaName(1));
    ASSERT_EQ(String("FieldId0"),msm.fieldName(0));
    ASSERT_EQ(String("SpwId0"),msm.spwName(0));
    
    Vector<String> antnames, fldnames;
    msm.antennaNames(antnames);
    msm.fieldNames(fldnames);
    //cout << "antnames = " << antnames << endl;
    //cout << "fldnames = " << fldnames << endl;
    ASSERT_EQ(uInt(4),antnames.nelements());
    for (Int iant=0;iant<4;++iant) 
      ASSERT_EQ(String("AntennaId"+String::toString(iant)),antnames[iant]);

    ASSERT_EQ(uInt(1),fldnames.nelements());
    ASSERT_EQ(String("FieldId0"),fldnames[0]);



    // These are information-free, essentially, for the "<noms>" case
    //cout << "msm.fieldIdAtTime(123456789.0) = " << msm.fieldIdAtTime(123456789.0) << endl;
    //cout << "msm.scanNumberAtTime(123456789.0) = " << msm.scanNumberAtTime(123456789.0) << endl;
    ASSERT_EQ(-1,msm.fieldIdAtTime(123456789.0));
    ASSERT_EQ(-1,msm.scanNumberAtTime(123456789.0));


  }

  {
    MSMetaInfoForCal msm(10,1,5);

    // Basic MS info
    ASSERT_EQ(False,msm.msOk());
    ASSERT_EQ(String("<noms>"),msm.msname());
    
    // Shapes
    ASSERT_EQ(10,Int(msm.nAnt()));
    ASSERT_EQ(1,Int(msm.nSpw()));
    ASSERT_EQ(5,Int(msm.nFld()));

    ASSERT_EQ(String("AntennaId9"),msm.antennaName(9));
    ASSERT_EQ(String("FieldId4"),msm.fieldName(4));
    ASSERT_EQ(String("SpwId0"),msm.spwName(0));

    Vector<String> antnames, fldnames;
    msm.antennaNames(antnames);
    msm.fieldNames(fldnames);
    //cout << "antnames = " << antnames << endl;
    //cout << "fldnames = " << fldnames << endl;
    ASSERT_EQ(uInt(10),antnames.nelements());
    for (Int iant=0;iant<10;++iant) 
      ASSERT_EQ(String("AntennaId"+String::toString(iant)),antnames[iant]);

    ASSERT_EQ(uInt(5),fldnames.nelements());
    for (Int ifld=0;ifld<5;++ifld)
      ASSERT_EQ(String("FieldId"+String::toString(ifld)),fldnames[ifld]);

    // These are information-free, essentially, for the "<noms>" case
    //cout << "msm.fieldIdAtTime(123456789.0) = " << msm.fieldIdAtTime(123456789.0) << endl;
    //cout << "msm.scanNumberAtTime(123456789.0) = " << msm.scanNumberAtTime(123456789.0) << endl;
    ASSERT_EQ(-1,msm.fieldIdAtTime(123456789.0));
    ASSERT_EQ(-1,msm.scanNumberAtTime(123456789.0));

  }


}

TEST(MSMetaInfoForCal, MSbyName) {
  
  // Path to an MS in the DR
  String *casapath = new String[2];
  split(EnvironmentVariable::get("CASAPATH"), casapath, 2, String(" "));
  // Use of Path().absoluteName() absorbs relative stuff in casapath
  String mspath(Path(casapath[0]+"/data/regression/unittest/flagdata/Four_ants_3C286.ms").absoluteName());

  MSMetaInfoForCal msm(mspath);

  if (msm.msOk()) {
    // Found MS in data repository

    // Shapes
    ASSERT_EQ(4,Int(msm.nAnt()));
    ASSERT_EQ(16,Int(msm.nSpw()));
    ASSERT_EQ(2,Int(msm.nFld()));
    
    // Basic MS info
    ASSERT_EQ(True,msm.msOk());
    
    //cout << "msm.msname()=" << msm.msname() << endl;
    //cout << "      mspath=" << mspath << endl;
    ASSERT_EQ(mspath,msm.msname());
    
    // Some names
    //  cout << "msm.antennaName(1)=" << msm.antennaName(1) << endl;
    ASSERT_EQ(String("ea11"),msm.antennaName(1));
    //  cout << "msm.fieldName(1)=" << msm.fieldName(1) << endl;
    ASSERT_EQ(String("3C286_A"),msm.fieldName(1));
    //  cout << "msm.spwName(0)=" << msm.spwName(5) << endl;
    ASSERT_EQ(String("Subband:5"),msm.spwName(5));

    Vector<String> antnames, fldnames;
    msm.antennaNames(antnames);
    msm.fieldNames(fldnames);
    //cout << "antnames = " << antnames << endl;
    //cout << "fldnames = " << fldnames << endl;
    ASSERT_EQ(uInt(4),antnames.nelements());
    ASSERT_EQ(String("ea01"),antnames[0]);
    ASSERT_EQ(String("ea11"),antnames[1]);
    ASSERT_EQ(String("ea19"),antnames[2]);
    ASSERT_EQ(String("ea24"),antnames[3]);
    ASSERT_EQ(uInt(2),fldnames.nelements());
    ASSERT_EQ(String("3C286_A"),fldnames[0]);
    ASSERT_EQ(String("3C286_A"),fldnames[1]);

    //cout << "msm.fieldIdAtTime(4793957198.) = " << msm.fieldIdAtTime(4793957198.0) << endl;
    ASSERT_EQ(1,msm.fieldIdAtTime(4793957190.0));  // near end of scan 30

    //cout << "msm.scanNumberAtTime(4793957190.0) = " << msm.scanNumberAtTime(4793957190.0) << endl;
    //cout << "msm.scanNumberAtTime(4793957200.0) = " << msm.scanNumberAtTime(4793957200.0) << endl;
    ASSERT_EQ(30,msm.scanNumberAtTime(4793957190.0));  // near end of scan 30
    ASSERT_EQ(31,msm.scanNumberAtTime(4793957200.0));  // near beg of scan 31

    /*
    cout << "msm.scanNumberAtTime(4793957198.0) = " << msm.scanNumberAtTime(4793957198.0) << endl;
    cout << "msm.scanNumberAtTime(4793957198.9) = " << msm.scanNumberAtTime(4793957198.9) << endl;
    cout << "msm.scanNumberAtTime(4793957196.0) = " << msm.scanNumberAtTime(4793957196.0) << endl;
    cout << "msm.scanNumberAtTime(4793957287.2999993) = " << msm.scanNumberAtTime(4793957287.2999993) << endl;
    cout << "msm.scanNumberAtTime(4793957287.4) = " << msm.scanNumberAtTime(4793957287.4) << endl;
    cout << "msm.scanNumberAtTime(4793957290.0) = " << msm.scanNumberAtTime(4793957290.0) << endl;
    */

  }


  else
    // Must not have full test DR
    cout << "Can't find real test MS: " << mspath << endl;

}

TEST(MSMetaInfoForCal, MSbyMS) {
  
  // Path to an MS in the DR
  String *casapath = new String[2];
  split(EnvironmentVariable::get("CASAPATH"), casapath, 2, String(" "));

  // Use of Path().absoluteName() absorbs relative stuff in casapath
  String mspath(Path(casapath[0]+"/data/regression/unittest/flagdata/Four_ants_3C286.ms").absoluteName());
  
  MeasurementSet ms(mspath);

  MSMetaInfoForCal msm(ms);

  // Shapes
  ASSERT_EQ(4,Int(msm.nAnt()));
  ASSERT_EQ(16,Int(msm.nSpw()));
  ASSERT_EQ(2,Int(msm.nFld()));
  
  // Basic MS info
  ASSERT_EQ(True,msm.msOk());

  //cout << "msm.msname()=" << msm.msname() << endl;
  //cout << "      mspath=" << mspath << endl;
  ASSERT_EQ(mspath,msm.msname());
  
  // Some names
  //  cout << "msm.antennaName(1)=" << msm.antennaName(1) << endl;
  ASSERT_EQ(String("ea11"),msm.antennaName(1));
  //  cout << "msm.fieldName(1)=" << msm.fieldName(1) << endl;
  ASSERT_EQ(String("3C286_A"),msm.fieldName(1));
  //  cout << "msm.spwName(0)=" << msm.spwName(5) << endl;
  ASSERT_EQ(String("Subband:5"),msm.spwName(5));

  Vector<String> antnames, fldnames;
  msm.antennaNames(antnames);
  msm.fieldNames(fldnames);
  //cout << "antnames = " << antnames << endl;
  //cout << "fldnames = " << fldnames << endl;
  ASSERT_EQ(uInt(4),antnames.nelements());
  ASSERT_EQ(String("ea01"),antnames[0]);
  ASSERT_EQ(String("ea11"),antnames[1]);
  ASSERT_EQ(String("ea19"),antnames[2]);
  ASSERT_EQ(String("ea24"),antnames[3]);
  ASSERT_EQ(uInt(2),fldnames.nelements());
  ASSERT_EQ(String("3C286_A"),fldnames[0]);
  ASSERT_EQ(String("3C286_A"),fldnames[1]);


  //cout << "msm.fieldIdAtTime(4793957198.) = " << msm.fieldIdAtTime(4793957198.0) << endl;
  ASSERT_EQ(1,msm.fieldIdAtTime(4793957190.0));  // near end of scan 30
  
  //cout << "msm.scanNumberAtTime(4793957190.0) = " << msm.scanNumberAtTime(4793957190.0) << endl;
  //cout << "msm.scanNumberAtTime(4793957200.0) = " << msm.scanNumberAtTime(4793957200.0) << endl;
  ASSERT_EQ(30,msm.scanNumberAtTime(4793957190.0));  // near end of scan 30
  ASSERT_EQ(31,msm.scanNumberAtTime(4793957200.0));  // near beg of scan 31

}




