//# StokesUtil.cc:  
//# Copyright (C) 1996,1999
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/aips.h>
#include <synthesis/MeasurementEquations/StokesUtil.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template<class T, class U> void
unpackStokes(Array<U>& output, const Array<T>& input){
  // Create space for the output array
  {
    IPosition outSize(input.shape());
    outSize.append(IPosition(1,4));
    output.resize(outSize);
  }
  // Create a reference to the arrays that allows the data to be accessed
  // linearly
  uInt nelements = input.nelements();
  Vector<T> linput(input.reform(IPosition(1,nelements), IPosition(1,0)));
  Vector<U> loutput(output.reform(IPosition(1,output.nelements()), 
				  IPosition(1,0)));
  // Now copy the data
  uInt j = 0;
  for (uInt pol = 0; pol < 4; pol++){
    for (uInt i = 0; i < nelements; i++){
      loutput(j++) = linput(i)(pol);
    }
  }
};

template<class T, class U> void
repackStokes(Array<U>& output, const Array<T>& input){
  // Find which axis is the stokes axis
  uInt stokesAxis;
  IPosition inputSize = input.shape();
  // First remove any trailing degenerate axes
  stokesAxis = inputSize.nelements();
  while ((stokesAxis > 0) && (inputSize(stokesAxis-1) == 1))
    stokesAxis--; 
  // Then check that the input array has a slowest dimension of 4
  if (inputSize(stokesAxis-1) != 4) throw(AipsError(
     "StokesVector::repackStokes() last dimension does not have 4 elements"));
  // resize the output array to suit (include trailing axes)
  {
    IPosition outSize = inputSize.getFirst(stokesAxis-1);
    if (stokesAxis < input.ndim()){
      outSize.append(inputSize.getLast(input.ndim() - stokesAxis));
    }
    output.resize(outSize);
  }
  
  // Create a reference to the arrays that allows the data to be accessed 
  // linearly
  Vector<T> linput(input.reform(IPosition(1,input.nelements()), 
				IPosition(1,0)));
  Vector<U> loutput(output.reform(IPosition(1,output.nelements()), 
				  IPosition(1,0)));
  // Now copy the data
  uInt nelements = output.nelements();
  uInt j = 0;
  for (uInt pol = 0; pol < 4; pol++){
    for (uInt i = 0; i < nelements; i++){
      loutput(i)(pol) = linput(j++);
    }
  }
};

} //# NAMESPACE CASA - END

