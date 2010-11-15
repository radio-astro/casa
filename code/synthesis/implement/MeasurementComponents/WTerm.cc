//# WTerm.cc: implementation of WTerm
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
#include <synthesis/MeasurementComponents/WTerm.h>


namespace casa { //# NAMESPACE CASA - BEGIN
  
  void WTerm::apply(Matrix<Complex>& screen, const Int wPixel, 
		    const Vector<Double>& sampling,
		    const Int wConvSize, const Double wScale,
		    const Int inner)
  {
    Int convSize = screen.shape()(0);
    //    for (Int iw=0;iw<wConvSize;iw++) 
      {
	screen=0.0;
	if(wConvSize>1) 
	  {
	    Double twoPiW=2.0*C::pi*Double(wPixel*wPixel)/wScale;
	    for (Int iy=-inner/2;iy<inner/2;iy++) 
	      {
		Double m=sampling(1)*Double(iy);
		Double msq=m*m;
		for (Int ix=-inner/2;ix<inner/2;ix++) 
		  {
		    Double l=sampling(0)*Double(ix);
		    Double rsq=l*l+msq;
		    if(rsq<1.0) 
		      {
			Double phase=twoPiW*(sqrt(1.0-rsq)-1.0);
			screen(ix+convSize/2,iy+convSize/2)=Complex(cos(phase),sin(phase));
		      }
		  }
	      }
	  }
	else 
	  screen=1.0;
      }
  }

};
