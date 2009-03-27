//# GBTCorrelation.cc: GBTCorrelation holds spw and pol info
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
//# $Id$

//# Includes

#include <nrao/GBTFillers/GBTCorrelation.h>

#include <nrao/GBTFillers/GBTFeed.h>

#include <casa/Arrays/ArrayLogical.h>
#include <measures/Measures/Stokes.h>
#include <casa/namespace.h>

GBTCorrelation::GBTCorrelation(Int nstate, Int nchan) 
    : itsNstate(nstate), itsPolId(-1), itsNumCorr(1), itsNumPol(1),
      itsFrozen(False), itsIsDefault(True), itsBank(""), itsSrbank(""),
      itsShape(2,1,nchan), itsDataDescId(nstate,-1),
      itsRecptMap(0), itsCorrType(1, Stokes::XX), itsSamplerRows(1,-1),
      itsCorrProduct(2,1,0), itsReceptors(1,""), itsCorrTypeString(1, "X")
{;}

GBTCorrelation::GBTCorrelation(const GBTFeed &feed, Int nstate, Int nchan,
			       const String &bank)
    : itsNstate(nstate), itsPolId(-1),
      itsFrozen(False), itsIsDefault(False), itsBank(bank), itsSrbank(""),
      itsShape(2,1,nchan), itsDataDescId(nstate,-1), itsRecptMap(0)
{
    // digest the feed.polType() vector into itsRecptMap
    Vector<String> pols(feed.polType());
    for (uInt i=0;i<pols.nelements();i++) {
	itsRecptMap(pols[i]) = i;
    }
    // set things for all possible correlations
    itsNumPol = pols.nelements();
    itsNumCorr = itsNumPol*itsNumPol;
    itsCorrType.resize(itsNumCorr);
    itsCorrTypeString.resize(itsNumCorr);
    itsSamplerRows.resize(itsNumCorr);
    itsCorrProduct.resize(2,itsNumCorr);
    itsReceptors.resize(itsNumCorr);
    Int elem = 0;
    for (uInt i=0;i<itsNumPol;i++) {
	for (uInt j=0;j<itsNumPol;j++) {
	    itsCorrType[elem] = Stokes::type(pols[i]+pols[j]);
	    if (pols[i] == pols[j]) {
		itsCorrTypeString[elem] = pols[i];
	    } else {
		itsCorrTypeString[elem] = pols[i]+pols[j];
	    }
	    itsCorrProduct(0,elem) = i;
	    itsCorrProduct(1,elem) = j;
	    elem++;
	}
    }
    itsSamplerRows = -1;
    itsReceptors = "";
}

GBTCorrelation::GBTCorrelation(const GBTCorrelation &other) 
    : itsNstate(other.itsNstate), itsPolId(other.itsPolId),
      itsNumCorr(other.itsNumCorr), itsNumPol(other.itsNumPol),
      itsFrozen(other.itsFrozen),
      itsIsDefault(other.itsIsDefault),
      itsBank(other.itsBank), itsSrbank(other.itsSrbank),
      itsShape(other.itsShape),
      itsDataDescId(other.itsDataDescId.nelements()),
      itsRecptMap(other.itsRecptMap),
      itsCorrType(other.itsCorrType.nelements()),
      itsSamplerRows(other.itsSamplerRows.nelements()),
      itsCorrProduct(other.itsCorrProduct.shape()),
      itsReceptors(other.itsReceptors.shape()),
      itsCorrTypeString(other.itsCorrTypeString.nelements())
{
    itsDataDescId = other.itsDataDescId;
    itsCorrType = other.itsCorrType;
    itsCorrTypeString = other.itsCorrTypeString;
    itsSamplerRows = other.itsSamplerRows;
    itsCorrProduct = other.itsCorrProduct;
    itsReceptors = other.itsReceptors;
}

GBTCorrelation &GBTCorrelation::operator=(const GBTCorrelation &other)
{
    if (this != &other) {
	itsNstate = other.itsNstate;
	itsPolId = other.itsPolId;
	itsNumCorr = other.itsNumCorr;
	itsNumPol = other.itsNumPol;
	itsFrozen = other.itsFrozen;
	itsIsDefault = other.itsIsDefault;
	itsBank = other.itsBank;
	itsSrbank = other.itsSrbank;
	itsShape = other.itsShape;
	itsDataDescId.resize(other.itsDataDescId.nelements());
	itsDataDescId = other.itsDataDescId;
	itsRecptMap.clear();
	for (uInt i=0;i<other.itsRecptMap.ndefined();i++) {
	    itsRecptMap(other.itsRecptMap.getKey(i)) = other.itsRecptMap.getVal(i);
	}
	itsCorrType.resize(other.itsCorrType.nelements());
	itsCorrType = other.itsCorrType;
	itsCorrTypeString.resize(other.itsCorrTypeString.nelements());
	itsCorrTypeString = other.itsCorrTypeString;
	itsSamplerRows.resize(other.itsSamplerRows.nelements());
	itsSamplerRows = other.itsSamplerRows;
	itsCorrProduct.resize(other.itsCorrProduct.shape());
	itsCorrProduct = other.itsCorrProduct;
	itsReceptors.resize(other.itsReceptors.nelements());
	itsReceptors = other.itsReceptors;
    }
    return *this;
}

