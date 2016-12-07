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

#ifndef SYNTHESIS_MC_TEST_VISCALTESTBASE_H
#define SYNTHESIS_MC_TEST_VISCALTESTBASE_H

#include <casa/Arrays/Array.h>
//#include <casa/Arrays/ArrayMath.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>

#include <synthesis/MeasurementComponents/MSMetaInfoForCal.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/VisBuffer2.h>

#include <gtest/gtest.h>

// <summary>
// Base class for VisCal GoogleTesting
// </summary>

class VisCalTestBase : public ::testing::Test {

public:

 VisCalTestBase(int nFld_=1,int nScan_=1,int nSpw_=1,
		int nAnt_=4,int nCorr_=4,int nChan_=8,
		int nTime_=1,bool doParang=false) :
    nFld(nFld_),nScan(nScan_),nSpw(nSpw_),
    nAnt(nAnt_),nCorr(nCorr_),nChan(nChan_),nTime(nTime_),
    ssvp(nFld,nScan,nSpw,nAnt,nCorr,
	 casacore::Vector<int>(nFld,nTime),
	 casacore::Vector<int>(nSpw,nChan),casacore::Complex(1.0),
	 doParang),  // turns on poln
    ssf(ssvp),
    vi2(casacore::Vector<casa::vi::ViiLayerFactory*>(1,&ssf)),
    vb2(vi2.getImpl()->getVisBuffer()),
    msmc(ssvp)
  {}
  
  void summary(casacore::String name) {
    cout << name << ":" << endl
	 << " nFld =" << nFld << endl
	 << " nScan=" << nScan << endl
	 << " nAnt =" << nAnt << endl
	 << " nSpw =" << nSpw << endl
	 << " nChan=" << nChan << endl
	 << " nCorr=" << nCorr << endl
	 << " nTime=" << nTime << endl;
  }

  int nFld,nScan,nSpw,nAnt,nCorr,nChan,nTime;

  casa::vi::SimpleSimVi2Parameters ssvp;
  casa::vi::SimpleSimVi2LayerFactory ssf;
  casa::vi::VisibilityIterator2 vi2;
  casa::vi::VisBuffer2* vb2;

  casa::MSMetaInfoForCal msmc;

};

#endif  // SYNTHESIS_MC_TEST_VISCALTESTBASE_H
