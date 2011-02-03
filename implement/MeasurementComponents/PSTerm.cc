//# PSTerm.cc: implementation of PSTerm
//# Copyright (C) 2007
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$
#include <synthesis/MeasurementComponents/PSTerm.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  //
  //-------------------------------------------------------------------------
  //	       
  void PSTerm::init(const IPosition shape, 
		    const Vector<Double>& uvScale,
		    const Vector<Double>& uvOffset)
  {
    psCtor_p   = new ConvolveGridder<Double, Complex>(shape, uvScale, uvOffset, "SF");
    sampling_p = (*psCtor_p).cSupport();
    support_p  = (*psCtor_p).cSampling();
  }
  //
  //-------------------------------------------------------------------------
  //	       
  void PSTerm::applySky(Matrix<Complex>& screen, 
			const Int wPixel, 
			const Vector<Double>& sampling,
			const Double wScale,
			const Int inner)
  {
    AlwaysAssert(psCtor_p.null()==False, SynthesisFTMachineError);

    Int convSize = screen.shape()[0];
    Vector<Complex> correction(inner);
    for (Int iy=-inner/2;iy<inner/2;iy++) {
      psCtor_p->correctX1D(correction, iy+inner/2);
      for (Int ix=-inner/2;ix<inner/2;ix++) {
	screen(ix+convSize/2,iy+convSize/2)*=correction(ix+inner/2);
      }
    }
    (void)wPixel; (void)sampling; (void)wScale;
  }
  //
  //-------------------------------------------------------------------------
  //	       
  void PSTerm::normalizeImage(Lattice<Complex>& skyImage,
			      const Matrix<Float>& weights)
  {
    Int inx = skyImage.shape()(0);
    Int iny = skyImage.shape()(1);
    Vector<Complex> correction(inx);
    correction=Complex(1.0, 0.0);
    IPosition cursorShape(4, inx, 1, 1, 1);
    IPosition axisPath(4, 0, 1, 2, 3);
    LatticeStepper lsx(skyImage.shape(), cursorShape, axisPath);
    LatticeIterator<Complex> lix(skyImage, lsx);

    for(lix.reset();!lix.atEnd();lix++) 
      {
	Int pol=lix.position()(2), chan=lix.position()(3);

	if(weights(pol, chan)!=0.0) 
	  {
	    Int iy=lix.position()(1);
	    psCtor_p->correctX1D(correction,iy);
	    // for (Int ix=0;ix<inx;ix++) 
	    //   correction(ix)*=sincConv(ix)*sincConv(iy);

	    lix.rwVectorCursor()/=correction;
	  }
	else 
	  lix.woCursor()=0.0;
      }
  }
};
