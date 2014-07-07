//# tUVSub.cc: Tests creation of a residual data set by subtraction of
//#            the field model from the corrected data.
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
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <casa/OS/Timer.h>
#include <casa/iomanip.h>
#include <casa/namespace.h>
using namespace std;

int main(int argc, char **argv) {

  try {
    if (argc < 2) throw(AipsError("no table given on the command line"));

    MS ms(argv[1], Table::Update);

    Block<int> sort(4);
    sort[2] = MS::FIELD_ID;
    sort[3] = MS::ARRAY_ID;
    sort[1] = MS::DATA_DESC_ID;
    sort[0] = MS::TIME;
    Matrix<Int> allChannels;
    Double intrvl;

    VisSet vs(ms,sort,allChannels,intrvl);
    VisIter& vi(vs.iter());
    VisBuffer vb(vi);

    Int row, chn, pol;

    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin();vi.more();vi++) {

	Cube<Complex>& vc= vb.correctedVisCube();
	Cube<Complex>& mc= vb.modelVisCube();

	Int nRow=vb.nRow();
	Int nChan=vb.nChannel();
        Int nPol=vi.visibilityShape()(0);
	Cube<Complex> residualData(nPol,nChan,nRow);
	for (row=0; row<nRow; row++) {
	  for (pol=0; pol<nPol; pol++) {
	    for (chn=0; chn<nChan; chn++) {
	      residualData(pol,chn,row) = vc(pol,chn,row)-mc(pol,chn ,row);
	    }
	  }
	}
	vi.setVis(residualData,VisibilityIterator::Corrected);
      }
    }
  }
  catch (const AipsError &x) {
    cerr << "***Exception:" << endl;
    cerr << x.getMesg() << endl;
  }
  
  return 0;
}

