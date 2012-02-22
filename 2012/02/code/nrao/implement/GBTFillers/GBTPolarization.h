//# GBTPolarization: GBTPolarization holds pol. info for a correlation set.
//# Copyright (C) 2003
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
//#
//# $Id$

#ifndef NRAO_GBTPOLARIZATION_H
#define NRAO_GBTPOLARIZATION_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/SimOrdMap.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// GBTPolarization holds polarization info for a correlation set.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tGBTPolarization" demos="">
// </reviewed>

// <prerequisite>
//   <li> The POLARIZATION table of the MeasurementSet.
//   <li> The GBT FITS files.
//   <li> The GBT filler.
// </prerequisite>
//
// <etymology>
// This holds information appropriate for a row of the POLARIZATION table of
// a MS plus information useful in associated a specific correlation
// with a specific sampler in a GBT backend FITS file (and from there,
// to an appropriate row in the IF table and a specific feed).  Hence
// this holds GBT-specific polarization information.
// </etymology>
//
// <motivation>
// When this information was being held by a larger class, it was getting
// confused with other issues related to that class.  By putting it in a
// smaller, simpler class the maintenance will be easier and usage will
// be less confusing.
// </motivation>

class GBTPolarization
{
public:
    // The default correlation.  NUM_CORR=1 and CORR_TYPE[0]=code for "X".
    GBTPolarization();

    // A specific correlation from vector of receptor polarization and
    // a specific NUM_CORR.  If NUM_CORR == 1 or 2 only the self-products
    // are used and polType.nelements() should equal numCorr.  If NUM_CORR==4
    // then polType.nelements() should equal 2 and cross-products will
    // also be used.  NUM_CORR==3 should never be used.  For efficiency,
    // this code only checks those values using DebugAssert and hence
    // are only done if AIPS_DEBUG was defined at compile time. 
    GBTPolarization(Int numCorr, const Vector<String> polType);

    // copy constructor
    GBTPolarization(const GBTPolarization &other);

    ~GBTPolarization() {;}

    // Assignment operator, uses copy syntax.
    GBTPolarization &operator=(const GBTPolarization &other);

    // Set up information about a given sampler row.
    // Returns False if the implied element in polType doesn't exist.
    // samplerInfo always returns True if the default constructor was
    // used.  In that case polA and polB always combine to map to
    // the single correlation of "XX".
    Bool samplerInfo(Int samplerRow,
		     const String &polA, const String &polB);

    // the number of correlations
    Int numCorr() const {return itsNumCorr;}

    // The polarization of the numCorr correlations
    const Vector<Int> &corrType() const {return itsCorrType;}

    // The receptor cross-products.
    const Matrix<Int> &corrProduct() const {return itsCorrProduct;}
    
    // The samplerRows associated with corrType.  An element is -1
    // if no information has been provided yet.
    const Vector<Int> samplerRows() const {return itsSamplerRows;}
private:
    Int itsNumCorr;
    Vector<Int> itsCorrType, itsSamplerRows;
    Matrix<Int> itsCorrProduct;

    //# facilitates lookup from corr code to element in corrProduct
    SimpleOrderedMap<Int, Int> itsCorrMap;

    Bool itsIsDefault;
};
		     
#endif
