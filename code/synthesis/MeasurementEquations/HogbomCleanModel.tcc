//# HogbomCleanModel.cc:  this defines HogbomCleanModel
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

#include <synthesis/MeasurementEquations/HogbomCleanModel.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template<class T> Bool HogbomCleanModel<T>::
solve(ResidualEquation<Array<T> > & eqn){
   Array<T> res;
   T minVal, maxVal;
   IPosition minPos(theModel.ndim(), 0), maxPos(theModel.ndim(), 0);
   for (uInt n = 0; n < numberIterations(); n++){
     eqn.residual(res, *this); 
     minMax(minVal, maxVal, minPos, maxPos, res);
     if (abs(minVal) > abs(maxVal)){ // select the max residual even if it 
       maxVal = minVal;              // is negative.
       maxPos = minPos;
     }
     theModel(maxPos) += gain()*maxVal;
   }
   return True;
}


template<class T> Bool HogbomCleanModel<T>::
solve(ConvolutionEquation & eqn){
  Array<T> residual;
  T minVal, maxVal;
  IPosition minPos(theModel.ndim(), 0), maxPos(theModel.ndim(), 0);
  eqn.residual(residual, *this); 
  Array<Float> psf(residual.shape());
  T amplitude;
  uInt n = 0;
  minMax(minVal, maxVal, minPos, maxPos, residual); 
  if (abs(minVal) > abs(maxVal)){
    maxVal = minVal; 
    maxPos = minPos;
  }
  while ((n < numberIterations()) && (abs(maxVal) > threshold())){
    n++;
    if (eqn.evaluate(psf, maxPos, 1.0f, residual.shape())){
      amplitude = gain()*maxVal;
      theModel(maxPos) += amplitude; 
      residual -= psf*amplitude;
    }
    minMax(minVal, maxVal, minPos, maxPos, residual);
    if (abs(minVal) > abs(maxVal)){
      maxVal = minVal; 
      maxPos = minPos;
    }
  }
  setNumberIterations(n);
  setThreshold(abs(maxVal));
  return True;
};


} //# NAMESPACE CASA - END

