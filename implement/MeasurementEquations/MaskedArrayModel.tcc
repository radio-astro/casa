//# MaskedArrayModel.cc:  this defines MaskedArrayModel
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

#include <synthesis/MeasurementEquations/MaskedArrayModel.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/LogiArray.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template<class T> MaskedArrayModel<T>::
MaskedArrayModel(){};

template<class T> MaskedArrayModel<T>::
MaskedArrayModel(const MaskedArray<T> & model):
  theModel(model.copy()) {}; 

template<class T> MaskedArrayModel<T>::
MaskedArrayModel(const Array<T> & model){
  LogicalArray mask(model.shape());
  mask = False;
  theModel.setData(model,mask);
}; 

template<class T> void MaskedArrayModel<T>::
getModel(Array<T> & model) const {
  model = theModel.getArray().copy();
};

template<class T> void MaskedArrayModel<T>::
getModel(MaskedArray<T> & model) const {
  model.setData(theModel.getArray(), theModel.getMask(), True);
};

template<class T>  MaskedArray<T> MaskedArrayModel<T>::
getModel() const {
  return MaskedArray<T>(theModel, True);
};

template<class T> void MaskedArrayModel<T>::
setModel(const MaskedArray<T> & model){
  theModel.setData(model);
};

template<class T> void MaskedArrayModel<T>::
setModel(const Array<T> & model){
  LogicalArray mask(model.shape());
  mask = False;
  theModel.setData(model, mask);
};

} //# NAMESPACE CASA - END

