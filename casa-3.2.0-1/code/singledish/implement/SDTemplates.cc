//#---------------------------------------------------------------------------
//# SDTemplates.cc: explicit templates for aips++
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------
#include <singledish/SDMemTable.h>

#include <casa/aips.h>
#include <casa/namespace.h>
#include <casa/Exceptions/Error.cc>
//#include <casa/Exceptions/Error2.cc>
#include <casa/Utilities/CountedPtr.cc>
//#include <casa/Utilities/CountedPtr2.cc>

namespace asap {
  template class casa::CountedConstPtr<SDMemTable>;
  template class casa::CountedPtr<SDMemTable>;
  template class casa::PtrRep<SDMemTable>;
  template class casa::SimpleCountedConstPtr<SDMemTable>;
  template class casa::SimpleCountedPtr<SDMemTable>;
}

#include <casa/Arrays/ArrayLogical.cc>
#include <casa/Arrays/ArrayMath.cc>
#include <casa/Arrays/MaskArrMath.cc>
#include <casa/Arrays/MaskedArray.cc>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Array.cc>
#include <casa/Arrays/Vector.h>
#include <casa/Utilities/BinarySearch.cc>
#include <coordinates/Coordinates/FrequencyAligner.cc>
#include <lattices/Lattices/Lattice.h>
#include <lattices/Lattices/LatticeUtilities.cc>
#include <scimath/Mathematics/InterpolateArray1D.cc>
#include <tables/Tables/BaseMappedArrayEngine.cc>

template void convertArray<Bool, uChar>(Array<Bool> &, Array<uChar> const &);
template void convertArray<uChar, Bool>(Array<uChar> &, Array<Bool> const &);

template Array<Float>& operator/=<Float>(Array<Float>&, MaskedArray<Float> const&);
template MaskedArray<Float> const& operator*=<Float>(MaskedArray<Float> const&, Float const&);
template MaskedArray<Float> const& operator*=<Float>(MaskedArray<Float> const&, Array<Float> const&);
template MaskedArray<Float> const& operator/=<Float>(MaskedArray<Float> const&, Float const&);
template MaskedArray<Float> operator+<Float>(MaskedArray<Float> const&, MaskedArray<Float> const&);
template MaskedArray<Float> operator-<Float>(MaskedArray<Float> const&, MaskedArray<Float> const&);
template MaskedArray<Float> operator-<Float>(MaskedArray<Float> const&, Array<Float> const&);

template MaskedArray<Float> operator/<Float>(MaskedArray<Float> const&, MaskedArray<Float> const&);

template MaskedArray<Float> operator*<Float>(MaskedArray<Float> const&, MaskedArray<Float> const&);
template MaskedArray<Float> operator*<Float>(MaskedArray<Float> const&, Array<Float> const&);
template MaskedArray<Float> operator*<Float>(Array<Float> const&, MaskedArray<Float> const&);
template Float stddev<Float>(MaskedArray<Float> const&);
template Float median<Float>(MaskedArray<Float> const&, Bool, Bool);
template Float sumsquares<Float>(MaskedArray<Float> const&);
template Float avdev<Float>(MaskedArray<Float> const&);

template void LatticeUtilities::bin(MaskedArray<float>&, MaskedArray<float> const&, uInt, uInt);

template class FrequencyAligner<Float>;

template class BaseMappedArrayEngine<Float, Float>;

#include <singledish/MathUtils2.cc>
#include <singledish/SDPol2.cc>
namespace mathutil {
  template void hanning(Vector<Float>&, Vector<Bool>&,
			const Vector<Float>&, 
			const Vector<Bool>&, 
			Bool, Bool);
  template uInt addEntry(Vector<uInt>&, uInt);
  template void extendLastArrayAxis(Array<Int>&, const Array<Int>&,
				   const Int& initVal);
}
template Array<Bool> SDPolUtil::stokesData (Array<Bool>& dataIn, Bool);
template Array<Float> SDPolUtil::computeStokesDataForWriter(Array<Float>& dataIn, Bool);
template Array<uChar> SDPolUtil::computeStokesDataForWriter(Array<uChar>& dataIn, Bool);

