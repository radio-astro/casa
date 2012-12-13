//# LattPADMVector2.cc: Specializations for LatticePADMVector
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
//# $Id: 

#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>

#include <display/DisplayDatas/LatticePADMVector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template <>
Matrix<Float> LatticePADMVector<Complex>::getAmplitude(const Matrix<Complex>& data) const
{
   return phase(data);
}

template <>
Matrix<Float> LatticePADMVector<Float>::getAmplitude(const Matrix<Float>& data) const
//
// Never called
//
{
   Matrix<Float> t;
   return t;
}

} //# NAMESPACE CASA - END

