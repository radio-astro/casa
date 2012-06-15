//# tVisibilityIterator.cc: Tests the Synthesis MeasurementSet Iterator
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

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <synthesis/MSVis/VisSet.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <casa/OS/Timer.h>
#include <casa/iomanip.h>
#include <casa/namespace.h>
#include <synthesis/MSVis/VisModelData.h>
using namespace std;

int main(int argc, char **argv) {
  Timer timer;
  for(Int i=0;i<20;i++) cout<<endl;  cout<<"Start."<<endl;
  try {
    if (argc < 2) throw(AipsError("no table given on the command line"));


    MS ms(argv[1], Table::Update);

    Block<int> sort(4);
    sort[2] = MS::FIELD_ID;
    sort[3] = MS::ARRAY_ID;
    sort[1] = MS::DATA_DESC_ID;
    sort[0] = MS::TIME;
//    sort[4] = MS::ANTENNA1;	//these _really_ slow things down. (fctr 100)
//    sort[5] = MS::ANTENNA2;   //cause 1 row per chunk or iter
    Matrix<Int> allChannels;
    Double intrvl=2;
    VisSet vs(ms,sort,allChannels,intrvl);
    VisIter& vi(vs.iter());
//    VisIter vi(ms,sort);
    VisBuffer vb(vi);

    Int iRow=0, iTime=-1, iIter=0, iChunk=0,iAnt1=-1,iAnt2=-1;
    Double oldTime=-1., time0; Vector<Double> curTime;
    Complex xxx;
    Int row, chn, pol;
    Bool newTime;
    Vector<Int> ant1, ant2;   Int a1, a2, oldAnt1=-1,oldAnt2=-1,fd1,fd2;
    Vector<Int> feed1,feed2;  Int oldFeed1=-1,oldFeed2=-1,iFeed1=-1,iFeed2=-1;

    timer.show(" setup: "); timer.mark();

    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin();vi.more();vi++) {
	
          iRow++;  }
        iIter++;  }

    // 0 changes means the same value for the whole dataset
    cout<<endl<<"TOTALS: Chunks: "<<iChunk<<"  Time chgs: "<<iTime<<"  Iters: "<<iIter <<endl;
} 
  catch (const AipsError &x) {
    cerr << "***Exception:" << endl;
    cerr << x.getMesg() << endl;
  }
  
  timer.show("Done.  "); cout<<endl<<endl<<endl;
  return 0;  }

