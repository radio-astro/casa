//# tVisCalTestBase_GT: Base class for VisCal google testing
//# Copyright (C) 2013
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

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/namespace.h>

#include <synthesis/MeasurementComponents/DJones.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
#include <synthesis/MeasurementComponents/MSMetaInfoForCal.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/VisBuffer2.h>

#include <gtest/gtest.h>

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

// <summary>
// Test program for KJones-related classes
// </summary>

class VisCalTestBase : public ::testing::Test {

public:

  VisCalTestBase(Int nFld_=1,Int nScan_=1,Int nSpw_=1,
		 Int nAnt_=4,Int nCorr_=4,Int nChan_=8,
		 Int nTime_=1,Bool doParang=false) :
    nFld(nFld_),nScan(nScan_),nSpw(nSpw_),
    nAnt(nAnt_),nCorr(nCorr_),nChan(nChan_),nTime(nTime_),
    ssvp(nFld,nScan,nSpw,nAnt,nCorr,
	 Vector<Int>(nFld,nTime),
	 Vector<Int>(nSpw,nChan),Complex(1.0),
	 doParang),  // turns on poln
    ssf(ssvp),
    vi2(Vector<ViiLayerFactory*>(1,&ssf)),
    vb2(vi2.getImpl()->getVisBuffer()),
    msmc(ssvp)
  {}
  
  void summary(String name) {
    cout << name << ":" << endl
	 << " nFld =" << nFld << endl
	 << " nScan=" << nScan << endl
	 << " nAnt =" << nAnt << endl
	 << " nSpw =" << nSpw << endl
	 << " nChan=" << nChan << endl
	 << " nCorr=" << nCorr << endl
	 << " nTime=" << nTime << endl;
  }

  Int nFld,nScan,nSpw,nAnt,nCorr,nChan,nTime;

  SimpleSimVi2Parameters ssvp;
  SimpleSimVi2LayerFactory ssf;
  VisibilityIterator2 vi2;
  VisBuffer2* vb2;

  MSMetaInfoForCal msmc;

};

