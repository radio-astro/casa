//# GBTFeed.cc:  this holds information about a specific feed with IFs
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

#include <nrao/GBTFillers/GBTFeed.h>

#include <nrao/GBTFillers/GBTIF.h>

#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Utilities/Assert.h>

GBTFeed::GBTFeed()
    : itsFeedId(-1), itsSRFeedId(-1), itsName("-1"), itsTrckbeam("-1"), itsRxName(""),
      itsSRFeedPtr(0), itsNif(0), itsIFPtr(1), itsFrozen(False), itsPolType(4), 
      itsRecptNames(4), itsXeloffset(0), itsEloffset(0), itsFitsFeedId(-1), 
      itsSRFitsFeedId(-1)
{;}

GBTFeed::GBTFeed(Double xeloffset, Double eloffset, const String &feed,
		 Int srfeed1, Int srfeed2, const String &trckbeam)
    : itsFeedId(-1), itsSRFeedId(-1), itsName(feed), itsTrckbeam(trckbeam), itsRxName(""),
      itsSRFeedPtr(0), itsNif(0), itsIFPtr(1), itsFrozen(False), itsPolType(4), 
      itsRecptNames(4), itsXeloffset(xeloffset), itsEloffset(eloffset), 
      itsFitsFeedId(atoi(feed.chars())), itsSRFitsFeedId(-1)
{
    // need to sort out which is the other feed
    if (srfeed1==srfeed2 || srfeed1 <= 0 || srfeed2 <= 0) {
	// no switching involved
	itsSRFitsFeedId = itsFitsFeedId;
    } else {
	if (srfeed1 == itsFitsFeedId) {
	    itsSRFitsFeedId = srfeed2;
	} else {
	    // just assume its srfeed1
	    itsSRFitsFeedId = srfeed1;
	}
    }
}

GBTFeed::GBTFeed(const GBTFeed &other)
    : itsFeedId(other.itsFeedId), itsSRFeedId(other.itsSRFeedId),
      itsName(other.itsName),
      itsTrckbeam(other.itsTrckbeam),
      itsRxName(other.itsRxName),
      itsSRFeedPtr(other.itsSRFeedPtr),
      itsNif(other.itsNif), 
      itsIFPtr(other.itsIFPtr.nelements()),
      itsFrozen(other.itsFrozen), itsPolType(other.itsPolType.nelements()),
      itsRecptNames(other.itsRecptNames),
      itsXeloffset(other.itsXeloffset), itsEloffset(other.itsEloffset),
      itsOffsets(other.itsOffsets.shape()), itsFitsFeedId(other.itsFitsFeedId),
      itsSRFitsFeedId(other.itsSRFitsFeedId)
{
    itsPolType = other.itsPolType;
    itsRecptNames = other.itsRecptNames;
    itsOffsets = other.itsOffsets;
    for (Int i=0;i<itsNif;i++) {
	itsIFPtr[i] = new GBTIF(other.getIF(i));
	AlwaysAssert(itsIFPtr[i], AipsError);
    }
}

GBTFeed::~GBTFeed()
{
    for (Int i=0;i<itsNif;i++) {
	delete itsIFPtr[i];
	itsIFPtr[i] = 0;
    }
    itsNif = 0;
}

GBTFeed &GBTFeed::operator=(const GBTFeed &other)
{
    if (this != &other) {
	// first, clear out the itsIFPtr contents
	for (Int i=0;i<itsNif;i++) {
	    delete itsIFPtr[i];
	    itsIFPtr[i] = 0;
	}
	itsFeedId = other.itsFeedId;
	itsSRFeedId = other.itsSRFeedId;
	itsName = other.itsName;
	itsTrckbeam = other.itsTrckbeam;
	itsRxName = other.itsRxName;
	itsSRFeedPtr = other.itsSRFeedPtr;
	itsNif = other.itsNif;
	itsIFPtr.resize(other.itsIFPtr.nelements());
	for (Int i=0;i<itsNif;i++) {
	    itsIFPtr[i] = new GBTIF(other.getIF(i));
	    AlwaysAssert(itsIFPtr[i], AipsError);
	}
	itsFrozen = other.itsFrozen;
	itsPolType.resize(other.itsPolType.shape());
	itsPolType = other.itsPolType;
	itsRecptNames.resize(other.itsRecptNames.shape());
	itsRecptNames = other.itsRecptNames;
	itsXeloffset = other.itsXeloffset;
	itsEloffset = other.itsEloffset;
	itsOffsets.resize(other.itsOffsets.shape());
	itsOffsets = other.itsOffsets;
	itsFitsFeedId = other.itsFitsFeedId;
	itsSRFitsFeedId = other.itsSRFitsFeedId;
    }
  
    return *this;
}

