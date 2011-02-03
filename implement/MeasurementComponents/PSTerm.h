//# PSTerm.h: Definition for PSTerm
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

#ifndef SYNTHESIS_PSTERM_H
#define SYNTHESIS_PSTERM_H


#include <casa/Arrays/Vector.h>
#include <msvis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <synthesis/MeasurementComponents/CFStore.h>

namespace casa{
  // <summary>  A class to apply the W-Term to the given image </summary>
  
  // <use visibility=export>
  // <prerequisite>
  // </prerequisite>
  // <etymology>
  // W-Term to account for non co-planar baselines
  // </etymology>
  //
  // <synopsis> 
  // 
  //</synopsis>
  class PSTerm
  {
  public:
    PSTerm (const IPosition shape, 
	    const Vector<Double>& uvScale,
	    const Vector<Double>& uvOffset):
      psCtor_p() 
    {init(shape,uvScale,uvOffset);};
    ~PSTerm () {};

    void init(const IPosition shape, 
	      const Vector<Double>& uvScale,
	      const Vector<Double>& uvOffset);
    void applySky(Matrix<Complex>& screen, 
		  const Int wPixel, 
		  const Vector<Double>& sampling,
		  const Double wScale,
		  const Int inner);
    void normalizeImage(Lattice<Complex>& skyImage,
			const Matrix<Float>& weights);

  private:
    // spheroidal function
    CountedPtr<ConvolveGridder<Double, Complex> > psCtor_p;
    Vector<Double>  support_p;
    Vector<Int> sampling_p;
  };

};

#endif
