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
//# $Id

//# Includes

#include <nrao/GBTFillers/GBTPolarization.h>

#include <casa/Exceptions/Error.h>
#include <measures/Measures/Stokes.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

GBTPolarization::GBTPolarization()
    : itsNumCorr(1), itsCorrType(1, Stokes::XX), itsSamplerRows(1,-1),
      itsCorrProduct(1,1,0), itsCorrMap(0), itsIsDefault(True)
{;}

GBTPolarization::GBTPolarization(Int numCorr, const Vector<String> polType)
    : itsNumCorr(numCorr), itsCorrType(numCorr), itsSamplerRows(numCorr,-1),
      itsCorrProduct(2, numCorr), itsCorrMap(0, numCorr),
      itsIsDefault(False)
{
    DebugAssert(((numCorr==1 || numCorr==2) && Int(polType.nelements())==numCorr) ||
		(numCorr==4 && polType.nelements()==2),
		AipsError);

    if (numCorr==4) {
	Vector<Int> cprod(2);
	Int icorr = 0;
	for (Int i=0;i<2;i++) {
	    for (Int j=0;j<2;j++) {
		String thisCorr = polType[i]+polType[j];
		Int s = Stokes::type(thisCorr);
		itsCorrType[icorr] = s;
		cprod[0] = i;
		cprod[1] = j;
		itsCorrProduct.column(icorr) = cprod;
		itsCorrMap(s) = icorr;
		icorr++;
	    }
	}
    } else {
	// just the auto products
	Vector<Int> cprod(2);
	for (Int i=0;i<numCorr;i++) {
	    String thisCorr = polType[i]+polType[i];
	    Int s = Stokes::type(thisCorr);
	    itsCorrType[i] = s;
	    cprod = i;
	    itsCorrProduct.column(i) = cprod;
	    itsCorrMap(s) = i;
	}
    }
}

GBTPolarization::GBTPolarization(const GBTPolarization &other)
    : itsNumCorr(other.itsNumCorr), itsCorrType(itsNumCorr),
      itsSamplerRows(itsNumCorr), itsCorrProduct(2,itsNumCorr), 
      itsCorrMap(other.itsCorrMap), itsIsDefault(other.itsIsDefault)
{
    itsCorrType = other.itsCorrType;
    itsSamplerRows = other.itsSamplerRows;
    itsCorrProduct = other.itsCorrProduct;
}

GBTPolarization &GBTPolarization::operator=(const GBTPolarization &other)
{
    if (this != &other) {
	itsNumCorr = other.itsNumCorr;
	itsCorrType.resize(itsNumCorr);
	itsCorrType = other.itsCorrType;
	itsSamplerRows.resize(itsNumCorr);
	itsSamplerRows = other.itsSamplerRows;
	itsCorrProduct.resize(2,itsNumCorr);
	itsCorrProduct = other.itsCorrProduct;
	itsCorrMap = other.itsCorrMap;
	itsIsDefault = other.itsIsDefault;
    }
    return *this;
}

Bool GBTPolarization::samplerInfo(Int samplerRow,
				  const String &polA, const String &polB)
{
    Bool result = False;
    if (itsIsDefault) {
	result = True;
	itsSamplerRows[0] = samplerRow;
    } else {
	String thisCor = polA + polB;
	Int s = Stokes::type(thisCor);
	result = itsCorrMap.isDefined(s);
	if (result) {
	    itsSamplerRows[itsCorrMap(s)] = samplerRow;
	}
    }
    return result;
}
				  