Bool GBTFeed::addReceptor(const String &pol, const String &receptorName)
{
    Bool result = True;
    if (!itsFrozen) {
	uInt index = 0;
	if (pol == "X") index = 0;
	else if (pol == "Y") index = 1;
	else if (pol == "R") index = 2;
	else if (pol == "L") index = 3;
	if (itsRecptNames[index].length()>0 &&
	    itsRecptNames[index]!= receptorName) {
	    result = False;
	} else {
	    itsPolType[index] = pol;
	    itsRecptNames[index] = receptorName;
	}
    }
    return result;
}

void GBTFeed::freeze()
{
    uInt polCount = 0;
    Int maxPolCount = itsPolType.nelements();
    for (Int i=0;i<maxPolCount;i++) {
	if (itsPolType[i].length() > 0) {
	    polCount++;
	} else {
	    for (Int j=i;j<(maxPolCount-1);j++) {
		itsPolType[j] = itsPolType[j+1];
		itsRecptNames[j] = itsRecptNames[j+1];
	    }
	    maxPolCount--;
	    i--;
	}
    }
    itsPolType.resize(polCount,True);
    itsRecptNames.resize(polCount, True);
    if (polCount > 0) {
	itsOffsets.resize(2,polCount);
	itsOffsets.row(0) = itsXeloffset*C::pi/180.0;
	itsOffsets.row(1) = itsEloffset*C::pi/180.0;
    } // else they should and probably won't be used
    itsFrozen = True;
}

Int GBTFeed::addUniqueIF(const GBTIF &newIF, Bool mergeCorr, Bool bankCheck) 
{
    Int result = -1;
    for (uInt i=0;i<nif();i++) {
	if (newIF==getIF(i)) {
	    result = i;
	    // merge in the correlations from here to there
	    for (uInt j=0;j<newIF.ncorr();j++) {
		Bool found = False;
		for (uInt k=0;k<itsIFPtr[result]->ncorr();k++) {
		    if (mergeCorr) {
			if (itsIFPtr[result]->getCorr(k).mergeCorr(newIF.getCorr(j),
								   bankCheck)) {
			    found = True;
			    break;
			}
		    } else {
			if (itsIFPtr[result]->getCorr(k).mergeCheck(newIF.getCorr(j),
								    bankCheck)) {
			    found = True;
			    break;
			}
		    }
		}
		if (!found) {
		    // mark it not being seen
		    // I think that the way this is used there is no danger of
		    // some mergings being ok and others failing.  Either they all
		    // fail or they all succeed.  Still, I wonder if a trap for that
		    // here wouldn't be a good idea.  Not sure what to do, though
		    // since they can't be un merged at this point and its not
		    // clear what ifrows should or shouldn't be added then.
		    result = -1;
		} else if (!mergeCorr) {
		    // here, mergeCheck returned True, but the correlation hasn't
		    // actually been added.  This is used by the DCR where the
		    // two polarizations are kept separate, but we need to do this
		    // here so that they use the same IF
		    itsIFPtr[result]->addCorr(newIF.getCorr(j));
		}
	    }
	    // add in rows from newIF to this one
	    if (result >= 0) {
		Vector<Int> newRows(newIF.ifrows());
		for (uInt j=0;j<newRows.nelements();j++) {
		    itsIFPtr[result]->addIFRow(newRows[j]);
		}
		break;
	    }
	}
    }
    if (result < 0) {
	// do we need to resize itsIFPtr
	itsNif++;
	result = itsNif-1;
	if (result >= Int(itsIFPtr.nelements())) {
	    itsIFPtr.resize(itsIFPtr.nelements()*2);
	}
	itsIFPtr[result] = new GBTIF(newIF);
	AlwaysAssert(itsIFPtr[result], AipsError);
    }
    return result;
}

#include <casa/Utilities/Copy.tcc>
// Needed to avoid namespace conflicts
template void objcopy<GBTFeed *>(GBTFeed **, GBTFeed * const *, uInt);
template void objset<GBTFeed *>(GBTFeed **, GBTFeed *, uInt);
