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
#include <casa/iostream.h>
#include <msvis/MSVis/ViiLayerFactory.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/iomanip.h>
#include <gtest/gtest.h>

using namespace std;
using namespace casa;
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
 