Bool GBTCorrelation::setSamplerRow(Int samplerRow,
				   const String &polA, const String &polB,
				   const String &receptorA, const String &receptorB)
{
    Bool result = !itsFrozen;
    if (result) {
	if (itsIsDefault) {
	    // default case, actual polA and polB don't matter
	    if (itsSamplerRows[0] < 0) {
		itsSamplerRows[0] = samplerRow;
		// itsReceptor[0] will already be set to the empty string
	    } else {
		// already set
		result = False;
	    }
	} else {
	    Int recA = itsRecptMap(polA);
	    Int recB = itsRecptMap(polB);
	    result = !((recA < 0) || (recB < 0));
	    if (result) {
		Int indx = recA*itsNumPol + recB;
		if (itsSamplerRows[indx] < 0) {
		    itsSamplerRows[indx] = samplerRow;
		    if (receptorA == receptorB) {
			itsReceptors[indx] = receptorA;
		    } else {
			itsReceptors[indx] = receptorA + "x" + receptorB;
		    }
		} else {
		    // already set
		    result = False;
		}
	    } // else this isn't found in the supplied feed
	}
    }
    return result;
}

Bool GBTCorrelation::mergeCheck(const GBTCorrelation &otherCorr,
				Bool bankCheck)
{
    Bool result = itsNstate == otherCorr.itsNstate;
    result = result && !itsFrozen && !otherCorr.itsFrozen;
    if (bankCheck) {
	result = result && itsBank == otherCorr.itsBank;
    }
    result = result && itsSamplerRows.nelements() == otherCorr.itsSamplerRows.nelements();
    if (result) {
	// make sure wherever this is set, it isn't already set in the other
	for (uInt i=0;i<otherCorr.itsSamplerRows.nelements();i++) {
	    if (otherCorr.itsSamplerRows[i] >= 0) {
		result = itsSamplerRows[i] < 0;
		if (!result) break;
	    }
	}
    }
    return result;
}

Bool GBTCorrelation::mergeCorr(const GBTCorrelation &otherCorr,
			       Bool bankCheck)
{
    Bool result = mergeCheck(otherCorr, bankCheck);
    if (result) {
	for (uInt i=0;i<otherCorr.itsSamplerRows.nelements();i++) {
	    if (otherCorr.itsSamplerRows[i] >= 0) {
		itsSamplerRows[i] = otherCorr.itsSamplerRows[i];
		itsReceptors[i] = otherCorr.itsReceptors[i];
	    }
	}
    }
    return result;
}

void GBTCorrelation::freeze()
{
    // don't freeze if already frozen
    if (!itsFrozen) {
	// only keep those with samplerRows >= 0
	uInt oldSize = itsNumCorr;
	itsNumCorr = 0;
	for (uInt i=0;i<itsSamplerRows.nelements();i++) {
	    if (itsSamplerRows[i] >= 0) {
		// valid but don't move anything unless its out of step
		if (itsNumCorr != i) {
		    itsSamplerRows[itsNumCorr] = itsSamplerRows[i];
		    itsCorrType[itsNumCorr] = itsCorrType[i];
		    itsCorrTypeString[itsNumCorr] = itsCorrTypeString[i];
		    itsCorrProduct.column(itsNumCorr) = itsCorrProduct.column(i);
		    itsReceptors[itsNumCorr] = itsReceptors[i];
		}
		itsNumCorr++;
	    }
	}
	if (itsNumCorr != oldSize) {
	    // need to resize
	    itsCorrType.resize(itsNumCorr, True);
	    itsCorrTypeString.resize(itsNumCorr, True);
	    itsSamplerRows.resize(itsNumCorr, True);
	    Vector<Int> cp0, cp1;
	    cp0 = itsCorrProduct.row(0);
	    cp1 = itsCorrProduct.row(1);
	    cp0.resize(itsNumCorr, True);
	    cp1.resize(itsNumCorr, True);
	    itsCorrProduct.resize(2, itsNumCorr);
	    itsCorrProduct.row(0) = cp0;
	    itsCorrProduct.row(1) = cp1;
	    itsReceptors.resize(itsNumCorr, True);
	}
	itsShape[0] = itsNumCorr;
	itsFrozen = True;
    }
}

Bool GBTCorrelation::operator==(const GBTCorrelation &other) const
{
    Bool result = True;
    if (this != &other) {
	result = (other.shape() == shape()) &&
	    (other.nstate() == nstate()) &&
	    (other.numCorr() == numCorr()) &&
	    allEQ(other.corrType(),corrType()) &&
	    allEQ(other.corrProduct(),corrProduct()) &&
	    allEQ(other.receptors(),receptors());
    }
    return result;
}

// These must be defined here because templates need things to be in the casa namespace!

#include <casa/Utilities/Copy.tcc>
template void objcopy<GBTCorrelation *>(GBTCorrelation **, GBTCorrelation * const *, uInt);
template void objset<GBTCorrelation *>(GBTCorrelation **, GBTCorrelation *, uInt);

