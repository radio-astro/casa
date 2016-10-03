//# tViiLayerFactory.cc: Tests Recursive factory for layered VI2s
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

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casacore/casa/OS/EnvVar.h>
#include <casacore/casa/OS/Path.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casa/iostream.h>
#include <msvis/MSVis/IteratingParameters.h>
#include <msvis/MSVis/ViiLayerFactory.h>
#include <msvis/MSVis/LayeredVi2Factory.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/iomanip.h>
#include <gtest/gtest.h>

using namespace std;
using namespace casacore;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

 
TEST( ViiLayerFactoryTest , ViiLayerFactoryBasicTest ) {
 
  // A very rudimentary test of a single layer

  SimpleSimVi2Parameters s0;
  SimpleSimVi2LayerFactory fac(s0);

  Vector<ViiLayerFactory*> facts(1);
  facts[0]=&fac;

  VisibilityIterator2 *vi = new VisibilityIterator2(facts);
  VisBuffer2 *vb = vi->getImpl()->getVisBuffer();

  // Has a viable VI2 been generated?
  Int niter(0);
  for (vi->originChunks();vi->moreChunks();vi->nextChunk()) {
    for (vi->origin();vi->more();vi->next()) {
      ASSERT_EQ(4,vb->nAntennas());
      ++niter;
    }
  }
  ASSERT_EQ(1,niter);
}


TEST( ViiLayerFactoryTest , ViiLayerFactoryRealDataBasicTest ) {
 
  // A very rudimentary test of a single layer, using a real MS


  String *casapath = new String[2];
  split(EnvironmentVariable::get("CASAPATH"), casapath, 2, String(" "));
  // Use of Path().absoluteName() absorbs relative stuff in casapath
  String mspath(Path(casapath[0]+"/data/regression/unittest/flagdata/Four_ants_3C286.ms").absoluteName());

  MeasurementSet ms(mspath);

  Double interval(60000.0);
  IteratingParameters ipar(interval);   // seems to include SCAN_NUMBER automatically?
  VisIterImpl2LayerFactory fac(&ms,ipar,False); 

  Vector<ViiLayerFactory*> facts(1);
  facts[0]=&fac;

  VisibilityIterator2 *vi = new VisibilityIterator2(facts);
  VisBuffer2 *vb = vi->getImpl()->getVisBuffer();

  // Has a viable VI2 been generated?
  Int chunk(0),niter(0);
  for (vi->originChunks();vi->moreChunks();vi->nextChunk(),++chunk) {
    vi->origin();
    /*
    cout << "ch="<< chunk
	 << " scan="<<vb->scan()(0)
	 << " field="<< vb->fieldId()(0)
	 << " spw="<< vb->spectralWindows()(0)
	 << endl;
    */

    for (vi->origin();vi->more();vi->next(),++niter) {

      /*
      cout << "*************************************" << endl;
      cout << "chunk="<< chunk << ";  niter=" << niter << endl;

      cout << " scan=" << vb->scan()(0) << endl;
      cout << " fieldId=" << vb->fieldId()(0) << endl;

      cout << "nAntennas=" << vb->nAntennas() << endl;
      cout << "nRows=" << vb->nRows() << endl;
      cout << "nChannels=" << vb->nChannels() << endl;
      cout << "nCorrelations=" << vb->nCorrelations() << endl;
      cout << "spectralWindows=" << vb->spectralWindows() << endl;
      */

      ASSERT_EQ(4,vb->nAntennas());
      ASSERT_EQ(6,vb->nRows());
      ASSERT_EQ(64,vb->nChannels());  // all spws
      ASSERT_EQ(4,vb->nCorrelations());

    }
  }

  //cout << "chunk=" << chunk << endl;
  //  cout << "niter=" << niter << endl;

  ASSERT_EQ(32,chunk);
  ASSERT_EQ(2864,niter);
}
 
